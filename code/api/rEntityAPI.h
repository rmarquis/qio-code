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
// rEntityAPI.h - renderer entity class interface
#ifndef __RENTITYAPI_H__
#define __RENTITYAPI_H__

class rEntityAPI_i {
public:
	virtual void setOrigin(const class vec3_c &newXYZ) = 0;
	virtual void setAngles(const class vec3_c &newAngles) = 0;
	virtual void setScale(const class vec3_c &newScale) = 0;
	virtual void setModel(class rModelAPI_i *mod) = 0;
	// used eg. to attach md3 head models to RTCW .mds skeletal models
	virtual void setAttachment(u32 which, const char *modelName, const char *boneName) = 0;
	virtual void setAnim(const class skelAnimAPI_i *anim, int newFlags = 0) = 0;
	virtual void setAnim(const char *animName, int newFlags = 0) = 0;
	virtual void setTorsoAnim(const class skelAnimAPI_i *anim, int newFlags = 0) = 0;
	// for keyframed (md3,md2, mdc...) models
	virtual void setAnimationFrame(int frameNum) = 0;
	virtual void setThirdPersonOnly(bool bOn) = 0;
	virtual void setFirstPersonOnly(bool bOn) = 0;
	virtual void setIsPlayerModel(bool bNewIsPlayerModel) = 0;
	virtual void setRagdoll(const class afDeclAPI_i *af) = 0;
	virtual void setRagdollBodyOr(u32 partIndex, const class boneOrQP_c &or) = 0;
	virtual void setDeclModelAnimLocalIndex(int localAnimIndex, int newFlags = 0) = 0;
	virtual void setTIKIModelAnimLocalIndex(int localAnimIndex, int newFlags = 0) = 0;
	virtual void setTIKIModelTorsoAnimLocalIndex(int localAnimIndex, int newFlags = 0) = 0;
	virtual void setQ3LegsAnimLocalIndex(int localAnimIndex) = 0;
	virtual void setQ3TorsoAnimLocalIndex(int localAnimIndex) = 0;
	virtual void setSkin(const char *skinName) = 0;
	virtual void setNetworkingEntityNumber(int newNetEntNum) = 0;
	virtual void setEntityType(int newEntityType) = 0;
	virtual void setColor(const float *rgba) = 0;

	virtual class rModelAPI_i *getModel() const = 0;
	virtual const char *getModelName() const = 0;
	virtual const class axis_c &getAxis() const = 0;
	virtual const class vec3_c &getOrigin() const = 0;
	virtual const class matrix_c &getMatrix() const = 0;
	virtual const class aabb &getBoundsABS() const = 0;
	virtual int getNetworkingEntityNumber() const = 0;

	virtual void hideModel() = 0;
	virtual void showModel() = 0;
	virtual void hideSurface(u32 surfNum) = 0;
	virtual bool isRagdoll() const = 0;
	virtual bool hasDeclModel() const = 0;
	virtual bool hasTIKIModel() const = 0;
	virtual bool isQ3PlayerModel() const = 0;
	virtual bool isSprite() const = 0;
	// for .md3/.md2/.mdc models
	virtual bool isKeyframed() const = 0;

	// returns true if entity model has wolfAnim.cfg
	// (for RTCW player models)
	virtual bool hasWolfAnimConfig() const = 0;
	// for ET players (.char file models)
	virtual bool hasCharacterFile() const = 0;

	virtual bool hasAnim(const char *animName) const = 0;

	// trace must be transformed into entity coordinates before calling this function.
	// Returns true if a collision occured
	virtual bool rayTraceLocal(class trace_c &tr) const = 0;
	// this will automatically transform ray from world to this entity
	// coordinates and call "rayTraceLocal"
	virtual bool rayTraceWorld(class trace_c &tr) const = 0;

	/// for skeletal models
	// NOTE: this will force bones recalculation if skeleton is not up to date!
	virtual bool getBoneWorldOrientation(const char *boneName, class matrix_c &out) = 0;
	virtual bool getBoneWorldOrientation(int localBoneIndex, class matrix_c &out) = 0;

	virtual int addDecalWorldSpace(const class vec3_c &pos, 
		const class vec3_c &normal, float radius, class mtrAPI_i *material) = 0;
};

#endif // __RENTITYAPI_H__

