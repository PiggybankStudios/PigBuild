/*
File:   random.c
Author: Taylor Robbins
Date:   05\25\2026
Description: 
	** None
*/

#include "random.h"

u64 RandomNext(u64* state)
{
	//n(x+1) = n(x) * A + C modulo M
	//Values taken from https://nuclear.llnl.gov/CNP/rng/rngman/node4.html
	u64 newState = ((*state) * 2862933555777941757ULL + 3037000493ULL) & 0xFFFFFFFFFFFFFFFFULL;
	*state = newState;
	return newState;
}
