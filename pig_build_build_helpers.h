/*
File:   pig_build_build_helpers.h
Author: Taylor Robbins
Date:   06\19\2025
*/

#ifndef _PIG_BUILD_BUILD_HELPERS_H
#define _PIG_BUILD_BUILD_HELPERS_H

#include "pig_build_base.h"
#include "pig_build_str8.h"
#include "pig_build_file.h"
#include "pig_build_misc.h"
#include "pig_build_str_array.h"
#include "pig_build_cli.h"

static inline Str8 ExtractStrDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ZEROED;
	if (!TryExtractDefineFrom(buildConfigContents, defineName, &defineValueStr))
	{
		PrintLine_E("Couldn't find #define %.*s in build_config.h!", StrPrint(defineName));
		exit(4);
	}
	return defineValueStr;
}
static inline bool ExtractBoolDefine(Str8 buildConfigContents, Str8 defineName)
{
	Str8 defineValueStr = ExtractStrDefine(buildConfigContents, defineName);
	bool result = false;
	if (!TryParseBoolArg(defineValueStr, &result))
	{
		PrintLine_E("#define %.*s has a non-bool value: \"%.*s\"", StrPrint(defineName), StrPrint(defineValueStr));
		exit(4);
	}
	return result;
}

static inline void RunBatchFileAndApplyDumpedEnvironment(Str8 batchFilePath, Str8 environmentFilePath, bool skipRunningIfFileExists)
{
	CliArgList cmd = ZEROED;
	AddArgStr(&cmd, CLI_QUOTED_ARG, environmentFilePath);
	Str8 fixedBatchFilePath = CopyStr8(batchFilePath, false);
	FixPathSlashes(fixedBatchFilePath, PATH_SEP_CHAR);
	
	if (!DoesFileExist(environmentFilePath) || !skipRunningIfFileExists)
	{
		int statusCode = RunCliProgram(fixedBatchFilePath, &cmd); //this batch file runs emsdk_env.bat and then dumps it's environment variables to environment.txt. We can then open and parse that file and change our environment to match what emsdk_env.bat changed
		if (statusCode != 0)
		{
			PrintLine_E("%.*s failed! Status Code: %d", StrPrint(fixedBatchFilePath), statusCode);
			exit(statusCode);
		}
	}
	
	Str8 environmentFileContents = ZEROED;
	if (!TryReadFile(environmentFilePath, &environmentFileContents))
	{
		PrintLine_E("%.*s did not create \"%.*s\"! Or we can't open it for some reason", StrPrint(batchFilePath), StrPrint(environmentFilePath));
		exit(4);
	}
	
	ParseAndApplyEnvironmentVariables(environmentFileContents);
	
	free(fixedBatchFilePath.chars);
	free(environmentFileContents.chars);
}

static inline void InitializeMsvcIf(Str8 pigCoreFolder, bool* isMsvcInitialized)
{
	if (*isMsvcInitialized == false)
	{
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/init_msvc.bat"), false);
		Str8 environmentPath = StrLit_Const("msvc_environment.txt");
		if (DoesFileExist(environmentPath)) { WriteLine("Loading MSVC Environment..."); }
		else { WriteLine("Initializing MSVC Compiler..."); }
		RunBatchFileAndApplyDumpedEnvironment(batchPath, environmentPath, true);
		*isMsvcInitialized = true;
	}
}

static inline void InitializeEmsdkIf(Str8 pigCoreFolder, bool* isEmsdkInitialized)
{
	if (*isEmsdkInitialized == false)
	{
		PrintLine("Initializing Emscripten SDK...");
		Str8 batchPath = JoinStrings2(pigCoreFolder, StrLit("/init_emsdk.bat"), false);
		RunBatchFileAndApplyDumpedEnvironment(batchPath, StrLit("emsdk_environment.txt"), false);
		*isEmsdkInitialized = true;
	}
}

