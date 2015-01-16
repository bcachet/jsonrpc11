#include <iostream>
#include <numeric>

#include <catch.hpp>

#include <jsonrpc11.hpp>


using namespace jsonrpc11;
using std::string;

string concatenate(string what, int times) {
  std::list<string> words(times, what);
  return std::accumulate(words.begin(), words.end(), string());
}

Json say(string what, int times)
{
  return Json::object{ { "result", concatenate(what, times) } };
}

class Talker {
public:
  string what;
  int times;

  Talker(json11::Json const & item) {
    what = item["what"].string_value();
    times = item["times"].int_value();
  }

  string say() {
    return concatenate(what, times);
  }
};

bool compare_results(Json left, Json right, std::initializer_list<string> keys) {
  return std::all_of(keys.begin(), keys.end(), [&left, &right](string key) -> bool {
    if (left[key] != right[key])
      std::cout << "Inequality for \"" + key + "\" key: " << left[key].dump() << " | " << right[key].dump() << std::endl;
    return left[key] == right[key];
  });
}

TEST_CASE("Json-Rpc request handling", "[jsonrpc]") {
  JsonRpcHandler server;
  auto check_result_for = [&server](string req, string resp) {
    string err;
    Json result = Json(server.handle(req));
    Json expected = Json::parse(resp, err);
    REQUIRE(compare_results(result, expected, { "jsonrpc", "id", "result" }));
    REQUIRE(result.object_items().count("error") == expected.object_items().count("error"));
    if (expected.object_items().count("error") > 0)
      REQUIRE(compare_results(result["error"], expected["error"], { "code", "message" }));
    REQUIRE(result.object_items().count("result") == expected.object_items().count("result"));
    REQUIRE(result["result"] == expected["result"]);
  };

  SECTION("Check Json-Rpc Request validity") {
    SECTION("Invalid Request") {
      check_result_for(
        R"({"method": "add", "params": {"a": 1, "b": 1}, "id": 1})",
        R"({"jsonrpc": "2.0", "error": {"code": -32600, "message": "Invalid Request"}, "id": 1})");
    }

    SECTION("Method Not Found") {
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "add", "params": {"a": 1, "b": 1}, "id": 1})",
        R"({"jsonrpc": "2.0", "error": {"code": -32601, "message": "Method not found"}, "id": 1})");
    }

    SECTION("Parse Error") {
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "add", "params": )",
        R"({"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null})");
    }
  }

  SECTION("Named parameters") {
    server.register_named_params_function("say",
        { { "what", Json::STRING }, { "times", Json::NUMBER } },
        std::function<Json(string, int)>(say));
    SECTION("Params with invalid type") {
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": "3"}, "id": 1})",
        R"({"jsonrpc": "2.0", "error": {"code": -32602, "message": "Invalid params"}, "id": 1})");
    }

    SECTION("Params with invalid name") {
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "unknown": 3}, "id": 1})",
        R"({"jsonrpc": "2.0", "error": {"code": -32602, "message": "Invalid params"}, "id": 1})");
    }

    SECTION("Valid params") {
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": 1})",
        R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
    }

    SECTION("Method with complex types") {
      server.register_named_params_function("say", { { "talker", Json::OBJECT } }, std::function<Json(Json)>([](Json const& talker) -> Json {
        Talker t(talker);
        return Json::object{ { "result", t.say() } };
      }));
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "say", "params": {"talker": { "what": "fu", "times": 3}}, "id": 1})",
        R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
    }
  }

  SECTION("Positional parameters") {
    SECTION("Parameters of same type T -> list<T>") {
      server.register_positional_params_function_with_list<double>("add", { Json::NUMBER }, [](std::list<double>const& values) {
        return Json::object{ { "result", std::accumulate(values.cbegin(), values.cend(), 0.0) } };
      });
      SECTION("Valid request") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1], "id": 1})",
          R"({"jsonrpc": "2.0", "result": 3, "id": 1})");
      }
      SECTION("Request composed of params of different types") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, "1", true], "id": 1})",
          R"({"jsonrpc": "2.0", "error": {"code": -32602, "message": "Invalid params"}, "id": 1})");
      }
    }

    SECTION("Parameters of different types") {
      server.register_positional_params_function("say", { Json::STRING, Json::NUMBER },
          std::function<Json(string, int)>([](string what, int times) -> Json {
        return Json::object{ { "result", concatenate(what, times) } };
      }));
      SECTION("Valid request") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "say", "params": [ "fu", 3], "id": 1})",
          R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
      }
      SECTION("Request with too much params") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "say", "params": [ "fu", 3, 3], "id": 1})",
          R"({"jsonrpc": "2.0", "error": {"code": -32602, "message": "Invalid params"}, "id": 1})");
      }
    }
    SECTION("Parameters with complex types") {
      server.register_positional_params_function("say", { Json::OBJECT }, std::function<Json(Json)>([](Json const& talker) {
        Talker t(talker);
        return Json::object{ { "result", t.say() } };
      }));
      SECTION("Valid request") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "say", "params": [ {"what": "fu", "times": 3}], "id": 1})",
          R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
      }
    }
  }

  SECTION("Testing Argument parsing") {
    std::tuple<int, int> t = parse<int, int>(Json::array({1, 2}));
    REQUIRE(t == std::make_tuple(1, 2));

    std::tuple<int, int> t2 = parse<int, int>(Json::object({{"a", 1}, {"b",2}}), {"a", "b"});
    REQUIRE(t2 == std::make_tuple(1, 2));

    std::tuple<std::list<int>> t3 = parse_to_list<int>(Json::array({ 1, 2, 3 }));
    REQUIRE(t3 == std::make_tuple(std::list<int>({ 1, 2, 3 })));
  }
}

TEST_CASE("Fun with lambda")
{
  SECTION("Play with lambda")
  {

    Json::shape say_def = { { "what", Json::STRING }, { "times", Json::NUMBER } };
    Json::object say_params = Json::object({ { "what", "fu" }, { "times", 3 } });
    std::string err = "";
    std::function<bool(Json, std::string&)> check_say = validate_named_params(say_def);
    REQUIRE(check_say(say_params, err) == true);
    std::function<std::tuple<string, int>(Json)> extract = args_from_named_params < string, int >(say_def) ;
    REQUIRE(extract(say_params) == std::make_tuple("fu", 3));
  }
}
