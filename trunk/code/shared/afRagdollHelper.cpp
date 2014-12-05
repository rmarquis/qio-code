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
// afRagdollHelper.cpp
#include "afRagdollHelper.h"
#include <math/aabb.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <api/afDeclAPI.h>
#include <api/coreAPI.h>
#include <api/skelAnimAPI.h>
#include <api/declManagerAPI.h>
#include <api/modelDeclAPI.h>

void CU_InitD3TRMBoneConvexPoints(float length, float width, arraySTD_c<vec3_c> &out) {
	float halfLength = length * 0.5f;
	out.resize(5);
	// set vertices
	out[0].set(0.0f, 0.0f, -halfLength);
	out[1].set(0.0f, width * -0.5f, 0.0f);
	out[2].set(width * 0.5f, width * 0.25f, 0.0f);
	out[3].set(width * -0.5f, width * 0.25f, 0.0f);
	out[4].set(0.0f, 0.0f, halfLength);
}

void CU_InitD3TRMCylinderConvexPoints(const aabb &cylBounds, int numSides, arraySTD_c<vec3_c> &out) {
	int n = numSides;
	if ( n < 3 ) {
		n = 3;
	}
	out.resize(n*2);
	vec3_c offset = ( cylBounds.mins + cylBounds.maxs ) * 0.5f;
	vec3_c halfSize = cylBounds.maxs - offset;
	for(u32 i = 0; i < n; i++) {
		// verts
		float angle = (2.0*M_PI) * i / n;
		out[i].x = cos( angle ) * halfSize.x + offset.x;
		out[i].y = sin( angle ) * halfSize.y + offset.y;
		out[i].z = -halfSize.z + offset.z;
		out[n+i].x = out[i].x;
		out[n+i].y = out[i].y;
		out[n+i].z = halfSize.z + offset.z;
	}
}

void CU_InitD3TRMConeConvexPoints(const aabb &coneBounds, int numSides, arraySTD_c<vec3_c> &out) {
	int n = numSides;
	if ( n < 2 ) {
		n = 3;
	}
	out.resize(n + 1);
	vec3_c offset = ( coneBounds.mins + coneBounds.maxs ) * 0.5f;
	vec3_c halfSize = coneBounds.maxs - offset;
	out[n].set( 0.0f, 0.0f, halfSize.z + offset.z );
	for (u32 i = 0; i < n; i++) {
		// verts
		float angle = (2.0*M_PI) * i / n;
		out[i].x = cos( angle ) * halfSize.x + offset.x;
		out[i].y = sin( angle ) * halfSize.y + offset.y;
		out[i].z = -halfSize.z + offset.z;
	}
}


void CU_InitD3TRMDodecahedronConvexPoints(const aabb &dodBounds, arraySTD_c<vec3_c> &out) {
	float s, d;
	vec3_c a, b, c;

	a[0] = a[1] = a[2] = 0.5773502691896257f; // 1.0f / ( 3.0f ) ^ 0.5f;
	b[0] = b[1] = b[2] = 0.3568220897730899f; // ( ( 3.0f - ( 5.0f ) ^ 0.5f ) / 6.0f ) ^ 0.5f;
	c[0] = c[1] = c[2] = 0.9341723589627156f; // ( ( 3.0f + ( 5.0f ) ^ 0.5f ) / 6.0f ) ^ 0.5f;
	d = 0.5f / c[0];
	s = ( dodBounds.maxs[0] - dodBounds.mins[0] ) * d;
	a[0] *= s;
	b[0] *= s;
	c[0] *= s;
	s = ( dodBounds.maxs[1] - dodBounds.mins[1] ) * d;
	a[1] *= s;
	b[1] *= s;
	c[1] *= s;
	s = ( dodBounds.maxs[2] - dodBounds.mins[2] ) * d;
	a[2] *= s;
	b[2] *= s;
	c[2] *= s;

	vec3_c offset = ( dodBounds.mins + dodBounds.maxs ) * 0.5f;

	// set vertices
	out.resize(20);
	out[ 0].set( offset.x + a[0], offset.y + a[1], offset.z + a[2] );
	out[ 1].set( offset.x + a[0], offset.y + a[1], offset.z - a[2] );
	out[ 2].set( offset.x + a[0], offset.y - a[1], offset.z + a[2] );
	out[ 3].set( offset.x + a[0], offset.y - a[1], offset.z - a[2] );
	out[ 4].set( offset.x - a[0], offset.y + a[1], offset.z + a[2] );
	out[ 5].set( offset.x - a[0], offset.y + a[1], offset.z - a[2] );
	out[ 6].set( offset.x - a[0], offset.y - a[1], offset.z + a[2] );
	out[ 7].set( offset.x - a[0], offset.y - a[1], offset.z - a[2] );
	out[ 8].set( offset.x + b[0], offset.y + c[1], offset.z        );
	out[ 9].set( offset.x - b[0], offset.y + c[1], offset.z        );
	out[10].set( offset.x + b[0], offset.y - c[1], offset.z        );
	out[11].set( offset.x - b[0], offset.y - c[1], offset.z        );
	out[12].set( offset.x + c[0], offset.y       , offset.z + b[2] );
	out[13].set( offset.x + c[0], offset.y       , offset.z - b[2] );
	out[14].set( offset.x - c[0], offset.y       , offset.z + b[2] );
	out[15].set( offset.x - c[0], offset.y       , offset.z - b[2] );
	out[16].set( offset.x       , offset.y + b[1], offset.z + c[2] );
	out[17].set( offset.x       , offset.y - b[1], offset.z + c[2] );
	out[18].set( offset.x       , offset.y + b[1], offset.z - c[2] );
	out[19].set( offset.x       , offset.y - b[1], offset.z - c[2] );
}

