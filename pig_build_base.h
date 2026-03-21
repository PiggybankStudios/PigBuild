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
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float r32;
typedef double r64;

// +--------------------------------------------------------------+
// |                            Macros                            |
// +--------------------------------------------------------------+
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define CheckStrLit(stringLiteral) ("" stringLiteral "")
//In C, when assigning a structure to a designated initializer AFTER it has been declared, we must have the type prefixing the curly brackets
#define NEW_STRUCT(type) (type)

#define WriteLine(messageStr) printf(messageStr "\n")
#define WriteLine_E(messageStr) fprintf(stderr, messageStr "\n")
#define PrintLine(formatStr, ...) printf(formatStr "\n", ##__VA_ARGS__)
#define PrintLine_E(formatStr, ...) fprintf(stderr, formatStr "\n", ##__VA_ARGS__)

#define IS_SLASH(character) ((character) == '\\' || (character) == '/')

#endif //  _PIG_BUILD_BASE_H
