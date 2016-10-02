/*
============================================================================
Copyright (C) 2016 V.

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
// autocompletion.cpp
#include "common.h"
#include <api/moduleManagerAPI.h>
#include <api/rAPI.h>
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <api/materialSystemAPI.h>
#include <api/editorAPI.h>
#include <api/tikiAPI.h>
#include <shared/array.h>
#include <shared/str.h>

static arraySTD_c<str> ac_matches;
static const char *completionString;
static str shortestMatch;
static int	matchCount;
// field we are working on
static char *completionBuffer;
static int *completionCursor;
static int completionMax;


static void FindMatches( const char *s ) {
	if ( _strnicmp( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		shortestMatch = s;
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( int i = 0 ; i < shortestMatch.size(); i++ ) {
		if ( i >= strlen( s ) ) {
			shortestMatch.capLen(i);
			break;
		}

		if ( tolower(shortestMatch[i]) != tolower(s[i]) ) {
			shortestMatch.capLen(i);
		}
	}
}

static void PrintMatches( const char *s ) {
	if ( !_strnicmp( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

static void PrintCvarMatches( const char *s ) {
	if ( !_strnicmp( s, shortestMatch, strlen( shortestMatch ) ) ) {
		str value;
		value.truncatePrintString(Cvar_VariableString( s ),64);
		Com_Printf( "    %s = \"%s\"\n", s, value.c_str() );
	}
}


static bool AC_Complete()
{
	if( matchCount == 0 )
		return true;

	int completionOffset = strlen( completionBuffer ) - strlen( completionString );

	Q_strncpyz( &completionBuffer[ completionOffset ], shortestMatch,
		completionMax - completionOffset );

	(*completionCursor) = strlen( completionBuffer );

	if( matchCount == 1 )
	{
		Q_strcat( completionBuffer, completionMax, " " );
		(*completionCursor)++;
		return true;
	}

	Com_Printf( "]%s\n", completionBuffer );

	return false;
}

#ifndef DEDICATED
void AC_CompleteKeyname()
{
	matchCount = 0;
	shortestMatch.clear();

	Key_KeynameCompletion( FindMatches );

	if( !AC_Complete( ) )
		Key_KeynameCompletion( PrintMatches );
}
#else
void AC_CompleteKeyname() {
}
#endif

void AC_AddMatch_Unique(const char *m) {
	ac_matches.add_unique(m);
}

// V: allow multiple extensions,
// this is needed for eg. map files
// (we're supporting direct loading of .bsp, .map, and .proc files)
void AC_CompleteFilename( const char *dir,
		const char *ext, const char *ext2, const char *ext3, 
		bool stripExt, bool allowNonPureFilesOnDisk )
{
	matchCount = 0;
	shortestMatch.clear();

	FS_FilenameCompletion( dir, ext, stripExt, FindMatches, allowNonPureFilesOnDisk );
	if(ext2) {
		FS_FilenameCompletion( dir, ext2, stripExt, FindMatches, allowNonPureFilesOnDisk );
	}
	if(ext3) {
		FS_FilenameCompletion( dir, ext3, stripExt, FindMatches, allowNonPureFilesOnDisk );
	}

	if( !AC_Complete( ) ) {
		FS_FilenameCompletion( dir, ext, stripExt, AC_AddMatch_Unique, allowNonPureFilesOnDisk );
		if(ext2) {
			FS_FilenameCompletion( dir, ext2, stripExt, AC_AddMatch_Unique, allowNonPureFilesOnDisk );
		}
		if(ext3) {
			FS_FilenameCompletion( dir, ext3, stripExt, AC_AddMatch_Unique, allowNonPureFilesOnDisk );
		}

		for(u32 i = 0; i < ac_matches.size(); i++) {
			PrintMatches(ac_matches[i]);
		}
		ac_matches.clear();
	}
}

// V: for "spawn" command, 
// autocompletion of entityDef names from Doom3 .def files
void AC_CompleteEntityDefName()
{
	matchCount = 0;
	shortestMatch.clear();

	if(g_declMgr == 0)
		return;

	g_declMgr->iterateEntityDefNames(FindMatches);

	if( !AC_Complete( ) ) {
		g_declMgr->iterateEntityDefNames(PrintMatches);
	}
}

// V: for "cg_testEmitter" command, 
// autocompletion of particle decl names from Doom3 .prt files
void AC_CompleteEmitterName()
{
	matchCount = 0;
	shortestMatch.clear();

	g_declMgr->iterateParticleDefNames(FindMatches);

	if( !AC_Complete( ) ) {
		g_declMgr->iterateParticleDefNames(PrintMatches);
	}
}

// V: for "cg_testMaterial", "rf_setCrosshairSurfaceMaterial" commands
// autocompletion of material names from .mtr / .shader files
void AC_CompleteMaterialName()
{
	if(g_ms == 0)
		return;

	matchCount = 0;
	shortestMatch.clear();

	g_ms->iterateAllAvailableMaterialNames(FindMatches);

	if( !AC_Complete( ) ) {
		g_ms->iterateAllAvailableMaterialNames(PrintMatches);
	}
}

void AC_CompleteMaterialFileName()
{
	if(g_ms == 0)
		return;

	matchCount = 0;
	shortestMatch.clear();

	g_ms->iterateAllAvailableMaterialFileNames(FindMatches);

	if( !AC_Complete( ) ) {
		g_ms->iterateAllAvailableMaterialFileNames(PrintMatches);
	}
}

void AC_CompleteCommand(char *cmd, bool doCommands, bool doCvars)
{
	// Skip leading whitespace and quotes
	while(*cmd) {
		if(*cmd != ' ' && *cmd != '\\' && *cmd != '/')
			break;
		cmd++;
	}

	Cmd_TokenizeStringIgnoreQuotes( cmd );
	int completionArgument = Cmd_Argc( );

	// If there is trailing whitespace on the cmd
	if( *( cmd + strlen( cmd ) - 1 ) == ' ' )
	{
		completionString = "";
		completionArgument++;
	}
	else
		completionString = Cmd_Argv( completionArgument - 1 );

#ifndef DEDICATED
	// Unconditionally add a '\' to the start of the buffer
	if( completionBuffer[ 0 ] &&
			completionBuffer[ 0 ] != '\\' )
	{
		if( completionBuffer[ 0 ] != '/' )
		{
			// Buffer is full, refuse to complete
			if( strlen( completionBuffer ) + 1 >=
				completionMax )
				return;

			memmove( &completionBuffer[ 1 ],
				&completionBuffer[ 0 ],
				strlen( completionBuffer ) + 1 );
			(*completionCursor)++;
		}

		completionBuffer[ 0 ] = '\\';
	}
#endif

	if( completionArgument > 1 )
	{
		const char *baseCmd = Cmd_Argv( 0 );
		char *p;

#ifndef DEDICATED
		// This should always be true
		if( baseCmd[ 0 ] == '\\' || baseCmd[ 0 ] == '/' )
			baseCmd++;
#endif

		if((p = strchr(cmd, ';')))
			AC_CompleteCommand( p + 1, true, true ); // Compound command
		else
			Cmd_CompleteArgument( baseCmd, cmd, completionArgument ); 
	}
	else
	{
		if( completionString[0] == '\\' || completionString[0] == '/' )
			completionString++;

		matchCount = 0;
		shortestMatch.clear();

		if( strlen( completionString ) == 0 )
			return;

		if( doCommands )
			Cmd_CommandCompletion( FindMatches );

		if( doCvars )
			Cvar_CommandCompletion( FindMatches );

		if( !AC_Complete( ) )
		{
			// run through again, printing matches
			if( doCommands )
				Cmd_CommandCompletion( PrintMatches );

			if( doCvars )
				Cvar_CommandCompletion( PrintCvarMatches );
		}
	}
}

void AC_AutoComplete(char *buf, int maxSize, int *cursor)
{
	completionBuffer = buf;
	completionMax = maxSize;
	completionCursor = cursor;

	AC_CompleteCommand( completionBuffer, true, true );
}


