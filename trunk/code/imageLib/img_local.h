#ifndef __IMG_LOCAL_H__
#define __IMG_LOCAL_H__

#include <shared/typedefs.h>

// img_main.cpp
void IMG_GetDefaultImage(byte **outData, u32 *outW, u32 *outH);

// img_devil.cpp
void IMG_InitDevil();
const char *IMG_LoadImageInternal( const char *fname, byte **imageData, u32 *width, u32 *height );

// img_convert.cpp
void IMG_Convert8BitImageToRGBA32(byte **converted, u32 *outWidth, u32 *outHeight, const byte *pixels, u32 width, u32 height, const byte *palette);

#endif // __IMG_LOCAL_H__

