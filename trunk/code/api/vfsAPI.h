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
// vfsAPI.h - virtual file system interface

#ifndef __VFSAPI_H__
#define __VFSAPI_H__

#include "iFaceBase.h"

#define VFS_API_IDENTSTR "VFSAPI0001"

// these are only temporary function pointers, TODO: rework them?
struct vfsAPI_s : public iFaceBase_c {
	int (*FS_FOpenFile)( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void (*FS_Read)( void *buffer, int len, fileHandle_t f );
	void (*FS_Write)( const void *buffer, int len, fileHandle_t f );
	void (*FS_FCloseFile)( fileHandle_t f );
	int (*FS_GetFileList)( const char *path, const char *extension, char *listbuf, int bufsize );
	int (*FS_Seek)( fileHandle_t f, long offset, int origin ); // fsOrigin_t
};

#endif // __VFSAPI_H__
