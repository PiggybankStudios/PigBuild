
#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

#define BUILD_WINDOWS (BUILDING_ON_WINDOWS)
#define BUILD_LINUX   (BUILDING_ON_LINUX || (0 && BUILDING_ON_WINDOWS))
#define BUILD_OSX     (BUILDING_ON_OSX)

#define DEBUG_BUILD     1
#define RUN_AFTER_BUILD 0

int main(int argc, const char* argv[])
{
	RecompileIfNeeded(nullptr);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	Str pigBuildFolder = StrLit("../..");
	
	//TODO: Download Raylib from: https://github.com/raysan5/raylib/releases/tag/5.5
	//      Windows: ?
	//      Linux: ?
	//      OSX: https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_macos.tar.gz
	//TODO: Download GLFW from: https://github.com/glfw/glfw/releases/tag/3.4
	//      Windows: ?
	//      Linux: ?
	//      OSX: https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.MACOS.zip
	
	// +--------------------------------------------------------------+
	// |                           Windows                            |
	// +--------------------------------------------------------------+
	#if BUILD_WINDOWS
	{
		InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
		WriteLine("[Building for Windows...]");
		
		CliArgList args = EMPTY;
		args.pathSepChar = PATH_SEP_CHAR;
		args.rootDirPath = StrLit("..");
		AddArg(&args, CL_NO_LOGO);
		AddArg(&args, CL_FULL_FILE_PATHS);
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/main.c");
		AddArgNt(&args, CL_BINARY_FILE, "raylib_window.exe");
		AddArgNt(&args, CL_PDB_FILE, "raylib_window.pdb");
		#if DEBUG_BUILD
		AddArg(&args, CL_DEBUG_INFO);
		#endif
		AddArgNt(&args, CL_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		AddArgNt(&args, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
		AddArg(&args, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
		AddArg(&args, CL_LINK);
		AddArgNt(&args, CLI_QUOTED_ARG, "raylib.lib"); //NOTE: raylib.lib MUST be before User32.lib and others
		AddArgNt(&args, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
		AddArgNt(&args, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
		AddArgNt(&args, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
		AddArgNt(&args, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
		AddArgNt(&args, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
		AddArgNt(&args, CLI_QUOTED_ARG, "Kernel32.lib");
		AddArgNt(&args, CLI_QUOTED_ARG, "Winmm.lib");
		
		RunCliProgramAndExitOnFailure(StrLit("cl"), "", &args, StrLit("Failed to build raylib_window.exe!"));
		AssertFileExist(StrLit("raylib_window.exe"), true);
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                            Linux                             |
	// +--------------------------------------------------------------+
	#if BUILD_LINUX
	{
		WriteLine("[Building for Linux...]");
		AssertMsg(false, "OSX is not yet implemented in build_script.c"); //TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             OSX                              |
	// +--------------------------------------------------------------+
	#if BUILD_OSX
	{
		WriteLine("[Building for OSX...]");
		
		CliArgList args = EMPTY;
		CopyFileToPath(StrLit("../main.c"), StrLit("main.m"), true);
		AddArgNt(&args, CLI_QUOTED_ARG, "main.m");
		AddArgNt(&args, CLANG_OUTPUT_FILE, "raylib_window");
		AddArgNt(&args, CLANG_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&args, CLANG_INCLUDE_DIR, "[ROOT]/raylib/include");
		AddArg(&args, CLANG_FULL_FILE_PATHS);
		AddArgNt(&args, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		//TODO: Add clang arguments for optimization and debug info
		// AddArg(&args, "-fno-threadsafe-statics"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
		
		AddArgNt(&args, CLANG_LIBRARY_DIR, "[ROOT]/raylib/lib");
		AddArgNt(&args, CLANG_LIBRARY_DIR, "[ROOT]/glfw/lib-arm64"); //TODO: We should choose the appropriate architecture for intel apple machines
		AddArgNt(&args, CLANG_FRAMEWORK, "CoreText");
		AddArgNt(&args, CLANG_FRAMEWORK, "Cocoa");
		AddArgNt(&args, CLANG_FRAMEWORK, "QuartzCore");
		AddArgNt(&args, CLANG_FRAMEWORK, "CoreFoundation");
		// AddArgNt(&args, CLANG_FRAMEWORK, "AudioToolbox");
		// AddArgNt(&args, CLANG_FRAMEWORK, "Foundation");
		// AddArgNt(&args, CLANG_FRAMEWORK, "UIKit");
		// AddArgNt(&args, CLANG_FRAMEWORK, "AVFoundation");
		AddArgNt(&args, CLANG_FRAMEWORK, "Metal");
		AddArgNt(&args, CLANG_FRAMEWORK, "MetalKit");
		AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "raylib");
		AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "glfw.3");
		// AddArg(&args, "-Wl,-force_load,./../raylib/lib/libraylib.a"); //TODO: It seems like the static version of Raylib still links with the dynamic version of GLFW??
		// AddArg(&args, "-Wl,-force_load,./../glfw/lib-arm64/libglfw3.a");
		AddArgNt(&args, CLANG_RPATH_DIR, "."); //Add the current folder to RPATH so the .dylibs can be found when the program is run from this folder
		AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "stdc++"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
		
		RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build raylib_window!"));
		AssertFileExist(StrLit("raylib_window"), true);
		
		//Copy the .dylibs to the build folder so they can be found when the program is run from here (in conjunction with RPATH above)
		CopyFileToFolder(StrLit("../raylib/lib/libraylib.550.dylib"), StrLit("."), true);
		CopyFileToFolder(StrLit("../glfw/lib-arm64/libglfw.3.dylib"), StrLit("."), true);
	}
	#endif
	
	WriteLine("[Build finished!]");
	
	// +--------------------------------------------------------------+
	// |                             Run                              |
	// +--------------------------------------------------------------+
	#if RUN_AFTER_BUILD
	{
		CliArgList args = EMPTY;
		Str executableName = (BUILDING_ON_WINDOWS ? StrLit("raylib_window.exe") : StrLit("./raylib_window"));
		PrintLine("\n[Running %.*s]", StrPrint(executableName));
		RunCliProgram(executableName, "", &args);
	}
	#endif
	
	return 0;
}