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
// Weapon.h

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "ModelEntity.h"

class Weapon : public ModelEntity {
	// custom viewmodel for Doom3-style weapons
	str model_view;
	bool autoFire;
	// those values are in msec
	u32 delayBetweenShots;
	u32 lastShotTime;
	class entityDeclAPI_i *invWeaponDecl;
	int clipSize;
	int curClipSize;
	// time needed to raise weapon (from raise animation)
	u32 raiseTime;
	// time needed to lower weapon (from lower animation)
	u32 lowerTime;
	// time needed to reload weapon (from reload animation)
	u32 reloadTime;
	str weaponName;
	str ddaName;
	vec3_c viewOffset, viewAngles;
	str def_projectile;
	str smoke_muzzle;
	// D3 muzzle flash lighting effect
	vec3_c flashColor;
	float flashRadius;
protected:
	safePtr_c<Player> owner;

	void setDelayBetweenShorts(u32 newDelayInMsec) {
		this->delayBetweenShots = newDelayInMsec;
	}
public:
	Weapon();
	virtual ~Weapon();

	DECLARE_CLASS( Weapon );

	u32 getRaiseTime() const {
		return raiseTime;
	}
	u32 getPutawayTime() const {
		return lowerTime;
	}
	u32 getDelayBetweenShots() const {
		return delayBetweenShots;
	}
	u32 getReloadTime() const {
		return reloadTime;
	}
	u32 getClipSize() const {
		return clipSize;
	}
	u32 getCurrentClipSize() const {
		return curClipSize;
	}
	void fillClip(u32 newCurClipSize) {
		curClipSize = newCurClipSize;
	}
	bool hasEmptyClip() const {
		if(curClipSize == 0)
			return true;
		return false;
	}
	const vec3_c &getViewModelAngles() const {
		return viewAngles;
	}
	const vec3_c &getViewModelOffset() const {
		return viewOffset;
	}

	virtual BaseEntity *getOwner() const;

	void onFireKeyHeld();
	void onFireKeyDown();
	virtual void onSecondaryFireKeyHeld();
	virtual void onSecondaryFireKeyDown();
	virtual void onSecondaryFireKeyUp();

	virtual void doWeaponAttack();
	virtual void doWeaponAttackSecondary();
	bool canFireAgain() const;

	bool hasCustomViewModel() const {
		if(model_view.length())
			return true;
		return false;
	}
	const char *getCustomViewModelName() const {
		return model_view;
	}

	void setOwner(class Player *newOwner) {
		owner = newOwner;
	}

	virtual void onWeaponPickedUp() {

	}

	void setViewModel(const char *newViewModelName);

	virtual void setKeyValue(const char *key, const char *value); 
	virtual bool doUse(class Player *activator);
};

#endif // __WEAPON_H__
