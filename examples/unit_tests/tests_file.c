/*
File:   tests_file.c
Author: Taylor Robbins
Date:   09\22\2026
Description: 
	** Holds tests for Scan code inside "pig_build_file.h"
*/

void CollapseParentDirPathParts_Test_(Str input, Str expectedOutput)
{
	Str output = CollapseParentDirPathParts(input);
	AssertFmt(StrExactEquals(output, expectedOutput), "Collapsed \"%.*s\" became \"%.*s\" not \"%.*s\"", StrPrint(input), StrPrint(output), StrPrint(expectedOutput));
	PrintLine("Checked CollapseParentDirPathParts(\"%.*s\") -> \"%.*s\"", StrPrint(input), StrPrint(output));
}
#define CollapseParentDirPathParts_Test(inputLit, expectedOutputLit) CollapseParentDirPathParts_Test_(StrLit(inputLit), StrLit(expectedOutputLit))

void RunTests_File()
{
	CollapseParentDirPathParts_Test("foo", "foo");
	CollapseParentDirPathParts_Test("foo/", "foo/");
	CollapseParentDirPathParts_Test("foo/bar", "foo/bar");
	CollapseParentDirPathParts_Test("foo/bar/baz", "foo/bar/baz");
	CollapseParentDirPathParts_Test("foo/bar/baz/", "foo/bar/baz/");
	CollapseParentDirPathParts_Test("foo/../bar", "bar");
	CollapseParentDirPathParts_Test("foo/../bar/", "bar/");
	CollapseParentDirPathParts_Test("../bar/", "../bar/");
	CollapseParentDirPathParts_Test("foo/bar/../baz/", "foo/baz/");
	CollapseParentDirPathParts_Test("foo/bar/../../baz/", "baz/");
	CollapseParentDirPathParts_Test("foo/bar/../../../baz/", "../baz/");
	CollapseParentDirPathParts_Test("foo/bar/../../baz/../", "./");
	CollapseParentDirPathParts_Test("foo/bar/../../../baz/../", "../");
	CollapseParentDirPathParts_Test("foo/../../..", "../..");
	CollapseParentDirPathParts_Test("foo/..", ".");
	CollapseParentDirPathParts_Test("foo/../", "./");
	CollapseParentDirPathParts_Test("/foo/../", "/");
	CollapseParentDirPathParts_Test("/foo/", "/foo/");
}
