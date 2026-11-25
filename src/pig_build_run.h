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

#define PIG_RUN_FAILED_TO_START      -1
#define PIG_RUN_PIPE_CREATION_FAILED -2

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_array.h"
#include "pig_build_str_array.h"
#include "pig_build_arg_list.h"

//TODO: Do we need to do any markup on globals?
bool pig_run_tolerate_failures = false;

typedef struct pig_run_args pig_run_args;
struct pig_run_args
{
	CliArgs args;
	StrArray exe_tags;
	const char* error_message;
	bool tolerate_failure;
	bool print_cmd;
	bool print_exit_code;
	bool dry_run;
	bool run_in_shell;
	Str* stdoutStr;
	Str* stderrStr;
	//TODO: silent? no_stdout? no_stderr?
};
typedef struct { pig_run_args wrapped; } pig_run_args_wrapper;
#define pig_run(executablePathNt, ...) pig_run_(MakeStrNt(executablePathNt), INIT(pig_run_args_wrapper){{ __VA_ARGS__ }}.wrapped)

#if BUILDING_ON_WINDOWS
int pig_run_win32(Str joinedExecutableAndArgsStr, pig_run_args* args);
#endif
#if BUILDING_ON_UNIX
int pig_run_unix(Str joinedExecutableAndArgsStr, pig_run_args* args);
#endif

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
		
		#if BUILDING_ON_WINDOWS
		int exitCode = pig_run_win32(joinedExecutableAndArgsStr, &args);
		#elif BUILDING_ON_UNIX
		int exitCode = pig_run_unix(joinedExecutableAndArgsStr, &args);
		#else
		#error pig_run doesn't have an implementation for the current platform!
		#endif
		
		if (PigBuildDebugMode || args.print_exit_code) { PrintLine("Exit Code: %d", exitCode); }
		
		if (exitCode != 0)
		{
			
			PrintLine_E("ERROR: \"%.*s\" exited with code %d\nCmd >> %s",
				StrPrint(executablePath),
				exitCode,
				joinedExecutableAndArgsStr.chars
			);
			
			if (!IsEmptyNt(args.error_message)) { PrintLine_E("%s", args.error_message); }
			
			if (!args.tolerate_failure && !pig_run_tolerate_failures)
			{
				if (IsEmptyNt(args.error_message)) { Assert(exitCode == 0); }
				else { exit(exitCode); }
			}
		}
		
		FreeStr(&joinedExecutableAndArgsStr);
		
		return exitCode;
	}
}

// +--------------------------------------------------------------+
// |                    Windows Implementation                    |
// +--------------------------------------------------------------+
#if BUILDING_ON_WINDOWS

typedef struct pig_run_pipe_servicer_context pig_run_pipe_servicer_context;
struct pig_run_pipe_servicer_context
{
	HANDLE readHandle;
	HANDLE writeHandle; //if set: the received bytes are forwarded to this handle through WriteFile
	Array_u8 buffer; //if writeHandle is NOT set, then we push bytes into this array as we receive them
};

static DWORD WINAPI pig_run_pipe_servicer(void* contextPntr)
{
	pig_run_pipe_servicer_context* context = (pig_run_pipe_servicer_context*)contextPntr;
	u8 stackBuffer[4096];
	DWORD numBytesRead = 0;
	while (ReadFile(context->readHandle, &stackBuffer[0], sizeof(stackBuffer), &numBytesRead, NULL) && numBytesRead > 0)
	{
		if (context->writeHandle != INVALID_HANDLE_VALUE)
		{
			// If the calling code only wanted stderr not stout (or vice versa),
			// then we need to forward the bytes to the console using WriteFile
			DWORD numBytesWritten = 0;
			BOOL writeResult = WriteFile(
				context->writeHandle, //hFile
				&stackBuffer[0], //lpBuffer
				numBytesRead, //nNumberOfBytesToWrite
				&numBytesWritten, //lpNumberOfBytesWritten
				NULL //lpOverlapped
			);
			AssertMsg(writeResult != 0, "Failed to forward stdout/stderr bytes to console using WriteFile!");
			AssertFmt(numBytesWritten == numBytesRead, "Failed to forward %lu byte%s to console! Only %lu/%lu byte%s forwarded",
				numBytesRead, Plural(numBytesRead, "s"),
				numBytesWritten,
				numBytesRead, Plural(numBytesRead, "s")
			);
		}
		else
		{
			AddValuesArray_u8(&context->buffer, (u64)numBytesRead, &stackBuffer[0]);
		}
	}
	return 0;
}

