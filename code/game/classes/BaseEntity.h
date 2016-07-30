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

#ifndef __BASEENTITY_H__
#define __BASEENTITY_H__

#include "../g_classes.h" // DECLARE_CLASS, etc
#include <shared/array.h>
#include <shared/safePtr.h>
#include <shared/str.h>
#include <shared/eventReceiverAPI.h>
#include <math/matrix.h>
#include <protocol/entityState.h>
// LUA event callbacks
#include "../lua/g_lua_eventHandler.h"
#include <api/serverAPI.h>
#include <api/keyValueSetter.h>


class BaseEntity : public safePtrObject_c, public eventReceiverBaseAPI_i, public keyValueSetter_i {
	struct entityState_s *_myEntityState; // this is NULL only for players !!! (they are using playerState_s instead)
	matrix_c matrix;
	// for entity attaching
	arraySTD_c<BaseEntity*> attachments; // for parents
	safePtr_c<BaseEntity> parent; // for children
	str targetName; // name of this entity; set in radiant or trough script
	str target; // entity's target - FIXME: use safePtr<BaseEntity> here instead of string?
	// for ET/RTCW .script support
	str scriptName;
	// used to handle ET/RTCW script execution
	class wsEntityInstance_c *wsScript;
	// our own internal event system 
	class eventList_c *eventList;
#ifdef G_ENABLE_LUA_SCRIPTING
	// LUA event callbacks
	luaEventHandlerList_c lua_runFrameHandlers;
#endif
	bool bMarkedForDelete;

protected:
	// called through eventReceiverBaseAPI_i
	virtual void processEvent(class eventBaseAPI_i *ev);
	// sets the entity type
	void setEntityType(int newEType);
	// entity's edict, set once during entity allocation
	struct edict_s *myEdict;
	// loaded from .map file
	int spawnFlags;
public:
	BaseEntity();
	virtual ~BaseEntity();

	DECLARE_CLASS( BaseEntity );
	
	void printTouchingAreas() const;
	virtual void printInfo() const;

	// zOfs should be usually the mins.z value of local entity bbox
	void dropToFloor(float zOfs);

	virtual void setKeyValue(const char *key, const char *value);
	virtual void iterateKeyValues(class keyValuesListener_i *listener) const;
	void applyKeyValues(const class entDefAPI_i *list);
	void postEvent(int execTime, const char *eventName, const char *arg0 = 0, const char *arg1 = 0, const char *arg2 = 0, const char *arg3 = 0);
	void removeAfterDelay(int delay);
	virtual const char *getRenderModelName() const {
		return "";
	}

	// maybe I should put those functions in ModelEntity...
	void link();
	void unlink();
	void recalcABSBounds();

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setAngles(const class vec3_c &newAngles);
	virtual void setMatrix(const class matrix_c &newMat);
	const class vec3_c &getOrigin() const;
	class vec3_c getCenter() const;
	virtual const class vec3_c &getPhysicsOrigin() const {
		return getOrigin();
	}
	virtual vec3_c getCModelOrigin() const {
		return getOrigin(); // overriden in Player class
	}
	const class vec3_c &getAngles() const;
	const class vec3_c &getScale() const;
	const class matrix_c &getMatrix() const {
		return matrix;
	}
	vec3_c getForward() const {
		return matrix.getForward();
	}
	vec3_c getLeft() const {
		return matrix.getLeft();
	}
	vec3_c getUp() const {
		return matrix.getUp();
	}
	vec3_c transformWorldPointToEntityCoordinates(const vec3_c &in) const {
		return matrix.getInversed().transformPoint2(in);
	}

	edict_s *getEdict() {
		return this->myEdict;
	}
	u32 getEntNum() const;
	virtual BaseEntity *getOwner() const {
		return 0;
	}

	bool hasClassName(const char *className) const;

	virtual bool setColModel(class cMod_i *newCModel) {
		return true;
	}
	virtual bool setColModel(const char *s) {
		return true;
	}
	// for Movers
	virtual bool isMover() const {
		return false;
	}
	virtual bool isMoverMoving() const {
		return false;
	}
	virtual const char *getMoverTeam() const {
		return "";
	}

