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
// g_configStrings.cpp
#include <protocol/configStrings.h>
#include <api/serverAPI.h>

u32 G_RenderModelIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_MODELS, MAX_MODELS);
}
u32 G_CollisionModelIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_COLLMODELS, MAX_MODELS);
}
u32 G_SoundIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_SOUNDS, MAX_SOUNDS);
}
u32 G_AnimationIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_ANIMATIONS, MAX_ANIMATIONS);
}
u32 G_RagdollDefIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_RAGDOLLDEFSS, MAX_RAGDOLLDEFS);
}
u32 G_RenderSkinIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_SKINS, MAX_SKINS);
}
u32 G_RenderMaterialIndex( const char *name ) {
	return g_server->RegisterConfigString(name, CS_MATERIALS, MAX_MATERIALS);
}