vec3_c afRagdollHelper_c::getAFVec3Value(const afVec3_s &v) {
	if(v.type == AFVEC3_RAWDATA)
		return v.rawData;
	if(v.type == AFVEC3_BONECENTER) {
		int b0 = anim->getLocalBoneIndexForBoneName(v.boneName);
		int b1 = anim->getLocalBoneIndexForBoneName(v.secondBoneName);
		if(b0 != -1 && b1 != -1) {
			vec3_c p0 = bones[b0].mat.getOrigin();
			vec3_c p1 = bones[b1].mat.getOrigin();
			vec3_c ret;
			ret.lerp(p0,p1,0.5f);
			return ret;
		}
	} 
	if(v.type == AFVEC3_JOINT) {
		int boneNum = anim->getLocalBoneIndexForBoneName(v.boneName);
		if(boneNum != -1) {
			return bones[boneNum].mat.getOrigin();
		}
	} 
	return vec3_c(0,0,0);
}	
bool afRagdollHelper_c::createConvexPointSoupForAFModel(const afModel_s &m, arraySTD_c<vec3_c> &outPoints) {
	outPoints.clear();
	if(m.type == AFM_BOX) {
		aabb bb;
		bb.mins = getAFVec3Value(m.v[0]);
		bb.maxs = getAFVec3Value(m.v[1]);
		// create convex volume
		for(u32 i = 0; i < 8; i++) {
			vec3_c p = bb.getPoint(i);
			outPoints.push_back(p.floatPtr());
		}
		return false;
	} else if(m.type == AFM_CYLINDER) {
		aabb bb;
		bb.mins = getAFVec3Value(m.v[0]);
		bb.maxs = getAFVec3Value(m.v[1]);
		CU_InitD3TRMCylinderConvexPoints(bb,m.numSides,outPoints);
		return false;
	} else if(m.type == AFM_BONE) {
		vec3_c v0 = getAFVec3Value(m.v[0]);
		vec3_c v1 = getAFVec3Value(m.v[1]);
		axis_c axis;
		// direction of bone
		axis[2] = v1 - v0;
		float length = axis[2].normalize2();
		// axis of bone trace model
		axis[2].makeNormalVectors( axis[0], axis[1] );
		axis[1] = -axis[1];
		// create bone trace model
		arraySTD_c<vec3_c> points;
		CU_InitD3TRMBoneConvexPoints(length,m.width,outPoints);
		return false;
	} else if(m.type == AFM_CONE) {
		vec3_c v0 = getAFVec3Value(m.v[0]);
		vec3_c v1 = getAFVec3Value(m.v[1]);
		aabb bb;
		bb.mins = v0;
		bb.maxs = v1;
		// place the apex at the origin
		bb.mins.z -= bb.maxs.z;
		bb.maxs.z = 0.0f;

		// create cone trace model
		arraySTD_c<vec3_c> points;
		CU_InitD3TRMConeConvexPoints(bb,m.numSides,outPoints);
		return false;		
	} else if(m.type == AFM_DODECAHEDRON) {
		aabb bb;
		bb.mins = getAFVec3Value(m.v[0]);
		bb.maxs = getAFVec3Value(m.v[1]);
		// create dodecahedron trace model
		CU_InitD3TRMDodecahedronConvexPoints(bb,outPoints);
		return false;
	}
	g_core->RedWarning("afRagdollHelper_c::createConvexPointSoupForAFModel: failed to spawn model of type %i\n",m.type);
	return true; // error
}

