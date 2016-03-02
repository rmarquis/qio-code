/*
============================================================================
Copyright (C) 2012-2016 V.

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
// edict.h - single entity slot representation
#ifndef __EDICT_H__
#define __EDICT_H__

#include <math/vec3.h>
#include <math/aabb.h>

// DO NOT MODIFY THIS STRUCT
// (unless you're able to rebuild both server and game)
struct edict_s {
	// communicated by server to clients; this is non-zero only for active entities
	struct entityState_s *s;
	// time when the object was freed
	int freetime;	
	// entity class for game-only usage
	class BaseEntity *ent;
	// for serverside entity culling (BSP PVS)
	aabb absBounds;
	struct bspBoxDesc_s *bspBoxDesc;
};

#endif // __EDICT_H__

