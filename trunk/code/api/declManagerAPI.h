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
// declManagerAPI.h
#ifndef __DECLMANAGERAPI_H__
#define __DECLMANAGERAPI_H__

#include "iFaceBase.h"
#include "iFaceMgrAPI.h" // for IFM_GetCurModule

#define DECL_MANAGER_API_IDENTSTR "DeclMgr0001"

class declManagerAPI_i : public iFaceBase_i {
	virtual class modelDeclAPI_i *_registerModelDecl(const char *name, qioModule_e userModule) = 0;
	virtual class entityDeclAPI_i *_registerEntityDecl(const char *name, qioModule_e userModule) = 0;
	virtual class afDeclAPI_i *_registerAFDecl(const char *name, qioModule_e userModule) = 0;
	virtual class q3PlayerModelAPI_i *_registerQ3PlayerDecl(const char *name, qioModule_e userModule) = 0;
	virtual class particleDeclAPI_i *_registerParticleDecl(const char *name, qioModule_e userModule) = 0;
public:
	virtual void init() = 0;
	// NOTE: those functions must be inlined, otherwise IFM_GetCurModule() trick wouldnt work
	inline class modelDeclAPI_i *registerModelDecl(const char *name) {
		// NOTE: IFM_GetCurModule must be implemented in each and every Qio module!
		qioModule_e userModule = IFM_GetCurModule();
		return _registerModelDecl(name,userModule);
	}
	inline class entityDeclAPI_i *registerEntityDecl(const char *name) {
		// NOTE: IFM_GetCurModule must be implemented in each and every Qio module!
		qioModule_e userModule = IFM_GetCurModule();
		return _registerEntityDecl(name,userModule);
	}
	inline class afDeclAPI_i *registerAFDecl(const char *name) {
		// NOTE: IFM_GetCurModule must be implemented in each and every Qio module!
		qioModule_e userModule = IFM_GetCurModule();
		return _registerAFDecl(name,userModule);
	}
	inline class q3PlayerModelAPI_i *registerQ3PlayerDecl(const char *name) {
		// NOTE: IFM_GetCurModule must be implemented in each and every Qio module!
		qioModule_e userModule = IFM_GetCurModule();
		return _registerQ3PlayerDecl(name,userModule);
	}
	inline class particleDeclAPI_i *registerParticleDecl(const char *name) {
		// NOTE: IFM_GetCurModule must be implemented in each and every Qio module!
		qioModule_e userModule = IFM_GetCurModule();
		return _registerParticleDecl(name,userModule);
	}
	// clear up unused decls
	virtual void onGameShutdown() = 0;
	virtual void onRendererShutdown() = 0;
	// used for console command autocompletion
	virtual void iterateEntityDefNames(void (*callback)(const char *s)) = 0;
};

extern declManagerAPI_i *g_declMgr;

#endif // __DECLMANAGERAPI_H__
