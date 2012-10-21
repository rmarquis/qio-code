#ifndef __SHARED_H__
#define __SHARED_H__

#include "typedefs.h"

inline void memcpy_strided(void *_dest, const void *_src, int elementCount, int elementSize, int destStride, int sourceStride) {
	byte *dest = (byte*) _dest;
	byte *src = (byte*) _src;
	if(destStride == 0) {
		destStride = elementSize;
	}
	if(sourceStride == 0) {
		sourceStride = elementSize;
	}
	for(int i = 0; i < elementCount; i++) {
		memcpy(dest,src,elementSize);
		dest += destStride;
		src += sourceStride;
	}
}

#endif // __SHARED_H__
