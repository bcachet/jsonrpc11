#pragma once

#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <type_traits>

#include <json11.hpp>

using namespace json11;

#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

// TODO: Optional params can be of different types

namespace jsonrpc11
{
  class JsonRpcResponse {
  public:
    enum Status {OK, METHOD_NOT_FOUND, INVALID_PARAMS, INVALID_MSG};

    JsonRpcResponse(Json resp, Status s = OK, std::string m = "") : status_(s), message_(m), response_(resp) {};

    Status get_status() {return status_;}
    std::string get_message() {return message_;}
    Json get_response() { return response_; }
  private:
    Status status_;
    std::string message_;
    Json response_;
  };

  class FunctionDefinition
  {
    
  public:
    virtual ~FunctionDefinition() {}

    virtual bool validate_params(Json const&, std::string&) = 0;
    virtual Json call_with_params(Json const&) = 0;    
  };

  template<typename R>
  R get_value(Json p);

  template<typename ...T>
  class FunctionDefitionWithPositionalParams : public FunctionDefinition {
  };

  template<>
  class FunctionDefitionWithPositionalParams<> : public FunctionDefinition{
    bool validate_params(Json const& params, std::string&) override
    {
      if ((!params.is_array()) || (params.array_items().size() != params_def_.size()))
        return false;
      for (size_t i = 0; i < params_def_.size(); ++i)
        if (params_def_[i] != params[i].type())
          return false;
      return true;
    }
  protected:
    std::vector<Json::Type> params_def_;
  public:
    FunctionDefitionWithPositionalParams(std::initializer_list<Json::Type> def) : params_def_(def) {}
  };

  template<typename A>
  class FunctionDefitionWithPositionalParams<A> : public FunctionDefitionWithPositionalParams<> {
    Json call_with_params(Json const& params) override
    {
      return callback_(get_value<A>(params[0]));
    }

    std::function<Json(A)> callback_;
  public:
    FunctionDefitionWithPositionalParams(std::initializer_list<Json::Type> params_def, std::function<Json(A)> cb) :
      FunctionDefitionWithPositionalParams<>(params_def),
      callback_(cb)
    {

    }
    virtual ~FunctionDefitionWithPositionalParams() {}
  };

  template<typename A, typename B>
  class FunctionDefitionWithPositionalParams<A, B> : public FunctionDefitionWithPositionalParams<>{
    Json call_with_params(Json const& params) override
    {
      return callback_(get_value<A>(params[0]), get_value<B>(params[1]));
    }

    std::function<Json(A, B)> callback_;
  public:
    FunctionDefitionWithPositionalParams(std::initializer_list<Json::Type> params_def, std::function<Json(A, B)> cb) :
      FunctionDefitionWithPositionalParams<>(params_def),
      callback_(cb)
    {

    }
    virtual ~FunctionDefitionWithPositionalParams() {}
  };

  template<typename A>
  class FunctionDefitionWithPositionalParam : public FunctionDefinition {
    Json call_with_params(Json const& params) override
    {
      std::list<A> cb_params;
      std::for_each(params.array_items().begin(), params.array_items().end(), [&cb_params](Json p)
      {
        cb_params.push_back(get_value<A>(p));
      });
      return callback_(cb_params);
    }

    bool validate_params(Json const& params, std::string&) override
    {
      switch (params_def_.size())
      {
      case 0: return params.array_items().size() == 0;
      case 1:return params.array_items().size() > 0 && std::all_of(params.array_items().begin(), params.array_items().end(), [this](Json item) -> bool
      {
        return item.type() == params_def_[0];
      });
      default: return false;
      }
    }
    std::vector<Json::Type> params_def_;
    std::function<Json(std::list<A>const&)> callback_;
  public:
    FunctionDefitionWithPositionalParam(std::initializer_list<Json::Type> params_def, std::function<Json(std::list<A>)> cb) :
      params_def_(params_def),
      callback_(cb)
    {

    }
    virtual ~FunctionDefitionWithPositionalParam() {}
  };

  template <typename ...T>
  class FunctionDefinitionWithNamedParams : public FunctionDefinition {

  };

  template <>
  class FunctionDefinitionWithNamedParams<> : public FunctionDefinition {
    bool validate_params(Json const &params, std::string &err) override {
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
  protected:
    std::vector<std::pair<std::string, Json::Type>> params_def_;
  public:
    FunctionDefinitionWithNamedParams(Json::shape def) : params_def_(def) {}
  };

  template <typename A>
  class FunctionDefinitionWithNamedParams<A> : public FunctionDefinitionWithNamedParams<> {

    Json call_with_params(Json const& params) override
    {
      return callback_(get_value<A>(params[params_def_[0].first]));
    }

    std::function<Json(A)> callback_;

  public:
    FunctionDefinitionWithNamedParams(Json::shape def, std::function<Json(A)> cb) :
      FunctionDefinitionWithNamedParams<>(def),
      callback_(cb) {}
    virtual ~FunctionDefinitionWithNamedParams() {}
  };


  template <typename A, typename B>
  class FunctionDefinitionWithNamedParams<A, B> : public FunctionDefinitionWithNamedParams<> {

    Json call_with_params(Json const& params) override
    {
      return callback_(get_value<A>(params[params_def_[0].first]), get_value<B>(params[params_def_[1].first]));
    }

    std::function<Json(A, B)> callback_;

  public:
    FunctionDefinitionWithNamedParams(Json::shape def, std::function<Json(A, B)> cb) :
        FunctionDefinitionWithNamedParams<>(def),
        callback_(cb) {}
    virtual ~FunctionDefinitionWithNamedParams() {}
  };

  class FunctionDefinitionWithNoParams : public FunctionDefinition {
    bool validate_params(Json const& p, std::string&) override
    {
      return p.type() == Json::NUL;
    }

    Json call_with_params(Json const&) override
    {
      return callback_();
    }
    std::function<Json()> callback_;
  public:
    FunctionDefinitionWithNoParams(std::function<Json()> cb) :
      callback_(cb) {}
    virtual ~FunctionDefinitionWithNoParams() {}
  };


  class JsonRpcHandler
  {
  public:
    template<typename A>
    void register_function(std::string name, Json::shape def, std::function<Json(A)> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefinitionWithNamedParams<A>>(def, cb));
    };

    template<typename A, typename B>
    void register_function(std::string name, Json::shape def, std::function<Json(A, B)> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefinitionWithNamedParams<A, B>>(def, cb));
    };

    template <typename A>
    void register_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(A)> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefitionWithPositionalParams<A>>(def, cb));
    };

    template <typename A, typename B>
    void register_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(A, B)> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefitionWithPositionalParams<A, B>>(def, cb));
    };

    template <typename T>
    void register_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(std::list<T>)> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefitionWithPositionalParam<T>>(def, cb));
    };

    void register_function(std::string name, std::function<Json()> cb) {
      methods_[name].push_back(std::make_shared<FunctionDefinitionWithNoParams>(cb));
    };

    JsonRpcResponse handle(std::string message);

  private:
    std::map<std::string, std::list<std::shared_ptr<FunctionDefinition>>> methods_;
  };
}


