/*
============================================================================
Copyright (C) 2012 V.

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
// bspPhysicsDataLoader.cpp
#include <qcommon/q_shared.h>
#include "bspPhysicsDataLoader.h"
#include <api/vfsAPI.h>
#include <fileFormats/bspFileFormat.h>
#include <shared/cmSurface.h>
#include <shared/cmBezierPatch.h>
#include <shared/bspPhysicsDataLoader.h>

bspPhysicsDataLoader_c::bspPhysicsDataLoader_c() {
	h = 0;
}
bspPhysicsDataLoader_c::~bspPhysicsDataLoader_c() {
	clear();
}
void bspPhysicsDataLoader_c::clear() {
	if(h) {
		free(h);
		h = 0;
	}
}
bool bspPhysicsDataLoader_c::loadBSPFile(const char *fname) {
	char buf[256];
	strcpy(buf,"maps/");
	strcat(buf,fname);
	strcat(buf,".bsp");
	fileHandle_t f;
	int len = g_vfs->FS_FOpenFile(buf,&f,FS_READ);
	if(len < 0) {
		return true; // error
	}
	byte *data = (byte*)malloc(len);
	g_vfs->FS_Read(data,len,f);
	g_vfs->FS_FCloseFile(f);
	h = (q3Header_s*)data;
	return false;
}

void bspPhysicsDataLoader_c::iterateModelBrushes(u32 modelNum, void (*perBrushCallback)(u32 brushNum, u32 matNum)) {
	const q3Model_s *mod = h->getModels() + modelNum;
	const q3Brush_s *b = h->getBrushes() + mod->firstBrush;
	for(u32 i = 0; i < mod->numBrushes; i++, b++) {
		perBrushCallback(mod->firstBrush+i, getMaterialContentFlags(b->materialNum));
	}
}
void bspPhysicsDataLoader_c::iterateBrushPlanes(u32 brushNum, void (*sideCallback)(const float planeEq[4])) {
	const q3Brush_s *b = h->getBrushes() + brushNum;
	const q3Plane_s *planes = h->getPlanes();
	for(u32 i = 0; i < b->numSides; i++) {
		const q3BrushSide_s *s = h->getBrushSide(b->firstSide+i);
		const q3Plane_s &plane = planes[s->planeNum];
		sideCallback((const float*)&plane);
	}
}
void bspPhysicsDataLoader_c::iterateModelBezierPatches(u32 modelNum, void (*perBezierPatchCallback)(u32 surfNum, u32 matNum)) {
	const q3Model_s *mod = h->getModels() + modelNum;
	const q3Surface_s *sf = h->getSurfaces() + mod->firstSurface;
	for(u32 i = 0; i < mod->numSurfaces; i++) {
		if(sf->surfaceType == Q3MST_PATCH) {
			perBezierPatchCallback(mod->firstSurface+i, getMaterialContentFlags(sf->materialNum));
		}
		sf = h->getNextSurface(sf);
	}
}
void bspPhysicsDataLoader_c::convertBezierPatchToTriSurface(u32 surfNum, u32 tesselationLevel, class cmSurface_c &out) {
	const q3Surface_s *sf = h->getSurface(surfNum);
	// convert bsp bezier surface to cmBezierPatch_c
	cmBezierPatch_c bp;
	const q3Vert_s *v = h->getVerts() + sf->firstVert;
	for(u32 j = 0; j < sf->numVerts; j++, v++) {
		bp.addVertex(v->xyz);
	}
	bp.setHeight(sf->patchHeight);
	bp.setWidth(sf->patchWidth);
	// convert cmBezierPatch_c to cmSurface_c
	bp.tesselate(tesselationLevel,&out);
}

u32 bspPhysicsDataLoader_c::getMaterialContentFlags(u32 matNum) const {
	return h->getMat(matNum)->contentFlags;
}

u32 bspPhysicsDataLoader_c::getNumInlineModels() const {
	return h->getNumModels();
}
void bspPhysicsDataLoader_c::getInlineModelBounds(u32 modelNum, class aabb &bb) const {
	const q3Model_s *m = h->getModels() + modelNum;
	bb.fromTwoPoints(m->maxs,m->mins);
}

