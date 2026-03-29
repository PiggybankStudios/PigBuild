/*
File:   pig_build_pig_core_build_flags.h
Author: Taylor Robbins
Date:   06\23\2025
Description: 
	** Contains all the flags that PigCore uses to compile in all it's configurations.
	** These are pulled into a separate file so that various Pig Core based programs
	** can use these functions as a starting point for their own build scripts.
*/

//test

#ifndef _PIG_BUILD_PIG_CORE_BUILD_FLAGS_H
#define _PIG_BUILD_PIG_CORE_BUILD_FLAGS_H

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
	// +==============================+
	// |        cl_CommonFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, EXE_MSVC_CL, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
		AddTaggedArg(compilerFlags, EXE_MSVC_CL, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
		AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]");
		
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==true", CL_STD_LIB_DYNAMIC_DBG);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_STD_LIB_DYNAMIC);
		
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==true",  CL_OPTIMIZATION_LEVEL, "d");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "y");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "t");
		
		AddTaggedArgNt(compilerFlags,  EXE_MSVC_CL, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_DISABLE_WARNING, CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
		
		Str8 freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
		AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|BUILD_WITH_FREETYPE", CL_INCLUDE_DIR, freetypeDir);
		Str8 plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
		AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|BUILD_WITH_FREETYPE", CL_INCLUDE_DIR, plutosvgDir);
		
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DEBUG_INFO);
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|DEBUG_BUILD", CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
		
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|DUMP_ASSEMBLY", CL_GENERATE_ASSEMB_LISTING, "s"); //Generate assembly listing files with source code included
		
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DUMP_PREPROCESSOR", CL_PRECOMPILE_ONLY);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|DUMP_PREPROCESSOR", CL_PRECOMPILE_PRESERVE_COMMENTS);
	}
	
	// +==============================+
	// |        cl_LangCFlags         |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|LangC", CL_LANG_VERSION, "clatest"); //Use latest C language spec features
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|LangC", CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
		// AddTaggedArg(compilerFlags, EXE_MSVC_CL "|LangC", CL_ENABLE_ADDRESS_SANATIZER);
	}
	
	// +==============================+
	// |       cl_LangCppFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|LangCpp", CL_LANG_VERSION, "c++20");
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|LangCpp", CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
		AddTaggedArgInt(compilerFlags, EXE_MSVC_CL "|LangCpp", CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
	}
	
	// +==============================+
	// |      clang_CommonFlags       |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, EXE_CLANG, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
		AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_CMD_LINE_ARG);
		// AddTaggedArgNt(compilerFlags, EXE_CLANG, CLANG_DEFINE, "_GNU_SOURCE"); //TODO: Maybe we need this for some GNU standard library features?
		
		Str8 freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|BUILD_WITH_FREETYPE", CLANG_INCLUDE_DIR, freetypeDir);
		Str8 plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|BUILD_WITH_FREETYPE", CLANG_INCLUDE_DIR, plutosvgDir);
		
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|DEBUG_BUILD", CLANG_DISABLE_WARNING, "unused-parameter");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|DEBUG_BUILD", CLANG_DISABLE_WARNING, "unused-variable");
		
		AddTaggedArg(compilerFlags, EXE_CLANG "|DUMP_PREPROCESSOR", CLANG_PRECOMPILE_ONLY);
		AddTaggedArg(compilerFlags, EXE_CLANG "|DUMP_PREPROCESSOR", CLANG_INCLUDE_MACROS);
	}
	
	// +==============================+
	// |       clang_LangCFlags       |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LangC", CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
		
		//TODO: Figure out why these are needed when linking with imgui.o with Clang on Linux
		// AddTaggedArg(compilerFlags, EXE_CLANG "|LangC|BUILD_WITH_IMGUI", "-lstdc++"); //TODO: Since this is being added to clang_LangCppFlags flags now (was needed for tracy.so as well as imgui.so) we probably don't need to add it here
		AddTaggedArg(compilerFlags, EXE_CLANG "|LangC|BUILD_WITH_IMGUI", "-fno-threadsafe-statics"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
	}
	
	// +==============================+
	// |      clang_LangCppFlags      |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LangCpp", CLANG_LANG_VERSION, "c++20"); // TODO: What option should we actually choose here?
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LangCpp", CLANG_SYSTEM_LIBRARY, "stdc++"); // Fixes tracy.so link-time errors regarding stuff like `operator delete(void*, unsigned long)`
	}
	
	// +==============================+
	// |  clang_LangObjectiveCFlags   |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LangObjectiveC", CLANG_LANG_VERSION, "gnu2x"); //NOTE: We still ask for gnu23 features in Objective-C mode, the distinguishing factor is that we compile a .m file not a .c file
		AddTaggedArg(compilerFlags, EXE_CLANG "|LangObjectiveC", CLANG_ENABLE_OBJC_ARC);
	}
	
	// +==============================+
	// |    clang_LinuxOrOsxFlags     |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|DEBUG_BUILD==true",  CLANG_OPTIMIZATION_LEVEL, "0");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|LinuxOrOsx|DEBUG_BUILD==true",  CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|LinuxOrOsx|DEBUG_BUILD==false", CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
		#if !BUILDING_ON_OSX
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_M_FLAG, "ssse3"); //For MeowHash to work we need sse3 support
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_M_FLAG, "aes"); //For MeowHash to work we need aes support
		#endif
		//TODO: Really we should do `pkg-config dbus-1 --cflags`
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/include/dbus-1.0");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/dbus-1.0/include"); //This was the path on Lubuntu
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx", CLANG_INCLUDE_DIR, "/usr/lib64/dbus-1.0/include"); //This is the path on Fedora Workstation
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|DEBUG_BUILD", CLANG_DEBUG_INFO, "dwarf-4");
		
		//TODO: Really we should do `pkg-config --cflags gtk4`
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "fpmath=sse");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "sse");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", CLANG_M_FLAG, "sse2");
		AddTaggedArg(compilerFlags, EXE_CLANG "|LinuxOrOsx|BUILD_WITH_GTK", "-pthread");
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
	}
	
	// +==============================+
	// |     cl_CommonLinkerFlags     |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|DEBUG_BUILD==true",  LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_debug");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_LINK "|DEBUG_BUILD==false", LINK_LIBRARY_DIR, "[ROOT]/third_party/_lib_release");
		AddTaggedArg(linkerFlags, EXE_MSVC_LINK, LINK_DISABLE_INCREMENTAL);
	}
	
	// +==============================+
	// |    clang_CommonLibraries     |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_CLANG, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
		AddTaggedArgNt(linkerFlags, EXE_CLANG, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
	}
	
	// +==============================+
	// |  clang_LinuxCommonLibraries  |
	// +==============================+
	{
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
	}
	
	// +==============================+
	// |   clang_OsxCommonLibraries   |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "Cocoa");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "QuartzCore");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|OSX|BUILD_WITH_SOKOL_APP", CLANG_FRAMEWORK, "AudioToolbox");
	}
	
	// +==============================+
	// |     cl_PigCoreLibraries      |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "raylib.lib"); //NOTE: raylib.lib MUST be before User32.lib and others
		
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "Kernel32.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_RAYLIB", CLI_QUOTED_ARG, "Winmm.lib");
		
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_BOX2D",  CLI_QUOTED_ARG, "box2d.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_SDL",    CLI_QUOTED_ARG, "SDL2.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_OPENVR", CLI_QUOTED_ARG, "openvr_api.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_PHYSX",  CLI_QUOTED_ARG, "PhysX_static_64.lib");
		AddTaggedArgNt(linkerFlags, EXE_MSVC_CL "|BUILD_WITH_HTTP",   CLI_QUOTED_ARG, "Winhttp.lib");
	}
	
	// +==============================+
	// | clang_PigCoreLinuxLibraries  |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_CLANG, CLANG_SYSTEM_LIBRARY, "pthread");
		AddTaggedArgNt(linkerFlags, EXE_CLANG, CLANG_SYSTEM_LIBRARY, "fontconfig");
		
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_SYSTEM_LIBRARY, "GL");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d");
	}
	
	// +==============================+
	// |  clang_PigCoreOsxLibraries   |
	// +==============================+
	{
		AddTaggedArgNt(linkerFlags, EXE_CLANG, CLANG_SYSTEM_LIBRARY, "pthread");
		
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "Foundation");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "UIKit");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "AudioToolbox");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "AVFoundation");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "Metal");
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "MetalKit");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "OpenGL");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "OpenGLES");
		// AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_SOKOL_GFX", CLANG_FRAMEWORK, "GLKit");
		
		AddTaggedArgNt(linkerFlags, EXE_CLANG "|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d");
	}
	
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
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|WASM", CLANG_TARGET_ARCHITECTURE, "wasm32");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|WASM", CLANG_M_FLAG, "bulk-memory");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|WASM", CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArg(compilerFlags,   EXE_CLANG "|WASM|DEBUG_BUILD==true",  CLANG_DEBUG_INFO_DEFAULT);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|WASM|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
	}
	
	// +==============================+
	// |        clang_WebFlags        |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==true",  EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==true",  EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_INCLUDE_DIR, "[ROOT]/wasm/std/include");
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_ENTRYPOINT);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_ALLOW_UNDEFINED);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_STD_LIBRARIES);
		AddTaggedArg(compilerFlags,   EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_NO_STD_INCLUDES);
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Web|USE_EMSCRIPTEN==false", CLANG_EXPORT_SYMBOL, "__heap_base");
	}
	
	// +==============================+
	// |       clang_OrcaFlags        |
	// +==============================+
	{
		AddTaggedArg(compilerFlags, EXE_CLANG "|Orca", CLANG_NO_ENTRYPOINT);
		AddTaggedArg(compilerFlags, EXE_CLANG "|Orca", CLANG_EXPORT_DYNAMIC);
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Orca", CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin"), false));
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Orca", CLANG_SYSTEM_LIBRARY, "orca_wasm");
		AddTaggedArgNt(compilerFlags, EXE_CLANG "|Orca", CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
	}
	
	// +====================================+
	// | cl_PlaydateSimulatorCompilerFlags  |
	// +====================================+
	{
		//TODO: Just use cl_CommonFlags?
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_LOGO);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_WARNING_LEVEL, "3");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_NO_WARNINGS_AS_ERRORS);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_STD_LIB_DYNAMIC_DBG);
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_STD_LIB_DYNAMIC);
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==true",  CL_OPTIMIZATION_LEVEL, "d");
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD==false", CL_OPTIMIZATION_LEVEL, "2");
		AddTaggedArg(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator|DEBUG_BUILD", CL_DEBUG_INFO);
		
		//TODO: Just use cl_LangCFlags?
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_LANG_VERSION, "clatest"); //Use latest C language spec features
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
		
		AddTaggedArgNt(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, "[ROOT]");
		if (playdateSdkDir_C_API.length > 0) { AddTaggedArgStr(compilerFlags, EXE_MSVC_CL "|Playdate|Simulator", CL_INCLUDE_DIR, playdateSdkDir_C_API); }
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

