/*
File:   pig_build_recompile.h
Author: Taylor Robbins
Date:   03\20\2026
Description:
	** This file contains the logic that allows us to detect if we
	** need to rebuild the build_script.c (if any of the source
	** files that the build script depends on changed).
	** We do this by calculating a combined hash of the build_script.c
	** file and all supporting files in this folder "pig_build/"
*/

#ifndef _PIG_BUILD_RECOMPILE_H
#define _PIG_BUILD_RECOMPILE_H

// NOTE: If you want to change any of these #defines, make sure you change them in build.sh and build.bat
// If we exit(REBUILD_EXIT_CODE) then the shell script will re-compile and re-run us
#define REBUILD_EXIT_CODE 42
#if BUILDING_ON_WINDOWS
#define BUILD_SCRIPT_EXE_NAME      "builder.exe"
#else
#define BUILD_SCRIPT_EXE_NAME      "builder"
#endif
#define BUILD_SCRIPT_HASH_PATH "builder_hash.txt"
#define BUILD_SCRIPT_SOURCE_NAME "build_script.c"
#define BUILD_SCRIPT_SOURCE_PATH "../" BUILD_SCRIPT_SOURCE_NAME
#define PIG_BUILD_FOLDER_NAME "pig_build"
#define PIG_BUILD_FOLDER_PATH "../" PIG_BUILD_FOLDER_NAME

bool TryParseHexU64(Str8 str, u64* valueOut)
{
	u8 charIndex = 0;
	u64 result = 0;
	str = TrimWhitespace(str);
	if (str.length >= 2 && str.chars[0] == '0' && str.chars[1] == 'x') { str.chars += 2; str.length-=2; }
	if (str.length == 0) { return false; }
	while (str.length > 0 && charIndex < 16)
	{
		if (str.chars[0] >= '0' && str.chars[0] <= '9')
		{
			result = result * 16ULL + (u64)(str.chars[0] - '0');
		}
		else if (str.chars[0] >= 'A' && str.chars[0] <= 'F')
		{
			result = result * 16ULL + (10 + (u64)(str.chars[0] - 'A'));
		}
		else if (str.chars[0] >= 'a' && str.chars[0] <= 'f')
		{
			result = result * 16ULL + (10 + (u64)(str.chars[0] - 'a'));
		}
		else
		{
			PrintLine("Invalid char in hex at index[%u]: \'%c\'", charIndex, str.chars[0]);
			return false;
		}
		str.chars++; str.length--;
		charIndex++;
	}
	if (str.length > 0)
	{
		PrintLine("String is too long for u64 hex! Remaining: [%lu]\"%.*s\"", str.length, StrPrint(str));
		return false;
	}
	if (valueOut != nullptr) { *valueOut = result; }
	return true;
}

char GetHexChar(u8 hexValue, bool upperCase)
{
	if (hexValue <= 9) { return '0' + hexValue; }
	else if (hexValue < 16) { return (upperCase ? 'A' : 'a') + (hexValue - 10); }
	else { return '?'; }
}
Str8 ConvertU64ToHexStr(u64 value, bool upperCase)
{
	Str8 result;
	result.length = 2 + (sizeof(u64)*2);
	result.chars = (char*)malloc(result.length+1);
	result.chars[0] = '0';
	result.chars[1] = 'x';
	for (u8 bIndex = 0; bIndex < (sizeof(u64)*2); bIndex++)
	{
		result.chars[(result.length-1) - bIndex] = GetHexChar((u8)(value & 0x0FULL), upperCase);
		value = (value >> 4ULL);
	}
	result.chars[result.length] = '\0';
	return result;
}

// This is the hashing algorithm we use to detect changes. It doesn't need to be
// cryptographically safe or anything, just very unlikely to collide.
#define FNV_HASH_BASE_U64   0xcbf29ce484222325ULL //= DEC(14,695,981,039,346,656,037)
#define FNV_HASH_PRIME_U64  0x00000100000001b3ULL //= DEC(1,099,511,628,211)
u64 FnvHash(const void* bufferPntr, u64 numBytes, u64 startingState)
{
	const u8* bytePntr = (const u8*)bufferPntr;
	u64 result = startingState;
	for (u64 bIndex = 0; bIndex < numBytes; bIndex++)
	{
		result = result ^ bytePntr[bIndex];
		result = result * FNV_HASH_PRIME_U64;
	}
	return result;
}

