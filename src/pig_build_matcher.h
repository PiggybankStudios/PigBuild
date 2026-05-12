/*
File:   pig_build_matcher.h
Author: Taylor Robbins
Date:   05\11\2026
Description:
	** Many build scripts need to match pieces of strings with something more complicated than
	** a single substring match. Many people use Regular Expressions to do this job but I find
	** regex to be quite hard to read, and most of the symbols map to regular code like a while
	** loop that could just be written out plainly rather than encoding it in symbols. This
	** file is an attempt at making a system that replaces the need for regular expressions with
	** C code that accomplishes the same task but with plainly visible logic.
*/

#ifndef _PIG_BUILD_MATCHER_H
#define _PIG_BUILD_MATCHER_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_array.h"

typedef struct Matcher Matcher;
struct Matcher
{
	bool matchFailed;
	Str errorStr;
	Str str;
	u64 cursor;
};

typedef struct MatcherCharRange MatcherCharRange;
struct MatcherCharRange
{
	char min;
	char max; //inclusive
};

TYPED_ARRAY(Array_MatcherCharRange, MatcherCharRange, ranges);

typedef struct MatcherCharSet MatcherCharSet;
struct MatcherCharSet
{
	bool negative;
	Array_MatcherCharRange ranges;
};

Matcher NewMatcher(Str str)
{
	Matcher result = EMPTY;
	result.matchFailed = false;
	result.str = str;
	result.cursor = 0;
	return result;
}

#define MatcherSet_Any          MatcherSet_Not(MatcherStr_StrLit(""))
#define MatcherSet_Whitespace   MatcherSet_StrLit(" \t")
#define MatcherSet_Uppercase    MatcherSet_Range('A', 'Z')
#define MatcherSet_Lowercase    MatcherSet_Range('a', 'z')
#define MatcherSet_Numeric      MatcherSet_Range('0', '9')
#define MatcherSet_Alphabetic   MatcherSet_Both(MatcherSet_Uppercase, MatcherSet_Lowercase)
#define MatcherSet_AlphaNumeric MatcherSet_All3(MatcherSet_Uppercase, MatcherSet_Lowercase, MatcherSet_Numeric)

// +--------------------------------------------------------------+
// |                   MatcherCharSet Creation                    |
// +--------------------------------------------------------------+
MatcherCharSet MatcherSet_Not(MatcherCharSet set)
{
	MatcherCharSet result = set;
	result.negative = !result.negative;
	return result;
}
MatcherCharSet MatcherSet_Char(char c)
{
	MatcherCharSet result = EMPTY;
	MatcherCharRange* range = AddItemArray_MatcherCharRange(&result.ranges);
	range->min = c;
	range->max = c;
	return result;
}
MatcherCharSet MatcherSet_Range(char minChar, char maxChar)
{
	MatcherCharSet result = EMPTY;
	MatcherCharRange* range = AddItemArray_MatcherCharRange(&result.ranges);
	range->min = Min2(minChar, maxChar);
	range->max = Max2(minChar, maxChar);
	return result;
}
MatcherCharSet MatcherSet_Str(Str charsStr)
{
	MatcherCharSet result = EMPTY;
	Str strCopy = CopyStr(charsStr);
	for (u64 cIndex = 0; cIndex < strCopy.length; cIndex++)
	{
		if (strCopy.chars[cIndex] != '\0')
		{
			MatcherCharRange* range = AddItemArray_MatcherCharRange(&result.ranges);
			range->min = strCopy.chars[cIndex];
			range->max = strCopy.chars[cIndex];
			for (u64 cIndex2 = cIndex+1; cIndex2 < strCopy.length; cIndex2++)
			{
				if (strCopy.chars[cIndex2] == strCopy.chars[cIndex]) { strCopy.chars[cIndex2] = '\0'; }
			}
		}
	}
	FreeStr(&strCopy);
	return result;
}
#define MatcherSet_StrLit(stringLit) MatcherSet_Str(StrLit(stringLit))
#define MatcherSet_StrNt(nullTermStr) MatcherSet_Str(MakeStrNt(nullTermStr))

MatcherCharSet MatcherSet_Both(MatcherCharSet leftSet, MatcherCharSet rightSet)
{
	MatcherCharSet result = EMPTY;
	GrowArray_MatcherCharRange(&result.ranges, leftSet.ranges.length + rightSet.ranges.length);
	for (u64 rIndex = 0; rIndex < leftSet.ranges.length; rIndex++)
	{
		AddValueArray_MatcherCharRange(&result.ranges, leftSet.ranges.items[rIndex]);
	}
	for (u64 rIndex = 0; rIndex < rightSet.ranges.length; rIndex++)
	{
		AddValueArray_MatcherCharRange(&result.ranges, rightSet.ranges.items[rIndex]);
	}
	return result;
}
#define MatcherSet_All2(leftSet, rightSet) MatcherSet_Both((leftSet), (rightSet))
MatcherCharSet MatcherSet_All3(MatcherCharSet set1, MatcherCharSet set2, MatcherCharSet set3)
{
	return MatcherSet_Both(MatcherSet_Both(set1, set2), set3);
}
MatcherCharSet MatcherSet_All4(MatcherCharSet set1, MatcherCharSet set2, MatcherCharSet set3, MatcherCharSet set4)
{
	return MatcherSet_Both(MatcherSet_Both(set1, set2), MatcherSet_Both(set3, set4));
}

