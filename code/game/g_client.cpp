/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "g_local.h"
#include <api/serverAPI.h>
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <api/declManagerAPI.h>
#include <math/vec3.h>
#include "classes/ModelEntity.h"
#include "classes/FakePlayer.h"
#include "classes/VehicleCar.h"
#include <shared/colorTable.h>
#include <shared/infoString.h>

// g_client.c -- client functions that don't happen every frame

//======================================================================

/*
================
ClientRespawn
================
*/
void ClientRespawn( edict_s *ent ) {


	ClientSpawn(ent);
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	Player *pl = dynamic_cast<Player*>(g_entities[clientNum].ent);

	char buf[MAX_INFO_STRING];
	g_server->GetUserinfo(clientNum,buf,sizeof(buf));
	const char *s = Info_ValueForKey(buf, "name");
	if(s == 0) {
		pl->setNetName("UnnamedPlayer");
	} else {
		pl->setNetName(s);
	}
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be true the very first time a client connects
to the server machine, but false on map changes and tournement
restarts.
============
*/
const char *ClientConnect( int clientNum, bool firstTime, bool isBot ) {
	Player *pl;
	edict_s	*ent;

	ent = &g_entities[ clientNum ];

	if(ent->ent) {
		G_Printf(S_COLOR_YELLOW"ClientBegin: freeing old player class\n");
		delete ent->ent;
	}
	// create a player class for given edict
	BE_SetForcedEdict(ent);
	ent->ent = pl = new Player;

	// they can connect
	pl->pers.connected = CON_CONNECTING;

	// get and distribute relevent paramters
	G_Printf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		g_server->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", pl->getNetName()) );
	}

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	edict_s	*ent;

	ent = g_entities + clientNum;

	if(ent->ent == 0) {
		// this should never happen
		g_core->DropError("ClientBegin on edict without entity!");
		return;
	}

	Player *pl = dynamic_cast<Player*>(ent->ent);

	pl->pers.connected = CON_CONNECTED;
	pl->pers.enterTime = level.time;

	// locate ent at a spawn point
	ClientSpawn( ent );

	g_server->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", pl->getNetName()) );

	G_Printf( "ClientBegin: %i\n", clientNum );

}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(edict_s *ent) {
	int		index;

	index = ent - g_entities;

	if(ent->ent == 0) {
		// this should never happen
		g_core->DropError("ClientSpawn on edict without entity!");
		return;
	}
	Player *pl = dynamic_cast<Player*>(ent->ent);
	pl->ps.clientNum = index;

	pl->setHealth(100);

	vec3_c	spawnOrigin, spawnAngles;
	spawnOrigin.zero();
	spawnAngles.zero();

	BaseEntity *spawnPoint = G_GetRandomEntityOfClass("InfoPlayerStart");
	if(spawnPoint) {
		spawnOrigin = spawnPoint->getOrigin();
		spawnAngles = spawnPoint->getAngles();
	}
	pl->setOrigin(spawnOrigin);
	pl->setAngles(spawnAngles);
	g_server->GetUsercmd( index, &pl->pers.cmd );
	pl->setClientViewAngle(spawnAngles);


	// don't allow full run speed for a bit

#if 1
	pl->setPlayerModel("models/player/shina/body.md5mesh");
#else
	// load q3 player model (three .md3's)
	pl->setPlayerModel("$sarge");
#endif

	// run a pl frame to drop exactly to the floor,
	// initialize animations and other things
	pl->ps.commandTime = level.time - 100;
	pl->pers.cmd.serverTime = level.time;
	ClientThink( index );
	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	//BG_PlayerStateToEntityState( &pl->ps, &ent->s, true );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	edict_s	*ent;

	ent = g_entities + clientNum;
	if (!ent->ent) {
		return;
	}
	//// ||
	//Player *pl = dynamic_cast<Player*>(ent->ent);
	//if(pl->pers.connected == CON_DISCONNECTED) {
	//	return;
	//}

	if(ent->ent) {
		delete ent->ent;
		ent->ent = 0;
	}

//ent->classname = "disconnected";
	//ent->client->pers.connected = CON_DISCONNECTED;

#ifdef CS_PLAYERS
	g_server->SetConfigstring( CS_PLAYERS + clientNum, "");
#endif
}
ModelEntity *G_SpawnRandomBoxAt(const vec3_c &pos, const vec3_c &rot) {
	int r = rand() % 4;
	ModelEntity *e = new ModelEntity;
	if(r == 0) {
		e->setRenderModel("models/props/crates/crate1.obj");
		e->setColModel("models/props/crates/crate1.map");
	} else if(r == 1) {
		e->setRenderModel("models/props/crates/crate2.obj");
		e->setColModel("models/props/crates/crate2.map");
	} else if(r == 2) {
		e->setRenderModel("models/props/crates/crate3.obj");
		e->setColModel("models/props/crates/crate3.map");
	} else {
		e->setRenderModel("models/props/crates/crate4.obj");
		e->setColModel("models/props/crates/crate4.map");
	}
	e->setOrigin(pos);
	e->setAngles(rot);
	e->initRigidBodyPhysics();
	return e;
}
bool FixRenderModelPath(str &s) {
	if(g_vfs->FS_FileExists(s))
		return true; // path was valid
	for(u32 i = 0; i < 2; i++) {
		str tmp;
		if(i == 0) {
			tmp = s;
		} else {
			tmp = "models/";
			tmp.append(s);
		}
		if(g_vfs->FS_FileExists(tmp)) {
			s = tmp;
			return true; // path fixed
		}
		// known render model extensions
		static const char *extensions[] = {
			"md3",
			"md5mesh",
			"lwo",
			"ase",
			"obj",
			"map",
			"mdlpp",
		};
		static u32 numExtensions = sizeof(extensions)/sizeof(extensions[0]);
		for(u32 j = 0; j < numExtensions; j++) {
			const char *ext = extensions[i];
			tmp.setExtension(ext);
			if(g_vfs->FS_FileExists(tmp)) {
				s = tmp;
				return true; // path fixed
			}
		}
	}
	// invalid path
	return false;
}
void ClientCommand( int clientNum ) {
	Player *pl = (Player*)g_entities[clientNum].ent;
	if(pl == 0) {
		g_core->Print("ClientCommand: client %i is NULL\n",clientNum);
		return;
	}

	const char *cmd = g_core->Argv(0);
	if(!_stricmp(cmd,"noclip")) {
		pl->toggleNoclip();
	} else if(!_stricmp(cmd,"shootbox")) {
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		G_SpawnRandomBoxAt(p,pl->getAngles());
	} else if(!_stricmp(cmd,"createboxstack")) {
		vec3_c p = pl->getOrigin();
		//p.z += float(pl->getViewHeight())*0.5;
		p.z += 32;
		p += pl->getForward() * 64.f;
		vec3_c up = pl->getUp();
		vec3_c left = pl->getLeft();
		float boxSize = 32.f;
		vec3_c angles = pl->getAngles();
		// bottom boxes
		G_SpawnRandomBoxAt(p+left*boxSize,angles);
		G_SpawnRandomBoxAt(p,angles);
		G_SpawnRandomBoxAt(p-left*boxSize,angles);
		// middle boxes
		G_SpawnRandomBoxAt(p+left*boxSize*0.5+up*boxSize,angles);
		G_SpawnRandomBoxAt(p-left*boxSize*0.5+up*boxSize,angles);
		// top box
		G_SpawnRandomBoxAt(p+up*boxSize*2.f,angles);
	} else if(!_stricmp(cmd,"createkubelwagen") || !_stricmp(cmd,"createporshe")) {
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 128.f;
		vec3_c angles = pl->getAngles();
		//angles.y += 90;

		VehicleCar *veh = new VehicleCar;
		veh->setOrigin(p);
		veh->setAngles(angles);
		if(!_stricmp(cmd,"createkubelwagen")) {
			veh->setRenderModel("models/vehicles/kubeldakwre/kubeldakwre.obj");
			veh->setColModel("models/vehicles/kubeldakwre/kubeldakwre.map");
		} else {
			veh->setRenderModel("models/vehicles/Porsche_911/porsche-911.obj");
			veh->setColModel("models/vehicles/Porsche_911/porsche-911.map");
		}
		veh->spawnPhysicsVehicle();
	} else if(!_stricmp(cmd,"createbarrel")) {
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setRenderModel("models/props/barrel_c/barrel_c.obj");
		e->setColModel("models/props/barrel_c/barrel_c.map");
		e->setOrigin(p);
		//e->setAngles(rot);
		e->initRigidBodyPhysics();
	} else if(!_stricmp(cmd,"spawn")) {
		str model = g_core->Argv(1);
		if(model.length()) {
			vec3_c spawnPos = pl->getOrigin();
			spawnPos.z += pl->getViewHeight();
			spawnPos += pl->getForward() * 64.f;
			if(g_vfs->FS_ReadFile(model,0) < 1) {
				str fixed = "models/";
				fixed.append(model);
				if(g_vfs->FS_ReadFile(model,0) < 1) {
					// if "model" string is not a file name,
					// it might a entityDef name from .def files (Doom3 declarations)
					if(g_declMgr->registerEntityDecl(model)) {
						BaseEntity *be = G_SpawnEntityFromEntDecl(model);
						if(be) {
							be->setOrigin(spawnPos);
						//	be->postSpawn();
						} else {
							g_core->Print("Failed to spawn %s\n",model.c_str());
						}
					} else {						
						// maybe the model is the classname itself
						BaseEntity *bent = G_SpawnClass(model);
						if(bent) {
							bent->setOrigin(spawnPos);
							bent->postSpawn();
						} else {
							g_core->Print("%s does not exist\n",model.c_str());
						}
					}
					return;
				}
				model = fixed;
			}
			const char *ext = G_strgetExt(model);
			if(ext && !_stricmp(ext,"entDef")) {
				BaseEntity *be = G_SpawnFirstEntDefFromFile(model);
				if(be == 0) {
					g_core->Print("Failed to spawn %s\n",model.c_str());
				} else {
					be->setOrigin(spawnPos);
				}
			} else {
				str collisionModelName = model;
				collisionModelName.setExtension("map");
				ModelEntity *me = new ModelEntity;
				me->setRenderModel(model);
				me->setColModel(collisionModelName);
				me->setOrigin(spawnPos);
				me->initRigidBodyPhysics();
			}
		}
	} else if(!_stricmp(cmd,"spawnStatic")) {
		str model = g_core->Argv(1);
		if(model.length()) {
			if(g_vfs->FS_ReadFile(model,0) < 1) {
				str fixed = "models/";
				fixed.append(model);
				if(g_vfs->FS_ReadFile(model,0) < 1) {
					g_core->Print("%s does not exist\n",model.c_str());
					return;
				}
				model = fixed;
			}
			str collisionModelName = model;
			collisionModelName.setExtension("map");
			vec3_c p = pl->getOrigin();
			p.z += pl->getViewHeight();
			p += pl->getForward() * 64.f;
			ModelEntity *e = new ModelEntity;
			e->setRenderModel(model);
			if(e->setColModel(collisionModelName) == true) {
				// .map file collision model not present,
				// try to load triangles directly from render model file
				e->setColModel(model);
			}
			e->setOrigin(p);
			e->initStaticBodyPhysics();
		}
	} else if(!_stricmp(cmd,"spawntestbtragdoll")) {
		//vec3_c p = pl->getOrigin();
		//p.z += pl->getViewHeight();
		//p += pl->getForward() * 64.f;
		//BT_TestSpawnInternalRagDoll(p);
	} else if(!_stricmp(cmd,"spawntestaf_old")) {
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		G_SpawnTestRagdollFromAF(afName,p,vec3_c(0,0,0));
	} else if(!_stricmp(cmd,"spawntestaf")) {
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		if(e->initRagdollRenderAndPhysicsObject(afName)) {
			delete e; // failed to initialzie ragdoll
		}
	} else if(!_stricmp(cmd,"quicksave")) {
		// well, this should be a server command and NOT a client command, but I'll leave it here for testing
		G_SaveCurrentSceneToMapFile("maps/saves/quicksave.map");
	} else if(!_stricmp(cmd,"tele")) {
		if(g_core->Argc() >= 3) {
			vec3_c newPos;
			newPos.x = atof(g_core->Argv(1));
			newPos.y = atof(g_core->Argv(2));
			newPos.z = atof(g_core->Argv(3));
			pl->setOrigin(newPos);
		}
	} else if(!_stricmp(cmd,"forcePlayerModelChange")) {
		str newPlayerModel = g_core->Argv(1);
		pl->setPlayerModel(newPlayerModel);
	} else if(!_stricmp(cmd,"kill")) {
		pl->onDeath();
	} else if(!_stricmp(cmd,"addbot")) {
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;

		edict_s *newEdict = G_Spawn();
		BE_SetForcedEdict(newEdict);
		FakePlayer *np = new FakePlayer;
		np->setOrigin(p);
		// see if we can use sarge player model from Quake3
		if(g_vfs->FS_FileExists("models/players/sarge/animation.cfg")) {
			np->setPlayerModel("$sarge");
		} else {
			np->setPlayerModel("models/player/shina/body.md5mesh");
		}
		np->enableCharacterController();
		np->setHealth(100);
		np->setLeader(pl);
	} else if(!_stricmp(cmd,"removentitiesofclass")) {
		str className = g_core->Argv(1);
		G_RemoveEntitiesOfClass(className);
	} else if(!_stricmp(cmd,"model_spawn") || !_stricmp(cmd,"mdlpp_spawn") || !_stricmp(cmd,"mdl_spawn") || !_stricmp(cmd,"psk_spawn") || !_stricmp(cmd,"mdm_spawn") || !_stricmp(cmd,"smd_spawn")) {
		str model = g_core->Argv(1);
		if(model.length()) {
			if(model[0] == '_' || g_declMgr->registerModelDecl(model) || FixRenderModelPath(model)) {
				vec3_c p = pl->getOrigin();
				p.z += pl->getViewHeight();
				p += pl->getForward() * 64.f;
				ModelEntity *e = new ModelEntity;
				e->setRenderModel(model);
				//e->setColModel(model);
				e->setOrigin(p);
			} else {
				g_core->RedWarning("%s is not a valid model file\n",model.c_str());
			}
		}
	} else if(!_stricmp(cmd,"physics_spawn")) {
		str model = g_core->Argv(1);
		if(model.length()) {
			if(model[0] == '_' || g_declMgr->registerModelDecl(model) || FixRenderModelPath(model)) {
				vec3_c p = pl->getOrigin();
				p.z += pl->getViewHeight();
				p += pl->getForward() * 64.f;
				ModelEntity *e = new ModelEntity;
				e->setRenderModel(model);
				e->setColModel(model);
				e->initRigidBodyPhysics();
				e->setOrigin(p);
			} else {
				g_core->RedWarning("%s is not a valid model file\n",model.c_str());
			}
		}
	} else if(!_stricmp(cmd,"say")) {
		char buff[8192];
		g_core->Args(buff,sizeof(buff));
		pl->cmdSay(buff);
	} else if(!_stricmp(cmd,"removeLighs")) {
		G_RemoveEntitiesOfClass("light");
	} else if(!_stricmp(cmd,"mdm_test1")) {
		// V: simple ET skeletal model test (model will show deformed without animation)
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/axis/temperate/cvops/body.mdm");
	} else if(!_stricmp(cmd,"mdm_test2")) {
		// V: simple ET skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/temperate/axis/cvops/body.mdm");
		e->setAnimation("animations/human/base/cqb_rifle_aimed.mdx");
	} else if(!_stricmp(cmd,"mdm_test3")) {
		// V: simple ET skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("body.mdm");
		e->setAnimation("akimbo.mdx");
	} else if(!_stricmp(cmd,"mdm_test4")) {
		// V: simple ET skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/temperate/axis/cvops/body.mdm");
		e->setAnimation("animations/human/base/body.mdx");
		// V: skin file is: models/players/temperate/axis/cvops/body_cvops.skin
		// but we use only "cvops" because the path is set automatically
		e->setRenderModelSkin("cvops");
	} else if(!_stricmp(cmd,"net_setWorldSurfaceMaterial")) {
		// command used to debug (display) various materials on world surfaces
		int areaNum = atoi(g_core->Argv(1));
		int surfaceNum = atoi(g_core->Argv(2));
		const char *matName = g_core->Argv(3);
		// broadcast world material change to all the clients
		g_server->SendServerCommand(-1,va("stufftext rf_setWorldSurfaceMaterial %i %i %s",areaNum,surfaceNum,matName));
	} else if(!_stricmp(cmd,"mds_test1")) {
		// V: simple RTCW skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/loper/body.mds");
		e->setAnimation("models/players/loper/body.mds");
		e->setRenderModelAttachment(0,"tag_head","models/players/loper/head.mdc");
		e->setRenderModelSkin("default");
	} else if(!_stricmp(cmd,"mds_test2")) {
		// V: simple RTCW skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/venom/body.mds");
		e->setAnimation("models/players/venom/body.mds");
		e->setRenderModelSkin("default");
	} else if(!_stricmp(cmd,"mds_test3")) {
		// V: simple RTCW skeletal model test
		// NOTE: mdm models has no bone data in file
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/players/infantryss/body.mds");
		e->setAnimation("models/players/infantryss/body.mds");
		e->setRenderModelAttachment(0,"tag_head","models/players/infantryss/head.mdc");
		e->setRenderModelSkin("default");
	} else if(!_stricmp(cmd,"smd_test1")) {
		str afName = g_core->Argv(1);
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 64.f;
		ModelEntity *e = new ModelEntity;
		e->setOrigin(p);
		e->setRenderModel("models/tests/smd_anim/Male_06_reference.smd");
		e->setAnimation("models/tests/smd_anim/MP5_RunN.smd");
	} else {
		g_core->RedWarning("Unknown client command %s\n",cmd);
		////vec3_c tmp(1400,1340,470);
		//////BT_CreateVehicle(tmp);
		////BT_CreateBoxEntity(tmp + vec3_c(0,0,128),vec3_c(16,16,16),0);
	}
}

playerState_s dummy;
struct playerState_s *ClientGetPlayerState(u32 clientNum) {
	edict_s *ed = &g_entities[clientNum];
	if(ed->s == 0) {
		g_core->Print(S_COLOR_RED"ClientGetPlayerState: edict %i isnt activeb (has NULL state)\n",clientNum);
		memset(&dummy,0,sizeof(dummy));
		return &dummy;
	}
	Player *pl = dynamic_cast<Player*>(ed->ent);
	if(pl == 0) {
		g_core->Print(S_COLOR_RED"ClientGetPlayerState: can't get Player class of edict %i\n",clientNum);
		memset(&dummy,0,sizeof(dummy));
		return &dummy;
	}
	return pl->getPlayerState();
}

