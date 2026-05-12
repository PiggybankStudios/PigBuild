/*
File:   tests_matcher.c
Author: Taylor Robbins
Date:   05\11\2026
Description:
	** Holds tests for Matcher code inside "pig_build_matcher.h"
*/

void PrintMatcher(Matcher* matcher)
{
	Str beforeCursorStr = StrSlice(matcher->str, 0, matcher->cursor);
	Str afterCursorStr = StrSliceFrom(matcher->str, matcher->cursor);
	bool ellipsesLeft = false;
	bool ellipsesRight = false;
	if (beforeCursorStr.length > 8) { beforeCursorStr = StrSliceFrom(beforeCursorStr, beforeCursorStr.length - 8); ellipsesLeft = true; }
	if (afterCursorStr.length > 8) { afterCursorStr = StrSlice(afterCursorStr, 0, 8); ellipsesRight = true; }
	PrintLine("\t%s \"%.*s\" at %llu/%llu \"%s%.*s|%.*s%s\"",
		matcher->matchFailed ? "Failed" : "Matched",
		StrPrint(matcher->errorStr),
		matcher->cursor,
		matcher->str.length,
		ellipsesLeft ? "..." : "",
		StrPrint(beforeCursorStr),
		StrPrint(afterCursorStr),
		ellipsesRight ? "..." : ""
	);
}

void RunTests_Matcher()
{
	Str loremIpsum = StrLit("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	
	{
		Str equivalentRegex = StrLit("$lorem");
		Matcher m = NewMatcher(loremIpsum);
		MatchAnyCaseStr(&m, StrLit("lorem"));
		
		PrintLine("%.*s", StrPrint(equivalentRegex));
		PrintMatcher(&m);
	}
	
	{
		Str equivalentRegex = StrLit("$ipsum");
		Matcher m = NewMatcher(loremIpsum);
		MatchAnyCaseStr(&m, StrLit("ipsum"));
		
		PrintLine("%.*s", StrPrint(equivalentRegex));
		PrintMatcher(&m);
	}
	
	{
		Str equivalentRegex = StrLit("lorem\\s*ipsum");
		Matcher m = NewMatcher(loremIpsum);
		MatchAnyCaseStr(&m, StrLit("lorem"));
		MatchAtLeast(&m, MatcherSet_Whitespace, 1);
		MatchAnyCaseStr(&m, StrLit("ipsum"));
		
		PrintLine("%.*s", StrPrint(equivalentRegex));
		PrintMatcher(&m);
	}
	
	{
		Str equivalentRegex = StrLit("(?:[A-Za-z]+\\s+)+");
		Matcher m = NewMatcher(loremIpsum);
		while (!m.matchFailed)
		{
			MatchAtLeast(&m, MatcherSet_Both(MatcherSet_Alphabetic, MatcherSet_Char(',')), 1);
			MatchAtLeast(&m, MatcherSet_Whitespace, 1);
		}
		
		PrintLine("%.*s", StrPrint(equivalentRegex));
		PrintMatcher(&m);
	}
}
