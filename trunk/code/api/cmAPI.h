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
// cm_api.h
#ifndef __CM_API_H__
#define __CM_API_H__

#include "iFaceBase.h"

enum cModType_e {
	CMOD_BAD,
	CMOD_CAPSULE,
	CMOD_BBEXTS, // bounding box defined by halfsizes
};

// cm object base
class cMod_i {
public:
	virtual const char *getName() const = 0;
	virtual enum cModType_e getType() const = 0;

	virtual bool isCapsule() const {
		return getType() == CMOD_CAPSULE;
	}
	virtual bool isBBExts() const {
		return getType() == CMOD_BBEXTS;
	}
	virtual class cmBBExts_i *getBBExts() = 0;
	virtual class cmCapsule_i *getCapsule() = 0;
};

// cm primitives
class cmCapsule_i : public cMod_i {
public:
	virtual float getHeight() const = 0;
	virtual float getRadius() const = 0;
};

class cmBBExts_i : public cMod_i {
public:
	virtual const class vec3_c &getHalfSizes() const = 0;
};

#define CM_API_IDENTSTR "CM0001"

class cmAPI_i : public iFaceBase_i {
public:
	///
	/// primitive shapes registration. They are used for collision 
	/// detection in both game module (Bullet physics) and cgame.
	///
	// capsule for bullet character controller
	virtual class cmCapsule_i *registerCapsule(float height, float radius) = 0;
	// simple box for basic items / physics testing
	virtual class cmBBExts_i *registerBoxExts(float halfSizeX, float halfSizeY, float halfSizeZ) = 0;
	class cmBBExts_i *registerBoxExts(const float *hfs) {
		return registerBoxExts(hfs[0],hfs[1],hfs[2]);
	}

};

extern cmAPI_i *cm;


#endif // __CM_API_H__

