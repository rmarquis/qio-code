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
// serverAPI.h - server core API, used by serverGame module

#ifndef __CLIENTAPI_H__
#define __CLIENTAPI_H__

#include "iFaceBase.h"

#define CLIENT_API_IDENTSTR "ClientAPI0001"

// these are only temporary function pointers, TODO: rework them?
struct clAPI_s : public iFaceBase_i {
	// the gamestate should be grabbed at startup, and whenever a
	// configstring changes
	void (*GetGameState)( gameState_s *gamestate );
	// cgame will poll each frame to see if a newer snapshot has arrived
	// that it is interested in.  The time is returned seperately so that
	// snapshot latency can be calculated.
	void (*GetCurrentSnapshotNumber)( int *snapshotNumber, int *serverTime );
	// a snapshot get can fail if the snapshot (or the entties it holds) is so
	// old that it has fallen out of the client system queue
	bool (*GetSnapshot)( int snapshotNumber, snapshot_t *snapshot );
	// retrieve a text command from the server stream
	// the current snapshot will hold the number of the most recent command
	// false can be returned if the client system handled the command
	// argc() / argv() can be used to examine the parameters of the command
	bool (*GetServerCommand)( int serverCommandNumber );
	// returns the most recent command number that can be passed to GetUserCmd
	// this will always be at least one higher than the number in the current
	// snapshot, and it may be quite a few higher if it is a fast computer on
	// a lagged connection
	int (*GetCurrentCmdNumber)( void );	
	bool (*GetUserCmd)( int cmdNumber, userCmd_s *ucmd );
};

extern clAPI_s *g_client;

#endif // __CLIENTAPI_H__
