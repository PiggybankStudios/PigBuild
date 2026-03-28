/*
File:   pig_build_not_regex.h
Author: Taylor Robbins
Date:   03\27\2026
Description:
	** Many build scripts need to do some amount of structured text walking and comparison
	** The goto tool for this is Regular Expressions which is quite effective
	** Unfortunately it's also rather complex to implement and running a regular expression
	** is computationally unbounded because of certain patterns requiring lookahead and
	** branching to every possible way a regex might match a string.
	** Usually though, in build scripts we just need a somewhat reasonable way to compose
	** various kinds of logic together into a certain kind of well-defined walked over
	** the input string using a series of "rules".
	** This file is an experiment in trying to avoid implementing regular expressions
	** while accomplishing a small set of tasks we normally would use regex for.
*/

#ifndef _PIG_BUILD_NOT_REGEX_H
#define _PIG_BUILD_NOT_REGEX_H

#include "pig_build_base.h"
#include "pig_build_str8.h"

//NOTE: The macros and functions below act sort of like regular expressions.
//      For example, CONSUME_WHITESPACE is similar to \s* in RE syntax.

#define CONSUME_WHITESPACE(linePntr) do                                \
{                                                                      \
	while ((linePntr)->length > 0 &&                                   \
		((linePntr)->chars[0] == ' ' || (linePntr)->chars[0] == '\t')) \
	{                                                                  \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_STR(linePntr, expectedStr) do                              \
{                                                                          \
	if (!StrExactStartsWith(*(linePntr), (expectedStr))) { return false; } \
	*(linePntr) = StrSliceFrom(*(linePntr), (expectedStr).length);         \
} while(0)

#define CONSUME_NT_STR(linePntr, expectedStrNt) do  \
{                                                   \
	Str8 expectedStr = StrLit_Const(expectedStrNt); \
	CONSUME_STR((linePntr), expectedStr);           \
} while(0)

#define CONSUME_UNTIL(linePntr, expectedStr) do                        \
{                                                                      \
	while((linePntr)->length > 0)                                      \
	{                                                                  \
		if (StrExactStartsWith(*(linePntr), (expectedStr))) { break; } \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_UNTIL_CHARS(linePntr, expectedCharsStr) do                                                   \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (u64 cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (isInExpectedStr) { break; }                                                                      \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define CONSUME_UNTIL_NOT_CHARS(linePntr, expectedCharsStr) do                                               \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (u64 cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (!isInExpectedStr) { break; }                                                                     \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define UPPERCASE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWERCASE_CHARS "abcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define IDENTIFIER_CHARS "_" NUMBER_CHARS UPPERCASE_CHARS LOWERCASE_CHARS

#endif //  _PIG_BUILD_NOT_REGEX_H
