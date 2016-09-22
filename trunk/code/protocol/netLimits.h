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
// netLimits.h
#ifndef _NETLIMITS_H__
#define _NETLIMITS_H__


//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility

#define	GENTITYNUM_BITS		12		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

// NOTE: 9 modelnum bits is not enough for Prey's game/feedingtowera
// (because of very large func_static .proc model count)
#define	MODELNUM_BITS		10		// don't need to send any more
#define	MAX_MODELS			(1<<MODELNUM_BITS)

#define TAGNUM_BITS			8
#define MAX_BONES			(1<<TAGNUM_BITS)

// It can't be just 8 bits,
// models/julie.tik from FAKK has more than 300 animations.
// (NOTE: .tik animation names are not send through config strings)
#define ANIMNUM_BITS		10
#define MAX_ANIMATIONS		(1<<ANIMNUM_BITS)

#define RAGDOLLDEFNUM_BITS	8
#define MAX_RAGDOLLDEFS		(1<<RAGDOLLDEFNUM_BITS)

#define SOUNDNUM_BITS		8
#define MAX_SOUNDS			(1<<SOUNDNUM_BITS)

#define SKINNUM_BITS	8
#define MAX_SKINS		(1<<SKINNUM_BITS)

#define MATERIALNUM_BITS	8
#define MAX_MATERIALS		(1<<MATERIALNUM_BITS)

#define	MAX_CONFIGSTRINGS	8192 // 2048

//#define	MAX_GAMESTATE_CHARS	16000 // I need more to run game/lotaa.map (Prey)
#define MAX_GAMESTATE_CHARS 32768

#endif // _NETLIMITS_H__
