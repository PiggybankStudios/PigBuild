/*
File:   pig_build_shader_shdc.h
Author: Taylor Robbins
Date:   08\30\2026
Description:
	** This file holds CrossCompileShadersInFolderWithShdc(...) which
	** finds all .glsl files in a folder, runs shdc.exe
	** (shader cross-compiler from Sokol tools) on each shader to produce
	** a C header file that contains the translated shader versions,
	** and then compiles those header files to .obj. These .obj files
	** are added to the linkerArgs under the T_SHADER_OBJS tag so they
	** can be linked with later. The paths for each shader are returned
	** as a Array_ShaderInfo so the build script can loop over them if needed.
*/

#ifndef _PIG_BUILD_SHADER_SHDC_H
#define _PIG_BUILD_SHADER_SHDC_H

#include "pig_build_base.h"
#include "pig_build_file.h"
#include "pig_build_array.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_arg_list.h"
#include "pig_build_shader_scraping.h"

typedef enum ShaderTargetPlatform ShaderTargetPlatform;
enum ShaderTargetPlatform
{
	ShaderTargetPlatform_None = 0x00,
	ShaderTargetPlatform_ThisPlatform  = 0x01,
	ShaderTargetPlatform_LinuxViaWsl   = 0x02,
	ShaderTargetPlatform_Android       = 0x04,
	ShaderTargetPlatform_Web           = 0x08,
	ShaderTargetPlatform_WebEmscripten = 0x10,
	ShaderTargetPlatform_Orca          = 0x20,
};

typedef struct ShaderInfo ShaderInfo;
struct ShaderInfo
{
	Str name;
	Str glslPath;
	Str headerPath;
	Str sourcePath;
	Str objPath;
	Str linuxObjPath;
	Str androidObjPaths[AndroidTargetArchitecture_Count];
	Str webObjPath;
	Str webEmscriptenObjPath;
	Str orcaObjPath;
};
TYPED_ARRAY(Array_ShaderInfo, ShaderInfo, infos);

