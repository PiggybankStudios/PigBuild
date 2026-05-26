/*
File:   level.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _LEVEL_H
#define _LEVEL_H

#include "common.h"

#include "vectors.h"

typedef enum Tile Tile;
enum Tile
{
	Tile_Invalid = 0,
	Tile_Empty,
	Tile_Wall,
	Tile_Count,
};

typedef struct Level Level;
struct Level
{
	union
	{
		Vec2i size;
		struct { i32 width, height; };
	};
	u64 numTiles;
	Tile* tiles;
};

void InitLevel(Level* level, Vec2i size);

#endif //  _LEVEL_H
