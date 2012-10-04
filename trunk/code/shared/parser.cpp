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
#include "parser.h"

void parser_c::setup(const char *newText, const char *newP) {
	base = newText;
	if(newP == 0) {
		p = base;
	} else {
		p = newP;
	}
}
void parser_c::setDebugFileName(const char *newDebugFileName) {
	this->debugFileName = newDebugFileName;
}
bool parser_c::skipToNextToken() {
	while(*p) {
		if(p[0] == '/') {
			if(p[1] == '*') {
				p += 2; // skip "/*"
				while(1) {
					if(*p == 0) {
						printf("Error: unexpected end of file hit in multi-line comment\n");
						return true; // EOF
					} else if(p[0] == '*' && p[1] == '/') {
						break;
					}
				}
				p += 2;  // skip "*/"
			} else if(p[1] == '/') {
				p += 2; // skip "//"
				while(*p != '\n') {
					if(*p == 0) {
						printf("Error: unexpected end of file hit in single-line comment\n");
						return true; // EOF
					}
					p++;
				}
				p++; // skip '\n'
			} else {
				break;
			}
		} else if(G_isWS(*p) == false) {
			break;
		} else {
			p++;
		}
	}
	if(*p == 0)
		return true; // EOF
	return false;
}

const char *parser_c::getToken(str &out) {
	if(skipToNextToken()) {
		printf("parser_c::getToken: EOF reached\n");
		out.clear();
		return 0;
	}
	const char *start, *end;
	if(*p == '"') {
		p++;
		start = p;
		while(*p != '"') {
			if(*p == 0) {
				printf("parser_c::getToken: unexpected end of file hit in quoted string\n");
				break;
			}
		}
		end = p;
		if(*p != 0) {
			p++;
		}
	} else {
		start = p;
		while(G_isWS(*p) == false) {
			p++;
		}
		end = p;
	}
	out.setFromTo(start,end);
	return out;
}
bool parser_c::atChar(char ch) {
	if(skipToNextToken()) {
		printf("parser_c::atChar: EOF reached\n");
		return false;
	}
	if(*p == ch) {
		p++;
		return true;
	}
	return false;
}
bool parser_c::atWord(const char *word) {
	if(skipToNextToken()) {
		printf("parser_c::atChar: EOF reached\n");
		return false;
	}
	u32 checkLen = strlen(word);
	if(!Q_stricmpn(word,p,checkLen) && G_isWS(p[checkLen])) {
		p += checkLen;
		return true;
	}
	return false;
}
