/*
File:   imgui_main.cpp
Author: Taylor Robbins
Date:   04\11\2026
Description: 
	** Dear ImGui is a C++ library but our example is in C.
	** In order to interface with it we need to compile it in C++ mode to .o\.obj and then link with it.
	** This file acts as a unity build for cimgui.cpp and imgui.cpp (and helper imgui .cpp files).
	** So we can just compile this one file rather than worrying about cimgui's makefiles.
*/

// #include "ui/ui_imconfig.h"

// #undef Min
// #undef Max

// #if COMPILER_IS_MSVC
// #pragma warning(push)
// #pragma warning(disable:4100) //unreferenced formal parameter
// #pragma warning(disable:5262) //implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
// #endif

// #if defined(__clang__)
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wnontrivial-memcall" //warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiTextEditState'
// #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion" //warning: bitwise operation between different enumeration types ('ImGuiSelectableFlagsPrivate_' and 'ImGuiSelectableFlags_') is deprecated
// #endif

// #define IMGUI_DEFINE_MATH_OPERATORS
// #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui/imgui/imgui_internal.h"
#include "cimgui/cimgui.h"
#include "cimgui/cimgui.cpp"

// #include "cimgui/imgui/imgui_internal.h"
#include "cimgui/imgui/imgui.cpp"
#include "cimgui/imgui/imgui_demo.cpp"
#include "cimgui/imgui/imgui_draw.cpp"
// #include "cimgui/imgui/imgui_tables.cpp"
// #include "cimgui/imgui/imgui_widgets.cpp"

// #if TARGET_IS_WINDOWS
// #pragma warning(pop)
// #endif
// #if defined(__clang__)
// #pragma clang diagnostic pop
// #endif

#include "cimgui/imgui/backends/imgui_impl_glfw.h"
#include "cimgui/imgui/backends/imgui_impl_glfw.cpp"

#if 0
START_EXTERN_C
Arena* imguiArena = nullptr; //Declared in in ui_imgui.h
END_EXTERN_C

int ImFormatString(char* bufferPntr, size_t bufferSize, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	va_end(args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}
int ImFormatStringV(char* bufferPntr, size_t bufferSize, const char* fmt, va_list args)
{
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}
ImFileHandle ImFileOpen(const char* filename, const char* mode)
{
	NotNull(filename);
	NotNull(mode);
	NotNull(imguiArena);
	ImGuiFile* result = AllocType(ImGuiFile, imguiArena);
	NotNull(result);
	ClearPointer(result);
	result->arena = imguiArena;
	Str8 modeStr = MakeStr8Nt(mode);
	result->convertNewLines = !StrExactContains(modeStr, StrLit("b"));
	OsOpenFileMode openMode = OsOpenFileMode_None;
	if (StrExactContains(modeStr, StrLit("a"))) { openMode = OsOpenFileMode_Append; }
	else if (StrExactContains(modeStr, StrLit("w"))) { openMode = OsOpenFileMode_Write; }
	else if (StrExactContains(modeStr, StrLit("r"))) { openMode = OsOpenFileMode_Read; }
	//TODO: Handle "wt" mode
	//TODO: Handle "r+" mode?
	//TODO: Handle "w+" mode?
	bool openResult = OsOpenFile(imguiArena, MakeFilePathNt(filename), openMode, (openMode != OsOpenFileMode_Write), &result->file);
	if (!openResult) { if (CanArenaFree(imguiArena)) { FreeType(ImGuiFile, imguiArena, result); } return nullptr; }
	return result;
}
bool ImFileClose(ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	OsCloseFile(&file->file);
	if (CanArenaFree(file->arena)) { FreeType(ImGuiFile, file->arena, file); }
	return true;
}
u64 ImFileGetSize(ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	return file->file.fileSize;
}
u64 ImFileRead(void* data, u64 size, u64 count, ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	if (size == 0 || count == 0) { return 0; }
	NotNull(data);
	Assert(size * count <= UINTXX_MAX);
	uxx numBytesRead = 0;
	Result readResult = OsReadFromOpenFile(&file->file, (uxx)(size * count), file->convertNewLines, data, &numBytesRead);
	if (readResult != Result_Success && readResult != Result_Partial) { return 0; }
	return numBytesRead;
}
u64 ImFileWrite(const void* data, u64 size, u64 count, ImFileHandle file)
{
	NotNull(file);
	NotNull(file->arena);
	NotNull(file->file.arena);
	bool writeResult = OsWriteToOpenFile(&file->file, MakeSlice((size * count), data), file->convertNewLines);
	return (writeResult ? (size * count) : 0);
}
#endif
