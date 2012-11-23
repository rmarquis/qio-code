#ifndef __CM_LOCAL_H__
#define __CM_LOCAL_H__

// cm_model.cpp
class cMod_i *CM_FindModelInternal(const char *name);
class cmCapsule_i *CM_RegisterCapsule(float height, float radius);
class cmBBExts_i *CM_RegisterBoxExts(float halfSizeX, float halfSizeY, float halfSizeZ);
class cMod_i *CM_RegisterModel(const char *modName);
class cmSkelModel_i *CM_RegisterSkelModel(const char *skelModelName);

// cm_modelLoaderWrapper.cpp
bool CM_LoadRenderModelToSingleSurface(const char *rModelName, class cmSurface_c &out); // returns true if model loading fails

// cm_cmds.cpp
void CM_AddConsoleCommands();
void CM_RemoveConsoleCommands();

#endif // __CM_LOCAL_H__
