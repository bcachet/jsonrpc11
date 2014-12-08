# jsonrpc11 [![Build Status](https://travis-ci.org/bcachet/jsonrpc11.svg)](https://travis-ci.org/bcachet/jsonrpc11)

JSON-RPC server based on [json11](https://github.com/dropbox/json11) library.

## Goal

```cpp
#include <cstdio>
#include <iostream>

#include <jsonrpc11.hpp>
using namespace jsonrpc11;

const int ESCAPE = 27;

class Point {
public:
    int x;
    int y;
    Point (int x, int y) : x(x), y(y) {}
    Point (Json::object o) : x(o["x"].int_value()), y(o["y"].int_value()) {}
    std::ostream& operator<<(std::ostream& os, const Point& obj) {
      os << "(X: " << x << ", Y: " << y << ")";
    }
};

void draw_circle(Json::object center_as_json, int radius) {
  Point center(center_as_json);
  std::cout << "Draw a circle at position" << center << " with radius: " << radius << std::endl;
}

int main() {
  JsonRpcServer server(TCP, 8080);
  server.bindMethod("circle", {{"center", Json::OBJECT}, {"radius", Json::NUMBER}}, &draw_circle);
  server.start();
  while (std::getchar() != ESCAPE) {
  }
  server.stop();
  return 0;
}
```

You can send order to the server with cURL command:

```sh
curl --data "{\"jsonrpc\":\"2.0\",\"method\":\"circle\",\"id\":1,\"params\":{\"center\":{\"x\": 10, \"y\": 10}, \"radius\": 10}}" localhost:8080
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
