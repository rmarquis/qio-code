/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Qio source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qio source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include <api/coreAPI.h>
#include <api/clientAPI.h>
#include <api/rAPI.h>
#include <api/fontAPI.h>
#include <shared/colorTable.h>
#include <protocol/userCmd.h>
#include <protocol/snapFlags.h>

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static void CG_DrawFPS( float y ) {
	const char	*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = g_core->Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	fontAPI_i *f = g_core->GetDefaultFont();
	if(f == 0) {
		return;
	}	
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		w = f->getStringWidth( s ) + 64;

		f->drawString( rf->getWinWidth() - w, y + 2, s, 1.0F);
	}
}

/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight()
{
	if (cg_drawFPS.getInt()) {
		CG_DrawFPS( 0 );
	}
}


//===========================================================================================


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


struct lagometer_t {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
};

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo() {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect() {
	float		x, y;
	int			cmdNum;
	userCmd_s	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = g_client->GetCurrentCmdNumber() - CMD_BACKUP + 1;
	g_client->GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}
	fontAPI_i *f = g_core->GetDefaultFont();
	if(f == 0)
		return;

	// also add text in center of screen
	s = "Connection Interrupted";
	w = f->getStringWidth( s );
	f->drawString( rf->getWinWidth() * 0.5f - w/2, 100, s);

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = rf->getWinWidth() - 48;
	y = rf->getWinHeight() - 48;

	rf->drawStretchPic( x, y, 48, 48, 0, 0,1,1, rf->registerMaterial("gfx/2d/net.tga" ) );
}


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D()
{
	if ( cg_draw2D.getInt() == 0 ) {
		return;
	}
	CG_DrawDisconnect();

	CG_DrawUpperRight();

	CG_DrawChat();

	if(cg.snap) {
		fontAPI_i *f = g_core->GetDefaultFont();
		if(f) {
			const char *s = va("%i/%i", cg.snap->ps.viewWeaponCurClipSize, cg.snap->ps.viewWeaponMaxClipSize);
			float w = f->getStringWidth(s) + 64;
			f->drawString(rf->getWinWidth() - w, rf->getWinHeight()-60, s);
		}
	}
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive() {
	// optionally draw the info screen instead
	if ( !cg.snap ) {
		//CG_DrawInformation();
		return;
	}

	// draw 3D view
	rf->draw3DView();
	//trap_R_RenderScene();

	// draw status bar and other floating elements
 	CG_Draw2D();
}



