/*
File:   pig_build_arg_list.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Holds the CliArgList structure which is sort of just a list of strings
	** There are a few extra features that help us when putting together arguments
	** for the compiler or other CLI tools we want to call:
	**  1. We can add arguments that have a format string with "[VAL]" somewhere in it, that will get replaced with some argument (see AddArgNt\AddArgStr\AddArgInt). If [VAL] is in quotes, then the argument will be escaped
	**  2. Paths will have their slashes replaced with forward or backslash, based on "pathSepChar" (usually we set this if running a Windows program, otherwise it defaults to '/')
	**  3. Lists can be easily joined together to make longer argument lists TODO: We might want to add support for deduplicating arguments when joining?
	**  4. Since we are running a build script, we don't have to worry about freeing memory, so we just allocate all the strings we need and never worry about freeing them
	**
	** NOTE: See RunCliProgram which lives in pig_build_build_helpers.h which
	**       is used to actually call a program with arguments in a CliArgList
*/

#ifndef _PIG_BUILD_ARG_LIST_H
#define _PIG_BUILD_ARG_LIST_H

#include "pig_build_base.h"
#include "pig_build_str8.h"

// +--------------------------------------------------------------+
// |                   Composing Argument Lists                   |
// +--------------------------------------------------------------+
// We have this string inside a bunch of #defines in places like pig_build_cli_flags.h
// This allows us to replace that part of the argument string with an actual value, adding escaping if the argument is in quotes
#define CLI_VAL_STR      "[VAL]"
#define CLI_UNQUOTED_ARG "[VAL]"
#define CLI_QUOTED_ARG   "\"[VAL]\""
#if BUILDING_ON_WINDOWS
#define CLI_PIPE_OUTPUT_TO_FILE "> \"[VAL]\""
#else
#define CLI_PIPE_OUTPUT_TO_FILE "| \"[VAL]\""
#endif
#define CLI_ROOT_DIR "[ROOT]"

//When running a program on Linux/OSX/etc. we have to specify we want to run a program out of the current working directory with "./"
#if BUILDING_ON_WINDOWS
#define EXEC_PROGRAM_IN_FOLDER_PREFIX ""
#else
#define EXEC_PROGRAM_IN_FOLDER_PREFIX "./"
#endif

typedef struct CliArg CliArg;
struct CliArg
{
	Str8 format;
	Str8 value;
};

#define CLI_MAX_ARGS 256
typedef struct CliArgList CliArgList;
struct CliArgList
{
	Str8 rootDirPath;
	char pathSepChar;
	u64 numArgs;
	CliArg args[CLI_MAX_ARGS];
};

Str8 FormatArg(const CliArg* arg, Str8 rootDirPath, char pathSepChar)
{
	Str8 valTargetStr = StrLit_Const(CLI_VAL_STR);
	Str8 formatStr = arg->format;
	Str8 valueStr = StrReplace(arg->value, StrLit(CLI_ROOT_DIR), rootDirPath, false);
	FixPathSlashes(valueStr, pathSepChar);
	
	u64 insertValIndex = formatStr.length;
	for (u64 cIndex = 0; cIndex + valTargetStr.length <= formatStr.length; cIndex++)
	{
		if (StrExactEquals(StrSlice(formatStr, cIndex, cIndex+valTargetStr.length), valTargetStr))
		{
			insertValIndex = cIndex;
			if (cIndex > 0 && cIndex + valTargetStr.length < formatStr.length &&
				formatStr.chars[cIndex-1] == '\"' && formatStr.chars[cIndex + valTargetStr.length] == '\"')
			{
				Str8 escapedString = EscapeString(valueStr, false);
				free(valueStr.chars);
				valueStr = escapedString;
			}
			break;
		}
	}
	if (valueStr.length > 0 && insertValIndex >= formatStr.length)
	{
		PrintLine_E("Tried to fill value in CLI argument that doesn't take a value! %.*s", StrPrint(formatStr));
		exit(4);
	}
	if (valueStr.length == 0 && insertValIndex < formatStr.length)
	{
		PrintLine_E("Missing value in CLI argument that takes a value! %.*s - %.*s - %.*s", StrPrint(formatStr), StrPrint(valueStr), StrPrint(arg->value));
		// PrintLine_E("There are %u arguments in this list:", list->numArgs);
		// for (u64 aIndex = 0; aIndex < list->numArgs; aIndex++) { PrintLine_E("\t[%u] \"%.*s\"", aIndex, StrPrint(list->args[aIndex])); }
		exit(4);
	}
	
	Str8 result = CopyStr8(formatStr, false);
	if (insertValIndex < formatStr.length)
	{
		Str8 cliLeftPart = StrSlice(formatStr, 0, insertValIndex);
		Str8 cliRightPart = StrSliceFrom(formatStr, insertValIndex + valTargetStr.length);
		Str8 joinedStr = JoinStrings3(cliLeftPart, valueStr, cliRightPart, true);
		free(result.chars);
		result = joinedStr;
	}
	free(valueStr.chars);
	
	return result;
}

