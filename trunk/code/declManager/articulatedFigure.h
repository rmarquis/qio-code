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
// articulatedFigure.h - Doom3 .af parsing and in-memory representation
// AF's are used to define physics body of skeletal (md5mesh) model ragdoll.
#ifndef __ARTICULATEDFIGURE_H__
#define __ARTICULATEDFIGURE_H__

#include <shared/str.h>
#include <shared/array.h>
#include <math/vec3.h>
#include "declRefState.h"
#include <api/afDeclAPI.h>

class afDecl_c : public afDeclAPI_i, public declRefState_c {
	afDecl_c *hashNext;
	afPublicData_s data;
public:
	virtual const afPublicData_s *getData() const {
		return &data;
	}
	virtual const char *getDefaultRenderModelName() const {
		return data.modelName;
	}
	const char *getName() const {
		return data.name;
	}
	void setHashNext(afDecl_c *newHashNext) {
		hashNext = newHashNext;
	}
	afDecl_c *getHashNext() const {
		return hashNext;
	}
	bool isValid() const {
		return (data.isEmpty() == false);
	}
	void setDeclName(const char *newName) {
		data.name = newName;
	}
	bool parseAFVec3(class parser_c &p, struct afVec3_s &out);
	bool parseModel(class parser_c &p, struct afModel_s &out);
	bool parseBody(class parser_c &p);
	bool parseConstraint(class parser_c &p, afConstraint_s &newCon);
	bool parseSettings(class parser_c &p);
	bool parse(const char *text, const char *textBase, const char *fname);
};

#endif // __ARTICULATEDFIGURE_H__
