/*
File:   pig_build_file.h
Author: Taylor Robbins
Date:   03\21\2026
Description:
	** Contains wrappers around platform specific file system functions (like CreateFileA and fopen).
	** This file doesn't contain file path manipulation functions. Those are mostly
	** just string manipulation so they live in pig_build_str.h. The one exception is GetFullPath
	**
	** NOTE: These functions are not all fully tested and some functions are missing
	**       implementations for certain platforms. This file is provided for convenience
	**       and supports as many features as I've had time to implement and test
	**       but you are welcome to use any C or C++ library to abstract file operations
	**       or use the standard library's file functions directly. Some parts of
	**       Pig Build depend on these functions and would need to get updated to use
	**       whatever library or functions you want to replace this with.
	
	** When writing builds scripts in C we almost always need to
	** do some amount of file system manipulation. The C standard library
	** provides basic things like fopen, fclose, fread, etc. but that
	** does not cover all kinds of file interactions we want to do
	** (for example reading file write time, or creating\querying directories).
	**
	** There are also POSIX extensions that allow for some functionality
	** that isn't available if we are compiled by something like the MSVC compiler.
	**
	** It's also just generally useful to have a common point where all
	** file interaction routes through so we can do debug logging,
	** assertions, file path fixup, etc.
	**
	** For all these reasons we have our own API for doing file manipulation.
	**
	** You are not required to use this API if you don't want, but all file
	** manipulation internal to Pig Build functions will go through this API
	**
	** The design of this API is somewhat specialized to the needs of a build script.
	** We can assume that strings and arrays do not need to be freed,
	** we prioritize readability and ease of use over the smallest possible API,
	** we assume we want good error messages automatically at the cost of some code complexity.
	** We assume that we need to handle desktop OS differences
	** (file path quirks like forward vs back slash and file line ending differences '\n' vs '\r\n')
	** but we don't need to handle non-desktop OS use cases.
*/

#ifndef _PIG_BUILD_FILE_H
#define _PIG_BUILD_FILE_H

#include "pig_build_base.h"
#include "pig_build_str.h"

#define USE_NEW_API 0

//TODO: Is Windows the only file system where captilization doesn't matter?
//      Can we really assume capitalization importance based on OS?
#if BUILDING_ON_WINDOWS
#define IS_OS_FILESYSTEM_CASE_SENSITIVE 0
#else
#define IS_OS_FILESYSTEM_CASE_SENSITIVE 1
#endif

#if USE_NEW_API

// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// |                                               New API                                                |
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+

