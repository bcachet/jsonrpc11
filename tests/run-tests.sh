#!/bin/bash
cd "$(dirname "$0")"

if [ ! -d ./bin ]; then
  mkdir -p ./bin
fi

# Ensure we fail immediately if any command fails.
set -e

pushd ./bin > /dev/null
  cmake -Wno-dev -DCPM_SHOW_HIERARCHY=TRUE ..
  cmake --build . --target tests
  ./tests
popd

