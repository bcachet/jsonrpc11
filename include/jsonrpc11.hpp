#pragma once

#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <type_traits>

#include <json11.hpp>

#include "jsonrpc11/request.hpp"
#include "jsonrpc11/response.hpp"
#include "jsonrpc11/callback.hpp"

#include <iostream>

using namespace json11;

#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

// TODO: Optional params can be of different types

namespace jsonrpc11
{
  class FunctionDefinition
  {
    
  public:
    virtual ~FunctionDefinition() {}

    virtual bool validate_params(Json const&, std::string&) = 0;
    virtual Json call_with_params(Json const&) = 0;    
  };

  template<typename R>
  R get_value(Json p);

//  template<typename V, typename std::enable_if<std::is_same<V, std::list<typename V::value_type>>::value>::type = 0>
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
  
  template< std::size_t... Ns >
  struct indices
  {
    typedef indices< Ns..., sizeof...( Ns ) > next;
  };

  template< std::size_t N >
  struct make_indices
  {
    typedef typename make_indices< N - 1 >::type::next type;
  };

  template<>
  struct make_indices< 0 >
  {
    typedef indices<> type;
  };

  template<typename... Args, std::size_t... Ns>
  std::tuple<Args...> parse_impl(Json p, indices<Ns...> ) {
    return std::make_tuple(get_value<Args>(p[Ns])...);
  }

  template<typename... Args>
  std::tuple<Args...> parse(Json p) {
    return parse_impl<Args...>( p, typename make_indices<sizeof...(Args)>::type() );
  }

  template<typename T>
  std::tuple<std::list<T>> parse_to_list(Json p) {
    return std::make_tuple(get_value<T>(p.array_items()));
  }

  template<typename... Args, std::size_t... Ns>
  std::tuple<Args...> parse_impl(Json p, std::vector<std::string> n,indices<Ns...> ) {
    return std::make_tuple(get_value<Args>(p[n[Ns]])...);
  }

  template<typename... Args>
  std::tuple<Args...> parse(Json p, std::vector<std::string> n) {
    return parse_impl<Args...>( p, n, typename make_indices<sizeof...(Args)>::type() );
  }



  template<typename ...T>
  class FunctionDefinitionWithPositionalParams : public FunctionDefinition {
    std::vector<Json::Type> params_def_;
    std::function<Json(T...)> callback_;
  public:
    FunctionDefinitionWithPositionalParams(std::initializer_list<Json::Type> params_def, std::function<Json(T...)> cb) :
      params_def_(params_def),
      callback_(cb)
    { }

    virtual bool validate_params(Json const& params, std::string&) override
    {
      if ((!params.is_array()) || (params.array_items().size() != params_def_.size()))
        return false;
      for (size_t i = 0; i < params_def_.size(); ++i)
        if (params_def_[i] != params[i].type())
          return false;
      return true;
    }

    Json call_with_params(Json const& params) override
    {
      return (Json)apply_tuple<std::function<Json(T...)>, std::tuple<T...>>(std::move(callback_), std::move(parse<T...>(params)));
    }
 
  };


  template<typename A>
  class FunctionDefitionWithPositionalParam : public FunctionDefinition {
    std::vector<Json::Type> params_def_;
    std::function<Json(std::list<A>)> callback_;
    Json call_with_params(Json const& params) override
    {
      return (Json)apply_tuple<std::function<Json(std::list<A>)>, std::tuple<std::list<A>>>(std::move(callback_), std::move(parse_to_list<A>(params)));
    }

    bool validate_params(Json const& p, std::string&) override
    {
      Json::array params = p.array_items();
      switch (params_def_.size())
      {
      case 0: return params.size() == 0;
      case 1: 
        return params.size() > 0 && 
               std::all_of(params.begin(), params.end(), [this](Json item) -> bool {
                 return item.type() == params_def_[0];
               });
      default: return false;
      }
    }
  public:
    FunctionDefitionWithPositionalParam(std::initializer_list<Json::Type> params_def, std::function<Json(std::list<A>)> cb) :
      params_def_(params_def),
      callback_(cb)
    { }
  };

  template <typename ...T>
  class FunctionDefinitionWithNamedParams : public FunctionDefinition {
    std::vector<std::pair<std::string, Json::Type>> params_def_;
    std::function<Json(T...)> callback_;
  public:
    bool validate_params(Json const &params, std::string &err) override {
      err = "";
      return std::all_of(params_def_.begin(), params_def_.end(), [&params, &err](std::pair<std::string, Json::Type> p_def) -> bool {
        if (params[p_def.first].type() == p_def.second)
          return true;
        else {
          err += err.size() > 0 ? " | " : "";
          err += "Invalid type for " + p_def.first + " in " + params.dump();
          return false;
        }
      });
    }

    Json call_with_params(Json const& params) override
    {
      std::vector<std::string> names;
      std::for_each(params_def_.begin(), params_def_.end(), [&names](std::pair<std::string, Json::Type> v) {
        names.insert(names.end(), v.first);
      });
      std::tuple<T...> args = parse<T...>(params, names);
      return (Json)apply_tuple<std::function<Json(T...)>, std::tuple<T...>>(std::move(callback_), std::move(args));
    }
  public:

    FunctionDefinitionWithNamedParams(Json::shape def, std::function<Json(T...)> cb) :
      params_def_(def),
      callback_(cb)
    { }
  };


  class FunctionDefinitionWithNoParams : public FunctionDefinition {
    std::function<Json()> callback_;

    bool validate_params(Json const& p, std::string&) override
    {
      return p.type() == Json::NUL;
    }

    Json call_with_params(Json const&) override
    {
      return callback_();
    }

  public:
    FunctionDefinitionWithNoParams(std::function<Json()> cb) :
      callback_(cb)
    { }
  };


  class JsonRpcHandler
  {
  public:

    void register_function(std::string name, std::shared_ptr<FunctionDefinition> fd) {
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
    std::map<std::string, std::list<std::shared_ptr<FunctionDefinition>>> methods_;
  };
}


