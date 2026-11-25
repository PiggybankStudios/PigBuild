/*
File:   tests_run.c
Author: Taylor Robbins
Date:   11\13\2026
Description: 
	** Holds tests for Scan code inside "pig_build_run.h"
*/

void RunTests_Run()
{
	#if BUILDING_ON_WINDOWS
	
	Str dirOutput = Str_Empty_Const;
	pig_run("dir", .run_in_shell=true, .stdoutStr=&dirOutput);
	PrintLine("dir printed %llu bytes", dirOutput.length);
	
	#elif BUILDING_ON_UNIX
	
	Str lsOutput = Str_Empty_Const;
	pig_run("ls", .run_in_shell=true, .stdoutStr=&lsOutput);
	PrintLine("ls printed %llu bytes\n\n", lsOutput.length);
	
	pig_run("ls -l", .run_in_shell=true, .stdoutStr=&lsOutput);
	PrintLine("ls -l printed %llu bytes\n\n", lsOutput.length);
	
	CliArgs args1 = EMPTY; AddArg(&args1, "-l"); AddArg(&args1, "-a");
	pig_run("ls", .run_in_shell=true, .args=args1);
	
	#endif
	
	CliArgs args2 = EMPTY; AddTaggedArg(&args2, "clang", "-c"); AddTaggedArg(&args2, "cl", "/c");
	#if BUILDING_ON_UNIX
	pig_run("clang", .args=args2, .exe_tags=MakeStrArrayVa("clang"), .dry_run=true);
	#elif BUILDING_ON_WINDOWS
	pig_run("cl", .args=args2, .exe_tags=MakeStrArrayVa("cl"), .dry_run=true);
	#endif
	
	#if BUILDING_ON_UNIX
	pig_run("clang abcdf",
		.error_message="Clang returned an error but we didn't exit because we set tolerate_failure",
		.tolerate_failure=true
	);
	
	pig_run_tolerate_failures = false;
	pig_run("clang abcdf", .error_message="Clang returned an error but we aren't exiting because pig_run_tolerate_failures global is disabled!");
	pig_run_tolerate_failures = true;
	
	pig_run("clang abcdf", .error_message="Goodbye :)");
	AssertMsg(false, "Unreachable!");
	#endif //BUILDING_ON_UNIX
}
