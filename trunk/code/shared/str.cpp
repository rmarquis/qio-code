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
// str.cpp
#include "str.h"

#include <stdarg.h>

const char *va(const char *fmt, ...) {
	va_list args;
	// for nested-function calls
	static char string[2][32000];
	static int which = 0;
	char *selected = string[which];
	which = !which;

	va_start (args, fmt);
	vsnprintf (selected, sizeof(*string), fmt, args);
	va_end (args);

	return selected;
}

// V: this will treat '/' and '\' as equal
int stricmpn_slashes(const char *s1, const char *s2, int n) {
	int		c1, c2;

	if (s1 == NULL) {
		if (s2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (s2 == NULL)
		return 1;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}

		if (c1 != c2) {
			if ((c1 == '/' || c1 == '\\') && (c2 == '/' || c2 == '\\'))
				continue; // OK, slashes
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);

	return 0;		// strings are equal
}


const char *STR_SkipCurlyBracedSection(const char *p) {
	u32 level = 1;
	while (level) {
		p = G_SkipToNextToken(p);
		if (p == 0 || *p == 0)
			return 0; // EOF hit
		if (*p == '{')
			level++;
		else if (*p == '}')
			level--;
		p++;
	}
	return p;
}