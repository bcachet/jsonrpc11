#include <algorithm>

#include <jsonrpc11.hpp>

namespace jsonrpc11
{
  bool compare(param const& l, param const& r)
  {
    return (l.first == r.first) && (l.second == r.second);
  }

  bool compare(Json::shape const& l, Json::shape const& r)
  {
    if (l.size() == r.size())
    {
      return std::all_of(l.begin(), l.end(), [&r](param p) -> bool {
        return std::any_of(r.begin(), r.end(), [&p](param const & other_p) -> bool {
          return compare(p, other_p);
        });
      });
    }
    return false;
  }
}
