/*
File:   common.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _COMMON_H
#define _COMMON_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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

#define nullptr ((void*)0)
#define EMPTY  {0}

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define CheckStrLit(stringLiteral) ("" stringLiteral "")

#define WriteLine(messageStr)       printf(messageStr "\n")
#define WriteLine_E(messageStr)     fprintf(stderr, messageStr "\n")
#define PrintLine(formatStr, ...)   printf(formatStr "\n", ##__VA_ARGS__)
#define PrintLine_E(formatStr, ...) fprintf(stderr, formatStr "\n", ##__VA_ARGS__)

#define Min2(number1, number2) (((number1) <= (number2)) ? (number1) : (number2))
#define Max2(number1, number2) (((number1) >= (number2)) ? (number1) : (number2))

#define AssertMsg(condition, message)        assert((condition) && (message))
#define Assert(condition)                    assert(condition)
#define AssertFmt(condition, formatStr, ...) if (!(condition)) { PrintLine_E("Assertion Failed! Message:\n" formatStr, ##__VA_ARGS__); AssertMsg((condition), (formatStr)); }
#define NotNull(pntr)                        Assert((pntr) != nullptr)

#endif //  _COMMON_H
