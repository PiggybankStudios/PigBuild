#!/bin/bash

# In pig_build examples we need to set PIG_BUILD_ROOT to a full path (not relative)
# so that the real shell script inside pig_build/shell can know where to find pig_build
# In your own build.sh for your project you can omit all this and the
# pig_build/shell/build.sh will assume pig_build just exists inside a folder called "pig_build"

# Take the path to the current shell script, get the directory part, cd to it, and store the output from `pwd` which should give us a full path
PIG_BUILD_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

PIG_BUILD_ROOT=$PIG_BUILD_ROOT PIG_BUILD_FLAGS=$PIG_BUILD_FLAGS $PIG_BUILD_ROOT/shell/build.sh $@