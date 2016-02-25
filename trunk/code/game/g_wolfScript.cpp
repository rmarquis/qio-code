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
#include "g_local.h"
#include "classes/BaseEntity.h"
#include <shared/wolfScript.h>
#include <api/coreAPI.h>

wsScript_c g_script;

void G_InitWolfScripts(const char *mapName) {
	str fixed = "maps/";
	fixed.append(mapName);
	fixed.setExtension("script");
	if(g_script.loadScriptFile(fixed)) {
		return;
	}
	g_core->Print("G_InitWolfScripts: succesfully loaded Wolf script file %s\n",fixed.c_str());

}
/*
This will start a scriptblock with specified label on all entities with the scriptName.
Example:

my_mover
{
	trigger myLabel
	{
		print "Hello world"
		wait 100	
		gotomarker my_marker 10 wait
		print "I have arrived!"
		wait 100	
		gotomarker other_marker 10 wait
		print "Move finished!"
	}
}

This script can be started from within a code by using:

G_WolfScript_StartScript("my_mover","myLabel");

It will be started on every entity which has scriptName set to "my_mover".

*/
void G_WolfScript_StartScript(const char *scriptName, const char *labelName) {
	const wsEntity_c *e = g_script.findEntity(scriptName);
	if(e == 0) {
		g_core->RedWarning("G_WolfScript_StartScript: script %s not found (maybe wrong scriptName?)\n",scriptName);
		return;
	}
	const wsScriptBlock_c *b = e->findLabel(labelName);
	if(b == 0) {
		g_core->RedWarning("G_WolfScript_StartScript: script label %s not found in %s\n",labelName,scriptName);
		return;
	}
	arraySTD_c<BaseEntity *> ents;
	G_GetEntitiesWithScriptName(scriptName,ents);
	for(u32 i = 0; i < ents.size(); i++) {
		BaseEntity *e = ents[i];
		e->startWolfScript(b);

	//	break;
	}
}
void G_WolfScript_StartScript_Spawn(const char *scriptName) {
	const wsEntity_c *e = g_script.findEntity(scriptName);
	if(e == 0) {
		g_core->RedWarning("G_WolfScript_StartScript_Spawn: script %s not found (maybe wrong scriptName?)\n",scriptName);
		return;
	}
	const wsScriptBlock_c *b = e->getSpawnScript();
	if(b == 0) {
		return;
	}
	arraySTD_c<BaseEntity *> ents;
	G_GetEntitiesWithScriptName(scriptName,ents);
	for(u32 i = 0; i < ents.size(); i++) {
		BaseEntity *e = ents[i];
		e->startWolfScript(b);

		//break;
	}
}