# jsonrpcpp [![Build Status](https://travis-ci.org/bcachet/jsonrpc11.svg)](https://travis-ci.org/bcachet/jsonrpc11)

JSON-RPC server based on [json11](https://github.com/dropbox/json11) library.

## Goal

```cpp
#include <cstdio>
#include <iostream>
#include <numeric>
#include <list>

#include <jsonrpcpp.hpp>
using namespace jsonrpcpp;

const int ESCAPE = 27;

double add(std::list<double>const& values) {
  return std::accumulate(values.begin(), values.end(), 0.0);
}

std::string say(std::string const& what, int const & times)
{
  std::list<std::string> words = std::string(times, what);
  return std::accumulate(words.begin(), words.end(), std::string());
}

int main() {
  JsonRpcServer server(8080, TCP);
  server.register_method<double, double>("add", {Json::NUMBER}, add);
  server.register_method("say", {{"what", Json::STRING}, {"times", Json::NUMBER}}, std::function<std::string(std::string, int)>(say));
  server.start();
  while (std::getchar() != ESCAPE) {
  }
  server.stop();
  return 0;
}
```

You can send order to the server with cURL command:

```sh
curl --data "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":1,\"params\":[1, 1, 1]}" localhost:8080
```

## Build environment

 We use [cmake](http://cmake.org) to build library and associated tests.

