#include "g_local.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/serverAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/gameAPI.h>

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
svAPI_s *g_server = 0;
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
// exports
static gameAPI_s g_staticGameAPI;
static gameClientAPI_s g_staticGameClientsAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_staticGameAPI.InitGame = G_InitGame;
	g_staticGameAPI.RunFrame = G_RunFrame;
	g_staticGameAPI.ShutdownGame = G_ShutdownGame;
	g_iFaceMan->registerInterface(&g_staticGameAPI,GAME_API_IDENTSTR);

	g_staticGameClientsAPI.ClientBegin = ClientBegin;
	g_staticGameClientsAPI.ClientCommand = ClientCommand;
	g_staticGameClientsAPI.ClientUserinfoChanged = ClientUserinfoChanged;
	g_staticGameClientsAPI.ClientConnect = ClientConnect;
	g_staticGameClientsAPI.ClientThink = ClientThink;
	g_staticGameClientsAPI.ClientDisconnect = ClientDisconnect;
	g_iFaceMan->registerInterface(&g_staticGameClientsAPI,GAMECLIENTS_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_server,SERVER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_GAME;
}