void Fill_cl_CommonFlags(CliArgList* cl_CommonFlags, Str8 pigCoreThirdPartyPath, bool DEBUG_BUILD, bool DUMP_PREPROCESSOR, bool DUMP_ASSEMBLY, bool BUILD_WITH_FREETYPE)
{
	AddArg(cl_CommonFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArg(cl_CommonFlags, CL_FULL_FILE_PATHS); //we need full file paths in errors for Sublime Text to be able to parse the errors and display them in the editor
	AddArg(cl_CommonFlags, CL_NO_LOGO); //Suppress the annoying Microsoft logo and copyright info that the compiler prints out
	AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (!DEBUG_BUILD) { AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "y"); }
	if (!DEBUG_BUILD) { AddArgNt(cl_CommonFlags, CL_OPTIMIZATION_LEVEL, "t"); }
	AddArgNt(cl_CommonFlags, CL_WARNING_LEVEL, "X"); //Treat all warnings as errors
	if (DUMP_ASSEMBLY) { AddArgNt(cl_CommonFlags, CL_GENERATE_ASSEMB_LISTING, "s"); } //Generate assembly listing files with source code included
	AddArgInt(cl_CommonFlags, CL_WARNING_LEVEL, 4); //Use warning level 4, then disable various warnings we don't care about
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOGICAL_OP_ON_ADDRESS_OF_STR_CONST);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMELESS_STRUCT_OR_UNION);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_STRUCT_WAS_PADDED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_DECLARATION_HIDES_CLASS_MEMBER);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_REMOVED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_USAGE_OF_DEPRECATED);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_ASSIGNMENT_WITHIN_CONDITIONAL_EXPR);
	AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_NAMED_TYPEDEF_IN_PARENTHESES);
	AddArgInt(cl_CommonFlags, CL_ENABLE_WARNING, CL_WARNING_SWITCH_FALLTHROUGH);
	AddArgNt(cl_CommonFlags, CL_INCLUDE_DIR, "[ROOT]");
	if (BUILD_WITH_FREETYPE)
	{
		Str8 freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
		AddArgStr(cl_CommonFlags, CL_INCLUDE_DIR, freetypeDir);
		Str8 plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
		AddArgStr(cl_CommonFlags, CL_INCLUDE_DIR, plutosvgDir);
	}
	if (DEBUG_BUILD)
	{
		AddArg(cl_CommonFlags, CL_DEBUG_INFO);
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_SWITCH_ONLY_DEFAULT);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_FUNC_PARAMETER);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREFERENCED_LCOAL_VARIABLE);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_CONDITIONAL_EXPR_IS_CONSTANT);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_LOCAL_VAR_INIT_BUT_NOT_REFERENCED);
		AddArgInt(cl_CommonFlags, CL_DISABLE_WARNING, CL_WARNING_UNREACHABLE_CODE_DETECTED);
	}
	if (DUMP_PREPROCESSOR)
	{
		AddArg(cl_CommonFlags, CL_PRECOMPILE_ONLY);
		AddArg(cl_CommonFlags, CL_PRECOMPILE_PRESERVE_COMMENTS);
	}
}