static inline void ConcatAllFilesIntoSingleFile(const StrArray* pathArray, Str8 outputFilePath)
{
	//TODO: We really should handle new-line differences between Windows and Linux/etc. a little smarter here
	//      Just because we are building on Windows doesn't mean all these .js files are using Windows style line-endings
	
	StrArray allFilesContents = ZEROED;
	u64 totalLength = 0;
	for (u64 fIndex = 0; fIndex < pathArray->length; fIndex++)
	{
		Str8 inputPath = pathArray->strings[fIndex];
		Str8 inputFileContents = ZEROED;
		if (!TryReadFile(inputPath, &inputFileContents))
		{
			PrintLine_E("Couldn't find/open \"%.*s\"!", StrPrint(inputPath));
			exit(8);
		}
		AddStr(&allFilesContents, inputFileContents);
		if (totalLength > 0) { totalLength += BUILDING_ON_WINDOWS ? 2 : 1; } //+1-2 for the new-line between each file
		totalLength += inputFileContents.length;
		free(inputFileContents.chars);
	}
	
	Str8 combinedContents = ZEROED;
	combinedContents.length = totalLength;
	combinedContents.pntr = malloc(combinedContents.length + 1);
	
	u64 writeIndex = 0;
	for (u64 fIndex = 0; fIndex < allFilesContents.length; fIndex++)
	{
		Str8 inputFileContents = allFilesContents.strings[fIndex];
		if (writeIndex > 0)
		{
			#if BUILDING_ON_WINDOWS
			combinedContents.chars[writeIndex+0] = '\r';
			combinedContents.chars[writeIndex+1] = '\n';
			writeIndex += 2;
			#else
			combinedContents.chars[writeIndex] = '\n';
			writeIndex += 1;
			#endif
		}
		memcpy(&combinedContents.chars[writeIndex], inputFileContents.chars, inputFileContents.length);
		writeIndex += inputFileContents.length;
	}
	assert(writeIndex == combinedContents.length);
	combinedContents.chars[combinedContents.length] = '\0';
	
	CreateAndWriteFile(outputFilePath, combinedContents, false);
	
	FreeStrArray(&allFilesContents);
}

static inline Str8 GetEmscriptenSdkPath()
{
	const char* sdkEnvVariable = getenv("EMSCRIPTEN_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the EMSCRIPTEN_SDK_PATH environment variable before trying to build for the web with USE_EMSCRIPTEN");
		exit(7);
	}
	Str8 result = MakeStr8Nt(sdkEnvVariable);
	if (IS_SLASH(result.chars[result.length-1])) { result.length--; } //no trailing slash
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

#define FILENAME_ORCA_SDK_PATH  "orca_sdk_path.txt"

static inline Str8 GetOrcaSdkPath()
{
	CliArgList cmd = ZEROED;
	AddArg(&cmd, "sdk-path");
	AddArgNt(&cmd, CLI_PIPE_OUTPUT_TO_FILE, FILENAME_ORCA_SDK_PATH);
	int statusCode = RunCliProgram(StrLit("orca"), &cmd);
	if (statusCode != 0)
	{
		PrintLine_E("Failed to run \"orca sdk-path\"! Status code: %d", statusCode);
		WriteLine_E("Make sure Orca SDK is installed and is added to the PATH!");
		exit(statusCode);
	}
	AssertFileExist(StrLit(FILENAME_ORCA_SDK_PATH), false);
	Str8 result = ZEROED;
	bool readSuccess = TryReadFile(StrLit(FILENAME_ORCA_SDK_PATH), &result);
	assert(readSuccess == true);
	assert(result.length > 0);
	FixPathSlashes(result, PATH_SEP_CHAR);
	if (result.chars[result.length-1] == PATH_SEP_CHAR) { result.length--; } //no trailing slash
	return result;
}

static inline Str8 GetPlaydateSdkPath()
{
	const char* sdkEnvVariable = getenv("PLAYDATE_SDK_PATH");
	if (sdkEnvVariable == nullptr)
	{
		WriteLine_E("Please set the PLAYDATE_SDK_PATH environment variable before trying to build for the Playdate");
		exit(7);
	}
	Str8 result = MakeStr8Nt(sdkEnvVariable);
	if (IS_SLASH(result.chars[result.length-1])) { result.length--; }
	result = CopyStr8(result, true);
	FixPathSlashes(result, PATH_SEP_CHAR);
	return result;
}

// +--------------------------------------------------------------+
// |                  Shader Header File Parsing                  |
// +--------------------------------------------------------------+

//NOTE: The macros and functions below act sort of like regular expressions.
//      For example, CONSUME_WHITESPACE is similar to \s* in RE syntax.

