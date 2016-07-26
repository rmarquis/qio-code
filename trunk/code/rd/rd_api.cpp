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
// rd_api.cpp
#include <api/iFaceMgrAPI.h>

#include "demo/Sample.h"
#include "demo/Sample_SoloMesh.h"
#include "demo/MeshLoaderOBJ.h"
#include "demo/InputGeom.h"
//
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/rdAPI.h>
#include <api/cvarAPI.h>
#include <api/clientAPI.h>
#include <shared/autoCvar.h>
//#include <api/materialSystemAPI.h>
#include <shared/autoCmd.h>
//#include <api/mtrAPI.h>
//#include <api/mtrStageAPI.h>
//#include <api/materialSystemAPI.h>

#include <shared/trace.h>
#include <client/keyCodes.h>
#include <math/matrix.h>

static aCvar_c rd_build_cellSize("rd_build_cellSize","0.3f");
static aCvar_c rd_build_cellHeight("rd_build_cellHeight","0.2f");
static aCvar_c rd_build_agentHeight("rd_build_agentHeight","2.0f");
static aCvar_c rd_build_agentRadius("rd_build_agentRadius","0.6f");
static aCvar_c rd_build_agentMaxClimb("rd_build_agentMaxClimb","0.9f");
static aCvar_c rd_build_agentMaxSlope("rd_build_agentMaxSlope","45.0f");
static aCvar_c rd_build_regionMinSize("rd_build_regionMinSize","8");
static aCvar_c rd_build_regionMergeSize("rd_build_regionMergeSize","20");
static aCvar_c rd_build_edgeMaxLen("rd_build_edgeMaxLen","12.0f");
static aCvar_c rd_build_edgeMaxError("rd_build_edgeMaxError","1.3f");
static aCvar_c rd_build_vertsPerPoly("rd_build_vertsPerPoly","6.0f");
static aCvar_c rd_build_detailSampleDist("rd_build_detailSampleDist","6.0f");
static aCvar_c rd_build_detailSampleMaxError("rd_build_detailSampleMaxError","1.0f");
	


