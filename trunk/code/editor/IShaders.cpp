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

// this is a modified version of Texture_ForName
qtexture_t* WINAPI QERApp_TryTextureForName(const char* name)
{
	qtexture_t *q;
	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
	{
		if (!strcmp(name,  q->filename))
				return q;
	}

	HDC currentHDC = wglGetCurrentDC();
	HGLRC currentHGLRC = wglGetCurrentContext();

	if (currentHDC != g_qeglobals.d_hdcBase || currentHGLRC != g_qeglobals.d_hglrcBase)
		wglMakeCurrent( g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase );

	//++timo I don't set back the GL context .. I don't know how safe it is
	//  wglMakeCurrent( currentHDC, currentHGLRC );

	char fullName[256];
	sprintf(fullName,"textures/%s",name);
	mtrAPI_i *mat = g_ms->registerMaterial(fullName);
	if (mat)
	{
		qtexture_t* q = (qtexture_t*)qmalloc(sizeof(*q));
		q->width = mat->getImageWidth();
		q->height = mat->getImageHeight();
		q->flags = 0;
		q->contents = 0;
		q->qioMat = mat;
		//++timo storing the filename .. will be removed by shader code cleanup
		// this is dirty, and we sure miss some places were we should fill the filename info
		strcpy( q->filename, name );
		Sys_Printf ("done.\n", name);
	//	free(pPixels);
		q->next = g_qeglobals.d_qtextures;
		g_qeglobals.d_qtextures = q;
		return q;
	}
	return NULL;
}
