#ifndef __MAT_PUBLIC_H__
#define __MAT_PUBLIC_H__

#include <shared/typedefs.h>

// hardcoded alpha func test values (for non-blended transparency)
enum alphaFunc_e {
	AF_NONE,
	AF_GT0,
	AF_LT128,
	AF_GE128,
};

// texCoordGens automatically
// overwrites existing texcoords
enum texCoordGen_e {
	TCG_NONE,
	TCG_ENVIRONMENT,
	// added for Doom3
	//TCG_SKYBOX,
	//TCG_REFLECT,
	TCG_NUM_GENS,
};

// vertex color generator (red, green, blue)
enum rgbGen_e {
	RGBGEN_NONE,
	RGBGEN_WAVE,
	RGBGEN_VERTEX,
	RGBGEN_CONST,
	RGBGEN_IDENTITY,
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

class skyBoxAPI_i {
public:
	virtual class textureAPI_i *getUp() const = 0;
	virtual class textureAPI_i *getDown() const = 0;
	virtual class textureAPI_i *getRight() const = 0;
	virtual class textureAPI_i *getLeft() const = 0;
	virtual class textureAPI_i *getFront() const = 0;
	virtual class textureAPI_i *getBack() const = 0;
};

class skyParmsAPI_i {
public:
	virtual float getCloudHeight() const = 0;
	virtual const skyBoxAPI_i *getFarBox() const = 0;
	virtual const skyBoxAPI_i *getNearBox() const = 0;
};

#endif // __MAT_PUBLIC_H__
