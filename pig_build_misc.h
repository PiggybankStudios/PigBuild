/*
File:   pig_build_misc.h
Author: Taylor Robbins
Date:   06\16\2025
*/

#ifndef _PIG_BUILD_MISC_H
#define _PIG_BUILD_MISC_H

#include "pig_build_base.h"
#include "pig_build_str8.h"

// +--------------------------------------------------------------+
// |                            Types                             |
// +--------------------------------------------------------------+
typedef struct LineParser LineParser;
struct LineParser
{
	u64 byteIndex;
	u64 lineBeginByteIndex;
	u64 lineIndex; //This is not zero based! It's more like a line number you'd see in the gutter of a text editor! It also contains the total number of lines in the input after the iteration has finished
	Str8 inputStr;
	//TODO: Should we add support for Streams again?
};

// +--------------------------------------------------------------+
// |                        Print Helpers                         |
// +--------------------------------------------------------------+
void TwoPassPrint(Str8* resultStr, u64* currentByteIndex, const char* formatString, ...)
{
	u64 printSize = 0;
	va_list args;
	
	va_start(args, formatString);
	int measureResult = vsnprintf(nullptr, 0, formatString, args);
	va_end(args);
	assert(measureResult >= 0);
	
	printSize = (u64)measureResult;
	if (resultStr->chars != nullptr)
	{
		assert(*currentByteIndex <= resultStr->length);
		u64 spaceLeft = resultStr->length - *currentByteIndex;
		assert(printSize <= spaceLeft);
		va_start(args, formatString);
		int printResult = vsnprintf(&resultStr->chars[*currentByteIndex], measureResult+1, formatString, args);
		assert(printResult == measureResult);
		resultStr->chars[*currentByteIndex + printSize] = '\0';
		va_end(args);
	}
	
	*currentByteIndex += printSize;
}

// +--------------------------------------------------------------+
// |                         Line Parser                          |
// +--------------------------------------------------------------+
static inline LineParser NewLineParser(Str8 inputStr)
{
	LineParser result = ZEROED;
	result.byteIndex = 0;
	result.lineIndex = 0;
	result.inputStr = inputStr;
	return result;
}

static inline bool LineParserGetLine(LineParser* parser, Str8* lineOut)
{
	if (parser->byteIndex >= parser->inputStr.length) { return false; }
	parser->lineIndex++;
	parser->lineBeginByteIndex = parser->byteIndex;
	
	u64 endOfLineByteSize = 0;
	u64 startIndex = parser->byteIndex;
	while (parser->byteIndex < parser->inputStr.length)
	{
		char nextChar = parser->inputStr.chars[parser->byteIndex];
		char nextNextChar = parser->inputStr.chars[parser->byteIndex+1];
		//TODO: Should we handle \n\r sequence? Windows is \r\n and I don't know of any space where \n\r is considered a valid single new-line
		if (nextChar != nextNextChar &&
			(nextChar     == '\n' || nextChar     == '\r') &&
			(nextNextChar == '\n' || nextNextChar == '\r'))
		{
			endOfLineByteSize = 2;
			break;
		}
		else if (nextChar == '\n' || nextChar == '\r')
		{
			endOfLineByteSize = 1;
			break;
		}
		else
		{
			parser->byteIndex++;
		}
	}
	
	Str8 line = MakeStr8(parser->byteIndex - startIndex, &parser->inputStr.chars[startIndex]);
	parser->byteIndex += endOfLineByteSize;
	if (lineOut != nullptr) { *lineOut = line; }
	return true;
}

// +--------------------------------------------------------------+
// |                     Extract Define Logic                     |
// +--------------------------------------------------------------+
static bool IsHeaderLineDefine(Str8 targetDefineName, Str8 line, Str8* valueStrOut)
{
	line = TrimWhitespace(line);
	u64 firstWhitespaceIndex = FindNextWhitespace(line, 0);
	if (firstWhitespaceIndex < line.length)
	{
		Str8 firstToken = StrSlice(line, 0, firstWhitespaceIndex);
		if (StrExactEquals(firstToken, StrLit("#define")))
		{
			line = TrimWhitespace(StrSliceFrom(line, firstWhitespaceIndex+1));
			u64 identifierEndIndex = FindNextNonIdentifierChar(line, 0);
			if (identifierEndIndex < line.length)
			{
				Str8 nameStr = StrSlice(line, 0, identifierEndIndex);
				if (StrExactEquals(nameStr, targetDefineName))
				{
					Str8 valueStr = TrimWhitespace(StrSliceFrom(line, identifierEndIndex+1));
					if (valueStrOut != nullptr) { *valueStrOut = valueStr; }
					return true;
				}
			}
		}
	}
	return false;
}

bool TryExtractDefineFrom(Str8 headerFileContents, Str8 defineName, Str8* valueOut)
{
	u64 lineStartIndex = 0;
	for (u64 byteIndex = 0; byteIndex < headerFileContents.length; byteIndex++)
	{
		char character = headerFileContents.chars[byteIndex];
		char nextCharacter = headerFileContents.chars[byteIndex+1]; //requires null-terminator we added above
		if (character == '\n' ||
			(character == '\r' && nextCharacter == '\n') ||
			(character == '\n' && nextCharacter == '\r'))
		{
			bool isTwoCharacterNewLine =
				(character == '\r' && nextCharacter == '\n') ||
				(character == '\n' && nextCharacter == '\r');
			
			Str8 lineStr = MakeStr8(byteIndex - lineStartIndex, &headerFileContents.chars[lineStartIndex]);
			
			Str8 defineValue = ZEROED;
			if (IsHeaderLineDefine(defineName, lineStr, &defineValue))
			{
				if (valueOut != nullptr) { *valueOut = defineValue; }
				return true;
			}
			
			if (isTwoCharacterNewLine) { byteIndex++; }
			lineStartIndex = byteIndex+1;
		}
	}
	return false;
}

#endif //  _PIG_BUILD_MISC_H
