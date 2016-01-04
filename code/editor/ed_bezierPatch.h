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
// ed_bezierPatch.h
#ifndef __EDITOR_BEZIERPATCH_H__
#define __EDITOR_BEZIERPATCH_H__

#include <renderer/rVertex.h>

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

class patchMesh_c {
	bool bSelected;


	float calcPatchWidth();
	float calcPatchWidthDistanceTo(int j);
	float calcPatchHeight();
	float calcPatchHeightDistanceTo(int j);
public:
	int	width, height;		// in control points, not patches
	int   contents, flags, value, type;
	class mtrAPI_i *d_texture;
	class edBrush_c *pSymbiot;
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


#endif // __EDITOR_BEZIERPATCH_H__

