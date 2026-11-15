/*
File:   pig_build_shell_equivalents.c
Author: Taylor Robbins
Date:   10\30\2026
Description: 
	** Since Pig Build replaces the majority of shell usage in
	** your build system, we make an effort to reduce the onboarding
	** if you come from writing a lot of bash or cmd or other shell
	** scripts for your build. Mostly this API focuses on tools
	** you would use for file manipulation or navigation
*/

#ifndef _PIG_BUILD_SHELL_EQUIVALENTS_H
#define _PIG_BUILD_SHELL_EQUIVALENTS_H

#include "pig_build_base.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_unicode.h"

// +--------------------------------------------------------------+
// |                pig_word_count (wc equivalent)                |
// +--------------------------------------------------------------+
typedef struct pig_word_count_args pig_word_count_args;
struct pig_word_count_args
{
	bool path; //false = treat the pathOrString arg as a string, true = treat the pathOrString arg as a file path and operate on the contents of that file
	bool lines; //false = count words, true = count words
	bool longest; //return the length of the longest word/line
	bool codepoints; //count the number of UTF-8 codepoints rather than the number of bytes in the encoding (only when lines=false)
	const char* word_chars;
	const char* non_word_chars;
	const char* new_line_sequences; //Comma separated list of sequences that we count as a line separator (default = "\n,\r\n")
	bool count_last_line; //Count the last line of the input even if there is no trailing new-line sequence (this is not the standard for unix files, so you must request this explicitly)
};
typedef struct { pig_word_count_args wrapped; } pig_word_count_args_wrapper;
#define pig_word_count(pathNt, ...) pig_word_count_(MakeStrNt(pathNt), INIT(pig_word_count_args_wrapper){{ __VA_ARGS__ }}.wrapped)

u64 pig_word_count_(Str pathOrString, pig_word_count_args args)
{
	Str targetStr = args.path ? ReadEntireFile(pathOrString) : CopyStr(pathOrString);
	if (args.new_line_sequences == nullptr) { args.new_line_sequences = "\n,\r\n"; }
	if (IsEmptyNt(args.word_chars) && IsEmptyNt(args.non_word_chars))
	{
		args.word_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	}
	
	u64 result = 0;
	
	if (args.lines)
	{
		StrArray newLineSequences = MakeStrArrayBySplitting(false, StrLit(","), MakeStrNt(args.new_line_sequences));
		u64 lineStartIndex = 0;
		for (u64 cIndex = 0; cIndex < targetStr.length; cIndex++)
		{
			bool isNewLineSequence = false;
			u64 newLineSequenceNumBytes = 0;
			for (u64 sIndex = 0; sIndex < newLineSequences.length; sIndex++)
			{
				Str newLineSequence = newLineSequences.strings[sIndex];
				if (cIndex + newLineSequence.length <= targetStr.length &&
					StrExactEquals(StrSlice(targetStr, cIndex, cIndex + newLineSequence.length), newLineSequence))
				{
					isNewLineSequence = true;
					newLineSequenceNumBytes = newLineSequence.length;
					break;
				}
			}
			
			if (isNewLineSequence)
			{
				if (args.longest)
				{
					u64 lineLength = cIndex - lineStartIndex;
					if (args.codepoints)
					{
						lineLength = CountCodepointsInUtf8Str(StrSlice(targetStr, lineStartIndex, cIndex));
					}
					result = Max2(result, lineLength);
				}
				else
				{
					result++;
				}
				cIndex += newLineSequenceNumBytes-1;
				lineStartIndex = cIndex+1;
			}
		}
		FreeStrArray(&newLineSequences);
		
		if (args.count_last_line) { result += 1; }
	}
	else
	{
		bool usingWordCodepoints = true;
		Array_u32 wordCodepoints = EMPTY;
		Array_u32 nonWordCodepoints = EMPTY;
		if (args.word_chars != nullptr && args.word_chars[0] != '\0')
		{
			AssertMsg(args.non_word_chars == nullptr || args.non_word_chars[0] == '\0', "You cannot pass word_chars and non_word_chars to pig_word_count at the same time!");
			Str wordCharsStr = MakeStrNt(args.word_chars);
			while (wordCharsStr.length > 0) { AddValueArray_u32(&wordCodepoints, TakeNextUtf8Codepoint(&wordCharsStr)); }
			usingWordCodepoints = true;
		}
		else if (args.non_word_chars != nullptr && args.non_word_chars[0] != '\0')
		{
			Str nonWordCharsStr = MakeStrNt(args.non_word_chars);
			while (nonWordCharsStr.length > 0) { AddValueArray_u32(&nonWordCodepoints, TakeNextUtf8Codepoint(&nonWordCharsStr)); }
			usingWordCodepoints = false;
		}
		else { AssertMsg(false, "Unreachable because of check at start of function!"); }
		
		Str remainingStr = targetStr;
		bool wasPrevWord = false;
		while (remainingStr.length > 0)
		{
			u32 targetCodepoint = TakeNextUtf8Codepoint(&remainingStr);
			bool isWordCodepoint = false;
			if (usingWordCodepoints)
			{
				for (u64 cIndex = 0; cIndex < wordCodepoints.length; cIndex++)
				{
					if (wordCodepoints.values[cIndex] == targetCodepoint) { isWordCodepoint = true; break; }
				}
			}
			else
			{
				isWordCodepoint = true;
				for (u64 cIndex = 0; cIndex < nonWordCodepoints.length; cIndex++)
				{
					if (nonWordCodepoints.values[cIndex] == targetCodepoint) { isWordCodepoint = false; break; }
				}
			}
			
			if (isWordCodepoint != wasPrevWord)
			{
				if (!isWordCodepoint)
				{
					result++;
				}
				wasPrevWord = isWordCodepoint;
			}
		}
		if (wasPrevWord && targetStr.length > 0) { result++; }
	}
	
	FreeStr(&targetStr);
	return result;
}

// TODO: awk/grep/sed
// TODO: basename/dirname
// TODO: cksum?
// TODO: cmp
// TODO: cp/mv/rm
// TODO: cat
// TODO: cut
// TODO: echo/printf
// TODO: env
// TODO: find
// TODO: kill
// TODO: ls/cd
// TODO: mkdir
// TODO: mktemp
// TODO: pidof?
// TODO: pwd
// TODO: readlink
// TODO: realpath
// TODO: rmdir
// TODO: sleep/usleep
// TODO: stat
// TODO: tee?
// TODO: date/time
// TODO: touch?
// TODO: tar/uncompress/unzip?
// TODO: watch?
// TODO: wc
// TODO: which
// TODO: whoami

#endif //  _PIG_BUILD_SHELL_EQUIVALENTS_H
