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
// cm_cmds.cpp - console commands for models processing and testing
#include "cm_local.h"
#include <api/coreAPI.h>
#include <shared/str.h>
#include <shared/cmSurface.h>
#include <shared/cmBrush.h>

static void CM_CreateHullForRenderModelInternal(bool createForAABB) {
	str in = g_core->Argv(1);
	str out;
	if(g_core->Argc() > 2) {
		out = g_core->Argv(2);
	} else {
		out = in;
		out.stripExtension();
		out.append("_singleBrushHull.map");
	}
	// 1. load trimesh data from triangle model
	cmSurface_c sf;
	if(CM_LoadRenderModelToSingleSurface(in,sf)) {
		str fixed = "models/";
		fixed.append(in);
		if(CM_LoadRenderModelToSingleSurface(fixed,sf)) {
			g_core->RedWarning("Couldn't load %s\n",in.c_str());
			return;
		}
	}
	// 2. convert trimesh points to single convex hull
	cmBrush_c br;
	if(createForAABB) {
		br.fromBounds(sf.getAABB());
	} else {
		br.fromPoints(sf.getVertsArray());
	}
	// 3. export single hull to .map file
	br.writeSingleBrushToMapFileVersion2(out);
}
static void CM_CreateHullForRenderModel_f() {
	if(g_core->Argc() < 1) {
		g_core->Print("Usage: cm_createHullForRenderModel <render model name> [<output file name>]\n");
		return;
	}
	CM_CreateHullForRenderModelInternal(false);
}
static void CM_CreateHullForRenderModelAABB_f() {
	if(g_core->Argc() < 1) {
		g_core->Print("Usage: cm_createHullForRenderModelAABB <render model name> [<output file name>]\n");
		return;
	}
	CM_CreateHullForRenderModelInternal(true);
}

void CM_AddConsoleCommands() {
	g_core->Cmd_AddCommand("cm_createHullForRenderModel",CM_CreateHullForRenderModel_f);
	g_core->Cmd_AddCommand("cm_createHullForRenderModelAABB",CM_CreateHullForRenderModelAABB_f);
}
void CM_RemoveConsoleCommands() {
	g_core->Cmd_RemoveCommand("cm_createHullForRenderModel");
	g_core->Cmd_RemoveCommand("cm_createHullForRenderModelAABB");
}


