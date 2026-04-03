#!/bin/bash

parentFolder=$(basename "$PWD")
if [ "$parentFolder" = "shell" ]; then
    cd ../..
fi

mkdir -p build
pushd build > /dev/null

# The flags are only used when compiling the build_script.c (not for compiling your main program, those flags should be defined inside the build_script.c)
compiler_flags="-std=gnu2x -fdiagnostics-absolute-paths -O0 -g -I.. -I../pig_build/src -o builder ../build_script.c"

build_and_run() {
    rm -fr builder builder_hash.txt builder.dSYM
	echo "Compiling build_script.c..."
	clang $compiler_flags
	./builder $@
}

if [ ! -e "builder" ] || [ ! -e "builder_hash.txt" ]; then
	build_and_run
else
	set +e
	./builder "$@"
	builder_exit_code="$?"
	set -e
	if [ $builder_exit_code -eq 42 ]; then
		echo "Recompile requested"
		build_and_run
	fi
fi

popd
