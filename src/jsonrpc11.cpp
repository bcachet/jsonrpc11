#include <jsonrpc11.hpp>
#include <iostream>

namespace jsonrpc11
{
  enum ErrorCode { PARSE_ERROR = -32700, INVALID_REQUEST = -32600, METHOD_NOT_FOUND = -32601, INVALID_PARAMS = -32602 };

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

  Json JsonRpcHandler::handle_message(Json& msg) {
    JsonRpcRequest req(std::move(msg));
    std::string err = "";
    if (!req.is_valid())
      return error(req.id, INVALID_REQUEST, "Invalid Request", err);
    if (!(methods_.count(req.method) > 0))
      return error(req.id, METHOD_NOT_FOUND, "Method not found", "Method " + req.method + " not found");
    Json params = req.params;
    auto methods = methods_[req.method];
    auto meth = std::find_if(methods.begin(), methods.end(), [&params, &err](FunctionDefinition& fd) -> bool {
      return fd.validate_params(params, err);
    });
    if (meth == methods.end())
      return error(req.id, INVALID_PARAMS, "Invalid params", err);
    Json result = (*meth).call_with_params(params);
    return req.is_notification() ? Json(nullptr) : success(req.id, result);
  }

  std::string JsonRpcHandler::handle(std::string msg_str)
  {
    std::string parser_err = "";
    Json msg_json = Json::parse(msg_str, parser_err);
    if (parser_err != "")
      return error(Json(), PARSE_ERROR, "Parse error", parser_err).dump();
    if (msg_json.is_array())
    {
      Json::array responses;
      std::for_each(msg_json.array_items().begin(), msg_json.array_items().end(), [this, &responses](Json j) {
        Json result = handle_message(j);
        if (!result.is_null())
          responses.push_back(result);
      });
      return responses.size() == 0 ? "" : Json(responses).dump();
    }
    else
    {
      Json result = handle_message(msg_json);
      return result.is_null() ? "" : result.dump();
    }
  }
}
