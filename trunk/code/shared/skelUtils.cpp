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
// skelUtils.cpp - helper functions for skeletal animation
// This file should be included in each module using bones system.
#include "skelUtils.h"
#include <math/quat.h>

void boneOrArray_c::localBonesToAbsBones(const class boneDefArray_c *boneDefs) {
	// TODO: validate the bones order and their names?

	const boneDef_s *def = boneDefs->getArray();
	boneOr_s *or = this->getArray();
	for(u32 i = 0; i < size(); i++, or++, def++) {
		if(def->parentIndex == -1) {
			// do nothing
		} else {
			matrix_c parent = (*this)[def->parentIndex].mat;
			or->mat = parent * or->mat;
		}
	}
}
void boneOrArray_c::setBlendResult(const boneOrArray_c &from, const boneOrArray_c &to, float frac) {
	// this works only if bones count and order in "from" is the same as in "to"
	this->resize(from.size());
	boneOr_s *or = this->getArray();
	const boneOr_s *orFrom = from.getArray();
	const boneOr_s *orTo = to.getArray();
	for(u32 i = 0; i < size(); i++, or++, orFrom++, orTo++) {
		quat_c qFrom, qTo;
		vec3_c pFrom, pTo;
		qFrom = orFrom->mat.getQuat();
		qTo = orTo->mat.getQuat();
		pFrom = orFrom->mat.getOrigin();
		pTo = orTo->mat.getOrigin();
		vec3_c p;
		p.lerp(pFrom,pTo,frac);
		quat_c q;
		q.slerp(qFrom,qTo,frac);
		or->mat.fromQuatAndOrigin(q,p);
		or->boneName = orTo->boneName;
	}
}
u32 boneOrArray_c::findNearestBone(const vec3_c &pos, float *outDist) const {
	u32 best = 0;
	float bestDist = getBonePos(0).distSQ(pos);
	for(u32 i = 1; i < size(); i++) {
		const vec3_c &other = getBonePos(i);
		float newDist = other.distSQ(pos);
		if(newDist < bestDist) {
			bestDist = newDist;
			best = i;
		}
	}
	if(outDist) {
		*outDist = bestDist;
	}
	return best;
}
void boneOrArray_c::scale(float scale) {
	boneOr_s *or = this->getArray();
	for(u32 i = 0; i < size(); i++, or++) {
		/////or->mat.scale(scale,scale,scale);
		vec3_c p = or->mat.getOrigin();
		p *= scale;
		or->mat.setOrigin(p);
	}
}
void boneOrArray_c::scaleXYZ(const vec3_c &vScaleXYZ) {
	boneOr_s *or = this->getArray();
	for(u32 i = 0; i < size(); i++, or++) {
		/////or->mat.scale(scale,scale,scale);
		vec3_c p = or->mat.getOrigin();
		p.scaleXYZ(vScaleXYZ.x,vScaleXYZ.y,vScaleXYZ.z);
		or->mat.setOrigin(p);
	}
}