#include <algorithm>

#include <jsonrpc11.hpp>
#include <iostream>

using std::pair;
using std::string;

namespace jsonrpc11
{
  bool compare(pair<string, Json::Type> const& l, pair<string, Json::Type> const& r)
  {
    return (l.first == r.first) && (l.second == r.second);
  }

  bool compare(Json::shape const& l, Json::shape const& r)
  {
    return (l.size() == r.size()) && std::all_of(l.begin(), l.end(), [&r](pair<string, Json::Type> p) -> bool {
      return std::any_of(r.begin(), r.end(), [&p](pair<string, Json::Type> const & other_p) -> bool {
        return compare(p, other_p);
      });
    });
  }

  void JsonRpcHandler::bindMethodToCallback(string name, MethodDefinition definition, Callback cb) {
    methods_[name].push_back(std::make_pair(definition, cb));
  }


  bool validate_params(JsonRpcHandler::MethodDefinition def, Json params) {
    if (params.is_array()) {
      return def.size() == 0;
    }
    if (params.is_object()) {
      for (auto & item : def) {
        if (params[item.first].type() != item.second) {
          return false;
        }
      }
      return def.size() > 0;
    }
    if (params.is_null()) {
      return def.size() == 0;
    }
    return false;
  }

  JsonRpcResponse JsonRpcHandler::handleMessage(Json const& msg) {
    string err;

    Json::shape const rpc_call = {{"jsonrpc", Json::STRING}, {"method", Json::STRING}};
    if (!msg.has_shape(rpc_call, err)) {
      return JsonRpcResponse(JsonRpcResponse::INVALID_MSG, "");
    }
    string method = msg["method"].string_value();
    if (methods_.count(method) == 0) {
      return JsonRpcResponse(JsonRpcResponse::METHOD_NOT_FOUND, "");
    }
    auto registered_method_defs = methods_[method];
    Json params = msg["params"];

    auto cb = std::find_if(registered_method_defs.cbegin(), registered_method_defs.cend(), [&params](pair<MethodDefinition, Callback>const& def) -> bool
      {
        return validate_params(def.first, params);
      });
    if (cb == registered_method_defs.end()) {
      return JsonRpcResponse(JsonRpcResponse::INVALID_PARAMS, "");
    }
    Json result = cb->second(params);
    return JsonRpcResponse(JsonRpcResponse::OK, result.dump());
  }
}
