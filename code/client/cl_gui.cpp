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
// cl_gui.cpp - graphical user interface bridge

#include "client.h"
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/coreAPI.h>
#include <api/guiAPI.h>

#include <shared/str.h>

static moduleAPI_i *cl_guiDLL = 0;
guiAPI_i *gui;

void CL_InitGUI() {
	cl_guiDLL = g_moduleMgr->load("gui");
	if ( !cl_guiDLL ) {
		g_core->RedWarning("GUI module not present\n");
		return;
	}
	g_iFaceMan->registerIFaceUser(&gui,GUI_API_IDENTSTR);
}
void CL_ShutdownGUI() {
//	gui->shutdown();
	// first unload frontend
	g_moduleMgr->unload(&cl_guiDLL);
}
bool GUI_GameCommand() {
	return false;
}
