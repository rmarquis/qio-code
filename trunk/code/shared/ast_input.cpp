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
#include "ast_input.h"
#include <api/tableListAPI.h>
#include <shared/autoCvar.h>

// used to force a specific value of AST parameters
static aCvar_c ast_forceParm0Value("ast_forceParm0Value","-");
static aCvar_c ast_forceParm1Value("ast_forceParm1Value","-");
static aCvar_c ast_forceParm2Value("ast_forceParm2Value","-");
static aCvar_c ast_forceParm3Value("ast_forceParm3Value","-");
static aCvar_c ast_forceParm4Value("ast_forceParm4Value","-");
static aCvar_c ast_forceParm5Value("ast_forceParm5Value","-");

inline bool AST_ForceValueEnabled(const char *s) {
	if(s[0] == '-' && s[1] == 0) {
		return false;
	}
	if(!stricmp(s,"none"))
		return false;
	return true;
}	

int astInput_c::findVariable(const char *varName) const {
	for(u32 i = 0; i < variables.size(); i++) {
		if(variables[i].hasName(varName))
			return i;
	}
	return -1;
}
float astInput_c::getTableValue(const char *tableName, float index) const {
	if(tableList == 0)
		return 0.f;
	return tableList->getTableValue(tableName,index);
}
float astInput_c::getVariableValue(const char *varName) const {
	// allow developers to force value of Doom3 material parms
	if(AST_ForceValueEnabled(ast_forceParm0Value.getStr())) {
		if(!stricmp(varName,"parm0")) {
			return ast_forceParm0Value.getFloat();
		}
	}
	if(AST_ForceValueEnabled(ast_forceParm1Value.getStr())) {
		if(!stricmp(varName,"parm1")) {
			return ast_forceParm1Value.getFloat();
		}
	}
	if(AST_ForceValueEnabled(ast_forceParm2Value.getStr())) {
		if(!stricmp(varName,"parm2")) {
			return ast_forceParm2Value.getFloat();
		}
	}
	if(AST_ForceValueEnabled(ast_forceParm3Value.getStr())) {
		if(!stricmp(varName,"parm3")) {
			return ast_forceParm3Value.getFloat();
		}
	}
	if(AST_ForceValueEnabled(ast_forceParm4Value.getStr())) {
		if(!stricmp(varName,"parm4")) {
			return ast_forceParm4Value.getFloat();
		}
	}
	if(AST_ForceValueEnabled(ast_forceParm5Value.getStr())) {
		if(!stricmp(varName,"parm5")) {
			return ast_forceParm5Value.getFloat();
		}
	}
	int i = findVariable(varName);
	if(i<0)
		return 0.f;
	return variables[i].value;
}
void astInput_c::setVariable(const char *varName, float value) {
	int i = findVariable(varName);
	if(i<0) {
		variables.push_back(floatVar_s(varName,value));
	} else {
		variables[i].value = value;
	}
}
void astInput_c::setTableList(const class tableListAPI_i *newTableList) {
	tableList = newTableList;
}
