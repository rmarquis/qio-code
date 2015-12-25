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

// a texturename of the form (0 0 0) will
// create a solid color texture

void	Texture_Init (bool bHardInit = true);
void	Texture_FlushUnused ();
void	Texture_Flush (bool bReload = false);
void	Texture_ClearInuse (void);
void	Texture_ShowInuse (void);
void	Texture_ShowDirectory (int menunum, bool bLinked = false);
void	Texture_ShowAll();
void Texture_Cleanup(CStringList *pList = NULL);

mtrAPI_i * WINAPI QERApp_TryTextureForName(const char* name);
void	Texture_Init (void);
void	Texture_SetTexture (texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale = false, bool bSetSelection = true);

void	Texture_SetMode(int iMenu);	// GL_TEXTURE_NEAREST, etc..
void Texture_ResetPosition();

int  WINAPI Texture_LoadSkin(char *pName, int *pnWidth, int *pnHeight);
void Texture_StartPos (void);
mtrAPI_i *Texture_NextPos (int *x, int *y);
