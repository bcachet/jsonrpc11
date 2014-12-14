#include <catch.hpp>

#include <jsonrpc11.hpp>
#include <iostream>

using namespace jsonrpc11;
using std::string;
using std::initializer_list;

TEST_CASE("Extract method name and parameters from Json-Rpc", "[jsonrpc]") {
  Json::shape echo = {{"jsonrpc", Json::STRING}, {"method", Json::STRING}, {"params", Json::ARRAY}};
  SECTION("Json-Rpc message with method name and params") {
    initializer_list<string> valid_msgs = {
        R"({"jsonrpc": "2.0", "method": "echo", "params": ["Hello JSON-RPC"], "id": 1})",
        R"({"jsonrpc": "2.0", "method": "echo", "params": ["Hello", "JSON-RPC"], "id": 1})",
        R"({"jsonrpc": "2.0", "method": "echo", "params": [], "id": 1})",
    };
    string err;
    std::for_each(valid_msgs.begin(), valid_msgs.end(), [&echo, &err](string msg) {
      auto json_rpc = Json::parse(msg, err);
      REQUIRE(json_rpc.has_shape(echo, err));
    });
  }

  SECTION("Json-Rpc message with method name and params") {
    initializer_list<string> invalid_msgs = {
        R"({"jsonrpc": "2.0", "method": "echo", "id": 1})",
        R"({"jsonrpc": "2.0", "method": "echo", "params": "{"param1" : null}", "id": 1})",
        R"({"method": "echo", "params": ["Hello JSON-RPC"], "id": 1})",
    };
    string err;
    std::for_each(invalid_msgs.begin(), invalid_msgs.end(), [&echo, &err](string msg) {
      auto json_rpc = Json::parse(msg, err);
      REQUIRE_FALSE(json_rpc.has_shape(echo, err));
    });
  }

  SECTION("Bind method to JsonRpc Server") {
    JsonRpcHandler server;
    std::function<Json(Json const&)> add_cb = [](Json const& params)->Json {return params["a"].number_value() + params["b"].number_value();};
    server.bindMethodToCallback("add", {{"a", Json::NUMBER}, {"b", Json::NUMBER}}, add_cb);
    string err;
    Json valid_echo_json_rpc = Json::parse(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})", err);
    JsonRpcResponse result = server.handleMessage(valid_echo_json_rpc);
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    Json invalid_method_name = Json::parse(R"({"jsonrpc": "2.0", "method": "aaddd", "params": {"a": 1, "b": 1}, "id": 1})", err);
    result = server.handleMessage(invalid_method_name);
    REQUIRE(result.get_status() == JsonRpcResponse::METHOD_NOT_FOUND);
    Json invalid_json_rpc_format = Json::parse(R"({"method": "add", "params": {"a": 1, "b": 1}, "id": 1})", err);
    result = server.handleMessage(invalid_json_rpc_format);
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_MSG);
    Json invalid_params = Json::parse(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "unknown": 1}, "id": 1})", err);
    result = server.handleMessage(invalid_params);
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Add with array params") {
    JsonRpcHandler server;
    std::function<Json(Json const&)> add_cb = [](Json const& params)->Json {
        int sum = 0;
        std::for_each(params.array_items().begin(), params.array_items().end(), [&sum](Json const& a) {
            int v = a.int_value();
            sum += v;
        });
        return Json::object {{"result", sum }};
    };
    server.bindMethodToCallback("add", {}, add_cb);
    string err;
    Json valid_echo_json_rpc = Json::parse(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})", err);
    JsonRpcResponse result = server.handleMessage(valid_echo_json_rpc);
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    REQUIRE(Json::parse(result.get_message(), err)["result"].int_value() == 3);
    Json invalid_echo_json_rpc = Json::parse(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})", err);
    result = server.handleMessage(invalid_echo_json_rpc);
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }
}
