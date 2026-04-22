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
	** NOTE: See RunCliProgram which lives in pig_build_misc.h which
	**       is used to actually call a program with arguments in a CliArgList
*/

#ifndef _PIG_BUILD_ARG_LIST_H
#define _PIG_BUILD_ARG_LIST_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_tags.h"
#include "pig_build_str_array.h"

// +--------------------------------------------------------------+
// |                   Composing Argument Lists                   |
// +--------------------------------------------------------------+
// We have this string inside a bunch of #defines in places like pig_build_cli_flags.h
// This allows us to replace that part of the argument string with an actual value, adding escaping if the argument is in quotes
#define CLI_VAL_STR      "[VAL]"
#define CLI_UNQUOTED_ARG "[VAL]"
#define CLI_SINGLE_QUOTED_ARG   "\'[VAL]\'"
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

//TODO: We should probably change these from fixed array to VarArray-like structure
//      We are blowing the stack on main entry if we set these too high and we try and put a CliArgList on the stack
#define CLI_MAX_ARGS 400

typedef struct CliArg CliArg;
struct CliArg
{
	Str format;
	Str value;
	StrArray includeTags;
	StrArray excludeTags;
};

typedef struct CliArgList CliArgList;
struct CliArgList
{
	Str rootDirPath;
	char pathSepChar;
	u64 numArgs;
	CliArg* args;
};

Str FormatArg(const CliArg* arg, Str rootDirPath, char pathSepChar)
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

void SplitTagsListStr(Str tagsListStr, StrArray* tagArrayPntr)
{
	u64 lastCommaIndex = 0;
	for (u64 cIndex = 0; cIndex <= tagsListStr.length; cIndex++)
	{
		if (cIndex == tagsListStr.length || tagsListStr.chars[cIndex] == ',' || tagsListStr.chars[cIndex] == '&' || tagsListStr.chars[cIndex] == '|')
		{
			Str tagStr = StrSlice(tagsListStr, lastCommaIndex, cIndex);
			TrimWhitespace(tagStr);
			if (tagStr.length > 0) { AddStr(tagArrayPntr, tagStr); }
			lastCommaIndex = cIndex+1;
		}
	}
}
void SplitIncludeExcludeTagsListStr(Str tagsListStr, StrArray* includeArrayPntr, StrArray* excludeArrayPntr)
{
	u64 lastCommaIndex = 0;
	for (u64 cIndex = 0; cIndex <= tagsListStr.length; cIndex++)
	{
		if (cIndex == tagsListStr.length || tagsListStr.chars[cIndex] == ',' || tagsListStr.chars[cIndex] == '&' || tagsListStr.chars[cIndex] == '|')
		{
			Str tagStr = StrSlice(tagsListStr, lastCommaIndex, cIndex);
			TrimWhitespace(tagStr);
			if (tagStr.length > 0)
			{
				Str equalsTrueStr = StrLit("==true");
				Str equalsFalseStr = StrLit("==false");
				if (tagStr.chars[cIndex] == '!') { AddStr(excludeArrayPntr, StrSliceFrom(tagStr, 1)); }
				else if (StrExactEndsWith(tagStr, equalsFalseStr)) { AddStr(excludeArrayPntr, StrSlice(tagStr, 0, tagStr.length - equalsFalseStr.length)); }
				else if (StrExactEndsWith(tagStr, equalsTrueStr)) { AddStr(includeArrayPntr, StrSlice(tagStr, 0, tagStr.length - equalsTrueStr.length)); }
				else { AddStr(includeArrayPntr, tagStr); }
			}
			lastCommaIndex = cIndex+1;
		}
	}
}

