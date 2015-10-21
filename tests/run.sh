#!/bin/bash

# Ensure we fail immediately if any command fails.
set -e

cur_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
output_dir="${cur_dir}/bin"

# Prepare environment if not existing
prepare() {
  mkdir -p ${output_dir}
  pushd ${output_dir} > /dev/null
    cmake -Wno-dev -DCPM_SHOW_HIERARCHY=TRUE ..
  popd
}

build() {
  pushd ${output_dir} > /dev/null
    cmake --build . --target tests
  popd
}

run() {
  ${output_dir}/tests
}

pushd ${cur_dir} > /dev/null
  while getopts "gb" opt; do
      case "$opt" in
      g) prepare
         exit
         ;;
      b) build
         exit
         ;;
      esac
  done
  if [ ! -d ${output_dir} ]; then
    prepare
  fi
  build
  run
popd > /dev/null
