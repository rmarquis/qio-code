#ifndef __CM_LOCAL_H__
#define __CM_LOCAL_H__

#include <shared/typedefs.h>

// cm_model.cpp
class cMod_i *CM_FindModelInternal(const char *name);
class cmCapsule_i *CM_RegisterCapsule(float height, float radius);
class cmBBExts_i *CM_RegisterBoxExts(float halfSizeX, float halfSizeY, float halfSizeZ);
class cmHull_i *CM_RegisterHull(const char *modName, const class vec3_c *points, u32 numPoints);
class cmBBMinsMaxs_i *CM_RegisterAABB(const class aabb &bb);
class cMod_i *CM_RegisterModel(const char *modName);
class cmSkelModel_i *CM_RegisterSkelModel(const char *skelModelName);
void CM_AddCObjectBaseToHashTable(class cmObjectBase_c *newCMObject);
void CM_FreeAllModels();

// cm_modelLoaderWrapper.cpp
bool CM_LoadRenderModelToSingleSurface(const char *rModelName, class cmSurface_c &out); // returns true if model loading fails

// cm_cmds.cpp
void CM_AddConsoleCommands();
void CM_RemoveConsoleCommands();

// cm_inlineBSPModel.cpp
// load any inline model from any bsp file
class cMod_i *CM_LoadBSPFileSubModel(const char *bspFileName, u32 subModelNumber);

// cm_world.cpp
bool CM_LoadWorldMap(const char *mapName);
cMod_i *CM_GetWorldModel();
cMod_i *CM_GetWorldSubModel(unsigned int subModelIndex);
bool CM_TraceWorldRay(class trace_c &tr);
bool CM_TraceWorldSphere(class trace_c &tr);
bool CM_TraceWorldAABB(class trace_c &tr);

// cm_phy.cpp - Source Engine .phy support
class cMod_i *CM_LoadModelFromPHYFile(const char *fname);

#endif // __CM_LOCAL_H__
