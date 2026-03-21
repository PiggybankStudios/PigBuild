# Pig Build

Rather than putting all of our build logic into Bash or Batch, we have a 2-stage build system that uses a small shell script to build a C program that contains the majority of the build logic and then runs that build program.

The build logic for each project is contained in a `build_script.c` file in the root directory of the project with `build.sh`/`build.bat` shell scripts beside it. However, a bunch of logic and types and other systems are re-usable between projects so this repository contains that re-usable code to help support all C build scripts.

All files in this repo are expected to compile with the default settings for MSVC and Clang so that the shell script doesn't have to know anything about the build script in order to build it.

Additionally, rather than building the `build_script.c` every time we want to build the project we only build it if it either the binary doesn't exist OR if the current binary exits with a special return code saying that it wants to be re-compiled. This allows the `build_script.c` to do whatever work it needs to check it's own source code for changes and request compilation if the code has changed since the last time the script was compiled. Usually this is hashing all the files in this repo and the `build_script.c` and saving the hash to disk so it can compare and detect changes to any of those files' contents.

The code in this repo used to live in [Pig Core](https://github.com/PiggybankStudios/PigCore), for a full history of changes check the commits in that repository. I have opted to pull this logic out for 2 reasons:

1. None of the code that a `build_script.c` depends on should be a part of the thing it is building, since doing so could cause some nasty problems and invalid states.

2. The system is potentially useful for non PigCore projects and possibly useful for other people. This repository aims to be a clear example for anyone looking to write their own C build system.