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
// declManagerIMPL.h
#include <api/declManagerAPI.h>
#include <shared/str.h>
#include <shared/array.h>
#include <shared/hashTableTemplate.h>

struct defFile_s {
	str fname;
	str text;
};

class modelDecl_c;
class entityDecl_c;

class declManagerIMPL_c : public declManagerAPI_i {
	// raw .def files text
	arraySTD_c<defFile_s*> defFiles;
	u32 totalDefBytes;
	// parsed structures
	hashTableTemplateExt_c<modelDecl_c> modelDecls;
	hashTableTemplateExt_c<entityDecl_c> entityDecls;

	void cacheDefFileText(const char *fname);
	const char *findDeclInText(const char *declName, const char *declType, const char *text);
	bool findDeclText(const char *declName, const char *declType, struct declTextDef_s &out);

	virtual void init();
	virtual class modelDeclAPI_i *_registerModelDecl(const char *name, qioModule_e userModule);
	virtual class entityDeclAPI_i *_registerEntityDecl(const char *name, qioModule_e userModule);

	void removeUnrefrencedDecls();
	virtual void onGameShutdown();
	virtual void onRendererShutdown();
};

