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
// articulatedFigure.cpp - Doom3 .af parsing and in-memory representation
#include "articulatedFigure.h"
#include <shared/parser.h>
#include <api/coreAPI.h>

bool afDecl_c::parseAFVec3(class parser_c &p, struct afVec3_s &out) {
	if(p.atWord_dontNeedWS(",")) {
		// just skip ','
	}
	if(p.atWord_dontNeedWS("bonecenter")) {
		out.type = AFVEC3_BONECENTER;
		if(p.atWord_dontNeedWS("(") == false) {
			return true;
		}
		out.boneName = p.getD3Token();
		out.secondBoneName = p.getD3Token();
		if(p.atWord_dontNeedWS(")") == false) {
			return true;
		}
	} else if(p.atWord_dontNeedWS("joint")) {
		out.type = AFVEC3_JOINT;
		if(p.atWord_dontNeedWS("(") == false) {
			return true;
		}
		out.boneName = p.getD3Token();
		if(p.atWord_dontNeedWS(")") == false) {
			return true;
		}
	} else {
		out.type = AFVEC3_RAWDATA;
		bool error = p.getFloatMatD3_braced(out.rawData,3);
		return error;
	}
	if(p.atWord_dontNeedWS(",")) {
		// just skip ','
	}
	return false;
}
bool afDecl_c::parseModel(class parser_c &p, afModel_s &out) {
	if(p.atWord_dontNeedWS("bone")) {
		out.type = AFM_BONE;
		if(p.atWord_dontNeedWS("(") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected '(' to follow \"bone\", found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
		parseAFVec3(p,out.v[0]);
		parseAFVec3(p,out.v[1]);
		out.width = p.getD3Float();
		if(p.atWord_dontNeedWS(")") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected ')' after \"bone\" block, found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}		
	} else if(p.atWord_dontNeedWS("cone")) {
		out.type = AFM_CONE;
		if(p.atWord_dontNeedWS("(") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected '(' to follow \"cone\", found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
		parseAFVec3(p,out.v[0]);
		parseAFVec3(p,out.v[1]);
		out.numSides = p.getD3Integer();
		if(p.atWord_dontNeedWS(")") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected ')' after \"cone\" block, found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
	} else if(p.atWord_dontNeedWS("cylinder")) {
		out.type = AFM_CYLINDER;
		if(p.atWord_dontNeedWS("(") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected '(' to follow \"cylinder\", found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
		parseAFVec3(p,out.v[0]);
		parseAFVec3(p,out.v[1]);
		out.numSides = p.getD3Integer();
		if(p.atWord_dontNeedWS(")") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected ')' after \"cylinder\" block, found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
	} else if(p.atWord_dontNeedWS("dodecahedron")) {
		out.type = AFM_DODECAHEDRON;
		if(p.atWord_dontNeedWS("(") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected '(' to follow \"dodecahedron\", found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
		parseAFVec3(p,out.v[0]);
		parseAFVec3(p,out.v[1]);
		if(p.atWord_dontNeedWS(")") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected ')' after \"dodecahedron\" block, found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
	} else if(p.atWord_dontNeedWS("box")) {
		out.type = AFM_BOX;
		if(p.atWord_dontNeedWS("(") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected '(' to follow \"box\", found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
		parseAFVec3(p,out.v[0]);
		parseAFVec3(p,out.v[1]);
		if(p.atWord_dontNeedWS(")") == false) {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parseModel: expected ')' after \"box\" block, found %s at line %i of file %s\n",
				tok.c_str(),line,p.getDebugFileName());
			return true;
		}
	} else {
		int line = p.getCurrentLineNumber();
		str tok = p.getToken();
		g_core->RedWarning("afDecl_c::parseModel: unknown model type %s at line %i of file %s\n",
			tok.c_str(),line,p.getDebugFileName());
		return true;
	}
	return false; // no error
}
bool afDecl_c::parseBody(class parser_c &p) {
	afBody_s &newBody = data.bodies.pushBack();
	newBody.name = p.getToken();
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		g_core->RedWarning("afDecl_c::parseBody: expected '{' to follow \"body %s\" of articulatedFigure %s declaration in file %s at line %i\n",
			newBody.name.c_str(),getName(),p.getDebugFileName(),line);
		return true; // error
	}
	// parse body
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atEOF()) {
			g_core->RedWarning("afDecl_c::parseBody: unexpected EOF hit while parsing \"body %s\" block of articulatedFigure %s declaration in file %s\n",
				newBody.name.c_str(),getName(),p.getDebugFileName());
			break;
		}
		if(p.atWord("joint")) {
			newBody.jointName = p.getToken();
		} else if(p.atWord("model")) {
			if(parseModel(p,newBody.model))
				return true; // parse error
		} else if(p.atWord("origin")) {
			if(parseAFVec3(p,newBody.origin))
				return true; // parse error
		} else if(p.atWord("angles")) {
			if(p.getFloatMatD3_braced(newBody.angles,3))
				return true; // parse error
		} else if(p.atWord("density")) {
			newBody.density = p.getFloat();
		} else {
			p.getToken();
		}
	}
	return false;
}
bool afDecl_c::parseConstraint(class parser_c &p, afConstraint_s &newCon) {
	newCon.name = p.getToken();
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		g_core->RedWarning("afDecl_c::parseConstraint: expected '{' to follow \"universalJoint %s\" of articulatedFigure %s declaration in file %s at line %i\n",
			newCon.name.c_str(),getName(),p.getDebugFileName(),line);
		return true; // error
	}
	// parse universalJoint
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atEOF()) {
			g_core->RedWarning("afDecl_c::parseConstraint: unexpected EOF hit while parsing \"universalJoint %s\" block of articulatedFigure %s declaration in file %s\n",
				newCon.name.c_str(),getName(),p.getDebugFileName());
			break;
		}
		if(p.atWord("body1")) {
			newCon.body0Name = p.getToken();
		} else if(p.atWord("body2")) {
			newCon.body1Name = p.getToken();
		} else if(p.atWord("anchor")) {
			if(parseAFVec3(p,newCon.anchor))
				return true; // parse error
		} else if(p.atWord("axis")) {
			// rotation axis for hinge contstraints?
			if(parseAFVec3(p,newCon.axis))
				return true; // parse error
		} else {
			p.getToken();
		}
	}
	return false;
}
bool afDecl_c::parseSettings(class parser_c &p) {
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		g_core->RedWarning("afDecl_c::parseSettings: expected '{' to follow \"settings\" of articulatedFigure %s declaration in file %s at line %i\n",
			getName(),p.getDebugFileName(),line);
		return true; // error
	}
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atEOF()) {
			g_core->RedWarning("afDecl_c::parseSettings: unexpected EOF hit while parsing \"settings\" block of articulatedFigure %s declaration in file %s\n",
				getName(),p.getDebugFileName());
			break;
		}
		if(p.atWord("model")) {
			this->data.modelName = p.getToken();
		} else {
			p.getToken();
		}
	}
	return false;
}
bool afDecl_c::parse(const char *text, const char *textBase, const char *fname) {
	parser_c p;
	p.setup(textBase, text);
	p.setDebugFileName(fname);
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atEOF()) {
			g_core->RedWarning("afDecl_c::parse: unexpected EOF hit while parsing articulatedFigure %s declaration in file %s\n",
				getName(),fname);
			break;
		}
		if(p.atWord_dontNeedWS("settings")) {
			if(parseSettings(p)) {
				return true; // error
			}
		} else if(p.atWord("body")) {
			if(parseBody(p)) {
				return true; // error
			}
		} else if(p.atWord("universalJoint")) {
			afConstraint_s &newCon = data.constraints.pushBack();
			newCon.type = AFC_UNIVERSALJOINT;
			if(parseConstraint(p,newCon)) {
				return true;
			}
		} else if(p.atWord("hinge")) {
			afConstraint_s &newCon = data.constraints.pushBack();
			newCon.type = AFC_HINGE;
			if(parseConstraint(p,newCon)) {
				return true;
			}
		} else {
			int line = p.getCurrentLineNumber();
			str tok = p.getToken();
			g_core->RedWarning("afDecl_c::parse: unknown token %s at line %i in file %s\n",tok.c_str(),line,p.getDebugFileName());
		}
	}
	return false;
}