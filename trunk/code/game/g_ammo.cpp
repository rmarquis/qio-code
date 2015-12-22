/*
============================================================================
Copyright (C) 2013 V.

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
// g_ammo.cpp - ammo types managment
#include <shared/str.h>
#include <shared/array.h>
#include <shared/autoCmd.h>
#include <api/declManagerAPI.h>
#include <api/entityDeclAPI.h>
#include <api/entDefAPI.h>
#include <api/coreAPI.h>

class ammoDef_c {
	str name; // eg. "ammo_grenades"
	str printName; // eg. "Hand Grenades"
public:
	void setName(const char *nn) {
		name = nn;
	}
	void setPrintName(const char *npn) {
		printName = npn;
	}
	bool hasName(const char *n) const {
		return !_stricmp(n,name);
	}
	const char *getName() const {
		return name;
	}
	const char *getPrintName() const {
		return printName;
	}
};

static arraySTD_c<ammoDef_c> g_ammoTypes;

// name is an internal ammo name, eg "ammo_grenade"
void G_SetAmmoTypeName(int index, const char *name) {
	if(index < 0)
		return;
	if(index > 64) {
		return;
	}
	if(g_ammoTypes.size() <= index)
		g_ammoTypes.resize(index+1);
	g_ammoTypes[index].setName(name);
}
int G_AmmoIndexForName(const char *name) {
	for(u32 i = 0; i < g_ammoTypes.size(); i++) {
		if(g_ammoTypes[i].hasName(name))
			return i;
	}
	return -1;
}
// name is an internal ammo name, eg. "ammo_grenades"
// printName is an ammo label that is printed to console, eg. "Hand Grenades"
void G_SetAmmoTypePrintName(const char *name, const char *printName) {
	int index = G_AmmoIndexForName(name);
	if(index < 0) {

		return;
	}
	g_ammoTypes[index].setPrintName(printName);
}

// Called at game module startup
// Loads ammoTypes from Doom3 .def files
// (New ammoTypes can be added at runtime as well)
void G_InitAmmoTypes() {
	if(g_declMgr == 0) {
		g_core->RedWarning("G_InitAmmoTypes: cannot init ammo types because decl manager is not present.\n");
		return;
	}
	entityDeclAPI_i *ammo_types = g_declMgr->registerEntityDecl("ammo_types");
	entityDeclAPI_i *ammo_names = g_declMgr->registerEntityDecl("ammo_names");
	if(ammo_types == 0) {
		g_core->RedWarning("G_InitAmmoTypes: no 'ammo_types' entityDef found\n");
		return;
	}
	if(ammo_names == 0) {
		g_core->RedWarning("G_InitAmmoTypes: no 'ammo_names' entityDef found\n");
		return;
	}
	// set internal ammo names
	const entDefAPI_i *eAmmoTypes = ammo_types->getEntDefAPI();
	for(u32 i = 0; i < eAmmoTypes->getNumKeyValues(); i++) {
		const char *k, *v;
		eAmmoTypes->getKeyValue(i,&k,&v);
		int index = atoi(v);
		G_SetAmmoTypeName(index,k);
	}
	// set print ammo names
	const entDefAPI_i *eAmmoNames = ammo_names->getEntDefAPI();
	for(u32 i = 0; i < eAmmoNames->getNumKeyValues(); i++) {
		const char *k, *v;
		eAmmoNames->getKeyValue(i,&k,&v);
		G_SetAmmoTypePrintName(k,v);
	}
}

static void G_PrintGameAmmoTypesList_f() {
	for(u32 i = 0; i < g_ammoTypes.size(); i++) {
		g_core->Print("%i - %s (%s)\n",i,g_ammoTypes[i].getName(),g_ammoTypes[i].getPrintName());
	}	
}

static aCmd_c g_printGameAmmoTypesList("g_printGameAmmoTypesList",G_PrintGameAmmoTypesList_f);
