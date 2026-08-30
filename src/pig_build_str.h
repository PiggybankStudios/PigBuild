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
	union { char* chars; void* pntr; u8* bytes; };
};

typedef struct Str16 Str16;
struct Str16
{
	union { u64 length; u64 numWords; }; //(2x byte count)
	union { u16* words; void* pntr; u8* bytes; };
};

//TODO: MacTypes.h defines Str32 as an array of 32 chars. We should choose a different name for this if we don't want to conflict
//      For now we don't really need StrFull in any places where we #include <MacTypes.h> so we'll just remove it if we see the __MACTYPES__ #define
typedef struct StrFull StrFull;
struct StrFull
{
	union { u64 length; u64 numCodepoints; }; //(4x byte count)
	union { u32* codepoints; void* pntr; u8* bytes; };
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
#define MakeStr_Const(lengthValue, pntrValue)            { .length=(lengthValue),               .pntr=(void*)(pntrValue) }
#define MakeStr16_Const(numWordsValue, pntrValue)        { .numWords=(numWordsValue),           .pntr=(void*)(pntrValue) }
#define MakeStrFull_Const(numCodepointsValue, pntrValue) { .numCodepoints=(numCodepointsValue), .pntr=(void*)(pntrValue) }
#else
#define MakeStr_Const(lengthValue, pntrValue)            { (lengthValue),        (void*)(pntrValue) }
#define MakeStr16_Const(numWordsValue, pntrValue)        { (numWordsValue),      (void*)(pntrValue) }
#define MakeStrFull_Const(numCodepointsValue, pntrValue) { (numCodepointsValue), (void*)(pntrValue) }
#endif
#define MakeStr(length, pntr)            INIT(Str)MakeStr_Const((length), (pntr))
#define MakeStr16(numWords, pntr)        INIT(Str16)MakeStr16_Const((numWords), (pntr))
#define MakeStrFull(numCodepoints, pntr) INIT(StrFull)MakeStrFull_Const((numCodepoints), (pntr))

#define Str_Empty_Const         MakeStr_Const(0, nullptr)
#define Str16_Empty_Const     MakeStr16_Const(0, nullptr)
#define StrFull_Empty_Const MakeStrFull_Const(0, nullptr)
#define Str_Empty                     MakeStr(0, nullptr)
#define Str16_Empty                 MakeStr16(0, nullptr)
#define StrFull_Empty             MakeStrFull(0, nullptr)

#define StrLitLength(stringLiteral)       ((sizeof(stringLiteral) / sizeof((stringLiteral)[0])) - sizeof((stringLiteral)[0]))
#define Str16LitLength(wideStringLiteral) ((sizeof(wideStringLiteral) / sizeof((wideStringLiteral)[0])) - sizeof((wideStringLiteral)[0]))
#define StrLit_Const(stringLiteral)       MakeStr_Const(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define StrLit(stringLiteral)             MakeStr(StrLitLength(CheckStrLit(stringLiteral)), (stringLiteral))
#define Str16Lit(wideStringLiteral)       MakeStr16(Str16LitLength(wideStringLiteral), (wideStringLiteral))
#define MakeStrNt(nullTermPntr)           MakeStr((u64)strlen(nullTermPntr), (nullTermPntr))
#define MakeStr16Nt(nullTermPntr)         MakeStr16((u64)wcslen(nullTermPntr), (nullTermPntr))

//NOTE: This is meant to be used when formatting Str using any printf like functions
//      Use the format specifier %.*s (or %.*ls) and then this macro in the var-args
#define StrPrint(string)   (int)(string).length, (string).chars
#define Str16Print(string) (int)(string).numWords, (string).words

#define IsEmptyStr(string)             ((string).length == 0)
#define IsEmptyStr16(string)           ((string).numWords == 0)
#define IsEmptyStrFull(string)         ((string).numCodepoints == 0)
#define IsEmptyStrPntr(stringPntr)     ((stringPntr) == nullptr || (stringPntr)->length == 0)
#define IsEmptyStr16Pntr(stringPntr)   ((stringPntr) == nullptr || (stringPntr)->numWords == 0)
#define IsEmptyStrFullPntr(stringPntr) ((stringPntr) == nullptr || (stringPntr)->numCodepoints == 0)

#define IsNullTerminated(string)       ((string).pntr != nullptr && (string).chars[(string).length] == '\0')
#define IsNullStr(string)              ((string).length > 0 && (string).pntr == nullptr)

#define AssertNullTerm(string)      Assert(IsNullTerminated(string))
#define NotNullStr(string)          Assert(!IsNullStr(string))
#define NotNullStrPntr(stringPntr)  Assert((stringPntr) != nullptr && !IsNullStr(*(stringPntr)))
#define NotEmptyStr(string)         Assert(!IsEmptyStr(string))
#define NotEmptyStrPntr(stringPntr) Assert(stringPntr != nullptr && !IsEmptyStr(*(stringPntr)))

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
void FreeStr16(Str16* strPntr)
{
	Assert(strPntr->numWords == 0 || strPntr->words != nullptr);
	if (strPntr->words == nullptr) { memset(strPntr, 0x00, sizeof(Str16)); return; }
	free(strPntr->words);
	memset(strPntr, 0x00, sizeof(Str16));
}
void FreeStrFull(StrFull* strPntr)
{
	Assert(strPntr->numCodepoints == 0 || strPntr->codepoints != nullptr);
	if (strPntr->codepoints == nullptr) { memset(strPntr, 0x00, sizeof(StrFull)); return; }
	free(strPntr->codepoints);
	memset(strPntr, 0x00, sizeof(StrFull));
}
Str CopyStr(Str strToCopy)
{
	Str result = Str_Empty_Const;
	result.length = strToCopy.length;
	result.chars = (char*)malloc(strToCopy.length + 1);
	NotNull(result.chars);
	if (strToCopy.length > 0) { memcpy(result.chars, strToCopy.chars, strToCopy.length); }
	result.chars[result.length] = '\0';
	return result;
}
Str16 CopyStr16(Str16 strToCopy)
{
	Str16 result = Str16_Empty_Const;
	result.numWords = strToCopy.numWords;
	result.words = (u16*)malloc(sizeof(u16) * (strToCopy.numWords + 1));
	NotNull(result.words);
	if (strToCopy.numWords > 0) { memcpy(result.words, strToCopy.words, sizeof(u16) * (strToCopy.numWords)); }
	result.words[result.numWords] = 0x0000;
	return result;
}
StrFull CopyStrFull(StrFull strToCopy)
{
	StrFull result = StrFull_Empty_Const;
	result.numCodepoints = strToCopy.numCodepoints;
	result.codepoints = (u32*)malloc(sizeof(u32) * (strToCopy.numCodepoints + 1));
	NotNull(result.codepoints);
	if (strToCopy.numCodepoints > 0) { memcpy(result.codepoints, strToCopy.codepoints, sizeof(u32) * (strToCopy.numCodepoints)); }
	result.codepoints[result.numCodepoints] = 0x00000000;
	return result;
}
Str CopyStrNt(const char* strToCopyNt)
{
	return CopyStr(MakeStrNt(strToCopyNt));
}
#define CopyStrLit(stringLiteral) CopyStr(StrLit(stringLiteral))
Str AllocStr(u64 length)
{
	Str result = Str_Empty_Const;
	result.length = length;
	result.chars = (char*)malloc(length + 1);
	NotNull(result.chars);
	if (length > 0) { memset(result.chars, 0x00, length); }
	result.chars[result.length] = '\0';
	return result;
}
Str16 AllocStr16(u64 numWords)
{
	Str16 result = Str16_Empty_Const;
	result.numWords = numWords;
	result.words = (u16*)malloc(sizeof(u16) * (numWords + 1));
	NotNull(result.words);
	if (numWords > 0) { memset(result.words, 0x00, sizeof(u16) * numWords); }
	result.words[result.numWords] = 0x0000;
	return result;
}
StrFull AllocStrFull(u64 numCodepoints)
{
	StrFull result = StrFull_Empty_Const;
	result.numCodepoints = numCodepoints;
	result.codepoints = (u32*)malloc(sizeof(u32) * (numCodepoints + 1));
	NotNull(result.codepoints);
	if (numCodepoints > 0) { memset(result.codepoints, 0x00, sizeof(u32) * numCodepoints); }
	result.codepoints[result.numCodepoints] = 0x00000000;
	return result;
}

Str StrSlice(Str target, u64 startIndex, u64 endIndex)
{
	Assert(startIndex <= target.length);
	Assert(endIndex <= target.length);
	Assert(startIndex <= endIndex);
	return MakeStr(endIndex - startIndex, target.chars + startIndex);
}
Str StrSliceFrom(Str target, u64 startIndex) { return StrSlice(target, startIndex, target.length); }

Str FormatStr(const char* formatString, ...)
{
	va_list args;
	va_start(args, formatString);
	int firstPrintResult = vsnprintf(nullptr, 0, formatString, args);
	va_end(args);
	AssertFmt(firstPrintResult >= 0, "FormatStr print failed: \"%s\"", formatString);
	Str result = AllocStr(firstPrintResult);
	if (result.length == 0) { return result; }
	va_start(args, formatString);
	int secondPrintResult = vsnprintf(result.chars, result.length+1, formatString, args);
	va_end(args);
	Assert(secondPrintResult == result.length);
	return result;
}

#define FormatVaListStr(formatString, vaListName, strVarName) Str strVarName = Str_Empty_Const; do {    \
	va_list vaListName;                                                                                 \
	va_start(vaListName, formatString);                                                                 \
	int firstPrintResult = vsnprintf(nullptr, 0, formatString, vaListName);                             \
	va_end(vaListName);                                                                                 \
	AssertFmt(firstPrintResult >= 0, "vsnprintf failed: \"%s\"", formatString);                         \
	strVarName = AllocStr(firstPrintResult);                                                            \
	if (strVarName.length == 0) { break; }                                                              \
	va_start(vaListName, formatString);                                                                 \
	int secondPrintResult = vsnprintf(strVarName.chars, strVarName.length+1, formatString, vaListName); \
	va_end(vaListName);                                                                                 \
	Assert(secondPrintResult == strVarName.length);                                                     \
} while(0)

