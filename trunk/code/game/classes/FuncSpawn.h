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
// FuncSpawn.h - FAKK func_spawn
// Can be tested on funcspawn.bsp from FAKK sdk.
// "modelname" (NOT "model") key defines what will be spawned.
#ifndef __FUNCSPAWN_H__
#define __FUNCSPAWN_H__

#include "BaseEntity.h"

class FuncSpawn : public BaseEntity {
	str modelToSpawn;

public:
	FuncSpawn();

	DECLARE_CLASS( FuncSpawn );

	virtual void setKeyValue(const char *key, const char *value);

	virtual void triggerBy(class BaseEntity *from, class BaseEntity *activator);
};

#endif // __FUNCSPAWN_H__

