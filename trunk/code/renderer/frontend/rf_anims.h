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
// rf_anims.h - animation loader wrapper for renderer
#ifndef __RF_ANIMS_H__
#define __RF_ANIMS_H__

#include <shared/str.h>

class rfAnimation_c {
	str name;
	class skelAnimAPI_i *api;
	class rfAnimation_c *hashNext;
public:
	const char *getName() const {
		return name;
	}
	rfAnimation_c *getHashNext() const {
		return hashNext;
	}
	void setHashNext(rfAnimation_c *newNext) {
		hashNext = newNext;
	}
	const class skelAnimAPI_i *getAPI() const {
		return api;
	}
	
	void clear();

friend rfAnimation_c *RF_RegisterAnimation(const char *animName);
friend const skelAnimAPI_i *RF_RegisterAnimation_GetAPI(const char *animName);
};

#endif // __RF_ANIMS_H__