CliArg* AddTaggedArgStr(CliArgList* list, const char* includeExcludeTagsStr, const char* formatStrNt, Str valueStr)
{
	if (list->args == nullptr)
	{
		list->args = (CliArg*)malloc(sizeof(CliArg) * CLI_MAX_ARGS);
		NotNull(list->args);
	}
	if (list->numArgs >= CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	CliArg* newArg = &list->args[list->numArgs];
	memset(newArg, 0x00, sizeof(CliArg));
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
	list->numArgs++;
	return newArg;
}
CliArg* AddTaggedArgNt(CliArgList* list, const char* includeExcludeTagsStr, const char* formatStrNt, const char* valueStr)
{
	return AddTaggedArgStr(list, includeExcludeTagsStr, formatStrNt, MakeStrNt(valueStr));
}
CliArg* AddTaggedArgInt(CliArgList* list, const char* includeExcludeTagsStr, const char* formatStrNt, i32 valueInt)
{
	char printBuffer[12];
	int printResult = snprintf(&printBuffer[0], 12, "%d", valueInt);
	printBuffer[printResult] = '\0';
	return AddTaggedArgStr(list, includeExcludeTagsStr, formatStrNt, MakeStr((u64)printResult, &printBuffer[0]));
}
CliArg* AddTaggedArg(CliArgList* list, const char* includeExcludeTagsStr, const char* formatStrNt)
{
	return AddTaggedArgStr(list, includeExcludeTagsStr, formatStrNt, Str_Empty);
}

CliArg* AddArgStr(CliArgList* list, const char* formatStrNt, Str valueStr)       { return AddTaggedArgStr(list, "", formatStrNt, valueStr); }
CliArg* AddArgNt(CliArgList* list, const char* formatStrNt, const char* valueStr) { return  AddTaggedArgNt(list, "", formatStrNt, valueStr); }
CliArg* AddArgInt(CliArgList* list, const char* formatStrNt, i32 valueInt)        { return AddTaggedArgInt(list, "", formatStrNt, valueInt); }
CliArg* AddArg(CliArgList* list, const char* formatStrNt)                         { return    AddTaggedArg(list, "", formatStrNt);           }

void AddArgList(CliArgList* dest, const CliArgList* source)
{
	if (dest->args == nullptr)
	{
		dest->args = (CliArg*)malloc(sizeof(CliArg) * CLI_MAX_ARGS);
		NotNull(dest->args);
	}
	if (dest->numArgs + source->numArgs > CLI_MAX_ARGS) { WriteLine_E("Too many CLI arguments!"); exit(4); }
	for (u64 aIndex = 0; aIndex < source->numArgs; aIndex++)
	{
		const CliArg* sourceArg = &source->args[aIndex];
		CliArg* destArg = &dest->args[dest->numArgs];
		memset(destArg, 0x00, sizeof(CliArg));
		destArg->format = CopyStr(sourceArg->format);
		destArg->value = CopyStr(sourceArg->value);
		for (u64 iIndex = 0; iIndex < sourceArg->includeTags.length; iIndex++)
		{
			AddStr(&destArg->includeTags, sourceArg->includeTags.strings[iIndex]);
		}
		for (u64 eIndex = 0; eIndex < sourceArg->excludeTags.length; eIndex++)
		{
			AddStr(&destArg->excludeTags, sourceArg->excludeTags.strings[eIndex]);
		}
		dest->numArgs++;
	}
}

bool DoesArgMatchTags(const CliArg* arg, const StrArray* tagsListPntr)
{
	//If the CLI we are running doesn't use tags, then we assume all args match
	if (tagsListPntr == nullptr || tagsListPntr->length == 0) { return true; }
	
	bool anyExcludesMatched = false;
	for (u64 eIndex = 0; eIndex < arg->excludeTags.length; eIndex++)
	{
		for (u64 tIndex = 0; tIndex < tagsListPntr->length; tIndex++)
		{
			if (StrExactEquals(arg->excludeTags.strings[eIndex], tagsListPntr->strings[tIndex]))
			{
				// PrintLine("\"%.*s\" excluded because \"%.*s\"", StrPrint(arg->format), StrPrint(arg->excludeTags.strings[eIndex]));
				anyExcludesMatched = true;
				break;
			}
		}
		if (anyExcludesMatched) { break; }
	}
	if (anyExcludesMatched) { return false; }
	
	bool allIncludesMatched = true;
	for (u64 iIndex = 0; iIndex < arg->includeTags.length; iIndex++)
	{
		bool includeMatched = false;
		for (u64 tIndex = 0; tIndex < tagsListPntr->length; tIndex++)
		{
			if (StrExactEquals(arg->includeTags.strings[iIndex], tagsListPntr->strings[tIndex])) { includeMatched = true; break; }
		}
		if (!includeMatched)
		{
			// PrintLine("\"%.*s\" not included because missing \"%.*s\" in %llu tag%s", StrPrint(arg->format), StrPrint(arg->includeTags.strings[iIndex]), tagsListPntr->length, Plural(tagsListPntr->length, "s"));
			allIncludesMatched = false;
			break;
		}
	}
	if (!allIncludesMatched) { return false; }
	
	return true;
}

Str FilterAndJoinCliArgsList(Str prefix, const CliArgList* list, StrArray* tagsListPntr)
{
	StrArray localEmptyTagList = EMPTY;
	if (tagsListPntr == nullptr) { tagsListPntr = &localEmptyTagList; }
	
	char pathSepChar = list->pathSepChar;
	if (pathSepChar == '\0') { pathSepChar = PATH_SEP_CHAR; }
	Str rootDirPath = Str_Empty_Const;
	if (IsEmptyStr(list->rootDirPath)) { rootDirPath = CopyStrLit(".."); }
	else { rootDirPath = CopyStr(list->rootDirPath); }
	FixPathSlashes(rootDirPath, pathSepChar);
	
	u64 numFormattedStrings = 0;
	Str* formattedStrings = (list->numArgs > 0) ? (Str*)malloc(sizeof(Str) * list->numArgs) : nullptr;
	u64 totalLength = prefix.length;
	for (u64 aIndex = 0; aIndex < list->numArgs; aIndex++)
	{
		if (DoesArgMatchTags(&list->args[aIndex], tagsListPntr))
		{
			formattedStrings[numFormattedStrings] = FormatArg(&list->args[aIndex], rootDirPath, pathSepChar);
			if (formattedStrings[numFormattedStrings].length > 0)
			{
				if (totalLength > 0) { totalLength++; } //+1 for space between arguments
				totalLength += formattedStrings[numFormattedStrings].length;
				numFormattedStrings++;
			}
		}
	}
	free(rootDirPath.chars);
	
	Str result = AllocStr(totalLength);
	u64 writeIndex = 0;
	memcpy(&result.chars[writeIndex], &prefix.chars[0], prefix.length); writeIndex += prefix.length;
	
	for (u64 aIndex = 0; aIndex < numFormattedStrings; aIndex++)
	{
		if (writeIndex > 0)
		{
			result.chars[writeIndex] = ' ';
			writeIndex++;
		}
		memcpy(&result.chars[writeIndex], formattedStrings[aIndex].chars, formattedStrings[aIndex].length);
		writeIndex += formattedStrings[aIndex].length;
	}
	Assert(writeIndex == result.length);
	
	result.chars[writeIndex] = '\0';
	// PrintLine("Filtered %llu arguments to %llu", list->numArgs, numFormattedStrings);
	return result;
}

#endif //  _PIG_BUILD_ARG_LIST_H
