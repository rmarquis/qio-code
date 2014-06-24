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

// img_utils.cpp
void IMG_HorizontalFlip(byte *data, u32 width, u32 height);
void IMG_VerticalFlip(byte *data, u32 width, u32 height) ;
void IMG_RotatePic(byte *data, u32 width);

// img_write.cpp
bool IMG_WriteTGA(const char *fname, byte *pic, u32 width, u32 height, u32 bpp);

#endif // __IMG_LOCAL_H__

