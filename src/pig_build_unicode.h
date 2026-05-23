/*
File:   pig_build_unicode.h
Author: Taylor Robbins
Date:   05\22\2026
*/

#ifndef _PIG_BUILD_UNICODE_H
#define _PIG_BUILD_UNICODE_H

#include "pig_build_base.h"
#include "pig_build_str.h"

#define UTF8_MAX_CODEPOINT     0x10FFFFUL
#define UTF8_MAX_CHAR_SIZE     4 //bytes
#define UTF16_MAX_CHAR_SIZE    2 //words

// +--------------------------------------------------------------+
// |                            UTF-8                             |
// +--------------------------------------------------------------+
u8 CodepointUtf8Size(u32 codepoint)
{
	if (codepoint <= 0x7F) //0xxx xxxx
	{
		return 1;
	}
	else if (codepoint <= 0x7FF) //110x xxxx  10xx xxxx
	{
		return 2;
	}
	else if (codepoint >= 0xD800 && codepoint <= 0xDFFF) //invalid block
	{
		return 0;
	}
	else if (codepoint <= 0xFFFF) //1110 xxxx  10xx xxxx  10xx xxxx
	{
		return 3;
	}
	else if (codepoint <= UTF8_MAX_CODEPOINT) //1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
	{
		return 4;
	}
	else //everything above this point is also invalid
	{
		return 0;
	}
}

u8 Utf8NextCodepointSize(Str str)
{
	if (str.length == 0) { return 0; }
	
	const u8* bytePntr = str.bytes;
	if (bytePntr[0] <= 127) //0xxx xxxx
	{
		return 1;
	}
	else if (bytePntr[0] < 0xC0) //invalid block
	{
		AssertFmt(bytePntr[0] >= 0xC0, "10xx xxxx format for the first byte of a character is invalid UTF-8: 0x%02X", bytePntr[0]);
		return 0;
	}
	else if (bytePntr[0] < 0xE0) //110x xxxx  10xx xxxx
	{
		AssertFmt(str.length >= 2, "Expected at least one more byte in string following: 0x%02X", bytePntr[0]);
		return 2;
	}
	else if (bytePntr[0] < 0xF0) //1110 xxxx  10xx xxxx  10xx xxxx
	{
		AssertFmt(str.length >= 3, "Expected at least two more bytes in string following: 0x%02X", bytePntr[0]);
		return 3;
	}
	else if (bytePntr[0] < 0xF8) //1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
	{
		AssertFmt(str.length >= 4, "Expected at least three more bytes in string following: 0x%02X", bytePntr[0]);
		return 4;
	}
	else
	{
		AssertFmt(bytePntr[0] < 0xF8, "Invalid byte in UTF-8 sequence: 0x%02X", bytePntr[0]);
		return 0;
	}
}

Str CodepointToUtf8InBuffer(u32 codepoint, u8* byteBuffer)
{
	Str result = MakeStr_Const(0, byteBuffer);
	if (codepoint <= 0x7F) //0xxx xxxx
	{
		result.chars[0] = (u8)codepoint;
		result.length = 1;
	}
	else if (codepoint <= 0x7FF) //110x xxxx  10xx xxxx
	{
		result.chars[0] = (0xC0 | ((codepoint >> 6) & 0x1F));
		result.chars[1] = (0x80 | (codepoint & 0x3F));
		result.length = 2;
	}
	else if (codepoint >= 0xD800 && codepoint <= 0xDFFF) //invalid block
	{
		AssertFmt(codepoint < 0xD800 || codepoint > 0xDFFF, "Invalid codepoint for UTF-8: 0x%08X", codepoint);
	}
	else if (codepoint <= 0xFFFF) //1110 xxxx  10xx xxxx  10xx xxxx
	{
		result.chars[0] = (0xE0 | ((codepoint >> 12) & 0x0F));
		result.chars[1] = (0x80 | ((codepoint>>6) & 0x3F));
		result.chars[2] = (0x80 | (codepoint & 0x3F));
		result.length = 3;
	}
	else if (codepoint <= UTF8_MAX_CODEPOINT) //1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
	{
		result.chars[0] = (0xF0 | ((codepoint >> 18) & 0x07));
		result.chars[1] = (0x80 | ((codepoint>>12) & 0x3F));
		result.chars[2] = (0x80 | ((codepoint>>6) & 0x3F));
		result.chars[3] = (0x80 | (codepoint & 0x3F));
		result.length = 4;
	}
	else //everything above this point is also invalid
	{
		AssertFmt(codepoint <= UTF8_MAX_CODEPOINT, "Invalid codepoint for UTF-8: 0x%08X", codepoint);
	}
	return result;
}

