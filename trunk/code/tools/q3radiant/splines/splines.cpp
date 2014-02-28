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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

//#include "stdafx.h"
//#include "qe3.h"

#include "q_shared.h"
#include "splines.h"

extern "C" {
int FS_Write( const void *buffer, int len, fileHandle_t h );
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );
fileHandle_t FS_FOpenFileWrite( const char *filename );
void FS_FCloseFile( fileHandle_t f );
}

float Q_fabs( float f ) {
	int tmp = * ( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return * ( float * ) &tmp;
}
