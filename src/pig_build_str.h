/*
File:   pig_build_str.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_STR_H
#define _PIG_BUILD_STR_H

#include "pig_build_base.h"

typedef struct Str Str;
struct Str
{
	u64 length;
	union { char* chars; u8* bytes; void* pntr; };
};

// +--------------------------------------------------------------+
// |                         Str Macros                          |
// +--------------------------------------------------------------+
#define MakeStr_Const(lengthValue, pntrValue) { .length=(lengthValue), .pntr=(void*)(pntrValue) }
#define MakeStr(length, pntr) (Str)MakeStr_Const((length), (pntr))
#define Str_Empty_Const MakeStr_Const(0, nullptr)
#define Str_Empty       MakeStr(0, nullptr)

#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define StrLit_Const(stringLiteral) MakeStr_Const(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define StrLit(stringLiteral)       MakeStr(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define MakeStrNt(nullTermPntr)    MakeStr((u64)strlen(nullTermPntr), (nullTermPntr))

//NOTE: This is meant to be used when formatting Str using any printf like functions
//      Use the format specifier %.*s and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars

#define IsEmptyStr(string) ((string).length == 0)
#define IsEmptyStrPntr(stringPntr) ((stringPntr) == nullptr || (stringPntr)->length == 0)

// +--------------------------------------------------------------+
// |                        Str Functions                        |
// +--------------------------------------------------------------+
void FreeStr(Str* strPntr)
{
	assert(strPntr->length == 0 || strPntr->chars != nullptr);
	if (strPntr->chars == nullptr) { memset(strPntr, 0x00, sizeof(Str)); return; }
	free(strPntr->chars);
	memset(strPntr, 0x00, sizeof(Str));
}
Str CopyStr(Str strToCopy, bool addNullTerm)
{
	Str result = Str_Empty_Const;
	if (strToCopy.length == 0 && !addNullTerm) { return result; }
	result.length = strToCopy.length;
	result.chars = (char*)malloc(strToCopy.length + (addNullTerm ? 1 : 0));
	assert(result.chars != nullptr);
	if (strToCopy.length > 0) { memcpy(result.chars, strToCopy.chars, strToCopy.length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
Str CopyStrNt(const char* strToCopyNt, bool addNullTerm)
{
	return CopyStr(MakeStrNt(strToCopyNt), addNullTerm);
}
#define CopyStrLit(stringLiteral, addNullTerm) CopyStr(StrLit(stringLiteral), (addNullTerm))
Str AllocStr(u64 length, bool addNullTerm)
{
	Str result = Str_Empty_Const;
	if (length == 0 && !addNullTerm) { return result; }
	result.length = length;
	result.chars = (char*)malloc(length + (addNullTerm ? 1 : 0));
	assert(result.chars != nullptr);
	if (length > 0) { memset(result.chars, 0x00, length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

bool StrExactEquals(Str left, Str right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	assert(left.chars != nullptr);
	assert(right.chars != nullptr);
	return (memcmp(left.chars, right.chars, left.length) == 0);
}
Str StrSlice(Str target, u64 startIndex, u64 endIndex)
{
	assert(startIndex <= target.length);
	assert(endIndex <= target.length);
	assert(startIndex <= endIndex);
	return MakeStr(endIndex - startIndex, target.chars + startIndex);
}
Str StrSliceFrom(Str target, u64 startIndex)
{
	return StrSlice(target, startIndex, target.length);
}
bool StrExactContains(Str haystack, Str needle)
{
	assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (u64 bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, bIndex, bIndex+needle.length), needle)) { return true; }
	}
	return false;
}
bool StrExactStartsWith(Str target, Str prefix)
{
	assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
bool StrExactEndsWith(Str target, Str suffix)
{
	assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
Str GetDirectoryPart(Str fullPath, bool includeTrailingSlash)
{
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSlice(fullPath, 0, lastSlashIndex + (includeTrailingSlash ? 1 : 0)); }
	else { return fullPath; }
}
Str GetFileNamePart(Str fullPath, bool includeExtension)
{
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSliceFrom(fullPath, lastSlashIndex+1); }
	else { return fullPath; }
}
Str GetFileExtPart(Str fullPath)
{
	u64 periodIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { periodIndex = fullPath.length; } //reset periodIndex
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
Str TrimWhitespace(Str target)
{
	assert(target.length == 0 || target.chars != nullptr);
	Str result = target;
	while (result.length > 0 && IsCharWhitespace(result.chars[0])) { result.chars++; result.length--; }
	while (result.length > 0 && IsCharWhitespace(result.chars[result.length-1])) { result.length--; }
	return result;
}
u64 FindNextWhitespace(Str targetStr, u64 startIndex)
{
	assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (IsCharWhitespace(targetStr.chars[bIndex])) { return bIndex; }
	}
	return targetStr.length;
}
u64 FindNextNonIdentifierChar(Str targetStr, u64 startIndex)
{
	assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (!IsCharIdentifier(targetStr.chars[bIndex], (bIndex == startIndex))) { return bIndex; }
	}
	return targetStr.length;
}

bool TryParseBoolArg(Str boolStr, bool* valueOut)
{
	if (StrExactEquals(boolStr, StrLit("1"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("0"))) { *valueOut = false; return true; }
	if (StrExactEquals(boolStr, StrLit("true"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("false"))) { *valueOut = false; return true; }
	return false;
}

Str EscapeString(Str unescapedString, bool addNullTerm)
{
	Str result = Str_Empty_Const;
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
		
		if (pass == 0) { result = AllocStr(byteIndex, addNullTerm); }
		else if (addNullTerm) { result.chars[result.length] = '\0'; }
	}
	return result;
}

Str JoinStrings2(Str left, Str right, bool addNullTerm)
{
	Str result = AllocStr(left.length + right.length, addNullTerm);
	if (left.length > 0) { memcpy(&result.chars[0], &left.chars[0], left.length); }
	if (right.length > 0) { memcpy(&result.chars[left.length], &right.chars[0], right.length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}
Str JoinStrings3(Str left, Str middle, Str right, bool addNullTerm)
{
	Str result = AllocStr(left.length + middle.length + right.length, addNullTerm);
	if (left.length > 0) { memcpy(&result.chars[0], &left.chars[0], left.length); }
	if (middle.length > 0) { memcpy(&result.chars[left.length], &middle.chars[0], middle.length); }
	if (right.length > 0) { memcpy(&result.chars[left.length + middle.length], &right.chars[0], right.length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

//Returns the number of target characters that were replaced
u64 StrReplaceChars(Str haystack, char targetChar, char replaceChar)
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

void FixPathSlashes(Str path, char slashChar)
{
	StrReplaceChars(path, (slashChar == '/') ? '\\' : '/', slashChar);
}

bool HasTrailingSlash(Str path)
{
	return (path.length > 0 && IsSlash(path.chars[path.length-1]));
}

Str WithoutTrailingSlash(Str path)
{
	if (HasTrailingSlash(path)) { return StrSlice(path, 0, path.length-1); }
	else { return path; }
}
Str WithTrailingSlash(Str path)
{
	if (HasTrailingSlash(path)) { return path; }
	else { return JoinStrings2(path, StrLit("/"), true); }
}

Str JoinPaths(Str leftPath, Str rightPath, bool addNullTerm)
{
	if (leftPath.length == 0 || rightPath.length == 0 || HasTrailingSlash(leftPath) || IsSlash(rightPath.chars[0])) { return JoinStrings2(leftPath, rightPath, addNullTerm); }
	else { return JoinStrings3(leftPath, StrLit("/"), rightPath, addNullTerm); }
}

Str StrReplace(Str haystack, Str target, Str replacement, bool addNullTerm)
{
	Str result = Str_Empty_Const;
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
	result = AllocStr(result.length, addNullTerm);
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
Str StrReplaceRange(Str targetStr, u64 startIndex, u64 endIndex, Str replacementStr, bool addNullTerm)
{
	assert(startIndex <= targetStr.length);
	assert(endIndex <= targetStr.length);
	Str leftPart = StrSlice(targetStr, 0, Min2(startIndex, endIndex));
	Str rightPart = StrSliceFrom(targetStr, Max2(startIndex, endIndex));
	return JoinStrings3(leftPart, replacementStr, rightPart, addNullTerm);
}
Str StrInsert(Str targetStr, u64 insertIndex, Str insertStr, bool addNullTerm)
{
	return StrReplaceRange(targetStr, insertIndex, insertIndex, insertStr, addNullTerm);
}

#endif //  _PIG_BUILD_STR_H
