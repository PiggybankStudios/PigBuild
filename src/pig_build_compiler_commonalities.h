/*
File:   pig_build_compiler_commonalities.h
Author: Taylor Robbins
Date:   08\07\2026
Description:
	** Some options between Clang, GCC, and MSVC compilers are effectively the
	** same but they have different syntax. These functions allow a quick way
	** to add each arg variant for each compiler with tags to match the compiler
	** they apply to. If you use these functions then you need to pass tags when
	** you call RunCliProgram that match which compiler is being used
*/

#ifndef _PIG_BUILD_COMPILER_COMMONALITIES_H
#define _PIG_BUILD_COMPILER_COMMONALITIES_H

#include "pig_build_base.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"

//System lib?
//Output file?
//Make library?
//Entry point?
//Warnings as errors?
//Language Version?

// +==============================+
// |         Compile-only         |
// +==============================+
void AddTaggedCompileOnlyArg(CliArgs* argsPntr, const char* includeExcludeTagsStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArg(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_COMPILE);
	AddTaggedArg(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_COMPILE);
	AddTaggedArg(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_COMPILE);
}
void AddCompileOnlyArg(CliArgs* argsPntr)
{
	AddTaggedArg(argsPntr, T_MSVC_CL, CL_COMPILE);
	AddTaggedArg(argsPntr, T_CLANG,   CLANG_COMPILE);
	AddTaggedArg(argsPntr, T_GCC,     GCC_COMPILE);
}

// +==============================+
// |       Full File Paths        |
// +==============================+
void AddTaggedFullFilePathsArg(CliArgs* argsPntr, const char* includeExcludeTagsStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArg(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_FULL_FILE_PATHS);
	AddTaggedArg(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_FULL_FILE_PATHS);
	//GCC has full file paths as the defualt behavior *if* you pass full paths. There is no option to configure this
}
void AddFullFilePathsArg(CliArgs* argsPntr)
{
	AddTaggedArg(argsPntr, T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(argsPntr, T_CLANG,   CLANG_FULL_FILE_PATHS);
	//GCC has full file paths as the defualt behavior *if* you pass full paths. There is no option to configure this
}

// +==============================+
// |          Debug Info          |
// +==============================+
void AddTaggedDebugInfoArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str debugInfoFormatStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArg(argsPntr,    msvcIncludeExcludeTagsStr.chars,  CL_DEBUG_INFO); //TODO: Can MSVC compiler take a format specifier?
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_DEBUG_INFO, debugInfoFormatStr);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_DEBUG_INFO,   debugInfoFormatStr);
}
void AddDebugInfoArgStr(CliArgs* argsPntr, Str debugInfoFormatStr)
{
	AddTaggedArg(argsPntr,    T_MSVC_CL, CL_DEBUG_INFO); //TODO: Can MSVC compiler take a format specifier?
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_DEBUG_INFO, debugInfoFormatStr);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_DEBUG_INFO,   debugInfoFormatStr);
}
void AddTaggedDebugInfoArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* debugInfoFormatNt) { AddTaggedDebugInfoArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(debugInfoFormatNt)); }
void AddDebugInfoArgNt(CliArgs* argsPntr, const char* debugInfoFormatNt)                                          { AddDebugInfoArgStr(argsPntr, MakeStrNt(debugInfoFormatNt)); }
#define AddTaggedDebugInfoArgLit(argsPntr, includeExcludeTagsStr, debugInfoFormatLit) AddTaggedDebugInfoArgStr((argsPntr), (includeExcludeTagsStr), StrLit(debugInfoFormatLit))
#define AddDebugInfoArgLit(argsPntr, debugInfoFormatLit)                              AddDebugInfoArgStr((argsPntr), StrLit(debugInfoFormatLit))


