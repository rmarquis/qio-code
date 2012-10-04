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
	str debugFileName;
	str lastToken;

	// returns true if eof is reached
	bool skipToNextToken();

public:
	void setup(const char *newText, const char *newP = 0);
	void setDebugFileName(const char *newDebugFileName);
	const char *getToken(str &out);
	const char *getToken() {
		return getToken(this->lastToken);
	}
	bool atChar(char ch);
	bool atWord(const char *word);

	inline bool atEOF() const {
		return *p == 0;
	}


};


#endif // __SHARED_PARSER_H__

