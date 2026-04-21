/*
File:   main.c
Author: Taylor Robbins
Date:   04\08\2026
Description: 
	** This is a simple graphical application that creates a Window and uses Vulkan to render a single triangle
*/

// For Apple See: https://github.com/ocornut/imgui/blob/master/examples/example_glfw_metal/main.mm

//These are defined in build_script.c args to compiler
#ifndef TARGET_IS_WINDOWS
#define TARGET_IS_WINDOWS 0
#endif
#ifndef TARGET_IS_LINUX
#define TARGET_IS_LINUX 0
#endif
#ifndef TARGET_IS_OSX
#define TARGET_IS_OSX 0
#endif
#if !TARGET_IS_WINDOWS && !TARGET_IS_LINUX && !TARGET_IS_OSX
#error TARGET_IS_X macro not set in compiler options!
#endif
#if (TARGET_IS_WINDOWS + TARGET_IS_LINUX + TARGET_IS_OSX) > 1
#error Multiple TARGET_IS_X macros set in compiler options!
#endif

#if TARGET_IS_WINDOWS
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdint.h>

#if TARGET_IS_WINDOWS
// #define GL_SILENCE_DEPRECATION //TODO: Do we want to do this?
#elif TARGET_IS_OSX
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#if TARGET_IS_OSX
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "imgui_impl_glfw.h"
#if TARGET_IS_WINDOWS
#include "imgui_impl_opengl3.h"
#elif TARGET_IS_OSX
#include "imgui_impl_metal.h"
#endif

int main(int argc, const char* argv[])
{
	#if DEBUG_BUILD
	printf("Hello Dear ImGui! (DEBUG)\n");
	#else
	printf("Hello Dear ImGui!\n");
	#endif
	
	int initResult = glfwInit();
	assert(initResult == GLFW_TRUE);
	
	#if TARGET_IS_WINDOWS
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	#elif TARGET_IS_OSX
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	#endif
	float monitorScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	GLFWwindow* window = glfwCreateWindow(640*monitorScale, 480*monitorScale, "Dear ImGui Window", /*monitor*/ nullptr, /*share*/ nullptr);
	assert(window != nullptr);
	#if TARGET_IS_WINDOWS
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	#endif
	
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
	bool glfwInitSuccess = ImGui_ImplGlfw_InitForOpenGL(window, true);
	assert(glfwInitSuccess);
	#if TARGET_IS_WINDOWS
	bool openglInitSuccess = ImGui_ImplOpenGL3_Init("#version 130");
	assert(openglInitSuccess);
	#elif TARGET_IS_OSX
	id <MTLDevice> device = MTLCreateSystemDefaultDevice();
	id <MTLCommandQueue> commandQueue = [device newCommandQueue];
	bool metalInitSuccess = ImGui_ImplMetal_Init(device);
	assert(metalInitSuccess);
	
	NSWindow* nsWindow = glfwGetCocoaWindow(window);
	CAMetalLayer* layer = [CAMetalLayer layer];
	layer.device = device;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	nsWindow.contentView.layer = layer;
	nsWindow.contentView.wantsLayer = YES;
	
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];
	#endif //TARGET_IS_OSX
	
	bool showDemoWindow = true;
    float clearColor[4] = {0.45f, 0.55f, 0.60f, 1.00f};
	while (!glfwWindowShouldClose(window))
	{
		#if TARGET_IS_OSX
		@autoreleasepool
		#endif
        {
			glfwPollEvents();
			if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) { ImGui_ImplGlfw_Sleep(10); continue; }
			
			int framebufferWidth, framebufferHeight;
			glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
			
			#if TARGET_IS_OSX
			layer.drawableSize = CGSizeMake(framebufferWidth, framebufferHeight);
			id<CAMetalDrawable> drawable = [layer nextDrawable];
			
			id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
			renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0] * clearColor[3], clearColor[1] * clearColor[3], clearColor[2] * clearColor[3], clearColor[3]);
			renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
			renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
			id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
			[renderEncoder pushDebugGroup:@"Dear ImGui Window"];
			#endif
			
			#if TARGET_IS_WINDOWS
			ImGui_ImplOpenGL3_NewFrame();
			#elif TARGET_IS_OSX
			ImGui_ImplMetal_NewFrame(renderPassDescriptor);
			#endif
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			if (showDemoWindow) { ImGui::ShowDemoWindow(&showDemoWindow); }
			else { glfwSetWindowShouldClose(window, true); }
			
			ImGui::Render();
			#if TARGET_IS_WINDOWS
			glViewport(0, 0, framebufferWidth, framebufferHeight);
			glClearColor(clearColor[0] * clearColor[4], clearColor[1] * clearColor[4], clearColor[2] * clearColor[4], clearColor[4]);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			#elif TARGET_IS_OSX
			ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
			[renderEncoder popDebugGroup];
			[renderEncoder endEncoding];
			[commandBuffer presentDrawable:drawable];
			[commandBuffer commit];
			#endif
		}
	}
	
	#if TARGET_IS_OSX
	ImGui_ImplMetal_Shutdown();
	#endif
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
