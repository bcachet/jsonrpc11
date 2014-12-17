#include <jsonrpc11.hpp>

namespace jsonrpc11
{

  JsonRpcResponse JsonRpcHandler::handle(std::string message)
  {
    std::string err = "";
    Json req = Json::parse(message, err);
    err = "";
    if (!(req.has_shape({{"jsonrpc", Json::STRING}, {"method", Json::STRING}}, err) && (req["jsonrpc"].string_value() == "2.0")))
      return JsonRpcResponse(Json(), JsonRpcResponse::INVALID_MSG, err);
    std::string meth_name = req["method"].string_value();
    if (!(methods_.count(meth_name) > 0))
      return JsonRpcResponse(Json(), JsonRpcResponse::METHOD_NOT_FOUND, "Method " + req["method"].string_value() + " not found");
    err = "";
    Json params = req["params"];
    std::list<std::shared_ptr<FunctionDefinition>>::iterator meth = std::find_if(methods_[meth_name].begin(), methods_[meth_name].end(), [&params, &err](std::shared_ptr<FunctionDefinition> fd) -> bool {
      return fd->validate_params(params, err);
    });
    if (meth == methods_[meth_name].end())
      return JsonRpcResponse(Json(), JsonRpcResponse::INVALID_PARAMS, err);
    return JsonRpcResponse((*meth)->call_with_params(params), JsonRpcResponse::OK);
  }

  template<typename R>
  R get_value(Json p)
  {
    switch (p.type())
    {
    case Json::ARRAY: return static_cast<R>(p.array_items());
    default: return R();
    }
  }

  template<>
  bool get_value(Json p)
  {
    return p.bool_value();
  }

  template<>
  double get_value(Json p)
  {
    return p.number_value();
  }

  template<>
  int get_value(Json p)
  {
    return p.int_value();
  }

  template<>
  std::string get_value(Json p)
  {
    return p.string_value();
  }

  template<>
  Json get_value(Json p)
  {
    return p;
  }

}
