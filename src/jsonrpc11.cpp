#include <jsonrpc11.hpp>

namespace jsonrpc11
{

  MethodDefinition::MethodDefinition(Json::shape const& s) :
    std::list<param>(s)
  {
    _sort_by_param_name = [](param lh, param rh) -> bool {
      return lh.first.compare(rh.first) > 0;
    };
    sort(_sort_by_param_name);
  }

  bool MethodDefinition::operator==(MethodDefinition const& other)
  {
    return (size() == other.size()) && std::equal(begin(), end(), other.begin());
  }

}