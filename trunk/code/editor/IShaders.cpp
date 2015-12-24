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
//-----------------------------------------------------------------------------
//
// $LogFile$
// $Revision: 1.1.2.2 $
// $Author: ttimo $
// $Date: 2000/02/24 22:24:45 $
// $Log: IShaders.cpp,v $
// Revision 1.1.2.2  2000/02/24 22:24:45  ttimo
// RC2
//
// Revision 1.1.2.1  2000/02/11 03:52:30  ttimo
// working on the IShader interface
//
//
// DESCRIPTION:
// implementation of the shaders / textures interface
//

#include "stdafx.h"
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/materialSystemAPI.h>

// this is a modified version of Texture_ForName
qtexture_t* WINAPI QERApp_TryTextureForName(const char* name)
{
	qtexture_t *q;
	char filename[1024];
	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
	{
		if (!strcmp(name,  q->filename))
				return q;
	}
	// try loading from file .. this is a copy of the worst part of Texture_ForName
	//char cWork[1024];
	//sprintf (filename, "%s/%s.tga", ValueForKey (g_qeglobals.d_project_entity, "texturepath"), name);
	//QE_ConvertDOSToUnixName( cWork, filename );
	//strcpy(filename, cWork);
	//unsigned char* pPixels = NULL;
	//int nWidth;
	//int nHeight;
	//LoadImage(filename, &pPixels, &nWidth, &nHeight);
	//if (pPixels == NULL)
	//{
	//	// try jpg
	//	// blatant assumption of .tga should be fine since we sprintf'd it above
	//	int nLen = strlen(filename);
	//	filename[nLen-3] = 'j';
	//	filename[nLen-2] = 'p';
	//	filename[nLen-1] = 'g';
	//	LoadImage(filename, &pPixels, &nWidth, &nHeight);;
	//	if (pPixels == NULL)
	//	{
	//		// try png
	//		// blatant assumption of .tga should be fine since we sprintf'd it above
	//		int nLen = strlen(filename);
	//		filename[nLen-3] = 'p';
	//		filename[nLen-2] = 'n';
	//		filename[nLen-1] = 'g';
	//		LoadImage(filename, &pPixels, &nWidth, &nHeight);
	//	}
	//}

		        HDC currentHDC = wglGetCurrentDC();
	        HGLRC currentHGLRC = wglGetCurrentContext();
		////	HGLRC qioRC;
		//	rf->getHGLRC(&qioRC);
          //++timo FIXME: this may duplicate with qtexture_t* WINAPI QERApp_Texture_ForName (const char *name)
          //++timo FIXME: we need a list of lawfull GL contexts or something?
          // I'd rather always use the same GL context for binding...

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
	q->value = 0;
	q->contents = 0;
	q->qioMat = mat;
		//++timo storing the filename .. will be removed by shader code cleanup
		// this is dirty, and we sure miss some places were we should fill the filename info
		strcpy( q->filename, name );
		SetNameShaderInfo(q, filename, name);
		Sys_Printf ("done.\n", name);
	//	free(pPixels);
		return q;
	}
	return NULL;
}