// Result is always a copied string with null-term
// TODO: On Linux/OSX, a path that doesn't exist cannot be resolved currently, because 'realpath' doesn't work on non-existent paths. On Windows GetFullPathNameA does work for non-existent files
// TODO: On Windows we should be converting UTF-8 to UTF-16 and calling GetFullPathNameW instead of GetFullPathNameA
Str GetFullPath(Str relativePath)
{
	Str result = Str_Empty_Const;
	if (relativePath.length == 0) { relativePath = StrLit("."); }
	Str relativePathNt = CopyStr(relativePath);
	FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		AssertFmt(getPathResult1 != 0, "Failed to get full path for \"%s\". GetFullPathNameA returned %d", relativePathNt.chars, getPathResult1);
		
		result = AllocStr((u64)getPathResult1-1);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		AssertFmt(getPathResult2+1 == getPathResult1, "First and second GetFullPathNameA(\"%s\") calls did not return the same value!", relativePathNt.chars);
		AssertFmt(result.chars[result.length] == '\0', "GetFullPathNameA(\"%s\") did not write a null-term character!", relativePathNt.chars);
		
		FixPathSlashes(result, '/');
	}
	#elif BUILDING_ON_UNIX
	{
		char* temporaryBuffer = (char*)malloc(PATH_MAX);
		
		char* realPathResult = realpath(
			relativePathNt.chars,
			temporaryBuffer
		);
		AssertFmt(realPathResult != nullptr || errno == ENOENT, "realpath(\"%s\") returned nullptr", relativePathNt.chars);
		
		if (realPathResult == nullptr && errno == ENOENT)
		{
			//TODO: This is a temporary solution where we just hope the relativePath is okay for the calling code.
			//      We should probably attempt resolving using parent directories one-by-one until we find one that does resolve, then maybe we do our own collapsing of folder names and ".." pieces?
			result = CopyStr(relativePath);
		}
		else
		{
			result = CopyStr(MakeStrNt(realPathResult));
		}
		
		FixPathSlashes(result, '/');
		free(temporaryBuffer);
	}
	#else
	AssertMsg(false, "GetFullPath does not support the current platform yet!");
	#endif
	
	FreeStr(&relativePathNt);
	return result;
}
//TODO: Get rid of this once GetFullPath calls GetFullPathNameW instead of GetFullPathNameA
Str16 GetFullPath16(Str16 relativePath)
{
	Str16 result = Str16_Empty_Const;
	
	#if BUILDING_ON_WINDOWS
	{
		Str16 relativePathNt = CopyStr16(relativePath);
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameW(
			relativePathNt.words, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		AssertFmt(getPathResult1 != 0, "Failed to get full path for \"%ls\". GetFullPathNameW returned %d", relativePathNt.words, getPathResult1);
		
		result = AllocStr16((u64)getPathResult1-1);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameW(
			relativePathNt.words, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.words, //lpBuffer
			nullptr //lpFilePart
		);
		AssertFmt(getPathResult2+1 == getPathResult1, "First and second GetFullPathNameW(\"%ls\") calls did not return the same value!", relativePathNt.words);
		AssertFmt(result.words[result.length] == 0x0000, "GetFullPathNameA(\"%ls\") did not write a null-term character!", relativePathNt.words);
		
	}
	#else
	AssertMsg(false, "GetFullPath16 does not support the current platform yet!");
	#endif
	
	FixPathSlashes16(result, '/');
	return result;
}

//TODO: Should we move this to pig_build_str.h since it's string manipulation only?
//TODO: Should we handle things like C:/path - //path - ///path - //?/path - C:path - COM1 - etc.?
Str CollapseParentDirPathParts(Str pathWithParentFolderParts)
{
	Array_u64 pathPartIndices = EMPTY;
	u64 numPathParts = CountPathParts(pathWithParentFolderParts);
	GrowArray_u64(&pathPartIndices, numPathParts);
	for (u64 pIndex = 0; pIndex < numPathParts; pIndex++) { AddValueArray_u64(&pathPartIndices, pIndex); }
	bool hasLeadingSlash = HasLeadingSlash(pathWithParentFolderParts);
	bool hasTrailingSlash = HasTrailingSlash(pathWithParentFolderParts);
	
	for (u64 pIndex = 1; pIndex <= numPathParts; pIndex++)
	{
		u64 partIndex = pathPartIndices.values[pIndex-1];
		Str partStr = GetPathPartAtIndex(pathWithParentFolderParts, partIndex);
		if (StrExactEquals(partStr, StrLit("..")))
		{
			u64 prevPartToRemoveIndex = UINT64_MAX;
			for (u64 pIndex2 = pIndex-1; pIndex2 > 0; pIndex2--)
			{
				u64 prevPartIndex = pathPartIndices.values[pIndex2-1];
				Str prevPartStr = GetPathPartAtIndex(pathWithParentFolderParts, prevPartIndex);
				if (!StrExactEquals(prevPartStr, StrLit("..")))
				{
					prevPartToRemoveIndex = pIndex2-1;
					// PrintLine("In \"%.*s\": Part %llu \"..\" eliminates part %llu \"%.*s\"", StrPrint(pathWithParentFolderParts), partIndex, prevPartToRemoveIndex, StrPrint(prevPartStr));
					break;
				}
			}
			
			if (prevPartToRemoveIndex < numPathParts)
			{
				RemoveItemArray_u64(&pathPartIndices, pIndex-1);
				RemoveItemArray_u64(&pathPartIndices, prevPartToRemoveIndex);
				numPathParts -= 2;
				pIndex -= 2;
			}
			// else { PrintLine_E("Failed to find a prev part to remove for part %llu", partIndex); }
		}
	}
	
	bool hasNoParts = (pathPartIndices.length == 0);
	bool needsPeriod = (hasNoParts && !hasLeadingSlash);
	bool needsTrailingSlash = (hasTrailingSlash && (!hasNoParts || needsPeriod));
	Str result = EMPTY;
	for (u64 pIndex = 0; pIndex < numPathParts; pIndex++)
	{
		u64 partIndex = pathPartIndices.values[pIndex];
		Str partStr = GetPathPartAtIndex(pathWithParentFolderParts, partIndex);
		result.length += (pIndex > 0 ? 1 : 0) + partStr.length;
	}
	result = AllocStr((hasLeadingSlash ? 1 : 0) + result.length + (needsPeriod ? 1 : 0) + (needsTrailingSlash ? 1 : 0));
	
	u64 writeIndex = 0;
	if (hasLeadingSlash)
	{
		Assert(writeIndex + 1 <= result.length);
		result.chars[writeIndex] = '/';
		writeIndex++;
	}
	for (u64 pIndex = 0; pIndex < numPathParts; pIndex++)
	{
		u64 partIndex = pathPartIndices.values[pIndex];
		Str partStr = GetPathPartAtIndex(pathWithParentFolderParts, partIndex);
		if (pIndex > 0)
		{
			Assert(writeIndex + 1 <= result.length);
			result.chars[writeIndex] = '/';
			writeIndex++;
		}
		Assert(writeIndex + partStr.length <= result.length);
		memcpy(&result.chars[writeIndex], partStr.chars, partStr.length);
		writeIndex += partStr.length;
	}
	if (needsPeriod)
	{
		Assert(writeIndex + 1 <= result.length);
		result.chars[writeIndex] = '.';
		writeIndex++;
	}
	if (needsTrailingSlash)
	{
		Assert(writeIndex + 1 <= result.length);
		result.chars[writeIndex] = '/';
		writeIndex++;
	}
	
	return result;
}

// Rather than just checking string comparison, this will resolve the paths to their actual locations and determine if the location is the same
bool ArePathsEqual(Str leftPath, Str rightPath) //TODO: Implement me!
{
	return true;
}

// This will normalize both paths and do a simple string-based comparison to see if they are the same path
// This handles:
//   * Capitlization on file systems where it doesn't matter
//   * Trailing slashes
//   * and ".." parts of paths that can be collapsed
// This does NOT handle absolute vs relative paths
bool ArePathsStrEqual(Str leftPath, Str rightPath)
{
	Str leftCollapsedPath = WithoutTrailingSlash(CollapseParentDirPathParts(leftPath));
	Str rightCollapsedPath = WithoutTrailingSlash(CollapseParentDirPathParts(rightPath));
	#if IS_OS_FILESYSTEM_CASE_SENSITIVE
	return StrExactEquals(leftCollapsedPath, rightCollapsedPath);
	#else
	return StrAnyCaseEquals(leftCollapsedPath, rightCollapsedPath);
	#endif
}

bool IsPathInsideFolder(Str parentDirPath, Str innerPath)
{
	Str parentDirFullPath = GetFullPath(parentDirPath);
	Str innerFullPath = GetFullPath(innerPath);
	#if IS_OS_FILESYSTEM_CASE_SENSITIVE
	if (StrExactStartsWith(innerPath, parentDirPath))
	#else
	if (StrAnyCaseStartsWith(innerPath, parentDirPath))
	#endif
	{
		return true;
	}
	else { return false; }
}

bool DoesFileOrFolderExist(Str path, bool* isFolderOut)
{
	bool result = false;
	Str fullPathNt = GetFullPath(path);
	FixPathSlashes(fullPathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		DWORD fileType = GetFileAttributesA(fullPathNt.chars);
		if (fileType != INVALID_FILE_ATTRIBUTES)
		{
			result = true;
			if (isFolderOut != nullptr) { *isFolderOut = IsFlagSet(fileType, FILE_ATTRIBUTE_DIRECTORY); }
		}
		else { result = false; }
	}
	#elif BUILDING_ON_UNIX
	{
		struct stat statStruct = EMPTY;
		int statResult = stat(fullPathNt.chars, &statStruct);
		if (statResult == 0)
		{
			result = true;
			if (isFolderOut != nullptr) { *isFolderOut = IsFlagSet(statStruct.st_mode, S_IFDIR); }
		}
		else { result = false; }
	}
	#else
	AssertMsg(false, "DoesFileOrFolderExist does not support the current platform yet!");
	#endif
	
	FreeStr(&fullPathNt);
	return result;
}
bool DoesPathExist(Str path)
{
	return DoesFileOrFolderExist(path, nullptr);
}
bool DoesFileExist(Str path)
{
	bool isFolder = false;
	bool doesExist = DoesFileOrFolderExist(path, &isFolder);
	return (doesExist && !isFolder);
}
bool DoesFolderExist(Str path)
{
	bool isFolder = false;
	bool doesExist = DoesFileOrFolderExist(path, &isFolder);
	return (doesExist && isFolder);
}
void AssertFileExists(Str filePath, Str errorMessage)
{
	AssertFmt(DoesFileExist(filePath),
		"Missing file \"%.*s\"!%s%.*s",
		StrPrint(filePath), IsEmptyStr(errorMessage) ? "" : "\n",
		StrPrint(errorMessage)
	);
}

// +--------------------------------------------------------------+
// |                        Create Folder                         |
// +--------------------------------------------------------------+
//NOTE: We cannot use the name "CreateFolder" because it is taken by the Windows API
// Returns true if the folder did not exist and was actually created
bool MakeFolderEx(Str folderPath, bool createParentFoldersIfNeeded) //TODO: Implement me!
{
	return true;
}
bool MakeFolder(Str folderPath) { return MakeFolderEx(folderPath, /*createParentFoldersIfNeeded=*/true); }

// +--------------------------------------------------------------+
// |                    Delete File or Folder                     |
// +--------------------------------------------------------------+
bool TryRemoveFile(Str path) //TODO: Implement me!
{
	return true;
}

//NOTE: We cannot use the name "DeleteFile" because it is taken by the Windows API
void RemoveFileWithErrorMsgStr(Str path, Str errorMessage) //TODO: Implement me!
{
}
#define RemoveFileWithErrorMsg(path, errorMsgFmt, ...) RemoveFileWithErrorMsgStr((path), FormatStr(errorMsgFmt, ##__VA_ARGS__))
void RemoveFile(Str path)                            { RemoveFileWithErrorMsgStr(path, FormatStr("Failed to delete file \"%.*s\"", StrPrint(path))); }

bool TryRemoveFolder(Str path, bool removeContents) //TODO: Implement me!
{
	return true;
}

void RemoveFolderWithErrorMsgStr(Str path, bool removeContents, Str errorMessage) //TODO: Implement me!
{
}
#define RemoveFolderWithErrorMsg(path, removeContents, errorMsgFmt, ...) RemoveFolderWithErrorMsgStr((path), (removeContents), FormatStr(errorMsgFmt, ##__VA_ARGS__))
void RemoveFolder(Str path, bool removeContents)                       { RemoveFolderWithErrorMsgStr( path,   removeContents,  FormatStr("Failed to delete folder \"%.*s\"", StrPrint(path))); }

bool TryRemoveFileOrFolder(Str path, bool removeFolderContents) //TODO: Implement me!
{
	return true;
}

void RemoveFolderOrFolderWithErrorMsgStr(Str path, bool removeContents, Str errorMessage)
{
	if (DoesFolderExist(path)) { RemoveFolderWithErrorMsgStr(path, removeContents, errorMessage); }
	else if (DoesFileExist(path)) { RemoveFileWithErrorMsgStr(path, errorMessage); }
	else
	{
		if (!IsEmptyStr(errorMessage)) { AssertFmt(DoesFileOrFolderExist(path, nullptr), "No file or folder exists at \"%.*s\" to delete!\n%.*s", StrPrint(path), StrPrint(errorMessage)); }
		else { AssertFmt(DoesFileOrFolderExist(path, nullptr), "No file or folder exists at \"%.*s\" to delete!", StrPrint(path)); }
	}
}
#define RemoveFolderOrFolderWithErrorMsg(path, removeContents, errorMsgFmt, ...) RemoveFolderOrFolderWithErrorMsgStr((path), (removeContents), FormatStr(errorMsgFmt, ##__VA_ARGS__))
void RemoveFolderOrFolder(Str path, bool removeContents)                       { RemoveFolderOrFolderWithErrorMsgStr( path,   removeContents,  FormatStr("Failed to delete file/folder \"%.*s\"", StrPrint(path))); }

// +--------------------------------------------------------------+
// |                          Read File                           |
// +--------------------------------------------------------------+
Str TryReadEntireFile(Str path, Str defaultContents, bool convertNewLines) //TODO: Implement me!
{
	return Str_Empty;
}
#define TryReadEntireBinFileWithDefault(path, defaultContents)  TryReadEntireFile((path), (defaultContents), /*convertNewLines=*/false)
#define TryReadEntireTextFileWithDefault(path, defaultContents) TryReadEntireFile((path), (defaultContents), /*convertNewLines=*/true)
#define TryReadEntireBinFile(path)                              TryReadEntireFile((path), Str_Empty,         /*convertNewLines=*/false)
#define TryReadEntireTextFile(path)                             TryReadEntireFile((path), Str_Empty,         /*convertNewLines=*/true)

// This Asserts on failure (either the file doesn't exist, or it couldn't be opened for reading)
Str ReadEntireFile(Str path, bool convertNewLines, Str errorMessage) //TODO: Implement me!
{
	return Str_Empty;
}
#define ReadEntireBinFileWithErrorMsg(path, errorMsg, ...)   ReadEntireFile((path), /*convertNewLines=*/false, FormatStr(errorMsg, ##__VA_ARGS__))
#define ReadEntireTextFileWithErrorMsg(path, errorMsg, ...)  ReadEntireFile((path), /*convertNewLines=*/true,  FormatStr(errorMsg, ##__VA_ARGS__))
Str ReadEntireBinFile(Str path)                     { return ReadEntireFile( path,  /*convertNewLines=*/false, FormatStr("Failed to read file \"%.*s\"", StrPrint(path))); }
Str ReadEntireTextFile(Str path)                    { return ReadEntireFile( path,  /*convertNewLines=*/true,  FormatStr("Failed to read file \"%.*s\"", StrPrint(path))); }

// +--------------------------------------------------------------+
// |                          Write File                          |
// +--------------------------------------------------------------+
bool TryWriteEntireFile(Str path, bool overwriteExisting, Str fileContents, bool convertNewLines) //TODO: Implement me!
{
	return true;
}
#define TryWriteEntireBinFile(path, fileContents)  TryWriteEntireFile((path), /*overwriteExisting=*/true, (fileContents), /*convertNewLines=*/false)
#define TryWriteEntireTextFile(path, fileContents) TryWriteEntireFile((path), /*overwriteExisting=*/true, (fileContents), /*convertNewLines=*/true)
bool TryWriteEntireFileFmt(Str path, const char* fileContentsFormatStr, ...)
{
	FormatVaListStr(fileContentsFormatStr, args, fileContents);
	return TryWriteEntireFile(path,
		true, //overwriteExisting
		fileContents,
		true //convertNewLines
	);
}

// This Asserts on failure (can't overwrite existing file, parent folder doesn't exist, etc.)
void WriteEntireFile(Str path, bool overwriteExisting, Str fileContents, bool convertNewLines, Str errorMessage) //TODO: Implement me!
{
}
#define WriteEntireBinFileWithErrorMsg(path, overwriteExisting, fileContents, errorMsg, ...)  WriteEntireFile((path), (overwriteExisting), (fileContents), /*convertNewLines=*/false, FormatStr(errorMsg, ##__VA_ARGS__))
#define WriteEntireTextFileWithErrorMsg(path, overwriteExisting, fileContents, errorMsg, ...) WriteEntireFile((path), (overwriteExisting), (fileContents), /*convertNewLines=*/true,  FormatStr(errorMsg, ##__VA_ARGS__))
void WriteEntireBinFile(Str path, bool overwriteExisting, Str fileContents)                 { WriteEntireFile( path,   overwriteExisting,   fileContents,  /*convertNewLines=*/false, FormatStr("Failed to write %llu bytes to file \"%.*s\"", fileContents.length, StrPrint(path))); }
void WriteEntireTextFile(Str path, bool overwriteExisting, Str fileContents)                { WriteEntireFile( path,   overwriteExisting,   fileContents,  /*convertNewLines=*/true,  FormatStr("Failed to write %llu bytes to file \"%.*s\"", fileContents.length, StrPrint(path))); }
void WriteEntireFileFmt(Str path, bool overwriteExisting, const char* fileContentsFormatStr, ...)
{
	FormatVaListStr(fileContentsFormatStr, args, fileContents);
	WriteEntireFile(path,
		overwriteExisting,
		fileContents,
		true, //convertNewLines
		FormatStr("Failed to write %llu bytes to file \"%.*s\"", fileContents.length, StrPrint(path))
	);
}

// +--------------------------------------------------------------+
// |                         Append File                          |
// +--------------------------------------------------------------+
bool TryAppendToFile(Str path, Str newContentsToAppend, bool createIfNeeded, bool convertNewLines) //TODO: Implement me!
{
	return true;
}
#define TryAppendToExistingBinFile(path, newContentsToAppend)       TryAppendToFile((path), (newContentsToAppend),                          /*createIfNeeded=*/false, /*convertNewLines=*/false)
#define TryAppendToExistingTextFile(path, newContentsToAppend)      TryAppendToFile((path), (newContentsToAppend),                          /*createIfNeeded=*/false, /*convertNewLines=*/true)
#define TryAppendToExistingFileFmt(path, newContentsFormatStr, ...) TryAppendToFile((path), FormatStr(newContentsFormatStr, ##__VA_ARGS__), /*createIfNeeded=*/false, /*convertNewLines=*/true)
#define TryAppendToBinFile(path, newContentsToAppend)               TryAppendToFile((path), (newContentsToAppend),                          /*createIfNeeded=*/true,  /*convertNewLines=*/false)
#define TryAppendToTextFile(path, newContentsToAppend)              TryAppendToFile((path), (newContentsToAppend),                          /*createIfNeeded=*/true,  /*convertNewLines=*/true)
#define TryAppendToFileFmt(path, newContentsFormatStr, ...)         TryAppendToFile((path), FormatStr(newContentsFormatStr, ##__VA_ARGS__), /*createIfNeeded=*/true,  /*convertNewLines=*/true)

// This Asserts on failure (can't open existing file for writing, parent folder doesn't exist, etc.)
void AppendToFile(Str path, Str newContentsToAppend, bool createIfNeeded, bool convertNewLines, Str errorMessage) //TODO: Implement me!
{
}
#define AppendToExistingBinFileWithErrorMsg(path, newContentsToAppend, errorMsg, ...)  AppendToFile((path), (newContentsToAppend), /*createIfNeeded=*/false, /*convertNewLines=*/false, FormatStr(errorMsg, ##__VA_ARGS__))
#define AppendToExistingTextFileWithErrorMsg(path, newContentsToAppend, errorMsg, ...) AppendToFile((path), (newContentsToAppend), /*createIfNeeded=*/false, /*convertNewLines=*/true,  FormatStr(errorMsg, ##__VA_ARGS__))
void AppendToExistingBinFile(Str path, Str newContentsToAppend)                      { AppendToFile( path,   newContentsToAppend,  /*createIfNeeded=*/false, /*convertNewLines=*/false, FormatStr("Failed to append %llu bytes to file \"%.*s\"", newContentsToAppend.length, StrPrint(path))); }
void AppendToExistingTextFile(Str path, Str newContentsToAppend)                     { AppendToFile( path,   newContentsToAppend,  /*createIfNeeded=*/false, /*convertNewLines=*/true,  FormatStr("Failed to append %llu bytes to file \"%.*s\"", newContentsToAppend.length, StrPrint(path))); }
void AppendToExistingFileFmt(Str path, const char* newContentsFormatStr, ...)
{
	FormatVaListStr(newContentsFormatStr, args, contentsToAppend);
	AppendToFile(path,
		contentsToAppend,
		false, //createIfNeeded
		true, //convertNewLines
		FormatStr("Failed to append %llu bytes to file \"%.*s\"", contentsToAppend.length, StrPrint(path))
	);
}
#define AppendToBinFileWithErrorMsg(path, newContentsToAppend, errorMsg, ...)  AppendToFile((path), (newContentsToAppend), /*createIfNeeded=*/true, /*convertNewLines=*/false, FormatStr(errorMsg, ##__VA_ARGS__))
#define AppendToTextFileWithErrorMsg(path, newContentsToAppend, errorMsg, ...) AppendToFile((path), (newContentsToAppend), /*createIfNeeded=*/true, /*convertNewLines=*/true,  FormatStr(errorMsg, ##__VA_ARGS__))
void AppendToBinFile(Str path, Str newContentsToAppend)                      { AppendToFile( path,   newContentsToAppend,  /*createIfNeeded=*/false, /*convertNewLines=*/false, FormatStr("Failed to append %llu bytes to file \"%.*s\"", newContentsToAppend.length, StrPrint(path))); }
void AppendToTextFile(Str path, Str newContentsToAppend)                     { AppendToFile( path,   newContentsToAppend,  /*createIfNeeded=*/false, /*convertNewLines=*/true,  FormatStr("Failed to append %llu bytes to file \"%.*s\"", newContentsToAppend.length, StrPrint(path))); }
void AppendToFileFmt(Str path, const char* newContentsFormatStr, ...)
{
	FormatVaListStr(newContentsFormatStr, args, contentsToAppend);
	AppendToFile(path,
		contentsToAppend,
		true, //createIfNeeded
		true, //convertNewLines
		FormatStr("Failed to append %llu bytes to file \"%.*s\"", contentsToAppend.length, StrPrint(path))
	);
}

// +--------------------------------------------------------------+
// |                     Copy File or Folder                      |
// +--------------------------------------------------------------+
bool TryCopyFileToPath(Str srcFilePath, Str destFilePath) //TODO: Implement me!
{
	return true;
}
//The source file name is used to make a destFilePath from destFolderPath
bool TryCopyFileToFolder(Str srcFilePath, Str destFolderPath)
{
	Str fileName = GetFileNamePart(srcFilePath, /*includeExtension=*/true);
	Str destFilePath = JoinPaths(destFolderPath, fileName);
	return TryCopyFileToPath(srcFilePath, destFilePath);
}

void CopyFileToPathWithErrorMsgStr(Str srcFilePath, Str destFilePath, Str errorMessage) //TODO: Implement me!
{
}
//The source file name is used to make a destFilePath from destFolderPath
void CopyFileToFolderWithErrorMsgStr(Str srcFilePath, Str destFolderPath, Str errorMessage)
{
	Str fileName = GetFileNamePart(srcFilePath, /*includeExtension=*/true);
	Str destFilePath = JoinPaths(destFolderPath, fileName);
	CopyFileToPathWithErrorMsgStr(srcFilePath, destFilePath, errorMessage);
}
#define CopyFileToPathWithErrorMsg(srcFilePath, destFilePath, errorMsgFmt, ...)     CopyFileToPathWithErrorMsgStr((srcFilePath), (destFilePath), FormatStr(errorMsg, ##__VA_ARGS__))
#define CopyFileToFolderWithErrorMsg(srcFilePath, destFolderPath, errorMsgFmt, ...) CopyFileToPathWithErrorMsgStr((srcFilePath), (destFolderPath), FormatStr(errorMsg, ##__VA_ARGS__))
void CopyFileToPath(Str srcFilePath, Str destFilePath)                     { CopyFileToPathWithErrorMsgStr(  srcFilePath, destFilePath,   FormatStr("Failed to copy file \"%.*s\" to \"%.*s\"",        StrPrint(srcFilePath), StrPrint(destFilePath)));   }
void CopyFileToFolder(Str srcFilePath, Str destFolderPath)                 { CopyFileToFolderWithErrorMsgStr(srcFilePath, destFolderPath, FormatStr("Failed to copy file \"%.*s\" to folder \"%.*s\"", StrPrint(srcFilePath), StrPrint(destFolderPath))); }

#endif //USE_NEW_API

// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// |                                               Old API                                                |
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
// +------------------------------------------------------------------------------------------------------+
typedef struct FileIter FileIter;
struct FileIter
{
	bool finished;
	Str folderPathNt;
	u64 index;
	u64 nextIndex;
	
	#if BUILDING_ON_WINDOWS
	Str folderPathWithWildcard;
	WIN32_FIND_DATAA findData;
	HANDLE handle;
	#elif BUILDING_ON_LINUX || BUILDING_ON_OSX
	DIR* dirHandle;
	#endif
};

#define RECURSIVE_DIR_WALK_CALLBACK_DEF(functionName) bool functionName(Str path, bool isFolder, void* contextPntr)
typedef RECURSIVE_DIR_WALK_CALLBACK_DEF(RecursiveDirWalkCallback_f);

// +--------------------------------------------------------------+
// |                        File Functions                        |
// +--------------------------------------------------------------+
#if !USE_NEW_API
// Result is always null-terminated
// TODO: On linux this will not work properly for paths to folders that don't exist yet
Str GetFullPath(Str relativePath)
{
	Str result = Str_Empty_Const;
	if (relativePath.length == 0) { relativePath = StrLit("."); }
	
	#if BUILDING_ON_WINDOWS
	{
		Str relativePathNt = CopyStr(relativePath);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		AssertFmt(getPathResult1 != 0, "Failed to get full path for \"%s\". GetFullPathNameA returned %d", relativePathNt.chars, getPathResult1);
		
		result = AllocStr((u64)getPathResult1-1);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		Assert(getPathResult2+1 == getPathResult1);
		Assert(result.chars[result.length] == '\0');
		
		free(relativePathNt.chars);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		Str relativePathNt = CopyStr(relativePath);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		char* temporaryBuffer = (char*)malloc(PATH_MAX);
		char* realPathResult = realpath(relativePathNt.chars, temporaryBuffer);
		AssertFmt(realPathResult != nullptr || errno == ENOENT, "realpath returned nullptr for \"%s\"", relativePathNt.chars);
		if (realPathResult == nullptr && errno == ENOENT)
		{
			//TODO: This is a temporary solution where we just hope the relativePath is okay for the calling code.]
			//      We should probably attempt resolving using parent directories one-by-one until we find one that does resolve, then maybe we do our own collapsing of folder names and ".." pieces?
			result = CopyStr(relativePath);
		}
		else
		{
			result = CopyStr(MakeStrNt(realPathResult));
		}
		
		free(temporaryBuffer);
		free(relativePathNt.chars);
	}
	#else
	AssertMsg(false, "GetFullPath does not support the current platform yet!");
	#endif
		
	if (!IsEmptyStr(result)) { FixPathSlashes(result, '/'); }
	return result;
}
#endif //!USE_NEW_API

#if !USE_NEW_API
bool DoesFileOrFolderExist(Str path, bool* isFolderOut)
{
	bool result = false;
	Str fullPathNt = GetFullPath(path);
	FixPathSlashes(fullPathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		BOOL fileExistsResult = PathFileExistsA(fullPathNt.chars);
		if (fileExistsResult == TRUE)
		{
			if (isFolderOut != nullptr)
			{
				DWORD fileType = GetFileAttributesA(fullPathNt.chars);
				if (fileType != INVALID_FILE_ATTRIBUTES)
				{
					*isFolderOut = IsFlagSet(fileType, FILE_ATTRIBUTE_DIRECTORY);
					result = true;
				}
				else { result = false; }
			}
			else { result = true; }
		}
		else
		{
			result = false;
		}
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX || BUILDING_ON_ANDROID)
	{
		int accessResult = access(fullPathNt.chars, F_OK);
		result = (accessResult == 0);
		
		if (isFolderOut != nullptr && result)
		{
			struct stat statStruct = EMPTY;
			int statResult = stat(fullPathNt.chars, &statStruct);
			if (statResult == 0)
			{
				*isFolderOut = IsFlagSet(statStruct.st_mode, S_IFDIR);
			}
			else
			{
				PrintLine_E("stat(\"%s\") call failed! Can't determine if that path is a folder or file!", fullPathNt.chars);
				*isFolderOut = false;
			}
		}
		
	}
	#else
	AssertMsg(false, "DoesFileOrFolderExist does not support the current platform yet!");
	#endif
	
	FreeStr(&fullPathNt);
	return result;
}
bool DoesPathExist(Str path)
{
	return DoesFileOrFolderExist(path, nullptr);
}
bool DoesFileExist(Str path)
{
	bool isFolder = false;
	bool doesExist = DoesFileOrFolderExist(path, &isFolder);
	return (doesExist && !isFolder);
}
bool DoesFolderExist(Str path)
{
	bool isFolder = false;
	bool doesExist = DoesFileOrFolderExist(path, &isFolder);
	return (doesExist && isFolder);
}
#endif //!USE_NEW_API
void AssertFileExist(Str filePath, bool wasCreatedByBuild)
{
	if (!DoesFileExist(filePath))
	{
		PrintLine_E("Missing file \"%.*s\" %s!", StrPrint(filePath), wasCreatedByBuild ? "was not created" : "was not found");
		exit(6);
	}
}

void MyCreateFolder(Str path, bool createParentFoldersIfNeeded)
{
	u64 numPathParts = CountPathParts(path);
	if (createParentFoldersIfNeeded && numPathParts > 1)
	{
		for (u64 pIndex = 0; pIndex < numPathParts; pIndex++)
		{
			Str pathPart = GetPathPartAtIndex(path, pIndex);
			NotEmptyStr(pathPart);
			Assert(IsSizedPntrWithin(path.chars, path.length, pathPart.chars, pathPart.length));
			u64 partEndIndex = (u64)((pathPart.chars + pathPart.length) - path.chars);
			Str partialPath = StrSlice(path, 0, partEndIndex);
			if (!DoesFolderExist(partialPath))
			{
				MyCreateFolder(partialPath, false);
			}
		}
	}
	
	#if BUILDING_ON_WINDOWS
	{
		if (!DoesFolderExist(path))
		{
			Str pathNt = CopyStr(path);
			BOOL createResult = CreateDirectoryA(
				pathNt.chars, //lpPathName
				NULL //lpSecurityAttributes
			);
			if (createResult != TRUE)
			{
				PrintLine_E("Failed to create folder in MyCreateFolder: \"%.*s\"", StrPrint(path));
				Assert(createResult == TRUE);
			}
		}
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		if (!DoesFolderExist(path))
		{
			Str pathNt = CopyStr(path);
			int mkdirResult = mkdir(pathNt.chars, FOLDER_PERMISSIONS);
			if (mkdirResult != 0)
			{
				PrintLine_E("Failed to create folder in MyCreateFolder: \"%.*s\"", StrPrint(path));
				Assert(mkdirResult == 0);
			}
		}
	}
	#else
	#error MyCreateFolder does not support the current platform yet!
	#endif
}

bool TryReadFile(Str filePath, Str* contentsOut)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	//NOTE: We open the file in binary mode because otherwise the result from jumping to SEEK_END to
	//      check the file size does not match the result of fread because the new-lines get converted
	//      in the fread NOT in the result from ftell
	FILE* fileHandle = fopen(filePathNt.chars, "rb");
	free(filePathNt.chars);
	if (fileHandle == nullptr)
	{
		// fprintf(stderr, "Couldn't open file at \"%.*s\"!\n", StrPrint(filePath));
		return false;
	}
	
	int seekResult1 = fseek(fileHandle, 0, SEEK_END); Assert(seekResult1 == 0);
	long fileSize = ftell(fileHandle); Assert(fileSize >= 0); Assert(fileSize <= INT_MAX);
	int seekResult2 = fseek(fileHandle, 0, SEEK_SET); Assert(seekResult2 == 0);
	
	*contentsOut = AllocStr((u64)fileSize);
	
	int readResult = fread(
		contentsOut->chars,
		1,
		fileSize,
		fileHandle
	);
	contentsOut->chars[fileSize] = '\0';
	if (readResult != (int)fileSize)
	{
		fprintf(stderr, "Failed to read all %d byte%s from file! Only read %d byte%s\n",
			(int)fileSize, (fileSize == 1 ? "" : "s"),
			readResult, (readResult == 1 ? "" : "s")
		);
		free(contentsOut->chars);
		fclose(fileHandle);
		return false;
	}
	
	fclose(fileHandle);
	return true;
}
#if !USE_NEW_API
//NOTE: We can't name this "ReadFile" because it conflicts with a Windows function
Str ReadEntireFile(Str filePath)
{
	Str result = Str_Empty_Const;
	bool readSuccess = TryReadFile(filePath, &result);
	if (!readSuccess) { exit(3); }
	return result;
}
#endif //!USE_NEW_API

