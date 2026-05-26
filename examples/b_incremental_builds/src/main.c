/*
File:   main.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** A simple hello world program
*/

#include "common.h"
#include "game.h"

int main(int argc, char* argv[])
{
	WriteLine("Starting game...");
	
	GameState gameState = EMPTY;
	InitGame(&gameState, 1); //TODO: Pick a random seed based off time!
	SpawnPlayer(&gameState.player, NewVec2i(0, 2));
	
	u64 frameIndex = 0;
	while (!GameShouldEnd(&gameState))
	{
		RunGameLoop(&gameState, frameIndex);
		frameIndex++;
	}
	
	WriteLine("Game Over!");
	return 0;
}