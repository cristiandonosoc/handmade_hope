/*  =====================================================================
    $File: common_types.h
    $Creation Date: 2015-01-08
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    =====================================================================

    Defines common data types to be used throughout the application

    ===================================================================== */

#ifndef _WIN32_COMMON_TYPES_INCLUDED
#define _WIN32_COMMON_TYPES_INCLUDED

#include <stdint.h>

// We rename static to some aliases to make more transparent the use of each
#define internal static           // Makes functions scoped to the 'translation unit'
#define global_variable static    // A variable available to all (or many) scopes
#define local_persist static      // A scoped-variable that survives such scope

// Convenient typedef taken from stdint.h
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;

#define PI32 3.14159265359f
#endif
