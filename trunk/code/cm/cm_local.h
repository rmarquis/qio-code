#ifndef __CM_LOCAL_H__
#define __CM_LOCAL_H__

#include <shared/typedefs.h>

// cm_model.cpp
class cMod_i *CM_FindModelInternal(const char *name);
class cmCapsule_i *CM_RegisterCapsule(float height, float radius);
class cmBBExts_i *CM_RegisterBoxExts(float halfSizeX, float halfSizeY, float halfSizeZ);
class cmBBMinsMaxs_i *CM_RegisterAABB(const class aabb &bb);
class cMod_i *CM_RegisterModel(const char *modName);
class cmSkelModel_i *CM_RegisterSkelModel(const char *skelModelName);
void CM_AddCObjectBaseToHashTable(class cmObjectBase_c *newCMObject);

// cm_modelLoaderWrapper.cpp
bool CM_LoadRenderModelToSingleSurface(const char *rModelName, class cmSurface_c &out); // returns true if model loading fails

// cm_cmds.cpp
void CM_AddConsoleCommands();
void CM_RemoveConsoleCommands();

// cm_inlineBSPModel.cpp
// load any inline model from any bsp file
class cMod_i *CM_LoadBSPFileSubModel(const char *bspFileName, u32 subModelNumber);

#endif // __CM_LOCAL_H__
