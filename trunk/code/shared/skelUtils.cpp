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

void boneOr_s::setBlendResult(const boneOr_s &from, const boneOr_s &to, float frac) {
	quat_c qFrom, qTo;
	vec3_c pFrom, pTo;
	qFrom = from.mat.getQuat();
	qTo = to.mat.getQuat();
	pFrom = from.mat.getOrigin();
	pTo = to.mat.getOrigin();
	vec3_c p;
	p.lerp(pFrom,pTo,frac);
	quat_c q;
	q.slerp(qFrom,qTo,frac);
	this->mat.fromQuatAndOrigin(q,p);
	this->boneName = to.boneName;
}

void boneOrArray_c::absBonesToLocalBones(const class boneDefArray_c *boneDefs, const boneOrArray_c *absBones) {
	// TODO: validate the bones order and their names?
	this->resize(boneDefs->size());
	boneOr_s *out = this->getArray();
	const boneOr_s *in = absBones->getArray();
	const boneDef_s *b = boneDefs->getArray();
	for(u32 i = 0; i < size(); i++, out++, in++, b++) {
		if(b->parentIndex == -1) {
			out->mat = in->mat;
		} else {
			matrix_c parent = (*absBones)[b->parentIndex].mat;
			parent.inverse();
			out->mat = parent * in->mat;
		}
	}
}
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
		or->setBlendResult(*orFrom,*orTo,frac);
	}
}
void boneOrArray_c::setBlendResult(const boneOrArray_c &from, const boneOrArray_c &to, float frac, const arraySTD_c<u32> &bonesToBlend) {
	// this works only if bones count and order in "from" is the same as in "to"
	this->resize(from.size());
	for(u32 i = 0; i < bonesToBlend.size(); i++) {
		u32 boneIndex = bonesToBlend[i];
		(*this)[boneIndex].setBlendResult(from[boneIndex],to[boneIndex],frac);
	}
}
void boneOrArray_c::setBones(const boneOrArray_c &from, const arraySTD_c<u32> &bonesToSet) {
	for(u32 i = 0; i < bonesToSet.size(); i++) {
		u32 boneIndex = bonesToSet[i];
		(*this)[boneIndex] = from[boneIndex];
	}
}
void boneOrArray_c::inverse() {
	for(u32 i = 0; i < size(); i++) {
		(*this)[i].mat.inverse();
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
void boneOrArray_c::transform(const matrix_c &ofs) {
	boneOr_s *or = this->getArray();
	for(u32 i = 0; i < size(); i++, or++) {
		or->mat = ofs * or->mat;
	}
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
boneOrArray_c boneOrArray_c::getInversed() const {
	boneOrArray_c r = *this;
	for(u32 i = 0; i < size(); i++) {
		r[i].mat.inverse();
	}
	return r;

}
