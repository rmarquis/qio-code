/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2012-2014 V.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

// g_public.h -- game module information visible to server
#ifndef __G_PUBLIC_H__
#define __G_PUBLIC_H__

#include <math/vec3.h>
#include <math/aabb.h>

// DO NOT MODIFY THIS STRUCT
// (unless you're able to rebuild both server and game)
struct edict_s {
	entityState_s *s;	// communicated by server to clients; this is non-zero only for active entities
	int freetime;	// level.time when the object was freed
	// entity class for game-only usage
	class BaseEntity *ent;
	// for serverside entity culling (BSP PVS)
	aabb absBounds;
	struct bspBoxDesc_s *bspBoxDesc;
};

////===============================================================
//
#endif // __G_PUBLIC_H__
