#include <catch.hpp>

#include <jsonrpc11/callback.hpp>

#include <list>
#include <numeric>


TEST_CASE("Call function with Tuple", "[callback]") {
  SECTION("Handle multiple number of arguments will call") {

    REQUIRE((int)apply_tuple([](int a, int b) -> int {
      return a + b;
    }, std::make_tuple(2, 2))  == 4);

    REQUIRE((int)apply_tuple([](int a, int b, int c) -> int {
      return a + b + c;
    }, std::make_tuple(2, 2, 2)) == 6);
  }

  SECTION("Handle list as argument") {
    REQUIRE((int)apply_tuple([](std::list<int> values)-> int {
      return std::accumulate(values.begin(), values.end(), 0);
    }, std::make_tuple(std::list<int>({2, 2, 2}))) == 6);

    int res = apply_tuple([](int a, int b) -> int {return a + b;}, std::make_tuple(2, 2));
    REQUIRE(res == 4);
  }
}