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
#include "pig_build_str8.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"

void FillPigCoreFlags(CliArgList* compilerFlags, CliArgList* linkerFlags,
	Str8 pigCoreThirdPartyPath,
	Str8 androidNdkDir, Str8 androidNdkToolchainDir,
	Str8 orcaSdkPath,
	Str8 playdateSdkDir, Str8 playdateSdkDir_C_API)
{
	// +--------------------------------------------------------------+
	// |                        Compiler Flags                        |
	// +--------------------------------------------------------------+
	// +====================================+
	// | Common MSVC Compiler/Linker Flags  |
	// +====================================+
	AddTaggedArg(compilerFlags, EXE_MSVC_CL, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddTaggedArg(compilerFlags, EXE_MSVC_CL, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddTaggedArg(linkerFlags, EXE_MSVC_CL, LINK_DISABLE_INCREMENTAL);
	
	// +==============================+
	// | Common Clang Compiler flags  |
	// +==============================+
	AddTaggedArg(compilerFlags, EXE_CLANG, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	// AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_DEFINE, "_GNU_SOURCE"); //TODO: Maybe we need this for some GNU standard library features?
	#if !BUILDING_ON_OSX
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_M_FLAG, "ssse3"); //For MeowHash to work we need sse3 support
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_M_FLAG, "aes"); //For MeowHash to work we need aes support
	#endif
	//TODO: Really we should do `pkg-config --cflags gtk4`
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "fpmath=sse");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "sse");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "sse2");
	AddTaggedArg(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", "-pthread");
	
	// +==========================================================+
	// | Language-specific Flags (C vs. C++ vs. Objective-C/C++)  |
	// +==========================================================+
	AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL "|LangC", CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL "|LangC", CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	// AddTaggedArg(compilerFlags, EXE_MSVC_CL "|LangC", CL_ENABLE_ADDRESS_SANATIZER);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG   "|LangC", CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL "|LangCpp", CL_LANG_VERSION, "c++20");
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|LangCpp", CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|LangCpp", CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG   "|LangCpp", CLANG_LANG_VERSION, "c++20"); // TODO: What option should we actually choose here?
	AddTaggedArgNt(compilerFlags,  EXE_CLANG   "|LangCpp", CLANG_SYSTEM_LIBRARY, "stdc++"); // Fixes tracy.so link-time errors regarding stuff like `operator delete(void*, unsigned long)`
	AddTaggedArgNt(compilerFlags,  EXE_CLANG.  "|LangObjectiveC", CLANG_LANG_VERSION, "gnu2x"); //NOTE: We still ask for gnu23 features in Objective-C mode, the distinguishing factor is that we compile a .m file not a .c file
	AddTaggedArg(compilerFlags,    EXE_CLANG.  "|LangObjectiveC", CLANG_ENABLE_OBJC_ARC);
	//TODO: Figure out why these are needed when linking with imgui.o with Clang on Linux
	// AddTaggedArg(compilerFlags, EXE_CLANG   "|LangC|BUILD_WITH_IMGUI", "-lstdc++"); //TODO: Since this is being added to clang_LangCppFlags flags now (was needed for tracy.so as well as imgui.so) we probably don't need to add it here
	AddTaggedArg(compilerFlags,    EXE_CLANG   "|LangC|BUILD_WITH_IMGUI", "-fno-threadsafe-statics"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
	
	// +===============================+
	// | Debug/Release Dependent Flags |
	// +===============================+
	AddTaggedArg(compilerFlags,   EXE_MSVC_CL "|DEBUG_BUILD", CL_DEBUG_INFO);
	AddTaggedArg(compilerFlags,   EXE_MSVC_CL "|DEBUG_BUILD==true", CL_STD_LIB_DYNAMIC_DBG);
	AddTaggedArg(compilerFlags,   EXE_MSVC_CL "|DEBUG_BUILD==false", CL_STD_LIB_DYNAMIC);
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==true",  CL_OPTIMIZATION_LEVEL, "d");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "y");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "t");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|LinuxOrOsx|DEBUG_BUILD==true",  CLANG_OPTIMIZATION_LEVEL, "0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|LinuxOrOsx|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|LinuxOrOsx|DEBUG_BUILD", CLANG_DEBUG_INFO, "dwarf-4");
	
	// +==============================+
	// |      Configure warnings      |
	// +==============================+
	AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,   CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,   CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	//We set the highest warning level above and then remove the warnings we don't care about here
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_DISABLE_WARNING, CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL,                CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,                  CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,                  CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,                  CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,                  CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG,                  CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_CMD_LINE_ARG);
	//We don't care about these warnings in DEBUG_BUILDs, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
	AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	AddTaggedArgNt(compilerFlags,  EXE_CLANG   "|DEBUG_BUILD", CLANG_DISABLE_WARNING, "unused-parameter");
	AddTaggedArgNt(compilerFlags,  EXE_CLANG   "|DEBUG_BUILD", CLANG_DISABLE_WARNING, "unused-variable");
	
	// +==============================+
	// |     Include Directories      |
	// +==============================+
	AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "[ROOT]");
	//TODO: Really we should do `pkg-config dbus-1 --cflags`
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/include/dbus-1.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/dbus-1.0/include"); //This was the path on Lubuntu
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/lib64/dbus-1.0/include"); //This is the path on Fedora Workstation
	Str8 freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
	AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|BUILD_WITH_FREETYPE", CL_INCLUDE_DIR, freetypeDir);
	AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|BUILD_WITH_FREETYPE", CL_INCLUDE_DIR, plutosvgDir);
	Str8 plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
	AddTaggedArgStr(compilerFlags, EXE_CLANG "|BUILD_WITH_FREETYPE", CLANG_INCLUDE_DIR, freetypeDir);
	AddTaggedArgStr(compilerFlags, EXE_CLANG "|BUILD_WITH_FREETYPE", CLANG_INCLUDE_DIR, plutosvgDir);
	//TODO: Really we should do `pkg-config --cflags gtk4`
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/gtk-4.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/glib-2.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/glib-2.0/include");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/x86_64-linux-gnu");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/cairo");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/pango-1.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/harfbuzz");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/freetype2");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/libpng16");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/libmount");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/blkid");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/fribidi");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/pixman-1");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/gdk-pixbuf-2.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/webp");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/include/graphene-1.0");
	AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/graphene-1.0/include");
	
	// +==============================+
	// |     Library Directories      |
	// +==============================+
	AddTaggedArgNt(linkerFlags,  EXE_MSVC_CL "|DEBUG_BUILD==true",  LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_debug");
	AddTaggedArgNt(linkerFlags,  EXE_MSVC_CL "|DEBUG_BUILD==false", LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_release");
	AddTaggedArgStr(linkerFlags, EXE_CLANG   "|LinuxOrOsx|DEBUG_BUILD==true",  CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
	AddTaggedArgStr(linkerFlags, EXE_CLANG   "|LinuxOrOsx|DEBUG_BUILD==false", CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
	
	// +==============================+
	// |          Libraries           |
	// +==============================+
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Wasm==false", CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Wasm==false", CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "raylib.lib"); //NOTE: raylib.lib MUST be before User32.lib and others
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore", CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore", CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore", CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore", CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore", CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "Kernel32.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "Winmm.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_BOX2D",  CLI_QUOTED_ARG, "box2d.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_SDL",    CLI_QUOTED_ARG, "SDL2.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_OPENVR", CLI_QUOTED_ARG, "openvr_api.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_PHYSX",  CLI_QUOTED_ARG, "PhysX_static_64.lib");
	AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|PigCore|BUILD_WITH_HTTP",   CLI_QUOTED_ARG, "Winhttp.lib");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|LinuxOrOsx", CLANG_SYSTEM_LIBRARY, "pthread");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|Linux", CLANG_SYSTEM_LIBRARY, "fontconfig");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|Linux|BUILD_WITH_SOKOL_GFX", CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|LinuxOrOsx|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d");
	//OSX Frameworks
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "CoreText");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "CoreFoundation");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "AudioToolbox");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "Foundation");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "UIKit");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "AudioToolbox");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "AVFoundation");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "MetalKit");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "OpenGL");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "OpenGLES");
	// AddTaggedArgNt(linkerFlags, EXE_CLANG "|PigCore|OSX|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "GLKit");
	//TODO: Really we should do `pkg-config dbus-1 --libs`
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux", CLANG_SYSTEM_LIBRARY, "dbus-1");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_SOKOL_APP", CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_SOKOL_APP", CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_SOKOL_APP", CLANG_SYSTEM_LIBRARY, "Xcursor");
	//TODO: Really we should do `pkg-config --libs gtk4`
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "gtk-4");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "pangocairo-1.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "pango-1.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "harfbuzz");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "gdk_pixbuf-2.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "cairo-gobject");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "cairo");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "vulkan");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "graphene-1.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "gio-2.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "gobject-2.0");
	AddTaggedArgNt(linkerFlags, EXE_CLANG "|Linux|BUILD_WITH_GTK", CLANG_SYSTEM_LIBRARY, "glib-2.0");
	
	// +======================================+
	// | DUMP_ASSEMBLY and DUMP_PREPROCESSOR  |
	// +======================================+
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DUMP_ASSEMBLY", CL_GENERATE_ASSEMB_LISTING, "s"); //Generate assembly listing files with source code included
	AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DUMP_PREPROCESSOR", CL_PRECOMPILE_ONLY);
	AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DUMP_PREPROCESSOR", CL_PRECOMPILE_PRESERVE_COMMENTS);
	AddTaggedArg(compilerFlags, EXE_CLANG "|DUMP_PREPROCESSOR", CLANG_PRECOMPILE_ONLY);
	AddTaggedArg(compilerFlags, EXE_CLANG "|DUMP_PREPROCESSOR", CLANG_INCLUDE_MACROS);
	
	// +==============================+
	// |   Flags for Building Tracy   |
	// +==============================+
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy", CL_INCLUDE_DIR,    "[ROOT]/third_party/tracy");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_INCLUDE_DIR, "[ROOT]/third_party/tracy");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy", CL_DEFINE, "TRACY_ENABLE");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy", CL_DEFINE, "TRACY_EXPORTS");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_DEFINE, "TRACY_ENABLE");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_DEFINE, "TRACY_EXPORTS");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy", CL_CONFIGURE_EXCEPTION_HANDLING, "s"); //enable stack-unwinding
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy", CL_CONFIGURE_EXCEPTION_HANDLING, "c"); //extern "C" functions don't through exceptions
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_DISABLE_WARNING, CLANG_WARNING_SHADOWING); // declaration shadows a local variable
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FIELD_INITIALIZERS); // missing field 'extra' initializer
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Tracy", CLANG_DISABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH); // unannotated fall-through between switch labels
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Tracy|DUMP_ASSEMBLY", CL_ASSEMB_LISTING_FILE, "tracy.asm");
	
	// +===============================+
	// | Flags for Building Dear ImGui |
	// +===============================+
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Imgui", CL_INCLUDE_DIR,    "[ROOT]/third_party/imgui");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|Imgui", CLANG_INCLUDE_DIR, "[ROOT]/third_party/imgui");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Imgui|DUMP_ASSEMBLY", CL_ASSEMB_LISTING_FILE, "imgui.asm");
	
	// +===============================+
	// | Flags for Building PhysX_capi |
	// +===============================+
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|PhysX", CL_INCLUDE_DIR,    "[ROOT]/third_party/physx");
	AddTaggedArgNt(compilerFlags, EXE_CLANG   "|PhysX", CLANG_INCLUDE_DIR, "[ROOT]/third_party/physx");
	AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|PhysX|DUMP_ASSEMBLY", CL_ASSEMB_LISTING_FILE, "physx.asm");
	
	// +==============================+
	// |      clang_AndroidFlags      |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android|DEBUG_BUILD==true",  CLANG_OPTIMIZATION_LEVEL, "0");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Android", CLANG_STDLIB_FOLDER, JoinStrings2(androidNdkToolchainDir, StrLit("/sysroot"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Android", CLANG_INCLUDE_DIR, JoinStrings2(androidNdkDir, StrLit("/sources/android/native_app_glue"), false));
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android|DEBUG_BUILD", CLANG_DEBUG_INFO_DEFAULT); //TODO: Should we do dwarf-4 debug info instead?
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_DEFINE, "pig_core_EXPORTS"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_DEFINE, "ANDROID"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_DEFINE, "_FORTIFY_SOURCE=2"); //TODO: Can we remove this?
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_DATA_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_FUNCTION_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_UNWIND_TABLES);
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_STACK_PROTECTOR_STRONG);
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_NO_CANONICAL_PREFIXES);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_ENABLE_WARNING, "format");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_ENABLE_WARNING, "error=format-security");
		AddTaggedArg(compilerFlags, EXE_CLANG "|Android", CLANG_NO_STDLIB_CPP);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Android", CLANG_Q_FLAG, "unused-arguments");
	}
	
	// +==============================+
	// |    clang_AndroidLinkFlags    |
	// +==============================+
	{
		AddTaggedArg(linkerFlags, EXE_CLANG "|Android", CLANG_fPIC);
		AddTaggedArgStr(linkerFlags, EXE_CLANG "|Android|DEBUG_BUILD==true",  CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
		AddTaggedArgStr(linkerFlags, EXE_CLANG "|Android|DEBUG_BUILD==false", CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
		AddTaggedArg(linkerFlags, EXE_CLANG "|Android", CLANG_NO_UNDEFINED);
		AddTaggedArg(linkerFlags, EXE_CLANG "|Android", CLANG_FATAL_WARNINGS);
		AddTaggedArg(linkerFlags, EXE_CLANG "|Android", CLANG_NO_UNDEFINED_VERSION);
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_MAX_PAGE_SIZE, "16384");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_BUILD_ID, "sha1");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "m");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "dl");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "android");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "log");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "atomic");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "EGL");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "GLESv3");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "pthread"); //TODO: Do we need this on Android? What is it called if so?
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "fontconfig"); //TODO: Do we need this on Android? What is it called if so?
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|Android|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d"); //TODO: We probably need a separate folder or lib name for a Box2D that was compiled for Android!
		// TODO: -Wl,--dependency-file=CMakeFiles\pig-core.dir\link.d
	}
	
	// +==============================+
	// |       clang_WasmFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Wasm", CLANG_TARGET_ARCHITECTURE, "wasm32");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Wasm", CLANG_M_FLAG, "bulk-memory");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Wasm", CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Wasm|DEBUG_BUILD==true",  CLANG_DEBUG_INFO_DEFAULT);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Wasm|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
	}
	
	// +==============================+
	// |        clang_WebFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_INCLUDE_DIR, "[ROOT]/wasm/std/include");
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_ENTRYPOINT);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_ALLOW_UNDEFINED);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_STD_LIBRARIES);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_STD_INCLUDES);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_EXPORT_SYMBOL, "__heap_base");
		
		AddTaggedArgNt(compilerFlags, EXE_EMSCRIPTEN_COMPILER "|Web",  EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddTaggedArgNt(compilerFlags, EXE_EMSCRIPTEN_COMPILER "|Web",  EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
	
	// +==============================+
	// |       clang_OrcaFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Orca", CLANG_NO_ENTRYPOINT);
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Orca", CLANG_EXPORT_DYNAMIC);
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin"), false));
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Orca", CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
		
		AddTaggedArgNt(linkerFlags,    EXE_CLANG "|Orca", CLANG_SYSTEM_LIBRARY, "orca_wasm");
	}
	
	// +====================================+
	// | cl_PlaydateSimulatorCompilerFlags  |
	// +====================================+
	{
		//TODO: Just use cl_CommonFlags?
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_LOGO);
		// AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_WARNING_LEVEL, "3");
		// AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_WARNINGS_AS_ERRORS);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_STD_LIB_DYNAMIC_DBG);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_OPTIMIZATION_LEVEL, "d");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "2");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD", CL_DEBUG_INFO);
		
		//TODO: Just use cl_LangCFlags?
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_LANG_VERSION, "clatest"); //Use latest C language spec features
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
		
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CL_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0)
		{
			AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, playdateSdkDir_C_API);
			AddTaggedArgStr(compilerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CL_INCLUDE_DIR, playdateSdkDir_C_API);
		}
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "TARGET_SIMULATOR=1");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDLL");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_MBCS");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "WIN32");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDOWS");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DEFINE, "_WINDLL=1");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_BUFFER_SECURITY_CHECK);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_DISABLE_MINIMAL_REBUILD);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
	}
	
	// +====================================+
	// | link_PlaydateSimulatorLinkerFlags  |
	// +====================================+
	{
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_NO_LOGO);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_TARGET_ARCHITECTURE, "X64");
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_DATA_EXEC_COMPAT);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_ENABLE_ASLR);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_CONSOLE_APPLICATION);
		AddTaggedArgInt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", LINK_TYPELIB_RESOURCE_ID, 1);
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_ENABLE_INCREMENTAL);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator", LINK_CREATE_ASSEMBLY_MANIFEST);
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgNt(linkerFlags,  EXE_MSVC_LINK "|Playdate|Simulator", LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
		AddTaggedArg(linkerFlags,    EXE_MSVC_LINK "|Playdate|Simulator|DEBUG_BUILD", LINK_DEBUG_INFO);
	}
	
	// +==================================+
	// | link_PlaydateSimulatorLibraries  |
	// +==================================+
	{
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "kernel32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "user32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "gdi32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "winspool.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "shell32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "ole32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "oleaut32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "uuid.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "comdlg32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|Playdate|Simulator", CLI_QUOTED_ARG, "advapi32.lib");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceCommonFlags |
	// +===============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0) { AddTaggedArgStr(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_INCLUDE_DIR, playdateSdkDir_C_API); }
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "TARGET_PLAYDATE=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "TARGET_EXTENSION=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__HEAP_SIZE=8388208");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__STACK_SIZE=61800");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEFINE, "__FPU_USED=1");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_THUMB);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_CPU, "cortex-m7");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_TARGET_FPU, "fpv5-sp-d16");
	}
	
	// +==================================+
	// | gcc_PlaydateDeviceCompilerFlags  |
	// +==================================+
	{
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEBUG_INFO_EX, "3");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEBUG_INFO_EX, "dwarf-2");
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
		AddTaggedArgInt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ALIGN_FUNCS_TO, 16);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_SEP_DATA_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_SEP_FUNC_SECTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_EXCEPTIONS);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_OMIT_FRAME_PNTR);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_GLOBAL_VAR_NO_COMMON);
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
		AddTaggedArg(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ONLY_RELOC_WORD_SIZE);
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_WARNING_LEVEL, "all");
		// AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ENABLE_WARNING, "double-promotion");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "unknown-pragmas");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "comment");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "switch");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "nonnull");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "unused");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "missing-braces");
		AddTaggedArgNt(compilerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_WARNING, "char-subscripts");
	}
	
	// +===============================+
	// | gcc_PlaydateDeviceLinkerFlags |
	// +===============================+
	{
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_NO_STD_STARTUP);
		AddTaggedArgNt(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_ENTRYPOINT_NAME, "eventHandler");
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_RWX_WARNING);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_CREF);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_GC_SECTIONS);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_DISABLE_MISMATCH_WARNING);
		AddTaggedArg(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_EMIT_RELOCATIONS);
		AddTaggedArgStr(linkerFlags, EXE_ARM_GCC "|Playdate|Device", GCC_LINKER_SCRIPT, JoinStrings2(playdateSdkDir, StrLit("/C_API/buildsupport/link_map.ld"), false));
	}
	
	// +==============================+
	// |       pdc_CommonFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, EXE_PDC "|Playdate", PDC_QUIET); //Quiet mode, suppress non-error output
		if (playdateSdkDir.length > 0) { AddTaggedArgStr(compilerFlags, EXE_PDC "|Playdate", PDC_SDK_PATH, playdateSdkDir); }
	}
}

#endif //  _PIG_BUILD_PIG_CORE_FLAGS_H
