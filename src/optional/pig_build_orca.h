/*
File:   pig_build_orca.h
Author: Taylor Robbins
Date:   04\05\2026
Description:
	** Contains code that we only need if we are compiling for
	** Orca as a target platform for WebAssembly (WASM). See https://orca-app.dev/
*/

#ifndef _PIG_BUILD_ORCA_H
#define _PIG_BUILD_ORCA_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"
#include "pig_build_misc.h"

#define FILENAME_ORCA_SDK_PATH  "orca_sdk_path.txt"

#define T_ORCA "|Orca"

Str GetOrcaSdkPath()
{
	CliArgs cmd = EMPTY;
	AddArg(&cmd, "sdk-path");
	AddArgNt(&cmd, CLI_PIPE_OUTPUT_TO_FILE, FILENAME_ORCA_SDK_PATH);
	int statusCode = RunCliProgram(StrLit("orca"), &cmd);
	if (statusCode != 0)
	{
		PrintLine_E("Failed to run \"orca sdk-path\"! Status code: %d", statusCode);
		WriteLine_E("Make sure Orca SDK is installed and is added to the PATH!");
		exit(statusCode);
	}
	AssertFileExist(StrLit(FILENAME_ORCA_SDK_PATH), false);
	Str result = Str_Empty_Const;
	bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &result);
	assert(readSuccess == true);
	assert(result.length > 0);
	FixPathSlashes(result, PATH_SEP_CHAR);
	if (result.chars[result.length-1] == PATH_SEP_CHAR) { result.length--; } //no trailing slash
	return result;
}

void FillOrcaFlags(CliArgs* compilerFlags, CliArgs* linkerFlags, Str orcaSdkPath)
{
	AddTaggedArg(compilerFlags,    T_CLANG T_ORCA, CLANG_NO_ENTRYPOINT);
	AddTaggedArg(compilerFlags,    T_CLANG T_ORCA, CLANG_EXPORT_DYNAMIC);
	AddTaggedArgStr(compilerFlags, T_CLANG T_ORCA, CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc")));
	AddTaggedArgStr(compilerFlags, T_CLANG T_ORCA, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src")));
	AddTaggedArgStr(compilerFlags, T_CLANG T_ORCA, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext")));
	AddTaggedArgStr(compilerFlags, T_CLANG T_ORCA, CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin")));
	AddTaggedArgNt(compilerFlags,  T_CLANG T_ORCA, CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
	AddTaggedArgNt(linkerFlags,    T_CLANG T_ORCA, CLANG_SYSTEM_LIBRARY, "orca_wasm");
}

#endif //  _PIG_BUILD_ORCA_H
