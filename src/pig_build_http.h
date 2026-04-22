/*
File:   pig_build_http.h
Author: Taylor Robbins
Date:   04\21\2026
Description:
	** Holds DownloadFromUrl which helps you download files from particular
	** http(s) urls using the "curl" CLI tool (or libcurl).
	** This is most commonly used to download dependencies automatically during build.
	**
	** If you want your builder program to link directly with libcurl instead of calling
	** out to the curl CLI tool, #define PIG_BUILD_ENABLE_LIB_CURL 1 and update your build.sh
	** routing shell script to do: PIG_BUILD_FLAGS="-lcurl"
*/

#ifndef _PIG_BUILD_HTTP_H
#define _PIG_BUILD_HTTP_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_file.h"
#include "pig_build_hash.h"
#include "pig_build_zip.h"

//NOTE: If you enable this then we expect to link with libcurl
//      On Linux/OSX we just need `-lcurl` to be passed as a compiler flag. Add this to your build.sh:
//        PIG_BUILD_FLAGS="-lcurl"
//      On Windows you'll need to add the include directory and the library directories, and link with libcurl.a.
//        You can download it from https://curl.se/windows/ and then add these options to your build.bat:
//        set PIG_BUILD_FLAGS=/I"[libcurl]\include" /link /LIBPATH:"[libcurl]\lib libcurl.a"
#ifndef PIG_BUILD_ENABLE_LIB_CURL
#define PIG_BUILD_ENABLE_LIB_CURL 0
#endif

#if PIG_BUILD_ENABLE_LIB_CURL
#include <curl/curl.h>
#endif

void DownloadFromUrl(Str url, Str filePath)
{
	#if PIG_BUILD_ENABLE_LIB_CURL
	{
		Str filePathNt = CopyStr(filePath, true);
		Str urlNt = CopyStr(url, true);
		FILE* fileHandle = fopen(filePathNt.chars, "wb");
		
		CURL* curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, urlNt.chars);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fileHandle);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects
		
		CURLcode curlResult = curl_easy_perform(curl);
		AssertFmt(curlResult == CURLE_OK, "Download from \"%.*s\" failed! Curl error: %s\n", StrPrint(url), curl_easy_strerror(curlResult));
		curl_easy_cleanup(curl);
		AssertFileExist(filePath, false);
		
		fclose(fileHandle);
		FreeStr(&filePathNt);
		FreeStr(&urlNt);
	}
	#else //!PIG_BUILD_ENABLE_LIB_CURL
	{
		Str urlNt = CopyStr(url);
		CliArgList args = EMPTY;
		AddArg(&args, "-Ls");
		AddArgStr(&args, "-o \"[VAL]\"", filePath);
		AddArg(&args, urlNt.chars);
		//TODO: Make the error message here much clearer! Is curl not installed? Did the URL return a error code? etc.
		RunCliProgramAndExitOnFailure(StrLit("curl"), "", &args, StrLit("Failed to download file using \"curl\" CLI tool!"));
		AssertFileExist(filePath, false);
	}
	#endif
}

void DownloadFromUrlAndCheck(Str url, Str filePath, u64 expectedSize, u64 expectedHash)
{
	DownloadFromUrl(url, filePath);
	EnsureFileSizeAndHash(filePath, expectedSize, expectedHash);
}

void DownloadAndExtractArchive(Str url, Str archiveFilePath, u64 expectedArchiveSize, u64 expectedArchiveHash, Str outputFolderPath, Str archiveDirToExtract)
{
	Str tempPath = AddSuffixToFileName(archiveFilePath, StrLit("_TEMP"));
	DownloadFromUrlAndCheck(url, tempPath, expectedArchiveSize, expectedArchiveHash);
	CopyFileToPath(tempPath, archiveFilePath, true);
	RemoveFile(tempPath);
	
	if (!DoesFolderExist(outputFolderPath))
	{
		Str outputFolderPathNt = CopyStr(outputFolderPath);
		mkdir(outputFolderPathNt.chars, FOLDER_PERMISSIONS);
		FreeStr(&outputFolderPathNt);
	}
	
	UnzipResult unzipResult = UnzipEntireArchiveInto(archiveFilePath, archiveDirToExtract, outputFolderPath);
	if (!unzipResult.success)
	{
		PrintLine_E("Failed to unpack archive %.*s, from \"%.*s\"", StrPrint(unzipResult.errorStr), StrPrint(url));
		exit(6);
	}
}

#endif //  _PIG_BUILD_HTTP_H
