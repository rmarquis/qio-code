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

//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

#define MAX_WORLD_COORD		( 128*1024 )
#define MIN_WORLD_COORD		( -128*1024 )
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

#define	MAX_MAP_ENTITIES	2048


#define	MAX_MAP_BRUSHES		8192


#define MAX_BRUSH_SIZE		( WORLD_SIZE )



// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1		// an eye is never valid in a solid

#define	CONTENTS_DETAIL			    0x8000000	  // brushes to be added after vis leafs
#define	CONTENTS_NEGATIVE_CURVE 0x40000000	// reverse inside / outside

#define	CONTENTS_KEEP	(CONTENTS_DETAIL | CONTENTS_NEGATIVE_CURVE)





#define	SURF_LIGHT		0x1		// value will hold the light strength

#define	SURF_SLICK		0x2		// effects game physics

#define	SURF_SKY		0x4		// don't draw, but add to skybox
#define	SURF_WARP		0x8		// turbulent water warp
#define	SURF_TRANS33	0x10
#define	SURF_TRANS66	0x20
#define	SURF_FLOWING	0x40	// scroll towards angle
#define	SURF_NODRAW		0x80	// don't bother referencing the texture

#define SURF_PATCH        0x20000000
#define	SURF_CURVE_FAKE		0x40000000
#define	SURF_CURVE		    0x80000000
#define	SURF_KEEP		(SURF_CURVE | SURF_CURVE_FAKE | SURF_PATCH)

