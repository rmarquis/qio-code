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
// tableList.cpp - tables manager for Doom3 .mtr tables
#include <math.h> // floor
#include "tableList.h"
#include "parser.h"

bool table_c::parse(const char *at, const char *textBase, const char *sourceFileName) {
	parser_c p;
	p.setup(textBase,at);
	if(p.atWord_dontNeedWS("{") == false) {
		return true;
	}
	bSnap = false;
	while(p.atWord_dontNeedWS("{") == false) {
		const char *parm = p.getToken();
		if(!_stricmp(parm,"snap")) {
			bSnap = true;
		} else {
			g_core->RedWarning("table_c::parse: table %s from file %s has unknown modifier '%s'.\n",
				this->getName(),sourceFileName,parm);
		}
	}
	while(p.atWord_dontNeedWS("}") == false) {
		float val = p.getD3Float();
		values.push_back(val);
	}
	if(p.atWord_dontNeedWS("}") == false) {
		return true;
	}
	return false; // no error
}
float table_c::getValue(float idx) const {
	if(values.size() == 0.f)
		return 0.f;
	if(bSnap) {
		u32 iIndex = idx;
		iIndex = iIndex % values.size();
		return values[iIndex];
	}
	u32 prev = floor(idx);
	u32 next = prev + 1;
	float frac = (idx - prev) / 1.f;
	prev = prev % values.size();
	next = next % values.size();
	float val0 = values[prev];
	float val1 = values[next];
	return val0 + frac * (val1 - val0);
}

tableList_c::tableList_c(TABLE_FindTableTextFunc_t newFindTextFunc) {
	findTextFunc = newFindTextFunc;
}
tableList_c::~tableList_c() {
	for(u32 i = 0; i < tables.size(); i++) {
		delete tables[i];
	}
	tables.clear();
}

float tableList_c::getTableValue(const char *tableName, float index) const {
	table_c *table = tables.getEntry(tableName);
	if(table == 0) {
		// create a new table entry
		table = new table_c;
		table->setName(tableName);
		if(findTextFunc) {
			// try to parse it
			const char *p, *textBase, *sourceFileName;
			if(findTextFunc(tableName,&p,&textBase,&sourceFileName)) {
				table->parse(p,textBase,sourceFileName);
			}
		}
		// add it to hashtable
		tables.addObject(table);
	}
	return table->getValue(index);
}


