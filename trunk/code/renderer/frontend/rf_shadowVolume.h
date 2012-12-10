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
// rf_shadowVolume.h - shadow volume class for stencil shadows (Doom3-style)
#ifndef __RF_SHADOWVOLUME_H__
#define __RF_SHADOWVOLUME_H__

#include "../rIndexBuffer.h"
#include "../rPointBuffer.h"

// NOTE: 
// sizeof(vec3_c) == 12, 
// sizeof(hashVec3_c) == 16 !!!
class rIndexedShadowVolume_c {
	rIndexBuffer_c indices;
	rPointBuffer_c points;
public:
	const rIndexBuffer_c &getIndices() const {
		return indices;
	}
	const rPointBuffer_c &getPoints() const {
		return points;
	}

	void clear() {
		indices.setNullCount();
		points.setNullCount();
	}

	// before calling this rf_currentEntity and rf_currentLight must be set!
	void addDrawCall();

	// shadow volume creation
	void createShadowVolumeForEntity(class rEntityImpl_c *ent, const vec3_c &light);
	void addRSurface(const class r_surface_c *sf, const vec3_c &light);
	void fromRModel(const class r_model_c *m, const vec3_c &light);
	void addTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2, const vec3_c &light);
};

class rEntityShadowVolume_c {
	rIndexedShadowVolume_c data;
	class rEntityAPI_i *ent;
public:
	void addDrawCall();
};

#endif // __RF_SHADOWVOLUME_H__