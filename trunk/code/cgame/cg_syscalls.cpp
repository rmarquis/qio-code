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
// cg_syscalls.c -- this file is only included when building a dll
// cg_syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif

#include "cg_local.h"

static intptr_t (QDECL *syscall)( intptr_t arg, ... ) = (intptr_t (QDECL *)( intptr_t, ...))-1;


Q_EXPORT void dllEntry( intptr_t (QDECL  *syscallptr)( intptr_t arg,... ) ) {
	syscall = syscallptr;
}


int PASSFLOAT( float x ) {
	floatint_t fi;
	fi.f = x;
	return fi.i;
}

void	trap_Print( const char *fmt ) {
	syscall( CG_PRINT, fmt );
}

void trap_Error(const char *fmt)
{
	syscall(CG_ERROR, fmt);
	// shut up GCC warning about returning functions, because we know better
	exit(1);
}

int		trap_Milliseconds( void ) {
	return syscall( CG_MILLISECONDS ); 
}

void	trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	syscall( CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags );
}

void	trap_Cvar_Update( vmCvar_t *vmCvar ) {
	syscall( CG_CVAR_UPDATE, vmCvar );
}

void	trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( CG_CVAR_SET, var_name, value );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

int		trap_Argc( void ) {
	return syscall( CG_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( CG_ARGV, n, buffer, bufferLength );
}

void	trap_Args( char *buffer, int bufferLength ) {
	syscall( CG_ARGS, buffer, bufferLength );
}

void	trap_SendConsoleCommand( const char *text ) {
	syscall( CG_SENDCONSOLECOMMAND, text );
}

void	trap_AddCommand( const char *cmdName ) {
	syscall( CG_ADDCOMMAND, cmdName );
}

void	trap_RemoveCommand( const char *cmdName ) {
	syscall( CG_REMOVECOMMAND, cmdName );
}

void	trap_SendClientCommand( const char *s ) {
	syscall( CG_SENDCLIENTCOMMAND, s );
}

void	trap_UpdateScreen( void ) {
	syscall( CG_UPDATESCREEN );
}

void	trap_R_LoadWorldMap( const char *mapname ) {
	syscall( CG_R_LOADWORLDMAP, mapname );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	return syscall( CG_R_REGISTERSHADER, name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return syscall( CG_R_REGISTERSHADERNOMIP, name );
}

void	trap_R_ClearScene( void ) {
	syscall( CG_R_CLEARSCENE );
}

void	trap_R_RenderScene( const refdef_t *fd ) {
	syscall( CG_R_RENDERSCENE, fd );
}

void	trap_R_SetColor( const float *rgba ) {
	syscall( CG_R_SETCOLOR, rgba );
}

void	trap_R_DrawStretchPic( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall( CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader );
}

void		trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall( CG_GETGLCONFIG, glconfig );
}

void		trap_GetGameState( gameState_t *gamestate ) {
	syscall( CG_GETGAMESTATE, gamestate );
}

void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	syscall( CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime );
}

qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return syscall( CG_GETSNAPSHOT, snapshotNumber, snapshot );
}

qboolean	trap_GetServerCommand( int serverCommandNumber ) {
	return syscall( CG_GETSERVERCOMMAND, serverCommandNumber );
}

int			trap_GetCurrentCmdNumber( void ) {
	return syscall( CG_GETCURRENTCMDNUMBER );
}

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return syscall( CG_GETUSERCMD, cmdNumber, ucmd );
}

qboolean trap_Key_IsDown( int keynum ) {
	return syscall( CG_KEY_ISDOWN, keynum );
}

int trap_Key_GetCatcher( void ) {
	return syscall( CG_KEY_GETCATCHER );
}

void trap_Key_SetCatcher( int catcher ) {
	syscall( CG_KEY_SETCATCHER, catcher );
}

int trap_Key_GetKey( const char *binding ) {
	return syscall( CG_KEY_GETKEY, binding );
}

int trap_RealTime(qtime_t *qtime) {
	return syscall( CG_REAL_TIME, qtime );
}
