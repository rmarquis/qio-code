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
#include <math/matrix.h>

class BaseEntity : public safePtrObject_c {
	struct entityState_s *_myEntityState; // this is NULL only for players !!! (they are using playerState_s instead)
	matrix_c matrix;
	// for entity attaching
	arraySTD_c<BaseEntity*> attachments; // for parents
	BaseEntity *parent; // for children
protected:
	// entity's edict, set once during entity allocation
	struct edict_s *myEdict;
public:
	BaseEntity();
	virtual ~BaseEntity();

	DECLARE_CLASS( BaseEntity );

	virtual void setKeyValue(const char *key, const char *value);

	// maybe I should put those functions in ModelEntity...
	void link();
	void unlink();
	void recalcABSBounds();

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setAngles(const class vec3_c &newAngles);
	virtual void setMatrix(const class matrix_c &newMat);
	const class vec3_c &getOrigin() const;
	virtual vec3_c getCModelOrigin() const {
		return getOrigin(); // overriden in Player class
	}
	const class vec3_c &getAngles() const;
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

	edict_s *getEdict() {
		return this->myEdict;
	}
	u32 getEntNum() const;
	virtual BaseEntity *getOwner() const {
		return 0;
	}

	void setParent(BaseEntity *newParent, int tagNum = -1);
	void detachFromParent();

	virtual void getLocalBounds(aabb &out) const;

	virtual bool traceWorldRay(class trace_c &tr) {
		return false;
	}	
	//virtual bool traceLocalRay(class trace_c &tr) {
	//	return false;
	//}
	virtual void runFrame() {

	}
	virtual void doUse(class Player *activator) {

	}
	virtual void onBulletHit(const vec3_c &dirWorld, int damage) {

	}
	virtual void debugDrawAbsBounds(class rDebugDrawer_i *dd);

	virtual void debugDrawCollisionModel(class rDebugDrawer_i *dd) {
		// this will be overriden by ModelEntity
	}
};

void BE_SetForcedEdict(edict_s *nfe);

#endif // __BASEENTITY_H__