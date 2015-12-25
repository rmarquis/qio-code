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
// TexWnd.cpp : implementation file
//

#include "stdafx.h"
#include <assert.h>
#include "Radiant.h"
#include "TexWnd.h"
#include "qe3.h"
#include "io.h"
#include "PrefsDlg.h"
#include "str.h"
#include "PrefsDlg.h"
#include <windows.h>
#include <gl/glu.h>
#include <vector>
#include "qgl.h"
#include <api/rAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/materialSystemAPI.h>
#include <api/textureAPI.h>
#include <api/vfsAPI.h>

Str m_gStr;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

mtrAPI_i	*notexture = NULL;
mtrAPI_i	*g_pluginTexture = NULL;

static bool	nomips = false;

#define	FONT_HEIGHT	10

HGLRC s_hglrcTexture = NULL;
HDC	 s_hdcTexture = NULL;

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
//int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

// this is the global counter for GL bind numbers
//int		texture_extension_number = 1;
std::vector<unsigned int> textureNumbers;

int g_nCurrentTextureMenuName;

int g_nTextureOffset = 0;

// current active texture directory.  if empty, show textures in use
char		texture_directory[128];	// use if texture_showinuse is false
bool	texture_showinuse;

bool g_bFilterEnabled = false;
CString g_strFilter;

// texture layout functions
u32 curIndex = 0;
int			current_x, current_y, current_row;

int			texture_nummenus;
#define		MAX_TEXTUREDIRS	128
char		texture_menunames[MAX_TEXTUREDIRS][128];

bool	g_dontuse = true;		// set to true to load the texture but not flag as used

// void SelectTexture (int mx, int my, bool bShift = false);
void SelectTexture (int mx, int my, bool bShift, bool bFitScale=false);

void	Texture_MouseDown (int x, int y, int buttons);
void	Texture_MouseUp (int x, int y, int buttons);
void	Texture_MouseMoved (int x, int y, int buttons);

CPtrArray g_lstShaders;



// gets active texture extension
// 
// FIXME: fix this to be generic from project file
//
int GetTextureExtensionCount()
{
  return 3;
}

const char* GetTextureExtension(int nIndex)
{
  if ( nIndex == 0)
  {
	 return "tga" ;
  }
  if ( nIndex == 1)
  {
	 return "png" ;
  }
  return "jpg";
}

void SortTextures(void)
{	
	//mtrAPI_i	*q, *qtemp, *qhead, *qcur, *qprev;

	//// standard insertion sort
	//// Take the first texture from the list and
	//// add it to our new list
	//if ( g_qeglobals.d_qtextures == NULL)
	//	return;	

	//qhead = g_qeglobals.d_qtextures;
	//q = g_qeglobals.d_qtextures->next;
	//qhead->next = NULL;
	//
	//// while there are still things on the old
	//// list, keep adding them to the new list
	//while (q)
	//{
	//	qtemp = q;
	//	q = q->next;
	//	
	//	qprev = NULL;
	//	qcur = qhead;

	//	while (qcur)
	//	{
	//		// Insert it here?
	//		if (strcmp(qtemp->getName(), qcur->getName()) < 0)
	//		{
	//			qtemp->next = qcur;
	//			if (qprev)
	//				qprev->next = qtemp;
	//			else
	//				qhead = qtemp;
	//			break;
	//		}
	//		
	//		// Move on

	//		qprev = qcur;
	//		qcur = qcur->next;


	//		// is this one at the end?

	//		if (qcur == NULL)
	//		{
	//			qprev->next = qtemp;
	//			qtemp->next = NULL;
	//		}
	//	}


	//}

	//g_qeglobals.d_qtextures = qhead;
}

void SetTexParameters (void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_mode );
	
	switch ( texture_mode )
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	}
}

/*
============
Texture_SetMode
============
*/
void Texture_SetMode(int iMenu)
{
	int iMode;
	HMENU hMenu;
	bool texturing = true;

	hMenu = GetMenu(g_qeglobals.d_hwndMain);

	switch(iMenu) {
	case ID_VIEW_NEAREST:					
		iMode = GL_NEAREST;
		break;
	case ID_VIEW_NEARESTMIPMAP:
		iMode = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ID_VIEW_LINEAR:
		iMode = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ID_VIEW_BILINEAR:
		iMode = GL_LINEAR;
		break;
	case ID_VIEW_BILINEARMIPMAP:
		iMode = GL_LINEAR_MIPMAP_NEAREST;
		break;
	case ID_VIEW_TRILINEAR:
		iMode = GL_LINEAR_MIPMAP_LINEAR;
		break;

	case ID_TEXTURES_WIREFRAME:
		iMode = 0;
		texturing = false;
		break;

	case ID_TEXTURES_FLATSHADE:
		iMode = 0;
		texturing = false;
		break;

	}

	CheckMenuItem(hMenu, ID_VIEW_NEAREST, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_NEARESTMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEARMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_TRILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_FLATSHADE, MF_BYCOMMAND | MF_UNCHECKED);

	CheckMenuItem(hMenu, iMenu, MF_BYCOMMAND | MF_CHECKED);

	g_qeglobals.d_savedinfo.iTexMenu = iMenu;
	texture_mode = iMode;

  if (g_PrefsDlg.m_bSGIOpenGL)
  {
    if (s_hdcTexture && s_hglrcTexture)
    {
      //if (!qwglMakeCurrent(g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase))
      if (!wglMakeCurrent(s_hdcTexture, s_hglrcTexture))
		    Error ("wglMakeCurrent in LoadTexture failed");
    }
    else
      return;
  }

	if ( texturing )
		SetTexParameters ();

	if ( !texturing && iMenu == ID_TEXTURES_WIREFRAME)
	{
		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_wire;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;

	} else if ( !texturing && iMenu == ID_TEXTURES_FLATSHADE) {

		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_solid;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;
	}

	//for (i=1 ; i<texture_extension_number ; i++)
	for (u32 i = 0, ilen = textureNumbers.size(); i < ilen; i++)
	{
		glBindTexture( GL_TEXTURE_2D, textureNumbers[i] );
		SetTexParameters ();
	}

	// select the default texture
	glBindTexture( GL_TEXTURE_2D, 0 );

	glFinish();

	if (g_pParentWnd->GetCamera()->Camera().draw_mode != cd_texture)
	{
		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_texture;
		Map_BuildBrushData();
	}

	Sys_UpdateWindows (W_ALL);
}

