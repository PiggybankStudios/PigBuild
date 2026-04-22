/*
File:   pig_build_file.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_FILE_H
#define _PIG_BUILD_FILE_H

#include "pig_build_base.h"
#include "pig_build_str.h"

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
// Result is always null-terminated
// TODO: On linux this will not work properly for paths to folders that don't exist yet
Str GetFullPath(Str relativePath, char slashChar)
{
	Str result = Str_Empty_Const;
	
	#if BUILDING_ON_WINDOWS
	{
		Str relativePathNt = CopyStr(relativePath, true);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		Assert(getPathResult1 != 0);
		
		result = AllocStr((u64)getPathResult1-1, true);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		Assert(getPathResult2+1 == getPathResult1);
		Assert(result.chars[result.length] == '\0');
		
		FixPathSlashes(result, slashChar);
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
		
		FixPathSlashes(result, slashChar);
		free(temporaryBuffer);
		free(relativePathNt.chars);
	}
	#else
	AssertMsg(false, "GetFullPath does not support the current platform yet!");
	#endif
		
	return result;
}

bool DoesFileOrFolderExist(Str path, bool* isFolderOut)
{
	bool result = false;
	
	#if BUILDING_ON_WINDOWS
	{
		Str fullPath = GetFullPath(path, PATH_SEP_CHAR);
		BOOL fileExistsResult = PathFileExistsA(fullPath.chars);
		if (fileExistsResult == TRUE)
		{
			if (isFolderOut != nullptr)
			{
				DWORD fileType = GetFileAttributesA(fullPath.chars);
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
		Str fullPath = GetFullPath(path, PATH_SEP_CHAR);
		
		int accessResult = access(fullPath.chars, F_OK);
		result = (accessResult == 0);
		
		if (isFolderOut != nullptr && result)
		{
			struct stat statStruct = EMPTY;
			int statResult = stat(fullPath.chars, &statStruct);
			if (statResult == 0)
			{
				*isFolderOut = IsFlagSet(statStruct.st_mode, S_IFDIR);
			}
			else
			{
				PrintLine_E("stat(\"%.*s\") call failed! Can't determine if that path is a folder or file!", StrPrint(fullPath));
				*isFolderOut = false;
			}
		}
		
	}
	#else
	AssertMsg(false, "DoesFileOrFolderExist does not support the current platform yet!");
	#endif
	
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
void AssertFileExist(Str filePath, bool wasCreatedByBuild)
{
	if (!DoesFileExist(filePath))
	{
		PrintLine_E("Missing file \"%.*s\" %s!", StrPrint(filePath), wasCreatedByBuild ? "was not created" : "was not found");
		exit(6);
	}
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
//NOTE: We can't name this "ReadFile" because it conflicts with a Windows function
Str ReadEntireFile(Str filePath)
{
	Str result = Str_Empty_Const;
	bool readSuccess = TryReadFile(filePath, &result);
	if (!readSuccess) { exit(3); }
	return result;
}

void CreateAndWriteFile(Str filePath, Str contents, bool convertNewLines)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contents = StrReplace(contents, StrLit("\n"), StrLit("\r\n"), false); }
		HANDLE fileHandle = CreateFileA(
			filePathNt.chars,      //Name of the file
			GENERIC_WRITE,         //Open for writing
			0,                     //Do not share
			NULL,                  //Default security
			CREATE_ALWAYS,         //Always overwrite
			FILE_ATTRIBUTE_NORMAL, //Default file attributes
			0                      //No Template File
		);
		Assert(fileHandle != INVALID_HANDLE_VALUE);
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

void AppendToFile(Str filePath, Str contentsToAppend, bool convertNewLines)
{
	Str filePathNt = CopyStr(filePath);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		if (convertNewLines) { contentsToAppend = StrReplace(contentsToAppend, StrLit("\n"), StrLit("\r\n"), false); }
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
			AssertFmt(fileHandle != INVALID_HANDLE_VALUE, "CreateFileA error: %d", errorCode);
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
	char printBuffer[512];
	
	va_list args;
	va_start(args, formatString);
	int printResult = vsnprintf(&printBuffer[0], ArrayCount(printBuffer), formatString, args);
	va_end(args);
	Assert(printResult >= 0);
	Assert(printResult < ArrayCount(printBuffer));
	Str printedStr = MakeStr((u64)printResult, &printBuffer[0]);
	AppendToFile(filePath, printedStr, true);
}

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

void MyRemoveDirectory(Str folderPath, bool recursive)
{
	Str folderPathNt = CopyStr(folderPath);
	FixPathSlashes(folderPathNt, PATH_SEP_CHAR);
	
	if (!recursive)
	{
		int rmResult = rmdir("apk_temp");
		if (rmResult != 0)
		{
			AssertFmt(rmResult == 0, "rmdir(\"%s\") failed: errno=%d", folderPathNt.chars, errno);
		}
	}
	else
	{
		#if BUILDING_ON_WINDOWS
		{
			Str searchStr = JoinPaths(folderPathNt, StrLit("*"), true);
			
			WIN32_FIND_DATAA findData = EMPTY;
			HANDLE iterHandle = FindFirstFileA(searchStr.chars, &findData);
			if (iterHandle == INVALID_HANDLE_VALUE) { return; }
			
			do
			{
				Str fileNameStr = MakeStrNt(findData.cFileName);
				if (StrExactEquals(fileNameStr, StrLit(".")) || StrExactEquals(fileNameStr, StrLit(".."))) { continue; }
				Str fullPath = JoinPaths(folderPath, fileNameStr, false);
				
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// PrintLine("Recursing into \"%.*s\"", StrPrint(fullPath));
					MyRemoveDirectory(fullPath, true);
				}
				else
				{
					// PrintLine("Removing file \"%.*s\"", StrPrint(fullPath));
					RemoveFile(fullPath);
				}
				
			} while(FindNextFileA(iterHandle, &findData) != 0);
			
			BOOL removeResult = RemoveDirectoryA(folderPathNt.chars);
			if (removeResult == 0)
			{
				DWORD errorCode = GetLastError();
				AssertFmt(removeResult != 0, "Failed to remove \"%s\". Error: %d", folderPathNt.chars, errorCode);
			}
		}
		#else
		AssertMsg(false, "RemoveDirectory does not support the current platform yet!");
		#endif
	}
}

void CopyFileToPath(Str filePath, Str newFilePath, bool copyPermissions)
{
	Str fileContents = Str_Empty_Const;
	bool readSuccess = TryReadFile(filePath, &fileContents);
	Assert(readSuccess);
	CreateAndWriteFile(newFilePath, fileContents, false);
	free(fileContents.chars);
	#if BUILDING_ON_LINUX
	if (copyPermissions)
	{
		Str filePathNt = CopyStr(filePath);
		struct stat oldFileStats = EMPTY;
		int statResult = stat(filePathNt.chars, &oldFileStats);
		Assert(statResult == 0);
		free(filePathNt.chars);
		
		Str newFilePathNt = CopyStr(newFilePath);
		int modResult = chmod(newFilePathNt.chars, oldFileStats.st_mode);
		Assert(modResult == 0);
		free(newFilePathNt.chars);
	}
	#endif
}
void CopyFileToFolder(Str filePath, Str folderPath, bool copyPermissions)
{
	Str newPath = JoinPaths(folderPath, GetFileNamePart(filePath, true));
	CopyFileToPath(filePath, newPath, copyPermissions);
	free(newPath.chars);
}

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
		result.folderPathWithWildcard = JoinStrings2(result.folderPathNt, StrLit("*"), true);
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

// Ex version gives isFolderOut
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
				*pathOut = JoinStrings2(fileIter->folderPathNt, fileName, true);
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

#endif //  _PIG_BUILD_FILE_H
