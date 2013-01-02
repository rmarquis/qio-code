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
// q3PlayerModelDecl.h - Quake3 three-parts player model system
#ifndef __Q3PLAYERMODELDECL_H__
#define __Q3PLAYERMODELDECL_H__

#include <shared/str.h>
#include "declRefState.h"
#include <api/q3PlayerModelDeclAPI.h>
#include <math/aabb.h>

class q3PlayerModelDecl_c : public q3PlayerModelAPI_i, public declRefState_c {
	str name;
	str legsModelPath;
	str torsoModelPath;
	str headModelPath;
	class kfModelAPI_i *legsModel;
	class kfModelAPI_i *torsoModel;
	class kfModelAPI_i *headModel;
	class q3AnimCfg_c *cfg;
	q3PlayerModelDecl_c *hashNext;
	bool valid;
	// player hull for movement simulation
	aabb playerHull; // (by default the same as in Q3)
	float standEyeHeight;
public:
	q3PlayerModelDecl_c() {
		legsModel = 0;
		torsoModel = 0;
		headModel = 0;
		cfg = 0;
		valid = false;
		// Quake3 players bounds: 
		// mins = (-15,-15,-24)
		// maxs = (15,15,32)
		// so player sizes are (30,30,56)
		// maxZCrouching = 16 (ducked)
		float sizeXY = 15;
		playerHull.mins.set(-sizeXY,-sizeXY,-24);
		playerHull.maxs.set(sizeXY,sizeXY,32);
		standEyeHeight = 26; // default Q3 viewheight
	}
	virtual const char *getName() const {
		return name;
	}
	virtual const aabb &getPlayerHull() const {
		return playerHull;
	}
	q3PlayerModelDecl_c *getHashNext() const {
		return hashNext;
	}
	void setHashNext(q3PlayerModelDecl_c *newHashNext) {
		this->hashNext = newHashNext;
	}
	void setDeclName(const char *newName) {
		this->name = newName;
	}
	bool isValid() const {
		return valid;
	}

	virtual u32 getNumTotalSurfaces() const;
	virtual const class kfModelAPI_i *getLegsModel() const {
		return legsModel;
	}
	virtual const class kfModelAPI_i *getTorsoModel() const {
		return torsoModel;
	}
	virtual const class kfModelAPI_i *getHeadModel() const {
		return headModel;
	}
	virtual const char *getLegsModelName() const {
		return legsModelPath;
	}
	virtual const char *getTorsoModelName() const {
		return torsoModelPath;
	}
	virtual const char *getHeadModelName() const {
		return headModelPath;
	}
	virtual const struct q3AnimDef_s *getAnimCFGForIndex(u32 localAnimIndex) const;

	bool loadQ3PlayerDecl();
};
#endif // __Q3PLAYERMODELDECL_H__