// +==============================+
// |      Optimization Level      |
// +==============================+
void AddTaggedOptimizationLevelArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str optLevelStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArgStr(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_OPTIMIZATION_LEVEL,    optLevelStr);
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_OPTIMIZATION_LEVEL, optLevelStr);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_OPTIMIZATION_LEVEL,   optLevelStr);
}
void AddOptimizationLevelArgStr(CliArgs* argsPntr, Str optLevelStr)
{
	AddTaggedArgStr(argsPntr, T_MSVC_CL, CL_OPTIMIZATION_LEVEL,    optLevelStr);
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_OPTIMIZATION_LEVEL, optLevelStr);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_OPTIMIZATION_LEVEL,   optLevelStr);
}
void AddTaggedOptimizationLevelArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* optLevelNt) { AddTaggedOptimizationLevelArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(optLevelNt)); }
void AddOptimizationLevelArgNt(CliArgs* argsPntr, const char* optLevelNt)                                          { AddOptimizationLevelArgStr(argsPntr, MakeStrNt(optLevelNt)); }
#define AddTaggedOptimizationLevelArgLit(argsPntr, includeExcludeTagsStr, optLevelLit) AddTaggedOptimizationLevelArgStr((argsPntr), (includeExcludeTagsStr), StrLit(optLevelLit))
#define AddOptimizationLevelArgLit(argsPntr, optLevelLit)                              AddOptimizationLevelArgStr((argsPntr), StrLit(optLevelLit))

// +==============================+
// |         Include Dir          |
// +==============================+
void AddTaggedIncludeDirArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str includePath)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArgStr(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_INCLUDE_DIR,    includePath);
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_INCLUDE_DIR, includePath);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_INCLUDE_DIR,   includePath);
}
void AddIncludeDirArgStr(CliArgs* argsPntr, Str includePath)
{
	AddTaggedArgStr(argsPntr, T_MSVC_CL, CL_INCLUDE_DIR,    includePath);
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_INCLUDE_DIR, includePath);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_INCLUDE_DIR,   includePath);
}
void AddTaggedIncludeDirArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* includePathNt) { AddTaggedIncludeDirArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(includePathNt)); }
void AddIncludeDirArgNt(CliArgs* argsPntr, const char* includePathNt)                                          { AddIncludeDirArgStr(argsPntr, MakeStrNt(includePathNt)); }
#define AddTaggedIncludeDirArgLit(argsPntr, includeExcludeTagsStr, includePathLit) AddTaggedIncludeDirArgStr((argsPntr), (includeExcludeTagsStr), StrLit(includePathLit))
#define AddIncludeDirArgLit(argsPntr, includePathLit)                              AddIncludeDirArgStr((argsPntr), StrLit(includePathLit))

// +==============================+
// |         Library Dir          |
// +==============================+
void AddTaggedLibraryDirArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str libraryPath)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_LINK "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG     "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC       "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArgStr(argsPntr, msvcIncludeExcludeTagsStr.chars,  LINK_LIBRARY_DIR,  libraryPath); //TODO: This is a little weird, we have only used the LINK argument, never an argument that we pass to CL
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_LIBRARY_DIR, libraryPath);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_INCLUDE_DIR,   libraryPath);
}
void AddLibraryDirArgStr(CliArgs* argsPntr, Str libraryPath)
{
	AddTaggedArgStr(argsPntr, T_MSVC_CL, CL_INCLUDE_DIR,    libraryPath);
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_INCLUDE_DIR, libraryPath);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_INCLUDE_DIR,   libraryPath);
}
void AddTaggedLibraryDirArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* libraryPathNt) { AddTaggedLibraryDirArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(libraryPathNt)); }
void AddLibraryDirArgNt(CliArgs* argsPntr, const char* libraryPathNt)                                          { AddLibraryDirArgStr(argsPntr, MakeStrNt(libraryPathNt)); }
#define AddTaggedLibraryDirArgLit(argsPntr, includeExcludeTagsStr, libraryPathLit) AddTaggedLibraryDirArgStr((argsPntr), (includeExcludeTagsStr), StrLit(libraryPathLit))
#define AddLibraryDirArgLit(argsPntr, libraryPathLit)                              AddLibraryDirArgStr((argsPntr), StrLit(libraryPathLit))

// +==============================+
// |            Define            |
// +==============================+
void AddTaggedDefineArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str defineStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArgStr(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_DEFINE,    defineStr);
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_DEFINE, defineStr);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_DEFINE,   defineStr);
}
void AddDefineArgStr(CliArgs* argsPntr, Str defineStr)
{
	AddTaggedArgStr(argsPntr, T_MSVC_CL, CL_DEFINE,    defineStr);
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_DEFINE, defineStr);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_DEFINE,   defineStr);
}
void AddTaggedDefineArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* defineStrNt) { AddTaggedDefineArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(defineStrNt)); }
void AddDefineArgNt(CliArgs* argsPntr, const char* defineStrNt)                                          { AddDefineArgStr(argsPntr, MakeStrNt(defineStrNt)); }
#define AddTaggedDefineArgLit(argsPntr, includeExcludeTagsStr, defineStrLit) AddTaggedDefineArgStr((argsPntr), (includeExcludeTagsStr), StrLit(defineStrLit))
#define AddDefineArgLit(argsPntr, defineStrLit)                              AddDefineArgStr((argsPntr), StrLit(defineStrLit))

