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
// TargetScriptTrigger.h
#ifndef __TARGETSCRIPTTRIGGER_H__
#define __TARGETSCRIPTTRIGGER_H__

#include "BaseEntity.h"

class TargetScriptTrigger : public BaseEntity {
	// name of script that will be called. "trigger [target]"
	str target; 
	// name of script block where to look for target
	str scriptName;
public:
	TargetScriptTrigger();

	DECLARE_CLASS( TargetScriptTrigger );

	virtual void triggerBy(class BaseEntity *from, class BaseEntity *activator);

	virtual void postSpawn();

	virtual void setKeyValue(const char *key, const char *value);
};

#endif // __TARGETSCRIPTTRIGGER_H__
