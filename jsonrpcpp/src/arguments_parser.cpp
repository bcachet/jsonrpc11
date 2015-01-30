#include <jsonrpcpp/arguments_parser.hpp>

namespace jsonrpcpp
{
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