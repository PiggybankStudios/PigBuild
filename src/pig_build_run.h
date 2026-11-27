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
#include "pig_build_unicode.h"

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
		FreeStr(&joinedExecutableAndArgsStr);
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

//TODO: We should probably check if the buffer is valid UTF-8 and leave it as-is instead of trying to convert it with MultiByteToWideChar
//      Many programs do not follow the selected codepage for the console they are attached to and many things output UTF-8 already,
//      or they output ANSI only which is compatible with being treated as UTF-8.
//      The only scenario I was really trying to handle here is a program that tries to be maximally compatible on Windows by outputting
//      wide characters, but I don't see too many examples of that happening right now.
//      Alternatively we could just provide an explicit option the pig_run_args that allows the calling code to decide what to treat the bytes as
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
	bool capturingOutput = (args->stdoutStr != nullptr || args->stderrStr != nullptr);
	
	SECURITY_ATTRIBUTES securityAttribs = { .nLength=sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor=NULL, .bInheritHandle=TRUE };
	HANDLE stdoutReadHandle, stdoutWriteHandle;
	HANDLE stderrReadHandle, stderrWriteHandle;
	if (capturingOutput)
	{
		if (!CreatePipe(&stdoutReadHandle, &stdoutWriteHandle, &securityAttribs, /*nSize=*/0))
		{
			AssertMsg(false, "Failed to create stdout pipe in pig_run_win32!");
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		BOOL setHandleInfoResult1 = SetHandleInformation(stdoutReadHandle, HANDLE_FLAG_INHERIT, /*dwFlags=*/0);
		AssertMsg(setHandleInfoResult1 != 0, "SetHandleInformation call on stdout handle failed inside pig_run!");
		
		if (!CreatePipe(&stderrReadHandle, &stderrWriteHandle, &securityAttribs, /*nSize=*/0))
		{
			AssertMsg(false, "Failed to create stderr pipe in pig_run_win32!");
			CloseHandle(stdoutReadHandle);
			CloseHandle(stdoutWriteHandle);
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		BOOL setHandleInfoResult2 = SetHandleInformation(stderrReadHandle, HANDLE_FLAG_INHERIT, /*dwFlags=*/0);
		AssertMsg(setHandleInfoResult2 != 0, "SetHandleInformation call on stderr handle failed inside pig_run!");
	}
	
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
		AssertMsg(stderrServiceThreadHandle != NULL, "Failed to create thread to service stderr in pig_run_win32!");
		
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

extern char** environ; //TODO: Do we have to implement this?

int pig_run_unix(Str joinedExecutableAndArgsStr, pig_run_args* args)
{
	AssertMsg(joinedExecutableAndArgsStr.length > 0, "Called pig_run with no executable name!");
	
	// int exitCode = system(joinedExecutableAndArgsStr.chars);
	// exitCode = WEXITSTATUS(exitCode);
	
	bool capturingOutput = (args->stdoutStr != nullptr || args->stderrStr != nullptr);
	
	int stdoutPipeHandles[2];
	int stderrPipeHandles[2];
	if (capturingOutput)
	{
		//NOTE: We can't use pipe2 here because it's only available on MacOS 27.0 or newer, and the default target for Apple Clang in my experience is 26.0 so that's how the build.sh compiles the build_script.c
		if (pipe(stdoutPipeHandles) != 0)
		{
			AssertMsg(false, "Failed to create stdout pipe in pig_run_unix!");
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		//NOTE: The separation of pipe() and fcntl() allows a potential race condition if you spawn sub-processes from other threads!
		//TODO: Maybe we should use pipe2 and change out call to Apple Clang to enforce MacOS 27.0 target?
		//TODO: Maybe we could use POSIX_SPAWN_CLOEXEC_DEFAULT extension which has been available since MacOS 10.7
		int fcntlResult1 = fcntl(stdoutPipeHandles[0], F_SETFD, FD_CLOEXEC);
		Assert(fcntlResult1 == 0);
		
		if (pipe(stderrPipeHandles) != 0)
		{
			AssertMsg(false, "Failed to create stderr pipe in pig_run_unix!");
			close(stdoutPipeHandles[0]); close(stdoutPipeHandles[1]);
			return PIG_RUN_PIPE_CREATION_FAILED;
		}
		int fcntlResult2 = fcntl(stderrPipeHandles[0], F_SETFD, FD_CLOEXEC);
		Assert(fcntlResult2 == 0);
	}
	
	// We need to split the joined string into null-terminated array of char*
	// First we split into StrArray, then we collect all the pntrs into Array_CStr
	StrArray argvStrs = SplitStringBySpacesOutsideQuotes(joinedExecutableAndArgsStr);
	AssertFmt(argvStrs.length >= 1, "Failed to split \"%.*s\" into at least one part", StrPrint(joinedExecutableAndArgsStr));
	Array_CStr argvCStrs = EMPTY;
	GrowArray_CStr(&argvCStrs, argvStrs.length);
	for (u64 sIndex = 0; sIndex < argvStrs.length; sIndex++)
	{
		AddValueArray_CStr(&argvCStrs, argvStrs.strings[sIndex].chars);
	}
	AddValueArray_CStr(&argvCStrs, nullptr);
	// PrintLine_E("Split \"%.*s\" into %llu part%s", StrPrint(joinedExecutableAndArgsStr), argvStrs.length, Plural(argvStrs.length, "s"));
	// for (u64 sIndex = 0; sIndex < argvCStrs.length; sIndex++) { PrintLine_E("\tArg[%llu]: \"%s\"", sIndex, argvCStrs.values[sIndex]); }
	
	posix_spawn_file_actions_t fileActions;
	posix_spawn_file_actions_init(&fileActions);
	if (capturingOutput)
	{
		posix_spawn_file_actions_adddup2(&fileActions, stdoutPipeHandles[1], STDOUT_FILENO);
		posix_spawn_file_actions_adddup2(&fileActions, stderrPipeHandles[1], STDERR_FILENO);
	}
	
	pid_t spawnedPid = 0;
	int spawnResult = posix_spawnp(
		&spawnedPid,
		argvCStrs.values[0],
		&fileActions,
		NULL,
		argvCStrs.values,
		environ
	);
	posix_spawn_file_actions_destroy(&fileActions);
	if (capturingOutput) { close(stdoutPipeHandles[1]); close(stderrPipeHandles[1]); }
	
	if (spawnResult != 0)
	{
		if (capturingOutput) { close(stdoutPipeHandles[0]); close(stderrPipeHandles[0]); }
		FreeArray_CStr(&argvCStrs);
		FreeStrArray(&argvStrs);
		return PIG_RUN_FAILED_TO_START;
	}
	
	if (capturingOutput)
	{
		struct pollfd pollFileDescriptors[2] = {
			{ stdoutPipeHandles[0], POLLIN, 0 },
			{ stderrPipeHandles[0], POLLIN, 0 },
		};
		u8 stackBuffer[4096];
		Array_u8 stdoutBuffer = EMPTY;
		Array_u8 stderrBuffer = EMPTY;
		u8 numFilesDescriptorsOpen = 2;
		while (numFilesDescriptorsOpen > 0)
		{
			int pollResult = poll(pollFileDescriptors, 2, -1);
			if (pollResult < 0)
			{
				if (errno == EINTR) { continue; }
				else { break; }
			}
			for (u8 hIndex = 0; hIndex < 2; hIndex++)
			{
				if (pollFileDescriptors[hIndex].fd < 0 || !pollFileDescriptors[hIndex].revents) { continue; }
				
				ssize_t numBytesRead = read(
					pollFileDescriptors[hIndex].fd,
					&stackBuffer[0],
					sizeof(stackBuffer)
				);
				if (numBytesRead > 0)
				{
					AddValuesArray_u8(
						(hIndex == 0) ? &stdoutBuffer : &stderrBuffer,
						(u64)numBytesRead,
						&stackBuffer[0]
					);
				}
				else if (numBytesRead < 0 && errno == EINTR)
				{
					continue;
				}
				else
				{
					close(pollFileDescriptors[hIndex].fd);
					pollFileDescriptors[hIndex].fd = -1;
					numFilesDescriptorsOpen--;
				}
			}
		}
	}
	
	int exitCode = 0;
	int waitResult = 0;
	do
	{
		waitResult = waitpid(
			spawnedPid,
			&exitCode,
			0 //options
		);
	} while(waitResult >= 0 || errno == EINTR);
	
	FreeArray_CStr(&argvCStrs);
	FreeStrArray(&argvStrs);
	return WIFEXITED(exitCode) ? WEXITSTATUS(exitCode) : 128 + WTERMSIG(exitCode);
}



#endif

#endif //  _PIG_BUILD_RUN_H
