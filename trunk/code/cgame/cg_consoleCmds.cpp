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
// cg_consoleCmds.cpp - cgame console commands
#include <shared/autoCmd.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <shared/rendererSurfaceRef.h>

void CG_SetCrosshairSurfaceMaterial_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("Usage: cg_setCrosshairSurfaceMaterial <material_name>\n");
		return;
	}
	const char *matName = g_core->Argv(1);
	rendererSurfaceRef_s ref;
	rf->getLookatSurfaceInfo(ref);
	// send remap command to server so it can broadcast it to all clients
	g_core->Cbuf_ExecuteText(0,va(";net_setWorldSurfaceMaterial %i %i %s;",ref.areaNum,ref.surfaceNum,matName));
	//rf->setWorldSurfaceMaterial(matName,ref.surfaceNum,ref.areaNum);
}

static aCmd_c cg_setCrosshairSurfaceMaterial_f("cg_setCrosshairSurfaceMaterial",CG_SetCrosshairSurfaceMaterial_f);

