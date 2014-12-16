#pragma once

#include <vector>
#include <list>
#include <functional>
#include <algorithm>

#include <json11.hpp>

using namespace json11;

#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

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

  class JsonRpcHandler
  {
  public:
    void register_function(std::string name, Json::shape def, std::function<Json(Json)> cb);
    template <typename T>
    void register_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(std::list<T>const&)> cb);
    void register_function(std::string name, std::function<Json()> cb);

    JsonRpcResponse handle(std::string message);

  private:
    std::map<std::string, std::list<std::shared_ptr<FunctionDefinition>>> methods_;
  };



  template<typename R>
  R get_value(Json p);

  template<typename T>
  class FunctionDefitionWithPositionalParams : public FunctionDefinition {
    Json call_with_params(Json const& params) override
    {
      std::list<T> cb_params;
      std::for_each(params.array_items().begin(), params.array_items().end(), [&cb_params](Json p)
      {
        cb_params.push_back(get_value<T>(p));
      });
      return callback_(cb_params);
    }

    bool validate_params(Json const& params, std::string&) override
    {
      return  params.is_array() && params.array_items().size() > 0 && params.array_items()[0].type() == params_def_.front();
    }

    std::function<Json(std::list<T>const&)> callback_;
    std::list<Json::Type> params_def_;
  public:
    FunctionDefitionWithPositionalParams(std::initializer_list<Json::Type> params_def,
      std::function<Json(std::list<T>const&)> cb) :
      callback_(cb),
      params_def_(params_def)
    {

    }
    virtual ~FunctionDefitionWithPositionalParams() {}
  };

  // TODO: Try to unpack params from Json to -> function arguments
  class FunctionDefinitionWithNamedParams : public FunctionDefinition {
    bool validate_params(Json const& params, std::string& err) override
    {
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
      return callback_(params);
    }

    std::function<Json(Json)> callback_;
    std::list<std::pair<std::string, Json::Type>> params_def_;
  public:
    FunctionDefinitionWithNamedParams(Json::shape def, std::function<Json(Json)> cb) :
      callback_(cb),
      params_def_(def) {}
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
  

  template <typename T>
  void JsonRpcHandler::register_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(std::list<T>const&)> cb)
  {
    methods_[name].push_back(std::make_shared<FunctionDefitionWithPositionalParams<T>>(def, cb));
  }
}


