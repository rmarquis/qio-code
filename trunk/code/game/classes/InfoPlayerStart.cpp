/*
============================================================================
Copyright (C) 2012 V.

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
// InfoPlayerStart.cpp - player spawnpoint
#include "InfoPlayerStart.h"

DEFINE_CLASS(InfoPlayerStart, "BaseEntity");
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_start);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_intermission);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_red);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_blue);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_deathmatch);
// MoHAA-specific spawnpoints
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_axis);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_allied);
// NOTE: this is confusing... https://developer.valvesoftware.com/wiki/Info_player_allies
// CSS spawnpoints:
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_counterterrorist);
DEFINE_CLASS_ALIAS(InfoPlayerStart, info_player_terrorist);






