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
// flags32.h - utility class for handling bit flags
#ifndef __FLAGS32_H__
#define __FLAGS32_H__

#include "typedefs.h"

class flags32_c {
	u32 value;
public:
	flags32_c() {
		value = 0;
	}
	u32 getValue() const {
		return value;
	}
	void setValue(const u32 newVal) {
		value = newVal;
	}
	void setFlag(u32 newFlag) {
		value |= newFlag;
	}
	void clearFlag(u32 flagToClear) {
		value &= ~flagToClear;
	}
	bool isFlag(u32 flagToCheck) const {
		return value & flagToCheck;
	}
	void setFlag(u32 flag, bool on) {
		if(on) {
			value |= flag;
		} else {
			value &= ~flag;
		}
	}
};

#endif // __FLAGS32_H__
