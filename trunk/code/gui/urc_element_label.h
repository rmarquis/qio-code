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
// urc_element_label.h
#include "urc_element_base.h"
#include <shared/ePairsList.h>
// this is used when displaying a CVAR value with label,
// so you can link an integer to label, so URC label
// linked to "rf_shadows" with value "1" can display
// "Stencil shadows" instead of "1".
//struct linkString_s {
//	str cvarValue; // eg. "1"
//	str displayValue; // eg. "Stencil shadows."
//};

class urcElementLabel_c : public urcElementBase_c {
	str title;
	//arraySTD_c<linkString_s> linkStrings;
	ePairList_c linkStrings;
public:
	
	virtual bool parseURCProperty(class parser_c &p);
	virtual void renderURCElement(class urcMgr_c *pMgr);

	
	virtual bool isLabel() const {
		return true;
	}
};