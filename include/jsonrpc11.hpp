#ifndef JSONRPC11_H_
#define JSONRPC11_H_

#include <json11.hpp>
using namespace json11;

#include <functional>
#include <list>

namespace jsonrpc11
{
  typedef std::pair<std::string, Json::Type> param;
  class MethodDefinition : public std::list<param>
  {
  public:
    explicit MethodDefinition(Json::shape const & s);
    bool operator==(MethodDefinition const & other);

  private:
    std::function<bool(param, param)> _sort_by_param_name;
  };
}

#endif

