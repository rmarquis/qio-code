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
// btp_vehicle.cpp
#include "btp_vehicle.h"
#include "btp_convert.h"
#include "btp_cMod2BulletShape.h"
#include <math/matrix.h>
#include <api/cmAPI.h>
#include "btp_world.h"

#define CUBE_HALF_EXTENTS 1.f

static btVector3 wheelDirectionCS0(0,0,-1);
static btVector3 wheelAxleCS(1,0,0);


static float	wheelRadius = 0.5f;
static float	wheelWidth = 0.4f;
static float	wheelFriction = 1000;//BT_LARGE_FLOAT;
static float	suspensionStiffness = 10.f;//;
static float	suspensionDamping = 2.3f;//;
static float	suspensionCompression = 4.4f;//;
static float	rollInfluence = 0.1f;//1.0f;
//
static btScalar suspensionRestLength(0.6f);



btRigidBody* BT_CreateRigidBodyInternal(btDynamicsWorld *dynamicsWorld, float mass, const btTransform& startTransform, btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

//	shape->setMargin(bt_collisionMargin);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#if 1
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

	//body->setContactProcessingThreshold(0);
	//body->setContactProcessingThreshold(1e30);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif//

#ifdef TRYTOFIX_INTERNAL_EDGES
	//enable custom material callback
	body->setCollisionFlags(body->getCollisionFlags()  | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
#endif 

	dynamicsWorld->addRigidBody(body);

	body->setCcdMotionThreshold(32.f*QIO_TO_BULLET);
	body->setCcdSweptSphereRadius(6.f*QIO_TO_BULLET);

	return body;
}

btVehicle_c::btVehicle_c() {
	m_vehicle = 0;
	m_carChassis = 0;
	m_vehicleRayCaster = 0;
	m_wheelShape = 0;
	curEngineForce = 500.f;
	curSteerRot = 0.1f;
}
btVehicle_c::~btVehicle_c() {
	destroyVehicle();
}
void btVehicle_c::getMatrix(matrix_c &out) {
	btTransform trans;
	const btMotionState *ms = m_carChassis->getMotionState();
	ms->getWorldTransform(trans);	
	trans.getOpenGLMatrix(out);
	out.scaleOriginXYZ(BULLET_TO_QIO);
}
void btVehicle_c::setOrigin(const class vec3_c &newPos) {
	btTransform newTrans = m_carChassis->getWorldTransform();
	newTrans.setOrigin((btVector3(newPos*QIO_TO_BULLET)));
	m_carChassis->setWorldTransform(newTrans);
}
void btVehicle_c::init(class bulletPhysicsWorld_c *pWorld, const vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel) {
	btCompoundShape *compound = new btCompoundShape();

	// pass NULL cmodel here to use default car shape
	//cmodel = 0;

	const float hOfs = 0.f;//32.f;
	btTransform localTrans;
	localTrans.setIdentity();
	bool doLocalTrans = true;
	//localTrans effectively shifts the center of mass with respect to the chassis
	if(doLocalTrans) {
		localTrans.setOrigin(btVector3(0,0,hOfs));
	}
	
	if(cmodel) {
		BT_AddCModelToCompoundShape(compound,localTrans,cmodel);
	} else {
		btCollisionShape *chassisShape = new btBoxShape(btVector3(1.f,2.f, 0.5f));
		compound->addChildShape(localTrans,chassisShape);
	}

	btTransform tr;
	tr.setIdentity();
	matrix_c mat;
	mat.fromAnglesAndOrigin(angles,pos*QIO_TO_BULLET);
	tr.setFromOpenGLMatrix(mat);

	//compound->setMargin(bt_collisionMargin);
	m_carChassis = BT_CreateRigidBodyInternal(pWorld->getBTDynamicsWorld(),400,tr,compound);

	m_wheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));
	//m_wheelShape->setMargin(bt_collisionMargin);

	m_vehicleRayCaster = new btDefaultVehicleRaycaster(pWorld->getBTDynamicsWorld());

	//m_tuning.m_maxSuspensionTravelCm *= 100.f;
	m_vehicle = new btRaycastVehicle(m_tuning,m_carChassis,m_vehicleRayCaster);
	
	///never deactivate the vehicle
	m_carChassis->setActivationState(DISABLE_DEACTIVATION);

	pWorld->getBTDynamicsWorld()->addVehicle(m_vehicle);

	float connectionHeight;
	if(cmodel) {
		aabb bb;
		cmodel->getBounds(bb);
		bb.scaleBB(QIO_TO_BULLET);
		connectionHeight = bb.mins.z;
		connectionHeight += wheelRadius*0.5 + hOfs;
	} else if(doLocalTrans) {
		connectionHeight = 1.2f + hOfs;
	} else 
	{
		connectionHeight = 1.2f + hOfs;
	}

	m_vehicle->setCoordinateSystem(0,2,1);

	bool bDefaultWheels = true;
	if(cmodel == 0 || bDefaultWheels) {
		bool isFrontWheel=true;

		//choose coordinate system
		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);
		isFrontWheel = false;
		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
	} else {
		const class cmHelper_i *h = cmodel->getNextHelperOfClass("car_wheel");
		while(h) {
			vec3_c wheelOrigin;
			h->getKeyVec3("origin", wheelOrigin);
			btVector3 connectionPointCS0(wheelOrigin*QIO_TO_BULLET);
			bool isFrontWheel=false;

			m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

			h = cmodel->getNextHelperOfClass("car_wheel",h);
		}
	}
	
	for (int i=0;i<m_vehicle->getNumWheels();i++)
	{
		btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = suspensionStiffness;
		wheel.m_wheelsDampingRelaxation = suspensionDamping;
		wheel.m_wheelsDampingCompression = suspensionCompression;
		wheel.m_frictionSlip = wheelFriction;
		wheel.m_rollInfluence = rollInfluence;
	}
}
void btVehicle_c::destroyVehicle() {
	if(m_vehicleRayCaster) {
		delete m_vehicleRayCaster;
		m_vehicleRayCaster = 0;
	}
	if(m_vehicle) {
		delete m_vehicle;
		m_vehicle = 0;
	}
	if(m_wheelShape) {
		delete m_wheelShape;
		m_wheelShape = 0;
	}
}
void btVehicle_c::setSteering(float newEngineForce, float steerRot) {
	this->curEngineForce = newEngineForce;
	this->curSteerRot = steerRot;
}
void btVehicle_c::runFrame() {
	if(m_vehicle == 0)
		return;
	//synchronize the wheels with the (interpolated) chassis worldtransform
	for(u32 i = 0; i < 4; i++) {
		m_vehicle->updateWheelTransform(i,true);
	}
	int wheelIndex = 2;
	m_vehicle->applyEngineForce(this->curEngineForce,wheelIndex);
	//m_vehicle->setBrake(gBreakingForce,wheelIndex);
	wheelIndex = 3;
	m_vehicle->applyEngineForce(this->curEngineForce,wheelIndex);
	//m_vehicle->setBrake(gBreakingForce,wheelIndex);

	wheelIndex = 0;
	m_vehicle->setSteeringValue(this->curSteerRot,wheelIndex);
	wheelIndex = 1;
	m_vehicle->setSteeringValue(this->curSteerRot,wheelIndex);
}

