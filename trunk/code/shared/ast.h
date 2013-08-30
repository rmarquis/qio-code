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
// shared/ast.h - Abstract Syntax Tree for Doom3 material expressions evaluation
#ifndef __SHARED_AST_H__
#define __SHARED_AST_H__

#include "typedefs.h"

class astInputAPI_i {
public:
	virtual float getTableValue(const char *tableName, float index) const = 0;
	virtual float getVariableValue(const char *varName) const = 0;
};
class astAPI_i {
public:
	virtual astAPI_i *duplicateAST() const = 0;
	virtual void destroyAST() = 0;
	virtual float execute(const class astInputAPI_i *in) const = 0;
};

class astAPI_i *AST_ParseExpression(const char *s);

#endif // __SHARED_AST_H__
