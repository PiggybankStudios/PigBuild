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

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_file.h"
#include "pig_build_hash.h"

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
#ifndef PIG_BUILD_FOLDER_PATH
#define PIG_BUILD_FOLDER_PATH "../" PIG_BUILD_FOLDER_NAME
#endif
#define MSVC_ENVIRONMENT_TXT_PATH "msvc_environment.txt"

// Call this function at the top of your build_scropt.c main function.
// If the source code for build_script.c changes OR if any of the helper files in the build_script/ folder
// change then your builder will be re-compiled (this function will call exit(REBUILD_EXIT_CODE);)
void RecompileIfNeeded(StrArray buildScriptDependencies)
{
	StrArray dependencies = EMPTY;
	AddStrArray(&dependencies, &buildScriptDependencies);
	
	// For convenience we are going to add pig_build/src and it's subfolders automatically if the build_script didn't mention them
	Str pigBuildFullPath = GetFullPath(StrLit(PIG_BUILD_ROOT), '/');
	if (DoesFolderExist(pigBuildFullPath))
	{
		Str srcFolderPath = JoinPathsLit(pigBuildFullPath, "/src");
		Str optionalFolderPath = JoinPathsLit(pigBuildFullPath, "/optional");
		Str thirdPartyFolderPath = JoinPathsLit(pigBuildFullPath, "/third_party");
		bool dependenciesContainPigBuildSrc = false;
		bool dependenciesContainPigBuildSrcOptional = false;
		bool dependenciesContainPigBuildSrcThirdParty = false;
		for (u64 fIndex = 0; fIndex < dependencies.length; fIndex++)
		{
			if (DoesFolderExist(dependencies.strings[fIndex]))
			{
				Str dependencyFullPath = GetFullPath(dependencies.strings[fIndex], '/');
				if (StrAnyCaseEquals(dependencyFullPath, srcFolderPath)) { dependenciesContainPigBuildSrc = true; }
				if (StrAnyCaseEquals(dependencyFullPath, optionalFolderPath)) { dependenciesContainPigBuildSrcOptional = true; }
				if (StrAnyCaseEquals(dependencyFullPath, thirdPartyFolderPath)) { dependenciesContainPigBuildSrcThirdParty = true; }
				FreeStr(&dependencyFullPath);
			}
		}
		if (!dependenciesContainPigBuildSrc)
		{
			// PrintLine("Adding %.*s to dependencies", StrPrint(srcFolderPath));
			AddStr(&dependencies, StrLit(PIG_BUILD_ROOT "/src"));
		}
		if (!dependenciesContainPigBuildSrcOptional)
		{
			// PrintLine("Adding %.*s to dependencies", StrPrint(optionalFolderPath));
			AddStr(&dependencies, StrLit(PIG_BUILD_ROOT "/src/optional"));
		}
		if (!dependenciesContainPigBuildSrcThirdParty)
		{
			// PrintLine("Adding %.*s to dependencies", StrPrint(thirdPartyFolderPath));
			AddStr(&dependencies, StrLit(PIG_BUILD_ROOT "/src/third_party"));
		}
	}
	
	Str buildScriptFilePath = StrLit_Const(BUILD_SCRIPT_SOURCE_PATH);
	Str buildScriptContents = Str_Empty_Const;
	if (!TryReadFile(buildScriptFilePath, &buildScriptContents))
	{
		PrintLine("Failed to read script contents to check if it's changed. Looking at \"%.*s\"", StrPrint(buildScriptFilePath));
		exit(REBUILD_EXIT_CODE);
	}
	u64 buildScriptHash = FnvHash(buildScriptContents.chars, buildScriptContents.length, FNV_HASH_BASE_U64);
	free(buildScriptContents.chars);
	for (u64 fIndex = 0; fIndex < dependencies.length; fIndex++)
	{
		Str folderOrFilePath = dependencies.strings[fIndex];
		if (DoesFolderExist(folderOrFilePath))
		{
			FileIter fileIter = StartFileIter(folderOrFilePath);
			Str fileIterPath = Str_Empty_Const;
			bool fileIterIsFolder = false;
			while (StepFileIter(&fileIter, &fileIterPath, &fileIterIsFolder))
			{
				//TODO: We should probably only hash files that have extensions like ".c" or ".h" or ".cpp" or etc.
				if (!fileIterIsFolder)
				{
					Str sourceFileContents = Str_Empty_Const;
					if (!TryReadFile(fileIterPath, &sourceFileContents))
					{
						PrintLine("Failed to read build system file contents to check if it's changed. Looking at \"%.*s\"", StrPrint(fileIterPath));
						exit(REBUILD_EXIT_CODE);
					}
					buildScriptHash = FnvHash(sourceFileContents.chars, sourceFileContents.length, buildScriptHash);
					free(sourceFileContents.chars);
				}
			}
		}
		else if (DoesFileExist(folderOrFilePath))
		{
			Str sourceFileContents = Str_Empty_Const;
			if (!TryReadFile(folderOrFilePath, &sourceFileContents))
			{
				PrintLine("Failed to read build system file contents to check if it's changed. Looking at \"%.*s\"", StrPrint(folderOrFilePath));
				exit(REBUILD_EXIT_CODE);
			}
			buildScriptHash = FnvHash(sourceFileContents.chars, sourceFileContents.length, buildScriptHash);
			free(sourceFileContents.chars);
		}
		else { PrintLine_E("WARNING: A folder or file passed to RecompileIfNeeded doesn't exist: \"%.*s\"", StrPrint(folderOrFilePath)); }
	}
	
	Str buildHashFilePath = StrLit_Const(BUILD_SCRIPT_HASH_PATH);
	u64 savedHash = 0;
	bool hashesMatch = false;
	bool hashFileExisted = false;
	Str buildHashContents = Str_Empty_Const;
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
		Str buildScriptHashString = ConvertU64ToHexStr(buildScriptHash, true);
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
