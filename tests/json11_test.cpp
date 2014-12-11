#include <string>
#include <catch.hpp>
#include <jsonrpc11.hpp>
#include <list>
#include <algorithm>
#include <iostream>
#include <functional>

using namespace jsonrpc11;
using std::string;

const Json circle = Json::object {{"center", Json::object {{"x", 10}, {"y", 20}}},
                                  {"radius", 10}};
const string circle_as_str = R"({"center": {"x": 10, "y": 20}, "radius": 10})";
const Json::shape circle_def =  {{"center", Json::OBJECT},
                          {"radius", Json::NUMBER}};

TEST_CASE("Json object initialisation", "[json11]" ) {
  REQUIRE(circle.dump() == circle_as_str);
  string err = "";
  REQUIRE(Json::parse(circle_as_str, err) == circle);
}

TEST_CASE("Json object as params definition", "[json11]") {
  string err = "";
  REQUIRE(circle.has_shape(circle_def, err));
}

bool compare(Json::shape const&  l, Json::shape const& r) {
  MethodDefinition lp(l), rp(r);
  return lp == rp;
}

TEST_CASE("Json shape can be compared") {
  Json::shape same_def = {{"center", Json::OBJECT}, {"radius", Json::NUMBER}};
  Json::shape same_def_but_mixed = {{"radius", Json::NUMBER}, {"center", Json::OBJECT}};
  Json::shape different_def = {{"center", Json::ARRAY}, {"radius", Json::NUMBER}};
  REQUIRE(compare(same_def, circle_def));
  REQUIRE(compare(same_def_but_mixed, circle_def));
  REQUIRE_FALSE(compare(different_def, circle_def));
}

TEST_CASE("We can find Json shape from a List") {
  MethodDefinition target({ { "center", Json::OBJECT }, { "radius", Json::NUMBER } });
  std::initializer_list<MethodDefinition> shapes = {
    MethodDefinition({ { "center", Json::OBJECT }, { "radius", Json::NUMBER } }),
    MethodDefinition({ { "other", Json::OBJECT } }),
    MethodDefinition({ { "another", Json::OBJECT }, { "wiht2params", Json::NUMBER } })
  };
  REQUIRE(std::find_if( shapes.begin(), shapes.end(), [&target](const MethodDefinition& md) -> bool {return md == target;}) == shapes.begin());
}

class Point {
public:
  int x;
  int y;
  Point(int x, int y) : x(x), y(y) {}
  explicit Point(Json o) : x(o["x"].int_value()), y(o["y"].int_value()) {}
};

TEST_CASE("Json type conversion", "[json11]") {
  string err = "";
  Point p = Point(Json::parse(circle["center"].dump(), err));
  REQUIRE(p.x == 10);
  REQUIRE(p.y == 20);
}