	// "targetname" field (unique name of this entity)
	const char *getTargetName() const;
	void setTargetName(const char *newTargetName);
	bool hasTargetName() const;
	// "scriptname" field - for ET/RTCW
	const char *getScriptName() const;
	void setScriptName(const char *newTargetName);
	bool hasScriptName() const;
	// "target" field
	const char *getTarget() const;
	void setTarget(const char *newTarget);
	bool hasTarget() const;

	// called by trigger on its targets
	virtual void triggerBy(class BaseEntity *from, class BaseEntity *activator) {

	}

	virtual void setRenderModel(const char *newRModelName) {

	}
	virtual bool hasPhysicsObject() const {
		return false;
	}
	virtual bool hasCollisionModel() const {
		return false;
	}
	virtual bool isDynamic() const {
		return false;
	}
	virtual class physObjectAPI_i *getRigidBody() const {
		return 0;
	}

	virtual void applyCentralForce(const vec3_c &velToAdd) {

	}
	virtual void applyCentralImpulse(const vec3_c &impToAdd) {

	}
	virtual void applyPointImpulse(const vec3_c &impToAdd, const vec3_c &pointAbs) {

	}
	virtual void runWaterPhysics(float curWaterLevel) {

	}
	// used to create rigid bodies
	virtual void postSpawn();
	// used to create constraints linking rigid bodies
	virtual void postSpawn2() {
	
	}

	u32 processPendingEvents();

	// hides entity from clients (entity will not be send to clients)
	void hideEntity();
	// shows hidden entity (entity will be send to clients again)
	void showEntity();
	void toggleEntityVisibility();

	void setParent(BaseEntity *newParent, int tagNum = -1, bool enableLocalOffset = false);
	void setParent(BaseEntity *newParent, const char *tagName, bool enableLocalOffset = false);
	void setParent(const char *parentTargetName, int tagNum = -1, bool enableLocalOffset = false);
	void detachFromParent();
	// "this" is an attachment (not parent)
	void setLocalAttachmentAngles(const vec3_c &newAngles);
	// update origin/angles/matrix fields of entity attached to another
	void updateAttachmentOrigin();

	// returns true on error
	virtual bool getBoneWorldOrientation(u32 tagNum, class matrix_c &out);

	// entity light
	void setEntityLightRadius(float newEntityLightRadius);
	void setEntityLightColor(const vec3_c &color);
	// entity emitter
	void setTrailEmitterMaterial(const char *matName);
	void setTrailEmitterSpriteRadius(float newRadius);
	void setTrailEmitterInterval(int newInterval);

	virtual void getLocalBounds(aabb &out) const;
	const class aabb &getAbsBounds() const;

	// returns the count of BSP areas touching this entity
	u32 getNumTouchingAreas() const;
	u32 getTouchingArea(u32 localIdx) const;
	// for Doors, etc
	bool openAreaPortalIfPossible();
	bool closeAreaPortalIfPossible();

	// RTCW / ET scripting
	void startWolfScript(const class wsScriptBlock_c *block);

	// for lua wrapper
	virtual bool addLuaEventHandler(struct lua_State *L, const char *eventName, int func);

	virtual int getBoneNumForName(const char *boneName) {
		return -1;
	}
	void runLuaFrameHandlers() {
#ifdef G_ENABLE_LUA_SCRIPTING
		lua_runFrameHandlers.runCallbacks("e",this->getEdict());
#endif
	}

	virtual bool traceWorldRay(class trace_c &tr) {
		return false;
	}	
	//virtual bool traceLocalRay(class trace_c &tr) {
	//	return false;
	//}
	virtual void runFrame() {

	}
	void runWolfScript();
	virtual bool doUse(class Player *activator) {
		return false;
	}
	virtual void damage(int damage) {

	}
	// used to inflict damage specified by Doom3 damage def
	virtual void applyDamageFromDef(const char *defName, const class trace_c *tr) {

	}
	virtual void onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damage) {

	}
	virtual void debugDrawAbsBounds(class rDebugDrawer_i *dd);

	virtual void debugDrawCollisionModel(class rDebugDrawer_i *dd) {
		// this will be overriden by ModelEntity
	}
};

void BE_SetForcedEdict(edict_s *nfe);

#endif // __BASEENTITY_H__
