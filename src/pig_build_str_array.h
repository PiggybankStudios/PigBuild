/*
File:   pig_build_str_array.h
Author: Taylor Robbins
Date:   06\20\2025
Description:
	** StrArray (aka Array_Str) is TYPED_ARRAY (see pig_build_array.h) of Str (see pig_build_str.h)
	** with some extra logic on top to copy the strings when they are added and free
	** them when they are removed
*/

#ifndef _PIG_BUILD_STR_ARRAY_H
#define _PIG_BUILD_STR_ARRAY_H

#include "pig_build_base.h"
#include "pig_build_array.h"
#include "pig_build_str.h"

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
	Str* result = AddItemArray_Str(array);
	*result = CopyStr(newString);
	return result;
}
#define AddStrLit(arrayPntr, strLit)     AddStr((arrayPntr), StrLit(strLit))
#define AddStrNt(arrayPntr, nullTermStr) AddStr((arrayPntr), MakeStrNt(nullTermStr))

//Since tag macros all contain leading '|' character, we need to slice it off when adding it to a StrArray
Str* AddTag(StrArray* array, const char* newStringWithLeadingSepChar)
{
	Str tagStr = MakeStrNt(newStringWithLeadingSepChar);
	if (tagStr.length >= 1 && tagStr.chars[0] == '|') { tagStr = StrSliceFrom(tagStr, 1); }
	return AddStr(array, tagStr);
}

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
