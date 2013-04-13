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
#include <shared/typedefs.h>
#include <math/vec3.h>

enum cModType_e {
	CMOD_BAD,
	CMOD_CAPSULE,
	CMOD_BBEXTS, // bounding box defined by halfsizes
	CMOD_HULL, // aka brush - single convex volume
	CMOD_COMPOUND,
	CMOD_TRIMESH, // for static (non-moveable) physics object
	CMOD_SKELMODEL, // dynamic, skeletal model which must be instanced before raycasting
	CMOD_BBMINSMAXS, // bounding box defined by mins and maxs vectors
};

// cm helpers are used to position joints,
// motors and car wheels
class cmHelper_i {
public:
	virtual u32 getNumKeyPairs() const = 0;
	virtual const char *getKeyValue(const char *key) const = 0;
};

// cm object base
class cMod_i {
public:
	virtual const char *getName() const = 0;
	virtual enum cModType_e getType() const = 0;

	bool isCapsule() const {
		return getType() == CMOD_CAPSULE;
	}
	bool isBBExts() const {
		return getType() == CMOD_BBEXTS;
	}
	bool isHull() const {
		return getType() == CMOD_HULL;
	}
	bool isCompound() const {
		return getType() == CMOD_COMPOUND;
	}
	bool isTriMesh() const {
		return getType() == CMOD_TRIMESH;
	}
	bool isBBMinsMaxs() const {
		return getType() == CMOD_BBMINSMAXS;
	}

	virtual class cmBBExts_i *getBBExts() {
		return 0;
	}
	virtual class cmCapsule_i *getCapsule() {
		return 0;
	}
	virtual class cmHull_i *getHull() {
		return 0;
	}
	virtual class cmCompound_i *getCompound() {
		return 0;
	}
	virtual class cmTriMesh_i *getTriMesh() {
		return 0;
	}
	virtual class cmSkelModel_i *getSkelModel() {
		return 0;
	}
	virtual const class cmBBExts_i *getBBExts() const {
		return 0;
	}
	virtual const class cmCapsule_i *getCapsule() const {
		return 0;
	}
	virtual const class cmHull_i *getHull() const {
		return 0;
	}
	virtual const class cmCompound_i *getCompound() const {
		return 0;
	}
	virtual const class cmTriMesh_i *getTriMesh() const {
		return 0;
	}
	virtual const class cmSkelModel_i *getSkelModel() const {
		return 0;
	}

	virtual void getBounds(class aabb &out) const = 0;

	// helpers
	virtual u32 getNumHelpers() const = 0;
	virtual cmHelper_i *getHelper(u32 helperNum) = 0;
	virtual cmCompound_i *getSubModel(u32 subModelNum) = 0;
	///virtual cmHelper_i *getNextHelperOfClass(const char *className, cmHelper_i *cur = 0) = 0;

	virtual bool traceRay(class trace_c &tr) = 0;

	// NEW testing functions
	// collision shapes postprocessing
	virtual void translateXYZ(const class vec3_c &ofs) {

	}
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

class cmBBMinsMaxs_i : public cMod_i {
public:
	// use cMod_i::getBounds() to access AABB shape
};

// single convex hull (aka brush)
class cmHull_i : public cMod_i {
public:
	virtual u32 getNumSides() const = 0;
	virtual const class plane_c &getSidePlane(u32 sideNum) const = 0;
	virtual void iterateSidePlanes(void (*callback)(const float planeEq[4])) const = 0;
};

// compound shape - cm object made of multiple cm primitives
class cmCompound_i : public cMod_i {
public:
	virtual u32 getNumSubShapes() const = 0;
	virtual const cMod_i *getSubShapeN(u32 subShapeNum) const = 0;
};

// trimesh object for static physics object
class cmTriMesh_i : public cMod_i {
public:
	virtual const vec3_c *getVerts() const = 0;
	virtual const vec3_c *getScaledVerts() const = 0;
	virtual const u32 *getIndices() const = 0;
	virtual u32 getNumIndices() const = 0;
	virtual u32 getNumVerts() const = 0;
	u32 getNumTris() const {
		return this->getNumIndices() / 3;
	}
	virtual const class cmSurface_c *getCMSurface() const = 0;

	virtual void precacheScaledVerts(float scaledVertsScale) const = 0;
};

// skeletal model
class cmSkelModel_i : public cMod_i {
public:
	virtual const class skelModelAPI_i *getSkelModelAPI() const = 0;
	virtual int getBoneNumForName(const char *boneName) const = 0;
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
	// aabb defined by mins and maxs
	virtual class cmBBMinsMaxs_i *registerAABB(const class aabb &bb) = 0;

	// works with any model type
	virtual class cMod_i *registerModel(const char *modName) = 0;
	virtual class cmSkelModel_i *registerSkelModel(const char *skelModelName) = 0;
	virtual void freeAllModels() = 0;

	virtual void loadMap(const char *mapName) = 0;
};

extern cmAPI_i *cm;


#endif // __CM_API_H__