// Flags that we use when compiling any C (not C++) program using MSVC compiler
void Fill_cl_LangCFlags(CliArgList* cl_LangCFlags)
{
	AddArgNt(cl_LangCFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(cl_LangCFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	// AddArg(cl_LangCFlags, CL_ENABLE_ADDRESS_SANATIZER);
}

// Flags that we use when compiling any C++ program using MSVC compiler
void Fill_cl_LangCppFlags(CliArgList* cl_LangCppFlags)
{
	AddArgNt(cl_LangCppFlags, CL_LANG_VERSION, "c++20");
	AddArgInt(cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_ENUMERATION_MUST_HAVE_UNDERLYING_TYPE);
	AddArgInt(cl_LangCppFlags, CL_DISABLE_WARNING, CL_WARNING_BITWISE_OP_BETWEEN_ENUMS);
}

// Flags that we use when compiling any C program using Clang
void Fill_clang_CommonFlags(CliArgList* clang_CommonFlags, Str8 pigCoreThirdPartyPath, bool DEBUG_BUILD, bool DUMP_PREPROCESSOR, bool BUILD_WITH_FREETYPE)
{
	AddArg(clang_CommonFlags, CLANG_FULL_FILE_PATHS); //Print absolute paths in diagnostics TODO: Figure out how to resolve these back to windows paths for Sublime error linking?
	AddArgNt(clang_CommonFlags, CLANG_WARNING_LEVEL, "all"); //This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros
	AddArgNt(clang_CommonFlags, CLANG_WARNING_LEVEL, "extra"); //This enables some extra warning flags that are not enabled by -Wall
	AddArgNt(clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_SHADOWING);
	AddArgNt(clang_CommonFlags, CLANG_ENABLE_WARNING, CLANG_WARNING_MISSING_FALLTHROUGH_IN_SWITCH);
	AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_SWITCH_MISSING_CASES);
	AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_FUNCTION);
	AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, CLANG_WARNING_UNUSED_CMD_LINE_ARG);
	// AddArgNt(clang_CommonFlags, CLANG_DEFINE, "_GNU_SOURCE"); //TODO: Maybe we need this for some GNU standard library features?
	if (BUILD_WITH_FREETYPE)
	{
		Str8 freetypeDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/freetype/include"), false);
		AddArgStr(clang_CommonFlags, CLANG_INCLUDE_DIR, freetypeDir);
		Str8 plutosvgDir = JoinStrings2(pigCoreThirdPartyPath, StrLit("/plutosvg"), false);
		AddArgStr(clang_CommonFlags, CLANG_INCLUDE_DIR, plutosvgDir);
	}
	if (DEBUG_BUILD)
	{
		//We don't care about these warnings in debug builds, but we will solve them when we go to build in release mode because they probably indicate mistakes at that point
		AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-parameter");
		AddArgNt(clang_CommonFlags, CLANG_DISABLE_WARNING, "unused-variable");
	}
	if (DUMP_PREPROCESSOR)
	{
		AddArg(clang_CommonFlags, CLANG_PRECOMPILE_ONLY);
		AddArg(clang_CommonFlags, CLANG_INCLUDE_MACROS);
	}
}

