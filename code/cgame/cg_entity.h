/*
============================================================================
Copyright (C) 2012-2016 V.

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
// cg_entity.h
#ifndef __CG_ENTITY_H__
#define __CG_ENTITY_H__

class cgEntity_c {
	//
	//	ENTITY STATES RECEIVED FROM SERVER
	// The only data here that is networked.
	// 
	entityState_s currentState;	// from cg.frame
	entityState_s nextState;		// from cg.nextFrame, if available
	// 
	//  NETWORK-RELATED STUFF
	//
	bool bCurrentValid;	// true if cg.frame holds this entity
	bool bInterpolate;	// true if next is valid to interpolate to
	int snapShotTime;	// last time this entity was found in a snapshot
	int lastUpdateFrame; // cg.frameNum when entity was updated

	// results of entity orientation interpolation for the current frame
	vec3_c lerpOrigin;
	vec3_c lerpAngles;

	//
	//	RENDERER & EFFECTS STUFF
	// Our rendering system is modular,
	// so it's not directly connected to CG entities.
	class rEntityAPI_i *rEnt; // for all entity types except ET_LIGHT
	class rLightAPI_i *rLight; // for ET_LIGHT and for all entities with entityState_t::lightRadius != 0.f
	class emitterBase_c *emitter; // for all entities with entity emitter enabled

	// 
	//	FUNCTIONS CALLED EVERY FRAME
	//
	void updateEntityEmitter();
	void onEntityOrientationChange();
	void interpolateEntityOrientation();
	void calcEntityLerpOrientation();
public:
	void updateCGEntity();
	bool rayTrace(class trace_c &tr, u32 skipEntNum) const;
	void setupNewEntity(u32 entNum);
	void clearEntity();
	void transitionLight();
	void resetEntity();
	void transitionEntity();
	void transitionModel();
	void transitionLocalPlayer(const struct playerState_s &ps, bool bUseCurrentState);
	// called at snapshots transition
	void setNextState(const struct entityState_s *newNext);
	// called on initial snapshot
	void setCurState(const struct entityState_s *newCur);
	class rEntityAPI_i *getRenderEntity() {
		return rEnt;
	}
	bool isCurrentValid() const {
		return bCurrentValid;
	}
};

#endif // __CG_ENTITY_H__
