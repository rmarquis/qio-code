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
// urc_mgr.h - Ultimate ResourCe manager
#include <shared/hashTableTemplate.h>
#include <shared/str.h>
#include "urc.h"

class urcNameMappingCache_c : public urcNameMapping_c {
	urcNameMappingCache_c *hashNext;
public:
	void setHashNext(urcNameMappingCache_c *h) {
		hashNext = h;
	}
	urcNameMappingCache_c *getHashNext() {
		return hashNext;
	}
};
class urcMgr_c {
	hashTableTemplateExt_c<urcNameMappingCache_c> nameCache;
	hashTableTemplateExt_c<urc_c> loaded;
	arraySTD_c<urc_c*> stack;

	class urcElementField_c *activeField;

	void precacheURCFile(const char *fname);
public:
	urcMgr_c();

	void precacheURCFiles();

	void setActiveField(class urcElementField_c *f) {
		activeField = f;
	}

	const char *getURCFileNameForURCInternalName(const char *internalName) const;

	// internal name is not a file name, it's a URC name specified inside URC file
	urc_c *registerURC(const char *internalName);

	void drawURCs();

	void popAllMenus();
	void popMenu();
	void pushMenu(const char *name);
	void onKeyDown(int keyCode);
	void onMouseDown(int keyCode, int mouseX, int mouseY);
	void onMouseMove(int mouseX, int mouseY);
};
