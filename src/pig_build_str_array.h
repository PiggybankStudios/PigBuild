/*
File:   pig_build_str_array.h
Author: Taylor Robbins
Date:   06\20\2025
Description:
	** StrArray (aka Array_Str) is TYPED_ARRAY (see pig_build_array.h) of Str (see pig_build_str.h)
	** with some extra logic on top to copy the strings when they are added and free
	** them when they are removed. Also function names like AddStr are slightly shorter
	** than they would be directly generated from TYPED_ARRAY
*/

#ifndef _PIG_BUILD_STR_ARRAY_H
#define _PIG_BUILD_STR_ARRAY_H

#include "pig_build_base.h"
#include "pig_build_array.h"
#include "pig_build_str.h"

// The true name of the struct is Array_Str but
// In order for our wrapper functions below to not conflict with the generated API
// function names, we make the real name of the array Array_Str and make an alias "StrArray"
TYPED_ARRAY(Array_Str, Str, strings);
typedef Array_Str StrArray;

void FreeStrArray(StrArray* array)
{
	for (u64 sIndex = 0; sIndex < array->length; sIndex++) { FreeStr(&array->strings[sIndex]); }
	FreeArray_Str(array);
}
void EmptyStrArray(StrArray* array)
{
	for (u64 sIndex = 0; sIndex < array->length; sIndex++) { FreeStr(&array->strings[sIndex]); }
	EmptyArray_Str(array);
}

Str* AddStr(StrArray* array, Str newString)
{
	Str* result = AddValueArray_Str(array, CopyStr(newString));
	return result;
}
#define AddStrLit(arrayPntr, strLit)     AddStr((arrayPntr), StrLit(strLit))
#define AddStrNt(arrayPntr, nullTermStr) AddStr((arrayPntr), MakeStrNt(nullTermStr))

Str* InsertStr(StrArray* array, Str newString, u64 insertIndex)
{
	Str* result = InsertItemArray_Str(array, insertIndex);
	*result = CopyStr(newString);
	return result;
}

Str* AddStrArray(StrArray* dest, const StrArray* src)
{
	Str* result = AppendArray_Str(dest, src);
	for (u64 sIndex = 0; sIndex < src->length; sIndex++)
	{
		result[sIndex] = CopyStr(src->strings[sIndex]);
	}
	return result;
}

void RemoveStrAtIndex(StrArray* array, u64 index)
{
	Assert(index < array->length);
	FreeStr(&array->strings[index]);
	RemoveItemArray_Str(array, index);
}

void PopStr(StrArray* array)
{
	Assert(array->length >= 1);
	FreeStr(&array->strings[array->length-1]);
	PopItemArray_Str(array);
}

u64 FindStr(const StrArray* array, Str targetStr, bool ignoreCase)
{
	for (u64 sIndex = 0; sIndex < array->length; sIndex++)
	{
		if (StrEquals(array->strings[sIndex], targetStr, ignoreCase))
		{
			return sIndex;
		}
	}
	return array->length;
}
bool ContainsStr(const StrArray* array, Str targetStr, bool ignoreCase)
{
	return (FindStr(array, targetStr, ignoreCase) < array->length);
}

bool RemoveStr(StrArray* array, Str targetStr, bool ignoreCase)
{
	u64 index = FindStr(array, targetStr, ignoreCase);
	if (index >= array->length) { return false; }
	else
	{
		RemoveStrAtIndex(array, index);
		return true;
	}
}

// NOTE: The following functions are a bit experimental and may be slightly confusing with their macro
//       wrappers but they allow you to make StrArrays or add to them in ergonamic ways.
//
//       For example:
//         StrArray sourceFiles = MakeStrArrayVa("main.c", "file.c", "ui.c");
//         AddStrsVa(&sourceFiles, "helpers.c", "debug.c");
//
// NOTE: In order for this to work we need a first argument AND we need a nullptr as the last argument.
//       The first argument is required for the va_start call (if compiling in C23+ you could omit this).
//       The nullptr last argument is required so we can discover the number of arguments automatically.
//       Both requirements are enforced by the macros, so you should call those, not the functions with trailing underscore.
//
// NOTE: You cannot have a trailing comma at the end of your strings, and you cannot have nullptr as a value (use "" instead)

StrArray MakeStrArrayVa_(int firstArgument, ...)
{
	StrArray result = EMPTY;
	va_list args;
	va_start(args, firstArgument); //omitting second argument here is available after C23
	while (true)
	{
		const char* nextStr = va_arg(args, const char*);
		if (nextStr == nullptr) { break; }
		AddStrNt(&result, nextStr);
	}
	va_end(args);
	return result;
}
#define MakeStrArrayVa(...) MakeStrArrayVa_(0, ##__VA_ARGS__, (const char*)0)

void AddStrsVa_(StrArray* array, ...)
{
	va_list args;
	va_start(args, array);
	while (true)
	{
		const char* nextStr = va_arg(args, const char*);
		if (nextStr == nullptr) { break; }
		AddStrNt(array, nextStr);
	}
	va_end(args);
}
#define AddStrsVa(arrayPntr, ...) AddStrsVa_((arrayPntr), ##__VA_ARGS__, (const char*)0)

#define STR_VA_ARGS_SENTINEL MakeStr(UINT64_MAX, nullptr)
StrArray MakeStrArrayVaStr_(int firstArgument, ...)
{
	StrArray result = EMPTY;
	va_list args;
	va_start(args, firstArgument); //omitting second argument here is available after C23
	while (true)
	{
		Str nextStr = va_arg(args, Str);
		if (nextStr.length == UINT64_MAX) { break; }
		AddStr(&result, nextStr);
	}
	va_end(args);
	return result;
}
#define MakeStrArrayVaStr(...) MakeStrArrayVaStr_(0, ##__VA_ARGS__, STR_VA_ARGS_SENTINEL)

void AddStrsVaStr_(StrArray* array, ...)
{
	va_list args;
	va_start(args, array);
	while (true)
	{
		Str nextStr = va_arg(args, Str);
		if (nextStr.length == UINT64_MAX) { break; }
		AddStr(array, nextStr);
	}
	va_end(args);
}
#define AddStrsVaStr(arrayPntr, ...) AddStrsVaStr_((arrayPntr), ##__VA_ARGS__, STR_VA_ARGS_SENTINEL)

#endif //  _PIG_BUILD_STR_ARRAY_H
