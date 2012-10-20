#ifndef __CULLRESULT_H__
#define __CULLRESULT_H__

enum cullResult_e {
	CULL_IN, // 0		// completely unclipped
	CULL_CLIP, // 1		// clipped by one or more planes
	CULL_OUT // 2		// completely outside the clipping planes
};

#endif // __CULLRESULT_H__
