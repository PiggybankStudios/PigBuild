/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** This is a basic build script. It compiles a single .c file into an
	** executable for Windows, Linux, and Mac
*/

//Enable this to have all calls to RunCliProgram printed to the debug output
#define PIG_BUILD_PRINT_SYS_CMDS 0

#include "pig_build.h"

// Enable this to run the built executable after build succeeds
#define RUN_AFTER_BUILD   1

// If you have Windows Subsystem for Linux (WSL) installed on Windows, and clang
// installed inside, then you can enable this to compile for Linux using WSL
#define CROSS_COMPILE_WITH_WSL   0

// +--------------------------------------------------------------+
// |                            Build                             |
// +--------------------------------------------------------------+
void Build(Str routingPrefix, Str compiler, Str exeName, bool windowsShell)
{
	bool isMsvcCompiler = StrAnyCaseEquals(compiler, StrLit("cl"));
	if (isMsvcCompiler)
	{
		// If VsDevCmd.bat (or older vcvarsall.bat) wasn't run, then we need to run it ourselves before we can use the MSVC compiler
		bool isMsvcInitialized = WasMsvcDevBatchRun();
		InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
	}
	
	// NOTE: See "pig_build_cli_flags.h" for these compiler flag #defines
	CliArgs args = EMPTY;
	args.pathSepChar = windowsShell ? '\\' : '/';
	AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/main.c");
	AddArgStr(&args, isMsvcCompiler ? CL_BINARY_FILE     : CLANG_OUTPUT_FILE, exeName);
	AddArg(&args,    isMsvcCompiler ? CL_FULL_FILE_PATHS : CLANG_FULL_FILE_PATHS);
	if (isMsvcCompiler) { AddArgNt(&args, CL_OBJ_FILE, "main.obj"); }
	if (isMsvcCompiler) { AddArg(&args, CL_NO_LOGO); }
	
	WriteLine("[Building...]");
	
	int exitCode = RunCliProgram(JoinStrings2(routingPrefix, compiler), &args);
	AssertFmt(exitCode == 0, "Compiler returned: %d", exitCode);
	
	WriteLine("[Done!]\n");
}

// +--------------------------------------------------------------+
// |                             Main                             |
// +--------------------------------------------------------------+
int main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty); //See "pig_build_recompile.h" for how this works
	WriteLine("=========Build Script=========\n");
	
	bool buildingForWindows = (BUILDING_ON_WINDOWS && !CROSS_COMPILE_WITH_WSL);
	bool crossCompiling     = (BUILDING_ON_WINDOWS &&  CROSS_COMPILE_WITH_WSL);
	Str exeName = MakeStrNt(buildingForWindows ? "hello_world.exe" : "hello_world");
	Str wslPrefix = crossCompiling ? StrLit("wsl ") : Str_Empty;
	Str compiler = buildingForWindows ? StrLit("cl") : StrLit("clang");
	
	Build(wslPrefix, compiler, exeName, buildingForWindows);
	
	if (RUN_AFTER_BUILD)
	{
		Str sameFolderExePrefix = buildingForWindows ? Str_Empty : StrLit("./");
		PrintLine("[Running \"%.*s\"...]", StrPrint(exeName));
		RunCliProgram(JoinStrings3(wslPrefix, sameFolderExePrefix, exeName), nullptr);
		WriteLine("[Done!]");
	}
	
	WriteLine("\n=======Build Script END=======");
	
	return 0;
}
