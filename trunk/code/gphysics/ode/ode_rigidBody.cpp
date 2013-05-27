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
// btp_rigidBody.cpp
#include "ode_rigidBody.h"
#include "ode_shape.h"
#include "ode_headers.h"
#include "ode_convert.h"
#include "ode_world.h"
#include <shared/physObjectDef.h>
#include <math/quat.h>

odeRigidBody_c::odeRigidBody_c() {
	body = 0;
	geom = 0;
	bounciness = 0.f;
}
odeRigidBody_c::~odeRigidBody_c() {
	if(body) {
		dBodyDestroy(body);
		body = 0;
	}
	if(geom) {
		dGeomDestroy(geom);
		geom = 0;
	}
}
static void ODE_SetMatrix4x4(dReal *_dvR, dReal *_dvPos, const float *mat4x4) {
	_dvR[0] = mat4x4[0];
	_dvR[4] = mat4x4[1];
	_dvR[8] = mat4x4[2];
	//mat4x4[3]=  0;
	_dvR[1] = mat4x4[4];
	_dvR[5] = mat4x4[5];
	_dvR[9] = mat4x4[6];
	//mat4x4[7]=  0;
	_dvR[2] = mat4x4[8];
	_dvR[6] = mat4x4[9];
	_dvR[10] = mat4x4[10];
	//mat4x4[11]= 0;
	_dvPos[0] = mat4x4[12];
	_dvPos[1] = mat4x4[13];
	_dvPos[2] = mat4x4[14];
	//mat4x4[15]= 1;
}
void odeRigidBody_c::init(class odePhysicsWorld_c *world, class odeColShape_c *newShape, const struct physObjectDef_s &def) {
	shape = newShape;

	bounciness = def.bounciness;

	geom = newShape->getODEGeom();

	if(def.isStatic())
	{
		body = 0;
		return;
	}

    body = dBodyCreate (world->getODEWorld());
	dBodySetData(body,this);

	//float odeDensity;
	float odeDensity = def.mass*0.5f;
	dMass m;
	if(newShape->isBox()) {
		const float *sizes = newShape->getBoxSizes();
		dMassSetBox (&m,odeDensity,sizes[0],sizes[1],sizes[2]);
	} else if(newShape->isTriMesh()) {
		dMassSetTrimesh( &m, odeDensity, geom );
		m.c[0] = 0;
		m.c[1] = 0;
		m.c[2] = 0;
	} else{
		dMassSetSphere (&m,odeDensity,0.5);
	}
    dBodySetMass (body,&m);
    dGeomSetBody (geom,body);

	const matrix_c &com = shape->getCenterOfMassTransform();
	matrix_c odeMat = def.transform;
	odeMat.scaleOriginXYZ(QIO_TO_ODE);	
	odeMat = com * odeMat;

	dMatrix3 odeRot;
	dReal odePos[3];
	ODE_SetMatrix4x4(odeRot,odePos,odeMat);

    // set initial position
    dBodySetPosition (body,odeMat.getOrigin()[0],odeMat.getOrigin()[1],odeMat.getOrigin()[2]);	
	dBodySetRotation(body,odeRot);
}
void odeRigidBody_c::setOrigin(const class vec3_c &newPos) {
	vec3_c scaled = newPos * QIO_TO_ODE;
	if(body == 0) {
		dGeomSetPosition(geom,scaled.x,scaled.y,scaled.z);
		return;
	}
	dBodySetPosition(body,scaled.x,scaled.y,scaled.z);
}
static void ODE_GetMatrix4x4(const dReal *_dvR, const dReal *_dvPos, float *mat4x4) {
	mat4x4[0]=  _dvR[0];
	mat4x4[1]=  _dvR[4];
	mat4x4[2]=  _dvR[8];
	mat4x4[3]=  0;
	mat4x4[4]=  _dvR[1];
	mat4x4[5]=  _dvR[5];
	mat4x4[6]=  _dvR[9];
	mat4x4[7]=  0;
	mat4x4[8]=  _dvR[2];
	mat4x4[9]=  _dvR[6];
	mat4x4[10]= _dvR[10];
	mat4x4[11]= 0;
	mat4x4[12]= _dvPos[0];
	mat4x4[13]= _dvPos[1];
	mat4x4[14]= _dvPos[2];
	mat4x4[15]= 1;
}
static void ODE_GetMatrix4x4(dBodyID ode_body, float *mat4x4) {
	const dReal *_dvR = dBodyGetRotation(ode_body);
	const dReal *_dvPos = dBodyGetPosition(ode_body);
	ODE_GetMatrix4x4(_dvR,_dvPos,mat4x4);
}
static void ODE_GetMatrix4x4(dGeomID ode_geom, float *mat4x4) {
	const dReal *_dvR = dGeomGetRotation(ode_geom);
	const dReal *_dvPos = dGeomGetPosition(ode_geom);
	ODE_GetMatrix4x4(_dvR,_dvPos,mat4x4);
}
const class vec3_c odeRigidBody_c::getRealOrigin() const {
	class matrix_c mat;
	if(body) {
		ODE_GetMatrix4x4(body,mat);
	} else {
		ODE_GetMatrix4x4(geom,mat);
	}
	mat.scaleOriginXYZ(ODE_TO_QIO);
	return mat.getOrigin();
}


