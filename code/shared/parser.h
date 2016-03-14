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
#include <api/coreAPI.h>

class parser_c {
	const char *base; // start of the text
	const char *p; // current position in the text
	char *fileData; // alloced by filesystem in parser_c::openFile
	str debugFileName;
	str lastToken;

	static bool isCharInCharset(const char *set, char c) {
		if(set == 0)
			return false;
		while(*set) {
			if(*set == c)
				return true;
			set++;
		}
		return false;
	}
public:
	parser_c();
	~parser_c();
	bool openFile(const char *fname);
	void setup(const char *newText, const char *newP = 0);
	void clear();
	void setDebugFileName(const char *newDebugFileName);

	// retusn true if next token is inside a pair of "
	bool isNextTokenInQuotationMarks();

	// returns true if eof is reached
	bool skipToNextToken();
	bool tryToGetNextToken(const char *stopSet = 0) {
		getToken(this->lastToken, stopSet);
		if(this->lastToken.size())
			return false;
		return true;
	}	
	const char *getToken(str &out, const char *stopSet = 0, bool bAllowStringEscaping = false);
	const char *getToken(const char *stopSet = 0) {
		return getToken(this->lastToken, stopSet);
	}	
	const char *getLine(str &out, const char *stopSet = 0);
	const char *getLine(const char *stopSet = 0) {
		return getLine(this->lastToken,stopSet);
	}
	const char *getLastStoredToken() const {
		return lastToken;
	}
	// allows token to be separated by ','
	const char *getD3Token(); // (q3 CommaParse equivalent)
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
	// numbers might be separatede by ','
	bool getFloatMatD3(float *out, u32 dims) {
		const char *s;
		for(u32 i = 0; i < dims; i++) {
			s = getD3Token();
			out[i] = atof(s);
		}
		return false;
	}
	// eg "( 0, 0, 1, 128 )"
	bool getFloatMatD3_braced(float *out, u32 dims) {
		if(atWord_dontNeedWS("(") == false)
			return true; // error
		const char *s;
		for(u32 i = 0; i < dims; i++) {
			s = getD3Token();
			out[i] = atof(s);
		}
		if(atWord_dontNeedWS(")") == false)
			return true; // error
		// skip optional (?) ','
		if(atWord_dontNeedWS(",")) {

		}
		return false; // OK
	}
	bool atChar(char ch);
	bool atWord(const char *word);
	bool atWord_dontNeedWS(const char *word);
	int getD3Integer() {
		const char *w = getD3Token();
		if(w == 0)
			return 0;
		return atoi(w);
	}
	float getD3Float() {
		const char *w = getD3Token();
		if(w == 0)
			return 0;
		return atof(w);
	}
	const char *getNextWordInLine() {
		// see if we're at the end of the line
		while(1) {
			if(*p == '\n') {
				return 0;
			} else if(p[0] == '/' && p[1] == '/') {
				return 0;
			} else if(p[0] == '/' && p[1] == '*') {
				p+=2;
				while((p[0] == '*' && p[1] == '/') == false) {
					if(*p == 0) {
						return 0;
					}
					p++;
				}
			} else if(G_isWS(*p)==false) {
				break;
			} else {
				p++;
			}
		}
		if(*p == 0)
			return 0;
		getToken();
		return lastToken;
	}
	void skipLine();
	void skipBytes(u32 numBytes) {
		p += numBytes;
	}
	void readBytes(void *out, u32 numBytes) {
		memcpy(out,p,numBytes);
		p += numBytes;
	}

	const char *getDebugFileName() const {
		return debugFileName;
	}
	u32 getCurrentLineNumber() const;

	inline bool atEOF() const {
		const char *tmp = p;
		while(*tmp) {
			if(tmp[0] == '/') {
				if(tmp[1] == '/') {
					// skip single line comment
					tmp += 2;
					while(*tmp != '\n') {
						if(*tmp == 0) {
							return true; // EOF hit
						}
						tmp++;
					}
				} else if(tmp[1] == '*') {
					tmp += 2;
					while(1) {
						if(*tmp == 0) {
							return true; // EOF hit
						} else if(tmp[0] == '*' && tmp[1] == '/') {
							tmp += 2;
							if(*tmp == 0)
								return true; // is at eof
							break;
						} else {
							tmp++;
						}
					}
				}
			}
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
				// skip comments
				if(tmp[0] == '/' && tmp[1] == '/')
					return true; // single line comment is a end of line
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
	bool getBracedBlock() {
		skipToNextToken();
		if(*p != '(') {
			return true; // error
		}
		const char *start = p;
		p++;
		int level = 1;
		lastToken.clear();
		while(level) {
			if(*p == '(') {
				level++;
			} else if(*p == ')') {
				level--;
			} else if(*p == 0) {
				g_core->RedWarning("parser_c::getBracedBlock: unexpected end of file\n");
				return true; // error
			} else if(p[0] == '/') {
				if(p[1] == '*') {
					str tmp;
					tmp.setFromTo(start,p);
					lastToken.append(tmp);
					// skip multi-line comment
					p+=2;
					while(1) {
						if(*p == 0) {
							g_core->RedWarning("parser_c::getBracedBlock: unexpected end of file found in multiline comment\n");
							return true; // error
						} else if(p[0] == '*' && p[1] == '/') {
							p+=2;
							break;
						}
					}
					start = p;
					continue;
				} else if(p[1] == '/') {
					str tmp;
					tmp.setFromTo(start,p);
					lastToken.append(tmp);
					// skip single-line comment
					p+=2;
					while(*p != '\n') {
						if(*p == 0) {
							g_core->RedWarning("parser_c::getBracedBlock: unexpected end of file found in comment\n");
							return true; // error
						}
						p++;
					}
					start = p;
					continue;
				}
			}
			p++;
		}
		if(start != p) {
			str tmp;
			tmp.setFromTo(start,p);
			lastToken.append(tmp);
		}
		return false; // OK
	}
	const char *getCurDataPtr() const {
		return p;
	}
	void setCurDataPtr(const char *newP) {
		p = newP;
	}
};


#endif // __SHARED_PARSER_H__

