/*
============================================================================
Copyright (C) 2016 Dusan Jocic <dusanjocic@msn.com

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

#include <shared/str.h>
#include "../common/common.h"
#include "sys_local.h"
#include "windows.h"
#include <shared/field.h>
#include <common/autocompletion.h>

#define CONSOLE_HISTORY 32

static WORD console_attrib;

// saved console status
static DWORD console_orig_mode;
static CONSOLE_CURSOR_INFO console_orig_cursorinfo;

// console command history
static char console_history[ CONSOLE_HISTORY ][ MAX_EDIT_LINE ];
static int console_history_pos = -1;
static int console_history_lines = 0;
static int console_history_oldest = 0;

// current edit buffer
static char console_line[ MAX_EDIT_LINE ];
static int console_linelen = 0;
static bool console_drawinput = true;
static int console_cursor;

static HANDLE console_hout;
static HANDLE console_hin;

/*
==================
CON_CtrlHandler

The Windows Console doesn't use signals for terminating the application
with Ctrl-C, logging off, window closing, etc.  Instead it uses a special
handler routine.  Fortunately, the values for Ctrl signals don't seem to
overlap with true signal codes that Windows provides, so calling
Sys_SigHandler() with those numbers should be safe for generating unique
shutdown messages.
==================
*/
static BOOL WINAPI CON_CtrlHandler( DWORD sig )
{
	Sys_SigHandler( sig );
	return TRUE;
}

/*
==================
CON_HistAdd
==================
*/
static void CON_HistAdd(void)
{
	Q_strncpyz(console_history[console_history_oldest], console_line,
		sizeof(console_history[console_history_oldest]));

	if (console_history_lines < CONSOLE_HISTORY)
		console_history_lines++;

	if (console_history_oldest >= CONSOLE_HISTORY - 1)
		console_history_oldest = 0;
	else
		console_history_oldest++;

	console_history_pos = console_history_oldest;
}

/*
==================
CON_HistNext
==================
*/
static void CON_HistNext(void)
{
	int pos;

	// don' t allow looping through history
	if (console_history_pos == console_history_oldest)
		return;

	pos = (console_history_pos >= CONSOLE_HISTORY - 1) ?
		0 : (console_history_pos + 1);

	// clear the edit buffer if they try to advance to a future command
	if (pos == console_history_oldest)
	{
		console_history_pos = pos;
		console_line[0] = '\0';
		console_linelen = 0;
		console_cursor = console_linelen;
		return;
	}

	console_history_pos = pos;
	Q_strncpyz(console_line, console_history[console_history_pos],
		sizeof(console_line));
	console_linelen = strlen(console_line);
	console_cursor = console_linelen;
}

/*
==================
CON_HistPrev
==================
*/
static void CON_HistPrev(void)
{
	int pos;

	pos = (console_history_pos < 1) ?
		(CONSOLE_HISTORY - 1) : (console_history_pos - 1);

	// don' t allow looping through history
	if (pos == console_history_oldest || pos >= console_history_lines)
		return;

	console_history_pos = pos;
	Q_strncpyz(console_line, console_history[console_history_pos],
		sizeof(console_line));
	console_linelen = strlen(console_line);
	console_cursor = console_linelen;
}

/*
==================
CON_Show
==================
*/
static void CON_Show( void )
{
	CONSOLE_SCREEN_BUFFER_INFO binfo;
	COORD writeSize = { MAX_EDIT_LINE, 1 };
	COORD writePos = { 0, 0 };
	SMALL_RECT writeArea = { 0, 0, 0, 0 };
	int i;
	CHAR_INFO line[ MAX_EDIT_LINE ];

	GetConsoleScreenBufferInfo( console_hout, &binfo );

	// if we're in the middle of printf, don't bother writing the buffer
	if( binfo.dwCursorPosition.X != 0 )
		return;

	writeArea.Left = 0;
	writeArea.Top = binfo.dwCursorPosition.Y; 
	writeArea.Bottom = binfo.dwCursorPosition.Y; 
	writeArea.Right = MAX_EDIT_LINE;

	// build a space-padded CHAR_INFO array
	for( i = 0; i < MAX_EDIT_LINE; i++ )
	{
		if( i < console_linelen )
			line[ i ].Char.AsciiChar = console_line[ i ];
		else
			line[ i ].Char.AsciiChar = ' ';

		line[ i ].Attributes =  console_attrib;
	}

	if( console_linelen > binfo.srWindow.Right )
	{
		WriteConsoleOutput( console_hout, 
			line + (console_linelen - binfo.srWindow.Right ),
			writeSize, writePos, &writeArea );
	}
	else
	{
		WriteConsoleOutput( console_hout, line, writeSize,
			writePos, &writeArea );
	}
}