bool StrExactEquals(Str left, Str right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	NotNull(left.chars);
	NotNull(right.chars);
	return (memcmp(left.chars, right.chars, left.length) == 0);
}
bool StrAnyCaseEquals(Str left, Str right)
{
	if (left.length != right.length) { return false; }
	if (left.length == 0) { return true; }
	NotNull(left.chars);
	NotNull(right.chars);
	for (u64 cIndex = 0; cIndex < left.length; cIndex++)
	{
		char lowercaseLeft = (left.chars[cIndex] >= 'A' && left.chars[cIndex] <= 'Z') ? (left.chars[cIndex] - 'A' + 'a') : left.chars[cIndex];
		char lowercaseRight = (right.chars[cIndex] >= 'A' && right.chars[cIndex] <= 'Z') ? (right.chars[cIndex] - 'A' + 'a') : right.chars[cIndex];
		if (lowercaseLeft != lowercaseRight) { return false; }
	}
	return true;
}
bool StrEquals(Str left, Str right, bool ignoreCase) { return ignoreCase ? StrAnyCaseEquals(left, right) : StrExactEquals(left, right); }

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
bool StrAnyCaseContains(Str haystack, Str needle)
{
	Assert(needle.length > 0);
	if (haystack.length < needle.length) { return false; }
	for (u64 bIndex = 0; bIndex <= haystack.length - needle.length; bIndex++)
	{
		if (StrAnyCaseEquals(StrSlice(haystack, bIndex, bIndex+needle.length), needle)) { return true; }
	}
	return false;
}
bool StrContains(Str haystack, Str needle, bool ignoreCase) { return ignoreCase ? StrAnyCaseContains(haystack, needle) : StrExactContains(haystack, needle); }

