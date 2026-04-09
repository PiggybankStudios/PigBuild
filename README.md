# Pig Build

A simple framework for writing build logic in C rather than using another build system that has it's own coniguration langauge. The basic idea is we have a small static shell script that builds and runs a C program that does the actual logic for building the main program. This approach is primarily beneficial to C or C++ projects since the build logic can be written in the same language and compiler as the main project. This reduces your dependencies to *just* a C/C++ compiler and reduces the cognitive burden of anyone who needs to touch or understand the build system. This is an evolution of the build.bat/sh system made popular in [Handmade Hero](https://handmadehero.org/) (also partially inspired by [nob.h](https://github.com/tsoding/nob.h)).

Each project contains a `build_script.c` which gets built into the `builder(.exe)` that can then be run to build the main project. This repository mostly contains a collection of single-header include libraries that can be used by the `build_script.c` to make common build tasks easier (like string manipulation, and CLI argument organization). There are also template `build.bat` and `build.sh` files in `shell/` folder that can be copied into the root of your project to make it easy to get started. All files in this repo are expected to compile with the default settings for MSVC and Clang so that the shell script doesn't have to know anything about the build script in order to build it.

Additionally, rather than building the `build_script.c` every time we want to build the project we only build it if either the binary doesn't exist OR if the current binary exits with a special return code saying that it wants to be re-compiled. This allows the `build_script.c` to do the work of hashing it's source files and detecting changes to trigger re-compilation. This is done through a single function call `RecompileIfNeeded(...)` at the top of `main(...)` (the implementation lives in `src/pig_build_recompile.h`)

This repository can act as a "framework" that you use without modification to get your C program building but it's recommended that you fork, modify, and refine the system to your needs. Much of the baked-in logic is easy to understand and change. For example `RecompileIfNeeded` can easily be updated to find it's source files in a different way, or it could be implemented using a different hashing function or a different file I/O approach. Rather than PigBuild being designed to support every possible use-case through configuration, it serves more as a single example of things you might do in your `build_script.c` and it's expected that each project and modify and update the code to their needs.