#define CONSUME_WHITESPACE(linePntr) do                                \
{                                                                      \
	while ((linePntr)->length > 0 &&                                   \
		((linePntr)->chars[0] == ' ' || (linePntr)->chars[0] == '\t')) \
	{                                                                  \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_STR(linePntr, expectedStr) do                              \
{                                                                          \
	if (!StrExactStartsWith(*(linePntr), (expectedStr))) { return false; } \
	*(linePntr) = StrSliceFrom(*(linePntr), (expectedStr).length);         \
} while(0)

#define CONSUME_NT_STR(linePntr, expectedStrNt) do  \
{                                                   \
	Str8 expectedStr = StrLit_Const(expectedStrNt); \
	CONSUME_STR((linePntr), expectedStr);           \
} while(0)

#define CONSUME_UNTIL(linePntr, expectedStr) do                        \
{                                                                      \
	while((linePntr)->length > 0)                                      \
	{                                                                  \
		if (StrExactStartsWith(*(linePntr), (expectedStr))) { break; } \
		(linePntr)->chars++;                                           \
		(linePntr)->length--;                                          \
	}                                                                  \
} while(0)

#define CONSUME_UNTIL_CHARS(linePntr, expectedCharsStr) do                                                   \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (u64 cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (isInExpectedStr) { break; }                                                                      \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define CONSUME_UNTIL_NOT_CHARS(linePntr, expectedCharsStr) do                                               \
{                                                                                                            \
	while((linePntr)->length > 0)                                                                            \
	{                                                                                                        \
		bool isInExpectedStr = false;                                                                        \
		for (u64 cIndex = 0; cIndex < (expectedCharsStr).length; cIndex++)                                   \
		{                                                                                                    \
			if ((linePntr)->chars[0] == (expectedCharsStr).chars[cIndex]) { isInExpectedStr = true; break; } \
		}                                                                                                    \
		if (!isInExpectedStr) { break; }                                                                     \
		(linePntr)->chars++;                                                                                 \
		(linePntr)->length--;                                                                                \
	}                                                                                                        \
} while(0)

#define UPPERCASE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWERCASE_CHARS "abcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define IDENTIFIER_CHARS "_" NUMBER_CHARS UPPERCASE_CHARS LOWERCASE_CHARS

static inline bool IsShaderHeaderLine_Name(Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	Shader program: 'main2d':
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "Shader program:");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "\'");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, "\':");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_Attribute(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define ATTR_main2d_position (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define ATTR_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_View(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define IMG_main2d_texture0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define VIEW_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_Sampler(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	#define SMP_main2d_sampler0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define SMP_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

static inline bool IsShaderHeaderLine_UniformStruct(Str8 shaderName, Str8 line, Str8* nameOut)
{
	//Matches something like:
	//	SOKOL_SHDC_ALIGN(16) typedef struct main2d_VertParams_t {
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "SOKOL_SHDC_ALIGN(");
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(NUMBER_CHARS));
	CONSUME_NT_STR(&line, ")");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "typedef");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "struct");
	CONSUME_WHITESPACE(&line);
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str8 nameStr = line;
	CONSUME_UNTIL(&line, StrLit("_t"));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, "_t");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "{");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}
