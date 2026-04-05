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
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_file.h"
#include "pig_build_misc.h"
#include "pig_build_recompile.h"
#include "pig_build_arg_list.h"

int RunCliProgramTagArray(Str programPath, StrArray* tagsListPntr, const CliArgList* args)
{
	// PrintLine("Joining/filtering %llu arguments against %llu tags for \"%.*s\"", args->numArgs, (tagsListPntr != nullptr) ? tagsListPntr->length : 0ULL, StrPrint(programPath));
	Str joinedArgs = FilterAndJoinCliArgsList(programPath, args, tagsListPntr, true);
	#if PIG_BUILD_PRINT_SYS_CMDS
	PrintLine(">> %s", joinedArgs.chars);
	#endif
	fflush(stdout);
	fflush(stderr);
	int resultCode = system(joinedArgs.chars);
	free(joinedArgs.chars);
	return resultCode;
}
int RunCliProgram(Str programPath, const char* tagsListStr, const CliArgList* args)
{
	StrArray tagArray = ZEROED;
	SplitTagsListStr(MakeStrNt(tagsListStr), &tagArray);
	// if (tagArray.length > 0)
	// {
	// 	PrintLine("%.*s with %llu tag%s:", StrPrint(programPath), tagArray.length, Plural(tagArray.length, "s"));
	// 	for (u64 tIndex = 0; tIndex < tagArray.length; tIndex++) { PrintLine("\t[%llu] \"%.*s\"", tIndex, StrPrint(tagArray.strings[tIndex])); }
	// }
	int result = RunCliProgramTagArray(programPath, &tagArray, args);
	FreeStrArray(&tagArray);
	return result;
}
void RunCliProgramTagArrayAndExitOnFailure(Str programPath, StrArray* tagsListPntr, const CliArgList* args, Str errorMessage)
{
	int statusCode = RunCliProgramTagArray(programPath, tagsListPntr, args);
	if (statusCode != 0)
	{
		Str programNamePart = GetFileNamePart(programPath, true);
		PrintLine_E("%.*s\n%.*s Status Code: %d",
			StrPrint(errorMessage),
			StrPrint(programNamePart),
			statusCode
		);
		exit(statusCode);
	}
}
void RunCliProgramAndExitOnFailure(Str programPath, const char* tagListStr, const CliArgList* args, Str errorMessage)
{
	StrArray tagArray = ZEROED;
	SplitTagsListStr(MakeStrNt(tagListStr), &tagArray);
	RunCliProgramTagArrayAndExitOnFailure(programPath, &tagArray, args, errorMessage);
	FreeStrArray(&tagArray);
}

bool WasMsvcDevBatchRun()
{
	const char* versionEnvVarValue = getenv("VSCMD_VER");
    return (versionEnvVarValue != nullptr);
}

// We like to have a build_config.h that we pull information from to decide what kind of build we are doing.
// These functions help us find a particular #define in a C/C++ header file and retrieve it's value
Str ExtractStrDefine(Str buildConfigContents, Str defineName)
{
	Str defineValueStr = Str_Empty_Const;
	if (!TryExtractDefineFrom(buildConfigContents, defineName, &defineValueStr))
	{
		PrintLine_E("Couldn't find #define %.*s in build_config.h!", StrPrint(defineName));
		exit(4);
	}
	return defineValueStr;
}
bool ExtractBoolDefine(Str buildConfigContents, Str defineName)
{
	Str defineValueStr = ExtractStrDefine(buildConfigContents, defineName);
	bool result = false;
	if (!TryParseBoolArg(defineValueStr, &result))
	{
		PrintLine_E("#define %.*s has a non-bool value: \"%.*s\"", StrPrint(defineName), StrPrint(defineValueStr));
		exit(4);
	}
	return result;
}

