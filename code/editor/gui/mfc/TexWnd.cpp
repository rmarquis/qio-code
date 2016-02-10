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

#include <stdafx.h>
#include <assert.h>
#include "Radiant.h"
#include "TexWnd.h"
#include "qe3.h"
#include "io.h"
#include "PrefsDlg.h"
#include "PrefsDlg.h"
#include <windows.h>
#include <gl/glu.h>
#include <vector>
#include <api/rAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/materialSystemAPI.h>
#include <api/textureAPI.h>
#include <api/vfsAPI.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

mtrAPI_i	*g_pluginTexture = NULL;

static bool	nomips = false;

#define	FONT_HEIGHT	10

HGLRC s_hglrcTexture = NULL;
HDC	 s_hdcTexture = NULL;

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

// void SelectTexture (int mx, int my, bool bShift = false);
void SelectTexture (int mx, int my, bool bShift, bool bFitScale=false);

void	Texture_MouseDown (int x, int y, int buttons);
void	Texture_MouseUp (int x, int y, int buttons);
void	Texture_MouseMoved (int x, int y, int buttons);

void ReplaceQTexture(mtrAPI_i *pOld, mtrAPI_i *pNew, edBrush_c *pList)
{ 
	for (edBrush_c* pBrush = pList->next ; pBrush != pList; pBrush = pBrush->next)
	{
		if (pBrush->patchBrush)
		{
			Patch_ReplaceQTexture(pBrush, pOld, pNew);
		}

		for (face_s* pFace = pBrush->getFirstFace(); pFace; pFace = pFace->next)
		{
			if (pFace->d_texture == pOld)
			{
			pFace->d_texture = pNew;
			}
		}
		
		//Brush_Build(pBrush);
	}
}


/*
==================
FillTextureMenu

==================
*/
void FillTextureMenu (CStringArray* pArray)
{
	HMENU	hmenu;
	int		i;
	struct _finddata_t fileinfo;
	int		handle;
	char	dirstring[1024];
	char	*path;


	hmenu = GetSubMenu (GetMenu(g_qeglobals.d_hwndMain), MENU_TEXTURE);

	// delete everything
	for (i=0 ; i<texture_nummenus ; i++)
		DeleteMenu (hmenu, CMD_TEXTUREWAD+i, MF_BYCOMMAND);

	HMENU hSubmenuFiles = CreatePopupMenu();
	AppendMenu(hmenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubmenuFiles, "Browse material (.mtr/.script) files: ");

	u32 matFiles = g_ms->getNumCachedMaterialFiles();
	texture_nummenus = 0;
	for(u32 i = 0; i < matFiles; i++) {
		const char *test = g_ms->getMaterialFileName(i);
		AppendMenu (hSubmenuFiles, MF_ENABLED|MF_STRING, CMD_TEXTUREWAD+texture_nummenus, (LPCTSTR)test);
		strcpy (texture_menunames[texture_nummenus], test);
		texture_nummenus++;
	}
	HMENU hSubmenuKeywords = CreatePopupMenu();
	AppendMenu(hmenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubmenuKeywords, "Browse materials by keyword: ");
	HMENU hSubmenuDirs = CreatePopupMenu();
	AppendMenu(hmenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubmenuKeywords, "Browse materials by directory: ");

}


/*
==================
Texture_ClearInuse

A new map is being loaded, so clear inuse markers
==================
*/
void Texture_ClearInuse ()
{
}


void ED_PrepareMaterialsLoading();

/*
==============
Texture_ShowDirectory
==============
*/
void	Texture_ShowDirectory (int menunum, bool bLinked)
{
	ED_PrepareMaterialsLoading();
	const char *s = texture_menunames[menunum-CMD_TEXTUREWAD];
	g_ms->cacheAllMaterialsFromMatFile(s);
}


// this can be combined with the above, but per usual i am in a hurry
//
void	Texture_ShowDirectory (char* pPath, bool bLinked)
{

}



void Texture_ResetPosition()
{
  SelectTexture (16, g_qeglobals.d_texturewin.height -16 ,false);
  g_qeglobals.d_texturewin.originy = 0;
}


void Texture_SetInuse ()
{
}


void	Texture_ShowAll()
{
  Texture_SetInuse();
	Sys_Printf("Showing all textures...\n");
	Sys_UpdateWindows (W_TEXTURE);
}


void	Texture_ShowInuse ()
{
	face_s	*f;
	edBrush_c	*b;
	char	name[1024];

	texture_showinuse = true;

	g_qeglobals.d_texturewin.originy = 0;	

	Texture_ClearInuse();
	g_ms->clearMaterialInUseFlags();
	Sys_Status("Selecting active textures\n", 0);

	for (b=active_brushes.next ; b != NULL && b != &active_brushes ; b=b->next)
	{
		if (b->patchBrush)
		{
			b->pPatch->d_texture->markAsUsed();
		}
		else
		{
			for (f=b->getFirstFace() ; f ; f=f->next)
			{
				f->d_texture->markAsUsed();
			}
		}
	}

	for (b=selected_brushes.next ; b != NULL && b != &selected_brushes ; b=b->next)
	{
		if (b->patchBrush)
		{
			b->pPatch->d_texture->markAsUsed();
		}
		else
		{
			for (f=b->getFirstFace() ; f ; f=f->next)
			{
				f->d_texture->markAsUsed();
			}
		}
	}
	g_ms->freeUnusedMaterials();

	//SetInspectorMode(W_TEXTURE);
	Sys_UpdateWindows (W_TEXTURE);

	sprintf (name, "Textures: in use");
	SetWindowText(g_qeglobals.d_hwndEntity, name);

	// select the first texture in the list
	if (!g_qeglobals.d_texturewin.getMatName()[0])
	{
		SelectTexture (16, g_qeglobals.d_texturewin.height -16, false);
	}
}

/*
============================================================================

TEXTURE LAYOUT

============================================================================
*/

void Texture_StartPos ()
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

	if (texdef->getName()[0] == '(')
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

  g_dlgFind.updateTextures(texdef->getName());

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
		if (!_strcmpi(texdef->getName(), q->getName()))
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
			tex.setName(q->getName());
			Texture_SetTexture ( &tex, &brushprimit_tex, bFitScale, 1);
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
			else if(q->getFirstEditorImageStage() && q->getFirstEditorImageStage()->getTexture(0))
				glBindTexture( GL_TEXTURE_2D, q->getFirstEditorImageStage()->getTexture(0)->getInternalHandleU32());
			else
				printf("missing texture colormap\n");
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
			if (!_strcmpi(g_qeglobals.d_texturewin.getMatName(), q->getName()))
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

void Texture_Cleanup()
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

void Texture_Flush (bool bReload)
{
  if (!ConfirmModified())
    return;

  Map_New ();

  CWaitCursor cursor;
  Texture_Cleanup();

 /* GLuint* pGln = new GLuint[texture_extension_number-1];
  glGenTextures(texture_extension_number-1, pGln);
  QE_CheckOpenGLForErrors();
  glDeleteTextures(texture_extension_number-1, pGln);
  QE_CheckOpenGLForErrors();
  delete []pGln;*/
 // texture_extension_number = 1
	//glGenTextures(1,& q->texture_number);
	//textureNumbers.push_back(q->texture_number);


///	g_qeglobals.d_qtextures = NULL;

  if (bReload)
  {
    //POSITION pos = strList.GetHeadPosition();
    //while (pos)
    //{
    //  CString strTex = strList.GetNext(pos);
		  //QERApp_TryTextureForName (strTex.GetBuffer(0));
    //}
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
