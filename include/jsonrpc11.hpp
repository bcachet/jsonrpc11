#pragma once

#ifdef _MSC_VER
#pragma warning(disable:4503) // Decorated name length exceeded, name was truncated
#endif

#include <list>
#include <functional>
#include <algorithm>

#include <json11.hpp>
using namespace json11;

#include "jsonrpc11/callback.hpp"
#include "jsonrpc11/arguments_parser.hpp"

namespace jsonrpc11
{
  template <typename Ret, typename ... Args>
  inline std::function<Json(Json)> apply_args(std::function<Ret(Args...)> cb, std::function<std::tuple<Args...>(Json)> parser) {
    return [=](Json json_params) {
      return Json(apply_tuple<std::function<Ret(Args...)>, std::tuple<Args...>>(cb, parser(json_params)));
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

    template<typename Ret, typename ...Args>
    void register_named_params_function(std::string name, Json::shape def, std::function<Ret(Args...)> cb) {
      register_function(name, {validate_named_params(def), apply_args(std::move(cb), args_from_named_params<Args...>(def))});
    };

    template <typename Ret, typename ...Args>
    void register_positional_params_function(std::string name, std::initializer_list<Json::Type> def, std::function<Ret(Args...)> cb) {
      register_function(name, {validate_positional_params(def), apply_args(std::move(cb), args_from_positional_params<Args...>())});
    };

    template <typename Ret, typename A>
    void register_positional_params_function_with_list(std::string name, Json::Type type, std::function<Ret(std::list<A>)> cb) {
      register_function(name, {validate_params_as_list_of(type), apply_args(std::move(cb), args_from_positional_params_as_list<A>())});
    };

    template <typename Ret>
    void register_no_params_function(std::string name, std::function<Ret()> cb) {
      register_function(name, {[](Json json_params, std::string&) {return json_params.is_null();}, [cb](Json) -> Json {return Json(cb());}});
    };

    std::string handle(std::string message);

  private:
    std::map<std::string, std::list<FunctionDefinition>> methods_;
    Json handle_message(Json&);
  };
}


