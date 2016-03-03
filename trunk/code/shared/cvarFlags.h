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
#ifndef __SHARED_CVARFLAGS_H__
#define __SHARED_CVARFLAGS_H__

enum {
	CVAR_ARCHIVE		=	0x0001,	// set to cause it to be saved to vars.rc
							// used for system variables, not for player
							// specific configurations
	CVAR_USERINFO		=	0x0002,	// sent to server on connect or change
	CVAR_SERVERINFO		=	0x0004,	// sent in response to front end requests
	CVAR_SYSTEMINFO		=	0x0008,	// these cvars will be duplicated on all clients
	CVAR_INIT			=	0x0010,	// don't allow change from console at all,
							// but can be set from the command line
	CVAR_LATCH			=	0x0020,	// will only change when C code next does
							// a Cvar_Get(), so it can't be changed
							// without proper initialization.  modified
							// will be set, even though the value hasn't
							// changed yet
	CVAR_ROM			=	0x0040,	// display only, cannot be set by user at all
	CVAR_USER_CREATED	=	0x0080,	// created by a set command
	CVAR_TEMP			=	0x0100,	// can be set even when cheats are disabled, but is not archived
	CVAR_CHEAT			=	0x0200,	// can not be changed if cheats are disabled
	CVAR_NORESTART		=	0x0400,	// do not clear when a cvar_restart is issued

	CVAR_SERVER_CREATED	=	0x0800,	// cvar was created by a server the client connected to.
	CVAR_VM_CREATED		=	0x1000,	// cvar was created exclusively in one of the VMs.
	CVAR_PROTECTED		=	0x2000,	// prevent modifying this var from VMs or the server
// These flags are only returned by the Cvar_Flags() function
	CVAR_MODIFIED		=	0x40000000,	// Cvar was modified
	CVAR_NONEXISTENT	=	0x80000000,	// Cvar doesn't exist.
};

#endif // __SHARED_CVARFLAGS_H__
