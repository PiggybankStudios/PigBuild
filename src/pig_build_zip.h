/*
File:   pig_build_zip.h
Author: Taylor Robbins
Date:   04\21\2026
Description:
	** Holds the UnpackArchive function that either uses miniz.h for .zip files or
	** the `tar` command-line tool to decompress archive file formats like .zip, .tar.gz, etc.
	** This is primarily useful when downloading dependencies at build time, since many
	** sites distribute their libraries inside one of these file formats.
*/

#ifndef _PIG_BUILD_ZIP_H
#define _PIG_BUILD_ZIP_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_file.h"

//TODO: We should probably do MINIZ_NO_STDIO and use our own file wrappers to read/write files
#include "third_party/miniz.c"

typedef struct UnzipResult UnzipResult;
struct UnzipResult
{
	bool success;
	Str errorStr;
	u64 numFiles;
	u64 decompressedSize;
};

UnzipResult UnzipEntireArchiveInto(Str archiveFilePath, Str folderToExtractInArchive, Str outputFolderPath)
{
	UnzipResult result = EMPTY;
	result.errorStr = StrLit("Unknown");
	
	//TODO: We should probabl automatically create the output folder if it doesn't already exist
	
	Str finalExtension = GetFileExtPart(archiveFilePath, false);
	Str subfinalExtension = GetFileExtPart(StrSlice(archiveFilePath, 0, archiveFilePath.length-finalExtension.length), false);
	if (StrExactEquals(finalExtension, StrLit(".zip")))
	{
		Str archiveFilePathNt = CopyStr(archiveFilePath, true);
		mz_zip_archive archive = EMPTY;
		mz_bool initResult = mz_zip_reader_init_file(
			&archive,
			archiveFilePathNt.chars,
			0 //No MZ_ZIP_FLAGs
		);
		if (initResult != MZ_TRUE)
		{
			PrintLine_E("Failed to parse zip archive with miniz.h %s: \"%.*s\"", mz_zip_get_error_string(mz_zip_get_last_error(&archive)), StrPrint(archiveFilePath));
			result.errorStr = StrLit("Failed to parse zip archive with miniz.h");
			return result;
		}
		
		result.numFiles = (u64)mz_zip_reader_get_num_files(&archive);
		result.decompressedSize = mz_zip_get_archive_size(&archive);
		
		//TODO: Create the folder if it doesn't already exist
		
		for (u64 fIndex = 0; fIndex < result.numFiles; fIndex++)
		{
			u64 entryPathLength = (u64)mz_zip_reader_get_filename(&archive, (mz_uint)fIndex, nullptr, 0);
			Str entryPath = AllocStr(entryPathLength, true);
			u64 secondEntryPathLength = mz_zip_reader_get_filename(&archive, (mz_uint)fIndex, entryPath.chars, entryPath.length);
			Assert(secondEntryPathLength == entryPathLength);
			entryPath.chars[entryPath.length] = '\0';
			
			if (folderToExtractInArchive.length == 0 || StrExactStartsWith(entryPath, folderToExtractInArchive))
			{
				Str entryRelativePath = (folderToExtractInArchive.length > 0) ? StrSliceFrom(entryPath, folderToExtractInArchive.length) : entryPath;
				Str outputPath = JoinPaths(outputFolderPath, entryRelativePath, true);
				
				if (mz_zip_reader_is_file_a_directory(&archive, (mz_uint)fIndex))
				{
					// PrintLine("Creating output folder for zip archive: \"%.*s\"", StrPrint(outputPath));
					mkdir(outputPath.chars, FOLDER_PERMISSIONS);
				}
				else
				{
					if (mz_zip_reader_is_file_supported(&archive, (mz_uint)fIndex))
					{
						// Str fileNameAndExt = CopyStr(GetFileNamePart(entryRelativePath, true), true);
						// PrintLine("Extracting %llu/%llu \"%.*s\" to \"%.*s\"...", fIndex+1, result.numFiles, StrPrint(fileNameAndExt), StrPrint(outputPath));
						mz_bool extractResult = mz_zip_reader_extract_to_file(&archive, (mz_uint)fIndex, outputPath.chars, 0);
						AssertFmt(extractResult == MZ_TRUE, "Failed to extract file[%llu] \"%.*s\", %s: inside \"%.*s\"", fIndex, StrPrint(entryPath), mz_zip_get_error_string(mz_zip_get_last_error(&archive)), StrPrint(archiveFilePath));
					}
					else { PrintLine("Unsupported entry in zip archive: \"%.*s\"", StrPrint(entryPath)); }
				}
				
				FreeStr(&outputPath);
			}
			
			FreeStr(&entryPath);
		}
		
		mz_bool archiveCloseResult = mz_zip_reader_end(&archive);
		AssertFmt(archiveCloseResult == MZ_TRUE, "Failed to close archive %s: \"%.*s\"", mz_zip_get_error_string(mz_zip_get_last_error(&archive)), StrPrint(archiveFilePath));
		
		result.success = true;
	}
	else if (StrExactStartsWith(subfinalExtension, StrLit(".tar")))
	{
		CliArgList args = EMPTY;
		AddArg(&args, "--extract");
		AddArgStr(&args, "--file=\"[VAL]\"", archiveFilePath);
		AddArgStr(&args, "-C \"[VAL]\"", outputFolderPath);
		
		if (StrExactEquals(finalExtension, StrLit(".gz"))) { AddArg(&args, "--gzip"); }
		else if (StrExactEquals(finalExtension, StrLit(".bz2"))) { AddArg(&args, "--bzip2"); }
		else if (StrExactEquals(finalExtension, StrLit(".xz"))) { AddArg(&args, "--xz"); }
		else { AssertFmt(false, "Unsupported .tar subtype \"%.*s\"! We support .gz, .bz2 and .xz", StrPrint(finalExtension)); }
		
		if (!IsEmptyStr(folderToExtractInArchive))
		{
			AddArgStr(&args, CLI_QUOTED_ARG, WithTrailingSlash(folderToExtractInArchive));
			AddArgInt(&args, "--strip-components=[VAL]", (i32)CountPathParts(folderToExtractInArchive));
		}
		
		// RunCliProgramAndExitOnFailure(StrLit("tar"), "", &args, StrLit("Failed to unpack .tar file with \"tar\" CLI tool. Is it not installed? Is the file corrupt?"));
		int tarReturnCode = RunCliProgram(StrLit("tar"), "", &args);
		if (tarReturnCode != 0)
		{
			result.errorStr = StrLit("Failed to unpack .tar file with \"tar\" CLI tool. Is it not installed? Is the file corrupt?");
			return result;
		}
		
		result.success = true;
	}
	else
	{
		AssertFmt(false, "We only support .zip and .tar files right now! Not \"%.*s\"", StrPrint(finalExtension));
	}
	
	return result;
}

#endif //  _PIG_BUILD_ZIP_H
