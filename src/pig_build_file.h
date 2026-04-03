/*
File:   pig_build_file.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_FILE_H
#define _PIG_BUILD_FILE_H

#include "pig_build_base.h"
#include "pig_build_str8.h"

typedef struct FileIter FileIter;
struct FileIter
{
	bool finished;
	Str8 folderPathNt;
	u64 index;
	u64 nextIndex;
	
	#if BUILDING_ON_WINDOWS
	Str8 folderPathWithWildcard;
	WIN32_FIND_DATAA findData;
	HANDLE handle;
	#elif BUILDING_ON_LINUX || BUILDING_ON_OSX
	DIR* dirHandle;
	#endif
};

#define RECURSIVE_DIR_WALK_CALLBACK_DEF(functionName) bool functionName(Str8 path, bool isFolder, void* contextPntr)
typedef RECURSIVE_DIR_WALK_CALLBACK_DEF(RecursiveDirWalkCallback_f);

// +--------------------------------------------------------------+
// |                        File Functions                        |
// +--------------------------------------------------------------+
// Result is always null-terminated
// TODO: On linux this will not work properly for paths to folders that don't exist yet
Str8 GetFullPath(Str8 relativePath, char slashChar)
{
	Str8 result = ZEROED;
	
	#if BUILDING_ON_WINDOWS
	{
		Str8 relativePathNt = CopyStr8(relativePath, true);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		// Returns required buffer size +1 when the nBufferLength is too small
		DWORD getPathResult1 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			0, //nBufferLength
			nullptr, //lpBuffer
			nullptr //lpFilePart
		);
		assert(getPathResult1 != 0);
		
		result.length = (u64)getPathResult1-1;
		result.chars = (char*)malloc(result.length + 1);
		
		// Returns the length of the string (not +1) when nBufferLength is large enough
		DWORD getPathResult2 = GetFullPathNameA(
			relativePathNt.chars, //lpFileName
			(DWORD)(result.length+1), //nBufferLength
			result.chars, //lpBuffer
			nullptr //lpFilePart
		);
		assert(getPathResult2+1 == getPathResult1);
		assert(result.chars[result.length] == '\0');
		
		FixPathSlashes(result, slashChar);
		free(relativePathNt.chars);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		Str8 relativePathNt = CopyStr8(relativePath, true);
		FixPathSlashes(relativePathNt, PATH_SEP_CHAR);
		
		char* temporaryBuffer = (char*)malloc(PATH_MAX);
		char* realPathResult = realpath(relativePathNt.chars, temporaryBuffer);
		assert(realPathResult != nullptr);
		
		result.length = (u64)strlen(realPathResult);
		result.chars = (char*)malloc(result.length + 1);
		memcpy(result.chars, realPathResult, result.length);
		result.chars[result.length] = '\0';
		
		FixPathSlashes(result, slashChar);
		free(temporaryBuffer);
		free(relativePathNt.chars);
	}
	#else
	assert(false && "GetFullPath does not support the current platform yet!");
	#endif
		
	return result;
}

bool TryReadFile(Str8 filePath, Str8* contentsOut)
{
	Str8 filePathNt = CopyStr8(filePath, true);
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
	
	int seekResult1 = fseek(fileHandle, 0, SEEK_END); assert(seekResult1 == 0);
	long fileSize = ftell(fileHandle); assert(fileSize >= 0); assert(fileSize <= INT_MAX);
	int seekResult2 = fseek(fileHandle, 0, SEEK_SET); assert(seekResult2 == 0);
	
	contentsOut->length = (u64)fileSize;
	contentsOut->chars = (char*)malloc(fileSize+1);
	assert(contentsOut->chars != nullptr);
	
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
Str8 ReadEntireFile(Str8 filePath)
{
	Str8 result = ZEROED;
	bool readSuccess = TryReadFile(filePath, &result);
	if (!readSuccess) { exit(3); }
	return result;
}

void CreateAndWriteFile(Str8 filePath, Str8 contents, bool convertNewLines)
{
	Str8 filePathNt = CopyStr8(filePath, true);
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
		assert(fileHandle != INVALID_HANDLE_VALUE);
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
			assert(writeResult == TRUE);
			assert((u64)bytesWritten == contents.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contents.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		FILE* fileHandle = fopen(filePathNt.chars, "w");
		assert(fileHandle != nullptr);
		if (contents.length > 0)
		{
			size_t writeResult = fwrite(
				contents.pntr, //ptr
				1, //size
				contents.length, //count
				fileHandle //stream
			);
			assert(writeResult >= 0);
			assert((u64)writeResult == contents.length);
		}
		fclose(fileHandle);
	}
	#else
	assert(false && "CreateAndWriteFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}

void AppendToFile(Str8 filePath, Str8 contentsToAppend, bool convertNewLines)
{
	Str8 filePathNt = CopyStr8(filePath, true);
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
			PrintLine_E("CreateFileA error: %d", errorCode);
			assert(fileHandle != INVALID_HANDLE_VALUE);
		}
		
		DWORD moveResult = SetFilePointer(
			fileHandle, //hFile
			0, //lDistanceToMove,
			NULL, //lDistanceToMoveHigh
			FILE_END
		);
		assert(moveResult != INVALID_SET_FILE_POINTER);
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
			assert(writeResult == TRUE);
			assert((u64)bytesWritten == contentsToAppend.length);
		}
		CloseHandle(fileHandle);
		if (convertNewLines) { free(contentsToAppend.chars); }
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		FILE* fileHandle = fopen(filePathNt.chars, "a");
		assert(fileHandle != nullptr);
		if (contentsToAppend.length > 0)
		{
			size_t writeResult = fwrite(
				contentsToAppend.pntr, //ptr
				1, //size
				contentsToAppend.length, //count
				fileHandle //stream
			);
			assert(writeResult >= 0);
			assert((u64)writeResult == contentsToAppend.length);
		}
		fclose(fileHandle);
	}
	#else
	assert(false && "AppendToFile does not support the current platform yet!");
	#endif
	
	free(filePathNt.chars);
}
void AppendPrintToFile(Str8 filePath, const char* formatString, ...)
{
	char printBuffer[512];
	
	va_list args;
	va_start(args, formatString);
	int printResult = vsnprintf(&printBuffer[0], ArrayCount(printBuffer), formatString, args);
	va_end(args);
	assert(printResult >= 0);
	assert(printResult < ArrayCount(printBuffer));
	Str8 printedStr = MakeStr8((u64)printResult, &printBuffer[0]);
	AppendToFile(filePath, printedStr, true);
}

void RemoveFile(Str8 filePath)
{
	Str8 filePathNt = CopyStr8(filePath, true);
	FixPathSlashes(filePathNt, PATH_SEP_CHAR);
	
	#if BUILDING_ON_WINDOWS
	{
		BOOL deleteResult = DeleteFileA(filePathNt.chars);
		if (deleteResult == 0)
		{
			DWORD errorCode = GetLastError();
			assert(errorCode == ERROR_FILE_NOT_FOUND);
		}
	}
	#else
	assert(false && "RemoveFile does not support the current platform yet!");
	#endif
}

void MyRemoveDirectory(Str8 folderPath, bool recursive)
{
	Str8 folderPathNt = CopyStr8(folderPath, true);
	FixPathSlashes(folderPathNt, PATH_SEP_CHAR);
	
	if (!recursive)
	{
		int rmResult = rmdir("apk_temp");
		if (rmResult != 0)
		{
			PrintLine_E("rmdir(\"%s\") failed: errno=%d", folderPathNt.chars, errno);
			assert(rmResult == 0);
		}
	}
	else
	{
		#if BUILDING_ON_WINDOWS
		{
			bool needsTrailingSlash = !(folderPathNt.length > 0 && folderPathNt.chars[folderPathNt.length-1] == PATH_SEP_CHAR);
			Str8 searchStr = JoinStrings3(
				folderPath,
				needsTrailingSlash ? StrLit(PATH_SEP_CHAR_STR) : StrLit(""),
				StrLit("*"),
				true
			);
			
			WIN32_FIND_DATAA findData = ZEROED;
			HANDLE iterHandle = FindFirstFileA(searchStr.chars, &findData);
			if (iterHandle == INVALID_HANDLE_VALUE) { return; }
			
			do
			{
				if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) { continue; }
				Str8 fullPath = JoinStrings3(
					folderPath,
					needsTrailingSlash ? StrLit(PATH_SEP_CHAR_STR) : StrLit(""),
					MakeStr8Nt(findData.cFileName),
					false
				);
				
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
				PrintLine_E("Failed to remove \"%s\". Error: %d", folderPathNt.chars, errorCode);
				assert(removeResult != 0);
			}
		}
		#else
		assert(false && "RemoveDirectory does not support the current platform yet!");
		#endif
	}
}

void CopyFileToPath(Str8 filePath, Str8 newFilePath, bool copyPermissions)
{
	Str8 fileContents = ZEROED;
	bool readSuccess = TryReadFile(filePath, &fileContents);
	assert(readSuccess);
	CreateAndWriteFile(newFilePath, fileContents, false);
	free(fileContents.chars);
	#if BUILDING_ON_LINUX
	if (copyPermissions)
	{
		Str8 filePathNt = CopyStr8(filePath, true);
		struct stat oldFileStats = ZEROED;
		int statResult = stat(filePathNt.chars, &oldFileStats);
		assert(statResult == 0);
		free(filePathNt.chars);
		
		Str8 newFilePathNt = CopyStr8(newFilePath, true);
		int modResult = chmod(newFilePathNt.chars, oldFileStats.st_mode);
		assert(modResult == 0);
		free(newFilePathNt.chars);
	}
	#endif
}
void CopyFileToFolder(Str8 filePath, Str8 folderPath, bool copyPermissions)
{
	Str8 fileName = GetFileNamePart(filePath, true);
	const char* joinStr = (folderPath.length == 0 || !IsSlash(folderPath.chars[folderPath.length-1])) ? "/" : "";
	Str8 newPath = JoinStrings3(folderPath, MakeStr8Nt(joinStr), fileName, false);
	CopyFileToPath(filePath, newPath, copyPermissions);
	free(newPath.chars);
}

bool DoesFileExist(Str8 filePath)
{
	char* filePathNt = (char*)malloc(filePath.length+1);
	memcpy(filePathNt, filePath.chars, filePath.length);
	filePathNt[filePath.length] = '\0';
	#if BUILDING_ON_WINDOWS
	{
		BOOL fileExistsResult = PathFileExistsA(filePathNt);
		free(filePathNt);
		return (fileExistsResult == TRUE);
	}
	#elif (BUILDING_ON_LINUX || BUILDING_ON_OSX)
	{
		int accessResult = access(filePathNt, F_OK);
		free(filePathNt);
		return (accessResult == 0);
	}
	#else
	assert(false && "pig_build_file.h's DoesFileExist does not support the current platform yet!");
	return false;
	#endif
}

void AssertFileExist(Str8 filePath, bool wasCreatedByBuild)
{
	if (!DoesFileExist(filePath))
	{
		PrintLine_E("Missing file \"%.*s\" %s!", StrPrint(filePath), wasCreatedByBuild ? "was not created" : "was not found");
		exit(6);
	}
}

FileIter StartFileIter(Str8 folderPath)
{
	FileIter result = ZEROED;
	result.index = UINT64_MAX;
	result.nextIndex = 0;
	result.finished = false;
	bool needsTrailingSlash = (folderPath.length == 0 || (folderPath.chars[folderPath.length-1] != '\\' && folderPath.chars[folderPath.length-1] != '/'));
	result.folderPathNt.length = folderPath.length + (needsTrailingSlash ? 1 : 0);
	result.folderPathNt.chars = (char*)malloc(result.folderPathNt.length + 1);
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
	assert(false && "StartFileIter does not support the current platform yet!");
	result.finished = true;
	#endif
	
	return result;
}

// Ex version gives isFolderOut
bool StepFileIter(FileIter* fileIter, Str8* pathOut, bool* isFolderOut)
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
			
			Str8 fileName = MakeStr8Nt(fileIter->findData.cFileName);
			
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
			
			Str8 fileName = MakeStr8Nt(entry->d_name);
			if (StrExactEquals(fileName, StrLit(".")) || StrExactEquals(fileName, StrLit(".."))) { continue; } //ignore current and parent folder entries
			
			Str8 fullPath = JoinStrings2(fileIter->folderPathNt, fileName, true);
			if (isFolderOut != nullptr)
			{
				struct stat statStruct = ZEROED;
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
	assert(false && "StepFileIter does not support the current platform yet!");
	fileIter->finished = true;
	#endif
	
	return false;
}

void RecursiveDirWalk(Str8 rootDir, RecursiveDirWalkCallback_f* callback, void* contextPntr)
{
	FileIter iter = StartFileIter(rootDir);
	Str8 path = ZEROED;
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
