#pragma once

#include <json11.hpp>
#include <string>

using namespace json11;

namespace jsonrpc11 {
  
  class Id {
  public:
    Id(Json const& id = Json()) : id_(id) {}

    int operator()() const {return id_.int_value();}
    Json to_json() const { return id_ ;}
  private:
    Json id_;
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
    Json id() const;
  };

}