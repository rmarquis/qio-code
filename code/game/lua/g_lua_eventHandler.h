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
// g_lua_eventHandler.cpp
#ifndef __G_LUA_EVENTHANDLER_H__
#define __G_LUA_EVENTHANDLER_H__

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <shared/array.h>

class luaEventHandler_c {
	struct lua_State *l;
	int func;
public:
	void set(struct lua_State *nL, int nFunc) {
		l = nL;
		func = nFunc;
	}
	struct lua_State *getLuaState() {
		return l;
	}
	int getLuaFunc() const {
		return func;
	}
};

class luaEventHandlerList_c {
	arraySTD_c<luaEventHandler_c> handlers;
public:
	void addEventHandler(struct lua_State *L, int func);
	void runCallbacks(const char *args, ...);
};

#endif // __G_LUA_EVENTHANDLER_H__
