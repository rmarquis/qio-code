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
// map.h -- the state of the current world that all views are displaying

extern	char		currentmap[1024];

// head/tail of doubly linked lists
extern	brush_s	active_brushes;	// brushes currently being displayed
extern	brush_s	selected_brushes;	// highlighted


extern CPtrArray& g_ptrSelectedFaces;
extern CPtrArray& g_ptrSelectedFaceBrushes;
extern	brush_s	filtered_brushes;	// brushes that have been filtered or regioned

extern	entity_s	entities;
extern	entity_s	*world_entity;	// the world entity is NOT included in
									// the entities chain

extern	bool	modified;		// for quit confirmations

extern	class edVec3_c region_mins, region_maxs;
extern	bool	region_active;

void 	Map_LoadFile (char *filename);
void 	Map_SaveFile (char *filename, bool use_region);
void	Map_New (void);
void	Map_BuildBrushData(void);

void	Map_RegionOff (void);
void	Map_RegionXY (void);
void	Map_RegionTallBrush (void);
void	Map_RegionBrush (void);
void	Map_RegionSelectedBrushes (void);
bool Map_IsBrushFiltered (brush_s *b);

void Map_SaveSelected(CMemFile* pMemFile, CMemFile* pPatchFile = NULL);
void Map_ImportBuffer (char* buf);
