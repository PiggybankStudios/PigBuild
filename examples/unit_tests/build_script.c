/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\01\2026
Description: 
	** This file is not really a build script, but by building and running it
	** we can test various things inside the Pig Build header files.
*/

#define PIG_BUILD_FOLDER_PATH "../../.."
#include "pig_build.h"

#include "tests_scan.c"
#include "tests_str_list.c"
#include "tests_unicode.c"

int main(int argc, const char* argv[])
{
	StrArray sourceFolders = EMPTY;
	AddStrLit(&sourceFolders, "..");
	AddStrLit(&sourceFolders, PIG_BUILD_FOLDER_PATH "/src");
	RecompileIfNeeded(sourceFolders);
	Str pigBuildFolder = StrLit(PIG_BUILD_FOLDER_PATH);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	
	// RunTests_StrList();
	// RunTests_Scan();
	RunTests_Unicode();
	
	return 0;
}