//
//class rdWorldBase_c {
//protected:
//	class InputGeom* m_geom;
//	class dtNavMesh* m_navMesh;
//	class dtNavMeshQuery* m_navQuery;
//	class dtCrowd* m_crowd;
//
//	unsigned char m_navMeshDrawFlags;
//
//	float m_cellSize;
//	float m_cellHeight;
//	float m_agentHeight;
//	float m_agentRadius;
//	float m_agentMaxClimb;
//	float m_agentMaxSlope;
//	float m_regionMinSize;
//	float m_regionMergeSize;
//	float m_edgeMaxLen;
//	float m_edgeMaxError;
//	float m_vertsPerPoly;
//	float m_detailSampleDist;
//	float m_detailSampleMaxError;
//	int m_partitionType;
//	
//};
//class rdWorldSimple_c : public rdWorldBase_c {
//
//	void cleanup();
//	void buildRDWorld();
//};
//void rdWorldSimple_c::cleanup() {
//
//}
//void rdWorldSimple_c::buildRDWorld() {
//	if (!m_geom || !m_geom->getMesh())
//	{
//		g_core->RedWarning("buildNavigation: Input mesh is not specified.");
//		return false;
//	}
//	
//	cleanup();
//	
//	const float* bmin = m_geom->getNavMeshBoundsMin();
//	const float* bmax = m_geom->getNavMeshBoundsMax();
//	const float* verts = m_geom->getMesh()->getVerts();
//	const int nverts = m_geom->getMesh()->getVertCount();
//	const int* tris = m_geom->getMesh()->getTris();
//	const int ntris = m_geom->getMesh()->getTriCount();
//
//}
#include <windows.h>
#include <gl/gl.h>
class rdIMPL_c : public rdAPI_i {
	Sample *s;
	BuildContext *ctx;
	float scale;
	float scaleInv;
	matrix_c rd2qio;
	matrix_c qio2rd;

public:
	rdIMPL_c() {
		s = 0;
		ctx = 0;
		scale = 0.05f;
		scaleInv = 1.f / scale;
		rd2qio.setupXRotation(90);
		rd2qio.scale(scaleInv,scaleInv,-scaleInv);
		qio2rd = rd2qio.getInversed();
	}
	virtual void init() {
		AUTOCVAR_RegisterAutoCvars();
		AUTOCMD_RegisterAutoConsoleCommands();
	}
	virtual void doDebugDrawing3D() {
		if(s) {
			glPushMatrix();
			//glRotatef(90,1,0,0);
			//glScalef(scaleInv,scaleInv,-scaleInv);
			glMultMatrixf(rd2qio);
			s->handleRender();
			glPopMatrix();
			
			bool lmb = g_client->Key_IsDown(K_MOUSE1);
			bool rmb = g_client->Key_IsDown(K_MOUSE2);
			if(lmb || rmb) {
				trace_c tr;
				rf->doCameraTrace(tr,true);

				vec3_c sl, hl;
				sl = qio2rd.transformPoint2(tr.getStartPos());
				hl = qio2rd.transformPoint2(tr.getHitPos());
				s->handleClick(sl,hl,rmb);
			}
		}
	}
	virtual void shutdown() {
		AUTOCVAR_UnregisterAutoCvars();
		AUTOCMD_UnregisterAutoConsoleCommands();
	}
	virtual void onRenderWorldMapLoaded() {
		buildNavMesh();
	}
	void buildNavMesh() {
		rcMeshLoaderObj *obj = new rcMeshLoaderObj();
		obj->setScale(scale);
		rf->iterateWorldSolidTriangles(obj);
		if(obj->getTriCount() == 0) {
			delete obj;
			obj = 0;
			return;
		}
		ctx = new BuildContext();
		s = new Sample_SoloMesh();
		s->setAgentHeight(rd_build_agentHeight.getFloat());
		s->setAgentMaxClimb(rd_build_agentMaxClimb.getFloat());
		s->setAgentMaxSlope(rd_build_agentMaxSlope.getFloat());
		s->setAgentRadius(rd_build_agentRadius.getFloat());
		s->setCellHeight(rd_build_cellHeight.getFloat());
		s->setCellSize(rd_build_cellSize.getFloat());
		s->setDetailSampleDist(rd_build_detailSampleDist.getFloat());
		s->setDetailSampleMaxError(rd_build_detailSampleMaxError.getFloat());
		s->setEdgeMaxError(rd_build_edgeMaxError.getFloat());
		s->setEdgeMaxLen(rd_build_edgeMaxLen.getFloat());
		s->setRegionMergeSize(rd_build_regionMergeSize.getFloat());
		s->setRegionMinSize(rd_build_regionMinSize.getFloat());

		s->setContext(ctx);
		InputGeom *ig = new InputGeom();
		ig->fromOBJ(ctx,obj);
		s->handleMeshChanged(ig);
		s->handleBuild();
	}
};


// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rAPI_i *rf = 0;
rbAPI_i *rb = 0;
clAPI_s *g_client = 0;
//declManagerAPI_i *g_declMgr = 0;
// exports
static rdIMPL_c g_staticRDAPI;
rdAPI_i *rd = &g_staticRDAPI;

void RD_SetPointA_f() {
}
void RD_Build_f() {
	g_staticRDAPI.buildNavMesh();
}
static aCmd_c rd_setPointA("rd_setPointA",RD_SetPointA_f);
static aCmd_c rd_build("rd_build",RD_Build_f);

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;
//
//	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)rd,RD_API_IDENTSTR);
//
//	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_client,CLIENT_API_IDENTSTR);
//	g_iFaceMan->registerIFaceUser(&g_declMgr,DECL_MANAGER_API_IDENTSTR);
//
//	g_declMgr->loadAllEntityDecls();
//	g_core->Print("Editor detected %i ent defs\n",g_declMgr->getNumLoadedEntityDecls());
}

qioModule_e IFM_GetCurModule() {
	return QMD_RECASTNAVIGATION;
}

