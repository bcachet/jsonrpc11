#pragma once

#include <json11.hpp>
using namespace json11;

#include <list>

template< std::size_t... Ns >
struct indices
{
  typedef indices< Ns..., sizeof...(Ns) > next;
};

template< std::size_t N >
struct make_indices
{
  typedef typename make_indices< N - 1 >::type::next type;
};

template<>
struct make_indices< 0 >
{
  typedef indices<> type;
};

template<typename... Args, std::size_t... Ns>
std::tuple<Args...> parse_impl(Json p, indices<Ns...>) {
  return std::make_tuple(get_value<Args>(p[Ns])...);
}

template<typename... Args>
std::tuple<Args...> parse(Json p) {
  return parse_impl<Args...>(p, typename make_indices<sizeof...(Args)>::type());
}

template<typename T>
std::tuple<std::list<T>> parse_to_list(Json p) {
  return std::make_tuple(get_value<T>(p.array_items()));
}

template<typename... Args, std::size_t... Ns>
std::tuple<Args...> parse_impl(Json p, std::vector<std::string> n, indices<Ns...>) {
  return std::make_tuple(get_value<Args>(p[n[Ns]])...);
}

template<typename... Args>
std::tuple<Args...> parse(Json p, std::vector<std::string> n) {
  return parse_impl<Args...>(p, n, typename make_indices<sizeof...(Args)>::type());
}