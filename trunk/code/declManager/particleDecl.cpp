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
// particleDecl.cpp - Doom3 particle decls handling
#include "particleDecl.h"
#include <api/coreAPI.h>
#include <shared/parser.h>

//
// particleDistribution_c
//
bool particleDistribution_c::parseParticleDistribution(class parser_c &p, const char *fname) {
	if(p.atWord("sphere")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'sphere' sizes at line %i of %s\n",line,fname);
			return true;
		}
		frac = p.getFloat();
		type = PDIST_SPHERE;
	} else if(p.atWord("cylinder")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'cylinder' sizes at line %i of %s\n",line,fname);
			return true;
		}
		type = PDIST_CYLINDER;
	} else if(p.atWord("rect")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'rect' sizes at line %i of %s\n",line,fname);
			return true;
		}
		type = PDIST_RECT;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleDistribution_c::parse: unknown particle distribution type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}
//
// particleDirection_c
//
bool particleDirection_c::parseParticleDirection(class parser_c &p, const char *fname) {
	if(p.atWord("cone")) {
		parm = p.getFloat();
		type = PDIRT_CONE;
	} else if(p.atWord("outward")) {
		parm = p.getFloat();
		type = PDIRT_OUTWARD;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleDirection_c::parse: unknown particle distribution type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}
//
// particleParm_c
//
bool particleParm_c::parseParticleParm(class parser_c &p, const char *fname) {
	const char *s = p.getToken();
	if(isdigit(s[0])) {
		from = atof(s);
		if(p.atWord("to")) {
			to = p.getFloat();
		} else {
			to = from;
		}
	} else {
		// table name
		tableName = s;
	}
	return false;
}
//
// particleOrientation_c
//
bool particleOrientation_c::parseParticleOrientation(class parser_c &p, const char *fname) {
	if(p.atWord("view")) {

		type = POR_VIEW;
	} else if(p.atWord("x")) {

		type = POR_X;
	} else if(p.atWord("y")) {

		type = POR_Y;
	} else if(p.atWord("z")) {

		type = POR_Z;
	} else if(p.atWord("aimed")) {

		type = POR_AIMED;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleOrientation_c::parse: unknown particle orientation type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}
//
// particleStage_c
//
bool particleStage_c::parseParticleStage(class parser_c &p, const char *fname) {
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atWord("count")) {
			count = p.getInteger();
		} else if(p.atWord("material")) {
			material = p.getToken();
		} else if(p.atWord("time")) {
			time = p.getFloat();
		} else if(p.atWord("cycles")) {
			cycles = p.getFloat();
		} else if(p.atWord("bunching")) {
			bunching = p.getFloat();
		} else if(p.atWord("distribution")) {
			if(distribution.parseParticleDistribution(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'distribution'\n");
			}
		} else if(p.atWord("direction")) {
			if(direction.parseParticleDirection(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'direction'\n");
			}
		} else if(p.atWord("orientation")) {
			if(orientation.parseParticleOrientation(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'orientation'\n");
			}
		} else if(p.atWord("speed")) {
			if(speed.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("size")) {
			if(size.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("aspect")) {
			if(aspect.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("rotation")) {
			if(rotSpeed.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'rotation' parm\n");
			}
		} else if(p.atWord("randomDistribution")) {
			bRandomDistribution = p.getInteger();
		} else if(p.atWord("boundsExpansion")) {
			boundsExpansion = p.getFloat();
		} else if(p.atWord("fadeIn")) {
			fadeIn = p.getFloat();
		} else if(p.atWord("fadeOut")) {
			fadeOut = p.getFloat();
		} else if(p.atWord("fadeIndex")) {
			fadeIndex = p.getFloat();
		} else if(p.atWord("color")) {
			if(p.getFloatMat(color,4)) {
				g_core->RedWarning("Failed to parse 'color' matrix (4xfloat)\n");
			}
		} else if(p.atWord("fadeColor")) {
			if(p.getFloatMat(fadeColor,4)) {
				g_core->RedWarning("Failed to parse 'fadeColor' matrix (4xfloat)\n");
			}
		} else if(p.atWord("offset")) {
			if(p.getFloatMat(offset,3)) {
				g_core->RedWarning("Failed to parse 'offset' matrix (3xfloat)\n");
			}
		} else if(p.atWord("gravity")) {
			if(p.atWord("world")) {
				bWorldGravity = true;
			}	
			gravity = p.getFloat();
		} else {
			int line = p.getCurrentLineNumber();
			const char *unk = p.getToken();
			g_core->RedWarning("particleStage_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
		}
	}
	return false;
}

	
//
// particleDecl_c
//
const char *particleDecl_c::getName() const {
	return particleDeclName;
}
void particleDecl_c::setDeclName(const char *newDeclName) {
	particleDeclName = newDeclName;
}
bool particleDecl_c::isValid() const {
	return true;
}
bool particleDecl_c::parse(const char *text, const char *textBase, const char *fname) {
	parser_c p;
	p.setup(textBase, text);
	p.setDebugFileName(fname);
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atWord_dontNeedWS("{")) {
			particleStage_c *newStage = new particleStage_c;
			if(newStage->parseParticleStage(p,fname)) {
				delete newStage;
			} else {
				stages.push_back(newStage);
			}
		} else if(p.atWord("depthHack")) {
			depthHack = p.getFloat();
		} else {
			int line = p.getCurrentLineNumber();
			const char *unk = p.getToken();
			g_core->RedWarning("particleDecl_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
		}
	}
	return false;
}

