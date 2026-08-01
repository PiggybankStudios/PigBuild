/*
File:   pig_build_misc.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Contains a variety of functions that don't have a "home" in another file.
*/

#ifndef _PIG_BUILD_MISC_H
#define _PIG_BUILD_MISC_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_file.h"
#include "pig_build_misc.h"
#include "pig_build_recompile.h"
#include "pig_build_arg_list.h"

// +--------------------------------------------------------------+
// |                        RunCliProgram                         |
// +--------------------------------------------------------------+
int RunCliProgramTags(Str programPath, StrArray tagsList, const CliArgs* args)
{
	// PrintLine("Joining/filtering %llu arguments against %llu tags for \"%.*s\"", args->array.length, tagsList.length, StrPrint(programPath));
	// for (u64 tIndex = 0; tIndex < tagsList.length; tIndex++) { PrintLine("Tag[%llu]: \"%.*s\"", tIndex, StrPrint(tagsList.strings[tIndex])); }
	Str joinedArgs = (args != nullptr) ? FilterAndJoinCliArgsList(programPath, args, &tagsList) : programPath;
	#if PIG_BUILD_PRINT_SYS_CMDS
	u64 numMatchingArgs = GetNumMatchingArgs(args, &tagsList);
	if (tagsList.length > 0)
	{
		PrintLine(">> Matched %llu/%llu argument%s on %llu tag%s",
			numMatchingArgs, args->array.length, Plural(numMatchingArgs, "s"),
			tagsList.length, Plural(tagsList.length, "s")
		);
		// for (u64 tIndex = 0; tIndex < tagsList.length; tIndex++){ PrintLine("\tTag[%llu]: \"%.*s\"", tIndex, StrPrint(tagsList.strings[tIndex])); }
	}
	else
	{
		u64 numArgs = (args != nullptr) ? args->array.length : 0;
		PrintLine(">> No tags given. All %llu arg%s match", numArgs, Plural(numArgs, "s"));
	}
	PrintLine(">> %s", joinedArgs.chars);
	#endif
	fflush(stdout);
	fflush(stderr);
	int resultCode = system(joinedArgs.chars);
	free(joinedArgs.chars);
	return resultCode;
}
#define RunCliProgram(programPathStr, argsPntr)                         RunCliProgramTags((programPathStr), SplitTagsLit(""), (argsPntr))
#define RunCliProgramTagsNt(programPathStr, tagsListNullTerm, argsPntr) RunCliProgramTags((programPathStr), SplitTagsNt(tagsListNullTerm), (argsPntr))
#define RunCliProgramTagsLit(programPathStr, tagsListStrLit, argsPntr)  RunCliProgramTags((programPathStr), SplitTagsLit(tagsListStrLit),  (argsPntr))

