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
#include <shared/cmBrush.h>
#include <shared/cmSurface.h>
#include <shared/trace.h>
#include <math/aabb.h>
#include "cm_helper.h"

class cmHelpersList_c : public arraySTD_c<class cmHelper_c*> {

public:
	class cmCompound_i *findSubModel(u32 subModelNum) {
		u32 curModelNum = 0;
		for(u32 i = 0; i < this->size(); i++) {
			cmHelper_c *h = (*this)[i];
			if(h->hasCompoundModel()) {
				if(curModelNum == subModelNum)
					return h->getCompoundAPI();
				curModelNum++;
			}
		}
		return 0;
	}
};
class cmObjectBase_c {
	cmObjectBase_c *hashNext;
protected:
	str name;
	aabb bounds;
	cmHelpersList_c helpers;
	vec3_c centerOfMassOffset;
	bool bHasCenterOfMassOffset;
public:
	cmObjectBase_c() {
		centerOfMassOffset.set(0,0,0);
		bHasCenterOfMassOffset = false;
	}
	void setCenterOfMassOffset(const vec3_c &newOfs) {
		centerOfMassOffset = newOfs;
		bHasCenterOfMassOffset = true;
	}
	cmObjectBase_c *getHashNext() const {
		return hashNext;
	}
	void setHashNext(cmObjectBase_c *p) {
		hashNext = p;
	}
	const char *getName() const {
		return name;
	}
	void addHelper(class cmHelper_c *newHelper) {
		helpers.push_back(newHelper);
	}