// In order to avoid running VsDevCmd.bat every single time we compile, we run it once and dump the modified environment variables to a .txt file
// Then on later runs we just open this .txt file and apply all the environment variable values before trying to run the compiler
void ParseAndApplyEnvironmentVariables(Str environmentVars)
{
	u64 lineIndex = 0;
	u64 lineStart = 0;
	u64 equalsIndex = 0;
	for (u64 cIndex = 0; cIndex < environmentVars.length; cIndex++)
	{
		char character = environmentVars.chars[cIndex];
		char nextChar = (cIndex+1 < environmentVars.length) ? environmentVars.chars[cIndex+1] : '\0';
		if (character == '\n' || (character == '\r' && nextChar == '\n'))
		{
			Str line = MakeStr(cIndex - lineStart, &environmentVars.chars[lineStart]);
			
			if (equalsIndex >= lineStart)
			{
				Str varName = CopyStr(StrSlice(line, 0, equalsIndex-lineStart), true);
				Str varValue = CopyStr(StrSliceFrom(line, (equalsIndex-lineStart)+1), true);
				
				// PrintLine("set %.*s=%.*s", StrPrint(varName), StrPrint(varValue));
				#if BUILDING_ON_WINDOWS
				_putenv_s(varName.chars, varValue.chars);
				#else
				Str varEqualsValueStr = JoinStrings3(varName, StrLit("="), varValue, true);
				putenv(varEqualsValueStr.chars);
				#endif
				free(varName.chars);
				free(varValue.chars);
			}
			else if (line.length > 0)
			{
				PrintLine_E("WARNING: No \'=\' character found in line %llu of environment file. Ignoring line: \"%.*s\"", lineIndex+1, StrPrint(line));
			}
			
			if (character == '\r' && nextChar == '\n') { cIndex++; }
			lineStart = cIndex + 1;
			lineIndex++;
		}
		if (character == '=') { equalsIndex = cIndex; }
	}
}
// We expect the script at batchFilePath to do `set > "%~1"` at the end to dump all environment variables into environmentFilePath
void RunBatchFileAndApplyDumpedEnvironment(Str batchFilePath, Str environmentFilePath, bool skipRunningIfFileExists)
{
	CliArgList cmd = ZEROED;
	AddArgStr(&cmd, CLI_QUOTED_ARG, environmentFilePath);
	Str fixedBatchFilePath = CopyStr(batchFilePath, false);
	FixPathSlashes(fixedBatchFilePath, PATH_SEP_CHAR);
	
	if (!DoesFileExist(environmentFilePath) || !skipRunningIfFileExists)
	{
		int statusCode = RunCliProgram(fixedBatchFilePath, "", &cmd);
		if (statusCode != 0)
		{
			PrintLine_E("%.*s failed! Status Code: %d", StrPrint(fixedBatchFilePath), statusCode);
			exit(statusCode);
		}
	}
	
	Str environmentFileContents = Str_Empty_Const;
	if (!TryReadFile(environmentFilePath, &environmentFileContents))
	{
		PrintLine_E("%.*s did not create \"%.*s\"! Or we can't open it for some reason", StrPrint(batchFilePath), StrPrint(environmentFilePath));
		exit(4);
	}
	
	ParseAndApplyEnvironmentVariables(environmentFileContents);
	
	free(fixedBatchFilePath.chars);
	free(environmentFileContents.chars);
}

// We only need initialize MSVC once but we may not need to initialize at all.
// So we pass a pointer to a bool that tracks if we have initialized and we pepper
// these calls before any spot in the build_script.c that needs to use the MSVC compiler
void InitializeMsvcIf(Str pigCoreFolder, bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		Str batchPath = JoinStrings2(pigCoreFolder, StrLit("/" PIG_BUILD_FOLDER_NAME "/shell/init_msvc.bat"), false);
		Str environmentPath = StrLit_Const(MSVC_ENVIRONMENT_TXT_PATH);
		if (DoesFileExist(environmentPath)) { WriteLine("Loading MSVC Environment..."); }
		else { WriteLine("Initializing MSVC Compiler..."); }
		RunBatchFileAndApplyDumpedEnvironment(batchPath, environmentPath, true);
		*isMsvcInitialized = true;
	}
}

// This is mostly useful for WebAssembly builds where we need to do stitching of multiple Javascript files into one
// TODO: We could use something like WebPack to minify and join but it doesn't seem worth it right now
void ConcatAllFilesIntoSingleFile(const StrArray* pathArray, Str outputFilePath)
{
	//TODO: We really should handle new-line differences between Windows and Linux/etc. a little smarter here
	//      Just because we are building on Windows doesn't mean all these .js files are using Windows style line-endings
	
	StrArray allFilesContents = ZEROED;
	u64 totalLength = 0;
	for (u64 fIndex = 0; fIndex < pathArray->length; fIndex++)
	{
		Str inputPath = pathArray->strings[fIndex];
		Str inputFileContents = Str_Empty_Const;
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
	
	Str combinedContents = AllocStr(totalLength, true);
	
	u64 writeIndex = 0;
	for (u64 fIndex = 0; fIndex < allFilesContents.length; fIndex++)
	{
		Str inputFileContents = allFilesContents.strings[fIndex];
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

#define FILENAME_ORCA_SDK_PATH  "orca_sdk_path.txt"

Str GetOrcaSdkPath()
{
	CliArgList cmd = ZEROED;
	AddArg(&cmd, "sdk-path");
	AddArgNt(&cmd, CLI_PIPE_OUTPUT_TO_FILE, FILENAME_ORCA_SDK_PATH);
	int statusCode = RunCliProgram(StrLit("orca"), "", &cmd);
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

#endif //  _PIG_BUILD_BUILD_HELPERS_H