// +==============================+
// |        Warning Level         |
// +==============================+
//TODO: We should verify which levels exist for each compiler, they may not have identical options
//NOTE: The warnings enabled at each level are probably different per compiler. You will likely need
//      to disable specific warnings on each compiler after setting this level
void AddTaggedWarningLevelArgStr(CliArgs* argsPntr, const char* includeExcludeTagsStr, Str warningLevelStr)
{
	Str msvcIncludeExcludeTagsStr  = JoinStrings2(StrLit(T_MSVC_CL "|"), MakeStrNt(includeExcludeTagsStr));
	Str clangIncludeExcludeTagsStr = JoinStrings2(StrLit(T_CLANG   "|"), MakeStrNt(includeExcludeTagsStr));
	Str gccIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_GCC     "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArgStr(argsPntr, msvcIncludeExcludeTagsStr.chars,  CL_WARNING_LEVEL,    warningLevelStr);
	AddTaggedArgStr(argsPntr, clangIncludeExcludeTagsStr.chars, CLANG_WARNING_LEVEL, warningLevelStr);
	AddTaggedArgStr(argsPntr, gccIncludeExcludeTagsStr.chars,   GCC_WARNING_LEVEL,   warningLevelStr);
}
void AddWarningLevelArgStr(CliArgs* argsPntr, Str warningLevelStr)
{
	AddTaggedArgStr(argsPntr, T_MSVC_CL, CL_WARNING_LEVEL,    warningLevelStr);
	AddTaggedArgStr(argsPntr, T_CLANG,   CLANG_WARNING_LEVEL, warningLevelStr);
	AddTaggedArgStr(argsPntr, T_GCC,     GCC_WARNING_LEVEL,   warningLevelStr);
}
void AddTaggedWarningLevelArgNt(CliArgs* argsPntr, const char* includeExcludeTagsStr, const char* warningLevelNt) { AddTaggedWarningLevelArgStr(argsPntr, includeExcludeTagsStr, MakeStrNt(warningLevelNt)); }
void AddWarningLevelArgNt(CliArgs* argsPntr, const char* warningLevelNt)                                          { AddWarningLevelArgStr(argsPntr, MakeStrNt(warningLevelNt)); }
#define AddTaggedWarningLevelArgLit(argsPntr, includeExcludeTagsStr, warningLevelLit) AddTaggedWarningLevelArgStr((argsPntr), (includeExcludeTagsStr), StrLit(warningLevelLit))
#define AddWarningLevelArgLit(argsPntr, warningLevelLit)                              AddWarningLevelArgStr((argsPntr), StrLit(warningLevelLit))

// +==============================+
// |           No Logo            |
// +==============================+
//This is only needed for MSVC but it needs to be passed to CL and to LINK binaries
void AddTaggedNoLogoArg(CliArgs* argsPntr, const char* includeExcludeTagsStr)
{
	Str msvcClIncludeExcludeTagsStr   = JoinStrings2(StrLit(T_MSVC_CL   "|"), MakeStrNt(includeExcludeTagsStr));
	Str msvcLinkIncludeExcludeTagsStr = JoinStrings2(StrLit(T_MSVC_LINK "|"), MakeStrNt(includeExcludeTagsStr));
	AddTaggedArg(argsPntr, msvcClIncludeExcludeTagsStr.chars,   CL_NO_LOGO);
	AddTaggedArg(argsPntr, msvcLinkIncludeExcludeTagsStr.chars, LINK_NO_LOGO);
}
void AddNoLogoArg(CliArgs* argsPntr)
{
	AddTaggedArg(argsPntr, T_MSVC_CL,   CL_NO_LOGO);
	AddTaggedArg(argsPntr, T_MSVC_LINK, LINK_NO_LOGO);
}

#endif //  _PIG_BUILD_COMPILER_COMMONALITIES_H