/*
================
R_MipMap

Operates in place, quartering the size of the texture
================
*/
void R_MipMap (byte *in, int &width, int &height)
{
	int		i, j;
	byte	*out;
	int		row;
	
	row = width * 4;
	width >>= 1;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=row)
	{
		for (j=0 ; j<width ; j++, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[row+0] + in[row+4])>>2;
			out[1] = (in[1] + in[5] + in[row+1] + in[row+5])>>2;
			out[2] = (in[2] + in[6] + in[row+2] + in[row+6])>>2;
			out[3] = (in[3] + in[7] + in[row+3] + in[row+7])>>2;
		}
	}
}





void ReplaceQTexture(mtrAPI_i *pOld, mtrAPI_i *pNew, brush_s *pList)
{ 
	for (brush_s* pBrush = pList->next ; pBrush != pList; pBrush = pBrush->next)
	{
		if (pBrush->patchBrush)
		{
			Patch_ReplaceQTexture(pBrush, pOld, pNew);
		}

		for (face_s* pFace = pBrush->brush_faces; pFace; pFace = pFace->next)
		{
			if (pFace->d_texture == pOld)
			{
			pFace->d_texture = pNew;
			}
		}
		
		//Brush_Build(pBrush);
	}
}
//
//
//void Texture_Remove(mtrAPI_i *q)
//{
////  mtrAPI_i* pTex = g_qeglobals.d_qtextures->next;
////  if (q == g_qeglobals.d_qtextures)   // it is the head
////  {
////    g_qeglobals.d_qtextures->next = q->next->next;
////    g_qeglobals.d_qtextures = q->next;
////  }
////  else
////  {
////    mtrAPI_i* pLast = g_qeglobals.d_qtextures;
////    while (pTex != NULL && pTex != g_qeglobals.d_qtextures)
////    {
////      if (pTex == q)
////      {
////        pLast->next = q->next;
////        break;
////      }
////      pLast = pTex;
////      pTex = pTex->next;
////    }
////  }
//////  glDeleteTextures(1, reinterpret_cast<const unsigned int*>(&q->texture_number));
////
//// 
////
////  free(q);
//
//}
//
//
//

/*
==================
FillTextureMenu

==================
*/
void FillTextureMenu (CStringArray* pArray)
{
	//HMENU	hmenu;
	//int		i;
	//struct _finddata_t fileinfo;
	//int		handle;
	//char	dirstring[1024];
	//char	*path;
	//DIRLIST	*list = NULL, *temp;

	//hmenu = GetSubMenu (GetMenu(g_qeglobals.d_hwndMain), MENU_TEXTURE);

	//// delete everything
	//for (i=0 ; i<texture_nummenus ; i++)
	//	DeleteMenu (hmenu, CMD_TEXTUREWAD+i, MF_BYCOMMAND);

 // texture_nummenus = 0;

	//// add everything
 // if (g_qeglobals.d_project_entity)
 // {
 //   //if (g_PrefsDlg.m_bUseShaders)
 //   //{
	//   // path = ValueForKey (g_qeglobals.d_project_entity, "basepath");
	//   // sprintf (dirstring, "%s/scripts/*.shader", path);
 //   //
 //   //}
 //   //--else
 //   //--{
	//    path = ValueForKey (g_qeglobals.d_project_entity, "texturepath");
	//    sprintf (dirstring, "%s/*.*", path);
 //   //--}

	//  handle = _findfirst (dirstring, &fileinfo);
	//  if (handle != -1)
 //   {
	//    do
	//    {
 //       //--if (g_PrefsDlg.m_bUseShaders)
 //       //--{
	//	    //--  if ((fileinfo.attrib & _A_SUBDIR))
 //       //--    continue;
 //       //--}
 //       //--else
 //       //--{
	//	      if (!(fileinfo.attrib & _A_SUBDIR))
	//	        continue;
	//	      if (fileinfo.name[0] == '.')
	//	        continue;
 //       //--}
 //       // add this directory to the menu
	//      AddToDirListAlphabetized(&list, fileinfo.name, FROMDISK);
	//    } while (_findnext( handle, &fileinfo ) != -1);

	//    _findclose (handle);
 //   }

 //   //--if (!g_PrefsDlg.m_bUseShaders)
 //   //--{
 //     GetPackTextureDirs(&list);
 //   //--}

	//  for(temp = list; temp; temp = temp->next)
	//  {
	//	  AppendMenu (hmenu, MF_ENABLED|MF_STRING, CMD_TEXTUREWAD+texture_nummenus, (LPCTSTR)temp->dirname);
	//	  strcpy (texture_menunames[texture_nummenus], temp->dirname);
 //     //--if (!g_PrefsDlg.m_bUseShaders)
 //     //--{
	//	    strcat (texture_menunames[texture_nummenus], "/");
 //     //--}
 //     if (pArray)
 //       pArray->Add(temp->dirname);
	//	  if (++texture_nummenus == MAX_TEXTUREDIRS)
	//	   break;
	//  }

	//  ClearDirList(&list);
 // }


}


/*
==================
Texture_ClearInuse

A new map is being loaded, so clear inuse markers
==================
*/
void Texture_ClearInuse (void)
{
	//mtrAPI_i	*q;

	//for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
 //   {
	//	q->inuse = false;
	//}
}




