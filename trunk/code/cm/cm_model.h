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
// cm_model.h
#include <api/cmAPI.h>
#include <shared/str.h>
#include <math/aabb.h>

class cmObjectBase_c {
	cmObjectBase_c *hashNext;
protected:
	str name;
	aabb bounds;
public:
	cmObjectBase_c *getHashNext() const {
		return hashNext;
	}
	void setHashNext(cmObjectBase_c *p) {
		hashNext = p;
	}
	virtual const char *getName() const {
		return name;
	}
};
class cmCapsule_c : public cmObjectBase_c, public cmCapsule_i {
	// capsule properties
	float height;
	float radius;
public:
	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_CAPSULE;
	}
	virtual class cmBBExts_i *getBBExts() {
		return 0;
	}
	virtual class cmCapsule_i *getCapsule() {
		return this;
	}
	virtual bool traceRay(class trace_c &tr) {
		return false; // TODO
	}
	// cmCapsule_c access
	virtual float getHeight() const {
		return height;
	}
	virtual float getRadius() const {
		return radius;
	}

	cmCapsule_c(const char *newName, float newHeight, float newRadius) {
		this->name = newName;
		this->height = newHeight;
		this->radius = newRadius;
	}
};
class cmBBExts_c : public cmObjectBase_c, public cmBBExts_i {
	// bb properties
	vec3_c halfSizes;
public:

	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_BBEXTS;
	}
	virtual class cmBBExts_i *getBBExts() {
		return this;
	}
	virtual class cmCapsule_i *getCapsule() {
		return 0;
	}
	virtual bool traceRay(class trace_c &tr) {
		return false; // TODO
	}
	// cmBBExts_i access
	virtual const class vec3_c &getHalfSizes() const {
		return halfSizes;
	}

	cmBBExts_c(const char *newName, const vec3_c &newHalfSizes) {
		this->name = newName;
		this->halfSizes = newHalfSizes;
	}
};



