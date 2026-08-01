/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** Many build systems support the idea of compiling .obj/.o files only when
	** the source file they are based on has changed since the last time it was
	** built. This example shows how you would do that sort of logic inside your
	** build script using basic FnvHash64 of file contents and #include walking
*/

//TODO: Finish the actual game logic so it does something somewhat interesting
//TODO: Factor build_script.c changes into account!
//TODO: Put hash and object files in folders

#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

typedef struct BuildFile BuildFile;
struct BuildFile
{
	bool isHeader;
	Str path;
	Str fileName;
	Str objPath;
	bool objExists;
	
	Str hashPath;
	u64 prevHash;
	u64 newHash;
	bool hasChanged;
	
	StrArray directDependencies;
	bool hasDependentChanged;
};
TYPED_ARRAY(Array_BuildFile, BuildFile, files);

BuildFile* TryFindFile(Array_BuildFile* files, Str fileName)
{
	for (u64 fIndex = 0; fIndex < files->length; fIndex++)
	{
		if (StrAnyCaseEquals(files->files[fIndex].fileName, fileName))
		{
			return &files->files[fIndex];
		}
	}
	return nullptr;
}

void CalculateHash(BuildFile* file)
{
	file->hashPath = JoinStrings2(file->fileName, StrLit(".hash"));
	file->newHash = FnvHashFile(file->path, FNV_HASH_BASE_U64);
	if ((file->isHeader || file->objExists) && DoesFileExist(file->hashPath))
	{
		Str hashFileContents = ReadEntireFile(file->hashPath);
		if (!TryParseHexU64(hashFileContents, &file->prevHash))
		{
			PrintLine_E("WARNING: Failed to parse hash file contents \"%.*s\": %.*s", StrPrint(file->hashPath), StrPrint(hashFileContents));
		}
		FreeStr(&hashFileContents);
		
		file->hasChanged = (file->prevHash != file->newHash);
		if (file->hasChanged) { PrintLine("Hash changed for \"%.*s\": 0x%08llX != 0x%08llX", StrPrint(file->fileName), file->prevHash, file->newHash); }
	}
	else
	{
		PrintLine("obj/hash don't exist for \"%.*s\" ", StrPrint(file->fileName));
		file->hasChanged = true;
	}
}

void FindIncludeLines(BuildFile* file)
{
	Str sourceCode = ReadEntireFile(file->path);
	LineParser parser = NewLineParser(sourceCode);
	Str line = Str_Empty_Const;
	u64 lineIndex = 0;
	while (LineParserGetLine(&parser, &line))
	{
		line = TrimWhitespace(line);
		if (StrExactEndsWith(line, StrLit("\r"))) { line = StrSlice(line, 0, line.length-1); }
		if (StrExactStartsWith(line, StrLit("#include")))
		{
			line = TrimWhitespace(StrSliceFrom(line, 8));
			if (StrExactStartsWith(line, StrLit("\"")) && StrExactEndsWith(line, StrLit("\"")))
			{
				Str includePath = StrSlice(line, 1, line.length-1);
				// PrintLine("\t%.*s depends on %.*s", StrPrint(file->fileName), StrPrint(includePath));
				AddStr(&file->directDependencies, includePath);
			}
		}
		lineIndex++;
	}
	if (file->directDependencies.length == 0)
	{
		// PrintLine("\t%.*s has no #include lines in %llu lines", StrPrint(file->fileName), lineIndex);
	}
	FreeStr(&sourceCode);
}

void FindSourceFilesInFolder(Array_BuildFile* files, Str folder, bool recursive)
{
	FileIter iter = StartFileIter(folder);
	Str iterPath = Str_Empty_Const;
	bool iterIsFolder = false;
	while (StepFileIter(&iter, &iterPath, &iterIsFolder))
	{
		if (iterIsFolder)
		{
			if (recursive)
			{
				FindSourceFilesInFolder(files, iterPath, recursive);
			}
		}
		else
		{
			if (StrAnyCaseEndsWith(iterPath, StrLit(".h")) ||
				StrAnyCaseEndsWith(iterPath, StrLit(".c")))
			{
				BuildFile* file = AddItemArray_BuildFile(files);
				file->isHeader = StrAnyCaseEndsWith(iterPath, StrLit(".h"));
				file->path = GetFullPath(iterPath, '/');
				file->fileName = GetFileNamePart(file->path, /*includeExtension=*/true);
				if (!file->isHeader)
				{
					file->objPath = ChangePathExtension(file->fileName, StrLit(OBJ_EXT), /*replaceSubExtensions=*/false);
					file->objExists = DoesFileExist(file->objPath);
				}
				// PrintLine("Found %s \"%.*s\"", file->isHeader ? "header" : "source", StrPrint(file->fileName));
				CalculateHash(file); //fills out hashPath, prevHash, newHash, and hasChanged
				FindIncludeLines(file); //fills out directDependencies
			}
		}
	}
}

