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
// Player.h - Game Client class header

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "ModelEntity.h"

#include "../../shared/str.h"
#include "../../qcommon/q_shared.h" // for ucmd

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

//
#define MAX_NETNAME			36

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_s	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'

class Player : public ModelEntity {
	class btKinematicCharacterController *characterController;
	str netName;
	class Weapon *curWeapon;
	vec3_c characterControllerOffset;
	bool onGround; // this is always false if player is using "noclip"
public:
	Player();
	virtual ~Player();

	DECLARE_CLASS( Player );

	playerState_s	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;

	bool noclip;

	int buttons;
	int oldButtons;
	int latchedButtons;
	
	bool useHeld; // we might use latchedButtons instead of it
	bool fireHeld;

	class VehicleCar *vehicle;

	void disableCharacterController();
	void enableCharacterController();
	void createCharacterControllerCapsule(float cHeight, float cRadius);
	void runPlayer(struct usercmd_s *ucmd);
	void onUseKeyDown();
	void onFireKeyHeld();
	void onFireKeyDown();
	void setClientViewAngle(const vec3_c &angle);
	void setNetName(const char *newNetName);
	const char *getNetName() const;
	int getViewHeight() const;
	vec3_c getEyePos() const;
	virtual vec3_c getCModelOrigin() const {
		return getOrigin()+characterControllerOffset;
	}
	const vec3_c &getViewAngles() const {
		return ps.viewangles;
	}
	void setVehicle(class VehicleCar *newVeh);

	void toggleNoclip();

	struct playerState_s *getPlayerState();

	// called from Weapon::doUse
	void addWeapon(class Weapon *newWeapon);
};

#endif // __PLAYER_H__