/*
==============
Texture_ShowDirectory
==============
*/
void	Texture_ShowDirectory (int menunum, bool bLinked)
{
//	struct _finddata_t fileinfo;
//	int		handle;
//	char	name[1024];
//	char	dirstring[1024];
//	char	linkstring[1024];
//	FILELIST			*list = NULL, *temp;
//  CString strTemp;
//
//  //Texture_Flush(false);
//	//Select_Deselect();
//	Texture_ClearInuse();
//	texture_showinuse = false;
//	strcpy (texture_directory, texture_menunames[menunum-CMD_TEXTUREWAD]);
//
//  //if (g_pParentWnd->GetPlugInMgr().GetTextureInfo() != NULL)
//  //{
//  //  if (g_pParentWnd->GetPlugInMgr().GetTextureInfo()->m_bWadStyle)
//  //    return;
//  //}
//
//  // new
///*
//  if (!g_PrefsDlg.m_bShaderTest)
//  {
//	g_dontuse = true;	// needed because this next piece of code calls QERApp_TryTextureForName() internally! -slc
//    LoadDeferred(texture_directory);
//    g_dontuse = false;
//  }
//*/
//
//
//	g_qeglobals.d_texturewin.originy = 0;
//
//  //--if (g_PrefsDlg.m_bUseShaders)
//  //--{
//  //--  sprintf (dirstring, "%s/scripts/%s", ValueForKey (g_qeglobals.d_project_entity, "basepath"), texture_directory);
//	//--  Sys_Printf("loading textures from shader %s\n", dirstring);
//  //--  LoadShader(dirstring);
//  //--}
//  //--else
//  //--{
//	  Sys_Status("Loading textures\n", 0);
//
//	  // load all image files
//                                          
//    sprintf (linkstring, "%s/textures/%stextureinfo.ini", ValueForKey (g_qeglobals.d_project_entity, "basepath"), texture_menunames[menunum-CMD_TEXTUREWAD]);
//
//    for (int nExt = 0; nExt < GetTextureExtensionCount(); nExt++)
//    {
//      sprintf (dirstring, "%s/textures/%s*.%s", ValueForKey (g_qeglobals.d_project_entity, "basepath"), texture_menunames[menunum-CMD_TEXTUREWAD],GetTextureExtension(nExt));
//      Sys_Printf ("Scanning %s\n", dirstring);
//	    handle = _findfirst (dirstring, &fileinfo);
//
//      if (handle == -1)
//      {
//        sprintf(dirstring, "%s/%s*.%s", ValueForKey (g_qeglobals.d_project_entity, "texturepath"), texture_menunames[menunum-CMD_TEXTUREWAD],GetTextureExtension(nExt));
//        handle = _findfirst (dirstring, &fileinfo);
//      }
//      if (handle != -1)
//	    {
//		    do
//  		  {
//	  		  sprintf (name, "%s%s", texture_directory, fileinfo.name);
//		  	  AddToFileListAlphabetized(&list, name, FROMDISK, 0, false);
//  		  } while (_findnext( handle, &fileinfo ) != -1);
//	  	  _findclose (handle);
//  	  }
//	    else
//	    {
//	      sprintf (dirstring, "%s*.%s", texture_menunames[menunum-CMD_TEXTUREWAD],GetTextureExtension(nExt));
//  		  GetPackFileList(&list, dirstring);
//	    }
//    }
//
//	  g_dontuse = true;
//	  for(temp = list; temp; temp = temp->next)
//	  {
//		  if(temp->offset == -1)
//			  sprintf(name, "%s", temp->filename);
//		  else
//			  sprintf(name, "%s%s", texture_menunames[menunum-CMD_TEXTUREWAD], temp->filename);
//		  StripExtension (name);
//      strTemp = name;
//      strTemp.MakeLower();
//      if ( strTemp.Find(".specular") >= 0 ||
//           strTemp.Find(".glow") >= 0 ||
//           strTemp.Find(".bump") >= 0 ||
//           strTemp.Find(".diffuse") >= 0 ||
//           strTemp.Find(".blend") >= 0 ||
//           strTemp.Find(".alpha") >= 0
//         )
//        continue;
//      else
//      {
//		    QERApp_TryTextureForName (name, true);
//      }
//	  }
//
//	  ClearFileList(&list);
//  //--}
//
//
//	g_dontuse = false;
//
//  if (!bLinked)
//  {
//
//    for (int k = 0; k < 10; k++)
//    {
//      sprintf(name, "Path%d", k);
//      if (GetPrivateProfileString("Include", name, "", dirstring, 1024, linkstring) > 0)
//      {
//        Texture_ShowDirectory(dirstring, true);
//      }
//    }
//
//    SortTextures();
//	  
//    sprintf (name, "Textures: %s", texture_directory);
//	  SetWindowText(g_qeglobals.d_hwndEntity, name);
//
//	  // select the first texture in the list
//	  if (!g_qeglobals.d_texturewin.texdef.name[0])
//		  SelectTexture (16, g_qeglobals.d_texturewin.height -16, false);
//  }
}


// this can be combined with the above, but per usual i am in a hurry
//
void	Texture_ShowDirectory (char* pPath, bool bLinked)
{
	//struct _finddata_t fileinfo;
	//int		handle;
	//char	name[1024];
	//char	dirstring[1024];
	//char	linkstring[1024];
	//FILELIST			*list = NULL, *temp;

 // //Texture_Flush(false);

	//texture_showinuse = false;
	//Texture_ClearInuse();
	//strcpy (texture_directory, pPath);



	//g_qeglobals.d_texturewin.originy = 0;
	//Sys_Status("loading all textures\n", 0);

	//// load all .wal files
 // for (int nExt = 0; nExt < GetTextureExtensionCount(); nExt++)
 // {
 //   sprintf(dirstring, "%s*.%s", pPath,GetTextureExtension(nExt));
 //                                         
	//  Sys_Printf ("Scanning %s\n", dirstring);

	//  handle = _findfirst (dirstring, &fileinfo);

 //   if (handle != -1)
	//  {
	//	  do
	//	  {
	//		  sprintf (name, "%s%s", texture_directory, fileinfo.name);
	//		  AddToFileListAlphabetized(&list, name, FROMDISK, 0, false);
	//	  } while (_findnext( handle, &fileinfo ) != -1);
	//	  _findclose (handle);
	//  }
	//  else
	//  {
	//	  //sprintf (dirstring, "%s*.wal", texture_menunames[menunum-CMD_TEXTUREWAD]);
	//	  //if(!GetPackFileList(&list, dirstring))
	//		  return;
	//  }
 // }

	//g_dontuse = true;
	//for(temp = list; temp; temp = temp->next)
	//{
	//	if(temp->offset == -1)
	//		sprintf(name, "%s", temp->filename);
	//	else
	//	  sprintf(name, "%s%s", pPath, temp->filename);
	//	StripExtension (name);

 //   int nLen = strlen(name)-1;
 //   ASSERT(nLen > 0);
 //   while (name[nLen] != '\\')
 //     nLen--;
 //   // found first one
 //   nLen--;
 //   ASSERT(nLen > 0);
 //   while (name[nLen] != '\\')
 //     nLen--;
 //   ASSERT(nLen >= 0);
 //   QE_ConvertDOSToUnixName(name, name);
 //   QERApp_TryTextureForName(&name[nLen+1]);

	//}

	//ClearFileList(&list);

	//g_dontuse = false;

 // SortTextures();

 // if (!bLinked)
 // {

 //   for (int k = 0; k < 10; k++)
 //   {
 //     sprintf(name, "Path%d", k);
 //     if (GetPrivateProfileString("Include", name, "", dirstring, 1024, linkstring) > 0)
 //     {
 //       Texture_ShowDirectory(dirstring, true);
 //     }
 //   }


	//  sprintf (name, "Textures: %s", texture_directory);
	//  SetWindowText(g_qeglobals.d_hwndEntity, name);

	//  // select the first texture in the list
	//  if (!g_qeglobals.d_texturewin.texdef.name[0])
	//	  SelectTexture (16, g_qeglobals.d_texturewin.height -16 ,false);
 // }
}



