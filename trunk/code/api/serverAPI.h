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

#ifndef __SERVERAPI_H__
#define __SERVERAPI_H__

#include "iFaceBase.h"

#define SERVER_API_IDENTSTR "ServerAPI0001"

typedef struct gentity_s gentity_s;

// these are only temporary function pointers, TODO: rework them?
struct svAPI_s : public iFaceBase_i {
	void (*LocateGameData)( gentity_s *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
	void (*DropClient)( int clientNum, const char *reason );
	void (*SendServerCommand)( int clientNum, const char *text );
	void (*SetConfigstring)( int num, const char *string );
	void (*GetConfigstring)( int num, char *buffer, int bufferSize );
	void (*GetUserinfo)( int num, char *buffer, int bufferSize );
	void (*SetUserinfo)( int num, const char *buffer );
	void (*GetUsercmd)( int clientNum, usercmd_t *cmd );
};

extern svAPI_s *g_server;

#endif // __SERVERAPI_H__
