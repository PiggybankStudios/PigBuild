#!/bin/bash

# These shell scripts in the root folder are only for convenience when working on PigBuild examples
# The real scripts you want to call for any other application are in the shell/ folder

pushd examples/a_simple_example
# pushd examples/b_incremental_builds
# pushd examples/c_build_gui
# pushd examples/unit_tests
./build.sh
popd
