#ifndef __RF_LOCAL_H__
#define __RF_LOCAL_H__

#include "../cameraDef.h"

void RF_DoDebugDrawing();

class rEntityAPI_i *RFE_AllocEntity();
void RFE_RemoveEntity(class rEntityAPI_i *ent);
void RFE_AddEntityDrawCalls();
void RFE_ClearEntities();

extern class cameraDef_c rf_camera;

#endif // __RF_LOCAL_H__
