/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2012-2014 V.

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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file
#define C_ONLY

#define PRODUCT_NAME			"Qio"
#define BASEGAME			"baseqio"
#define CLIENT_WINDOW_TITLE     	"Qio"
#define CLIENT_WINDOW_MIN_TITLE 	"Qio"
#define GAMENAME_FOR_MASTER		"QioTestGame"	// must NOT contain whitespace

// Heartbeat for dpmaster protocol. You shouldn't change this unless you know what you're doing
#define HEARTBEAT_FOR_MASTER		"DarkPlaces"

#ifndef PRODUCT_VERSION
  #define PRODUCT_VERSION "0.1"
#endif

#define Q3_VERSION PRODUCT_NAME " " PRODUCT_VERSION

#define MAX_MASTER_SERVERS      5	// number of supported master servers

#define DEMOEXT	"dm_"			// standard demo extension

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#if (defined _MSC_VER)
#define Q_EXPORT __declspec(dllexport)
#elif (defined __SUNPRO_C)
#define Q_EXPORT __global
#elif ((__GNUC__ >= 3) && (!__EMX__) && (!sun))
#define Q_EXPORT __attribute__((visibility("default")))
#else
#define Q_EXPORT
#endif


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
  #include <io.h>

  typedef __int64 int64_t;
  typedef __int32 int32_t;
  typedef __int16 int16_t;
//  typedef __int8 int8_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int8 uint8_t;
#else
  #include <stdint.h>

#endif



#include "q_platform.h"

//=============================================================

#include "../shared/typedefs.h"




#ifndef NULL
#define NULL ((void *)0)
#endif

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))


#define STRING(s)			#s
// expand constants before stringifying them
#define XSTRING(s)			STRING(s)

#define	MAX_QPATH			256		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH		64		// max length of a client name

// paramters for command buffer stuffing
enum cbufExec_e {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
};

#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
enum errorParm_e {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
};

//=============================================

int Com_HexStrToInt( const char *str );

int QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

char *Com_SkipTokens( char *s, int numTokens, char *sep );

void Com_RandomBytes( byte *string, int len );

//=============================================

bool Q_isanumber( const char *s );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
// V: this will treat '/' and '\' as equal
int		Q_stricmpn_slashes(const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
const char	*Q_stristr( const char *s, const char *find);

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// removes color sequences from string
char *Q_CleanStr( char *string );
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=====================================================================

#endif	// __Q_SHARED_H
