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
// cl_renderer.cpp - renderer module access

#include "client.h"
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/rAPI.h>

#include "../sys/sys_local.h"
#include "../sys/sys_loadlib.h"

static moduleAPI_i *cl_rendererDLL = 0;
static moduleAPI_i *cl_rendererBackEndDLL = 0;
rAPI_i *rf;

/*
============
CL_RefMalloc
============
*/
void *CL_RefMalloc( int size ) {
	return Z_TagMalloc( size, TAG_RENDERER );
}

///*
//================
//CL_RefPrintf
//
//DLL glue
//================
//*/
//static __attribute__ ((format (printf, 2, 3))) void QDECL CL_RefPrintf( int print_level, const char *fmt, ...) {
//	va_list		argptr;
//	char		msg[MAXPRINTMSG];
//	
//	va_start (argptr,fmt);
//	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
//	va_end (argptr);
//
//	if ( print_level == PRINT_ALL ) {
//		Com_Printf ("%s", msg);
//	} else if ( print_level == PRINT_WARNING ) {
//		Com_Printf (S_COLOR_YELLOW "%s", msg);		// yellow
//	} else if ( print_level == PRINT_DEVELOPER ) {
//		Com_DPrintf (S_COLOR_RED "%s", msg);		// red
//	}
//}

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
	cl_rendererBackEndDLL = g_moduleMgr->load("backendNULL");
	if ( !cl_rendererBackEndDLL ) {
		Com_Error (ERR_FATAL, "Couldn't load renderer backend DLL" );
	}
	//g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);

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
	rf->shutdown( qtrue );
	g_moduleMgr->unload(&cl_rendererBackEndDLL);
	g_moduleMgr->unload(&cl_rendererDLL);
}

/*
============
CL_InitRenderer
============
*/
void CL_InitRenderer( void ) {
	// this sets up the renderer and calls R_Init
	//re.BeginRegistration( &cls.glconfig );
	rf->init();

	// load character sets
	cls.charSetShader = rf->registerMaterial( "gfx/2d/bigchars" );
	cls.whiteShader = rf->registerMaterial( "white" );
	cls.consoleShader = rf->registerMaterial( "console" );
	//g_console_field_width = rf->getWinWidth() / SMALLCHAR_WIDTH - 2;
	//g_consoleField.widthInChars = g_console_field_width;
}