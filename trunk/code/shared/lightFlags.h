/*
============================================================================
Copyright (C) 2013 V.

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
// shared/lightFlags.h - flags for light entities

#ifndef __SHARED_LIGHTFLAGS_H__
#define __SHARED_LIGHTFLAGS_H__

// light flags
enum {
	// set by "noshadows" keyword in .map keyvalues
	LF_NOSHADOWS = 1,
	// set automatically by .bsp compiler when lightmaps are baked
	LF_HASBSPLIGHTING = 2,
	// q3map style spotlight ("target" field must be set)
	LF_SPOTLIGHT = 4,
	// set if light has a custom color 
	LF_COLOURED = 8,

	LIGHTFLAGS_BITS = 4,
};

#endif // __SHARED_LIGHTFLAGS_H__
