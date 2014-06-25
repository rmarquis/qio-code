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
// rLightAPI.h - renderer light class interface
#ifndef __RLIGHTAPI_H__
#define __RLIGHTAPI_H__

enum rLightType_e {
	LT_POINT,
	LT_SPOTLIGHT,
};

class rLightAPI_i {
public:
	virtual void setOrigin(const class vec3_c &newXYZ) = 0;
	virtual void setRadius(float newRadius) = 0;
	virtual void setBNoShadows(bool newBNoShadows) = 0;
	virtual void setLightType(rLightType_e newLightType) = 0;
	virtual void setSpotLightTarget(const class vec3_c &newTargetPos) = 0;
	virtual void setSpotRadius(float newSpotRadius) = 0;
	virtual void setBColoured(bool newBColoured) = 0;
	virtual void setColor(const class vec3_c &newRGB) = 0;

	virtual const vec3_c &getOrigin() const = 0;
	virtual float getRadius() const = 0;
	virtual enum rLightType_e getLightType() const = 0;
	virtual const class vec3_c &getSpotLightDir() const = 0;
	virtual float getSpotRadius() const = 0;
	virtual float getSpotLightMaxCos() const = 0;
	virtual bool isSpotLight() const = 0;
	virtual const class frustum_c &getSpotLightFrustum() const = 0;
	virtual const class matrix_c &getSpotLightView() const = 0;
	virtual bool isColoured() const = 0;
	virtual const vec3_c &getColor() const = 0;

	virtual class occlusionQueryAPI_i *getOcclusionQuery() = 0;
	virtual bool getBCameraInside() const = 0;

	virtual const class matrix_c &getSMLightProj() const = 0;
	virtual const class matrix_c &getSMSideView(u32 sideNum) const = 0;
	virtual const class frustum_c &getSMSideFrustum(u32 sideNum) const = 0;
	
	virtual void calcPosInEntitySpace(const class rEntityAPI_i *ent, class vec3_c &out) const = 0;
};

#endif // __RLIGHTAPI_H__