// Call this function at the top of your build_scropt.c main function.
// If the source code for build_script.c changes OR if any of the helper files in the build_script/ folder
// change then your builder will be re-compiled (this function will call exit(REBUILD_EXIT_CODE);)
void RecompileIfNeeded()
{
	Str8 buildScriptFilePath = StrLit(BUILD_SCRIPT_SOURCE_PATH);
	Str8 buildSystemFolderPath = StrLit(PIG_BUILD_FOLDER_PATH);
	
	Str8 buildScriptContents = ZEROED;
	if (!TryReadFile(buildScriptFilePath, &buildScriptContents))
	{
		PrintLine("Failed to read script contents to check if it's changed. Looking at \"%.*s\"", StrPrint(buildScriptFilePath));
		exit(REBUILD_EXIT_CODE);
	}
	u64 buildScriptHash = FnvHash(buildScriptContents.chars, buildScriptContents.length, FNV_HASH_BASE_U64);
	free(buildScriptContents.chars);
	FileIter fileIter = StartFileIter(buildSystemFolderPath);
	Str8 fileIterPath = ZEROED;
	bool fileIterIsFolder = false;
	while (StepFileIter(&fileIter, &fileIterPath, &fileIterIsFolder))
	{
		//TODO: We should probably only hash files that have extensions like ".c" or ".h" or ".cpp" or etc.
		if (!fileIterIsFolder)
		{
			Str8 buildSystemFileContents = ZEROED;
			if (!TryReadFile(fileIterPath, &buildSystemFileContents))
			{
				PrintLine("Failed to read build system file contents to check if it's changed. Looking at \"%.*s\"", StrPrint(fileIterPath));
				exit(REBUILD_EXIT_CODE);
			}
			buildScriptHash = FnvHash(buildSystemFileContents.chars, buildSystemFileContents.length, buildScriptHash);
			free(buildSystemFileContents.chars);
		}
	}
	
	Str8 buildHashFilePath = StrLit(BUILD_SCRIPT_HASH_PATH);
	u64 savedHash = 0;
	bool hashesMatch = false;
	bool hashFileExisted = false;
	Str8 buildHashContents = ZEROED;
	if (TryReadFile(buildHashFilePath, &buildHashContents))
	{
		// PrintLine("Opened %u byte hash file: \"%.*s\"", buildHashContents.length, StrPrint(buildHashContents));
		hashFileExisted = true;
		if (TryParseHexU64(buildHashContents, &savedHash))
		{
			if (buildScriptHash == savedHash)
			{
				hashesMatch = true;
			}
		}
		else { PrintLine("Couldn't parse \"%.*s\" as hex", StrPrint(buildHashContents)); }
	}
	else if (DoesFileExist(buildHashFilePath)) { PrintLine("Couldn't open hash file at \"%.*s\"", StrPrint(buildHashFilePath)); }
	
	if (!hashFileExisted)
	{
		Str8 buildScriptHashString = ConvertU64ToHexStr(buildScriptHash, true);
		// PrintLine("Creating \"%.*s\" Calc=[%d]%.*s", StrPrint(buildHashFilePath), buildScriptHashString.length, StrPrint(buildScriptHashString));
		CreateAndWriteFile(buildHashFilePath, buildScriptHashString, true);
	}
	else if (!hashesMatch)
	{
		// PrintLine(BUILD_SCRIPT_SOURCE_NAME " hash doesn't match! Need a rebuild! Calc=0x%016lX Saved=0x%016lX", buildScriptHash, savedHash);
		exit(REBUILD_EXIT_CODE);
	}
	// else { PrintLine("No changes to " BUILD_SCRIPT_SOURCE_NAME ". Calc=0x%016lX Saved=0x%016lX", buildScriptHash, savedHash); }
}

#endif //  _PIG_BUILD_RECOMPILE_H