bool afRagdollHelper_c::getBodyTransform(u32 bodyNum, matrix_c &out) {
	const afBody_s &b = afd->bodies[bodyNum];
	const afModel_s &m = b.model;

	vec3_c p = getAFVec3Value(b.origin);
	matrix_c mat;
	if(m.type == AFM_BONE) {
		vec3_c v0 = getAFVec3Value(m.v[0]);
		vec3_c v1 = getAFVec3Value(m.v[1]);
		axis_c axis;
		axis[2] = v1 - v0;
		axis[2].normalize();
		axis[2].makeNormalVectors( axis[0], axis[1] );
		axis[1] = -axis[1];
		out.fromAxisAndOrigin(axis,p);
	} else {
		vec3_c angles = b.angles;
		out.fromAnglesAndOrigin(angles,p);
	}
	return false; // no error
}

afRagdollHelper_c::afRagdollHelper_c() {
	boneDefs = 0;
	anim = 0;
	af = 0;
	afd = 0;
	model = 0;
}
bool afRagdollHelper_c::setupRagdollHelper(const char *afName) {
	af = g_declMgr->registerAFDecl(afName);
	if(af == 0) {
		g_core->RedWarning("afRagdollHelper_c::setupRagdollHelper: failed to find articulatedFigure \"%s\"\n",afName);
		return true;
	}
	afd = af->getData();
	model = g_declMgr->registerModelDecl(afd->modelName);
	if(model == 0) {
		g_core->RedWarning("afRagdollHelper_c::setupRagdollHelper: failed to find model %s (needed for articulatedFigure \"%s\")\n",afd->modelName.c_str(),afName);
		return true;
	}
	anim = model->getSkelAnimAPIForAlias("af_pose");
	if(anim == 0) {
		g_core->RedWarning("afRagdollHelper_c::setupRagdollHelper: failed to find \"af_pose\" animation in model \"%s\" of af \"%s\"\n",
			afd->modelName.c_str(),afName);
		return true;
	}
	bones.resize(anim->getNumBones());
	anim->buildFrameBonesABS(0,bones);
	return false;
}
void UTIL_ContainedJointNamesArrayToJointIndexes(const arraySTD_c<str> &containedJoints, arraySTD_c<u32> &boneNumbers, const class skelAnimAPI_i *anim) {
	if(anim == 0) {
		g_core->RedWarning("UTIL_ContainedJointNamesArrayToJointIndexes: NULL anim pointer\n");
		return;
	}
	boneNumbers.reserve(anim->getNumBones()*2);
	for(u32 j = 0; j < containedJoints.size(); j++) {
		const char *boneNameStr = containedJoints[j];
		if(boneNameStr[0] == '*') {
			anim->addChildrenOf(boneNumbers,boneNameStr+1);
			continue;
		} else if(boneNameStr[0] == '-' && boneNameStr[1] == '*') {
			anim->removeChildrenOf(boneNumbers,boneNameStr+2);
			continue;
		} else {
			int boneNum = anim->getLocalBoneIndexForBoneName(boneNameStr);
			if(boneNum == -1) {
				g_core->RedWarning("Cant find bone %s in %s\n",boneNameStr,anim->getName());
				continue;
			}
			boneNumbers.push_back(boneNum);
		}
	}
}
bool afRagdollHelper_c::calcBoneParentBody2BoneOfsets(const char *afName, arraySTD_c<matrix_c> &out) {
	if(setupRagdollHelper(afName))
		return true;
	arraySTD_c<u32> refCounts;
	refCounts.resize(anim->getNumBones());
	out.resize(anim->getNumBones());
	for(u32 i = 0; i < anim->getNumBones(); i++) {
		out[i].identity();
	}
	refCounts.nullMemory();
	for(u32 i = 0; i < afd->bodies.size(); i++) {
		const afBody_s &b = afd->bodies[i];
		matrix_c bodyMat;
		getBodyTransform(i,bodyMat);
		matrix_c bodyMatInv = bodyMat.getInversed();
		arraySTD_c<u32> boneNumbers;
		boneNumbers.reserve(anim->getNumBones()*2);
		UTIL_ContainedJointNamesArrayToJointIndexes(b.containedJoints,boneNumbers,anim);
		for(u32 j = 0; j < boneNumbers.size(); j++) {
			u32 boneNum = boneNumbers[j];
			if(refCounts[boneNum]) {
				// it should NEVER happen
				g_core->RedWarning("Bone %i is referenced more than once in AF\n",boneNum);
				//continue;
				return false;
			}
			refCounts[boneNum] ++;
			const matrix_c &boneABS = bones[boneNum].mat;
			out[boneNum] = bodyMatInv*boneABS;
		}
	}
	return false;
}