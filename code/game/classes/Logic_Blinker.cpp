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
// Logic_Blinker.cpp
#include "Logic_Blinker.h"
#include "../g_local.h"

DEFINE_CLASS(Logic_Blinker, "BaseEntity");
DEFINE_CLASS_ALIAS(Logic_Blinker, qio_logic_blinker);

Logic_Blinker::Logic_Blinker() {
	bBlinkerState = false;
	wait_enabled = 1000;
	wait_disabled = 1000;
	lastSwitchTime = g_time;
}
void Logic_Blinker::runFrame() {
	u32 timePassed = g_time - lastSwitchTime;
	u32 cycleTime = bBlinkerState ? wait_disabled : wait_enabled;
	while(timePassed > cycleTime) {
		lastSwitchTime += cycleTime;
		if(bBlinkerState) {
			bBlinkerState = false;
			//if(target) {
				G_HideEntitiesWithTargetName(getTarget());
			//}
		} else {
			bBlinkerState = true;
			//if(target) {
				G_ShowEntitiesWithTargetName(getTarget());
			//}
		}
		timePassed = g_time - lastSwitchTime;
		cycleTime = bBlinkerState ? wait_disabled : wait_enabled;
	}
}
void Logic_Blinker::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"wait_enabled")) {
		wait_enabled = atoi(value);
	} else if(!_stricmp(key,"wait_disabled")) {
		wait_disabled = atoi(value);
	} else {
		BaseEntity::setKeyValue(key,value);
	}
}
