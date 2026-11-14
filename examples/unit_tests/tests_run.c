/*
File:   tests_run.c
Author: Taylor Robbins
Date:   11\13\2026
Description: 
	** Holds tests for Scan code inside "pig_build_run.h"
*/

void RunTests_Run()
{
	pig_run("ls");
	pig_run("ls -l");
	
	CliArgs args1 = EMPTY; AddArg(&args1, "-l"); AddArg(&args1, "-a");
	pig_run("ls", .args=args1);
	
	CliArgs args2 = EMPTY; AddTaggedArg(&args2, "clang", "-c"); AddTaggedArg(&args2, "cl", "/c");
	pig_run("clang", .args=args2, .exe_tags=MakeStrArrayVa("clang"), .dry_run=true);
	pig_run("cl", .args=args2, .exe_tags=MakeStrArrayVa("cl"), .dry_run=true);
	
	pig_run("clang abcdf",
		.error_message="Clang didn't like the arguments we passed to it!",
		.dont_exit_on_failure=true
	);
}
