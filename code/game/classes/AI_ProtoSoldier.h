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
// AI_ProtoSoldier.h
#ifndef __AI_PROTOSOLDIER_H__
#define __AI_PROTOSOLDIER_H__

#include "AI_RTCW_Base.h"

class AI_ProtoSoldier : public AI_RTCW_Base {
public:
	DECLARE_CLASS( AI_ProtoSoldier );

	AI_ProtoSoldier();

	virtual bool isProtoSoldier() const {
		return true;
	}
};

#endif // __AI_PROTOSOLDIER_H__
