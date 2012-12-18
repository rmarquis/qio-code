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
// g_bullet_debugDraw.cpp - implementation of btIDebugDraw class
#include "g_local.h"
#include "bt_include.h"
#include <api/ddAPI.h>
#include <shared/autoCvar.h>
#include "physics_scale.h"

aCvar_c btd_drawWireFrame("btd_drawWireFrame","0");
aCvar_c btd_drawAABB("btd_drawAABB","0");
aCvar_c btd_disableDebugDraw("btd_disableDebugDraw","0");

static class rDebugDrawer_i *g_dd = 0;

class qioBulletDebugDraw_c : public btIDebugDraw {
	virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color) {
		g_dd->drawLineFromTo(from*BULLET_TO_QIO,to*BULLET_TO_QIO,color);
	}
	virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)  {

	}
	virtual void	reportErrorWarning(const char* warningString) {

	}
	virtual void	draw3dText(const btVector3& location,const char* textString) {

	}
	virtual void	setDebugMode(int debugMode) {

	}
	virtual int		getDebugMode() const {
		int flags = 0;
		if(btd_drawWireFrame.getInt()) {
			flags |= DBG_DrawWireframe;
		}
		if(btd_drawAABB.getInt()) {
			flags |= DBG_DrawAabb;
		}
		return flags;
	}

};

qioBulletDebugDraw_c g_bulletDebugDrawer;

void G_DoBulletDebugDrawing(class rDebugDrawer_i *dd) {
	if(btd_disableDebugDraw.getInt()) {
		return; // no debug drawing at all
	}
	g_dd = dd;
	dynamicsWorld->setDebugDrawer(&g_bulletDebugDrawer);
	dynamicsWorld->debugDrawWorld();
}

