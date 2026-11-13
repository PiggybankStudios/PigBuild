/*
File:   tests_shell_equivalents.c
Author: Taylor Robbins
Date:   11\12\2026
Description: 
	** Holds tests for Scan code inside "pig_build_shell_equivalents.h"
*/

void TestCaseLines_pig_word_count(u64 expectedCount, const char* targetStrOrPath, bool isPath)
{
	u64 count = pig_word_count(targetStrOrPath, .path=isPath, .lines=true, .count_last_line=true);
	AssertFmt(expectedCount == count, "Expected \"%s\" to have %llu lines not %llu (path=%s)",
		targetStrOrPath,
		expectedCount, count,
		isPath ? "true" : "false"
	);
	PrintLine("\"%s\" has %llu line%s", targetStrOrPath, count, (count == 1) ? "" : "s");
}
void TestCaseWords_pig_word_count(u64 expectedCount, const char* targetStr, const char* word_chars, const char* non_word_chars)
{
	u64 count = pig_word_count(targetStr, .word_chars=word_chars, .non_word_chars=non_word_chars);
	AssertFmt(expectedCount == count, "Expected \"%s\" to have %llu words not %llu (word_chars=\"%s\", non_word_chars=\"%s\")",
		targetStr,
		expectedCount, count,
		word_chars, non_word_chars
	);
	PrintLine("\"%s\" has %llu word%s", targetStr, count, (count == 1) ? "" : "s");
}

void RunTests_ShellEquivalents()
{
	TestCaseLines_pig_word_count(13, PIG_BUILD_ROOT "/build.bat", true);
	TestCaseLines_pig_word_count(8, "Line1\nLine2\nLine3\nLine4\nLine5\nLine6\nLine7\n", false);
	TestCaseWords_pig_word_count(5, "Lorem ipsum dolor sit amet", nullptr, nullptr);
	TestCaseWords_pig_word_count(3, "able-bodied high-spirited round-trip", "abcdefghijklmnopqrstuvwyz-", nullptr);
	TestCaseWords_pig_word_count(3, "able-bodied r&b function(&a,b,*c)", nullptr, " ");
}
