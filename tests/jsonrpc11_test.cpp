#include <catch.hpp>

#include <jsonrpc11.hpp>

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
}
