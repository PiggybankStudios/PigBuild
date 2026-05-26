/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** This is a basic build script. It compiles a single .c file into an executable
	** for Windows, Linux, and Mac
*/

//Enable this to have all calls to RunCliProgram printed to the debug output
#define PIG_BUILD_PRINT_SYS_CMDS 0

#include "pig_build.h"

int main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty); //See "pig_build_reocmpile.h" for how this works
	
	bool usingMsvc = BUILDING_ON_WINDOWS;
	bool usingClang = !usingMsvc;
	Str compilerName = MakeStrNt(usingMsvc ? "cl" : "clang");
	Str exeName = MakeStrNt(BUILDING_ON_WINDOWS ? "hello_world.exe" : "hello_world");
	
	if (usingMsvc)
	{
		// If VsDevCmd.bat (or older vcvarsall.bat) wasn't run, then we need to run it ourselves before we can use the MSVC compiler
		bool isMsvcInitialized = WasMsvcDevBatchRun();
		InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
	}
	
	WriteLine("[Building...]");
	
	// NOTE: See "pig_build_cli_flags.h" for these compiler flag #defines
	CliArgs args = EMPTY;
	AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/main.c");
	if (usingMsvc)
	{
		AddArgNt(&args, CL_OBJ_FILE, "main.obj");
		AddArgStr(&args, CL_BINARY_FILE, exeName);
		AddArg(&args, CL_NO_LOGO);
	}
	else
	{
		AddArgStr(&args, CLANG_OUTPUT_FILE, exeName);
	}
	
	int exitCode = RunCliProgram(compilerName, &args);
	AssertFmt(exitCode == 0, "Compiler returned: %d", exitCode);
	
	WriteLine("[DONE!]");
	
	PrintLine("Running \"%.*s\"...\n", StrPrint(exeName));
	RunCliProgram(exeName, nullptr);
	
	return 0;
}
