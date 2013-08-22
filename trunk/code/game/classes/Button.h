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
// Button.h - q3 func_button
#ifndef __FUNC_BUTTON_H__
#define __FUNC_BUTTON_H__

#include "ModelEntity.h"

enum moverState_e {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1,
};

class Button : public ModelEntity {
	float lip;
	moverState_e state;
	vec3_c moverAngles;
	vec3_c pos1, pos2;
	vec3_c moveDir;
	float speed;
public:
	Button();

	DECLARE_CLASS( Button );

	virtual void setKeyValue(const char *key, const char *value);
	virtual void postSpawn();
	virtual bool doUse(class Player *activator);
	virtual void runFrame();

	void onMoverReachPos1();
	void onMoverReachPos2();
};

#endif // __FUNC_BUTTON_H__

