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
	union { void* pntr; char* chars; u8* bytes; };
};

// +--------------------------------------------------------------+
// |                         Str Macros                           |
// +--------------------------------------------------------------+
// _Const variants should be used when declaring and initializing a Str variable on the same line. For example:
//   Str name = Str_Empty_Const;
// The non _Const versions should be used when assigning a Str after declaration or constructing a Str anywhere that isn't getting directly assigned to a newly declared Str variable
//   Str name;
//   name = Str_Empty;
// Depending on which version of C/C++ you are compiling with and on which compiler this distinction may be more or less enforced
#if LANGUAGE_IS_C
#define MakeStr_Const(lengthValue, pntrValue) { .length=(lengthValue), .pntr=(void*)(pntrValue) }
#else
#define MakeStr_Const(lengthValue, pntrValue) { (lengthValue), (void*)(pntrValue) }
#endif
#define MakeStr(length, pntr) INIT(Str)MakeStr_Const((length), (pntr))
#define Str_Empty_Const MakeStr_Const(0, nullptr)
#define Str_Empty       MakeStr(0, nullptr)

#define StrLitLength(stringLiteral) ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define StrLit_Const(stringLiteral) MakeStr_Const(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define StrLit(stringLiteral)       MakeStr(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define MakeStrNt(nullTermPntr)     MakeStr((u64)strlen(nullTermPntr), (nullTermPntr))

//NOTE: This is meant to be used when formatting Str using any printf like functions
//      Use the format specifier %.*s and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars

#define IsEmptyStr(string) ((string).length == 0)
#define IsEmptyStrPntr(stringPntr) ((stringPntr) == nullptr || (stringPntr)->length == 0)

// +--------------------------------------------------------------+
// |                     Basic Str Functions                      |
// +--------------------------------------------------------------+
void FreeStr(Str* strPntr)
{
	Assert(strPntr->length == 0 || strPntr->chars != nullptr);
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
	NotNull(result.chars);
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
	NotNull(result.chars);
	if (length > 0) { memset(result.chars, 0x00, length); }
	if (addNullTerm) { result.chars[result.length] = '\0'; }
	return result;
}

bool StrExactEquals(Str left, Str right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	NotNull(left.chars);
	NotNull(right.chars);
	return (memcmp(left.chars, right.chars, left.length) == 0);
}
Str StrSlice(Str target, u64 startIndex, u64 endIndex)
{
	Assert(startIndex <= target.length);
	Assert(endIndex <= target.length);
	Assert(startIndex <= endIndex);
	return MakeStr(endIndex - startIndex, target.chars + startIndex);
}
Str StrSliceFrom(Str target, u64 startIndex)
{
	return StrSlice(target, startIndex, target.length);
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

bool StrExactContains(Str haystack, Str needle)
{
	Assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (u64 bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrExactEquals(StrSlice(haystack, bIndex, bIndex+needle.length), needle)) { return true; }
	}
	return false;
}
bool StrExactStartsWith(Str target, Str prefix)
{
	Assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
bool StrExactEndsWith(Str target, Str suffix)
{
	Assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
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
	Assert(target.length == 0 || target.chars != nullptr);
	Str result = target;
	while (result.length > 0 && IsCharWhitespace(result.chars[0])) { result.chars++; result.length--; }
	while (result.length > 0 && IsCharWhitespace(result.chars[result.length-1])) { result.length--; }
	return result;
}
u64 FindNextWhitespace(Str targetStr, u64 startIndex)
{
	Assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (IsCharWhitespace(targetStr.chars[bIndex])) { return bIndex; }
	}
	return targetStr.length;
}
u64 FindNextNonIdentifierChar(Str targetStr, u64 startIndex)
{
	Assert(startIndex <= targetStr.length);
	for (u64 bIndex = startIndex; bIndex < targetStr.length; bIndex++)
	{
		if (!IsCharIdentifier(targetStr.chars[bIndex], (bIndex == startIndex))) { return bIndex; }
	}
	return targetStr.length;
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
	Assert(startIndex <= targetStr.length);
	Assert(endIndex <= targetStr.length);
	Str leftPart = StrSlice(targetStr, 0, Min2(startIndex, endIndex));
	Str rightPart = StrSliceFrom(targetStr, Max2(startIndex, endIndex));
	return JoinStrings3(leftPart, replacementStr, rightPart, addNullTerm);
}
Str StrInsert(Str targetStr, u64 insertIndex, Str insertStr, bool addNullTerm)
{
	return StrReplaceRange(targetStr, insertIndex, insertIndex, insertStr, addNullTerm);
}

