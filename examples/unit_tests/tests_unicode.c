/*
File:   tests_unicode.c
Author: Taylor Robbins
Date:   05\22\2026
Description:
	** Holds tests for UTF-8 and USC2 conversion code inside "pig_build_unicode.h"
*/

void RunTests_Unicode()
{
	u8 utf8Bytes[] = { 0xC2, 0x8F, 0xE3, 0x81, 0x8A, 0x71 };
	Str utf8Str = MakeStr(ArrayCount(utf8Bytes), &utf8Bytes[0]);
	u64 codepointIndex = 0; 
	while (utf8Str.length > 0)
	{
		u32 codepoint = TakeNextUtf8Codepoint(&utf8Str);
		PrintLine("Codepoint[%llu]: 0x%08X", codepointIndex, codepoint);
		codepointIndex++;
	}
	
	u32 codepoints[] = { 0x304A, 0x80BA, 0x8449, 0x3054, 0x3056, 0x3044, 0x307E, 0x3059 }; //ohaiyougozaimasu
	for (u64 cIndex = 0; cIndex < ArrayCount(codepoints); cIndex++)
	{
		Str codepointStr = CodepointToUtf8(codepoints[cIndex]);
		if (codepointStr.length == 0) { PrintLine("Codepoint 0x%08X couldn't be encoded into UTF-8!", codepoints[cIndex]); }
		if (codepointStr.length == 1) { PrintLine("Codepoint 0x%08X becomes [1]{0x%02X}\"%.*s\"", codepoints[cIndex], codepointStr.bytes[0], StrPrint(codepointStr)); }
		else if (codepointStr.length == 2) { PrintLine("Codepoint 0x%08X becomes [2]{0x%02X, 0x%02X}\"%.*s\"", codepoints[cIndex], codepointStr.bytes[0], codepointStr.bytes[1], StrPrint(codepointStr)); }
		else if (codepointStr.length == 3) { PrintLine("Codepoint 0x%08X becomes [3]{0x%02X, 0x%02X, 0x%02X}\"%.*s\"", codepoints[cIndex], codepointStr.bytes[0], codepointStr.bytes[1], codepointStr.bytes[2], StrPrint(codepointStr)); }
		else if (codepointStr.length == 4) { PrintLine("Codepoint 0x%08X becomes [4]{0x%02X, 0x%02X, 0x%02X, 0x%02X}\"%.*s\"", codepoints[cIndex], codepointStr.bytes[0], codepointStr.bytes[1], codepointStr.bytes[2], codepointStr.bytes[3], StrPrint(codepointStr)); }
		
		Str16 codepointStr16 = CodepointToUtf16(codepoints[cIndex]);
		if (codepointStr16.length == 0) { PrintLine("Codepoint 0x%08X couldn't be encoded into UTF-16!", codepoints[cIndex]); }
		if (codepointStr16.length == 1) { PrintLine("Codepoint 0x%08X becomes [1]{0x%04X}", codepoints[cIndex], codepointStr16.words[0]); }
		else if (codepointStr16.length == 2) { PrintLine("Codepoint 0x%08X becomes [2]{0x%04X, 0x%04X}", codepoints[cIndex], codepointStr16.words[0], codepointStr16.words[1]); }
	}
	StrFull codepointsStrFull = MakeStrFull(ArrayCount(codepoints), &codepoints[0]);
	Str fullStr = CodepointsToUtf8Str(codepointsStrFull);
	PrintLine("fullStr [%llu]\"%.*s\"", fullStr.length, StrPrint(fullStr));
}