/*
File:   game.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** None
*/

#include "game.h"

#include "random.h"

Enemy* AddEnemy(GameState* game, Vec2i pos)
{
	Enemy* newEnemies = (Enemy*)malloc(sizeof(Enemy) * game->numEnemies+1);
	if (game->numEnemies > 0)
	{
		memcpy(newEnemies, game->enemies, sizeof(Enemy) * game->numEnemies);
		free(game->enemies);
	}
	game->enemies = newEnemies;
	Enemy* newEnemy = &game->enemies[game->numEnemies];
	game->numEnemies++;
	memset(newEnemy, 0x00, sizeof(Enemy));
	newEnemy->pos = pos;
	newEnemy->maxHealth = 65 + ((i32)(RandomNext(&game->random)%50) - 25);
	newEnemy->health = newEnemy->maxHealth;
	newEnemy->rotation = (RandomNext(&game->random)%4);
	newEnemy->attack = 8 + ((i32)(RandomNext(&game->random)%4) - 2);
	return newEnemy;
}

void InitGame(GameState* game, u64 randSeed)
{
	game->random = randSeed;
	
	InitLevel(&game->currentLevel, FillVec2i(15));
	
	for (u64 x = 0; x < game->currentLevel.width; x++)
	{
		for (u64 y = 0; y < game->currentLevel.width; y++)
		{
			Tile* tilePntr = &game->currentLevel.tiles[(y * (game->currentLevel.width)) + x];
			if (x == 0 || y == 0 || x == game->currentLevel.width-1 || y == game->currentLevel.height-1)
			{
				*tilePntr = Tile_Wall;
			}
			else
			{
				*tilePntr = Tile_Empty;
			}
		}
	}
	
	for (u64 x = 0; x < game->currentLevel.width; x++)
	{
		for (u64 y = 0; y < game->currentLevel.width; y++)
		{
			Tile tile = game->currentLevel.tiles[(y * (game->currentLevel.width)) + x];
			
			if (tile == Tile_Empty)
			{
				if ((RandomNext(&game->random) % 100) < 20)
				{
					AddEnemy(game, NewVec2i(x, y));
				}
			}
		}
	}
}

bool GameShouldEnd(GameState* game)
{
	return (game->player.health == 0);
}

void RunGameLoop(GameState* game, u64 frameIndex)
{
	Vec2i nextPlayerMove = NewVec2i(
		(i32)(RandomNext(&game->random)%3) - 1,
		(i32)(RandomNext(&game->random)%3) - 1
	);
	Vec2i newPos = AddVec2i(game->player.pos, nextPlayerMove);
	bool hitWall = false;
	if (newPos.x < 0 || newPos.y < 0 || newPos.x >= game->currentLevel.width || newPos.y >= game->currentLevel.height) { hitWall = true; }
	else if (game->currentLevel.tiles[(newPos.y * game->currentLevel.width) + newPos.x] == Tile_Wall) { hitWall = true; }
	
	if (!hitWall)
	{
		PrintLine("Player moves to (%d, %d)", newPos.x, newPos.y);
		game->player.pos = newPos;
	}
	
	//TODO: Implement the rest here!
	
	if (frameIndex >= 1000)
	{
		WriteLine_E("The player ran out of time!");
		game->player.health = 0;
	}
}
