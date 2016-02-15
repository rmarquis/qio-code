/*
============================================================================
Copyright (C) 2016 V.

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
// rf_terrain.cpp
#include "rf_surface.h"
#include <shared/array.h>


void calcNormal(vec3_c &n, const vec3_c &a, const vec3_c &b, const vec3_c &c) {
	vec3_c e0 = a - b;
	vec3_c e1 = a - c;
	e0.normalize();
	e1.normalize();
	n = e0.crossProduct(e1);
	n.normalize();
}

class terrainMod_c {
	vec3_c center;
	float maxDist;
	float force;
public:
	terrainMod_c() {
		maxDist = 128.f;
		force = 64.f;
	}
	virtual void processVertex(float x, float y, float &z) const {
		vec3_c v(x,y,z);
		float dist = center.distSQ(v);
		if(dist > Square(maxDist)) {
			return;
		}
		float f = 1.f - dist / Square(maxDist);
		z += f * force;
	}
};
class hmDesc_c {
protected:
	float stepX;
	float stepY;
	u32 w, h;
};
class heightmap_c : public hmDesc_c {
friend class heightmapInstance_c;
	arraySTD_c<float> heights;
public:
	void initFlat(float stepX, float stepY, u32 w, u32 h);
	float getHeight(u32 i) const {
		return heights[i];
	}
};
void heightmap_c::initFlat(float stepX, float stepY, u32 w, u32 h) {
	this->stepX = stepX;
	this->stepY = stepY;
	this->w = w;
	this->h = h;
	heights.resize(w*h);
}
class heightmapInstance_c : public hmDesc_c {
	arraySTD_c<vec3_c> xyzs;
	arraySTD_c<vec2_c> tcs;
	arraySTD_c<vec3_c> normals;
	aabb bb;
	float scale;
public:
	bool loadFromImage(const char *name, bool bGenTexCoords = false, bool bGenNormals = false);
	bool loadFromImage(const class image_c &img, bool bGenTexCoords = false, bool bGenNormals = false);
	bool initInstance(const heightmap_c &h, bool bGenTexCoords = false, bool bGenNormals = false);
	bool samplePosition(const class vec3_c &at, vec3_c &o, class plane_c *op) const;

	void addZ(float f) {
		for(u32 i = 0; i < xyzs.size(); i++) {
			xyzs[i].addZ(f);
		}
		bb.addZ(f);
	}
	void processTerrain(const class terrainMod_c &m) {
		for(u32 i = 0; i < xyzs.size(); i++) {
			vec3_c &v = xyzs[i];
			m.processVertex(v.x,v.y,v.z);
			bb.addPoint(v);
		}
	}
	void scaleXYZ(float f) {
		scale *= f;
		for(u32 i = 0; i < xyzs.size(); i++) {
			xyzs[i] *= f;
		}
		bb.scaleBB(f);
	}
	u32 getW() const {
		return w;
	}
	u32 getH() const {
		return h;
	}
	u32 getNumVertices() const {
		return xyzs.size();
	}
	const vec3_c *getXYZs() const {
		return xyzs.getArray();
	}
	const vec2_c *getTCs() const {
		return tcs.getArray();
	}
	const vec3_c *getNormals() const {
		return normals.getArray();
	}
};

bool heightmapInstance_c::initInstance(const heightmap_c &h, bool bGenTexCoords, bool bGenNormals) {
	this->stepX = h.stepX;
	this->stepY = h.stepY;
	this->w = h.w;
	this->h = h.h;
	u32 numVerts = this->w * this->h;
	xyzs.resize(numVerts);
	u32 v = 0;
	scale = 1.f;
	bb.clear();
	for(u32 i = 0; i < this->w; i++) {
		for(u32 j = 0; j < this->h; j++) {
			xyzs[v].set(i*stepX,j*stepY,h.heights[(v)]);
			bb.addPoint(xyzs[v]);
			v++;
		}
	}
	if(bGenTexCoords) {
		v = 0;
		tcs.resize(numVerts);
		for(u32 i = 0; i < this->w; i++) {
			for(u32 j = 0; j < this->h; j++) {
				tcs[v].set(float(i)/this->w,float(j)/this->h);
				v++;
			}
		}
	}
	if(bGenNormals) {
		normals.resize(numVerts);
		for(u32 i = 1; i < this->w; i++) {
			for(u32 j = 1; j < this->h; j++) {
				u32 i3 = (i-1) * this->h + j - 1;
				u32 i2 = (i) * this->h + j - 1;
				u32 i1 = (i) * this->h + j;
				u32 i0 = (i-1) * this->h + j;
				const vec3_c &v0 = xyzs[i0];
				const vec3_c &v1 = xyzs[i1];
				const vec3_c &v2 = xyzs[i2];
				const vec3_c &v3 = xyzs[i3];
				vec3_c nA, nB;
				calcNormal(nA,v0,v1,v2);
				calcNormal(nB,v2,v3,v0);
				normals[i0] += nA;
				normals[i1] += nA;
				normals[i2] += nA;

				normals[i2] += nB;
				normals[i3] += nB;
				normals[i0] += nB;
			}
		}
		for(u32 i = 0; i < numVerts; i++) {
			normals[i].normalize();
		}
	}

	return false;
}
class rTerrainPatch_c {
	r_surface_c sf;
public:
	void initTerrain(const heightmapInstance_c &hi) {
		sf.initTerrain(hi.getW(),hi.getH(),hi.getXYZs(),hi.getTCs(),hi.getNormals());
	}
	void addTerrainDrawCalls() {
		sf.addDrawCall();
	}
};
arraySTD_c<rTerrainPatch_c*> r_terrain;

rTerrainPatch_c *RFT_AllocTerrain() {
	rTerrainPatch_c *t = new rTerrainPatch_c();
	r_terrain.push_back(t);
	return t;
}
void RFT_InitTerrain() {
	heightmap_c h;
	h.initFlat(16.f,16.f,16,16);
	heightmapInstance_c hi;
	hi.initInstance(h,true,true);
	hi.addZ(10.f);
	terrainMod_c tm;
	hi.processTerrain(tm);
	RFT_AllocTerrain()->initTerrain(hi);
}
void RFT_AddTerrainDrawCalls() {
	for(u32 i = 0; i < r_terrain.size(); i++) {
		r_terrain[i]->addTerrainDrawCalls();
	}
}
void RFT_ShutdownTerrain() {
	for(u32 i = 0; i < r_terrain.size(); i++) {
		delete r_terrain[i];
	}
	r_terrain.clear();
}


