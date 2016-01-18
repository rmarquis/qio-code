/*
============================================================================
Copyright (C) 2013 V.

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
// btp_shape.h
#ifndef __BTP_SHAPE_H__
#define __BTP_SHAPE_H__

#include <shared/str.h>
#include <math/matrix.h>

#include "btp_headers.h"

// NOTE: single bulletColShape_c can be used by several Bullet rigid bodies
class bulletColShape_c {
	str name;
	matrix_c centerOfMassTransform;
	bool bHasCenterOfMassTransform;
	// Bullet BHV shape can be used only for static objects,
	// so we have to distinguish between moveable
	// and non-moveable bodies.
	bool isStatic;
	// game collision model (with vertices in Quake units)
	const class cMod_i *cModel;
	// Bullet collision shape (with vertices in Bullet units)
	class btCollisionShape *bulletShape;

	// bullet rigid bodies using this shapes
	//arraySTD_c<class bulletRigidBody_c*> users;

	bulletColShape_c *hashNext;
public:
	bulletColShape_c();
	~bulletColShape_c();

	bool init(const class cMod_i *newCModel, bool newBIStatic, const vec3_c *scale = 0);

	void setName(const char *newName) {
		name = newName;
	}
	void setHashNext(bulletColShape_c *newHashNext) {
		hashNext = newHashNext;
	}
	btCollisionShape *getBulletCollisionShape() const {
		return bulletShape;
	}
	const matrix_c &getCenterOfMassTransform() const {
		return centerOfMassTransform;
	}
	bool hasCenterOfMassTransform() const {
		return bHasCenterOfMassTransform;
	}
	bulletColShape_c *getHashNext() const {
		return hashNext;
	}
	const char *getName() const {
		return name;
	}
};

#endif // __BTP_SHAPE_H__
