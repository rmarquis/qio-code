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
// iFaceMgrAPI.h - shared interface manager API

#ifndef __IFACEMGRAPI_H__
#define __IFACEMGRAPI_H__

class iFaceBase_i;

enum qioModule_e {
	QMD_CORE,
	QMD_GAME,
	QMD_CGAME,
};

class iFaceMgrAPI_i {
public:
	//
	//	INTERFACE PROVIDERS
	//
	// this should be called for every interface of interface provider dll when its loaded
	virtual void registerInterface(iFaceBase_i *iFace, const char *iFaceName, qioModule_e module) = 0;
	// this MUST be called before interface provider DLL is unloaded, so all of the external
	// interface pointers are properly NULLed
	virtual void unregisterModuleInterfaces(qioModule_e module) = 0;
	//
	//	INTERFACE USERS
	//
	// this should be called once at module startup. iFaceMgr will automatically set the iFaceUser ptr
	// as soon as given API is avaible (registered trough registerInterface)
	virtual void registerIFaceUser(iFaceBase_i **iFaceUser, const char *iFaceName, qioModule_e module) = 0;
};


extern iFaceMgrAPI_i *g_iFaceMan;


#endif // __IFACEMGRAPI_H__
