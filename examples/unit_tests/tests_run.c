/*
File:   tests_run.c
Author: Taylor Robbins
Date:   11\13\2026
Description: 
	** Holds tests for Scan code inside "pig_build_run.h"
*/

#if BUILDING_ON_WINDOWS
void CommandPromptEchoTest(bool useStdErr, Str stringToEcho, Str expectedEcho)
{
	if (IsEmptyStr(expectedEcho)) { expectedEcho = JoinStrings2(stringToEcho, StrLit("\r\n")); }
	
	CliArgs args = EMPTY;
	AddArgStr(&args, CLI_UNQUOTED_ARG, stringToEcho);
	
	Str echoOutput = Str_Empty_Const;
	pig_run(useStdErr ? "1>&2 echo" : "echo",
		.args=args,
		.run_in_shell=true,
		.stdoutStr=useStdErr ? nullptr : &echoOutput,
		.stderrStr=useStdErr ? &echoOutput : nullptr
	);
	
	Str expectedEchoHex = ConvertNonAsciiGlyphCharsToHex(expectedEcho, true, false, "<%02X>");
	Str echoOutputHex = ConvertNonAsciiGlyphCharsToHex(echoOutput, true, false, "<%02X>");
	AssertFmt(StrExactEquals(echoOutput, expectedEcho), "Expected output to be\n%.*s\nnot\n%.*s", StrPrint(expectedEchoHex), StrPrint(echoOutputHex))
	// PrintLine("Echo gave us%s: %.*s", useStdErr ? " (Through stderr)" : "", StrPrint(echoOutputHex));
	
	FreeStr(&echoOutputHex);
	FreeStr(&echoOutput);
}
#endif //BUILDING_ON_WINDOWS

void RunTests_Run()
{
	Assert(pig_run("exit 0",   .run_in_shell=true) == 0);
	Assert(pig_run("exit 1",   .run_in_shell=true, .tolerate_failure=true) == 1);
	Assert(pig_run("exit 42",  .run_in_shell=true, .tolerate_failure=true) == 42);
	Assert(pig_run("exit 255", .run_in_shell=true, .tolerate_failure=true) == 255);
	Assert(pig_run("exit 256", .run_in_shell=true, .tolerate_failure=true) == (BUILDING_ON_WINDOWS ? 256 : 0)); //NOTE: Unix truncates to 8-bit exit status
	
	#if BUILDING_ON_WINDOWS
	
	Str dirOutput = Str_Empty_Const;
	pig_run("dir", .run_in_shell=true, .stdoutStr=&dirOutput);
	PrintLine("dir printed %llu bytes", dirOutput.length);
	
	CommandPromptEchoTest(false, StrLit("Hello World!"), Str_Empty);
	CommandPromptEchoTest(true,  StrLit("Hello Error!"), Str_Empty);
	CommandPromptEchoTest(false, StrLit("a\bc"), Str_Empty);
	CommandPromptEchoTest(false, StrLit("a \t c"), Str_Empty);
	CommandPromptEchoTest(false, StrLit("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"), Str_Empty);
	CommandPromptEchoTest(false, StrLit("~!@#$?.,-=_+:;[]{}()`\' \" \\%*"), Str_Empty); //NOTE: Characters that cause problems: ^ < > / | &
	
	#elif BUILDING_ON_UNIX
	
	Str lsOutput = Str_Empty_Const;
	pig_run("ls", .run_in_shell=true, .stdoutStr=&lsOutput);
	PrintLine("ls printed %llu bytes", lsOutput.length);
	
	pig_run("ls -l", .run_in_shell=true, .stdoutStr=&lsOutput);
	PrintLine("ls -l printed %llu bytes", lsOutput.length);
	
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
	
	pig_run_tolerate_failures = true;
	pig_run("clang abcdf", .error_message="Clang returned an error but we aren't exiting because pig_run_tolerate_failures global is enabled!");
	pig_run_tolerate_failures = false;
	
	pig_run("clang abcdf", .error_message="Goodbye :)");
	AssertMsg(false, "Unreachable!");
	#endif //BUILDING_ON_UNIX
}
