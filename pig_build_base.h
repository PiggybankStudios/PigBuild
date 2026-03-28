/*
File:   pig_build_base.h
Author: Taylor Robbins
Date:   03\21\2026
*/

#ifndef _PIG_BUILD_BASE_H
#define _PIG_BUILD_BASE_H

//#define this before including this file to enable a printout inside `RunCliProgram` like `>> clang main.c -O0 -g -o program`
#ifndef PIG_BUILD_PRINT_SYS_CMDS
#define PIG_BUILD_PRINT_SYS_CMDS 0
#endif

// +--------------------------------------------------------------+
// |                    BUILDING_ON_X Defines                     |
// +--------------------------------------------------------------+
#if defined(_WIN32)
#define BUILDING_ON_WINDOWS 1
#else
#define BUILDING_ON_WINDOWS 0
#endif

#if defined(__linux__) || defined(__unix__)
#define BUILDING_ON_LINUX 1
#else
#define BUILDING_ON_LINUX 0
#endif

#ifdef __APPLE__
#define BUILDING_ON_OSX 1
#else
#define BUILDING_ON_OSX 0
#endif

#if !BUILDING_ON_OSX
#define BUILDING_ON_OSX_ARM   0
#define BUILDING_ON_OSX_INTEL 0
#elif defined(__arm64__) || defined(__aarch64__)
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
#define ZEROED {0}
#else
#define ZEROED {}
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

#endif //  _PIG_BUILD_BASE_H
