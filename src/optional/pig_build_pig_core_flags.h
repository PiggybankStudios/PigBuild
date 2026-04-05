/*
File:   pig_build_pig_core_flags.h
Author: Taylor Robbins
Date:   06\23\2025
Description: 
	** Contains all the flags that PigCore uses to compile in all it's configurations.
	** These are pulled into a separate file so that various Pig Core based programs
	** can use these functions as a starting point for their own build scripts.
*/

//test

#ifndef _PIG_BUILD_PIG_CORE_FLAGS_H
#define _PIG_BUILD_PIG_CORE_FLAGS_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_cli_flags.h"
#include "pig_build_tags.h"
#include "pig_build_arg_list.h"
#include "pig_build_android.h"
#include "pig_build_emscripten.h"
#include "pig_build_playdate.h"
#include "pig_build_orca.h"

// Build Targets Tags
#define T_PIG_CORE        "|PigCore"
#define T_PIGGEN          "|Piggen"
#define T_TRACY           "|Tracy"
#define T_DEAR_IMGUI      "|Imgui"
#define T_PHYSX           "|PhysX"

// Common build_config.h Define Tags
#define T_BUILD_WITH_SOKOL_APP "|BUILD_WITH_SOKOL_APP"
#define T_BUILD_WITH_SOKOL_GFX "|BUILD_WITH_SOKOL_GFX"
#define T_BUILD_WITH_FREETYPE  "|BUILD_WITH_FREETYPE"
#define T_BUILD_WITH_GTK       "|BUILD_WITH_GTK"
#define T_BUILD_WITH_RAYLIB    "|BUILD_WITH_RAYLIB"
#define T_BUILD_WITH_BOX2D     "|BUILD_WITH_BOX2D"
#define T_BUILD_WITH_SDL       "|BUILD_WITH_SDL"
#define T_BUILD_WITH_OPENVR    "|BUILD_WITH_OPENVR"
#define T_BUILD_WITH_PHYSX     "|BUILD_WITH_PHYSX"
#define T_BUILD_WITH_HTTP      "|BUILD_WITH_HTTP"
#define T_BUILD_WITH_IMGUI     "|BUILD_WITH_IMGUI"
#define T_USE_EMSCRIPTEN       "|USE_EMSCRIPTEN"
#define T_DUMP_ASSEMBLY        "|DUMP_ASSEMBLY"
#define T_DUMP_PREPROCESSOR    "|DUMP_PREPROCESSOR"

#define T_NOT_WASM       T_WASM "==false"
#define T_NOT_EMSCRIPTEN T_USE_EMSCRIPTEN "==false"

