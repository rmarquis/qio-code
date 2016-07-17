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
// fileStreamHelper.cpp
#include <shared/typedefs.h>
#include <string.h>
#include "fileStreamHelper.h"
#include <stdarg.h>
#include <stdio.h>
#include <api/vfsAPI.h>

fileStreamHelper_c::fileStreamHelper_c() {
	handle = 0;
}
fileStreamHelper_c::~fileStreamHelper_c() {
	clear();
}
void fileStreamHelper_c::clear() {
	if(handle) {
		g_vfs->FS_FCloseFile(handle);
		handle = 0;
	}
}
bool fileStreamHelper_c::beginWriting(const char *fname) {
	int res = g_vfs->FS_FOpenFile(fname,&handle,FS_WRITE);
	if(handle == 0)
		return true;
	return false;
}
	
void fileStreamHelper_c::writeText(const char *fmt, ...) {
	va_list argptr;

	char msg[8192];
	va_start (argptr,fmt);	
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);
	
	u32 len = strlen(msg);

	g_vfs->FS_Write(msg, len, handle);
}