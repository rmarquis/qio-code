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

#ifdef G_ENABLE_LUA_SCRIPTING

#include <api/coreAPI.h>
#include "g_lua_eventHandler.h"
#include "g_lua.h"
#include <lauxlib.h>

void luaEventHandlerList_c::addEventHandler(struct lua_State *L, int func) {
	handlers.pushBack().set(L,func);
}
void luaEventHandlerList_c::runCallbacks(const char *sig, ...) {
	for(u32 i = 0; i < handlers.size(); i++) {
		va_list         vl;
		int             narg, nres;	// number of arguments and results

		lua_State *L = handlers[i].getLuaState();
		int ref = handlers[i].getLuaFunc();

		va_start(vl, sig);

		lua_getref ( L, ref );

		// push arguments
		narg = 0;
		while(*sig)
		{
			switch (*sig++)
			{
				case 'f':
					// float argument
					lua_pushnumber(L, va_arg(vl, float));

					break;

				case 'i':
					// int argument
					lua_pushnumber(L, va_arg(vl, int));

					break;

				case 's':
					// string argument
					lua_pushstring(L, va_arg(vl, char *));

					break;

				case 'e':
					// entity argument
					lua_pushentity(L, va_arg(vl, edict_s *));
					break;

				case 'v':
					// vector argument
					lua_pushvector(L, va_arg(vl, vec_t *));
					break;

				case '>':
					goto endwhile;

				default:
					g_core->Print("G_RunLuaFunction: invalid option (%c)\n", *(sig - 1));
			}
			narg++;
			luaL_checkstack(L, 1, "too many arguments");
		}
	  endwhile:

		// do the call
		nres = strlen(sig);			// number of expected results
		if(lua_pcall(L, narg, nres, 0) != 0)	// do the call
			g_core->Print("G_RunLuaFunction: error running function `%i': %s\n", ref, lua_tostring(L, -1));

		// retrieve results
		nres = -nres;				// stack index of first result
		while(*sig)
		{							// get results
			switch (*sig++)
			{

				case 'f':
					// float result
					if(!lua_isnumber(L, nres))
						g_core->Print("G_RunLuaFunction: wrong result type\n");
					*va_arg(vl, float *) = lua_tonumber(L, nres);

					break;

				case 'i':
					// int result
					if(!lua_isnumber(L, nres))
						g_core->Print("G_RunLuaFunction: wrong result type\n");
					*va_arg(vl, int *) = (int)lua_tonumber(L, nres);

					break;

				case 's':
					// string result
					if(!lua_isstring(L, nres))
						g_core->Print("G_RunLuaFunction: wrong result type\n");
					*va_arg(vl, const char **) = lua_tostring(L, nres);

					break;

				default:
					g_core->Print("G_RunLuaFunction: invalid option (%c)\n", *(sig - 1));
			}
			nres++;
		}
		va_end(vl);
	}
}
#endif
