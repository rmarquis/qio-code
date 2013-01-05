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
#include <shared/perPlaneCallback.h>

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
	if(h->isBSPCoD1()) {
		h->swapCoDLumpLenOfsValues();
	}
	return false;
}
void bspPhysicsDataLoader_c::nodeBrushes_r(int nodeNum, arraySTD_c<u32> &out) const {
	const q2Node_s *nodes = (const q2Node_s *)h->getLumpData(Q2_NODES);
	while(nodeNum >= 0) {
		const q2Node_s &node = nodes[nodeNum];
		nodeNum = node.children[0];
		nodeBrushes_r(node.children[1],out);
	}
	const u16 *leafBrushNums = (const u16*)h->getLumpData(Q2_LEAFBRUSHES);
	const q2Leaf_s *l = (const q2Leaf_s *)h->getLumpData(Q2_LEAFS)+(-nodeNum+1);
	for(u32 i = 0; i < l->numLeafBrushes; i++) {
		u32 brushNum = leafBrushNums[l->firstLeafBrush+i];
		out.add_unique(brushNum);
	}
}
void bspPhysicsDataLoader_c::iterateModelBrushes(u32 modelNum, void (*perBrushCallback)(u32 brushNum, u32 contentFlags)) {
	if(h->isBSPQ2()) {
		const q2Model_s *q2Mod = (const q2Model_s *)h->getLumpData(Q2_MODELS);
		arraySTD_c<u32> brushes;
		nodeBrushes_r(q2Mod->headnode,brushes);
		for(u32 i = 0; i < brushes.size(); i++) {
			u32 brushNum = brushes[i];
			const q2Brush_s *q2Brush = (const q2Brush_s *)h->getLumpData(Q2_BRUSHES)+brushNum;
			perBrushCallback(brushNum,q2Brush->contents);
		}
		return;
	}

	const q3Model_s *mod = h->getModel(modelNum);
	if(h->isBSPCoD1()) {
		const cod1Brush_s *codBrush = (const cod1Brush_s *)h->getLumpData(COD1_BRUSHES);
		for(u32 i = 0; i < mod->numBrushes; i++, codBrush++) {
			perBrushCallback(mod->firstBrush+i, getMaterialContentFlags(codBrush->materialNum));
		}	
	} else {
		const q3Brush_s *b = h->getBrushes() + mod->firstBrush;
		for(u32 i = 0; i < mod->numBrushes; i++, b++) {
			perBrushCallback(mod->firstBrush+i, getMaterialContentFlags(b->materialNum));
		}
	}
}
void bspPhysicsDataLoader_c::iterateModelTriSurfs(u32 modelNum, void (*perSurfCallback)(u32 surfNum, u32 contentFlags)) {
	const q3Model_s *mod = h->getModel(modelNum);
	if(h->isBSPCoD1()) {
		const cod1Surface_s *codSF = h->getCoD1Surfaces() + mod->firstSurface;
		for(u32 i = 0; i < mod->numSurfaces; i++, codSF++) {
			perSurfCallback(mod->firstBrush+i, getMaterialContentFlags(codSF->materialNum));
		}		
	} else {
		const q3Surface_s *sf = h->getSurface(mod->firstSurface);
		for(u32 i = 0; i < mod->numSurfaces; i++) {
			perSurfCallback(mod->firstBrush+i, getMaterialContentFlags(sf->materialNum));
			sf = h->getNextSurface(sf);
		}
	}
}
struct codBrushSideAxial_s {
	float distance; // axial plane distance to origin
	int material;
};
struct codBrushSideNonAxial_s {
	int planeNumber;
	int material;
};
struct codBrushSides_s {
	codBrushSideAxial_s axialSides[6]; // always 6
	codBrushSideNonAxial_s nonAxialSides[32]; // variable-sized
};
void bspPhysicsDataLoader_c::iterateBrushPlanes(u32 brushNum, void (*sideCallback)(const float planeEq[4])) {
	if(h->isBSPQ2()) {
		const q2Plane_s *q2Planes = (const q2Plane_s *)h->getLumpData(Q2_PLANES);
		const q2Brush_s *q2Brush = (const q2Brush_s *)h->getLumpData(Q2_BRUSHES)+brushNum;
		const q2BrushSide_s *q2Bs = (const q2BrushSide_s *)h->getLumpData(Q2_BRUSHSIDES)+q2Brush->firstside;
		for(u32 i = 0; i < q2Brush->numsides; i++, q2Bs++) {
			const q2Plane_s &sidePlane = q2Planes[q2Bs->planenum];
			sideCallback(sidePlane.normal);
		}
		return;
	}
	const q3Plane_s *planes = h->getPlanes();
	if(h->isBSPCoD1()) {
		const cod1Brush_s *codBrush = (const cod1Brush_s *)h->getLumpData(COD1_BRUSHES);
		u32 sidesDataOffset = 0;
		// get the side offset
		u32 tmp = brushNum;
		while(tmp) {
			sidesDataOffset += codBrush->numSides * 8;
			codBrush++;
			tmp--;
		}
		u32 totalSidesDataLen = h->getLumpSize(COD1_BRUSHSIDES);
		if(totalSidesDataLen <= sidesDataOffset) {
			return;
		}
		const codBrushSides_s *sides = (const codBrushSides_s*)(((const byte*)h->getLumpData(COD1_BRUSHSIDES))+sidesDataOffset);
		// that's the order of normals from q3 bsp file...
#if 1
		vec3_c normals [] = {
			vec3_c(-1,0,0),
			vec3_c(1,0,0),
			vec3_c(0,-1,0),
			vec3_c(0,1,0),
			vec3_c(0,0,-1),
			vec3_c(0,0,1),
		};
#else
		// TODO: find the valid order of normals
#endif
		for(u32 i = 0; i < 6; i++) {
			plane_c pl;
			pl.norm = normals[i];
			pl.dist = sides->axialSides[i].distance;
			sideCallback(pl.norm);
		}
		u32 extraSides = codBrush->numSides - 6;
		for(u32 i = 0; i < extraSides; i++) {
			const q3Plane_s &plane = planes[sides->nonAxialSides[i].planeNumber];
			sideCallback((const float*)&plane);
		}
	} else {
		const q3Brush_s *b = h->getBrushes() + brushNum;
		for(u32 i = 0; i < b->numSides; i++) {
			const q3BrushSide_s *s = h->getBrushSide(b->firstSide+i);
			const q3Plane_s &plane = planes[s->planeNum];
			sideCallback((const float*)&plane);
		}
	}
}
void bspPhysicsDataLoader_c::iterateBrushPlanes(u32 brushNum, class perPlaneCallbackListener_i *callBack) {
	const q3Plane_s *planes = h->getPlanes();
	if(h->isBSPCoD1()) {	
		// TODO
	} else {
		const q3Brush_s *b = h->getBrushes() + brushNum;
		for(u32 i = 0; i < b->numSides; i++) {
			const q3BrushSide_s *s = h->getBrushSide(b->firstSide+i);
			const q3Plane_s &plane = planes[s->planeNum];
			callBack->perPlaneCallback((const float*)&plane);
		}
	}
}
void bspPhysicsDataLoader_c::iterateModelBezierPatches(u32 modelNum, void (*perBezierPatchCallback)(u32 surfNum, u32 matNum)) {
	if(h->isBSPQ2()) {
		// no bezier patches in Q2?
		return;
	}
	if(h->isBSPCoD1()) {
		// it seems that there are no bezier patches in CoD bsp files...
		return;
	}
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
void bspPhysicsDataLoader_c::getTriangleSurface(u32 surfNum, class cmSurface_c &out) {
	if(h->isBSPCoD1()) {
		const cod1Surface_s *codSF = h->getCoD1Surfaces()+surfNum;
		const q3Vert_s *v = h->getVerts() + codSF->firstVert;
		const u16 *codIndices = (const u16*)h->getLumpData(COD1_DRAWINDEXES);
		out.setNumVerts(codSF->numVerts);
		for(u32 i = 0; i < codSF->numVerts; i++, v++) {
			out.setVertex(i,v->xyz);
		}
		out.setNumIndices(codSF->numIndexes);
		for(u32 i = 0; i < codSF->numIndexes; i++) {
			u16 val = codIndices[codSF->firstIndex+i];
			out.setIndex(i,val);
		}
	} else {

	}
}
u32 bspPhysicsDataLoader_c::getMaterialContentFlags(u32 matNum) const {
	const q3BSPMaterial_s *m = h->getMat(matNum);
	return m->contentFlags;
}

u32 bspPhysicsDataLoader_c::getNumInlineModels() const {
	return h->getNumModels();
}
void bspPhysicsDataLoader_c::getInlineModelBounds(u32 modelNum, class aabb &bb) const {
	const q3Model_s *m = h->getModels() + modelNum;
	bb.fromTwoPoints(m->maxs,m->mins);
}
u32 bspPhysicsDataLoader_c::getInlineModelBrushCount(u32 modelNum) const {
	const q3Model_s *mod = h->getModel(modelNum);
	return mod->numBrushes;
}
u32 bspPhysicsDataLoader_c::getInlineModelSurfaceCount(u32 modelNum) const {
	const q3Model_s *mod = h->getModel(modelNum);
	return mod->numSurfaces;
}
u32 bspPhysicsDataLoader_c::getInlineModelGlobalBrushIndex(u32 subModelNum, u32 localBrushIndex) const {
	const q3Model_s *mod = h->getModel(subModelNum);
	u32 retBrushIndex = mod->firstBrush + localBrushIndex;
	return retBrushIndex;
}

bool bspPhysicsDataLoader_c::isCoD1BSP() const {
	return h->isBSPCoD1();
}