The code in this repo used to live in [PigCore](https://github.com/PiggybankStudios/PigCore), for a full history of changes check the commits in that repository. I have opted to pull this logic out for 2 reasons:

1. None of the code that a `build_script.c` depends on should be a part of the thing it is building, since doing so could cause some nasty problems and invalid states.

2. The system is potentially useful for non-PigCore projects and possibly useful for other people. This repository aims to be a clear example for anyone looking to write their own C build system.


## Getting Started

1. Clone this repository into your project under a folder called `pig_build`
	
	* **NOTE:** The folder name must be `pig_build` (with proper capitalization on platforms where that matters). If you really want to change this you can change the shell scripts and the defines in `pig_build_recompile.h`.

2. Copy the `build.bat` and `build.sh` from the `shell/` folder into your project's root folder

3. Create a `build_script.c` with your own `int main(int argc, char* argv[]) { ... }` just like any C program. Feel free to `#include` any standard libraries you would like to use like `<stdio.h>` or `<string.h>`.

	* **NOTE:** The name of your build script must be `build_script.c` right now. If you really want to change this you can change the shell scripts and the defines in `pig_build_recompile.h`.

4. `#include "pig_build.h"` to get all `pig_build_[x].h` helper files, OR `#include` specific files from the `src/` folder that you want. Some of these files depend on each other, for example including `pig_build_file.h` will automatically pull in `pig_build_str.h`. Note that every file depends on `pig_build_base.h`. The files can be included in any order and their dependencies will be worked out automatically.

	* `#include "pig_build_base.h"` - This file is included by every other file in the `src/` directory. It provides common `#define`s like `BUILDING_ON_WINDOWS` or `LANGUAGE_IS_C`, common typedefs like `u32`, `i16`, and `r64`, and function-like macros like `Assert(condition)` and `ArrayCount(array)`.
	
	* `#include "pig_build_str.h"` - Contains `struct Str { u64 length; const char* chars; };` which acts as an alternative to using null-terminated strings. Memory management and const-correctness are not enforced by the `Str` structure, it simply acts as a common template for joining a length and pointer together for any purpose. The calling code decides how the string is allocated, and the lifespan of the memory.
	
	* `#include "pig_build_str_array.h"` - Contains `struct StrArray` which holds a dynamic buffer of `Str` structures that can be manipulated through a simple API like `Str* AddStr(StrArray* array, Str newString)`, `Str* InsertStr(StrArray* array, Str newString, u64 insertIndex)`, etc.
	
	* `#include "pig_build_file.h"` - Contains methods for creating, reading, writing, and otherwise manipulating files and folders with implementations for all of the major desktop platforms.
	
	* `#include "pig_build_recompile.h"` - Contains the `RecompileIfNeeded` function that helps the build-script hash it's own source code and decide that it needs to be recompiled
	
	* `#include "pig_build_cli_flags.h"` - Contains a bunch of `#define`s that give readable names to CLI flags that we pass to the compilers: Clang, MSVC and GCC. That way we can do `AddStr(&args, CL_WARNINGS_AS_ERRORS)` instead of `AddStr(&args, "/WX")`
	
	* `#include "pig_build_tags.h"` - Tags are an optional system built into `CliArgList` that allows us to put all out arguments into one list that conditionally apply to each compiler invocation based on the matching of `includeTags` and `excludeTags`. This file just `#define`s a basic set of tag strings like `T_MSVC_CL`, `T_LINUX`, `T_LANG_C` etc.
	
	* `#include "pig_build_arg_list.h"` - Contains the `CliArgList` structure which acts like a `StrArray` but with more features that make it easier for us to handle common problems with authoring program arguments. For example it contains logic for escaping quoted strings properly and a system of `includeTags` and `excludeTags` that can be used to conditionally apply arguments to each compiler invocation. This list can be passed directly to `RunCliProgram` in `pig_build_misc.h`
	
	* `#include "pig_build_misc.h"` - Contains extra helper functions and types that don't belong in any other file. Things like `RunCliProgram`, `LineParser`, `TwoPassPrint`, etc.
	
	* `optional/` - This folder contains files that I re-use between my projects but are probably not useful for other people's projects. For example `pig_build_pig_core_flags.h` contains all of the common compiler flags and tagging logic that we use to compile for the matrix of targets and configurations that PigCore supports, `pig_build_android.h` contains some logic and flags for compiling and bundling Android applications, etc. Feel free to take a look and/or use these files but they are likely not as useful unless you are using PigCore or targeting niche platforms like Playdate.
	
5. At the beginning of main call `RecompileIfNeeded(nullptr);`, this will exit with a specific status code if it detects the script needs to be re-built. The `build.bat` or `build.sh` will see this and rebuild your `build_script.c` and then rerun it automatically.

	* **NOTE:** You need `pig_build_recompile.h` included for this function to be available

6. Add any logic you want to build your main program. For example:

```C
#include <stdio.h>
#include <stdlib.h>
#include "pig_build.h"

int main(int argc, char* argv[])
{
	RecompileIfNeeded(nullptr);
	printf("Building...\n");
	
	// Do stuff like
	system("clang main.c -o my_program");
	
	// Or use CliArgList
	CliArgList args = EMPTY;
	AddArg(&args, CL_NO_LOGO);
	AddArgNt(&args, CLI_QUOTED_ARG, "src/main.c");
	AddArgNt(&args, CL_BINARY_FILE, "my_program.exe");
	AddArgNt(&args, CL_DEFINE, "DEBUG_BUILD=1");
	AddArgNt(&args, CL_LANG_VERSION, "clatest");
	int compilerExitCode = RunCliProgram(StrLit("cl"), nullptr, &args);
	Assert(compilerExitCode == 0);
	
	printf("Done!\n");
	return 0;
}
```

7. Run `build.sh` or `build.bat` whenever you want to build. Build errors for your `build_script.c` are reported in the same manner as errors from your main program so they should be easy to match. You can also debug the `builder` executable if you have crashes during the build process that you need to debug.

8. **NOTE:** If, for any reason, the `builder` and the `builder_hash.txt` get out of sync and automatic recompilation is not happening. You can either delete both of these files and run `build.sh`/`build.bat` OR you can just make a small change to `build_script.c`, like adding a temporary comment on some random line, building, and then removing it on the next build.