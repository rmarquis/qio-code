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
// str.h - ASCII string class

#ifndef __SHARED_STR_H__
#define __SHARED_STR_H__

#include "typedefs.h"
#include <stdlib.h> // malloc, free
#include <string.h> // strcpy, strcat
#include <ctype.h> // isdigit
#include "../qcommon/q_shared.h"
#include "array.h"

// returns true if a given char is whitespace
inline bool G_isWS(char c) {
	return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}
// returns pointer to the extension of file path (first character afer '.')
inline const char* G_strgetExt(const char *str) {
	// -2, since we're assuming that '.' is not a last char
	// l != 1 because there must be at least one character before the '.'
	for(int l = (strlen(str)-2); l > 1; l--) {
		if(str[l] == '.') {
			l++;
			return &str[l];
		}
	}
	return 0;
}

// skip all comments and whitespaces before next token
inline const char *G_SkipToNextToken(const char *p) {
	while(*p) {
		if(p[0] == '/') {
			if(p[1] == '*') {
				p += 2; // skip "/*"
				while(1) {
					if(*p == 0) {
						printf("Error: unexpected end of file hit in multi-line comment\n");
						return 0;
					} else if(p[0] == '*' && p[1] == '/') {
						break;
					}
					p++;
				}
				p += 2;  // skip "*/"
			} else if(p[1] == '/') {
				p += 2; // skip "//"
				while(*p != '\n') {
					if(*p == 0) {
						printf("Error: unexpected end of file hit in single-line comment\n");
						return 0;		
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
		return 0;
	return p;
}

class str {
	char *data;
	char buffer[16];
	u32 len;
	u32 allocated;
public:
	str() {
		data = buffer;
		buffer[0] = 0;
		len = 0;
		allocated = 16;
	}
	str(const str &otherStr) {
		if(otherStr.len + 1 >= 16) {
			allocated = otherStr.len + 1;
			data = (char*)malloc(allocated);
		} else {
			allocated = 16;
			data = buffer;
		}
		strcpy(data,otherStr.c_str());
		len = otherStr.len;
	}
	str(const char *otherStr) {
		if(otherStr == 0 || otherStr[0] == 0) {
			data = buffer;
			buffer[0] = 0;
			len = 0;
			allocated = 16;
			return;
		}
		u32 otherStrLen = strlen(otherStr);
		if(otherStrLen + 1 >= 16) {
			allocated = otherStrLen + 1;
			data = (char*)malloc(allocated);
		} else {
			allocated = 16;
			data = buffer;
		}
		strcpy(data,otherStr);
		len = otherStrLen;
	}
	~str() {
		if(data != buffer) {
			free(data);
		}
	}
	void freeMemory() {
		if(data != buffer) {
			free(data);
			data = buffer;
			allocated = 16;
		}
		buffer[0] = 0;
		len = 0;
	}
	void ensureAllocated(u32 newSize) {
		newSize += 2;
		if(allocated >= newSize)
			return;
		if(data == buffer) {
			data = (char*)malloc(newSize + 8);
			strcpy(data,buffer);
		} else {
			data = (char*)realloc(data,newSize + 8);
		}
		allocated = newSize + 8;
	}
	void allocEmptyString(u32 stringLen) {
		ensureAllocated(stringLen+1);
		for(u32 i = 0; i < stringLen; i++) {
			data[i] = ' ';
		}
		data[stringLen] = 0;
		this->len = stringLen;
	}
	void append(const char *addToString) {
		if(addToString == 0)
			return;
		if(*addToString == 0)
			return;
		u32 addLen = strlen(addToString);
		this->ensureAllocated(this->len + addLen + 1);
		this->len += addLen;
		strcat(this->data,addToString);
	}
	void append(const char *start, const char *stop) {
		if(start >= stop)
			return;
		u32 addLen = stop - start;
		this->ensureAllocated(this->len + addLen + 1);
		memcpy(this->data+this->len,start,addLen);
		this->len += addLen;
		this->data[this->len] = 0;
	}
	void truncatePrintString(const char *s, u32 truncateLenght = 64) {
		u32 sLen = strlen( s );
		if(sLen <= truncateLenght) {
			*this = s;
			return;
		}
		// add the beginning of the string
		this->setFromTo(s, s+( truncateLenght / 2 ) - 3 );
		// skip the middle
		this->append(" ... ");
		// add the ending of the strng
		this->append(s + sLen - ( truncateLenght / 2 ) + 3 );
	}
	char getLastChar() const {
		if(len == 0)
			return 0;
		return this->data[len-1];
	}
	bool isLastChar(char check) const {
		if(len == 0)
			return false;
		if(this->data[len-1] == check)
			return true;
		return false;
	}
	void appendChar(char ch) {
		char tmp[2];
		tmp[0] = ch;
		tmp[1] = 0;
		this->append(tmp);
	}
	void appendInt(int i) {
		char tmp[32];
		sprintf(tmp,"%i",i);
		this->append(tmp);
	}
	void removeCharBefore(int pos) {
		if(pos < 1)
			return;
		if(pos > len) {
			return;
		}
		for(u32 i = pos-1; i < len; i++) {
			this->data[i] = this->data[i+1];
		}
		this->len--;
		this->data[len] = 0;
	}
	void removeCharacter(char ch) {
		u32 j = 0;
		for(u32 i = 0; i < len; i++) {
			if(data[i] == ch) {
			
			} else {
				data[j] = data[i];
				j++;
			}
		}
		len = j;
		data[j] = 0;
	}
	void insertAt(int pos, char ch) {
		if(pos == len) {
			this->appendChar(ch);
			return;
		} else if(pos > len)
			return;
		this->ensureAllocated(len+2);
		len++;
		for(u32 i = len; i != pos; i--) {
			data[i] = data[i-1];
		}
		data[pos] = ch;
	}
	void makeEmpty() {
		len = 0;
		data[0] = 0;
	}
	void clear() {
		makeEmpty();
	}
	u32 countCharacter(char ch) const {
		u32 r=  0;
		for(u32 i = 0; i < len; i++) {
			if(data[i] == ch)
				r++;
		}
		return r;
	}
	void setFromTo(const char *from, const char *to) {
		if(from >= to) {
			makeEmpty();
			return;
		}
		this->len = to - from;
		this->ensureAllocated(this->len + 1);
		memcpy(this->data,from,this->len);
		this->data[len] = 0;
	}
	u32 length() const {
		return len;
	}
	u32 size() const {
		return len;
	}
	bool capLen(u32 newLen) {
		if(newLen >= len)
			return false; // cant cap to higher len than the current is
		len = newLen;
		data[len] = 0;
		return true; // truncated
	}
	// strips any characters AFTER '/' (or '\')
	void toDir() {
		int l = this->len-1;
		while (l > 0 && (data[l] != '/' && data[l] != '\\')) {
			l--;
		}
		if (l) {
			capLen(l+1);
		}
	}
	void stripLastDir() {
		int l = this->len-2; // -2, because we're ignoring last character that is '/'
		while (l > 0 && (data[l] != '/' && data[l] != '\\')) {
			l--;
		}
		if (l) {
			capLen(l+1);
		}
	}
	str getDir() const {
		str ret = *this;
		ret.toDir();
		return ret;
	}
	str getFName() const {
		int l = this->len-1;
		while (l > 0 && (data[l] != '/' && data[l] != '\\')) {
			l--;
		}
		const char *ret = data + l + 1;
		return ret;
	}
	void stripExtension() {
		int l = this->len-1;
		while (l > 0 && data[l] != '.') {
			l--;
			if (data[l] == '/' || data[l] == ':')
				return;		// no extension
		}
		if (l) {
			capLen(l);
		}
	}
	// strips everything after "::" (including "::")
	bool stripLastSubfile() {
		int l = this->len-1;
		while (l > 1) {
			if(data[l] == ':' && data[l-1] == ':') {
				data[l-1] = 0;
				this->len = l-1;
				return true; // done.
			}
			l--;
		}
		return false; // didnt strip anything
	}
	void setExtension(const char *newExt) {
		stripExtension();
		if(newExt[0] != '.') {
			append(".");
		}
		append(newExt);
	}
	void backSlashesToSlashes() {
		for ( u32 i = 0; i < len; i++ ) {
			if ( data[i] == '\\' )
				data[i] = '/';
		}
	}
	void slashesToBackSlashes() {
		for ( u32 i = 0; i < len; i++ ) {
			if ( data[i] == '/' )
				data[i] = '\\';
		}
	}
	void defaultExtension(const char *defExt) {
		if(getExt())
			return;
		this->setExtension(defExt);
	}
	bool contains(const char *other) const {
		u32 oLen = strlen(other);
		return !Q_stricmpn(this->data,other,oLen);
	}
	bool matchesFilter(const char *strFilter) const {
		u32 filterLen = strlen(strFilter);
		for(u32 i = 0; i < this->len; i++) {
			if(!Q_stricmpn(this->data + i,strFilter,filterLen)) {
				return true;
			}
		}
		return false;
	}
	const char *getExt() const {
		if(len < 3) {
			return 0;
		}
		// -2, since we're assuming that '.' is not a last char
		// l != 1 because there must be at least one character before the '.'
		for(int l = (len-2); l !=  1; l--) {
			if(data[l] == '.') {
				l++;
				return &data[l];
			}
		}
		return 0;
	}
	const char *getLastNonDigitCharacter() {
		if(len == 0)
			return 0;
		const char *p = data + len - 1;
		do {
			if(p == data)
				return 0;
			p--;
		} while(isdigit(*p));
		return p;
	}
	char *getData() {
		return data;
	}
	bool hasExt(const char *extToCheck) const {
		const char *ext = this->getExt();
		if(ext == 0)
			return false;
		if(!_stricmp(ext,extToCheck))
			return true;
		return false;
	}
	bool isNumerical() const {
		const char *p = data;
		bool hasDot = false;
		while(*p) {
			if(isdigit(*p) == false) {
				return false;
			}
			if(*p == '.') {
				if(hasDot)
					return false;
				hasDot = true;
			}
			p++;
		}
		return true;
	}
	const char *getToken(str &out, const char *p = 0) {
		out.makeEmpty();
		if(this->len == 0) {
			return 0;
		}
		if(p == 0) {
			p = this->c_str();
		}
		while(G_isWS(*p)) {
			p++;
		}
		const char *start = p;
		while(G_isWS(*p) == false && *p) {
			p++;
		}
		out.setFromTo(start,p);
		while(G_isWS(*p) && *p) {
			p++;
		}
		if(*p == 0)
			return 0;
		return p;
	}
	bool fromFloatMat(const float *in, u32 numFloats) {
		data[0] = 0;
		len = 0;
		char tmp[64];
		for(u32 i = 0; i < numFloats; i++) {
			if(i != 0) {
				// append a whitespace separating float
				appendChar(' ');
			}
			sprintf(tmp,"%f",in[i]);
			u32 tmpLen = strlen(tmp);
			if(tmpLen == 0) {
				// this should never happen
			}
			char *last = &tmp[tmpLen];
			last--;
			while(last != tmp) {
				if(*last == '.') {
					*last = 0;
					break;
				} else if(*last == '0') {
					*last = 0;
				} else {
					break;
				}
				last--;
			}
			this->append(tmp);
		}
		return false; // no error
	}
	bool fromVec3(const float *in) {
		return fromFloatMat(in,3);
	}
	bool getFloatMat(float *out, const char **p, u32 numFloats) {
		const char *at = *p;
		str tmp;
		for(u32 i = 0; i < numFloats; i++) {
			if(at == 0)
				return true; // error
			at = getToken(tmp,at);
			out[i] = atof(tmp);
		}
		*p = at;
		return false; // no error
	}
	str getToken(const char **p) {
		str tmp;
		*p = this->getToken(tmp,*p);
		return tmp;
	}
	const char *findToken(const char *token, const char *start = 0, bool needWS = true) const {
		u32 tokenLen = strlen(token);
		if(start == 0)
			start = this->data;
		while(*start) {
			if(!Q_stricmpn(start, token, tokenLen) && (needWS==false || G_isWS(start[tokenLen]))) {
				return start;
			}
			start++;
		}
		return 0;
	}
	static const char *isNextToken(const char *at, const char *check, bool needWS = true) {
		// skip current word
		while(G_isWS(*at) == false) {
			if(*at == 0)
				return 0;
			at++;
		}
		again:
		// skip whitespaces
		while(G_isWS(*at) == true) {
			if(*at == 0)
				return 0;
			at++;
		}
		if(at[0] == '/' && at[1] == '/') {
			at += 2;
			while(*at != '\n') {
				if(*at == 0)
					return 0;
				at++;
			}
			goto again;
		}
		u32 checkLen = strlen(check);
		if(!Q_stricmpn(at, check, checkLen) && (needWS == false || G_isWS(at[checkLen]))) {
			return at + checkLen;
		}
		return 0;
	}
	void stripTrailingChar() {
		if(this->len == 0)
			return;
		this->len--;
		this->data[this->len] = 0;
	}
	void stripTrailing(const char *charSet) {
		if(this->len == 0)
			return;
		u32 at = this->len-1;
		u32 setLen = strlen(charSet);
		while(1) {
			u32 i;
			// see if data[at] character is in given charater set
			for(i = 0; i < setLen; i++) {
				if(this->data[at] == charSet[i]) {
					break;
				}
			}
			if(i == setLen) {
				return;
			}
			// strip that character
			this->data[at] = 0;
			len--;
			at--;
		}
	}
	void stripLeading(const char *charSet) {
		u32 setLen = strlen(charSet);
		while(0 < len) {
			u32 i;
			// see if data[at] character is in given charater set
			for(i = 0; i < setLen; i++) {
				if(this->data[0] == charSet[i]) {
					break;
				}
			}
			if(i == setLen) {
				return;
			}
			len--;
			for(u32 j = 0; j < len; j++) {
				data[j] = data[j+1];
			}
			data[len] = 0;
		}
	}
	bool removeLineFromStart() {
		const char *stop = 0;
		u32 i = 0;
		while(i < len) {
			if(data[i] == '\n') {
				stop = data + i;
				break;
			}
			i++;
		}
		if(stop == 0) {
			// no newline characters found
			return true;
		}
		str temp = stop+1;
		this->set(temp);
		return false;
	}
	void tokenize(class arraySTD_c<str> &out) {
		const char *p = this->c_str();
		while(p && *p) {
			out.push_back(getToken(&p));
		}
	}
	void operator = (const str &other) {
		this->len = other.len;
		ensureAllocated(this->len+1);
		strcpy(data,other.c_str());
	}
	void operator = (const char *otherString) {
		if(otherString == 0) {
			this->clear();
			return;
		}
		this->len = strlen(otherString);
		ensureAllocated(this->len+1);
		strcpy(data,otherString);
	}
	bool operator ==(const char *otherString) const {
		return !strcmp(c_str(),otherString);
	}
	void set(const char *otherStr) {
		(*this) = otherStr;
	}
	
	const char *c_str() const {
		return data;
	}
	inline operator const char * () const {
		return data;
	}
};

#endif // __SHARED_STR_H__