bool CheckIfDependencyChanged(Array_BuildFile* files, BuildFile* file)
{
	bool result = false;
	StrArray toWalkList = EMPTY;
	StrArray walkedList = EMPTY;
	AddStrArray(&toWalkList, &file->directDependencies);
	
	while (toWalkList.length > 0 && !result)
	{
		Str nextFileName = CopyStr(toWalkList.strings[0]);
		RemoveStrAtIndex(&toWalkList, 0);
		BuildFile* walkFile = TryFindFile(files, nextFileName);
		if (walkFile != nullptr)
		{
			if (walkFile->hasChanged)
			{
				PrintLine("Changes in \"%.*s\" mean that \"%.*s\" needs to recompile!", StrPrint(nextFileName), StrPrint(file->fileName));
				result = true;
				break;
			}
			for (u64 dIndex = 0; dIndex < walkFile->directDependencies.length; dIndex++)
			{
				Str walkFileDep = walkFile->directDependencies.strings[dIndex];
				if (!ContainsStr(&toWalkList, walkFileDep, true) && !ContainsStr(&walkedList, walkFileDep, true))
				{
					AddStr(&toWalkList, walkFileDep);
				}
			}
		}
		else { PrintLine_E("Couldn't find \"%.*s\"", StrPrint(nextFileName)); }
		FreeStr(&nextFileName);
	}
	
	FreeStrArray(&toWalkList);
	FreeStrArray(&walkedList);
	
	return result;
}

