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
// rf_entities.cpp
#include "rf_entities.h"
#include "rf_model.h"
#include "rf_local.h"
#include "rf_decals.h"
#include "rf_surface.h"
#include "rf_anims.h"
#include <api/coreAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>
#include <shared/autoCvar.h>

aCvar_c anim_printAnimCtrlTime("anim_printAnimCtrlTime","1");

// unfinished animation blender
class animController_c {
	float time;
	const class skelAnimAPI_i *anim;
	int lastUpdateTime;
	// extra variables for blending between new and old animation
	singleAnimLerp_s oldState;
	const class skelAnimAPI_i *nextAnim;
	float blendTime;

	static void getSingleLoopAnimLerpValuesForTime(singleAnimLerp_s &out, const class skelAnimAPI_i *anim, float time) {
		if(anim->getNumFrames() == 1) {
			out.from = 0;
			out.to = 0;
			out.frac = 0.f;
			return;
		}
		float step = anim->getFrameTime();
		float cur = 0;
		float next = step;
		for(u32 i = 0; i < anim->getNumFrames(); i++) {
			if(time >= cur && time < next) {
				out.from = i;
				out.to = i + 1;
				if(out.to == anim->getNumFrames()) {
					out.to = 0;
				}
				out.frac = (time - cur) / step;
				return;
			}
			next += step;
			cur += step;
		}
	}
public:
	void resetToAnim(const class skelAnimAPI_i *newAnim) {
		time = 0.f;
		anim = newAnim;
		nextAnim = anim;
		lastUpdateTime = rf_curTimeMsec;
	}
	void setNextAnim(const class skelAnimAPI_i *newAnim) {
		if(anim == newAnim)
			return;
		if(nextAnim == newAnim)
			return;
		if(nextAnim != anim) {
			g_core->RedWarning("animController_c::setNextAnim: havent finished lerping the previous animation change; jittering might be visible\n");
		}
		if(anim->getBLoopLastFrame() && (time > anim->getTotalTimeSec())) {
			oldState.from = oldState.to = anim->getNumFrames()-1;
			oldState.frac = 0.f;
		} else {
			getSingleLoopAnimLerpValuesForTime(oldState,anim,time);
		}
		time = 0;
		blendTime = 0.1f;
		nextAnim = newAnim;
	}
	void runAnimController() {
		int deltaTime = rf_curTimeMsec - lastUpdateTime;
		if(deltaTime == 0)
			return;
		lastUpdateTime = rf_curTimeMsec;
		float deltaTimeSec = float(deltaTime) * 0.001f;
		time += deltaTimeSec;
		if(anim == nextAnim) {
			if(anim->getBLoopLastFrame()) {
				return; // dont loop this animation, just stop at the last frame
			} else {
				while(time > anim->getTotalTimeSec()) {
				//	g_core->Print("Clamping time %f by %f\n",time,anim->getTotalTimeSec());
					time -= anim->getTotalTimeSec();
				}
				if(anim_printAnimCtrlTime.getInt()) {
				//	g_core->Print("Final time: %f\n",time);
				}
			}
		} else {
			while(time > blendTime) {
				time -= blendTime;
				anim = nextAnim;
			}
		}
	}
	void updateModelAnimation(const class skelModelAPI_i *skelModel, class r_model_c *instance) {
		boneOrArray_c bones;
		if(anim == nextAnim) {
			bones.resize(anim->getNumBones());
			if(anim->getBLoopLastFrame() && (time > ((anim->getNumFrames()-1)*anim->getFrameTime()))) {
				anim->buildFrameBonesLocal(anim->getNumFrames()-1,bones);			
			} else {
				singleAnimLerp_s lerp;
				getSingleLoopAnimLerpValuesForTime(lerp,anim,time);
				//g_core->Print("From %i to %i - %f\n",lerp.from,lerp.to,lerp.frac);

				//anim->buildFrameBonesLocal(lerp.from,bones);
				anim->buildLoopAnimLerpFrameBonesLocal(lerp,bones);
			}
		} else {
			// build old skeleton first
			// ( TODO: we might do it once and just store the bones here...)
			boneOrArray_c previous;
			previous.resize(anim->getNumBones());
			anim->buildLoopAnimLerpFrameBonesLocal(oldState,previous);
			
			// build skeleton for the first frame of new animation
			boneOrArray_c newBones;
			newBones.resize(nextAnim->getNumBones());
			nextAnim->buildFrameBonesLocal(0,newBones);
			
			// do the interpolation
			float frac = this->time / this->blendTime;
		//	g_core->Print("Blending between two anims, frac: %f\n",frac);	
			bones.setBlendResult(previous, newBones, frac);		
		}
		bones.localBonesToAbsBones(anim->getBoneDefs());
		if(skelModel->hasCustomScaling()) {
			bones.scaleXYZ(skelModel->getScaleXYZ());
		}
		instance->updateSkelModelInstance(skelModel,bones);	
	}
};

aCvar_c rf_skipEntities("rf_skipEntities","0");

