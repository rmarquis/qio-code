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
// cg_viewmodel.cpp - first person weapon and hands animation
#include "cg_local.h"
#include <shared/autoCvar.h>
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <api/coreAPI.h>
#include <api/modelDeclAPI.h>
#include <renderer/rModelAPI.h>
#include <math/matrix.h>
#include <math/axis.h>

static aCvar_c cg_gunX("cg_gunX","0");
static aCvar_c cg_gunY("cg_gunY","0");
static aCvar_c cg_gunZ("cg_gunZ","0");
static aCvar_c cg_gunRotX("cg_gunRotX","0");
static aCvar_c cg_gunRotY("cg_gunRotY","0");
static aCvar_c cg_gunRotZ("cg_gunRotZ","0");
static aCvar_c cg_printCurViewModelName("cg_printCurViewModelName","0");
static aCvar_c cg_printCurViewModelAnimationCount("cg_printCurViewModelAnimationCount","0");
static aCvar_c cg_printCurViewModelBoneNames("cg_printCurViewModelBoneNames","0");
static aCvar_c cg_forceViewModelAnimationIndex("cg_forceViewModelAnimationIndex","none");
static aCvar_c cg_forceViewModelAnimationName("cg_forceViewModelAnimationName","none");
static aCvar_c cg_forceViewModelAnimationFlags("cg_forceViewModelAnimationFlags","none");
static aCvar_c cg_printViewModelAnimName("cg_printViewModelAnimName","0");
static aCvar_c cg_printViewWeaponClipSize("cg_printViewWeaponClipSize","0");
static aCvar_c cg_printViewModelBobbingOffset("cg_printViewModelBobbingOffset","0");

static class rEntityAPI_i *cg_viewModelEntity = 0;

void CG_FreeViewModelEntity() {
	if(cg_viewModelEntity == 0)
		return;
	rf->removeEntity(cg_viewModelEntity);
	cg_viewModelEntity = 0;
}
void CG_AllocViewModelEntity() {
	if(cg_viewModelEntity)
		return;
	cg_viewModelEntity = rf->allocEntity();
}

// viewmodel animation parameters
struct viewModelAnimationConfig_s {
	float offsetMax; // offset limit
	float offsetSpeed;
	vec3_c sway;
	vec3_c offsetAir; // front, side, up
	float offetVelBase;
	vec3_c offsetVel; // front, side, up
	float offsetUpVel;

	viewModelAnimationConfig_s() {
		sway.set(0.7,0.17,0.1);
		offsetAir.set(-3,1.5,-6);
		offsetVel.set(-2,1.5,-4);
		offsetMax = 8.f;
		offsetSpeed = 6.f;
		offetVelBase = 100.f;
		offsetUpVel = 0.0025;
	}
};
// helper class for viewmodel bobbing animation
class viewModelAnimator_c {
	float currentAmplitude;
	float currentPhase;
	vec3_c currentPosOffset;
	vec3_c currentMovement;
	viewModelAnimationConfig_s cfg;

	void calcMovementInternal(const vec3_c &playerVelocity, bool isOnGround, float frameTimeSeconds) {
		currentMovement[0] = sin(currentPhase + M_PI) * currentAmplitude * cfg.sway.x;
		currentMovement[1] = sin(currentPhase + M_PI) * currentAmplitude * cfg.sway.y;
		currentMovement[2] = currentAmplitude * (sin(2.f*currentPhase - M_PI*0.4f) + 0.125*sin(4.f*currentPhase + M_PI*0.7f)) * cfg.sway.z;

		vec3_c vmOfs;
		if(isOnGround == false) {
			// player is in air
			vmOfs = cfg.offsetAir;
			// append offset depending on Z velocity
			if(playerVelocity[2] != 0.f) {
				vmOfs[2] -= playerVelocity[2] * cfg.offsetUpVel;
			}
		} else {
			// calculate offset based on movement velocity
			vmOfs.zero();

			float tmp = playerVelocity.len() - cfg.offetVelBase;

			if(tmp > 0) {
				float tmp2 = 250.0 - cfg.offetVelBase;
				if( tmp > tmp2 )
					tmp = tmp2;
				float frac = tmp / tmp2;	
				vmOfs += frac * cfg.offsetVel;
			}
		}
		// for each coordinate
		for(u32 i = 0; i < 3; i++) {
			float ofsDelta = vmOfs[i] - currentPosOffset[i];
			currentPosOffset[i] += frameTimeSeconds * ofsDelta * cfg.offsetSpeed;
		}

		currentMovement += currentPosOffset;

		// clamp the gun offset to the offset limit
		vec3_c vMovementNormalized;
		float vMovementLen = VectorNormalize2(currentMovement, vMovementNormalized);
		if(cfg.offsetMax < vMovementLen) {
			currentMovement = cfg.offsetMax * vMovementNormalized;
		}
	}
public:
	viewModelAnimator_c() {
		currentAmplitude = 0.f;
		currentPhase = 0.f;
		currentPosOffset.set(0,0,0);
		currentMovement.set(0,0,0);
	}
	void setConfig(const viewModelAnimationConfig_s &newCFG) {
		cfg = newCFG;
	}
	void calcViewModelOffset(bool isOnGround, const vec3_c &velocity, const vec3_c &viewAngles, float frameTimeSeconds) {
		if (isOnGround) {
			float vel = velocity.len();
			currentPhase = frameTimeSeconds * M_PI * (vel * 0.0015 + 0.9) + currentPhase;
			if(currentAmplitude != 0.0)
				vel = vel * 0.5;
			currentAmplitude = vel;

			currentAmplitude = (1.0 - fabs(viewAngles[PITCH]) * 0.01111111138015985 * 0.5) * 0.5 * currentAmplitude;
		} else {
			if(currentAmplitude > 0.0) {
				currentAmplitude -= (2.f * frameTimeSeconds * currentAmplitude);
			}
		}
		if(currentAmplitude == 0.f)
			currentPhase = 0.f;
		//g_core->Print("Phase: %f, Amplitude: %f\n",currentPhase,currentAmplitude);
		calcMovementInternal(velocity,isOnGround,frameTimeSeconds);
	}
	const vec3_c &getCurrentMovement() const {
		return currentMovement;
	}
};

