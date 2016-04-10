/*
============================================================================
Copyright (C) 2016 Dusan Jocic <dusanjocic@msn.com>

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

#ifdef _WIN32

#include "../client/client.h"
#include "resource.h"
#include "sys_win32.h"
#include <shared/str.h>
#include <shared/colorTable.h>

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <windows.h>

#define SYSCON_DEFAULT_WIDTH    540
#define SYSCON_DEFAULT_HEIGHT   450

#define COPY_ID         1
#define QUIT_ID         2
#define CLEAR_ID        3

#define ERRORBOX_ID     10
#define ERRORTEXT_ID    11

#define EDIT_ID         100
#define INPUT_ID        101

typedef struct {
	HWND		handWnd;
	HWND		handWndBuffer;
	HWND		handWndButtonClear;
	HWND		handWndButtonCopy;
	HWND		handWndButtonQuit;
	HWND		handWndErrorBox;

	HBITMAP		handBitMapLogo;
	HBITMAP		handBitClearBitmap;

	HBRUSH		handleBrushEditBackground;
	HBRUSH		handleBrushErrorBackground;

	HFONT		handleFont;

	HWND		handWndInputLine;

	char		errString[128];
	char		conText[512], retText[512];

	s32			visLevel;
	s32			windowWidth, windowHeight;

	bool		CloseOnQuit;
	bool		timerActive;

	WNDPROC		SystemInputLineWndProc;
} WinConsoleData;

static WinConsoleData s_wcond;

/*
==============
ConWndProc
==============
*/
static LONG WINAPI ConWndProc( HWND handWnd, u32 uMsg, WPARAM wParam, LPARAM lParam ) {
	char *cmdString;
	static bool s_timePolarity;
	s32 cx, cy;
	f32 sx, sy, x, y, w, h;

	switch ( uMsg )
	{
	case WM_SIZE:
		cx = LOWORD( lParam );
		cy = HIWORD( lParam );

		//Dushan - default console size 540x450
		if ( cx < SYSCON_DEFAULT_WIDTH ) {
			cx = SYSCON_DEFAULT_WIDTH;
		}
		if ( cy < SYSCON_DEFAULT_HEIGHT ) {
			cy = SYSCON_DEFAULT_HEIGHT;
		}

		sx = (f32)cx / SYSCON_DEFAULT_WIDTH;
		sy = (f32)cy / SYSCON_DEFAULT_HEIGHT;

		x = 5;
		y = 40;
		w = cx - 15;
		h = cy - 100;
		SetWindowPos(s_wcond.handWndBuffer, NULL, x, y, w, h, 0 );

		y = y + h + 8;
		h = 20;
		SetWindowPos(s_wcond.handWndInputLine, NULL, x, y, w, h, 0 );

		y = y + h + 4;
		w = 72 * sx;
		h = 24;
		SetWindowPos(s_wcond.handWndButtonCopy, NULL, x, y, w, h, 0 );

		x = x + w + 2;
		SetWindowPos(s_wcond.handWndButtonClear, NULL, x, y, w, h, 0 );

		x = cx - 15 - w;
		SetWindowPos(s_wcond.handWndButtonQuit, NULL, x, y, w, h, 0 );

		s_wcond.windowWidth = cx;
		s_wcond.windowHeight = cy;
		break;

	case WM_ACTIVATE:
		if ( LOWORD( wParam ) != WA_INACTIVE ) {
			SetFocus(s_wcond.handWndInputLine);
		}

		if ( com_viewlog && ( com_dedicated && !com_dedicated->integer ) ) {
			// if the viewlog is open, check to see if it's being minimized
			if ( com_viewlog->integer == 1 ) {
				if ( HIWORD( wParam ) ) {
					Cvar_Set( "viewlog", "2" );
				}
			} else if ( com_viewlog->integer == 2 )   {
				if ( !HIWORD( wParam ) ) {
					Cvar_Set( "viewlog", "1" );
				}
			}
		}
		break;

	case WM_CLOSE:
		if ( ( com_dedicated && com_dedicated->integer ) ) {
			if ( s_wcond.CloseOnQuit) {
				PostQuitMessage( 0 );
			} else {
				Sys_ShowConsole( 0, false );
				Cvar_Set( "viewlog", "0" );
			}
		}
		return 0;

	case WM_CTLCOLORSTATIC:
		if ( ( HWND ) lParam == s_wcond.handWndBuffer ) {
			SetBkColor( ( HDC ) wParam, RGB( 204, 204, 204 ) );
			SetTextColor( ( HDC ) wParam, RGB( 0, 0, 0 ) );

			return ( long )s_wcond.handleBrushEditBackground;
		} else if ( ( HWND ) lParam == s_wcond.handWndErrorBox) {
			if ( s_timePolarity & 1 ) {
				SetBkColor( ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0xff, 0x0, 0x00 ) );
			} else {
				SetBkColor( ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0x00, 0x0, 0x00 ) );
			}
			return ( long ) s_wcond.handleBrushErrorBackground;
		}
		return FALSE;
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE) {
			PostQuitMessage(0);
		}
		break;
	case WM_COMMAND:
		if ( wParam == COPY_ID ) {
			SendMessage( s_wcond.handWndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcond.handWndBuffer, WM_COPY, 0, 0 );
		} else if ( wParam == QUIT_ID )   {
			if ( s_wcond.CloseOnQuit) {
				PostQuitMessage( 0 );
			} else {
				cmdString = _strdup( "quit" );
				Com_QueueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, cmdString );
			}
		} else if ( wParam == CLEAR_ID )   {
			SendMessage( s_wcond.handWndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcond.handWndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
			UpdateWindow( s_wcond.handWndBuffer);
		}
		break;

	case WM_CREATE:
		s_wcond.handBitMapLogo = LoadBitmap( g_WinV.hInstance, MAKEINTRESOURCE( IDB_BITMAP1 ) );
		s_wcond.handBitClearBitmap = LoadBitmap( g_WinV.hInstance, MAKEINTRESOURCE( IDB_BITMAP2 ) );
		s_wcond.handleBrushEditBackground = CreateSolidBrush( RGB( 204, 204, 204 ) );
		s_wcond.handleBrushErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
		SetTimer( handWnd, 1, 1000, NULL );
		break;

	case WM_ERASEBKGND:
		return DefWindowProc( handWnd, uMsg, wParam, lParam );
		break;

	case WM_TIMER:
		if ( wParam == 1 ) {
			s_timePolarity = (bool)!s_timePolarity;
			if (s_wcond.handWndErrorBox) {
				InvalidateRect(s_wcond.handWndErrorBox, NULL, FALSE );
			}
		}
		break;

	}

	return DefWindowProc( handWnd, uMsg, wParam, lParam );
}

