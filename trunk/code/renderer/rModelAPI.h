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
// rModelAPI.h - interface of renderer model class.
// This interface must NOT be used in server / serverGame,
// because renderer models are present only on client side.
#ifndef __RMODELAPI_H__
#define __RMODELAPI_H__

#include <shared/typedefs.h>

class rModelAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual const class aabb &getBounds() const = 0;
	// does an in-place raytrace check against all of the models triangles.
	// This obviously works only for static models.
	// Dynamic (animated) models must be instanced before tracing.
	// Returns true if a collision occurred.
	virtual bool rayTrace(class trace_c &tr) const = 0;
	// returns true if model is static (non-animated)
	virtual bool isStatic() = 0;
	// TODO: use decalBatcherAPI_i or smth like that
	virtual bool createStaticModelDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
		const class vec3_c &normal,	float radius, class mtrAPI_i *material) = 0;
	// this will return NULL if this model is not a skeletal model
	virtual class skelModelAPI_i *getSkelModelAPI() const = 0;
	virtual class modelDeclAPI_i *getDeclModelAPI() const = 0;
	// this works only if this is a decl model
	virtual const class skelAnimAPI_i *getDeclModelAFPoseAnim() const = 0;
	virtual class kfModelAPI_i *getKFModelAPI() const = 0;
	virtual const class q3PlayerModelAPI_i *getQ3PlayerModelAPI() const = 0;
	virtual bool isValid() const = 0;
	virtual bool isInlineBSPModel() const = 0;
	virtual bool isInlinePROCModel() const = 0;
	virtual bool isDeclModel() const = 0;
	virtual bool isSkeletal() const = 0;
	virtual bool isKeyframed() const = 0;
	virtual bool isQ3PlayerModel() const = 0;
	virtual bool isSprite() const = 0;
	virtual float getSpriteRadius() const = 0;
	virtual bool hasWolfAnimConfig() const = 0;
	virtual const skelAnimAPI_i *findSkelAnim(const char *animName) const = 0;
	virtual bool findWolfAnimData(const char *animName, int *firstFrame, int *lastFrame, float *fps) const = 0;
	virtual mtrAPI_i *getSpriteMaterial() const = 0;
	// returns the number of surfaces in static/skeletal model
	virtual unsigned int getNumSurfaces() const = 0;
	// returns the number of model animations (eg. for decl models)
	virtual unsigned int getNumAnims() const = 0;
	// check if the animation with given name is present
	virtual bool hasAnim(const char *animName) const = 0;
	// for keyframed models
	//virtual void getTagOrientation(int tagNum, const struct singleAnimLerp_s &state, class matrix_c &out) = 0;
	// for q3 player models
	virtual bool getTagOrientation(int tagNum, const struct singleAnimLerp_s &legs, const struct singleAnimLerp_s &torso, class matrix_c &out) const = 0;
	// for static models / keyframed models
	virtual bool getTagOrientation(int tagNum, class matrix_c &out) const = 0;
	// used to get raw model data of staticmodels
	virtual bool getModelData(class staticModelCreatorAPI_i *out) const = 0;
	// debug output (this should work for tags as well)
	virtual void printBoneNames() const = 0;
	// will return -1 if bone not found
	virtual int findBone(const char *boneName) const = 0;
	// returns the total number of triangles in model
	virtual u32 getTotalTriangleCount() const = 0;
	virtual bool hasStageWithoutBlendFunc() const = 0;
};

#endif // __RMODELAPI_H__
