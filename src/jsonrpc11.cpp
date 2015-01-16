#include <jsonrpc11.hpp>

namespace jsonrpc11
{

  Response JsonRpcHandler::handle(std::string message)
  {
    std::string err = "";
    Request req(Json::parse(message, err));
    if (err != "")
      return Response(Json(), Error(PARSE_ERROR, "Parse error", err), Id());
    if (!req.is_valid(err))
      return Response(Json(), Error(INVALID_REQUEST, "Invalid Request", err), req.id());
    if (!(methods_.count(req.method()) > 0))
      return Response(Json(), Error(METHOD_NOT_FOUND, "Method not found", "Method " + req.method() + " not found"), req.id());
    Json params = req.parameters();
    auto methods = methods_[req.method()];
    auto meth = std::find_if(methods.begin(), methods.end(), [&params, &err](std::shared_ptr<IFunctionDefinition> fd) -> bool {
      return fd->validate_params(params, err);
    });
    if (meth == methods.end())
      return Response(Json(), Error(INVALID_PARAMS, "Invalid params", err), req.id());
    return Response((*meth)->call_with_params(params), OK, req.id());
  }


  template<typename R>
  R get_value(Json p)
  {
    return R(p);
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
