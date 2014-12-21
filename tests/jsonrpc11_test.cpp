#include <iostream>
#include <numeric>

#include <catch.hpp>

#include <jsonrpc11.hpp>


using namespace jsonrpc11;
using std::string;
using std::initializer_list;

class Talker
{
public:
  std::string what;

  Talker(json11::Json const & item) {
    what = item["what"].string_value();
  }
};

TEST_CASE("Check Json-Rpc message validity", "[jsonrpc]")
{
  JsonRpcHandler server;
  SECTION("Invalid message")
  {
    Response const result = server.handle(R"({"method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == INVALID_REQUEST);
  }

  SECTION("Invalid method name")
  {
    auto result = server.handle(R"({"jsonrpc": "2.0", "method": "aaddd", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == METHOD_NOT_FOUND);
  }

  SECTION("Parse error")
  {
    auto result = server.handle(R"({"jsonrpc": "2.0", "method})");
    REQUIRE(Json(result)["error"]["code"].int_value() == PARSE_ERROR);
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
    REQUIRE(Json(response)["error"]["code"].int_value() == INVALID_PARAMS);
  }

  SECTION("Params with invalid name")
  {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "unknown": 3}, "id": 1})");
    REQUIRE(Json(response)["error"]["code"].int_value() == INVALID_PARAMS);
  }

  SECTION("Valid params")
  {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": 1})");
    REQUIRE(Json(response)["error"]["code"].int_value() == OK);
  }
}

TEST_CASE("Named params with complex types support", "[jsonrpc]") {
  JsonRpcHandler server;

  std::function<Json(Json, int)> say = [](Json const& talker, int const& times) {
    Talker t(talker);
    REQUIRE(t.what == "fu");
    REQUIRE(times == 3);
    return Json();
  };
  server.register_function("say", { { "talker", Json::OBJECT }, { "times", Json::NUMBER } }, say);

  SECTION("Params with complex type") {
    auto response = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": {"talker": { "what": "fu" }, "times": 3}, "id": 1})");
    REQUIRE(Json(response)["error"]["code"].int_value() == OK);
  }
}


TEST_CASE("Positional params ", "[jsonrpc]") {
  
  SECTION("Callback with std::list") {
    JsonRpcHandler server;
    server.register_function<double>("add", { Json::NUMBER }, [](std::list<double>const& values) {
      double result = std::accumulate(values.cbegin(), values.cend(), 0.0, [](double &res, double const& x)
      {
        return res += static_cast<double>(x);
      });
      return Json::object{ { "result", result } };
    });
    Response result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == OK);
    REQUIRE(Json(result)["result"].int_value() == 3);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == INVALID_PARAMS);
  }

  SECTION("Callback with simple types") {
    JsonRpcHandler server;
    server.register_function<double, double>("add", { Json::NUMBER, Json::NUMBER }, [](double const& a, double const& b) {
      return Json::object{ { "result", a + b } };
    });
    auto result = Json(server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1], "id": 1})"));
    REQUIRE(result["error"]["code"].int_value() == OK);
    REQUIRE(result["result"].int_value() == 2);
    result = Json(server.handle(R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})"));
    REQUIRE(result["error"]["code"].int_value() == INVALID_PARAMS);
  }

  SECTION("Callback with complex types") {

    JsonRpcHandler server;
    server.register_function<Json, int>("say", { Json::OBJECT, Json::NUMBER }, [](Json const& talker, int const& times) {
      Talker t(talker);
      REQUIRE(t.what == "fu");
      REQUIRE(times == 3);
      return Json();
    });
    Response result = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": [ {"what": "fu"}, 3], "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == OK);
    result = server.handle(R"({"jsonrpc": "2.0", "method": "say", "params": { "what": "fu", "times": 3 }, "id": 1})");
    REQUIRE(Json(result)["error"]["code"].int_value() == INVALID_PARAMS);
  }
}


