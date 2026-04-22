/*
File:   pig_build_emscripten.h
Author: Taylor Robbins
Date:   04\05\2026
Description:
	** Contains code that we only need if we are compiling for
	** Web\WASM using the Emscripten toolchain. See https://emscripten.org/
	** NOTE: For now, the user must set EMSCRIPTEN_SDK_PATH for this to work
*/

#ifndef _PIG_BUILD_EMSCRIPTEN_H
#define _PIG_BUILD_EMSCRIPTEN_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_misc.h"

//NOTE: See shell/init_emsdk.bat

#define EXE_EMSCRIPTEN_COMPILER "emcc"
//NOTE: Emscripten's compiler accepts all the same flags that Clang does
#define EMSCRIPTEN_S_FLAG "-s[VAL]" //TODO: Give this a better name!

#define EMSDK_ENVIRONMENT_TXT_PATH "emsdk_environment.txt"

#define T_EMCC "|emcc"

// NOTE: For the time being we just require the user to set up an EMSCRIPTEN_SDK_PATH environment variable to tell us where the Emscripten SDK lives
Str GetEmscriptenSdkPath()
{
	const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
		exit(7);
	}
	Str result = CopyStr(WithoutTrailingSlash(MakeStrNt(sdkEnvVariable)));
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

bool WasEmsdkEnvBatchRun()
{
	const char* sdkEnvVarValue = getenv("EMSDK");
    return (sdkEnvVarValue != nullptr);
}

void InitializeEmsdkIf(Str pigCoreFolder, bool* isEmsdkInitialized)
{
	if (*isEmsdkInitialized == false)
	{
		PrintLine("Initializing Emscripten SDK...");
		Str batchPath = JoinStrings2(pigCoreFolder, StrLit("/" PIG_BUILD_FOLDER_NAME "/shell/init_emsdk.bat"));
		RunBatchFileAndApplyDumpedEnvironment(batchPath, StrLit(EMSDK_ENVIRONMENT_TXT_PATH), false);
		*isEmsdkInitialized = true;
	}
}

#endif //  _PIG_BUILD_EMSCRIPTEN_H
