/*
File:   pig_build_scan.h
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

#ifndef _PIG_BUILD_SCAN_H
#define _PIG_BUILD_SCAN_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_array.h"

typedef struct Scan Scan;
struct Scan
{
	bool failed;
	Str errorStr;
	Str str;
	u64 cursor;
};

typedef struct ScanCharRange ScanCharRange;
struct ScanCharRange
{
	char min;
	char max; //inclusive
};

TYPED_ARRAY(Array_ScanCharRange, ScanCharRange, ranges);

typedef struct ScanSet ScanSet;
struct ScanSet
{
	bool negative;
	Array_ScanCharRange ranges;
};

void FreeScanSet(ScanSet* set)
{
	FreeArray_ScanCharRange(&set->ranges);
	memset(set, 0x00, sizeof(ScanSet));
}

Scan NewScan(Str str)
{
	Scan result = EMPTY;
	result.failed = false;
	result.errorStr = Str_Empty;
	result.str = str;
	result.cursor = 0;
	return result;
}

#define ScanSet_Any          ScanSet_Not(ScanSet_StrLit(""))
#define ScanSet_Whitespace   ScanSet_StrLit(" \t")
#define ScanSet_Uppercase    ScanSet_Range('A', 'Z')
#define ScanSet_Lowercase    ScanSet_Range('a', 'z')
#define ScanSet_Numeric      ScanSet_Range('0', '9')
#define ScanSet_Alphabetic   ScanSet_Both(ScanSet_Uppercase, ScanSet_Lowercase)
#define ScanSet_AlphaNumeric ScanSet_All3(ScanSet_Uppercase, ScanSet_Lowercase, ScanSet_Numeric)

// +--------------------------------------------------------------+
// |                       ScanSet Creation                       |
// +--------------------------------------------------------------+
ScanSet ScanSet_Not(ScanSet set)
{
	ScanSet result = set;
	result.negative = !result.negative;
	return result;
}
ScanSet ScanSet_Char(char c)
{
	ScanSet result = EMPTY;
	ScanCharRange* range = AddItemArray_ScanCharRange(&result.ranges);
	range->min = c;
	range->max = c;
	return result;
}
ScanSet ScanSet_Range(char minChar, char maxChar)
{
	ScanSet result = EMPTY;
	ScanCharRange* range = AddItemArray_ScanCharRange(&result.ranges);
	range->min = Min2(minChar, maxChar);
	range->max = Max2(minChar, maxChar);
	return result;
}
ScanSet ScanSet_Str(Str charsStr)
{
	ScanSet result = EMPTY;
	Str strCopy = CopyStr(charsStr);
	for (u64 cIndex = 0; cIndex < strCopy.length; cIndex++)
	{
		if (strCopy.chars[cIndex] != '\0')
		{
			ScanCharRange* range = AddItemArray_ScanCharRange(&result.ranges);
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
#define ScanSet_StrLit(stringLit) ScanSet_Str(StrLit(stringLit))
#define ScanSet_StrNt(nullTermStr) ScanSet_Str(MakeStrNt(nullTermStr))

ScanSet ScanSet_Both(ScanSet leftSet, ScanSet rightSet)
{
	ScanSet result = EMPTY;
	GrowArray_ScanCharRange(&result.ranges, leftSet.ranges.length + rightSet.ranges.length);
	for (u64 rIndex = 0; rIndex < leftSet.ranges.length; rIndex++)
	{
		AddValueArray_ScanCharRange(&result.ranges, leftSet.ranges.items[rIndex]);
	}
	for (u64 rIndex = 0; rIndex < rightSet.ranges.length; rIndex++)
	{
		AddValueArray_ScanCharRange(&result.ranges, rightSet.ranges.items[rIndex]);
	}
	return result;
}
#define ScanSet_All2(leftSet, rightSet) ScanSet_Both((leftSet), (rightSet))
ScanSet ScanSet_All3(ScanSet set1, ScanSet set2, ScanSet set3)
{
	return ScanSet_Both(ScanSet_Both(set1, set2), set3);
}
ScanSet ScanSet_All4(ScanSet set1, ScanSet set2, ScanSet set3, ScanSet set4)
{
	return ScanSet_Both(ScanSet_Both(set1, set2), ScanSet_Both(set3, set4));
}

// +--------------------------------------------------------------+
// |                      Matching Functions                      |
// +--------------------------------------------------------------+
bool Scan1(Scan* scan, ScanSet set)
{
	if (scan->failed) { return false; }
	if (scan->cursor >= scan->str.length)
	{
		scan->failed = true;
		scan->errorStr = StrLit("Reached end");
		return false;
	}
	
	char nextChar = scan->str.chars[scan->cursor];
	bool charIsInSet = false;
	for (u64 rIndex = 0; rIndex < set.ranges.length; rIndex++)
	{
		ScanCharRange* range = &set.ranges.items[rIndex];
		if (range->min <= nextChar && range->max >= nextChar) { charIsInSet = true; break; }
	}
	
	if (charIsInSet == !set.negative) { scan->cursor++; }
	else
	{
		scan->failed = true;
		scan->errorStr = set.negative ? StrLit("Matching char") : StrLit("Non-matching char");
	}
	return (charIsInSet == !set.negative);
}
u64 ScanN(Scan* scan, ScanSet set, u64 numCharsToMatch)
{
	if (scan->failed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= numCharsToMatch && Scan1(scan, set)) { numCharsMatched++; }
	if (scan->failed) { scan->failed = false; scan->errorStr = Str_Empty; }
	if (numCharsMatched != numCharsToMatch)
	{
		scan->failed = true;
		scan->errorStr = (numCharsMatched < numCharsToMatch) ? StrLit("Too few matching chars") : StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
u64 ScanMin(Scan* scan, ScanSet set, u64 minNumChars)
{
	if (scan->failed) { return 0; }
	u64 numCharsMatched = 0;
	while (Scan1(scan, set)) { numCharsMatched++; }
	if (scan->failed) { scan->failed = false; scan->errorStr = Str_Empty; }
	if (numCharsMatched < minNumChars)
	{
		scan->failed = true;
		scan->errorStr = StrLit("Too few matching chars");
	}
	return numCharsMatched;
}
u64 ScanMax(Scan* scan, ScanSet set, u64 maxNumChars)
{
	if (scan->failed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= maxNumChars && Scan1(scan, set)) { numCharsMatched++; }
	if (scan->failed) { scan->failed = false; scan->errorStr = Str_Empty; }
	if (numCharsMatched > maxNumChars)
	{
		scan->failed = true;
		scan->errorStr = StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
u64 MatchMinMax(Scan* scan, ScanSet set, u64 minNumChars, u64 maxNumChars)
{
	if (scan->failed) { return 0; }
	u64 numCharsMatched = 0;
	while (numCharsMatched <= maxNumChars && Scan1(scan, set)) { numCharsMatched++; }
	if (scan->failed) { scan->failed = false; scan->errorStr = Str_Empty; }
	if (numCharsMatched < minNumChars || numCharsMatched > maxNumChars)
	{
		scan->failed = true;
		scan->errorStr = (numCharsMatched < minNumChars) ? StrLit("Too few matching chars") : StrLit("Too many matching chars");
	}
	return numCharsMatched;
}
#define ScanZeroOrMore(scan, set) ScanMin((scan), (set), 0)
#define ScanOneOrMore(scan, set)  ScanMin((scan), (set), 1)
#define ScanZeroOrOne(scan, set)  ScanMinMax((scan), (set), 0, 1)

bool ScanStr(Scan* scan, Str str, bool caseSensitive)
{
	for (u64 cIndex = 0; cIndex < str.length; cIndex++)
	{
		char nextChar = str.chars[cIndex];
		ScanSet charSet = ScanSet_Char(nextChar);
		if (!caseSensitive)
		{
			if (nextChar >= 'A' && nextChar <= 'Z') { charSet = ScanSet_Both(charSet, ScanSet_Char((nextChar - 'A') + 'a')); }
			if (nextChar >= 'a' && nextChar <= 'z') { charSet = ScanSet_Both(charSet, ScanSet_Char((nextChar - 'a') + 'A')); }
		}
		bool charMatched = Scan1(scan, charSet);
		FreeScanSet(&charSet);
		if (!charMatched) { return false; }
	}
	return true;
}
bool ScanExactStr(Scan* scan, Str str) { return ScanStr(scan, str, true); }
bool ScanAnyCaseStr(Scan* scan, Str str) { return ScanStr(scan, str, false); }

u64 ScanWhitespace(Scan* scan)   { return ScanZeroOrMore(scan, ScanSet_Whitespace);   }
u64 ScanAlphabetic(Scan* scan)   { return ScanZeroOrMore(scan, ScanSet_Alphabetic);   }
u64 ScanNumeric(Scan* scan)      { return ScanZeroOrMore(scan, ScanSet_Numeric);      }
u64 ScanAlphaNumeric(Scan* scan) { return ScanZeroOrMore(scan, ScanSet_AlphaNumeric); }

#endif //  _PIG_BUILD_SCAN_H
