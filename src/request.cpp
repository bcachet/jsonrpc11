#include <jsonrpc11/request.hpp>

namespace jsonrpc11 {
  using namespace json11;

  Request::Request(Json const& r) : message_(r), parameters_(r["params"]), method_(r["method"].string_value()),
                                    version_(r["jsonrpc"].string_value()), id_(r["id"])
  { }

  Json Request::to_json() const {
    return Json::object { {"jsonrpc", version_}, {"parameters", parameters()}, {"method", method()}, {"id", id_ }};
  }

  bool Request::is_valid(std::string &err) {
    Json::shape rpc = { {"jsonrpc", Json::STRING}, {"method", Json::STRING} };
    if (!message_.has_shape(rpc, err))
      return false;
    if (version_ != "2.0") {
      err = "Invalid version: " + version_;
      return false;
    }
    return true;
  }

  Json Request::parameters() const {
    return parameters_;
  };

  std::string Request::method() const {
    return method_;
  }

  Json Request::id() const {
    return id_();
  }
}