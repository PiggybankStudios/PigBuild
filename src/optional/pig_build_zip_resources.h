/*
File:   pig_build_zip_resources.h
Author: Taylor Robbins
Date:   07\15\2026
Description:
	** For many non-game applications the "resources" are not very
	** large and it's not worth having them live separately on disk
	** which can be the cause of problems. For these applications
	** we have a system where we take all the resources at build
	** time and compress them into a .zip archive. We then take
	** the bytes of that file and put them directly into an array
	** in generated C source code so that they become part of the
	** executable. These bytes can then be decompressed at runtime
	** and used directly without needing to do any file access.
*/

#ifndef _PIG_BUILD_ZIP_RESOURCES_H
#define _PIG_BUILD_ZIP_RESOURCES_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_file.h"
#include "pig_build_zip.h"

#if 0
//NOTE: We use miniz.h when BUNDLE_RESOURCES_ZIP is enabled
#define MINIZ_NO_STDIO //to disable all usage and any functions which rely on stdio for file I/O.
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_LITTLE_ENDIAN                  1
#include "core/third_party/miniz/miniz.h"
#include "core/third_party/miniz/miniz.c"
#endif

typedef struct BundleResourcesContext BundleResourcesContext;
struct BundleResourcesContext
{
	mz_zip_archive zip;
	Str relativePath;
	StrArray resourcePaths;
	u64 uncompressedSize;
	u64 archiveAllocSize;
	u64 archiveSize;
	u8* archivePntr;
};

// +==============================+
// |   BundleResourcesCallback    |
// +==============================+
// bool BundleResourcesCallback(Str path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(BundleResourcesCallback)
{
	BundleResourcesContext* context = (BundleResourcesContext*)contextPntr;
	if (!isFolder)
	{
		Str fileContents = ReadEntireFile(path);
		assert(StrExactStartsWith(path, context->relativePath));
		Str inZipPath = StrSliceFrom(path, context->relativePath.length);
		if (inZipPath.length > 0 && IsSlash(inZipPath.chars[0])) { inZipPath.length--; inZipPath.chars++; }
		Str inZipPathNt = CopyStr(inZipPath);
		FixPathSlashes(inZipPathNt, '/');
		mz_bool addMemSuccess = mz_zip_writer_add_mem(&context->zip, inZipPathNt.chars, fileContents.bytes, (size_t)fileContents.length, (mz_uint)MZ_BEST_COMPRESSION);
		assert(addMemSuccess == MZ_TRUE);
		context->uncompressedSize += fileContents.length;
		AddStr(&context->resourcePaths, inZipPath);
		free(inZipPathNt.chars);
		free(fileContents.chars);
	}
	return true;
}
size_t ZipFileWriteCallback(void* contextPntr, mz_uint64 fileOffset, const void* bufferPntr, size_t numBytes)
{
	// PrintLine("ZipFileWriteCallback(%p, %llu, %p, %zu)", contextPntr, fileOffset, bufferPntr, numBytes);
	BundleResourcesContext* context = (BundleResourcesContext*)contextPntr;
	assert(context != nullptr);
	if (context->archiveAllocSize < fileOffset + numBytes)
	{
		u64 newAllocSize = context->archiveAllocSize;
		if (newAllocSize < 8) { newAllocSize = 8; }
		while (newAllocSize < fileOffset + numBytes) { newAllocSize *= 2; }
		void* newAllocPntr = malloc(newAllocSize);
		if (context->archiveSize > 0) { memcpy(newAllocPntr, context->archivePntr, context->archiveSize); }
		if (context->archivePntr != nullptr) { free(context->archivePntr); }
		context->archivePntr = newAllocPntr;
		context->archiveAllocSize = newAllocSize;
	}
	memcpy(&context->archivePntr[fileOffset], bufferPntr, numBytes);
	if (context->archiveSize < fileOffset + numBytes) { context->archiveSize = fileOffset + numBytes; }
	return numBytes;
}

