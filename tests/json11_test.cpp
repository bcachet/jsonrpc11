#include <string>
#include <catch.hpp>
#include <json11.hpp>
#include <list>
using namespace json11;
using std::string;

class Point {
public:
  int x;
  int y;
  Point (int x, int y) : x(x), y(y) {}
  Point (Json o) : x(o["x"].int_value()), y(o["y"].int_value()) {}
};

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

TEST_CASE("Json type conversion", "[json11]") {
  string err = "";
  Point p = Point(Json::parse(circle["center"].dump(), err));
  REQUIRE(p.x == 10);
  REQUIRE(p.y == 20);
}

bool compare(Json::shape const&  l, Json::shape const& r) {
  auto sort_params = [](std::pair<std::string, Json::Type> lh,
      std::pair<std::string, Json::Type> rh) {
    return lh.first.compare(rh.first) > 0;
  };
  if (l.size() == r.size()) {
    std::list<std::pair<std::string, Json::Type>> ll(l), rl(r);
    ll.sort(sort_params); rl.sort(sort_params);
    return std::equal(ll.begin(), ll.end(), rl.begin());
  }
  return false;
}

TEST_CASE("Json shape can be compared") {
  Json::shape same_def = {{"center", Json::OBJECT}, {"radius", Json::NUMBER}};
  Json::shape same_def_but_mixed = {{"radius", Json::NUMBER}, {"center", Json::OBJECT}};
  Json::shape different_def = {{"center", Json::ARRAY}, {"radius", Json::NUMBER}};
  REQUIRE(compare(same_def, circle_def));
  REQUIRE(compare(same_def_but_mixed, circle_def));
  REQUIRE_FALSE(compare(different_def, circle_def));
}

