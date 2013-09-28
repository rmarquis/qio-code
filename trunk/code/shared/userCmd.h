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
// userCmd.h - userCmd_s struct
#ifndef __USERCMD_H__
#define __USERCMD_H__
//
// usercmd_s->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		1
#define	BUTTON_TALK			2			// displays talk balloon and disables actions
#define	BUTTON_USE_HOLDABLE	4
#define	BUTTON_GESTURE		8
#define	BUTTON_WALKING		16			// walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps
#define BUTTON_ATTACK_SECONDARY	32 // was: BUTTON_AFFIRMATIVE
//#define	BUTTON_NEGATIVE		64
//
//#define BUTTON_GETFLAG		128
//#define BUTTON_GUARDBASE	256
//#define BUTTON_PATROL		512
//#define BUTTON_FOLLOWME		1024

#define	BUTTON_ANY			2048			// any key whatsoever

#define	MOVE_RUN			120			// if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

// usercmd_s is sent to the server each client frame
struct usercmd_s {
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;           // weapon 
	signed char	forwardmove, rightmove, upmove;

	bool hasMovement() const {
		if(forwardmove)
			return true;
		if(rightmove)
			return true;
		if(upmove)
			return true;
		return false;
	}
	void clear() {
		memset(this,0,sizeof(*this));
	}
	void setAngles(const float *newAngles) {
		angles[0] = ANGLE2SHORT(newAngles[0]);
		angles[1] = ANGLE2SHORT(newAngles[1]);
		angles[2] = ANGLE2SHORT(newAngles[2]);
	}
	void deltaYaw(float delta) {
		angles[1] = ANGLE2SHORT( (SHORT2ANGLE(angles[1]) + delta) );
	}
};

#endif // __USERCMD_H__
