/*
File:   pig_build_android.h
Author: Taylor Robbins
Date:   04\05\2026
*/

//TODO: Should we set -DANDROID_ABI=[architecture] ?

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
// When compiling a native binary to put into the .apk, we need to potentially include versions compiled to other architectures to match the ABI
// These three are what we support compiling for. When BUILD_FAT_APK=0 we only compile for Arm8
// Each compiled .so goes into a sub-folder in the `lib` folder named: `arm64-v8a`, `armeabi-v7a`, and `x86_64` respectively
// See: https://developer.android.com/ndk/guides/abis
#if LANGUAGE_IS_C
typedef enum AndroidTargetArchitecture AndroidTargetArchitecture;
#endif
enum AndroidTargetArchitecture
{
	AndroidTargetArchitecture_None = 0,
	AndroidTargetArchitecture_Arm8, //64-bit ARM, most phones
	AndroidTargetArchitecture_Arm7, //32-bit ARM, very old phones (could run on 64-bit if entire process is run with 32-bit ABI)
	AndroidTargetArchitecture_x86_64, //64-bit, mostly emulators (and some chromebooks)
	AndroidTargetArchitecture_Count,
};
const char* GetAndroidTargetArchitectureStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_None:   return "None";
		case AndroidTargetArchitecture_Arm8:   return "Arm8";
		case AndroidTargetArchitecture_Arm7:   return "Arm7";
		case AndroidTargetArchitecture_x86_64: return "x86_64";
		default: return "Unknown";
	}
}
const char* GetAndroidTargetArchitectureFolderName(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:   return "arm64-v8a";
		case AndroidTargetArchitecture_Arm7:   return "armeabi-v7a";
		case AndroidTargetArchitecture_x86_64: return "x86_64";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitectureTargetStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:   return "aarch64-none-linux-android35";
		case AndroidTargetArchitecture_Arm7:   return "armv7a-none-linux-androideabi35";
		case AndroidTargetArchitecture_x86_64: return "x86_64-none-linux-android35";
		default: return "unknown";
	}
}
const char* GetAndroidTargetArchitectureToolchainFolderStr(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:   return "aarch64-linux-android";
		case AndroidTargetArchitecture_Arm7:   return "arm-linux-androideabi";
		case AndroidTargetArchitecture_x86_64: return "x86_64-linux-android";
		default: return "unknown";
	}
}

// Either use #define if it's defined, or look for an environment variable called ANDROID_SDK being set
Str GetAndroidSdkPath()
{
	#ifdef ANDROID_SDK
	const char* sdkEnvVariable = ANDROID_SDK;
	#else
	const char* sdkEnvVariable = getenv("ANDROID_SDK");
	#endif
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the ANDROID_SDK environment variable before trying to build for Android");
		exit(7);
	}
	Str result = CopyStr(WithoutTrailingSlash(MakeStrNt(sdkEnvVariable)));
	FixPathSlashes(result, PATH_SEP_CHAR);
	//TODO: Confirm this path exists?
	return result;
}

typedef struct AndroidBinPaths AndroidBinPaths;
struct AndroidBinPaths
{
	Str sdkDir;
	Str ndkVersionStr;
	Str platformVersionStr;
	Str buildToolsVersionStr;
	Str hostVersionStr;
	
	Str ndkDir;
	Str ndkToolchainDir;
	Str buildToolsDir;
	Str platformDir;
	
	Str clang;
	Str d8;
	Str aapt2;
	Str apksigner;
	Str zipalign;
	Str javac;
	Str androidJar;
};
void FillAndroidBinPaths(AndroidBinPaths* pathsOut, Str sdkDir, Str ndkVersionStr, Str platformVersionStr, Str buildToolsVersionStr)
{
	NotNull(pathsOut);
	memset(pathsOut, 0x00, sizeof(*pathsOut));
	
	pathsOut->sdkDir = CopyStr(sdkDir);
	pathsOut->ndkVersionStr = CopyStr(ndkVersionStr);
	pathsOut->platformVersionStr = CopyStr(platformVersionStr);
	pathsOut->buildToolsVersionStr = CopyStr(buildToolsVersionStr);
	
	#if BUILDING_ON_WINDOWS
	pathsOut->hostVersionStr = StrLit("windows-x86_64");
	// #elif BUILDING_ON_LINUX
	//TODO: Implement me!
	#elif BUILDING_ON_OSX
	pathsOut->hostVersionStr = StrLit("darwin-x86_64");
	#else
	AssertMsg(false, "FillAndroidBinPaths has not been implemented on the current platform: " BUILDING_ON_NAME);
	#endif
	
	pathsOut->ndkDir           = JoinPaths3(pathsOut->sdkDir, StrLit("/ndk/"),                      ndkVersionStr);
	pathsOut->ndkToolchainDir  = JoinPaths3(pathsOut->ndkDir, StrLit("/toolchains/llvm/prebuilt/"), pathsOut->hostVersionStr);
	pathsOut->buildToolsDir    = JoinPaths3(pathsOut->sdkDir, StrLit("/build-tools/"),              buildToolsVersionStr);
	pathsOut->platformDir      = JoinPaths3(pathsOut->sdkDir, StrLit("/platforms/"),                platformVersionStr);
	
	#if BUILDING_ON_WINDOWS
	#define BAT_ON_WINDOWS ".bat"
	#else
	#define BAT_ON_WINDOWS ""
	#endif
	
	pathsOut->clang      = JoinPathsLit(pathsOut->ndkToolchainDir, "/bin/clang" EXE_EXT);
	pathsOut->d8         = JoinPathsLit(pathsOut->buildToolsDir, "/d8" BAT_ON_WINDOWS);
	pathsOut->aapt2      = JoinPathsLit(pathsOut->buildToolsDir, "/aapt2" EXE_EXT);
	pathsOut->apksigner  = JoinPathsLit(pathsOut->buildToolsDir, "/apksigner" BAT_ON_WINDOWS);
	pathsOut->zipalign   = JoinPathsLit(pathsOut->buildToolsDir, "/zipalign");
	pathsOut->javac      = StrLit("javac" EXE_EXT); //TODO: Should we always assume that java compiler is in the search PATH?
	pathsOut->androidJar = JoinPathsLit(pathsOut->platformDir, "/android.jar");
	
	//TODO: We should check to see if all these folders actually exist and give a nice error to the user when they need to install something or change the build_config.h
}

void FillAndroidFlags(CliArgs* compilerFlags, CliArgs* linkerFlags, const AndroidBinPaths* androidPaths)
{
	// +==============================+
	// |      clang_AndroidFlags      |
	// +==============================+
	{
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID T_DEBUG_BUILD,  CLANG_OPTIMIZATION_LEVEL, "0");
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID T_RELEASE_BUILD, CLANG_OPTIMIZATION_LEVEL, "2");
		AddTaggedArgNt(compilerFlags,  T_CLANG T_ANDROID, CLANG_INCLUDE_DIR, "[ROOT]");
		AddTaggedArgStr(compilerFlags, T_CLANG T_ANDROID, CLANG_STDLIB_FOLDER, JoinPaths(androidPaths->ndkToolchainDir, StrLit("/sysroot")));
		AddTaggedArgStr(compilerFlags, T_CLANG T_ANDROID, CLANG_INCLUDE_DIR, JoinPaths(androidPaths->ndkDir, StrLit("/sources/android/native_app_glue")));
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
