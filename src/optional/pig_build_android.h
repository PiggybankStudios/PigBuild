/*
File:   pig_build_android.h
Author: Taylor Robbins
Date:   04\05\2026
*/

#ifndef _PIG_BUILD_ANDROID_H
#define _PIG_BUILD_ANDROID_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_cli_flags.h"
#include "pig_build_arg_list.h"

// +--------------------------------------------------------------+
// |                       Android Helpers                        |
// +--------------------------------------------------------------+
typedef enum AndroidTargetArchitechture AndroidTargetArchitechture;
enum AndroidTargetArchitechture
{
	AndroidTargetArchitechture_None = 0,
	AndroidTargetArchitechture_Arm8,
	AndroidTargetArchitechture_Arm7,
	AndroidTargetArchitechture_x86,
	AndroidTargetArchitechture_Count,
};
const char* GetAndroidTargetArchitechtureStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_None:  return "None";
		case AndroidTargetArchitechture_Arm8:  return "Arm8";
		case AndroidTargetArchitechture_Arm7:  return "Arm7";
		case AndroidTargetArchitechture_x86:   return "x86";
		default: return "Unknown";
	}
}
const char* GetAndroidTargetArchitechtureFolderName(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "arm64-v8a";
		case AndroidTargetArchitechture_Arm7:  return "armeabi-v7a";
		case AndroidTargetArchitechture_x86:   return "x86_64";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitechtureTargetStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "aarch64-none-linux-android35";
		case AndroidTargetArchitechture_Arm7:  return "armv7a-none-linux-androideabi35";
		case AndroidTargetArchitechture_x86:   return "x86_64-none-linux-android35";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitechtureToolchainFolderStr(AndroidTargetArchitechture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitechture_Arm8:  return "aarch64-linux-android";
		case AndroidTargetArchitechture_Arm7:  return "arm-linux-androideabi";
		case AndroidTargetArchitechture_x86:   return "x86_64-linux-android";
		default: return "unknown";
	}
}

Str GetAndroidSdkPath()
{
	const char* sdkEnvVariable = getenv("ANDROID_SDK");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the ANDROID_SDK environment variable before trying to build for Android");
		exit(7);
	}
	Str result = CopyStr(WithoutTrailingSlash(MakeStrNt(sdkEnvVariable)), true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

void FillAndroidFlags(CliArgList* compilerFlags, CliArgList* linkerFlags, Str androidNdkDir, Str androidNdkToolchainDir)
{
	// +==============================+
	// |      clang_AndroidFlags      |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android|DEBUG_BUILD==true",  CLANG_OPTIMIZATION_LEVEL, "0");
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android|DEBUG_BUILD==false", CLANG_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Android", CLANG_STDLIB_FOLDER, JoinStrings2(androidNdkToolchainDir, StrLit("/sysroot"), false));
		AddTaggedArgStr(compilerFlags, EXE_CLANG "|Android", CLANG_INCLUDE_DIR, JoinStrings2(androidNdkDir, StrLit("/sources/android/native_app_glue"), false));
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android|DEBUG_BUILD", CLANG_DEBUG_INFO_DEFAULT); //TODO: Should we do dwarf-4 debug info instead?
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_DEFINE, "pig_core_EXPORTS"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_DEFINE, "ANDROID"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_DEFINE, "_FORTIFY_SOURCE=2"); //TODO: Can we remove this?
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_DATA_SECTIONS);
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_FUNCTION_SECTIONS);
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_UNWIND_TABLES);
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_STACK_PROTECTOR_STRONG);
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_NO_CANONICAL_PREFIXES);
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_ENABLE_WARNING, "format");
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_ENABLE_WARNING, "error=format-security");
		AddTaggedArg(compilerFlags,    EXE_CLANG "|Android", CLANG_NO_STDLIB_CPP);
		AddTaggedArgNt(compilerFlags,  EXE_CLANG "|Android", CLANG_Q_FLAG, "unused-arguments");
	}
	
	// +==============================+
	// |    clang_AndroidLinkFlags    |
	// +==============================+
	{
		AddTaggedArg(linkerFlags,    EXE_CLANG "|Android", CLANG_fPIC);
		AddTaggedArgStr(linkerFlags, EXE_CLANG "|Android|DEBUG_BUILD==true",  CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
		AddTaggedArgStr(linkerFlags, EXE_CLANG "|Android|DEBUG_BUILD==false", CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
		AddTaggedArg(linkerFlags,    EXE_CLANG "|Android", CLANG_NO_UNDEFINED);
		AddTaggedArg(linkerFlags,    EXE_CLANG "|Android", CLANG_FATAL_WARNINGS);
		AddTaggedArg(linkerFlags,    EXE_CLANG "|Android", CLANG_NO_UNDEFINED_VERSION);
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_MAX_PAGE_SIZE, "16384");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_BUILD_ID, "sha1");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "m");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "dl");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "android");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "log");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "atomic");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "EGL");
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "GLESv3");
		// AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "pthread"); //TODO: Do we need this on Android? What is it called if so?
		// AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android", CLANG_SYSTEM_LIBRARY, "fontconfig"); //TODO: Do we need this on Android? What is it called if so?
		AddTaggedArgNt(linkerFlags,  EXE_CLANG "|Android|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d"); //TODO: We probably need a separate folder or lib name for a Box2D that was compiled for Android!
		// TODO: -Wl,--dependency-file=CMakeFiles\pig-core.dir\link.d
	}
}

#endif //  _PIG_BUILD_ANDROID_H
