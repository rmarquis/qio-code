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
// rf_bsp.h - header for rBspTree_c class
#ifndef __RF_BSP_H__
#define __RF_BSP_H__

#include <shared/typedefs.h>
#include <math/aabb.h>
#include "../rVertexBuffer.h"
#include "../rIndexBuffer.h"

enum bspSurfaceType_e {
	BSPSF_PLANAR,
	BSPSF_BEZIER,
	BSPSF_TRIANGLES,
};
struct bspTriSurf_s {
	class mtrAPI_i *mat;
	class textureAPI_i *lightmap;
	// indexes to rBspTree_c::verts array (global vertices), for batching
	rIndexBuffer_c absIndexes;
};
struct bspSurfBatch_s {
	// we can only merge surfaces with the same material and lightmap....
	class mtrAPI_i *mat;
	class textureAPI_i *lightmap;
	// surfaces to merge
	arraySTD_c<bspTriSurf_s*> sfs;
	// this index buffer will be recalculated and reuploaded to GPU
	// everytime a new vis cluster is entered.
	rIndexBuffer_c indices;
	// bounds are recalculated as well
	aabb bounds;

	void addSurface(bspTriSurf_s *nSF) {
		sfs.push_back(nSF);
		indices.addIndexBuffer(nSF->absIndexes);
	}
	void initFromSurface(bspTriSurf_s *nSF) {
		mat = nSF->mat;
		lightmap = nSF->lightmap;
		addSurface(nSF);
	}
};
struct bspSurf_s {
	bspSurfaceType_e type;
	union {
		struct bspTriSurf_s *sf; // only if type == BSPSF_PLANAR || type == BSPSF_TRIANGLES
		class r_bezierPatch_c *patch; // only if type == BSPSF_BEZIER
	};
};
struct bspModel_s {
	u32 firstSurf;
	u32 numSurfs;
};
struct bspPlane_s {
	float normal[3];
	float dist;
};
class rBspTree_c {
	byte *fileData;
	const struct q3Header_s *h;
	u32 c_bezierPatches;
	u32 c_flares;

	rVertexBuffer_c verts;
	arraySTD_c<textureAPI_i*> lightmaps;
	arraySTD_c<bspSurf_s> surfs;
	arraySTD_c<bspSurfBatch_s*> batches;
	arraySTD_c<bspModel_s> models;
	arraySTD_c<bspPlane_s> planes;

	void addSurfToBatches(u32 surfNum);
	void createBatches();
	void deleteBatches();

	bool loadPlanes(u32 lumpPlanes);
	bool loadNodesAndLeaves(u32 lumpNodes, u32 lumpLeaves, u32 sizeOfLeaf);
	bool loadSurfs(u32 lumpSurfs, u32 sizeofSurf, u32 lumpIndexes, u32 lumpVerts, u32 lumpMats, u32 sizeofMat);
	bool loadModels(u32 modelsLump);
public:
	bool load(const char *fname);

	void addDrawCalls();
};

rBspTree_c *RF_LoadBSP(const char *fname);

#endif // __RF_BSP_H__

