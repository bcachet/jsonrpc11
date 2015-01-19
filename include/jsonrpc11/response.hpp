#pragma once

#include <jsonrpc11/request.hpp>

#include <string>

#include <json11.hpp>
using namespace json11;

namespace jsonrpc11 {
  using namespace json11;

  enum Status { OK, PARSE_ERROR = -32700, INVALID_REQUEST = -32600, METHOD_NOT_FOUND = -32601, INVALID_PARAMS = -32602 };

  class Error {
  public:
    Error(Status status = OK, std::string const& message = "", std::string const& data = "") : status_(status), message_(message), data_(data) {}
    Json to_json() const;
  private:
    Status status_;
    std::string message_;
    std::string data_;
  };

  class Response {
  public:
    Response(Json const& res, Error const& error, Id const& id) : id_(id), error_(error), result_(res) {
    }

    Json to_json() const;

  private:
    Id id_;
    Error error_;
    Json result_;
  };
}