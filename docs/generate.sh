#!/bin/bash
set -e

command_exists() {
  hash "$1" 2> /dev/null ;
}

function extract {
  # Use to create restructured text file from documentation contained in
  # source file.
  # Documentation are blocks inside /**md     */
  # Code are inside blocks ```   ```
  mkdir -p src
  filename=$(basename "$1")
  filename="${filename%.*}"
  python extract.py ../$1 > src/${filename}.rst
}

function prepare {
  if ! command_exists sphinx-build ; then
    pip install sphinx sphinx_rtd_theme click
  fi
  if ! command_exists doxygen ; then
    echo
  fi
}

pushd "$(dirname "$0")" > /dev/null
  prepare
  if command_exists sphinx-build ; then
    mkdir -p sphinx
    sphinx-build -b html . sphinx
  fi

  if command_exists doxygen ; then
    mkdir -p doxygen
    doxygen Doxyfile
  fi
popd > /dev/null

