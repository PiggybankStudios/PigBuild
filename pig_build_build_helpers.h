/*
File:   pig_build_build_helpers.h
Author: Taylor Robbins
Date:   06\19\2025
Description:
	** Holds a bunch of functions that don't really have a proper "home" in another file
	** Many of these things are somewhat niche but are re-used between projects still
*/

#ifndef _PIG_BUILD_BUILD_HELPERS_H
#define _PIG_BUILD_BUILD_HELPERS_H

#include "pig_build_base.h"
#include "pig_build_str8.h"
#include "pig_build_str_array.h"
#include "pig_build_file.h"
#include "pig_build_misc.h"
#include "pig_build_recompile.h"
#include "pig_build_cli.h"

Str8 ExtractStrDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ZEROED;
	if (!TryExtractDefineFrom(buildConfigContents, defineName, &defineValueStr))
	{
		PrintLine_E("Couldn't find #define %.*s in build_config.h!", StrPrint(defineName));
		exit(4);
	}
	return defineValueStr;
}
bool ExtractBoolDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ExtractStrDefine(buildConfigContents, defineName);
	bool result = false;
	if (!TryParseBoolArg(defineValueStr, &result))
	{
		PrintLine_E("#define %.*s has a non-bool value: \"%.*s\"", StrPrint(defineName), StrPrint(defineValueStr));
		exit(4);
	}
	return result;
}

void RunBatchFileAndApplyDumpedEnvironment(Str8 batchFilePath, Str8 environmentFilePath, bool skipRunningIfFileExists)
{
	CliArgList cmd = ZEROED;
	AddArgStr(&cmd, CLI_QUOTED_ARG, environmentFilePath);
	Str8 fixedBatchFilePath = CopyStr8(batchFilePath, false);
	FixPathSlashes(fixedBatchFilePath, PATH_SEP_CHAR);
	
	if (!DoesFileExist(environmentFilePath) || !skipRunningIfFileExists)
	{
		int statusCode = RunCliProgram(fixedBatchFilePath, &cmd); //this batch file runs emsdk_env.bat and then dumps it's environment variables to environment.txt. We can then open and parse that file and change our environment to match what emsdk_env.bat changed
		if (statusCode != 0)
		{
			PrintLine_E("%.*s failed! Status Code: %d", StrPrint(fixedBatchFilePath), statusCode);
			exit(statusCode);
		}
	}
	
	Str8 environmentFileContents = ZEROED;
	if (!TryReadFile(environmentFilePath, &environmentFileContents))
	{
		PrintLine_E("%.*s did not create \"%.*s\"! Or we can't open it for some reason", StrPrint(batchFilePath), StrPrint(environmentFilePath));
		exit(4);
	}
	
	ParseAndApplyEnvironmentVariables(environmentFileContents);
	
	free(fixedBatchFilePath.chars);
	free(environmentFileContents.chars);
}

void InitializeMsvcIf(Str8 pigCoreFolder, bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/" PIG_BUILD_FOLDER_NAME "/shell/init_msvc.bat"), false);
		Str8 environmentPath = StrLit_Const(MSVC_ENVIRONMENT_TXT_PATH);
		if (DoesFileExist(environmentPath)) { WriteLine("Loading MSVC Environment..."); }
		else { WriteLine("Initializing MSVC Compiler..."); }
		RunBatchFileAndApplyDumpedEnvironment(batchPath, environmentPath, true);
		*isMsvcInitialized = true;
	}
}

void InitializeEmsdkIf(Str8 pigCoreFolder, bool* isEmsdkInitialized)
{
	if (*isEmsdkInitialized == false)
	{
		PrintLine("Initializing Emscripten SDK...");
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/" PIG_BUILD_FOLDER_NAME "/shell/init_emsdk.bat"), false);
		RunBatchFileAndApplyDumpedEnvironment(batchPath, StrLit(EMSDK_ENVIRONMENT_TXT_PATH), false);
		*isEmsdkInitialized = true;
	}
}

//This is mostly useful for WebAssembly builds where we need to do stitching of multiple Javascript files into one
void ConcatAllFilesIntoSingleFile(const StrArray* pathArray, Str8 outputFilePath)
{
	//TODO: We really should handle new-line differences between Windows and Linux/etc. a little smarter here
	//      Just because we are building on Windows doesn't mean all these .js files are using Windows style line-endings
	
	StrArray allFilesContents = ZEROED;
	u64 totalLength = 0;
	for (u64 fIndex = 0; fIndex < pathArray->length; fIndex++)
	{
		Str8 inputPath = pathArray->strings[fIndex];
		Str8 inputFileContents = ZEROED;
		if (!TryReadFile(inputPath, &inputFileContents))
		{
			PrintLine_E("Couldn't find/open \"%.*s\"!", StrPrint(inputPath));
			exit(8);
		}
		AddStr(&allFilesContents, inputFileContents);
		if (totalLength > 0) { totalLength += BUILDING_ON_WINDOWS ? 2 : 1; } //+1-2 for the new-line between each file
		totalLength += inputFileContents.length;
		free(inputFileContents.chars);
	}
	
	Str8 combinedContents = ZEROED;
	combinedContents.length = totalLength;
	combinedContents.pntr = malloc(combinedContents.length + 1);
	
	u64 writeIndex = 0;
	for (u64 fIndex = 0; fIndex < allFilesContents.length; fIndex++)
	{
		Str8 inputFileContents = allFilesContents.strings[fIndex];
		if (writeIndex > 0)
		{
			#if BUILDING_ON_WINDOWS
			combinedContents.chars[writeIndex+0] = '\r';
			combinedContents.chars[writeIndex+1] = '\n';
			writeIndex += 2;
			#else
			combinedContents.chars[writeIndex] = '\n';
			writeIndex += 1;
			#endif
		}
		memcpy(&combinedContents.chars[writeIndex], inputFileContents.chars, inputFileContents.length);
		writeIndex += inputFileContents.length;
	}
	assert(writeIndex == combinedContents.length);
	combinedContents.chars[combinedContents.length] = '\0';
	
	CreateAndWriteFile(outputFilePath, combinedContents, false);
	
	FreeStrArray(&allFilesContents);
}

Str8 GetEmscriptenSdkPath()
{
	const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
		exit(7);
	}
	Str8 result = MakeStr8Nt(sdkEnvVariable);
	if (IsSlash(result.chars[result.length-1])) { result.length--; } //no trailing slash
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

#define FILENAME_ORCA_SDK_PATH  "orca_sdk_path.txt"

Str8 GetOrcaSdkPath()
{
	CliArgList cmd = ZEROED;
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
	Str8 result = ZEROED;
	bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &result);
	assert(readSuccess == true);
	assert(result.length > 0);
	FixPathSlashes(result, PATH_SEP_CHAR);
	if (result.chars[result.length-1] == PATH_SEP_CHAR) { result.length--; } //no trailing slash
	return result;
}

Str8 GetPlaydateSdkPath()
{
	const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
		exit(7);
	}
	Str8 result = MakeStr8Nt(sdkEnvVariable);
	if (IsSlash(result.chars[result.length-1])) { result.length--; }
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

#endif //  _PIG_BUILD_BUILD_HELPERS_H
