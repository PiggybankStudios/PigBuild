/*
File:   main.c
Author: Taylor Robbins
Date:   04\08\2026
Description: 
	** This is a simple graphical application that creates a Window and uses Vulkan to render a single triangle
*/

#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define TEXT "This is an example!"
#define TEXT_SIZE 20

int main(int argc, const char* argv[])
{
	#if DEBUG_BUILD
	printf("Hello Raylib! (DEBUG)\n");
	#else
	printf("Hello Raylib!\n");
	#endif
	
	InitWindow(640, 480, "Raylib Window");
	SetWindowState(FLAG_VSYNC_HINT);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	uint64_t frame = 0;
	while (!WindowShouldClose())
	{
		// PollInputEvents();
		ClearBackground(DARKGRAY);
		BeginDrawing();
		int textWidth = MeasureText(TEXT, TEXT_SIZE);
		float textX = -(float)textWidth + (float)((frame+100)%(GetScreenWidth() + textWidth));
		DrawText(TEXT, textX, GetScreenHeight()/2, TEXT_SIZE, LIGHTGRAY);
		EndDrawing();
		frame++;
	}
	
	return 0;
}
