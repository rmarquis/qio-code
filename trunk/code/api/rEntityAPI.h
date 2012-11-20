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
	virtual void setModel(class rModelAPI_i *mod) = 0;
	virtual void setAnim(const class skelAnimAPI_i *anim) = 0;
	virtual void setThirdPersonOnly(bool bOn) = 0;
	virtual void setFirstPersonOnly(bool bOn) = 0;

	virtual class rModelAPI_i *getModel() const = 0;
	virtual const class axis_c &getAxis() const = 0;
	virtual const class vec3_c &getOrigin() const = 0;
	virtual const class matrix_c &getMatrix() const = 0;
	virtual const class aabb &getBoundsABS() const = 0;

	virtual void hideSurface(u32 surfNum) = 0;

	// trace must be transformed into entity coordinates before calling this function.
	// Returns true if a collision occured
	virtual bool rayTrace(class trace_c &tr) const = 0;

	/// for skeletal models
	// NOTE: this will force bones recalculation if skeleton is not up to date!
	virtual bool getBoneWorldOrientation(const char *boneName, class matrix_c &out) = 0;
	virtual bool getBoneWorldOrientation(int localBoneIndex, class matrix_c &out) = 0;

	virtual int addDecalWorldSpace(const class vec3_c &pos, 
		const class vec3_c &normal, float radius, class mtrAPI_i *material) = 0;
};

#endif // __RENTITYAPI_H__

