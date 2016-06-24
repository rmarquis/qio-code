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
#ifndef __SHARED_FIELD_H__
#define __SHARED_FIELD_H__

#define	MAX_EDIT_LINE	256

struct field_s {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];

private:
	void variableSizeDraw(int x, int y, int width, int size, bool showCursor, bool noColorEscape);
public:
	void clear();
	void paste();
	void draw(int x, int y, int width, bool showCursor, bool noColorEscape);

	void keyDownEvent(int key);
	void charEvent(int ch);
	
	int getMaxSize() const {
		return sizeof(buffer);	
	}
	int *getCursorPtr() {
		return &cursor;
	}
};

#endif // __SHARED_FIELD_H__