void RunCliProgramAndExitOnFailureTags(Str programPath, StrArray tagsList, const CliArgs* args, Str errorMessage)
{
	int statusCode = RunCliProgramTags(programPath, tagsList, args);
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
#define RunCliProgramAndExitOnFailure(programPathStr, argsPntr, errorMessageStr)                         RunCliProgramAndExitOnFailureTags((programPathStr), SplitTagsLit(""), (argsPntr), (errorMessageStr))
#define RunCliProgramAndExitOnFailureTagsNt(programPathStr, tagsListNullTerm, argsPntr, errorMessageStr) RunCliProgramAndExitOnFailureTags((programPathStr), SplitTagsNt(tagsListNullTerm), (argsPntr), (errorMessageStr))
#define RunCliProgramAndExitOnFailureTagsLit(programPathStr, tagsListStrLit, argsPntr, errorMessageStr)  RunCliProgramAndExitOnFailureTags((programPathStr), SplitTagsLit(tagsListStrLit),  (argsPntr), (errorMessageStr))

// +--------------------------------------------------------------+
// |                            Types                             |
// +--------------------------------------------------------------+
typedef struct LineParser LineParser;
struct LineParser
{
	u64 byteIndex;
	u64 lineBeginByteIndex;
	u64 lineIndex; //This is not zero based! It's more like a line number you'd see in the gutter of a text editor! It also contains the total number of lines in the input after the iteration has finished
	Str inputStr;
	//TODO: Should we add support for Streams again?
};

// +--------------------------------------------------------------+
// |                        Print Helpers                         |
// +--------------------------------------------------------------+
void TwoPassPrint(Str* resultStr, u64* currentByteIndex, const char* formatString, ...)
{
	u64 printSize = 0;
	va_list args;
	
	va_start(args, formatString);
	int measureResult = vsnprintf(nullptr, 0, formatString, args);
	va_end(args);
	Assert(measureResult >= 0);
	
	printSize = (u64)measureResult;
	if (resultStr->chars != nullptr)
	{
		Assert(*currentByteIndex <= resultStr->length);
		u64 spaceLeft = resultStr->length - *currentByteIndex;
		Assert(printSize <= spaceLeft);
		va_start(args, formatString);
		int printResult = vsnprintf(&resultStr->chars[*currentByteIndex], measureResult+1, formatString, args);
		Assert(printResult == measureResult);
		resultStr->chars[*currentByteIndex + printSize] = '\0';
		va_end(args);
	}
	
	*currentByteIndex += printSize;
}

// +--------------------------------------------------------------+
// |                         Line Parser                          |
// +--------------------------------------------------------------+
LineParser NewLineParser(Str inputStr)
{
	LineParser result = EMPTY;
	result.byteIndex = 0;
	result.lineIndex = 0;
	result.inputStr = inputStr;
	return result;
}

bool LineParserGetLine(LineParser* parser, Str* lineOut)
{
	if (parser->byteIndex >= parser->inputStr.length) { return false; }
	parser->lineIndex++;
	parser->lineBeginByteIndex = parser->byteIndex;
	
	u64 endOfLineByteSize = 0;
	u64 startIndex = parser->byteIndex;
	while (parser->byteIndex < parser->inputStr.length)
	{
		char nextChar = parser->inputStr.chars[parser->byteIndex];
		char nextNextChar = parser->inputStr.chars[parser->byteIndex+1];
		//TODO: Should we handle \n\r sequence? Windows is \r\n and I don't know of any space where \n\r is considered a valid single new-line
		if (nextChar != nextNextChar &&
			(nextChar     == '\n' || nextChar     == '\r') &&
			(nextNextChar == '\n' || nextNextChar == '\r'))
		{
			endOfLineByteSize = 2;
			break;
		}
		else if (nextChar == '\n' || nextChar == '\r')
		{
			endOfLineByteSize = 1;
			break;
		}
		else
		{
			parser->byteIndex++;
		}
	}
	
	Str line = MakeStr(parser->byteIndex - startIndex, &parser->inputStr.chars[startIndex]);
	parser->byteIndex += endOfLineByteSize;
	if (lineOut != nullptr) { *lineOut = line; }
	return true;
}

// +--------------------------------------------------------------+
// |                     Extract Define Logic                     |
// +--------------------------------------------------------------+
bool IsHeaderLineDefine(Str targetDefineName, Str line, Str* valueStrOut)
{
	line = TrimWhitespace(line);
	u64 firstWhitespaceIndex = FindNextWhitespace(line, 0);
	if (firstWhitespaceIndex < line.length)
	{
		Str firstToken = StrSlice(line, 0, firstWhitespaceIndex);
		if (StrExactEquals(firstToken, StrLit("#define")))
		{
			line = TrimWhitespace(StrSliceFrom(line, firstWhitespaceIndex+1));
			u64 identifierEndIndex = FindNextNonIdentifierChar(line, 0);
			if (identifierEndIndex < line.length)
			{
				Str nameStr = StrSlice(line, 0, identifierEndIndex);
				if (StrExactEquals(nameStr, targetDefineName))
				{
					Str valueStr = TrimWhitespace(StrSliceFrom(line, identifierEndIndex+1));
					if (valueStrOut != nullptr) { *valueStrOut = valueStr; }
					return true;
				}
			}
		}
	}
	return false;
}

bool TryExtractDefineFrom(Str headerFileContents, Str defineName, Str* valueOut)
{
	u64 lineStartIndex = 0;
	for (u64 byteIndex = 0; byteIndex < headerFileContents.length; byteIndex++)
	{
		char character = headerFileContents.chars[byteIndex];
		char nextCharacter = headerFileContents.chars[byteIndex+1]; //requires null-terminator we added above
		if (character == '\n' ||
			(character == '\r' && nextCharacter == '\n') ||
			(character == '\n' && nextCharacter == '\r'))
		{
			bool isTwoCharacterNewLine =
				(character == '\r' && nextCharacter == '\n') ||
				(character == '\n' && nextCharacter == '\r');
			
			Str lineStr = MakeStr(byteIndex - lineStartIndex, &headerFileContents.chars[lineStartIndex]);
			
			Str defineValue = Str_Empty_Const;
			if (IsHeaderLineDefine(defineName, lineStr, &defineValue))
			{
				if (valueOut != nullptr) { *valueOut = defineValue; }
				return true;
			}
			
			if (isTwoCharacterNewLine) { byteIndex++; }
			lineStartIndex = byteIndex+1;
		}
	}
	return false;
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

// This is mostly useful for WebAssembly builds where we need to do stitching of multiple Javascript files into one
// TODO: We could use something like WebPack to minify and join but it doesn't seem worth it right now
void ConcatAllFilesIntoSingleFile(const StrArray* pathArray, Str outputFilePath)
{
	//TODO: We really should handle new-line differences between Windows and Linux/etc. a little smarter here
	//      Just because we are building on Windows doesn't mean all these .js files are using Windows style line-endings
	
	StrArray allFilesContents = EMPTY;
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
	
	Str combinedContents = AllocStr(totalLength);
	
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
	Assert(writeIndex == combinedContents.length);
	combinedContents.chars[combinedContents.length] = '\0';
	
	CreateAndWriteFile(outputFilePath, combinedContents, false);
	
	FreeStrArray(&allFilesContents);
}

// +--------------------------------------------------------------+
// |                       Windows Helpers                        |
// +--------------------------------------------------------------+
bool WasMsvcDevBatchRun()
{
	const char* versionEnvVarValue = getenv("VSCMD_VER");
    return (versionEnvVarValue != nullptr);
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
				Str varName = CopyStr(StrSlice(line, 0, equalsIndex-lineStart));
				Str varValue = CopyStr(StrSliceFrom(line, (equalsIndex-lineStart)+1));
				
				// PrintLine("set %.*s=%.*s", StrPrint(varName), StrPrint(varValue));
				#if BUILDING_ON_WINDOWS
				_putenv_s(varName.chars, varValue.chars);
				#else
				Str varEqualsValueStr = JoinStrings3(varName, StrLit("="), varValue);
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
//TODO: This should be renamed, it only runs the batch file if the environment file doesn't exist!
void RunBatchFileAndApplyDumpedEnvironment(Str batchFilePath, Str environmentFilePath, bool skipRunningIfFileExists)
{
	CliArgs cmd = EMPTY;
	AddArgStr(&cmd, CLI_QUOTED_ARG, environmentFilePath);
	Str fixedBatchFilePath = CopyStr(batchFilePath);
	FixPathSlashes(fixedBatchFilePath, PATH_SEP_CHAR);
	
	if (!DoesFileExist(environmentFilePath) || !skipRunningIfFileExists)
	{
		int statusCode = RunCliProgram(fixedBatchFilePath, &cmd);
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
void InitializeMsvcIf(Str pigBuildFolder, bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		Str batchPath = JoinPaths(pigBuildFolder, StrLit("shell/init_msvc.bat"));
		Str environmentPath = StrLit_Const(MSVC_ENVIRONMENT_TXT_PATH);
		if (DoesFileExist(environmentPath)) { WriteLine("Loading MSVC Environment..."); }
		// else { WriteLine("Initializing MSVC Compiler..."); } //NOTE: This is already printed inside init_msvc.bat
		RunBatchFileAndApplyDumpedEnvironment(batchPath, environmentPath, true);
		*isMsvcInitialized = true;
	}
}

#endif //  _PIG_BUILD_MISC_H
