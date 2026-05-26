/*
File:   player.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _PLAYER_H
#define _PLAYER_H

#include "common.h"

#include "vectors.h"

typedef struct Player Player;
struct Player
{
	Vec2i pos;
	u8 rotation;
	r32 health;
	r32 maxHealth;
	r32 attack;
};

void SpawnPlayer(Player* player, Vec2i startPos);

#endif //  _PLAYER_H
