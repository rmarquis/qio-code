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
// VehicleCar.cpp
#include "VehicleCar.h"
#include "../g_local.h"
#include "../g_physVehicleAPI.h"

DEFINE_CLASS(VehicleCar, "ModelEntity");

VehicleCar::VehicleCar() {
	physVehicle = 0;
}
VehicleCar::~VehicleCar() {

}

void VehicleCar::spawnPhysicsVehicle() {
	physVehicle = BT_CreateVehicle(this->getOrigin());
}
void VehicleCar::runPhysicsObject() {
	if(physVehicle == 0)
		return;
	matrix_c mat;
	physVehicle->getMatrix(mat);
	BaseEntity::setMatrix(mat);
}





