#include <string>
#include <catch.hpp>
#include <json11.hpp>

using namespace json11;
using std::string;

TEST_CASE("Json object initialisation", "[json11]" ) {
  Json circle = Json::object {{"center", Json::object {{"x", 10}, {"y", 10}}},
                              {"radius", 10}};
  const string circle_as_str = R"({"center": {"x": 10, "y": 10}, "radius": 10})";
  REQUIRE( circle.dump() == circle_as_str);
}

TEST_CASE("Json object as params definition", "[json11]") {
  Json::shape def = {{"center", Json::OBJECT},
                     {"radius", Json::NUMBER}};
  const string params_as_str = R"({"center": {"x": 10, "y": 10}, "radius": 10})";
  string err = "";
  Json obj = Json::parse(params_as_str, err);
  REQUIRE(obj.has_shape(def, err));
}

class Point {
public:
  int x;
  int y;
  Point (int x, int y) : x(x), y(y) {}
  Point (Json o) : x(o["x"].int_value()), y(o["y"].int_value()) {}
};

TEST_CASE("Json type conversion", "[json11]") {
  const string point_as_str = R"({"x": 10, "y": 20})";
  string err = "";
  Json point_as_json = Json::parse(point_as_str, err);
  Point p = Point(point_as_json);
  REQUIRE(p.x == 10);
  REQUIRE(p.y == 20);
}
