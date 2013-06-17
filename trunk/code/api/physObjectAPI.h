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
// physObjectAPI.h
#ifndef __PHYSOBJECT_API_H__
#define __PHYSOBJECT_API_H__

class physObjectAPI_i {
public:
	virtual void setOrigin(const class vec3_c &newXYZ) = 0;
	virtual const class vec3_c getRealOrigin() const = 0;
	virtual void getCurrentMatrix(class matrix_c &out) const = 0;
	virtual void applyCentralForce(const class vec3_c &velToAdd) = 0;
	virtual void applyCentralImpulse(const class vec3_c &impToAdd) = 0;
	// linear velocity access (in Quake units)
	virtual const class vec3_c getLinearVelocity() const = 0;
	virtual void setLinearVelocity(const class vec3_c &newVel) = 0;
	// angular velocity access
	virtual const vec3_c getAngularVelocity() const = 0;
	virtual void setAngularVelocity(const class vec3_c &newAVel) = 0;
	// misc
	virtual void setKinematic() = 0;
	virtual bool isDynamic() const = 0;
	virtual void setEntityPointer(class BaseEntity *ent) = 0;
	virtual BaseEntity *getEntityPointer() const = 0;
	// water physics
	virtual void runWaterPhysics(float curWaterLevel) = 0;
};

#endif // __PHYSOBJECT_API_H__
