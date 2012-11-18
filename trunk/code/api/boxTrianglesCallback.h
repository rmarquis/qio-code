#ifndef __BOXTRIANGLESCALLBACK_H__
#define __BOXTRIANGLESCALLBACK_H__

class boxTrianglesCallback_i {
public:
	virtual void onBoxTriangle(const class vec3_c &p0, const class vec3_c &p1, const class vec3_c &p2) = 0;
};

#endif // __BOXTRIANGLESCALLBACK_H__
