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
// FakePlayer.cpp
#include "FakePlayer.h"
#include <api/simplePathAPI.h>
#include <shared/userCmd.h>
#include "../g_pathNodes.h"

DEFINE_CLASS(FakePlayer, "Player");

// component class used for AI navigation (pathfinding)
class navigator_c {
	// AI entity (FakePlayer - bot, or Actor)
	safePtr_c<BaseEntity> entity;
	// target entity
	safePtr_c<BaseEntity> target;
	// last target position (used to recalculate path if target is moving)
	vec3_c lastTargetPosition;
	// current path (a set of ordered nodes)
	simplePathAPI_i *path;
	// current target node number
	int cur;

	void recalculatePath() {
		if(path)
			delete path;
		path = G_FindPath(entity->getOrigin(),target->getOrigin());
		lastTargetPosition = target->getOrigin();
		cur = 0;
	}
public:
	navigator_c() {
		path = 0;
		cur = 0;
	}
	void update(usercmd_s &out) {
		// reset the userCmd
		out.clear();
		// see if we have already reached the target
		if(entity->getOrigin().distSQ(target->getOrigin()) < Square(128.f)) {
			if(path) {
				delete path;
				path = 0;
			}
			return;
		}
		if(path == 0) {
			recalculatePath();
			if(path == 0) {
				return;
			}
		} else if(lastTargetPosition.distSQ(target->getOrigin()) > Square(64)) {
			recalculatePath();
			if(path == 0) {
				return;
			}
		}
		vec3_c at = entity->getOrigin();
		vec3_c targetPos;
		if(cur >= path->getNumPathPoints()) {
			targetPos = target->getOrigin();
		} else {
			targetPos = path->getPointOrigin(cur);
		}
		// project target/bot position on XY plane
		vec3_c atXY(at.x,at.y,0.f);
		vec3_c targetXY(targetPos.x,targetPos.y,0.f);
		float dist = atXY.distSQ(targetXY);
		// see if we have reached the current waypoint
		// (TODO: do more specific check, compare current
		// at->target direction with previous one ??? )
		if(dist < Square(100.f)) {
			cur++;
			if(cur >= path->getNumPathPoints()) {
				return;
			}
			update(out);
			return;
		}
		vec3_c dir = targetPos - at;
		vec3_c angles = dir.toAngles();
		out.angles[0] = 0;
		out.angles[1] = ANGLE2SHORT(angles.y);
		out.angles[2] = 0;
		out.forwardmove = 127;
	}
	void setEntity(BaseEntity *newEnt) {
		entity = newEnt;
	}
	void setTarget(BaseEntity *newTarget) {
		target = newTarget;
	}
};

FakePlayer::FakePlayer() {
	leader = 0;
	nav = 0;
}
FakePlayer::~FakePlayer() {
	if(nav)
		delete nav;
}

void FakePlayer::runFrame() {
	if(nav) {
		nav->update(pers.cmd);
	}
	Player::runFrame();
}


void FakePlayer::setLeader(class Player *newLeader) {
	leader = newLeader;
	// automatically start following leader
	if(nav)
		delete nav;
	nav = new navigator_c;
	nav->setEntity(this);
	nav->setTarget(leader);
}
