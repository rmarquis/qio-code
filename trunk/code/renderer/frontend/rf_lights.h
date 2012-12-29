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
// rf_lights.h
#ifndef __RF_LIGHTS_H__
#define __RF_LIGHTS_H__

#include <math/vec3.h>
#include <math/aabb.h>
#include <api/rLightAPI.h>
#include <shared/array.h>

enum staticSurfInteractionType_e {
	SSINTER_BAD,
	SSINTER_BSP,
	SSINTER_RSURF,
};
struct staticSurfInteraction_s {
	class r_surface_c *sf;
	u32 bspSurfaceNumber;

	staticSurfInteraction_s() {
		sf = 0;
		bspSurfaceNumber = 0;
	}
};
struct entityInteraction_s {
	class rEntityImpl_c *ent;
	class rIndexedShadowVolume_c *shadowVolume; // only for stencil shadows

	entityInteraction_s() {
		ent = 0;
		shadowVolume = 0;
	}
};

class rLightImpl_c : public rLightAPI_i {
	vec3_c pos;
	float radius;

	aabb absBounds;

	arraySTD_c<staticSurfInteraction_s> staticInteractions;
	int numCurrentStaticInteractions;
	rIndexedShadowVolume_c *staticShadowVolume; 

	arraySTD_c<entityInteraction_s> entityInteractions;
	int numCurrentEntityInteractions;

	bool bCulled;
	class occlusionQueryAPI_i *oq;
	bool bCameraInside; // true if camera eye is inside light sphere

public:
	rLightImpl_c();
	~rLightImpl_c();

	void setCulled(bool newBCulled) {
		bCulled = newBCulled;
	}
	bool getBCulled() const {
		return bCulled;
	}

	class occlusionQueryAPI_i *ensureOcclusionQueryAllocated();
	virtual class occlusionQueryAPI_i *getOcclusionQuery() {
		return oq;
	}
	bool setBCameraInside();
	virtual bool getBCameraInside() const {
		return bCameraInside;
	}

	void clearInteractions();
	void clearInteractionsWithDynamicEntities();
	void recalcShadowVolumeOfStaticInteractions();
	void recalcLightInteractionsWithStaticWorld();
	void recalcLightInteractionsWithDynamicEntities();
	void recalcLightInteractions();

	void addLightInteractionDrawCalls();
	void addLightShadowVolumesDrawCalls();

	staticSurfInteraction_s &getNextStaticInteraction() {	
		numCurrentStaticInteractions++;
		if(staticInteractions.size() < numCurrentStaticInteractions) {
			staticInteractions.resize(numCurrentStaticInteractions);
		}
		return staticInteractions[numCurrentStaticInteractions-1];
	}
	void addStaticModelSurfaceInteraction(/*class r_model_c *mod, */class r_surface_c *sf) {
		staticSurfInteraction_s &n = getNextStaticInteraction();
		n.sf = sf;
		n.bspSurfaceNumber = 0;
	}
	void addBSPSurfaceInteraction(u32 bspSurfaceNum) {
		staticSurfInteraction_s &n = getNextStaticInteraction();
		n.sf = 0;
		n.bspSurfaceNumber = bspSurfaceNum;
	}

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setRadius(float newRadius);

	virtual void calcPosInEntitySpace(const rEntityAPI_i *ent, vec3_c &out) const;

	virtual const vec3_c &getOrigin() const {
		return pos;
	}
	virtual float getRadius() const {
		return radius;
	}
	const aabb &getABSBounds() const {
		return absBounds;
	}
};

#endif // __RF_LIGHTS_H__
