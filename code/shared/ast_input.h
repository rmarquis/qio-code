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
// shared/ast_input.h - varlist for ASTs
#ifndef __SHARED_AST_INPUT_H__
#define __SHARED_AST_INPUT_H__

#include "ast.h"
#include "str.h"
#include "array.h"

struct floatVar_s {
	str name;
	float value;

	floatVar_s() {
		value = 0.f;
	}
	floatVar_s(const char *newName, float newValue) {
		name = newName;
		value = newValue;
	}
	bool hasName(const char *s) const {
		if(!_stricmp(name,s))
			return true;
		return false;
	}
};

class astInput_c : public astInputAPI_i {
	const class tableListAPI_i *tableList;
	arraySTD_c<floatVar_s> variables;

	int findVariable(const char *varName) const;
public:
	virtual float getTableValue(const char *tableName, float index) const;
	virtual float getVariableValue(const char *varName) const;

	void setVariable(const char *varName, float value);
	void setTableList(const class tableListAPI_i *newTableList);
};


#endif // __SHARED_AST_INPUT_H__
