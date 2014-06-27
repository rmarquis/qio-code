/*
============================================================================
Copyright (C) 2012-2014 V.

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
// cl_renderer.cpp - renderer module access

#include "client.h"
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/rAPI.h>
#include <api/coreAPI.h>

#include <shared/str.h>

#include "../sys/sys_local.h"
#include "../sys/sys_loadlib.h"

static moduleAPI_i *cl_rendererDLL = 0;
static moduleAPI_i *cl_rendererBackEndDLL = 0;
rAPI_i *rf;

int CL_ScaledMilliseconds(void) {
	return Sys_Milliseconds()*com_timescale->value;
}

/*
============
CL_InitRef
============
*/
void CL_InitRef( void ) {
	// new renderer initialization
	Com_Printf( "----- Initializing Renderer BackEnd DLL ----\n" );
	if ( cl_rendererBackEndDLL ) {
		Com_Error (ERR_FATAL, "Renderer BackEnd DLL already loaded!" );
	}
	str backEndModuleName = "backend";
	backEndModuleName.append(cl_r_backEnd->string);
	cl_rendererBackEndDLL = g_moduleMgr->load(backEndModuleName);
	if ( !cl_rendererBackEndDLL ) {
		g_core->RedWarning("Failed to initialize renderer backend \"%s\"\n",cl_r_backEnd->string);
		g_core->Print("Trying to load GL backend instead...\n");
		cl_rendererBackEndDLL = g_moduleMgr->load("backendGL");
		if ( !cl_rendererBackEndDLL ) {
			Com_Error (ERR_FATAL, "Couldn't load renderer backend DLL" );
		}
	}

	Com_Printf( "----- Initializing Renderer DLL ----\n" );
	if ( cl_rendererDLL ) {
		Com_Error (ERR_FATAL, "Renderer DLL already loaded!" );
	}
	cl_rendererDLL = g_moduleMgr->load("renderer");
	if ( !cl_rendererDLL ) {
		Com_Error (ERR_FATAL, "Couldn't load renderer DLL" );
	}
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	Com_Printf( "-------------------------------\n");

	// unpause so the cgame definately gets a snapshot and renders a frame
	Cvar_Set( "cl_paused", "0" );
}

/*
============
CL_ShutdownRef
============
*/
void CL_ShutdownRef( void ) {
	if ( !cl_rendererDLL ) {
		return;
	}
	rf->shutdown( true );
	// first unload frontend
	g_moduleMgr->unload(&cl_rendererDLL);
	// and then backend
	g_moduleMgr->unload(&cl_rendererBackEndDLL);
}

/*
============
CL_InitRenderer
============
*/
void CL_InitRenderer( void ) {
	rf->init();

	// load character sets
	cls.charSetShader = rf->registerMaterial( "gfx/2d/bigchars" );
	cls.whiteShader = rf->registerMaterial( "white" );
	cls.consoleShader = rf->registerMaterial( "console" );
}