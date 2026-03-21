# Pig Build

Rather than putting all of our build logic into Bash or Batch, we have a 2-stage build system that uses a small shell script to build a C program that contains the majority of the build logic and then runs that build program.

The build logic for each project is contained in a `build_script.c` file in the root directory of the project with `build.sh`/`build.bat` shell scripts beside it. However, a bunch of logic and types and other systems are re-usable between projects so this repository contains that re-usable code to help support all kinds of C build scripts.

All files in this repo are expected to compile with the default settings for MSVC and Clang so that the shell script doesn't have to know anything about the build script in order to build it.

Additionally, rather than building the `build_script.c` every time we want to build the project we only build it if it either the binary doesn't exist OR if the current binary exits with a special return code saying that it wants to be re-compiled. This allows the `build_script.c` to do whatever work it needs to check it's own source code for changes and request compilation if the code has changed since the last time the script was compiled. Usually this is hashing all the files in this repo and the `build_script.c` and saving the hash to disk so it can compare and detect changes to any of those files' contents.

The code in this repo used to live in [PigCore](https://github.com/PiggybankStudios/PigCore), for a full history of changes check the commits in that repository. I have opted to pull this logic out for 2 reasons:

1. None of the code that a `build_script.c` depends on should be a part of the thing it is building, since doing so could cause some nasty problems and invalid states.

2. The system is potentially useful for non PigCore projects and possibly useful for other people. This repository aims to be a clear example for anyone looking to write their own C build system.


## Getting Started

1. Clone this repository into your project under a folder called `pig_build`
	
	* **NOTE:** The folder name must be `pig_build` (with proper capitalization on platforms where that matters). If you really want to change this you can change the shell scripts and the defines in `pig_build_recompile.h`.

2. Copy the `build.bat` and `build.sh` from the `template/` folder into your project's root folder

3. Create a `build_script.c` with your own `int main(int argc, char* argv[]) { ... }` just like any C program. Feel free to `#include` any standard libraries you would like to use like `<stdio.h>` or `<string.h>`.

	* **NOTE:** The name of your build script must be `build_script.c` right now. If you really want to change this you can change the shell scripts and the defines in `pig_build_recompile.h`.

4. *(Optional)* `#include` any or all of the following files from this repository:

	* `#include "pig_build_shared.h"` - **NOTE:** This file is required to be included and included first if you want to use any of the other files. This also includes a few standard library headers that are used in other pig_build files
	
	* `#include "pig_build_recompile.h"` -- **NOTE:** This is required if you want to use `RecompileIfNeeded()`
	
	* `#include "pig_build_cli_flags.h"`
	
	* `#include "pig_build_str_array.h"`
	
	* `#include "pig_build_cli.h"`
	
	* `#include "pig_build_build_helpers.h"`
	
	* `#include "pig_build_pig_core_build_flags.h"` - **NOTE:** This is only used in [PigCore](https://github.com/PiggybankStudios/PigCore) projects
	
	* `#include "pig_build_android_build_helpers.h"` - **NOTE:** This is only needed for Android builds. And probably only works well for [PigCore](https://github.com/PiggybankStudios/PigCore) projects

5. At the beginning of main call `RecompileIfNeeded();`, this will exit with a specific status code if it detects the script needs to be re-built. The `build.bat` or `build.sh` will see this and rebuild your `build_script.c` and then rerun it automatically.

	* **NOTE:** You need `pig_build_recompile.h` included for this function to be available

6. Add any logic you want to build your main program. For example:

```C
#include "<stdio.h>"

#include "pig_build_shared.h"
#include "pig_build_recompile.h"
#include "pig_build_cli_flags.h"
#include "pig_build_str_array.h"
#include "pig_build_cli.h"
#include "pig_build_build_helpers.h"

int main(int argc, char* argv[])
{
	RecompileIfNeeded();
	printf("Building...\n");
	
	//... do stuff like system("clang main.c -o my_program");
	
	printf("Done!\n");
	return 0;
}
```