void Texture_ResetPosition()
{
  SelectTexture (16, g_qeglobals.d_texturewin.height -16 ,false);
  g_qeglobals.d_texturewin.originy = 0;
}



/*
==================
Texture_SetInuse

==================
*/
void Texture_SetInuse (void)
{
	//mtrAPI_i	*q;

	//for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
 // {
	//	q->inuse = true;
	//}
}


/*
==============
Texture_ShowAll
==============
*/
void	Texture_ShowAll()
{
  Texture_SetInuse();
	Sys_Printf("Showing all textures...\n");
	Sys_UpdateWindows (W_TEXTURE);
}

/*
==============
Texture_ShowInuse
==============
*/
void	Texture_ShowInuse (void)
{
	face_s	*f;
	brush_s	*b;
	char	name[1024];

	texture_showinuse = true;
	g_dontuse = false;

	g_qeglobals.d_texturewin.originy = 0;	

	Texture_ClearInuse();
	Sys_Status("Selecting active textures\n", 0);

	for (b=active_brushes.next ; b != NULL && b != &active_brushes ; b=b->next)
	{
		if (b->patchBrush)
		{
			QERApp_TryTextureForName(b->pPatch->d_texture->getName());
		}
		else
		{
			for (f=b->brush_faces ; f ; f=f->next)
			{
				QERApp_TryTextureForName (f->texdef.name);
			}
		}
	}

	for (b=selected_brushes.next ; b != NULL && b != &selected_brushes ; b=b->next)
	{
		if (b->patchBrush)
		{
			QERApp_TryTextureForName(b->pPatch->d_texture->getName());
		}
		else
		{
			for (f=b->brush_faces ; f ; f=f->next)
			{
				QERApp_TryTextureForName (f->texdef.name);
			}
		}
	}

	SortTextures();
	//SetInspectorMode(W_TEXTURE);
	Sys_UpdateWindows (W_TEXTURE);

	sprintf (name, "Textures: in use");
	SetWindowText(g_qeglobals.d_hwndEntity, name);

	// select the first texture in the list
	if (!g_qeglobals.d_texturewin.texdef.name[0])
	{
		SelectTexture (16, g_qeglobals.d_texturewin.height -16, false);
	}
}

/*
============================================================================

TEXTURE LAYOUT

============================================================================
*/

void Texture_StartPos (void)
{
	curIndex = 0;
	current_x = 8;
	current_y = -8;
	current_row = 0;
}

mtrAPI_i *Texture_NextPos (int *x, int *y)
{
	mtrAPI_i	*q = 0;

	while (curIndex < g_ms->getNumAllocatedMaterials())
	{
		q = g_ms->getAllocatedMaterial(curIndex);
		curIndex++;
		if (!q)
			return 0;
///		q->inuse = true;
		///current_texture = current_texture->next;
		if (q->getName()[0] == '(')	// fake color texture
			continue;

		if (g_bFilterEnabled)
		{
			CString strName = q->getName();
			int nPos = strName.Find('\\');
			if (nPos == -1)
				nPos = strName.Find('/');
			if (nPos >= 0)
				strName = strName.Right(strName.GetLength() - nPos - 1);
			if (_strnicmp(g_strFilter.GetBuffer(0), strName, g_strFilter.GetLength()) == 0)
				break;
			else
				continue;
		}

		//if (q->bFromShader && g_PrefsDlg.m_bShowShaders == FALSE)
		//{
		//	continue;
		//}

	////	if (q->inuse)
			break;			// always show in use

	////	if (!texture_showinuse && !_strnicmp (q->getName(), texture_directory, strlen(texture_directory)))
	//		break;
	////	continue;
	}
	if(q == 0)
		return 0;
	int nWidth = q->getImageWidth() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
	int nHeight =q->getImageHeight() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
	if (current_x + nWidth > g_qeglobals.d_texturewin.width-8 && current_row)
	{	// go to the next row unless the texture is the first on the row
		current_x = 8;
		current_y -= current_row + FONT_HEIGHT + 4;
		current_row = 0;
	}

	*x = current_x;
	*y = current_y;

	// Is our texture larger than the row? If so, grow the 
	// row height to match it

	if (current_row < nHeight)
		current_row = nHeight;

	// never go less than 64, or the names get all crunched up
	current_x += nWidth < 64 ? 64 : nWidth;
	current_x += 8;

	return q;
}

/*
============================================================================

  MOUSE ACTIONS

============================================================================
*/

static	int	textures_cursorx, textures_cursory;


