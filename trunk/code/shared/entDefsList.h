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
// entDefsList.h - list of entity definitions (keyValues + primitives)
#ifndef __ENTDEFSLIST_H__
#define __ENTDEFSLIST_H__

#include "ePairsList.h"
#include "entDef.h"

class entDefsList_c {
	arraySTD_c<entDef_c*> entities;

	bool fromString(const char *txt);
	bool loadEntitiesFromBSPFile(const char *mapName); // .bsp (Q1, Q2, Q3, RTCW, ET, MoH, etc..)
	bool loadEntitiesFromMapFile(const char *mapName); // .map (D3, Q4, raw .map files)
	bool loadEntitiesFromEntitiesFile(const char *mapName); // .entities (ET:QW)
public:
	entDefsList_c();
	~entDefsList_c();
	void clear();

	// loads entity list from .map / .bsp / .ent file
	bool load(const char *mapName);


	u32 size() const {
		return entities.size();
	}
	entDef_c * operator [] (u32 entNum) {
		return entities[entNum];
	}
	const entDef_c * operator [] (u32 entNum) const {
		return entities[entNum];
	}
};

#endif // __ENTDEFSLIST_H__
