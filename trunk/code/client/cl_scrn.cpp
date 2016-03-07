/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

#include "client.h"
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <shared/str.h>
#include <shared/colorTable.h>
#include <shared/keyCatchers.h>

bool	scr_initialized;		// ready to draw



//===============================================================================

/*
=================
SCR_DrawDemoRecording
=================
*/
void SCR_DrawDemoRecording() {
	char	string[1024];
	int		pos;

	if ( !clc.demorecording ) {
		return;
	}
	if ( clc.spDemoRecording ) {
		return;
	}

	pos = FS_FTell( clc.demofile );
	sprintf( string, "RECORDING %s: %ik", clc.demoName, pos / 1024 );

	//SCR_DrawStringExt( 320 - strlen( string ) * 4, 20, 8, string, g_color_table[7], true, false );
	// TODO
	rf->drawString(300,300,string);
}


#ifdef USE_VOIP
/*
=================
SCR_DrawVoipMeter
=================
*/
void SCR_DrawVoipMeter() {
	char	buffer[16];
	char	string[256];
	int limit, i;

	if (!cl_voipShowMeter->integer)
		return;  // player doesn't want to show meter at all.
	else if (!cl_voipSend->integer)
		return;  // not recording at the moment.
	else if (clc.state != CA_ACTIVE)
		return;  // not connected to a server.
	else if (!clc.voipEnabled)
		return;  // server doesn't support VoIP.
	else if (clc.demoplaying)
		return;  // playing back a demo.
	else if (!cl_voip->integer)
		return;  // client has VoIP support disabled.

	limit = (int) (clc.voipPower * 10.0f);
	if (limit > 10)
		limit = 10;

	for (i = 0; i < limit; i++)
		buffer[i] = '*';
	while (i < 10)
		buffer[i++] = ' ';
	buffer[i] = '\0';

	sprintf( string, "VoIP: [%s]", buffer );
	//SCR_DrawStringExt( 320 - strlen( string ) * 4, 10, 8, string, g_color_table[7], true, false );
	// TODO
	rf->drawString(300,300,string);
}
#endif




void SCR_Init() {
	scr_initialized = true;
}


//=======================================================

/*
==================
SCR_DrawScreenField

This will be called twice if rendering in stereo mode
==================
*/
void SCR_DrawScreenField() {
	bool uiFullscreen;

	rf->beginFrame();

	if(clc.state != CA_ACTIVE) {
		rf->setRenderTimeMsec(cls.realtime);
	}

	// let the loadingScreenMGR override old drawing routines
	if(g_loadingScreen && g_loadingScreen->isEmpty() == false) {
		g_loadingScreen->addDrawCalls();
		return; 
	}

	uiFullscreen = 0;// (uivm && VM_Call( uivm, UI_IS_FULLSCREEN ));

	// wide aspect ratio screens need to have the sides cleared
	// unless they are displaying game renderings
	if ( uiFullscreen || (clc.state != CA_ACTIVE && clc.state != CA_CINEMATIC) ) {
		if ( rf->getWinWidth() * 480 > rf->getWinHeight() * 640 ) {
			rf->set2DColor( g_color_table[0] );
			rf->drawStretchPic( 0, 0, rf->getWinWidth(), rf->getWinHeight(), 0, 0, 0, 0, cls.whiteShader );
			rf->set2DColor( NULL );
		}
	}

	// if the menu is going to cover the entire screen, we
	// don't need to render anything under it
	if ( 1 && !uiFullscreen ) {
		switch( clc.state ) {
		default:
			Com_Error( ERR_FATAL, "SCR_DrawScreenField: bad clc.state" );
			break;
		case CA_CINEMATIC:
			SCR_DrawCinematic();
			break;
		case CA_DISCONNECTED:
			// force menu up
	//		S_StopAllSounds();
//			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			if(gui) {
				Key_SetCatcher(Key_GetCatcher() | KEYCATCH_UI);
				gui->drawGUI();
			}
			break;
		case CA_CONNECTING:
		case CA_CHALLENGING:
		case CA_CONNECTED:
			// connecting clients will only show the connection dialog
			// refresh to update the time
//			VM_Call( uivm, UI_REFRESH, cls.realtime );
//			VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, false );
			break;
		case CA_LOADING:
		case CA_PRIMED:
			// draw the game information screen and loading progress
			CL_CGameRendering();

			// also draw the connection information, so it doesn't
			// flash away too briefly on local or lan games
			// refresh to update the time
//			VM_Call( uivm, UI_REFRESH, cls.realtime );
//			VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, true );
			break;
		case CA_ACTIVE:
			// always supply STEREO_CENTER as vieworg offset is now done by the engine.
			CL_CGameRendering();
			SCR_DrawDemoRecording();
#ifdef USE_VOIP
			SCR_DrawVoipMeter();
#endif
			break;
		}
	}

	// the menu draws next
//	if ( Key_GetCatcher( ) & KEYCATCH_UI && uivm ) {
//		VM_Call( uivm, UI_REFRESH, cls.realtime );
//	}

	// console draws next
	Con_DrawConsole ();

	if(1 && clc.downloadName[0]) {
		str txt = va("Downloading %s....\n",clc.downloadName);
		rf->drawString(30,30,txt);
		txt = va("Block %i, count %i\n",clc.downloadBlock,clc.downloadCount);
		rf->drawString(30,60,txt);
	}
}

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen() {
	static int	recursive;

	if ( !scr_initialized ) {
		return;				// not initialized yet
	}

	if ( ++recursive > 2 ) {
		Com_Error( ERR_FATAL, "SCR_UpdateScreen: recursively called" );
	}
	recursive = 1;

	// If there is no VM, there are also no rendering commands issued. Stop the renderer in
	// that case.
	if( 1 || com_dedicated->integer )
	{	

		//// XXX
		//int in_anaglyphMode = Cvar_VariableIntegerValue("r_anaglyphMode");
		//// if running in stereo, we need to draw the frame twice
		//if ( cls.glconfig.stereoEnabled || in_anaglyphMode) {
		//	SCR_DrawScreenField( STEREO_LEFT );
		//	SCR_DrawScreenField( STEREO_RIGHT );
		//} else {
			SCR_DrawScreenField();
		//}
		

		//if ( com_speeds->integer ) {
		//	re.EndFrame( &time_frontend, &time_backend );
		//} else {
			rf->endFrame();
		//}
	}
	
	recursive = 0;
}