void Fill_clang_LangCFlags(CliArgList* clang_LangCFlags, bool BUILD_WITH_IMGUI)
{
	AddArgNt(clang_LangCFlags, CLANG_LANG_VERSION, "gnu2x"); //Use C20+ language spec (NOTE: We originally had -std=c2x but that didn't define MAP_ANONYMOUS and mmap was failing)
	if (BUILD_WITH_IMGUI)
	{
		//TODO: Figure out why these are needed when linking with imgui.o with Clang on Linux
		// AddArg(clang_LangCFlags, "-lstdc++"); //TODO: Since this is being added to clang_LangCppFlags flags now (was needed for tracy.so as well as imgui.so) we probably don't need to add it here
		AddArg(clang_LangCFlags, "-fno-threadsafe-statics"); //Eliminates undefined references to stuff like "__cxa_guard_acquire"
	}
}
void Fill_clang_LangCppFlags(CliArgList* clang_LangCppFlags)
{
	AddArgNt(clang_LangCppFlags, CLANG_LANG_VERSION, "c++20"); // TODO: What option should we actually choose here?
	AddArgNt(clang_LangCppFlags, CLANG_SYSTEM_LIBRARY, "stdc++"); // Fixes tracy.so link-time errors regarding stuff like `operator delete(void*, unsigned long)`
}
void Fill_clang_LangObjectiveCFlags(CliArgList* clang_LangObjectiveCFlags)
{
	AddArgNt(clang_LangObjectiveCFlags, CLANG_LANG_VERSION, "gnu2x"); //NOTE: We still ask for gnu23 features in Objective-C mode, the distinguishing factor is that we compile a .m file not a .c file
	AddArg(clang_LangObjectiveCFlags, CLANG_ENABLE_OBJC_ARC);
}

// Flags for when we are compiling the linux version of a program using Clang
void Fill_clang_LinuxOrOsxFlags(CliArgList* clang_LinuxOrOsxFlags, bool DEBUG_BUILD, bool BUILD_WITH_GTK)
{
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "[ROOT]");
	AddArgStr(clang_LinuxOrOsxFlags, CLANG_LIBRARY_DIR, DEBUG_BUILD ? StrLit("[ROOT]/third_party/_lib_debug") : StrLit("[ROOT]/third_party/_lib_release"));
	#if !BUILDING_ON_OSX
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_M_FLAG, "ssse3"); //For MeowHash to work we need sse3 support
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_M_FLAG, "aes"); //For MeowHash to work we need aes support
	#endif
	//TODO: Really we should do `pkg-config dbus-1 --cflags`
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/dbus-1.0");
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/dbus-1.0/include"); //This was the path on Lubuntu
	AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/lib64/dbus-1.0/include"); //This is the path on Fedora Workstation
	if (DEBUG_BUILD) { AddArgNt(clang_LinuxOrOsxFlags, CLANG_DEBUG_INFO, "dwarf-4"); }
	if (BUILD_WITH_GTK)
	{
		//TODO: Really we should do `pkg-config --cflags gtk4`
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_M_FLAG, "fpmath=sse");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_M_FLAG, "sse");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_M_FLAG, "sse2");
		AddArg(clang_LinuxOrOsxFlags, "-pthread");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/gtk-4.0");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/glib-2.0");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/glib-2.0/include");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/x86_64-linux-gnu");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/cairo");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/pango-1.0");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/harfbuzz");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/freetype2");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/libpng16");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/libmount");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/blkid");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/fribidi");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/pixman-1");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/gdk-pixbuf-2.0");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/webp");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/include/graphene-1.0");
		AddArgNt(clang_LinuxOrOsxFlags, CLANG_INCLUDE_DIR, "/usr/lib/x86_64-linux-gnu/graphene-1.0/include");
	}
}

