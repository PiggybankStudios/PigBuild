/*
File:   enemy.h
Author: Taylor Robbins
Date:   05\25\2026
*/

#ifndef _ENEMY_H
#define _ENEMY_H

#include "common.h"

#include "vectors.h"

typedef struct Enemy Enemy;
struct Enemy
{
	Vec2i pos;
	u8 rotation;
	r32 health;
	r32 maxHealth;
	r32 attack;
};

#endif //  _ENEMY_H
