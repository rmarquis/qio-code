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
// Weapon_PhysGun.h - HalfLife2 style physgun

#ifndef __WEAPON_PHYSGUN_H__
#define __WEAPON_PHYSGUN_H__

#include "Weapon.h"

class Weapon_PhysGun : public Weapon {
	safePtr_c<ModelEntity> holdingEntity;
	bool bDropped;
	float phMaxDist;
	float phPickupDist;
	float phDragForce;
	float phImpulseForce;
	float phHoldDist;
public:
	Weapon_PhysGun();

	DECLARE_CLASS( Weapon_PhysGun );

	// physgun entity manipulation
	// picking up entity - right mouse button
	void tryToPickUpEntity();
	// shooting entity - left mouse button
	void shootPickedUpEntity();
	// dropping picked up entity - right mouse button
	void dropPickedUpEntity();
	// add gravity impulse to crossair entity
	void addGravityImpulse();

	// update picked up entity orientation
	virtual void runFrame();

	virtual void postSpawn();

	virtual void onWeaponPickedUp();

	virtual void doWeaponAttack();
	virtual void onSecondaryFireKeyHeld();
	virtual void onSecondaryFireKeyDown();
	virtual void onSecondaryFireKeyUp();

	virtual void setKeyValue(const char *key, const char *value);
};

#endif // __Q3WEAPON_H__
