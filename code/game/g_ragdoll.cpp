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
// g_ragdoll.cpp
#include "g_local.h"
#include "g_ragdoll.h"
#include <api/afDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/skelAnimAPI.h>
#include <api/modelDeclAPI.h>
#include <api/coreAPI.h>
#include <api/physAPI.h>
#include <api/physObjectAPI.h>
#include <api/cmAPI.h>
#include <shared/autoCvar.h>
#include <shared/skelUtils.h>
#include <shared/afRagdollHelper.h>
#include <shared/boneOrQP.h>
#include <shared/physObjectDef.h>
#include <math/axis.h>

class ragdoll_c : public ragdollAPI_i {
friend class afRagdollSpawner_c;
	// ragdoll definition
	const afDeclAPI_i *af;
	const afPublicData_s *afd;
	// Game Physics ragdoll representation
	arraySTD_c<physObjectAPI_i*> bodies;
	arraySTD_c<physConstraintAPI_i*> constraints;
	// bodies transforms, updated every frame
	arraySTD_c<matrix_c> curTransforms;
public:
	ragdoll_c() {
		af = 0;
		afd = 0;
	}
	~ragdoll_c() {
		for(u32 i = 0; i < constraints.size(); i++) {
			g_physWorld->destroyPhysicsConstraint(constraints[i]);
			constraints[i] = 0;
		}
		constraints.clear();
		for(u32 i = 0; i < bodies.size(); i++) {
			g_physWorld->destroyPhysicsObject(bodies[i]);
			bodies[i] = 0;
		}
		bodies.clear();
	}
	virtual const arraySTD_c<matrix_c> &getCurWorldMatrices() const {
		return curTransforms;
	}
	virtual void updateWorldTransforms() {
		for(u32 i = 0; i < bodies.size(); i++) {
			physObjectAPI_i *b = bodies[i];
			if(b == 0) {
				g_core->RedWarning("ragdoll_c::updateWorldTransforms: ragdoll has NULL body %i\n",i);
				continue;
			}
			b->getCurrentMatrix(curTransforms[i]);
		}
	}
	virtual bool setPose(const class boneOrQPArray_t &bodyORs) {
		if(bodyORs.size() != bodies.size())
			return true; // error
		for(u32 i = 0; i < bodies.size(); i++) {
			physObjectAPI_i *b = bodies[i];
			const boneOrQP_c &bor = bodyORs[i];
			matrix_c mat;
			mat.fromQuatAndOrigin(bor.getQuat(),bor.getPos());
			printf("todo");
			b->setMatrix(mat);
		}
		return false;
	}
	virtual bool setPoseFromRenderModelBonesArray(const class boneOrArray_c &boneOrs, const class skelAnimAPI_i *anim) {
		if(anim == 0) {
			g_core->RedWarning("ragdoll_c::setPoseFromRenderModelBonesArray: anim is NULL.\n");
			return true; // error
		}
		// first get bone indexes
		arraySTD_c<int> boneIndexes;
		boneIndexes.resize(bodies.size());
		for(u32 i = 0; i < bodies.size(); i++) {
			const char *jointName = af->getBodyParentJointName(i);
			int boneIndex = anim->getLocalBoneIndexForBoneName(jointName);
			if(boneIndex < 0) {
				g_core->RedWarning("ragdoll_c::setPoseFromRenderModelBonesArray: bone %s not found. Failed to set pose.\n",jointName);
				return true; // error
			}
			boneIndexes[i] = boneIndex;
		}
		// then calculate new body matrices
		arraySTD_c<matrix_c> boneParentBody2Bone;
		afRagdollHelper_c rh;
		rh.calcBoneParentBody2BoneOfsets(af->getName(),boneParentBody2Bone);
		// ragdoll is made of multiple rigid bodies connected together
		// set the matrix of each rigid body
		for(u32 i = 0; i < bodies.size(); i++) {
			physObjectAPI_i *b = bodies[i];
			if(b == 0) {
				continue;
			}
			int boneIndex = boneIndexes[i];
			const matrix_c &offset = boneParentBody2Bone[boneIndex];
			const matrix_c &worldBone = boneOrs[boneIndex].mat;
			matrix_c worldBody = worldBone * offset.getInversed();
			b->setMatrix(worldBody);
		}
		return false;
	}
};

