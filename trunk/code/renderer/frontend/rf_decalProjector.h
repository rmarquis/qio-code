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
// rf_decalProjector.h
#ifndef __RF_DECALPROJECTOR_H__
#define __RF_DECALPROJECTOR_H__

#include <math/aabb.h>
#include <math/plane.h>
#include <shared/cmWinding.h>
#include <shared/simpleTexturedPoly.h>
#include <api/boxTrianglesCallback.h>

class decalProjector_c : public boxTrianglesCallback_i {
	plane_c planes[6];
	aabb bounds;
	arraySTD_c<cmWinding_c> results;
	mtrAPI_i *mat;
	vec3_c inPos;
	vec3_c inNormal;
	float inRadius;
	vec3_c perp, perp2; // vectors perpendicular to inNormal
public:
	decalProjector_c();
	void init(const vec3_c &pos, const vec3_c &normal, float radius);
	void setMaterial(class mtrAPI_i *newMat);
	u32 clipTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2);
	void iterateResults(void (*untexturedTriCallback)(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2));
	void iterateResults(class staticModelCreatorAPI_i *smc);
	void addResultsToDecalBatcher(class simpleDecalBatcher_c *batcher);
	const aabb &getBounds() const;
	u32 getNumCreatedWindings() const {
		return results.size();
	}

	// boxTrianglesCallback_i impl
	virtual void onBoxTriangle(const class vec3_c &p0, const class vec3_c &p1, const class vec3_c &p2) {
		clipTriangle(p0,p1,p2);
	}
};

#endif // __RF_DECALPROJECTOR_H__
