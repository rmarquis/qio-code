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
// typedefs.h

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

// signed types
typedef __int64				s64;
typedef signed int			s32;
typedef signed short		s16;
typedef signed char			s8;
typedef char				c8;

// unsigned types
typedef unsigned __int64	u64;
typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;
typedef unsigned char		byte;

// floating point types
typedef float				f32;
typedef double				f64;

// boleans
typedef s32					b32; // 32 bit boolean

// null
#ifndef NULL
#define NULL 0
#endif

#endif //__TYPEDEFS_H__