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

void WalkDependencies(BuildFile* file)
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

int main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty);
	bool isMsvcInitialized = WasMsvcDevBatchRun();
	
	Str exeName = StrLit("game" EXE_EXT);
	
	CliArgs commonCompilerArgs = EMPTY;
	AddArg(&commonCompilerArgs, CL_NO_LOGO);
	AddArg(&commonCompilerArgs, CL_FULL_FILE_PATHS);
	AddArgNt(&commonCompilerArgs, CL_INCLUDE_DIR, "[ROOT]/include");
	AddArg(&commonCompilerArgs, CL_COMPILE);
	
	CliArgs commonLinkerArgs = EMPTY;
	AddArg(&commonLinkerArgs, LINK_NO_LOGO);
	AddArgStr(&commonLinkerArgs, LINK_OUTPUT_FILE, exeName);
	
	Array_BuildFile files = EMPTY;
	
	{
		FileIter iter = StartFileIter(StrLit("../include"));
		Str iterPath = Str_Empty_Const;
		bool iterIsFolder = false;
		while (StepFileIter(&iter, &iterPath, &iterIsFolder))
		{
			if (!iterIsFolder && StrAnyCaseEndsWith(iterPath, StrLit(".h")))
			{
				BuildFile* file = AddItemArray_BuildFile(&files);
				file->isHeader = true;
				file->path = GetFullPath(iterPath, '/');
				file->fileName = GetFileNamePart(file->path, /*includeExtension=*/true);
				// PrintLine("Found header \"%.*s\"", StrPrint(file->fileName));
				CalculateHash(file);
				WalkDependencies(file);
			}
		}
	}
	
	{
		FileIter iter = StartFileIter(StrLit("../src"));
		Str iterPath = Str_Empty_Const;
		bool iterIsFolder = false;
		while (StepFileIter(&iter, &iterPath, &iterIsFolder))
		{
			if (!iterIsFolder && StrAnyCaseEndsWith(iterPath, StrLit(".c")))
			{
				BuildFile* file = AddItemArray_BuildFile(&files);
				file->isHeader = false;
				file->path = GetFullPath(iterPath, '/');
				file->fileName = GetFileNamePart(file->path, /*includeExtension=*/true);
				file->objPath = ChangePathExtension(file->fileName, StrLit(OBJ_EXT), /*replaceSubExtensions=*/false);
				file->objExists = DoesFileExist(file->objPath);
				// PrintLine("Found source \"%.*s\"", StrPrint(file->fileName));
				CalculateHash(file);
				WalkDependencies(file);
			}
		}
	}
	
	WriteLine_E("[Walking dependencies...]");
	for (u64 fIndex = 0; fIndex < files.length; fIndex++)
	{
		BuildFile* file = &files.files[fIndex];
		if (!file->isHeader && !file->hasChanged && file->objExists)
		{
			StrArray walkList = EMPTY;
			StrArray walkedList = EMPTY;
			for (u64 dIndex = 0; dIndex < file->directDependencies.length; dIndex++)
			{
				AddStr(&walkList, file->directDependencies.strings[dIndex]);
			}
			
			file->hasDependentChanged = false;
			while (walkList.length > 0 && !file->hasDependentChanged)
			{
				Str nextFileName = CopyStr(walkList.strings[0]);
				RemoveStrAtIndex(&walkList, 0);
				BuildFile* walkFile = TryFindFile(&files, nextFileName);
				if (walkFile != nullptr)
				{
					if (walkFile->hasChanged)
					{
						PrintLine("Changes in \"%.*s\" mean that \"%.*s\" needs to recompile!", StrPrint(nextFileName), StrPrint(file->fileName));
						file->hasDependentChanged = true;
						break;
					}
					for (u64 dIndex = 0; dIndex < walkFile->directDependencies.length; dIndex++)
					{
						Str walkFileDep = walkFile->directDependencies.strings[dIndex];
						if (!ContainsStr(&walkList, walkFileDep, true) && !ContainsStr(&walkedList, walkFileDep, true))
						{
							AddStr(&walkList, walkFileDep);
						}
					}
				}
				else { PrintLine_E("Couldn't find \"%.*s\"", StrPrint(nextFileName)); }
				FreeStr(&nextFileName);
			}
			
			FreeStrArray(&walkList);
			FreeStrArray(&walkedList);
			
			// if (!file->hasDependentChanged) { PrintLine("Don't need to compile \"%.*s\"", StrPrint(file->fileName)); }
		}
		
	}
	
	// Remove all objects that had a change or dependenct change
	// That way they continue to get compiled even if we mark all
	// the dependents as "up-to-date" in subsequent compile attempts
	WriteLine_E("[Deleting objects...]");
	for (u64 fIndex = 0; fIndex < files.length; fIndex++)
	{
		BuildFile* file = &files.files[fIndex];
		if (!file->isHeader)
		{
			if (!file->objExists || file->hasChanged || file->hasDependentChanged)
			{
				if (DoesFileExist(file->objPath)) { RemoveFile(file->objPath); }
				if (DoesFileExist(file->hashPath)) { RemoveFile(file->hashPath); }
			}
		}
	}
	
	WriteLine_E("[Compiling objects...]");
	bool anyObjectsCompiled = false;
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
				InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
				PrintLine("[%sompiling \"%.*s\"...]", file->objExists ? "Rec" : "C", StrPrint(file->fileName));
				
				CliArgs args = EMPTY;
				AddArgList(&args, &commonCompilerArgs);
				AddArgStr(&args, CLI_QUOTED_ARG, file->path);
				AddArgStr(&args, CL_OBJ_FILE, file->objPath);
				
				RemoveFile(file->objPath);
				RunCliProgramAndExitOnFailure(StrLit("cl"), &args, FormatStr("Failed to compile \"%.*s\"", StrPrint(file->fileName)));
				AssertFileExist(file->objPath, true);
				
				CreateAndWriteFile(file->hashPath, FormatStr("0x%08llX", file->newHash), true);
				
				anyObjectsCompiled = true;
			}
		}
	}
	
	if (!DoesFileExist(exeName) || anyObjectsCompiled)
	{
		InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
		PrintLine("[%sinking \"%.*s\"...]", DoesFileExist(exeName) ? "Rel" : "L", StrPrint(exeName));
		
		CliArgs args = EMPTY;
		AddArgList(&args, &commonLinkerArgs);
		for (u64 fIndex = 0; fIndex < files.length; fIndex++)
		{
			BuildFile* file = &files.files[fIndex];
			if (!file->isHeader) { AddArgStr(&args, CLI_QUOTED_ARG, file->objPath); }
		}
		RunCliProgramAndExitOnFailure(StrLit("link"), &args, FormatStr("Failed to link \"%.*s\"", StrPrint(exeName)));
		AssertFileExist(exeName, true);
	}
	else
	{
		WriteLine("[Nothing to compile!]");
	}
	
	return 0;
}
