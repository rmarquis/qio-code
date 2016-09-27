/*
============================================================================
Copyright (C) 2012-2016 V.

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
#include "cg_local.h"
#include "cg_entity.h"
#include "cg_entities.h"
#include "cg_emitter.h" // default emitter
#include "cg_emitter_d3.h" // Doom3 emitter
#include <api/rEntityAPI.h>
#include <api/rLightAPI.h>
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <api/rAPI.h>
#include <api/mtrAPI.h>
#include <math/matrix.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <shared/entityType.h>
#include <shared/quake3Anims.h>
#include <api/afDeclAPI.h>
#include <shared/boneOrQP.h>

static aCvar_c cg_printNewSnapEntities("cg_printNewSnapEntities","0");
static aCvar_c cg_printLightFlags("cg_printLightFlags","0");
static aCvar_c cg_printAttachedEntities("cg_printAttachedEntities","0");
static aCvar_c cg_ignoreBakedLights("cg_ignoreBakedLights","0");

void cgEntity_c::resetEntity() {
	this->lerpOrigin = this->currentState.origin;
	this->lerpAngles = this->currentState.angles;
}

void cgEntity_c::setCurState(const struct entityState_s *newCur) {
	memcpy(&this->currentState, newCur, sizeof(entityState_s));
	//cent->currentState = *state;
	this->bInterpolate = false;
	this->bCurrentValid = true;
}
void cgEntity_c::setNextState(const struct entityState_s *newNext) {
	memcpy(&this->nextState, newNext, sizeof(entityState_s));
	//cent->nextState = *es;

	// if this frame is a teleport, or the entity wasn't in the
	// previous frame, don't interpolate
	if ( !this->bCurrentValid ) {//|| ( ( cent->currentState.eFlags ^ es->eFlags ) & EF_TELEPORT_BIT )  ) {
		this->bInterpolate = false;
	} else {
		this->bInterpolate = true;
	}
}
void cgEntity_c::transitionLocalPlayer(const struct playerState_s &ps, bool bUseCurrentState) {
	if(bUseCurrentState) {
		ps.toEntityState(&currentState, false);
	} else {
		ps.toEntityState(&nextState, false);
	}
	this->bInterpolate = false;
	if(this->rEnt) {
		// set the playerModel flag as soon as its possible
		this->rEnt->setIsPlayerModel(true);
	}
}
void cgEntity_c::transitionLight() {
	if(cg_ignoreBakedLights.getInt()) {
		// ignore (don't add to renderer) lights that have lightmaps baked into BSP file
		if(this->currentState.lightFlags & LF_HASBSPLIGHTING) {
			if(this->rLight) {
				rf->removeLight(this->rLight);
				this->rLight = 0;
			}
			return;
		}
	}
	if(this->rLight == 0) {
		g_core->RedWarning("cgEntity_c::transitionLight: found ET_LIGHT without rLight (entity %i)\n",this->currentState.number);
		// it still happens sometimes, let's fix it here for now.
#if 0
		return;
#else
		this->rLight = rf->allocLight();
#endif
	}
	this->rLight->setOrigin(this->currentState.origin);
	this->rLight->setRadius(this->currentState.lightRadius);
}
void cgEntity_c::transitionModel() {
	if(this->rEnt == 0) {
		g_core->RedWarning("cgEntity_c::transitionModel: found entity without rEntity (entity %i)\n",this->currentState.number);
		// it still happens sometimes, let's fix it here for now.
#if 0
		return;
#else
		this->rEnt = rf->allocEntity();
#endif
	}
	this->rEnt->setOrigin(this->currentState.origin);
	this->rEnt->setAngles(this->currentState.angles);
	this->rEnt->setModel(cgs.gameModels[this->currentState.rModelIndex]);
	for(u32 i = 0; i < MAX_RMODEL_ATTACHMENTS; i++) {
		if(this->currentState.attachments[i].isEmpty() == false) {
			const char *boneName = CG_ConfigString(CS_MODELS+this->currentState.attachments[i].boneIndex);
			const char *modelName = CG_ConfigString(CS_MODELS+this->currentState.attachments[i].modelIndex);
			this->rEnt->setAttachment(i,modelName,boneName);
		} else {
			this->rEnt->setAttachment(i,0,0);
		}
	}
	if(this->rEnt->hasDeclModel()) {
		this->rEnt->setDeclModelAnimLocalIndex(this->currentState.animIndex);
	} else if(this->rEnt->hasTIKIModel()) {
		this->rEnt->setTIKIModelAnimLocalIndex(this->currentState.animIndex);
		this->rEnt->setTIKIModelTorsoAnimLocalIndex(this->currentState.torsoAnim);
	} else if(this->rEnt->isQ3PlayerModel()) {
		this->rEnt->setQ3LegsAnimLocalIndex(this->currentState.animIndex);
		this->rEnt->setQ3TorsoAnimLocalIndex(TORSO_STAND);
	} else if(this->rEnt->hasWolfAnimConfig() || this->rEnt->hasCharacterFile()) {
		this->rEnt->setAnim(CG_ConfigString(CS_ANIMATIONS+this->currentState.animIndex));
	} else if(this->rEnt->isKeyframed()) {
		this->rEnt->setAnimationFrame(this->currentState.animIndex);
	} else {
		this->rEnt->setAnim(cgs.gameAnims[this->currentState.animIndex]);
		this->rEnt->setTorsoAnim(cgs.gameAnims[this->currentState.torsoAnim]);
	}
	if(this->currentState.rSkinIndex) {
		// if we have a custom skin...
		const char *skinName = CG_ConfigString(CS_SKINS + this->currentState.rSkinIndex);
		this->rEnt->setSkin(skinName);
	}
	if(this->currentState.number == cg.clientNum) {
		this->rEnt->setThirdPersonOnly(true);
	} else {
		this->rEnt->setThirdPersonOnly(false);
	}
	if(this->currentState.eFlags & EF_HIDDEN) {
		this->rEnt->hideModel();
	} else {
		this->rEnt->showModel();
	}
	if(this->currentState.activeRagdollDefNameIndex) {
		const afDeclAPI_i *af = cgs.gameAFs[this->currentState.activeRagdollDefNameIndex];
		this->rEnt->setRagdoll(af);
		if(af) {
			for(u32 i = 0; i < af->getNumBodies(); i++) {
				const netBoneOr_s &in = this->currentState.boneOrs[i];
				boneOrQP_c or;
				or.setPos(in.xyz);
				or.setQuatXYZ(in.quatXYZ);
				this->rEnt->setRagdollBodyOr(i,or);
			}
		}
	} else {
		this->rEnt->setRagdoll(0);
	}
}
void cgEntity_c::transitionEntity() {
	entityState_s previousState = this->currentState;
	this->currentState = this->nextState;
	this->bCurrentValid = true;

	// reset if the entity wasn't in the last frame or was teleported
	if ( !this->bInterpolate ) {
		this->resetEntity();
	}

	// set all the values here in case that entity cant be interpolated between two snapshots
	
	// NOTE: some centities might have both rEnt and rLight present
	if(this->currentState.eType == ET_LIGHT) {
		this->transitionLight();
	} else {
		// handle extra entity-light effect as well
		if(this->currentState.lightRadius > 0.f) {
			if(this->rLight == 0) {
				// entity light has been enabled
				this->rLight = rf->allocLight();
			}
			this->transitionLight();
		} else {
			if(this->rLight) {	
				// entity light has been disabled
				rf->removeLight(this->rLight);
				this->rLight = 0;
			}
		}
		this->transitionModel();
	}
	if(this->rEnt) {
		this->rEnt->setEntityType(this->currentState.eType);
	}

	// clear the next state.  if will be set by the next CG_SetNextSnap
	this->bInterpolate = false;
	// remember time of snapshot this entity was last updated in
	this->snapShotTime = cg.snap->serverTime;
}



void cgEntity_c::setupNewEntity(u32 entNum) {
#if 1
	if(this->currentState.number != entNum) {
		this->currentState = this->nextState;
	}
#endif
	if(cg_printNewSnapEntities.getInt()) {
		CG_Printf("cgEntity_c::setupNewEntity: entNum %i, eType %i, ptr %i\n",entNum,this->currentState.eType,this);
	}
	if(this->currentState.eType == ET_LIGHT) {
		// new render light
		if(this->rLight) {
			g_core->RedWarning("cgEntity_c::setupNewEntity: cgEntity_c %i already have rLight assigned\n",entNum);
			rf->removeLight(this->rLight);
		}
		this->rLight = rf->allocLight();
	} else {
		// new render entity
		if(this->rEnt) {
			g_core->RedWarning("cgEntity_c::setupNewEntity: cgEntity_c %i already have rEntity assigned\n",entNum);
			rf->removeEntity(this->rEnt);
		}
		this->rEnt = rf->allocEntity();
		this->rEnt->setNetworkingEntityNumber(entNum);
		if(this->currentState.lightRadius >= 0) {
			// new render light
			if(this->rLight) {
				g_core->RedWarning("cgEntity_c::setupNewEntity: cgEntity_c %i already have rLight assigned\n",entNum);
				rf->removeLight(this->rLight);
			}
			this->rLight = rf->allocLight();
		}
	}
}

bool cgEntity_c::rayTrace(class trace_c &tr, u32 skipEntNum) const {
	if(this->bCurrentValid == false) {
		return false;
	}
	if(this->rEnt == 0) {
		return false;
	}
	if(skipEntNum != ENTITYNUM_NONE) {
		if(this->currentState.number == skipEntNum)
			return false;
		if(this->currentState.parentNum == skipEntNum)
			return false;
	}
	const aabb &bb = this->rEnt->getBoundsABS();
	if(tr.getTraceBounds().intersect(bb) == false) {
		return false;
	}
	if(this->rEnt->rayTraceWorld(tr)) {
		return true;
	}
	return false;
}
void cgEntity_c::updateEntityEmitter() {
	if(this->currentState.isEmitterActive()) {
		// get emitter name (it might be a material name or Doom3 particleDecl name)
		const char *emitterName = CG_ConfigString(CS_MATERIALS+this->currentState.trailEmitterMaterial);
		if(emitterName[0] == 0) {
			g_core->RedWarning("cgEntity_c::updateEntityEmitter: NULL entity emitter name\n");
		} else {
			// see if we have a Doom3 .prt decl for it
			class particleDeclAPI_i *prtDecl = g_declMgr->registerParticleDecl(emitterName);
			// if particle decl was not present, fall back to default simple emitter
			if(prtDecl == 0) {
				class mtrAPI_i *mat = cgs.gameMaterials[this->currentState.trailEmitterMaterial];
				// if we have a valid material
				if(mat) {
					// create a simple material-based emitter
					if(this->emitter == 0) {
						this->emitter = new emitterDefault_c(cg.time);
						rf->addCustomRenderObject(this->emitter);
					}
					this->emitter->setMaterial(mat);
				}
			} else {
				// create a Doom3 particle system emitter
				if(this->emitter == 0) {
					this->emitter = new emitterD3_c;;
					rf->addCustomRenderObject(this->emitter);
				}	
				this->emitter->setParticleDecl(prtDecl);
			}
			this->emitter->setOrigin(this->lerpOrigin);
			this->emitter->setRadius(this->currentState.trailEmitterSpriteRadius);
			this->emitter->setInterval(this->currentState.trailEmitterInterval);

			this->emitter->updateEmitter(cg.time);
		}
	} else {
		if(this->emitter) {
			delete this->emitter;
			this->emitter = 0;
		}
	}
}

void cgEntity_c::calcEntityLerpOrientation() {
	// fast test
	if(this->rEnt) {
		this->rEnt->setScale(this->currentState.scale);
	}
	if(this->currentState.parentNum != ENTITYNUM_NONE) {
		cgEntity_c *parent = &cg_entities[this->currentState.parentNum];
		if(parent->rEnt == 0)
			return;
		if(cg_printAttachedEntities.getInt()) {
			g_core->Print("Entity %i is attached to %i\n",this->currentState.number,this->currentState.parentNum);
		}
		// first we have to update parent orientation (pos + rot),
		// then we can attach current entity to it
		parent->updateCGEntity();
		matrix_c mat;
		parent->rEnt->getBoneWorldOrientation(this->currentState.parentTagNum,mat);
		this->lerpAngles = mat.getAngles();
		this->lerpOrigin = mat.getOrigin();
		if(this->currentState.parentOffset.isAlmostZero() == false) {
			matrix_c matAngles = mat;
			matAngles.setOrigin(vec3_c(0,0,0));
			vec3_c ofs;
			matAngles.transformPoint(this->currentState.parentOffset,ofs);
			this->lerpOrigin += ofs;
		}
		if(this->currentState.localAttachmentAngles.isAlmostZero() == false) {
			this->lerpAngles += this->currentState.localAttachmentAngles;
		}
		// NOTE: some centities might have both rEnt and rLight present
		// update render entity and/or render light
		this->onEntityOrientationChange();
		return;
	}

	if ( this->bInterpolate) {
		this->interpolateEntityOrientation();
		return;
	}
}

void cgEntity_c::onEntityOrientationChange() {
	// NOTE: some thisities might have both rEnt and rLight present
	if(this->rEnt) {
		this->rEnt->setOrigin(this->lerpOrigin);
		this->rEnt->setAngles(this->lerpAngles);
	} 
	if(this->rLight) {
		if(cg_printLightFlags.getInt()) {
			g_core->Print("Light entity %i lightFlags %i\n",this->currentState.number,this->currentState.lightFlags);
		}
		this->rLight->setOrigin(this->lerpOrigin);
		// TODO: lerp light radius?
		this->rLight->setRadius(this->currentState.lightRadius);
		this->rLight->setBNoShadows(this->currentState.lightFlags & LF_NOSHADOWS);
		if(this->currentState.lightFlags & LF_SPOTLIGHT) {
			// see if the spotlight can find it's target
			const cgEntity_c *target = &cg_entities[this->currentState.lightTarget];
			this->rLight->setSpotRadius(this->currentState.spotLightRadius);
			this->rLight->setLightType(LT_SPOTLIGHT);
			if(target->bCurrentValid == false) {
				vec3_c targetPos = this->lerpOrigin + this->lerpAngles.getForward() * 64.f;
				this->rLight->setSpotLightTarget(targetPos);
			} else {
				this->rLight->setSpotLightTarget(target->lerpOrigin);
			}
		} else {
			this->rLight->setLightType(LT_POINT);
		}
		if(this->currentState.lightFlags & LF_COLOURED) {
			this->rLight->setBColoured(true);
			this->rLight->setColor(this->currentState.lightColor);
		} else {
			this->rLight->setBColoured(false);
		}
	}
}

void cgEntity_c::clearEntity() {
//	if(this->currentState.eType == ET_LIGHT) {
		if(this->rLight) {
			rf->removeLight(this->rLight);
			this->rLight = 0;
		}
///	} else {
		if(this->rEnt) {
			rf->removeEntity(this->rEnt);
			this->rEnt = 0;
		}
//	}
	if(this->emitter) {
		delete this->emitter;
		this->emitter = 0;
	}
	this->bCurrentValid = false;
}
void cgEntity_c::interpolateEntityOrientation() {
	if ( cg.nextSnap == NULL ) {
		// It should never happen, but it actually happens sometimes when the FPS is very low...
		g_core->RedWarning( "cgEntity_c::interpolateEntityOrientation: cg.nextSnap == NULL\n" );
	}
	const float *current = this->currentState.origin;
	const float *next = this->nextState.origin;

	this->lerpOrigin[0] = current[0] + cg.frameInterpolation * ( next[0] - current[0] );
	this->lerpOrigin[1] = current[1] + cg.frameInterpolation * ( next[1] - current[1] );
	this->lerpOrigin[2] = current[2] + cg.frameInterpolation * ( next[2] - current[2] );

	this->lerpAngles = this->currentState.angles.lerpDegrees(this->nextState.angles, cg.frameInterpolation);

	// update render entity and/or render light
	this->onEntityOrientationChange();
}

void cgEntity_c::updateCGEntity() {
	if(this->lastUpdateFrame == cg.clientFrame)
		return; // it was already updated (this may happen for attachment parents)
	this->lastUpdateFrame = cg.clientFrame;

	// calculate the current origin
	this->calcEntityLerpOrientation();
	// update entity emitter
	this->updateEntityEmitter();
}