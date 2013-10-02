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
// g_ragdoll.h
#ifndef __G_RAGDOLL_H__
#define __G_RAGDOLL_H__

#include <shared/array.h>
#include <math/matrix.h>

class ragdollAPI_i {
public:
	virtual ~ragdollAPI_i() {
	}

	virtual const arraySTD_c<matrix_c> &getCurWorldMatrices() const = 0;
	virtual void updateWorldTransforms() = 0;
	// bodyORs is an array of ragdoll bodies transform
	// (bodies, the ones loaded from .af file, and not the model bones!)
	virtual bool setPose(const class boneOrQPArray_t &bodyORs) = 0;
	// boneOrs is an array of render model bones transforms
	virtual bool setPoseFromRenderModelBonesArray(const class boneOrArray_c &boneOrs, const class skelAnimAPI_i *anim) = 0;
};

#endif // __G_RAGDOLL_H__
