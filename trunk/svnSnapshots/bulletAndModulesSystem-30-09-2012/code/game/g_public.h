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
//

// g_public.h -- game module information visible to server

#define	GAME_API_VERSION	8

class test_c {

};

//===============================================================





// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t	s;				// communicated by server to clients
	qboolean	inuse;
} sharedEntity_t;



//===============================================================

//
// system traps provided by the main engine
//
typedef enum {
	//============== general Quake services ==================

	G_PRINT,		// ( const char *string );
	// print message on the local console

	G_ERROR,		// ( const char *string );
	// abort the game

	G_MILLISECONDS,	// ( void );
	// get current time for profiling reasons
	// this should NOT be used for any game related tasks,
	// because it is not journaled

	// console variable interaction
	G_CVAR_REGISTER,	// ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	G_CVAR_UPDATE,	// ( vmCvar_t *vmCvar );
	G_CVAR_SET,		// ( const char *var_name, const char *value );
	G_CVAR_VARIABLE_INTEGER_VALUE,	// ( const char *var_name );

	G_CVAR_VARIABLE_STRING_BUFFER,	// ( const char *var_name, char *buffer, int bufsize );

	G_ARGC,			// ( void );
	// ClientCommand and ServerCommand parameter access

	G_ARGV,			// ( int n, char *buffer, int bufferLength );

	G_FS_FOPEN_FILE,	// ( const char *qpath, fileHandle_t *file, fsMode_t mode );
	G_FS_READ,		// ( void *buffer, int len, fileHandle_t f );
	G_FS_WRITE,		// ( const void *buffer, int len, fileHandle_t f );
	G_FS_FCLOSE_FILE,		// ( fileHandle_t f );

	//=========== server specific functionality =============

	G_LOCATE_GAME_DATA,		// ( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
	//							playerState_t *clients, int sizeofGameClient );
	// the game needs to let the server system know where and how big the gentities
	// are, so it can look at them directly without going through an interface

	G_DROP_CLIENT,		// ( int clientNum, const char *reason );
	// kick a client off the server with a message

	G_SEND_SERVER_COMMAND,	// ( int clientNum, const char *fmt, ... );
	// reliably sends a command string to be interpreted by the given
	// client.  If clientNum is -1, it will be sent to all clients

	G_SET_CONFIGSTRING,	// ( int num, const char *string );
	// config strings hold all the index strings, and various other information
	// that is reliably communicated to all clients
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	// All confgstrings are cleared at each level start.

	G_GET_CONFIGSTRING,	// ( int num, char *buffer, int bufferSize );

	G_GET_USERINFO,		// ( int num, char *buffer, int bufferSize );
	// userinfo strings are maintained by the server system, so they
	// are persistant across level loads, while all other game visible
	// data is completely reset

	G_SET_USERINFO,		// ( int num, const char *buffer );

	G_GET_SERVERINFO,	// ( char *buffer, int bufferSize );
	// the serverinfo info string has all the cvars visible to server browsers

	G_GET_USERCMD,	// ( int clientNum, usercmd_t *cmd )


	G_FS_GETFILELIST,
	G_REAL_TIME,



} gameImport_t;


//
// functions exported by the game subsystem
//
typedef enum {
	GAME_INIT,	// ( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	GAME_SHUTDOWN,	// (void);

	GAME_CLIENT_CONNECT,	// ( int clientNum, qboolean firstTime, qboolean isBot );
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	GAME_CLIENT_BEGIN,				// ( int clientNum );

	GAME_CLIENT_USERINFO_CHANGED,	// ( int clientNum );

	GAME_CLIENT_DISCONNECT,			// ( int clientNum );

	GAME_CLIENT_COMMAND,			// ( int clientNum );

	GAME_CLIENT_THINK,				// ( int clientNum );

	GAME_RUN_FRAME,					// ( int levelTime );

	GAME_CONSOLE_COMMAND,			// ( void );
	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue trap_argc() / trap_argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.

} gameExport_t;