/*
============
Texture_SetTexture

brushprimit_texdef must be understood as a mtrAPI_i with width=2 height=2 ( the default one )
============
*/
void Texture_SetTexture (texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale, bool bSetSelection )
{
	mtrAPI_i	*q;
	int			x,y;

	if (texdef->name[0] == '(')
	{
		Sys_Status("Can't select an entity texture\n", 0);
		return;
	}
	g_qeglobals.d_texturewin.texdef = *texdef;
	g_qeglobals.d_texturewin.texdef.flags &= ~SURF_KEEP;
	g_qeglobals.d_texturewin.texdef.contents &= ~CONTENTS_KEEP;
	// store the texture coordinates for new brush primitive mode
	// be sure that all the callers are using the default 2x2 texture
	if (g_qeglobals.m_bBrushPrimitMode)
	{
		g_qeglobals.d_texturewin.brushprimit_texdef = *brushprimit_texdef;
	}


	Sys_UpdateWindows (W_TEXTURE);

  g_dlgFind.updateTextures(texdef->name);

  if (!g_dlgFind.isOpen() && bSetSelection)
  {
    Select_SetTexture(texdef,brushprimit_texdef,bFitScale);
  }


	// scroll origin so the texture is completely on screen
	Texture_StartPos ();
	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;

    int nWidth =q->getImageWidth() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
    int nHeight = q->getImageHeight() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
		if (!_strcmpi(texdef->name, q->getName()))
		{
			if (y > g_qeglobals.d_texturewin.originy)
			{
				g_qeglobals.d_texturewin.originy = y;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			if (y-nHeight-2*FONT_HEIGHT < g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height)
			{
				g_qeglobals.d_texturewin.originy = y-nHeight-2*FONT_HEIGHT+g_qeglobals.d_texturewin.height;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			return;
		}
	}
}


HWND FindEditWindow()
{
  HWND hwnd = FindWindow("TEditPadForm", NULL);
  HWND hwndEdit = NULL;
  if (hwnd != NULL)
  {
    HWND hwndTab = FindWindowEx(hwnd, NULL, "TTabControl", NULL);
    if (hwndTab != NULL)
    {
      hwndEdit = FindWindowEx(hwndTab, NULL, "TRicherEdit", NULL);
    }
  }
  return hwndEdit;
}

void Delay(float fSeconds)
{
  DWORD dw = ::GetTickCount();
  DWORD dwTil = dw + (fSeconds * 1000);
  while (::GetTickCount() < dwTil)
  {
    MSG msg;
    if (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) 
    { 
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}


void ViewShader(const char *pFile, const char *pName)
{
  CString str;
  char* pBuff = NULL;
  int nSize = g_vfs->FS_ReadFile(pFile, reinterpret_cast<void**>(&pBuff));
  if (nSize > 0)
  {
    str = pBuff;
	g_vfs->FS_FreeFile(pBuff);
  }
  int nStart = 0; 
  if (str.GetLength() > 0)
  {
    CString strFind = pName;
    CString strLook = str;
    strLook.MakeLower();
    strFind.MakeLower();
    int n = strLook.Find(strFind);
    if (n >= 0)
    {
      nStart = n;
    }
  }

  CString s= "notepad ";
  s += pFile;
  WinExec(s, SW_SHOWNORMAL);

  Delay(1.5);

  HWND hwndEdit = FindEditWindow();

  if (hwndEdit != NULL)
  {
    PostMessage(hwndEdit, EM_SETSEL, nStart, nStart);
  }
  else
  {
    Sys_Printf("Unable to load shader editor.\n");
  }


}

/*
==============
SelectTexture

  By mouse click
==============
*/
void SelectTexture (int mx, int my, bool bShift, bool bFitScale)
{
	int		x, y;
	mtrAPI_i	*q;
	texdef_t	tex;
	brushprimit_texdef_s brushprimit_tex;

	my += g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height;
	
	Texture_StartPos ();
	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;
		int nWidth =  q->getImageWidth() * ((float)g_PrefsDlg.m_nTextureScale / 100);
		int nHeight = q->getImageHeight() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
		if (mx > x && mx - x < nWidth
			&& my < y && y - my < nHeight + FONT_HEIGHT)
		{
			if (bShift)
			{
				if (q->getSourceFileName() !=  0)
				{
					//CString s = "notepad ";
					//s += q->shadername;
					//WinExec(s, SW_SHOWNORMAL);	
	
					ViewShader(q->getSourceFileName(), q->getName());				

				}
			}
			memset (&tex, 0, sizeof(tex));
			memset (&brushprimit_tex, 0, sizeof(brushprimit_tex));
			if (g_qeglobals.m_bBrushPrimitMode)
			{
				// brushprimit fitted to a 2x2 texture
				brushprimit_tex.coords[0][0] = 1.0f;
				brushprimit_tex.coords[1][1] = 1.0f;
			}
			else
			{
				tex.scale[0] =0.5;
				tex.scale[1] = 0.5;
			}
			tex.flags = 0; //q->flags;
			tex.value = 0;//q->value;
			tex.contents = q->getEditorContentFlags();
			//strcpy (tex.name, q->name);
			tex.SetName(q->getName());
			Texture_SetTexture ( &tex, &brushprimit_tex, bFitScale, 0);
			CString strTex;
			CString strName = q->getName();
			//int nPos = strName.Find('\\');
			//if (nPos == -1)
			//  nPos = strName.Find('/');
			//if (nPos >= 0)
			//  strName = strName.Right(strName.GetLength() - nPos - 1);
			strTex.Format("%s W: %i H: %i", strName.GetBuffer(0), q->getImageWidth(), q->getImageHeight());
			g_pParentWnd->SetStatusText(3, strTex);
			return;
		}
	}

	Sys_Status("Did not select a texture\n", 0);
}

/*
==============
Texture_MouseDown
==============
*/
void Texture_MouseDown (int x, int y, int buttons)
{
	Sys_GetCursorPos (&textures_cursorx, &textures_cursory);

	// lbutton = select texture
	if (buttons == MK_LBUTTON || buttons == (MK_LBUTTON | MK_SHIFT) || buttons == (MK_LBUTTON | MK_CONTROL))
	{
    SelectTexture (x, g_qeglobals.d_texturewin.height - 1 - y, buttons & MK_SHIFT, buttons & MK_CONTROL);
    UpdateSurfaceDialog();
    UpdatePatchInspector();
	}
}

/*
==============
Texture_MouseUp
==============
*/
void Texture_MouseUp (int x, int y, int buttons)
{
}

/*
==============
Texture_MouseMoved
==============
*/
void Texture_MouseMoved (int x, int y, int buttons)
{
	int scale = 1;

	if ( buttons & MK_SHIFT )
		scale = 4;

	// rbutton = drag texture origin
	if (buttons & MK_RBUTTON)
	{
		Sys_GetCursorPos (&x, &y);
		if ( y != textures_cursory)
		{
			g_qeglobals.d_texturewin.originy += ( y-textures_cursory) * scale;
			if (g_qeglobals.d_texturewin.originy > 0)
				g_qeglobals.d_texturewin.originy = 0;
			Sys_SetCursorPos (textures_cursorx, textures_cursory);
      CWnd *pWnd = CWnd::FromHandle(g_qeglobals.d_hwndTexture);
      if (g_PrefsDlg.m_bTextureScrollbar && pWnd != NULL)
      {
        pWnd->SetScrollPos(SB_VERT, abs(g_qeglobals.d_texturewin.originy));
      }
		  InvalidateRect(g_qeglobals.d_hwndTexture, NULL, false);
		  UpdateWindow (g_qeglobals.d_hwndTexture);
		}
		return;
	}
}


/*
============================================================================

DRAWING

============================================================================
*/

int imax(int iFloor, int i) { if (i>iFloor) return iFloor; return i; }
HFONT ghFont = NULL;

/*
============
Texture_Draw2
============
*/
void Texture_Draw2 (int width, int height)
{
	mtrAPI_i	*q;
	int			x, y;
	const char		*name;

	glClearColor (g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][0], g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][2],0);
	glViewport (0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable (GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glOrtho (0, width, g_qeglobals.d_texturewin.originy-height, g_qeglobals.d_texturewin.originy, -100, 100);
	glEnable (GL_TEXTURE_2D);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	g_qeglobals.d_texturewin.width = width;
	g_qeglobals.d_texturewin.height = height;
	Texture_StartPos ();

	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;

		int nWidth =q->getImageWidth() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
		int nHeight =  q->getImageHeight() * ((float)g_PrefsDlg.m_nTextureScale / 100) ;
		// Is this texture visible?
		if ( (y-nHeight-FONT_HEIGHT < g_qeglobals.d_texturewin.originy) && (y > g_qeglobals.d_texturewin.originy - height) )
		{

			// if in use, draw a background
		//	if ((q->inuse && !texture_showinuse) || q->bFromShader)
			{
				glLineWidth (1);

				///if (q->bFromShader)
				{
					glColor3f (1,1,1);
				}
				//else
				//{
				//	glColor3f (0.5,1,0.5);
				//}
				glDisable (GL_TEXTURE_2D);

				glBegin (GL_LINE_LOOP);
				glVertex2f (x-1,y+1-FONT_HEIGHT);
				glVertex2f (x-1,y-nHeight-1-FONT_HEIGHT);
				glVertex2f (x+1+nWidth,y-nHeight-1-FONT_HEIGHT);
				glVertex2f (x+1+nWidth,y+1-FONT_HEIGHT);
				glEnd ();

				glEnable (GL_TEXTURE_2D);
			}

			// Draw the texture
			float fScale =  ((float)g_PrefsDlg.m_nTextureScale / 100);

			if(q->getFirstColorMapStage() && q->getFirstColorMapStage()->getTexture(0))
				glBindTexture( GL_TEXTURE_2D, q->getFirstColorMapStage()->getTexture(0)->getInternalHandleU32());
			QE_CheckOpenGLForErrors();
			glColor3f (1,1,1);
			glBegin (GL_QUADS);
			glTexCoord2f (0,0);
			glVertex2f (x,y-FONT_HEIGHT);
			glTexCoord2f (1,0);
			glVertex2f (x+nWidth,y-FONT_HEIGHT);
			glTexCoord2f (1,1);
			glVertex2f (x+nWidth,y-FONT_HEIGHT-nHeight);
			glTexCoord2f (0,1);
			glVertex2f (x,y-FONT_HEIGHT-nHeight);
			glEnd ();

			// draw the selection border
			if (!_strcmpi(g_qeglobals.d_texturewin.texdef.name, q->getName()))
			{
				glLineWidth (3);
				glColor3f (1,0,0);
				glDisable (GL_TEXTURE_2D);

				glBegin (GL_LINE_LOOP);
				glVertex2f (x-4,y-FONT_HEIGHT+4);
				glVertex2f (x-4,y-FONT_HEIGHT-nHeight-4);
				glVertex2f (x+4+nWidth,y-FONT_HEIGHT-nHeight-4);
				glVertex2f (x+4+nWidth,y-FONT_HEIGHT+4);
				glEnd ();

				glEnable (GL_TEXTURE_2D);
				glLineWidth (1);
			}

			// draw the texture name
			glColor3f (0,0,0);

			glRasterPos2f (x, y-FONT_HEIGHT+2);

			// don't draw the directory name
			for (name = q->getName() ; *name && *name != '/' && *name != '\\' ; name++)
			;
			if (!*name)
				name = q->getName();
			else
				name++;

			//if ( q->shadername[0] != 0)
			//{
			//	// slow as shit
			//	CString s = "[";
			//	s += name;
			//	s += "]";
			//	glCallLists (s.GetLength(), GL_UNSIGNED_BYTE, s.GetBuffer(0));
			//}
			//else
			{
				glCallLists (strlen(name), GL_UNSIGNED_BYTE, name);
			}
		}
	}

	g_qeglobals.d_texturewin.m_nTotalHeight = abs(y) + 100;
	// reset the current texture
	glBindTexture( GL_TEXTURE_2D, 0 );
	glFinish();
}


void Texture_Init (bool bHardInit)
{
	byte	*pal = NULL;

	// create the fallback texture

  if (bHardInit)
  {
		notexture = QERApp_TryTextureForName("default");
////	  g_qeglobals.d_qtextures = NULL;
  }
}

void Texture_FlushUnused()
{
  CWaitCursor cursor;
  //Texture_ShowInuse();
  //if (g_qeglobals.d_qtextures)
  //{
	 // mtrAPI_i* pTex = g_qeglobals.d_qtextures->next;
  //  mtrAPI_i *pPrev = g_qeglobals.d_qtextures;
  //  while (pTex != NULL && pTex != g_qeglobals.d_qtextures)
  //  {
  //    mtrAPI_i* pNextTex = pTex->next;

  //    if (!pTex->inuse)
  //    {
  //   //// ...  unsigned int nTexture = pTex->texture_number;
  //  ////    glDeleteTextures(1, &nTexture);
  //      pPrev->next = pNextTex;
	 //     free(pTex);
  //    }
  //    else
  //    {
  //      pPrev = pTex;
  //    }
  //    pTex = pNextTex;
  //  }
  //}
}

void Texture_Cleanup(CStringList *pList)
{
  //if (g_qeglobals.d_qtextures)
  //{
	 // mtrAPI_i* pTex = g_qeglobals.d_qtextures->next;
  //  while (pTex != NULL && pTex != g_qeglobals.d_qtextures)
  //  {
  //    mtrAPI_i* pNextTex = pTex->next;
  //    if (pList)
  //    {
  //      if (pTex->getName()[0] != '(')
  //      {
  //        pList->AddTail(pTex->getName());
  //      }
  //    }

  //
	 //   free(pTex);
  //    pTex = pNextTex;
  //  }
  //}
}

/*
==================
Texture_Flush
==================
*/
void Texture_Flush (bool bReload)
{
  if (!ConfirmModified())
    return;

  Map_New ();

  CWaitCursor cursor;
  CStringList strList;
  Texture_Init(false);
  Texture_Cleanup(&strList);

 /* GLuint* pGln = new GLuint[texture_extension_number-1];
  glGenTextures(texture_extension_number-1, pGln);
  QE_CheckOpenGLForErrors();
  glDeleteTextures(texture_extension_number-1, pGln);
  QE_CheckOpenGLForErrors();
  delete []pGln;*/
 // texture_extension_number = 1;

  //for(int i = 0; i < textureNumbers.size(); i++) {
  for (u32 i = 0, ilen = textureNumbers.size(); i < ilen; i++) {
	  glDeleteTextures(1,&textureNumbers[i]);
  }
  textureNumbers.clear();
	//glGenTextures(1,& q->texture_number);
	//textureNumbers.push_back(q->texture_number);


///	g_qeglobals.d_qtextures = NULL;

  if (bReload)
  {
    POSITION pos = strList.GetHeadPosition();
    while (pos)
    {
      CString strTex = strList.GetNext(pos);
		  QERApp_TryTextureForName (strTex.GetBuffer(0));
    }
  }

}



/////////////////////////////////////////////////////////////////////////////
// CTexWnd
IMPLEMENT_DYNCREATE(CTexWnd, CWnd);

CTexWnd::CTexWnd()
{
  m_bNeedRange = true;
}

CTexWnd::~CTexWnd()
{
}


BEGIN_MESSAGE_MAP(CTexWnd, CWnd)
	//{{AFX_MSG_MAP(CTexWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PARENTNOTIFY()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_TEXTURES_FLUSH, OnTexturesFlush)
  ON_BN_CLICKED(1200, OnShaderClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTexWnd message handlers

/*
============
WTexWndProc
============
*/
LONG WINAPI TexWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	int		xPos, yPos;
    RECT	rect;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_CREATE:
    s_hdcTexture = GetDC(hWnd);
		QEW_SetupPixelFormat(s_hdcTexture, false);

		if ( ( s_hglrcTexture = wglCreateContext( s_hdcTexture ) ) == 0 )
			Error( "wglCreateContext in WTex_WndProc failed" );

		if (!wglShareLists( g_qeglobals.d_hglrcBase, s_hglrcTexture ) )
			Error( "wglShareLists in WTex_WndProc failed" );

    if (!wglMakeCurrent( s_hdcTexture, s_hglrcTexture ))
		  Error ("wglMakeCurrent in WTex_WndProc failed");

	  g_qeglobals.d_hwndTexture = hWnd;
		return 0;

	case WM_DESTROY:
		//wglMakeCurrent( NULL, NULL );
		//wglDeleteContext( s_hglrcTexture );
		 ReleaseDC( hWnd, s_hdcTexture );
		return 0;
#if 0
	case WM_PAINT:
        { 
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);

        if ( !qwglMakeCurrent( s_hdcTexture, s_hglrcTexture ) )
        //if ( !wglMakeCurrent( ps.hdc, s_hglrcTexture ) )
        {
          Sys_Printf("ERROR: wglMakeCurrent failed..\n ");
          Sys_Printf("Please restart Q3Radiant if the Texture view is not working\n");
        }
        else
        {
			    Texture_Draw2 (rect.right-rect.left, rect.bottom-rect.top - g_nTextureOffset);
			    SwapBuffers(s_hdcTexture);
          TRACE("Texture Paint\n");
        }
		    EndPaint(hWnd, &ps);
        }
		return 0;
#endif
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		SetCapture( g_qeglobals.d_hwndTexture );
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		
		Texture_MouseDown (xPos, yPos - g_nTextureOffset, wParam);
		return 0;

	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		
		Texture_MouseUp (xPos, yPos - g_nTextureOffset, wParam);
		if (! (wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			ReleaseCapture ();
		return 0;

	case WM_MOUSEMOVE:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		
		Texture_MouseMoved (xPos, yPos - g_nTextureOffset, wParam);
		return 0;
    }

    return DefWindowProc (hWnd, uMsg, wParam, lParam);
}



BOOL CTexWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
  WNDCLASS wc;
  HINSTANCE hInstance = AfxGetInstanceHandle();
  if (::GetClassInfo(hInstance, TEXTURE_WINDOW_CLASS, &wc) == FALSE)
  {
    // Register a new class
  	memset (&wc, 0, sizeof(wc));
    wc.style         = CS_NOCLOSE | CS_OWNDC;
    wc.lpszClassName = TEXTURE_WINDOW_CLASS;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.lpfnWndProc = TexWndProc;
	wc.hInstance = hInstance; // V: added
    if (AfxRegisterClass(&wc) == FALSE)
      Error ("CZWnd RegisterClass: failed");
  }

  cs.lpszClass = TEXTURE_WINDOW_CLASS;
  cs.lpszName = "TEX";
  if (cs.style != QE3_CHILDSTYLE && cs.style != QE3_STYLE)
    cs.style = QE3_SPLITTER_STYLE;

	return CWnd::PreCreateWindow(cs);
}

int CTexWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

  CRect rctEdit(8, 5, 20, 20);
  g_nTextureOffset = 0;

/*
  if (g_PrefsDlg.m_bShaderTest)
  {
    m_wndShaders.Create("Show Shaders", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, rctEdit, this, 1200);
    m_wndShaders.ModifyStyleEx(0, WS_EX_CLIENTEDGE, 0);
    m_wndShaders.SetCheck(g_PrefsDlg.m_bShowShaders);
    g_nTextureOffset = 25;
  }
*/
  rctEdit.SetRect(8, g_nTextureOffset, 20, 20);
  m_wndFilter.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT, rctEdit, this, 1201);
  m_wndFilter.ModifyStyleEx(0, WS_EX_CLIENTEDGE, 0);
  m_wndFilter.SetTexWnd(this);

  g_nTextureOffset += 25;
  if (!g_PrefsDlg.m_bTextureWindow)
  {
    m_wndFilter.ShowWindow(SW_HIDE);
    g_nTextureOffset -= 25;
  }

  ShowScrollBar(SB_VERT, g_PrefsDlg.m_bTextureScrollbar);
  m_bNeedRange = true;

	return 0;
}

void CTexWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
  CRect rctClient;
  GetClientRect(rctClient);
/*
  if (g_PrefsDlg.m_bShaderTest && m_wndShaders.GetSafeHwnd())
  {
    m_wndShaders.SetWindowPos(NULL, rctClient.left + 8, rctClient.top + 5, rctClient.right - 16, 20, 0);
  }
*/
  m_wndFilter.SetWindowPos(NULL, rctClient.left + 8, rctClient.top + 25, rctClient.right - 16, 20, 0);
  m_bNeedRange = true;
}