static inline bool IsShaderHeaderLine_UniformStructEnd(Str8 shaderName, Str8 uniformBlockName, Str8 line)
{
	//Matches something like:
	//	} main2d_VertParams_t;
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "}");
	CONSUME_WHITESPACE(&line);
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	CONSUME_STR(&line, uniformBlockName);
	CONSUME_NT_STR(&line, "_t;");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	return true;
}
static inline bool IsShaderHeaderLine_UniformMember(Str8 line, Str8* typeOut, Str8* nameOut)
{
	//Matches something like:
	//	mat4 world;
	CONSUME_WHITESPACE(&line);
	Str8 typeStr = line;
	CONSUME_UNTIL_CHARS(&line, StrLit(" \t"));
	if (line.chars == typeStr.chars) { return false; }
	typeStr.length = (u64)(line.chars - typeStr.chars);
	CONSUME_WHITESPACE(&line);
	Str8 nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, ";");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (typeOut != nullptr) { *typeOut = typeStr; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

void ScrapeShaderHeaderFileAndAddExtraInfo(Str8 headerPath, Str8 shaderPath)
{
	Str8 headerFileContents = ReadEntireFile(headerPath);
	
	Str8 shaderName = ZEROED;
	StrArray shaderAttributes = ZEROED;
	StrArray shaderViews = ZEROED;
	StrArray shaderSamplers = ZEROED;
	StrArray shaderUniformBlocks = ZEROED;
	StrArray shaderUniforms = ZEROED;
	StrArray shaderUniformsBlockNames = ZEROED;
	
	bool insideUniformBlock = false;
	Str8 uniformBlockName = ZEROED;
	Str8 line = ZEROED;
	LineParser lineParser = NewLineParser(headerFileContents);
	while (LineParserGetLine(&lineParser, &line))
	{
		if (shaderName.length == 0)
		{
			if (IsShaderHeaderLine_Name(line, &shaderName))
			{
				assert(shaderName.length > 0);
				// PrintLine("Shader name: \"%.*s\"", StrPrint(shaderName));
			}
		}
		else if (insideUniformBlock)
		{
			Str8 uniformType = ZEROED;
			Str8 uniformName = ZEROED;
			if (IsShaderHeaderLine_UniformStructEnd(shaderName, uniformBlockName, line))
			{
				insideUniformBlock = false;
			}
			else if (IsShaderHeaderLine_UniformMember(line, &uniformType, &uniformName))
			{
				// PrintLine("Found uniform \"%.*s\" \"%.*s\"", StrPrint(uniformType), StrPrint(uniformName));
				AddStr(&shaderUniforms, uniformName);
				AddStr(&shaderUniformsBlockNames, uniformBlockName);
			}
		}
		else
		{
			Str8 name = ZEROED;
			if (IsShaderHeaderLine_Attribute(shaderName, line, &name))
			{
				// PrintLine("Found attribute \"%.*s\"", StrPrint(name));
				AddStr(&shaderAttributes, name);
			}
			else if (IsShaderHeaderLine_View(shaderName, line, &name))
			{
				// PrintLine("Found view \"%.*s\"", StrPrint(name));
				AddStr(&shaderViews, name);
			}
			else if (IsShaderHeaderLine_Sampler(shaderName, line, &name))
			{
				// PrintLine("Found sampler \"%.*s\"", StrPrint(name));
				AddStr(&shaderSamplers, name);
			}
			else if (IsShaderHeaderLine_UniformStruct(shaderName, line, &name))
			{
				// PrintLine("Found uniform block \"%.*s\"", StrPrint(name));
				uniformBlockName = name;
				insideUniformBlock = true;
			}
		}
	}
	
	assert(shaderName.length > 0);
	
	Str8 shaderFullPath = GetFullPath(shaderPath, '/');
	Str8 escapedFullShaderPath = EscapeString(shaderFullPath, false);
	AppendToFile(headerPath, StrLit(
		"\n\n//NOTE: These lines were added by pig_build.exe\n"
		"//NOTE: Because an empty array is invalid in C, we always add at least one dummy entry to these definition #defines while the corresponding COUNT #define will remain 0\n"
		"#ifndef NO_ENTRIES_STR\n"
		"#define NO_ENTRIES_STR \"no_entries\"\n"
		"#endif\n"),
		true
	);
	AppendPrintToFile(headerPath,
		"#define %.*s_SHADER_FILE_PATH \"%.*s\"\n",
		StrPrint(shaderName),
		StrPrint(escapedFullShaderPath)
	);
	
	//Attributes
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_ATTR_COUNT %u\n"
			"#define %.*s_SHADER_ATTR_DEFS { \\\n",
			StrPrint(shaderName),
			shaderAttributes.length,
			StrPrint(shaderName)
		);
		free(shaderFullPath.chars);
		free(escapedFullShaderPath.chars);
		for (u64 attributeIndex = 0; attributeIndex < shaderAttributes.length; attributeIndex++)
		{
			Str8 attributeName = shaderAttributes.strings[attributeIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s\", .index=ATTR_%.*s_%.*s }, \\\n",
				StrPrint(attributeName),
				StrPrint(shaderName),
				StrPrint(attributeName)
			);
		}
		if (shaderAttributes.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderAttributeDef struct found in gfx_shader.h\n"), true);
	}
	
	//Views
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_VIEW_COUNT %u\n"
			"#define %.*s_SHADER_VIEW_DEFS { \\\n",
			StrPrint(shaderName),
			shaderViews.length,
			StrPrint(shaderName)
		);
		for (u64 viewIndex = 0; viewIndex < shaderViews.length; viewIndex++)
		{
			Str8 viewName = shaderViews.strings[viewIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s_%.*s\", .index=VIEW_%.*s_%.*s }, \\\n",
				StrPrint(shaderName),
				StrPrint(viewName),
				StrPrint(shaderName),
				StrPrint(viewName)
			);
		}
		if (shaderViews.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderViewDef struct found in gfx_shader.h\n"), true);
	}
	
	//Samplers
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_SAMPLER_COUNT %u\n"
			"#define %.*s_SHADER_SAMPLER_DEFS { \\\n",
			StrPrint(shaderName),
			shaderSamplers.length,
			StrPrint(shaderName)
		);
		for (u64 samplerIndex = 0; samplerIndex < shaderSamplers.length; samplerIndex++)
		{
			Str8 samplerName = shaderSamplers.strings[samplerIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s_%.*s\", .index=SMP_%.*s_%.*s }, \\\n",
				StrPrint(shaderName),
				StrPrint(samplerName),
				StrPrint(shaderName),
				StrPrint(samplerName)
			);
		}
		if (shaderSamplers.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .index=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderSamplerDef struct found in gfx_shader.h\n"), true);
	}
	
	//Uniforms
	{
		AppendPrintToFile(headerPath,
			"#define %.*s_SHADER_UNIFORM_COUNT %u\n"
			"#define %.*s_SHADER_UNIFORM_DEFS { \\\n",
			StrPrint(shaderName),
			shaderUniforms.length,
			StrPrint(shaderName)
		);
		for (u64 uniformIndex = 0; uniformIndex < shaderUniforms.length; uniformIndex++)
		{
			Str8 uniformName = shaderUniforms.strings[uniformIndex];
			Str8 uniformBlockName = shaderUniformsBlockNames.strings[uniformIndex];
			AppendPrintToFile(headerPath,
				"\t{ .name=\"%.*s\", "
				".blockIndex=UB_%.*s_%.*s, "
				".offset=STRUCT_VAR_OFFSET(%.*s_%.*s_t, %.*s), "
				".size=STRUCT_VAR_SIZE(%.*s_%.*s_t, %.*s) }, \\\n",
				StrPrint(uniformName),
				StrPrint(shaderName), StrPrint(uniformBlockName),
				StrPrint(shaderName), StrPrint(uniformBlockName), StrPrint(uniformName),
				StrPrint(shaderName), StrPrint(uniformBlockName), StrPrint(uniformName)
			);
		}
		if (shaderUniforms.length == 0) { AppendToFile(headerPath, StrLit("\t{ .name=NO_ENTRIES_STR, .blockIndex=0, .offset=0 } \\\n"), true); }
		AppendToFile(headerPath, StrLit("} // These should match ShaderUniformDef struct found in gfx_shader.h\n"), true);
	}
	
	free(headerFileContents.chars);
}

