/*
File:   pig_build_hash.h
Author: Taylor Robbins
Date:   04\21\2026
*/

#ifndef _PIG_BUILD_HASH_H
#define _PIG_BUILD_HASH_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_file.h"

#define FNV_HASH_BASE_U64   0xcbf29ce484222325ULL //= DEC(14,695,981,039,346,656,037)
#define FNV_HASH_PRIME_U64  0x00000100000001b3ULL //= DEC(1,099,511,628,211)

// This is the hashing algorithm we use to detect changes and verify contents of files.
// It doesn't need to be cryptographically safe or anything, just very unlikely to collide.
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

u64 FnvHashStr(Str str, u64 startingState)
{
	return FnvHash(str.pntr, str.length, startingState);
}

u64 FnvHashFile(Str filePath, u64 startingState)
{
	Str fileContents = ReadEntireFile(filePath);
	u64 fileHash = FnvHashStr(fileContents, startingState);
	FreeStr(&fileContents);
	return fileHash;
}

void EnsureFileSizeAndHash(Str filePath, u64 expectedSize, u64 expectedHash)
{
	Str fileContents = Str_Empty_Const;
	bool readFileSuccessfully = TryReadFile(filePath, &fileContents);
	AssertFmt(readFileSuccessfully, "Failed to open file \"%.*s\" to ensure content size/hash", StrPrint(filePath));
	AssertFmt(fileContents.length == expectedSize, "File is %llu bytes instead of expected %llu bytes: \"%.*s\"", fileContents.length, expectedSize, StrPrint(filePath));
	u64 fileHash = FnvHashStr(fileContents, FNV_HASH_BASE_U64);
	FreeStr(&fileContents);
	AssertFmt(fileHash == expectedHash, "File hash is 0x%016llX instead of expected 0x%016llX: \"%.*s\"", fileHash, expectedHash, StrPrint(filePath));
}

#endif //  _PIG_BUILD_HASH_H
