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
// VehicleCar.h

#ifndef __VEHICLECAR_H__
#define __VEHICLECAR_H__

#include "ModelEntity.h"

class VehicleCar : public ModelEntity {
public:
	DECLARE_CLASS( VehicleCar );

	class physVehicleAPI_i *physVehicle;
	class Player *driver;

	VehicleCar();
	~VehicleCar();

	void spawnPhysicsVehicle();
	void destroyPhysicsVehicle();

	virtual void setOrigin(const class vec3_c &newPos);
	// calls "spawnPhysicsVehicle"
	virtual void postSpawn();

	void steerUCmd(const struct usercmd_s *ucmd);

	void detachPlayer(class Player *pl) {
		if(pl == driver) {
			driver = 0;
		}
	}
	virtual bool doUse(class Player *activator);
	virtual void runPhysicsObject();
};

#endif // __VEHICLECAR_H__
