/*
File:   pig_build_pig_core_gui_app.h
Author: Taylor Robbins
Date:   07\15\2026
Description:
	** Contains common code that is re-used between CSwitch, Crest,
	** and COSM and any newer GUI applications that have similar
	** goals and dependencies. Usually this involves an application
	** that is built on Sokol, uses Clay and/or Pig UI for UI layout,
	** runs on Windows, MacOS and Linux (and sometimes WASM/Web and Android),
	** can be built into a format that allows the application code to be
	** hot-reloaded by keeping it in a separate .dll from the main
	** "platform" executable (and PigCore is also in pig_core.dll/so/dylib),
	** but also supports being built into a single executable with
	** core resources being packaged into a .zip and stored directly
	** in the executable, the build process is driven by a "build_config.h"
	** file that is scraped by the builder and #included directly
	** by the application.
*/

#ifndef _PIG_BUILD_PIG_CORE_GUI_APP_H
#define _PIG_BUILD_PIG_CORE_GUI_APP_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_array.h"
#include "pig_build_str_array.h"
#include "pig_build_arg_list.h"
#include "pig_build_cli_flags.h"
#include "pig_build_tags.h"
#include "pig_build_misc.h"
#include "pig_build_file.h"
#include "optional/pig_build_zip_resources.h"

#if (BUILDING_ON_LINUX || BUILDING_ON_OSX)
#define PIG_CORE_DLL_NAME      "libpig_core"
#else
#define PIG_CORE_DLL_NAME      "pig_core"
#endif

#define T_SHADER_OBJS      "|ShaderObjs"

void MakeAndMoveIntoLinuxFolder() { MyCreateFolder(StrLit("linux"), false); chdir("linux"); }
void PopOutOfLinuxFolder() { chdir(".."); }