bool StrExactStartsWith(Str target, Str prefix)
{
	Assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrExactEquals(StrSlice(target, 0, prefix.length), prefix);
}
bool StrAnyCaseStartsWith(Str target, Str prefix)
{
	Assert(prefix.length > 0);
	if (target.length < prefix.length) { return false; }
	return StrAnyCaseEquals(StrSlice(target, 0, prefix.length), prefix);
}
bool StrStartsWith(Str target, Str prefix, bool ignoreCase) { return ignoreCase ? StrAnyCaseStartsWith(target, prefix) : StrExactStartsWith(target, prefix); }

bool StrExactEndsWith(Str target, Str suffix)
{
	Assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrExactEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
bool StrAnyCaseEndsWith(Str target, Str suffix)
{
	Assert(suffix.length > 0);
	if (target.length < suffix.length) { return false; }
	return StrAnyCaseEquals(StrSlice(target, target.length - suffix.length, target.length), suffix);
}
bool StrEndsWith(Str target, Str suffix, bool ignoreCase) { return ignoreCase ? StrAnyCaseEndsWith(target, suffix) : StrExactEndsWith(target, suffix); }

Str JoinStrings2(Str left, Str right)
{
	Str result = AllocStr(left.length + right.length);
	if (left.length > 0) { memcpy(&result.chars[0], &left.chars[0], left.length); }
	if (right.length > 0) { memcpy(&result.chars[left.length], &right.chars[0], right.length); }
	return result;
}
Str JoinStrings3(Str left, Str middle, Str right)
{
	Str result = AllocStr(left.length + middle.length + right.length);
	if (left.length > 0) { memcpy(&result.chars[0], &left.chars[0], left.length); }
	if (middle.length > 0) { memcpy(&result.chars[left.length], &middle.chars[0], middle.length); }
	if (right.length > 0) { memcpy(&result.chars[left.length + middle.length], &right.chars[0], right.length); }
	return result;
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

Str StrReplace(Str haystack, Str target, Str replacement)
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
	result = AllocStr(result.length);
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
	return result;
}
Str StrReplaceRange(Str targetStr, u64 startIndex, u64 endIndex, Str replacementStr)
{
	Assert(startIndex <= targetStr.length);
	Assert(endIndex <= targetStr.length);
	Str leftPart = StrSlice(targetStr, 0, Min2(startIndex, endIndex));
	Str rightPart = StrSliceFrom(targetStr, Max2(startIndex, endIndex));
	return JoinStrings3(leftPart, replacementStr, rightPart);
}
Str StrInsert(Str targetStr, u64 insertIndex, Str insertStr)
{
	return StrReplaceRange(targetStr, insertIndex, insertIndex, insertStr);
}

//TODO: RemoveLeadingStr, RemoveLeadingChars
//TODO: RemoveTrailingStr, RemoveTrailingChars
//TODO: SplitStrByChar, SplitStrByStr

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
	else { return JoinStrings2(path, StrLit("/")); }
}

