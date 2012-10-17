#ifndef __MAT_PUBLIC_H__
#define __MAT_PUBLIC_H__

// hardcoded alpha func test values (for non-blended transparency)
enum alphaFunc_e {
	AF_NONE,
	AF_GT0,
	AF_LT128,
	AF_GE128,
};

// blend modes; abstracted so they can apply to both opengl and dx
// if you edit this enum, remember to mirror your changes in blendModeEnumToGLBlend array.
enum blendMode_e {
	BM_NOT_SET,
	BM_ZERO,
	BM_ONE,
	BM_ONE_MINUS_SRC_COLOR,
	BM_ONE_MINUS_DST_COLOR,
	BM_ONE_MINUS_SRC_ALPHA,
	BM_ONE_MINUS_DST_ALPHA,
	BM_DST_COLOR,
	BM_DST_ALPHA,
	BM_SRC_COLOR,
	BM_SRC_ALPHA,
	BM_SRC_ALPHA_SATURATE,
};

struct blendDef_s {
	u16 src;
	u16 dst;

	blendDef_s() {
		src = BM_NOT_SET;
		dst = BM_NOT_SET;
	}
};

#endif // __MAT_PUBLIC_H__
