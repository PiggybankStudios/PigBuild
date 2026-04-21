/*
File:   pig_build_http.h
Author: Taylor Robbins
Date:   04\21\2026
Description:
	** Holds DownloadFromUrl which helps you download files from particular
	** http(s) urls using libcurl (or curl CLI) on Unix platforms and WinHTTP on Windows.
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

//NOTE: If you enable this then we expect `-lcurl` to be passed as a compiler flag,
//      use PIG_BUILD_FLAGS in your routing shell script to add compiler flags for your build script
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
		CliArgList args = EMPTY;
		AddArg(&args, "-Ls");
		AddArgStr(&args, "-o \'[VAL]\'", filePath);
		AddArgStr(&args, CLI_SINGLE_QUOTED_ARG, url);
		//TODO: Make the error message here much clearer! Is curl not installed? Did the URL return a error code? etc.
		RunCliProgramAndExitOnFailure(StrLit("curl"), "", &args, StrLit("Failed to download file using \"curl\" CLI tool!"));
		AssertFileExist(filePath, false);
	}
	#endif
}

#endif //  _PIG_BUILD_HTTP_H