#define QIO_COMMAND_HISTORY 128

static field_s qio_consoleField, qio_historyEditLines[ QIO_COMMAND_HISTORY ];
static char qio_completionString[ MAX_TOKEN_CHARS ], qio_currentMatch[ MAX_TOKEN_CHARS ];
static s32 qio_acLength, qio_matchCount, qio_matchIndex, qio_findMatchIndex;
static s32 qio_tabTime = 0, qio_nextHistoryLine = 0, qio_historyLine = 0;

/*
==============
Qio_FindIndexMatch
==============
*/
static void Qio_FindIndexMatch( const char *s ) {
	if (stricmpn_slashes( s, qio_completionString, strlen( qio_completionString ) ) ) {
		return;
	}
	if ( qio_findMatchIndex == qio_matchIndex ) {
		Q_strncpyz( qio_currentMatch, s, sizeof( qio_currentMatch ) );
	}

	qio_findMatchIndex++;
}

/*
==============
Qio_FindMatches
==============
*/
static void Qio_FindMatches( const char *s ) {
	int i;

	if (stricmpn_slashes( s, qio_completionString, strlen( qio_completionString ) ) ) {
		return;
	}
	qio_matchCount++;
	if ( qio_matchCount == 1 ) {
		Q_strncpyz( qio_currentMatch, s, sizeof( qio_currentMatch ) );
		return;
	}

	// cut currentMatch to the amount common with s
	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( qio_currentMatch[i] ) != tolower( s[i] ) ) {
			qio_currentMatch[i] = 0;
		}
	}
	qio_currentMatch[i] = 0;
}

