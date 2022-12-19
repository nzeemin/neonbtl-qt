// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _MSC_VER
//NOTE: I know, we use unsafe string copy functions
#define _CRT_SECURE_NO_WARNINGS
#endif

// C RunTime Header Files
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstdbool>

#ifdef _MSC_VER
#include <cstdlib>
#endif

#ifdef __APPLE__
#include <cstdlib>
#else
#include <malloc.h>
#endif

#include <memory.h>

#ifndef __GNUC__
#include <tchar.h>
#endif

#include "Common.h"
