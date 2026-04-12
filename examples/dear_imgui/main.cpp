/*
File:   main.c
Author: Taylor Robbins
Date:   04\08\2026
Description: 
	** This is a simple graphical application that creates a Window and uses Vulkan to render a single triangle
*/

// For Apple See: https://github.com/ocornut/imgui/blob/master/examples/example_glfw_metal/main.mm

#include <stdio.h>
#include <stdint.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

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
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	float monitorScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	GLFWwindow* window = glfwCreateWindow(640*monitorScale, 480*monitorScale, "Dear ImGui Window", /*monitor*/ nullptr, /*share*/ nullptr);
	assert(window != nullptr);
	
	IMGUI_CHECKVERSION();
	ImGuiContext* ctx = ImGui::CreateContext();
	ImGuiIO* io = &ImGui::GetIO();
	ImGuiPlatformIO* platformIo = &ImGui::GetPlatformIO(); 
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(monitorScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = monitorScale;
	
	// Setup Platform/Renderer backends
	id <MTLDevice> device = MTLCreateSystemDefaultDevice();
	id <MTLCommandQueue> commandQueue = [device newCommandQueue];
	bool glfwInitSuccess = ImGui_ImplGlfw_InitForOpenGL(window, true);
	bool metalInitSuccess = ImGui_ImplMetal_Init(device);
	
	NSWindow* nsWindow = glfwGetCocoaWindow(window);
	CAMetalLayer* layer = [CAMetalLayer layer];
	layer.device = device;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	nsWindow.contentView.layer = layer;
	nsWindow.contentView.wantsLayer = YES;
	
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];
	
	bool showDemoWindow = true;
    float clearColor[4] = {0.45f, 0.55f, 0.60f, 1.00f};
	while (!glfwWindowShouldClose(window))
	{
		@autoreleasepool
        {
			glfwPollEvents();
			
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			layer.drawableSize = CGSizeMake(width, height);
			id<CAMetalDrawable> drawable = [layer nextDrawable];
			
			id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
			renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
			renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
			renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
			id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
			[renderEncoder pushDebugGroup:@"Dear ImGui Window"];
			
			ImGui_ImplMetal_NewFrame(renderPassDescriptor);
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			if (showDemoWindow) { ImGui::ShowDemoWindow(&showDemoWindow); }
			else { glfwSetWindowShouldClose(window, true); }
			
			ImGui::Render();
			ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
			
			[renderEncoder popDebugGroup];
			[renderEncoder endEncoding];
			
			[commandBuffer presentDrawable:drawable];
			[commandBuffer commit];
		}
	}
	
	ImGui_ImplMetal_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
