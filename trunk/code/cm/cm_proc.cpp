/*
============================================================================
Copyright (C) 2015 V.

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
// cm_proc.cpp - Doom3 .proc loading
// V: shouldn't we use .cm instead of .proc?
#include "cm_local.h"
#include <shared/parser.h>
#include <shared/cmSurface.h>
#include <api/coreAPI.h>

bool CM_LoadProcSubModels(const char *mapName) {
	str fname = mapName;
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("CM_LoadProcSubModels: cannot open %s\n",fname);
		return true; // error
	}

	// check for Doom3 ident first
	if(p.atWord("mapProcFile003") == false) {
		if(p.atWord("PROC")) {
			// Quake4 ident
			str version = p.getToken();
		} else {
			g_core->RedWarning("CM_LoadProcSubModels: %s has bad ident %s, should be %s or %s\n",fname,p.getToken(),"mapProcFile003","PROC");
			return true; // error
		}
	}

	while(p.atEOF() == false) {
		if(p.atWord("model")) {
			if(p.atWord("{")==false) {
				g_core->RedWarning("cmSurface_c::loadDoom3ProcFileWorldModel:  expected '{' to follow \"model\" in file %s at line %i, found %s\n",
					p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
				return true; // error
			}
			str modName = p.getToken();
			if(!Q_stricmpn(modName,"_area",5)) {
				// skip curly braced block
				p.skipCurlyBracedBlock(false);
			} else {
				cmSurface_c *mod = new cmSurface_c();
				if(mod->parseDoom3ProcModelData(p,fname)) {
					delete mod;
					return true;
				}
				mod->recalcBoundingBoxes();
				CM_CreateTriMesh(modName,mod);
			}
		} else if(p.atWord("interAreaPortals")) {
			p.skipCurlyBracedBlock();
		} else if(p.atWord("nodes")) {
			p.skipCurlyBracedBlock();
		} else if(p.atWord("shadowModel")) {
			p.skipCurlyBracedBlock();
		} else {
			g_core->RedWarning("CM_LoadProcSubModels: skipping unknown token %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
		}
	}

	return false;
}
