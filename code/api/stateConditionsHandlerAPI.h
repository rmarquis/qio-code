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
#ifndef __API_STATECONDITIONSHANDLERAPI_H__
#define __API_STATECONDITIONSHANDLERAPI_H__

enum stConditionType_e {
	CT_NORMAL,
	CT_EDGETRUE,
	CT_EDGEFALSE,
	CT_NEGATE
};

class stateConditionsHandler_i {
public:
	virtual bool hasAnim(const char *animName) const = 0;
	virtual bool isConditionTrue(enum stConditionType_e conditionType, const char *conditionName, const class stringList_c *arguments, class patternMatcher_c *patternMatcher = 0) = 0;
};
#endif // __API_STATECONDITIONSHANDLERAPI_H__
