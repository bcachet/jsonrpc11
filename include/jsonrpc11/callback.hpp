#pragma once

#include <tuple>
#include <type_traits>
#include <utility>


template<size_t N>
struct Apply {
  template<typename F, typename T, typename... A>
  static inline auto apply_tuple(F f, T t, A ... a) -> decltype(Apply<N - 1>::apply_tuple(f, t, std::get<N - 1>(t), a...)) {
    return Apply<N - 1>::apply_tuple(f, t, std::get<N - 1>(t), a...);
  }
};

template<>
struct Apply<0> {
  template<typename F, typename T, typename... A>
  static inline auto apply_tuple(F f, T, A... a) -> decltype(f(a...)) {
    return f(a...);
  }
};

template<typename F, typename T>
inline auto apply_tuple(F  f, T  t) -> decltype(Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply_tuple(f, t))
{
  return Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply_tuple(f, t);
}