void AddArgStr(CliArgList* list, const char* formatStrNt, Str8 valueStr)
{
	if (list->numArgs >= CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	list->args[list->numArgs].format = CopyStr8(MakeStr8Nt(formatStrNt), false);
	list->args[list->numArgs].value = CopyStr8(valueStr, false);
	list->numArgs++;
}
void AddArgNt(CliArgList* list, const char* formatStrNt, const char* valueStr)
{
	AddArgStr(list, formatStrNt, MakeStr8Nt(valueStr));
}
void AddArgInt(CliArgList* list, const char* formatStrNt, i32 valueInt)
{
	char printBuffer[12];
	int printResult = snprintf(&printBuffer[0], 12, "%d", valueInt);
	printBuffer[printResult] = '\0';
	AddArgStr(list, formatStrNt, MakeStr8((u64)printResult, &printBuffer[0]));
}
void AddArg(CliArgList* list, const char* formatStrNt)
{
	AddArgStr(list, formatStrNt, Str8_Empty);
}

void AddArgList(CliArgList* dest, const CliArgList* source)
{
	if (dest->numArgs + source->numArgs > CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	for (u64 aIndex = 0; aIndex < source->numArgs; aIndex++)
	{
		dest->args[dest->numArgs].format = CopyStr8(source->args[aIndex].format, false);
		dest->args[dest->numArgs].value = CopyStr8(source->args[aIndex].value, false);
		dest->numArgs++;
	}
}

Str8 JoinCliArgsList(Str8 prefix, const CliArgList* list, bool addNullTerm)
{
	char pathSepChar = list->pathSepChar;
	if (pathSepChar == '\0') { pathSepChar = PATH_SEP_CHAR; }
	Str8 rootDirPath = ZEROED;
	if (list->rootDirPath.length == 0) { rootDirPath = CopyStr8(StrLit(".."), false); }
	else { rootDirPath = CopyStr8(list->rootDirPath, false); }
	FixPathSlashes(rootDirPath, pathSepChar);
	
	Str8* formattedStrings = (list->numArgs > 0) ? (Str8*)malloc(sizeof(Str8) * list->numArgs) : nullptr;
	u64 totalLength = prefix.length;
	for (u64 aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		formattedStrings[aIndex] = FormatArg(&list->args[aIndex], rootDirPath, pathSepChar);
		if (formattedStrings[aIndex].length > 0)
		{
			if (totalLength > 0) { totalLength++; } //+1 for space between arguments
			totalLength += formattedStrings[aIndex].length;
		}
	}
	free(rootDirPath.chars);
	
	Str8 result;
	result.length = totalLength;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	
	u64 writeIndex = 0;
	memcpy(&result.chars[writeIndex], &prefix.chars[0], prefix.length); writeIndex += prefix.length;
	
	for (u64 aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		if (formattedStrings[aIndex].length > 0)
		{
			if (writeIndex > 0)
			{
				result.chars[writeIndex] = ' ';
				writeIndex++;
			}
			
			memcpy(&result.chars[writeIndex], formattedStrings[aIndex].chars, formattedStrings[aIndex].length);
			writeIndex += formattedStrings[aIndex].length;
		}
	}
	
	if (addNullTerm) { result.chars[writeIndex] = '\0'; }
	return result;
}

#endif //  _PIG_BUILD_ARG_LIST_H
