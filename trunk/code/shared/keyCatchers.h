/*
============================================================================
Copyright (C) 2014 V.

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
#ifndef __KEYCATCHERS_H__
#define __KEYCATCHERS_H__

// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
enum {
	KEYCATCH_CONSOLE	=		0x0001,
	KEYCATCH_UI			=		0x0002,
	KEYCATCH_MESSAGE	=		0x0004,
	KEYCATCH_CGAME		=		0x0008,
};

#endif // __KEYCATCHERS_H__
