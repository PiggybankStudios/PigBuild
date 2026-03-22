/*
File:   pig_build_str8.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_STR_8_H
#define _PIG_BUILD_STR_8_H

#include "pig_build_base.h"

typedef struct Str8 Str8;
struct Str8
{
	u64 length;
	union { char* chars; u8* bytes; void* pntr; };
};

// +--------------------------------------------------------------+
// |                         Str8 Macros                          |
// +--------------------------------------------------------------+
#define MakeStr8_Const(lengthValue, pntrValue) { .length=(lengthValue), .pntr=(void*)(pntrValue) }
#define MakeStr8(length, pntr) NEW_STRUCT(Str8)MakeStr8_Const((length), (pntr))
#define Str8_Empty_Const MakeStr8_Const(0, nullptr)
#define Str8_Empty       MakeStr8(0, nullptr)

#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define StrLit_Const(stringLiteral) MakeStr8_Const(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define StrLit(stringLiteral)       MakeStr8(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define MakeStr8Nt(nullTermPntr)    MakeStr8((u64)strlen(nullTermPntr), (nullTermPntr))

//NOTE: This is meant to be used when formatting Str8 using any printf like functions
//      Use the format specifier %.*s and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars

// +--------------------------------------------------------------+
// |                        Str8 Functions                        |
// +--------------------------------------------------------------+
bool StrExactEquals(Str8 left, Str8 right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	assert(left.chars != nullptr);
	assert(right.chars != nullptr);
	return (memcmp(left.chars, right.chars, left.length) == 0);
}
Str8 StrSlice(Str8 target, u64 startIndex, u64 endIndex)
{
	assert(startIndex <= target.length);
	assert(endIndex <= target.length);
	assert(startIndex <= endIndex);
	return MakeStr8(endIndex - startIndex, target.chars + startIndex);
}
Str8 StrSliceFrom(Str8 target, u64 startIndex)
{
	return StrSlice(target, startIndex, target.length);
}
bool StrExactContains(Str8 haystack, Str8 needle)
{
	assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (u64 bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, bIndex, bIndex+needle.length), needle)) { return true; }
	}
	return false;
}
bool StrExactStartsWith(Str8 target, Str8 prefix)
{
	assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
bool StrExactEndsWith(Str8 target, Str8 suffix)
{
	assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
Str8 GetDirectoryPart(Str8 fullPath, bool includeTrailingSlash)
{
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSlice(fullPath, 0, lastSlashIndex + (includeTrailingSlash ? 1 : 0)); }
	else { return fullPath; }
}
Str8 GetFileNamePart(Str8 fullPath, bool includeExtension)
{
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSliceFrom(fullPath, lastSlashIndex+1); }
	else { return fullPath; }
}
Str8 GetFileExtPart(Str8 fullPath)
{
	u64 periodIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IS_SLASH(character)) { periodIndex = fullPath.length; } //reset periodIndex
		else if (character == '.') { periodIndex = cIndex; }
	}
	if (periodIndex < fullPath.length) { return StrSliceFrom(fullPath, periodIndex); }
	else { return StrSliceFrom(fullPath, fullPath.length); }
}
bool IsCharWhitespace(char character)
{
	if (character == ' ') { return true; }
	else if (character == '\t') { return true; }
	else { return false; }
}
bool IsCharIdentifier(char character, bool isFirstChar)
{
	if (character == '_') { return true; }
	if (character >= 'A' && character <= 'Z') { return true; }
	if (character >= 'a' && character <= 'z') { return true; }
	if (!isFirstChar && character >= '0' && character <= '9') { return true; }
	return false;
}
Str8 TrimWhitespace(Str8 target)
{
	assert(target.length == 0 || target.chars != nullptr);
	Str8 result = target;
	while (result.length > 0 && IsCharWhitespace(result.chars[0])) { result.chars++; result.length--; }
	while (result.length > 0 && IsCharWhitespace(result.chars[result.length-1])) { result.length--; }
	return result;
}
u64 FindNextWhitespace(Str8 targetStr, u64 startIndex)
{
	assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (IsCharWhitespace(targetStr.chars[bIndex])) { return bIndex; }
	}
	return targetStr.length;
}
u64 FindNextNonIdentifierChar(Str8 targetStr, u64 startIndex)
{
	assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (!IsCharIdentifier(targetStr.chars[bIndex], (bIndex == startIndex))) { return bIndex; }
	}
	return targetStr.length;
}

bool TryParseBoolArg(Str8 boolStr, bool* valueOut)
{
	if (StrExactEquals(boolStr, StrLit("1"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("0"))) { *valueOut = false; return true; }
	if (StrExactEquals(boolStr, StrLit("true"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("false"))) { *valueOut = false; return true; }
	return false;
}

Str8 CopyStr8(Str8 strToCopy, bool addNullTerm)
{
	Str8 result = ZEROED;
	if (strToCopy.length == 0) { return result; }
	result.length = strToCopy.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(result.chars, strToCopy.chars, strToCopy.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

Str8 EscapeString(Str8 unescapedString, bool addNullTerm)
{
	Str8 result = ZEROED;
	for (int pass = 0; pass < 2; pass++)
	{
		u64 byteIndex = 0;
		for (u64 cIndex = 0; cIndex < unescapedString.length; cIndex++)
		{
			char character = unescapedString.chars[cIndex];
			if (character == '\"' || character == '\\' || character == '\'')
			{
				if (result.chars != nullptr)
				{
					result.chars[byteIndex+0] = '\\';
					result.chars[byteIndex+1] = character;
				}
				byteIndex += 2;
			}
			else if (character == '\n' || character == '\r' || character == '\t')
			{
				if (result.chars != nullptr)
				{
					result.chars[byteIndex+0] = '\\';
					if (character == '\n') { result.chars[byteIndex+1] = 'n'; }
					if (character == '\r') { result.chars[byteIndex+1] = 'r'; }
					if (character == '\t') { result.chars[byteIndex+1] = 't'; }
				}
				byteIndex += 2;
			}
			else
			{
				if (result.chars != nullptr) { result.chars[byteIndex] = character; }
				byteIndex++;
			}
		}
		
		if (pass == 0)
		{
			result.length = byteIndex;
			result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
		}
		else if (addNullTerm) { result.chars[result.length] = '\0'; }
	}
	return result;
}

Str8 JoinStrings2(Str8 left, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + right.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(&result.chars[0], &left.chars[0], left.length);
	memcpy(&result.chars[left.length], &right.chars[0], right.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
Str8 JoinStrings3(Str8 left, Str8 middle, Str8 right, bool addNullTerm)
{
	Str8 result;
	result.length = left.length + middle.length + right.length;
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	memcpy(&result.chars[0], &left.chars[0], left.length);
	memcpy(&result.chars[left.length], &middle.chars[0], middle.length);
	memcpy(&result.chars[left.length + middle.length], &right.chars[0], right.length);
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

//Returns the number of target characters that were replaced
u64 StrReplaceChars(Str8 haystack, char targetChar, char replaceChar)
{
	u64 numReplacements = 0;
	for (u64 cIndex = 0; cIndex < haystack.length; cIndex++)
	{
		if (haystack.chars[cIndex] == targetChar)
		{
			haystack.chars[cIndex] = replaceChar;
			numReplacements++;
		}
	}
	return numReplacements;
}

void FixPathSlashes(Str8 path, char slashChar)
{
	StrReplaceChars(path, (slashChar == '/') ? '\\' : '/', slashChar);
}

Str8 StrReplace(Str8 haystack, Str8 target, Str8 replacement, bool addNullTerm)
{
	Str8 result = ZEROED;
	for (u64 cIndex = 0; cIndex < haystack.length; cIndex++)
	{
		if (cIndex + target.length <= haystack.length &&
			StrExactEquals(StrSlice(haystack, cIndex, cIndex+target.length), target))
		{
			result.length += replacement.length;
			cIndex += target.length-1;
		}
		else { result.length += 1; }
	}
	result.pntr = malloc(result.length + (addNullTerm ? 1 : 0));
	u64 writeIndex = 0;
	for (u64 cIndex = 0; cIndex < haystack.length; cIndex++)
	{
		if (cIndex + target.length <= haystack.length &&
			StrExactEquals(StrSlice(haystack, cIndex, cIndex+target.length), target))
		{
			memcpy(&result.chars[writeIndex], replacement.chars, replacement.length);
			writeIndex += replacement.length;
			cIndex += target.length-1;
		}
		else
		{
			result.chars[writeIndex] = haystack.chars[cIndex];
			writeIndex += 1;
		}
	}
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

#endif //  _PIG_BUILD_STR_8_H
