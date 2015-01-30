#!/bin/bash
set -e

pushd "$(dirname "$0")" > /dev/null
  sphinx-build -b html . sphinx
  doxygen Doxyfile
popd