static Str convert_win32_codepage_buffer_to_utf8(UINT consoleCodepage, u64 bufferLength, const u8* bufferPntr)
{
	// MultiByteToWideChar treats a bufferLength of 0 as an error, so we return early instead
	if (bufferLength == 0) { return Str_Empty; }
	
	int multiByteToWideCharResult1 = MultiByteToWideChar(
		consoleCodepage, //CodePage
		0, //dwFlags
		(LPCCH)bufferPntr, //lpMultiByteStr
		bufferLength, //cbMultiByte
		nullptr, //lpWideCharStr
		0 //cchWideChar
	);
	if (multiByteToWideCharResult1 == 0)
	{
		PrintLine_E("Failed to convert %llu byte buffer to UTF-16 using MultiByteToWideChar on Windows!", bufferLength);
		return Str_Empty;
	}
	
	Str16 resultWide = AllocStr16(multiByteToWideCharResult1);
	
	int multiByteToWideCharResult2 = MultiByteToWideChar(
		consoleCodepage, //CodePage
		0, //dwFlags
		(LPCCH)bufferPntr, //lpMultiByteStr
		bufferLength, //cbMultiByte
		resultWide.words, //lpWideCharStr
		resultWide.length+1 //cchWideChar
	);
	Assert(multiByteToWideCharResult2 == multiByteToWideCharResult1);
	
	Str result = Utf16ToUtf8Str(resultWide);
	FreeStr16(&resultWide);
	return result;
}

