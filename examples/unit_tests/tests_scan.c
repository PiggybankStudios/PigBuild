/*
File:   tests_scan.c
Author: Taylor Robbins
Date:   05\11\2026
Description:
	** Holds tests for Scan code inside "pig_build_scan.h"
*/

void PrintScan(Scan* scan)
{
	Str beforeCursorStr = StrSlice(scan->str, 0, scan->cursor);
	Str afterCursorStr = StrSliceFrom(scan->str, scan->cursor);
	bool ellipsesLeft = false;
	bool ellipsesRight = false;
	if (beforeCursorStr.length > 8) { beforeCursorStr = StrSliceFrom(beforeCursorStr, beforeCursorStr.length - 8); ellipsesLeft = true; }
	if (afterCursorStr.length > 8) { afterCursorStr = StrSlice(afterCursorStr, 0, 8); ellipsesRight = true; }
	PrintLine("\t%s \"%.*s\" at %llu/%llu \"%s%.*s|%.*s%s\"",
		scan->failed ? "Failed" : "Matched",
		StrPrint(scan->errorStr),
		scan->cursor,
		scan->str.length,
		ellipsesLeft ? "..." : "",
		StrPrint(beforeCursorStr),
		StrPrint(afterCursorStr),
		ellipsesRight ? "..." : ""
	);
}

void RunTests_Scan()
{
	Str loremIpsum = StrLit("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	
	{
		Str regex = StrLit("$lorem");
		Scan m = NewScan(loremIpsum);
		ScanAnyCaseStr(&m, StrLit("lorem"));
		PrintLine("%.*s", StrPrint(regex));
		PrintScan(&m);
	}
	
	{
		Str regex = StrLit("$ipsum");
		Scan m = NewScan(loremIpsum);
		ScanAnyCaseStr(&m, StrLit("ipsum"));
		PrintLine("%.*s", StrPrint(regex));
		PrintScan(&m);
	}
	
	{
		Str regex = StrLit("lorem\\s*ipsum");
		Scan m = NewScan(loremIpsum);
		ScanAnyCaseStr(&m, StrLit("lorem"));
		ScanMin(&m, ScanSet_Whitespace, 1);
		ScanAnyCaseStr(&m, StrLit("ipsum"));
		PrintLine("%.*s", StrPrint(regex));
		PrintScan(&m);
	}
	
	{
		Str regex = StrLit("(?:[A-Za-z]+\\s+)+");
		Scan m = NewScan(loremIpsum);
		while (!m.failed && m.cursor < m.str.length)
		{
			ScanMin(&m, ScanSet_Both(ScanSet_Alphabetic, ScanSet_StrLit(",.")), 1);
			ScanZeroOrMore(&m, ScanSet_Whitespace);
		}
		PrintLine("%.*s", StrPrint(regex));
		PrintScan(&m);
	}
}
