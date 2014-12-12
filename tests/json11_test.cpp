#include <string>
#include <list>
#include <algorithm>
#include <iostream>
#include <functional>

#include <catch.hpp>

#include <jsonrpc11.hpp>

using namespace jsonrpc11;
using std::string;

const Json circle = Json::object {{"center", Json::object {{"x", 10}, {"y", 20}}}, {"radius", 10}};
const string circle_as_str = R"({"center": {"x": 10, "y": 20}, "radius": 10})";
const Json::shape circle_def = { { "center", Json::OBJECT }, { "radius", Json::NUMBER } };

TEST_CASE("Json object initialisation", "[json11]" ) {
  REQUIRE(circle.dump() == circle_as_str);
  string err = "";
  REQUIRE(Json::parse(circle_as_str, err) == circle);
}

TEST_CASE("Json object as params definition", "[json11]") {
  string err = "";
  REQUIRE(circle.has_shape(circle_def, err));
  Json circle_with_unordered_params = Json::object{ { "radius", 10 }, { "center", Json::object{ { "x", 10 }, { "y", 20 } } } };
  REQUIRE(circle_with_unordered_params.has_shape(circle_def, err));
  Json circle_with_added_params = Json::object{ {"color", 0}, { "radius", 10 }, { "center", Json::object{ { "x", 10 }, { "y", 20 } } } };
  REQUIRE(circle_with_added_params.has_shape(circle_def, err));
  REQUIRE(circle_with_added_params.object_items().size() > circle_def.size());
}

TEST_CASE("We can find Json shape from a List") {
  Json::shape target = { { "center", Json::OBJECT }, { "radius", Json::NUMBER } };
  std::initializer_list<Json::shape> shapes = {
    { { "other", Json::OBJECT } },
    { { "center", Json::OBJECT }, { "radius", Json::NUMBER } },
    { { "another", Json::OBJECT }, { "wiht2params", Json::NUMBER } },
  };
  auto pos = std::find_if(shapes.begin(), shapes.end(), [&target](Json::shape p) -> bool {
    return compare(p, target);
  });
  REQUIRE(pos != shapes.end());
  string err;
  REQUIRE(circle.has_shape(*pos, err));
}

TEST_CASE("Json type conversion", "[json11]") {
  string err = "";
  class Point {
  public:
    int x;
    int y;
    Point(int x, int y) : x(x), y(y) {}
    explicit Point(Json o) : x(o["x"].int_value()), y(o["y"].int_value()) {}
  };
  Point p(Json::parse(circle["center"].dump(), err));
  REQUIRE(p.x == 10);
  REQUIRE(p.y == 20);
}

