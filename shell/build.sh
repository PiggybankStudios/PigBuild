#!/bin/bash

# Call this script from the folder where build_script.c exists.
#
# This script will create a "build" folder and push into it so all build
#   artifacts will be put into a folder that can be ignored by version control.
#
# If needed you can set PIG_BUILD_ROOT to the pig_build root folder
#   if it is in a non-standard location. If PIG_BUILD_ROOT is not set
#   then we will assume this build.sh exists inside "pig_build/shell" folder
#   and set PIG_BUILD_ROOT to the parent folder.
#
# We use `$(cd ... && pwd)` resolve a relative path to absolute
#
# Order of Operations:
#   1. Determine where this build.sh file lives (BUILD_SH_FOLDER)
#   2. Move up 2 folders if the build.sh was run by double clicking it
#   3. Save the current working directory as PROJECT_ROOT
#   4. Create a "build" folder if it doesn't exist, pushd into it
#   5. Decide on compiler flags for building "build_script.c" into "builder"
#   6. Check that clang is installed and available in PATH as "clang"
#   7. Compile "build_script.c" into "builder" if it doesn't exist
#   8. Run "builder" and save exit code
#   9. If builder returned 42 then compile "build_script.c" into "builder" and run again
#  10. popd out of the "build" folder

# Find this script's directory (works even when source'd from another script).
# BASH_SOURCE[0] always points to this file, unlike $0 which changes when source'd.
# NOTE: We have to do this before we pushd into the "build" folder
BUILD_SH_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# If someone runs this script directly from the pig_build/shell/ folder,
# assume they want to build the project two directories up.
if [ "$(pwd)" = "$BUILD_SH_FOLDER" ]; then
	echo "build.sh was run directly from pig_build/shell/ folder. Moving up to build project in two folders above..."
    cd ../..
fi
# We take the current working directory as the project that we want to build
PROJECT_ROOT="$(pwd)"
if [ ! -f "$PROJECT_ROOT/build_script.c" ]; then
	echo "Couldn't find build_script.c in the current working directory. pig_build requires that the build.sh is run from the project's root directory and that a build_script.c exists in that root folder"
fi
# Make a "build" folder if it doesn't already exist and push into it
mkdir -p build || { echo "Failed to create \"build\" folder"; exit 1; }
pushd build > /dev/null || { echo "Failed to enter \"build\" folder"; exit 1; }

# Default PIG_BUILD_ROOT to this script's parent directory
# (i.e. assume this script lives at <pig_build>/shell/build.sh)
if [ -z "$PIG_BUILD_ROOT" ]; then
	PIG_BUILD_ROOT="$(cd "$BUILD_SH_FOLDER/.." && pwd)"
fi

# The flags are only used when compiling the build_script.c (not for compiling your main program, those flags should be defined inside the build_script.c)
# We use a bash array to properly handle arguments with quotes and spaces
compiler_flags=(
	-std=gnu2x
	-fdiagnostics-absolute-paths
	-O0
	-g
	"-I$PROJECT_ROOT"
	"-I$PIG_BUILD_ROOT/src"
	"-DPIG_BUILD_ROOT=\"$PIG_BUILD_ROOT\""
	-o builder
	"$PROJECT_ROOT/build_script.c"
	$PIG_BUILD_FLAGS
)

# Check if clang is installed
if ! command -v clang &> /dev/null; then
	echo "Clang is not installed. Clang needs to be installed and available as 'clang' in order to compile this project"
	exit 1
fi

compile_builder() {
    rm -fr builder builder_hash.txt builder.dSYM
	echo "[Compiling build_script.c...]"
	if ! clang "${compiler_flags[@]}"; then
		echo "Failed to compile build_script.c into builder!"
		exit 1
	fi
}

if [ ! -e "builder" ] || [ ! -e "builder_hash.txt" ]; then
	# If builder doesn't exist, or the builder_hash.txt doesn't exist, we assume we need to build the builder before running
	compile_builder
fi

# Run the builder and save the exit code so we can handle if it requests a rebuild
builder_exit_code=0
./builder "$@" || builder_exit_code=$?

if [ $builder_exit_code -eq 42 ]; then
	echo "[Recompile requested]"
	compile_builder
	./builder "$@"
fi

# Leave the "build" folder
popd > /dev/null
