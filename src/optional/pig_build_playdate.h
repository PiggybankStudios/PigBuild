/*
File:   pig_build_playdate.h
Author: Taylor Robbins
Date:   04\05\2026
Description:
	** Contains some logic that we only need if we are compiling
	** for the Panic Playdate handheld gaming console. See https://play.date/
*/

#ifndef _PIG_BUILD_PLAYDATE_H
#define _PIG_BUILD_PLAYDATE_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"

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
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_LOGO);
		// AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_WARNING_LEVEL, "3");
		// AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_WARNINGS_AS_ERRORS);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_STD_LIB_DYNAMIC_DBG);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_OPTIMIZATION_LEVEL, "d");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "2");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD", CL_DEBUG_INFO);
		
		//TODO: Just use cl_LangCFlags?
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_LANG_VERSION, "clatest"); //Use latest C language spec features
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
		
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CL_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0)
		{
			AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, playdateSdkDir_C_API);
			AddTaggedArgStr(compilerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CL_INCLUDE_DIR, playdateSdkDir_C_API);
		}
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "TARGET_SIMULATOR=1");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDLL");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_MBCS");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "WIN32");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDOWS");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDLL=1");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_BUFFER_SECURITY_CHECK);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DISABLE_MINIMAL_REBUILD);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
	}
	
	// +====================================+
	// | link_PlaydateSimulatorLinkerFlags  |
	// +====================================+
	{
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_NO_LOGO);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_TARGET_ARCHITECTURE, "X64");
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_DATA_EXEC_COMPAT);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_ENABLE_ASLR);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_CONSOLE_APPLICATION);
		AddTaggedArgInt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", LINK_TYPELIB_RESOURCE_ID, 1);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_ENABLE_INCREMENTAL);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_CREATE_ASSEMBLY_MANIFEST);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator|DEBUG_BUILD", LINK_DEBUG_INFO);
	}
	
	// +==================================+
	// | link_PlaydateSimulatorLibraries  |
	// +==================================+
	{
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "kernel32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "user32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "gdi32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "winspool.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "shell32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "ole32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "oleaut32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "uuid.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "comdlg32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "advapi32.lib");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceCommonFlags |
	// +===============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0) { AddTaggedArgStr(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_INCLUDE_DIR, playdateSdkDir_C_API); }
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "TARGET_PLAYDATE=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__FPU_USED=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_THUMB);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_CPU, "cortex-m7");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_FPU, "fpv5-sp-d16");
	}
	
	// +==================================+
	// | gcc_PlaydateDeviceCompilerFlags  |
	// +==================================+
	{
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEBUG_INFO_EX, "3");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEBUG_INFO_EX, "dwarf-2");
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgInt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ALIGN_FUNCS_TO, 16);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_SEP_DATA_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_SEP_FUNC_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_EXCEPTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_OMIT_FRAME_PNTR);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_GLOBAL_VAR_NO_COMMON);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ONLY_RELOC_WORD_SIZE);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_WARNING_LEVEL, "all");
		// AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ENABLE_WARNING, "double-promotion");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "unknown-pragmas");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "comment");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "switch");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "nonnull");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "unused");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "missing-braces");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "char-subscripts");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceLinkerFlags |
	// +===============================+
	{
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_NO_STD_STARTUP);
		AddTaggedArgNt(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ENTRYPOINT_NAME, "eventHandler");
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_RWX_WARNING);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_CREF);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_GC_SECTIONS);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_MISMATCH_WARNING);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_EMIT_RELOCATIONS);
		AddTaggedArgStr(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_LINKER_SCRIPT, JoinStrings2(playdateSdkDir, StrLit("/C_API/buildsupport/link_map.ld"), false));
	}
	
	// +==============================+
	// |       pdc_CommonFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, EXE_PDC "|Playdate", PDC_QUIET); //Quiet mode, suppress non-error output
		if (playdateSdkDir.length > 0) { AddTaggedArgStr(compilerFlags, EXE_PDC "|Playdate", PDC_SDK_PATH, playdateSdkDir); }
	}
}

#endif //  _PIG_BUILD_PLAYDATE_H
