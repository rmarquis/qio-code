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
// autoCvar.cpp - automatic cvar system implementation.
// This cpp file must be included in EVERY module which is using automatic vars.
#include "autoCvar.h"
#include <api/cvarAPI.h>
#include <shared/cvar.h>

static aCvar_c *module_autoCvars = 0;

aCvar_c::aCvar_c(const char *newName, const char *newDefaultStr, int newCvarFlags) {
	this->name = newName;
	this->valStr = newDefaultStr;
	this->valFloat = atof(this->valStr);
	this->valInt = atoi(this->valStr);
	this->cvarFlags = newCvarFlags;
	this->extraModificationCallback = 0;

	this->nextModuleCvar = module_autoCvars;
	module_autoCvars = this;
}
aCvar_c::~aCvar_c() {

}

void aCvar_c::setExtraModificationCallback(autoCvarModificationCallback_t newModCallback) {
	this->extraModificationCallback = newModCallback;
}
void aCvar_c::setString(const char *newStr) {
	g_cvars->Cvar_Set(this->name,newStr);
}
void aCvar_c::setFloat(float f) {
	char asString[256];
	sprintf(asString,"%f",f);
	g_cvars->Cvar_Set(this->name,asString);
}
void aCvar_c::onCvarModified(const char *newText) {
	this->valStr = newText;
	this->valFloat = atof(this->valStr);
	this->valInt = atoi(this->valStr);
	if(this->extraModificationCallback) {
		this->extraModificationCallback(this);
	}
}
void AUTOCVAR_RegisterAutoCvars() {
	aCvar_c *c = module_autoCvars;
	while(c) {
		c->internalCvar = g_cvars->Cvar_Get(c->name,c->valStr,c->cvarFlags);
		c->onCvarModified(c->internalCvar->string);
		g_cvars->Cvar_AddModificationCallback(c->internalCvar,c);
		c = c->nextModuleCvar;
	}
}
void AUTOCVAR_UnregisterAutoCvars() {
	aCvar_c *c = module_autoCvars;
	while(c) {
		g_cvars->Cvar_RemoveModificationCallback(c->internalCvar,c);
		c = c->nextModuleCvar;
	}
}