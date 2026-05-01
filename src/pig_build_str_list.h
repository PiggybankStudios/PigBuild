/*
File:   pig_build_str_list.h
Author: Taylor Robbins
Date:   04\25\2026
Description:
	** StrList (aka List_Str) is TYPED_LIST (see pig_build_list.h) of Str (see pig_build_str.h)
	** with some extra logic on top to copy the strings when they are added and free
	** them when they are removed.
*/

#ifndef _PIG_BUILD_STR_LIST_H
#define _PIG_BUILD_STR_LIST_H

#include "pig_build_base.h"
#include "pig_build_list.h"
#include "pig_build_str.h"

TYPED_LIST(List_Str, ListLink_Str, Str, str);
typedef ListLink_Str StrListLink;
typedef List_Str StrList;

void FreeStrList(StrList* list)
{
	StrListLink* link = list->first;
	while (link != nullptr) { FreeStr(&link->str); link = link->next; }
	FreeList_Str(list);
}

Str* AddStrList(StrList* list, Str strValue)
{
	Str* newString = AddItemList_Str(list);
	*newString = CopyStr(strValue);
	return newString;
}
#define AddStrLitList(listPntr, stringLiteral) AddStrList((listPntr), StrLit(stringLiteral))
#define AddStrNtList(listPntr, nullTermStr)    AddStrList((listPntr), MakeStrNt(nullTermStr))

void RemoveStrList(StrList* list, Str* strPntr)
{
	FreeStr(strPntr);
	RemoveItemList_Str(list, strPntr);
}

StrListLink* GetStrListLink(Str* listStrPntr) { return GetLinkList_Str(listStrPntr); }
#define FirstStrList(listPntr) FirstItemList_Str(listPntr)
#define LastStrList(listPntr)  LastItemList_Str(listPntr)
Str* NextStr(Str* listStrPntr) { return NextItemList_Str(listStrPntr); }
Str* PrevStr(Str* listStrPntr) { return PrevItemList_Str(listStrPntr); }

#endif //  _PIG_BUILD_STR_LIST_H
