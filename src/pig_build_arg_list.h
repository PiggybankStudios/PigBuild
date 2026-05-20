/*
File:   pig_build_arg_list.h
Author: Taylor Robbins
Date:   06\16\2025
Description:
	** Holds CliArgs which is like StrArray but with a bunch of extra features
	** to help us manage arguments that need to get passed to tools that have
	** a command-line interface (CLI).
	**  1. Each entry is a "format" and optional "value" which don't get combined
	**     until FilterAndJoinCliArgsList is called. The "format" string has the
	**     text "[VAL]" inside it to indicate where the value string should get inserted
	**  2. When values are inserted into formats, if "[VAL]" is inside quotes, we
	**     escape special characters in the value string (see EscapeString in pig_build_str.h)
	**  3. Forward and backslashes are normalized to match "pathSepChar" option in
	**     CliArgs. This defaults to '\' on Windows and '/' on everything else.
	**     This can be overridden by calling code before joining the list.
	**  4. The text "[ROOT]" is replaced with a specific path given by "rootDirPath"
	**     in CliArgs. This defaults to "..", assuming that root means one folder
	**     up from where we are building artifacts, since most PigBuild projects
	**     create a folder named "build" in the root of your project and pushd
	**     into the folder. This can be overridden by calling code before joining.
	**  5. Arguments can have tags attached to include or exclude them based on the
	**     tagList that is passed to FilterAndJoinCliArgsList. If both includeTags
	**     and excludeTags are empty for an argument then it always applies. Similarly
	**     if the tagList is empty then all args apply.
	**  6. TODO: We probably want to add a system that de-duplicates arguments so
	**     we don't accidentally pass the same argument twice to a CLI tool
	**
	** NOTE: See RunCliProgram which lives in pig_build_misc.h which
	**       is used to actually call a program with arguments in a CliArgs
	** NOTE: There is no need to free CliArgs, since the build script itself only
	**     runs for a short time, memory can just get freed when the build ends
*/

#ifndef _PIG_BUILD_ARG_LIST_H
#define _PIG_BUILD_ARG_LIST_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_tags.h"

// +--------------------------------------------------------------+
// |                   Composing Argument Lists                   |
// +--------------------------------------------------------------+
// We have this string inside a bunch of #defines in places like pig_build_cli_flags.h
// This allows us to replace that part of the argument string with an actual value, adding escaping if the argument is in quotes
#define CLI_VAL_STR             "[VAL]"
#define CLI_UNQUOTED_ARG        "[VAL]"
#define CLI_SINGLE_QUOTED_ARG   "\'[VAL]\'"
#define CLI_QUOTED_ARG          "\"[VAL]\""
#if BUILDING_ON_WINDOWS
#define CLI_PIPE_OUTPUT_TO_FILE "> \"[VAL]\""
#else
#define CLI_PIPE_OUTPUT_TO_FILE "| \"[VAL]\""
#endif
#define CLI_ROOT_DIR            "[ROOT]"

//When running a program on Linux/OSX/etc. we have to specify we want to run a program out of the current working directory with "./"
#if BUILDING_ON_WINDOWS
#define EXEC_PROGRAM_IN_FOLDER_PREFIX ""
#else
#define EXEC_PROGRAM_IN_FOLDER_PREFIX "./"
#endif

typedef struct CliArg CliArg;
struct CliArg
{
	Str format;
	Str value;
	StrArray includeTags;
	StrArray excludeTags;
};

TYPED_ARRAY(Array_CliArg, CliArg, args);

typedef struct CliArgs CliArgs;
struct CliArgs
{
	Str rootDirPath;
	char pathSepChar;
	Array_CliArg array;
};

Str ResolveRootTo(Str path, Str rootPath)
{
	return StrReplace(path, StrLit(CLI_ROOT_DIR), rootPath);
}

CliArg* AddTaggedArgStr(CliArgs* args, const char* includeExcludeTagsStr, const char* formatStrNt, Str valueStr)
{
	CliArg* newArg = AddItemArray_CliArg(&args->array);
	newArg->format = CopyStrNt(formatStrNt);
	newArg->value = CopyStr(valueStr);
	SplitIncludeExcludeTagsListStr(MakeStrNt(includeExcludeTagsStr), &newArg->includeTags, &newArg->excludeTags);
	// if (newArg->includeTags.length > 0 || newArg->excludeTags.length > 0)
	// {
	// 	PrintLine("\"%.*s\" has %llu include tag%s and %llu exclude tag%s:",
	// 		StrPrint(newArg->format),
	// 		newArg->includeTags.length, Plural(newArg->includeTags.length, "s"),
	// 		newArg->excludeTags.length, Plural(newArg->excludeTags.length, "s")
	// 	);
	// 	for (u64 iIndex = 0; iIndex < newArg->includeTags.length; iIndex++) { PrintLine("\tinclude[%llu]: \"%.*s\"", iIndex, StrPrint(newArg->includeTags.strings[iIndex])); }
	// 	for (u64 eIndex = 0; eIndex < newArg->excludeTags.length; eIndex++) { PrintLine("\texclude[%llu]: \"%.*s\"", eIndex, StrPrint(newArg->excludeTags.strings[eIndex])); }
	// }
	return newArg;
}
CliArg* AddTaggedArgNt(CliArgs* args, const char* includeExcludeTagsStr, const char* formatStrNt, const char* valueStr) { return AddTaggedArgStr(args, includeExcludeTagsStr, formatStrNt, MakeStrNt(valueStr)); }
CliArg* AddTaggedArg(CliArgs* args, const char* includeExcludeTagsStr, const char* formatStrNt) { return AddTaggedArgStr(args, includeExcludeTagsStr, formatStrNt, Str_Empty); }
CliArg* AddArgStr(CliArgs* args, const char* formatStrNt, Str valueStr)        { return AddTaggedArgStr(args, "", formatStrNt, valueStr); }
CliArg* AddArgNt(CliArgs* args, const char* formatStrNt, const char* valueStr) { return  AddTaggedArgNt(args, "", formatStrNt, valueStr); }
CliArg* AddArg(CliArgs* args, const char* formatStrNt)                         { return    AddTaggedArg(args, "", formatStrNt);           }

