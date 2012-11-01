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
// mod_postProceess.cpp - .mdlpp files execution
// .mdlpp files (MoDeLPostProcess) are used to scale,
// rotate, and translate loaded model after loading
// them from various file formats.
#include <shared/str.h>
#include <shared/parser.h>
#include <math/aabb.h>
#include <api/coreAPI.h>
#include <api/modelPostProcessFuncs.h>

bool MOD_ApplyPostProcess(const char *modName, class modelPostProcessFuncs_i *inout) {
	str mdlppName = modName;
	mdlppName.setExtension("mdlpp");
	parser_c p;
	if(p.openFile(mdlppName)) {
		return true; // optional mdlpp file is not present
	}
	while(p.atEOF() == false) {
		if(p.atWord("scale")) {
			float scale = p.getFloat();
			inout->scaleXYZ(scale);
		} else if(p.atWord("swapYZ")) {
			inout->swapYZ();
		} else if(p.atWord("translateY")) {
			float ofs = p.getFloat();
			inout->translateY(ofs);
		} else if(p.atWord("multTexY")) {
			float f = p.getFloat();
			inout->multTexCoordsY(f);
		} else if(p.atWord("centerize")) {
			aabb bb;
			inout->getCurrentBounds(bb);
			vec3_c center = bb.getCenter();
			inout->translateXYZ(-center);
		} else {
			int line = p.getCurrentLineNumber();
			str token = p.getToken();
			g_core->RedWarning("MOD_ApplyPostProcess: unknown postprocess command %s at line %i of file %s\n",
				token.c_str(),line,mdlppName.c_str());
		}
	}
	return false;
}