void odeRigidBody_c::getCurrentMatrix(class matrix_c &out) const {
	if(body) {
		ODE_GetMatrix4x4(body,out);
	} else {
		ODE_GetMatrix4x4(geom,out);
	}
	if(this->shape->hasCenterOfMassTransform()) {
		out = out * this->shape->getCenterOfMassTransform().getInversed();
	}
	out.scaleOriginXYZ(ODE_TO_QIO);
}
void odeRigidBody_c::getPhysicsMatrix(class matrix_c &out) const {
	if(body) {
		ODE_GetMatrix4x4(body,out);
	} else {
		ODE_GetMatrix4x4(geom,out);
	}
	// dont add center of mass transform
	out.scaleOriginXYZ(ODE_TO_QIO);
}
void odeRigidBody_c::applyCentralForce(const class vec3_c &velToAdd) {
	if(body == 0)
		return;
	vec3_c scaled = velToAdd * QIO_TO_ODE;
	dBodyEnable(body);
	dBodyAddForce(body,scaled.x,scaled.y,scaled.z);
///	bulletRigidBody->activate(true);
//	bulletRigidBody->applyCentralForce((velToAdd*QIO_TO_ODE).floatPtr());
}
void odeRigidBody_c::applyCentralImpulse(const class vec3_c &impToAdd) {
	if(body == 0)
		return;
	vec3_c scaled = impToAdd * QIO_TO_ODE;
	// HACK
	scaled *= 5.f;
	dBodyEnable(body);
	dBodyAddForce(body,scaled.x,scaled.y,scaled.z);
///	bulletRigidBody->activate(true);
//	bulletRigidBody->applyCentralImpulse((impToAdd*QIO_TO_ODE).floatPtr());
}
// linear velocity access (in Quake units)
const class vec3_c odeRigidBody_c::getLinearVelocity() const {
	if(body == 0)
		return vec3_c(0,0,0);
	vec3_c ret = dBodyGetLinearVel(body);
	return ret*ODE_TO_QIO;
}
void odeRigidBody_c::setLinearVelocity(const class vec3_c &newVel) {
	if(body == 0)
		return;
	vec3_c scaled = newVel * QIO_TO_ODE;
	dBodySetLinearVel(body,scaled.x,scaled.y,scaled.z);
}
// angular velocity access
const vec3_c odeRigidBody_c::getAngularVelocity() const {
	if(body == 0)
		return vec3_c(0,0,0);
	vec3_c ret = dBodyGetAngularVel(body);
	return ret;
}
void odeRigidBody_c::setAngularVelocity(const class vec3_c &newAVel) {
	dBodySetAngularVel(body,newAVel.x,newAVel.y,newAVel.z);
}
bool odeRigidBody_c::isDynamic() const {
	if(body == 0)
		return false;
	//if(bulletRigidBody->isStaticObject())
	//	return false;
	return true;
}
void odeRigidBody_c::setEntityPointer(class BaseEntity *ent) {
	myEntity = ent;
}
BaseEntity *odeRigidBody_c::getEntityPointer() const {
	return myEntity;
}
// water physics
void odeRigidBody_c::runWaterPhysics(float curWaterLevel) {

}
