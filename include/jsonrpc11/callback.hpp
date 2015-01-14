#pragma once

#include <tuple>
#include <list>
#include <numeric>
#include <functional>

template <std::size_t...>
struct indices;

template <std::size_t N, typename Indices, typename... Types>
struct make_indices_impl;

template <std::size_t N, std::size_t... Indices, typename Type, typename... Types>
struct make_indices_impl<N, indices<Indices...>, Type, Types...>
{
  typedef
  typename make_indices_impl<N+1, indices<Indices...,N>, Types...>::type
      type
  ;
};

template <std::size_t N, std::size_t... Indices>
struct make_indices_impl<N, indices<Indices...>>
{
  typedef indices<Indices...> type;
};

template <std::size_t N, typename... Types>
struct make_indices
{
  typedef typename make_indices_impl<0, indices<>, Types...>::type type;
};


//===========================================================================

template <
    typename Indices
>
struct apply_tuple_impl;

template <
    template <std::size_t...> class I,
    std::size_t... Indices
>
struct apply_tuple_impl<I<Indices...>>
{
  template <
      typename Op,
      typename... OpArgs,
      template <typename...> class T = std::tuple
  >
  static auto apply_tuple(Op&& op, T<OpArgs...>&& t)
  -> typename std::result_of<Op(OpArgs...)>::type
  {
    return op( std::forward<OpArgs>(std::get<Indices>(t))... );
  }
};

template <
    typename Op,
    typename... OpArgs,
    typename Indices = typename make_indices<0, OpArgs...>::type,
    template <typename...> class T = std::tuple
>
auto apply_tuple(Op&& op, T<OpArgs...>&& t)
-> typename std::result_of<Op(OpArgs...)>::type
{
  return apply_tuple_impl<Indices>::apply_tuple(
      std::forward<Op>(op),
      std::forward<T<OpArgs...>>(t)
  );
}

//===========================================================================

//#include <cstddef>
//#include <tuple>
//#include <type_traits>
//#include <utility>
//
//template<size_t N>
//struct Apply {
//  template<typename F, typename T, typename... A>
//  static inline auto apply(F && f, T && t, A &&... a)
//        -> decltype(Apply<N-1>::apply(std::forward<F>(f),
//                    std::forward<T>(t),
//                    std::get<N-1>(std::forward<T>(t)),
//                    std::forward<A>(a)...))
//  {
//    return Apply<N-1>::apply(std::forward<F>(f), std::forward<T>(t),
//        std::get<N-1>(std::forward<T>(t)), std::forward<A>(a)...);
//  }
//};
//
//template<>
//struct Apply<0> {
//  template<typename F, typename T, typename... A>
//  static inline auto apply(F && f, T &&, A &&... a) -> decltype(std::forward<F>(f)(std::forward<A>(a)...))
//  {
//    return std::forward<F>(f)(std::forward<A>(a)...);
//  }
//};
//
//template<typename F, typename T>
//inline auto apply(F && f, T && t) -> decltype(Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t)))
//{
//  return Apply< std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t));
//}


