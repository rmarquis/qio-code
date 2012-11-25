#ifndef __G_RAGDOLL_H__
#define __G_RAGDOLL_H__

#include <shared/array.h>
#include <math/matrix.h>

class ragdollAPI_i {
public:
	virtual ~ragdollAPI_i() {
	}

	virtual const arraySTD_c<matrix_c> &getCurWorldMatrices() const = 0;
	virtual void updateWorldTransforms() = 0;
};

#endif // __G_RAGDOLL_H__
