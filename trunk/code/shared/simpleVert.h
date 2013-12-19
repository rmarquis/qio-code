#ifndef __SIMPLEVERT_H__
#define __SIMPLEVERT_H__

#include <math/vec3.h>
#include <math/vec2.h>

struct simpleVert_s {
	vec3_c xyz;
	vec2_c tc;

	void setXYZ(const double *d) {
		xyz[0] = d[0];
		xyz[1] = d[1];
		xyz[2] = d[2];
	}
	void setUV(const double *d) {
		tc[0] = d[0];
		tc[1] = d[1];
	}	
	void setXYZ(const float *f) {
		xyz[0] = f[0];
		xyz[1] = f[1];
		xyz[2] = f[2];
	}
	void setXYZ(float nX, float nY, float nZ) {
		xyz[0] = nX;
		xyz[1] = nY;
		xyz[2] = nZ;
	}
	void setUV(const float *f) {
		tc[0] = f[0];
		tc[1] = f[1];
	}
	void setUV(float u, float v) {
		tc[0] = u;
		tc[1] = v;
	}
};

#endif // __SIMPLEVERT_H__