// +--------------------------------------------------------------+
// |                      Matching Functions                      |
// +--------------------------------------------------------------+
bool MatchSingle(Matcher* matcher, MatcherCharSet set)
{
	if (matcher->matchFailed) { return false; }
	if (matcher->cursor >= matcher->str.length)
	{
		matcher->matchFailed = true;
		matcher->errorStr = StrLit("Reached end");
		return false;
	}
	
	char nextChar = matcher->str.chars[matcher->cursor];
	bool isMatch = false;
	for (u64 rIndex = 0; rIndex < set.ranges.length; rIndex++)
	{
		MatcherCharRange* range = &set.ranges.items[rIndex];
		if (range->min <= nextChar && range->max >= nextChar) { isMatch = true; break; }
	}
	
	if (isMatch == !set.negative) { matcher->cursor++; }
	else
	{
		matcher->matchFailed = true;
		matcher->errorStr = set.negative ? StrLit("Matching char") : StrLit("Non-matching char");
	}
	return (isMatch == !set.negative);
}
u64 MatchN(Matcher* matcher, MatcherCharSet set, u64 numCharsToMatch)
{
	if (matcher->matchFailed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= numCharsToMatch && MatchSingle(matcher, set)) { numCharsMatched++; }
	if (matcher->matchFailed) { matcher->matchFailed = false; matcher->errorStr = Str_Empty; }
	if (numCharsMatched != numCharsToMatch)
	{
		matcher->matchFailed = true;
		matcher->errorStr = (numCharsMatched < numCharsToMatch) ? StrLit("Too few matching chars") : StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
u64 MatchAtLeast(Matcher* matcher, MatcherCharSet set, u64 minNumChars)
{
	if (matcher->matchFailed) { return 0; }
	u64 numCharsMatched = 0;
	while (MatchSingle(matcher, set)) { numCharsMatched++; }
	if (matcher->matchFailed) { matcher->matchFailed = false; matcher->errorStr = Str_Empty; }
	if (numCharsMatched < minNumChars)
	{
		matcher->matchFailed = true;
		matcher->errorStr = StrLit("Too few matching chars");
	}
	return numCharsMatched;
}
u64 MatchAtMost(Matcher* matcher, MatcherCharSet set, u64 maxNumChars)
{
	if (matcher->matchFailed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= maxNumChars && MatchSingle(matcher, set)) { numCharsMatched++; }
	if (matcher->matchFailed) { matcher->matchFailed = false; matcher->errorStr = Str_Empty; }
	if (numCharsMatched > maxNumChars)
	{
		matcher->matchFailed = true;
		matcher->errorStr = StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
u64 MatchAtLeastAndMost(Matcher* matcher, MatcherCharSet set, u64 minNumChars, u64 maxNumChars)
{
	if (matcher->matchFailed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= maxNumChars && MatchSingle(matcher, set)) { numCharsMatched++; }
	if (matcher->matchFailed) { matcher->matchFailed = false; matcher->errorStr = Str_Empty; }
	if (numCharsMatched < minNumChars || numCharsMatched > maxNumChars)
	{
		matcher->matchFailed = true;
		matcher->errorStr = (numCharsMatched < minNumChars) ? StrLit("Too few matching chars") : StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
#define MatchAll(matcher, set) MatchAtLeast((matcher), (set), 0)
#define MatchOptional(matcher, set) MatchAtLeastAndMost((matcher), (set), 0, 1)

bool MatchStr(Matcher* matcher, Str str, bool caseSensitive)
{
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		char nextChar = str.chars[cIndex];
		MatcherCharSet charSet = MatcherSet_Char(nextChar);
		if (!caseSensitive)
		{
			if (nextChar >= 'A' && nextChar <= 'Z') { charSet = MatcherSet_Both(charSet, MatcherSet_Char((nextChar - 'A') + 'a')); }
			if (nextChar >= 'a' && nextChar <= 'z') { charSet = MatcherSet_Both(charSet, MatcherSet_Char((nextChar - 'a') + 'A')); }
		}
		if (!MatchSingle(matcher, charSet)) { return false; }
	}
	return true;
}
bool MatchExactStr(Matcher* matcher, Str str) { return MatchStr(matcher, str, true); }
bool MatchAnyCaseStr(Matcher* matcher, Str str) { return MatchStr(matcher, str, false); }

u64 MatchWhitespace(Matcher* matcher)   { return MatchAll(matcher, MatcherSet_Whitespace);   }
u64 MatchAlphabetic(Matcher* matcher)   { return MatchAll(matcher, MatcherSet_Alphabetic);   }
u64 MatchNumeric(Matcher* matcher)      { return MatchAll(matcher, MatcherSet_Numeric);      }
u64 MatchAlphaNumeric(Matcher* matcher) { return MatchAll(matcher, MatcherSet_AlphaNumeric); }

#endif //  _PIG_BUILD_MATCHER_H
