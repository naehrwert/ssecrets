/*
* Copyright 2012, 2013 naehrwert
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef _TYPES_H_
#define _TYPES_H_

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
#if defined(_WIN32) && defined(_MSC_VER)
typedef __int64 s64;
typedef unsigned __int64 u64;
#else
typedef long long int s64;
typedef unsigned long long int u64;
#endif

#define BOOL int
#define TRUE 1
#define FALSE 0

#endif
