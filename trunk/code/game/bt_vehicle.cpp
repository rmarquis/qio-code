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
// bt_vehicle.cpp
#include "g_local.h"
#include "bt_include.h"
#include "g_physVehicleAPI.h"
#include <math/matrix.h>
#include <math/aabb.h>
#include <api/cmAPI.h>

#define VEH_SCALE 48.f

#define CUBE_HALF_EXTENTS 1.f*VEH_SCALE

static btVector3 wheelDirectionCS0(0,0,-1);
static btVector3 wheelAxleCS(1,0,0);


static float	wheelRadius = 0.5f*VEH_SCALE;
static float	wheelWidth = 0.4f*VEH_SCALE;
static float	wheelFriction = 1000;//BT_LARGE_FLOAT;
static float	suspensionStiffness = 20.f;//*VEH_SCALE;
static float	suspensionDamping = 2.3f;//*VEH_SCALE;
static float	suspensionCompression = 4.4f;//*VEH_SCALE;
static float	rollInfluence = 0.1f;//1.0f;
//
static btScalar suspensionRestLength(0.6f*VEH_SCALE);

class btVehicle_c : public physVehicleAPI_i {
	btRaycastVehicle *m_vehicle;
	btRigidBody *m_carChassis;
	btVehicleRaycaster *m_vehicleRayCaster;
	btCollisionShape *m_wheelShape;
	btRaycastVehicle::btVehicleTuning m_tuning;
	float curEngineForce;
	float curSteerRot;
public:
	btVehicle_c() {
		m_vehicle = 0;
		m_carChassis = 0;
		m_vehicleRayCaster = 0;
		m_wheelShape = 0;
		curEngineForce = 100000.f;
		curSteerRot = 0.1f;
	}
	~btVehicle_c() {
		destroyVehicle();
	}
	virtual void getMatrix(matrix_c &out) {
		btTransform trans;
		m_carChassis->getMotionState()->getWorldTransform(trans);	
		trans.getOpenGLMatrix(out);
	}
	void init(const vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel) {
		btCompoundShape *compound = new btCompoundShape();
	
		// pass NULL cmodel here to use default car shape
		//cmodel = 0;

		const float hOfs = 0;//32.f;
		btTransform localTrans;
		localTrans.setIdentity();
		bool doLocalTrans = false;
		//localTrans effectively shifts the center of mass with respect to the chassis
		if(doLocalTrans) {
			localTrans.setOrigin(btVector3(0,0,1.f*VEH_SCALE+hOfs));
		}
		
		if(cmodel) {
			BT_AddCModelToCompoundShape(compound,localTrans,cmodel);
		} else {
			btCollisionShape *chassisShape = new btBoxShape(btVector3(1.f*VEH_SCALE,2.f*VEH_SCALE, 0.5f*VEH_SCALE));
			compound->addChildShape(localTrans,chassisShape);
		}

		btTransform tr;
		tr.setIdentity();
		matrix_c mat;
		mat.fromAnglesAndOrigin(angles,pos);
		tr.setFromOpenGLMatrix(mat);

		compound->setMargin(bt_collisionMargin);
		m_carChassis = BT_CreateRigidBodyInternal(800,tr,compound);

		m_wheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));
		m_wheelShape->setMargin(bt_collisionMargin);

		m_vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);

		//m_tuning.m_maxSuspensionTravelCm *= 100.f;
		m_vehicle = new btRaycastVehicle(m_tuning,m_carChassis,m_vehicleRayCaster);
		
		///never deactivate the vehicle
		m_carChassis->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addVehicle(m_vehicle);

		float connectionHeight;
		if(cmodel) {
			aabb bb;
			cmodel->getBounds(bb);
			connectionHeight = bb.mins.z;
			connectionHeight += wheelRadius;
		} else if(doLocalTrans) {
			connectionHeight = 1.2f*VEH_SCALE + hOfs;
		} else {
			connectionHeight = 1.2f*VEH_SCALE + hOfs;
		}

		bool isFrontWheel=true;

		//choose coordinate system
		m_vehicle->setCoordinateSystem(0,2,1);
		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);
		isFrontWheel = false;
		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);

		m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		
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
	void destroyVehicle() {
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
	virtual void setSteering(float newEngineForce, float steerRot) {
		this->curEngineForce = newEngineForce;
		this->curSteerRot = steerRot;
	}
	void runFrame() {
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
};

static arraySTD_c<btVehicle_c*> bt_vehicles;

physVehicleAPI_i *BT_CreateVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel) {
	btVehicle_c *nv = new btVehicle_c;
	nv->init(pos, angles, cmodel);
	bt_vehicles.push_back(nv);
	return nv;
}
void BT_RemoveVehicle(class physVehicleAPI_i *pv) {
	btVehicle_c *v = dynamic_cast<btVehicle_c*>(pv);
	if(v == 0) {
		return;
	}
	bt_vehicles.removeObject(v);
	v->destroyVehicle();
	delete v;
}

void BT_RunVehicles() {		
	for(u32 i = 0; i < bt_vehicles.size(); i++) {
		btVehicle_c *v = bt_vehicles[i];
		v->runFrame();
	}
}

void BT_ShutdownVehicles() {
	for(u32 i = 0; i < bt_vehicles.size(); i++) {
		btVehicle_c *v = bt_vehicles[i];
		v->destroyVehicle();
		delete v;
	}
}