/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown( void )
{
	SetConsoleMode( console_hin, console_orig_mode );
	SetConsoleCursorInfo( console_hout, &console_orig_cursorinfo );
	CloseHandle( console_hout );
	CloseHandle( console_hin );
}

/*
==================
CON_Init
==================
*/
void CON_Init( void )
{
	CONSOLE_CURSOR_INFO curs;
	CONSOLE_SCREEN_BUFFER_INFO info;
	int i;

	// handle Ctrl-C or other console termination
	SetConsoleCtrlHandler( CON_CtrlHandler, TRUE );

	console_hin = GetStdHandle( STD_INPUT_HANDLE );
	if( console_hin == INVALID_HANDLE_VALUE )
		return;

	console_hout = GetStdHandle( STD_OUTPUT_HANDLE );
	if( console_hout == INVALID_HANDLE_VALUE )
		return;

	GetConsoleMode( console_hin, &console_orig_mode );

	// allow mouse wheel scrolling
	SetConsoleMode( console_hin,
		console_orig_mode & ~ENABLE_MOUSE_INPUT );

	FlushConsoleInputBuffer( console_hin ); 

	GetConsoleScreenBufferInfo( console_hout, &info );
	console_attrib = info.wAttributes;

	SetConsoleTitle("QIO Dedicated Server Console");

	// make cursor invisible
	GetConsoleCursorInfo( console_hout, &console_orig_cursorinfo );
	curs.dwSize = 1;
	curs.bVisible = FALSE;
	SetConsoleCursorInfo( console_hout, &curs );

	// initialize history
	for( i = 0; i < CONSOLE_HISTORY; i++ )
		console_history[ i ][ 0 ] = '\0';
}

/*
==================
CON_Input
==================
*/
char *CON_Input( void )
{
	INPUT_RECORD buff[ MAX_EDIT_LINE ];
	DWORD count = 0, events = 0;
	WORD key = 0;
	int i;
	int newlinepos = -1;

	if( !GetNumberOfConsoleInputEvents( console_hin, &events ) )
		return NULL;

	if( events < 1 )
		return NULL;
  
	// if we have overflowed, start dropping oldest input events
	if( events >= MAX_EDIT_LINE )
	{
		ReadConsoleInput( console_hin, buff, 1, &events );
		return NULL;
	} 

	if( !ReadConsoleInput( console_hin, buff, events, &count ) )
		return NULL;

	FlushConsoleInputBuffer( console_hin );

	for( i = 0; i < count; i++ )
	{
		if( buff[ i ].EventType != KEY_EVENT )
			continue;
		if( !buff[ i ].Event.KeyEvent.bKeyDown ) 
			continue;

		key = buff[ i ].Event.KeyEvent.wVirtualKeyCode;

		if( key == VK_RETURN )
		{
			newlinepos = i;
			console_cursor = 0;
			break;
		}
		else if( key == VK_UP )
		{
			CON_HistPrev();
			break;
		}
		else if( key == VK_DOWN )
		{
			CON_HistNext();
			break;
		}
		else if (key == VK_LEFT)
		{
			console_cursor--;
			if (console_cursor < 0)
			{
				console_cursor = 0;
			}
			break;
		}
		else if (key == VK_RIGHT)
		{
			console_cursor++;
			if (console_cursor > console_linelen)
			{
				console_cursor = console_linelen;
			}
			break;
		}
		else if (key == VK_HOME)
		{
			console_cursor = 0;
			break;
		}
		else if (key == VK_END)
		{
			console_cursor = console_linelen;
			break;
		}
		else if( key == VK_TAB )
		{
			field_s f;

			Q_strncpyz( f.buffer, console_line,
				sizeof( f.buffer ) );
			AC_AutoComplete(f.buffer,f.getMaxSize(),f.getCursorPtr());
			Q_strncpyz( console_line, f.buffer,
				sizeof( console_line ) );
			console_linelen = strlen( console_line );
			break;
		}

		if( console_linelen < sizeof( console_line ) - 1 )
		{
			char c = buff[ i ].Event.KeyEvent.uChar.AsciiChar;

			if( key == VK_BACK )
			{
				int pos = ( console_linelen > 0 ) ?
					console_linelen - 1 : 0; 

				console_line[ pos ] = '\0';
				console_linelen = pos;
			}
			else if( c )
			{
				console_line[ console_linelen++ ] = c;
				console_line[ console_linelen ] = '\0'; 
			}
		}
	}

	if( newlinepos < 0) {
		CON_Show();

		return NULL;
	}

	if( !console_linelen ) {
		CON_Show();
		Com_Printf( "\n" );
		return NULL;
	}

	CON_HistAdd();
	Com_Printf( "]%s\n", console_line );

	console_linelen = 0;
	CON_Show();

	return console_line;
}

/*
==================
CON_Print
==================
*/
void CON_Print( const char *msg )
{
	fputs( msg, stderr );

	CON_Show( );
}