// +--------------------------------------------------------------+
// |                      File Path Helpers                       |
// +--------------------------------------------------------------+
Str GetDirectoryPart(Str fullPath, bool includeTrailingSlash)
{
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { lastSlashIndex = cIndex; }
	}
	if (lastSlashIndex < fullPath.length) { return StrSlice(fullPath, 0, lastSlashIndex + (includeTrailingSlash ? 1 : 0)); }
	else { return StrSlice(fullPath, 0, 0); }
}
//NOTE: If your filename contains multiple periods, this only chops off the last period onwards, aka the "real" extension. For example "library.tar.gz" becomes "libary.tar"
Str GetFileNamePart(Str fullPath, bool includeExtension)
{
	u64 periodIndex = fullPath.length;
	u64 lastSlashIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { lastSlashIndex = cIndex; periodIndex = fullPath.length; }
		else if (character == '.') { periodIndex = cIndex; }
	}
	return StrSlice(fullPath,
		(lastSlashIndex < fullPath.length) ? lastSlashIndex+1 : 0,
		includeExtension ? fullPath.length : periodIndex
	);
}
Str GetFileExtPart(Str fullPath, bool includeSubExtensions)
{
	u64 periodIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { periodIndex = fullPath.length; } //reset periodIndex
		else if (character == '.' && (!includeSubExtensions || periodIndex >= fullPath.length)) { periodIndex = cIndex; }
	}
	if (periodIndex < fullPath.length) { return StrSliceFrom(fullPath, periodIndex); }
	else { return StrSliceFrom(fullPath, fullPath.length); }
}
Str RemovePathExtension(Str fullPath, bool includeSubExtensions)
{
	u64 periodIndex = fullPath.length;
	for (u64 cIndex = 0; cIndex < fullPath.length; cIndex++)
	{
		char character = fullPath.chars[cIndex];
		if (IsSlash(character)) { periodIndex = fullPath.length; }
		else if (character == '.' && (!includeSubExtensions || periodIndex >= fullPath.length)) { periodIndex = cIndex; }
	}
	return StrSlice(fullPath, 0, periodIndex);
}
Str AddSuffixToFileName(Str filePath, Str suffix, bool addNullTerm)
{
	Str fileExtension = GetFileExtPart(filePath, true);
	Str fileDirAndName = RemovePathExtension(filePath, true);
	return JoinStrings3(fileDirAndName, suffix, fileExtension, addNullTerm);
}

u64 CountPathParts(Str fileOrFolderPath)
{
	u64 numParts = 0;
	u64 prevSlashIndex = 0;
	for (u64 cIndex = 0; cIndex <= fileOrFolderPath.length; cIndex++)
	{
		if (cIndex == fileOrFolderPath.length || IsSlash(fileOrFolderPath.chars[cIndex]))
		{
			if (prevSlashIndex == cIndex && (cIndex == 0 || cIndex == fileOrFolderPath.length))
			{
				//ignore leading or trailing slashes
			}
			else if (cIndex > prevSlashIndex)
			{
				numParts++;
			}
			else { AssertFmt(cIndex > prevSlashIndex, "Empty section in path is not valid! \"%.*s\"", StrPrint(fileOrFolderPath)); }
			prevSlashIndex = cIndex+1;
		}
	}
	return numParts;
}
Str GetPathPartAtIndex(Str fileOrFolderPath, u64 index)
{
	u64 partIndex = 0;
	u64 prevSlashIndex = 0;
	for (u64 cIndex = 0; cIndex <= fileOrFolderPath.length; cIndex++)
	{
		if (cIndex == fileOrFolderPath.length || IsSlash(fileOrFolderPath.chars[cIndex]))
		{
			if (prevSlashIndex == cIndex && (cIndex == 0 || cIndex == fileOrFolderPath.length))
			{
				//ignore leading or trailing slashes
			}
			else if (cIndex > prevSlashIndex)
			{
				if (partIndex == index) { return StrSlice(fileOrFolderPath, prevSlashIndex, cIndex); }
				partIndex++;
			}
			else { AssertFmt(cIndex > prevSlashIndex, "Empty section in path is not valid! \"%.*s\"", StrPrint(fileOrFolderPath)); }
			prevSlashIndex = cIndex+1;
		}
	}
	return StrSliceFrom(fileOrFolderPath, fileOrFolderPath.length);
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

// +--------------------------------------------------------------+
// |                      Parsing Functions                       |
// +--------------------------------------------------------------+
bool TryParseBoolArg(Str boolStr, bool* valueOut)
{
	if (StrExactEquals(boolStr, StrLit("1"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("0"))) { *valueOut = false; return true; }
	if (StrExactEquals(boolStr, StrLit("true"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("false"))) { *valueOut = false; return true; }
	return false;
}

bool TryParseHexU64(Str str, u64* valueOut)
{
	u8 charIndex = 0;
	u64 result = 0;
	str = TrimWhitespace(str);
	if (str.length >= 2 && str.chars[0] == '0' && str.chars[1] == 'x') { str.chars += 2; str.length-=2; }
	if (str.length == 0) { return false; }
	while (str.length > 0 && charIndex < 16)
	{
		if (str.chars[0] >= '0' && str.chars[0] <= '9')
		{
			result = result * 16ULL + (u64)(str.chars[0] - '0');
		}
		else if (str.chars[0] >= 'A' && str.chars[0] <= 'F')
		{
			result = result * 16ULL + (10 + (u64)(str.chars[0] - 'A'));
		}
		else if (str.chars[0] >= 'a' && str.chars[0] <= 'f')
		{
			result = result * 16ULL + (10 + (u64)(str.chars[0] - 'a'));
		}
		else
		{
			PrintLine("Invalid char in hex at index[%u]: \'%c\'", charIndex, str.chars[0]);
			return false;
		}
		str.chars++; str.length--;
		charIndex++;
	}
	if (str.length > 0)
	{
		PrintLine("String is too long for u64 hex! Remaining: [%llu]\"%.*s\"", str.length, StrPrint(str));
		return false;
	}
	if (valueOut != nullptr) { *valueOut = result; }
	return true;
}

char GetHexChar(u8 hexValue, bool upperCase)
{
	if (hexValue <= 9) { return '0' + hexValue; }
	else if (hexValue < 16) { return (upperCase ? 'A' : 'a') + (hexValue - 10); }
	else { return '?'; }
}
Str ConvertU64ToHexStr(u64 value, bool upperCase)
{
	Str result = AllocStr(2 + (sizeof(u64)*2), true);
	result.chars[0] = '0';
	result.chars[1] = 'x';
	for (u8 bIndex = 0; bIndex < (sizeof(u64)*2); bIndex++)
	{
		result.chars[(result.length-1) - bIndex] = GetHexChar((u8)(value & 0x0FULL), upperCase);
		value = (value >> 4ULL);
	}
	result.chars[result.length] = '\0';
	return result;
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

#endif //  _PIG_BUILD_STR_H
