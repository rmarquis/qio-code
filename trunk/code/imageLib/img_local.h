
#include <shared/typedefs.h>


void IMG_GetDefaultImage(byte **outData, u32 *outW, u32 *outH);
void IMG_InitDevil();
const char *IMG_LoadImageInternal( const char *fname, byte **imageData, u32 *width, u32 *height );

