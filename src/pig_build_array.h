/*
File:   pig_build_array.h
Author: Taylor Robbins
Date:   04\23\2026
Description:
	** Holds a system whereby we can create arrays that can grow and shrink as items
	** are added and removed. In order to do this in a type-safe way in C we have
	** to do a wierd thing where you call a macro to declare a new version of the
	** array that contains a specific element type. This macro declares the struct
	** for that typed array and it's API. That generated API routes the arguments
	** to the generic API here. Since the generated API has proper pointer types
	** the compiler will enforce correct types being passed and the casts to (void*)
	** and back are relatively safe.
*/

#ifndef _PIG_BUILD_ARRAY_H
#define _PIG_BUILD_ARRAY_H

#include "pig_build_base.h"

#define ARRAY_MIN_NUM_ITEMS 8
#define ARRAY_GROWTH_RATE   2 //double size every time we grow

void GrowArrayToAtLeast(u64 itemSize, u64 itemAlignment, u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr, u64 lengthNeeded)
{
	Assert(*itemsPntr != nullptr || *allocLengthPntr == 0);
	Assert(*lengthPntr <= *allocLengthPntr);
	if (*allocLengthPntr >= lengthNeeded) { return; }
	u64 newLength = *lengthPntr;
	if (newLength < ARRAY_MIN_NUM_ITEMS) { newLength = ARRAY_MIN_NUM_ITEMS; }
	while (newLength < lengthNeeded) { newLength *= ARRAY_GROWTH_RATE; }
	void* newItems = malloc(itemSize * newLength); //TODO: Do aligned_malloc?
	AssertFmt(newItems != nullptr, "Failed to grow Array to %llu items", newLength);
	if (*lengthPntr > 0) { memcpy(newItems, *itemsPntr, itemSize * *lengthPntr); }
	if (*itemsPntr != nullptr) { free(*itemsPntr); }
	*itemsPntr = newItems;
	*allocLengthPntr = newLength;
}

void FreeArray(u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr)
{
	Assert(*itemsPntr != nullptr || *allocLengthPntr == 0);
	if (*itemsPntr != nullptr) { free(*itemsPntr); }
	*lengthPntr = 0;
	*allocLengthPntr = 0;
	*itemsPntr = nullptr;
}
void EmptyArray(u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr)
{
	Assert(*itemsPntr != nullptr || *allocLengthPntr == 0);
	*lengthPntr = 0;
}

void* InsertItemsArray(u64 itemSize, u64 itemAlignment, u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr, u64 insertIndex, u64 numItems)
{
	Assert(insertIndex <= *lengthPntr);
	GrowArrayToAtLeast(itemSize, itemAlignment, lengthPntr, allocLengthPntr, itemsPntr, *lengthPntr + numItems);
	u8* bytePntr = (u8*)*itemsPntr;
	u8* insertPntr = bytePntr + (itemSize * insertIndex);
	if (numItems > 0)
	{
		if (insertIndex < *lengthPntr) { memmove(insertPntr + (itemSize * numItems), insertPntr, itemSize * numItems); }
		memset(insertPntr, 0x00, itemSize * numItems);
		*lengthPntr += numItems;
	}
	return insertPntr;
}

void* InsertValuesArray(u64 itemSize, u64 itemAlignment, u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr, u64 insertIndex, u64 numItems, const void* valuesPntr)
{
	void* result = InsertItemsArray(itemSize, itemAlignment, lengthPntr, allocLengthPntr, itemsPntr, insertIndex, numItems);
	if (result == nullptr) { return result; }
	Assert(valuesPntr != nullptr || numItems == 0);
	if (numItems > 0) { memcpy(result, valuesPntr, itemSize * numItems); }
	return result;
}

void RemoveItemsArray(u64 itemSize, u64 itemAlignment, u64* lengthPntr, u64* allocLengthPntr, void** itemsPntr, u64 removeIndex, u64 numItems)
{
	Assert(*itemsPntr != nullptr || *allocLengthPntr == 0);
	Assert(*lengthPntr <= *allocLengthPntr);
	Assert(removeIndex + numItems <= *lengthPntr);
	if (numItems == 0) { return; }
	u8* bytePntr = (u8*)itemsPntr;
	u8* removePntr = bytePntr + (itemSize * removeIndex);
	if (removeIndex + numItems < *lengthPntr) { memmove(removePntr, removePntr + (itemSize * numItems), itemSize * (*lengthPntr - (removeIndex + numItems))); }
	*lengthPntr -= numItems;
}

