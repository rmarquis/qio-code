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
// rAPI.h - renderer frontend interface

#ifndef __RF_API_H__
#define __RF_API_H__

#include "iFaceBase.h"

#define RENDERER_API_IDENTSTR "RendererAPI0001"

class rAPI_i : public iFaceBase_i {
public:
	// functions called every frame
	virtual void beginFrame() = 0;
	virtual void setup3DView(const class vec3_c &camPos, const vec3_c &camAngles) = 0;
	virtual void setupProjection3D(const struct projDef_s *pd) = 0;

	virtual class rEntityAPI_i *allocEntity() = 0;
	virtual void removeEntity(class rEntityAPI_i *ent) = 0;

	//virtual void registerRenderableForCurrentFrame(class iRenderable_c *r) = 0;
	//virtual void draw3DView() = 0;
	//virtual void setup2DView() = 0;
	virtual void set2DColor(const float *rgba) = 0;	// NULL = 1,1,1,1
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, class mtrAPI_i *material) = 0; // NULL = white
	virtual void endFrame() = 0;

	virtual void rayTraceWorldMap(class trace_c &tr) = 0;

	// misc functions	
	virtual void clearEntities() = 0;
	virtual void loadWorldMap(const char *mapName) = 0;
	virtual class mtrAPI_i *registerMaterial(const char *matName) = 0;
	virtual class rModelAPI_i *registerModel(const char *modName) = 0;

	virtual class rDebugDrawer_i *getDebugDrawer() = 0;

	virtual void init() = 0;
	virtual void endRegistration() = 0;
	virtual void shutdown(bool destroyWindow) = 0;
	virtual u32 getWinWidth() const = 0;
	virtual u32 getWinHeight() const = 0;
};

extern rAPI_i *rf;

#endif // __RF_API_H__
