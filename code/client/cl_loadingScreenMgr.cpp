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
// cl_loadingScreenMgr.cpp - loading screen manager
#include "client.h"
#include <shared/str.h>
#include <shared/array.h>
#include <api/iFaceMgrAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <api/rApi.h>

class clLoadingScreenMgrIMPL_c : public loadingScreenMgrAPI_i {
	str text;
	arraySTD_c<char> buffer;
	virtual void addDrawCalls() {
		const char *p = text.c_str();
		u32 x = 16;
		u32 y = 16;
		const char *start = p;
		while(1) {
			if(*p == '\n' || *p == 0) {
				u32 len = p - start;
				if(len) {
					if(buffer.size() < len + 1) {
						buffer.resize(len + 1);
					}
					memcpy(buffer.getArray(),start,len);
					buffer[len] = 0;
					rf->drawString(x,y,buffer.getArray());
				}
				if(*p == 0)
					break;
				start = p+1;
				y += SMALLCHAR_HEIGHT;
			}
			p++;
		}
	}
	void redraw() {
		//printf("clLoadingScreenMgrIMPL_c::redraw() callind SCR_UpdateScreen...\n");
		SCR_UpdateScreen();
		//printf("clLoadingScreenMgrIMPL_c::redraw() done.\n");
	}
	virtual void addLoadingString(const char *fmt, ...) {	
		va_list	argptr;
		char	msg[MAXPRINTMSG];

		va_start (argptr,fmt);
		_vsnprintf (msg, sizeof(msg), fmt, argptr);
		va_end (argptr);

		text.append(msg);
		Com_Printf("LoadingString: %s\n",msg);
		redraw();
	}
	virtual void clear() {
		text.clear();
	}
	virtual bool isEmpty() {
		if(text.length())
			return false;
		return true;
	}
};
static clLoadingScreenMgrIMPL_c g_loadingScreenMgrIMPL;
loadingScreenMgrAPI_i *g_loadingScreen = &g_loadingScreenMgrIMPL;

void CL_InitLoadingScreenMGR() {
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_loadingScreen,LOADINGSCREENMGR_API_IDENTSTR);
}

