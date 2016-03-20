/*
============================================================================
Copyright (C) 2013 V.

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
// physAPI.h - interface of physics engine module
#ifndef __PHYS_API_H__
#define __PHYS_API_H__

#include "iFaceBase.h"

class physWorldAPI_i {
public:
	virtual void init(const class vec3_c &newGravity) = 0;
	virtual bool loadMap(const char *mapName) = 0;
	virtual void runFrame(float frameTime) = 0;
	virtual void shutdown() = 0;

	//
	//	RIGID BODIES
	//
	// mass 0 means that object is static (non moveable)
	virtual class physObjectAPI_i *createPhysicsObject(const struct physObjectDef_s &def) = 0;
	virtual void destroyPhysicsObject(class physObjectAPI_i *p) = 0;
	//
	//	CONSTRAINTS
	//
	virtual class physConstraintAPI_i *createConstraintBall(const vec3_c &pos, physObjectAPI_i *b0, physObjectAPI_i *b1) = 0;
	virtual class physConstraintAPI_i *createConstraintHinge(const vec3_c &pos, const vec3_c &axis, physObjectAPI_i *b0, physObjectAPI_i *b1) = 0;
	virtual void destroyPhysicsConstraint(physConstraintAPI_i *p) = 0;
	//
	//
	//	CHARACTER CONTROLLER
	//
	virtual class physCharacterControllerAPI_i *createCharacter(const class vec3_c &pos, float characterHeight,  float characterWidth) = 0;
	virtual void freeCharacter(class physCharacterControllerAPI_i *p) = 0;
	//
	//	VEHICLES
	//
	virtual class physVehicleAPI_i *createVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cm) = 0;
	virtual void removeVehicle(class physVehicleAPI_i *v) = 0;

	// world
	virtual void setGravity(const vec3_c &newGravity) = 0;
	virtual const vec3_c &getGravity() const = 0;
	virtual bool traceRay(class trace_c &tr, const class BaseEntity *skip = 0) = 0;
};

#define GPHYSICS_API_IDENTSTR "GamePhysicsAPI0001"

class physDLLAPI_i : public iFaceBase_i {
public:
	virtual void initPhysicsSystem() = 0;
	virtual void shutdownPhysicsSystem() = 0;
	virtual physWorldAPI_i *allocWorld(const char *debugName) = 0;
	virtual void freeWorld(physWorldAPI_i *w) = 0;
	virtual void doDebugDrawing(class rDebugDrawer_i *dd) { };
};

extern class physDLLAPI_i *g_physAPI;
extern class physWorldAPI_i *g_physWorld;

#endif // __PHYS_API_H__