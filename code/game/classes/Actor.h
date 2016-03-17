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
// Actor.h
#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "ModelEntity.h"

class Actor : public ModelEntity {
	class stateMachineAPI_i *st;
	class physCharacterControllerAPI_i *characterController;
	vec3_c characterControllerOffset;

	void loadAIStateMachine(const char *fname);
public:
	Actor();
	~Actor();

	DECLARE_CLASS( Actor );

	void disableCharacterController();
	void enableCharacterController();

	virtual void runFrame();
	virtual void postSpawn();
	virtual void setOrigin(const vec3_c &newXYZ);

	virtual void setKeyValue(const char *key, const char *value);
};

#endif // __ACTOR_H__
