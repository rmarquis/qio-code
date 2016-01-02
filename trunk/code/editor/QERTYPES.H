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
// qertypes.h
//
// common types
// merged from brush.h, etc. for plugin support
//
#ifndef _QERTYPE_H
#define _QERTYPE_H

typedef float vec3_t[3];

#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/textureAPI.h>
#include <shared/str.h>


#include <shared/brush.h>

class texturewin_t
{
public:
  texturewin_t()
  {
  }
  ~texturewin_t()
  {
  }
	int			width, height;
	int			originy;
	// add brushprimit_texdef_s for brush primitive coordinates storage
	brushprimit_texdef_s	brushprimit_texdef;
	int m_nTotalHeight;
	texdef_t	texdef;


	const char *getMatName() const {
		return texdef.getName();
	}
};

#define QER_TRANS     0x00000001
#define QER_NOCARVE   0x00000002


#define	MIN_PATCH_WIDTH		3
#define	MIN_PATCH_HEIGHT 	3

#define	MAX_PATCH_WIDTH		16
#define	MAX_PATCH_HEIGHT	16

// patch type info
// type in lower 16 bits, flags in upper
// endcaps directly follow this patch in the list

// types
#define PATCH_GENERIC     0x00000000    // generic flat patch
#define PATCH_CYLINDER    0x00000001    // cylinder
#define PATCH_BEVEL       0x00000002    // bevel
#define PATCH_ENDCAP      0x00000004    // endcap
#define PATCH_HEMISPHERE  0x00000008    // hemisphere
#define PATCH_CONE        0x00000010    // cone
#define PATCH_TRIANGLE    0x00000020    // simple tri, assumes 3x3 patch

// behaviour styles
#define PATCH_CAP         0x00001000    // flat patch applied as a cap
#define PATCH_SEAM        0x00002000    // flat patch applied as a seam
#define PATCH_THICK       0x00004000    // patch applied as a thick portion

// styles
#define PATCH_BEZIER      0x00000000    // default bezier
#define PATCH_BSPLINE     0x10000000    // bspline

#define PATCH_TYPEMASK     0x00000fff    // 
#define PATCH_BTYPEMASK    0x0000f000    // 
#define PATCH_STYLEMASK    0xffff0000    // 

#include <renderer/rVertex.h>

class patchMesh_c {
	bool bSelected;


	float calcPatchWidth();
	float calcPatchWidthDistanceTo(int j);
	float calcPatchHeight();
	float calcPatchHeightDistanceTo(int j);
public:
	int	width, height;		// in control points, not patches
	int   contents, flags, value, type;
	mtrAPI_i *d_texture;
	class brush_s *pSymbiot;
	bool bOverlay;
	bool bDirty;
	int  nListID;
	rVert_c ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];

	patchMesh_c() {
		width = height = 0;
		contents = flags = value = type = 0;
		d_texture = 0;
		pSymbiot = 0;
		bSelected = false;
		bOverlay = false;
		bDirty = false;
		nListID = 0;
	}
	patchMesh_c(const patchMesh_c *o) {
		width = o->width;
		height = o->height;
		contents = o->contents;
		d_texture = o->d_texture;
		pSymbiot = o->pSymbiot;
		flags = o->flags;
		value = o->value;
		type = o->type;
		memcpy(ctrl,o->ctrl,sizeof(ctrl));
		this->bSelected = false;
		this->bDirty = true;
		this->bOverlay = false;
		this->nListID = -1;
	}
	void setSelected(bool b) {
		bSelected = b;
	}
	void fillPatch(vec3_t v);
	void naturalizePatch();
	void invertPatch();
	void meshNormals();
	void interpolateInteriorPoints();
	void rebuildPatch();
	void adjustPatchRows(int nRows);
	void adjustPatchColumns(int nCols);
	void movePatch(const vec3_t vMove, bool bRebuild = false);
	void calcPatchBounds(vec3_c& vMin, vec3_c& vMax);
	void drawPatchMesh(bool bPoints, bool bShade = false);
	void drawPatchXY();
	void removePatchColumn(bool bFirst);
	void drawPatchCam();
	void capTexture(bool bFaceCycle = false);
};

// allocated counter is only used for debugging
extern int g_allocatedCounter_brush;

class brush_s : public brush_c
{
	bool bIsLinkedListHeader;
public:
	class brush_s	*prev, *next;	// links in active/selected
	class brush_s	*oprev, *onext;	// links in entity
	class entity_s	*owner;

	//
	// curve brush extensions
	// all are derived from brush_faces
	bool	patchBrush;
	bool	hiddenBrush;
	bool	terrainBrush;
  
	patchMesh_c *pPatch;

	class entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	brush_s(bool bIsLinkedListHeader = true) {
		undoId = 0;
		redoId = 0;
		ownerId = 0;
		pUndoOwner = 0;
		pPatch = 0;
		prev = 0;
		next = 0;
		oprev = 0;
		onext = 0;
		owner = 0;
		patchBrush = 0;
		hiddenBrush = 0;
		terrainBrush = 0;
		this->bIsLinkedListHeader = bIsLinkedListHeader;
		// allocated counter is only used for debugging
		if(bIsLinkedListHeader==false) 
			g_allocatedCounter_brush++;
	}
	~brush_s() {
		// allocated counter is only used for debugging
		if(bIsLinkedListHeader==false) 
			g_allocatedCounter_brush--;
	}
	friend brush_s *Brush_Parse (class parser_c &p);
	void rotateBrush(vec3_t vAngle, vec3_t vOrigin, bool bBuild = true);
	brush_s *fullClone();
	void buildWindings(bool bSnap = true);
	void rebuildBrush(vec3_t vMins, vec3_t vMaxs);
	void parseBrushPrimit(class parser_c &p);
	void splitBrushByFace (face_s *f, brush_s **front, brush_s **back);
	brush_s *tryMergeWith(brush_s *brush2, int onlyshape);
	int moveVertex(const vec3_c &vertex, const vec3_c &delta, vec3_c &end, bool bSnap = true);

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
};


/*
** window bits
*/
#define	W_CAMERA		  0x0001
#define	W_XY			    0x0002
#define	W_XY_OVERLAY	0x0004
#define	W_Z				    0x0008
#define	W_TEXTURE		  0x0010
#define	W_Z_OVERLAY		0x0020
#define W_CONSOLE		  0x0040
#define W_ENTITY		  0x0080
#define W_CAMERA_IFON 0x0100
#define W_XZ          0x0200  //--| only used for patch vertex manip stuff
#define W_YZ          0x0400  //--|
#define W_GROUP       0x0800 
#define W_MEDIA       0x1000 
#define	W_ALL			0xFFFFFFFF

// used in some Drawing routines
enum VIEWTYPE {YZ, XZ, XY};

#endif