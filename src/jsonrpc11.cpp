#include <jsonrpc11.hpp>

namespace jsonrpc11
{
  enum ErrorCode { NO_ERROR, PARSE_ERROR = -32700, INVALID_REQUEST = -32600, METHOD_NOT_FOUND = -32601, INVALID_PARAMS = -32602 };

  class JsonRpcRequest {
  public:
    std::string jsonrpc;
    std::string method;
    Json params;
    Json id;

    JsonRpcRequest(Json&& msg) : jsonrpc(msg["jsonrpc"].string_value()), method(msg["method"].string_value()), params(msg["params"]), id(msg["id"])
    { }

    bool is_valid() {
      return (jsonrpc == "2.0") && (method != "");
    }

    bool is_notification() {
      return id.is_null();
    }
  };

  Json response(Json id, std::pair<std::string, Json> member)
  {
    return Json::object({ { "jsonrpc", "2.0" }, { "id", id }, { member.first, member.second } });
  }

  Json success(Json id, Json result)
  {
    return response(id, std::make_pair("result", result));
  }

  Json error(Json id, ErrorCode code, std::string message, std::string data)
  {
    return response(id, std::make_pair("error", Json::object({ { "code", static_cast<int>(code) }, { "message", message }, { "data", data } })));
  }

  std::string JsonRpcHandler::handle(std::string msg_str)
  {
    std::string err = "";
    JsonRpcRequest req(Json::parse(msg_str, err));
    if (err != "")
      return error(Json(), PARSE_ERROR, "Parse error", err).dump();
    if (!req.is_valid())
      return error(req.id, INVALID_REQUEST, "Invalid Request", err).dump();
    if (!(methods_.count(req.method) > 0))
      return error(req.id, METHOD_NOT_FOUND, "Method not found", "Method " + req.method + " not found").dump();
    Json params = req.params;
    auto methods = methods_[req.method];
    auto meth = std::find_if(methods.begin(), methods.end(), [&params, &err](FunctionDefinition& fd) -> bool {
      return fd.validate_params(params, err);
    });
    if (meth == methods.end())
      return error(req.id, INVALID_PARAMS, "Invalid params", err).dump();
    Json result = (*meth).call_with_params(params);
    return req.is_notification() ? "": success(req.id, result).dump();
  }
}
