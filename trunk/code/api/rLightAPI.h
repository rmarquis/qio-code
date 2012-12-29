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

class rLightAPI_i {
public:
	virtual void setOrigin(const class vec3_c &newXYZ) = 0;
	virtual void setRadius(float newRadius) = 0;

	virtual const vec3_c &getOrigin() const = 0;
	virtual float getRadius() const = 0;

	virtual class occlusionQueryAPI_i *getOcclusionQuery() = 0;
	virtual bool getBCameraInside() const = 0;

	
	virtual void calcPosInEntitySpace(const class rEntityAPI_i *ent, class vec3_c &out) const = 0;
};

#endif // __RLIGHTAPI_H__

