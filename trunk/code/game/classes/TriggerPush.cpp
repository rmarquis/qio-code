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
// TriggerPush.h - trigger_push class (used for Quake3 jumppads)
#include "../g_local.h"
#include "TriggerPush.h"
#include "ModelEntity.h"
#include <api/coreAPI.h>

DEFINE_CLASS(TriggerPush, "Trigger");
DEFINE_CLASS_ALIAS(TriggerPush, trigger_push);

bool AimAtTarget(const vec3_c &startPos, const vec3_c &targetPos, vec3_c &pushVelocity) {
	float height = targetPos.z - startPos.z;
	float gravity = 800; //g_gravity.value;
	float time = sqrt(height / (0.5f * gravity));
	if (!time) {
		return true; // error
	}
	pushVelocity = targetPos - startPos;
	pushVelocity.z = 0;
	float dist = pushVelocity.normalize2();

	float forward = dist / time;
	pushVelocity *= forward;

	pushVelocity.z = time * gravity;
	return false;
}



void TriggerPush::onTriggerContact(class ModelEntity *ent) {
	BaseEntity *destEnt = G_FindFirstEntityWithTargetName(this->getTarget());
	if(destEnt == 0) {
		g_core->RedWarning("TriggerPush::onTriggerContact: cannot find target entity \"%s\"\n",this->getTarget());
		return;
	}
	vec3_c pushVel;
	AimAtTarget(this->getAbsBounds().getCenter(),destEnt->getOrigin(),pushVel);
	ent->setLinearVelocity(pushVel);
}


