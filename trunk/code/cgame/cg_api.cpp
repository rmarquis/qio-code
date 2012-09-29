#include "cg_local.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/clientAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/cgameAPI.h>

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
clAPI_s *g_client = 0;
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
// exports
static cgameAPI_s g_staticCGameAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_staticCGameAPI.Init = CG_Init;
	g_staticCGameAPI.Shutdown = CG_Shutdown;
	g_staticCGameAPI.DrawActiveFrame = CG_DrawActiveFrame;
	g_iFaceMan->registerInterface(&g_staticCGameAPI,CGAME_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_client,CLIENT_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_CGAME;
}

