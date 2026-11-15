/*
File:   pig_build_init_msvc.h
Author: Taylor Robbins
Date:   10\31\2026
Description:
	** On Windows the primary compiler is usually installed with Visual Studio
	** but it is not added to the PATH by default. In order to call the MSVC compiler
	** we need to find it's vcvarsall.bat or VsDevCmd.bat in the appropriate VS
	** installation directory and run it in order to configure the environment
	** for building to a particular architecture. This file holds similar logic to
	** shell/init_msvc_environment_for_builder.bat but is meant to be used by
	** the builder to configure the MSVC compiler properly for building the main program,
	** whereas that script is meant to only be used for building the build script.
	** Also this version of the logic can be more complex and handle more scenarios
	** without suffering from language limitations like the batch script.
*/

#ifndef _PIG_BUILD_INIT_MSVC_H
#define _PIG_BUILD_INIT_MSVC_H

#include "pig_build_base.h"
#if BUILDING_ON_WINDOWS

typedef enum MsvcCompilerArchitecture MsvcCompilerArchitecture;
enum MsvcCompilerArchitecture
{
	MsvcCompilerArchitecture_None = 0,
	MsvcCompilerArchitecture_AMD64,
	MsvcCompilerArchitecture_ARM64,
	MsvcCompilerArchitecture_Count,
};
const char* GetMsvcCompilerArchitectureStr(MsvcCompilerArchitecture enumValue)
{
	switch (enumValue)
	{
		case MsvcCompilerArchitecture_None:  return "None";
		case MsvcCompilerArchitecture_AMD64: return "AMD64";
		case MsvcCompilerArchitecture_ARM64: return "ARM64";
		default: return "Unknown";
	}
}
const char* GetMsvcCompilerArchitectureEnvVarValue(MsvcCompilerArchitecture enumValue)
{
	switch (enumValue)
	{
		case MsvcCompilerArchitecture_AMD64: return "x64";
		case MsvcCompilerArchitecture_ARM64: return "arm64";
		default: return nullptr;
	}
}

// In order to save time, you can optionally pass a .txt file path as savedEnvironmentFilePath and we will
//   dump the environment variables from a previous successful configuration to that file and re-use them
//   on subsequent compiles. The vcvarsall.bat/VsDevCmd.bat often take around 1-3 seconds to run which can
//   be a significant time waste if it's happening for every build you do.
void InitializeMsvcCompiler(MsvcCompilerArchitecture targetArchitecture, Str savedEnvironmentFilePath)
{
	//Environment Variables affected by VsDevCmd.bat:
	// CommandPromptType, DevEnvDir, ExtensionSdkDir, EXTERNAL_INCLUDE, Framework40Version,
	// FrameworkDir, FrameworkDir32, FrameworkDir64, FrameworkVersion, FrameworkVersion32, FrameworkVersion64,
	// FSHARPINSTALLDIR, INCLUDE, LIB, LIBPATH, NETFXSDKDir, Path, UCRTVersion, UniversalCRTSdkDir,
	// VCIDEInstallDir, VCINSTALLDIR, VCToolsInstallDir, VCToolsRedistDir, VCToolsVersion, VisualStudioVersion,
	// VS170COMNTOOLS, VSCMD_ARG_app_plat, VSCMD_ARG_HOST_ARCH, VSCMD_ARG_TGT_ARCH, VSCMD_DEBUG, VSCMD_VER,
	// VSINSTALLDIR, VSSDK150INSTALL, VSSDKINSTALL, WindowsLibPath, WindowsSdkBinPath, WindowsSdkDir,
	// WindowsSDKLibVersion, WindowsSdkVerBinPath, WindowsSDKVersion, WindowsSDK_ExecutablePath_x64, WindowsSDK_ExecutablePath_x86,
	// __DOTNET_ADD_32BIT, __DOTNET_ADD_64BIT, __DOTNET_PREFERRED_BITNESS, __VSCMD_PREINIT_PATH, __VSCMD_PREINIT_VCToolsVersion,
	// __VSCMD_PREINIT_VS170COMNTOOLS, __VSCMD_script_err_count, __vscmd_vsdevcmd_errcount, __VSCMD_WINSDK_INCLUDE
	
	const char* vsDevCmdTargetArchEnvVarStrNt = getenv("VSCMD_ARG_TGT_ARCH");
	if (vsDevCmdTargetArchEnvVarStrNt != nullptr && vsDevCmdTargetArchEnvVarStrNt[0] != '\0' &&
		StrAnyCaseEquals(MakeStrNt(vsDevCmdTargetArchEnvVarStrNt), MakeStrNt(GetMsvcCompilerArchitectureEnvVarValue(targetArchitecture))))
	{
		//TODO: Should we confirm of the compiler is visible from the PATH and seems to be configured correctly?
		PrintLine("Environment is already configured for MSVC -arch=%s", GetMsvcCompilerArchitectureEnvVarValue(targetArchitecture));
		return;
	}
	
	//TODO: Check to see if MSVC is already initialized for the target architecture
	//TODO: Check if a savedEnvironmentFilePath exists and has environment vars for the target architecture
	//TODO: Find the Visual Studio installation and run it's vcvarsall.bat or VsDevCmd.bat
	//TODO: Confirm that the compiler and linker are available
	//TODO: Save the environment variables to savedEnvironmentFilePath
}

#define InitMsvcFor(arch) InitializeMsvcCompiler((arch), StrLit("msvc_environment.txt"))
#define InitMsvcForARM()  InitMsvcFor(MsvcCompilerArchitecture_ARM64)
#define InitMsvcForAMD()  InitMsvcFor(MsvcCompilerArchitecture_AMD64)
#define InitMsvcForHost() InitMsvcFor(BUILDING_ON_ARM ? MsvcCompilerArchitecture_ARM64 : MsvcCompilerArchitecture_AMD64)

#endif //BUILDING_ON_WINDOWS

#endif //  _PIG_BUILD_INIT_MSVC_H