Str CodepointToUtf8(u32 codepoint)
{
	Str result = AllocStr(UTF8_MAX_CHAR_SIZE);
	result.length = 0;
	if (codepoint <= 0x7F) //0xxx xxxx
	{
		result.chars[0] = (u8)codepoint;
		result.length = 1;
	}
	else if (codepoint <= 0x7FF) //110x xxxx  10xx xxxx
	{
		result.chars[0] = (0xC0 | ((codepoint >> 6) & 0x1F));
		result.chars[1] = (0x80 | (codepoint & 0x3F));
		result.length = 2;
	}
	else if (codepoint >= 0xD800 && codepoint <= 0xDFFF) //invalid block
	{
		AssertFmt(codepoint < 0xD800 || codepoint > 0xDFFF, "Invalid codepoint for UTF-8: 0x%08X", codepoint);
	}
	else if (codepoint <= 0xFFFF) //1110 xxxx  10xx xxxx  10xx xxxx
	{
		result.chars[0] = (0xE0 | ((codepoint >> 12) & 0x0F));
		result.chars[1] = (0x80 | ((codepoint>>6) & 0x3F));
		result.chars[2] = (0x80 | (codepoint & 0x3F));
		result.length = 3;
	}
	else if (codepoint <= UTF8_MAX_CODEPOINT) //1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
	{
		result.chars[0] = (0xF0 | ((codepoint >> 18) & 0x07));
		result.chars[1] = (0x80 | ((codepoint>>12) & 0x3F));
		result.chars[2] = (0x80 | ((codepoint>>6) & 0x3F));
		result.chars[3] = (0x80 | (codepoint & 0x3F));
		result.length = 4;
	}
	else //everything above this point is also invalid
	{
		AssertFmt(codepoint <= UTF8_MAX_CODEPOINT, "Invalid codepoint for UTF-8: 0x%08X", codepoint);
	}
	return result;
}

u32 Utf8ToCodepoint(Str* strPntr)
{
	NotNull(strPntr);
	u32 result = 0;
	u8 codepointByteLength = 0;
	AssertMsg(strPntr->length > 0, "Empty string passed to Utf8ToCodepoint!");
	
	const u8* bytePntr = strPntr->bytes;
	if (bytePntr[0] <= 127) //0xxx xxxx
	{
		result = (u32)bytePntr[0];
		codepointByteLength = 1;
	}
	else if (bytePntr[0] < 0xC0) //invalid block
	{
		AssertFmt(bytePntr[0] >= 0xC0, "10xx xxxx format for the first byte of a character is invalid UTF-8: 0x%02X", bytePntr[0]);
	}
	else if (bytePntr[0] < 0xE0) //110x xxxx  10xx xxxx
	{
		AssertFmt(strPntr->length >= 2, "Expected at least one more byte in string following: 0x%02X", bytePntr[0]);
		AssertFmt(bytePntr[1] >= 0x80 && bytePntr[1] < 0xC0, "2-byte UTF-8 sequence has invalid second byte: 0x%02X 0x%02X", bytePntr[0], bytePntr[1]);
		result = ((u32)(bytePntr[0] & 0x1F) << 6) | ((u32)(bytePntr[1] & 0x3F) << 0);
		codepointByteLength = 2;
	}
	else if (bytePntr[0] < 0xF0) //1110 xxxx  10xx xxxx  10xx xxxx
	{
		AssertFmt(strPntr->length >= 3, "Expected at least two more bytes in string following: 0x%02X", bytePntr[0]);
		AssertFmt(bytePntr[1] >= 0x80 && bytePntr[1] < 0xC0, "3-byte UTF-8 sequence has invalid second byte: 0x%02X 0x%02X 0x%02X", bytePntr[0], bytePntr[1], bytePntr[2]);
		AssertFmt(bytePntr[2] >= 0x80 && bytePntr[2] < 0xC0, "3-byte UTF-8 sequence has invalid third byte: 0x%02X 0x%02X 0x%02X", bytePntr[0], bytePntr[1], bytePntr[2]);
		result = ((u32)(bytePntr[0] & 0x0F) << 12) | ((u32)(bytePntr[1] & 0x3F) << 6) | ((u32)(bytePntr[2] & 0x3F) << 0);
		codepointByteLength = 3;
	}
	else if (bytePntr[0] < 0xF8) //1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
	{
		AssertFmt(strPntr->length >= 4, "Expected at least three more bytes in string following: 0x%02X", bytePntr[0]);
		AssertFmt(bytePntr[1] >= 0x80 && bytePntr[1] < 0xC0, "4-byte UTF-8 sequence has invalid second byte: 0x%02X 0x%02X 0x%02X 0x%02X", bytePntr[0], bytePntr[1], bytePntr[2], bytePntr[3]);
		AssertFmt(bytePntr[2] >= 0x80 && bytePntr[2] < 0xC0, "4-byte UTF-8 sequence has invalid third byte: 0x%02X 0x%02X 0x%02X 0x%02X", bytePntr[0], bytePntr[1], bytePntr[2], bytePntr[3]);
		AssertFmt(bytePntr[3] >= 0x80 && bytePntr[3] < 0xC0, "4-byte UTF-8 sequence has invalid fourth byte: 0x%02X 0x%02X 0x%02X 0x%02X", bytePntr[0], bytePntr[1], bytePntr[2], bytePntr[3]);
		result = ((u32)(bytePntr[0] & 0x07) << 18) | ((u32)(bytePntr[1] & 0x3F) << 12) | ((u32)(bytePntr[2] & 0x3F) << 6) | ((u32)(bytePntr[3] & 0x3F) << 0);
		codepointByteLength = 4;
	}
	else
	{
		AssertFmt(bytePntr[0] < 0xF8, "Invalid byte in UTF-8 sequence: 0x%02X", bytePntr[0]);
	}
	
	strPntr->length -= codepointByteLength;
	strPntr->bytes += codepointByteLength;
	return result;
}

