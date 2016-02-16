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
#include "rf_local.h"
#include "rf_drawCall.h"
#include <shared/array.h>
#include <shared/perlinNoise.h>
#include <api/materialSystemAPI.h>
#include <api/mtrAPI.h>
#include <shared/autocvar.h>

static aCvar_c rf_skipTerrain("rf_skipTerrain","0");

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
	void setForce(float f) {
		force = f;
	}
	void setMaxDist(float f) {
		maxDist = f;
	}
	void setOrigin(const vec3_c &no) {
		center = no;
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

	void applyNoise(class noise2DAPI_i *n, float scale) {
		for(u32 i = 0; i < xyzs.size(); i++) {
			vec3_c &v = xyzs[i];
			v.z += n->sampleNoise2D(v.x*scale,v.y*scale);
			bb.addPoint(v);
		}
	}
	void centerize() { 
		translate(-bb.getCenter());
	}
	void scaleTCs(float f) {
		for(u32 i = 0; i < xyzs.size(); i++) {
			tcs[i] *= (f);
		}
	}
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
	void translate(const vec3_c &v) {
		for(u32 i = 0; i < xyzs.size(); i++) {
			xyzs[i] += v;
		}
		bb.translate(v);
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
	const aabb &getBB() const {
		return bb;
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
				const vec3_c &v0 = xyzs[i3];
				const vec3_c &v1 = xyzs[i2];
				const vec3_c &v2 = xyzs[i1];
				const vec3_c &v3 = xyzs[i0];
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


class lodTerrainPatch_c {
friend class lodTerrain_c;
	aabb bounds;
	vec3_c center;
	lodTerrainPatch_c *top, *bottom, *right, *left;
	int curLOD;
	int savedLOD, savedLOD_top, savedLOD_bot, savedLOD_right, savedLOD_left;
	rIndexBuffer_c curIndices;
	// this is different only for border patches
	u32 sizeX, sizeY;
public:
	lodTerrainPatch_c() {
		curLOD = 0;
		savedLOD = -1;
	}
};
class lodTerrain_c {
	arraySTD_c<lodTerrainPatch_c> patches;
	rIndexBuffer_c curIndices;
	rVertexBuffer_c verts;
	u32 patchSizeEdges;
	u32 patchSizeVerts;
	u32 size;
	u32 patchCount;
	u32 maxLOD;
	float lodScale;
	class mtrAPI_i *mat;

	u32 calcIndex(u32 patchX, u32 patchY, u32 patchIndex, u32 vertexX, u32 vertexY) const;
	void calcPatchIndices(u32 patchX, u32 patchY);
public:
	lodTerrain_c();
	bool initLODTerrain(const class heightmapInstance_c &h, u32 lodPower, bool bExactSize);
	void scaleTexCoords(float s);
	void scaleXYZ(float x, float y, float z);

	void updateLOD(const class vec3_c &cam);
	void setLODScale(float newScale) {
		lodScale = newScale;
	}
	void addDrawCalls();
};
lodTerrain_c::lodTerrain_c() {
	lodScale = 1.f;
	mat = g_ms->registerMaterial("textures/qiotests/simplegrass");
}
void lodTerrain_c::calcPatchIndices(u32 patchX, u32 patchY) {

	u32 index = patchX * patchCount + patchY;
	lodTerrainPatch_c &p = patches[index];
	// see if there is a change
	int lod_bot = p.bottom != 0 ? p.bottom->curLOD : -1;
	int lod_top = p.top != 0 ? p.top->curLOD : -1;
	int lod_left = p.left != 0 ? p.left->curLOD : -1;
	int lod_right = p.right != 0 ? p.right->curLOD : -1;
	if(p.curLOD == p.savedLOD
		&&
		p.savedLOD_bot == lod_bot
		&&
		p.savedLOD_top == lod_top
		&&
		p.savedLOD_left == lod_left
		&&
		p.savedLOD_right == lod_right) {
		return;
	}
	p.savedLOD = p.curLOD;
	p.savedLOD_bot = lod_bot;
	p.savedLOD_top = lod_top;
	p.savedLOD_left = lod_left;
	p.savedLOD_right = lod_right;

	const u32 step = 1 << patches[index].curLOD;

	rIndexBuffer_c &indexBuffer = p.curIndices;
	indexBuffer.setNullCount();
	u32 x = 0;
	u32 z = 0;
	while (z < p.sizeY) {
		const u32 index11 = calcIndex(patchY, patchX, index, x, z);
		const u32 index21 = calcIndex(patchY, patchX, index, x + step, z);
		const u32 index12 = calcIndex(patchY, patchX, index, x, z + step);
		const u32 index22 = calcIndex(patchY, patchX, index, x + step, z + step);

		if(index11 >= verts.getNumVerts() || index12 >= verts.getNumVerts() || index21 >= verts.getNumVerts() || index22 >= verts.getNumVerts()) {
	//		printf("error");
		} else {
			indexBuffer.push_back(index12);
			indexBuffer.push_back(index11);
			indexBuffer.push_back(index22);
			indexBuffer.push_back(index22);
			indexBuffer.push_back(index11);
			indexBuffer.push_back(index21);
		}

		x += step;

		if (x >= patchSizeVerts) {
			x = 0;
			z += step;
		}
	}
	indexBuffer.uploadToGPU();
}
void lodTerrain_c::scaleTexCoords(float s) {
	verts.scaleTexCoords(s);
}
float g_randomColors[][3] = {
	{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 },

	{ 1, 1, 0 },
	{ 0, 1, 1 },
	{ 1, 0, 1 },

	{ 0.5, 1, 0.5 },
};
u32 g_numRandomColors = sizeof(g_randomColors)/sizeof(g_randomColors[0]);
void lodTerrain_c::scaleXYZ(float x, float y, float z) {
	verts.scaleXYZ(x,y,z);
	for(u32 i = 0; i < patches.size(); i++) {
		patches[i].center.scaleXYZ(x,y,z);
	}
}
void lodTerrain_c::updateLOD(const vec3_c &cam) {
	for(u32 i = 0; i < patchCount*patchCount; i++) {	
		const vec3_c &p = patches[i].center;
		float d = p.dist(cam);
		//if(1) {
		//	if(d > 100)
		//		patches[i].curLOD = 1;
		//	else 
		//		patches[i].curLOD = 0;
		//	continue;
		//}
		d *= lodScale;
		u32 lod = d / 800;
		if(lod > maxLOD)
			lod = maxLOD;
		patches[i].curLOD = lod;
	}
	for(u32 i = 0; i < patchCount; i++) {		
		for(u32 j = 0; j < patchCount; j++) {
			calcPatchIndices(i,j);
		}
	}
}
u32 lodTerrain_c::calcIndex(u32 patchX, u32 patchY, u32 patchIndex, u32 localVertexX, u32 localVertexY) const {
	if(patches[patchIndex].sizeX < localVertexY)
		localVertexY = patches[patchIndex].sizeX;
	if(patches[patchIndex].sizeY < localVertexX)
		localVertexX = patches[patchIndex].sizeY;

	if (localVertexY == 0) {
		if (patches[patchIndex].top &&
			patches[patchIndex].curLOD < patches[patchIndex].top->curLOD &&
			(localVertexX % (1 << patches[patchIndex].top->curLOD)) != 0 ) {
			localVertexX -= localVertexX % (1 << patches[patchIndex].top->curLOD);
		}
	} else if (localVertexY == (u32)patchSizeVerts) {
		if (patches[patchIndex].bottom &&
			patches[patchIndex].curLOD < patches[patchIndex].bottom->curLOD &&
			(localVertexX % (1 << patches[patchIndex].bottom->curLOD)) != 0) {
			localVertexX -= localVertexX % (1 << patches[patchIndex].bottom->curLOD);
		}
	}

	if (localVertexX == 0) {
		if (patches[patchIndex].left &&
			patches[patchIndex].curLOD < patches[patchIndex].left->curLOD &&
			(localVertexY % (1 << patches[patchIndex].left->curLOD)) != 0) {
			localVertexY -= localVertexY % (1 << patches[patchIndex].left->curLOD);
		}
	} else if (localVertexX == (u32)patchSizeVerts) {
		if (patches[patchIndex].right &&
			patches[patchIndex].curLOD < patches[patchIndex].right->curLOD &&
			(localVertexY % (1 << patches[patchIndex].right->curLOD)) != 0) {
			localVertexY -= localVertexY % (1 << patches[patchIndex].right->curLOD);
		}
	}

	if (localVertexY >= (u32)patchSizeEdges)
		localVertexY = patchSizeVerts;

	if (localVertexX >= (u32)patchSizeEdges)
		localVertexX = patchSizeVerts;
	
	//if(patches[patchIndex].sizeX < localVertexX)
	//	localVertexX = patches[patchIndex].sizeX;
	//if(patches[patchIndex].sizeY < localVertexY)
	//	localVertexY = patches[patchIndex].sizeY;

	return (localVertexY + ((patchSizeVerts) * patchY)) * size +
		(localVertexX + ((patchSizeVerts) * patchX));
}
bool lodTerrain_c::initLODTerrain(const class heightmapInstance_c &h, u32 lodPower, bool bExactSize) {
	verts.clear();
	patches.clear();

	this->maxLOD = lodPower;
	patchSizeVerts = 2 << maxLOD;
	patchSizeEdges = patchSizeVerts + 1;
	printf("Patch size verts: %i (for MAXLOD %i)\n",patchSizeVerts,maxLOD);
	verts.resize(h.getNumVertices());
	for(u32 i = 0; i < verts.size(); i++) {
		verts[i].xyz = h.getXYZs()[i];
		verts[i].tc = h.getTCs()[i];
		verts[i].normal = h.getNormals()[i];
	}
	if(bExactSize) {
		patchCount = ((h.getW()-1) / patchSizeVerts)+1;
	} else {
		patchCount = ((h.getW()-1) / patchSizeVerts);
	}
//	u32 checkSize = patchSizeVerts*patchCount + 1;
	size = h.getW();
	patches.resize(patchCount*patchCount);
	for(u32 i = 0; i < patchCount; i++) {
		for(u32 j = 0; j < patchCount; j++) {
			u32 index = i * patchCount + j;
			lodTerrainPatch_c &p = patches[index];
			u32 minX = i*patchSizeVerts;
			u32 maxX = minX+patchSizeVerts;
			u32 minY = j*patchSizeVerts;
			u32 maxZ = minY+patchSizeVerts;
			if(maxZ >= size) {
				u32 overflowZ = maxZ - (size-1);
				maxZ = size-1;
				p.sizeY = patchSizeVerts - overflowZ;
			} else {
				p.sizeY = patchSizeVerts;
			}
			if(maxX >= size) {
				u32 overflowX = maxX - (size-1);
				maxX = size-1;
				p.sizeX = patchSizeVerts - overflowX;
			} else {
				p.sizeX = patchSizeVerts;
			}
			for (u32 pX = minX; pX <= maxX; ++pX)
				for (u32 pY = minY; pY <= maxZ; ++pY)
					p.bounds.addPoint(verts.getXYZ(pX * size + pY));
			p.center = p.bounds.getCenter();
			p.top = i > 0 ? &patches[(i-1) * patchCount + j] : 0;
			p.bottom = (i < patchCount - 1) ? &patches[(i+1) * patchCount + j] : 0;
			p.left = (j > 0) ? &patches[i * patchCount + j - 1] : 0;
			p.right = (j < patchCount - 1) ? &patches[i * patchCount + j + 1] : 0;
		}
	}
	verts.uploadToGPU();
	return false;
}

void lodTerrain_c::addDrawCalls() {

	for(u32 i = 0; i < patches.size(); i++) {
		lodTerrainPatch_c &lp = patches[i];
		if(rf_camera.getFrustum().cull(lp.bounds)==CULL_OUT)
			continue;
		RF_AddDrawCall(&this->verts,&lp.curIndices,this->mat,0,this->mat->getSort(),0,0,0);
	}
}
class r_terrain_c {
	//r_surface_c sf;
	lodTerrain_c lt;
public:
	void initTerrain(const heightmapInstance_c &hi) {
		//sf.initTerrain(hi.getW(),hi.getH(),hi.getXYZs(),hi.getTCs(),hi.getNormals());
		lt.initLODTerrain(hi,5,false);
	}
	void addTerrainDrawCalls() {
		//sf.addDrawCall();
		lt.updateLOD(rf_camera.getOrigin());
		lt.addDrawCalls();
	}
};
arraySTD_c<r_terrain_c*> r_terrain;

r_terrain_c *RFT_AllocTerrain() {
	r_terrain_c *t = new r_terrain_c();
	r_terrain.push_back(t);
	return t;
}
void RFT_InitTerrain() {
	heightmap_c h;
	h.initFlat(16.f,16.f,128,128);
	heightmapInstance_c hi;
	hi.initInstance(h,true,true);
#if 1
	hi.centerize();
#else
	vec3_c tmp = hi.getBB().getSizes();
	tmp.x = 0;
	tmp.y *= -1;
	tmp.z = 0;
	hi.translate(tmp);
#endif
	hi.scaleTCs(100.f);
	hi.addZ(10.f);
	pn_c pn;
	pnDef_s pd;
	pd.amp = 1024.f;
	pn.setupPerlin(&pd);
	hi.applyNoise(&pn,0.0001f);
//	terrainMod_c tm;
//	hi.processTerrain(tm);
//	tm.setOrigin(vec3_c(64,64,64));
//	hi.processTerrain(tm);
	//for(u32 i = 0; i < 25; i++) {
	//	vec3_c p;
	//	hi.getBB().getRandomPointInside(p);
	//	tm.setOrigin(p);
	//	tm.setMaxDist(256+rand()%2048);
	//	tm.setForce(512 - rand()%2048);
	//	hi.processTerrain(tm);
	//}
	RFT_AllocTerrain()->initTerrain(hi);
}
void RFT_AddTerrainDrawCalls() {
	if(rf_skipTerrain.getInt())
		return;
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


