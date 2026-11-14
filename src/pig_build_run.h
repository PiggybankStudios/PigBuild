/*
File:   pig_build_run.h
Author: Taylor Robbins
Date:   11\13\2026
Description:
	** Holds the primary interface for running external applications,
	** getting their exit code when they finish, optionally routing their
	** stdout\stdin to\from various sources, running them asynchronously,
	** and killing or monitoring their state
*/

#ifndef _PIG_BUILD_RUN_H
#define _PIG_BUILD_RUN_H

#include "pig_build_base.h"
// #include "pig_build_base.h"

bool exit_on_run_failures = true;

typedef struct pig_run_args pig_run_args;
struct pig_run_args
{
	CliArgs args;
	StrArray exe_tags;
	const char* error_message;
	bool dont_exit_on_failure;
	bool print_cmd;
	bool print_exit_code;
	bool dry_run;
	//TODO: silent? no_stdout? no_stderr?
};
typedef struct { pig_run_args wrapped; } pig_run_args_wrapper;
#define pig_run(executablePathNt, ...) pig_run_(MakeStrNt(executablePathNt), INIT(pig_run_args_wrapper){{ __VA_ARGS__ }}.wrapped)

int pig_run_(Str executablePath, pig_run_args args)
{
	Str joinedExecutableAndArgsStr = (args.args.array.length > 0) ? FilterAndJoinCliArgsList(executablePath, &args.args, &args.exe_tags) : CopyStr(executablePath);
	
	if (PigBuildDebugMode || args.print_cmd)
	{
		u64 numMatchingArgs = GetNumMatchingArgs(&args.args, &args.exe_tags);
		if (args.exe_tags.length > 0)
		{
			PrintLine(">> Matched %llu/%llu argument%s on %llu tag%s for %.*s",
				numMatchingArgs, args.args.array.length, Plural(numMatchingArgs, "s"),
				args.exe_tags.length, Plural(args.exe_tags.length, "s"),
				StrPrint(executablePath)
			);
			// for (u64 tIndex = 0; tIndex < args.exe_tags.length; tIndex++){ PrintLine("\tTag[%llu]: \"%.*s\"", tIndex, StrPrint(args.exe_tags.strings[tIndex])); }
		}
		else
		{
			u64 numArgs = (args.args.array.length > 0) ? args.args.array.length : 0;
			PrintLine(">> No tags given. All %llu arg%s match", numArgs, Plural(numArgs, "s"));
		}
		PrintLine(">> %s", joinedExecutableAndArgsStr.chars);
	}
	
	if (args.dry_run)
	{
		if (!PigBuildDebugMode && !args.print_cmd) { PrintLine(">> %s", joinedExecutableAndArgsStr.chars); }
		return 0;
	}
	else
	{
		fflush(stdout);
		fflush(stderr);
		
		int resultCode = system(joinedExecutableAndArgsStr.chars);
		#if BUILDING_ON_UNIX
		resultCode = WEXITSTATUS(resultCode);
		#endif
		
		if (PigBuildDebugMode || args.print_exit_code) { PrintLine("Exit Code: %d", resultCode); }
		
		if (resultCode != 0)
		{
			if (!IsEmptyNt(args.error_message)) { PrintLine_E("%s", args.error_message); }
			
			if (resultCode != 0 &&
				!args.dont_exit_on_failure &&
				exit_on_run_failures)
			{
				AssertFmt(resultCode != 0,
					"ERROR: \"%.*s\" exited with code %d\nCmd >> %s",
					StrPrint(executablePath),
					resultCode,
					joinedExecutableAndArgsStr.chars
				);
			}
		}
		
		FreeStr(&joinedExecutableAndArgsStr);
		
		return resultCode;
	}
}

#endif //  _PIG_BUILD_RUN_H