Str CodepointsToUtf8Str(u64 numCodepoints, const u32* codepoints)
{
	Assert(codepoints != nullptr || numCodepoints == 0);
	Str result = Str_Empty_Const;
	for (u64 cIndex = 0; cIndex < numCodepoints; cIndex++)
	{
		result.length += (u64)CodepointUtf8Size(codepoints[cIndex]);
	}
	result = AllocStr(result.length);
	u64 byteIndex = 0;
	for (u64 cIndex = 0; cIndex < numCodepoints; cIndex++)
	{
		Str codepointStr = CodepointToUtf8InBuffer(codepoints[cIndex], &result.bytes[byteIndex]);
		byteIndex += codepointStr.length;
	}
	return result;
}

// +--------------------------------------------------------------+
// |                            UTF-16                            |
// +--------------------------------------------------------------+
Str16 CodepointToUtf16InBuffer(u32 codepoint, u16* wordBuffer)
{
	Str16 result = MakeStr16(0, wordBuffer);
	if (codepoint < 0x00D800)
	{
		result.words[0] = (u16)codepoint;
		result.length = 1;
	}
	else if (codepoint < 0x010000)
	{
		AssertFmt(codepoint < 0xD800 || codepoint > 0x10000, "Invalid codepoint for UTF-16: 0x%08X", codepoint);
	}
	else if (codepoint < 0x10FFFF)
	{
		result.words[0] = 0xD800 | (((codepoint - 0x010000) & 0x0FFC00) >> 10); //high surrogate
		result.words[0] = 0xDC00 | (((codepoint - 0x010000) & 0x0003FF) >>  0); //low surrogate
		result.length = 2;
	}
	else
	{
		AssertFmt(codepoint <= UTF8_MAX_CODEPOINT, "Invalid codepoint for UTF-16: 0x%08X", codepoint);
	}
	return result;
}

Str16 CodepointToUtf16(u32 codepoint)
{
	Str16 result = AllocStr16(UTF16_MAX_CHAR_SIZE);
	result.length = 0;
	if (codepoint < 0x00D800)
	{
		result.words[0] = (u16)codepoint;
		result.length = 1;
	}
	else if (codepoint < 0x010000)
	{
		AssertFmt(codepoint < 0xD800 || codepoint > 0x10000, "Invalid codepoint for UTF-16: 0x%08X", codepoint);
	}
	else if (codepoint < 0x10FFFF)
	{
		result.words[0] = 0xD800 | (((codepoint - 0x010000) & 0x0FFC00) >> 10); //high surrogate
		result.words[0] = 0xDC00 | (((codepoint - 0x010000) & 0x0003FF) >>  0); //low surrogate
		result.length = 2;
	}
	else
	{
		AssertFmt(codepoint <= UTF8_MAX_CODEPOINT, "Invalid codepoint for UTF-16: 0x%08X", codepoint);
	}
	return result;
}

u32 Utf16ToCodepoint(Str16* strPntr)
{
	NotNull(strPntr);
	u32 result = 0;
	u8 codepointWordLength = 0;
	AssertMsg(strPntr->length > 0, "Empty string passed to Utf8ToCodepoint!");
	
	const u16* wordPntr = strPntr->words;
	if (wordPntr[0] < 0xD800)
	{
		result = (u32)wordPntr[0];
		codepointWordLength = 1;
	}
	else if (wordPntr[0] < 0x010000) //invalid block
	{
		AssertFmt(wordPntr[0] < 0xD800 || wordPntr[0] >= 0x010000, "Invalid UTF-16 word: 0x%04X", wordPntr[0]);
	}
	else if (wordPntr[0] < 0xDC00) //surrogate pairs
	{
		AssertFmt(strPntr->length >= 2, "Expected at least one more word in string following: 0x%04X", wordPntr[0]);
		AssertFmt(wordPntr[1] >= 0xDC00 && wordPntr[1] < 0xE000, "UTF-16 surrogate pair has invalid second word: 0x%04X 0x%04X", wordPntr[0], wordPntr[1]);
		result = ((u32)(wordPntr[0] & 0x03FF) << 10) | ((u32)(wordPntr[1] & 0x03FF) << 0);
		codepointWordLength = 2;
	}
	else
	{
		AssertFmt(wordPntr[0] < 0xDC00, "Invalid word in UTF-16 sequence: 0x%04X", wordPntr[0]);
	}
	
	strPntr->length -= codepointWordLength;
	strPntr->words += codepointWordLength;
	return result;
}

#endif //  _PIG_BUILD_UNICODE_H