void CTexWnd::OnShaderClick()
{
  g_PrefsDlg.m_bShowShaders = (m_wndShaders.GetCheck() != 0);
  g_PrefsDlg.SavePrefs();
	RedrawWindow();
}

void CTexWnd::OnParentNotify(UINT message, LPARAM lParam) 
{
	CWnd::OnParentNotify(message, lParam);
}

int g_nLastLen = 0;
int g_nTimerHandle = -1;
char g_cLastChar;

void CTexWnd::UpdateFilter(const char* pFilter)
{
  if (g_nTimerHandle > 0)
    KillTimer(1);
  g_bFilterEnabled = false;
  if (pFilter)
  {
    g_strFilter = pFilter;
    if (g_strFilter.GetLength() > 0)
    {
      g_bFilterEnabled = true;
      if (g_pParentWnd->CurrentStyle() == QR_QE4 || g_pParentWnd->CurrentStyle() == QR_4WAY)
      {
        if (g_strFilter.GetLength() > g_nLastLen)
        {
          g_cLastChar = toupper(g_strFilter.GetAt(g_strFilter.GetLength()-1));
          if (g_cLastChar == 'N' || g_cLastChar == 'O') // one of the other popups
          {
            g_nTimerHandle = SetTimer(1, 800, NULL);   // half second timer
          }
        }
      }
    }
    g_nLastLen = g_strFilter.GetLength();
	  SortTextures();
  }
  Sys_UpdateWindows (W_TEXTURE);
}

