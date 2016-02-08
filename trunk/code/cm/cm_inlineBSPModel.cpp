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
// cm_inlineBSPModel.cpp
#include "cm_local.h"
#include "cm_model.h"
#include <shared/bspPhysicsDataLoader.h>
#include <api/coreAPI.h>

cMod_i *CM_LoadBSPFileSubModel(const bspPhysicsDataLoader_c *bsp, u32 subModelNumber) {
	if(subModelNumber >= bsp->getNumInlineModels()) {
		g_core->RedWarning("CM_LoadBSPFileSubModel: subModel index %i out of range <0,%i) - check bsp file %s\n",
			subModelNumber,bsp->getNumInlineModels(),bsp->getFileName());
		return 0;
	}
	str inlineModelName = va("*%i",subModelNumber);
	u32 numBrushes = bsp->getInlineModelBrushCount(subModelNumber);
	u32 nonBrushSurfaces = bsp->getInlineModelNonBrushSurfacesCount(subModelNumber);
	if(numBrushes == 1 && nonBrushSurfaces == 0) {
		// create single convex hull
		cmHull_c *retSingleBrush = new cmHull_c(inlineModelName);
		cmBrush_c &b = retSingleBrush->getMyBrush();
		u32 subModelBrushIndex = bsp->getInlineModelGlobalBrushIndex(subModelNumber,0);
		bsp->iterateBrushPlanes(subModelBrushIndex,&b);
		b.negatePlaneDistances();
		retSingleBrush->recalcBounds();
		CM_AddCObjectBaseToHashTable(retSingleBrush);
		return retSingleBrush;
	}
	// create a compound object made of multiple convex hull (brushes)
	cmCompound_c *retCompound = new cmCompound_c(inlineModelName);
	for(u32 i = 0; i < numBrushes; i++) {
		cmHull_c *brush = new cmHull_c(inlineModelName);
		cmBrush_c &b = brush->getMyBrush();
		u32 subModelBrushIndex = bsp->getInlineModelGlobalBrushIndex(subModelNumber,i);
		bsp->iterateBrushPlanes(subModelBrushIndex,&b);
		b.negatePlaneDistances();
		brush->recalcBounds();
		retCompound->addShape(brush);
	}
	// also add triangles of the bezier patches...
	if(nonBrushSurfaces) {
		u32 numSurfaces = bsp->getInlineModelSurfaceCount(subModelNumber);
		u32 firstSurface = bsp->getInlineModelFirstSurface(subModelNumber);
		for(u32 i = 0; i < numSurfaces; i++) {
			u32 sfIndex = firstSurface + i;
			u32 type = bsp->getSurfaceType(sfIndex);
			if(type != 1) {
				cmSurface_c *sf = new cmSurface_c;
				bsp->getTriangleSurface(sfIndex,*sf);
				char name[256];
				sprintf(name,"*%isf%i",subModelNumber,sfIndex);
				cmTriMesh_c *triMesh = new cmTriMesh_c(name,sf);
				retCompound->addShape(triMesh);
			}
		}
	}

	CM_AddCObjectBaseToHashTable(retCompound);
	return retCompound;
}
cMod_i *CM_LoadBSPFileSubModel(const char *bspFileName, u32 subModelNumber) {
#if 0
	return 0;
#endif
	bspPhysicsDataLoader_c l;
	if(l.loadBSPFile(bspFileName)) {
		str fixed = bspFileName;
		fixed.setExtension("bsp");
		if(l.loadBSPFile(fixed)) {
			fixed = "maps/";
			fixed.append(bspFileName);
			fixed.setExtension("bsp");
			if(l.loadBSPFile(fixed)) {
				g_core->RedWarning("CM_LoadBSPFileSubModel: cannot find BSP map %s\n",bspFileName);
				return 0;
			}
		}
	}
	cMod_i *ret = CM_LoadBSPFileSubModel(&l,subModelNumber);
	return ret;
}
