@echo off

rem These shell scripts in the root folder are only for convenience when working on PigBuild examples
rem The real scripts you want to call for any other application are in the shell/ folder

rem pushd examples\a_simple_example
rem pushd examples\b_incremental_builds
rem pushd examples\c_build_gui

pushd examples\unit_tests
call build.bat
popd
