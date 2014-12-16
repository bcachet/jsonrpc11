# jsonrpc11 [![Build Status](https://travis-ci.org/bcachet/jsonrpc11.svg)](https://travis-ci.org/bcachet/jsonrpc11)

JSON-RPC server based on [json11](https://github.com/dropbox/json11) library.

## Goal

```cpp
#include <cstdio>
#include <iostream>
#include <numeric>
#include <list>

#include <jsonrpc11.hpp>
using namespace jsonrpc11;

const int ESCAPE = 27;

Json add(std::list<double>const& values) {
  double result = std::accumulate(values.cbegin(), values.cend(), 0.0, [](double &res, double const& x)
  {
    return res += static_cast<double>(x);
  });
  return Json::object{ { "result", result } };
}

int main() {
  JsonRpcServer server(8080, TCP);
  server.register_function("add", {Json::NUMBER}, add);
  server.start();
  while (std::getchar() != ESCAPE) {
  }
  server.stop();
  return 0;
}
```

You can send order to the server with cURL command:

```sh
curl --data "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":1,\"params\":[1, 1, 1]" localhost:8080
```

## Build environment

 We use [cmake](http://cmake.org) to build library and associated tests.

 We use [Catch](https://github.com/philsquared/Catch) and [HippoMock](http://www.assembla.com/spaces/hippomocks/) libraries to write our tests and our specs.

 We use [Sphinx](http://sphinx.pocoo.org) to generate our documentation.

### Prepare your build environment

You will need to install cmake and Sphinx tools.

All other tools will be installed inside *thirdparty* directory as soon as you launch the **make prepare** command.

#### Debian/Ubuntu

```bash
apt-get install cmake
apt-get install python-sphinx
make prepare
```
#### Mac OS X

You can easily install needed libraries with [Homebrew](http://mxcl.github.com/homebrew/):

```bash
brew install cmake
brew install pip && pip install sphinx
make prepare
```
