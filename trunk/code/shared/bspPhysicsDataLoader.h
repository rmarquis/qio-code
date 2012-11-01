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
// bspPhysicsDataLoader.h - simple loaded for bsp brushes/surfaces
// This is for physics system only, so texture coordinate, vertex normals,
// lightmaps, etc are ignored.
#ifndef __BSPPHYSICSDATALOADER_H__
#define __BSPPHYSICSDATALOADER_H__

#include "typedefs.h"

class bspPhysicsDataLoader_c {
	struct q3Header_s *h;

	u32 getMaterialContentFlags(u32 matNum) const;
public:
	bspPhysicsDataLoader_c();
	~bspPhysicsDataLoader_c();

	void clear();
	bool loadBSPFile(const char *fname);

	void iterateModelBrushes(u32 modelNum, void (*perBrushCallback)(u32 brushNum, u32 contentFlags));
	void iterateModelTriSurfs(u32 modelNum, void (*perSurfCallback)(u32 surfNum, u32 contentFlags));
	void iterateModelBezierPatches(u32 modelNum, void (*perBezierPatchCallback)(u32 surfNum, u32 contentFlags));

	void iterateBrushPlanes(u32 brushNum, void (*sideCallback)(const float planeEq[4]));
	void convertBezierPatchToTriSurface(u32 surfNum, u32 tesselationLevel, class cmSurface_c &out);
	void getTriangleSurface(u32 surfNum, class cmSurface_c &out);

	u32 getNumInlineModels() const;
	void getInlineModelBounds(u32 modelNum, class aabb &bb) const;

	bool isCoD1BSP() const;
};

#endif // __BSPPHYSICSDATALOADER_H__
