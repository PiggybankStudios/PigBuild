/*
File:   vectors.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** None
*/

#include "vectors.h"

bool EqualsVec2(Vec2 left, Vec2 right) { return (left.x == right.x && left.y == right.y); }
Vec2 AddVec2(Vec2 left, Vec2 right) { return NewVec2(left.x + right.x, left.y + right.y); }
Vec2 SubVec2(Vec2 left, Vec2 right) { return NewVec2(left.x - right.x, left.y - right.y); }
Vec2 MulVec2(Vec2 left, Vec2 right) { return NewVec2(left.x * right.x, left.y * right.y); }
Vec2 DivVec2(Vec2 left, Vec2 right) { return NewVec2(left.x / right.x, left.y / right.y); }
Vec2 ScaleVec2(Vec2 vector, r32 scalar) { return NewVec2(vector.x * scalar, vector.y * scalar); }
Vec2 ShrinkVec2(Vec2 vector, r32 divisor) { return NewVec2(vector.x / divisor, vector.y / divisor); }
r32 DotVec2(Vec2 left, Vec2 right) { return (left.x * right.x) + (left.y * right.y); }
r32 LengthSquaredVec2(Vec2 vector) { return (vector.x * vector.x) + (vector.y * vector.y); }
r32 LengthVec2(Vec2 vector) { return sqrtf(LengthSquaredVec2(vector)); }

bool EqualsVec2i(Vec2i left, Vec2i right) { return (left.x == right.x && left.y == right.y); }
Vec2i AddVec2i(Vec2i left, Vec2i right) { return NewVec2i(left.x + right.x, left.y + right.y); }
Vec2i SubVec2i(Vec2i left, Vec2i right) { return NewVec2i(left.x - right.x, left.y - right.y); }
Vec2i MulVec2i(Vec2i left, Vec2i right) { return NewVec2i(left.x * right.x, left.y * right.y); }
Vec2i DivVec2i(Vec2i left, Vec2i right) { return NewVec2i(left.x / right.x, left.y / right.y); }
Vec2i ScaleVec2i(Vec2i vector, i32 scalar) { return NewVec2i(vector.x * scalar, vector.y * scalar); }
Vec2i ShrinkVec2i(Vec2i vector, i32 divisor) { return NewVec2i(vector.x / divisor, vector.y / divisor); }

bool EqualsRec(Rec left, Rec right) { return (left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height); }
Rec ScaleRec(Rec rectangle, r32 scalar) { return NewRec(rectangle.x * scalar, rectangle.y * scalar, rectangle.width * scalar, rectangle.height * scalar); }
Rec MoveRec(Rec rectangle, Vec2 vector) { return NewRec(rectangle.x + vector.x, rectangle.y + vector.y, rectangle.width, rectangle.height); }