void Fill_cl_CommonLinkerFlags(CliArgList* cl_CommonLinkerFlags, bool DEBUG_BUILD)
{
	AddArgNt(cl_CommonLinkerFlags, LINK_LIBRARY_DIR, DEBUG_BUILD ? "[ROOT]/third_party/_lib_debug" : "[ROOT]/third_party/_lib_release");
	AddArg(cl_CommonLinkerFlags, LINK_DISABLE_INCREMENTAL);
}

void Fill_clang_CommonLibraries(CliArgList* clang_CommonLibraries)
{
	AddArgNt(clang_CommonLibraries, CLANG_SYSTEM_LIBRARY, "m"); //Include the math library (required for stuff like sinf, atan, etc.)
	AddArgNt(clang_CommonLibraries, CLANG_SYSTEM_LIBRARY, "dl"); //Needed for dlopen and similar functions
}

void Fill_clang_LinuxCommonLibraries(CliArgList* clang_LinuxCommonLibraries, bool BUILD_WITH_SOKOL_APP, bool BUILD_WITH_GTK)
{
	//TODO: Really we should do `pkg-config dbus-1 --libs`
	AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "dbus-1");
	if (BUILD_WITH_SOKOL_APP)
	{
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "X11");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "Xi");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "Xcursor");
	}
	if (BUILD_WITH_GTK)
	{
		//TODO: Really we should do `pkg-config --libs gtk4`
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "gtk-4");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "pangocairo-1.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "pango-1.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "harfbuzz");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "gdk_pixbuf-2.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "cairo-gobject");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "cairo");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "vulkan");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "graphene-1.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "gio-2.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "gobject-2.0");
		AddArgNt(clang_LinuxCommonLibraries, CLANG_SYSTEM_LIBRARY, "glib-2.0");
	}
}
void Fill_clang_OsxCommonLibraries(CliArgList* clang_OsxCommonLibraries, bool BUILD_WITH_SOKOL_APP)
{
	if (BUILD_WITH_SOKOL_APP)
	{
		AddArgNt(clang_OsxCommonLibraries, CLANG_FRAMEWORK, "Cocoa");
		AddArgNt(clang_OsxCommonLibraries, CLANG_FRAMEWORK, "QuartzCore");
		// AddArgNt(clang_OsxCommonLibraries, CLANG_FRAMEWORK, "AudioToolbox");
	}
}

// These are all the libraries we need when compiling a Windows binary that contains code from PigCore
void Fill_cl_PigCoreLibraries(CliArgList* cl_PigCoreLibraries, bool BUILD_WITH_RAYLIB, bool BUILD_WITH_BOX2D, bool BUILD_WITH_SDL, bool BUILD_WITH_OPENVR, bool BUILD_WITH_IMGUI, bool BUILD_WITH_PHYSX, bool BUILD_WITH_HTTP)
{
	if (BUILD_WITH_RAYLIB) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "raylib.lib"); } //NOTE: raylib.lib MUST be before User32.lib and others
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Gdi32.lib"); //Needed for CreateFontA and other Windows graphics functions
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "User32.lib"); //Needed for GetForegroundWindow, GetDC, etc.
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Ole32.lib"); //Needed for Combaseapi.h, CoInitializeEx, CoCreateInstance, etc.
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shell32.lib"); //Needed for SHGetSpecialFolderPathA
	AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Shlwapi.lib"); //Needed for PathFileExistsA
	if (BUILD_WITH_RAYLIB)
	{
		AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Kernel32.lib");
		AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winmm.lib");
	}
	if (BUILD_WITH_BOX2D) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "box2d.lib"); }
	if (BUILD_WITH_SDL) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "SDL2.lib"); }
	if (BUILD_WITH_OPENVR) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "openvr_api.lib"); }
	if (BUILD_WITH_PHYSX) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "PhysX_static_64.lib"); }
	if (BUILD_WITH_HTTP) { AddArgNt(cl_PigCoreLibraries, CLI_QUOTED_ARG, "Winhttp.lib"); }
}

