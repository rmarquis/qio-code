/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// start of shared cmdlib stuff
// 

#ifndef __CMDLIB__
#define __CMDLIB__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifndef __BYTEBOOL__
#define __BYTEBOOL__

#ifndef __cplusplus
  typedef enum {false, true} boolean;
#else
  typedef unsigned char boolean;
#endif


typedef unsigned char byte;
//typedef unsigned char byte;
#endif

void 	StripFilename (char *path);
void 	StripExtension (char *path);
void	ExtractFileName (const char *path, char *dest);
void 	ExtractFileBase (const char *path, char *dest);

void *qmalloc (size_t size);
void* qblockmalloc(size_t nSize);




void Error(const char *pFormat, ...);
void Printf(const char *pFormat, ...);
void ErrorNum(int n, const char *pFormat, ...);
void PrintfNum(int n, const char *pFormat, ...);

void ConvertDOSToUnixName( char *dst, const char *src );



#endif