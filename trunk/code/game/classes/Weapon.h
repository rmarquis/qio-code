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
	class Player *owner;
	// custom viewmodel for Doom3-style weapons
	str model_view;
public:
	Weapon();
	virtual ~Weapon();

	DECLARE_CLASS( Weapon );

	virtual BaseEntity *getOwner() const;

	void onFireKeyHeld();
	void onFireKeyDown();
	bool hasCustomViewModel() const {
		if(model_view.length())
			return true;
		return false;
	}
	const char *getCustomViewModelName() const {
		return model_view;
	}

	virtual void setKeyValue(const char *key, const char *value); 
	virtual void doUse(class Player *activator);
};

#endif // __MODELENTITY_H__
