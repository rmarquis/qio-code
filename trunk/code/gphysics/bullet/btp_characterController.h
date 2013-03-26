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
// btp_characterController.h
#ifndef __BTP_CHARACTERCONTROLLER_H__
#define __BTP_CHARACTERCONTROLLER_H__

#include <api/physCharacterControllerAPI.h>

class btpCharacterController_c : public physCharacterControllerAPI_i {
	class btKinematicCharacterController *ch;
	class bulletPhysicsWorld_c *myWorld;
public:
	virtual void setCharacterVelocity(const class vec3_c &newVel);
	virtual void setCharacterEntity(class BaseEntity *ent);
	virtual void update(const class vec3_c &dir);
	virtual class vec3_c &getPos() const;
	virtual bool isOnGround() const;
	virtual bool tryToJump();

	void init(class bulletPhysicsWorld_c *pWorld, const class vec3_c &pos, float characterHeight, float characterWidth);
	void destroyCharacter();
};

#endif // __BTP_CHARACTERCONTROLLER_H__
