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
};

#endif // __RMODELAPI_H__
