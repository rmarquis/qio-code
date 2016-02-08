/*
============================================================================
Copyright (C) 2016 V.

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
// Mover.h
#ifndef __MOVER_H__
#define __MOVER_H__

#include "ModelEntity.h"

enum moverState_e {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1,
};

class Mover : public ModelEntity {
protected:
	// door offset = door_size - lip
	float lip;
	// open direction
	vec3_c direction;
	// current doors state
	moverState_e moverState;
	// door speed, by default 400.f
	float speed;
	// total open/close movement distance
	float distance;
	// current distance traveled (if door is moving)
	float traveled;
	// closed doors are at this position
	vec3_c pos1;
	// open doors are at this position
	vec3_c pos2;
	// teams are used to group doors together
	str team;

	// callbacks
	virtual void onMoverReachPos1() { }
	virtual void onMoverReachPos2()  { }
public:
	Mover();

	DECLARE_CLASS( Mover );

	virtual void postSpawn();

	virtual void runFrame();

	virtual void setKeyValue(const char *key, const char *value);

	virtual bool doUse(class Player *activator);


	virtual bool isMover() const {
		return true;
	}
	virtual const char *getMoverTeam() const {
		return team;
	}
};

#endif // __MOVER_H__

