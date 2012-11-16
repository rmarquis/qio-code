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
// skelModelImpl.h - implementation of skelModelAPI_i
#ifndef __SKELMODELIMPL_H__
#define __SKELMODELIMPL_H__

#include "sk_local.h"
#include <api/skelModelAPI.h>
#include <shared/array.h>
#include <shared/str.h>

class skelSurfIMPL_c : public skelSurfaceAPI_i {
friend class skelModelIMPL_c;
	str matName;
	str surfName;
	arraySTD_c<skelWeight_s> weights;
	arraySTD_c<skelVert_s> verts;
	arraySTD_c<u16> indices;
	virtual const char *getMatName() const {
		return matName;
	}
	virtual const char *getSurfName() const {
		return surfName;
	}
	virtual u32 getNumVerts() const {
		return verts.size();
	}
	virtual u32 getNumWeights() const {
		return weights.size();
	}
	virtual u32 getNumIndices() const {
		return indices.size();
	}
	// raw data access
	virtual const skelVert_s *getVerts() const {
		return verts.getArray();
	}
	virtual const skelWeight_s *getWeights() const {
		return weights.getArray();
	}
	virtual const u16 *getIndices() const {
		return indices.getArray();
	}
};
class skelModelIMPL_c : public skelModelAPI_i {
	str name;
	arraySTD_c<skelSurfIMPL_c> surfs;
	boneDefArray_c bones;
	boneOrArray_c baseFrameABS;

	virtual u32 getNumSurfs() const {
		return surfs.size();
	}
	virtual const skelSurfaceAPI_i *getSurface(u32 surfNum) const {
		return &surfs[surfNum];
	}
	virtual const boneOrArray_c &getBaseFrameABS() const {
		return baseFrameABS;
	}
public:
	skelModelIMPL_c();
	~skelModelIMPL_c();

	bool loadMD5Mesh(const char *fname);
};

#endif // __SKELMODELIMPL_H__
