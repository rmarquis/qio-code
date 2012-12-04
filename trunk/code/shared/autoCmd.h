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
// autoCmd.h - automatic console command system for dll modules
#ifndef __AUTOCMD_H__
#define __AUTOCMD_H__

#include "str.h"
#include "cvarModificationCallback.h"

typedef void (*consoleCommandFunc_t)(void);

class aCmd_c {
	const char *cmdName;
	consoleCommandFunc_t function;

	aCmd_c *nextModuleCMD;
public:
	aCmd_c(const char *newName, consoleCommandFunc_t newFunc);

// this should be called once on every module startup
friend void AUTOCMD_RegisterAutoConsoleCommands();
// and this on module shutdown (BEFORE g_cvars api is NULL'ed!)
friend void AUTOCMD_UnregisterAutoConsoleCommands();
};


#endif // __AUTOCMD_H__