/*
==============
Qio_KeyConcatArgs
==============
*/
static void Qio_KeyConcatArgs( void ) {
	s32			i;
	const char	*arg;

	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ), " " );
		arg = Cmd_Argv( i );
		while ( *arg ) {
			if ( *arg == ' ' ) {
				Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ),  "\"" );
				break;
			}
			arg++;
		}
		Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ),  Cmd_Argv( i ) );
		if ( *arg == ' ' ) {
			Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ),  "\"" );
		}
	}
}

/*
==============
Qio_ConcatRemaining
==============
*/
static void Qio_ConcatRemaining( const char *src, const char *start ) {
	const char *str;

	str = strstr( src, start );
	if ( !str ) {
		Qio_KeyConcatArgs();
		return;
	}

	str += strlen( start );
	Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ), str );
}

/*
==============
Qio_PrintMatches
==============
*/
static void Qio_PrintMatches( const char *s ) {
	if ( !stricmpn_slashes( s, qio_currentMatch, qio_acLength ) ) {
		Sys_Print( va( "  ^9%s^0\n", s ) );
	}
}

/*
==============
Qio_PrintCvarMatches

ydnar: to display cvar values
==============
*/
static void Qio_PrintCvarMatches( const char *s ) {
	if ( !stricmpn_slashes( s, qio_currentMatch, qio_acLength ) ) {
		Sys_Print( va( "  ^9%s = ^5%s^0\n", s, Cvar_VariableString( s ) ) );
	}
}

/*
==============
Qio_CompleteCommand
==============
*/
static void Qio_CompleteCommand( bool showMatches ) {
	field_s *edit, temp;

	edit = &qio_consoleField;

	if ( qio_acLength == 0 ) {
		// only look at the first token for completion purposes
		Cmd_TokenizeString( edit->buffer );

		Q_strncpyz( qio_completionString, Cmd_Argv( 0 ), sizeof( qio_completionString ) );
		if ( qio_completionString[0] == '\\' || qio_completionString[0] == '/' ) {
			Q_strncpyz( qio_completionString, qio_completionString + 1, sizeof( qio_completionString ) );
		}

		qio_matchCount = 0;
		qio_matchIndex = 0;
		qio_currentMatch[0] = 0;

		if ( strlen( qio_completionString ) == 0 ) {
			return;
		}

		Cmd_CommandCompletion( Qio_FindMatches );
		Cvar_CommandCompletion( Qio_FindMatches );

		if ( qio_matchCount == 0 ) {
			return; // no matches
		}

		memcpy( &temp, edit, sizeof( field_s ) );

		if ( qio_matchCount == 1 ) {
			Com_sprintf( edit->buffer, sizeof( edit->buffer ), "%s", qio_currentMatch );
			if ( Cmd_Argc() == 1 ) {
				Q_strcat( qio_consoleField.buffer, sizeof( qio_consoleField.buffer ), " " );
			} else {
				Qio_ConcatRemaining( temp.buffer, qio_completionString );
			}
			edit->cursor = strlen( edit->buffer );
		} else {
			Com_sprintf( edit->buffer, sizeof( edit->buffer ), "%s", qio_currentMatch );
			qio_acLength = edit->cursor = strlen( edit->buffer );
			Qio_ConcatRemaining( temp.buffer, qio_completionString );
			showMatches = true;
		}
	} else if ( qio_matchCount != 1 )    {
		char lastMatch[MAX_TOKEN_CHARS];

		Q_strncpyz( lastMatch, qio_currentMatch, sizeof( lastMatch ) );

		qio_matchIndex++;
		if ( qio_matchIndex == qio_matchCount ) {
			qio_matchIndex = 0;
		}
		qio_findMatchIndex = 0;
		Cmd_CommandCompletion( Qio_FindIndexMatch );
		Cvar_CommandCompletion( Qio_FindIndexMatch );

		memcpy( &temp, edit, sizeof( field_s ) );

		Com_sprintf( edit->buffer, sizeof( edit->buffer ), "%s", qio_currentMatch );
		edit->cursor = strlen( edit->buffer );
		Qio_ConcatRemaining( temp.buffer, lastMatch );
	}

	if ( qio_matchCount == 1 ) {
		qio_acLength = strlen( qio_currentMatch );
	}

	if ( showMatches && qio_matchCount > 0 ) {
		memcpy( &temp, edit, sizeof( *edit ) );
		temp.buffer[ qio_acLength ] = '\0';
		Sys_Print( va( "] %s\n", temp.buffer ) );
		Cmd_CommandCompletion( Qio_PrintMatches );
		Cvar_CommandCompletion( Qio_PrintCvarMatches );
	}
}