void FillPigCoreFlags(CliArgList* compilerFlags, CliArgList* linkerFlags, Str pigCoreThirdPartyPath)
{
	// +--------------------------------------------------------------+
	// |                        Compiler Flags                        |
	// +--------------------------------------------------------------+
	// +====================================+
	// | Common MSVC Compiler/Linker Flags  |
	// +====================================+
	AddTaggedArg(compilerFlags, T_MSVC_CL, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddTaggedArg(compilerFlags, T_MSVC_CL, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddTaggedArg(linkerFlags,   T_MSVC_CL, LINK_DISABLE_INCREMENTAL);
	
	// +==============================+
	// | Common Clang Compiler flags  |
	// +==============================+
	AddTaggedArg(compilerFlags, T_CLANG, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	// AddTaggedArgNt(compilerFlags, T_CLANG, CLANG_DEFINE, "_GNU_SOURCE"); //TODO: Maybe we need this for some GNU standard library features?
	#if !BUILDING_ON_OSX
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_M_FLAG, "ssse3"); //For MeowHash to work we need sse3 support
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_M_FLAG, "aes"); //For MeowHash to work we need aes support
	#endif
	//TODO: Really we should do `pkg-config --cflags gtk4`
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_M_FLAG, "fpmath=sse");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_M_FLAG, "sse");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_M_FLAG, "sse2");
	AddTaggedArg(compilerFlags,   T_CLANG T_UNIX T_BUILD_WITH_GTK, "-pthread");
	
	// +==========================================================+
	// | Language-specific Flags (C vs. C++ vs. Objective-C/C++)  |
	// +==========================================================+
	AddTaggedArgNt(compilerFlags,  T_MSVC_CL T_LANG_C, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(compilerFlags,  T_MSVC_CL T_LANG_C, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	// AddTaggedArg(compilerFlags, T_MSVC_CL T_LANG_C, CL_ENABLE_ADDRESS_SANATIZER);
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_LANG_C, CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	AddTaggedArgNt(compilerFlags,  T_MSVC_CL T_LANG_CPP, CL_LANG_VERSION, "c++20");
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_LANG_CPP, CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_LANG_CPP, CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_LANG_CPP, CLANG_LANG_VERSION, "c++20"); // TODO: What option should we actually choose here?
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_LANG_CPP, CLANG_SYSTEM_LIBRARY, "stdc++"); // Fixes tracy.so link-time errors regarding stuff like `operator delete(void*, unsigned long)`
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_LANG_OBJECTIVEC, CLANG_LANG_VERSION, "gnu2x"); //NOTE: We still ask for gnu23 features in Objective-C mode, the distinguishing factor is that we compile a .m file not a .c file
	AddTaggedArg(compilerFlags,    T_CLANG   T_LANG_OBJECTIVEC, CLANG_ENABLE_OBJC_ARC);
	//TODO: Figure out why these are needed when linking with imgui.o with Clang on Linux
	// AddTaggedArg(compilerFlags, T_CLANG   T_LANG_C T_BUILD_WITH_IMGUI, "-lstdc++"); //TODO: Since this is being added to clang_LangCppFlags flags now (was needed for tracy.so as well as imgui.so) we probably don't need to add it here
	AddTaggedArg(compilerFlags,    T_CLANG   T_LANG_C T_BUILD_WITH_IMGUI, "-fno-threadsafe-statics"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
	
	// +===============================+
	// | Debug/Release Dependent Flags |
	// +===============================+
	AddTaggedArg(compilerFlags,   T_MSVC_CL T_DEBUG_BUILD,        CL_DEBUG_INFO);
	AddTaggedArg(compilerFlags,   T_MSVC_CL T_DEBUG_BUILD,        CL_STD_LIB_DYNAMIC_DBG);
	AddTaggedArg(compilerFlags,   T_MSVC_CL T_RELEASE_BUILD,      CL_STD_LIB_DYNAMIC);
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD,        CL_OPTIMIZATION_LEVEL, "d");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "y");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_RELEASE_BUILD,      CL_OPTIMIZATION_LEVEL, "t");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_DEBUG_BUILD,   CLANG_OPTIMIZATION_LEVEL, "0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_RELEASE_BUILD, CLANG_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_DEBUG_BUILD,   CLANG_DEBUG_INFO, "dwarf-4");
	
	// +==============================+
	// |      Configure warnings      |
	// +==============================+
	AddTaggedArgNt(compilerFlags,  T_MSVC_CL, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	//We set the highest warning level above and then remove the warnings we don't care about here
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	AddTaggedArgNt(compilerFlags,  T_CLANG,   CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_CMD_LINE_ARG);
	//We don't care about these warnings in DEBUG_BUILDs, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
	AddTaggedArgInt(compilerFlags, T_MSVC_CL T_DEBUG_BUILD, CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_DEBUG_BUILD, CLANG_DISABLE_WARNING, "unused-parameter");
	AddTaggedArgNt(compilerFlags,  T_CLANG   T_DEBUG_BUILD, CLANG_DISABLE_WARNING, "unused-variable");
	
	// +==============================+
	// |     Include Directories      |
	// +==============================+
	AddTaggedArgNt(compilerFlags, T_MSVC_CL,      CL_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_INCLUDE_DIR, "[ROOT]");
	//TODO: Really we should do `pkg-config dbus-1 --cflags`
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_INCLUDE_DIR, "/usr/include/dbus-1.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/dbus-1.0/include"); //This was the path on Lubuntu
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX, CLANG_INCLUDE_DIR, "/usr/lib64/dbus-1.0/include"); //This is the path on Fedora Workstation
	Str freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
	AddTaggedArgStr(compilerFlags, T_MSVC_CL T_BUILD_WITH_FREETYPE, CL_INCLUDE_DIR, freetypeDir);
	AddTaggedArgStr(compilerFlags, T_CLANG   T_BUILD_WITH_FREETYPE, CLANG_INCLUDE_DIR, freetypeDir);
	Str plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
	AddTaggedArgStr(compilerFlags, T_MSVC_CL T_BUILD_WITH_FREETYPE, CL_INCLUDE_DIR, plutosvgDir);
	AddTaggedArgStr(compilerFlags, T_CLANG   T_BUILD_WITH_FREETYPE, CLANG_INCLUDE_DIR, plutosvgDir);
	//TODO: Really we should do `pkg-config --cflags gtk4`
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/gtk-4.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/glib-2.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/glib-2.0/include");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/x86_64-linux-gnu");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/cairo");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/pango-1.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/harfbuzz");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/freetype2");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/libpng16");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/libmount");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/blkid");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/fribidi");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/pixman-1");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/gdk-pixbuf-2.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/webp");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/include/graphene-1.0");
	AddTaggedArgNt(compilerFlags, T_CLANG T_UNIX T_BUILD_WITH_GTK, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/graphene-1.0/include");
	
	// +==============================+
	// |     Library Directories      |
	// +==============================+
	AddTaggedArgNt(linkerFlags,  T_MSVC_CL T_DEBUG_BUILD,        LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_debug");
	AddTaggedArgNt(linkerFlags,  T_MSVC_CL T_RELEASE_BUILD,      LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_release");
	AddTaggedArgStr(linkerFlags, T_CLANG T_UNIX T_DEBUG_BUILD,   CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
	AddTaggedArgStr(linkerFlags, T_CLANG T_UNIX T_RELEASE_BUILD, CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
	
	// +==============================+
	// |          Libraries           |
	// +==============================+
	AddTaggedArgNt(linkerFlags, T_CLANG T_NOT_WASM, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddTaggedArgNt(linkerFlags, T_CLANG T_NOT_WASM, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_RAYLIB, CLI_QUOTED_ARG, "raylib.lib"); //NOTE: raylib.lib MUST be before User32.lib and others
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_RAYLIB, CLI_QUOTED_ARG, "Kernel32.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_RAYLIB, CLI_QUOTED_ARG, "Winmm.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_BOX2D,  CLI_QUOTED_ARG, "box2d.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_SDL,    CLI_QUOTED_ARG, "SDL2.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_OPENVR, CLI_QUOTED_ARG, "openvr_api.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_PHYSX,  CLI_QUOTED_ARG, "PhysX_static_64.lib");
	AddTaggedArgNt(linkerFlags, T_MSVC_CL T_PIG_CORE T_BUILD_WITH_HTTP,   CLI_QUOTED_ARG, "Winhttp.lib");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_UNIX, CLANG_SYSTEM_LIBRARY, "pthread");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_LINUX, CLANG_SYSTEM_LIBRARY, "fontconfig");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_LINUX T_BUILD_WITH_SOKOL_GFX, CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_UNIX T_BUILD_WITH_BOX2D, CLANG_SYSTEM_LIBRARY, "box2d");
	//OSX Frameworks
	AddTaggedArgNt(linkerFlags, T_CLANG T_OSX, CLANG_FRAMEWORK, "CoreText"); //For functions like CTFontCollectionCreateMatchingFontDescriptors in os_font.h OsReadPlatformFont
	AddTaggedArgNt(linkerFlags, T_CLANG T_OSX T_BUILD_WITH_SOKOL_APP, CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(linkerFlags, T_CLANG T_OSX T_BUILD_WITH_SOKOL_APP, CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(linkerFlags, T_CLANG T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "CoreFoundation");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_OSX T_BUILD_WITH_SOKOL_APP, CLANG_FRAMEWORK, "AudioToolbox");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "Foundation");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "UIKit");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "AudioToolbox");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "AVFoundation");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "MetalKit");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "OpenGL");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "OpenGLES");
	// AddTaggedArgNt(linkerFlags, T_CLANG T_PIG_CORE T_OSX T_BUILD_WITH_SOKOL_GFX, CLANG_FRAMEWORK, "GLKit");
	//TODO: Really we should do `pkg-config dbus-1 --libs`
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "dbus-1");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_SOKOL_APP, CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_SOKOL_APP, CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_SOKOL_APP, CLANG_SYSTEM_LIBRARY, "Xcursor");
	//TODO: Really we should do `pkg-config --libs gtk4`
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "gtk-4");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "pangocairo-1.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "pango-1.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "harfbuzz");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "gdk_pixbuf-2.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "cairo-gobject");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "cairo");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "vulkan");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "graphene-1.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "gio-2.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "gobject-2.0");
	AddTaggedArgNt(linkerFlags, T_CLANG T_LINUX T_BUILD_WITH_GTK, CLANG_SYSTEM_LIBRARY, "glib-2.0");
	
	// +======================================+
	// | DUMP_ASSEMBLY and DUMP_PREPROCESSOR  |
	// +======================================+
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_DUMP_ASSEMBLY,     CL_GENERATE_ASSEMB_LISTING, "s"); //Generate assembly listing files with source code included
	AddTaggedArg(compilerFlags,   T_MSVC_CL T_DUMP_PREPROCESSOR, CL_PRECOMPILE_ONLY);
	AddTaggedArg(compilerFlags,   T_MSVC_CL T_DUMP_PREPROCESSOR, CL_PRECOMPILE_PRESERVE_COMMENTS);
	AddTaggedArg(compilerFlags,   T_CLANG   T_DUMP_PREPROCESSOR, CLANG_PRECOMPILE_ONLY);
	AddTaggedArg(compilerFlags,   T_CLANG   T_DUMP_PREPROCESSOR, CLANG_INCLUDE_MACROS);
	
	// +==============================+
	// |   Flags for Building Tracy   |
	// +==============================+
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY, CL_INCLUDE_DIR,    "[ROOT]/third_party/tracy");
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_INCLUDE_DIR, "[ROOT]/third_party/tracy");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY, CL_DEFINE, "TRACY_ENABLE");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY, CL_DEFINE, "TRACY_EXPORTS");
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_DEFINE, "TRACY_ENABLE");
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_DEFINE, "TRACY_EXPORTS");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY, CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY, CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_DISABLE_WARNING, CLANG_WARNING_SHADOWING); // declaration shadows a local variable
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FIELD_INITIALIZERS); // missing field 'extra' initializer
	AddTaggedArgNt(compilerFlags, T_CLANG   T_TRACY, CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH); // unannotated fall-through between switch labels
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_TRACY T_DUMP_ASSEMBLY, CL_ASSEMB_LISTING_FILE, "tracy.asm");
	
	// +===============================+
	// | Flags for Building Dear ImGui |
	// +===============================+
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_DEAR_IMGUI, CL_INCLUDE_DIR,    "[ROOT]/third_party/imgui");
	AddTaggedArgNt(compilerFlags, T_CLANG   T_DEAR_IMGUI, CLANG_INCLUDE_DIR, "[ROOT]/third_party/imgui");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_DEAR_IMGUI T_DUMP_ASSEMBLY, CL_ASSEMB_LISTING_FILE, "imgui.asm");
	
	// +===============================+
	// | Flags for Building PhysX_capi |
	// +===============================+
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PHYSX, CL_INCLUDE_DIR,    "[ROOT]/third_party/physx");
	AddTaggedArgNt(compilerFlags, T_CLANG   T_PHYSX, CLANG_INCLUDE_DIR, "[ROOT]/third_party/physx");
	AddTaggedArgNt(compilerFlags, T_MSVC_CL T_PHYSX T_DUMP_ASSEMBLY, CL_ASSEMB_LISTING_FILE, "physx.asm");
	
	// +==============================+
	// |       clang_WasmFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, T_CLANG T_WASM, CLANG_TARGET_ARCHITECTURE, "wasm32");
		AddTaggedArgNt(compilerFlags, T_CLANG T_WASM, CLANG_M_FLAG, "bulk-memory");
		AddTaggedArgNt(compilerFlags, T_CLANG T_WASM, CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArg(compilerFlags,   T_CLANG T_WASM T_DEBUG_BUILD,   CLANG_DEBUG_INFO_DEFAULT);
		AddTaggedArgNt(compilerFlags, T_CLANG T_WASM T_RELEASE_BUILD, CLANG_OPTIMIZATION_LEVEL, "2");
	}
	
	// +==============================+
	// |        clang_WebFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_INCLUDE_DIR, "[ROOT]/wasm/std/include");
		AddTaggedArg(compilerFlags,   T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_NO_ENTRYPOINT);
		AddTaggedArg(compilerFlags,   T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_ALLOW_UNDEFINED);
		AddTaggedArg(compilerFlags,   T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_NO_STD_LIBRARIES);
		AddTaggedArg(compilerFlags,   T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_NO_STD_INCLUDES);
		AddTaggedArgNt(compilerFlags, T_CLANG T_WEB T_NOT_EMSCRIPTEN, CLANG_EXPORT_SYMBOL, "__heap_base");
		
		AddTaggedArgNt(compilerFlags, T_EMCC T_WEB,  EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddTaggedArgNt(compilerFlags, T_EMCC T_WEB,  EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
}

#endif //  _PIG_BUILD_PIG_CORE_FLAGS_H