// Use this macro to create new typed arrays with a particular element type.
// This generates the struct and the API to add and remove elements from the array.
//
// For example, TYPED_ARRAY(Array_u64, u64, values) generates:
//  typedef struct Array_u64 Array_u64;
//  struct Array_u64
//  {
//  	u64 length;
//  	u64 allocLength;
//  	union { u64* items; u64* values; void* pntr; };
//  };
//  void         FreeArray_u64(Array_u64* arrayPntr) { ... }
//  void        EmptyArray_u64(Array_u64* arrayPntr) { ... }
//  u64*  InsertItemsArray_u64(Array_u64* arrayPntr, u64 insertIndex, u64 numItems) { ... }
//  u64*   InsertItemArray_u64(Array_u64* arrayPntr, u64 insertIndex) { ... }
//  u64*     AddItemsArray_u64(Array_u64* arrayPntr, u64 numItems) { ... }
//  u64*      AddItemArray_u64(Array_u64* arrayPntr) { ... }
//  u64* InsertValuesArray_u64(Array_u64* arrayPntr, u64 insertIndex, u64 numItems, const u64* valuesPntr) { ... }
//  u64*  InsertValueArray_u64(Array_u64* arrayPntr, u64 insertIndex, u64 value) { ... }
//  u64*    AddValuesArray_u64(Array_u64* arrayPntr, u64 numItems, const u64* valuesPntr) { ... }
//  u64*     AddValueArray_u64(Array_u64* arrayPntr, u64 value) { ... }
//  u64*       AppendArray_u64(Array_u64* destArray, const Array_u64* srcArray) { ... }
//  void  RemoveItemsArray_u64(Array_u64* arrayPntr, u64 removeIndex, u64 numItems) { ... }
//  void   RemoveItemArray_u64(Array_u64* arrayPntr, u64 removeIndex) { ... }
//  void     PopItemsArray_u64(Array_u64* arrayPntr, u64 numItems) { ... }
//  void      PopItemArray_u64(Array_u64* arrayPntr) { ... }
//
#define TYPED_ARRAY(structName, elemType, itemsAlias)                                                                                                                                                                                                                                                                                    \
struct structName                                                                                                                                                                                                                                                                                                                        \
{                                                                                                                                                                                                                                                                                                                                        \
	u64 length;                                                                                                                                                                                                                                                                                                                          \
	u64 allocLength;                                                                                                                                                                                                                                                                                                                     \
	union { elemType* items; elemType* itemsAlias; void* pntr; };                                                                                                                                                                                                                                                                        \
};                                                                                                                                                                                                                                                                                                                                       \
void              Free##structName(struct structName* arrayPntr) {  FreeArray(&arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items); }                                                                                                                                                                                 \
void             Empty##structName(struct structName* arrayPntr) { EmptyArray(&arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items); }                                                                                                                                                                                 \
elemType*  InsertItems##structName(struct structName* arrayPntr, u64 insertIndex, u64 numItems)  { return (elemType*)InsertItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, insertIndex,       numItems); }                                                      \
elemType*   InsertItem##structName(struct structName* arrayPntr, u64 insertIndex)                { return (elemType*)InsertItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, insertIndex,              1); }                                                      \
elemType*     AddItems##structName(struct structName* arrayPntr, u64 numItems)                   { return (elemType*)InsertItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length, numItems); }                                                      \
elemType*      AddItem##structName(struct structName* arrayPntr)                                 { return (elemType*)InsertItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length,        1); }                                                      \
elemType* InsertValues##structName(struct structName* arrayPntr, u64 insertIndex, u64 numItems, const elemType* valuesPntr) { return (elemType*)InsertValuesArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, insertIndex,       numItems, (const void*)valuesPntr); } \
elemType*  InsertValue##structName(struct structName* arrayPntr, u64 insertIndex, elemType value)                           { return (elemType*)InsertValuesArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, insertIndex,              1, (const void*)&value);     } \
elemType*    AddValues##structName(struct structName* arrayPntr, u64 numItems, const elemType* valuesPntr)                  { return (elemType*)InsertValuesArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length, numItems, (const void*)valuesPntr); } \
elemType*     AddValue##structName(struct structName* arrayPntr, elemType value)                                            { return (elemType*)InsertValuesArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length,        1, (const void*)&value);     } \
elemType*       Append##structName(struct structName* destArray, const struct structName* srcArray)                         { return (elemType*)InsertValuesArray(sizeof(elemType), _Alignof(elemType), &destArray->length, &destArray->allocLength, (void**)&destArray->items, destArray->length, srcArray->length, srcArray->pntr);  } \
void       RemoveItems##structName(struct structName* arrayPntr, u64 removeIndex, u64 numItems) { Assert(arrayPntr->length >= numItems); RemoveItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, removeIndex,                numItems); }                         \
void        RemoveItem##structName(struct structName* arrayPntr, u64 removeIndex)               { Assert(arrayPntr->length >= 1);        RemoveItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, removeIndex,                       1); }                         \
void          PopItems##structName(struct structName* arrayPntr, u64 numItems)                  { Assert(arrayPntr->length >= numItems); RemoveItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length-numItems, numItems); }                         \
void           PopItem##structName(struct structName* arrayPntr)                                { Assert(arrayPntr->length >= 1);        RemoveItemsArray(sizeof(elemType), _Alignof(elemType), &arrayPntr->length, &arrayPntr->allocLength, (void**)&arrayPntr->items, arrayPntr->length-1,               1); }                         \
/* This comes at the end so the semicolon after the macro invocation is required */                                                                                                                                                                                                                                                      \
typedef struct structName structName

TYPED_ARRAY(Array_u8,       u8, values);
TYPED_ARRAY(Array_u16,     u16, values);
TYPED_ARRAY(Array_u32,     u32, values);
TYPED_ARRAY(Array_u64,     u64, values);
TYPED_ARRAY(Array_i8,       i8, values);
TYPED_ARRAY(Array_i16,     i16, values);
TYPED_ARRAY(Array_i32,     i32, values);
TYPED_ARRAY(Array_i64,     i64, values);
TYPED_ARRAY(Array_r32,     r32, values);
TYPED_ARRAY(Array_r64,     r64, values);
TYPED_ARRAY(Array_pntrs, void*, pntrs);

#endif //  _PIG_BUILD_ARRAY_H
