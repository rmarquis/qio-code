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

int		Q_stricmpn (const char *s1, const char *s2, int n);

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
		} else if(p.atWord("setallsurfsmaterial")) {
			str matName = p.getToken();
			inout->setAllSurfsMaterial(matName);
		} else {
			int line = p.getCurrentLineNumber();
			str token = p.getToken();
			g_core->RedWarning("MOD_ApplyPostProcess: unknown postprocess command %s at line %i of file %s\n",
				token.c_str(),line,mdlppName.c_str());
		}
	}
	return false;
}
const char *G_getFirstOf(const char *s, const char *charSet) {
	const char *p = s;
	u32 charSetLen = strlen(charSet);
	while(*p) {
		for(u32 i = 0; i < charSetLen; i++) {
			if(charSet[i] == *p) {
				return p;
			}
		}
		p++;
	}
	return 0;
}
void MOD_GetInlineTextArg(str &out, const char **p) {
	const char *cur = *p;
	const char *end = G_getFirstOf(cur,",|");
	if(end) {
		out.setFromTo(cur,end);
		if(*end == ',' || *end == '|')
			end++;
		*p = end;
	} else {
		out = cur;
		*p = 0;
	}
}
float MOD_GetInlineTextArgAsFloat(const char **p) {
	str tmp;
	MOD_GetInlineTextArg(tmp,p);
	return atof(tmp);
}
bool MOD_ApplyInlinePostProcess(const char *cmdsText, class modelPostProcessFuncs_i *inout) {
	const char *p = cmdsText;
	str tmp;
	while(p && *p) {
		if(!Q_stricmpn(p,"scaleTexST",strlen("scaleTexST"))) {
			p += strlen("scaleTexST");
			float stScale = MOD_GetInlineTextArgAsFloat(&p);
			inout->multTexCoordsXY(stScale);
		} else if(!Q_stricmpn(p,"material",strlen("material"))) {
			p += strlen("material");
			MOD_GetInlineTextArg(tmp,&p);
			inout->setAllSurfsMaterial(tmp);
		} else {
			p++;
		}
	}
	return false;
}