	virtual void thisClassMustBePolyMorphic() {

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
	virtual void getBounds(class aabb &out) {
		out.fromCapsuleZ(height,radius);
	}
	virtual bool traceRay(class trace_c &tr) {
		if(tr.clipByAABB(this->bounds))
			return true;
		return false;
	}
	// cmCapsule_c access
	virtual float getHeight() const {
		return height;
	}
	virtual float getRadius() const {
		return radius;
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	cmCapsule_c(const char *newName, float newHeight, float newRadius) {
		this->name = newName;
		this->height = newHeight;
		this->radius = newRadius;
		this->bounds.fromCapsuleZ(height,radius);
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
	virtual void getBounds(class aabb &out) {
		out.fromHalfSizes(halfSizes);
	}
	virtual bool traceRay(class trace_c &tr) {
		if(tr.clipByAABB(this->bounds))
			return true;
		return false;
	}
	// cmBBExts_i access
	virtual const class vec3_c &getHalfSizes() const {
		return halfSizes;
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	cmBBExts_c(const char *newName, const vec3_c &newHalfSizes) {
		this->name = newName;
		this->halfSizes = newHalfSizes;
		this->bounds.fromHalfSizes(halfSizes);
	}
};

class cmBBMinsMaxs_c : public cmObjectBase_c, public cmBBMinsMaxs_i {
	// nothing else needed here...
public:

	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_BBMINSMAXS;
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
	virtual void getBounds(class aabb &out) {
		out = this->bounds;
	}
	virtual bool traceRay(class trace_c &tr) {
		if(tr.clipByAABB(this->bounds))
			return true;
		return false;
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	cmBBMinsMaxs_c(const char *newName, const aabb &newBounds) {
		this->name = newName;
		this->bounds = newBounds;
	}
};

class cmHull_c : public cmObjectBase_c, public cmHull_i {
	cmBrush_c myBrush;
public:
	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_HULL;
	}
	virtual class cmBBExts_i *getBBExts() {
		return 0;
	}
	virtual class cmCapsule_i *getCapsule() {
		return 0;
	}
	virtual class cmHull_i *getHull() {
		return this;
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
	virtual void getBounds(class aabb &out) {
		out = myBrush.getBounds();
	}
	virtual bool traceRay(class trace_c &tr) {
		return myBrush.traceRay(tr);
	}	
	virtual void translateXYZ(const class vec3_c &ofs) {
		myBrush.translateXYZ(ofs);
	}
	// cmHull_i access
	virtual u32 getNumSides() const {
		return myBrush.getNumSides();
	}
	virtual const class plane_c &getSidePlane(u32 sideNum) const {
		return myBrush.getSidePlane(sideNum);
	}
	virtual void iterateSidePlanes(void (*callback)(const float planeEq[4])) const {
		myBrush.iterateSidePlanes(callback);
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	void setSingleBrush(const class cmBrush_c &br) {
		this->myBrush = br;
	}

	cmHull_c(const char *newName, const class cmBrush_c &br) {
		this->name = newName;
		this->setSingleBrush(br);
	}
};

class cmCompound_c : public cmObjectBase_c, public cmCompound_i {
	arraySTD_c<cMod_i*> shapes;
public:
	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_COMPOUND;
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
		return this;
	}
	virtual class cmTriMesh_i *getTriMesh() {
		return 0;
	}
	virtual class cmSkelModel_i *getSkelModel() {
		return 0;
	}
	virtual void getBounds(class aabb &out) {
		out.clear();
		for(u32 i = 0; i < shapes.size(); i++) {
			aabb bb;
			shapes[i]->getBounds(bb);
			out.addBox(bb);
		}
	}
	virtual bool traceRay(class trace_c &tr) {
		bool hit = false;
		for(u32 i = 0; i < shapes.size(); i++) {
			if(shapes[i]->traceRay(tr)) {
				hit = true;
			}
		}
		return hit;
	}
	virtual bool hasCenterOfMassOffset() const {
		return bHasCenterOfMassOffset;
	}
	virtual const vec3_c &getCenterOfMassOffset() const {
		return centerOfMassOffset;
	}
	// cmCompound_i access
	virtual u32 getNumSubShapes() {
		return shapes.size();
	}
	virtual cMod_i *getSubShapeN(u32 subShapeNum) {
		return shapes[subShapeNum];
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	void addShape(class cMod_i *m) {
		shapes.push_back(m);
	}
	void translateXYZ(const vec3_c &ofs) {
		for(u32 i = 0; i < shapes.size(); i++) {
			shapes[i]->translateXYZ(ofs);
		}
	}

	cmCompound_c(const char *newName) {
		this->name = newName;
	}
};

class cmTriMesh_c : public cmObjectBase_c, public cmTriMesh_i {
	cmSurface_c *sf;
public:
	cmTriMesh_c() {
		sf = 0;
	}
	~cmTriMesh_c() {
		if(sf) {
			delete sf;
			sf = 0;
		}
	}
	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_TRIMESH;
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
		return this;
	}
	virtual class cmSkelModel_i *getSkelModel() {
		return 0;
	}
	virtual void getBounds(class aabb &out) {
		out = sf->getAABB();
	}
	virtual bool traceRay(class trace_c &tr) {
		return false;
	}
	// cmTriMesh_i access
	virtual const vec3_c *getVerts() const {
		return sf->getVerts();
	}
	virtual const u32 *getIndices() const  {
		return sf->getIndices();
	}
	virtual u32 getNumIndices() const  {
		return sf->getNumIndices();
	}
	virtual u32 getNumVerts() const  {
		return sf->getNumVerts();
	}
	virtual const class cmSurface_c *getCMSurface() const {
		return sf;
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return helpers.size();
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return helpers[helperNum];
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}

	cmTriMesh_c(const char *newName, cmSurface_c *newSFPtr) {
		this->name = newName;
		this->sf = newSFPtr;
	}
};

class cmSkelModel_c : public cmObjectBase_c, public cmSkelModel_i {
	skelModelAPI_i *skel;
public:
	cmSkelModel_c();
	~cmSkelModel_c();

	// cmObjectBase_c access
	virtual const char *getName() const {
		return name;
	}
	virtual enum cModType_e getType() const {
		return CMOD_SKELMODEL;
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
		return this;
	}
	virtual void getBounds(class aabb &out) {
		//out = sf->getAABB();
	}
	virtual bool traceRay(class trace_c &tr) {
		return false;
	}
	// helpers api
	virtual u32 getNumHelpers() const {
		return 0;
	}
	virtual cmHelper_i *getHelper(u32 helperNum) {
		return 0;
	}
	virtual cmCompound_i *getSubModel(u32 subModelNum) {
		return helpers.findSubModel(subModelNum);
	}
	// cmSkelModel_i access
	virtual const class skelModelAPI_i *getSkelModelAPI() const {
		return skel;
	}
	virtual int getBoneNumForName(const char *boneName) const;

	cmSkelModel_c(const char *newName, skelModelAPI_i *newSkel) {
		this->name = newName;
		this->skel = newSkel;
	}
};