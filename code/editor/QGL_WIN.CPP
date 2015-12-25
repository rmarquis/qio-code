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
/*
** QGL_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#include "stdafx.h"
#include <float.h>

HMODULE g_hGLDLL = NULL;

#	pragma warning (disable : 4113 4133 4047 )

BOOL  ( WINAPI * qwglSwapBuffers)(HDC);



/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void QGL_Shutdown( void )
{
	if (g_hGLDLL)
	{
    ::FreeLibrary(g_hGLDLL);
    g_hGLDLL = NULL;
	}

	
	qwglSwapBuffers              = NULL;

}

#	define GPA(h, a ) GetProcAddress( h, a )

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to 
** the appropriate GL stuff.  In Windows this means doing a 
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
** 
*/
bool QGL_Init(const char *dllname )
{

  g_hGLDLL = ::LoadLibrary(dllname);

if(g_hGLDLL == 0)
return false;
	qwglSwapBuffers           =  (BOOL (__stdcall *)(HDC))  GPA(g_hGLDLL, "wglSwapBuffers" );


	return true;
}

#pragma warning (default : 4113 4133 4047 )



