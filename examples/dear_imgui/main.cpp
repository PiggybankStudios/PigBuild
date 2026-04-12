/*
File:   main.c
Author: Taylor Robbins
Date:   04\08\2026
Description: 
	** This is a simple graphical application that creates a Window and uses Vulkan to render a single triangle
*/

#include <stdio.h>
#include <stdint.h>

#include "GLFW/glfw3.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"

int main(int argc, const char* argv[])
{
	#if DEBUG_BUILD
	printf("Hello Dear ImGui! (DEBUG)\n");
	#else
	printf("Hello Dear ImGui!\n");
	#endif
	
	int initResult = glfwInit();
	assert(initResult == GLFW_TRUE);
	// glfwWindowHint(int hint, int value)
	GLFWwindow* window = glfwCreateWindow(640, 480, "Dear ImGui Window", /*monitor*/ nullptr, /*share*/ nullptr);
	
	ImGuiContext* ctx = ImGui::CreateContext();
	ImGuiIO* io = &ImGui::GetIO();
	ImGuiPlatformIO* platformIo = &ImGui::GetPlatformIO(); 
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	
	// Setup Platform/Renderer backends
	bool glfwInitSuccess = ImGui_ImplGlfw_InitForVulkan(window, true);
	// bool metalInitSuccess = ImGui_ImplMetal_Init(id<MTLDevice> device);
	
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		// ImGui_ImplGlfw_NewFrame();
		// ImGui_ImplMetal_NewFrame();
		// ImGui::NewFrame();
		// ImGui::ShowDemoWindow();
	}
	
	
	glfwTerminate();
	return 0;
}
