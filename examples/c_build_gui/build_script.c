/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\31\2026
Description: 
	** This build_script is a stop-gap on our way to building a graphical
	** application that allows the user to run builds and track progress.
	** Since a GUI application takes dependencies and build flags to create,
	** we need this script to get compiled without any dependencies first so we
	** can implement that logic to download the dependencies and pass the right
	** flags to build the real build_gui.c
*/

#define PIG_BUILD_PRINT_SYS_CMDS 1
#include "pig_build.h"

void DownloadSokolIfNeeded();
void CrossCompileShaderIfNeeded();

int main(int argc, char** argv[])
{
	RecompileIfNeeded(StrArray_Empty);
	Str pigBuildFolder = StrLit(PIG_BUILD_ROOT);
	IF_WINDOWS(bool isMsvcInitialized = WasMsvcDevBatchRun());
	Str executableName = StrLit("builder_gui" EXE_EXT);
	
	if (DoesFileExist(executableName))
	{
		WriteLine("[Running GUI...]");
		int exitCode = RunCliProgram(executableName, nullptr);
		if (exitCode == 0) { return 0; }
		else if (exitCode == REBUILD_EXIT_CODE)
		{
			WriteLine("[GUI Recompile requested]");
		}
		else
		{
			PrintLine("ERROR: %.*s exited with code %d", StrPrint(executableName), exitCode);
			return exitCode;
		}
	}
	
	DownloadSokolIfNeeded();
	CrossCompileShaderIfNeeded();
	
	CliArgs args = EMPTY;
	
	// +==============================+
	// |     MSVC Compiler Flags      |
	// +==============================+
	AddTaggedArgNt(&args, T_MSVC_CL, CLI_QUOTED_ARG, "[ROOT]/build_gui.c");
	AddTaggedArg(&args, T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(&args, T_MSVC_CL, CL_NO_LOGO);
	AddTaggedArgNt(&args, T_MSVC_CL, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(&args, T_MSVC_CL, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	AddTaggedArgNt(&args, T_MSVC_CL, CL_INCLUDE_DIR, ".");
	AddTaggedArgNt(&args, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&args, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/sokol");
	AddTaggedArgNt(&args, T_MSVC_CL, CL_INCLUDE_DIR, PIG_BUILD_ROOT "/src");
	AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "PIG_BUILD_ROOT=\"" PIG_BUILD_ROOT "\"");
	AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, executableName);
	AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "builder_gui.pdb");
	AddTaggedArg(&args, T_MSVC_CL, CL_DEBUG_INFO);
	AddTaggedArgNt(&args, T_MSVC_CL, CL_OPTIMIZATION_LEVEL, "d");
	
	// +==============================+
	// |     Clang Compiler Flags     |
	// +==============================+
	AddTaggedArg(&args, T_CLANG, CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&args, T_CLANG, CLANG_OPTIMIZATION_LEVEL, "0");
	AddTaggedArg(&args, T_CLANG, CLANG_DEBUG_INFO_DEFAULT);
	AddTaggedArgNt(&args, T_CLANG, CLANG_LANG_VERSION, "gnu2x");
	AddTaggedArgNt(&args, T_CLANG, CLANG_INCLUDE_DIR, ".");
	AddTaggedArgNt(&args, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&args, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sokol");
	AddTaggedArgNt(&args, T_CLANG, CLANG_INCLUDE_DIR, PIG_BUILD_ROOT "/src");
	AddTaggedArgNt(&args, T_CLANG, CLANG_DEFINE, "PIG_BUILD_ROOT=\"" PIG_BUILD_ROOT "\"");
	AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, executableName);
	AddTaggedArgNt(&args, T_CLANG T_OSX "==false", CLI_QUOTED_ARG, "[ROOT]/main.c");
	AddTaggedArgNt(&args, T_CLANG T_OSX "==true", CLI_QUOTED_ARG, "main.m");
	
	// +==============================+
	// |     Linux Compiler Flags     |
	// +==============================+
	AddTaggedArgNt(&args, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "m");
	AddTaggedArgNt(&args, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(&args, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(&args, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(&args, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xcursor");
	
	// +==============================+
	// |      OSX Compiler Flags      |
	// +==============================+
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_DEFINE, "TARGET_IS_OSX=1");
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_FRAMEWORK, "CoreFoundation");
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(&args, T_CLANG T_OSX, CLANG_FRAMEWORK, "MetalKit");
	
	AddTaggedArg(&args, T_MSVC_CL, CL_LINK);
	AddTaggedArg(&args, T_MSVC_CL, LINK_DISABLE_INCREMENTAL);
	
	#if BUILDING_ON_WINDOWS
	{
		InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
		PrintLine("Buiding builder_gui.exe for WINDOWS!");
		RunCliProgramAndExitOnFailureTagsLit(StrLit("cl"), T_MSVC_CL T_WINDOWS, &args, StrLit("Failed to build builder_gui.exe!"));
		AssertFileExist(StrLit("builder_gui.exe"), true);
		PrintLine("Successfully built builder_gui.exe for WINDOWS!");
	}
	#elif BUILDING_ON_LINUX
	{
		PrintLine("Buiding builder_gui for LINUX!");
		RunCliProgramAndExitOnFailureTagsLit(StrLit("clang"), T_CLANG T_LINUX, &args, StrLit("Failed to build builder_gui!"));
		AssertFileExist(StrLit("builder_gui"), true);
		PrintLine("Successfully built builder_gui for LINUX!");
	}
	#elif BUILDING_ON_OSX
	{
		PrintLine("Buiding builder_gui for OSX!");
		//Create an main.m to make the compiler use Objective-C mode
		if (!DoesFileExist(StrLit("main.m"))) { CreateAndWriteFile(StrLit("main.m"), StrLit("\n#include \"main.c\"\n"), true); }
		RunCliProgramAndExitOnFailureTagsLit(StrLit("clang"), T_CLANG T_OSX, &args, StrLit("Failed to build builder_gui!"));
		AssertFileExist(StrLit("builder_gui"), true);
		PrintLine("Successfully built builder_gui for OSX!");
	}
	#else
	#error build_script.c needs to be updated to support the current platform!
	#endif
	
	// +--------------------------------------------------------------+
	// |                             Run                              |
	// +--------------------------------------------------------------+
	{
		CliArgs args = EMPTY;
		WriteLine("\n[Running GUI...]");
		RunCliProgram(executableName, nullptr);
	}
	
	return 0;
}

#if BUILDING_ON_WINDOWS
#define SHDC_BIN_PATH "..\\sokol_tools\\bin\\win32\\sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define SHDC_BIN_PATH "../sokol_tools/bin/linux/sokol-shdc"
#elif BUILDING_ON_OSX_ARM
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx_arm64/sokol-shdc"
#elif BUILDING_ON_OSX_INTEL
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx/sokol-shdc"
#else
#error build_script.c SHDC_BIN_PATH needs to be updated to support the current platform!
#endif

void DownloadSokolIfNeeded()
{
	// https://github.com/floooh/sokol/commit/453c71214fbb55d782683d20ea7e6c07314e3e9b
	// Commit 453c712 from May 14th 2026 - "sokol_framebuffer.h: fix some comment typos"
	Str sokolUrl = StrLit("https://github.com/floooh/sokol/archive/453c71214fbb55d782683d20ea7e6c07314e3e9b.zip");
	Str sokolZipPath = StrLit("sokol_453c712.zip");
	Str sokolZipRootFolder = StrLit("sokol-453c71214fbb55d782683d20ea7e6c07314e3e9b");
	Str sokolFolderPath = StrLit("../sokol");
	if (!DoesFileExist(sokolZipPath) || !DoesFolderExist(sokolFolderPath))
	{
		PrintLine("Downloading Sokol from \"%.*s\"", StrPrint(sokolUrl));
		// if (DoesFolderExist(sokolFolderPath)) { MyRemoveDirectory(sokolFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolUrl,
			sokolZipPath,
			1573414, 0x5707750981F0E26C,
			sokolFolderPath,
			sokolZipRootFolder
		);
	}
	
	// https://github.com/floooh/sokol-tools-bin/commit/1a9a4e54090fec42c5d13169b638f09f25474953
	// Commit 1a9a4e5 from April 26th 2026 - "updated (88)"
	Str sokolToolsUrl = StrLit("https://github.com/floooh/sokol-tools-bin/archive/1a9a4e54090fec42c5d13169b638f09f25474953.zip");
	Str sokolToolsZipPath = StrLit("sokol_tools_1a9a4e5.zip");
	Str sokolToolsZipRootFolder = StrLit("sokol-tools-bin-1a9a4e54090fec42c5d13169b638f09f25474953");
	Str sokolToolsFolderPath = StrLit("../sokol_tools");
	if (!DoesFileExist(sokolToolsZipPath) || !DoesFolderExist(sokolToolsFolderPath))
	{
		PrintLine("Downloading Sokol Tools from \"%.*s\"", StrPrint(sokolToolsUrl));
		// if (DoesFolderExist(sokolToolsFolderPath)) { MyRemoveDirectory(sokolToolsFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolToolsUrl,
			sokolToolsZipPath,
			18032476, 0xDF94D1D90D33715F,
			sokolToolsFolderPath,
			sokolToolsZipRootFolder
		);
		
		// Mark sokol-shdc as executable on Linux and OSX
		#if BUILDING_ON_OSX || BUILDING_ON_LINUX
		CliArgs chmodArgs = EMPTY;
		AddArg(&chmodArgs, "+x");
		AddArgNt(&chmodArgs, CLI_QUOTED_ARG, SHDC_BIN_PATH);
		RunCliProgramAndExitOnFailure(StrLit("chmod"), &chmodArgs, StrLit("Failed to make sokol-shdc executable!"));
		#endif
	}
}

void CrossCompileShaderIfNeeded()
{
	Str shaderSrcPath = StrLit("../basic_shader.glsl");
	Str shaderHeaderPath = StrLit("basic_shader.glsl.h");
	Str shaderHashFilePath = StrLit_Const("basic_shader_hash.txt");
	u64 newHash = FnvHashFile(shaderSrcPath, FNV_HASH_BASE_U64);
	
	u64 savedShaderHash = 0;
	bool hashFileExisted = false;
	{
		Str shaderHashFileContents = Str_Empty_Const;
		if (TryReadFile(shaderHashFilePath, &shaderHashFileContents))
		{
			if (TryParseHexU64(shaderHashFileContents, &savedShaderHash)) { hashFileExisted = true; }
			else { WriteLine_E("Invalid shader hash file contents!"); }
		}
		FreeStr(&shaderHashFileContents);
	}
	
	if (!DoesFileExist(shaderHeaderPath) || !hashFileExisted || newHash != savedShaderHash)
	{
		Str shaderFileName = GetFileNamePart(shaderSrcPath, true);
		PrintLine("Cross-compiling %.*s using sokol-shdc...", StrPrint(shaderFileName));
		
		CliArgs shdcArgs = EMPTY;
		AddArgNt(&shdcArgs, SHDC_FORMAT, "sokol_impl");
		AddArgNt(&shdcArgs, SHDC_ERROR_FORMAT, "msvc");
		// AddArg(&shdcArgs, SHDC_REFLECTION);
		AddArgNt(&shdcArgs, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:glsl300es:metal_macos");
		AddArgStr(&shdcArgs, SHDC_INPUT, shaderSrcPath);
		AddArgStr(&shdcArgs, SHDC_OUTPUT, shaderHeaderPath);
		
		RunCliProgramAndExitOnFailure(StrLit(SHDC_BIN_PATH), &shdcArgs, StrLit("Failed to cross-compile basic_shader.glsl using sokol-shdc!"));
		AssertFileExist(StrLit("basic_shader.glsl.h"), true);
		
		PrintLine("Successfully cross-compiled %.*s!", StrPrint(shaderFileName));
		
		Str newHashString = ConvertU64ToHexStr(newHash, true);
		CreateAndWriteFile(shaderHashFilePath, newHashString, true);
	}
}
