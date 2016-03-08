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
// com_systemEventCaster.cpp - sysEventCasterAPI implementation

#include "qcommon.h"
#include <api/sysEventCasterAPI.h>
#include <api/iFaceMgrAPI.h>
#include "../client/keycodes.h"

class sysEventCasterIMPL_c : public sysEventCasterAPI_c {


	virtual void postMouseMotionEvent(float deltaX, float deltaY) {
		Com_QueueEvent( 0, SE_MOUSE, deltaX, deltaY, 0, NULL );
	}
	virtual void postMouseRightButtonDownEvent() {
		Com_QueueEvent( 0, SE_KEY, K_MOUSE1, true, 0, NULL );
	}
	virtual void postMouseRightButtonUpEvent() {
		Com_QueueEvent( 0, SE_KEY, K_MOUSE1, false, 0, NULL );
	}
	virtual void postMouseLeftButtonDownEvent() {
		Com_QueueEvent( 0, SE_KEY, K_MOUSE2, true, 0, NULL );
	}
	virtual void postMouseLeftButtonUpEvent() {
		Com_QueueEvent( 0, SE_KEY, K_MOUSE2, false, 0, NULL );
	}

};
static sysEventCasterIMPL_c g_sysEventCasterIMPL;

void Com_InitSysEventCasterAPI() {

	g_iFaceMan->registerInterface(&g_sysEventCasterIMPL,SYSEVENTCASTER_API_IDENTSTR);
}

