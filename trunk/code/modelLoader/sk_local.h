#ifndef __SKEL_LOCAL_H__
#define __SKEL_LOCAL_H__

#include <shared/typedefs.h>
#include <shared/skelUtils.h>

u32 SK_RegisterString(const char *s);
const char *SK_GetString(u32 idx);

#endif // __SKEL_LOCAL_H__