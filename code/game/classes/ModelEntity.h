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
// BaseEntity.h - base class for all entities with 3d model

#ifndef __MODELENTITY_H__
#define __MODELENTITY_H__

#include "BaseEntity.h"
#include <shared/str.h>

class Weapon;
class Player;

class ModelEntity : public BaseEntity {
	str renderModelName;
	
	void setDamageZone(const char *zoneName, const char *value);
protected:
	// bullet physics object
	class physObjectAPI_i *body;
	// simplified model for collision detection
	class cMod_i *cmod;
	// cmSkeleton for serverside bones access and animation
	class cmSkelModel_i *cmSkel;
	// extra model decl access
	class modelDeclAPI_i *modelDecl;
	// extra model TIKI access
	class tiki_i *tiki;
	// ragdoll interface
	str ragdollDefName; // set by "ragdoll" key in Doom3; the name of articulatedFigure decl
	class ragdollAPI_i *ragdoll;
	class boneOrQPArray_t *initialRagdolPose; // if this is non-zero, spawned ragdoll bodies will use positions/quaternions from here
	int health;
	float mass;
	bool bTakeDamage;
	bool bUseRModelToCreateDynamicCVXShape;
	bool bUseDynamicConvexForTrimeshCMod;
	bool bPhysicsBodyKinematic;
	bool bRigidBodyPhysicsEnabled; // if false, this->initRigidBodyPhysics will always fail
	str animName; // current animation name
	str torsoAnimName; // torso animation override (for players and monsters)
	mutable vec3_c linearVelocity;
	float pvsBoundsSkinWidth;
	float physBounciness;
	vec3_c sizes; // this is set only by "sizes" keyvalue
	// for model entities
	///vec3_c scale;
	// damage zones (head/torso/arms/legs, etc)
	// they are set through key values from Doom3 entity defs
	// Also used to choose the proper pain animation.
	class damageZonesList_c *damageZones;
	// elapsed time after animation change
	u32 legsAnimationTime;
	u32 torsoAnimationTime;

	int getCurrentAnimationTotalTimeMs() const;
	int getCurrentTorsoAnimationTotalTimeMs() const;
	bool hasDamageZones() const;
	int findBoneDamageZone(int boneNum) const;
	const char *getDamageZoneName(int zoneNum) const;
	void printDamageZones() const;
	virtual const char *getRenderModelName() const {
		return renderModelName;
	}
	void updateAnimations();
public:
	ModelEntity();
	virtual ~ModelEntity();

	DECLARE_CLASS( ModelEntity );

	virtual const class vec3_c &getPhysicsOrigin() const;

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setAngles(const class vec3_c &newAngles);

	virtual void setRenderModel(const char *newRModelName);
	bool hasRenderModel(const char *checkRModelName) const;
	bool setColModel(const char *newCModelName);
	virtual bool setColModel(class cMod_i *newCModel);
	void setRagdollName(const char *ragName);
	void setRenderModelSkin(const char *newSkinName);
	void setSpriteModel(const char *newSpriteMaterial, float newSpriteRadius);
	// used in RTCW and ET for player heads
	// Player body is mds/mdm skeletal model and head is mdc keyframed (md3).
	void setRenderModelAttachment(u32 index, const char*boneName, const char *modelName);

	// for skeletal models
	virtual int getBoneNumForName(const char *boneName);

	int findAnimationIndex(const char *animName);

	void setAnimation(const char *animName);
	void setInternalAnimationIndex(int newAnimIndex);
	void setTorsoAnimation(const char *animName);

	bool hasDeclAnimation(const char *animName) const;

	// this is just an approximation, the model shape
	// might be slighty different on a client due
	// to lags and interpolation
	void getCurrentBonesArray(class boneOrArray_c &out);


	virtual void runPhysicsObject();

	void setPhysicsObjectKinematic(bool newBKinematic);
	void setRigidBodyPhysicsEnabled(bool bRBPhysEnable);
	void setPhysBounciness(float newBounciness);

	// returns true on error
	bool initRagdollRenderAndPhysicsObject(const char *afName);

	// rigid body physics
	void initRigidBodyPhysics(); // this will work only if this->bRigidBodyPhysicsEnabled is true
	void initStaticBodyPhysics(); // static body
	void destroyPhysicsObject();
	// ragdoll physics
	void initRagdollPhysics();
	//void destroyPhysicsRagdoll();

	// called after all of the key values are set
	virtual void postSpawn();

	virtual void applyCentralForce(const vec3_c &velToAdd);
	virtual void applyCentralImpulse(const vec3_c &impToAdd);
	virtual void applyPointImpulse(const vec3_c &impToAdd, const vec3_c &pointAbs);
	virtual void applyTorque(const vec3_c &torqueToAdd);
	virtual const vec3_c getLinearVelocity() const;
	virtual void setLinearVelocity(const vec3_c &newVel);
	virtual const vec3_c getAngularVelocity() const;
	virtual void setAngularVelocity(const vec3_c &newAVel);
	virtual void runWaterPhysics(float curWaterLevel);

	virtual void debugDrawCollisionModel(class rDebugDrawer_i *dd);

	virtual bool hasPhysicsObject() const;
	virtual bool hasCollisionModel() const;
	virtual bool isDynamic() const;
	virtual class physObjectAPI_i *getRigidBody() const {
		return body;
	}

	virtual void setKeyValue(const char *key, const char *value); 
	virtual void iterateKeyValues(class keyValuesListener_i *listener) const;

	virtual void runFrame();

	virtual void getLocalBounds(aabb &out) const;
	virtual bool getBoneWorldOrientation(u32 tagNum, class matrix_c &out);

	void setHealth(int newHealth) {
		this->health = newHealth;
	}

	virtual void onDeath();
	virtual void damage(int damage);
	// used to inflict damage specified by Doom3 damage def
	virtual void applyDamageFromDef(const char *defName, const class trace_c *tr);
	virtual void onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damage);

	virtual bool traceWorldRay(class trace_c &tr);
	virtual bool traceLocalRay(class trace_c &tr);
};

#endif // __MODELENTITY_H__
