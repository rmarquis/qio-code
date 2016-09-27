/*
============================================================================
Copyright (C) 2012-2016 V.

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
#include "playerState.h"
#include "userCmd.h"
#include <shared/entityType.h>

// This is done after each set of userCmd_s on the server,
// and after local prediction on the client
void playerState_s::toEntityState(entityState_s *s, bool snap) const {

	//if ( this->pm_type == PM_INTERMISSION || this->pm_type == PM_SPECTATOR ) {
	//	s->eType = ET_INVISIBLE;
	//} else if ( this->stats[STAT_HEALTH] <= GIB_HEALTH ) {
	//	s->eType = ET_INVISIBLE;
	//} else 
	{
		s->eType = ET_PLAYER;
	}

	s->number = this->clientNum;

	s->origin = this->origin;
	s->scale = this->scale;

	// copy angles (NOT viewangles)
	s->angles = this->angles;

	s->animIndex = this->animIndex;
	s->torsoAnim = this->torsoAnim;
	s->parentNum = this->parentNum;
	s->parentTagNum = this->parentTagNum;
	s->eFlags = this->eFlags;

	// set the trDelta for flag direction
	//VectorCopy( this->velocity, s->pos.trDelta );


	//s->apos.trType = TR_INTERPOLATE;
	//VectorCopy( this->viewangles, s->apos.trBase );
	//if ( snap ) {
	//	SnapVector( s->apos.trBase );
	//}

//	s->angles2[YAW] = this->movementDir;

////	s->clientNum = this->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config

	s->rModelIndex = this->rModelIndex;
	s->rSkinIndex = this->rSkinIndex;
	s->colModelIndex = this->colModelIndex;
	s->groundEntityNum = this->groundEntityNum;
}


// This can be used as another entry point when only the viewangles
// are being updated isntead of a full move
void playerState_s::updateViewAngles(const userCmd_s *cmd) {
	short		temp;
	int		i;


	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + this->delta_angles[i];
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				this->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				this->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		this->viewangles[i] = SHORT2ANGLE(temp);
	}

}
