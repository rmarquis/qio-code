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
// trace.cpp
#include "trace.h"
#include "collisionUtils.h"
#include <math/matrix.h>

void trace_c::setupRay(const vec3_c &newFrom, const vec3_c &newTo) {
	this->hitEntity = 0;
	this->clEntity = 0;
	this->hitREntity = 0;
	this->to = newTo;
	this->from = newFrom;
	this->hitPos = this->to;
	calcFromToDelta();
	this->fraction = 1.f;
	this->traveled = this->len;
	recalcRayTraceBounds();
}
void trace_c::setHitPos(const vec3_c &newHitPos) {
#if 1
	float newTraveled = this->from.dist(newHitPos);
	if(newTraveled > traveled)
		return;
#endif
	this->hitPos = newHitPos;
	updateForNewHitPos();
}
void trace_c::recalcRayTraceBounds() {
	traceBounds.reset(from);
	traceBounds.addPoint(hitPos);
	traceBounds.extend(0.25f);
}
void trace_c::updateForNewHitPos() {
	this->traveled = (hitPos - from).len();
	this->fraction = this->traveled / this->len;
	recalcRayTraceBounds();
}
void trace_c::updateForNewFraction() {
	this->traveled = this->fraction * this->len;
	this->hitPos = from + delta * fraction;
	recalcRayTraceBounds();
}
bool trace_c::clipByTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2, bool twoSided) {
	vec3_c newHit;
	int res = CU_RayTraceTriangle(from,hitPos,p0,p1,p2,&newHit);
	if(res != 1) {
		return false; 
	}
#if 1
	float checkLen = newHit.dist(from);
	if(checkLen >= this->traveled) {
		return false;
	}
#endif
	hitPos = newHit;
	this->updateForNewHitPos();
	this->hitPlane.fromThreePoints(p0,p1,p2);
	return true;
}
bool trace_c::clipByAABB(const aabb &bb) {
	vec3_c newHit;
	if(CU_IntersectLineAABB(this->from,this->to,bb,newHit)==false)
		return false;
#if 1
	float checkLen = newHit.dist(from);
	if(checkLen >= this->traveled) {
		return false;
	}
#endif
	hitPos = newHit;
	this->updateForNewHitPos();
	return true;
}
void trace_c::getTransformed(trace_c &out, const matrix_c &entityMatrix) const {
	matrix_c inv = entityMatrix;
	inv.inverse();
	inv.transformPoint(this->from,out.from);
	inv.transformPoint(this->to,out.to);
	out.fraction = this->fraction;
	out.calcFromToDelta();
	out.updateForNewFraction();
}

void trace_c::updateResultsFromTransformedTrace(trace_c &selfTransformed) {
	if(selfTransformed.fraction >= this->fraction)
		return;
	this->hitEntity = selfTransformed.hitEntity;
	this->hitREntity = selfTransformed.hitREntity;
	this->clEntity = selfTransformed.clEntity;
	this->fraction = selfTransformed.fraction;
	this->updateForNewFraction();
}