/*
==============
InputLineWndProc
==============
*/
LONG WINAPI InputLineWndProc( HWND handWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	switch ( uMsg ) {
	case WM_KILLFOCUS:
		if ( ( HWND ) wParam == s_wcond.handWnd ||
			 ( HWND ) wParam == s_wcond.handWndErrorBox) {
			SetFocus( handWnd );
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch ( wParam ) {
		case VK_UP:
			if ( ( qio_nextHistoryLine - qio_historyLine < QIO_COMMAND_HISTORY ) && qio_historyLine > 0 ) {
				qio_historyLine--;
			}
			qio_consoleField = qio_historyEditLines[ qio_historyLine % QIO_COMMAND_HISTORY ];
			SetWindowText( s_wcond.handWndInputLine, qio_consoleField.buffer );
			SendMessage( s_wcond.handWndInputLine, EM_SETSEL, qio_consoleField.cursor, 
				qio_consoleField.cursor );
			qio_acLength = 0;
			return 0;

		case VK_DOWN:
			if ( qio_historyLine < qio_nextHistoryLine ) {
				qio_historyLine++;
				qio_consoleField = qio_historyEditLines[ qio_historyLine % QIO_COMMAND_HISTORY ];
				SetWindowText( s_wcond.handWndInputLine, qio_consoleField.buffer );
				SendMessage( s_wcond.handWndInputLine, EM_SETSEL, qio_consoleField.cursor,
					qio_consoleField.cursor );
			}
			qio_acLength = 0;
			return 0;
		}
		break;

	case WM_CHAR:
		GetWindowText( s_wcond.handWndInputLine, qio_consoleField.buffer, sizeof( qio_consoleField.buffer ) );
		SendMessage( s_wcond.handWndInputLine, EM_GETSEL, (WPARAM) NULL, (LPARAM) &qio_consoleField.cursor );
		qio_consoleField.widthInChars = strlen( qio_consoleField.buffer );
		qio_consoleField.scroll = 0;

		// handle enter key
		if ( wParam == 13 ) {
			strncat( s_wcond.conText, qio_consoleField.buffer, sizeof( s_wcond.conText ) -
				strlen( s_wcond.conText) - 5 );
			strcat( s_wcond.conText, "\n" );
			SetWindowText( s_wcond.handWndInputLine, "" );

			Sys_Print( va( "]%s\n", qio_consoleField.buffer ) );

			// clear autocomplete length
			qio_acLength = 0;

			// copy line to history buffer
			if ( qio_consoleField.buffer[ 0 ] != '\0' ) {
				qio_historyEditLines[ qio_nextHistoryLine % QIO_COMMAND_HISTORY ] = qio_consoleField;
				qio_nextHistoryLine++;
				qio_historyLine = qio_nextHistoryLine;
			}

			return 0;
		}
		else if ( wParam == 9 ) {
			{
				int tabTime = Sys_Milliseconds();
				if ( ( tabTime - qio_tabTime ) < 100 ) {
					Qio_CompleteCommand( true );
					qio_tabTime = 0;
				} else {
					Qio_CompleteCommand( false );
					qio_tabTime = tabTime;
				}
			}

			SetWindowText( s_wcond.handWndInputLine, qio_consoleField.buffer );
			qio_consoleField.widthInChars = strlen( qio_consoleField.buffer );
			SendMessage( s_wcond.handWndInputLine, EM_SETSEL, qio_consoleField.cursor,
				qio_consoleField.cursor );

			return 0;
		}
		else {
			qio_acLength = 0;
		}
		break;
	}

	return CallWindowProc( s_wcond.SystemInputLineWndProc, handWnd, uMsg, wParam, lParam );
}

/*
==============
Sys_CreateConsole
==============
*/
void Sys_CreateConsole( void ) {
	HDC			hDC;
	WNDCLASS	wc;
	RECT		rect;

	const char *DEDCLASS = "Qio WinConsole";
	const char *WINDOWNAME = "Qio Console";

	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX;

	memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_WinV.hInstance;
	wc.hIcon         = LoadIcon( g_WinV.hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wc.hCursor       = LoadCursor( NULL,IDC_ARROW );
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = DEDCLASS;

	if ( !RegisterClass( &wc ) ) {
		return;
	}

	rect.left = 0;
	rect.right = SYSCON_DEFAULT_WIDTH;
	rect.top = 0;
	rect.bottom = SYSCON_DEFAULT_HEIGHT;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );

	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	s_wcond.windowWidth = rect.right - rect.left + 1;
	s_wcond.windowHeight = rect.bottom - rect.top + 1;

	s_wcond.handWnd = CreateWindowEx( 0,
		DEDCLASS,
		WINDOWNAME,
		DEDSTYLE,
		( swidth - 600 ) / 2, ( sheight - 450 ) / 2, rect.right - rect.left + 1, rect.bottom - rect.top + 1,
		NULL,
		NULL,
		g_WinV.hInstance,
		NULL );

	if ( s_wcond.handWnd == NULL ) {
		return;
	}

	//
	// create fonts
	//
	hDC = GetDC( s_wcond.handWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );

	s_wcond.handleFont = CreateFont( nHeight,
		0,
		0,
		0,
		FW_LIGHT,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_MODERN | FIXED_PITCH,
		"Courier New" );

	ReleaseDC( s_wcond.handWnd, hDC );

	//
	// create the input line
	//
	s_wcond.handWndInputLine = CreateWindowEx( WS_EX_CLIENTEDGE,
		"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
		6, 400, 528, 20,
		s_wcond.handWnd,
		( HMENU ) INPUT_ID,
		g_WinV.hInstance, NULL );

	//
	// create the buttons
	//
	s_wcond.handWndButtonCopy = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		5, 425, 72, 24,
		s_wcond.handWnd,
		( HMENU ) COPY_ID,
		g_WinV.hInstance, NULL );
	SendMessage( s_wcond.handWndButtonCopy, WM_SETTEXT, 0, ( LPARAM ) "Copy" );

	s_wcond.handWndButtonClear = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		82, 425, 72, 24,
		s_wcond.handWnd,
		( HMENU ) CLEAR_ID,
		g_WinV.hInstance, NULL );
	SendMessage( s_wcond.handWndButtonClear, WM_SETTEXT, 0, ( LPARAM ) "Clear" );

	s_wcond.handWndButtonQuit = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		462, 425, 72, 24,
		s_wcond.handWnd,
		( HMENU ) QUIT_ID,
		g_WinV.hInstance, NULL );
	SendMessage( s_wcond.handWndButtonQuit, WM_SETTEXT, 0, ( LPARAM ) "Quit" );


	//
	// create the scrollbuffer
	//
	s_wcond.handWndBuffer = CreateWindowEx( WS_EX_CLIENTEDGE,
		"edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		6, 40, 526, 354,
		s_wcond.handWnd,
		( HMENU ) EDIT_ID,
		g_WinV.hInstance, NULL );
	
	SendMessage( s_wcond.handWndBuffer, WM_SETFONT, ( WPARAM ) s_wcond.handleFont, 0 );

	s_wcond.SystemInputLineWndProc = ( WNDPROC ) SetWindowLong( s_wcond.handWndInputLine, GWL_WNDPROC,
		( long ) InputLineWndProc );
	SendMessage( s_wcond.handWndInputLine, WM_SETFONT, ( WPARAM ) s_wcond.handleFont, 0 );
	SendMessage( s_wcond.handWndBuffer, EM_LIMITTEXT, ( WPARAM )0x7fff, 0 );

	ShowWindow( s_wcond.handWnd, SW_SHOWDEFAULT );
	UpdateWindow( s_wcond.handWnd );
	SetForegroundWindow( s_wcond.handWnd );
	SetFocus( s_wcond.handWndInputLine);

	s_wcond.visLevel = 1;
}

