
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
		
		RunCliProgramAndExitOnFailure(StrLit("cl"), "", &args, StrLit("Failed to build vulkan_triangle.exe!"));
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                            Linux                             |
	// +--------------------------------------------------------------+
	#if BUILD_LINUX
	{
		//TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             OSX                              |
	// +--------------------------------------------------------------+
	#if BUILD_OSX
	{
		//TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             Run                              |
	// +--------------------------------------------------------------+
	#if RUN_AFTER_BUILD
	{
		CliArgList args = EMPTY;
		Str executableName = (BUILDING_ON_WINDOWS ? StrLit("vulkan_triangle.exe") : StrLit("vulkan_triangle"));
		PrintLine("\nRunning %.*s", StrPrint(executableName));
		RunCliProgram(executableName, "", &args);
	}
	#endif
	
	return 0;
}