// These are all the libraries we need when compiling a Linux binary that contains code from PigCore
void Fill_clang_PigCoreLinuxLibraries(CliArgList* clang_PigCoreLinuxLibraries, bool BUILD_WITH_BOX2D, bool BUILD_WITH_SOKOL_GFX)
{
	AddArgNt(clang_PigCoreLinuxLibraries, CLANG_SYSTEM_LIBRARY, "pthread");
	AddArgNt(clang_PigCoreLinuxLibraries, CLANG_SYSTEM_LIBRARY, "fontconfig");
	if (BUILD_WITH_SOKOL_GFX) { AddArgNt(clang_PigCoreLinuxLibraries, CLANG_SYSTEM_LIBRARY, "GL"); }
	if (BUILD_WITH_BOX2D) { AddArgNt(clang_PigCoreLinuxLibraries, CLANG_SYSTEM_LIBRARY, "box2d"); }
}
// These are all the libraries we need when compiling an OSX binary that contains code from PigCore
void Fill_clang_PigCoreOsxLibraries(CliArgList* clang_PigCoreOsxLibraries, bool BUILD_WITH_BOX2D, bool BUILD_WITH_SOKOL_GFX)
{
	AddArgNt(clang_PigCoreOsxLibraries, CLANG_SYSTEM_LIBRARY, "pthread");
	if (BUILD_WITH_SOKOL_GFX)
	{
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "Foundation");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "UIKit");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "AudioToolbox");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "AVFoundation");
		AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "Metal");
		AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "MetalKit");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "OpenGL");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "OpenGLES");
		// AddArgNt(clang_PigCoreOsxLibraries, CLANG_FRAMEWORK, "GLKit");
	}
	if (BUILD_WITH_BOX2D) { AddArgNt(clang_PigCoreOsxLibraries, CLANG_SYSTEM_LIBRARY, "box2d"); }
}

void Fill_clang_AndroidFlags(CliArgList* clang_AndroidFlags, Str8 androidNdkDir, Str8 androidNdkToolchainDir, bool DEBUG_BUILD)
{
	AddArgNt(clang_AndroidFlags, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	AddArgNt(clang_AndroidFlags, CLANG_INCLUDE_DIR, "[ROOT]");
	AddArgStr(clang_AndroidFlags, CLANG_STDLIB_FOLDER, JoinStrings2(androidNdkToolchainDir, StrLit("/sysroot"), false));
	AddArgStr(clang_AndroidFlags, CLANG_INCLUDE_DIR, JoinStrings2(androidNdkDir, StrLit("/sources/android/native_app_glue"), false));
	if (DEBUG_BUILD) { AddArg(clang_AndroidFlags, CLANG_DEBUG_INFO_DEFAULT); } //TODO: Should we do dwarf-4 debug info instead?
	AddArgNt(clang_AndroidFlags, CLANG_DEFINE, "pig_core_EXPORTS"); //TODO: Can we remove this?
	AddArgNt(clang_AndroidFlags, CLANG_DEFINE, "ANDROID"); //TODO: Can we remove this?
	AddArgNt(clang_AndroidFlags, CLANG_DEFINE, "_FORTIFY_SOURCE=2"); //TODO: Can we remove this?
	AddArg(clang_AndroidFlags, CLANG_DATA_SECTIONS);
	AddArg(clang_AndroidFlags, CLANG_FUNCTION_SECTIONS);
	AddArg(clang_AndroidFlags, CLANG_UNWIND_TABLES);
	AddArg(clang_AndroidFlags, CLANG_STACK_PROTECTOR_STRONG);
	AddArg(clang_AndroidFlags, CLANG_NO_CANONICAL_PREFIXES);
	AddArgNt(clang_AndroidFlags, CLANG_ENABLE_WARNING, "format");
	AddArgNt(clang_AndroidFlags, CLANG_ENABLE_WARNING, "error=format-security");
	AddArg(clang_AndroidFlags, CLANG_NO_STDLIB_CPP);
	AddArgNt(clang_AndroidFlags, CLANG_Q_FLAG, "unused-arguments");
}

void Fill_clang_AndroidLinkFlags(CliArgList* clang_AndroidLinkFlags, bool DEBUG_BUILD, bool BUILD_WITH_BOX2D)
{
	AddArg(clang_AndroidLinkFlags, CLANG_fPIC);
	AddArgStr(clang_AndroidLinkFlags, CLANG_LIBRARY_DIR, DEBUG_BUILD ? StrLit("[ROOT]/third_party/_lib_debug") : StrLit("[ROOT]/third_party/_lib_release"));
	AddArg(clang_AndroidLinkFlags, CLANG_NO_UNDEFINED);
	AddArg(clang_AndroidLinkFlags, CLANG_FATAL_WARNINGS);
	AddArg(clang_AndroidLinkFlags, CLANG_NO_UNDEFINED_VERSION);
	AddArgNt(clang_AndroidLinkFlags, CLANG_MAX_PAGE_SIZE, "16384");
	AddArgNt(clang_AndroidLinkFlags, CLANG_BUILD_ID, "sha1");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "m");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "dl");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "android");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "log");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "atomic");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "EGL");
	AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "GLESv3");
	// AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "pthread"); //TODO: Do we need this on Android? What is it called if so?
	// AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "fontconfig"); //TODO: Do we need this on Android? What is it called if so?
	if (BUILD_WITH_BOX2D) { AddArgNt(clang_AndroidLinkFlags, CLANG_SYSTEM_LIBRARY, "box2d"); } //TODO: We probably need a separate folder or lib name for a Box2D that was compiled for Android!
	// TODO: -Wl,--dependency-file=CMakeFiles\pig-core.dir\link.d
}