void AddArgList(CliArgs* dest, const CliArgs* source)
{
	AppendArray_CliArg(&dest->array, &source->array);
	for (u64 aIndex = 0; aIndex < source->array.length; aIndex++)
	{
		CliArg* destArg = &dest->array.items[dest->array.length - source->array.length + aIndex];
		destArg->format = CopyStr(destArg->format);
		destArg->value = CopyStr(destArg->value);
		for (u64 iIndex = 0; iIndex < destArg->includeTags.length; iIndex++)
		{
			destArg->includeTags.strings[iIndex] = CopyStr(destArg->includeTags.strings[iIndex]);
		}
		for (u64 eIndex = 0; eIndex < destArg->excludeTags.length; eIndex++)
		{
			destArg->excludeTags.strings[eIndex] = CopyStr(destArg->excludeTags.strings[eIndex]);
		}
	}
}

Str FormatCliArg(const CliArg* arg, Str rootDirPath, char pathSepChar)
{
	Str valTargetStr = StrLit_Const(CLI_VAL_STR);
	Str formatStr = arg->format;
	Str valueStr = StrReplace(arg->value, StrLit(CLI_ROOT_DIR), rootDirPath);
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
				Str escapedString = EscapeString(valueStr);
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
	
	Str result = CopyStr(formatStr);
	if (insertValIndex < formatStr.length)
	{
		result = StrReplaceRange(formatStr, insertValIndex, insertValIndex + valTargetStr.length, valueStr);
	}
	FreeStr(&valueStr);
	
	return result;
}

Str FilterAndJoinCliArgsList(Str prefix, const CliArgs* args, StrArray* tagsListPntr)
{
	StrArray localEmptyTagList = EMPTY;
	if (tagsListPntr == nullptr) { tagsListPntr = &localEmptyTagList; }
	
	char pathSepChar = args->pathSepChar;
	if (pathSepChar == '\0') { pathSepChar = PATH_SEP_CHAR; }
	Str rootDirPath = Str_Empty_Const;
	if (IsEmptyStr(args->rootDirPath)) { rootDirPath = CopyStrLit(".."); }
	else { rootDirPath = CopyStr(args->rootDirPath); }
	FixPathSlashes(rootDirPath, pathSepChar);
	
	StrArray formattedStrings = EMPTY;
	GrowArray_Str(&formattedStrings, args->array.length);
	u64 totalLength = prefix.length;
	for (u64 aIndex = 0; aIndex < args->array.length; aIndex++)
	{
		if (DoTagsMatch(tagsListPntr, &args->array.args[aIndex].includeTags, &args->array.args[aIndex].excludeTags))
		{
			Str formattedStr = FormatCliArg(&args->array.args[aIndex], rootDirPath, pathSepChar);
			if (formattedStr.length > 0)
			{
				if (totalLength > 0) { totalLength++; } //+1 for space between arguments
				totalLength += formattedStr.length;
				AddStr(&formattedStrings, formattedStr);
			}
		}
	}
	free(rootDirPath.chars);
	
	Str result = AllocStr(totalLength);
	u64 writeIndex = 0;
	memcpy(&result.chars[writeIndex], &prefix.chars[0], prefix.length); writeIndex += prefix.length;
	
	for (u64 aIndex = 0; aIndex < formattedStrings.length; aIndex++)
	{
		if (writeIndex > 0)
		{
			result.chars[writeIndex] = ' ';
			writeIndex++;
		}
		Str formattedStr = formattedStrings.strings[aIndex];
		memcpy(&result.chars[writeIndex], formattedStr.chars, formattedStr.length);
		writeIndex += formattedStr.length;
	}
	Assert(writeIndex == result.length);
	
	result.chars[writeIndex] = '\0';
	// PrintLine("Filtered %llu arguments to %llu", args->numArgs, numFormattedStrings);
	return result;
}

#endif //  _PIG_BUILD_ARG_LIST_H
