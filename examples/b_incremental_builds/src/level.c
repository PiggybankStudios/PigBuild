/*
File:   level.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** None
*/

#include "level.h"

void InitLevel(Level* level, Vec2i size)
{
	Assert(size.width > 0 && size.height > 0);
	level->numTiles = (u64)(size.width * size.height);
	level->tiles = (Tile*)malloc(sizeof(Tile) * level->numTiles);
	memset(level->tiles, 0x00, sizeof(Tile) * level->numTiles);
}