/*
==============
Sys_DestroyConsole
==============
*/
void Sys_DestroyConsole( void ) {
	if (!s_wcond.timerActive) {
		KillTimer(s_wcond.handWnd, 1);
	}
	if ( s_wcond.handleBrushEditBackground ) {
		DeleteObject( s_wcond.handleBrushEditBackground );
	}
	if ( s_wcond.handleBrushErrorBackground ) {
		DeleteObject( s_wcond.handleBrushErrorBackground );
	}
	if ( s_wcond.handleFont ) {
		DeleteObject( s_wcond.handleFont);
	}
	if ( s_wcond.handWnd ) {
		if (s_wcond.visLevel != 0)
		{
			ShowWindow( s_wcond.handWnd, SW_HIDE );
		}
		DestroyWindow( s_wcond.handWnd );
		s_wcond.handWnd = 0;
	}
}

/*
==============
Sys_ShowConsole
==============
*/
void Sys_ShowConsole( int visLevel, bool CloseOnQuit) {
	s_wcond.CloseOnQuit = CloseOnQuit;

	if ( visLevel == s_wcond.visLevel ) {
		return;
	}

	s_wcond.visLevel = visLevel;

	if ( !s_wcond.handWnd) {
		return;
	}

	switch ( visLevel ) {
	case 0:
		ShowWindow( s_wcond.handWnd, SW_HIDE );
		break;
	case 1:
		ShowWindow( s_wcond.handWnd, SW_SHOWNORMAL );
		SendMessage( s_wcond.handWndBuffer, EM_LINESCROLL, 0, 0xffff );
		break;
	case 2:
		ShowWindow( s_wcond.handWnd, SW_MINIMIZE );
		break;
	default:
		Sys_Error( "Invalid visLevel %d sent to Sys_ShowConsole\n", visLevel );
		break;
	}
}

