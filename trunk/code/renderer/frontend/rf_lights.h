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
#include <math/matrix.h>
#include <api/rLightAPI.h>
#include <shared/array.h>

enum staticSurfInteractionType_e {
	SIT_BAD,
	SIT_STATIC,
	SIT_BSP,
	SIT_PROC,
};

enum staticSurfInteractionFilterType {
	SIFT_NONE,
	SIFT_ONLY_SHADOW,
	SIFT_ONLY_LIGHTING,
};

struct staticSurfInteraction_s {
	staticSurfInteractionType_e type;
	staticSurfInteractionFilterType filter;
	class r_surface_c *sf;
	union {
		u32 bspSurfaceNumber; // for SIT_BSP
		int areaNum; // for SIT_PROC
	};

	staticSurfInteraction_s() {
		type = SIT_BAD;
		filter = SIFT_NONE;
		sf = 0;
		bspSurfaceNumber = 0;
	}
	bool isNeededForLighting() const {
		if(filter == SIFT_ONLY_SHADOW)
			return false;
		return true;
	}
	bool isNeededForShadows() const {
		if(filter == SIFT_ONLY_LIGHTING)
			return false;
		return true;
	}
};
struct entityInteraction_s {
	class rEntityImpl_c *ent;
	class rIndexedShadowVolume_c *shadowVolume; // only for stencil shadows
	u32 lastSilChangeTime;

	entityInteraction_s() {
		ent = 0;
		shadowVolume = 0;
		lastSilChangeTime = 0;
	}
	void clear();
	void zero() {
		memset(this,0,sizeof(*this));
	}
};

class rLightImpl_c : public rLightAPI_i {
	vec3_c pos;
	float radius;
	bool bNoShadows;
	
	aabb absBounds;

	arraySTD_c<staticSurfInteraction_s> staticInteractions;
	int numCurrentStaticInteractions;
	rIndexedShadowVolume_c *staticShadowVolume; 

	arraySTD_c<entityInteraction_s> entityInteractions;
	int numCurrentEntityInteractions;

	bool bCulled;
	class occlusionQueryAPI_i *oq;
	bool bCameraInside; // true if camera eye is inside light sphere

	// for shadow mapping
	int shadowMapW;
	int shadowMapH;
	matrix_c lightProj;
	matrix_c sideViews[6];
	frustum_c sideFrustums[6];
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

	virtual const matrix_c &getSMLightProj() const {
		return lightProj;
	}
	virtual const matrix_c &getSMSideView(u32 sideNum) const {
		return sideViews[sideNum];
	}
	virtual const frustum_c &getSMSideFrustum(u32 sideNum) const {
		return sideFrustums[sideNum];
	}

	void clearInteractions();
	void clearInteractionsWithDynamicEntities();
	void recalcShadowVolumeOfStaticInteractions();
	void recalcLightInteractionsWithStaticWorld();
	void refreshIntersection(entityInteraction_s &in);
	void recalcLightInteractionsWithDynamicEntities();
	void recalcLightInteractions();
	void removeEntityFromInteractionsList(class rEntityImpl_c *ent);

	float getShadowMapW() const;
	float getShadowMapH() const;

	// forward rendering
	void addStaticSurfInteractionDrawCall(struct staticSurfInteraction_s &in, bool addingForShadowMapping);
	void addLightInteractionDrawCalls();
	// only for stencil shadows
	void addLightShadowVolumesDrawCalls();
	// only for shadow mapping
	void recalcShadowMappingMatrices();
	void addShadowMapRenderingDrawCalls();

	bool isCulledByAreas() const;

	staticSurfInteraction_s &getNextStaticInteraction() {	
		numCurrentStaticInteractions++;
		if(staticInteractions.size() < numCurrentStaticInteractions) {
			staticInteractions.resize(numCurrentStaticInteractions);
		}
		return staticInteractions[numCurrentStaticInteractions-1];
	}
	void addStaticModelSurfaceInteraction(/*class r_model_c *mod, */class r_surface_c *sf) {
		staticSurfInteraction_s &n = getNextStaticInteraction();
		n.type = SIT_STATIC;
		n.sf = sf;
		n.bspSurfaceNumber = 0;
		n.filter = SIFT_NONE;
	}
	void addBSPSurfaceInteraction(u32 bspSurfaceNum) {
		staticSurfInteraction_s &n = getNextStaticInteraction();
		n.type = SIT_BSP;
		n.sf = 0;
		n.bspSurfaceNumber = bspSurfaceNum;
		n.filter = SIFT_NONE;
	}
	void addProcAreaSurfaceInteraction(int areaNum, class r_surface_c *sf, staticSurfInteractionFilterType filter = SIFT_NONE) {
		staticSurfInteraction_s &n = getNextStaticInteraction();
		n.type = SIT_PROC;
		n.sf = sf;
		n.areaNum = areaNum;
		n.filter = filter;
	}

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setRadius(float newRadius);
	virtual void setBNoShadows(bool newBNoShadows);

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
	bool getBNoShadows() const {
		return bNoShadows;
	}
};

#endif // __RF_LIGHTS_H__
