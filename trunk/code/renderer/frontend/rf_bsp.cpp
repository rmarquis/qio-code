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
#include <shared/shared.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/mtrAPI.h>

aCvar_c rf_bsp_noSurfaces("rf_bsp_noSurfaces","0");
aCvar_c rf_bsp_noBezierPatches("rf_bsp_noBezierPatches","0");
aCvar_c rf_bsp_drawBSPWorld("rf_bsp_drawBSPWorld","1");
aCvar_c rf_bsp_printFrustumCull("rf_bsp_printFrustumCull","0");
aCvar_c rf_bsp_printVisChangeStats("rf_bsp_printVisChangeStats","0");
aCvar_c rf_bsp_noVis("rf_bsp_noVis","0");
aCvar_c rf_bsp_forceEverythingVisible("rf_bsp_forceEverythingVisible","0");

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
}
rBspTree_c::~rBspTree_c() {
	clear();
}
void rBspTree_c::addSurfToBatches(u32 surfNum) {
	bspSurf_s *bs = &surfs[surfNum];
	if(bs->type != BSPSF_PLANAR && bs->type != BSPSF_TRIANGLES)
		return; // we're not batching bezier patches
	// ignore surfaces with 'sky' material; sky is drawn other way
	if(bs->sf->mat->getSkyParms() != 0)
		return;
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
		// TODO: merge only surfaces in the same cluster/area ? 
		b->addSurface(bs);
		return;
	}
	// create a new batch
	bspSurfBatch_s *nb = new bspSurfBatch_s;
	nb->initFromSurface(bs);
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
bool rBspTree_c::loadLightmaps(u32 lumpNum) {
	const lump_s &l = h->getLumps()[lumpNum];
	if(l.fileLen % (128*128*3)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadLightmaps: invalid lightmaps lump size\n");
		return true; // error
	}
	u32 numLightmaps = l.fileLen / (128*128*3);
	lightmaps.resize(numLightmaps);
	const byte *p = h->getLumpData(lumpNum);
	for(u32 i = 0; i < numLightmaps; i++) {
		textureAPI_i *t = lightmaps[i] = g_ms->createLightmap(p,128,128);
		p += (128*128*3);
	}
	return false; // OK
}
bool rBspTree_c::loadPlanes(u32 lumpPlanes) {
	const lump_s &pll = h->getLumps()[lumpPlanes];
	if(pll.fileLen % sizeof(q3Plane_s)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadPlanes: invalid planes lump size\n");
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
bool rBspTree_c::loadNodesAndLeaves(u32 lumpNodes, u32 lumpLeaves, u32 sizeOfLeaf) {
	const lump_s &nl = h->getLumps()[lumpNodes];
	if(nl.fileLen % sizeof(q3Node_s)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadNodesAndLeaves: invalid nodes lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(q3Node_s);
	nodes.resize(numNodes);
	memcpy(nodes.getArray(),h->getLumpData(lumpNodes),h->getLumpSize(lumpNodes));

	if(h->isBSPCoD1()) {
		const lump_s &ll = h->getLumps()[COD1_LEAFS];
		if(ll.fileLen % sizeof(cod1Leaf_s)) {
			g_core->Print(S_COLOR_RED "rBspTree_c::loadNodesAndLeaves: invalid leaves lump size\n");
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
			g_core->Print(S_COLOR_RED "rBspTree_c::loadNodesAndLeaves: invalid leaves lump size\n");
			return true; // error
		}
		u32 numLeaves = ll.fileLen / sizeOfLeaf;
		leaves.resize(numLeaves);
		memcpy_strided(leaves.getArray(),h->getLumpData(lumpLeaves),numLeaves,sizeof(q3Leaf_s),sizeof(q3Leaf_s),sizeOfLeaf);
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
	if(rb->getType() == BET_DX9) {
		for(u32 i = 0; i < numVerts; i++, ov++, iv++) {
			ov->xyz = iv->xyz;
			ov->tc = iv->st;
			ov->lc = iv->lightmap;
			ov->normal = iv->normal;
			// dx expects ARGB ....
			ov->color[0] = iv->color[3];
			ov->color[1] = iv->color[0];
			ov->color[2] = iv->color[1];
			ov->color[3] = iv->color[2];
		}
	} else {
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
	}
	return false; // no error
}
bool rBspTree_c::loadSurfs(u32 lumpSurfs, u32 sizeofSurf, u32 lumpIndexes, u32 lumpVerts, u32 lumpMats, u32 sizeofMat) {
	const lump_s &sl = h->getLumps()[lumpSurfs];
	if(sl.fileLen % sizeofSurf) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadSurfs: invalid surfs lump size\n");
		return true; // error
	}
	const lump_s &il = h->getLumps()[lumpIndexes];
	if(il.fileLen % sizeof(int)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadSurfs: invalid indexes lump size\n");
		return true; // error
	}
	const lump_s &vl = h->getLumps()[lumpVerts];
	if(vl.fileLen % sizeof(q3Vert_s)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadSurfs: invalid indexes lump size\n");
		return true; // error
	}
	const lump_s &ml = h->getLumps()[lumpMats];
	if(ml.fileLen % sizeofMat) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadSurfs: invalid material lump size\n");
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

	c_bezierPatches = 0;
	c_flares = 0;
	for(u32 i = 0; i < numSurfs; i++, out++) {
		const q3BSPMaterial_s *bspMaterial = (const q3BSPMaterial_s *)(materials + sizeofMat * sf->materialNum);
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
		if(mat->getSkyParms()) {
			RF_SetSkyMaterial(mat);
		}
		if(sf->surfaceType == Q3MST_PLANAR) {
			out->type = BSPSF_PLANAR;
parsePlanarSurf:;
			bspTriSurf_s *ts = out->sf = new bspTriSurf_s;
			ts->mat = mat;
			ts->lightmap = lightmap;
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
			c_flares++;
		}
		sf = (const q3Surface_s *) (((const byte*)sf)+sizeofSurf);
	}
	return false; // OK
}
bool rBspTree_c::loadSurfsCoD() {
	loadVerts(COD1_DRAWVERTS);
	const cod1Surface_s *sf = (const cod1Surface_s *)h->getLumpData(COD1_SURFACES);
	u32 numSurfs = h->getLumpStructNum(COD1_SURFACES,sizeof(cod1Surface_s));
	surfs.resize(numSurfs);
	bspSurf_s *out = surfs.getArray();
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
		if(mat->getSkyParms()) {
			RF_SetSkyMaterial(mat);
		}
		out->type = BSPSF_PLANAR; // is this really always a planar surface??
		bspTriSurf_s *ts = out->sf = new bspTriSurf_s;
		ts->mat = mat;
		ts->lightmap = lightmap;
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
	return false; // ok
}
bool rBspTree_c::loadModels(u32 modelsLump) {
	const lump_s &ml = h->getLumps()[modelsLump];
	if(ml.fileLen % h->getModelStructSize()) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadModels: invalid models lump size\n");
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
bool rBspTree_c::loadLeafIndexes(u32 leafSurfsLump) {
	const lump_s &sl = h->getLumps()[leafSurfsLump];
	if(sl.fileLen % sizeof(u32)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadLeafIndexes: invalid leafSurfaces lump size\n");
		return true; // error
	}	
	u32 numLeafSurfaces = sl.fileLen / sizeof(u32);
	leafSurfaces.resize(numLeafSurfaces);
	memcpy(leafSurfaces.getArray(),h->getLumpData(leafSurfsLump),sl.fileLen);
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
bool rBspTree_c::load(const char *fname) {
	fileData = 0;
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		g_core->Print(S_COLOR_RED "rBspTree_c::load: cannot open %s\n",fname);
		return true;
	}
	rf_bsp_forceEverythingVisible.setString("0");
	h = (const q3Header_s*) fileData;
	if(h->ident == BSP_IDENT_IBSP && (h->version == BSP_VERSION_Q3 || h->version == BSP_VERSION_ET)) {
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
	} else if(h->ident == BSP_IDENT_IBSP && h->version == BSP_VERSION_COD1) {
		// Call Of Duty .bsp file
		((q3Header_s*)h)->swapCoDLumpLenOfsValues();
		if(loadModels(COD1_MODELS)) {
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
		// temporary fix for Call of Duty bsp's.
		// (there is something wrong with leafSurfaces)
		rf_bsp_forceEverythingVisible.setString("1");
	} else {
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	g_vfs->FS_FreeFile(fileData);
	h = 0;
	fileData = 0;

	createBatches();
	createVBOandIBOs();
	createRenderModelsForBSPInlineModels();

	return false;
}
void rBspTree_c::clear() {
	for(u32 i = 0; i < lightmaps.size(); i++) {
		delete lightmaps[i];
		lightmaps[i] = 0;
	}
	if(vis) {
		free(vis);
		vis = 0;
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
		if(bb.intersect(this->surfs[sfNum].getBounds())) {
			out.add_unique(sfNum);
		}
	}
}
u32 rBspTree_c::boxSurfaces(const aabb &bb, arraySTD_c<u32> &out) const {
	boxSurfaces_r(bb,out,0);
	return out.size();
}
void rBspTree_c::updateVisibility() {
	int camCluster = pointInCluster(rf_camera.getOrigin());
	if(camCluster == lastCluster && prevNoVis == rf_bsp_noVis.getInt()) {
		return;
	}
	prevNoVis = rf_bsp_noVis.getInt();
	lastCluster = camCluster;
	this->visCounter++;
	q3Leaf_s *l = leaves.getArray();
	int c_leavesCulledByPVS = 0;
	int c_leavesCulledByAreaBits = 0;
	if(rf_bsp_forceEverythingVisible.getInt()) {
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
			for(u32 j = 0; j < l->numLeafSurfaces; j++) {
				u32 sfNum = this->leafSurfaces[l->firstLeafSurface + j];
				this->surfs[sfNum].lastVisCount = this->visCounter;
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
void rBspTree_c::addDrawCalls() {
	if(rf_bsp_drawBSPWorld.getInt() == 0)
		return;

	updateVisibility();

	u32 c_culledBatches = 0;
	// add batches made of planar bsp surfaces
	for(u32 i = 0; i < batches.size(); i++) {
		bspSurfBatch_s *b = batches[i];
		if(b->indices.getNumIndices() == 0)
			continue;
		if(rf_camera.getFrustum().cull(b->bounds) == CULL_OUT) {
			c_culledBatches++;
			continue;
		}
		if(rf_bsp_noSurfaces.getInt() == 0) {
			RF_AddDrawCall(&this->verts,&b->indices,b->mat,b->lightmap,b->mat->getSort(),true);
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
			if(rf_camera.getFrustum().cull(sf->patch->getBB()) == CULL_OUT) {
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
	} else {
		RF_AddDrawCall(&this->verts,&sf.sf->absIndexes,sf.sf->mat,sf.sf->lightmap,sf.sf->mat->getSort(),true);
	}
}
void rBspTree_c::addModelDrawCalls(u32 inlineModelNum) {
	bspModel_s &m = models[inlineModelNum];
	for(u32 i = 0; i < m.numSurfs; i++) {
		addBSPSurfaceDrawCall(m.firstSurf+i);
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
	if(material == 0) {
		material = g_ms->registerMaterial("defaultMaterial");
	}
	decalProjector_c proj;
	proj.init(pos,normal,radius);
	proj.setMaterial(material);
	arraySTD_c<u32> sfNums;
	boxSurfaces(proj.getBounds(),sfNums);
	for(u32 i = 0; i < sfNums.size(); i++) {
		createSurfDecals(sfNums[i],proj);
	}
	proj.addResultsToDecalBatcher(RF_GetWorldDecalBatcher());
	return 0;
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
	bspSurf_s &sf = surfs[surfNum];
	if(sf.type == BSPSF_BEZIER) {
		r_bezierPatch_c *bp = sf.patch;
		return bp->traceRay(out);
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
rBspTree_c *RF_LoadBSP(const char *fname) {
	rBspTree_c *bsp = new rBspTree_c;
	if(bsp->load(fname)) {
		delete bsp;
		return 0;
	}
	return bsp;
}
