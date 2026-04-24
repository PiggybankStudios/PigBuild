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

#define T_ANDROID "|Android"
#define T_ARM8    "|arm8"
#define T_ARM7    "|arm7"
#define T_X86     "|x86"

// +--------------------------------------------------------------+
// |                       Android Helpers                        |
// +--------------------------------------------------------------+
#if LANGUAGE_IS_C
typedef enum AndroidTargetArchitecture AndroidTargetArchitecture;
#endif
enum AndroidTargetArchitecture
{
	AndroidTargetArchitecture_None = 0,
	AndroidTargetArchitecture_Arm8,
	AndroidTargetArchitecture_Arm7,
	AndroidTargetArchitecture_x86,
	AndroidTargetArchitecture_Count,
};
const char* GetAndroidTargetArchitectureStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_None:  return "None";
		case AndroidTargetArchitecture_Arm8:  return "Arm8";
		case AndroidTargetArchitecture_Arm7:  return "Arm7";
		case AndroidTargetArchitecture_x86:   return "x86";
		default: return "Unknown";
	}
}
const char* GetAndroidTargetArchitectureFolderName(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:  return "arm64-v8a";
		case AndroidTargetArchitecture_Arm7:  return "armeabi-v7a";
		case AndroidTargetArchitecture_x86:   return "x86_64";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitectureTargetStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:  return "aarch64-none-linux-android35";
		case AndroidTargetArchitecture_Arm7:  return "armv7a-none-linux-androideabi35";
		case AndroidTargetArchitecture_x86:   return "x86_64-none-linux-android35";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitectureToolchainFolderStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:  return "aarch64-linux-android";
		case AndroidTargetArchitecture_Arm7:  return "arm-linux-androideabi";
		case AndroidTargetArchitecture_x86:   return "x86_64-linux-android";
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
	Str result = CopyStr(WithoutTrailingSlash(MakeStrNt(sdkEnvVariable)));
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

void FillAndroidFlags(CliArgs* compilerFlags, CliArgs* linkerFlags, Str androidNdkDir, Str androidNdkToolchainDir)
{
	// +==============================+
	// |      clang_AndroidFlags      |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID T_DEBUG_BUILD,  CLANG_OPTIMIZATION_LEVEL, "0");
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID T_RELEASE_BUILD, CLANG_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(compilerFlags, T_CLANG T_ANDROID, CLANG_STDLIB_FOLDER, JoinStrings2(androidNdkToolchainDir, StrLit("/sysroot")));
		AddTaggedArgStr(compilerFlags, T_CLANG T_ANDROID, CLANG_INCLUDE_DIR, JoinStrings2(androidNdkDir, StrLit("/sources/android/native_app_glue")));
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID T_DEBUG_BUILD, CLANG_DEBUG_INFO_DEFAULT); //TODO: Should we do dwarf-4 debug info instead?
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_DEFINE, "pig_core_EXPORTS"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_DEFINE, "ANDROID"); //TODO: Can we remove this?
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_DEFINE, "_FORTIFY_SOURCE=2"); //TODO: Can we remove this?
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_DATA_SECTIONS);
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_FUNCTION_SECTIONS);
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_UNWIND_TABLES);
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_STACK_PROTECTOR_STRONG);
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_NO_CANONICAL_PREFIXES);
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_ENABLE_WARNING, "format");
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_ENABLE_WARNING, "error=format-security");
		AddTaggedArg(compilerFlags,    T_CLANG T_ANDROID, CLANG_NO_STDLIB_CPP);
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_Q_FLAG, "unused-arguments");
	}
	
	// +==============================+
	// |    clang_AndroidLinkFlags    |
	// +==============================+
	{
		AddTaggedArg(linkerFlags,    T_CLANG T_ANDROID, CLANG_fPIC);
		AddTaggedArgStr(linkerFlags, T_CLANG T_ANDROID T_DEBUG_BUILD,  CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_debug"));
		AddTaggedArgStr(linkerFlags, T_CLANG T_ANDROID T_RELEASE_BUILD, CLANG_LIBRARY_DIR, StrLit("[ROOT]/third_party/_lib_release"));
		AddTaggedArg(linkerFlags,    T_CLANG T_ANDROID, CLANG_NO_UNDEFINED);
		AddTaggedArg(linkerFlags,    T_CLANG T_ANDROID, CLANG_FATAL_WARNINGS);
		AddTaggedArg(linkerFlags,    T_CLANG T_ANDROID, CLANG_NO_UNDEFINED_VERSION);
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_MAX_PAGE_SIZE, "16384");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_BUILD_ID, "sha1");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "m");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "dl");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "android");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "log");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "atomic");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "EGL");
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "GLESv3");
		// AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "pthread"); //TODO: Do we need this on Android? What is it called if so?
		// AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID, CLANG_SYSTEM_LIBRARY, "fontconfig"); //TODO: Do we need this on Android? What is it called if so?
		AddTaggedArgNt(linkerFlags,  T_CLANG T_ANDROID "|BUILD_WITH_BOX2D", CLANG_SYSTEM_LIBRARY, "box2d"); //TODO: We probably need a separate folder or lib name for a Box2D that was compiled for Android!
		// TODO: -Wl,--dependency-file=CMakeFiles\pig-core.dir\link.d
	}
}

#endif //  _PIG_BUILD_ANDROID_H
