/*
File:   tests_str_list.c
Author: Taylor Robbins
Date:   05\01\2026
Description:
	** Holds tests for StrList code inside "pig_build_str_list.h"
*/

void PrintStrList(StrList* list)
{
	PrintLine("List has %llu item%s:", list->length, Plural(list->length, "s"));
	u64 sIndex = 0;
	Str* strPntr = FirstStrList(list);
	Str nullStr = StrLit("[null]");
	while (strPntr != nullptr)
	{
		Str* next = NextStr(strPntr);
		Str* prev = PrevStr(strPntr);
		if (next == nullptr) { next = &nullStr; }
		if (prev == nullptr) { prev = &nullStr; }
		PrintLine("[%llu] \"%.*s\" (prev=\"%.*s\", next=\"%.*s\")", sIndex, StrPrint(*strPntr), StrPrint(*prev), StrPrint(*next));
		strPntr = NextStr(strPntr);
		sIndex++;
	}
}

void RunTests_StrList()
{
	WriteLine("+--------------------------------------------------------------+");
	WriteLine("|          Running Tests for pig_build_str_list.h...           |");
	WriteLine("+--------------------------------------------------------------+");
	
	StrList test1 = EMPTY;
	Str* helloStr1 = AddStrLitList(&test1, "Hello");
	Str* worldStr  = AddStrLitList(&test1, "World!");
	Str* helloStr2 = AddStrLitList(&test1, "Hello");
	Str* sailorStr = AddStrLitList(&test1, "Sailor!");
	PrintStrList(&test1);
	RemoveStrList(&test1, helloStr2); WriteLine("Removing Hello(2)");
	PrintStrList(&test1);
	RemoveStrList(&test1, helloStr1); WriteLine("Removing Hello(1)");
	PrintStrList(&test1);
	RemoveStrList(&test1, worldStr); WriteLine("Removing World!");
	PrintStrList(&test1);
	RemoveStrList(&test1, sailorStr); WriteLine("Removing Sailor!");
	PrintStrList(&test1);
	
	StrList test2 = EMPTY;
	Str* LoremStr       = AddStrLitList(&test2, "Lorem");
	Str* dolorStr       = AddStrLitList(&test2, "dolor");
	Str* sitStr         = AddStrLitList(&test2, "sit");
	Str* ametStr        = AddStrLitList(&test2, "amet,");
	Str* ipsumStr       = InsertValueList_Str(&test2, 1, StrLit("ipsum"));
	Str* consecteturStr = AddStrLitList(&test2, "consectetur");
	Str* adipiscingStr  = AddStrLitList(&test2, "adipiscing");
	Str* elitStr        = AddStrLitList(&test2, "elit");
	PrintStrList(&test2);
	
	StrList test3 = EMPTY;
	InsertValueList_Str(&test3, 0, StrLit("elit"));
	InsertValueList_Str(&test3, 0, StrLit("adipiscing"));
	InsertValueList_Str(&test3, 0, StrLit("consectetur"));
	InsertValueList_Str(&test3, 0, StrLit("amet,"));
	InsertValueList_Str(&test3, 0, StrLit("sit"));
	InsertValueList_Str(&test3, 0, StrLit("dolor"));
	InsertValueList_Str(&test3, 0, StrLit("ipsum"));
	InsertValueList_Str(&test3, 0, StrLit("Lorem"));
	PrintStrList(&test3);
	
}
