/*
File:   pig_build_str_array.h
Author: Taylor Robbins
Date:   06\20\2025
*/

#ifndef _PIG_BUILD_STR_ARRAY_H
#define _PIG_BUILD_STR_ARRAY_H

#include "pig_build_base.h"
#include "pig_build_str.h"

typedef struct StrArray StrArray;
struct StrArray
{
	u64 length;
	u64 allocLength;
	Str* strings;
};

void FreeStrArray(StrArray* array)
{
	for (u64 sIndex = 0; sIndex < array->length; sIndex++)
	{
		if (array->strings[sIndex].chars != nullptr) { free(array->strings[sIndex].chars); }
	}
	if (array->strings != nullptr) { free(array->strings); array->strings = nullptr; }
	array->length = 0;
	array->allocLength = 0;
}

Str* AddStr(StrArray* array, Str newString)
{
	if (array->length >= array->allocLength)
	{
		u64 newAllocLength = array->allocLength;
		if (newAllocLength < 8) { newAllocLength = 8; }
		else { newAllocLength = newAllocLength*2; }
		Str* newAllocSpace = (Str*)malloc(sizeof(Str) * newAllocLength);
		if (array->length > 0) { memcpy(newAllocSpace, array->strings, sizeof(Str) * array->length); }
		if (array->strings != nullptr) { free(array->strings); }
		array->strings = newAllocSpace;
		array->allocLength = newAllocLength;
	}
	
	Str* result = &array->strings[array->length];
	array->length++;
	*result = CopyStr(newString, false);
	return result;
}

Str* InsertStr(StrArray* array, Str newString, u64 insertIndex)
{
	Str strAtEnd = *AddStr(array, newString);
	if (insertIndex < array->length)
	{
		memmove(&array->strings[insertIndex+1], &array->strings[insertIndex], (array->length-1) - insertIndex);
		array->strings[insertIndex] = strAtEnd;
	}
	return &array->strings[insertIndex];
}

Str* AddStrArray(StrArray* dest, const StrArray* src)
{
	if (src->length == 0) { return nullptr; }
	if (dest->length + src->length > dest->allocLength)
	{
		u64 newAllocLength = dest->allocLength;
		if (newAllocLength < 8) { newAllocLength = 8; }
		while (newAllocLength < dest->length + src->length) { newAllocLength = newAllocLength*2; }
		Str* newAllocSpace = (Str*)malloc(sizeof(Str) * newAllocLength);
		if (dest->length > 0) { memcpy(newAllocSpace, dest->strings, sizeof(Str) * dest->length); }
		if (dest->strings != nullptr) { free(dest->strings); }
		dest->strings = newAllocSpace;
		dest->allocLength = newAllocLength;
	}
	u64 resultIndex = dest->length;
	for (u64 sIndex = 0; sIndex < src->length; sIndex++)
	{
		AddStr(dest, src->strings[sIndex]);
	}
	return &dest->strings[resultIndex];
}

void RemoveStrAtIndex(StrArray* array, u64 index)
{
	assert(index < array->length);
	if (array->strings[index].chars != nullptr) { free(array->strings[index].chars); }
	if (index < array->length-1)
	{
		memmove(&array->strings[index], &array->strings[index+1], sizeof(Str) * (array->length - (index+1)));
	}
	array->length--;
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
