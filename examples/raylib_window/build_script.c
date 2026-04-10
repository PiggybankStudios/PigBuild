
#include "pig_build.h"

#define BUILD_WINDOWS (BUILDING_ON_WINDOWS)
#define BUILD_LINUX   (BUILDING_ON_LINUX || (0 && BUILDING_ON_WINDOWS))
#define BUILD_OSX     (BUILDING_ON_OSX)

#define DEBUG_BUILD     1
#define RUN_AFTER_BUILD 1

int main(int argc, const char* argv[])
{
	RecompileIfNeeded(nullptr);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	Str pigBuildFolder = StrLit("../..");
	
	//TODO: Download Raylib from: https://github.com/raysan5/raylib/releases/tag/5.5
	//      Windows: ?
	//      Linux: ?
	//      OSX: https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_macos.tar.gz
	
	// +--------------------------------------------------------------+
	// |                           Windows                            |
	// +--------------------------------------------------------------+
	#if BUILD_WINDOWS
	{
		InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
		
		CliArgList args = EMPTY;
		args.pathSepChar = PATH_SEP_CHAR;
		args.rootDirPath = StrLit("..");
		AddArg(&args, CL_NO_LOGO);
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/main.c");
		AddArgNt(&args, CL_BINARY_FILE, "vulkan_triangle.exe");
		AddArgNt(&args, CL_PDB_FILE, "vulkan_triangle.pdb");
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
		
		RunCliProgramAndExitOnFailure(StrLit("cl"), "", &args, StrLit("Failed to build vulkan_triangle.exe!"));
		AssertFileExist(StrLit("vulkan_triangle.exe"), true);
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                            Linux                             |
	// +--------------------------------------------------------------+
	#if BUILD_LINUX
	{
		AssertMsg(false, "OSX is not yet implemented in build_script.c"); //TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             OSX                              |
	// +--------------------------------------------------------------+
	#if BUILD_OSX
	{
		CliArgList args = EMPTY;
		CopyFileToPath(StrLit("../main.c"), StrLit("main.m"), true);
		AddArgNt(&args, CLI_QUOTED_ARG, "main.m");
		AddArgNt(&args, CLANG_OUTPUT_FILE, "vulkan_triangle");
		AddArgNt(&args, CLANG_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&args, CLANG_INCLUDE_DIR, "[ROOT]/raylib/include");
		AddArgNt(&args, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		//TODO: Add optimization and debug info clang argumeents
		
		AddArgNt(&args, CLANG_LIBRARY_DIR, "[ROOT]/raylib/lib");
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
		AddArgNt(&args, CLANG_RPATH_DIR, ".");
		
		RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build vulkan_triangle!"));
		AssertFileExist(StrLit("vulkan_triangle"), true);
		
		CopyFileToFolder(StrLit("../raylib/lib/libraylib.550.dylib"), StrLit("."), true);
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             Run                              |
	// +--------------------------------------------------------------+
	#if RUN_AFTER_BUILD
	{
		CliArgList args = EMPTY;
		Str executableName = (BUILDING_ON_WINDOWS ? StrLit("vulkan_triangle.exe") : StrLit("./vulkan_triangle"));
		PrintLine("\nRunning %.*s", StrPrint(executableName));
		RunCliProgram(executableName, "", &args);
	}
	#endif
	
	return 0;
}