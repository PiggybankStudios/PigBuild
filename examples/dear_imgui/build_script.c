
#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

#define BUILD_WINDOWS (BUILDING_ON_WINDOWS)
#define BUILD_LINUX   (BUILDING_ON_LINUX || (0 && BUILDING_ON_WINDOWS))
#define BUILD_OSX     (BUILDING_ON_OSX)

#define DEBUG_BUILD     1
#define RUN_AFTER_BUILD 0
#define REBUILD_IMGUI   0

int main(int argc, const char* argv[])
{
	StrArray buildScriptFolders = EMPTY;
	AddStrLit(&buildScriptFolders, "../../../src");
	RecompileIfNeeded(&buildScriptFolders);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	Str pigBuildFolder = StrLit("../..");
	
	//TODO: Download Dear ImGui v1.92.7 from: https://github.com/ocornut/imgui/releases/tag/v1.92.7
	//      All Platforms: https://github.com/ocornut/imgui/archive/refs/tags/v1.92.7.zip
	
	// +--------------------------------------------------------------+
	// |                           Windows                            |
	// +--------------------------------------------------------------+
	#if BUILD_WINDOWS
	{
		WriteLine("[Building for Windows...]");
		AssertMsg(false, "Windows is not yet implemented in build_script.c"); //TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                            Linux                             |
	// +--------------------------------------------------------------+
	#if BUILD_LINUX
	{
		WriteLine("[Building for Linux...]");
		AssertMsg(false, "Linux is not yet implemented in build_script.c"); //TODO: Implement me!
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                             OSX                              |
	// +--------------------------------------------------------------+
	#if BUILD_OSX
	{
		WriteLine("[Building for OSX...]");
		
		if (!DoesFileExist(StrLit("main.mm")))
		{
			CreateAndWriteFile(StrLit("main.mm"), StrLit("\n#include \"../main.cpp\"\n"), true);
		}
		
		CliArgList commonArgs = EMPTY;
		AddArg(&commonArgs, CLANG_FULL_FILE_PATHS);
		AddArgNt(&commonArgs, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		AddArgNt(&commonArgs, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
		if (DEBUG_BUILD) { AddArg(&commonArgs, CLANG_DEBUG_INFO_DEFAULT); }
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui/backends");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/glfw/include");
		AddArgNt(&commonArgs, CLANG_LANG_VERSION, "c++20");
		AddTaggedArg(&commonArgs, ".mm", CLANG_ENABLE_OBJC_ARC); //Turn on Automatic Reference Counting only for .mm files (we do this using tags)
		
		StrArray sourceFiles = EMPTY;
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui.cpp"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_widgets.cpp"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_draw.cpp"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_tables.cpp"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_demo.cpp"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/backends/imgui_impl_metal.mm"));
		AddStr(&sourceFiles, StrLit("[ROOT]/imgui/backends/imgui_impl_glfw.cpp"));
		AddStr(&sourceFiles, StrLit("main.mm"));
		
		// Compile
		StrArray objectFiles = EMPTY;
		for (u64 fIndex = 0; fIndex < sourceFiles.length; fIndex++)
		{
			Str sourcePath = sourceFiles.strings[fIndex];
			Str sourceExt = GetFileExtPart(sourcePath);
			Str objectPath = JoinStrings2(GetFileNamePart(sourcePath, false), StrLit(".o"), false);
			
			// Rudamentary incremental build, only compile main.mm unconditionally,
			// all the imgui source files should never change so we only compile them if the object file doesn't already exist
			if (REBUILD_IMGUI || !DoesFileExist(objectPath) || StrExactEquals(sourcePath, StrLit("main.mm")))
			{
				PrintLine("Compiling \"%.*s\"", StrPrint(sourcePath));
				CliArgList args = EMPTY;
				AddArg(&args, CLANG_COMPILE);
				AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&args, CLANG_OUTPUT_FILE, objectPath);
				AddArgList(&args, &commonArgs);
				
				StrArray tags = EMPTY;
				AddStr(&tags, sourceExt);
				RunCliProgramTagArrayAndExitOnFailure(StrLit("clang"), &tags, &args, StrLit("Failed to compile source file!"));
				AssertFileExist(objectPath, true);
			}
			
			AddStr(&objectFiles, objectPath);
		}
		
		// Link
		{
			CliArgList args = EMPTY;
			AddArgList(&args, &commonArgs);
			for (u64 oIndex = 0; oIndex < objectFiles.length; oIndex++)
			{
				AddArgStr(&args, CLI_QUOTED_ARG, objectFiles.strings[oIndex]);
			}
			AddArgNt(&args, CLANG_OUTPUT_FILE, "imgui");
			AddArgNt(&args, CLANG_LIBRARY_DIR, "[ROOT]/glfw/lib-arm64");
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
			AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "glfw.3");
			AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "stdc++"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
			AddArgNt(&args, CLANG_RPATH_DIR, ".");
			
			RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build imgui!"));
			AssertFileExist(StrLit("imgui"), true);
			
			CopyFileToFolder(StrLit("../glfw/lib-arm64/libglfw.3.dylib"), StrLit("."), true);
		}
	}
	#endif
	
	WriteLine("[Build finished!]");
	
	// +--------------------------------------------------------------+
	// |                             Run                              |
	// +--------------------------------------------------------------+
	#if RUN_AFTER_BUILD
	{
		CliArgList args = EMPTY;
		Str executableName = (BUILDING_ON_WINDOWS ? StrLit("imgui.exe") : StrLit("./imgui"));
		PrintLine("\n[Running %.*s]", StrPrint(executableName));
		RunCliProgram(executableName, "", &args);
	}
	#endif
	
	return 0;
}