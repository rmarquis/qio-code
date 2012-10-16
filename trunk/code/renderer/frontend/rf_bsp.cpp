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
#include "rf_bsp.h"
#include "rf_bezier.h"
#include "rf_drawCall.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <api/materialSystemAPI.h>
#include <api/rbAPI.h>
#include <api/textureAPI.h>
#include <fileformats/bspFileFormat.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>

aCvar_c rf_bsp_noSurfaces("rf_bsp_noSurfaces","0");
aCvar_c rf_bsp_noBezierPatches("rf_bsp_noBezierPatches","0");

void memcpy_strided(void *_dest, const void *_src, int elementCount, int elementSize, int destStride, int sourceStride) {
	byte *dest = (byte*) _dest;
	byte *src = (byte*) _src;
	if(destStride == 0) {
		destStride = elementSize;
	}
	if(sourceStride == 0) {
		sourceStride = elementSize;
	}
	for(int i = 0; i < elementCount; i++) {
		memcpy(dest,src,elementSize);
		dest += destStride;
		src += sourceStride;
	}
}

rBspTree_c::rBspTree_c() {

}
rBspTree_c::~rBspTree_c() {
	clear();
}
void rBspTree_c::addSurfToBatches(u32 surfNum) {
	bspSurf_s *bs = &surfs[surfNum];
	if(bs->type != BSPSF_PLANAR && bs->type != BSPSF_TRIANGLES)
		return; // we're not batching bezier patches
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
		b->addSurface(sf);
		return;
	}
	// create a new batch
	bspSurfBatch_s *nb = new bspSurfBatch_s;
	nb->initFromSurface(sf);
	batches.push_back(nb);
}
void rBspTree_c::createBatches() {
	deleteBatches();
	u32 numWorldSurfs = models[0].numSurfs;
	for(u32 i = 0; i < numWorldSurfs; i++) {
		addSurfToBatches(i);
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
	const lump_s &ll = h->getLumps()[lumpLeaves];
	if(ll.fileLen % sizeOfLeaf) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadNodesAndLeaves: invalid leaves lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(q3Node_s);
	u32 numLeaves = ll.fileLen / sizeOfLeaf;
	nodes.resize(numNodes);
	leaves.resize(numLeaves);
	memcpy(nodes.getArray(),h->getLumpData(lumpNodes),h->getLumpSize(lumpNodes));
	memcpy_strided(leaves.getArray(),h->getLumpData(lumpLeaves),numLeaves,sizeof(q3Leaf_s),sizeof(q3Leaf_s),sizeOfLeaf);
	return false; // OK
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
	const q3Vert_s *iv = (const q3Vert_s*) h->getLumpData(lumpVerts);
	u32 numVerts = h->getLumpStructCount(lumpVerts,sizeof(q3Vert_s));
	verts.resize(numVerts);
	rVert_c *ov = verts.getArray();
	// convert vertices
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
bool rBspTree_c::loadModels(u32 modelsLump) {
	const lump_s &ml = h->getLumps()[modelsLump];
	if(ml.fileLen % sizeof(q3Model_s)) {
		g_core->Print(S_COLOR_RED "rBspTree_c::loadModels: invalid models lump size\n");
		return true; // error
	}
	u32 numModels = ml.fileLen / sizeof(q3Model_s);
	models.resize(numModels);
	const q3Model_s *m = (const q3Model_s *)h->getLumpData(modelsLump);
	bspModel_s *om = models.getArray();
	for(u32 i = 0; i < numModels; i++, m++, om++) {
		om->firstSurf = m->firstSurface;
		om->numSurfs = m->numSurfaces;
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
bool rBspTree_c::load(const char *fname) {
	fileData = 0;
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		g_core->Print(S_COLOR_RED "rBspTree_c::load: cannot open %s\n",fname);
		return true;
	}
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
	} else if(h->ident == BSP_IDENT_2015|| h->ident == BSP_IDENT_EALA) {
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
	} else {
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	g_vfs->FS_FreeFile(fileData);
	h = 0;
	fileData = 0;

	createBatches();
	createVBOandIBOs();

	return false;
}
void rBspTree_c::clear() {
	for(u32 i = 0; i < lightmaps.size(); i++) {
		delete lightmaps[i];
		lightmaps[i] = 0;
	}
}
void rBspTree_c::addDrawCalls() {
	if(1) {
		// no pvs
		for(u32 i = 0; i < batches.size(); i++) {
			bspSurfBatch_s *b = batches[i];
			if(rf_bsp_noSurfaces.getInt() == 0) {
				RF_AddDrawCall(&this->verts,&b->indices,b->mat,b->lightmap,DCS_OPAQUE_WORLD,true);
			}
		}
		bspSurf_s *sf = surfs.getArray();
		for(u32 i = 0; i < surfs.size(); i++, sf++) {
			if(sf->type == BSPSF_BEZIER) {
				r_bezierPatch_c *p = sf->patch;
				if(rf_bsp_noBezierPatches.getInt() == 0) {
					p->draw();
				}
			}
		}
	} 
}
void rBspTree_c::traceSurfaceRay(u32 surfNum, class trace_c &out) {
	bspSurf_s &sf = surfs[surfNum];
	if(sf.type == BSPSF_BEZIER) {
		r_bezierPatch_c *bp = sf.patch;
		bp->traceRay(out);
	} else {
		bspTriSurf_s *t = sf.sf;
		if(out.getTraceBounds().intersect(t->bounds) == false)
			return;
		for(u32 i = 0; i < t->absIndexes.getNumIndices(); i+=3) {
			u32 i0 = t->absIndexes[i+0];
			u32 i1 = t->absIndexes[i+1];
			u32 i2 = t->absIndexes[i+2];
			const rVert_c &v0 = this->verts[i0];
			const rVert_c &v1 = this->verts[i1];
			const rVert_c &v2 = this->verts[i2];
			out.clipByTriangle(v0.xyz,v1.xyz,v2.xyz,true);
		}
	}
}
void rBspTree_c::traceNodeRay(int nodeNum, class trace_c &out) {
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
        traceNodeRay(n.children[0],out);
	} else if (d0 < 0 && d1 < 0) {
		// trace is on the back side of the plane
        traceNodeRay(n.children[1],out);
	} else {
		// trace crosses the plane - both childs must be checked.
		// TODO: clip the trace start/end points?
        traceNodeRay(n.children[0],out);
        traceNodeRay(n.children[1],out);
	}
}	

void rBspTree_c::traceRay(class trace_c &out) {
	traceNodeRay(0,out);
}

rBspTree_c *RF_LoadBSP(const char *fname) {
	rBspTree_c *bsp = new rBspTree_c;
	if(bsp->load(fname)) {
		delete bsp;
		return 0;
	}
	return bsp;
}
