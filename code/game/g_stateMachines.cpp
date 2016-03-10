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
#include "stateMachineManager.h"
#include "stateMachine.h"

stateMachineManager_c *g_smm = 0;

void G_InitStateMachines() {
	g_smm = new stateMachineManager_c();
	stateMachineAPI_i *julieLegs = g_smm->registerStateMachine("global/julie_legs.st");
	if(julieLegs) {
	}
	stateMachineAPI_i *aldieLegs = g_smm->registerStateMachine("global/aldie_legs.st");

	// AI states
	stateMachineAPI_i *jart = g_smm->registerStateMachine("ai/jart.st");
	stateMachineAPI_i *shgliek = g_smm->registerStateMachine("ai/shgliek.st");
}
stateMachineAPI_i *G_LoadStateMachine(const char *fname) {
	return g_smm->registerStateMachine(fname);
}