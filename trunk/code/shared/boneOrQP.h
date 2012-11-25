#ifndef __BONEORQP_H_
#define __BONEORQP_H_

#include <math/vec3.h>
#include <math/quat.h>

// bone orientation defined by 3d vector and a quaterion - total 28 bytes
// (NOTE: boneOr_s from skelUtils.h is more than two times larger: 68 bytes)
class boneOrQP_c {
	vec3_c p;
	quat_c q;
public:
	void setPos(const float *newPos) {
		p = newPos;
	}
	void setQuatXYZ(const vec3_c &xyzQuat) {
		q.x = xyzQuat.x;
		q.y = xyzQuat.y;
		q.z = xyzQuat.z;
		q.calcW();
	}
	const quat_c &getQuat() const {
		return q;
	}
	const vec3_c &getPos() const {
		return p;
	}
};

class boneOrQPArray_t : public arraySTD_c<boneOrQP_c> {

};

#endif // __BONEORQP_H_
