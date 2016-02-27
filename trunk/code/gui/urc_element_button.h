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
// urc_element_button.h
#include "urc_element_base.h"

class urcElementButton_c : public urcElementBase_c {
	// NOTE: multiple commands can be separated by ;
	str stuffCommand;
	// command to execute when cursor enters this element
	str hoverCommand;
	// material to use when mouse cursor is inside this element rect
	str hoverMaterial;
public:
	
	virtual bool parseURCProperty(class parser_c &p);
	virtual void renderURCElement();
	virtual bool isClickable() const {
		return true;
	}
	virtual const char *getStuffCommand() const {
		return stuffCommand;
	}
	virtual const char *getHoverCommand() const {
		return hoverCommand;
	}
};