void CTexWnd::UpdatePrefs()
{
  if (!g_PrefsDlg.m_bTextureWindow)
  {
    m_wndFilter.ShowWindow(SW_HIDE);
    g_nTextureOffset = 0;
  }
  else
  {
    m_wndFilter.ShowWindow(SW_SHOW);
    g_nTextureOffset = 25;
  }
  ShowScrollBar(SB_VERT, g_PrefsDlg.m_bTextureScrollbar);
  m_bNeedRange = true;
  Invalidate();
  UpdateWindow();
}

void CTexWnd::FocusEdit()
{
  if (m_wndFilter.IsWindowVisible())
    m_wndFilter.SetFocus();
}

void CTexWnd::OnTimer(UINT nIDEvent) 
{
  KillTimer(1);
  g_nLastLen = 0;
  g_nTimerHandle = -1;
  ::SetFocus(g_qeglobals.d_hwndEntity);
  ::PostMessage(g_qeglobals.d_hwndEntity, WM_CHAR, g_cLastChar, 0);
}

void CTexWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
	//CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTexWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);
}

void CTexWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
  CRect rctClient;
  GetClientRect(rctClient);
  int nOld = g_qeglobals.d_texturewin.m_nTotalHeight;
  if (!wglMakeCurrent(s_hdcTexture, s_hglrcTexture))
  //if ( !qwglMakeCurrent(dc.m_hDC, s_hglrcTexture ) )
  {
    Sys_Printf("ERROR: wglMakeCurrent failed..\n ");
    Sys_Printf("Please restart Q3Radiant if the Texture view is not working\n");
  }
  else
  {
    Texture_Draw2 (rctClient.right-rctClient.left, rctClient.bottom-rctClient.top - g_nTextureOffset);
//		wgl
	SwapBuffers(s_hdcTexture);
    TRACE("Texture Paint\n");
  }
  if (g_PrefsDlg.m_bTextureScrollbar && (m_bNeedRange || g_qeglobals.d_texturewin.m_nTotalHeight != nOld))
  {
    m_bNeedRange = false;
    SetScrollRange(SB_VERT, 0, g_qeglobals.d_texturewin.m_nTotalHeight, TRUE);
  }
}

void CTexWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);

  int n = GetScrollPos(SB_VERT);;
  switch (nSBCode)
  {
    case SB_LINEUP :
    {
      n = (n - 15 >  0) ? n - 15 : 0; 
      break;
    }
    case SB_LINEDOWN :
    {
      n = (n + 15 < g_qeglobals.d_texturewin.m_nTotalHeight) ? n + 15 : n; 
      break;
    }
    case SB_PAGEUP :
    {
      n = (n - g_qeglobals.d_texturewin.height >  0) ? n - g_qeglobals.d_texturewin.height : 0; 
      break;
    }
    case SB_PAGEDOWN :
    {
      n = (n + g_qeglobals.d_texturewin.height < g_qeglobals.d_texturewin.m_nTotalHeight) ? n + g_qeglobals.d_texturewin.height : n; 
      break;
    }
    case SB_THUMBPOSITION :
    {
      n = nPos;
      break;
    }
    case SB_THUMBTRACK :
    {
      n = nPos;
      break;
    }
  }
  SetScrollPos(SB_VERT, n);
	g_qeglobals.d_texturewin.originy = -((int)n);
  Invalidate();
  UpdateWindow();
  //Sys_UpdateWindows(W_TEXTURE);
}

/*
and are the caps new caps?  anything done with older stuff will be fubar'd.. which brings up the point if you ever naturalize a cap, you cannot force it back to cap texturing.. i will add that too
*/

void CTexWnd::OnTexturesFlush() 
{
	// TODO: Add your command handler code here
	
}
