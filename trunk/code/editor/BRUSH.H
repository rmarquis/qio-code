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

// brush.h

void		Brush_AddToList (brush_s *b, brush_s *list);
void		Brush_Build(brush_s *b, bool bSnap = true, bool bMarkMap = true, bool bConvert = false);
void		Brush_BuildWindings( brush_s *b, bool bSnap = true );
brush_s*	Brush_Clone (brush_s *b);
brush_s*	Brush_FullClone(brush_s *b);
brush_s*	Brush_Create (vec3_t mins, vec3_t maxs, texdef_t *texdef);
void		Brush_Draw( brush_s *b );
void		Brush_DrawXY(brush_s *b, int nViewType);
// set bRemoveNode to false to avoid trying to delete the item in group view tree control
void		Brush_Free (brush_s *b, bool bRemoveNode = true);
int			Brush_MemorySize(brush_s *b);
void		Brush_MakeSided (int sides);
void		Brush_MakeSidedCone (int sides);
void		Brush_Move (brush_s *b, const vec3_t move, bool bSnap = true);
int			Brush_MoveVertex(brush_s *b, const vec3_c &vertex, const vec3_c &delta, vec3_c &end, bool bSnap = true);
void		Brush_ResetFaceOriginals(brush_s *b);
brush_s*	Brush_Parse (void);
face_s*		Brush_Ray (const vec3_c &origin, const vec3_c &dir, brush_s *b, float *dist);
void		Brush_RemoveFromList (brush_s *b);
void		Brush_SplitBrushByFace (brush_s *in, face_s *f, brush_s **front, brush_s **back);
void		Brush_SelectFaceForDragging (brush_s *b, face_s *f, bool shear);
void		Brush_SetTexture (brush_s *b, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale = false);
void		Brush_SideSelect (brush_s *b, vec3_t origin, vec3_t dir, bool shear);
void		Brush_SnapToGrid(brush_s *pb);
void		Brush_Rotate(brush_s *b, vec3_t vAngle, vec3_t vOrigin, bool bBuild = true);
void		Brush_MakeSidedSphere(int sides);
void		Brush_Write (brush_s *b, FILE *f);
void		Brush_Write (brush_s *b, CMemFile* pMemFile);
void		Brush_RemoveEmptyFaces ( brush_s *b );
winding_t*	Brush_MakeFaceWinding (brush_s *b, face_s *face);

int			AddPlanept (float *f);
float		SetShadeForPlane (const class edPlane_c &p);

void		Face_Draw( face_s *face );
void		Face_TextureVectors (face_s *f, float STfromXYZ[2][4]);
void		SetFaceTexdef (brush_s *b, face_s *f, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale = false);

void Brush_FitTexture( brush_s *b, int nHeight, int nWidth );
const char* Brush_GetKeyValue(brush_s *b, const char *pKey);