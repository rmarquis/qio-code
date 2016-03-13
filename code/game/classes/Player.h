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
#include <protocol/userCmd.h>
#include <protocol/playerState.h>

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
	userCmd_s	cmd;				// we would lose angles if not persistant
	bool	localClient;		// true if "ip" info key is "localhost"
	//int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
} clientPersistant_t;

enum weaponState_e {
	// player has no weapon
	WP_NONE,
	// player is raising a new weapon (curWeapon pointer is valid, nextWeapon pointer is NULL)
	WP_RAISE,
	// player is holstering old weapon (curWeapon pointer is valid, nextWeapon is not-null while switching weapons)
	WP_PUTAWAY,
	// player weapon is ready to fire (curWeapon pointer is valid, nextWeapon is NULL)
	WP_IDLE,
	// player is reloading his weapon
	WP_RELOADING,
	// weapon is firing
	WP_FIRING,
};

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'

class Player : public ModelEntity {
friend class testPlayerConditionsHandler_c;
	class physCharacterControllerAPI_i *characterController;
	str netName;
	safePtr_c<Weapon> curWeapon;
	safePtr_c<Weapon> curWeaponLeft;
	safePtr_c<Weapon> curWeaponRight;
	safePtr_c<Weapon> nextWeapon;
	enum weaponHand_e nextWeaponHand;
	enum weaponState_e weaponState;
	bool bPutaway;
	u32 weaponTime;
	vec3_c characterControllerOffset;
	bool onGround; // this is always false if player is using "noclip"
	class playerAnimControllerAPI_i *animHandler;
	u32 lastDeathTime; // in msec
	u32 lastPainTime;
	u32 curPainAnimationTime;
	str curPainAnimationName;
	// movement events
	bool bJumped;
	bool bLanding;
	float groundDist;
	// FAKK / MoHAA player state machine
	// Used to override game-code movement control.
	class stateMachineAPI_i *st_legs;
	class stateMachineAPI_i *st_torso;
	// current states
	str st_curStateLegs;
	str st_curStateTorso;
	// conditions handler
	class playerConditionsHandler_c *st_handler;
	// holstered weapons
	arraySTD_c<class Weapon*> weapons;

	void updateCurWeaponAttachment();
	void setViewModelAnim(const char *animName, int animFlags);
	void updateCurWeaponClipSize();

	void setPlayerAnimBoth(enum sharedGameAnim_e type);
	void setPlayerAnimTorso(enum sharedGameAnim_e type);

	bool isPainAnimActive() const;

	void deactivateWeapon(const char *handName);
	void activateNewWeapon();
protected:
	void playPainAnimation(const char *newPainAnimationName, u32 animTime = 0);
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
	bool secondaryFireHeld;

	class VehicleCar *vehicle;
	// Half Life2 - style entity carrying
	safePtr_c<ModelEntity> carryingEntity;

	void disableCharacterController();
	void enableCharacterController();
	void setCharacterControllerZOffset(float ofs);
	void createCharacterControllerCapsule(float cHeight, float cRadius);
	void touchTriggers();
	void updatePlayerWeapon();
	void runPlayerAnimation_gameCode();
	// use .st files to select proper animation
	// (FAKK / MOHAA way)
	void runPlayerAnimation_stateMachine();
	void runPlayer();
	void onUseKeyDown();
	void onFireKeyHeld();
	void onFireKeyDown();
	void onSecondaryFireKeyHeld();
	void onSecondaryFireKeyDown();
	void onSecondaryFireKeyUp();
	void holsterWeapon();
	void pickupPhysicsProp(class ModelEntity *ent);
	bool isCarryingEntity() const;
	void dropCarryingEntity();
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
	virtual void setKeyValue(const char *key, const char *value);
	virtual void setOrigin(const vec3_c &newXYZ);
	virtual void setLinearVelocity(const vec3_c &newVel);
	void setVehicle(class VehicleCar *newVeh);
	void setPlayerModel(const char *newPlayerModelName);
	void loadStateMachineLegs(const char *fname);
	void loadStateMachineTorso(const char *fname);
	bool hasActiveWeapon() const {
		if(curWeapon.getPtr())
			return true;
		return false;
	}
	void toggleNoclip();

	bool hasUserCmdForward() const;
	bool hasUserCmdBackward() const;
	bool hasUserCmdLeft() const;
	bool hasUserCmdRight() const;
	bool hasUserCmdUp() const;
	bool hasUserCmdDown() const;

	void cmdSay(const char *msg);

	struct playerState_s *getPlayerState();

	virtual void postSpawn();

	virtual void onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damageCount);
	virtual void onDeath();

	virtual void runFrame() {
		runPlayer();
	}
	// called from Weapon::doUse
	void addWeapon(class Weapon *newWeapon);
	bool canPickUpWeapon(class Weapon *newWeapon);

	void dropCurrentWeapon();

	//
	// state conditions
	// (for MoHAA/FAKK .st files support)
	//
	bool checkFalling(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkOnGround(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkForward(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkBackward(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkStrafeRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkStrafeLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkRun(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHasVelocity(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkBlocked(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkDuckedViewInWater(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkCrouch(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkJump(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkLookingUp(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAtLadder(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkKilled(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHeight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkRunning(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAnimDoneTorso(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAnimDoneLegs(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsWeaponActive(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsWeaponClassActive(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHasWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkNewWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkPutawayMain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsNewWeaponClass(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsNewWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAttackPrimary(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAttackSecondary(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsWeaponSemiAuto(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsWeaponReadyToFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkIsWeaponClassReadyToFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAttackRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAttackLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkPutawayRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkPutawayLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);

	bool checkReload(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkMinChargeTimeMet(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkChance(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool returnTrue(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) { return true; }
	bool returnFalse(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) { return false; }
};

#endif // __PLAYER_H__
