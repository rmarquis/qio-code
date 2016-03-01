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
// urc_element_base.h
#ifndef __URC_ELEMENT_BASE_H__
#define __URC_ELEMENT_BASE_H__

#include <shared/rect.h>
#include <shared/str.h>

class urcElementBase_c {
protected:
	// internal element name, never showed on screen
	str name;
	// 2d element coordinates
	rect_c rect;
	// default material name to use
	str matName;
	// related cvar
	str linkCvar;
	// if true, the cvar value is used instead of matname
	int linkCvarToMat;

	// called after element is parsed succesfully
	virtual void onURCElementParsed() { }
public:
	urcElementBase_c();

	bool parseURCElement(class parser_c &p);

	void translate(int dX, int dY);

	virtual bool parseURCProperty(class parser_c &p);


	virtual bool isClickable() const {
		return false;
	}
	virtual bool isField() const {
		return false;
	}
	virtual bool isCheckBox() const {
		return false;
	}
	virtual bool isPullDown() const {
		return false;
	}
	virtual bool isSlider() const {
		return false;
	}
	const rect_c &getRect() const {
		return rect;
	}
	const char *getName() const {
		return name;
	}
	const char *getMatName() const {
		return matName;
	}
	virtual const char *getStuffCommand() const {
		return "";
	}
	virtual const char *getHoverCommand() const {
		return "";
	}
	virtual void renderURCElement(class urcMgr_c *pMgr) = 0;
};

#endif // __URC_ELEMENT_BASE_H__
