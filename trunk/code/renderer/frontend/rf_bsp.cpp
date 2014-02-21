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
// rf_bsp.cpp - rBspTree_c class implementation
#include "rf_local.h"
#include "rf_bsp.h"
#include "rf_bezier.h"
#include "rf_drawCall.h"
#include "rf_model.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <api/materialSystemAPI.h>
#include <api/rbAPI.h>
#include <api/textureAPI.h>
#include <fileformats/bspFileFormat.h>
#include <fileformats/bspFileFormat_q2.h>
#include <shared/shared.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/mtrAPI.h>
#include "rf_lightGrid.h"
#include <shared/perStringCallback.h>
#include <shared/colorTable.h>

aCvar_c rf_bsp_noSurfaces("rf_bsp_noSurfaces","0");
aCvar_c rf_bsp_noBezierPatches("rf_bsp_noBezierPatches","0");
aCvar_c rf_bsp_drawBSPWorld("rf_bsp_drawBSPWorld","1");
aCvar_c rf_bsp_printFrustumCull("rf_bsp_printFrustumCull","0");
aCvar_c rf_bsp_noFrustumCull("rf_bsp_noFrustumCull","0");
aCvar_c rf_bsp_printVisChangeStats("rf_bsp_printVisChangeStats","0");
aCvar_c rf_bsp_noVis("rf_bsp_noVis","0");
aCvar_c rf_bsp_forceEverythingVisible("rf_bsp_forceEverythingVisible","0");
aCvar_c rf_bsp_doAllSurfaceBatchesOnCPU("rf_bsp_doAllSurfaceBatchesOnCPU","0");
aCvar_c rf_bsp_printCPUSurfVertsCount("rf_bsp_printCPUSurfVertsCount","0");
aCvar_c rf_bsp_skipGPUSurfaces("rf_bsp_skipGPUSurfaces","0");
aCvar_c rf_bsp_printCamClusterNum("rf_bsp_printCamClusterNum","0");
aCvar_c rf_bsp_printCamAreaNum("rf_bsp_printCamAreaNum","0");
// rf_bsp_rebuildBatchesOnAreaPortalsVisiblityChange
aCvar_c rf_bsp_rebuildBatchesOnAPVisChange("rf_bsp_rebuildBatchesOnAPVisChange","1");
aCvar_c rf_bsp_noFrustumAdjust("rf_bsp_noFrustumAdjust","0");
aCvar_c rf_bsp_showAreaPortals("rf_bsp_showAreaPortals","0");
aCvar_c rf_bsp_cullBackFacingAreaPortals("rf_bsp_cullBackFacingAreaPortals","1");
aCvar_c rf_bsp_useBSPForTracing("rf_bsp_useBSPForTracing","1");
aCvar_c rf_bsp_skipAreaPortals("rf_bsp_skipAreaPortals","0");
aCvar_c rf_bsp_printCamera2PortalDist("rf_bsp_printCamera2PortalDist","0");

const aabb &bspSurf_s::getBounds() const {
	if(type == BSPSF_BEZIER) {
		return patch->getBB();
	} else {
		return sf->bounds;
	}
}

