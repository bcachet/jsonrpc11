#include <iostream>
#include <numeric>

#include <catch.hpp>

#include <jsonrpc11.hpp>


using namespace jsonrpc11;
using std::string;
using std::initializer_list;

Json add(std::list<double>const& values) {
  double result = std::accumulate(values.cbegin(), values.cend(), 0.0, [](double &res, double const& x)
  {
    return res += static_cast<double>(x);
  });
  return Json::object{ { "result", result } };
}

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
    server.register_function("add", {{"a", Json::NUMBER}, {"b", Json::NUMBER}}, add_cb);
    JsonRpcResponse result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "aaddd", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::METHOD_NOT_FOUND);
    result = server.handle(R"({"method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_MSG);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "unknown": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Add with array params") {
    JsonRpcHandler server;
    server.register_function<double>("add", { Json::NUMBER }, add);
    JsonRpcResponse result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    REQUIRE(result.get_response()["result"].int_value() == 3);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Create FunctionDefinition obj")
  {
    std::string err;
    FunctionDefinition* fd = new FunctionDefitionWithPositionalParams<double>({ Json::NUMBER }, add);
    Json result = fd->call_with_params(Json::parse(R"([1, 1, 1])", err));
    REQUIRE((int)(result["result"].number_value()) == 3);
  }
}