void CreateAndWriteFile(Str filePath, Str contents, bool convertNewLines)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contents = StrReplace(contents, StrLit("\n"), StrLit("\r\n")); }
		HANDLE fileHandle = CreateFileA(
			filePathNt.chars,      //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			CREATE_ALWAYS,         //Always overwrite
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		AssertFmt(fileHandle != INVALID_HANDLE_VALUE, "Failed to CreateAndWriteFile(\"%.*s\", char[%llu], %s)", StrPrint(filePath), contents.length, convertNewLines ? "true" : "false");
		if (contents.length > 0)
		{
			DWORD bytesWritten = 0;
			BOOL writeResult = WriteFile(
				fileHandle, //hFile
				contents.chars, //lpBuffer
				(DWORD)contents.length, //nNumberOfBytesToWrite
				&bytesWritten, //lpNumberOfBytesWritten
				0 //lpOverlapped
			);
			Assert(writeResult == TRUE);
			Assert((u64)bytesWritten == contents.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contents.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		// PrintLine("Writing %llu bytes to \"%.*s\"", contents.length, StrPrint(filePath));
		FILE* fileHandle = fopen(filePathNt.chars, "w");
		NotNull(fileHandle);
		if (contents.length > 0)
		{
			size_t writeResult = fwrite(
				contents.pntr, //ptr
				1, //size
				contents.length, //count
				fileHandle //stream
			);
			Assert(writeResult >= 0);
			Assert((u64)writeResult == contents.length);
		}
		fclose(fileHandle);
	}
	#else
	AssertMsg(false, "CreateAndWriteFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}

#if !USE_NEW_API
void AppendToFile(Str filePath, Str contentsToAppend, bool convertNewLines)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contentsToAppend = StrReplace(contentsToAppend, StrLit("\n"), StrLit("\r\n")); }
		HANDLE fileHandle = CreateFileA(
			filePathNt.chars,      //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			OPEN_ALWAYS,           //Open if it exists, or create a new file if not
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD errorCode = GetLastError();
			AssertFmt(fileHandle != INVALID_HANDLE_VALUE, "CreateFileA error: %d When creating \"%s\"", errorCode, filePathNt.chars);
		}
		
		DWORD moveResult = SetFilePointer(
			fileHandle, //hFile
			0, //lDistanceToMove,
			NULL, //lDistanceToMoveHigh
			FILE_END
		);
		Assert(moveResult != INVALID_SET_FILE_POINTER);
		if (contentsToAppend.length > 0)
		{
			DWORD bytesWritten = 0;
			BOOL writeResult = WriteFile(
				fileHandle, //hFile
				contentsToAppend.chars, //lpBuffer
				(DWORD)contentsToAppend.length, //nNumberOfBytesToWrite
				&bytesWritten, //lpNumberOfBytesWritten
				0 //lpOverlapped
			);
			Assert(writeResult == TRUE);
			Assert((u64)bytesWritten == contentsToAppend.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contentsToAppend.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		FILE* fileHandle = fopen(filePathNt.chars, "a");
		NotNull(fileHandle);
		if (contentsToAppend.length > 0)
		{
			size_t writeResult = fwrite(
				contentsToAppend.pntr, //ptr
				1, //size
				contentsToAppend.length, //count
				fileHandle //stream
			);
			Assert(writeResult >= 0);
			Assert((u64)writeResult == contentsToAppend.length);
		}
		fclose(fileHandle);
	}
	#else
	AssertMsg(false, "AppendToFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}
void AppendPrintToFile(Str filePath, const char* formatString, ...)
{
	FormatVaListStr(formatString, args, printedStr);
	AppendToFile(filePath, printedStr, true);
	FreeStr(&printedStr);
}
#endif //!USE_NEW_API

#if !USE_NEW_API
//TODO: We can probably just use `remove` from the C standard library
void RemoveFile(Str filePath)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		BOOL deleteResult = DeleteFileA(filePathNt.chars);
		if (deleteResult == 0)
		{
			DWORD errorCode = GetLastError();
			AssertFmt(errorCode == ERROR_FILE_NOT_FOUND, "DeleteFileA Error: %d", errorCode);
		}
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		int unlinkResult = unlink(filePathNt.chars);
		AssertFmt(unlinkResult == 0, "Failed to delete file with unlink(\"%s\")", filePathNt.chars);
	}
	#else
	AssertMsg(false, "RemoveFile does not support the current platform yet!");
	#endif
}
bool TryRemoveFile(Str filePath)
{
	if (DoesFileExist(filePath)) { RemoveFile(filePath); return true; }
	else { return false; }
}
#endif //!USE_NEW_API

#if !USE_NEW_API
void CopyFileToPath(Str filePath, Str newFilePath)
{
	Str fileContents = Str_Empty_Const;
	bool readSuccess = TryReadFile(filePath, &fileContents);
	if (!readSuccess)
	{
		PrintLine_E("Failed to open/read file for copying at \"%.*s\"", StrPrint(filePath));
		AssertMsg(readSuccess, "Failed to read file contents for copying");
	}
	CreateAndWriteFile(newFilePath, fileContents, false);
	free(fileContents.chars);
	#if (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		Str filePathNt = CopyStr(filePath);
		struct stat oldFileStats = EMPTY;
		int statResult = stat(filePathNt.chars, &oldFileStats);
		Assert(statResult == 0);
		free(filePathNt.chars);
		
		Str newFilePathNt = CopyStr(newFilePath);
		// PrintLine("Copying permissions %d of file \"%.*s\" to \"%.*s\"", oldFileStats.st_mode, StrPrint(filePath), StrPrint(newFilePath));
		int modResult = chmod(newFilePathNt.chars, oldFileStats.st_mode);
		Assert(modResult == 0);
		free(newFilePathNt.chars);
	}
	#endif
}
void CopyFileToFolder(Str filePath, Str folderPath)
{
	Str newPath = JoinPaths(folderPath, GetFileNamePart(filePath, true));
	CopyFileToPath(filePath, newPath);
	free(newPath.chars);
}
#endif //!USE_NEW_API

FileIter StartFileIter(Str folderPath)
{
	FileIter result = EMPTY;
	result.index = UINT64_MAX;
	result.nextIndex = 0;
	result.finished = false;
	bool needsTrailingSlash = (folderPath.length == 0 || (folderPath.chars[folderPath.length-1] != '\\' && folderPath.chars[folderPath.length-1] != '/'));
	result.folderPathNt = AllocStr(folderPath.length + (needsTrailingSlash ? 1 : 0));
	memcpy(result.folderPathNt.chars, folderPath.chars, folderPath.length);
	if (needsTrailingSlash) { result.folderPathNt.chars[folderPath.length] = PATH_SEP_CHAR; }
	result.folderPathNt.chars[result.folderPathNt.length] = '\0';
	
	#if BUILDING_ON_WINDOWS
	{
		// ChangePathSlashesTo(result.folderPath, '\\'); //TODO: Should we do this?
		//NOTE: File iteration in windows requires that we have a slash on the end and a * wildcard character
		result.folderPathWithWildcard = JoinStrings2(result.folderPathNt, StrLit("*"));
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		//nothing to do
	}
	#else
	AssertMsg(false, "StartFileIter does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return result;
}

bool StepFileIter(FileIter* fileIter, Str* pathOut, bool* isFolderOut)
{
	if (fileIter->finished) { return false; }
	
	#if BUILDING_ON_WINDOWS
	{
		while (true)
		{
			bool firstIteration = (fileIter->index == UINT64_MAX);
			fileIter->index = fileIter->nextIndex;
			if (firstIteration)
			{
				fileIter->handle = FindFirstFileA(fileIter->folderPathWithWildcard.chars, &fileIter->findData);
				if (fileIter->handle == INVALID_HANDLE_VALUE)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					free(fileIter->folderPathWithWildcard.chars); fileIter->folderPathWithWildcard.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			else
			{
				BOOL findNextResult = FindNextFileA(fileIter->handle, &fileIter->findData);
				if (findNextResult == 0)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					free(fileIter->folderPathWithWildcard.chars); fileIter->folderPathWithWildcard.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			
			Str fileName = MakeStrNt(fileIter->findData.cFileName);
			
			//ignore current and parent folder entries
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit("..")))
			{
				continue;
			}
			
			bool isFolder = (fileIter->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			if (pathOut != nullptr)
			{
				*pathOut = JoinStrings2(fileIter->folderPathNt, fileName);
				// FixPathSlashes(*pathOut); //TODO: Should we do this?
			}
			if (isFolderOut != nullptr) { *isFolderOut = isFolder; }
			fileIter->nextIndex = fileIter->index+1;
			return true;
		}
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		while (true)
		{
			bool firstIteration = (fileIter->index == UINT64_MAX);
			fileIter->index = fileIter->nextIndex;
			if (firstIteration)
			{
				fileIter->dirHandle = opendir(fileIter->folderPathNt.chars);
				if (fileIter->dirHandle == nullptr)
				{
					free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
					fileIter->finished = true;
					return false;
				}
			}
			
			struct dirent* entry = readdir(fileIter->dirHandle);
			if (entry == nullptr)
			{
				free(fileIter->folderPathNt.chars); fileIter->folderPathNt.chars = nullptr;
				fileIter->finished = true;
				return false;
			}
			
			Str fileName = MakeStrNt(entry->d_name);
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit(".."))) { continue; } //ignore current and parent folder entries
			
			Str fullPath = JoinStrings2(fileIter->folderPathNt, fileName);
			if (isFolderOut != nullptr)
			{
				struct stat statStruct = EMPTY;
				int statResult = stat(fullPath.chars, &statStruct);
				if (statResult == 0)
				{
					if ((statStruct.st_mode & S_IFDIR) != 0)
					{
						if (isFolderOut != nullptr) { *isFolderOut = true; }
					}
					else if ((statStruct.st_mode & S_IFREG) != 0)
					{
						if (isFolderOut != nullptr) { *isFolderOut = false; }
					}
					else
					{
						PrintLine_E("Unknown file type for \"%.*s\"", StrPrint(fullPath));
						continue;
					}
				}
			}
			
			if (pathOut != nullptr) { *pathOut = fullPath; }
			fileIter->nextIndex = fileIter->index+1;
			return true;
		}
	}
	#else
	AssertMsg(false, "StepFileIter does not support the current platform yet!");
	fileIter->finished = true;
	#endif
	
	return false;
}

void RecursiveDirWalk(Str rootDir, RecursiveDirWalkCallback_f* callback, void* contextPntr)
{
	FileIter iter = StartFileIter(rootDir);
	Str path = Str_Empty_Const;
	bool isFolder = false;
	while (StepFileIter(&iter, &path, &isFolder))
	{
		bool callbackResult = callback(path, isFolder, contextPntr);
		if (isFolder && callbackResult)
		{
			RecursiveDirWalk(path, callback, contextPntr);
		}
	}
}
void RecursiveDirWalkBottomUp(Str rootDir, RecursiveDirWalkCallback_f* callback, void* contextPntr)
{
	FileIter iter = StartFileIter(rootDir);
	Str path = Str_Empty_Const;
	bool isFolder = false;
	while (StepFileIter(&iter, &path, &isFolder))
	{
		if (isFolder)
		{
			RecursiveDirWalkBottomUp(path, callback, contextPntr);
		}
		bool callbackResult = callback(path, isFolder, contextPntr);
		//NOTE: callbackResult is ignored in BottomUp version!
	}
}

RECURSIVE_DIR_WALK_CALLBACK_DEF(MyRemoveDirectory_RecursiveCallback); //implemented below

void MyRemoveDirectory(Str folderPath, bool recursive)
{
	if (!DoesFolderExist(folderPath)) { return; }
	
	if (!recursive)
	{
		//TODO: Should we use RemoveDirectoryA on Windows?
		Str folderPathNt = CopyStr(folderPath);
		FixPathSlashes(folderPathNt, PATH_SEP_CHAR);
		int rmResult = rmdir(folderPathNt.chars);
		if (rmResult != 0) { AssertFmt(rmResult == 0, "rmdir(\"%s\") failed: errno=%d", folderPathNt.chars, errno); }
	}
	else
	{
		RecursiveDirWalkBottomUp(folderPath, MyRemoveDirectory_RecursiveCallback, nullptr);
		MyRemoveDirectory(folderPath, false);
	}
}

// | MyRemoveDirectory_RecursiveCallback |
// bool MyRemoveDirectory_RecursiveCallback(Str path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(MyRemoveDirectory_RecursiveCallback)
{
	if (isFolder) { MyRemoveDirectory(path, false); }
	else { RemoveFile(path); }
	return true;
}

void CopyFolderTo(Str folderPath, Str destPath, bool copySubFolders)
{
	Str fullFolderPath = GetFullPath(folderPath);
	FixPathSlashes(fullFolderPath, PATH_SEP_CHAR);
	
	StrArray pathsToWalk = EMPTY;
	AddStr(&pathsToWalk, fullFolderPath);
	while (pathsToWalk.length > 0)
	{
		Str nextPath = CopyStr(pathsToWalk.strings[0]);
		RemoveStrAtIndex(&pathsToWalk, 0);
		Str nextPathRelative = nextPath;
		if (StrExactStartsWith(nextPath, fullFolderPath)) { nextPathRelative = StrSliceFrom(nextPath, fullFolderPath.length); }
		Str nextDestFolder = JoinPaths(destPath, nextPathRelative);
		MyCreateFolder(nextDestFolder, false);
		FileIter iter = StartFileIter(nextPath);
		Str nextFileOrFolder = EMPTY;
		bool isFolder = false;
		while (StepFileIter(&iter, &nextFileOrFolder, &isFolder))
		{
			if (!isFolder)
			{
				CopyFileToFolder(nextFileOrFolder, nextDestFolder);
			}
			else if (copySubFolders)
			{
				AddStr(&pathsToWalk, nextFileOrFolder);
			}
		}
		FreeStr(&nextPath);
	}
}

#endif //  _PIG_BUILD_FILE_H