void BundleResourcesZip(Str resourcesFolder, Str resourcesZipPath, Str resourcesHeaderPath, Str resourcesSourcePath, Str arrayName)
{
	BundleResourcesContext context = EMPTY;
	context.zip.m_pWrite = ZipFileWriteCallback;
	context.zip.m_pIO_opaque = &context;
	mz_bool initResult = mz_zip_writer_init(&context.zip, 0);
	if (initResult != MZ_TRUE) { PrintLine_E("zip error: %s", mz_zip_get_error_string(context.zip.m_last_error)); }
	assert(initResult == MZ_TRUE);
	context.relativePath = resourcesFolder;
	RecursiveDirWalk(resourcesFolder, BundleResourcesCallback, &context);
	mz_bool finalizeResult = mz_zip_writer_finalize_archive(&context.zip);
	assert(finalizeResult == MZ_TRUE);
	mz_zip_writer_end(&context.zip);
	PrintLine("Found %llu resource files, total %llu bytes uncompressed, %llu compressed (%.1f%%)", context.resourcePaths.length, context.uncompressedSize, context.archiveSize, ((float)context.archiveSize / (float)context.uncompressedSize) * 100.0);
	
	CreateAndWriteFile(resourcesZipPath, MakeStr(context.archiveSize, context.archivePntr), false);
	
	//Create resources_zip.h
	{
		Str headerFileContents = EMPTY;
		for (int pass = 0; pass < 2; pass++)
		{
			u64 fileSize = 0;
			
			Str headerFileName = GetFileNamePart(resourcesHeaderPath, true);
			Str headerGuardDefineName = StrLit("_RESOURCES_ZIP_H"); //TODO: We should probably generate this header guard name based on the file name
			TwoPassPrint(&headerFileContents, &fileSize,
				"/*\n"
				"File:   %.*s\n"
				"Author: WARNING: This file is generated by " BUILD_SCRIPT_SOURCE_NAME "! Any hand edits will be lost!\n"
				"*/\n\n"
				"#ifndef %.*s\n"
				"#define %.*s\n\n",
				StrPrint(headerFileName),
				StrPrint(headerGuardDefineName),
				StrPrint(headerGuardDefineName)
			);
			TwoPassPrint(&headerFileContents, &fileSize, "u8 %.*s[%u];\n\n", StrPrint(arrayName), context.archiveSize);
			TwoPassPrint(&headerFileContents, &fileSize, "#endif //%.*s\n", StrPrint(headerGuardDefineName));
			
			if (pass == 0)
			{
				headerFileContents.length = fileSize;
				headerFileContents.pntr = malloc(headerFileContents.length+1);
				assert(headerFileContents.pntr != nullptr);
			}
			else { assert(fileSize == headerFileContents.length); headerFileContents.chars[headerFileContents.length] = '\0'; }
		}
		
		CreateAndWriteFile(resourcesHeaderPath, headerFileContents, true);
		free(headerFileContents.chars);
	}
	
	//Create resources_zip.c
	{
		Str sourceFileContents = EMPTY;
		for (int pass = 0; pass < 2; pass++)
		{
			u64 fileSize = 0;
			
			TwoPassPrint(&sourceFileContents, &fileSize, "// This file is generated by " BUILD_SCRIPT_SOURCE_NAME "! Any hand edits will be lost!\n\n");
			TwoPassPrint(&sourceFileContents, &fileSize, "// Archive Contents (%u file%s, %u bytes uncompressed):\n", context.resourcePaths.length, (context.resourcePaths.length == 1) ? "" : "s", context.uncompressedSize);
			for (u64 rIndex = 0; rIndex < context.resourcePaths.length; rIndex++)
			{
				TwoPassPrint(&sourceFileContents, &fileSize, "//\t%.*s\n", StrPrint(context.resourcePaths.strings[rIndex]));
			}
			TwoPassPrint(&sourceFileContents, &fileSize, "\nu8 %.*s[%u] = {\n\t", StrPrint(arrayName), context.archiveSize);
			for (u64 bIndex = 0; bIndex < context.archiveSize; bIndex++)
			{
				if (bIndex > 0) { TwoPassPrint(&sourceFileContents, &fileSize, ",%s", (bIndex%32) == 0 ? "\n\t" : " "); }
				TwoPassPrint(&sourceFileContents, &fileSize, "0x%02X", context.archivePntr[bIndex]);
			}
			TwoPassPrint(&sourceFileContents, &fileSize, "\n};\n");
			
			if (pass == 0)
			{
				sourceFileContents.length = fileSize;
				sourceFileContents.pntr = malloc(sourceFileContents.length+1);
				assert(sourceFileContents.pntr != nullptr);
			}
			else { assert(fileSize == sourceFileContents.length); sourceFileContents.chars[sourceFileContents.length] = '\0'; }
		}
		CreateAndWriteFile(resourcesSourcePath, sourceFileContents, true);
		free(sourceFileContents.chars);
	}
}

#endif //  _PIG_BUILD_ZIP_RESOURCES_H
