
#define PIG_BUILD_PRINT_SYS_CMDS 0
#define PIG_BUILD_FOLDER_PATH "../../.."
#include "pig_build.h"

#define BUILD_WINDOWS (BUILDING_ON_WINDOWS)
#define BUILD_LINUX   (BUILDING_ON_LINUX || (0 && BUILDING_ON_WINDOWS))
#define BUILD_OSX     (BUILDING_ON_OSX)

#define DEBUG_BUILD     1
#define RUN_AFTER_BUILD 0
#define REBUILD_IMGUI   0

#if DEBUG_BUILD
#define IF_DEBUG(...) __VA_ARGS__
#else
#define IF_DEBUG(...) //nothing
#endif
#if BUILDING_ON_WINDOWS
#define IF_WINDOWS(...) __VA_ARGS__
#else
#define IF_WINDOWS(...) //nothing
#endif
#if BUILDING_ON_OSX
#define IF_OSX(...) __VA_ARGS__
#else
#define IF_OSX(...) //nothing
#endif

void DownloadDearImgui();
void DownloadGlfwAndCompileIfNeeded();

int main(int argc, const char* argv[])
{
	RecompileIfNeeded(nullptr);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	Str pigBuildFolder = StrLit(PIG_BUILD_FOLDER_PATH);
	
	DownloadDearImgui();
	DownloadGlfwAndCompileIfNeeded();
	
	StrArray sourceFiles = EMPTY;
	AddStr(&sourceFiles, BUILDING_ON_OSX ? StrLit("main.mm") : StrLit("[ROOT]/main.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_widgets.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_draw.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_tables.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/imgui_demo.cpp"));
	AddStr(&sourceFiles, StrLit("[ROOT]/imgui/backends/imgui_impl_glfw.cpp"));
	IF_WINDOWS(AddStr(&sourceFiles, StrLit("[ROOT]/imgui/backends/imgui_impl_opengl3.cpp"));)
	IF_OSX(AddStr(&sourceFiles, StrLit("[ROOT]/imgui/backends/imgui_impl_metal.mm"));)
	
	// +--------------------------------------------------------------+
	// |                           Windows                            |
	// +--------------------------------------------------------------+
	#if BUILD_WINDOWS
	{
		InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
		WriteLine("[Building for Windows...]");
		
		CliArgs commonArgs = EMPTY;
		AddArg(&commonArgs, CL_NO_LOGO);
		AddArg(&commonArgs, CL_FULL_FILE_PATHS);
		AddArgNt(&commonArgs, CL_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&commonArgs, CL_INCLUDE_DIR, "[ROOT]/imgui");
		AddArgNt(&commonArgs, CL_INCLUDE_DIR, "[ROOT]/imgui/backends");
		AddArgNt(&commonArgs, CL_INCLUDE_DIR, "[ROOT]/glfw/include");
		IF_DEBUG(AddArg(&commonArgs, CL_DEBUG_INFO));
		AddArgNt(&commonArgs, CL_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		AddArgNt(&commonArgs, CL_DEFINE, "TARGET_IS_WINDOWS=1");
		AddArgNt(&commonArgs, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
		AddArg(&commonArgs, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
		AddArgNt(&commonArgs, CL_LANG_VERSION, "c++20");
		AddArgNt(&commonArgs, CL_PDB_FILE, "imgui_demo.pdb");
		
		StrArray objectFiles = EMPTY;
		for (u64 fIndex = 0; fIndex < sourceFiles.length; fIndex++)
		{
			Str sourcePath = sourceFiles.strings[fIndex];
			Str sourceExt = GetFileExtPart(sourcePath, false);
			Str objectPath = JoinStrings2(GetFileNamePart(sourcePath, false), StrLit(".obj"));
			
			// Rudamentary incremental build, only compile main.mm unconditionally,
			// all the imgui source files should never change so we only compile them if the object file doesn't already exist
			if (REBUILD_IMGUI || !DoesFileExist(objectPath) || StrExactEquals(sourcePath, StrLit("[ROOT]/main.cpp")))
			{
				PrintLine("Compiling \"%.*s\" -> \"%.*s\"", StrPrint(sourcePath), StrPrint(objectPath));
				CliArgs args = EMPTY;
				args.pathSepChar = PATH_SEP_CHAR;
				args.rootDirPath = StrLit("..");
				AddArg(&args, CL_COMPILE);
				AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&args, CL_OBJ_FILE, objectPath);
				AddArgList(&args, &commonArgs);
				
				StrArray tags = EMPTY;
				AddStr(&tags, sourceExt);
				RunCliProgramTagArrayAndExitOnFailure(StrLit("cl"), &tags, &args, StrLit("Failed to compile source file!"));
				AssertFileExist(objectPath, true);
			}
			
			AddStr(&objectFiles, objectPath);
		}
		
		{
			CliArgs args = EMPTY;
			AddArg(&args, LINK_NO_LOGO);
			IF_DEBUG(AddArg(&args, LINK_DEBUG_INFO));
			AddArgNt(&args, LINK_OUTPUT_FILE, "imgui_demo.exe");
			AddArgNt(&args, LINK_DEBUG_INFO_FILE, "imgui_demo.pdb");
			for (u64 oIndex = 0; oIndex < objectFiles.length; oIndex++)
			{
				AddArgStr(&args, CLI_QUOTED_ARG, objectFiles.strings[oIndex]);
			}
			AddArgNt(&args, LINK_LIBRARY_DIR, "[ROOT]/glfw/lib-vc2022");
			AddArgNt(&args, CLI_QUOTED_ARG, "glfw3.lib");
			AddArgNt(&args, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateBitmap, CreateDCW, etc. in GLFW
			// AddArgNt(&args, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for ?
			AddArgNt(&args, CLI_QUOTED_ARG, "opengl32.lib");
			
			RunCliProgramAndExitOnFailure(StrLit("link"), "", &args, StrLit("Failed to build imgui_demo.exe!"));
			AssertFileExist(StrLit("imgui_demo.exe"), true);
		}
	}
	#endif
	
	// +--------------------------------------------------------------+
	// |                            Linux                             |
	// +--------------------------------------------------------------+
	#if BUILD_LINUX
	{
		WriteLine("[Building for Linux...]");
		
		CliArgs commonArgs = EMPTY;
		AddArg(&commonArgs, CLANG_FULL_FILE_PATHS);
		AddArgNt(&commonArgs, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		AddArgNt(&commonArgs, CLANG_DEFINE, "TARGET_IS_LINUX=1");
		AddArgNt(&commonArgs, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
		IF_DEBUG(AddArgNt(&commonArgs, CLANG_DEBUG_INFO, "dwarf-4"));
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui/backends");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/glfw/include");
		AddArgNt(&commonArgs, CLANG_LANG_VERSION, "c++20");
		
		// Compile
		StrArray objectFiles = EMPTY;
		for (u64 fIndex = 0; fIndex < sourceFiles.length; fIndex++)
		{
			Str sourcePath = sourceFiles.strings[fIndex];
			Str objectPath = JoinStrings2(GetFileNamePart(sourcePath, false), StrLit(".o"));
			
			// Rudamentary incremental build, only compile main.cpp unconditionally,
			// all the imgui source files should never change so we only compile them if the object file doesn't already exist
			if (REBUILD_IMGUI || !DoesFileExist(objectPath) || StrExactEquals(sourcePath, StrLit("[ROOT]/main.cpp")))
			{
				PrintLine("Compiling \"%.*s\" -> \"%.*s\"", StrPrint(sourcePath), StrPrint(objectPath));
				CliArgs args = EMPTY;
				AddArg(&args, CLANG_COMPILE);
				AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&args, CLANG_OUTPUT_FILE, objectPath);
				AddArgList(&args, &commonArgs);
				
				RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to compile source file!"));
				AssertFileExist(objectPath, true);
			}
			
			AddStr(&objectFiles, objectPath);
		}
		
		// Link
		{
			CliArgs args = EMPTY;
			AddArgList(&args, &commonArgs);
			for (u64 oIndex = 0; oIndex < objectFiles.length; oIndex++)
			{
				AddArgStr(&args, CLI_QUOTED_ARG, objectFiles.strings[oIndex]);
			}
			AddArgNt(&args, CLANG_OUTPUT_FILE, "imgui_demo");
			AddArgNt(&args, CLANG_LIBRARY_DIR, "[ROOT]/glfw/lib-arm64");
			AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "glfw.3");
			AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "stdc++"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
			AddArgNt(&args, CLANG_RPATH_DIR, ".");
			
			RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build imgui_demo!"));
			AssertFileExist(StrLit("imgui_demo"), true);
			
			CopyFileToFolder(StrLit("../glfw/lib-arm64/libglfw.3.so"), StrLit("."), true);
		}
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
		
		CliArgs commonArgs = EMPTY;
		AddArg(&commonArgs, CLANG_FULL_FILE_PATHS);
		AddArgNt(&commonArgs, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
		AddArgNt(&commonArgs, CLANG_DEFINE, "TARGET_IS_OSX=1");
		AddArgNt(&commonArgs, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
		IF_DEBUG(AddArg(&commonArgs, CLANG_DEBUG_INFO_DEFAULT));
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/imgui/backends");
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/glfw/include");
		AddArgNt(&commonArgs, CLANG_LANG_VERSION, "c++20");
		AddTaggedArg(&commonArgs, ".mm", CLANG_ENABLE_OBJC_ARC); //Turn on Automatic Reference Counting only for .mm files (we do this using tags)
		
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
				PrintLine("Compiling \"%.*s\" -> \"%.*s\"", StrPrint(sourcePath), StrPrint(objectPath));
				CliArgs args = EMPTY;
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
			CliArgs args = EMPTY;
			AddArgList(&args, &commonArgs);
			for (u64 oIndex = 0; oIndex < objectFiles.length; oIndex++)
			{
				AddArgStr(&args, CLI_QUOTED_ARG, objectFiles.strings[oIndex]);
			}
			AddArgNt(&args, CLANG_OUTPUT_FILE, "imgui_demo");
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
			
			RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build imgui_demo!"));
			AssertFileExist(StrLit("imgui_demo"), true);
			
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
		CliArgs args = EMPTY;
		Str executableName = (BUILDING_ON_WINDOWS ? StrLit("imgui.exe") : StrLit("./imgui"));
		PrintLine("\n[Running %.*s]", StrPrint(executableName));
		RunCliProgram(executableName, "", &args);
	}
	#endif
	
	return 0;
}

// +--------------------------------------------------------------+
// |                     Download Dear ImGui                      |
// +--------------------------------------------------------------+
void DownloadDearImgui()
{
	Str imguiUrl = StrLit("https://github.com/ocornut/imgui/archive/refs/tags/v1.92.7.zip");
	Str imguiZipPath = StrLit("imgui_v1.92.7.zip");
	Str imguiZipRootFolder = StrLit("imgui-1.92.7");
	Str imguiFolderPath = StrLit("../imgui");
	if (!DoesFileExist(imguiZipPath) || !DoesFolderExist(imguiFolderPath))
	{
		PrintLine("Downloading Dear ImGui from \"%.*s\"", StrPrint(imguiUrl));
		DownloadAndExtractArchive(
			imguiUrl,
			imguiZipPath,
			2247481, 0x223CB155060498F1,
			imguiFolderPath,
			imguiZipRootFolder
		);
	}
}

// +--------------------------------------------------------------+
// |                 Download (and compile) GLFW                  |
// +--------------------------------------------------------------+
void DownloadGlfwAndCompileIfNeeded()
{
	#if BUILDING_ON_WINDOWS
	Str glfwUrl = StrLit("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip");
	Str glfwZipRootFolder = StrLit("glfw-3.4.bin.WIN64");
	const u64 glfwExpectedSize = 3284918;
	const u64 glfwExpectedHash = 0x9D76DA3E48D5E5E7;
	#elif BUILDING_ON_LINUX
	Str glfwUrl = StrLit("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip");
	Str glfwZipRootFolder = StrLit("glfw-3.4");
	const u64 glfwExpectedSize = 1653725;
	const u64 glfwExpectedHash = 0xE4CF3D4137B793A4;
	#elif BUILDING_ON_OSX
	Str glfwUrl = StrLit("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.MACOS.zip");
	Str glfwZipRootFolder = StrLit("glfw-3.4.bin.MACOS");
	const u64 glfwExpectedSize = 1351252;
	const u64 glfwExpectedHash = 0x8E86944555E911BC;
	#endif
	Str glfwZipPath = GetFileNamePart(glfwUrl, true);
	Str glfwFolderPath = StrLit("../glfw");
	if (!DoesFileExist(glfwZipPath) || !DoesFolderExist(glfwFolderPath))
	{
		PrintLine("Downloading GLFW from \"%.*s\"", StrPrint(glfwUrl));
		DownloadAndExtractArchive(
			glfwUrl,
			glfwZipPath,
			glfwExpectedSize, glfwExpectedHash,
			glfwFolderPath,
			glfwZipRootFolder
		);
	}
	
	// +==============================+
	// |    Build GLFW (On Linux)     |
	// +==============================+
	#if BUILDING_ON_LINUX
	if (!DoesFileExist(StrLit("libglfw3.so")))
	{
		WriteLine("Building GLFW...");
		
		StrArray glfwSourceFiles = EMPTY;
		AddStrLit(&glfwSourceFiles, "../glfw/src/context.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/init.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/input.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/monitor.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/platform.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/vulkan.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/window.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/egl_context.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/osmesa_context.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/null_init.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/null_monitor.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/null_window.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/null_joystick.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/posix_module.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/posix_time.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/posix_thread.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/posix_poll.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/linux_joystick.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/xkb_unicode.c");
		//TODO: For Wayland builds we should use do: wl_init.c, wl_monitor.c, wl_window.c
		AddStrLit(&glfwSourceFiles, "../glfw/src/x11_init.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/x11_monitor.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/x11_window.c");
		AddStrLit(&glfwSourceFiles, "../glfw/src/glx_context.c");
		
		CliArgs commonArgs = EMPTY;
		AddArg(&commonArgs, CLANG_FULL_FILE_PATHS);
		AddArgNt(&commonArgs, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
		IF_DEBUG(AddArgNt(&commonArgs, CLANG_DEBUG_INFO, "dwarf-4"));
		AddArgNt(&commonArgs, CLANG_INCLUDE_DIR, "[ROOT]/glfw/include");
		AddArgNt(&commonArgs, CLANG_LANG_VERSION, "gnu2x"); //TODO: C instead of C++?
		
		// Compile
		StrArray glfwObjectFiles = EMPTY;
		for (u64 fIndex = 0; fIndex < glfwSourceFiles.length; fIndex++)
		{
			Str sourcePath = glfwSourceFiles.strings[fIndex];
			Str objectPath = JoinStrings2(GetFileNamePart(sourcePath, false), StrLit(".o"));
			
			PrintLine("Compiling \"%.*s\" -> \"%.*s\"", StrPrint(sourcePath), StrPrint(objectPath));
			CliArgs args = EMPTY;
			AddArg(&args, CLANG_COMPILE);
			AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
			AddArgStr(&args, CLANG_OUTPUT_FILE, objectPath);
			AddArgList(&args, &commonArgs);
			
			RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to compile source file!"));
			AssertFileExist(objectPath, true);
			
			AddStr(&glfwObjectFiles, objectPath);
		}
		
		// Link
		{
			CliArgs args = EMPTY;
			AddArgList(&args, &commonArgs);
			for (u64 oIndex = 0; oIndex < glfwObjectFiles.length; oIndex++)
			{
				AddArgStr(&args, CLI_QUOTED_ARG, glfwObjectFiles.strings[oIndex]);
			}
			AddArg(&args, CLANG_BUILD_SHARED_LIB);
			AddArg(&args, CLANG_fPIC);
			AddArgNt(&args, CLANG_OUTPUT_FILE, "libglfw3.so");
			// AddArgNt(&args, CLANG_SYSTEM_LIBRARY, "stdc++"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
			AddArgNt(&args, CLANG_RPATH_DIR, ".");
			
			RunCliProgramAndExitOnFailure(StrLit("clang"), "", &args, StrLit("Failed to build imgui_demo!"));
			AssertFileExist(StrLit("imgui_demo"), true);
			
			CopyFileToFolder(StrLit("../glfw/lib-arm64/libglfw.3.so"), StrLit("."), true);
		}
		
	}
	#endif //BUILDING_ON_LINUX
}
