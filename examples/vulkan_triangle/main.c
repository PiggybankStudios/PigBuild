/*
File:   main.c
Author: Taylor Robbins
Date:   04\08\2026
Description: 
	** This is a simple graphical application that creates a Window and uses Vulkan to render a single triangle
*/

#include <stdio.h>

#include "raylib.h"

int main(int argc, const char* argv[])
{
	#if DEBUG_BUILD
	printf("Hello Vulkan! (DEBUG)\n");
	#else
	printf("Hello Vulkan!\n");
	#endif
	
	InitWindow(640, 480, "Vulkan Triangle");
	while (!WindowShouldClose())
	{
		PollInputEvents();
		ClearBackground((Color){.r=255, .g=200, .b=187});
		BeginDrawing();
		
		EndDrawing();
	}
	
	//TODO: Use Raylib or GLFW or Sokol to make a Window with Vulkan support?
	//TODO: Use an existing hello triangle in Vulkan example in C (or go dig up my old one)
	
	return 0;
}
