/*
File:   player.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** None
*/

#include "player.h"

void SpawnPlayer(Player* player, Vec2i startPos)
{
	player->pos = startPos;
	player->rotation = 0;
	player->maxHealth = 200;
	player->health = player->maxHealth;
}
