/*
============================================================================
Copyright (C) 2016 V.

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
// urc.h
#include <shared/str.h>

// simple mapping between .urc file name and internal gui name
class urcNameMapping_c {
protected:
	str fname;
	str name;

public:
	void setFName(const char *s) {
		fname = s;
	}
	void setName(const char *s) {
		name = s;
	}
	const char *getName() const {
		return name;
	}
	const char *getFName() const {
		return fname;
	}
};


class urc_c : public urcNameMapping_c {
	urc_c *hashNext;
	arraySTD_c<class urcElementBase_c*> elements;
public:
	void setHashNext(urc_c *h) {
		hashNext = h;
	}
	urc_c *getHashNext() {
		return hashNext;
	}

	void drawURC();
	bool loadURCFile();
	void onKeyDown(int keyCode);
	void onMouseDown(int keyCode, int mouseX, int mouseY);
};
