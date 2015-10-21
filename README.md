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

We use [cmake](http://cmake.org) to build library and associated tests. Be
sure you have installed it.

We use [CPM](https://github.com/iauns/cpm) to handle our dependencies. Every
dependency is defined with [CPM_AddModule function](https://github.com/iauns/cpm#cpm-function-reference).
Dependencies informations are stored in ./CMakeLists.txt when needed by our
library or in ./tests/CMakeLists.txt when needed to build our tests.

We use [Catch](https://github.com/philsquared/Catch) library to write our
tests and our specs.


### Build/Run tests

Then we will run tests/run-tests.sh bash script to generate build environment,
build our libraries and its dependencies and run tests.

```bash
./tests/run.sh
```

This command perform 3 steps:
 * Generate build environment thanks to CMake
 * Build library/tests
 * Run tests

1st step is perform once, when no build environment exist yet. You can force
its execution with "-g" flag

```sh
./tests/run.sh -g
```

## Generate documentation

We use [Sphinx](http://sphinx.pocoo.org) and Doxygen to generate our documentation.

```bash
./docs/generate.sh
```

Documentation written inside `/**rst      */` block are converted to RST file that can
be added to Sphinx documentation.


You will need a Python environment in order to Sphinx command. If not present,
Sphinx will be installed using [PIP](https://pip.pypa.io/en/stable/) package manager.

