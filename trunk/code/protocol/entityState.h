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
// entityState.h - entityState_s struct
#ifndef __ENTITYSTATE_H__
#define __ENTITYSTATE_H__

#include "../math/vec3.h"
#include "../protocol/netLimits.h"
// flags for light objects - LF_NOSHADOWS, etc
#include "../shared/lightFlags.h"

// entity flags
enum entityFlags_e {
	EF_HIDDEN = 1,
	EF_NUM_FLAG_BITS = 1
};


// compressed bone orientation
struct netBoneOr_s {
	vec3_c xyz;
	vec3_c quatXYZ; // W can be easily recalculated
};
#define MAX_NETWORKED_BONES 64

#define MAX_RMODEL_ATTACHMENTS 4

// used to attach md3 model to skeletal model (eg. head)
struct rAttachment_s {
	int modelIndex;
	int boneIndex;

	bool isEmpty() const {
		return (modelIndex == boneIndex);
	}
};

// entityState_s is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_e
	int		eFlags;			// entityFlags_e

	class vec3_c	origin;
	class vec3_c	angles;
	class vec3_c	scale;

	int		groundEntityNum;	// ENTITYNUM_NONE = in air

	int	rModelIndex; // only for clientside rendering
	int rSkinIndex;
	int colModelIndex; // for collision detection

	int parentNum; // ENTITYNUM_NONE = not attached
	int parentTagNum;
	vec3_c parentOffset; // in local space
	vec3_c localAttachmentAngles;

	// allow to attach md3 models
	rAttachment_s attachments[MAX_RMODEL_ATTACHMENTS];

	//int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses

//	int		solid;			// for client side prediction, trap_linkentity sets this properly

	// base animation index
	// (full body animation)
	int animIndex;
	// torso animation index
	// (overrides the full body animation)
	int torsoAnim;

	int lightFlags;
	float lightRadius; // only for ET_LIGHT 
	int lightTarget; // for spotlights (light target's entity number)
	float spotLightRadius; // for spotlights
	vec3_c lightColor;

	// trail emitter data for all entity types (including BaseEntity)
	int trailEmitterMaterial; 
	float trailEmitterSpriteRadius;
	int trailEmitterInterval; // in msec

	// for networked ragdolls (and bone controllers?)
	netBoneOr_s boneOrs[MAX_NETWORKED_BONES];

	// index of ragdoll def for ACTIVE ragdoll entities (CS_RAGDOLLDEFS)
	int activeRagdollDefNameIndex; // Doom3 ArticulatedFigure decl name

	void setDefaults() {
		number = -1;
		eType = 0;
		origin.clear();
		angles.clear();
		groundEntityNum = ENTITYNUM_NONE;
		rModelIndex = 0;
		colModelIndex = 0;
		parentNum = ENTITYNUM_NONE;
		parentTagNum = -1;
		animIndex = 0;
		activeRagdollDefNameIndex = 0;
		rSkinIndex = 0;
		eFlags = 0;
		lightFlags = 0;
		lightRadius = 0.f;
		lightTarget = ENTITYNUM_NONE;
		spotLightRadius = 0.f;
		trailEmitterMaterial = 0;
		trailEmitterSpriteRadius = 0.f;
		trailEmitterInterval = 0;
		lightColor.set(1.f,1.f,1.f);
		scale.set(1.f,1.f,1.f);
		memset(attachments,0,sizeof(attachments));
	}
	entityState_s() {
		setDefaults();
	}
	inline bool isEmitterActive() const {
		if(trailEmitterMaterial == 0)
			return false;
		if(trailEmitterSpriteRadius <= 0)
			return false;
		return true;
	}
	inline bool isHidden() const {
		return (eFlags & EF_HIDDEN);
	}
	inline void hideEntity() {
		eFlags |= EF_HIDDEN;
	}
	inline void showEntity() {
		eFlags &= ~EF_HIDDEN;
	}
};

#endif // __ENTITYSTATE_H__
