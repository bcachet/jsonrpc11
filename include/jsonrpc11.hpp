#pragma once

#include <vector>
#include <list>
#include <functional>

#include <json11.hpp>

using namespace json11;

namespace jsonrpc11
{
  class JsonRpcResponse {
  public:
    enum Status {OK, METHOD_NOT_FOUND, INVALID_PARAMS, INVALID_MSG};

    JsonRpcResponse(Status s, std::string m) : status_(s), message_(m) {};

    Status get_status() {return status_;}
    std::string get_message() {return message_;}
  private:
    Status status_;
    std::string message_;
  };



  class JsonRpcHandler {
  public:
    // TODO: 3 kind of methods need to be handled ([Object, Array, Null])
    // TODO: Try to generate Callback definition from MethodDefinition
    typedef std::function<Json(Json const&)> Callback;
    typedef std::list<std::pair<std::string, Json::Type>> MethodDefinition;
    // TODO: bindMethodToCallback should not allow null Callback
    void bindMethodToCallback(std::string name, MethodDefinition definition, Callback cb);

    // TODO: handleMessage should handle stream instead of Json object
    JsonRpcResponse handleMessage(Json const&  msg);
  private:
    std::map<std::string, std::list<std::pair<MethodDefinition, Callback>>> methods_;
  };

  bool compare(Json::shape const& l, Json::shape const& r);
}