void Fill_clang_WasmFlags(CliArgList* clang_WasmFlags, bool DEBUG_BUILD)
{
	AddArgNt(clang_WasmFlags, CLANG_TARGET_ARCHITECTURE, "wasm32");
	AddArgNt(clang_WasmFlags, CLANG_M_FLAG, "bulk-memory");
	AddArgNt(clang_WasmFlags, CLANG_INCLUDE_DIR, "[ROOT]");
	if (DEBUG_BUILD) { AddArg(clang_WasmFlags, CLANG_DEBUG_INFO_DEFAULT); }
	else { AddArgNt(clang_WasmFlags, CLANG_OPTIMIZATION_LEVEL, "2"); }
}

void Fill_clang_WebFlags(CliArgList* clang_WebFlags, bool USE_EMSCRIPTEN)
{	
	if (USE_EMSCRIPTEN)
	{
		AddArgNt(clang_WebFlags, EMSCRIPTEN_S_FLAG, "USE_SDL");
		AddArgNt(clang_WebFlags, EMSCRIPTEN_S_FLAG, "ALLOW_MEMORY_GROWTH");
	}
	else
	{
		AddArgNt(clang_WebFlags, CLANG_INCLUDE_DIR, "[ROOT]/wasm/std/include");
		AddArg(clang_WebFlags, CLANG_NO_ENTRYPOINT);
		AddArg(clang_WebFlags, CLANG_ALLOW_UNDEFINED);
		AddArg(clang_WebFlags, CLANG_NO_STD_LIBRARIES);
		AddArg(clang_WebFlags, CLANG_NO_STD_INCLUDES);
		AddArgNt(clang_WebFlags, CLANG_EXPORT_SYMBOL, "__heap_base");
	}
}

void Fill_clang_OrcaFlags(CliArgList* clang_OrcaFlags, Str8 orcaSdkPath)
{
	AddArg(clang_OrcaFlags, CLANG_NO_ENTRYPOINT);
	AddArg(clang_OrcaFlags, CLANG_EXPORT_DYNAMIC);
	AddArgStr(clang_OrcaFlags, CLANG_STDLIB_FOLDER, JoinStrings2(orcaSdkPath, StrLit("/orca-libc"), false));
	AddArgStr(clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src"), false));
	AddArgStr(clang_OrcaFlags, CLANG_INCLUDE_DIR, JoinStrings2(orcaSdkPath, StrLit("/src/ext"), false));
	AddArgStr(clang_OrcaFlags, CLANG_LIBRARY_DIR, JoinStrings2(orcaSdkPath, StrLit("/bin"), false));
	AddArgNt(clang_OrcaFlags, CLANG_SYSTEM_LIBRARY, "orca_wasm");
	AddArgNt(clang_OrcaFlags, CLANG_DEFINE, "__ORCA__"); //#define __ORCA__ so that base_compiler_check.h can set TARGET_IS_ORCA
}

void Fill_cl_PlaydateSimulatorCompilerFlags(CliArgList* cl_PlaydateSimulatorCompilerFlags, bool DEBUG_BUILD, Str8 playdateSdkDir_C_API)
{
	//TODO: Just use cl_CommonFlags?
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_NO_LOGO);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_WARNING_LEVEL, "3");
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_NO_WARNINGS_AS_ERRORS);
	AddArg(cl_PlaydateSimulatorCompilerFlags, DEBUG_BUILD ? CL_STD_LIB_DYNAMIC_DBG : CL_STD_LIB_DYNAMIC);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	if (DEBUG_BUILD) { AddArg(cl_PlaydateSimulatorCompilerFlags, CL_DEBUG_INFO); }
	
	//TODO: Just use cl_LangCFlags?
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, "[ROOT]");
	if (playdateSdkDir_C_API.length > 0) { AddArgStr(cl_PlaydateSimulatorCompilerFlags, CL_INCLUDE_DIR, playdateSdkDir_C_API); }
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_SIMULATOR=1");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_MBCS");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "WIN32");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDOWS");
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_DEFINE, "_WINDLL=1");
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_BUFFER_SECURITY_CHECK);
	AddArg(cl_PlaydateSimulatorCompilerFlags, CL_DISABLE_MINIMAL_REBUILD);
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_RUNTIME_CHECKS, "1"); //Enable fast runtime checks (Equivalent to "su")
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_CALLING_CONVENTION, "d"); //Use __cdecl calling convention
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INLINE_EXPANSION_LEVEL, "0"); //Disable inline expansions
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_INTERNAL_COMPILER_ERROR_BEHAVIOR, "prompt"); //TODO: Do we need this?
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "forScope"); //Enforce Standard C++ for scoping rules (on by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "inline"); //Remove unreferenced functions or data if they're COMDAT or have internal linkage only (off by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_ENABLE_LANG_CONFORMANCE_OPTION, "wchar_t"); //wchar_t is a native type, not a typedef (on by default)
	AddArgNt(cl_PlaydateSimulatorCompilerFlags, CL_FLOATING_POINT_MODEL, "precise"); //"precise" floating-point model; results are predictable
}

