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

u64 FindStr(const StrArray* array, Str targetStr)
{
	for (u64 sIndex = 0; sIndex < array->length; sIndex++)
	{
		if (StrExactEquals(array->strings[sIndex], targetStr))
		{
			return sIndex;
		}
	}
	return array->length;
}
bool ContainsStr(const StrArray* array, Str targetStr)
{
	return (FindStr(array, targetStr) < array->length);
}

bool RemoveStr(StrArray* array, Str targetStr)
{
	u64 index = FindStr(array, targetStr);
	if (index >= array->length) { return false; }
	else
	{
		RemoveStrAtIndex(array, index);
		return true;
	}
}

#endif //  _PIG_BUILD_STR_ARRAY_H
