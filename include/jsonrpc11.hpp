#ifndef JSONRPC11_H_
#define JSONRPC11_H_

#include <json11.hpp>
using namespace json11;

#include <functional>
#include <list>

namespace jsonrpc11
{
  typedef std::pair<std::string, Json::Type> param;

  bool compare(Json::shape const& l, Json::shape const& r);
}



#endif

