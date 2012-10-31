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

class ModelEntity : public BaseEntity {
	str renderModelName;
protected:
	// bullet physics object
	class btRigidBody *body;
	// simplified model for collision detection
	class cMod_i *cmod;
public:
	ModelEntity();
	virtual ~ModelEntity();

	DECLARE_CLASS( ModelEntity );

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setAngles(const class vec3_c &newAngles);

	void setRenderModel(const char *newRModelName);
	void setColModel(const char *newCModelName);
	void setColModel(class cMod_i *newCModel);

	virtual void runPhysicsObject();

	void initRigidBodyPhysics();

	void debugDrawCMObject(class rDebugDrawer_i *dd);

	bool hasPhysicsObject() const {
		if(body)
			return true;
		return false;
	}

	virtual void setKeyValue(const char *key, const char *value); 

	virtual void runFrame();

	virtual void getLocalBounds(aabb &out) const;

	virtual bool traceWorldRay(class trace_c &tr);
	virtual bool traceLocalRay(class trace_c &tr);

	virtual void debugDraw(class rDebugDrawer_i *dd) {
		BaseEntity::debugDraw(dd);
		debugDrawCMObject(dd);
	}
};

#endif // __MODELENTITY_H__
