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
// iFaceMgrIMPL.h - shared interface manager implementation

#include <api/iFaceMgrAPI.h>
#include <shared/array.h>

struct iFaceUser_s {
	iFaceBase_i **ptr;
	qioModule_e module;
	const char *iFaceName;
};
struct iFaceProvider_s {
	iFaceBase_i *iFace;
	qioModule_e module;
	const char *iFaceName;
};

class iFaceMgrIMPL_c : public iFaceMgrAPI_i {
public:
	arraySTD_c<iFaceUser_s> users;
	arraySTD_c<iFaceProvider_s> providers;

	// helpers
	void nullUsersOf(iFaceBase_i *iFace) {
		for(int i = 0; i < users.size(); i++) {
			iFaceUser_s &u = users[i];
			if((*u.ptr) == iFace) {
				(*u.ptr) = 0;	
			}
		}
	}

	// providers
	virtual void registerInterface(iFaceBase_i *iFace, const char *iFaceName, qioModule_e module) {
		iFaceProvider_s &np = providers.pushBack();
		np.iFace = iFace;
		np.iFaceName = iFaceName;
		np.module = module;
		// link users
		for(int i = 0; i < users.size(); i++) {
			iFaceUser_s &u = users[i];
			if(!_stricmp(u.iFaceName,iFaceName)) {
				(*u.ptr) = iFace;	
			}
		}
	}
	virtual void unregisterModuleInterfaces(qioModule_e module) {
		// remove interfaces provided from given module,
		// and NULL it's users
		for(int i = 0; i < providers.size(); i++) {
			iFaceProvider_s &p = providers[i];
			if(p.module == module) {
				nullUsersOf(p.iFace);
				providers.erase(i);
				i--;
			}
		}
		// remove interface users that are in given module
		for(int i = 0; i < users.size(); i++) {
			iFaceUser_s &u = users[i];
			if(u.module == module) {
				(*u.ptr) = 0;	
				users.erase(i);
				i--;
			}
		}
	}
	// users
	virtual void registerIFaceUser(iFaceBase_i **iFaceUser, const char *iFaceName, qioModule_e module)  {
		iFaceUser_s &nu = users.pushBack();
		nu.ptr = iFaceUser;
		nu.iFaceName = iFaceName;
		nu.module = module;
		// try to link 
		for(int i = 0; i < providers.size(); i++) {
			iFaceProvider_s &p = providers[i];
			if(!_stricmp(p.iFaceName,iFaceName)) {
				(*nu.ptr) = p.iFace;
				return; // done, linked
			}
		}
		// mark as not linked yet (we'll link it later when a registerInterface is called)
		(*nu.ptr) = 0;
	}
};

// the one and only instance of iFaceMgrIMPL_c
static iFaceMgrIMPL_c g_staticIFaceMan;
iFaceMgrAPI_i *g_iFaceMan = &g_staticIFaceMan;


