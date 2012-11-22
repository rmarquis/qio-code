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
// parser.h - simple parser class
#ifndef __SHARED_PARSER_H__
#define __SHARED_PARSER_H__

#include "typedefs.h"
#include "str.h"

class parser_c {
	const char *base; // start of the text
	const char *p; // current position in the text
	char *fileData; // alloced by filesystem in parser_c::openFile
	str debugFileName;
	str lastToken;

	// returns true if eof is reached
	bool skipToNextToken();

public:
	parser_c();
	~parser_c();
	bool openFile(const char *fname);
	void setup(const char *newText, const char *newP = 0);
	void clear();
	void setDebugFileName(const char *newDebugFileName);
	const char *getToken(str &out);
	const char *getToken() {
		return getToken(this->lastToken);
	}	
	const char *getLine(str &out);
	const char *getLine() {
		return getLine(this->lastToken);
	}
	float getFloat();
	int getInteger();
	// eg "0 0 0 128"
	bool getFloatMat(float *out, u32 dims) {
		const char *s;
		for(u32 i = 0; i < dims; i++) {
			s = getToken();
			//if(isNotANumber(s))
			//return true; // error
			out[i] = atof(s);
		}
		return false; // OK
	}
	// eg "( 0 0 1 128 )"
	bool getFloatMat_braced(float *out, u32 dims) {
		if(atWord("(") == false)
			return true; // error
		const char *s;
		for(u32 i = 0; i < dims; i++) {
			s = getToken();
			out[i] = atof(s);
		}
		if(atWord(")") == false)
			return true; // error
		return false; // OK
	}
	bool getFloatMat2D_braced(int y, int x, float *m) {
		if(atWord("(") == false)
			return true; // error
		for(u32 i = 0; i < y; i++ ) {
			if (getFloatMat_braced(m + i * x, x)) {
				return true;
			}
		}
		if(atWord(")") == false)
			return true; // error
		return false; // OK
	}
	bool atChar(char ch);
	bool atWord(const char *word);
	bool atWord_dontNeedWS(const char *word);

	void skipLine();

	const char *getDebugFileName() const {
		return debugFileName;
	}
	u32 getCurrentLineNumber() const;

	inline bool atEOF() const {
		const char *tmp = p;
		while(*tmp) {
			if(G_isWS(*tmp) == false) {
				return false;
			}
			tmp++;
		}
		return true;
	}
	inline bool isAtEOL() const {
		const char *tmp = p;
		while(*tmp) {
			if(*tmp == '\n') {
				return true;
			}
			if(G_isWS(*tmp) == false) {
				return false;
			}
			tmp++;
		}
		return false;
	}
	bool skipCurlyBracedBlock(bool needFirstBrace = true) {
		skipToNextToken();
		if(*p == 0)
			return true;
		if(needFirstBrace) {
			if(*p != '{') {
				return true;
			}
		}
		int level = 1;
		p++;
		while(*p) {
			if(*p == '{') {
				level++;
			} else if(*p == '}') {
				level--;
				if(level == 0) {
					p++;
					return false; // OK
				}
			}
			p++;
		}
		return true; // unexpected end of file hit
	}


};


#endif // __SHARED_PARSER_H__