int pig_run_win32(Str joinedExecutableAndArgsStr, pig_run_args* args)
{
	Str16 joinedExecutableAndArgsWideStr = EMPTY;
	if (args->run_in_shell)
	{
		//TODO: we should escape joinedExecutableAndArgsStr so it can live inside the quotes
		Str shellWrappedExecutableAndArgsStr = JoinStrings3(StrLit("cmd.exe /d /s /c \""), joinedExecutableAndArgsStr, StrLit("\""));
		joinedExecutableAndArgsWideStr = Utf8ToUtf16Str(shellWrappedExecutableAndArgsStr);
		FreeStr(&shellWrappedExecutableAndArgsStr);
	}
	else
	{
		joinedExecutableAndArgsWideStr = Utf8ToUtf16Str(joinedExecutableAndArgsStr);
	}
	
	bool capturingOutput = (args->stdoutStr != nullptr || args->stderrStr != nullptr);
	
	SECURITY_ATTRIBUTES securityAttribs = { .nLength=sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor=NULL, .bInheritHandle=TRUE };
	HANDLE stdoutReadHandle, stdoutWriteHandle;
	HANDLE stderrReadHandle, stderrWriteHandle;
	if (capturingOutput)
	{
		if (!CreatePipe(&stdoutReadHandle, &stdoutWriteHandle, &securityAttribs, /*nSize=*/0))
		{
			AssertMsg(false, "Failed to create stdout pipe on Windows in pig_run!");
			FreeStr16(&joinedExecutableAndArgsWideStr);
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		BOOL setHandleInfoResult1 = SetHandleInformation(stdoutReadHandle, HANDLE_FLAG_INHERIT, /*dwFlags=*/0);
		AssertMsg(setHandleInfoResult1 != 0, "SetHandleInformation call on stdout handle failed inside pig_run!");
		
		if (!CreatePipe(&stderrReadHandle, &stderrWriteHandle, &securityAttribs, /*nSize=*/0))
		{
			AssertMsg(false, "Failed to create stderr pipe on Windows in pig_run!");
			CloseHandle(stdoutReadHandle);
			CloseHandle(stdoutWriteHandle);
			FreeStr16(&joinedExecutableAndArgsWideStr);
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		BOOL setHandleInfoResult2 = SetHandleInformation(stderrReadHandle, HANDLE_FLAG_INHERIT, /*dwFlags=*/0);
		AssertMsg(setHandleInfoResult2 != 0, "SetHandleInformation call on stderr handle failed inside pig_run!");
	}
	
	STARTUPINFOW startupInfo = {
		.cb=sizeof(STARTUPINFOW),
		.dwFlags=(capturingOutput ? STARTF_USESTDHANDLES : 0),
		.hStdInput=GetStdHandle(STD_INPUT_HANDLE),
		.hStdOutput=(capturingOutput ? stdoutWriteHandle : 0),
		.hStdError=(capturingOutput ? stderrWriteHandle : 0),
	};
	PROCESS_INFORMATION processInfo = EMPTY;
	BOOL createProcessResult = CreateProcessW(
		NULL,
		joinedExecutableAndArgsWideStr.words,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&startupInfo,
		&processInfo
	);
	if (capturingOutput) { CloseHandle(stdoutWriteHandle); CloseHandle(stderrWriteHandle); }
	if (createProcessResult == 0)
	{
		if (capturingOutput) { CloseHandle(stdoutReadHandle); CloseHandle(stderrReadHandle); }
		DWORD windowsErrorCode = GetLastError();
		PrintLine_E("CreateProcessW failed with error: %d", windowsErrorCode);
		FreeStr16(&joinedExecutableAndArgsWideStr);
		return PIG_RUN_FAILED_TO_START;
	}
	
	pig_run_pipe_servicer_context stderrServiceContext = EMPTY;
	pig_run_pipe_servicer_context stdoutServiceContext = EMPTY;
	if (capturingOutput)
	{
		// We create a thread to service the stderr pipe while the main thread services the stdout pipe
		stderrServiceContext.readHandle = stderrReadHandle;
		stderrServiceContext.writeHandle = (args->stderrStr == nullptr) ? GetStdHandle(STD_ERROR_HANDLE) : INVALID_HANDLE_VALUE;
		HANDLE stderrServiceThreadHandle = CreateThread(
			NULL, //lpThreadAttributes
			0, //dwStackSize
			pig_run_pipe_servicer, //lpStartAddress
			&stderrServiceContext, //lpParameter
			0, //dwCreationFlags
			NULL //lpThreadId
		);
		
		stdoutServiceContext.readHandle = stdoutReadHandle;
		stdoutServiceContext.writeHandle = (args->stdoutStr == nullptr) ? GetStdHandle(STD_OUTPUT_HANDLE) : INVALID_HANDLE_VALUE;
		DWORD stdoutServiceResult = pig_run_pipe_servicer((void*)&stdoutServiceContext);
		
		WaitForSingleObject(stderrServiceThreadHandle, INFINITE);
		CloseHandle(stderrServiceThreadHandle);
	}
	
	WaitForSingleObject(processInfo.hProcess, INFINITE);
	
	DWORD exitCodeDword = 0;
	GetExitCodeProcess(processInfo.hProcess, &exitCodeDword);
	int exitCode = (int)exitCodeDword;
	
	if (capturingOutput)
	{
		//NOTE: Since the child process inherited our terminal, it will likely use the same codepage as our terminal is configured to use
		//      So we can check out terminals codepage configuration and assume the child processes output is encoded accordingly
		// See https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
		UINT consoleCodepage = GetConsoleOutputCP();
		if (consoleCodepage != 0)
		{
			if (args->stdoutStr != nullptr)
			{
				*args->stdoutStr = convert_win32_codepage_buffer_to_utf8(consoleCodepage, stdoutServiceContext.buffer.length, stdoutServiceContext.buffer.values);
			}
			if (args->stderrStr != nullptr)
			{
				*args->stderrStr = convert_win32_codepage_buffer_to_utf8(consoleCodepage, stderrServiceContext.buffer.length, stderrServiceContext.buffer.values);
			}
		}
		else
		{
			if (args->stdoutStr != nullptr)
			{
				*args->stdoutStr = CopyStr(MakeStr(stdoutServiceContext.buffer.length, stdoutServiceContext.buffer.values));
			}
			if (args->stderrStr != nullptr)
			{
				*args->stderrStr = CopyStr(MakeStr(stderrServiceContext.buffer.length, stderrServiceContext.buffer.values));
			}
		}
		FreeArray_u8(&stdoutServiceContext.buffer);
		FreeArray_u8(&stderrServiceContext.buffer);
	}
	
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
	if (capturingOutput) { CloseHandle(stdoutReadHandle); CloseHandle(stderrReadHandle); }
	
	FreeStr16(&joinedExecutableAndArgsWideStr);
	return exitCode;
}

#endif //BUILDING_ON_WINDOWS

// +--------------------------------------------------------------+
// |                   Linux/OSX Implementation                   |
// +--------------------------------------------------------------+
#if BUILDING_ON_UNIX

int pig_run_unix(Str joinedExecutableAndArgsStr, pig_run_args* args)
{
	// AssertMsg(false, "pig_run_unix_is_not_implemented_yet"); //TODO: Implement me!
	
	int exitCode = system(joinedExecutableAndArgsStr.chars);
	exitCode = WEXITSTATUS(exitCode);
	
	return exitCode;
}



#endif

#endif //  _PIG_BUILD_RUN_H