int main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty);
	IF_WINDOWS(bool isMsvcInitialized = WasMsvcDevBatchRun());
	
	Str exeName = StrLit("game" EXE_EXT);
	Str compilerName = BUILDING_ON_WINDOWS ? StrLit("cl") : StrLit("clang");
	Str linkerName = BUILDING_ON_WINDOWS ? StrLit("link") : StrLit("clang");
	bool usingMsvc = BUILDING_ON_WINDOWS;
	
	CliArgs commonCompilerArgs = EMPTY;
	AddTaggedArg(&commonCompilerArgs,   "cl",    CL_NO_LOGO);
	AddTaggedArg(&commonCompilerArgs,   "cl",    CL_FULL_FILE_PATHS);
	AddTaggedArg(&commonCompilerArgs,   "clang", CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&commonCompilerArgs, "cl",    CL_INCLUDE_DIR,    "[ROOT]/include");
	AddTaggedArgNt(&commonCompilerArgs, "clang", CLANG_INCLUDE_DIR, "[ROOT]/include");
	AddTaggedArg(&commonCompilerArgs,   "cl",    CL_COMPILE);
	AddTaggedArg(&commonCompilerArgs,   "clang", CLANG_COMPILE);
	
	CliArgs commonLinkerArgs = EMPTY;
	AddTaggedArg(&commonLinkerArgs,    "link",  LINK_NO_LOGO);
	AddTaggedArgStr(&commonLinkerArgs, "link",  LINK_OUTPUT_FILE, exeName);
	AddTaggedArgStr(&commonLinkerArgs, "clang", CLANG_OUTPUT_FILE, exeName);
	AddTaggedArg(&commonLinkerArgs,    "clang", CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&commonLinkerArgs,  "clang", CLANG_INCLUDE_DIR, "[ROOT]/include");
	
	Array_BuildFile files = EMPTY;
	FindSourceFilesInFolder(&files, StrLit("../include"), /*recursive=*/false);
	FindSourceFilesInFolder(&files, StrLit("../src"), /*recursive=*/false);
	
	WriteLine_E("[Checking dependencies...]");
	for (u64 fIndex = 0; fIndex < files.length; fIndex++)
	{
		BuildFile* file = &files.files[fIndex];
		if (!file->isHeader && !file->hasChanged && file->objExists)
		{
			file->hasDependentChanged = CheckIfDependencyChanged(&files, file);
			// if (!file->hasDependentChanged) { PrintLine("Don't need to compile \"%.*s\"", StrPrint(file->fileName)); }
		}
	}
	
	// Remove all objects for files that need to get recompiled. This ensures we
	// continue recompiling them even if we exit this build attempt early
	// (if there are build errors in one of the files)
	bool numFilesThatNeedToBeCompiled = 0;
	for (u64 fIndex = 0; fIndex < files.length; fIndex++)
	{
		BuildFile* file = &files.files[fIndex];
		if (!file->isHeader)
		{
			if (!file->objExists || file->hasChanged || file->hasDependentChanged)
			{
				if (numFilesThatNeedToBeCompiled == 0) { WriteLine_E("[Deleting objects...]"); }
				numFilesThatNeedToBeCompiled++;
				
				if (DoesFileExist(file->objPath)) { RemoveFile(file->objPath); }
				if (DoesFileExist(file->hashPath)) { RemoveFile(file->hashPath); }
			}
		}
	}
	
	if (numFilesThatNeedToBeCompiled > 0)
	{
		WriteLine_E("[Compiling objects...]");
		for (u64 fIndex = 0; fIndex < files.length; fIndex++)
		{
			BuildFile* file = &files.files[fIndex];
			if (file->isHeader)
			{
				if (file->hasChanged || !DoesFileExist(file->hashPath))
				{
					CreateAndWriteFile(file->hashPath, FormatStr("0x%08llX", file->newHash), true);
				}
			}
			else
			{
				if (!file->objExists || file->hasChanged || file->hasDependentChanged)
				{
					IF_WINDOWS(InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized));
					PrintLine("[%sompiling \"%.*s\"...]", file->objExists ? "Rec" : "C", StrPrint(file->fileName));
					
					CliArgs args = EMPTY;
					AddArgList(&args, &commonCompilerArgs);
					AddArgStr(&args, CLI_QUOTED_ARG, file->path);
					AddTaggedArgStr(&args, "cl", CL_OBJ_FILE, file->objPath);
					AddTaggedArgStr(&args, "clang", CLANG_OUTPUT_FILE, file->objPath);
					
					StrArray tags = EMPTY;
					AddStr(&tags, compilerName);
					AddStrLit(&tags, "compiling");
					AddStrLit(&tags, BUILDING_ON_NAME);
					
					RunCliProgramAndExitOnFailureTags(compilerName, tags, &args, FormatStr("Failed to compile \"%.*s\"", StrPrint(file->fileName)));
					AssertFileExist(file->objPath, true);
					
					CreateAndWriteFile(file->hashPath, FormatStr("0x%08llX", file->newHash), true);
				}
			}
		}
	}
	
	if (!DoesFileExist(exeName) || numFilesThatNeedToBeCompiled > 0)
	{
		IF_WINDOWS(InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized));
		PrintLine("[%sinking \"%.*s\"...]", DoesFileExist(exeName) ? "Rel" : "L", StrPrint(exeName));
		
		CliArgs args = EMPTY;
		AddArgList(&args, &commonLinkerArgs);
		for (u64 fIndex = 0; fIndex < files.length; fIndex++)
		{
			BuildFile* file = &files.files[fIndex];
			if (!file->isHeader) { AddArgStr(&args, CLI_QUOTED_ARG, file->objPath); }
		}
		
		StrArray tags = EMPTY;
		AddStr(&tags, linkerName);
		AddStrLit(&tags, "linking");
		AddStrLit(&tags, BUILDING_ON_NAME);
		
		RunCliProgramAndExitOnFailureTags(linkerName, tags, &args, FormatStr("Failed to link \"%.*s\"", StrPrint(exeName)));
		AssertFileExist(exeName, true);
	}
	else
	{
		WriteLine("[Nothing to compile!]");
	}
	
	return 0;
}
