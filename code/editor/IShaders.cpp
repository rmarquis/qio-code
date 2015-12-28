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


#include "stdafx.h"
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/materialSystemAPI.h>

// this is a modified version of QERApp_TryTextureForName
mtrAPI_i * WINAPI QERApp_TryTextureForName(const char* name)
{
	char fullName[256];
	if(name[0] != '(' && strnicmp(name,"textures",strlen("textures"))) 
	{
		sprintf(fullName,"textures/%s",name);

		name = fullName; // HACK HACK HACK
	}
	else
	{
		strcpy(fullName,name);
	}
	HDC currentHDC = wglGetCurrentDC();
	HGLRC currentHGLRC = wglGetCurrentContext();

	if (currentHDC != g_qeglobals.d_hdcBase || currentHGLRC != g_qeglobals.d_hglrcBase)
		wglMakeCurrent( g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase );

	//++timo I don't set back the GL context .. I don't know how safe it is
	//  wglMakeCurrent( currentHDC, currentHGLRC );

	// This should always return a non-null pointer, altought it will be a default-image material if the valid one is missing
	mtrAPI_i *mat = g_ms->registerMaterial(fullName);
	Sys_Printf ("Loading material (or texture) %s done.\n", name);
	return mat;
}