void Fill_link_PlaydateSimulatorLinkerFlags(CliArgList* link_PlaydateSimulatorLinkerFlags, bool DEBUG_BUILD)
{
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_NO_LOGO);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_TARGET_ARCHITECTURE, "X64");
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_DATA_EXEC_COMPAT);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_ASLR);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_CONSOLE_APPLICATION);
	AddArgInt(link_PlaydateSimulatorLinkerFlags, LINK_TYPELIB_RESOURCE_ID, 1);
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_ENABLE_INCREMENTAL);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_INCREMENTAL_FILE_NAME, "tests.ilk"); //TODO: This should really move down below inside the tests.exe block
	AddArg(link_PlaydateSimulatorLinkerFlags, LINK_CREATE_ASSEMBLY_MANIFEST);
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_ASSEMBLY_MANIFEST_FILE, "tests.intermediate.manifest"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_LINK_TIME_CODEGEN_FILE, "tests.iobj"); //TODO: This should really move down below inside the tests.exe block
	AddArgNt(link_PlaydateSimulatorLinkerFlags, LINK_EMBED_UAC_INFO_EX, "level='asInvoker' uiAccess='false'");
	if (DEBUG_BUILD) { AddArg(link_PlaydateSimulatorLinkerFlags, LINK_DEBUG_INFO); }
}

void Fill_link_PlaydateSimulatorLibraries(CliArgList* link_PlaydateSimulatorLibraries)
{
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "kernel32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "user32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "gdi32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "winspool.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "shell32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "ole32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "oleaut32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "uuid.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "comdlg32.lib");
	AddArgNt(link_PlaydateSimulatorLibraries, CLI_QUOTED_ARG, "advapi32.lib");
}

void Fill_gcc_PlaydateDeviceCommonFlags(CliArgList* gcc_PlaydateDeviceCommonFlags, Str8 playdateSdkDir_C_API)
{
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, "[ROOT]");
	if (playdateSdkDir_C_API.length > 0) { AddArgStr(gcc_PlaydateDeviceCommonFlags, GCC_INCLUDE_DIR, playdateSdkDir_C_API); }
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_PLAYDATE=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "TARGET_EXTENSION=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__HEAP_SIZE=8388208");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__STACK_SIZE=61800");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_DEFINE, "__FPU_USED=1");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nano.specs"); //Required for things like _read, _write, _exit, etc. to not be pulled in as requirements from standard library
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_USE_SPEC_FILE, "nosys.specs"); //TODO: Is this helping?
	AddArg(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_THUMB);
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_CPU, "cortex-m7");
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_FLOAT_ABI_MODE, "hard"); //Use hardware for floating-point operations
	AddArgNt(gcc_PlaydateDeviceCommonFlags, GCC_TARGET_FPU, "fpv5-sp-d16");
}

void Fill_gcc_PlaydateDeviceCompilerFlags(CliArgList* gcc_PlaydateDeviceCompilerFlags)
{
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "3");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEBUG_INFO_EX, "dwarf-2");
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_STD_LIB_DYNAMIC);
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DEPENDENCY_FILE, "tests.d"); //TODO: This should really move down below inside the tests.exe block
	AddArgInt(gcc_PlaydateDeviceCompilerFlags, GCC_ALIGN_FUNCS_TO, 16);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_SEP_DATA_SECTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_SEP_FUNC_SECTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_EXCEPTIONS);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_OMIT_FRAME_PNTR);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_GLOBAL_VAR_NO_COMMON);
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_VERBOSE_ASSEMBLY); //TODO: Should this only be on when DEBUG_BUILD?
	AddArg(gcc_PlaydateDeviceCompilerFlags, GCC_ONLY_RELOC_WORD_SIZE);
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_WARNING_LEVEL, "all");
	// AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_ENABLE_WARNING, "double-promotion");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unknown-pragmas");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "comment");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "switch");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "nonnull");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "unused");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "missing-braces");
	AddArgNt(gcc_PlaydateDeviceCompilerFlags, GCC_DISABLE_WARNING, "char-subscripts");
}

void Fill_gcc_PlaydateDeviceLinkerFlags(CliArgList* gcc_PlaydateDeviceLinkerFlags, Str8 playdateSdkDir)
{
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_NO_STD_STARTUP);
	AddArgNt(gcc_PlaydateDeviceLinkerFlags, GCC_ENTRYPOINT_NAME, "eventHandler");
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_RWX_WARNING);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_CREF);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_GC_SECTIONS);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_DISABLE_MISMATCH_WARNING);
	AddArg(gcc_PlaydateDeviceLinkerFlags, GCC_EMIT_RELOCATIONS);
	AddArgStr(gcc_PlaydateDeviceLinkerFlags, GCC_LINKER_SCRIPT, JoinStrings2(playdateSdkDir, StrLit("/C_API/buildsupport/link_map.ld"), false));
}

void Fill_pdc_CommonFlags(CliArgList* pdc_CommonFlags, Str8 playdateSdkDir)
{
	AddArg(pdc_CommonFlags, PDC_QUIET); //Quiet mode, suppress non-error output
	if (playdateSdkDir.length > 0) { AddArgStr(pdc_CommonFlags, PDC_SDK_PATH, playdateSdkDir); }
}

#endif //  _PIG_BUILD_PIG_CORE_BUILD_FLAGS_H