/*
==============
Sys_ConsoleInput
==============
*/
char *Sys_ConsoleInput( void ) {
	if ( s_wcond.conText[0] == 0 ) {
		return NULL;
	}

	strcpy( s_wcond.retText, s_wcond.conText);
	s_wcond.conText[0] = 0;

	return s_wcond.retText;
}

/*
==============
Conbuf_AppendText
==============
*/
void Conbuf_AppendText( const char *pMsg ) {
	if (!s_wcond.handWnd) {
		return;
	}
	char buffer[BUFFER_CONSOLE_SIZE * 4], *b = buffer;
	const char *msg;
	s32 bufLen, i = 0;
	static unsigned long s_totalChars;

	if ( strlen( pMsg ) > BUFFER_CONSOLE_SIZE - 1 ) {
		msg = pMsg + strlen( pMsg ) - BUFFER_CONSOLE_SIZE + 1;
	} else {
		msg = pMsg;
	}

	while ( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) ) {
		if ( msg[i] == '\n' && msg[i + 1] == '\r' ) {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		} else if ( msg[i] == '\r' )     {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else if ( msg[i] == '\n' )     {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else if ( Q_IsColorString( &msg[i] ) )   {
			i++;
		} else {
			*b = msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	if ( s_totalChars > 0x7fff ) {
		SendMessage( s_wcond.handWndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	} else {
		SendMessage( s_wcond.handWndBuffer, EM_SETSEL, 0xFFFF, 0xFFFF );
	}

	SendMessage( s_wcond.handWndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( s_wcond.handWndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( s_wcond.handWndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
}

/*
==============
Sys_SetErrorText
==============
*/
void Sys_SetErrorText( const char *buf ) {
	Q_strncpyz( s_wcond.errString, buf, sizeof( s_wcond.errString ) );

	if (!s_wcond.timerActive) {
		SetTimer(s_wcond.handWnd, 1, 1000, NULL);
		s_wcond.timerActive = true;
	}

	if ( !s_wcond.handWndErrorBox) {
		s_wcond.handWndErrorBox = CreateWindow( "static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN,
			6, 5, 526, 30,
			s_wcond.handWnd,
			( HMENU ) ERRORBOX_ID,
			g_WinV.hInstance, NULL );
		
		SendMessage( s_wcond.handWndErrorBox, WM_SETFONT, ( WPARAM ) s_wcond.handleFont, 0 );
		SetWindowText( s_wcond.handWndErrorBox, s_wcond.errString );

		DestroyWindow( s_wcond.handWndInputLine);
		s_wcond.handWndInputLine = NULL;
	}
}

/*
==============
Sys_ClearViewlog_f
==============
*/
void Sys_ClearViewlog_f( void ) {
	SendMessage( s_wcond.handWndBuffer, WM_SETTEXT, 0, (LPARAM)"" );
}

#endif
