/*
File:   main.c
Author: Taylor Robbins
Date:   05\16\2026
Description: 
	** This is an example that uses sokol_app.h to create a window
	** on many platforms and sokol_gfx.h to interact with the
	** graphics API for that platform.
	**
	** NOTE: This is based on triangle-sapp.c from sokol-samples
*/

#if defined(_WIN32)
#define TARGET_IS_WINDOWS 1
#define TARGET_IS_LINUX   0
#define TARGET_IS_OSX     0
#elif defined(__APPLE__)
#define TARGET_IS_WINDOWS 0
#define TARGET_IS_LINUX   0
#define TARGET_IS_OSX     1
#elif defined(__linux__) || defined(__unix__)
#define TARGET_IS_WINDOWS 0
#define TARGET_IS_LINUX   1
#define TARGET_IS_OSX     0
#else
#error Unknown TARGET!
#endif

#if TARGET_IS_OSX
#define SOKOL_METAL
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
#else
#error main.c needs to be updated to support the current platform sokol_gfx.h choice!
#endif

#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

//NOTE: If you want to compile more than one shader implementation
//      into the same compilation unit, you have to modify the
//      generated header to resolve static variable name conflicts.
#define SOKOL_SHDC_IMPL
#include "basic_shader.glsl.h"

// application state
static struct
{
	sg_pipeline pip;
	sg_bindings bind;
	sg_pass_action pass_action;
} state;

void Appinit()
{
	sg_setup(&(sg_desc){
		.environment = sglue_environment(),
		.logger.func = slog_func,
	});
	
	// a vertex buffer with 3 vertices and view for binding
	float vertices[] = {
		// positions            // colors
		 0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
	};
	state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
		.data = SG_RANGE(vertices),
		.label = "vertex-buffer"
	});
	
	// create shader from code-generated sg_shader_desc
	sg_shader shd = sg_make_shader(basic_shader_shader_desc(sg_query_backend()));
	
	// create a pipeline object (default render states are fine for triangle)
	state.pip = sg_make_pipeline(&(sg_pipeline_desc){
		.shader = shd,
		// if the vertex layout doesn't have gaps, don't need to provide strides and offsets
		.layout = {
			.attrs = {
				[ATTR_basic_shader_position].format = SG_VERTEXFORMAT_FLOAT3,
				[ATTR_basic_shader_color0].format = SG_VERTEXFORMAT_FLOAT4
			}
		},
		.label = "triangle-pipeline"
	});
	
	// a pass action to clear framebuffer to black
	state.pass_action = (sg_pass_action) {
		.colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f } }
	};
}

void AppFrame()
{
	sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
	sg_apply_pipeline(state.pip);
	sg_apply_bindings(&state.bind);
	sg_draw(0, 3, 1);
	sg_end_pass();
	sg_commit();
}

void AppCleanup()
{
	sg_shutdown();
}

void AppEvent(const sapp_event* event)
{
	//TODO: Implement?
}

sapp_desc sokol_main(int argc, char* argv[])
{
	return (sapp_desc){
		.width = 640, .height = 480,
		.init_cb    = Appinit,
		.frame_cb   = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb   = AppEvent,
	};
}
