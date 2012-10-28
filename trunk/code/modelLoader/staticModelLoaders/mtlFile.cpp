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
#include "mtlFile.h"
#include <shared/parser.h>
#include <api/coreAPI.h>

bool mtlFile_c::loadMTL(const char *fileName) {
	parser_c p;
	if(p.openFile(fileName)) {
		g_core->RedWarning("mtlFile_c::loadMTL: cannot open %s\n",fileName);
		return true;
	}
	while(p.atEOF() == false) {
		if(p.atWord_dontNeedWS("#")) {
			p.skipLine();
		} else if(p.atWord("newmtl")) {
			parseNewMTL:
			mtlEntry_s &newMat = entries.pushBack();
			newMat.name = p.getToken();
			while(p.atEOF() == false) {
				if(p.atWord("Ns")) {
					p.getFloat();
				} else if(p.atWord("Ni")) {
					p.getFloat();
				} else if(p.atWord("d")) {
					p.getFloat();
				} else if(p.atWord("Tr")) {
					p.getFloat();
				} else if(p.atWord("Tf")) {
					p.getFloat();
					p.getFloat();
					p.getFloat();
				} else if(p.atWord("illum")) {
					p.getInteger();
				} else if(p.atWord("Ka")) {
					p.getFloat();
					p.getFloat();
					p.getFloat();
				} else if(p.atWord("Kd")) {
					p.getFloat();
					p.getFloat();
					p.getFloat();
				} else if(p.atWord("Ks")) {
					p.getFloat();
					p.getFloat();
					p.getFloat();
				} else if(p.atWord("Ke")) {
					p.getFloat();
					p.getFloat();
					p.getFloat();
				} else if(p.atWord("map_Ka")) {
					newMat.map_Ka = p.getToken();
				} else if(p.atWord("map_Kd")) {
					newMat.map_Kd = p.getToken();
				} else if(p.atWord("map_bump")) {
					newMat.map_bump = p.getToken();
				} else if(p.atWord("bump")) {
					newMat.bump = p.getToken();
				} else if(p.atWord("map_refl")) {
					newMat.map_refl = p.getToken();
				} else if(p.atWord("map_d")) {
					newMat.map_d = p.getToken();
				} else if(p.atWord("newmtl")) {
					goto parseNewMTL;
					break;
				} else {
					int line = p.getCurrentLineNumber();
					str token = p.getToken();
					g_core->RedWarning("mtlFile_c::loadMTL: unknown token %s at line %i of mtl file %s\n",token.c_str(),line,fileName);
				}
			}
		} else {
			int line = p.getCurrentLineNumber();
			str token = p.getToken();
			g_core->RedWarning("mtlFile_c::loadMTL: unknown token %s at line %i of mtl file %s\n",token.c_str(),line,fileName);
		}
	}
	return false;
}
const mtlEntry_s *mtlFile_c::findEntry(const char *mtlName) const {
	for(u32 i = 0; i < entries.size(); i++) {
		if(!stricmp(entries[i].name,mtlName)) {
			return &entries[i];
		}
	}
	return 0;
}
