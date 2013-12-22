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
// btp_vehicle.h
#include "../../game/g_physVehicleAPI.h"
#include "btp_headers.h"

class btVehicle_c : public physVehicleAPI_i {
	class btRaycastVehicle *m_vehicle;
	class btRigidBody *m_carChassis;
	class btVehicleRaycaster *m_vehicleRayCaster;
	class btCollisionShape *m_wheelShape;
	btRaycastVehicle::btVehicleTuning m_tuning;
	float curEngineForce;
	float curSteerRot;
public:
	btVehicle_c();
	~btVehicle_c();
	virtual void getMatrix(class matrix_c &out);
	void init(class bulletPhysicsWorld_c *pWorld, const class vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel);
	void destroyVehicle();
	virtual void setSteering(float newEngineForce, float steerRot);
	virtual void setOrigin(const class vec3_c &newPos);
	void runFrame();
};


