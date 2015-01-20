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

string say(string what, int times)
{
  return concatenate(what, times);
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
    string err = "";
    string res_str = server.handle(req);
    if (resp == "")
      REQUIRE(res_str == "");
    else {
      Json result = Json::parse(res_str, err);
      REQUIRE(err == ""); err = "";
      Json expected = Json::parse(resp, err);
      REQUIRE(err == "");
      REQUIRE(compare_results(result, expected, { "jsonrpc", "id", "result" }));
      REQUIRE(result.object_items().count("error") == expected.object_items().count("error"));
      if (expected.object_items().count("error") > 0)
        REQUIRE(compare_results(result["error"], expected["error"], { "code", "message" }));
      REQUIRE(result.object_items().count("result") == expected.object_items().count("result"));
      REQUIRE(result["result"] == expected["result"]);
    }
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
      server.register_named_params_function("say", { { "talker", Json::OBJECT } }, std::function<string(Json)>([](Json const& talker) -> string {
        Talker t(talker);
        return t.say();
      }));
      check_result_for(
        R"({"jsonrpc": "2.0", "method": "say", "params": {"talker": { "what": "fu", "times": 3}}, "id": 1})",
        R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
    }
    SECTION("Notification do not return Response") {
      check_result_for(R"({"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}})", "");
    }
  }

  SECTION("Positional parameters") {
    SECTION("Parameters of same type T -> list<T>") {
      server.register_positional_params_function_with_list("add", { Json::NUMBER }, std::function<double(std::list<double>)>([](std::list<double>const& values) -> double {
        return std::accumulate(values.cbegin(), values.cend(), 0.0);
      }));
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
      SECTION("Notification do not return Response") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "add", "params": [ 1, 1, 1]})",
          "");
      }
    }

    SECTION("Parameters of different types") {
      server.register_positional_params_function("say", { Json::STRING, Json::NUMBER },
          std::function<string(string, int)>([](string what, int times) -> string {
        return concatenate(what, times);
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
      SECTION("Notification do not return Response") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "say", "params": [ "fu", 3]})",
          "");
      }
    }

    SECTION("Parameters with complex types") {
      server.register_positional_params_function("say", { Json::OBJECT }, std::function<string(Json)>([](Json const& talker) {
        Talker t(talker);
        return t.say();
      }));
      SECTION("Valid request") {
        check_result_for(
          R"({"jsonrpc": "2.0", "method": "say", "params": [ {"what": "fu", "times": 3}], "id": 1})",
          R"({"jsonrpc": "2.0", "result": "fufufu", "id": 1})");
      }
    }
  }

  SECTION("Multicall Support") {
    server.register_named_params_function("say", { { "what", Json::STRING }, { "times", Json::NUMBER } }, std::function<Json(string, int)>(say));
    SECTION("Without Notifications") {
      check_result_for(
        R"([{"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": 1}, {"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": "2"}])",
        R"([{"jsonrpc": "2.0", "result": "fufufu", "id": 1}, {"jsonrpc": "2.0", "result": "fufufu", "id": "2"}])");
    }
    SECTION("Notification result should be omitted") {
      check_result_for(
        R"([{"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}, "id": 1}, {"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}}])",
        R"([{"jsonrpc": "2.0", "result": "fufufu", "id": 1}])");
    }
    SECTION("Batch of notification return nothing") {
      check_result_for(
        R"([{"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}}, {"jsonrpc": "2.0", "method": "say", "params": {"what": "fu", "times": 3}}])",
        "");
    }
  }
}

TEST_CASE("Some functions")
{
  SECTION("Argument parsing") {
    std::tuple<int, int> t = parse<int, int>(Json::array({1, 2}));
    REQUIRE(t == std::make_tuple(1, 2));

    std::tuple<int, int> t2 = parse<int, int>(Json::object({{"a", 1}, {"b",2}}), {"a", "b"});
    REQUIRE(t2 == std::make_tuple(1, 2));

    std::tuple<std::list<int>> t3 = parse_to_list<int>(Json::array({ 1, 2, 3 }));
    REQUIRE(t3 == std::make_tuple(std::list<int>({ 1, 2, 3 })));
  }

  SECTION("Json request validation/parsing")
  {

    Json::shape say_def = { { "what", Json::STRING }, { "times", Json::NUMBER } };
    Json::object say_params = Json::object({ { "what", "fu" }, { "times", 3 } });
    std::string err = "";
    std::function<bool(Json, std::string&)> check_say = validate_named_params(say_def);
    REQUIRE(check_say(say_params, err) == true);
    std::function<std::tuple<string, int>(Json)> extract = args_from_named_params < string, int >(say_def) ;
    REQUIRE(extract(say_params) == std::make_tuple("fu", 3));
    FunctionDefinition fd;
    fd.validate_params = validate_named_params(say_def);
    fd.call_with_params = apply_args(std::function<string(string, int)>(say), args_from_named_params<string, int>(say_def));
    REQUIRE(fd.validate_params(say_params, err) == true);
    REQUIRE(fd.call_with_params(say_params) == Json("fufufu"));
  }
}