int BuildPigCoreGuiApplication(StrArray* cliArgs, Str buildConfigContents, Str appFolderPath)
{
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	StrArray commonTags = EMPTY;
	Str resolvedAppFolderPath = ResolveRootTo(appFolderPath, StrLit(".."));
	
	// +==============================+
	// |       Extract Defines        |
	// +==============================+
	Str PROJECT_DLL_NAME      = CopyStr(ExtractStrDefine(buildConfigContents, StrLit("PROJECT_DLL_NAME")));
	Str PROJECT_EXE_NAME      = CopyStr(ExtractStrDefine(buildConfigContents, StrLit("PROJECT_EXE_NAME")));
	Str PROJECT_READABLE_NAME = CopyStr(ExtractStrDefine(buildConfigContents, StrLit("PROJECT_READABLE_NAME")));
	Str PROJECT_FOLDER_NAME   = CopyStr(ExtractStrDefine(buildConfigContents, StrLit("PROJECT_FOLDER_NAME")));
	
	//Find the config value in build_config.h and store it into a local variable of the same name,
	// also add it to commonTags if the value is `true`
	#define LOAD_CONFIG(CONFIG_NAME)                                                     \
		bool CONFIG_NAME = ExtractBoolDefine(buildConfigContents, StrLit(#CONFIG_NAME)); \
		if (CONFIG_NAME) { AddStrLit(&commonTags, #CONFIG_NAME); }                       \
		do {} while(0)
	LOAD_CONFIG(DEBUG_BUILD);
	LOAD_CONFIG(BUILD_INTO_SINGLE_UNIT);
	LOAD_CONFIG(USE_BUNDLED_RESOURCES);
	LOAD_CONFIG(BUILD_WINDOWS);
	LOAD_CONFIG(BUILD_LINUX);
	LOAD_CONFIG(BUILD_OSX);
	LOAD_CONFIG(BUILD_SHADERS);
	LOAD_CONFIG(BUILD_PIGGEN);
	LOAD_CONFIG(RUN_PIGGEN);
	LOAD_CONFIG(BUILD_TRACY_DLL);
	LOAD_CONFIG(PROFILING_ENABLED);
	LOAD_CONFIG(BUNDLE_RESOURCES_ZIP);
	LOAD_CONFIG(BUILD_PIG_CORE_DLL);
	LOAD_CONFIG(BUILD_APP_EXE);
	LOAD_CONFIG(BUILD_APP_DLL);
	LOAD_CONFIG(RUN_APP);
	LOAD_CONFIG(COPY_TO_DATA_DIRECTORY);
	LOAD_CONFIG(DUMP_PREPROCESSOR);
	LOAD_CONFIG(DUMP_ASSEMBLY);
	LOAD_CONFIG(BUILD_WITH_SOKOL_GFX);
	LOAD_CONFIG(BUILD_WITH_SOKOL_APP);
	LOAD_CONFIG(BUILD_WITH_FREETYPE);
	LOAD_CONFIG(BUILD_WITH_GTK);
	LOAD_CONFIG(BUILD_WITH_HTTP);
	#undef LOAD_CONFIG
	
	// +==============================+
	// | Parse Command-Line Arguments |
	// +==============================+
	if (cliArgs->length > 0)
	{
		PrintLine("Got %llu argument%s", cliArgs->length, (cliArgs->length == 1) ? "" : "s");
		for (u64 aIndex = 0; aIndex < cliArgs->length; aIndex++)
		{
			Str argStr = cliArgs->strings[aIndex];
			PrintLine("Arg[%llu]: %.*s", aIndex, StrPrint(argStr));
			//TODO: We should parse these arguments and use them as overrides to the #defines we loaded above!
		}
		WriteLine_E("Usage: " BUILD_SCRIPT_EXE_NAME " [DEBUG_BUILD={1/0}] [BUILD_TESTS={1/0}] ..."); //TODO: Only print this out if we find an argument we don't understand
		PrintLine_E("ERROR: Command-line arguments are not supported yet!");
		return 1;
	}
	
	// +====================================+
	// | Enforce Build Config Restrictions  |
	// +====================================+
	if (BUILD_WINDOWS && !BUILDING_ON_WINDOWS)
	{
		WriteLine_E("BUILD_WINDOWS does not working when building on non-Windows platforms");
		BUILD_WINDOWS = false;
	}
	if (BUILD_OSX && !BUILDING_ON_OSX)
	{
		WriteLine_E("BUILD_OSX does not working when building on non-Mac platforms");
		BUILD_OSX = false;
	}
	if (BUILD_LINUX && !BUILDING_ON_LINUX && !BUILDING_ON_WINDOWS)
	{
		WriteLine_E("BUILD_LINUX only works when building on Linux (or on Windows through WSL)");
		BUILD_LINUX = false;
	}
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_DLL && !BUILD_APP_EXE)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT works with BUILD_APP_EXE but only BUILD_APP_DLL is enabled. Assuming we want BUILD_APP_EXE instead");
		BUILD_APP_DLL = false;
		BUILD_APP_EXE = true;
	}
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_DLL)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT implies that BUILD_APP_DLL is unnecassary. Only BUILD_APP_EXE matters");
		BUILD_APP_DLL = false;
	}
	if (BUILD_INTO_SINGLE_UNIT && BUILD_APP_EXE && BUILD_PIG_CORE_DLL)
	{
		WriteLine_E("BUILD_INTO_SINGLE_UNIT implies that BUILD_PIG_CORE_DLL is unnecassary. Not building pig_core.dll/so");
		BUILD_PIG_CORE_DLL = false;
	}
	if (RUN_PIGGEN && !BUILD_PIGGEN && !DoesFileExist(StrLit("piggen" EXE_EXT)))
	{
		WriteLine("Building piggen" EXE_EXT " because it's missing");
		BUILD_PIGGEN = true;
	}
	if (PROFILING_ENABLED && !BUILD_TRACY_DLL && !DoesFileExist(StrLit("tracy" DLL_EXT)))
	{
		WriteLine("Building tracy" DLL_EXT " because it's missing");
		BUILD_TRACY_DLL = true;
	}
	if ((BUILD_APP_EXE || BUILD_APP_DLL) && !BUILD_PIG_CORE_DLL && !BUILD_INTO_SINGLE_UNIT && !DoesFileExist(StrLit(PIG_CORE_DLL_NAME DLL_EXT)))
	{
		WriteLine("Building " PIG_CORE_DLL_NAME DLL_EXT " because it's missing");
		BUILD_PIG_CORE_DLL = true;
	}
	#if BUILDING_ON_WINDOWS
	Str filenameAppExe = JoinStrings2(PROJECT_EXE_NAME, StrLit(".exe"));
	#else
	Str filenameAppExe = PROJECT_EXE_NAME;
	#endif
	if (RUN_APP && !BUILD_APP_EXE && !DoesFileExist(filenameAppExe))
	{
		PrintLine("Building %.*s because it's missing", StrPrint(filenameAppExe));
		BUILD_APP_EXE = true;
	}
	Str filenameAppDll = JoinStrings2(PROJECT_DLL_NAME, StrLit(DLL_EXT));
	if (RUN_APP && !BUILD_APP_DLL && !DoesFileExist(filenameAppDll))
	{
		PrintLine("Building %.*s because it's missing", StrPrint(filenameAppDll));
		BUILD_APP_DLL = true;
	}
	if (USE_BUNDLED_RESOURCES && !BUNDLE_RESOURCES_ZIP && !DoesFileExist(StrLit("resources.zip")))
	{
		WriteLine("Bundling resources.zip because it's missing");
		BUNDLE_RESOURCES_ZIP = true;
	}
	
	// +==============================+
	// |      Fill Common Flags       |
	// +==============================+
	CliArgs commonCompilerFlags = EMPTY;
	CliArgs commonLinkerFlags = EMPTY;
	{
		//Add our root folder and app folder to include dirs BEFORE directories from FillPigCoreFlags
		// This is important to ensure we use our own build_config.h instead of the one living in PigCore for tests
		AddTaggedArgNt(&commonCompilerFlags,  T_MSVC_CL, CL_INCLUDE_DIR,    "[ROOT]");
		AddTaggedArgNt(&commonCompilerFlags,  T_CLANG,   CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(&commonCompilerFlags, T_MSVC_CL, CL_INCLUDE_DIR,    appFolderPath);
		AddTaggedArgStr(&commonCompilerFlags, T_CLANG,   CLANG_INCLUDE_DIR, appFolderPath);
		AddTaggedArgNt(&commonCompilerFlags,  T_MSVC_CL, CL_INCLUDE_DIR,    "gen");
		AddTaggedArgNt(&commonCompilerFlags,  T_CLANG,   CLANG_INCLUDE_DIR, "gen");
		
		FillPigCoreFlags(&commonCompilerFlags, &commonLinkerFlags, StrLit("[ROOT]/core"));
	}
	
	// +--------------------------------------------------------------+
	// |                       Build piggen.exe                       |
	// +--------------------------------------------------------------+
	Str piggenExePath = StrLit("piggen" EXE_EXT);
	if (BUILD_PIGGEN)
	{
		Str piggenMainPath = StrLit("[ROOT]/core/src/piggen/piggen_main.c");
		
		// +==============================+
		// |   Build Piggen on Windows    |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			WriteLine("\n[Building piggen.exe for Windows...]");
			
			CliArgs cmd = EMPTY;
			AddArgStr(&cmd, CLI_QUOTED_ARG, piggenMainPath);
			AddArgStr(&cmd, CL_BINARY_FILE, piggenExePath);
			AddArgList(&cmd, &commonCompilerFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "piggen.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &commonLinkerFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_MSVC_CL);
			AddTag(&tags, T_WINDOWS);
			AddTag(&tags, T_LANG_C);
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, StrLit("Failed to build piggen.exe!"));
			AssertFileExist(piggenExePath, true);
			WriteLine("[Built piggen.exe for Windows!]");
		}
		
		// +==============================+
		// |    Build Piggen on Linux     |
		// +==============================+
		if (BUILD_LINUX)
		{
			WriteLine("\n[Building piggen for Linux...]");
			IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
			AddArgStr(&cmd, CLI_QUOTED_ARG, piggenMainPath);
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, piggenExePath);
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_LINUX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_C);
			
			Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
			RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, StrLit("Failed to build piggen!"));
			AssertFileExist(piggenExePath, true);
			WriteLine("[Built piggen for Linux!]");
			
			IF_NOT_LINUX(PopOutOfLinuxFolder());
		}
		
		// +==============================+
		// |     Build Piggen for OSX     |
		// +==============================+
		if (BUILD_OSX)
		{
			WriteLine("\n[Building piggen for OSX...]");
			
			CliArgs cmd = EMPTY;
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, "piggen");
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgList(&cmd, &commonLinkerFlags);
			AddArgNt(&cmd, CLANG_LANGUAGE, "objective-c");
			AddArgStr(&cmd, CLI_QUOTED_ARG, piggenMainPath);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_OSX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_OBJECTIVEC);
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &cmd, StrLit("Failed to build piggen!"));
			AssertFileExist(StrLit("piggen"), true);
			WriteLine("[Built piggen for OSX!]");
		}
		//TODO: Add OSX support
	}
	
	// +--------------------------------------------------------------+
	// |                        Run piggen.exe                        |
	// +--------------------------------------------------------------+
	if (RUN_PIGGEN)
	{
		WriteLine("\n[piggen" EXE_EXT "]");
		
		#define PIGGEN_OUTPUT_FOLDER "-o=\"[VAL]\""
		#define PIGGEN_EXCLUDE_FOLDER "-e=\"[VAL]\""
		
		CliArgs cmd = EMPTY;
		AddArgNt(&cmd, CLI_QUOTED_ARG, "..");
		AddArgNt(&cmd, PIGGEN_OUTPUT_FOLDER, "gen/");
		
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/data/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/media/");
		
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/src/base/base_defines_check.h");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/src/piggen/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/src/third_party/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/src/wasm/std/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/.git/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/build/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/media/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/template/");
		AddArgNt(&cmd, PIGGEN_EXCLUDE_FOLDER, "[ROOT]/core/fuzzing/");
		
		Str runnablePiggenExePath = StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX "piggen" EXE_EXT);
		RunCliProgramAndExitOnFailure(runnablePiggenExePath, &cmd, StrLit("piggen" EXE_EXT " Failed!"));
	}
	
	// +--------------------------------------------------------------+
	// |                       Build tracy.dll                        |
	// +--------------------------------------------------------------+
	if (BUILD_TRACY_DLL)
	{
		// +==============================+
		// |    Build Tracy on Windows    |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("[Building tracy.dll for Windows...]");
			
			CliArgs cmd = EMPTY;
			// AddArg(&cmd, CL_COMPILE);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CL_INCLUDE_DIR, "[ROOT]/core/third_party/tracy");
			AddArgNt(&cmd, CL_BINARY_FILE, "tracy.dll");
			AddArgNt(&cmd, CL_DEFINE, "TRACY_ENABLE");
			AddArgNt(&cmd, CL_DEFINE, "TRACY_EXPORTS");
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
			AddArgNt(&cmd, CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
			AddArgList(&cmd, &commonCompilerFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "tracy.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_MSVC_CL);
			AddTag(&tags, T_WINDOWS);
			AddTag(&tags, T_LANG_CPP);
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, StrLit("Failed to build tracy.dll!"));
			AssertFileExist(StrLit("tracy.dll"), true);
			WriteLine("[Built tracy.dll for Windows!]");
		}
		
		// +==============================+
		// |     Build Tracy on Linux     |
		// +==============================+
		if (BUILD_LINUX)
		{
			WriteLine("\n[Building tracy.so for Linux...]");
			IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
			AddArgNt(&cmd, CLI_QUOTED_ARG, "[ROOT]/core/third_party/tracy/TracyClient.cpp");
			AddArgNt(&cmd, CLANG_INCLUDE_DIR, "[ROOT]/core/third_party/tracy");
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, "tracy.so");
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgNt(&cmd, CLANG_DEFINE, "TRACY_ENABLE");
			AddArgNt(&cmd, CLANG_DEFINE, "TRACY_EXPORTS");
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgList(&cmd, &commonLinkerFlags);
			AddArgNt(&cmd, CLANG_DISABLE_WARNING, CLANG_WARNING_SHADOWING); // declaration shadows a local variable
			AddArgNt(&cmd, CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FIELD_INITIALIZERS); // missing field 'extra' initializer
			AddArgNt(&cmd, CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH); // unannotated fall-through between switch labels
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_LINUX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_CPP);
			
			Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
			RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, StrLit("Failed to build tracy.so!"));
			AssertFileExist(StrLit("tracy.so"), true);
			WriteLine("[Built tracy.so for Linux!]");
			
			IF_NOT_LINUX(PopOutOfLinuxFolder());
		}
		//TODO: Add OSX support
	}
	if (PROFILING_ENABLED)
	{
		AddTaggedArgNt(&commonLinkerFlags, T_MSVC_CL, CLI_QUOTED_ARG, "tracy" LIB_EXT);
		AddTaggedArgNt(&commonLinkerFlags, T_CLANG,   CLI_QUOTED_ARG, "tracy" LIB_EXT);
	}
	
	// +--------------------------------------------------------------+
	// |                       Bundle Resources                       |
	// +--------------------------------------------------------------+
	if (BUNDLE_RESOURCES_ZIP)
	{
		MyCreateFolder(StrLit("gen"), false);
		BundleResourcesZip(
			StrLit("../data/resources"),
			StrLit("resources.zip"),
			StrLit("gen/resources_zip.h"),
			StrLit("gen/resources_zip.c"),
			StrLit("resources_zip_bytes")
		);
	}
	
	// +--------------------------------------------------------------+
	// |                        Build Shaders                         |
	// +--------------------------------------------------------------+
	FindShadersContext findContext = EMPTY;
	{
		//NOTE: No ignoreList needed in findContext
		RecursiveDirWalk(resolvedAppFolderPath, FindShaderFilesCallback, &findContext);
		
		if (BUILD_WINDOWS)
		{
			for (u64 sIndex = 0; sIndex < findContext.objPaths.length; sIndex++)
			{
				Str objPath = findContext.objPaths.strings[sIndex];
				AddTaggedArgStr(&commonLinkerFlags, T_WINDOWS T_SHADER_OBJS, CLI_QUOTED_ARG, objPath);
				if (!DoesFileExist(objPath) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(objPath)); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_LINUX)
		{
			for (u64 sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str oPath = findContext.oPaths.strings[sIndex];
				AddTaggedArgStr(&commonLinkerFlags, T_NOT_WINDOWS T_SHADER_OBJS, CLI_QUOTED_ARG, oPath);
				Str oPathWithFolder = BUILDING_ON_LINUX ? CopyStr(oPath) : JoinPaths(StrLit("linux"), oPath);
				if (!DoesFileExist(oPathWithFolder) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(oPathWithFolder)); BUILD_SHADERS = true; }
			}
		}
		if (BUILD_OSX)
		{
			for (u64 sIndex = 0; sIndex < findContext.oPaths.length; sIndex++)
			{
				Str oPath = findContext.oPaths.strings[sIndex];
				AddTaggedArgStr(&commonLinkerFlags, T_NOT_WINDOWS T_SHADER_OBJS, CLI_QUOTED_ARG, oPath);
				if (!DoesFileExist(oPath) && !BUILD_SHADERS) { PrintLine("Building shaders because \"%.*s\" is missing!", StrPrint(oPath)); BUILD_SHADERS = true; }
			}
		}
		
		if (!BUILD_SHADERS)
		{
			FreeStrArray(&findContext.shaderPaths);
			FreeStrArray(&findContext.headerPaths);
			FreeStrArray(&findContext.sourcePaths);
			FreeStrArray(&findContext.objPaths);
			FreeStrArray(&findContext.oPaths);
		}
	}
	
	if (BUILD_SHADERS)
	{
		if (BUILD_WINDOWS) { InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized); }
		
		PrintLine("Found %llu shader%s", findContext.shaderPaths.length, findContext.shaderPaths.length == 1 ? "" : "s");
		// for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		// {
		// 	PrintLine("Shader[%u]", sIndex);
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.shaderPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.headerPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.sourcePaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.objPaths.strings[sIndex]));
		// 	PrintLine("\t\"%.*s\"", StrPrint(findContext.oPaths.strings[sIndex]));
		// }
		
		// First use shdc.exe to generate header files for each .glsl file
		for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str shaderPath = findContext.shaderPaths.strings[sIndex];
			Str headerPath = findContext.headerPaths.strings[sIndex];
			Str realHeaderPath = ResolveRootTo(headerPath, StrLit(".."));
			Str realShaderPath = ResolveRootTo(shaderPath, StrLit(".."));
			
			CliArgs cmd = EMPTY;
			AddArgNt(&cmd, SHDC_FORMAT, "sokol_impl");
			AddArgNt(&cmd, SHDC_ERROR_FORMAT, "msvc");
			// AddArg(&cmd, SHDC_REFLECTION);
			AddArgNt(&cmd, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:metal_macos");
			AddArgStr(&cmd, SHDC_INPUT, shaderPath);
			AddArgStr(&cmd, SHDC_OUTPUT, headerPath);
			
			PrintLine("Generating \"%.*s\"...", StrPrint(realHeaderPath));
			Str shdcExe = JoinPaths(StrLit("../core"), StrLit(EXE_SHDC));
			FixPathSlashes(shdcExe, PATH_SEP_CHAR);
			RunCliProgramAndExitOnFailure(shdcExe, &cmd, StrLit(EXE_SHDC_NAME " failed on TODO:!"));
			AssertFileExist(realHeaderPath, true);
			
			ScrapeShaderHeaderFileAndAddExtraInfo(realHeaderPath, realShaderPath);
			free(realHeaderPath.chars);
			free(realShaderPath.chars);
		}
		
		//Then compile each header file to an .o/.obj file
		for (u64 sIndex = 0; sIndex < findContext.shaderPaths.length; sIndex++)
		{
			Str headerPath = findContext.headerPaths.strings[sIndex];
			Str sourcePath = findContext.sourcePaths.strings[sIndex];
			Str headerFileName = GetFileNamePart(headerPath, true);
			Str headerDirectory = GetDirectoryPart(headerPath, true);
			Str realSourcePath = ResolveRootTo(sourcePath, StrLit(".."));
			
			//We need a .c file that #includes shader_include.h (which defines SOKOL_SHDC_IMPL) and then the shader header file
			Str sourceFileContents = JoinStrings3(
				StrLit("\n#include \"shader_include.h\"\n\n#include \""),
				headerFileName,
				StrLit("\"\n")
			);
			PrintLine("Generating \"%.*s\"...", StrPrint(realSourcePath));
			CreateAndWriteFile(realSourcePath, sourceFileContents, true);
			
			// +==============================+
			// |   Build Shader on Windows    |
			// +==============================+
			if (BUILD_WINDOWS)
			{
				Str objPath = findContext.objPaths.strings[sIndex];
				
				CliArgs cmd = EMPTY;
				AddArg(&cmd, CL_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CL_OBJ_FILE, objPath);
				AddArgStr(&cmd, CL_INCLUDE_DIR, headerDirectory);
				AddArgList(&cmd, &commonCompilerFlags);
				
				StrArray tags = EMPTY;
				AddStrArray(&tags, &commonTags);
				AddTag(&tags, T_MSVC_CL);
				AddTag(&tags, T_WINDOWS);
				AddTag(&tags, T_LANG_C);
				
				RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, FormatStr("Failed to build %.*s for Windows!", StrPrint(sourcePath)));
				AssertFileExist(objPath, true);
			}
			
			// +==============================+
			// |    Build Shader on Linux     |
			// +==============================+
			if (BUILD_LINUX)
			{
				IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
				Str oPath = findContext.oPaths.strings[sIndex];
				//TODO: The path we store in the findContext needs to have [ROOT] at the beginning somehow so we can get rid of this logic
				// Str fixedSourcePath = BUILDING_ON_LINUX ? CopyStr(sourcePath) : JoinStrings2(StrLit("../"), sourcePath, false);
				// FixPathSlashes(fixedSourcePath, '/');
				// Str fixedHeaderDirectory = BUILDING_ON_LINUX ? CopyStr(headerDirectory) : JoinStrings2(StrLit("../"), headerDirectory, false);
				// FixPathSlashes(fixedHeaderDirectory, '/');
				
				CliArgs cmd = EMPTY;
				cmd.pathSepChar = '/';
				IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
				AddArgNt(&cmd, CLANG_DISABLE_WARNING, "unused-command-line-argument"); //Clang likes to warn about _lib_debug/_lib_release library folder being unused
				AddArgList(&cmd, &commonCompilerFlags);
				
				StrArray tags = EMPTY;
				AddStrArray(&tags, &commonTags);
				AddTag(&tags, T_CLANG);
				AddTag(&tags, T_LINUX);
				AddTag(&tags, T_UNIX);
				AddTag(&tags, T_LANG_C);
				
				Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
				RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, FormatStr("Failed to build %.*s for Linux!", StrPrint(sourcePath)));
				AssertFileExist(oPath, true);
				
				IF_NOT_LINUX(PopOutOfLinuxFolder());
			}
			
			// +==============================+
			// |     Build Shader on OSX      |
			// +==============================+
			if (BUILD_OSX)
			{
				Str oPath = findContext.oPaths.strings[sIndex];
				
				CliArgs cmd = EMPTY;
				cmd.pathSepChar = '/';
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, oPath);
				AddArgStr(&cmd, CLANG_INCLUDE_DIR, headerDirectory);
				AddArgNt(&cmd, CLANG_DISABLE_WARNING, "unused-command-line-argument"); //Clang likes to warn about _lib_debug/_lib_release library folder being unused
				AddArgList(&cmd, &commonCompilerFlags);
				
				StrArray tags = EMPTY;
				AddStrArray(&tags, &commonTags);
				AddTag(&tags, T_CLANG);
				AddTag(&tags, T_OSX);
				AddTag(&tags, T_UNIX);
				AddTag(&tags, T_LANG_OBJECTIVEC);
				
				RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &cmd, FormatStr("Failed to build %.*s for OSX!", StrPrint(sourcePath)));
				AssertFileExist(oPath, true);
			}
		}
		
		FreeStrArray(&findContext.shaderPaths);
		FreeStrArray(&findContext.headerPaths);
		FreeStrArray(&findContext.sourcePaths);
		FreeStrArray(&findContext.objPaths);
		FreeStrArray(&findContext.oPaths);
	}
	
	// +--------------------------------------------------------------+
	// |                      Build pig_core.dll                      |
	// +--------------------------------------------------------------+
	if (BUILD_PIG_CORE_DLL)
	{
		Str pigCoreDllMainPath = StrLit("[ROOT]/core/src/dll/dll_main.c");
		
		// +==============================+
		// |   Build PigCore on Windows   |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			WriteLine("\n[Building pig_core.dll for Windows...]");
			
			CliArgs cmd = EMPTY;
			AddArgStr(&cmd, CLI_QUOTED_ARG, pigCoreDllMainPath);
			AddArgNt(&cmd, CL_BINARY_FILE, "pig_core.dll");
			AddArgList(&cmd, &commonCompilerFlags);
			if (DUMP_ASSEMBLY) { AddArgNt(&cmd, CL_ASSEMB_LISTING_FILE, "pig_core.asm"); }
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_MSVC_CL);
			AddTag(&tags, T_WINDOWS);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_PIG_CORE);
			AddTag(&tags, T_LIBRARY);
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, StrLit("Failed to build pig_core.dll!"));
			AssertFileExist(StrLit("pig_core.dll"), true);
			WriteLine("[Built pig_core.dll for Windows!]");
		}
		
		// +==============================+
		// |    Build PigCore on Linux    |
		// +==============================+
		if (BUILD_LINUX)
		{
			WriteLine("\n[Building libpig_core.so for Linux...]");
			IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
			AddArgStr(&cmd, CLI_QUOTED_ARG, pigCoreDllMainPath);
			AddArgNt(&cmd, CLANG_OUTPUT_FILE, "libpig_core.so");
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_LINUX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_PIG_CORE);
			AddTag(&tags, T_LIBRARY);
			
			Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
			RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, StrLit("Failed to build libpig_core.so!"));
			AssertFileExist(StrLit("libpig_core.so"), true);
			WriteLine("[Built libpig_core.so for Linux!]");
			
			IF_NOT_LINUX(PopOutOfLinuxFolder());
		}
		
		//TODO: Add OSX support
	}
	
	// +--------------------------------------------------------------+
	// |                  Build PROJECT_EXE_NAME.exe                  |
	// +--------------------------------------------------------------+
	if (BUILD_APP_EXE)
	{
		//NOTE: When BUILD_INTO_SINGLE_UNIT platform_main.c #includes app_main.c (and has PigCore implementations)
		Str platformMainPath = JoinPaths(appFolderPath, StrLit("platform_main.c"));
		
		// +==============================+
		// |   Build App.exe on Windows   |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %.*s for Windows...]", StrPrint(filenameAppExe));
			
			// Build app/win_resources.rc file into resources.res
			if (!DoesFileExist(StrLit("resources.res")))
			{
				WriteLine("Generating resources.res...");
				CliArgs rcCmd = EMPTY;
				AddArg(&rcCmd, RC_NO_LOGO);
				AddArgNt(&rcCmd, RC_OUTPUT_FILE, "resources.res");
				AddArgStr(&rcCmd, CLI_QUOTED_ARG, JoinPaths(appFolderPath, StrLit("win_resources.rc")));
				RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_RC), &rcCmd, StrLit("Failed to generate resources.res for Windows embedded icon in .exe!"));
			}
			
			CliArgs cmd = EMPTY;
			AddArgStr(&cmd, CLI_QUOTED_ARG, platformMainPath);
			AddArgStr(&cmd, CL_BINARY_FILE, filenameAppExe);
			AddArgList(&cmd, &commonCompilerFlags);
			if (DUMP_ASSEMBLY) { AddArgStr(&cmd, CL_ASSEMB_LISTING_FILE, ChangePathExtension(filenameAppExe, StrLit(".asm"), true)); }
			AddArg(&cmd, CL_LINK);
			AddArgList(&cmd, &commonLinkerFlags);
			if (!BUILD_INTO_SINGLE_UNIT) { AddArgNt(&cmd, CLI_QUOTED_ARG, "pig_core.lib"); }
			AddArgNt(&cmd, CLI_QUOTED_ARG, "resources.res");
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_MSVC_CL);
			AddTag(&tags, T_WINDOWS);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_PROGRAM);
			if (BUILD_INTO_SINGLE_UNIT)
			{
				AddTag(&tags, T_PIG_CORE);
				AddTag(&tags, T_SHADER_OBJS);
			}
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, FormatStr("Failed to build %.*s on Windows!", StrPrint(filenameAppExe)));
			AssertFileExist(filenameAppExe, true);
			PrintLine("[Built %.*s for Windows!]", StrPrint(filenameAppExe));
		}
		
		// +===============================+
		// | Build App Executable on Linux |
		// +===============================+
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %.*s for Linux...]", StrPrint(filenameAppExe));
			IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
			AddArgStr(&cmd, CLI_QUOTED_ARG, platformMainPath);
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, filenameAppExe);
			AddArgList(&cmd, &commonCompilerFlags);
			// AddArgNt(&cmd, CLANG_SYSTEM_LIBRARY, "GL"); //TODO: This should be getting added by PigCore flags!
			AddArgNt(&cmd, CLANG_RPATH_DIR, ".");
			if (!BUILD_INTO_SINGLE_UNIT) { AddArgNt(&cmd, CLI_QUOTED_ARG, "libpig_core.so"); }
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_LINUX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_PROGRAM);
			if (BUILD_INTO_SINGLE_UNIT)
			{
				AddTag(&tags, T_PIG_CORE);
				AddTag(&tags, T_SHADER_OBJS);
			}
			
			Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
			RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, FormatStr("Failed to build %.*s on Linux!", StrPrint(filenameAppExe)));
			AssertFileExist(filenameAppExe, true);
			PrintLine("[Built %.*s for Linux!]", StrPrint(filenameAppExe));
			
			IF_NOT_LINUX(PopOutOfLinuxFolder());
		}
		
		// +==============================+
		// | Build App Executable on OSX  |
		// +==============================+
		if (BUILD_OSX)
		{
			PrintLine("\n[Building %.*s for OSX...]", StrPrint(filenameAppExe));
			
			//TODO: I think we can avoid doing this with the right compiler flags, like CLANG_LANGUAGE?
			MyCreateFolder(StrLit("gen"), false);
			Str platformMainFileName = GetFileNamePart(platformMainPath, true);
			Str platformMainMPath = StrLit("gen/platform_main.m");
			if (!DoesFileExist(platformMainMPath))
			{
				WriteLine_E("Creating platform_main.m");
				CreateAndWriteFile(platformMainMPath, FormatStr("#include \"%.*s\"\n", StrPrint(platformMainFileName)), true);
			}
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			AddArgStr(&cmd, CLI_QUOTED_ARG, platformMainMPath);
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, filenameAppExe);
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgNt(&cmd, CLANG_RPATH_DIR, ".");
			if (!BUILD_INTO_SINGLE_UNIT) { AddArgNt(&cmd, CLI_QUOTED_ARG, "libpig_core.so"); }
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_OSX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_OBJECTIVEC);
			AddTag(&tags, T_PROGRAM);
			if (BUILD_INTO_SINGLE_UNIT)
			{
				AddTag(&tags, T_PIG_CORE);
				AddTag(&tags, T_SHADER_OBJS);
			}
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &cmd, FormatStr("Failed to build %.*s on OSX!", StrPrint(filenameAppExe)));
			AssertFileExist(filenameAppExe, true);
			PrintLine("[Built %.*s for OSX!]", StrPrint(filenameAppExe));
		}
	}
	
	// +--------------------------------------------------------------+
	// |                  Build PROJECT_DLL_NAME.dll                  |
	// +--------------------------------------------------------------+
	if (BUILD_APP_DLL)
	{
		Str appMainPath = JoinPaths(appFolderPath, StrLit("app_main.c"));
		
		// +==============================+
		// |   Build App.dll on Windows   |
		// +==============================+
		if (BUILD_WINDOWS)
		{
			InitializeMsvcIf(StrLit("../core"), &isMsvcInitialized);
			PrintLine("\n[Building %.*s for Windows...]", StrPrint(filenameAppDll));
			
			CliArgs cmd = EMPTY;
			AddArgStr(&cmd, CLI_QUOTED_ARG, appMainPath);
			AddArgStr(&cmd, CL_BINARY_FILE, filenameAppDll);
			AddArgList(&cmd, &commonCompilerFlags);
			if (DUMP_ASSEMBLY) { AddArgStr(&cmd, CL_ASSEMB_LISTING_FILE, ChangePathExtension(filenameAppDll, StrLit(".asm"), true)); }
			AddArg(&cmd, CL_LINK);
			AddArg(&cmd, LINK_BUILD_DLL);
			AddArgList(&cmd, &commonLinkerFlags);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "pig_core.lib");
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_MSVC_CL);
			AddTag(&tags, T_WINDOWS);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_LIBRARY);
			AddTag(&tags, T_SHADER_OBJS);
			
			RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, FormatStr("Failed to build %.*s!", StrPrint(filenameAppDll)));
			AssertFileExist(filenameAppDll, true);
			PrintLine("[Built %.*s for Windows!]", StrPrint(filenameAppDll));
		}
		
		// +==============================+
		// |    Build App.so on Linux     |
		// +==============================+
		if (BUILD_LINUX)
		{
			PrintLine("\n[Building %.*s for Linux...]", StrPrint(filenameAppDll));
			IF_NOT_LINUX(MakeAndMoveIntoLinuxFolder());
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			IF_NOT_LINUX(cmd.rootDirPath = StrLit("../.."));
			AddArgStr(&cmd, CLI_QUOTED_ARG, appMainPath);
			AddArgStr(&cmd, CLANG_OUTPUT_FILE, filenameAppDll);
			AddArg(&cmd, CLANG_BUILD_SHARED_LIB);
			AddArg(&cmd, CLANG_fPIC);
			AddArgNt(&cmd, CLI_QUOTED_ARG, "libpig_core.so");
			AddArgList(&cmd, &commonCompilerFlags);
			AddArgList(&cmd, &commonLinkerFlags);
			
			StrArray tags = EMPTY;
			AddStrArray(&tags, &commonTags);
			AddTag(&tags, T_CLANG);
			AddTag(&tags, T_LINUX);
			AddTag(&tags, T_UNIX);
			AddTag(&tags, T_LANG_C);
			AddTag(&tags, T_LIBRARY);
			AddTag(&tags, T_SHADER_OBJS);
			
			Str clangExe = MakeStrNt(BUILDING_ON_LINUX ? EXE_CLANG : EXE_WSL_CLANG);
			RunCliProgramAndExitOnFailureTags(clangExe, tags, &cmd, FormatStr("Failed to build %.*s!", StrPrint(filenameAppDll)));
			AssertFileExist(filenameAppDll, true);
			PrintLine("[Built %.*s for Linux!]", StrPrint(filenameAppDll));
			
			IF_NOT_LINUX(PopOutOfLinuxFolder());
		}
		
		//TODO: Add OSX support
	}
	
	// +--------------------------------------------------------------+
	// |                    Create OSX App Bundle                     |
	// +--------------------------------------------------------------+
	Str appBundleDir = JoinStrings2(PROJECT_FOLDER_NAME, StrLit(".app"));
	if (BUILD_OSX && BUILD_INTO_SINGLE_UNIT)
	{
		Str appContentsDir = JoinPaths(appBundleDir, StrLit("Contents"));
		Str infoPlistPath  = JoinPaths(appContentsDir, StrLit("Info.plist"));
		Str appMacOSDir    = JoinPaths(appContentsDir, StrLit("MacOS"));
		
		MyCreateFolder(appBundleDir, false);
		MyCreateFolder(appContentsDir, false);
		MyCreateFolder(appMacOSDir, false);
		
		Str plistContents = FormatStr(
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
			"<plist version=\"1.0\">\n"
			"<dict>\n"
			"\t<key>CFBundleExecutable</key>\n"
			"\t<string>%.*s</string>\n"
			"\t<key>CFBundleIdentifier</key>\n"
			"\t<string>com.piggybankstudios.%.*s</string>\n"
			"\t<key>CFBundleName</key>\n"
			"\t<string>%.*s</string>\n"
			"\t<key>CFBundlePackageType</key>\n"
			"\t<string>APPL</string>\n"
			"\t<key>NSPrincipalClass</key>\n"
			"\t<string>NSApplication</string>\n"
			"\t<key>NSHighResolutionCapable</key>\n"
			"\t<true/>\n"
			"</dict>\n"
			"</plist>\n",
			StrPrint(filenameAppExe), //CFBundleExecutable
			StrPrint(PROJECT_EXE_NAME), //CFBundleIdentifier
			StrPrint(PROJECT_READABLE_NAME) //CFBundleName
		);
		CreateAndWriteFile(infoPlistPath, plistContents, true);
		
		CopyFileToFolder(filenameAppExe, appMacOSDir, true);
	}
	
	// +--------------------------------------------------------------+
	// |                    Copy to data Directory                    |
	// +--------------------------------------------------------------+
	if (COPY_TO_DATA_DIRECTORY)
	{
		Str dataFolder = StrLit("../data");
		PrintLine("Copying files to %.*s...", StrPrint(dataFolder));
		if (BUILD_APP_EXE)      { CopyFileToFolder(filenameAppExe,                    dataFolder, true); }
		if (BUILD_PIG_CORE_DLL) { CopyFileToFolder(StrLit(PIG_CORE_DLL_NAME DLL_EXT), dataFolder, true); }
		if (BUILD_APP_DLL)      { CopyFileToFolder(filenameAppDll,                    dataFolder, true); }
		if (PROFILING_ENABLED)  { CopyFileToFolder(StrLit("tracy" DLL_EXT),           dataFolder, true); }
		if (BUILD_OSX && BUILD_INTO_SINGLE_UNIT) { CopyFolderTo(appBundleDir, JoinPaths(dataFolder, appBundleDir), true, true); }
	}
	
	// +--------------------------------------------------------------+
	// |                   Run PROJECT_EXE_NAME.exe                   |
	// +--------------------------------------------------------------+
	if (RUN_APP)
	{
		Str runAppStr = JoinStrings2(StrLit(EXEC_PROGRAM_IN_FOLDER_PREFIX), filenameAppExe);
		PrintLine("\n[%.*s]", StrPrint(runAppStr));
		RunCliProgramAndExitOnFailure(runAppStr, nullptr, FormatStr("%.*s exited with error!", StrPrint(filenameAppExe)));
	}
	
	PrintLine("\n[%s Finished Successfully]", BUILD_SCRIPT_EXE_NAME);
	return 0;
}

#endif //  _PIG_BUILD_PIG_CORE_GUI_APP_H
