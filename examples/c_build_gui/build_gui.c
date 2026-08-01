/*
File:   build_gui.c
Author: Taylor Robbins
Date:   05\16\2026
Description: 
	** This is a graphical program that allows the user to configure and visualize
	** the build process. It's compiled by build_script.c since it requires downloading
	** sokol and cross-compiling basic_shader.glsl but the main program still needs
	** to get built.
*/

#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build_base.h"
#define BUILD_SCRIPT_EXE_NAME "builder_gui" EXE_EXT
#define BUILD_SCRIPT_SOURCE_NAME "build_gui.c"
#define BUILD_SCRIPT_HASH_PATH "build_gui_hash.txt"
#include "pig_build.h"

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

#if TARGET_IS_WINDOWS
#define SOKOL_D3D11
// #define SOKOL_GLCORE
// #define SOKOL_WGPU
// #define SOKOL_VULKAN
#elif TARGET_IS_OSX
#define SOKOL_METAL
// #define SOKOL_GLCORE
#elif TARGET_IS_LINUX
#define SOKOL_GLCORE
// #define SOKOL_GLES3
// #define SOKOL_WGPU
// #define SOKOL_VULKAN
#else
#error main.c needs to be updated to support the current platform sokol_gfx.h choice!
#endif

typedef struct v2 v2;
struct v2
{
	r32 x, y;
};
typedef struct v3 v3;
struct v3
{
	r32 x, y, z;
};
typedef struct v4 v4;
struct v4
{
	r32 x, y, z, w;
};
typedef struct mat4 mat4;
struct mat4
{
	r32 r0c0, r1c0, r2c0, r3c0, r0c1, r1c1, r2c1, r3c1, r0c2, r1c2, r2c2, r3c2, r0c3, r1c3, r2c3, r3c3;
};

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

IF_WINDOWS(bool isMsvcInitialized = false);

void BuildProgram()
{
	IF_WINDOWS(InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized));
	
	bool usingMsvcCompiler = BUILDING_ON_WINDOWS;
	Str compiler = usingMsvcCompiler ? StrLit("cl") : StrLit("clang");
	
	CliArgs args = EMPTY;
	AddTaggedArg(&args,   "cl",    CL_NO_LOGO);
	AddTaggedArg(&args,   "cl",    CL_FULL_FILE_PATHS);
	AddTaggedArg(&args,   "clang", CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&args, "cl",    CL_BINARY_FILE,    "hello_world" EXE_EXT);
	AddTaggedArgNt(&args, "clang", CLANG_OUTPUT_FILE, "hello_world" EXE_EXT);
	AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/main.c");
	
	StrArray tags = EMPTY;
	AddStr(&tags, compiler);
	AddStrLit(&tags, BUILDING_ON_NAME);
	RunCliProgramAndExitOnFailureTags(compiler, tags, &args, StrLit("Failed to compile main.c!"));
	AssertFileExist(StrLit("hello_world" EXE_EXT), true);
}

void Appinit(void)
{
	IF_WINDOWS(isMsvcInitialized = WasMsvcDevBatchRun());
	
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
				[ATTR_basic_shader_texCoord].format = SG_VERTEXFORMAT_FLOAT2,
				[ATTR_basic_shader_color].format = SG_VERTEXFORMAT_FLOAT4
			}
		},
		.label = "triangle-pipeline"
	});
	
	// a pass action to clear framebuffer to black
	state.pass_action = (sg_pass_action) {
		.colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f } }
	};
}

void AppFrame(void)
{
	sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
	sg_apply_pipeline(state.pip);
	sg_apply_bindings(&state.bind);
	sg_draw(0, 3, 1);
	sg_end_pass();
	sg_commit();
}

void AppCleanup(void)
{
	sg_shutdown();
}

void AppEvent(const sapp_event* event)
{
	switch (event->type)
	{
		case SAPP_EVENTTYPE_KEY_DOWN:
		{
			// +==============================+
			// |  Handle ENTER Key to Build   |
			// +==============================+
			if (event->key_code == SAPP_KEYCODE_ENTER)
			{
				WriteLine("Building...");
				BuildProgram();
			}
		} break;
		case SAPP_EVENTTYPE_KEY_UP:            break;
		case SAPP_EVENTTYPE_CHAR:              break;
		case SAPP_EVENTTYPE_MOUSE_DOWN:        break;
		case SAPP_EVENTTYPE_MOUSE_UP:          break;
		case SAPP_EVENTTYPE_MOUSE_SCROLL:      break;
		case SAPP_EVENTTYPE_MOUSE_MOVE:        break;
		case SAPP_EVENTTYPE_MOUSE_ENTER:       break;
		case SAPP_EVENTTYPE_MOUSE_LEAVE:       break;
		case SAPP_EVENTTYPE_TOUCHES_BEGAN:     break;
		case SAPP_EVENTTYPE_TOUCHES_MOVED:     break;
		case SAPP_EVENTTYPE_TOUCHES_ENDED:     break;
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED: break;
		case SAPP_EVENTTYPE_RESIZED:           break;
		case SAPP_EVENTTYPE_ICONIFIED:         break;
		case SAPP_EVENTTYPE_RESTORED:          break;
		case SAPP_EVENTTYPE_FOCUSED:           break;
		case SAPP_EVENTTYPE_UNFOCUSED:         break;
		case SAPP_EVENTTYPE_SUSPENDED:         break;
		case SAPP_EVENTTYPE_RESUMED:           break;
		case SAPP_EVENTTYPE_QUIT_REQUESTED:    break;
		case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  break;
		case SAPP_EVENTTYPE_FILES_DROPPED:     break;
		default: break;
	}
	//TODO: Implement?
}

sapp_desc sokol_main(int argc, char* argv[])
{
	RecompileIfNeeded(StrArray_Empty);
	
	return (sapp_desc){
		.width = 640, .height = 480,
		.init_cb    = Appinit,
		.frame_cb   = AppFrame,
		.cleanup_cb = AppCleanup,
		.event_cb   = AppEvent,
	};
}
