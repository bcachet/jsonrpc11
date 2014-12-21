#include <jsonrpc11.hpp>

namespace jsonrpc11 {
  using namespace json11;

  Json Error::to_json() const {
    if (status_ == OK)
      return Json();
    if (data_ == "")
      return Json::object { { "code", status_}, {"message", message_} };
    else
      return Json::object { { "code", status_}, {"message", message_}, {"data", data_} };
  };

  Json Response::to_json() const {
    return Json::object {
        { "jsonrpc", "2.0" },
        { "result", result_["result"] },
        { "id", id_ },
        { "error", error_ }
    };
  }
}