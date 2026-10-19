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

void TestUnusualPath(Str pathStr, bool shouldExist)
{
	PrintLine("Checking folder \"%.*s\"...", StrPrint(pathStr));
	if (shouldExist) { AssertFmt(DoesFolderExist(pathStr), "Couldn't find \"%.*s\"", StrPrint(pathStr)); }
	else { AssertFmt(!DoesFolderExist(pathStr), "Found folder that we shouldn't \"%.*s\"", StrPrint(pathStr)); }
}

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
	
	AssertFmt(DoesFolderExist(StrLit("/dev/disk0")) == false, "DoesFolderExist(\"/dev/disk0\") reported as a folder when it really is a block device (more of a file than a folder)");
	AssertFmt(DoesFileExist(StrLit("/dev/disk0")) == true, "DoesFolderExist(\"/dev/disk0\") reported false");
	
	#define UNUSUAL_PATH_ROOT "/Users/robbitay/my/temp/test_file_api/"
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "__"), false);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT " _"), false);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "_ "), false);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "\\"), true);
	// TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "\\\\/foo"), true); //TODO: We can make this folder on Apple FS but we can't address it?
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "\\/\\"), true);
	// TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "/\\/\\/bar"), true); //TODO: We can make this folder on Apple FS but we can't address it?
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "♠︎Spade♠︎"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "😃emoji😃"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "1234567890"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "baño"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "ending space"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "ending space "), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "folder with spaces"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "not꞉colon"), true); //NOTE: This colon is actually U+A789 "Modifier Letter Colon"
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "period."), true); //NOTE: This colon is actually U+A789 "Modifier Letter Colon"
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_!_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_@_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_#_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_$_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_%_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_^_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_&_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_*_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_(_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_)_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_-_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_=_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_+_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_[_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_]_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_{_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_}_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_;_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_'_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_\"_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_,_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_._"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_<_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_>_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_:_"), true); //NOTE: This folder looks like it's named char_/_ in Finder but it's actually a colon that's used in the POSIX version of the path
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_?_"), true);
	// TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_\\_"), true); //TODO: The call to FixPathSlashes(fullPathNt, PATH_SEP_CHAR) messes this one up
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_|_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_`_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "char_~_"), true);
	TestUnusualPath(StrLit(UNUSUAL_PATH_ROOT "!@#$%^&*()-=_+[]{};'\",.<>:?|`~"), true);
}

