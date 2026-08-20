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
#define T_ANDROID_ARCH_ARM8   "|AndroidArchArm8"
#define T_ANDROID_ARCH_ARM7   "|AndroidArchArm7"
#define T_ANDROID_ARCH_X86_64 "|AndroidArchX86"

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
const char* GetAndroidTargetArchitectureTag(AndroidTargetArchitecture enumValue)
{
	switch (enumValue)
	{
		case AndroidTargetArchitecture_Arm8:   return T_ANDROID_ARCH_ARM8;
		case AndroidTargetArchitecture_Arm7:   return T_ANDROID_ARCH_ARM7;
		case AndroidTargetArchitecture_x86_64: return T_ANDROID_ARCH_X86_64;
		default: return "|Unknown";
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

//This creats a "lib" folder with sub-folders and chdir in/out of them when building for each architecture
void BuildAndroidSharedLibraries(const AndroidBinPaths* androidPaths, CliArgs* compilerArgs, StrArray* tags, Str libFolder, Str soFilename, bool buildFatApk)
{
	CliArgs commonArgs = EMPTY;
	if (compilerArgs != nullptr) { AddArgList(&commonArgs, compilerArgs); }
	AddArg(&commonArgs, CLANG_BUILD_SHARED_LIB);
	AddArgStr(&commonArgs, CLANG_LIB_SO_NAME, soFilename);
	AddArgStr(&commonArgs, CLANG_OUTPUT_FILE, soFilename);
	
	Str libFolderNt = CopyStr(libFolder);
	mkdir(libFolderNt.chars, FOLDER_PERMISSIONS);
	chdir(libFolderNt.chars);
	for (u64 archIndex = 1; archIndex < AndroidTargetArchitecture_Count; archIndex++)
	{
		AndroidTargetArchitecture architecture = (AndroidTargetArchitecture)archIndex;
		if (architecture == AndroidTargetArchitecture_Arm8 || buildFatApk)
		{
			mkdir(GetAndroidTargetArchitectureFolderName(architecture), FOLDER_PERMISSIONS);
			chdir(GetAndroidTargetArchitectureFolderName(architecture));
			PrintLine("Building for Android... %s/%s/%.*s", libFolderNt.chars, GetAndroidTargetArchitectureFolderName(architecture), StrPrint(soFilename));
			Str architectureStr = MakeStrNt(GetAndroidTargetArchitectureTargetStr(architecture));
			
			CliArgs cmd = EMPTY;
			cmd.pathSepChar = '/';
			cmd.rootDirPath = StrLit("../../../..");
			AddArgList(&cmd, &commonArgs);
			AddArgStr(&cmd, CLANG_TARGET_ARCHITECTURE, architectureStr);
			Str sysrootRelativePath = JoinPaths3(StrLit("/sysroot/usr/lib/"), architectureStr, StrLit("/35/"));
			AddArgStr(&cmd, CLANG_LIBRARY_DIR, JoinPaths(androidPaths->ndkToolchainDir, sysrootRelativePath));
			
			StrArray fullTags = EMPTY;
			if (tags != nullptr) { AddStrArray(&fullTags, tags); }
			AddTag(&fullTags, T_CLANG);
			AddTag(&fullTags, T_ANDROID);
			AddTag(&fullTags, GetAndroidTargetArchitectureTag(architecture));
			
			RunCliProgramAndExitOnFailureTags(androidPaths->clang, fullTags, &cmd, FormatStr("Failed to build %.*s for Android (architecture=%s)!", StrPrint(soFilename), GetAndroidTargetArchitectureFolderName(architecture)));
			AssertFileExist(soFilename, true);
			
			chdir("..");
		}
	}
	chdir("..");
}

// An .apk MUST contain a classes.dex, even if it contains no real Java code. For projects that are entirely built with native binaries, we generate a Dummy.java with an empty class and compile it to classes.dex file
// This requires that we call "javac" and "d8" binaries and "androidJar" from androidPaths
void CompileDummyJavaToClassesDex(const AndroidBinPaths* androidPaths, Str dummyJavaFilename, Str classesDexFilename)
{
	if (!DoesFileExist(dummyJavaFilename))
	{
		CreateAndWriteFile(dummyJavaFilename, StrLit("//This file is generated by the build script\n\npublic class Dummy { }\n"), true);
	}
	
	CliArgs javacCmd = EMPTY;
	javacCmd.pathSepChar = '/';
	javacCmd.rootDirPath = StrLit("../..");
	AddArgNt(&javacCmd, "-d \"[VAL]\"", ".");
	AddArgStr(&javacCmd, "-classpath \"[VAL]\"", androidPaths->androidJar);
	AddArgStr(&javacCmd, CLI_QUOTED_ARG, dummyJavaFilename);
	RunCliProgramAndExitOnFailure(androidPaths->javac, &javacCmd, FormatStr("Failed to compile %.*s for Android build!", StrPrint(dummyJavaFilename)));
	Str dummyClassFilename = ChangePathExtension(dummyJavaFilename, StrLit(".class"), true);
	AssertFileExist(dummyClassFilename, true);
	
	CliArgs d8Cmd = EMPTY;
	d8Cmd.pathSepChar = '/';
	d8Cmd.rootDirPath = StrLit("../..");
	AddArgStr(&d8Cmd, "--lib \"[VAL]\"", androidPaths->androidJar);
	AddArgNt(&d8Cmd, "--output \"[VAL]\"", "./");
	AddArgStr(&d8Cmd, CLI_QUOTED_ARG, dummyClassFilename);
	RunCliProgramAndExitOnFailure(androidPaths->d8, &d8Cmd, FormatStr("Failed to convert %.*s to %.*s for Android build!", StrPrint(dummyJavaFilename), StrPrint(classesDexFilename)));
	AssertFileExist(classesDexFilename, true);
}

// Resources (like the app icon) that need to be findable by the system, or need to have resolution or language dependent versions
// are stored in a special folder pattern inside a resources.zip that we pass to `aapt2 link`
// This function takes everything in a target directory and puts it into a properly formatted diff with `aapt2 compile`
void PackageAndroidResourcesZip(const AndroidBinPaths* androidPaths, Str resourcesDir, Str zipFilename)
{
	CliArgs compileResCmd = EMPTY;
	compileResCmd.pathSepChar = '/';
	compileResCmd.rootDirPath = StrLit("../..");
	AddArg(&compileResCmd, "compile");
	AddArgStr(&compileResCmd, "--dir \"[VAL]\"", resourcesDir);
	AddArgStr(&compileResCmd, "-o \"[VAL]\"", zipFilename);
	RunCliProgramAndExitOnFailure(androidPaths->aapt2, &compileResCmd, FormatStr("Failed to package %.*s for Android with aapt2!", StrPrint(zipFilename)));
	AssertFileExist(zipFilename, true);
}

// Put the Manifest.xml + resources.zip + android.jar together into the initial .apk file (we will need to insert the .so files and classes.dex manually after this)
void LinkAndroidApk(const AndroidBinPaths* androidPaths, Str manifestPath, Str resourcesZipPath, Str apkFilename)
{
	CliArgs linkApkCmd = EMPTY;
	linkApkCmd.pathSepChar = '/';
	linkApkCmd.rootDirPath = StrLit("../..");
	AddArg(&linkApkCmd, "link");
	AddArgStr(&linkApkCmd, "-o \"[VAL]\"", apkFilename);
	AddArgStr(&linkApkCmd, "-I \"[VAL]\"", androidPaths->androidJar);
	AddArgNt(&linkApkCmd, "-0 [VAL]", "resources.arsc");
	AddArgStr(&linkApkCmd, "--manifest \"[VAL]\"", manifestPath);
	AddArgStr(&linkApkCmd, CLI_QUOTED_ARG, resourcesZipPath);
	RunCliProgramAndExitOnFailure(androidPaths->aapt2, &linkApkCmd, FormatStr("Failed to link %.*s for Android!", StrPrint(apkFilename)));
	AssertFileExist(apkFilename, true);
}

#endif //  _PIG_BUILD_ANDROID_H