Array_ShaderInfo CrossCompileShadersInFolderWithShdc(Str pigCoreFolder, Str targetDir, Str generatedCodeDir, u8 targetPlatforms, bool forceRebuild, StrArray* compileTags, const CliArgs* compilerArgs, CliArgs* linkerArgs)
{
	Array_ShaderInfo result = EMPTY;
	bool buildForThisPlatform        = IsFlagSet(targetPlatforms, ShaderTargetPlatform_ThisPlatform);
	bool crossCompileForLinuxWithWsl = (BUILDING_ON_WINDOWS && IsFlagSet(targetPlatforms, ShaderTargetPlatform_LinuxViaWsl));
	bool buildForAndroid             = IsFlagSet(targetPlatforms, ShaderTargetPlatform_Android);
	bool buildForWeb                 = IsFlagSet(targetPlatforms, ShaderTargetPlatform_Web);
	bool buildForWebEmscripten       = IsFlagSet(targetPlatforms, ShaderTargetPlatform_WebEmscripten);
	bool buildForOrca                = IsFlagSet(targetPlatforms, ShaderTargetPlatform_Orca);
	
	Str generatedCodeDirResolved = ResolveRootTo(generatedCodeDir, StrLit(".."));
	MyCreateFolder(generatedCodeDirResolved, true);
	
	FileIter fileIter = StartFileIter(ResolveRootTo(targetDir, StrLit("..")));
	Str iterPath = EMPTY;
	bool iterIsFolder = false;
	while (StepFileIter(&fileIter, &iterPath, &iterIsFolder))
	{
		if (!iterIsFolder && StrAnyCaseEquals(GetFileExtPart(iterPath, false), StrLit(".glsl")))
		{
			ShaderInfo* newShader = AddItemArray_ShaderInfo(&result);
			memset(newShader, 0x00, sizeof(ShaderInfo));
			newShader->glslPath = CopyStr(iterPath);
			FixPathSlashes(newShader->glslPath, '/');
			Str headerName = JoinStrings2(GetFileNamePart(newShader->glslPath, true), StrLit(".h"));
			Str objName = JoinStrings2(GetFileNamePart(newShader->glslPath, false), StrLit(OBJ_EXT));
			newShader->name = GetFileNamePart(newShader->glslPath, false);
			if (StrAnyCaseEndsWith(newShader->name, StrLit("shader"))) { newShader->name.length -= StrLit("shader").length; }
			if (StrExactEndsWith(newShader->name, StrLit("_"))) { newShader->name.length -= StrLit("_").length; }
			newShader->headerPath = JoinPaths(generatedCodeDir, headerName);
			newShader->sourcePath = ChangePathExtension(newShader->headerPath, StrLit(".c"), false);
			if (buildForThisPlatform) { newShader->objPath = JoinPaths(StrLit("[ROOT]/build/"), objName); }
			if (crossCompileForLinuxWithWsl) { newShader->linuxObjPath = JoinPaths(StrLit("[ROOT]/build/linux"), objName); }
			if (buildForAndroid)
			{
				for (u8 archIndex = 1; archIndex < AndroidTargetArchitecture_Count; archIndex++)
				{
					AndroidTargetArchitecture architecture = (AndroidTargetArchitecture)archIndex;
					Str archFolderName = MakeStrNt(GetAndroidTargetArchitectureFolderName(architecture));
					//TODO: Maybe we should have an option to not put android artifacts in android sub-folder. For projects that only build for Mobile this is annoying
					newShader->androidObjPaths[archIndex] = JoinPaths3(StrLit("[ROOT]/build/android/lib/"), archFolderName, objName);
				}
			}
			//TODO: Add support for buildForWeb
			//TODO: Add support for buildForWebEmscripten
			//TODO: Add support for buildForOrca
		}
	}
	
	for (u64 sIndex = 0; sIndex < result.length; sIndex++)
	{
		ShaderInfo* shaderInfo = &result.infos[sIndex];
		// PrintLine("Looking at \"%.*s\" -> \"%.*s\" \"%.*s\"", StrPrint(shaderInfo->glslPath), StrPrint(shaderInfo->headerPath), StrPrint(shaderInfo->sourcePath));
		
		// +==============================+
		// |       Generate .h File       |
		// +==============================+
		if (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->headerPath, StrLit(".."))))
		{
			PrintLine("Cross-Compiling %.*s to %.*s...", StrPrint(shaderInfo->glslPath), StrPrint(shaderInfo->headerPath));
			
			StrArray targetLanguages = EMPTY;
			AddStrLit(&targetLanguages, "glsl430");
			AddStrLit(&targetLanguages, "glsl310es");
			if (BUILDING_ON_WINDOWS) { AddStrLit(&targetLanguages, "hlsl5"); }
			if (BUILDING_ON_OSX) { AddStrLit(&targetLanguages, "metal_macos"); }
			Str targetLanguagesStr = JoinStrArray(&targetLanguages, StrLit(":"), false);
			
			CliArgs cmd = EMPTY;
			AddArgNt(&cmd, SHDC_FORMAT, "sokol_impl");
			AddArgNt(&cmd, SHDC_ERROR_FORMAT, "msvc");
			// AddArg(&cmd, SHDC_REFLECTION);
			AddArgStr(&cmd, SHDC_SHADER_LANGUAGES, targetLanguagesStr);
			AddArgStr(&cmd, SHDC_INPUT, shaderInfo->glslPath);
			AddArgStr(&cmd, SHDC_OUTPUT, shaderInfo->headerPath);
			
			Str shdcExe = JoinPaths(ResolveRootTo(pigCoreFolder, StrLit("..")), StrLit(EXE_SHDC));
			FixPathSlashes(shdcExe, PATH_SEP_CHAR);
			RunCliProgramAndExitOnFailure(shdcExe, &cmd, FormatStr(EXE_SHDC_NAME " failed to generate C header for %.*s with target languages %.*s!", StrPrint(shaderInfo->glslPath), StrPrint(targetLanguagesStr)));
			AssertFileExist(ResolveRootTo(shaderInfo->headerPath, StrLit("..")), true);
			
			ScrapeShaderHeaderFileAndAddExtraInfo(ResolveRootTo(shaderInfo->headerPath, StrLit("..")), ResolveRootTo(shaderInfo->glslPath, StrLit("..")));
		}
		
		// +==============================+
		// |       Generate .c File       |
		// +==============================+
		if (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->sourcePath, StrLit(".."))))
		{
			PrintLine("Creating %.*s...", StrPrint(shaderInfo->sourcePath));
			
			Str headerFileName = GetFileNamePart(shaderInfo->headerPath, true);
			Str sourceFileContents = FormatStr(
				"\n"
				"#include \"shader_include.h\"\n"
				"\n"
				"#include \"%.*s\"\n",
				StrPrint(headerFileName)
			);
			CreateAndWriteFile(ResolveRootTo(shaderInfo->sourcePath, StrLit("..")), sourceFileContents, true);
		}
		
		// +==============================+
		// | Compile .c files to .obj/.o  |
		// +==============================+
		if (buildForThisPlatform && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->objPath, StrLit("..")))))
		{
			PrintLine("Building %.*s for %s...", StrPrint(shaderInfo->glslPath), BUILDING_ON_NAME);
			
			if (BUILDING_ON_WINDOWS)
			{
				CliArgs cmd = EMPTY;
				AddArg(&cmd, CL_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, shaderInfo->sourcePath);
				AddArgStr(&cmd, CL_OBJ_FILE, shaderInfo->objPath);
				AddIncludeDirArgStr(&cmd, GetDirectoryPart(shaderInfo->sourcePath, true));
				if (compilerArgs != nullptr) { AddArgList(&cmd, compilerArgs); }
				
				StrArray tags = EMPTY;
				if (compileTags != nullptr) { AddStrArray(&tags, compileTags); }
				AddTag(&tags, T_MSVC_CL);
				AddTag(&tags, T_WINDOWS);
				AddTag(&tags, T_LANG_C);
				AddTag(&tags, T_OBJECT);
				
				RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &cmd, FormatStr("Failed to build %.*s for Windows!", StrPrint(shaderInfo->sourcePath)));
				AssertFileExist(ResolveRootTo(shaderInfo->objPath, StrLit("..")), true);
			}
			if (BUILDING_ON_LINUX)
			{
				AssertMsg(false, "Unimplemented"); //TODO: Implement me!
			}
			if (BUILDING_ON_OSX)
			{
				CliArgs cmd = EMPTY;
				AddArg(&cmd, CLANG_COMPILE);
				AddArgNt(&cmd, CLANG_LANGUAGE, "objective-c");
				AddArgStr(&cmd, CLI_QUOTED_ARG, shaderInfo->sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, shaderInfo->objPath);
				AddIncludeDirArgStr(&cmd, GetDirectoryPart(shaderInfo->sourcePath, true));
				AddArgNt(&cmd, CLANG_DISABLE_WARNING, "unused-command-line-argument"); //Clang likes to warn about _lib_debug/_lib_release library folder being unused
				if (compilerArgs != nullptr) { AddArgList(&cmd, compilerArgs); }
				
				StrArray tags = EMPTY;
				if (compileTags != nullptr) { AddStrArray(&tags, compileTags); }
				AddTag(&tags, T_CLANG);
				AddTag(&tags, T_OSX);
				AddTag(&tags, T_UNIX);
				AddTag(&tags, T_LANG_OBJECTIVEC);
				AddTag(&tags, T_OBJECT);
				
				RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &cmd, FormatStr("Failed to build %.*s for OSX!", StrPrint(shaderInfo->sourcePath)));
				AssertFileExist(ResolveRootTo(shaderInfo->objPath, StrLit("..")), true);
			}
		}
		if (crossCompileForLinuxWithWsl && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->linuxObjPath, StrLit("..")))))
		{
			PrintLine("Building %.*s for Linux via WSL...", StrPrint(shaderInfo->glslPath));
			AssertMsg(false, "Unimplemented"); //TODO: Implement me!
		}
		if (buildForAndroid && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->androidObjPaths[AndroidTargetArchitecture_Arm8], StrLit("..")))))
		{
			PrintLine("Building %.*s for Android...", StrPrint(shaderInfo->glslPath));
			
			for (u64 archIndex = 1; archIndex < AndroidTargetArchitecture_Count; archIndex++)
			{
				AndroidTargetArchitecture architecture = (AndroidTargetArchitecture)archIndex;
				Str archFolderName = MakeStrNt(GetAndroidTargetArchitectureFolderName(architecture));
				
				Str objPath = shaderInfo->androidObjPaths[archIndex];
				Str objDir = ResolveRootTo(GetDirectoryPart(objPath, false), StrLit(".."));
				Str oldWorkingDir = GetFullPath(StrLit("."), '/');
				MyCreateFolder(objDir, true);
				chdir(objDir.chars);
				
				CliArgs cmd = EMPTY;
				cmd.pathSepChar = '/';
				cmd.rootDirPath = StrLit("../../../..");
				AddArg(&cmd, CLANG_COMPILE);
				AddArgStr(&cmd, CLI_QUOTED_ARG, shaderInfo->sourcePath);
				AddArgStr(&cmd, CLANG_OUTPUT_FILE, objPath);
				AddIncludeDirArgStr(&cmd, GetDirectoryPart(shaderInfo->sourcePath, true));
				AddArgNt(&cmd, CLANG_TARGET_ARCHITECTURE, GetAndroidTargetArchitectureTargetStr(architecture));
				if (compilerArgs != nullptr) { AddArgList(&cmd, compilerArgs); }
				
				StrArray tags = EMPTY;
				if (compileTags != nullptr) { AddStrArray(&tags, compileTags); }
				AddTag(&tags, T_CLANG);
				AddTag(&tags, T_ANDROID);
				AddTag(&tags, T_LANG_C);
				AddTag(&tags, T_OBJECT);
				AddStrNt(&tags, GetAndroidTargetArchitectureTag(architecture));
				
				RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &cmd, FormatStr("Failed to build %.*s for Android (arch=%s)", StrPrint(objPath), GetAndroidTargetArchitectureStr(architecture)));
				AssertFileExist(ResolveRootTo(objPath, StrLit("../../../..")), true);
				
				chdir(oldWorkingDir.chars);
			}
		}
		if (buildForWeb && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->webObjPath, StrLit("..")))))
		{
			PrintLine("Building %.*s for Web...", StrPrint(shaderInfo->glslPath));
			AssertMsg(false, "Unimplemented"); //TODO: Implement me!
		}
		if (buildForWebEmscripten && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->webEmscriptenObjPath, StrLit("..")))))
		{
			PrintLine("Building %.*s for Web (Emscripten)...", StrPrint(shaderInfo->glslPath));
			AssertMsg(false, "Unimplemented"); //TODO: Implement me!
		}
		if (buildForOrca && (forceRebuild || !DoesFileExist(ResolveRootTo(shaderInfo->orcaObjPath, StrLit("..")))))
		{
			PrintLine("Building %.*s for Orca...", StrPrint(shaderInfo->glslPath));
			AssertMsg(false, "Unimplemented"); //TODO: Implement me!
		}
		
		// +======================================+
		// | Add Objects to linkerArgs with Tags  |
		// +======================================+
		if (DoesFileExist(ResolveRootTo(shaderInfo->objPath, StrLit(".."))) && buildForThisPlatform)
		{
			AddTaggedArgStr(linkerArgs, T_SHADER_OBJS, CLI_QUOTED_ARG, shaderInfo->objPath);
		}
		if (DoesFileExist(ResolveRootTo(shaderInfo->linuxObjPath, StrLit(".."))) && crossCompileForLinuxWithWsl)
		{
			AddTaggedArgStr(linkerArgs, T_SHADER_OBJS T_LINUX, CLI_QUOTED_ARG, shaderInfo->linuxObjPath);
		}
		if (buildForAndroid)
		{
			for (u8 archIndex = 1; archIndex < AndroidTargetArchitecture_Count; archIndex++)
			{
				AndroidTargetArchitecture architecture = (AndroidTargetArchitecture)archIndex;
				if (DoesFileExist(ResolveRootTo(shaderInfo->androidObjPaths[archIndex], StrLit(".."))))
				{
					StrArray tags = EMPTY;
					AddTag(&tags, T_SHADER_OBJS);
					AddTag(&tags, T_ANDROID);
					AddTag(&tags, GetAndroidTargetArchitectureTag(architecture));
					AddTaggedArgStr(linkerArgs, JoinStrArray(&tags, StrLit("|"), false).chars, CLI_QUOTED_ARG, shaderInfo->androidObjPaths[archIndex]);
				}
			}
		}
		//TODO: Add support for buildForWeb
		//TODO: Add support for buildForWebEmscripten
		//TODO: Add support for buildForOrca
	}
	
	return result;
}

#endif //  _PIG_BUILD_SHADER_SHDC_H
