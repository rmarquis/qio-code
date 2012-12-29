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
// cStylePreprocessor.h - simple C-style preprocessor,
// with basic comments, #defines and #ifdefs handling
#ifndef __SHARED_CSTYLEPREPROCESSOR_H__
#define __SHARED_CSTYLEPREPROCESSOR_H__

#include <shared/str.h>
#include <shared/array.h>

struct cDefine_s {
	// #define <name> <arguments with spaces>
	str name;
	str arguments;
};

class cStyleDefinesArray_c {
	arraySTD_c<cDefine_s> data;
public:
	void addDefine(const char *defineName, const char *arguments = 0) {
		cDefine_s nd;
		nd.name = defineName;
		if(arguments && arguments[0]) {
			nd.arguments = arguments;
		}
		data.push_back(nd);
	}
	// TODO: use hash table?
	bool isDefined(const char *defineName) const {
		for(u32 i = 0; i < data.size(); i++) {
			if(!Q_stricmp(data[i].name,defineName)) {
				return true;
			}
		}
		return false;
	}
};

class cStylePreprocessor_c {
	str result;
	cStyleDefinesArray_c defines;
	const char *p;
	const char *start; // start of the current token

	const char *getToken(str &out);
	bool checkComment(bool getText);
	bool atToken(const char *token);
	bool skipIfBlock();
public:
	const char *getResultConstChar() const {
		return result;
	}
	const str &getResultConstStr() const {
		return result;
	}
	void addDefine(const char *defineName, const char *arguments = 0) {
		defines.addDefine(defineName,arguments);
	}

	bool preprocessFile(const char *fname);
	bool preprocessText(const char *rawTextData);
};	

#endif // __SHARED_CSTYLEPREPROCESSOR_H__