class afRagdollSpawner_c : public afRagdollHelper_c {

public:
	ragdoll_c *spawnRagdollFromAF(const char *afName, const vec3_c &pos, const vec3_c &angles) {
		if(setupRagdollHelper(afName)) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to setup afRagdollHelper for AF \"%s\"\n",afName);
			return 0;
		}
		if(cm == 0) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: can't spawn ragdoll %s because CM module is not prsent.\n",afName);
			return 0;
		}
		if(af == 0) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: can't spawn ragdoll because %s is not a valid AF name.\n",afName);
			return 0;
		}
		matrix_c extraWorldTransform;
		extraWorldTransform.fromAnglesAndOrigin(angles,pos);
		ragdoll_c *ret = new ragdoll_c;
		ret->af = af;
		ret->afd = afd;
		bones.resize(anim->getNumBones());
		anim->buildFrameBonesABS(0,bones);
		// spawn bullet bodies
		arraySTD_c<physObjectAPI_i*> &bodies = ret->bodies;
		arraySTD_c<matrix_c> matrices;
		matrices.resize(afd->bodies.size());
		bodies.resize(afd->bodies.size());
		ret->curTransforms.resize(afd->bodies.size());
		for(u32 i = 0; i < afd->bodies.size(); i++) {
			const afBody_s &b = afd->bodies[i];
			const afModel_s &m = b.model;

			matrix_c mat;
			getBodyTransform(i,mat);
			// add world transform
			mat = extraWorldTransform * mat;
			matrices[i] = mat;

			// get cModel for afModel
			str cModelName = afName;
			cModelName.append("::");
			cModelName.append(va("*%i",i));
			cMod_i *cMod = cm->findModel(cModelName);
			// if cMod was not present, create it
			if(cMod == 0) {
				arraySTD_c<vec3_c> points;
				createConvexPointSoupForAFModel(m, points);
				cMod = cm->registerHull(cModelName,points.getArray(),points.size());
			}
			physObjectDef_s def;
			def.collisionModel = cMod;
			def.mass = 50.f;
			def.transform = mat;
			bodies[i] = g_physWorld->createPhysicsObject(def);
		}
		// spawn bullet contraints
		arraySTD_c<physConstraintAPI_i*> &constraints = ret->constraints;
		constraints.resize(afd->constraints.size());
		for(u32 i = 0; i < afd->constraints.size(); i++) {
			const afConstraint_s &c = afd->constraints[i];
			int b0Index = afd->getLocalIndexForBodyName(c.body0Name);
			if(b0Index == -1) {
				g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find first body (\"%s\") of constraint \"%s\" (%i) of AF \"%s\"\n",
					c.body0Name.c_str(),c.name.c_str(),i,afName);
				constraints[i] = 0;
				continue;
			}
			int b1Index = afd->getLocalIndexForBodyName(c.body1Name);
			if(b0Index == -1) {
				g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find second body (\"%s\") of constraint \"%s\" (%i) of AF \"%s\"\n",
					c.body1Name.c_str(),c.name.c_str(),i,afName);
				constraints[i] = 0;
				continue;
			}
			physObjectAPI_i *b0 = bodies[b0Index];
			if(b0 == 0) {
				constraints[i] = 0;
				continue;
			}
			physObjectAPI_i *b1 = bodies[b1Index];
			if(b1 == 0) {
				constraints[i] = 0;
				continue;
			}
			vec3_c anchor = getAFVec3Value(c.anchor);
			// transform anchor to world coordinates
			extraWorldTransform.transformPoint(anchor);

			constraints[i] = g_physWorld->createConstraintBall(anchor,b0,b1);
		}
		return ret;
	}
};

class ragdollAPI_i *G_SpawnTestRagdollFromAF(const char *afName, const vec3_c &pos, const vec3_c &angles) {
	afRagdollSpawner_c s;
	return s.spawnRagdollFromAF(afName, pos, angles);
}