rBspTree_c::rBspTree_c() {
	vis = 0;
	h = 0;
	visCounter = 1;
	c_flares = 0;
	lightGrid = 0;
}
rBspTree_c::~rBspTree_c() {
	clear();
}
void rBspTree_c::getSurfaceAreas(u32 surfNum, arraySTD_c<u32> &out) {
	for(u32 i = 0; i < leaves.size(); i++) {
		const q3Leaf_s &l = leaves[i];
		if(l.area < 0)
			continue;
		for(u32 j = 0; j < l.numLeafSurfaces; j++) {
			u32 absIdx = l.firstLeafSurface + j;
			if(absIdx >= leafSurfaces.size()) {
				g_core->RedWarning("rBspTree_c::getSurfaceAreas: absIndex >= leafSurfaces.size() (%i >= %i)\n",
					absIdx,leafSurfaces.size());
				continue;
			}
			u32 sfIndex = leafSurfaces[absIdx];
			if(sfIndex == surfNum) {
				out.add_unique(l.area);
			}
		}
	}
}
u32 rBspTree_c::getSurfaceContentFlags(u32 surfNum) const {
	if(surfNum >= surfs.size())
		return 1;
	int bspMaterialNum = surfs[surfNum].bspMaterialIndex;
	if(bspMaterialNum < 0 || bspMaterialNum >= bspMaterials.size()) {
		return 1;
	}
	return bspMaterials[bspMaterialNum].contentFlags;
}
void rBspTree_c::addSurfToBatches(u32 surfNum) {
	bspSurf_s *bs = &surfs[surfNum];
	if(bs->type != BSPSF_PLANAR && bs->type != BSPSF_TRIANGLES)
		return; // we're not batching bezier patches and flares
	if(bs->sf == 0) {
		g_core->RedWarning("rBspTree_c::addSurfToBatches: surface %i of type %i has NULL sf ptr\n",surfNum,bs->type);
		return;
	}
	// ignore surfaces with 'sky' material; sky is drawn other way
	if(bs->sf->mat->getSkyParms() != 0)
		return;

	bool bMergeMutlipleAreaSurfaces ;
	if(this->areaPortals.size()) {
		// areaPortals data is available for Qio BSPs
		bMergeMutlipleAreaSurfaces = false;
	} else {
		bMergeMutlipleAreaSurfaces = true;
	}
	arraySTD_c<u32> areas;
	if(bMergeMutlipleAreaSurfaces == false) {
		getSurfaceAreas(surfNum,areas);
		g_core->Print("Surface %i is referenced by %i areas\n",surfNum,areas.size());
	}

	numBatchSurfIndexes += bs->sf->absIndexes.getNumIndices();
	bspTriSurf_s *sf = bs->sf;
	// see if we can add this surface to existing batch
	for(u32 i = 0; i < batches.size(); i++) {
		bspSurfBatch_s *b = batches[i];
		if(b->mat != sf->mat) {
			continue;
		}
		if(b->lightmap != sf->lightmap) {
			continue;
		}
		// NOTE: we don't really have to check it because surfaces
		// with the same lightmap always should have the same deluxemap...
		if(b->deluxemap != sf->deluxemap) {
			continue;
		}
		if(bMergeMutlipleAreaSurfaces == false) {
			if(b->areas != areas) {
				continue;
			}
		}
		// TODO: merge only surfaces in the same cluster/area ? 
		b->addSurface(bs);
		return;
	}
	// create a new batch
	bspSurfBatch_s *nb = new bspSurfBatch_s;
	nb->initFromSurface(bs);
	nb->areas = areas;
	batches.push_back(nb);
}
void rBspTree_c::createBatches() {
	deleteBatches();
	numBatchSurfIndexes = 0;
	u32 numWorldSurfs = models[0].numSurfs;
	for(u32 i = 0; i < numWorldSurfs; i++) {
		addSurfToBatches(i);
	}
	for(u32 i = 0; i < batches.size(); i++) {
		bspSurfBatch_s *b = batches[i];
		b->lastVisSet.init(b->sfs.size(),true);
	}
	u32 numMergableSurfs = surfs.size() - c_flares - c_bezierPatches;
	g_core->Print(S_COLOR_GREEN "rBspTree_c::createBatches: %i surfaces merged into %i batches\n",
		numMergableSurfs, batches.size());
}
void rBspTree_c::deleteBatches() {
	for(u32 i = 0; i < batches.size(); i++) {
		delete batches[i];
	}
	batches.clear();
}
void rBspTree_c::calcTangentVectors() {
	this->verts.nullTBN();
	for(u32 i = 0; i < surfs.size(); i++) {
		bspSurf_s &sf = surfs[i];
		if(sf.type == BSPSF_PLANAR || sf.type == BSPSF_TRIANGLES) {
			this->verts.calcTBNForIndices(sf.sf->absIndexes,0);
		}
	}
	this->verts.normalizeTBN();
}
void rBspTree_c::createVBOandIBOs() {
	verts.uploadToGPU();
	for(u32 i = 0; i < batches.size(); i++) {
		batches[i]->indices.uploadToGPU();
	}
}
void rBspTree_c::createRenderModelsForBSPInlineModels() {
	for(u32 i = 1; i < models.size(); i++) {
		str modName = va("*%i",i);
		model_c *m = RF_AllocModel(modName);
		m->initInlineModel(this,i);
		m->setBounds(models[i].bb);
	}
}
// Quake3, RTCW, ET, FAKK, MoH lightmap size = 128
// Call Of Duty lightmap size = 512
bool rBspTree_c::loadLightmaps(u32 lumpNum, u32 lightmapSize) {
	// NOTE: default lightmap size is 128
	const lump_s &l = h->getLumps()[lumpNum];
	if(l.fileLen % (lightmapSize*lightmapSize*3)) {
		g_core->RedWarning("rBspTree_c::loadLightmaps: invalid lightmaps lump size\n");
		return true; // error
	}
	u32 numLightmaps = l.fileLen / (lightmapSize*lightmapSize*3);
	lightmaps.resize(numLightmaps);
	const byte *p = h->getLumpData(lumpNum);
	for(u32 i = 0; i < numLightmaps; i++) {
		textureAPI_i *t = lightmaps[i] = g_ms->createLightmap(p,lightmapSize,lightmapSize);
		p += (lightmapSize*lightmapSize*3);
	}
	return false; // OK
}
bool rBspTree_c::loadExternalLightmaps(const char *path) {
	str fixedPath = path;
	fixedPath.stripExtension();
	fixedPath.appendChar('/');
	str lightmapPath = fixedPath;
	lightmapPath.append("lightmap0.png");
	if(g_vfs->FS_FileExists(lightmapPath) == false)
		return true; // error
	u32 counter = 0;
	while(1) {
		lightmapPath = fixedPath;
		lightmapPath.append(va("lightmap%i.png",counter));
		if(g_vfs->FS_FileExists(lightmapPath)==false)
			break;
		textureAPI_i *lm = g_ms->loadTexture(lightmapPath);
		if(lm == 0)
			break;
		lightmaps.push_back(lm);

		str deluxemapPath = fixedPath;
		deluxemapPath.append(va("deluxemap%i.png",counter));
		if(g_vfs->FS_FileExists(deluxemapPath)) {
			textureAPI_i *dm = g_ms->loadTexture(deluxemapPath);
			if(dm)
				deluxemaps.push_back(dm);
		}
		counter++;
	}
	return false;
}
bool rBspTree_c::loadPlanes(u32 lumpPlanes) {
	const lump_s &pll = h->getLumps()[lumpPlanes];
	if(pll.fileLen % sizeof(q3Plane_s)) {
		g_core->RedWarning("rBspTree_c::loadPlanes: invalid planes lump size\n");
		return true; // error
	}
	if(sizeof(bspPlane_s) != sizeof(q3Plane_s)) {
		g_core->DropError("rBspTree_c::loadPlanes: sizeof(bspPlane_s) != sizeof(q3Plane_s)");
		return true; // error
	}
	u32 numPlanes = pll.fileLen / sizeof(q3Plane_s);
	planes.resize(numPlanes);
	memcpy(planes.getArray(),h->getLumpData(lumpPlanes),pll.fileLen);
	return false; // OK
}
bool rBspTree_c::loadPlanesQ2(u32 lumpPlanes) {
	u32 planesLumpLen = h->getLumpSize(lumpPlanes);
	if(planesLumpLen % sizeof(q2Plane_s)) {
		g_core->RedWarning("rBspTree_c::loadPlanesQ2: invalid planes lump size\n");
		return true; // error
	}
	u32 numPlanes = planesLumpLen / sizeof(q2Plane_s);
	planes.resize(numPlanes);
	const q2Plane_s *ip = (const q2Plane_s*)h->getLumpData(lumpPlanes);
	bspPlane_s *pl = planes.getArray();
	for(u32 i = 0; i < numPlanes; i++, pl++, ip++) {
		pl->normal = ip->normal;
		pl->dist = ip->dist;
	}
	return false; // OK
}
bool rBspTree_c::loadNodesAndLeaves(u32 lumpNodes, u32 lumpLeaves, u32 sizeOfLeaf) {
	const lump_s &nl = h->getLumps()[lumpNodes];
	if(nl.fileLen % sizeof(q3Node_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeaves: invalid nodes lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(q3Node_s);
	nodes.resize(numNodes);
	memcpy(nodes.getArray(),h->getLumpData(lumpNodes),h->getLumpSize(lumpNodes));

	if(h->isBSPCoD1()) {
		const lump_s &ll = h->getLumps()[COD1_LEAFS];
		if(ll.fileLen % sizeof(cod1Leaf_s)) {
			g_core->RedWarning("rBspTree_c::loadNodesAndLeaves: invalid leaves lump size\n");
			return true; // error
		}
		u32 numLeaves = ll.fileLen / sizeOfLeaf;
		leaves.resize(numLeaves);
		q3Leaf_s *l = leaves.getArray();
		const cod1Leaf_s *il = (const cod1Leaf_s*)h->getLumpData(COD1_LEAFS);
		for(u32 i = 0; i < numLeaves; i++, l++, il++) {
			l->area = il->area;
			l->cluster = il->cluster;
			l->firstLeafBrush = il->firstLeafBrush;
			l->firstLeafSurface = il->firstLeafSurface;
			l->numLeafBrushes = il->numLeafBrushes;
			l->numLeafSurfaces = il->numLeafSurfaces;
		}
	} else {
		const lump_s &ll = h->getLumps()[lumpLeaves];
		if(ll.fileLen % sizeOfLeaf) {
			g_core->RedWarning("rBspTree_c::loadNodesAndLeaves: invalid leaves lump size\n");
			return true; // error
		}
		u32 numLeaves = ll.fileLen / sizeOfLeaf;
		leaves.resize(numLeaves);
		memcpy_strided(leaves.getArray(),h->getLumpData(lumpLeaves),numLeaves,sizeof(q3Leaf_s),sizeof(q3Leaf_s),sizeOfLeaf);
	}
	return false; // OK
}
bool rBspTree_c::loadNodesAndLeavesQ2(u32 lumpNodes, u32 lumpLeaves) {
	const lump_s &nl = h->getLumps()[lumpNodes];
	if(nl.fileLen % sizeof(q2Node_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeavesQ2: invalid nodes lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(q2Node_s);
	nodes.resize(numNodes);
	const q2Node_s *in = (const q2Node_s*)h->getLumpData(lumpNodes);
	q3Node_s *on = nodes.getArray();
	for(u32 i = 0; i < numNodes; i++, in++, on++) {
		on->children[0] = in->children[0];
		on->children[1] = in->children[1];
		on->planeNum = in->planeNum;
	}
	const lump_s &ll = h->getLumps()[lumpLeaves];
	if(ll.fileLen % sizeof(q2Leaf_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeavesQ2: invalid leaves lump size\n");
		return true; // error
	}
	u32 numLeaves = ll.fileLen / sizeof(q2Leaf_s);
	leaves.resize(numLeaves);
	q3Leaf_s *l = leaves.getArray();
	const q2Leaf_s *il = (const q2Leaf_s*)h->getLumpData(lumpLeaves);
	for(u32 i = 0; i < numLeaves; i++, l++, il++) {
		l->area = il->area;
		l->cluster = il->cluster;
		l->maxs[0] = il->maxs[0];
		l->maxs[1] = il->maxs[1];
		l->maxs[2] = il->maxs[2];
		l->mins[0] = il->mins[0];
		l->mins[1] = il->mins[1];
		l->mins[2] = il->mins[2];
		l->firstLeafBrush = il->firstLeafBrush;
		l->firstLeafSurface = il->firstLeafSurface;
		l->numLeafBrushes = il->numLeafBrushes;
		l->numLeafSurfaces = il->numLeafSurfaces;
	}
	
	return false; // OK
}
bool rBspTree_c::loadNodesAndLeavesHL(u32 lumpNodes, u32 lumpLeaves) {
	const lump_s &nl = h->getLumps()[lumpNodes];
	if(nl.fileLen % sizeof(hlNode_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeavesHL: invalid nodes lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(hlNode_s);
	nodes.resize(numNodes);
	const hlNode_s *in = (const hlNode_s*)h->getLumpData(lumpNodes);
	q3Node_s *on = nodes.getArray();
	for(u32 i = 0; i < numNodes; i++, in++, on++) {
		// NOTE: hlNode_s::children indexes are signed shorts,
		// and q3Node_s::children and q2Node_s::children are signed integers
		on->children[0] = in->children[0];
		on->children[1] = in->children[1];
		on->planeNum = in->planeNum;
	}
	const lump_s &ll = h->getLumps()[lumpLeaves];
	if(ll.fileLen % sizeof(hlLeaf_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeavesHL: invalid leaves lump size\n");
		return true; // error
	}
	u32 numLeaves = ll.fileLen / sizeof(hlLeaf_s);
	leaves.resize(numLeaves);
	q3Leaf_s *l = leaves.getArray();
	const hlLeaf_s *il = (const hlLeaf_s*)h->getLumpData(lumpLeaves);
	for(u32 i = 0; i < numLeaves; i++, l++, il++) {
		// there are no areas in HL bsps
		l->area = 0;//il->area;
		// there are no clusters in HL bsps....
		//l->cluster = il->cluster;
		// ...so fake the cluster number
		l->cluster = i;

		l->maxs[0] = il->maxs[0];
		l->maxs[1] = il->maxs[1];
		l->maxs[2] = il->maxs[2];
		l->mins[0] = il->mins[0];
		l->mins[1] = il->mins[1];
		l->mins[2] = il->mins[2];
		l->firstLeafSurface = il->firstMarkSurface;
		l->numLeafSurfaces = il->numMarkSurfaces;
		// there are no brush data in HL bsps
		l->firstLeafBrush = 0;
		l->numLeafBrushes = 0;
	}
	
	return false; // OK
}
bool rBspTree_c::loadNodesAndLeavesSE() {
	const srcLump_s &nl = srcH->getLumps()[SRC_NODES];
	if(nl.fileLen % sizeof(srcNode_s)) {
		g_core->RedWarning("rBspTree_c::loadNodesAndLeavesSE: invalid nodes lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(srcNode_s);
	nodes.resize(numNodes);
	const srcNode_s *in = (const srcNode_s*)srcH->getLumpData(SRC_NODES);
	q3Node_s *on = nodes.getArray();
	for(u32 i = 0; i < numNodes; i++, in++, on++) {
		on->children[0] = in->children[0];
		on->children[1] = in->children[1];
		on->maxs[0] = in->maxs[0];
		on->maxs[1] = in->maxs[1];
		on->maxs[2] = in->maxs[2];
		on->mins[0] = in->mins[0];
		on->mins[1] = in->mins[1];
		on->mins[2] = in->mins[2];
		on->planeNum = in->planeNum;
	}
	if(srcH->version == 19) {
		const srcLump_s &ll = srcH->getLumps()[SRC_LEAFS];
		if(ll.fileLen % sizeof(srcLeaf_s)) {
			g_core->RedWarning("rBspTree_c::loadNodesAndLeavesSE: invalid leaves lump size\n");
			return true; // error
		}
		u32 numLeaves = ll.fileLen / sizeof(srcLeaf_s);
		leaves.resize(numLeaves);
		q3Leaf_s *l = leaves.getArray();
		const srcLeaf_s *il = (const srcLeaf_s*)srcH->getLumpData(SRC_LEAFS);
		for(u32 i = 0; i < numLeaves; i++, l++, il++) {
			l->area = il->area;
			l->cluster = il->cluster;

			l->maxs[0] = il->maxs[0];
			l->maxs[1] = il->maxs[1];
			l->maxs[2] = il->maxs[2];
			l->mins[0] = il->mins[0];
			l->mins[1] = il->mins[1];
			l->mins[2] = il->mins[2];
			l->firstLeafSurface = il->firstLeafSurface;
			l->numLeafSurfaces = il->numLeafSurfaces;
			l->firstLeafBrush = il->firstLeafBrush;
			l->numLeafBrushes = il->numLeafBrushes;
		}
	} else {		
		const srcLump_s &ll = srcH->getLumps()[SRC_LEAFS];
		if(ll.fileLen % sizeof(srcLeaf_noLightCube_s)) {
			g_core->RedWarning("rBspTree_c::loadNodesAndLeavesSE: invalid leaves lump size\n");
			return true; // error
		}
		u32 numLeaves = ll.fileLen / sizeof(srcLeaf_noLightCube_s);
		leaves.resize(numLeaves);
		q3Leaf_s *l = leaves.getArray();
		const srcLeaf_noLightCube_s *il = (const srcLeaf_noLightCube_s*)srcH->getLumpData(SRC_LEAFS);
		for(u32 i = 0; i < numLeaves; i++, l++, il++) {
			l->area = il->area;
			l->cluster = il->cluster;

			l->maxs[0] = il->maxs[0];
			l->maxs[1] = il->maxs[1];
			l->maxs[2] = il->maxs[2];
			l->mins[0] = il->mins[0];
			l->mins[1] = il->mins[1];
			l->mins[2] = il->mins[2];
			l->firstLeafSurface = il->firstLeafSurface;
			l->numLeafSurfaces = il->numLeafSurfaces;
			l->firstLeafBrush = il->firstLeafBrush;
			l->numLeafBrushes = il->numLeafBrushes;
		}
	}
	return false; // OK
}
bool rBspTree_c::loadVerts(u32 lumpVerts) {
	const q3Vert_s *iv = (const q3Vert_s*) h->getLumpData(lumpVerts);
	u32 numVerts = h->getLumpStructCount(lumpVerts,sizeof(q3Vert_s));
	verts.resize(numVerts);
	rVert_c *ov = verts.getArray();
	// convert vertices
	// swap colors for DX9 backend
	//if(rb->getType() == BET_DX9) {
	//	for(u32 i = 0; i < numVerts; i++, ov++, iv++) {
	//		ov->xyz = iv->xyz;
	//		ov->tc = iv->st;
	//		ov->lc = iv->lightmap;
	//		ov->normal = iv->normal;
	//		// dx expects ARGB ....
	//		ov->color[0] = iv->color[3];
	//		ov->color[1] = iv->color[0];
	//		ov->color[2] = iv->color[1];
	//		ov->color[3] = iv->color[2];
	//	}
	//} else {
		for(u32 i = 0; i < numVerts; i++, ov++, iv++) {
			ov->xyz = iv->xyz;
			ov->tc = iv->st;
			ov->lc = iv->lightmap;
			ov->normal = iv->normal;
			ov->color[0] = iv->color[0];
			ov->color[1] = iv->color[1];
			ov->color[2] = iv->color[2];
			ov->color[3] = iv->color[3];
		}
//	}
	return false; // no error
}
bool rBspTree_c::loadSurfs(u32 lumpSurfs, u32 sizeofSurf, u32 lumpIndexes, u32 lumpVerts, u32 lumpMats, u32 sizeofMat) {
	const lump_s &sl = h->getLumps()[lumpSurfs];
	if(sl.fileLen % sizeofSurf) {
		g_core->RedWarning("rBspTree_c::loadSurfs: invalid surfs lump size\n");
		return true; // error
	}
	const lump_s &il = h->getLumps()[lumpIndexes];
	if(il.fileLen % sizeof(int)) {
		g_core->RedWarning("rBspTree_c::loadSurfs: invalid indexes lump size\n");
		return true; // error
	}
	const lump_s &vl = h->getLumps()[lumpVerts];
	if(vl.fileLen % sizeof(q3Vert_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfs: invalid indexes lump size\n");
		return true; // error
	}
	const lump_s &ml = h->getLumps()[lumpMats];
	if(ml.fileLen % sizeofMat) {
		g_core->RedWarning("rBspTree_c::loadSurfs: invalid material lump size\n");
		return true; // error
	}
	const byte *materials = h->getLumpData(lumpMats);
	const q3Surface_s *sf = (const q3Surface_s *)h->getLumpData(lumpSurfs);
	u32 numSurfs = sl.fileLen / sizeofSurf;
	surfs.resize(numSurfs);
	bspSurf_s *out = surfs.getArray();
	const u32 *indices = (const u32 *) h->getLumpData(lumpIndexes);
	// load vertices
	loadVerts(lumpVerts);
	// load bspMaterials contentFlags
	u32 numBSPMaterials = h->getNumMaterials();
	bspMaterials.resize(numBSPMaterials);
	for(u32 i = 0; i < numBSPMaterials; i++) {
		// TODO: convert content flags
		bspMaterials[i].contentFlags = h->getMat(i)->contentFlags;
	}
	c_bezierPatches = 0;
	c_flares = 0;
	for(u32 i = 0; i < numSurfs; i++, out++) {
		const q3BSPMaterial_s *bspMaterial = (const q3BSPMaterial_s *)(materials + sizeofMat * sf->materialNum);
		mtrAPI_i *mat = g_ms->registerMaterial(bspMaterial->shader);
		textureAPI_i *lightmap;
		textureAPI_i *deluxemap;
		if(sf->lightmapNum < 0) {
			lightmap = 0;
			deluxemap = 0;
		} else {
			if(sf->lightmapNum >= lightmaps.size()) {
				lightmap = 0;
			} else {
				lightmap = lightmaps[sf->lightmapNum];
			}
			if(sf->lightmapNum >= deluxemaps.size()) {
				deluxemap = 0;
			} else {
				deluxemap = deluxemaps[sf->lightmapNum];
			}
		}
		out->bspMaterialIndex = sf->materialNum;
		if(mat->getSkyParms()) {
			RF_SetSkyMaterial(mat);
		}
		if(sf->surfaceType == Q3MST_PLANAR) {
			out->type = BSPSF_PLANAR;
parsePlanarSurf:;
			bspTriSurf_s *ts = out->sf = new bspTriSurf_s;
			ts->mat = mat;
			ts->lightmap = lightmap;
			ts->deluxemap = deluxemap;
			ts->firstVert = sf->firstVert;
			ts->numVerts = sf->numVerts;
			if(sf->numVerts > 2) {
				ts->plane.fromThreePoints(verts[sf->firstVert + 0].xyz,
					verts[sf->firstVert + 1].xyz,
					verts[sf->firstVert + 2].xyz);
			}
			const u32 *firstIndex = indices + sf->firstIndex;
			// get the largest index value of this surface
			// to determine if we can use U16 index buffer
			u32 largestIndex = 0;
			for(u32 j = 0; j < sf->numIndexes; j++) {
				u32 idx = sf->firstVert + firstIndex[j];
				if(idx > largestIndex) {
					largestIndex = idx;
				}
			}
			ts->bounds.clear();
			if(largestIndex + 1 < U16_MAX) {
				g_core->Print("rBspTree_c::loadSurfs: using U16 index buffer for surface %i\n",i);
				u16 *u16Indexes = ts->absIndexes.initU16(sf->numIndexes);
				u16 *u16IndexesLocal = ts->localIndexes.initU16(sf->numIndexes);
				for(u32 j = 0; j < sf->numIndexes; j++) {
					u16IndexesLocal[j] = firstIndex[j];
					u16Indexes[j] = sf->firstVert + firstIndex[j];
					// update bounding box as well
					ts->bounds.addPoint(this->verts[u16Indexes[j]].xyz);
				}
			} else {
				g_core->Print("rBspTree_c::loadSurfs: using U32 index buffer for surface %i\n",i);
				u32 *u32Indexes = ts->absIndexes.initU32(sf->numIndexes);
				u16 *u16IndexesLocal = ts->localIndexes.initU16(sf->numIndexes);
				for(u32 j = 0; j < sf->numIndexes; j++) {
					u16IndexesLocal[j] = firstIndex[j];
					u32Indexes[j] = sf->firstVert + firstIndex[j];
					// update bounding box as well	
					ts->bounds.addPoint(this->verts[u32Indexes[j]].xyz);
				}
			}
		} else if(sf->surfaceType == Q3MST_PATCH) {
			out->type = BSPSF_BEZIER;
			r_bezierPatch_c *bp = out->patch = new r_bezierPatch_c;
			bp->setMaterial(mat);
			bp->setLightmap(lightmap);
			bp->setHeight(sf->patchHeight);
			bp->setWidth(sf->patchWidth);
			for(u32 j = 0; j < sf->numVerts; j++) {
				u32 vertIndex = sf->firstVert + j;
				const rVert_c &v = this->verts[vertIndex];
				bp->addVertex(v);
			}		
			bp->tesselate(4);
			c_bezierPatches++;
		} else if(sf->surfaceType == Q3MST_TRIANGLE_SOUP) {
			out->type = BSPSF_TRIANGLES;
			goto parsePlanarSurf;
		} else {
			out->type = BSPSF_FLARE;
			c_flares++;
		}
		sf = (const q3Surface_s *) (((const byte*)sf)+sizeofSurf);
	}
	return false; // OK
}
class q2PolyBuilder_c {
	arraySTD_c<rVert_c> verts;
	arraySTD_c<u16> indices;
public:
	void addEdge(const vec3_c &v0, const vec3_c &v1, u32 localNum) {
		if(localNum == 0) {
			verts.push_back(rVert_c(v0));
		}
		verts.push_back(rVert_c(v1));
	}
	void calcTriIndexes() {
		for(u32 i = 2; i < verts.size(); i++) {
			indices.push_back(0);
			indices.push_back(i-1);
			indices.push_back(i);
		}
	}
	void calcTexCoords(const float vecs[2][4], float texDimX, float texDimY) {
		rVert_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			float tU = v->xyz[0]*vecs[0][0] + v->xyz[1]*vecs[0][1] + v->xyz[2]*vecs[0][2] + vecs[0][3];
			float tV = v->xyz[0]*vecs[1][0] + v->xyz[1]*vecs[1][1] + v->xyz[2]*vecs[1][2] + vecs[1][3];
			tU /= texDimX;
			tV /= texDimY;
			v->tc.set(tU,tV);
		}
	}
	void calcLightmapCoords(const float vecs[2][4], float texDimX, float texDimY) {
		rVert_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			float tU = v->xyz[0]*vecs[0][0] + v->xyz[1]*vecs[0][1] + v->xyz[2]*vecs[0][2] + vecs[0][3];
			float tV = v->xyz[0]*vecs[1][0] + v->xyz[1]*vecs[1][1] + v->xyz[2]*vecs[1][2] + vecs[1][3];
			tU /= texDimX;
			tV /= texDimY;
			v->lc.set(tU,tV);
		}
	}
	void addVertsToBounds(aabb &out) {
		for(u32 i = 0; i < verts.size(); i++) {
			out.addPoint(verts[i].xyz);
		}
	}
	const arraySTD_c<rVert_c> &getVerts() const {
		return verts;
	}
	arraySTD_c<rVert_c> &getVerts() {
		return verts;
	}
	const arraySTD_c<u16> &getIndices() const {
		return indices;
	}
	u16 getIndex(u32 i) const {
		return indices[i];
	}
	u32 getNumVerts() const {
		return verts.size();
	}
	u32 getNumIndices() const {
		return indices.size();
	}
};

bool rBspTree_c::loadSurfsQ2() {
	const lump_s &sl = h->getLumps()[Q2_FACES];
	if(sl.fileLen % sizeof(q2Surface_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsQ2: invalid surfs lump size\n");
		return true; // error
	}
	const lump_s &el = h->getLumps()[Q2_EDGES];
	if(el.fileLen % sizeof(q2Edge_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsQ2: invalid edges lump size\n");
		return true; // error
	}
	const lump_s &tl = h->getLumps()[Q2_TEXINFO];
	if(tl.fileLen % sizeof(q2TexInfo_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsQ2: invalid texinfo lump size\n");
		return true; // error
	}
	const lump_s &vl = h->getLumps()[Q2_VERTEXES];
	if(vl.fileLen % sizeof(q2Vert_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsQ2: invalid vertexes lump size\n");
		return true; // error
	}
	const lump_s &sel = h->getLumps()[Q2_SURFEDGES];
	if(sel.fileLen % sizeof(int)) {
		g_core->RedWarning("rBspTree_c::loadSurfsQ2: invalid surfEdges lump size\n");
		return true; // error
	}
	u32 numSurfaces = sl.fileLen / sizeof(q2Surface_s);
	u32 numVerts = vl.fileLen / sizeof(q2Vert_s);
	const q2Surface_s *isf = (const q2Surface_s *)h->getLumpData(Q2_FACES);
	const q2Vert_s *iv = (const q2Vert_s *)h->getLumpData(Q2_VERTEXES);
	const q2Edge_s *ie = (const q2Edge_s *)h->getLumpData(Q2_EDGES);
	const q2TexInfo_s *it = (const q2TexInfo_s *)h->getLumpData(Q2_TEXINFO);
	const int *surfEdges = (const int *)h->getLumpData(Q2_SURFEDGES);
	surfs.resize(numSurfaces);
	bspSurf_s *oSF = surfs.getArray();
	for(u32 i = 0; i < numSurfaces; i++, isf++, oSF++) {
		const q2TexInfo_s *sfTexInfo = it + isf->texinfo;
		str matName = "textures/";
		matName.append(sfTexInfo->texture);
		matName.append(".wal");
		oSF->bspMaterialIndex = isf->texinfo;
		oSF->type = BSPSF_PLANAR;
		bspTriSurf_s *ts = oSF->sf = new bspTriSurf_s;
		ts->mat = g_ms->registerMaterial(matName);
		ts->lightmap = 0;
		ts->deluxemap = 0;
		q2PolyBuilder_c polyBuilder;
		for(int j = 0; j < isf->numEdges-1; j++) {
			int ei = surfEdges[isf->firstEdge + j];
			u32 realEdgeIndex;
			if(ei < 0) {
				realEdgeIndex = -ei;
				const q2Edge_s *ed = ie + realEdgeIndex;
				// reverse vertex order
				polyBuilder.addEdge(iv[ed->v[1]].point,iv[ed->v[0]].point,j);
			} else {
				realEdgeIndex = ei;
				const q2Edge_s *ed = ie + realEdgeIndex;
				polyBuilder.addEdge(iv[ed->v[0]].point,iv[ed->v[1]].point,j);
			}
		}
		polyBuilder.calcTexCoords(sfTexInfo->vecs,ts->mat->getImageWidth(),ts->mat->getImageHeight());
		ts->firstVert = verts.size();
		ts->numVerts = polyBuilder.getNumVerts();
		verts.addArray(polyBuilder.getVerts());
		polyBuilder.calcTriIndexes();
		polyBuilder.addVertsToBounds(ts->bounds);
		u32 *indicesU32 = ts->absIndexes.initU32(polyBuilder.getNumIndices());
		for(u32 j = 0; j < polyBuilder.getNumIndices(); j++) {
			indicesU32[j] = ts->firstVert+polyBuilder.getIndex(j);
		}
	}
	return false; // OK
}
bool rBspTree_c::loadSurfsHL() {
	const lump_s &sl = h->getLumps()[HL_FACES];
	if(sl.fileLen % sizeof(hlSurface_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsHL: invalid surfs lump size\n");
		return true; // error
	}
	const lump_s &el = h->getLumps()[HL_EDGES];
	if(el.fileLen % sizeof(hlEdge_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsHL: invalid edges lump size\n");
		return true; // error
	}
	const lump_s &tl = h->getLumps()[HL_TEXINFO];
	if(tl.fileLen % sizeof(hlTexInfo_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsHL: invalid texinfo lump size\n");
		return true; // error
	}
	const lump_s &vl = h->getLumps()[HL_VERTEXES];
	if(vl.fileLen % sizeof(hlVert_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsHL: invalid vertexes lump size\n");
		return true; // error
	}
	const lump_s &sel = h->getLumps()[HL_SURFEDGES];
	if(sel.fileLen % sizeof(int)) {
		g_core->RedWarning("rBspTree_c::loadSurfsHL: invalid surfEdges lump size\n");
		return true; // error
	}
	u32 numSurfaces = sl.fileLen / sizeof(hlSurface_s);
	u32 numVerts = vl.fileLen / sizeof(hlVert_s);
	const hlSurface_s *isf = (const hlSurface_s *)h->getLumpData(HL_FACES);
	const hlVert_s *iv = (const hlVert_s *)h->getLumpData(HL_VERTEXES);
	const hlEdge_s *ie = (const hlEdge_s *)h->getLumpData(HL_EDGES);
	const hlTexInfo_s *it = (const hlTexInfo_s *)h->getLumpData(HL_TEXINFO);
	const int *surfEdges = (const int *)h->getLumpData(HL_SURFEDGES);
	// textures data lump
	const hlMipTexLump_s *mipTexLump = (const hlMipTexLump_s*)h->getLumpData(HL_TEXTURES);

	surfs.resize(numSurfaces);
	bspSurf_s *oSF = surfs.getArray();
	arraySTD_c<mtrAPI_i*> mipTexCache;
	mipTexCache.resize(mipTexLump->numMipTex);
	mipTexCache.nullMemory();
	for(u32 i = 0; i < numSurfaces; i++, isf++, oSF++) {
		const hlTexInfo_s *sfTexInfo = it + isf->texInfo;
		str matName = "textures/";
		//matName.append(sfTexInfo->texture);
		//matName.append(".wal");
		oSF->type = BSPSF_PLANAR;
		bspTriSurf_s *ts = oSF->sf = new bspTriSurf_s;
		ts->deluxemap = 0;
		oSF->bspMaterialIndex = isf->texInfo;
		// get texture image data
		if(sfTexInfo->miptex < 0 || sfTexInfo->miptex >= mipTexLump->numMipTex) {
			g_core->RedWarning("rBspTree_c::loadSurfsHL(): miptex index %i out of range <0,%i) for surface %i\n",
				sfTexInfo->miptex,mipTexLump->numMipTex,i);
			ts->mat = g_ms->registerMaterial("invalidHLMiptexIndex");
		} else {
			if(mipTexCache[sfTexInfo->miptex] == 0) {
				int mipTexOfs = mipTexLump->dataofs[sfTexInfo->miptex];
				if(mipTexOfs >= 0) {
					const hlMipTex_s *mipTex = (const hlMipTex_s*)(h->getLumpData(HL_TEXTURES)+mipTexOfs);
					if(mipTex->offsets[0] != 0) {
						const byte *pal;
						if(h->ident == BSP_VERSION_HL) {
							// palette is just after texture data
							pal = ((const byte*)mipTex +
								mipTex->offsets[3] +
								(mipTex->width/8) * (mipTex->height/8))
								+ 2; // unknown 2 bytes, always [?] 0x00 0x01
						} else {
							pal = 0;
						}
						const byte *pixels = (((const byte*)mipTex) + mipTex->offsets[0]);
						str matName = va("*%s",mipTex->name);
						mipTexCache[sfTexInfo->miptex] = g_ms->createHLBSPTexture(matName,pixels,mipTex->width,mipTex->height,pal);
					} else {

					}
				} else {

				}
			}
			ts->mat = mipTexCache[sfTexInfo->miptex];
		}
		if(ts->mat == 0) {
			ts->mat = g_ms->registerMaterial("noMaterial");
		}
		ts->lightmap = 0;
		q2PolyBuilder_c polyBuilder;
		for(int j = 0; j < isf->numEdges-1; j++) {
			int ei = surfEdges[isf->firstEdge + j];
			u32 realEdgeIndex;
			if(ei < 0) {
				realEdgeIndex = -ei;
				const hlEdge_s *ed = ie + realEdgeIndex;
				// reverse vertex order
				polyBuilder.addEdge(iv[ed->v[1]].point,iv[ed->v[0]].point,j);
			} else {
				realEdgeIndex = ei;
				const hlEdge_s *ed = ie + realEdgeIndex;
				polyBuilder.addEdge(iv[ed->v[0]].point,iv[ed->v[1]].point,j);
			}
		}
		polyBuilder.calcTexCoords(sfTexInfo->vecs,ts->mat->getImageWidth(),ts->mat->getImageHeight());
		ts->firstVert = verts.size();
		ts->numVerts = polyBuilder.getNumVerts();
		verts.addArray(polyBuilder.getVerts());
		polyBuilder.calcTriIndexes();
		polyBuilder.addVertsToBounds(ts->bounds);
		u32 *indicesU32 = ts->absIndexes.initU32(polyBuilder.getNumIndices());
		for(u32 j = 0; j < polyBuilder.getNumIndices(); j++) {
			indicesU32[j] = ts->firstVert+polyBuilder.getIndex(j);
		}
	}
	return false; // OK
}
struct sfLightmapDef_s {
	u32 x, y;
	u32 h, w;
	u32 mergeIndex;
};
class lightmapAllocator_c {
	// renderer backend lightmaps
	arraySTD_c<class textureAPI_i*> lightmaps;
	// per-fragment lightmap definitions
	arraySTD_c<sfLightmapDef_s> surfLightmaps;
	u32 lightmapSize;
	// for statistics
	u32 numSubLightmaps;
	// current lightmap image allocation state
	arraySTD_c<u32> allocated; // [lightmapSize]
	// current lightmap image data (pixels RGB)
	arraySTD_c<byte> curData;
public:
	void prepareNextLightmap() {
		g_core->Print("lightmapAllocator_c::prepareNextLightmap: %i sublightmaps merged into lightmap texture %i\n",numSubLightmaps,lightmaps.size());
		allocated.nullMemory();
		textureAPI_i *prev = g_ms->createLightmap(curData.getArray(),lightmapSize,lightmapSize);
		lightmaps.push_back(prev);
		curData.nullMemory();
		numSubLightmaps = 0;
	}
	bool allocNextBlock(struct sfLightmapDef_s &lm) {
		int best = this->lightmapSize;

		for (int i = 0; i <= this->lightmapSize-lm.w; i++) {
			int best2 = 0;
			int j;
			for (j = 0; j < lm.w; j++) {
				if (allocated[i+j] >= best) {
					break;
				}
				if (allocated[i+j] > best2) {
					best2 = allocated[i+j];
				}
			}
			if (j == lm.w)	{	// this is a valid spot
				lm.x = i;
				lm.y = best = best2;
			}
		}

		if (best + lm.h > this->lightmapSize) {
			return false;
		}
		for (int i = 0; i < lm.w; i++) {
			allocated[lm.x + i] = best + lm.h;
		}
		return true;
	}
public:	
	void init(u32 newLightmapSize = 128) {
		this->lightmapSize = newLightmapSize;
		allocated.resize(this->lightmapSize);
		curData.resize(this->lightmapSize * this->lightmapSize * 3);
		allocated.nullMemory();
		numSubLightmaps = 0;
		assert(lightmaps.size() == 0);
	}
	u32 addLightmap(const byte *data, u32 w, u32 h) {
		u32 ret = surfLightmaps.size();
		sfLightmapDef_s &n = surfLightmaps.pushBack();
		n.w = w;
		n.h = h;
		if(allocNextBlock(n) == false) {
			this->prepareNextLightmap();
			if (!this->allocNextBlock(n)) {
				printf("lightmapAllocator_c::addLightmap: allocNextBlock failed twice!\n");
				return 0;
			}
		}
		this->numSubLightmaps++;
		n.mergeIndex = lightmaps.size();
		for(u32 i = 0; i < n.w; i++) {
			for(u32 j = 0; j < n.h; j++) {
				u32 nowX = n.x + i;
				u32 nowY = n.y + j;
				byte *out = &curData[(nowY*lightmapSize+nowX)*3];
				const byte *in = data + (j*w+i)*3;
				out[0] = in[0];
				out[1] = in[1];
				out[2] = in[2];
			}
		}
		return ret;
	}
	void finalize() {
		prepareNextLightmap();
	}
	const sfLightmapDef_s &getLightmapDef(u32 index) const {
		return surfLightmaps[index];
	}
	u32 getLightmapSize() const {
		return lightmapSize;
	}
	class textureAPI_i *getSurfLightmap(u32 sfIndex) const {
		return lightmaps[surfLightmaps[sfIndex].mergeIndex];
	}
};

bool rBspTree_c::loadSurfsSE() {
	const srcLump_s &sl = srcH->getLumps()[SRC_FACES];
	if(h->version == 18) {
		if(sl.fileLen % sizeof(srcSurfaceV18_s)) {
			g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid surfs lump size\n");
			return true; // error
		}
	} else {
		if(sl.fileLen % sizeof(srcSurface_s)) {
			g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid surfs lump size\n");
			return true; // error
		}
	}
	const srcLump_s &el = srcH->getLumps()[SRC_EDGES];
	if(el.fileLen % sizeof(srcEdge_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid edges lump size\n");
		return true; // error
	}
	const srcLump_s &tl = srcH->getLumps()[SRC_TEXINFO];
	if(tl.fileLen % sizeof(srcTexInfo_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid texinfo lump size\n");
		return true; // error
	}
	const srcLump_s &vl = srcH->getLumps()[SRC_VERTEXES];
	if(vl.fileLen % sizeof(srcVert_s)) {
		g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid vertexes lump size\n");
		return true; // error
	}
	const srcLump_s &sel = srcH->getLumps()[SRC_SURFEDGES];
	if(sel.fileLen % sizeof(int)) {
		g_core->RedWarning("rBspTree_c::loadSurfsSE: invalid surfEdges lump size\n");
		return true; // error
	}
	u32 numSurfaces;
	if(h->version == 18) {
		numSurfaces = sl.fileLen / sizeof(srcSurfaceV18_s);
	} else {
		numSurfaces = sl.fileLen / sizeof(srcSurface_s);
	}
	u32 numVerts = vl.fileLen / sizeof(srcVert_s);
	const srcVert_s *iv = (const srcVert_s *)srcH->getLumpData(SRC_VERTEXES);
	const srcEdge_s *ie = (const srcEdge_s *)srcH->getLumpData(SRC_EDGES);
	const srcTexInfo_s *it = (const srcTexInfo_s *)srcH->getLumpData(SRC_TEXINFO);
	const int *surfEdges = (const int *)srcH->getLumpData(SRC_SURFEDGES);
	const srcTexData_s *texData = (const srcTexData_s*)srcH->getLumpData(SRC_TEXDATA);
	const u32 *texDataIndexes = (u32*) srcH->getLumpData(SRC_TEXDATA_STRING_TABLE);
	const char *texDataStr = (const char*)srcH->getLumpData(SRC_TEXDATA_STRING_DATA);
	const byte *lightmaps = (const byte*)srcH->getLumpData(SRC_LIGHTING);
		
	const srcSurface_s *isf;
	const srcSurfaceV18_s *isf18;
	if(srcH->version == 18) {
		isf = 0;
		isf18 = (const srcSurfaceV18_s *)srcH->getLumpData(SRC_FACES);
	} else {
		isf = (const srcSurface_s *)srcH->getLumpData(SRC_FACES);
		isf18 = 0;
	}

	// used while converting SE lightmaps to our format
	arraySTD_c<byte> rgbs;

	lightmapAllocator_c la;
	la.init(1024);

	surfs.resize(numSurfaces);
	bspSurf_s *oSF = surfs.getArray();
	for(u32 i = 0; i < numSurfaces; i++, oSF++) {
		if(isf18) {
			isf = &isf18->s;
		}
		oSF->type = BSPSF_PLANAR;
		bspTriSurf_s *ts = oSF->sf = new bspTriSurf_s;
		// get vmt file name
		const srcTexInfo_s *sfTexInfo = it + isf->texInfo;
		
		str matName;
		if(sfTexInfo->texData < 0) {
			// sfTexInfo->texData might be -1
			matName = "nomaterial";
		} else {
			const srcTexData_s *sfTexData = texData + sfTexInfo->texData;
			u32 ofs = texDataIndexes[sfTexData->nameStringTableID];
			matName = texDataStr + ofs;
			matName.defaultExtension("vmt");
		}
		oSF->bspMaterialIndex = isf->texInfo;
		ts->mat = g_ms->registerMaterial(matName);
		ts->deluxemap = 0;
		u32 w = isf->lightmapTextureSizeInLuxels[0] + 1;
		u32 h = isf->lightmapTextureSizeInLuxels[1] + 1;
		u32 numSamples = w * h;
		if(isf->lightOfs != -1) {
			const byte *lmData = lightmaps + isf->lightOfs;
			// ensure that we have enough of bytes in the buffer
			if(rgbs.size() < numSamples * 3) {
				rgbs.resize(numSamples*3);
			}
			// convert each sample
			for(u32 j = 0; j < numSamples; j++) {
				byte *outColor = &rgbs[j*3];
				const byte *in = lmData + j * 4;
				if(0) {
					outColor[0] = in[0];
					outColor[1] = in[1];
					outColor[2] = in[2];
					continue;
				}
				// decode Source Engine color
				vec3_c rgb;
				// r,g,b are unsigned chars
				rgb.x = in[0];
				rgb.y = in[1];
				rgb.z = in[2];
				// exponent is signed
				float exp = ((char*)in)[3];
				float mult = pow(2.f,exp);
				rgb *= mult;
				rgb *= 255.f;
				// see if we have to normalize rgb into 0-255.f range
				if(rgb.getLargestAxisLen() > 255.f) {
					float l = rgb.getLargestAxisLen();
					float mult2 = 255.f/l;
					rgb *= mult2;
				}
				// save RGB color bytes
				outColor[0] = rgb.x;
				outColor[1] = rgb.y;
				outColor[2] = rgb.z;
			}
			//ts->lightmap = g_ms->createLightmap(rgbs.getArray(),w,h);
			ts->lightmap = (textureAPI_i*)(1 + la.addLightmap(rgbs.getArray(),w,h));
			//la.prepareNextLightmap();
		} else {
			ts->lightmap = 0;
		}
		q2PolyBuilder_c polyBuilder;
		for(int j = 0; j < isf->numEdges-1; j++) {
			int ei = surfEdges[isf->firstEdge + j];
			u32 realEdgeIndex;
			if(ei < 0) {
				realEdgeIndex = -ei;
				const srcEdge_s *ed = ie + realEdgeIndex;
				// reverse vertex order
				polyBuilder.addEdge(iv[ed->v[1]].point,iv[ed->v[0]].point,j);
			} else {
				realEdgeIndex = ei;
				const srcEdge_s *ed = ie + realEdgeIndex;
				polyBuilder.addEdge(iv[ed->v[0]].point,iv[ed->v[1]].point,j);
			}
		}
		polyBuilder.calcTexCoords(sfTexInfo->textureVecs,ts->mat->getImageWidth(),ts->mat->getImageHeight());
		if(ts->lightmap) {
			const sfLightmapDef_s &lightmapDef = la.getLightmapDef((u32)(ts->lightmap)-1);

			float invSize = 1.f / float(la.getLightmapSize());

			float ofsU = float(lightmapDef.x) * invSize;
			float scaleU = float(w) * invSize;

			float ofsV = float(lightmapDef.y) * invSize;
			float scaleV = float(h) * invSize;

			// calculate lightmap coordinates
			arraySTD_c<rVert_c> &pVerts = polyBuilder.getVerts();
			rVert_c *v = pVerts.getArray();
			for(u32 i = 0; i < pVerts.size(); i++, v++) {
				float tU = v->xyz[0]*sfTexInfo->lightmapVecs[0][0] + v->xyz[1]*sfTexInfo->lightmapVecs[0][1] + v->xyz[2]*sfTexInfo->lightmapVecs[0][2] + sfTexInfo->lightmapVecs[0][3];
				float tV = v->xyz[0]*sfTexInfo->lightmapVecs[1][0] + v->xyz[1]*sfTexInfo->lightmapVecs[1][1] + v->xyz[2]*sfTexInfo->lightmapVecs[1][2] + sfTexInfo->lightmapVecs[1][3];
				tU -= isf->lightmapTextureMinsInLuxels[0];
				tV -= isf->lightmapTextureMinsInLuxels[1];
				tU += 0.5f;
				tV += 0.5f;
				tU /= w;
				tV /= h;
				v->lc.set(ofsU+tU*scaleU,ofsV+tV*scaleV);
			}
		}

		ts->firstVert = verts.size();
		ts->numVerts = polyBuilder.getNumVerts();
		verts.addArray(polyBuilder.getVerts());
		polyBuilder.calcTriIndexes();
		polyBuilder.addVertsToBounds(ts->bounds);
		u32 *indicesU32 = ts->absIndexes.initU32(polyBuilder.getNumIndices());
		u16 *localIndicesU16 = ts->localIndexes.initU16(polyBuilder.getNumIndices());
		for(u32 j = 0; j < polyBuilder.getNumIndices(); j++) {
			u32 idx = polyBuilder.getIndex(j);;
			localIndicesU16[j] = idx;
			indicesU32[j] = ts->firstVert+idx;
		}
		if(isf18) {
			isf18++;
		} else {
			isf++;
		}
	}
	la.finalize();
	oSF = surfs.getArray();
	for(u32 i = 0; i < numSurfaces; i++, oSF++) {
		oSF->type = BSPSF_PLANAR;
		bspTriSurf_s *ts = oSF->sf;
		if(ts->lightmap) {
			u32 index = (u32)(ts->lightmap)-1;
			ts->lightmap = la.getSurfLightmap(index);
		}
	}
	return false; // OK
}
bool rBspTree_c::loadSurfsCoD() {
	loadVerts(COD1_DRAWVERTS);
	const cod1Surface_s *sf = (const cod1Surface_s *)h->getLumpData(COD1_SURFACES);
	u32 numSurfs = h->getLumpStructNum(COD1_SURFACES,sizeof(cod1Surface_s));
	surfs.resize(numSurfs);
	bspSurf_s *out = surfs.getArray();
	int highestLightmapNumReferenced = -1;
	// load bspMaterials contentFlags
	u32 numBSPMaterials = h->getNumMaterials();
	bspMaterials.resize(numBSPMaterials);
	for(u32 i = 0; i < numBSPMaterials; i++) {
		// TODO: convert content flags
		bspMaterials[i].contentFlags = h->getMat(i)->contentFlags;
	}
	// NOTE: CoD drawIndexes are 16 bit (not 32 bit like in Q3)
	const u16 *indices = (const u16 *) h->getLumpData(COD1_DRAWINDEXES);
	for(u32 i = 0; i < numSurfs; i++, sf++, out++) {
		const q3BSPMaterial_s *bspMaterial = h->getMat(sf->materialNum);;
		mtrAPI_i *mat = g_ms->registerMaterial(bspMaterial->shader);
		textureAPI_i *lightmap;
		if(sf->lightmapNum < 0) {
			lightmap = 0;
		} else {
			if(sf->lightmapNum >= lightmaps.size()) {
				lightmap = 0;
			} else {
				lightmap = lightmaps[sf->lightmapNum];
			}
		}
		if(sf->lightmapNum > highestLightmapNumReferenced) {
			highestLightmapNumReferenced = sf->lightmapNum;
		}
		if(mat->getSkyParms()) {
			RF_SetSkyMaterial(mat);
		}
		out->bspMaterialIndex = sf->materialNum;
		out->type = BSPSF_PLANAR; // is this really always a planar surface??
		bspTriSurf_s *ts = out->sf = new bspTriSurf_s;
		ts->mat = mat;
		ts->lightmap = lightmap;
		ts->deluxemap = 0;
		ts->firstVert = sf->firstVert;
		ts->numVerts = sf->numVerts;
		const u16 *firstIndex = indices + sf->firstIndex;
		// get the largest index value of this surface
		// to determine if we can use U16 index buffer
		u32 largestIndex = 0;
		for(u32 j = 0; j < sf->numIndexes; j++) {
			u32 idx = sf->firstVert + u32(firstIndex[j]);
			if(idx > largestIndex) {
				largestIndex = idx;
			}
		}
		ts->bounds.clear();
		if(largestIndex + 1 < U16_MAX) {
			g_core->Print("rBspTree_c::loadSurfs: using U16 index buffer for surface %i\n",i);
			u16 *u16Indexes = ts->absIndexes.initU16(sf->numIndexes);
			for(u32 j = 0; j < sf->numIndexes; j++) {
				u16Indexes[j] = sf->firstVert + firstIndex[j];	
				// update bounding box as well
				ts->bounds.addPoint(this->verts[u16Indexes[j]].xyz);
			}
		} else {
			g_core->Print("rBspTree_c::loadSurfs: using U32 index buffer for surface %i\n",i);
			u32 *u32Indexes = ts->absIndexes.initU32(sf->numIndexes);
			for(u32 j = 0; j < sf->numIndexes; j++) {
				u32Indexes[j] = sf->firstVert + firstIndex[j];
				// update bounding box as well	
				ts->bounds.addPoint(this->verts[u32Indexes[j]].xyz);
			}
		}
	}
	g_core->Print("rBspTree_c::loadSurfsCoD: highestLightmapNumReferenced: %i, numLightmaps %i\n",highestLightmapNumReferenced,lightmaps.size());
	return false; // ok
}
bool rBspTree_c::loadModels(u32 modelsLump) {
	const lump_s &ml = h->getLumps()[modelsLump];
	if(ml.fileLen % h->getModelStructSize()) {
		g_core->RedWarning("rBspTree_c::loadModels: invalid models lump size\n");
		return true; // error
	}
	u32 numModels = ml.fileLen / sizeof(q3Model_s);
	models.resize(numModels);
	const q3Model_s *m = (const q3Model_s *)h->getLumpData(modelsLump);
	bspModel_s *om = models.getArray();
	for(u32 i = 0; i < numModels; i++, om++) {
		om->firstSurf = m->firstSurface;
		om->numSurfs = m->numSurfaces;
		om->bb.fromTwoPoints(m->maxs,m->mins);

		m = h->getNextModel(m);
	}
	return false; // OK
}
bool rBspTree_c::loadModelsQ2(u32 modelsLump) {
	u32 lumpLen = h->getLumpSize(modelsLump);
	if(lumpLen % sizeof(q2Model_s)) {
		g_core->RedWarning("rBspTree_c::loadModelsQ2: invalid models lump size\n");
		return true; // error
	}
	u32 numModels = lumpLen / sizeof(q2Model_s);
	models.resize(numModels);
	const q2Model_s *m = (const q2Model_s *)h->getLumpData(modelsLump);
	bspModel_s *om = models.getArray();
	for(u32 i = 0; i < numModels; i++, om++, m++) {
		om->firstSurf = m->firstSurface;
		om->numSurfs = m->numSurfaces;
		om->bb.fromTwoPoints(m->maxs,m->mins);
	}
	return false; // OK
}
bool rBspTree_c::loadModelsHL(u32 modelsLump) {
	const lump_s &ml = h->getLumps()[modelsLump];
	if(ml.fileLen % sizeof(hlModel_s)) {
		g_core->RedWarning("rBspTree_c::loadModelsHL: invalid models lump size\n");
		return true; // error
	}
	u32 numModels = ml.fileLen / sizeof(hlModel_s);
	models.resize(numModels);
	const hlModel_s *m = (const hlModel_s *)h->getLumpData(modelsLump);
	bspModel_s *om = models.getArray();
	for(u32 i = 0; i < numModels; i++, om++, m++) {
		om->firstSurf = m->firstSurface;
		om->numSurfs = m->numSurfaces;
		om->bb.fromTwoPoints(m->maxs,m->mins);
	}
	return false; // OK
}
bool rBspTree_c::loadLeafIndexes(u32 leafSurfsLump) {
	const lump_s &sl = h->getLumps()[leafSurfsLump];
	if(sl.fileLen % sizeof(u32)) {
		g_core->RedWarning("rBspTree_c::loadLeafIndexes: invalid leafSurfaces lump size\n");
		return true; // error
	}	
	u32 numLeafSurfaces = sl.fileLen / sizeof(u32);
	leafSurfaces.resize(numLeafSurfaces);
	memcpy(leafSurfaces.getArray(),h->getLumpData(leafSurfsLump),sl.fileLen);
	return false;
}
bool rBspTree_c::loadLeafIndexes16Bit(u32 leafSurfsLump) {
	const lump_s &sl = h->getLumps()[leafSurfsLump];
	if(sl.fileLen % sizeof(u16)) {
		g_core->RedWarning("rBspTree_c::loadLeafIndexes16Bit: invalid leafSurfaces lump size\n");
		return true; // error
	}	
	u32 numLeafSurfaces = sl.fileLen / sizeof(u16);
	const u16 *inLeafSurfaces = (const u16*)h->getLumpData(leafSurfsLump);
	leafSurfaces.resize(numLeafSurfaces);

	for(u32 i = 0; i < numLeafSurfaces; i++) {
		leafSurfaces[i] = inLeafSurfaces[i];
	}
	return false;
}
bool rBspTree_c::loadVisibility(u32 visLump) {
	const lump_s &vl = h->getLumps()[visLump];
	if(vl.fileLen == 0) {
		g_core->Print(S_COLOR_YELLOW "rBspTree_c::loadVis: visibility lump is emtpy\n");
		return false; // dont do the error
	}
	vis = (visHeader_s*)malloc(vl.fileLen);
	memcpy(vis,h->getLumpData(visLump),vl.fileLen);
	return false; // no error
}
bool rBspTree_c::loadQ3LightGrid(u32 lightGridLump) {
	const lump_s &gridLump = h->getLumps()[lightGridLump];
	if(gridLump.fileLen == 0) {
		// map was compiled without grid lighting
		return false; // dont do the error
	}
	vec3_c lightGridSize(64,64,128);
	aabb worldBounds = this->models[0].bb;
	const byte *lightGridData = h->getLumpData(lightGridLump);
	q3BSPLightGrid_c *q3LightGrid = new q3BSPLightGrid_c;
	if(q3LightGrid->init(lightGridData,gridLump.fileLen,worldBounds,lightGridSize)) {
		g_core->Print(S_COLOR_YELLOW "rBspTree_c::loadQ3LightGrid: q3LightGrid->init error\n");
		delete q3LightGrid;
		return true; // error ?
	}
	this->lightGrid = q3LightGrid;
	return false;
}
void rBspTree_c::addPortalToArea(u32 areaNum, u32 portalNum) {
	if(areaNum >= areas.size()) {
		areas.resize(areaNum+1);
	}
	bspArea_c &a = areas[areaNum];
	a.portalNumbers.push_back(portalNum);
}
bool rBspTree_c::loadQioAreaPortals(u32 lumpNum) {
	const lump_s &vl = h->getLumps()[lumpNum];
	if(vl.fileLen == 0) {
	//	g_core->Print(S_COLOR_YELLOW "rBspTree_c::loadQioAreaPortals: areaportals lump is emtpy\n");
		return false; // dont do the error
	}
	u32 numAreaPortals = vl.fileLen / sizeof(dareaPortal_t);
	areaPortals.resize(numAreaPortals);
	areaPortalFrustums.resize(numAreaPortals);
	memcpy(areaPortals.getArray(),h->getLumpData(lumpNum),vl.fileLen);
	for(u32 i = 0; i < areaPortals.size(); i++) {
		const dareaPortal_t &p = areaPortals[i];
		addPortalToArea(p.areas[0],i);
		addPortalToArea(p.areas[1],i);
	}
	return false; // no error
}
bool rBspTree_c::loadQioPoints(u32 lumpNum) {
	const lump_s &vl = h->getLumps()[lumpNum];
	if(vl.fileLen == 0) {
	//	g_core->Print(S_COLOR_YELLOW "rBspTree_c::loadQioPoints: points lump is emtpy\n");
		return false; // dont do the error
	}
	u32 numPoints = vl.fileLen / sizeof(vec3_t);
	points.resize(numPoints);
	memcpy(points.getArray(),h->getLumpData(lumpNum),vl.fileLen);
	return false; // no error
}
bool rBspTree_c::load(const char *fname) {
	fileData = 0;
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		g_core->RedWarning("rBspTree_c::load: cannot open %s\n",fname);
		return true;
	}
	rf_bsp_forceEverythingVisible.setString("0");
	h = (const q3Header_s*) fileData;
	if(h->ident == BSP_IDENT_IBSP) {
		if((h->version == BSP_VERSION_Q3 || h->version == BSP_VERSION_ET)) {
			// Quake3 / ET / RTCW bsp
			if(loadExternalLightmaps(fname)) {
				if(loadLightmaps(Q3_LIGHTMAPS)) {
					g_vfs->FS_FreeFile(fileData);
					return true; // error
				}
			}
			if(loadSurfs(Q3_SURFACES, sizeof(q3Surface_s), Q3_DRAWINDEXES, Q3_DRAWVERTS, Q3_SHADERS, sizeof(q3BSPMaterial_s))) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadModels(Q3_MODELS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadNodesAndLeaves(Q3_NODES,Q3_LEAVES,sizeof(q3Leaf_s))) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadLeafIndexes(Q3_LEAFSURFACES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadPlanes(Q3_PLANES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadVisibility(Q3_VISIBILITY)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadQ3LightGrid(Q3_LIGHTGRID)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
		} else if(h->version == BSP_VERSION_Q2) {
			// QuakeII bsp
			if(loadPlanesQ2(Q2_PLANES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadNodesAndLeavesQ2(Q2_NODES,Q2_LEAFS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadModelsQ2(Q2_MODELS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadSurfsQ2()) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadLeafIndexes16Bit(Q2_LEAFFACES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
		} else if(h->version == BSP_VERSION_COD1) {
			// Call Of Duty .bsp file
			((q3Header_s*)h)->swapCoDLumpLenOfsValues();
			if(loadModels(COD1_MODELS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			// load CoD lightmaps
			if(loadLightmaps(COD1_LIGHTMAPS,512)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadSurfsCoD()) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadNodesAndLeaves(COD1_NODES,COD1_LEAFS,sizeof(cod1Leaf_s))) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadLeafIndexes(COD1_LEAFSURFACES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadPlanes(COD1_PLANES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			g_core->Print("%i cells\n",h->getLumpStructCount(17,52));
			g_core->Print("%i cull groups\n",h->getLumpStructCount(9,32));
			g_core->Print("%i portals\n",h->getLumpStructCount(18,16));
			g_core->Print("%i brushes\n",h->getLumpStructCount(4,4));
			g_core->Print("%i patchColls at %i\n",h->getLumpStructCount(24,16),h->getLumps()[24].fileOfs);
			// temporary fix for Call of Duty bsp's.
			// (there is something wrong with leafSurfaces)
			rf_bsp_forceEverythingVisible.setString("1");
		} else {
			g_core->RedWarning("rBspTree_c::load: IBSP has unknown version %i\n",h->version);
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else if(h->ident == BSP_IDENT_2015|| h->ident == BSP_IDENT_EALA) {
		// MoHAA/MoHBT/MoHSH bsp file
		if(loadLightmaps(MOH_LIGHTMAPS)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadSurfs(MOH_SURFACES, sizeof(q3Surface_s)+4, MOH_DRAWINDEXES, MOH_DRAWVERTS, MOH_SHADERS, sizeof(q3BSPMaterial_s)+64+4)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadModels(MOH_MODELS)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadNodesAndLeaves(MOH_NODES,MOH_LEAVES,sizeof(q3Leaf_s)+16)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadLeafIndexes(MOH_LEAFSURFACES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadPlanes(MOH_PLANES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadVisibility(MOH_VISIBILITY)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else if(h->ident == BSP_VERSION_HL || h->ident == BSP_VERSION_QUAKE1) {
		// Half Life and Counter Strike 1.6 bsps (de_dust2, etc)
		// older bsp versions dont have "ident" field
		// NOTE: HL_PLANES == Q2_PLANES.
		// The plane_t struct is the same as well
		if(loadPlanesQ2(HL_PLANES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadNodesAndLeavesHL(HL_NODES,HL_LEAFS)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		// hl model_t struct is different from Q2
		if(loadModelsHL(HL_MODELS)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadSurfsHL()) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadLeafIndexes16Bit(HL_MARKSURFACES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else if(h->ident == BSP_IDENT_VBSP) {
		// SourceEngine plane struct is the same as in Q2/Q1
		if(loadPlanesQ2(SRC_PLANES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}		
		if(loadModelsQ2(SRC_MODELS)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadLeafIndexes16Bit(SRC_LEAFFACES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadNodesAndLeavesSE()) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadSurfsSE()) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}	
	} else if(h->ident == BSP_IDENT_QIOBSP) {
		if(h->version == BSP_VERSION_QIOBSP) {
			if(loadLightmaps(Q3_LIGHTMAPS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadSurfs(Q3_SURFACES, sizeof(q3Surface_s), Q3_DRAWINDEXES, Q3_DRAWVERTS, Q3_SHADERS, sizeof(q3BSPMaterial_s))) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadModels(Q3_MODELS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadNodesAndLeaves(Q3_NODES,Q3_LEAVES,sizeof(q3Leaf_s))) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadLeafIndexes(Q3_LEAFSURFACES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadPlanes(Q3_PLANES)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadVisibility(Q3_VISIBILITY)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadQioPoints(QIO_POINTS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadQioAreaPortals(QIO_AREAPORTALS)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
			if(loadQ3LightGrid(Q3_LIGHTGRID)) {
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
		} else {
			g_core->RedWarning("rBspTree_c::load: QIO bsp has unknown version %i\n",h->version);
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else {
		g_core->RedWarning("rBspTree_c::load: unknown bsp type; cannot load %s\n",fname);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	g_vfs->FS_FreeFile(fileData);
	h = 0;
	fileData = 0;

	checkIsBSPValid();

	calcTangentVectors();
	createBatches();
	createVBOandIBOs();
	createRenderModelsForBSPInlineModels();

	return false;
}
bool rBspTree_c::checkIsBSPValid() {
	bool bIsOk = true;
	u32 highestSurfaceIndexInLeafSurfaces = 0;
	for(u32 i = 0; i < leafSurfaces.size(); i++) {
		u32 idx = leafSurfaces[i];
		if(idx > highestSurfaceIndexInLeafSurfaces) {
			highestSurfaceIndexInLeafSurfaces = idx;
		}
	}
	if(highestSurfaceIndexInLeafSurfaces+1 != surfs.size()) {
		g_core->RedWarning("rBspTree_c::checkIsBSPValid: surfs.size() mismatch\n");
		bIsOk = false;
	}
	u32 highestLeafSurfaceIndex = 0;
	for(u32 i = 0; i < leaves.size(); i++) {
		u32 idx = leaves[i].firstLeafSurface + leaves[i].numLeafSurfaces;
		if(idx > highestLeafSurfaceIndex) {
			highestLeafSurfaceIndex = idx;
		}
	}
	if(highestLeafSurfaceIndex != leafSurfaces.size()) {
		g_core->RedWarning("rBspTree_c::checkIsBSPValid: leafSurfaces mismatch\n");
		bIsOk = false;
	}

	return bIsOk; // ok
}
void rBspTree_c::clear() {
	//for(u32 i = 0; i < lightmaps.size(); i++) {
	//	delete lightmaps[i];
	//	lightmaps[i] = 0;
	//}
	if(vis) {
		free(vis);
		vis = 0;
	}
	if(lightGrid) {
		delete lightGrid;
		lightGrid = 0;
	}
}
int rBspTree_c::pointInLeaf(const vec3_c &pos) const {
	// special case for empty bsp trees
	if(nodes.size() == 0) {
		return 0;
	}
	int index = 0;
	do {
		const q3Node_s &n = nodes[index];
		const bspPlane_s &pl = planes[n.planeNum];
		float d = pl.distance(pos);
		if (d >= 0) 
			index = n.children[0]; // front
		else 
			index = n.children[1]; // back
	}
	while (index > 0) ;
	return -index - 1;
}
int rBspTree_c::pointInCluster(const vec3_c &pos) const {
	int leaf = pointInLeaf(pos);
	if(leaf < 0)
		return -1;
	return leaves[leaf].cluster;
}
// checks if one cluster is visible from another
bool rBspTree_c::isClusterVisible(int visCluster, int testCluster) const {
	if(rf_bsp_noVis.getInt())
		return true;
 	if(vis == 0)
		return true;
	if(visCluster < 0)
		return false;
	if(testCluster < 0)
		return false;
	byte visSet = vis->data[(visCluster * vis->clusterSize) + (testCluster >> 3)];
	return (visSet & (1<<(testCluster&7))) != 0;
}
void rBspTree_c::boxSurfaces_r(const aabb &bb, arraySTD_c<u32> &out, int nodeNum) const {
	while(nodeNum >= 0) {
		const q3Node_s &n = nodes[nodeNum];
		const bspPlane_s &pl = planes[n.planeNum];
		planeSide_e ps = pl.onSide(bb);
		if(ps == SIDE_FRONT) {
			nodeNum = n.children[0];
		} else if(ps == SIDE_BACK) {
			nodeNum = n.children[1];
		} else {
			nodeNum = n.children[0];
			boxSurfaces_r(bb,out,n.children[1]);
		}
	}
	int leafNum = -nodeNum - 1;
	const q3Leaf_s &l = leaves[leafNum];
	for(u32 i = 0; i < l.numLeafSurfaces; i++) {
		u32 sfNum = this->leafSurfaces[l.firstLeafSurface+i];
		if(this->surfs[sfNum].isFlare() == false && bb.intersect(this->surfs[sfNum].getBounds())) {
			out.add_unique(sfNum);
		}
	}
}
u32 rBspTree_c::boxSurfaces(const aabb &bb, arraySTD_c<u32> &out) const {
	boxSurfaces_r(bb,out,0);
	return out.size();
}
void rBspTree_c::boxAreas_r(const aabb &bb, arraySTD_c<u32> &out, int nodeNum) const {
	while(nodeNum >= 0) {
		const q3Node_s &n = nodes[nodeNum];
		const bspPlane_s &pl = planes[n.planeNum];
		planeSide_e ps = pl.onSide(bb);
		if(ps == SIDE_FRONT) {
			nodeNum = n.children[0];
		} else if(ps == SIDE_BACK) {
			nodeNum = n.children[1];
		} else {

			nodeNum = n.children[0];
			boxAreas_r(bb,out,n.children[1]);
		}
	}
	int leafNum = -nodeNum - 1;
	const q3Leaf_s &l = leaves[leafNum];
	if(l.area >= 0) {
		out.add_unique(l.area);
	}
}
u32 rBspTree_c::boxAreas(const aabb &bb, arraySTD_c<u32> &out) const {
	boxAreas_r(bb,out,0);
	return out.size();
}
#include <math/frustumExt.h>
void rBspTree_c::markAreas_r(int areaNum, const frustumExt_c &fr, dareaPortal_t *prevPortal) {
	if(areaNum >= areas.size() || areaNum < 0)
		return;
	bspArea_c *ar = &areas[areaNum];
	frustumAreaBits.set(areaNum,true);
	ar->portalVisCount = this->portalVisCount;

	for(u32 i = 0; i < ar->portalNumbers.size(); i++) {
		u32 portalNumber = ar->portalNumbers[i];
		dareaPortal_t *p = &areaPortals[portalNumber];
		if(p == prevPortal) {
			continue;
		}
		// first check if the portal is in the frustum
		if(fr.cull(*(const aabb*)p->bounds) == CULL_OUT)
			continue;
		// then check if the portal side facing camera
		// belong to current area. If not, portal is occluded
		// by the wall and is not really visible
		if(rf_bsp_cullBackFacingAreaPortals.getInt()) {
			float d = this->planes[p->planeNum].distance(rf_camera.getOrigin());
			if(p->areas[1] == areaNum) {
				if(d > 0)
					continue;
			} else {
				if(d < 0)
					continue;
			}
			if(rf_bsp_printCamera2PortalDist.getInt()) {
				g_core->Print("Area %i - portal %i - dist %f\n",areaNum,portalNumber,d);
			}
		}
		// adjust the frustum, so addAreaDrawCalls_r will never loop and cause a stack overflow...
		frustumExt_c adjusted;
		if(rf_bsp_noFrustumAdjust.getInt()) {
			adjusted = fr;
		} else {
			plane_c pl;
			pl.norm = -this->planes[p->planeNum].normal;
			pl.dist = this->planes[p->planeNum].dist;
			adjusted.adjustFrustum(fr,rf_camera.getOrigin(),points.getArray()+p->firstPoint,p->numPoints,pl);
		}
		if(adjusted.size() == 0) {
			// happens very often
			//g_core->RedWarning("rBspTree_c::markAreas_r: frustum chopped away\n");
			continue;
		}

		bspPortalData_c &pb = areaPortalFrustums[portalNumber];
		if(pb.portalVisCount != this->portalVisCount) {
			pb.portalVisCount = this->portalVisCount;
			pb.visitCount = 0;
		} else {
			// avoid endless loops (that would cause stack overflow)
			if(pb.visitCount >= MAX_PORTAL_VISIT_COUNT) {
				g_core->RedWarning("MAX_PORTAL_VISIT_COUNT!!!\n");
				continue;
			}
		}
		// save the frustum for latter culling
		pb.lastFrustum[pb.visitCount] = adjusted;
		pb.visitCount++;

		if(p->areas[0] == areaNum) {
			markAreas_r(p->areas[1],adjusted,p);
		} else {
			markAreas_r(p->areas[0],adjusted,p);
		}
	}
}
void rBspTree_c::markAreas() {
	if(areaPortals.size() == 0)
		return;
	if(rf_bsp_skipAreaPortals.getInt()) {
		frustumAreaBits.setAll(true);
		return;
	}
	this->portalVisCount++;
	int camLeaf = pointInLeaf(rf_camera.getPVSOrigin());
	if(camLeaf < 0) {
		return;
	}
	int camArea = leaves[camLeaf].area;
	frustumAreaBits.init(areas.size(),false);
	frustumExt_c baseFrustum(rf_camera.getFrustum());
	markAreas_r(camArea,baseFrustum,0);
}
void rBspTree_c::updateVisibility() {
	int camLeaf = pointInLeaf(rf_camera.getPVSOrigin());
	int camCluster, camArea;
	if(camLeaf < 0) {
		camCluster = -1;
		camArea = -1;
	} else {
		camCluster = leaves[camLeaf].cluster;
		camArea = leaves[camLeaf].area;
	}
	lastArea = camArea;
	if(camCluster == lastCluster && prevNoVis == rf_bsp_noVis.getInt()) {
		if(rf_bsp_rebuildBatchesOnAPVisChange.getInt()) {
			if(prevFrustumAreaBits.compare(frustumAreaBits) == false) {
				prevFrustumAreaBits = frustumAreaBits;
			} else {
				return;
			}
		} else {
			return;
		}
	}
	if(rf_bsp_printCamClusterNum.getInt()) {
		g_core->Print("rBspTree_c::updateVisibility(): cluster: %i\n",camCluster);
	}
	if(rf_bsp_printCamAreaNum.getInt()) {
		g_core->Print("rBspTree_c::updateVisibility(): area: %i\n",camArea);
	}
	prevNoVis = rf_bsp_noVis.getInt();
	lastCluster = camCluster;
	this->visCounter++;
	if(lastCluster == -1) {
		// if camera is outside map,
		// draw everything so we can locate BSP world
		for(u32 i = 0; i < surfs.size(); i++) {
			this->surfs[i].lastVisCount = this->visCounter;
		}
		return;
	}
	q3Leaf_s *l = leaves.getArray();
	int c_leavesCulledByPVS = 0;
	int c_leavesCulledByAreaBits = 0;
	if(rf_bsp_forceEverythingVisible.getInt() || (this->leafSurfaces.size()==0)) {
		for(u32 i = 0; i < surfs.size(); i++) {
			this->surfs[i].lastVisCount = this->visCounter;
		}
	} else {
		for(u32 i = 0; i < leaves.size(); i++, l++) {
			if(isClusterVisible(l->cluster,camCluster) == false) {
				c_leavesCulledByPVS++;
				continue; // skip leaves that are not visible
			}
			if(areaBits.get(l->area)) {
				c_leavesCulledByAreaBits++;
				continue;
			}
			if(areaPortals.size() && rf_bsp_rebuildBatchesOnAPVisChange.getInt()) {
				if(frustumAreaBits.get(l->area)==false) {
					continue;
				}
			}
			for(u32 j = 0; j < l->numLeafSurfaces; j++) {
				u32 sfNum = this->leafSurfaces[l->firstLeafSurface + j];
				if(sfNum >= this->surfs.size()) {
					g_core->RedWarning("rBspTree_c::updateVisibility: Surface index %i out of range <0,%i)\n",sfNum,this->surfs.size());
				} else {
					this->surfs[sfNum].lastVisCount = this->visCounter;
				}
			}
		}
	}
	u32 c_curBatchIndexesCount = 0;
	for(u32 i = 0; i < batches.size(); i++) {
		bspSurfBatch_s *b = batches[i];
		// see if we have to rebuild IBO of current batch
		bool changed = false;
		for(u32 j = 0; j < b->sfs.size(); j++) {
			bool visible = b->sfs[j]->lastVisCount == this->visCounter;
			bool wasVisible = b->lastVisSet.get(j);
			if(visible != wasVisible) {
				changed = true;
				b->lastVisSet.set(j,visible);
			}
		}
		if(changed == false) {
			c_curBatchIndexesCount += b->indices.getNumIndices();
			continue;
		}
		// recreate index buffer with the indices of potentially visible surfaces
		u32 newNumIndices = 0;
		b->bounds.clear();
		for(u32 j = 0; j < b->sfs.size(); j++) {
			bool isVisible = b->lastVisSet.get(j);
			if(isVisible == false) {
				continue;
			}
			bspTriSurf_s *sf = b->sfs[j]->sf;
			b->bounds.addBox(sf->bounds);
			for(u32 k = 0; k < sf->absIndexes.getNumIndices(); k++) {
				b->indices.setIndex(newNumIndices,sf->absIndexes[k]);
				newNumIndices++;
			}
		}
		b->indices.forceSetIndexCount(newNumIndices);
		b->indices.reUploadToGPU();

		c_curBatchIndexesCount += b->indices.getNumIndices();
	}

	if(rf_bsp_printVisChangeStats.getInt()) {
		int c_leavesInPVS = leaves.size()-c_leavesCulledByPVS;
		float leavesInPVSPercent = (float(c_leavesInPVS) / float(leaves.size()))*100.f;
		g_core->Print("rBspTree_c::updateVisibility: total leaf count %i, leaves in PVS: %i (%f percent)\n",
			leaves.size(),c_leavesInPVS,leavesInPVSPercent);
	}
	if(rf_bsp_printVisChangeStats.getInt()) {
		float usedIndicesPercent = (float(c_curBatchIndexesCount) / float(this->numBatchSurfIndexes))*100.f;
		g_core->Print("rBspTree_c::updateVisibility: active indices: %i, total %i (%f percent)\n",
			c_curBatchIndexesCount,numBatchSurfIndexes,usedIndicesPercent);
	}
}	
// alloc per-surface vertex buffer
// That's a big optimisation for q3 .shader effects
// which are done on CPU
void rBspTree_c::ensureSurfaceLocalVertsAllocated(bspTriSurf_s *stSF) {
	if(stSF->localVerts.size() == 0){
		stSF->localVerts.resize(stSF->numVerts);
		memcpy(stSF->localVerts.getArray(),this->verts.getArray()+stSF->firstVert,stSF->numVerts*sizeof(rVert_c));
	}
}
void rBspTree_c::addDrawCalls() {
	if(rf_bsp_drawBSPWorld.getInt() == 0)
		return;

	// use QioBSP areaPortals to mark which areas are intersecting player view frustum
	markAreas();
	// use Q3 BSP PVS to cull clusters and rebuild batches
	updateVisibility();

	u32 c_culledBatches = 0;
	// add batches made of planar bsp surfaces
	for(u32 i = 0; i < batches.size(); i++) {
		bspSurfBatch_s *b = batches[i];
		if(b->indices.getNumIndices() == 0)
			continue;
		if(rf_bsp_noFrustumCull.getInt() == 0 && rf_camera.getFrustum().cull(b->bounds) == CULL_OUT) {
			c_culledBatches++;
			continue;
		}
		if(areaPortals.size() && (rf_bsp_rebuildBatchesOnAPVisChange.getInt() == false)) {
			bool visible = false;
			for(u32 j = 0; j < b->areas.size(); j++) {
				if(frustumAreaBits.get(b->areas[j])==true) {
					visible = true;
					break;
				}
			}
			if(visible == false)
				continue;
		}
		if(rf_bsp_noSurfaces.getInt() == 0) {
			if(RF_MaterialNeedsCPU(b->mat) || rf_bsp_doAllSurfaceBatchesOnCPU.getInt()) {
				// Quake3 .shader files effects performs faster on small vertex arrays.
				// Don't use this->verts, use per-surface vertices instead
				for(u32 j = 0; j < b->sfs.size(); j++) {
					bspSurf_s *subSF = b->sfs[j];
					if(subSF->sf->numVerts == 0) {
						g_core->RedWarning("Found surface with numVerts == 0\n");
						continue;
					}
					if(subSF->lastVisCount != this->visCounter) {
						continue; // surface was culled by PVS
					}
					// cull separate surfaces
					if(rf_bsp_noFrustumCull.getInt() == 0 && rf_camera.getFrustum().cull(subSF->getBounds()) == CULL_OUT) {
						// we dont need this one
						continue;
					}
					//if(rf_camera.getPortalPlane().onSide(subSF->getBounds()) == SIDE_FRONT)
					//	continue;
					// add separate drawcalls
					bspTriSurf_s *stSF = subSF->sf;

					// alloc surface's private vertex buffer if its not present yet
					ensureSurfaceLocalVertsAllocated(stSF);

					if(rf_bsp_printCPUSurfVertsCount.getInt()) {
						g_core->Print("rBspTree_c::addDrawCalls()[%i]: %i verts, %i indices - mat %s, type %i\n",
							rf_curTimeMsec,stSF->localVerts.size(),stSF->localIndexes.getNumIndices(),b->mat->getName(),subSF->type);
					}
					RF_AddDrawCall(&stSF->localVerts,&stSF->localIndexes,b->mat,b->lightmap,b->mat->getSort(),true,b->deluxemap);
				}
			} else {
				if(rf_bsp_skipGPUSurfaces.getInt() == 0) {
					RF_AddDrawCall(&this->verts,&b->indices,b->mat,b->lightmap,b->mat->getSort(),true,b->deluxemap);
				}
			}
		}
	}
	u32 c_culledBezierPatches = 0;
	// add bezier patches (they are a subtype of bspSurf_s)
	bspSurf_s *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(sf->type == BSPSF_BEZIER) {
			if(sf->lastVisCount != this->visCounter) {
				continue; // culled by PVS
			}
			if(rf_bsp_noFrustumCull.getInt() == 0 && rf_camera.getFrustum().cull(sf->patch->getBB()) == CULL_OUT) {
				c_culledBezierPatches++;
				continue;
			}
			r_bezierPatch_c *p = sf->patch;
			if(rf_bsp_noBezierPatches.getInt() == 0) {
				p->addDrawCall();
			}
		}
	}
	if(rf_bsp_printFrustumCull.getInt()) {
		g_core->Print("%i patches and %i batches culled by frustum\n",c_culledBezierPatches,c_culledBatches);
	}
}
void rBspTree_c::addBSPSurfaceDrawCall(u32 sfNum) {
	bspSurf_s &sf = this->surfs[sfNum];
	if(sf.type == BSPSF_BEZIER) {
		sf.patch->addDrawCall();
	} else if(sf.type == BSPSF_FLARE) {
		// nothing to do for flares?
	} else {
		bspTriSurf_s *triSurf = sf.sf;
		if(RF_MaterialNeedsCPU(triSurf->mat)) {
			ensureSurfaceLocalVertsAllocated(triSurf);
			RF_AddDrawCall(&triSurf->localVerts,&triSurf->localIndexes,triSurf->mat,triSurf->lightmap,triSurf->mat->getSort(),true);
		} else {
#if 1
			// ensure that IBO is created
			if(triSurf->absIndexes.getInternalHandleVoid() == 0) {
				triSurf->absIndexes.uploadToGPU();
			}
#endif
			RF_AddDrawCall(&this->verts,&triSurf->absIndexes,triSurf->mat,triSurf->lightmap,triSurf->mat->getSort(),true);
		}
	}
}
#include "rf_shadowVolume.h"
void rBspTree_c::addBSPSurfaceToShadowVolume(u32 sfNum, const vec3_c &light, class rIndexedShadowVolume_c *staticShadowVolume, float lightRadius) {
	bspSurf_s &sf = this->surfs[sfNum];
	if(sf.type == BSPSF_BEZIER) {
		const r_surface_c *triSurf = sf.patch->getInstancePtr();
		staticShadowVolume->addRSurface(triSurf,light,0,lightRadius);
	//} else if(sf.type == BSPSF_PLANAR) {
	//	bspTriSurf_s *ts = sf.sf;
	//	float d = ts->plane.distance(light);
	//	if(d < 0)
	//		return;
	//	u32 prev = ts->numVerts-1;
	//	for(u32 i = 0; i < ts->numVerts; i++) {
	//		const vec3_c &v0 = verts[ts->firstVert+prev].xyz;
	//		const vec3_c &v1 = verts[ts->firstVert+i].xyz;
	//		staticShadowVolume->addEdge(v0,v1,light);
	//		prev = i;
	//	}
	//	staticShadowVolume->addFrontCapAndBackCapForIndexedVertsList(ts->absIndexes,this->verts,light);
	} else {
		bspTriSurf_s *ts = sf.sf;
		staticShadowVolume->addIndexedVertexList(ts->absIndexes,this->verts,light,0,lightRadius);
	}
}
void rBspTree_c::addModelDrawCalls(u32 inlineModelNum) {
	bspModel_s &m = models[inlineModelNum];
	for(u32 i = 0; i < m.numSurfs; i++) {
		addBSPSurfaceDrawCall(m.firstSurf+i);
	}
}
class mtrAPI_i *rBspTree_c::getSurfaceMaterial(u32 surfNum) const {
	const bspSurf_s &sf = surfs[surfNum];
	if(sf.type == BSPSF_BEZIER) {
		return sf.patch->getMat();
	}
	if(sf.type == BSPSF_PLANAR || sf.type == BSPSF_TRIANGLES)
		return sf.sf->mat;
	return 0;
}
void rBspTree_c::doDebugDrawing() {
	if(rf_bsp_showAreaPortals.getInt()) {
		for(u32 i = 0; i < areaPortals.size(); i++) {
			const dareaPortal_t &ap = areaPortals[i];
			u32 prev = ap.firstPoint + ap.numPoints - 1;
			for(u32 j = 0; j < ap.numPoints; j++) {
				u32 now = ap.firstPoint + j;
				rb->drawLineFromTo(this->points[now],this->points[prev],vec3_c(1,0,0));
				prev = now;
			}
		}
	}
}

#include "rf_decalProjector.h"
u32 rBspTree_c::createSurfDecals(u32 surfNum, class decalProjector_c &proj) const {
	u32 added = 0;
	const bspSurf_s &sf = surfs[surfNum];
	if(sf.type == BSPSF_PLANAR) {
		const rIndexBuffer_c &indices = sf.sf->absIndexes;
		for(u32 j = 0; j < indices.getNumIndices(); j+= 3) {
			u32 i0 = indices[j];
			u32 i1 = indices[j+1];
			u32 i2 = indices[j+2];
			const vec3_c &v0 = verts[i0].xyz;
			const vec3_c &v1 = verts[i1].xyz;
			const vec3_c &v2 = verts[i2].xyz;
			added += proj.clipTriangle(v0,v1,v2);
		}
	} else {
		
	}
	return added;
}	
int rBspTree_c::addWorldMapDecal(const vec3_c &pos, const vec3_c &normal, float radius, class mtrAPI_i *material) {
	decalProjector_c proj;
	proj.init(pos,normal,radius);
	proj.setMaterial(material);
	arraySTD_c<u32> sfNums;
	boxSurfaces(proj.getBounds(),sfNums);
	for(u32 i = 0; i < sfNums.size(); i++) {
		createSurfDecals(sfNums[i],proj);
	}
	proj.addResultsToDecalBatcher(RF_GetWorldDecalBatcher());
	return 0; // TODO: return valid decal handle?
}
bool rBspTree_c::cullBoundsByPortals(const aabb &absBB) {
	if(areaPortals.size() == 0)
		return false; // always visible, no areaportals on map
	if(rf_bsp_skipAreaPortals.getInt())
		return false; // ignoring areaportals

	arraySTD_c<u32> areaNums;
	boxAreas(absBB,areaNums);
	for(u32 i = 0; i < areaNums.size(); i++) {
		u32 areaNum = areaNums[i];
		if(areaNum == lastArea) {
			return false; // didnt cull
		}
	}
	for(u32 i = 0; i < areaNums.size(); i++) {
		u32 areaNum = areaNums[i];
		if(areaNum == lastArea) {
			continue;
		}
		if(areaNum >= areas.size()) {
			g_core->RedWarning("rBspTree_c::cullBoundsByPortals: areaNum >= areas.size() - ignored.\n");
			continue;
		}
		bspArea_c *ar = &areas[areaNum];
		if(ar->portalVisCount == this->portalVisCount) {
			//printf("Checking area %i with %i portals\n",areaNum,ar->portals.size());
			for(u32 j = 0; j < ar->portalNumbers.size(); j++) {
				u32 portalNum = ar->portalNumbers[j];
				bspPortalData_c &p = this->areaPortalFrustums[portalNum];
				if(p.portalVisCount == this->portalVisCount) 
				{
					//printf("Portal visicount %i\n",p->visitCount);
					for(u32 k = 0; k < p.visitCount; k++) {
						if(p.lastFrustum[k].cull(absBB) != CULL_OUT) {
							//printf("Area %i ent visible\n",areaNum);
							return false; // didnt cull
						}
					}
				}
			}
		}
	}
	// entity is not visible by player (culled by portals)
	return true;
}
void rBspTree_c::setWorldAreaBits(const byte *bytes, u32 numBytes) {
	if(areaBits.getSizeInBytes() == numBytes && !memcmp(areaBits.getArray(),bytes,numBytes)) {
		return; // no change
	}
	areaBits.fromBytes(bytes,numBytes);
	lastCluster = -4; // force updateVisibility refresh
}
bool rBspTree_c::traceSurfaceRay(u32 surfNum, class trace_c &out) {
	if(surfNum >= surfs.size()) {
		// it actually HAPPENS on CoD1 maps and I'm not sure why
		g_core->RedWarning("rBspTree_c::traceSurfaceRay: surface index %i out of range %i\n",surfNum,surfs.size());
		return false;
	}
	int contents = getSurfaceContentFlags(surfNum);
	if((contents & 1) == false)
		return false;
	bspSurf_s &sf = surfs[surfNum];
	if(sf.type == BSPSF_BEZIER) {
		r_bezierPatch_c *bp = sf.patch;
		if(bp->traceRay(out)) {
			out.setHitSurfaceNum(surfNum);
			return true; // collided with patch
		}
		return false; // didn't collide
	} else if(sf.type == BSPSF_FLARE) {
		return false; // never collide with flares
	} else {
		bool hasHit = false;
		bspTriSurf_s *t = sf.sf;
		if(out.getTraceBounds().intersect(t->bounds) == false)
			return false;
		for(u32 i = 0; i < t->absIndexes.getNumIndices(); i+=3) {
			u32 i0 = t->absIndexes[i+0];
			u32 i1 = t->absIndexes[i+1];
			u32 i2 = t->absIndexes[i+2];
			const rVert_c &v0 = this->verts[i0];
			const rVert_c &v1 = this->verts[i1];
			const rVert_c &v2 = this->verts[i2];
			if(out.clipByTriangle(v0.xyz,v1.xyz,v2.xyz,true)) {
				hasHit = true;
			}
		}
		if(hasHit) {
			out.setHitSurfaceNum(surfNum);
			out.setHitRMaterial(t->mat);
		}
		return hasHit;
	}
}
void rBspTree_c::traceNodeRay_r(int nodeNum, class trace_c &out) {
	if(nodeNum < 0) {
		// that's a leaf
		const q3Leaf_s &l = leaves[(-nodeNum-1)];
		for(u32 i = 0; i < l.numLeafSurfaces; i++) {
			u32 surfNum = this->leafSurfaces[l.firstLeafSurface + i];
			traceSurfaceRay(surfNum,out);
		}
		return; // done.
	}
	const q3Node_s &n = nodes[nodeNum];
	const bspPlane_s &pl = planes[n.planeNum];
	// classify ray against split plane
	float d0 = pl.distance(out.getStartPos());
	// hitPos is the actual endpos of the trace
	float d1 = pl.distance(out.getHitPos());

	if (d0 >= 0 && d1 >= 0) {
		// trace is on the front side of the plane
        traceNodeRay_r(n.children[0],out);
	} else if (d0 < 0 && d1 < 0) {
		// trace is on the back side of the plane
        traceNodeRay_r(n.children[1],out);
	} else {
		// trace crosses the plane - both childs must be checked.
		// TODO: clip the trace start/end points?
        traceNodeRay_r(n.children[0],out);
        traceNodeRay_r(n.children[1],out);
	}
}	
bool rBspTree_c::traceRay(class trace_c &out) {
	if(leafSurfaces.size() == 0 || (rf_bsp_useBSPForTracing.getInt()==0)) {
		bool bCollided = false;
		for(u32 i = 0; i < surfs.size(); i++) {
			if(traceSurfaceRay(i,out)) {
				bCollided = true;
			}
		}
		return bCollided;
	}

	float prevFrac = out.getFraction();
	traceNodeRay_r(0,out);
	if(out.getFraction() < prevFrac)
		return true;
	return false;
}
bool rBspTree_c::traceRayInlineModel(u32 inlineModelNum, class trace_c &out) {
	if(inlineModelNum == 0) {
		float initialFrac = out.getFraction();
		traceRay(out);
		if(initialFrac != out.getFraction())
			return true;
		return false;
	}
	bool hasHit = false;
	const bspModel_s &m = models[inlineModelNum];
	for(u32 i = 0; i < m.numSurfs; i++) {
		if(traceSurfaceRay(m.firstSurf+i,out)) {
			hasHit = true;
		}
	}
	return hasHit;
}	
bool rBspTree_c::createInlineModelDecal(u32 inlineModelNum, class simpleDecalBatcher_c *out, const class vec3_c &pos,
										const class vec3_c &normal, float radius, class mtrAPI_i *material) {
	decalProjector_c proj;
	proj.init(pos,normal,radius);
	proj.setMaterial(material);
	bool hasHit = false;
	const bspModel_s &m = models[inlineModelNum];
	for(u32 i = 0; i < m.numSurfs; i++) {
		if(createSurfDecals(m.firstSurf+i,proj)) {
			hasHit = true;
		}
	}
	proj.addResultsToDecalBatcher(out);
	return hasHit;
}
#include "rf_lights.h"
void rBspTree_c::cacheLightWorldInteractions(class rLightImpl_c *l) {
	arraySTD_c<u32> sfNums;
	boxSurfaces(l->getABSBounds(),sfNums);
	for(u32 i = 0; i < sfNums.size(); i++) {
		l->addBSPSurfaceInteraction(sfNums[i]);
	}
}
void rBspTree_c::getReferencedMatNames(class perStringCallbackListener_i *callback) const {
	for(u32 i = 0; i < surfs.size(); i++) {
		class mtrAPI_i *mat = getSurfaceMaterial(i);
		if(mat == 0)
			continue;
		callback->perStringCallback(mat->getName());
	}
}
bool rBspTree_c::getModelData(u32 modelNum, class staticModelCreatorAPI_i *out) const {
	if(modelNum >= models.size())
		return true;
	const bspModel_s &m = models[modelNum];
	for(u32 i = 0; i < m.numSurfs; i++) {
		const bspSurf_s &sf = surfs[m.firstSurf+i];
		if(sf.type == BSPSF_PLANAR || sf.type == BSPSF_TRIANGLES) {
			const bspTriSurf_s *ts = sf.sf;
			out->addSurface(ts->mat->getName(),this->verts.getArray()+ts->firstVert,ts->numVerts,ts->localIndexes.getU16Ptr(),ts->localIndexes.getNumIndices());
		} else {

		}
	}
	return false;
}
void rBspTree_c::setSurfaceMaterial(u32 surfaceNum, class mtrAPI_i *material) {
	if(surfaceNum >= surfs.size())
		return;
	bspSurf_s &sf = surfs[surfaceNum];
	//sf.bspMaterialIndex = this->registerMaterial(material);
	if(sf.sf) {
		sf.sf->mat = material;
	}
	rebuildBatches();
}
void rBspTree_c::setSurfaceMaterial(u32 surfaceNum, const char *matName) {
	setSurfaceMaterial(surfaceNum,g_ms->registerMaterial(matName));
}
rBspTree_c *RF_LoadBSP(const char *fname) {
	rBspTree_c *bsp = new rBspTree_c;
	if(bsp->load(fname)) {
		delete bsp;
		return 0;
	}
	return bsp;
}
