#include <iostream>
#include <numeric>

#include <catch.hpp>

#include <jsonrpc11.hpp>


using namespace jsonrpc11;
using std::string;
using std::initializer_list;


TEST_CASE("Check Json-Rpc message validity", "[jsonrpc]")
{
  JsonRpcHandler server;
  SECTION("Invalid message")
  {
    auto result = server.handle(R"({"method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_MSG);
  }

  SECTION("Invalid method name")
  {
    auto result = server.handle(R"({"jsonrpc": "2.0", "method": "aaddd", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::METHOD_NOT_FOUND);
  }
}


TEST_CASE("Named params with simple types support", "[jsonrpc]")
{
  JsonRpcHandler server;
  std::function<Json(std::string, int)> say = [](std::string what, int times)
  {
    REQUIRE(what == "fu");
    REQUIRE(times == 3);
    return Json();
  };
  server.register_function("say", { { "what", Json::STRING }, { "times", Json::NUMBER } }, say);

  SECTION("Params with invalid type")
  {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": "3"}, "id": 1})");
    REQUIRE(response.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Params with invalid name")
  {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "unknown": 3}, "id": 1})");
    REQUIRE(response.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Valid params")
  {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": 1})");
    REQUIRE(response.get_status() == JsonRpcResponse::OK);
  }
}

TEST_CASE("Named params with complex types support", "[jsonrpc]") {
  JsonRpcHandler server;
  class Talker
  {
    std::string what;
  };

  std::function<Json(Json, int)> say = [](Json talker, int times) {
    REQUIRE(talker["what"].string_value() == "fu");
    REQUIRE(times == 3);
    return Json();
  };
  server.register_function("say", { { "talker", Json::OBJECT }, { "times", Json::NUMBER } }, say);

  SECTION("Params with complex type") {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"talker": { "what": "fu" }, "times": 3}, "id": 1})");
    REQUIRE(response.get_status() == JsonRpcResponse::OK);
  }
}


TEST_CASE("Json-Rpc Handler", "[jsonrpc]") {
  
  SECTION("Positional params method to std::list") {
    JsonRpcHandler server;
    server.register_function<double>("add", { Json::NUMBER }, [](std::list<double>const& values) {
      double result = std::accumulate(values.cbegin(), values.cend(), 0.0, [](double &res, double const& x)
      {
        return res += static_cast<double>(x);
      });
      return Json::object{ { "result", result } };
    });
    JsonRpcResponse result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    REQUIRE(result.get_response()["result"].int_value() == 3);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Positional params method [simple types]") {
    JsonRpcHandler server;
    server.register_function<double, double>("add", { Json::NUMBER, Json::NUMBER }, [](double const& a, double const& b) {
      return Json::object{ { "result", a + b } };
    });
    JsonRpcResponse result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1], "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    REQUIRE(result.get_response()["result"].int_value() == 2);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }

  SECTION("Positional params method [complex types]") {
    JsonRpcHandler server;
    server.register_function<Json, int>("check", { Json::OBJECT, Json::NUMBER }, [](Json const& a, int const& b) {
      REQUIRE(a["a"] == 2);
      REQUIRE(b == 1);
      return Json();
    });
    JsonRpcResponse result = server.handle(R"({"jsonrpc": "2.0", "method": "check", "params": [ {"a": 2}, 1], "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::OK);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "check", "params": { "a": 1, "b": 2 }, "id": 1})");
    REQUIRE(result.get_status() == JsonRpcResponse::INVALID_PARAMS);
  }
}

