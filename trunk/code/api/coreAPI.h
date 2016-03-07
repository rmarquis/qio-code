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
// coreAPI.h - engine core interface

#ifndef __COREAPI_H__
#define __COREAPI_H__

#include "iFaceBase.h"
#define CORE_API_IDENTSTR "CoreEngineAPI0001"


// paramters for command buffer stuffing
enum cbufExec_e {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
};

#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
enum errorParm_e {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
};


// these are only temporary function pointers, TODO: rework them?
struct coreAPI_s : public iFaceBase_i {
	void (*Print)( const char *text, ... );
	void (*RedWarning)( const char *text, ... );
	void (*Error)( int level, const char *text, ... );
	void (*DropError)( const char *text, ... );
	// milliseconds should only be used for performance tuning, never
	// for anything game related.
	int  (*Milliseconds)();
	// engine command system api
	int	(*Argc)();
	void (*ArgvBuffer)( int n, char *buffer, int bufferLength );
	void (*Args)( char *buffer, int bufferLength );
	const char *(*Argv)( int n );
	void (*Cmd_AddCommand)(const char *name, void (*func)());
	void (*Cmd_RemoveCommand)(const char *name);
	void (*Cbuf_ExecuteText)(int exec_when, const char *text);
	void (*Cbuf_AddText)( const char *text );

	void (*EditorInitRenderer)();
};

extern coreAPI_s *g_core;

#endif // __COREAPI_H__