Str JoinPaths(Str leftPath, Str rightPath)
{
	Str result = Str_Empty_Const;
	if (leftPath.length > 0 && rightPath.length > 0 && HasTrailingSlash(leftPath) && IsSlash(rightPath.chars[0])) { result = JoinStrings2(leftPath, StrSliceFrom(rightPath, 1)); }
	else if (leftPath.length == 0 || rightPath.length == 0 || HasTrailingSlash(leftPath) || IsSlash(rightPath.chars[0])) { result = JoinStrings2(leftPath, rightPath); }
	else { result = JoinStrings3(leftPath, StrLit("/"), rightPath); }
	FixPathSlashes(result, '/');
	return result;
}
#define JoinPathsLit(leftPath, rightPathStrLiteral) JoinPaths((leftPath), StrLit(rightPathStrLiteral))
#define JoinPathsNt(leftPath, rightPathNullTerm)    JoinPaths((leftPath), MakeStrNt(rightPathNullTerm))

Str JoinPaths3(Str leftPath, Str middlePath, Str rightPath)
{
	return JoinPaths(JoinPaths(leftPath, middlePath), rightPath);
}
#define JoinPaths3Lit(leftPath, middlePath, rightPathStrLiteral) JoinPaths((leftPath), (middlePath), StrLit(rightPathStrLiteral))
#define JoinPaths3Nt(leftPath, middlePath, rightPathNullTerm) JoinPaths((leftPath), (middlePath), MakeStrNt(rightPathNullTerm))

Str JoinPaths4(Str firstPath, Str secondPath, Str thirdPath, Str fourthPath)
{
	return JoinPaths(JoinPaths(firstPath, secondPath), JoinPaths(thirdPath, fourthPath));
}

Str RemovePathExtension(Str path, bool removeSubExtensions)
{
	Str extensionPart = GetFileExtPart(path, removeSubExtensions);
	return StrSlice(path, 0, path.length - extensionPart.length);
}
Str ChangePathExtension(Str path, Str newExtension, bool replaceSubExtensions)
{
	Str pathNoExt = RemovePathExtension(path, replaceSubExtensions);
	return JoinStrings2(pathNoExt, newExtension);
}
Str ChangePathFolder(Str path, Str newFolder)
{
	Str fileNameWithExt = GetFileNamePart(path, true);
	return JoinPaths(newFolder, fileNameWithExt);
}
Str ChangePathFolderAndExtension(Str path, Str newFolder, Str newExtension, bool replaceSubExtensions)
{
	Str pathChangedFolder = ChangePathFolder(path, newFolder);
	Str finalPath = ChangePathExtension(pathChangedFolder, newExtension, replaceSubExtensions);
	FreeStr(&pathChangedFolder);
	return finalPath;
}

Str AddSuffixToFileName(Str filePath, Str suffix)
{
	Str fileExtension = GetFileExtPart(filePath, true);
	Str fileDirAndName = RemovePathExtension(filePath, true);
	return JoinStrings3(fileDirAndName, suffix, fileExtension);
}

// +--------------------------------------------------------------+
// |                      Parsing Functions                       |
// +--------------------------------------------------------------+
bool TryParseBoolArg(Str boolStr, bool* valueOut)
{
	if (StrExactEquals(boolStr, StrLit("1"))) { *valueOut = true; return true; }
	if (StrExactEquals(boolStr, StrLit("0"))) { *valueOut = false; return true; }
	if (StrAnyCaseEquals(boolStr, StrLit("true"))) { *valueOut = true; return true; }
	if (StrAnyCaseEquals(boolStr, StrLit("false"))) { *valueOut = false; return true; }
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
	Str result = AllocStr(2 + (sizeof(u64)*2));
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

Str EscapeString(Str unescapedString)
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
		
		if (pass == 0) { result = AllocStr(byteIndex); }
		else { result.chars[result.length] = '\0'; }
	}
	return result;
}

#endif //  _PIG_BUILD_STR_H
