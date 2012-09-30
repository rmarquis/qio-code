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
// cg_g_client->syscalls.c -- this file is only included when building a dll
// cg_g_client->syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif

#include "cg_local.h"
#include <api/clientAPI.h>

int PASSFLOAT( float x ) {
	floatint_t fi;
	fi.f = x;
	return fi.i;
}

void	trap_SendConsoleCommand( const char *text ) {
	g_client->syscall( CG_SENDCONSOLECOMMAND, text );
}

void	trap_AddCommand( const char *cmdName ) {
	g_client->syscall( CG_ADDCOMMAND, cmdName );
}

void	trap_RemoveCommand( const char *cmdName ) {
	g_client->syscall( CG_REMOVECOMMAND, cmdName );
}

void	trap_SendClientCommand( const char *s ) {
	g_client->syscall( CG_SENDCLIENTCOMMAND, s );
}


void	trap_R_LoadWorldMap( const char *mapname ) {
	g_client->syscall( CG_R_LOADWORLDMAP, mapname );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	return 0;
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return g_client->syscall( CG_R_REGISTERSHADERNOMIP, name );
}

void	trap_R_ClearScene( void ) {
	g_client->syscall( CG_R_CLEARSCENE );
}

void	trap_R_RenderScene( const refdef_t *fd ) {
	g_client->syscall( CG_R_RENDERSCENE, fd );
}

void	trap_R_SetColor( const float *rgba ) {
	g_client->syscall( CG_R_SETCOLOR, rgba );
}

void	trap_R_DrawStretchPic( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	g_client->syscall( CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader );
}

void		trap_GetGlconfig( glconfig_t *glconfig ) {
	g_client->syscall( CG_GETGLCONFIG, glconfig );
}


