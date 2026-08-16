/*
File:   pig_build_shader_scraping.h
Author: Taylor Robbins
Date:   03\27\2026
*/

#ifndef _PIG_BUILD_SHADER_SCRAPING_H
#define _PIG_BUILD_SHADER_SCRAPING_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_file.h"
#include "pig_build_misc.h"
#include "optional/pig_build_not_regex.h"

bool IsShaderHeaderLine_Name(Str line, Str* nameOut)
{
	//Matches something like:
	//	Shader program: 'main2d':
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "Shader program:");
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "\'");
	Str nameStr = line;
	CONSUME_UNTIL_NOT_CHARS(&line, StrLit(IDENTIFIER_CHARS));
	if (line.chars == nameStr.chars) { return false; }
	nameStr.length = (u64)(line.chars - nameStr.chars);
	CONSUME_NT_STR(&line, "\':");
	CONSUME_WHITESPACE(&line);
	if (line.length > 0) { return false; }
	if (nameOut != nullptr) { *nameOut = nameStr; }
	return true;
}

bool IsShaderHeaderLine_Attribute(Str shaderName, Str line, Str* nameOut)
{
	//Matches something like:
	//	#define ATTR_main2d_position (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define ATTR_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str nameStr = line;
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

bool IsShaderHeaderLine_View(Str shaderName, Str line, Str* nameOut)
{
	//Matches something like:
	//	#define IMG_main2d_texture0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define VIEW_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str nameStr = line;
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

bool IsShaderHeaderLine_Sampler(Str shaderName, Str line, Str* nameOut)
{
	//Matches something like:
	//	#define SMP_main2d_sampler0 (0)
	CONSUME_WHITESPACE(&line);
	CONSUME_NT_STR(&line, "#define SMP_");
	CONSUME_STR(&line, shaderName);
	CONSUME_NT_STR(&line, "_");
	Str nameStr = line;
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

bool IsShaderHeaderLine_UniformStruct(Str shaderName, Str line, Str* nameOut)
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
	Str nameStr = line;
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
bool IsShaderHeaderLine_UniformStructEnd(Str shaderName, Str uniformBlockName, Str line)
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
bool IsShaderHeaderLine_UniformMember(Str line, Str* typeOut, Str* nameOut)
{
	//Matches something like:
	//	mat4 world;
	CONSUME_WHITESPACE(&line);
	Str typeStr = line;
	CONSUME_UNTIL_CHARS(&line, StrLit(" \t"));
	if (line.chars == typeStr.chars) { return false; }
	typeStr.length = (u64)(line.chars - typeStr.chars);
	CONSUME_WHITESPACE(&line);
	Str nameStr = line;
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

void ScrapeShaderHeaderFileAndAddExtraInfo(Str headerPath, Str shaderPath)
{
	Str headerFileContents = ReadEntireFile(headerPath);
	
	Str shaderName = Str_Empty_Const;
	StrArray shaderAttributes = EMPTY;
	StrArray shaderViews = EMPTY;
	StrArray shaderSamplers = EMPTY;
	StrArray shaderUniformBlocks = EMPTY;
	StrArray shaderUniforms = EMPTY;
	StrArray shaderUniformsBlockNames = EMPTY;
	
	bool insideUniformBlock = false;
	Str uniformBlockName = Str_Empty_Const;
	Str line = Str_Empty_Const;
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
			Str uniformType = Str_Empty_Const;
			Str uniformName = Str_Empty_Const;
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
			Str name = Str_Empty_Const;
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
	
	Str shaderFullPath = GetFullPath(shaderPath, '/');
	Str escapedFullShaderPath = EscapeString(shaderFullPath);
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
			Str attributeName = shaderAttributes.strings[attributeIndex];
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
			Str viewName = shaderViews.strings[viewIndex];
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
			Str samplerName = shaderSamplers.strings[samplerIndex];
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
			Str uniformName = shaderUniforms.strings[uniformIndex];
			Str uniformBlockName = shaderUniformsBlockNames.strings[uniformIndex];
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
	Str* ignoreList;
	StrArray shaderPaths;
	StrArray headerPaths;
	StrArray sourcePaths;
	StrArray objPaths;
	StrArray oPaths;
};

// +==============================+
// |   FindShaderFilesCallback    |
// +==============================+
// bool FindShaderFilesCallback(Str path, bool isFolder, void* contextPntr)
RECURSIVE_DIR_WALK_CALLBACK_DEF(FindShaderFilesCallback)
{
	FindShadersContext* context = (FindShadersContext*)contextPntr;
	if (isFolder)
	{
		for (u64 iIndex = 0; iIndex < context->ignoreListLength; iIndex++)
		{
			if (StrAnyCaseContains(path, context->ignoreList[iIndex])) { return false; }
		}
	}
	
	if (!isFolder && StrAnyCaseEndsWith(path, StrLit(".glsl")))
	{
		Str shaderName = GetFileNamePart(path, false);
		Str rootPath = StrReplace(path, StrLit(".."), StrLit("[ROOT]"));
		FixPathSlashes(rootPath, '/');
		AddStr(&context->shaderPaths, rootPath);
		AddStr(&context->headerPaths, JoinStrings2(rootPath, StrLit(".h")));
		//TODO: If we are on OSX but building for something else like Android then we shouldn't generate a .m and we should generate a .c. If we are building for both OSX and Android then we need both. This logic is faulty
		#if BUILDING_ON_OSX
		AddStr(&context->sourcePaths, JoinStrings2(rootPath, StrLit(".m")));
		#else
		AddStr(&context->sourcePaths, JoinStrings2(rootPath, StrLit(".c")));
		#endif
		AddStr(&context->objPaths, JoinStrings2(shaderName, StrLit(".obj")));
		AddStr(&context->oPaths, JoinStrings2(shaderName, StrLit(".o")));
	}
	return true;
}

#endif //  _PIG_BUILD_SHADER_SCRAPING_H
