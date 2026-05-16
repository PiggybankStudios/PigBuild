/*
File:   pig_build_list.h
Author: Taylor Robbins
Date:   04\24\2026
Description:
	** Similar to pig_build_array.h this file defines a data structure macro TYPED_LIST
	** which can be used to stamp a new typed version of a doubly-linked list and it's API.
*/

// Example:
//  TYPED_LIST(List_u64, ListLink_u64, u64, value);
//          |
//      Generates
//          |
//          v
//  typedef struct ListLink_u64 ListLink_u64;
//  struct ListLink_u64
//  {
//  	union
//  	{
//  		ListLink header;
//  		struct { ListLink_u64* next; ListLink_u64* prev; };
//  	};
//  	union
//  	{
//  		u64 item;
//  		u64 value;
//  	};
//  };
//  typedef struct List_u64 List_u64;
//  struct List_u64
//  {
//  	u64 length;
//  	ListLink_u64* first;
//  	ListLink_u64* last;
//  };
//  ListLink_u64* GetLinkList_u64(u64* itemPntr) { ... }
//  u64*         NextItemList_u64(u64* itemPntr) { ... }
//  u64*         PrevItemList_u64(u64* itemPntr) { ... }
//  void        FirstItemList_u64(List_u64* list) { ... }
//  void         LastItemList_u64(List_u64* list) { ... }
//  void             FreeList_u64(List_u64* list) { ... }
//  u64*   GetItemAtIndexList_u64(List_u64* list, u64 index) { ... }
//  u64*       InsertItemList_u64(List_u64* list, u64 insertIndex) { ... }
//  u64*          AddItemList_u64(List_u64* list) { ... }
//  u64*      InsertValueList_u64(List_u64* list, u64 insertIndex, u64 value)
//  u64*         AddValueList_u64(List_u64* list, u64 value)
//  void       RemoveItemList_u64(List_u64* list, u64* itemPntr) { ... }

#ifndef _PIG_BUILD_LIST_H
#define _PIG_BUILD_LIST_H

#include "pig_build_base.h"

//TODO: AddItemsList/AddValuesList
//TODO: AppendList
//TODO: SortList
//TODO: ReverseList
//TODO: FindValueList
//TODO: IndexOfItemList

typedef struct ListLink ListLink;
struct ListLink
{
	ListLink* next;
	ListLink* prev;
};

void FreeList(u64 linkSize, u64 linkAlignment, u64* lengthPntr, ListLink** firstPntr, ListLink** lastPntr)
{
	ListLink* link = *firstPntr;
	while (link != nullptr)
	{
		ListLink* next = link->next;
		free(link);
		link = next;
	}
	*lengthPntr = 0;
	*firstPntr = nullptr;
	*lastPntr = nullptr;
}

ListLink* GetItemAtIndexList(u64 length, ListLink* first, ListLink* last, u64 index)
{
	ListLink* result = nullptr;
	if (index < length)
	{
		if (index >= length/2)
		{
			result = last;
			u64 iterIndex = length - 1;
			while (iterIndex > index)
			{
				Assert(result != nullptr);
				result = result->prev;
				iterIndex--;
			}
		}
		else
		{
			result = first;
			u64 iterIndex = 0;
			while (iterIndex < index)
			{
				Assert(result != nullptr);
				result = result->next;
				iterIndex++;
			}
		}
		Assert(result != nullptr);
	}
	return result;
}

void* InsertItemList(u64 linkSize, u64 linkAlignment, u64 itemOffset, u64* lengthPntr, ListLink** firstPntr, ListLink** lastPntr, u64 insertIndex)
{
	Assert(insertIndex <= *lengthPntr);
	ListLink* newLink = (ListLink*)malloc(linkSize); //TODO: Do aligned_malloc?
	memset(newLink, 0x00, linkSize);
	
	ListLink* itemAtIndex = GetItemAtIndexList(*lengthPntr, *firstPntr, *lastPntr, insertIndex);
	
	if (itemAtIndex != nullptr)
	{
		newLink->next = itemAtIndex;
		newLink->prev = itemAtIndex->prev;
		itemAtIndex->prev = newLink;
	}
	else
	{
		newLink->prev = *lastPntr;
	}
	
	if (newLink->prev != nullptr) { newLink->prev->next = newLink; }
	
	if (insertIndex == 0) { *firstPntr = newLink; }
	if (insertIndex == *lengthPntr) { *lastPntr = newLink; }
	
	(*lengthPntr)++;
	return ((u8*)newLink) + itemOffset;
}

void RemoveItemList(u64* lengthPntr, ListLink** firstPntr, ListLink** lastPntr, ListLink* itemLinkPntr)
{
	Assert(*lengthPntr > 0);
	if (itemLinkPntr->prev != nullptr) { itemLinkPntr->prev->next = itemLinkPntr->next; }
	else { *firstPntr = itemLinkPntr->next; }
	if (itemLinkPntr->next != nullptr) { itemLinkPntr->next->prev = itemLinkPntr->prev; }
	else { *lastPntr = itemLinkPntr->prev; }
	free(itemLinkPntr);
	(*lengthPntr)--;
}

