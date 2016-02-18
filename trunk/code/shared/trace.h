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
// trace.h
#ifndef __TRACE_H__
#define __TRACE_H__

#include "../math/vec3.h"
#include "../math/aabb.h"
#include "../math/plane.h"

class trace_c {
	// input
	vec3_c from;
	vec3_c to;
	// only for sphere traces
	float sphereRadius;
	// derived from input
	aabb traceBounds;
	vec3_c delta;
	float len;
	// output
	vec3_c hitPos;
	float fraction; // 1.f == didnt hit anything
	float traveled;
	plane_c hitPlane;

	//union {
		// SERVERSIDE trace hit info
		class BaseEntity *hitEntity; // for game module
		// CLIENTSIDE trace hit info
		struct centity_s *clEntity; // for cgame module
		class rEntityAPI_i *hitREntity; // for cgame and renderer
		class mtrAPI_i *hitRMaterial; // for cgame and renderer
		// for renderer
		class lodTerrain_c *hitRTerrain;
		class lodTerrainPatch_c *hitRTerrainPatch;
	//};
	u32 hitTriangleIndex;
	// -1 if not set
	int hitSurfaceNum;
	// -1 if not set
	int hitAreaNum;

	void updateForNewHitPos();
	void updateForNewFraction();
	inline void calcFromToDelta() {
		this->delta = this->to - this->from;
		this->len = this->delta.len();
	}
public:
	trace_c();

	void recalcRayTraceBounds();
	void setupRay(const vec3_c &newFrom, const vec3_c &newTo);
	void setSphereRadius(float newRad) {
		this->sphereRadius = newRad;
	}
	float getSphereRadius() const {
		return this->sphereRadius;
	}
	void setHitPos(const vec3_c &newHitPos);
	void setHitTriangleIndex(u32 newHitTriIndex) {
		hitTriangleIndex = newHitTriIndex;
	}
	u32 getHitTriangleIndex() const {
		return hitTriangleIndex;
	}
	void setHitSurfaceNum(int newSurfaceNum) {
		hitSurfaceNum = newSurfaceNum;
	}
	int getHitSurfaceNum() const {
		return hitSurfaceNum;
	}
	bool hasHitSurfaceIndexStored() const {
		return (hitSurfaceNum != -1);
	}
	void setHitAreaNum(int newAreaNum) {
		hitAreaNum = newAreaNum;
	}
	int getHitAreaNum() const {
		return hitAreaNum;
	}
	bool hasHitAreaIndexStored() const {
		return (hitAreaNum != -1);
	}

	bool clipByTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2, bool twoSided = false);
	bool clipByAABB(const aabb &bb);

	// transforms world-space trace into entity-space trace
	void getTransformed(trace_c &out, const class matrix_c &entityMatrix) const;
	void updateResultsFromTransformedTrace(trace_c &selfTransformed, const class matrix_c &entityMatrix);


	void setHitEntity(class BaseEntity *newHitEntity) {
		hitEntity = newHitEntity;
	}
	void setHitCGEntity(struct centity_s *newHitCGEntity) {
		clEntity = newHitCGEntity;
	}
	void setHitREntity(class rEntityAPI_i *newHitREnt) {
		hitREntity = newHitREnt;
		hitRTerrain = 0;
		hitRTerrainPatch = 0;
	}
	void setHitTerrain(class lodTerrain_c *nt, class lodTerrainPatch_c *np) {
		hitREntity = 0;
		hitRTerrain = nt;
		hitRTerrainPatch = np;
	}
	void setHitRMaterial(class mtrAPI_i *newHitRMaterial) {
		hitRMaterial = newHitRMaterial;
	}
	void setFraction(float newFrac) {
		this->fraction = newFrac;
	}
	class BaseEntity *getHitEntity() const {
		return hitEntity;
	}
	struct centity_s *getHitCGEntity() const {
		return clEntity;
	}
	class rEntityAPI_i *getHitREntity() const {
		return hitREntity;
	}
	class mtrAPI_i *getHitRMaterial() const {
		return hitRMaterial;
	}
	class lodTerrain_c *getHitTerrain() const {
		return hitRTerrain;
	}
	const vec3_c &getStartPos() const {
		return from;
	}
	const vec3_c &getFrom() const {
		return from;
	}
	const vec3_c &getTo() const {
		return to;
	}
	const vec3_c &getHitPos() const {
		return hitPos;
	}
	const vec3_c &getHitPlaneNormal() const {
		return hitPlane.norm;
	}
	const vec3_c getDir() const {
		return delta.getNormalized();
	}
	float getFraction() const {
		return fraction;
	}
	float getTraveled() const {
		return traveled;
	}
	const aabb &getTraceBounds() const {
		return traceBounds;
	}
	bool hasHit() const {
		return fraction != 1.f;
	}

};

#endif // __TRACE_H__