static viewModelAnimator_c viewModelMovement;
static viewModelAnimationConfig_s viewModelMovementConfig;

void CG_RunViewModel() {
	if(cg_printViewWeaponClipSize.getInt()) {
		if(cg.snap) {
			g_core->Print("ViewWeapon clip %i/%i\n",cg.snap->ps.viewWeaponCurClipSize,cg.snap->ps.viewWeaponMaxClipSize);
		}
	}

	int viewModelEntity = cg.snap->ps.curWeaponEntNum;
	if(cg_thirdPerson.integer) {
		CG_FreeViewModelEntity();
		if(viewModelEntity != ENTITYNUM_NONE && cg_entities[viewModelEntity].rEnt) {
			cg_entities[viewModelEntity].rEnt->showModel();
		}
		return;
	}
	if(viewModelEntity == ENTITYNUM_NONE) {
		CG_FreeViewModelEntity();
		return;
	}
	if(cg_entities[viewModelEntity].rEnt) {
		//cg_entities[viewModelEntity].rEnt->hideModel();
		cg_entities[viewModelEntity].rEnt->setThirdPersonOnly(true);
	}

	// local weapons offset (affected by cg_gunX/Y/Z cvars)
	vec3_c localOfs(0,0,0);
	// local weapon rotation (affected by cg_gunRotX/Y/Z cvars)
	vec3_c localRot(0,0,0);

	rModelAPI_i *viewModel;
	if(cg.snap->ps.customViewRModelIndex) {
		viewModel = cgs.gameModels[cg.snap->ps.customViewRModelIndex];
	} else {
		if(cg_entities[viewModelEntity].rEnt) {
			viewModel = cg_entities[viewModelEntity].rEnt->getModel();
		} else {
			viewModel = 0;
		}
	}
	if(viewModel == 0) {
		CG_FreeViewModelEntity();
		return;
	}
	// add hardcoded gun offset
	if(!stricmp(viewModel->getName(),"models/weapons2/plasma/plasma.md3")
		|| !stricmp(viewModel->getName(),"models/weapons2/railgun/railgun.md3")
		|| !stricmp(viewModel->getName(),"models/weapons2/rocketl/rocketl.md3")
		|| !stricmp(viewModel->getName(),"models/weapons2/shotgun/shotgun.md3")
		// it could be better for grenade launcher
		|| !stricmp(viewModel->getName(),"models/weapons2/grenadel/grenadel.md3")) {
		localOfs.set(5,-5,-10);
	} else if(!stricmp(viewModel->getName(),"models/weapons/w_physics.mdl")) {
		// Half Life2 physgun (for weapon_physgun)
		// "w_*" is a worldmodel
		// set 90 yaw rotation (around Z axis)
		localRot.set(0,0,90);
		localOfs.set(5,-5,-10);
	} else if(!stricmp(viewModel->getName(),"models/weapons/v_physcannon.mdl")) {
		// "v_*" is a viewmodel
#if 0
		localRot.set(0,15,90);
		localOfs.set(-35,0,-50);
#else
		localRot.set(0,0,90);
		localOfs.set(-15,0,-65);
#endif
	} else {
		localRot = cg.snap->ps.viewModelAngles;
		localOfs = cg.snap->ps.viewModelOffset;

		if(viewModel->isDeclModel()) {
			localOfs += viewModel->getDeclModelAPI()->getOffset();
		}
	}
	//g_core->Print("Player velocity: %f %f %f\n",cg.snap->ps.velocity.x,cg.snap->ps.velocity.y,cg.snap->ps.velocity.z);

	// calculate viewmodel bobbing offset based on player velocity
	viewModelMovement.setConfig(viewModelMovementConfig);
	viewModelMovement.calcViewModelOffset(cg.snap->ps.isOnGround(),cg.snap->ps.velocity,cg.refdefViewAngles,cg.frametime*0.001f);
	const vec3_c &currentMovement = viewModelMovement.getCurrentMovement();
	localOfs += currentMovement;
	if(cg_printViewModelBobbingOffset.getInt()) {
		g_core->Print("Viewmodel bobbing movement: %f %f %f\n",currentMovement.x,currentMovement.y,currentMovement.z);
	}

	if(cg_printCurViewModelName.getInt()) {
		g_core->Print("Current viewmodel name: %s\n",viewModel->getName());
	}
	if(cg_printCurViewModelAnimationCount.getInt()) {
		g_core->Print("Current viewmodel animation count: %i\n",viewModel->getNumAnims());
	}
	if(cg_printCurViewModelBoneNames.getInt()) {
		g_core->Print("Current viewmodel %s bonenames:\n",viewModel->getName());
		viewModel->printBoneNames();
	}

	CG_AllocViewModelEntity();

	vec3_c origin = cg.refdefViewOrigin;
	vec3_c angles = cg.refdefViewAngles;

	// apply extra gun offset
	localOfs.x += cg_gunX.getFloat();
	localOfs.y += cg_gunY.getFloat();
	localOfs.z += cg_gunZ.getFloat();

	// apply extra gun rotation
	localRot.x += cg_gunRotX.getFloat();
	localRot.y += cg_gunRotY.getFloat();
	localRot.z += cg_gunRotZ.getFloat();

	if(localRot.isNull()==false) {
		matrix_c m;
		m.fromAngles(angles);
		m.rotateX(localRot.x);
		m.rotateY(localRot.y);
		m.rotateZ(localRot.z);
		angles = m.getAngles();
	}

	// add local offset to hand origin
	origin.vectorMA(origin,cg.refdefViewAxis[0],localOfs.x);
	origin.vectorMA(origin,cg.refdefViewAxis[1],localOfs.y);
	origin.vectorMA(origin,cg.refdefViewAxis[2],localOfs.z);

	// always update viewmodel position
	cg_viewModelEntity->setOrigin(origin);
	cg_viewModelEntity->setAngles(angles);
	cg_viewModelEntity->setFirstPersonOnly(true);
	// set viewmodel model
	//rModelAPI_i *viewModel = rf->registerModel("models/testweapons/xrealMachinegun/machinegun_view.md5mesh");
	cg_viewModelEntity->setModel(viewModel);
	int viewModelAnimFlags;
	if(stricmp(cg_forceViewModelAnimationFlags.getStr(),"none")) {
		viewModelAnimFlags = cg_forceViewModelAnimationFlags.getInt();
	} else {
		viewModelAnimFlags = cg.snap->ps.viewModelAnimFlags;
	}
	if(stricmp(cg_forceViewModelAnimationIndex.getStr(),"none")) {
		int index = cg_forceViewModelAnimationIndex.getInt();
		cg_viewModelEntity->setDeclModelAnimLocalIndex(index,viewModelAnimFlags);
	} else if(stricmp(cg_forceViewModelAnimationName.getStr(),"none")) {
		const char *animName = cg_forceViewModelAnimationName.getStr();
		cg_viewModelEntity->setAnim(animName,viewModelAnimFlags);
	} else {
		const char *animName = CG_ConfigString(CS_ANIMATIONS+cg.snap->ps.viewModelAnim);
		if(cg_printViewModelAnimName.getInt()) {
			g_core->Print("ViewModelAnim: %s, flags %i\n",animName,viewModelAnimFlags);
		}
		cg_viewModelEntity->setAnim(animName,viewModelAnimFlags);
	}
}

bool CG_GetViewModelBonePos(const char *boneName, class vec3_c &out) {
	if(cg_viewModelEntity == 0)
		return true;
	matrix_c mat;
	cg_viewModelEntity->getBoneWorldOrientation(boneName,mat);
	out = mat.getOrigin();
	return false; // ok
}

