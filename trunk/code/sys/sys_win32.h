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

#ifdef _WIN32

#if defined ( _MSC_VER ) && ( _MSC_VER >= 1200 )
#pragma warning(disable : 4201)
#pragma warning( push )
#endif
#include <windows.h>
#if defined ( _MSC_VER ) && ( _MSC_VER >= 1200 )
#pragma warning( pop )
#endif

typedef struct {
	HWND		hWnd;
	HINSTANCE	hInstance;
	bool		isMinimized;
	u32			sysMsgTime;
} WinVars_t;

#define BUFFER_CONSOLE_SIZE 16384

extern WinVars_t g_WinV;

void	Sys_CreateConsole( void );
void    Sys_ShowConsole( int level, bool CloseOnQuit);

#endif
