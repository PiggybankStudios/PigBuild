/*
File:   game.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _GAME_H
#define _GAME_H

#include "common.h"

#include "vectors.h"
#include "enemy.h"
#include "player.h"
#include "level.h"

typedef struct GameState GameState;
struct GameState
{
	u64 random;
	
	Level currentLevel;
	
	Player player;
	
	int numEnemies;
	Enemy* enemies;
};

Enemy* AddEnemy(GameState* game, Vec2i pos);
void InitGame(GameState* game, u64 randSeed);
bool GameShouldEnd(GameState* game);
void RunGameLoop(GameState* game, u64 frameIndex);

#endif //  _GAME_H
