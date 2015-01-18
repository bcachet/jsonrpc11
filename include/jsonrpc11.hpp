#pragma once

#include <list>
#include <functional>
#include <algorithm>

#include <json11.hpp>
using namespace json11;

#include "jsonrpc11/callback.hpp"
#include "jsonrpc11/request.hpp"
#include "jsonrpc11/response.hpp"


#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

namespace jsonrpc11
{
  template<typename R>
  R get_value(Json p);

  template<typename T>
  std::list<T> get_value(Json::array p)
  {
    std::list<T> v;
    std::for_each(p.begin(), p.end(), [&v](Json j)
    {
      v.push_back(get_value<T>(j));
    });
    return v;
  }


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


  template <typename ... T>
  inline std::function<Json(Json)> apply_args(std::function<Json(T...)> cb, std::function<std::tuple<T...>(Json)> parser)
  {
    return [=](Json json_params)
    {
      return apply_tuple<std::function<Json(T...)>, std::tuple<T...>>(cb, parser(json_params));
    };
  }

  inline std::function<bool(Json, std::string&)> validate_positional_params(std::list<Json::Type> def) {
    return [def](Json json_params, std::string &) -> bool {
      std::list<Json::Type> params_types;
      std::transform(json_params.array_items().begin(), json_params.array_items().end(), std::back_inserter(params_types), [](Json p) {
        return p.type();
      });
      return params_types == def;
    };
  };

  inline std::function<bool(Json, std::string&)> validate_params_as_list_of(Json::Type type) {
    return [type](Json json_params, std::string&) -> bool {
      return std::all_of(json_params.array_items().begin(), json_params.array_items().end(), [&type](Json item) -> bool {
        return item.type() == type;
      });
    };
  }

  template <typename ... T>
  inline std::function<std::tuple<T...>(Json)> args_from_positional_params() {
    return [](Json json_params)  -> std::tuple<T...> {
      return parse<T...>(json_params);
    };
  };

  template <typename T>
  inline std::function<std::tuple<std::list<T>>(Json)> args_from_positional_params_as_list() {
    return [](Json json_params)  -> std::tuple<std::list<T>> {
      return parse_to_list<T>(json_params);
    };
  };

  inline std::function<bool(Json, std::string&)> validate_named_params(std::list<std::pair<std::string, Json::Type>> def) {
    return [def](Json json_params, std::string & err) -> bool {
      return std::all_of(def.begin(), def.end(), [&json_params, &err](std::pair<std::string, Json::Type> p_def) -> bool {
        err = "";
        if (json_params[p_def.first].type() == p_def.second)
          return true;
        else {
          err += err.size() > 0 ? " | " : "";
          err += "Invalid type for " + p_def.first + " in " + json_params.dump();
          return false;
        }});
    };
  };

  template <typename ... T>
  inline std::function<std::tuple<T...>(Json)> args_from_named_params(std::list<std::pair<std::string, Json::Type>> def) {
    return [def](Json json_params) -> std::tuple<T...> {
      std::vector<std::string> names;
      std::transform(def.begin(), def.end(), std::back_inserter(names), [](Json::shape::value_type pd) {return pd.first; });
      return parse<T...>(json_params, names);
    };
  };


  struct FunctionDefinition {
    std::function<bool(Json, std::string&)> validate_params;
    std::function<Json(Json)> call_with_params;
  };


  class JsonRpcHandler
  {
  public:

    void register_function(std::string name, FunctionDefinition fd) {
      methods_[name].push_back(fd);
    };

    template<typename ...Args>
    void register_named_params_function(std::string name, Json::shape def, std::function<Json(Args...)> cb) {
      register_function(name, {validate_named_params(def), apply_args(std::move(cb), args_from_named_params<Args...>(def))});
    };

    template <typename ...Args>
    void register_positional_params_function(std::string name, std::initializer_list<Json::Type> def, std::function<Json(Args...)> cb) {
      register_function(name, {validate_positional_params(def), apply_args(std::move(cb), args_from_positional_params<Args...>())});
    };

    template <typename A>
    void register_positional_params_function_with_list(std::string name, Json::Type type, std::function<Json(std::list<A>)> cb) {
      register_function(name, {validate_params_as_list_of(type), apply_args(std::move(cb), args_from_positional_params_as_list<A>())});
    };

    void register_no_params_function(std::string name, std::function<Json()> cb) {
      register_function(name, {[](Json json_params, std::string&) {return json_params.is_null();}, [cb](Json) -> Json {return cb();}});
    };

    Response handle(std::string message);

  private:
    std::map<std::string, std::list<FunctionDefinition>> methods_;
  };
}


