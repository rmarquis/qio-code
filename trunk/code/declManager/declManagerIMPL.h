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
#include "articulatedFigure.h"

// simple string list.
// performs much faster than arraySTD_c<str>.
class stringList_c {
	arraySTD_c<char*> list;
public:
	~stringList_c() {
		freeMemory();
	}
	void freeMemory() {
		for(u32 i = 0; i < list.size(); i++) {
			free(list[i]);
		}
		list.clear();
	}
	void iterateEntityDefNames(void (*callback)(const char *s)) {
		for(u32 i = 0; i < list.size(); i++) {
			callback(list[i]);
		}
	}
	void addString(const char *s) {
		u32 l = strlen(s);
		char *n = (char*)malloc(l+1);
		strcpy(n,s);
		list.push_back(n);
	}
	static int CompareStringQSort(const void *v0, const void *v1) {
		const char *s0 = *((const char **)v0);
		const char *s1 = *((const char **)v1);
		return stricmp(s0,s1);
	}
	void sortStrings() {
		qsort(list.getArray(),list.size(),list.getElementSize(),CompareStringQSort);
	}
};

struct defFile_s {
	str fname;
	str text;
};

class fileTextDataCache_c {
	// raw .def files text
	arraySTD_c<defFile_s*> defFiles;
	u32 totalDefBytes;
public:
	void cacheDefFileText(const char *fname);
	const char *findDeclInText(const char *declName, const char *declType, const char *text);
	bool findDeclText(const char *declName, const char *declType, struct declTextDef_s &out);
	u32 cacheFileList(const char *path, const char *ext);
	void listDeclNames(class stringList_c &out, const char *declType) const;

	u32 getTotalTextSizeInBytes() const {
		return totalDefBytes;
	}
	u32 getNumFiles() const {
		return defFiles.size();
	}
};

class modelDecl_c;
class entityDecl_c;
class q3PlayerModelDecl_c;

class declManagerIMPL_c : public declManagerAPI_i {
	// file text cached
	fileTextDataCache_c defFiles;
	fileTextDataCache_c afFiles;

	// parsed structures
	hashTableTemplateExt_c<modelDecl_c> modelDecls;
	hashTableTemplateExt_c<entityDecl_c> entityDecls;
	hashTableTemplateExt_c<afDecl_c> afDecls;
	hashTableTemplateExt_c<q3PlayerModelDecl_c> q3PlayerDecls;

	// precached list of entityDef names
	// used for console command autocompletion
	bool entDefNamesListReady;
	stringList_c entDefNamesList;

	void cacheEntDefNamesList();

	virtual void init();
	virtual class modelDeclAPI_i *_registerModelDecl(const char *name, qioModule_e userModule);
	virtual class entityDeclAPI_i *_registerEntityDecl(const char *name, qioModule_e userModule);
	virtual class afDeclAPI_i *_registerAFDecl(const char *name, qioModule_e userModule);
	virtual class q3PlayerModelAPI_i *_registerQ3PlayerDecl(const char *name, qioModule_e userModule);

	void removeUnrefrencedDecls();
	virtual void onGameShutdown();
	virtual void onRendererShutdown();
	virtual void iterateEntityDefNames(void (*callback)(const char *s));
};

