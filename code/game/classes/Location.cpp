/*
============================================================================
Copyright (C) 2014 V.

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
// Location.cpp
#include "Location.h"
#include <shared/entityType.h>

// idLocationEntity (info_location)
// Used to assign a string name to specific .proc area.
DEFINE_CLASS(Location, "BaseEntity");
DEFINE_CLASS_ALIAS(Location, idLocationEntity);

Location::Location() {
	this->setEntityType(ET_INFO_LOCATION);
}
void Location::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"name")) {
		locationName = value;
	} else {
		BaseEntity::setKeyValue(key,value);		
	}
}
