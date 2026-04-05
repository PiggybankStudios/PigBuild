/*
File:   pig_build_playdate.h
Author: Taylor Robbins
Date:   04\05\2026
Description:
	** Contains some logic that we only need if we are compiling
	** for the Panic Playdate handheld gaming console. See https://play.date/
	** NOTE: For now, the user must set PLAYDATE_SDK_PATH for this to work
*/

#ifndef _PIG_BUILD_PLAYDATE_H
#define _PIG_BUILD_PLAYDATE_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"

#define EXE_PDC "pdc"
#define EXE_ARM_GCC "arm-none-eabi-gcc" //we use this when compiling for the Playdate device

#define PDC_SDK_PATH      "-sdkpath \"[VAL]\"" //use the SDK at the given path instead of the default
#define PDC_LIBPATH       "--libpath \"[VAL]\"" //add the given path to the list of folders to search when resolving imports
#define PDC_STRIP         "--strip" //strip debug symbols
#define PDC_NO_COMPRESS   "--no-compress" //don't compress output files
#define PDC_MAIN          "--main" //compile lua script at <input> as if it were main.lua
#define PDC_VERBOSE       "--verbose" //verbose mode, gives info about what the compiler is doing
#define PDC_QUIET         "--quiet" //quiet mode, suppresses non-error output
#define PDC_SKIP_UNKNOWN  "--skip-unknown" //skip unrecognized files instead of copying them to the pdx folder
#define PDC_CHECK_FONTS   "--check-fonts" //perform additional validation on font data (may produce false warnings)

#define T_PDC           "|pdc"
#define T_ARM_GCC       "|arm-none-eabi-gcc"
#define T_PLAYDATE      "|Playdate"
#define T_DEVICE        "|Device"
#define T_SIMULATOR     "|Simulator"

// NOTE: For the time being we just require the user to set up an PLAYDATE_SDK_PATH environment variable to tell us where the Playdate SDK lives
Str GetPlaydateSdkPath()
{
	const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
		exit(7);
	}
	Str result = CopyStr(WithoutTrailingSlash(MakeStrNt(sdkEnvVariable)), true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

void FillPlaydateFlags(CliArgList* compilerFlags, CliArgList* linkerFlags, Str playdateSdkDir, Str playdateSdkDir_C_API)
{
	// +====================================+
	// | cl_PlaydateSimulatorCompilerFlags  |
	// +====================================+
	{
		//TODO: Just use cl_CommonFlags?
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_NO_LOGO);
		// AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_WARNING_LEVEL, "3");
		// AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_NO_WARNINGS_AS_ERRORS);
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR T_DEBUG_BUILD,  CL_STD_LIB_DYNAMIC_DBG);
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR T_RELEASE_BUILD, CL_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR T_DEBUG_BUILD,  CL_OPTIMIZATION_LEVEL, "d");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR T_RELEASE_BUILD, CL_OPTIMIZATION_LEVEL, "2");
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR T_DEBUG_BUILD, CL_DEBUG_INFO);
		
		//TODO: Just use cl_LangCFlags?
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
		
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgNt(compilerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CL_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0)
		{
			AddTaggedArgStr(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_INCLUDE_DIR, playdateSdkDir_C_API);
			AddTaggedArgStr(compilerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CL_INCLUDE_DIR, playdateSdkDir_C_API);
		}
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "TARGET_SIMULATOR=1");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "_WINDLL");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "_MBCS");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "WIN32");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "_WINDOWS");
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DEFINE, "_WINDLL=1");
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_ENABLE_BUFFER_SECURITY_CHECK);
		AddTaggedArg(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_DISABLE_MINIMAL_REBUILD);
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
		AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PLAYDATE T_SIMULATOR, CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
	}
	
	// +====================================+
	// | link_PlaydateSimulatorLinkerFlags  |
	// +====================================+
	{
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_NO_LOGO);
		AddTaggedArgNt(linkerFlags,  T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_TARGET_ARCHITECTURE, "X64");
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_DATA_EXEC_COMPAT);
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_ENABLE_ASLR);
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_CONSOLE_APPLICATION);
		AddTaggedArgInt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_TYPELIB_RESOURCE_ID, 1);
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_ENABLE_INCREMENTAL);
		AddTaggedArgNt(linkerFlags,  T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_CREATE_ASSEMBLY_MANIFEST);
		AddTaggedArgNt(linkerFlags,  T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  T_MSVC_LINK T_PLAYDATE T_SIMULATOR, LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
		AddTaggedArg(linkerFlags,    T_MSVC_LINK T_PLAYDATE T_SIMULATOR T_DEBUG_BUILD, LINK_DEBUG_INFO);
	}
	
	// +==================================+
	// | link_PlaydateSimulatorLibraries  |
	// +==================================+
	{
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "kernel32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "user32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "gdi32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "winspool.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "shell32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "ole32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "oleaut32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "uuid.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "comdlg32.lib");
		AddTaggedArgNt(linkerFlags, T_MSVC_LINK T_PLAYDATE T_SIMULATOR, CLI_QUOTED_ARG, "advapi32.lib");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceCommonFlags |
	// +===============================+
	{
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0) { AddTaggedArgStr(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_INCLUDE_DIR, playdateSdkDir_C_API); }
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEFINE, "TARGET_PLAYDATE=1");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEFINE, "__FPU_USED=1");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_TARGET_THUMB);
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_TARGET_CPU, "cortex-m7");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_TARGET_FPU, "fpv5-sp-d16");
	}
	
	// +==================================+
	// | gcc_PlaydateDeviceCompilerFlags  |
	// +==================================+
	{
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEBUG_INFO_EX, "3");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEBUG_INFO_EX, "dwarf-2");
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgInt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_ALIGN_FUNCS_TO, 16);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_SEP_DATA_SECTIONS);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_SEP_FUNC_SECTIONS);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_EXCEPTIONS);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_OMIT_FRAME_PNTR);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_GLOBAL_VAR_NO_COMMON);
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
		AddTaggedArg(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_ONLY_RELOC_WORD_SIZE);
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_WARNING_LEVEL, "all");
		// AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_ENABLE_WARNING, "double-promotion");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "unknown-pragmas");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "comment");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "switch");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "nonnull");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "unused");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "missing-braces");
		AddTaggedArgNt(compilerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_WARNING, "char-subscripts");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceLinkerFlags |
	// +===============================+
	{
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_NO_STD_STARTUP);
		AddTaggedArgNt(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_ENTRYPOINT_NAME, "eventHandler");
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_RWX_WARNING);
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_CREF);
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_GC_SECTIONS);
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_DISABLE_MISMATCH_WARNING);
		AddTaggedArg(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_EMIT_RELOCATIONS);
		AddTaggedArgStr(linkerFlags, T_ARM_GCC T_PLAYDATE T_DEVICE, GCC_LINKER_SCRIPT, JoinStrings2(playdateSdkDir, StrLit("/C_API/buildsupport/link_map.ld"), false));
	}
	
	// +==============================+
	// |       pdc_CommonFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, T_PDC T_PLAYDATE, PDC_QUIET); //Quiet mode, suppress non-error output
		if (playdateSdkDir.length > 0) { AddTaggedArgStr(compilerFlags, T_PDC T_PLAYDATE, PDC_SDK_PATH, playdateSdkDir); }
	}
}

#endif //  _PIG_BUILD_PLAYDATE_H
