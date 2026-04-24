/*
File:   pig_build_tags.h
Author: Taylor Robbins
Date:   04\05\2026
Description:
	** In order to help us wrangle a bunch of CLI args we made a system
	** where every arg that goes into a CliArgs can have some number of
	** includeTags and excludeTags that define which CLI tool invocations it
	** should be sent to based on whether it's include\excludeTags match
	** the tags given for that CLI tool invocation.
	**
	** Tags are just strings but to keep things consistent and readable we
	** usually use these macros when manipulating tags to make sure we don't
	** make any spelling mistakes in the string.
	**
	** We also support tag list strings where tags are joined together with "|"
	** as a delimating character in-between. Since we often are using string
	** literal implicit concatenation many of the macros here are prefixed with
	** "|" to make it easy to join them together and form a valid tag list string.
	**
	** For example we can do: `T_MSVC_CL T_WINDOWS T_LANG_C T_DEBUG_BUILD` to produce "|cl|Windows|LangC|DEBUG_BUILD==true" string literal
	**
	** NOTE: When adding tags to a StrArray you can use AddTag(&array, T_TAG_NAME) instead of AddStr(...) to strip the leading "|" character
*/

#ifndef _PIG_BUILD_TAGS_H
#define _PIG_BUILD_TAGS_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"

#define TAG_SEP_CHAR '|'

// CLI Executables
#define T_MSVC_CL         "|cl"
#define T_MSVC_LINK       "|link"
#define T_MSVC_CL_OR_LINK "|ClOrLink"
#define T_CLANG           "|clang"
#define T_GCC             "|gcc"

// Target Platforms
#define T_WINDOWS         "|Windows"
#define T_LINUX           "|Linux"
#define T_OSX             "|OSX"
#define T_UNIX            "|Unix" // basically (LINUX or OSX)
#define T_WEB             "|Web"
#define T_WASM            "|Wasm"

// Languages
#define T_LANG_C          "|LangC"
#define T_LANG_CPP        "|LangCpp"
#define T_LANG_OBJECTIVEC "|LangObjectiveC"

// Common build_config.h defines
#define T_DEBUG_BUILD     "|DEBUG_BUILD==true"
#define T_RELEASE_BUILD   "|DEBUG_BUILD==false"

// Types of binaries
#define T_OBJECT          "|Object" //.o or .obj
#define T_LIBRARY         "|Library" //.dll, .so, or .dylib
#define T_PROGRAM         "|Program" //.exe or extensionless
#define T_SHADER          "|Shader" //Usually this is compiling a shdc output to .o/.obj

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

bool DoTagsMatch(const StrArray* tagsListPntr, const StrArray* includeTags, const StrArray* excludeTags)
{
	//If the CLI we are running doesn't use tags, then we assume all args match
	if (tagsListPntr == nullptr || tagsListPntr->length == 0) { return true; }
	
	bool anyExcludesMatched = false;
	for (u64 eIndex = 0; eIndex < excludeTags->length; eIndex++)
	{
		for (u64 tIndex = 0; tIndex < tagsListPntr->length; tIndex++)
		{
			if (StrExactEquals(excludeTags->strings[eIndex], tagsListPntr->strings[tIndex]))
			{
				anyExcludesMatched = true;
				break;
			}
		}
		if (anyExcludesMatched) { break; }
	}
	if (anyExcludesMatched) { return false; }
	
	bool allIncludesMatched = true;
	for (u64 iIndex = 0; iIndex < includeTags->length; iIndex++)
	{
		bool includeMatched = false;
		for (u64 tIndex = 0; tIndex < tagsListPntr->length; tIndex++)
		{
			if (StrExactEquals(includeTags->strings[iIndex], tagsListPntr->strings[tIndex])) { includeMatched = true; break; }
		}
		if (!includeMatched)
		{
			allIncludesMatched = false;
			break;
		}
	}
	if (!allIncludesMatched) { return false; }
	
	return true;
}

#endif //  _PIG_BUILD_TAGS_H
