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
// rf_terrain.h

class lodTerrainPatch_c {
friend class lodTerrain_c;
	aabb bounds;
	vec3_c center;
	class lodTerrainPatch_c *top, *bottom, *right, *left;
	int curLOD;
	int savedLOD, savedLOD_top, savedLOD_bot, savedLOD_right, savedLOD_left;
	rIndexBuffer_c curIndices;
	class textureAPI_i *blendMap;
	// this is different only for border patches
	u32 sizeX, sizeY;
public:
	lodTerrainPatch_c() {
		curLOD = 0;
		savedLOD = -1;
		blendMap = 0;
	}
};
class lodTerrain_c {
	arraySTD_c<lodTerrainPatch_c> patches;
	rIndexBuffer_c curIndices;
	rVertexBuffer_c verts;
	u32 patchSizeEdges;
	u32 patchSizeVerts;
	u32 sizeX;
	u32 sizeY;
	u32 patchCountX;
	u32 patchCountY;
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
	void setMaterial(mtrAPI_i *mat);
	void setTexDef(const texCoordCalc_c &tc);
	bool traceRay(class trace_c &tr);
	void applyMod(const class terrainMod_c &m);
	void applyPaint(const vec3_c &p, const byte rgba[4], float innerRadius, float outerRadius);
	void recalcBounds();
};

class r_terrain_c {
	//r_surface_c sf;
	lodTerrain_c lt;
public:
	void initTerrain(const heightmapInstance_c &hi, u32 lodPower);
	void setTexDef(const texCoordCalc_c &tc);
	void addTerrainDrawCalls();
	bool traceRay(class trace_c &tr);
	void setMaterial(mtrAPI_i *mat) ;
	void setMaterial(const char *matName);
};