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
// cStylePreprocessor.cpp - C-style preprocessor implementation
#include "cStylePreprocessor.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>

bool cStylePreprocessor_c::preprocessFile(const char *fname) {
	byte *fileData = 0;
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // error
	}
	const char *fileText = (const char*)fileData;
	preprocessText(fileText);
	return false; // OK (ignore parse errors)
}
const char *cStylePreprocessor_c::getToken(str &out) {
	while(G_isWS(*p)) {
		if(*p == 0) {
			return 0; // EOF hit
		}
		p++;
	}
	const char *start = p;
	while(*p) {
		if(G_isWS(*p)) {
			break;
		}
		p++;
	}
	out.setFromTo(start,p);
	return out;
}
bool cStylePreprocessor_c::skipIfBlock() {
	u32 level = 1;
	while(level) {
		if(*p == 0) {
			g_core->RedWarning("cStylePreprocessor_c::skipIfBlock(): unexpected end of file hit\n");
			return true;
		}
		// check for comments
		if(checkComment(true)) {
			continue;
		} else 	if(p[0] == '#') {
			if(!Q_stricmpn(p+1,"define",6) && G_isWS(p[7])) {
				p += 7; // skip "#define"
				continue; // parsed "#define"
			} else if(!Q_stricmpn(p+1,"ifdef",5) && G_isWS(p[6])) {
				p += 6; // skip "#ifdef"
				str token;
				this->getToken(token);
				level++;
				continue; // parsed "#ifdef"
			} else if(!Q_stricmpn(p+1,"endif",5)  && G_isWS(p[6])) {
				p += 6; // skip "#endif"
				level--;
				continue;
			}
		} else {
			p++;
		}
	}
	return false; // no error
}
bool cStylePreprocessor_c::checkComment(bool getText) {
	// check for comments
	if(p[0] == '/') {
		if(p[1] == '/') {
			// that's a single line comment
			if(getText) {
				result.append(start,p);
			}
			p += 2; // skip "//"
			// skip to EOL
			while(*p != '\n' && *p) {
				p++;
			}
			start = p;
			return true; // parsed single line comment
		} else if(p[1] == '*') {
			if(getText) {
				result.append(start,p);
			}
			p += 2; // skip "/*"
			// skip to first "*/"
			while(1) {
				if(*p == 0) {
					g_core->RedWarning("cStylePreprocessor_c::preprocessText: unexpected EOF hit in comment\n");
					break;
				} else if(p[0] == '*' && p[1] == '/') {
					p += 2;
					start = p;
					break;
				} else {
					p++;
				}
			}
			return true; // parsed multi line comment
		}
	}
	return false;
}
bool cStylePreprocessor_c::preprocessText(const char *rawTextData) {
	this->p = rawTextData;
	start = rawTextData;
	while(*p) {
		// check for comments
		if(checkComment(true)) {
			continue;
		} else 
		// check for preprocessor commands
		if(p[0] == '#') {
			if(!Q_stricmpn(p+1,"define",6) && G_isWS(p[7])) {
				result.append(start,p);
				p += 7; // skip "#define"

				continue; // parsed "#define"
			} else if(!Q_stricmpn(p+1,"ifdef",5) && G_isWS(p[6])) {
				result.append(start,p);
				p += 6; // skip "#ifdef"
				str token;
				this->getToken(token);
				if(defines.isDefined(token)) {
					// continue parsing
				} else {
					// skip #ifndef -> #endif block
					skipIfBlock();
				}
				start = p;
				continue; // parsed "#ifdef"
			} else if(!Q_stricmpn(p+1,"endif",5)  && G_isWS(p[6])) {
				result.append(start,p);
				p += 6; // skip "#endif"
				start = p;
				continue; // parsed "#endif"
			}
		}
		p++;
	}
	if(p > start) {
		result.append(start,p);
	}
	return false;
}

