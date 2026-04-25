/*
File:   pig_build.h
Author: Taylor Robbins
Date:   03\21\2026
Description:
	** If you just want to include all the pig_build helper files you can include this file
	** Alternatively you can include individual files instead of including this file
*/

#ifndef _PIG_BUILD_H
#define _PIG_BUILD_H

#include "pig_build_base.h"
#include "pig_build_array.h"
#include "pig_build_list.h"
#include "pig_build_str.h"
#include "pig_build_str_array.h"
#include "pig_build_str_list.h"
#include "pig_build_file.h"
#include "pig_build_hash.h"
#include "pig_build_misc.h"
#include "pig_build_recompile.h"
#include "pig_build_cli_flags.h"
#include "pig_build_tags.h"
#include "pig_build_arg_list.h"
#include "pig_build_http.h"
#include "pig_build_zip.h"

// These are optional headers, mostly for PigCore-based projects or other PiggybankStudios repositories.
// They are included in PigBuild for reference but not expected to be used by other projects
#ifndef PIG_BUILD_INCLUDE_OPTIONAL_HEADERS
#define PIG_BUILD_INCLUDE_OPTIONAL_HEADERS 0
#endif
#if PIG_BUILD_INCLUDE_OPTIONAL_HEADERS
#include "optional/pig_build_pig_core_flags.h"
#include "optional/pig_build_not_regex.h"
#include "optional/pig_build_shader_scraping.h"
#include "optional/pig_build_android.h"
#include "optional/pig_build_emscripten.h"
#include "optional/pig_build_playdate.h"
#include "optional/pig_build_orca.h"
#endif

#endif //  _PIG_BUILD_H
