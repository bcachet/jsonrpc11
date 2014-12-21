#pragma once

#include <json11.hpp>
#include <string>

using namespace json11;

namespace jsonrpc11 {
  int const NULL_ID = std::numeric_limits<int>::min();
  class Id {
  public:
    Id(int id = NULL_ID) : id_(id) {}
    explicit Id(Json const& j) : id_(j == Json() ? NULL_ID : j.int_value()) {}

    int operator()() const {return id_;}
    Json to_json() const { return Json(id_ == NULL_ID ? "null" : std::to_string(id_)) ;}
  private:
    int id_;
  };


  class Request {
    Json message_;
    Json parameters_;
    std::string method_;
    std::string version_;
    Id id_;


  public:
    Request(Json const&  r);
    Json to_json() const;

    bool is_valid(std::string &);

    Json parameters() const;
    std::string method() const;
    int id() const;
  };

}