rEntityImpl_c::rEntityImpl_c() {
	model = 0;
	staticDecals = 0;
	instance = 0;
	animCtrl = 0;
}
rEntityImpl_c::~rEntityImpl_c() {
	if(staticDecals) {
		delete staticDecals;
		staticDecals = 0;
	}
	if(instance) {
		delete instance;
		instance = 0;
	}
	if(animCtrl) {
		delete animCtrl;
		animCtrl = 0;
	}
}

void rEntityImpl_c::recalcABSBounds() {
	if(model == 0) {
		absBB.clear();
		return;
	}
	const aabb &bb = model->getBounds();
	matrix.transformAABB(bb,this->absBB);
}
void rEntityImpl_c::recalcMatrix() {
	// TODO: use axis instead of angles
	matrix.fromAnglesAndOrigin(angles,origin);
	recalcABSBounds();
}
void rEntityImpl_c::setOrigin(const vec3_c &newXYZ) {
	origin = newXYZ;
	recalcMatrix();
}
void rEntityImpl_c::setAngles(const vec3_c &newAngles) { 
	angles = newAngles;
	axis.fromAngles(angles);
	recalcMatrix();
}

void rEntityImpl_c::setModel(class rModelAPI_i *newModel) {
	if(model == newModel) {
		return;
	}
	if(staticDecals) {
		delete staticDecals;
		staticDecals = 0;
	}
	if(instance) {
		delete instance;
		instance = 0;
	}
	if(newModel->isStatic() == false) {
		instance = new r_model_c;
		skelModelAPI_i *skelModel = newModel->getSkelModelAPI();
		instance->initSkelModelInstance(skelModel);
#if 1
		instance->updateSkelModelInstance(skelModel,skelModel->getBaseFrameABS());
#else
		rfAnimation_c *anim = RF_RegisterAnimation("models/player/shina/run.md5anim");
		boneOrArray_c bones;
		bones.resize(anim->getAPI()->getNumBones());
		anim->getAPI()->buildFrameBonesLocal(0,bones);
		bones.localBonesToAbsBones(anim->getAPI()->getBoneDefs());
		if(skelModel->hasCustomScaling()) {
			bones.scaleXYZ(skelModel->getScaleXYZ());
		}
		instance->updateSkelModelInstance(skelModel,bones);	
#endif
	}
	model = newModel;
	recalcABSBounds();
}
void rEntityImpl_c::setAnim(const class skelAnimAPI_i *anim) {
	if(anim == 0 && animCtrl == 0)
		return; // ignore
	if(animCtrl == 0) {
		animCtrl = new animController_c;
		animCtrl->resetToAnim(anim);
	} else {
		// TODO
		animCtrl->setNextAnim(anim);
	}
}
void rEntityImpl_c::addDrawCalls() {
	if(model->isStatic()) {
		((model_c*)model)->addModelDrawCalls();
		if(staticDecals) {
			staticDecals->addDrawCalls();
		}
	} else if(instance) {
		if(animCtrl) {
			animCtrl->runAnimController();
			animCtrl->updateModelAnimation(model->getSkelModelAPI(),instance);
		}
		instance->addDrawCalls();
	}
}
bool rEntityImpl_c::rayTrace(class trace_c &tr) const {
	return model->rayTrace(tr);
}
int rEntityImpl_c::addDecalWorldSpace(const class vec3_c &pos, 
	const class vec3_c &normal, float radius, class mtrAPI_i *material) {
	if(model == 0)
		return -1;
	matrix_c matrixInverse = matrix.getInversed();
	vec3_c posLocal;
	matrixInverse.transformPoint(pos,posLocal);
	vec3_c normLocal;
	matrixInverse.transformNormal(normal,normLocal);
	if(model->isStatic()) {
		if(this->staticDecals == 0) {
			this->staticDecals = new simpleDecalBatcher_c;
		}
		model->createStaticModelDecal(this->staticDecals,posLocal,normLocal,radius,material);
	}
	return -1;
}

static arraySTD_c<rEntityImpl_c*> rf_entities;
rEntityAPI_i *rf_currentEntity = 0;

class rEntityAPI_i *RFE_AllocEntity() {
	rEntityImpl_c *ent = new rEntityImpl_c;
	rf_entities.push_back(ent);
	return ent;
}
void RFE_RemoveEntity(class rEntityAPI_i *ent) {
	rEntityImpl_c *rent = (rEntityImpl_c*)ent;
	rf_entities.remove(rent);
	delete rent;
}

void RFE_AddEntityDrawCalls() {
	if(rf_skipEntities.getInt())
		return;

	u32 c_entitiesCulledByABSBounds = 0;
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		rf_currentEntity = ent;
		model_c *model = (model_c*)ent->getModel();
		if(model == 0) {
			continue;
		}
		if(rf_camera.getFrustum().cull(ent->getBoundsABS()) == CULL_OUT) {
			c_entitiesCulledByABSBounds++;
			continue;
		}
		ent->addDrawCalls();
	}
	if(0) {
		g_core->Print("RFE_AddEntityDrawCalls: %i of %i entities culled\n",c_entitiesCulledByABSBounds,rf_entities.size());
	}
	rf_currentEntity = 0;
}

void RFE_ClearEntities() {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		delete rf_entities[i];
		rf_entities[i] = 0;
	}
	rf_entities.clear();
}
