#pragma once

#include <list>
#include <functional>
#include <algorithm>

#include <json11.hpp>
using namespace json11;

#include "jsonrpc11/callback.hpp"
#include "jsonrpc11/arguments.hpp"
#include "jsonrpc11/request.hpp"
#include "jsonrpc11/response.hpp"


#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

namespace jsonrpc11
{
  template<typename R>
  R get_value(Json p);

  template<typename T>
  std::list<T> get_value(Json::array p)
  {
    std::list<T> v;
    std::for_each(p.begin(), p.end(), [&v](Json j)
    {
      v.push_back(get_value<T>(j));
    });
    return v;
  }

  template <typename ... T>
  inline std::function<Json(Json)> apply_args(std::function<Json(T...)>&& cb, std::function<std::tuple<T...>(Json)>&& parser)
  {
    return [&](Json json_params)
    {
      return apply_tuple<std::function<Json(T...)>, std::tuple<T...>>(std::move(cb), parser(json_params));
    };
  }

  inline std::function<bool(Json, std::string&)> validate_positional_params(std::list<Json::Type> def) {
    return [def](Json json_params, std::string &) -> bool {
      std::list<Json::Type> params_types;
      std::transform(json_params.array_items().begin(), json_params.array_items().end(), std::back_inserter(params_types), [](Json p) {
        return p.type();
      });
      return params_types == def;
    };
  };

  template <typename ... T>
  inline std::function<std::tuple<T...>(Json)> args_from_positional_params() {
    return [](Json json_params)  -> std::tuple<T...> {
      return parse<T...>(json_params);
    };
  };

  inline std::function<bool(Json, std::string&)> validate_named_params(std::list<std::pair<std::string, Json::Type>> def) {
    return [def](Json json_params, std::string & err) -> bool {
      return std::all_of(def.begin(), def.end(), [&json_params, &err](std::pair<std::string, Json::Type> p_def) -> bool {
        err = "";
        if (json_params[p_def.first].type() == p_def.second)
          return true;
        else {
          err += err.size() > 0 ? " | " : "";
          err += "Invalid type for " + p_def.first + " in " + json_params.dump();
          return false;
        }});
    };
  };

  template <typename ... T>
  inline std::function<std::tuple<T...>(Json)> args_from_named_params(std::list<std::pair<std::string, Json::Type>> def) {
    return [def](Json json_params) -> std::tuple<T...> {
      std::vector<std::string> names;
      std::transform(def.begin(), def.end(), std::back_inserter(names), [](Json::shape::value_type pd) {return pd.first; });
      return parse<T...>(json_params, names);
    };
  };

  class IFunctionDefinition
  {
  public:
    virtual bool validate_params(Json const& , std::string& ) = 0;
    virtual Json call_with_params(Json const& ) = 0;
  };

  template<typename ...T>
  class FunctionDefinitionWithPositionalParams : public IFunctionDefinition {
    std::list<Json::Type> params_def_;
    std::function<Json(T...)> callback_;
  public:
    FunctionDefinitionWithPositionalParams(std::initializer_list<Json::Type> params_def, std::function<Json(T...)> cb) :
      params_def_(params_def),
      callback_(cb)
    { }

    virtual bool validate_params(Json const& params, std::string& err) override {
      return validate_positional_params(params_def_)(params, err);
    }

    Json call_with_params(Json const& params) override {
      return apply_args<T...>(std::move(callback_), std::move(args_from_positional_params<T...>()))(params);
    }
  };

  template<typename A>
  class FunctionDefitionWithPositionalParam : public IFunctionDefinition {
    std::vector<Json::Type> params_def_;
    std::function<Json(std::list<A>)> callback_;
  public:
    FunctionDefitionWithPositionalParam(std::initializer_list<Json::Type> params_def, std::function<Json(std::list<A>)> cb) :
      params_def_(params_def), callback_(cb)
    { }

    Json call_with_params(Json const& params) override {
      return apply_tuple<std::function<Json(std::list<A>)>, std::tuple<std::list<A>>>(std::move(callback_), std::move(parse_to_list<A>(params)));
    }

    bool validate_params(Json const& p, std::string&) override {
      Json::array params = p.array_items();
      switch (params_def_.size()) {
        case 0: return params.size() == 0;
        case 1: 
          return params.size() > 0 && 
                 std::all_of(params.begin(), params.end(), [this](Json item) -> bool {
                   return item.type() == params_def_[0];
                 });
        default: return false;
      }
    }
  };


  template <typename ...T>
  class FunctionDefinitionWithNamedParams : public IFunctionDefinition {
    std::list<std::pair<std::string, Json::Type>> params_def_;
    std::function<Json(T...)> callback_;
  public:
    FunctionDefinitionWithNamedParams(Json::shape def, std::function<Json(T...)> cb) :
      params_def_(def),
      callback_(cb)
    { }

    bool validate_params(Json const &params, std::string &err) override {
      return validate_named_params(params_def_)(params, err);
    }

    Json call_with_params(Json const& params) override
    {
      return apply_args<T...>(std::move(callback_), std::move(args_from_named_params<T...>(params_def_)))(params);
    }
  };


  class FunctionDefinitionWithNoParams : public IFunctionDefinition {
    std::function<Json()> callback_;
  public:
    FunctionDefinitionWithNoParams(std::function<Json()> cb) :
      callback_(cb)
    { }

    bool validate_params(Json const& p, std::string&) override {
      return p.type() == Json::NUL;
    }

    Json call_with_params(Json const&) override {
      return callback_();
    }
  };


  class JsonRpcHandler
  {
  public:

    void register_function(std::string name, std::shared_ptr<IFunctionDefinition> fd) {
      methods_[name].push_back(fd);
    };

    template<typename ...Args>
    void register_named_params_function(std::string name, Json::shape def, std::function<Json(Args...)> cb) {
      register_function(name, std::make_shared<FunctionDefinitionWithNamedParams<Args...>>(def, std::function<Json(Args...)>(cb)));
    };

    template <typename ...Args>
    void register_positional_params_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(Args...)> cb) {
      register_function(name, std::make_shared<FunctionDefinitionWithPositionalParams<Args...>>(def, cb));
    };

    template <typename A>
    void register_positional_params_function_with_list(std::string name, std::initializer_list<Json::Type> def, std::function<Json(std::list<A>)> cb) {
      register_function(name, std::make_shared<FunctionDefitionWithPositionalParam<A>>(def, cb));
    };

    void register_no_params_function(std::string name, std::function<Json()> cb) {
      register_function(name, std::make_shared<FunctionDefinitionWithNoParams>(cb));
    };

    Response handle(std::string message);

  private:
    std::map<std::string, std::list<std::shared_ptr<IFunctionDefinition>>> methods_;
  };
}


