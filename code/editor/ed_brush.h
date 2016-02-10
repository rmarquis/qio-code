/*
============================================================================
Copyright (C) 2015 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// ed_brush.h
#ifndef __EDITOR_BRUSH_H__
#define __EDITOR_BRUSH_H__

#include <shared/brush.h>
#include <api/rStaticModelAPI.h>

#define MAX_WORLD_COORD		( 128*1024 )
#define MIN_WORLD_COORD		( -128*1024 )
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )
#define MAX_BRUSH_SIZE		( WORLD_SIZE )

// allocated counter is only used for debugging
extern int g_allocatedCounter_brush;

class edBrush_c : public brush_c
{
	bool bIsLinkedListHeader;
	class rStaticModelAPI_i *rData;
public:
	class edBrush_c	*prev, *next;	// links in active/selected
	class edBrush_c	*oprev, *onext;	// links in entity
	class entity_s	*owner;

	bool hasWindingsGenerationFailedFlag() const {
		return bWindingsGenerationFailed;
	}
	//
	// curve brush extensions
	// all are derived from brush_faces
	bool	patchBrush;
	bool	hiddenBrush;
	bool	terrainBrush;
  
	class patchMesh_c *pPatch;

	class entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	edBrush_c(bool bIsLinkedListHeader = true);
	~edBrush_c();
	void freeBrushRenderData();

	void onBrushSelectedStateChanged(bool newBIsSelected);
	friend edBrush_c *Brush_Parse (class parser_c &p);
	void rotateBrush(vec3_t vAngle, vec3_t vOrigin, bool bBuild = true);
	edBrush_c *fullClone();
	void rebuildRendererStaticModelData();
	void buildWindings(bool bSnap = true);
	void rebuildBrush(vec3_t vMins, vec3_t vMaxs);
	void parseBrushPrimit(class parser_c &p);
	void splitBrushByFace (face_s *f, edBrush_c **front, edBrush_c **back);
	edBrush_c *tryMergeWith(edBrush_c *brush2, int onlyshape);
	int moveVertex(const vec3_c &vertex, const vec3_c &delta, vec3_c &end, bool bSnap = true);

	bool isLinkedListHeader() const {
		return bIsLinkedListHeader;
	}
	void setFaces(face_s *p) {
		brush_faces = p;
	}
	const aabb &getBounds() const {
		return bounds;
	}
	const vec3_c &getMins() const {
		return bounds.getMins();
	}
	const vec3_c &getMaxs() const {
		return bounds.getMaxs();
	}

friend void		Brush_AddToList (edBrush_c *b, edBrush_c *list);
friend void		Brush_Build(edBrush_c *b, bool bSnap = true, bool bMarkMap = true, bool bConvert = false);
friend edBrush_c*	Brush_Clone (edBrush_c *b);
friend edBrush_c*	Brush_Create (vec3_t mins, vec3_t maxs, texdef_t *texdef);
friend void		Brush_Draw( edBrush_c *b, bool bIsSelected );
friend void		Brush_DrawXY(edBrush_c *b, int nViewType);
// set bRemoveNode to false to avoid trying to delete the item in group view tree control
friend void		Brush_Free (edBrush_c *b, bool bFreeOwnerEntityRModel = true);
friend int			Brush_MemorySize(edBrush_c *b);
friend void		Brush_MakeSided (int sides);
friend void		Brush_MakeSidedCone (int sides);
friend void		Brush_Move (edBrush_c *b, const vec3_t move, bool bSnap = true);
friend void		Brush_ResetFaceOriginals(edBrush_c *b);
friend edBrush_c*	Brush_Parse (class parser_c &p);
friend face_s*		Brush_Ray (const vec3_c &origin, const vec3_c &dir, edBrush_c *b, float *dist);
friend void		Brush_RemoveFromList (edBrush_c *b);
friend void		Brush_SelectFaceForDragging (edBrush_c *b, face_s *f, bool shear);
friend void		Brush_SetTexture (edBrush_c *b, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale = false);
friend void		Brush_SideSelect (edBrush_c *b, vec3_t origin, vec3_t dir, bool shear);
friend void		Brush_SnapToGrid(edBrush_c *pb);
friend void		Brush_MakeSidedSphere(int sides);
friend void		Brush_Write (edBrush_c *b, FILE *f);
friend void		Brush_Write (edBrush_c *b, class CMemFile* pMemFile);

friend int			AddPlanept (float *f);
friend float		SetShadeForPlane (const class plane_c &p);

friend void		SetFaceTexdef (edBrush_c *b, face_s *f, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale = false);
;
friend const char* Brush_GetKeyValue(edBrush_c *b, const char *pKey);
};



#endif // __EDITOR_BRUSH_H__