// Use this macro to create new typed list with a particular element type.
// This generates the list struct, a link struct that wraps a ListLink header
// and the element type, and the API to add and remove elements from the list.
#define TYPED_LIST(structName, linkStructName, elemType, itemAlias)                                                                                                                                                                                                                                                                                        \
typedef struct linkStructName linkStructName;                                                                                                                                                                                                                                                                                                              \
struct linkStructName                                                                                                                                                                                                                                                                                                                                      \
{                                                                                                                                                                                                                                                                                                                                                          \
	union                                                                                                                                                                                                                                                                                                                                                  \
	{                                                                                                                                                                                                                                                                                                                                                      \
		ListLink header;                                                                                                                                                                                                                                                                                                                                   \
		struct { linkStructName* next; linkStructName* prev; };                                                                                                                                                                                                                                                                                            \
	};                                                                                                                                                                                                                                                                                                                                                     \
	union                                                                                                                                                                                                                                                                                                                                                  \
	{                                                                                                                                                                                                                                                                                                                                                      \
		elemType item;                                                                                                                                                                                                                                                                                                                                     \
		elemType itemAlias;                                                                                                                                                                                                                                                                                                                                \
	};                                                                                                                                                                                                                                                                                                                                                     \
};                                                                                                                                                                                                                                                                                                                                                         \
struct structName                                                                                                                                                                                                                                                                                                                                          \
{                                                                                                                                                                                                                                                                                                                                                          \
	u64 length;                                                                                                                                                                                                                                                                                                                                            \
	linkStructName* first;                                                                                                                                                                                                                                                                                                                                 \
	linkStructName* last;                                                                                                                                                                                                                                                                                                                                  \
};                                                                                                                                                                                                                                                                                                                                                         \
linkStructName*     GetLink##structName(elemType* itemPntr)                                       { return (linkStructName*)(((u8*)itemPntr) - MEMBER_OFFSET(linkStructName, item)); }                                                                                                                                                                     \
elemType*          NextItem##structName(elemType* itemPntr)                                       { linkStructName* wrapper = GetLink##structName(itemPntr); return (wrapper->next != nullptr) ? &wrapper->next->item : nullptr; }                                                                                                                         \
elemType*          PrevItem##structName(elemType* itemPntr)                                       { linkStructName* wrapper = GetLink##structName(itemPntr); return (wrapper->prev != nullptr) ? &wrapper->prev->item : nullptr; }                                                                                                                         \
elemType*         FirstItem##structName(struct structName* list)                                  { return (list->first != nullptr) ? &list->first->item : nullptr; }                                                                                                                                                                                      \
elemType*          LastItem##structName(struct structName* list)                                  { return (list->last != nullptr) ? &list->last->item : nullptr; }                                                                                                                                                                                        \
void                   Free##structName(struct structName* list)                                  { FreeList(sizeof(linkStructName), _Alignof(linkStructName), &list->length, (ListLink**)&list->first, (ListLink**)&list->last); }                                                                                                                        \
elemType*    GetItemAtIndex##structName(struct structName* list, u64 index)                       { linkStructName* itemLink = (linkStructName*)GetItemAtIndexList(list->length, &list->first->header, &list->last->header, index); return (itemLink != nullptr) ? &itemLink->item : nullptr; }                                                            \
elemType*        InsertItem##structName(struct structName* list, u64 insertIndex)                 { return (elemType*)InsertItemList(sizeof(linkStructName), _Alignof(linkStructName), MEMBER_OFFSET(linkStructName, item), &list->length, (ListLink**)&list->first, (ListLink**)&list->last,  insertIndex); }                                             \
elemType*           AddItem##structName(struct structName* list)                                  { return (elemType*)InsertItemList(sizeof(linkStructName), _Alignof(linkStructName), MEMBER_OFFSET(linkStructName, item), &list->length, (ListLink**)&list->first, (ListLink**)&list->last, list->length); }                                             \
elemType*       InsertValue##structName(struct structName* list, u64 insertIndex, elemType value) { elemType* result = (elemType*)InsertItemList(sizeof(linkStructName), _Alignof(linkStructName), MEMBER_OFFSET(linkStructName, item), &list->length, (ListLink**)&list->first, (ListLink**)&list->last,  insertIndex); *result = value; return result; } \
elemType*          AddValue##structName(struct structName* list, elemType value)                  { elemType* result = (elemType*)InsertItemList(sizeof(linkStructName), _Alignof(linkStructName), MEMBER_OFFSET(linkStructName, item), &list->length, (ListLink**)&list->first, (ListLink**)&list->last, list->length); *result = value; return result; } \
void             RemoveItem##structName(struct structName* list, elemType* itemPntr)              { RemoveItemList(&list->length, (ListLink**)&list->first, (ListLink**)&list->last, (ListLink*)GetLink##structName(itemPntr)); }                                                                                                                          \
/* This comes at the end so the semicolon after the macro invocation is required */                                                                                                                                                                                                                                                                        \
typedef struct structName structName

TYPED_LIST(List_int,   ListLink_int,     int, value);
TYPED_LIST(List_long,  ListLink_long,   long, value);
TYPED_LIST(List_char,  ListLink_char,   char, value);
TYPED_LIST(List_bool,  ListLink_bool,   bool, value);
TYPED_LIST(List_u8,    ListLink_u8,       u8, value);
TYPED_LIST(List_u16,   ListLink_u16,     u16, value);
TYPED_LIST(List_u32,   ListLink_u32,     u32, value);
TYPED_LIST(List_u64,   ListLink_u64,     u64, value);
TYPED_LIST(List_i8,    ListLink_i8,       i8, value);
TYPED_LIST(List_i16,   ListLink_i16,     i16, value);
TYPED_LIST(List_i32,   ListLink_i32,     i32, value);
TYPED_LIST(List_i64,   ListLink_i64,     i64, value);
TYPED_LIST(List_r32,   ListLink_r32,     r32, value);
TYPED_LIST(List_r64,   ListLink_r64,     r64, value);
TYPED_LIST(List_pntrs, ListLink_pntrs, void*, pntr);
TYPED_LIST(List_CStr,  ListLink_CStr,  char*, value);

#endif //  _PIG_BUILD_LIST_H
