/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Qio source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qio source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../common/common.h"
#include "client.h"

/*
=================
CL_Netchan_TransmitNextFragment
=================
*/
bool CL_Netchan_TransmitNextFragment(netchan_t *chan)
{
	if(chan->unsentFragments)
	{
		Netchan_TransmitNextFragment(chan);
		return true;
	}
	
	return false;
}

/*
===============
CL_Netchan_Transmit
================
*/
void CL_Netchan_Transmit( netchan_t *chan, msg_c* msg ) {
	msg->writeByte(clc_EOF );

	Netchan_Transmit(chan, msg->cursize, msg->data);
	
	// Transmit all fragments without delay
	while(CL_Netchan_TransmitNextFragment(chan))
	{
		Com_DPrintf("WARNING: #462 unsent fragments (not supposed to happen!)\n");
	}
}

/*
=================
CL_Netchan_Process
=================
*/
bool CL_Netchan_Process( netchan_t *chan, msg_c *msg ) {
	int ret;

	ret = Netchan_Process( chan, msg );
	if (!ret)
		return false;

	return true;
}
