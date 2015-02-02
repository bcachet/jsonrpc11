#!/bin/bash
cd "$(dirname "$0")"

# Ensure we fail immediately if any command fails.
set -e

# Prepare environment if not existing
prepare() {
  mkdir -p ./bin
  pushd ./bin > /dev/null
    cmake -Wno-dev -DCPM_SHOW_HIERARCHY=TRUE ..
  popd
}

build() {
  pushd ./bin > /dev/null
    cmake --build . --target tests
  popd
}

run() {
  ./bin/tests
}

if [ ! -d ./bin ]; then
  prepare
fi

while getopts "f" opt; do
    case "$opt" in
    f)  prepare
        ;;
    esac
done

build
run
