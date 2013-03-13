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
// BaseEntity.h - base class for all entities

#include "../g_local.h"
#include "BaseEntity.h"
#include <math/vec3.h>
#include <math/axis.h>
#include "Player.h"
#include <api/serverAPI.h>
#include <shared/keyValuesListener.h>

DEFINE_CLASS(BaseEntity, "None");
// Quake3 "misc_teleporter_dest" for q3dm0 teleporter 
DEFINE_CLASS_ALIAS(BaseEntity, misc_teleporter_dest);
// Q3 "target_position" for q3dm11 teleporter (yes, "misc_teleporter_dest" is not used on that map!)
DEFINE_CLASS_ALIAS(BaseEntity, target_position);

// use this to force BaseEntity::ctor() to use a specific edict instead of allocating a new one
static edict_s *be_forcedEdict = 0;
void BE_SetForcedEdict(edict_s *nfe) {
	be_forcedEdict = nfe;
}

BaseEntity::BaseEntity() {
	if(be_forcedEdict) {
		myEdict = be_forcedEdict;
		be_forcedEdict = 0;
		// HACK: use playerState_s for players
		Player *pl = (Player*)this;
		// playerState_s is a superset of entityState, so we can point s to ps
		playerState_s *ps = pl->getPlayerState();
		myEdict->s = ps;
		_myEntityState = 0;
		// save clientNum
		ps->clientNum = myEdict - g_entities;
		// set entity type
		myEdict->s->eType = ET_PLAYER;
	} else {
		myEdict = G_Spawn();
		// if that's not a player, alloc an entityState_s
		_myEntityState = myEdict->s = new entityState_s;
		// set entity type
		myEdict->s->eType = ET_GENERAL;
	}
	// set entityState_s::number
	myEdict->s->number = myEdict - g_entities;
	myEdict->ent = this;
	parent = 0;
	matrix.identity();
}
BaseEntity::~BaseEntity() {
	detachFromParent();
	unlink(); // fries edict_s::bspBoxDesc
	if(_myEntityState) {
		// free the entityState_s that we have alloced
		assert(_myEntityState == myEdict->s);
		delete _myEntityState;
		_myEntityState = 0;
	}
	//G_FreeEntity(myEdict);
	memset (myEdict, 0, sizeof(*myEdict));
	myEdict->freetime = level.time;
	myEdict->s = 0;
}
void BaseEntity::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"origin")) {
		this->setOrigin(value);
	} else if(!stricmp(key,"angles")) {
		this->setAngles(value);
	} else if(!stricmp(key,"rotation")) {
		axis_c ax;
		ax.fromString(value);
		this->setAngles(ax.toAngles());
	} else if(!stricmp(key,"angle")) {
		// it's used in Q3 maps and somehow
		// in Prey's game/roadhouse as well
		float angle = atof(value);
		this->setAngles(vec3_c(0,angle,0));
	} else if(!stricmp(key,"targetname")) {
		this->setTargetName(value);
	} else if(!stricmp(key,"target")) {
		this->setTarget(value);
	} else if(!stricmp(key,"parent")) {
		this->setParent(value,-1,true);
	} else {

	}
}	
void BaseEntity::iterateKeyValues(class keyValuesListener_i *listener) const {
	vec3_c curPosition = this->getOrigin();
	if(curPosition.isNull() == false) {
		listener->addKeyValue("origin",curPosition);
	}
	vec3_c curAngles = this->getAngles();
	if(curAngles.isNull() == false) {
		listener->addKeyValue("angles",curAngles);
	}
	if(this->hasTargetName()) {
		listener->addKeyValue("targetname",getTargetName());
	}
	if(this->hasTarget()) {
		listener->addKeyValue("target",getTarget());
	}
}
// maybe I should put those functions in ModelEntity...
void BaseEntity::link() {
	// let the server handle the linking
	g_server->linkEntity(this->myEdict);
}
void BaseEntity::unlink() {
	// unlink entity from server
	g_server->unlinkEntity(this->myEdict);
}
void BaseEntity::setOrigin(const vec3_c &newXYZ) {
	myEdict->s->origin = newXYZ;
	matrix.fromAnglesAndOrigin(this->myEdict->s->angles,this->myEdict->s->origin);
	recalcABSBounds();
	link();
}
void BaseEntity::setAngles(const class vec3_c &newAngles) {
	myEdict->s->angles = newAngles;
	matrix.fromAnglesAndOrigin(this->myEdict->s->angles,this->myEdict->s->origin);
	recalcABSBounds();
	link();
}
void BaseEntity::setMatrix(const class matrix_c &newMat) {
	myEdict->s->angles = newMat.getAngles();
	myEdict->s->origin = newMat.getOrigin();	
	this->matrix = newMat;
	recalcABSBounds();
	link();
}
const vec3_c &BaseEntity::getOrigin() const {
	return myEdict->s->origin;
}
const vec3_c &BaseEntity::getAngles() const {
	return myEdict->s->angles;
}
void BaseEntity::recalcABSBounds() {
	aabb tmpLocalBB;
	this->getLocalBounds(tmpLocalBB);
	matrix.transformAABB(tmpLocalBB,myEdict->absBounds);
}
void BaseEntity::getLocalBounds(aabb &out) const {
	out.fromHalfSize(16.f);
}
u32 BaseEntity::getEntNum() const {
	return myEdict->s->number;
}
const char *BaseEntity::getTargetName() const {
	return targetName;
}
void BaseEntity::setTargetName(const char *newTargetName) {
	targetName = newTargetName;
}
bool BaseEntity::hasTargetName() const {
	if(targetName.length())
		return true;
	return false;
}
const char *BaseEntity::getTarget() const {
	return target;
}
void BaseEntity::setTarget(const char *newTarget) {
	target = newTarget;
}
bool BaseEntity::hasTarget() const {
	if(target.length())
		return true;
	return false;
}
void BaseEntity::setParent(BaseEntity *newParent, int tagNum, bool enableLocalOffset) {
	if(parent) {
		detachFromParent();
	}
	if(newParent == 0)
		return;
	parent = newParent;
	myEdict->s->parentNum = newParent->getEntNum();
	myEdict->s->parentTagNum = tagNum;
	if(enableLocalOffset) {
		const matrix_c &thisMat = this->getMatrix();
		const matrix_c &parentMat = parent->getMatrix();
		matrix_c ofs = parentMat.getInversed() * thisMat;
		myEdict->s->parentOffset = ofs.getOrigin();
	} else {
		myEdict->s->parentOffset.zero();
	}
	parent->attachments.push_back(this);
}
void BaseEntity::setParent(const char *parentTargetName, int tagNum, bool enableLocalOffset) {
	setParent(G_FindFirstEntityWithTargetName(parentTargetName),tagNum,enableLocalOffset);
}
void BaseEntity::detachFromParent() {
	if(parent == 0) {
		assert(this->myEdict->s->parentNum == ENTITYNUM_NONE);
		return;
	}
	parent->attachments.remove(this);
	parent = 0;
	myEdict->s->parentNum = ENTITYNUM_NONE;
	myEdict->s->parentTagNum = -1;
}
// BaseEntity has no model so we cant do much here.
// This function is virtual and overriden in ModelEntity
bool BaseEntity::getBoneWorldOrientation(u32 tagNum, class matrix_c &out) {
	out = this->matrix;
	return true; // error
}
// update origin/angles/matrix fields of entity attached to another
void BaseEntity::updateAttachmentOrigin() {
	if(parent == 0) {
		return;
	}
	// recursively update orientation
	parent->updateAttachmentOrigin();

	matrix_c mat;
	if(myEdict->s->parentTagNum == -1) { 
		// parent matrix
		mat = parent->getMatrix();
	} else {
		// get world space matrix of parent tag/bone
		parent->getBoneWorldOrientation(myEdict->s->parentTagNum,mat);
	}
	this->setOrigin(mat.getOrigin());
	this->setAngles(mat.getAngles());
	//if(cent->currentState.parentOffset.isAlmostZero() == false) {
	//	matrix_c matAngles = mat;
	//	matAngles.setOrigin(vec3_origin);
	//	vec3_c ofs;
	//	matAngles.transformPoint(cent->currentState.parentOffset,ofs);
	//	cent->lerpOrigin += ofs;
	//}
}
void BaseEntity::setEntityLightRadius(float newEntityLightRadius) {
	myEdict->s->lightRadius = newEntityLightRadius;
}
const aabb &BaseEntity::getAbsBounds() const {
	return this->myEdict->absBounds;
}
#include <shared/bspBoxDesc.h>
// returns the count of BSP areas touching this entity
u32 BaseEntity::getNumTouchingAreas() const {
	return this->myEdict->bspBoxDesc->getNumAreas();
}
u32 BaseEntity::getTouchingArea(u32 localIdx) const {
	return this->myEdict->bspBoxDesc->getArea(localIdx);
}
#include <api/ddAPI.h>

void BaseEntity::debugDrawAbsBounds(class rDebugDrawer_i *dd) {
	dd->drawBBLines(this->myEdict->absBounds);
}