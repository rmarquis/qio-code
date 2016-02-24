/*
============================================================================
Copyright (C) 2016 V.

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
// wolfScript.h - ET/RTCW script parser, used for both .ai and .script files
#include <shared/array.h>
#include <shared/str.h>
#include <shared/hashTableTemplate.h>


class wsBlockStatement_c {
friend class wsScript_c;
	str txt;
public:
	const char *getText() const {
		return txt;
	}
};
class wsScriptBlock_c {
friend class wsScript_c;
	arraySTD_c<wsBlockStatement_c> statements;
public:
	u32 getNumStatements() const {
		return statements.size();
	}
	const wsBlockStatement_c *getStatement(u32 i) const {
		return &statements[i];
	}
};
class wsScriptBlockNamed_c : public wsScriptBlock_c {
friend class wsScript_c;
	str name;
public:
	const char *getName() const {
		return name;
	}
};
class wsEntity_c {
friend class wsScript_c;
	str name;
	wsEntity_c *hashNext;
	arraySTD_c<wsScriptBlockNamed_c*> triggers;
	

public:
	wsEntity_c(const char *nn) {
		name = nn;
	}
	const char *getName() const {
		return name;
	}
	void setHashNext(wsEntity_c *nhn) {
		hashNext = nhn;
	}
	wsEntity_c *getHashNext() const {
		return hashNext;
	}
	const wsScriptBlockNamed_c *findLabel(const char *name) const {
		for(u32 i = 0; i < triggers.size(); i++) {
			if(!stricmp(triggers[i]->getName(),name))
				return triggers[i];
		}
		return 0;
	}
};

class wsScript_c {
	hashTableTemplateExt_c<wsEntity_c> entities;

	bool parseScriptBlock(wsScriptBlock_c *o, class parser_c &p);
	bool parseEntity(class parser_c &p, const char *entityName);
public:
	const wsEntity_c *findEntity(const char *name) const;
	
	bool loadScriptFile(const char *fname);
};
