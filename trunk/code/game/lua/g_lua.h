/*
===========================================================================
Copyright (C) 2007 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2013 V.

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// g_lua.h
#ifndef __G_LUA_H__
#define __G_LUA_H__

extern "C" {
#include <lua.h>
}
#include <math/math.h>
#include "../g_local.h"

//
// g_lua.c
//
void            G_RunLuaFunction(struct lua_State *L, const char *func, const char *sig, ...);

//
// lua_entity.c
//
typedef struct
{
	struct edict_s      *e;
} lua_Entity;

int             luaopen_entity(lua_State * L);
void            lua_pushentity(lua_State * L, edict_s * ent);
lua_Entity     *lua_getentity(lua_State * L, int argNum);

//
// lua_game.c
//
int             luaopen_game(lua_State * L);

//
// lua_qmath.c
//
int             luaopen_qmath(lua_State * L);

//
// lua_vector.c
//
int             luaopen_vector(lua_State * L);
void            lua_pushvector(lua_State * L, vec3_t v);
vec_t          *lua_getvector(lua_State * L, int argNum);

#endif // __G_LUA_H__
