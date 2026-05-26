/*
File:   vectors.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _VECTORS_H
#define _VECTORS_H

#include "common.h"

typedef struct Vec2 Vec2;
struct Vec2
{
	union { r32 x; r32 width; };
	union { r32 y; r32 height; };
};
typedef struct Vec2i Vec2i;
struct Vec2i
{
	union { i32 x; i32 width; };
	union { i32 y; i32 height; };
};

typedef struct Rec Rec;
struct Rec
{
	union
	{
		Vec2 topLeft;
		struct { r32 x, y; };
	};
	union
	{
		Vec2 size;
		struct { r32 width, height; };
	};
};

#define NewVec2(xValue, yValue) (Vec2){ .x=(xValue), .y=(yValue) }
#define NewVec2i(xValue, yValue) (Vec2i){ .x=(xValue), .y=(yValue) }
#define FillVec2(value) NewVec2((value), (value)) //NOTE: This evaluates "value" twice!
#define FillVec2i(value) NewVec2i((value), (value)) //NOTE: This evaluates "value" twice!
#define NewRec(xValue, yValue, widthValue, heightValue) (Rec){ .x=(xValue), .y=(yValue), .width=(widthValue), .height=(heightValue) }

bool EqualsVec2(Vec2 left, Vec2 right);
Vec2 AddVec2(Vec2 left, Vec2 right);
Vec2 SubVec2(Vec2 left, Vec2 right);
Vec2 MulVec2(Vec2 left, Vec2 right);
Vec2 DivVec2(Vec2 left, Vec2 right);
Vec2 ScaleVec2(Vec2 vector, r32 scalar);
Vec2 ShrinkVec2(Vec2 vector, r32 divisor);
r32 DotVec2(Vec2 left, Vec2 right);
r32 LengthSquaredVec2(Vec2 vector);
r32 LengthVec2(Vec2 vector);

bool EqualsVec2i(Vec2i left, Vec2i right);
Vec2i AddVec2i(Vec2i left, Vec2i right);
Vec2i SubVec2i(Vec2i left, Vec2i right);
Vec2i MulVec2i(Vec2i left, Vec2i right);
Vec2i DivVec2i(Vec2i left, Vec2i right);
Vec2i ScaleVec2i(Vec2i vector, i32 scalar);
Vec2i ShrinkVec2i(Vec2i vector, i32 divisor);

bool EqualsRec(Rec left, Rec right);
Rec ScaleRec(Rec rectangle, r32 scalar);
Rec MoveRec(Rec rectangle, Vec2 vector);

#endif //  _VECTORS_H
