/*
File:   pig_build_base.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_BASE_H
#define _PIG_BUILD_BASE_H

// +--------------------------------------------------------------+
// |                    BUILDING_ON_X Defines                     |
// +--------------------------------------------------------------+
#if defined(__arm64__) || defined(__aarch64__)
#define BUILDING_ON_INTEL 0
#define BUILDING_ON_ARM   1
#else
#define BUILDING_ON_INTEL 1
#define BUILDING_ON_ARM   0
#endif

#if defined(_WIN32)
#define BUILDING_ON_WINDOWS 1
#define BUILDING_ON_NAME "WINDOWS"
#else
#define BUILDING_ON_WINDOWS 0
#endif

#if defined(__linux__) || defined(__unix__)
#define BUILDING_ON_LINUX 1
#define BUILDING_ON_NAME "LINUX"
#else
#define BUILDING_ON_LINUX 0
#endif

#ifdef __APPLE__
#define BUILDING_ON_OSX 1
#define BUILDING_ON_NAME "OSX"
#else
#define BUILDING_ON_OSX 0
#endif

#ifndef BUILDING_ON_NAME
#define BUILDING_ON_NAME "UNKNOWN"
#endif

#if !BUILDING_ON_LINUX
#define BUILDING_ON_LINUX_ARM   0
#define BUILDING_ON_LINUX_INTEL 0
#elif BUILDING_ON_ARM
#define BUILDING_ON_LINUX_ARM   1
#define BUILDING_ON_LINUX_INTEL 0
#else
#define BUILDING_ON_LINUX_ARM   0
#define BUILDING_ON_LINUX_INTEL 1
#endif

#if !BUILDING_ON_OSX
#define BUILDING_ON_OSX_ARM   0
#define BUILDING_ON_OSX_INTEL 0
#elif BUILDING_ON_ARM
#define BUILDING_ON_OSX_ARM   1
#define BUILDING_ON_OSX_INTEL 0
#else
#define BUILDING_ON_OSX_ARM   0
#define BUILDING_ON_OSX_INTEL 1
#endif

#ifdef __cplusplus
#define LANGUAGE_IS_C   0
#define LANGUAGE_IS_CPP 1
#else
#define LANGUAGE_IS_C   1
#define LANGUAGE_IS_CPP 0
#endif

#if BUILDING_ON_WINDOWS
#define IF_WINDOWS(...)     __VA_ARGS__
#define IF_NOT_WINDOWS(...) //nothing
#else
#define IF_WINDOWS(...)     //nothing
#define IF_NOT_WINDOWS(...) __VA_ARGS__
#endif
#if BUILDING_ON_LINUX
#define IF_LINUX(...)     __VA_ARGS__
#define IF_NOT_LINUX(...) //nothing
#else
#define IF_LINUX(...)     //nothing
#define IF_NOT_LINUX(...) __VA_ARGS__
#endif
#if BUILDING_ON_OSX
#define IF_OSX(...)     __VA_ARGS__
#define IF_NOT_OSX(...) //nothing
#else
#define IF_OSX(...)     //nothing
#define IF_NOT_OSX(...) __VA_ARGS__
#endif

#if (BUILDING_ON_OSX || BUILDING_ON_LINUX)
#define BUILDING_ON_UNIX   1
#define IF_UNIX(...)       __VA_ARGS__
#define IF_NOT_UNIX(...)   //nothing
#else
#define BUILDING_ON_UNIX   0
#define IF_UNIX(...)       //nothing
#define IF_NOT_UNIX(...)   __VA_ARGS__
#endif

#if LANGUAGE_IS_C
#define IF_LANG_C(...) __VA_ARGS__
#else
#define IF_LANG_C(...) //nothing
#endif
#if LANGUAGE_IS_CPP
#define IF_LANG_CPP(...) __VA_ARGS__
#else
#define IF_LANG_CPP(...) //nothing
#endif

#if BUILDING_ON_INTEL
#define IF_INTEL(...)     __VA_ARGS__
#define IF_NOT_INTEL(...) //nothing
#else
#define IF_INTEL(...)     //nothing
#define IF_NOT_INTEL(...) __VA_ARGS__
#endif
#if BUILDING_ON_ARM
#define IF_ARM(...)     __VA_ARGS__
#define IF_NOT_ARM(...) //nothing
#else
#define IF_ARM(...)     //nothing
#define IF_NOT_ARM(...) __VA_ARGS__
#endif

// +--------------------------------------------------------------+
// |                  Standard Library Includes                   |
// +--------------------------------------------------------------+
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
// #include <float.h>
#include <limits.h>
// #include <stddef.h>
// #include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
// #include <math.h>
#include <errno.h>
#include <sys/stat.h>

//Platform-dependent includes
#if BUILDING_ON_WINDOWS
#include <windows.h>
#include <direct.h> //for _rmdir, _mkdir, _chdir
#pragma comment(lib, "Shlwapi.lib")
#include "Shlwapi.h" //for PathFileExistsA
#else //!BUILDING_ON_WINDOWS
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif //!BUILDING_ON_WINDOWS

// +--------------------------------------------------------------+
// |                           Defines                            |
// +--------------------------------------------------------------+
#if LANGUAGE_IS_C
#define nullptr ((void*)0)
#define EMPTY  {0}
#else
#define EMPTY  {}
#endif

// This macro is surrounds the type with parenthesis when compiling in C, but no parenthesis when compiling in C++
// This is useful for making C/C++ agnostic curly bracket initializers. For example:
// myStr = INIT(Str){ 4, "test" };
#if LANGUAGE_IS_C
#define INIT(type) (type)
#else
#define INIT(type) type
#endif

#if BUILDING_ON_WINDOWS
#define PATH_SEP_CHAR '\\'
#define PATH_SEP_CHAR_STR "\\"
#else
#define PATH_SEP_CHAR '/'
#define PATH_SEP_CHAR_STR "/"
#endif

#if BUILDING_ON_WINDOWS
#define FOLDER_PERMISSIONS 0
#else
#define FOLDER_PERMISSIONS S_IRWXU|S_IRWXG|S_IRWXO
#endif

//TODO: Really this should be something like COMPILER_IS_MSVC not BUILDING_ON_WINDOWS
#if BUILDING_ON_WINDOWS
#define rmdir(dirname) _rmdir(dirname)
#define chdir(dirname) _chdir(dirname)
#define mkdir(dirname, permissions) _mkdir(dirname) //permissions are ignored
#endif

#if BUILDING_ON_WINDOWS
#define OBJ_EXT ".obj"
#define DLL_EXT ".dll"
#define LIB_EXT ".lib"
#define EXE_EXT ".exe"
#elif BUILDING_ON_OSX
#define OBJ_EXT ".o"
#define DLL_EXT ".dylib"
#define LIB_EXT ".dylib"
#define EXE_EXT ""
#else
#define OBJ_EXT ".o"
#define DLL_EXT ".so"
#define LIB_EXT ".so"
#define EXE_EXT ""
#endif

// +--------------------------------------------------------------+
// |                    stdint.h Type Aliases                     |
// +--------------------------------------------------------------+
// NOTE: Both "long" and "long long" in Clang are 8 bytes, so int64_t/uint64_t are "long"
//       Meanwhile in MSVC "long" is 4 bytes while "long long" is 8 bytes, so int64_t/uint64_t are "long long"
//       Format arguments like %llu will complain if this is technically a "long" and not a "long long"
//       So to make sure we can always use %llu arguments across Windows and Linux we specifically typedef long long, not int64_t/uint64_t from stdint.h.
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef unsigned long long u64;
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef long long i64;
typedef float r32;
typedef double r64;

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
// Arrays can be measures using sizeof(array) and dividing by the size of each element in the array sizeof(array[0])
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

// This macro does nothing when stringLiteral IS actually a string literally. If it's a pointer or an array or anything else it will produce a compile-time error
// We often use this macro when we want to do sizeof(stringLiteral) in a macro and we are expecting number of chars in the string (+1 for null-term char)
#define CheckStrLit(stringLiteral) ("" stringLiteral "")

// We often don't like typing "\n" at the end of our format strings, since writing to the console without a new-line is the less-common case
// These macros make adding "\n" implicit. Additionally when printing errors we should route to stderr without explicitly writing fprintf(stderr, ...)
#define WriteLine(messageStr)       printf(messageStr "\n")
#define WriteLine_E(messageStr)     fprintf(stderr, messageStr "\n")
#define PrintLine(formatStr, ...)   printf(formatStr "\n", ##__VA_ARGS__)
#define PrintLine_E(formatStr, ...) fprintf(stderr, formatStr "\n", ##__VA_ARGS__)

// Shorthand for checking both forward and backslash (often a thing we do in build scripts because of Windows vs OtherOS path separating characters)
#define IsSlash(character) ((character) == '\\' || (character) == '/')

// Used when doing debug logs in english, a particular number might have sway on the plurality of a following word
// For example we say "1 bird" and "3 birds", and also "0 birds". So basically use a suffix (like "s") when the number != 1
#define PluralEx(number, singularSuffix, multipleSuffix) (((number) == 1) ? (singularSuffix) : (multipleSuffix))
#define Plural(number, multipleSuffix) (((number) == 1) ? "" : (multipleSuffix))

#define Min2(number1, number2) (((number1) <= (number2)) ? (number1) : (number2))
#define Max2(number1, number2) (((number1) >= (number2)) ? (number1) : (number2))

#define AssertMsg(condition, message)        assert((condition) && (message))
#define Assert(condition)                    assert(condition)
#define AssertFmt(condition, formatStr, ...) if (!(condition)) { PrintLine_E("Assertion Failed! Message:\n" formatStr, ##__VA_ARGS__); AssertMsg((condition), (formatStr)); }
#define NotNull(pntr)                        Assert((pntr) != nullptr)

// Macros used to check or modify specific bit(s) in a field, the pattern of using individual bits as booleans is often referred to as "flags"
#define IsFlagSet(BitwiseField, Bit) ((Bit) != 0 && ((BitwiseField) & (Bit)) == (Bit))
#define FlagSet(BitwiseField, Bit)   (BitwiseField) |= (Bit)
#define FlagUnset(BitwiseField, Bit) (BitwiseField) &= ~(Bit)
#define FlagToggle(BitwiseField, Bit) ((BitwiseField) ^= (Bit))
#define FlagSetTo(BitwiseField, Bit, condition) if (condition) { FlagSet((BitwiseField), (Bit)); } else { FlagUnset((BitwiseField), (Bit)); }

//TODO: Write a description of these macros
#ifndef STRINGIFY_DEFINE
#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#endif
#ifndef STRINGIFY
#define STRINGIFY(text)          #text
#endif

#define MEMBER_SIZE(structName, memberName)   sizeof(((const structName*)1)->memberName)
#define MEMBER_OFFSET(structName, memberName) (u32)((const u8*)&((const structName*)1)->memberName - (const u8*)((const structName*)1))

// Converts all 3 pointers to u8* and does pointer arithmetic to determine if pntr is >= regionStart and < (regionStart + regionSize)
#define IsPntrWithin(regionStart, regionSize, pntr) (((u8*)(pntr)) >= ((u8*)(regionStart)) && ((u8*)(pntr)) <= (((u8*)(regionStart)) + (regionSize)))
#define IsSizedPntrWithin(regionStart, regionSize, pntr, size) (((u8*)(pntr)) >= ((u8*)(regionStart)) && (((u8*)(pntr)) + (size)) <= (((u8*)(regionStart)) + (regionSize)))

//TODO: we should do a proper explanation of what's happening here
// Preprocessor macros are a bit finicky and we need the 2 layer deep macro thing to concat 2 things when one or both of the parts are preprocessor macros we want to expand before doing the concat
#define PIG_BUILD_CONCAT_INNER(leftPart, rightPart) leftPart ## rightPart
#define PIG_BUILD_CONCAT(leftPart, rightPart)       PIG_BUILD_CONCAT_INNER(leftPart, rightPart)

//TODO: We can probably get rid of the Ex variants
//Use a for loop to execute code at the end of a block (warning: if a break is hit inside the block then the endCode will NOT run!)
#define DeferBlockEx(uniqueName, endCode)                                 for (int uniqueName = 0; uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferBlock(endCode)                                               DeferBlockEx(PIG_BUILD_CONCAT(DeferBlockIter, __LINE__), (endCode))
//startCode runs at beginning of block
#define DeferBlockWithStartEx(uniqueName, startCode, endCode)             for (int uniqueName = ((startCode), 0); uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferBlockWithStart(startCode, endCode)                           DeferBlockWithStartEx(PIG_BUILD_CONCAT(DeferBlockIter, __LINE__), (startCode), (endCode))
//startCode returns bool to determine if block should run, endCode always runs
#define DeferIfBlockEx(uniqueName, startCodeAndCondition, endCode)        for (int uniqueName = 2 * !(startCodeAndCondition); (uniqueName == 2) ? ((endCode), false) : (uniqueName == 0); (uniqueName = 1, (endCode)))
#define DeferIfBlock(startCodeAndCondition, endCode)                      DeferIfBlockEx(PIG_BUILD_CONCAT(DeferBlockIter, __LINE__), (startCodeAndCondition), (endCode))
//startCode returns bool to determine block should run, endCode only runs if startCode returns true
#define DeferIfBlockCondEndEx(uniqueName, startCodeAndCondition, endCode) for (int uniqueName = 1 * !(startCodeAndCondition); uniqueName == 0; (uniqueName = 1, (endCode)))
#define DeferIfBlockCondEnd(startCodeAndCondition, endCode)               DeferIfBlockCondEndEx(PIG_BUILD_CONCAT(DeferBlockIter, __LINE__), (startCodeAndCondition), (endCode))

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
// Set this to true before calling things in PigBuild in order to get extra debug output (like RunCliProgram functions will print resolved arguments on lines that start with ">> ")
bool PigBuildDebugMode = false;

#endif //  _PIG_BUILD_BASE_H