typedef struct FindShadersContext FindShadersContext;
struct FindShadersContext
{
	u64 ignoreListLength;
	Str8* ignoreList;
	StrArray shaderPaths;
	StrArray headerPaths;
	StrArray sourcePaths;
	StrArray objPaths;
	StrArray oPaths;
};

// +==============================+
// |   FindShaderFilesCallback    |
// +==============================+
// bool FindShaderFilesCallback(Str8 path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(FindShaderFilesCallback)
{
	FindShadersContext* context = (FindShadersContext*)contextPntr;
	if (isFolder)
	{
		for (u64 iIndex = 0; iIndex < context->ignoreListLength; iIndex++)
		{
			if (StrExactContains(path, context->ignoreList[iIndex])) { return false; }
		}
	}
	
	if (!isFolder && StrExactEndsWith(path, StrLit(".glsl")))
	{
		Str8 shaderName = GetFileNamePart(path, false);
		Str8 rootPath = StrReplace(path, StrLit(".."), StrLit("[ROOT]"), false);
		FixPathSlashes(rootPath, '/');
		AddStr(&context->shaderPaths, rootPath);
		AddStr(&context->headerPaths, JoinStrings2(rootPath, StrLit(".h"), true));
		#if BUILDING_ON_OSX
		AddStr(&context->sourcePaths, JoinStrings2(rootPath, StrLit(".m"), true));
		#else
		AddStr(&context->sourcePaths, JoinStrings2(rootPath, StrLit(".c"), true));
		#endif
		AddStr(&context->objPaths, JoinStrings2(shaderName, StrLit(".obj"), true));
		AddStr(&context->oPaths, JoinStrings2(shaderName, StrLit(".o"), true));
	}
	return true;
}

#endif //  _PIG_BUILD_BUILD_HELPERS_H
