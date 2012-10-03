/*
============================================================================
Copyright (C) 2012 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// matrix.h - 4x4 matrix class
#ifndef __MATH_MATRIX_H__
#define __MATH_MATRIX_H__

#include "math.h"
#include "vec3.h"


class matrix_c {
	union {
		struct {
			vec4_t _m[4];
		};
		struct {
			vec_t _v[16];
		};
	};
public:
	inline matrix_c() {
		//identity();
	}
	matrix_c(float m0, float m1, float m2, float m3,
		 float m4, float m5, float m6, float m7,
		 float m8, float m9, float m10,float m11,
		 float m12,float m13,float m14,float m15) {
		this->_v[0] = m0; this->_v[4] = m4; this->_v[8]  = m8;  this->_v[12] = m12;
		this->_v[1] = m1; this->_v[5] = m5; this->_v[9]  = m9;  this->_v[13] = m13;
		this->_v[2] = m2; this->_v[6] = m6; this->_v[10] = m10; this->_v[14] = m14;
		this->_v[3] = m3; this->_v[7] = m7; this->_v[11] = m11; this->_v[15] = m15;
	}
	void setX(const float newX) {
		_v[12] = newX;
	}
	void setY(const float newY) {
		_v[13] = newY;
	}
	void setZ(const float newZ) {
		_v[14] = newZ;
	}
	void setOrigin(const float *o) {
		_v[12] = o[0];
		_v[13] = o[1];
		_v[14] = o[2];
	}
	const float	*getOrigin() const {
		return &_v[12];
	}
	const float *getForward() const {
		return &_v[0];
	}
	const float *getLeft() const {
		return &_v[4];
	}
	const float *getUp() const {
		return &_v[8];
	}
	float *getForward() {
		return &_v[0];
	}
	float *getLeft() {
		return &_v[4];
	}
	float *getUp() {
		return &_v[8];
	}
	class axis_c getAxis() const;
	vec3_c getAngles() const {
		vec3_c outAngles;
#if 1
		double          theta;
		double          cp;
		double          sp;

		sp = _v[2];

		// cap off our sin value so that we don't get any NANs
		if(sp > 1.0)
		{
			sp = 1.0;
		}
		else if(sp < -1.0)
		{
			sp = -1.0;
		}

		theta = -asin(sp);
		cp = cos(theta);
#ifndef FLT_EPSILON
		#define FLT_EPSILON (0.00001)
#endif //fixme
		if(cp > 8192 * FLT_EPSILON)
		{
			outAngles[0] = RAD2DEG(theta);
			outAngles[1] = RAD2DEG(atan2(_v[1], _v[0]));
			outAngles[2] = RAD2DEG(atan2(_v[6], _v[10]));
		}
		else
		{
			outAngles[0] = RAD2DEG(theta);
			outAngles[1] = RAD2DEG(-atan2(_v[4], _v[5]));
			outAngles[2] = 0;
		}
#else
		double          a;
		double          ca;

		a = asin(-_v[2]);
		ca = cos(a);

		if(fabs(ca) > 0.005)		// Gimbal lock?
		{
			outAngles[0] = RAD2DEG(atan2(_v[6] / ca, _v[10] / ca));
			outAngles[1] = RAD2DEG(a);
			outAngles[2] = RAD2DEG(atan2(_v[1] / ca, _v[0] / ca));
		}
		else
		{
			// Gimbal lock has occurred
			outAngles[0] = RAD2DEG(atan2(-_v[9], _v[5]));
			outAngles[1] = RAD2DEG(a);
			outAngles[2] = 0;
		}
#endif
		return outAngles;
	}
	class quat_c getQuat() const;
	void fromVectorsFLUAndOrigin(const vec3_c &forward, const vec3_c &left, const vec3_c &up, const vec3_c &origin) {
		_v[ 0] = forward[0];     _v[ 4] = left[0];        _v[ 8] = up[0];  _v[12] = origin[0];
		_v[ 1] = forward[1];     _v[ 5] = left[1];        _v[ 9] = up[1];  _v[13] = origin[1];
		_v[ 2] = forward[2];     _v[ 6] = left[2];        _v[10] = up[2];  _v[14] = origin[2];
		_v[ 3] = 0;              _v[ 7] = 0;              _v[11] = 0;      _v[15] = 1;
	}
	void fromAxisAndOrigin(const axis_c &ax, const vec3_c &origin);
	void setupXRotation(vec_t degrees) {
		vec_t a = DEG2RAD(degrees);

		_v[ 0] = 1;      _v[ 4] = 0;              _v[ 8] = 0;              _v[12] = 0;
		_v[ 1] = 0;      _v[ 5] = cos(a);         _v[ 9] =-sin(a);         _v[13] = 0;
		_v[ 2] = 0;      _v[ 6] = sin(a);         _v[10] = cos(a);         _v[14] = 0;
		_v[ 3] = 0;      _v[ 7] = 0;              _v[11] = 0;              _v[15] = 1;
	}

	void setupYRotation(vec_t degrees) {
		vec_t a = DEG2RAD(degrees);

		_v[ 0] = cos(a);         _v[ 4] = 0;      _v[ 8] = sin(a);         _v[12] = 0;
		_v[ 1] = 0;              _v[ 5] = 1;      _v[ 9] = 0;              _v[13] = 0;
		_v[ 2] =-sin(a);         _v[ 6] = 0;      _v[10] = cos(a);         _v[14] = 0;
		_v[ 3] = 0;              _v[ 7] = 0;      _v[11] = 0;              _v[15] = 1;
	}

	void setupZRotation(vec_t degrees) {
		vec_t a = DEG2RAD(degrees);

		_v[ 0] = cos(a);         _v[ 4] =-sin(a);         _v[ 8] = 0;      _v[12] = 0;
		_v[ 1] = sin(a);         _v[ 5] = cos(a);         _v[ 9] = 0;      _v[13] = 0;
		_v[ 2] = 0;              _v[ 6] = 0;              _v[10] = 1;      _v[14] = 0;
		_v[ 3] = 0;              _v[ 7] = 0;              _v[11] = 0;      _v[15] = 1;
	}
	void setupOrigin(float x, float y, float z) {
		this->_v[0] = 1.0; this->_v[4] = 0.0; this->_v[8]  = 0.0;  this->_v[12] = x;
		this->_v[1] = 0.0; this->_v[5] = 1.0; this->_v[9]  = 0.0;  this->_v[13] = y;
		this->_v[2] = 0.0; this->_v[6] = 0.0; this->_v[10] = 1.0; this->_v[14] = z;
		this->_v[3] = 0.0; this->_v[7] = 0.0; this->_v[11] = 0.0; this->_v[15] = 1.0;
	}
	void rotateX(vec_t degrees) {
		matrix_c rot;
		rot.setupXRotation(degrees);
		*this = *this * rot;
	}
	void rotateY(vec_t degrees) {
		matrix_c rot;
		rot.setupYRotation(degrees);
		*this = *this * rot;
	}
	void rotateZ(vec_t degrees) {
		matrix_c rot;
		rot.setupZRotation(degrees);
		*this = *this * rot;
	}
	void translate(float x, float y, float z) {
		matrix_c ofs;
		ofs.setupOrigin(x,y,z);
		*this = *this * ofs;
	}
	void scale(float scaleX, float scaleY, float scaleZ) {
		_v[ 0] *= scaleX;     _v[ 4] *= scaleY;        _v[ 8] *= scaleZ;
		_v[ 1] *= scaleX;     _v[ 5] *= scaleY;        _v[ 9] *= scaleZ;
		_v[ 2] *= scaleX;     _v[ 6] *= scaleY;        _v[10] *= scaleZ;
		_v[ 3] *= scaleX;     _v[ 7] *= scaleY;        _v[11] *= scaleZ;
	}
	void setupProjection(float fovX, float fovY, float zNear, float zFar) {
		float ymax = zNear * tan( fovY * M_PI / 360.0f );
		float ymin = -ymax;

		float xmax = zNear * tan( fovX * M_PI / 360.0f );
		float xmin = -xmax;

		float width = xmax - xmin;
		float height = ymax - ymin;
		float depth = zFar - zNear;

		_v[0] = 2 * zNear / width;
		_v[4] = 0;
		_v[8] = ( xmax + xmin ) / width;	// usually 0
		_v[12] = 0;

		_v[1] = 0;
		_v[5] = 2 * zNear / height;
		_v[9] = ( ymax + ymin ) / height;	// usually 0
		_v[13] = 0;

		_v[2] = 0;
		_v[6] = 0;
		_v[10] = -( zFar + zNear ) / depth;
		_v[14] = -2 * zFar * zNear / depth;

		_v[3] = 0;
		_v[7] = 0;
		_v[11] = -1;
		_v[15] = 0;
	}
	// http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
	void setupProjectionOrtho(float left, float right,
		float bottom, float top, float zNear, float zFar) {

		_v[0] = 2.f / (right - left);
		_v[4] = 0;
		_v[8] = 0;
		_v[12] = -((right + left) / (right - left));

		_v[1] = 0;
		_v[5] = 2.f / (top - bottom);
		_v[9] = 0;
		_v[13] = -((top + bottom) / (top - bottom));

		_v[2] = 0;
		_v[6] = 0;
		_v[10] = -2.f / (zFar - zNear);
		_v[14] = -((zFar + zNear) / (zFar - zNear));

		_v[3] = 0;
		_v[7] = 0;
		_v[11] = 0;
		_v[15] = 1.f;
	}
	float det() const {
		return ((this->_v[0] * this->_v[5] * this->_v[10]) +
				(this->_v[4] * this->_v[9] * this->_v[2])  +
				(this->_v[8] * this->_v[1] * this->_v[6])  -
				(this->_v[8] * this->_v[5] * this->_v[2])  -
				(this->_v[4] * this->_v[1] * this->_v[10]) -
				(this->_v[0] * this->_v[9] * this->_v[6]));
	}
	void getInversed(matrix_c& out) const {
		float idet = 1.0f / det();
		out[0]  =  (this->_v[5] * this->_v[10] - this->_v[9] * this->_v[6]) * idet;
		out[1]  = -(this->_v[1] * this->_v[10] - this->_v[9] * this->_v[2]) * idet;
		out[2]  =  (this->_v[1] * this->_v[6]  - this->_v[5] * this->_v[2]) * idet;
		out[3]  = 0.0;
		out[4]  = -(this->_v[4] * this->_v[10] - this->_v[8] * this->_v[6]) * idet;
		out[5]  =  (this->_v[0] * this->_v[10] - this->_v[8] * this->_v[2]) * idet;
		out[6]  = -(this->_v[0] * this->_v[6]  - this->_v[4] * this->_v[2]) * idet;
		out[7]  = 0.0;
		out[8]  =  (this->_v[4] * this->_v[9] - this->_v[8] * this->_v[5]) * idet;
		out[9]  = -(this->_v[0] * this->_v[9] - this->_v[8] * this->_v[1]) * idet;
		out[10] =  (this->_v[0] * this->_v[5] - this->_v[4] * this->_v[1]) * idet;
		out[11] = 0.0;
		out[12] = -(this->_v[12] * (out)[0] + this->_v[13] * (out)[4] + this->_v[14] * (out)[8]);
		out[13] = -(this->_v[12] * (out)[1] + this->_v[13] * (out)[5] + this->_v[14] * (out)[9]);
		out[14] = -(this->_v[12] * (out)[2] + this->_v[13] * (out)[6] + this->_v[14] * (out)[10]);
		out[15] = 1.0;
	}
	matrix_c getInversed() const {
		matrix_c ret;
		this->getInversed(ret);
		return ret;
	}
	void inverse() {
		matrix_c copy = *this;
		copy.getInversed(*this);
	}
	//vec3_c getRight() const {
	//	return -vec3_c(&_v[4]);
	//}
	void identity() {
		this->_v[0] = 1.0; this->_v[4] = 0.0; this->_v[8]  = 0.0;  this->_v[12] = 0.0;
		this->_v[1] = 0.0; this->_v[5] = 1.0; this->_v[9]  = 0.0;  this->_v[13] = 0.0;
		this->_v[2] = 0.0; this->_v[6] = 0.0; this->_v[10] = 1.0; this->_v[14] = 0.0;
		this->_v[3] = 0.0; this->_v[7] = 0.0; this->_v[11] = 0.0; this->_v[15] = 1.0;
	}
	void toGL() {
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		const matrix_c matrix_flipToGL(
			0, 0, -1, 0,
			-1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 1
		);
		*this = matrix_flipToGL * *this;
	}
	matrix_c operator * (const matrix_c &_v) const {
		matrix_c ret;
		ret._v[ 0] = _v._v[ 0]*this->_v[ 0] + _v._v[ 1]*this->_v[ 4] + _v._v[ 2]*this->_v[ 8] + _v._v[ 3]*this->_v[12];
		ret._v[ 1] = _v._v[ 0]*this->_v[ 1] + _v._v[ 1]*this->_v[ 5] + _v._v[ 2]*this->_v[ 9] + _v._v[ 3]*this->_v[13];
		ret._v[ 2] = _v._v[ 0]*this->_v[ 2] + _v._v[ 1]*this->_v[ 6] + _v._v[ 2]*this->_v[10] + _v._v[ 3]*this->_v[14];
		ret._v[ 3] = _v._v[ 0]*this->_v[ 3] + _v._v[ 1]*this->_v[ 7] + _v._v[ 2]*this->_v[11] + _v._v[ 3]*this->_v[15];

		ret._v[ 4] = _v._v[ 4]*this->_v[ 0] + _v._v[ 5]*this->_v[ 4] + _v._v[ 6]*this->_v[ 8] + _v._v[ 7]*this->_v[12];
		ret._v[ 5] = _v._v[ 4]*this->_v[ 1] + _v._v[ 5]*this->_v[ 5] + _v._v[ 6]*this->_v[ 9] + _v._v[ 7]*this->_v[13];
		ret._v[ 6] = _v._v[ 4]*this->_v[ 2] + _v._v[ 5]*this->_v[ 6] + _v._v[ 6]*this->_v[10] + _v._v[ 7]*this->_v[14];
		ret._v[ 7] = _v._v[ 4]*this->_v[ 3] + _v._v[ 5]*this->_v[ 7] + _v._v[ 6]*this->_v[11] + _v._v[ 7]*this->_v[15];

		ret._v[ 8] = _v._v[ 8]*this->_v[ 0] + _v._v[ 9]*this->_v[ 4] + _v._v[10]*this->_v[ 8] + _v._v[11]*this->_v[12];
		ret._v[ 9] = _v._v[ 8]*this->_v[ 1] + _v._v[ 9]*this->_v[ 5] + _v._v[10]*this->_v[ 9] + _v._v[11]*this->_v[13];
		ret._v[10] = _v._v[ 8]*this->_v[ 2] + _v._v[ 9]*this->_v[ 6] + _v._v[10]*this->_v[10] + _v._v[11]*this->_v[14];
		ret._v[11] = _v._v[ 8]*this->_v[ 3] + _v._v[ 9]*this->_v[ 7] + _v._v[10]*this->_v[11] + _v._v[11]*this->_v[15];

		ret._v[12] = _v._v[12]*this->_v[ 0] + _v._v[13]*this->_v[ 4] + _v._v[14]*this->_v[ 8] + _v._v[15]*this->_v[12];
		ret._v[13] = _v._v[12]*this->_v[ 1] + _v._v[13]*this->_v[ 5] + _v._v[14]*this->_v[ 9] + _v._v[15]*this->_v[13];
		ret._v[14] = _v._v[12]*this->_v[ 2] + _v._v[13]*this->_v[ 6] + _v._v[14]*this->_v[10] + _v._v[15]*this->_v[14];
		ret._v[15] = _v._v[12]*this->_v[ 3] + _v._v[13]*this->_v[ 7] + _v._v[14]*this->_v[11] + _v._v[15]*this->_v[15];
		return ret;
	}
	// recoded from Quake 3 SDK, see tr_main.c::R_RotateForViewer
	void invFromAxisAndVector(const vec3_t axis[3], const vec3_t origin) {
		_v[0] = axis[0][0];
		_v[4] = axis[0][1];
		_v[8] = axis[0][2];
		_v[12] = -origin[0] * _v[0] + -origin[1] * _v[4] + -origin[2] * _v[8];

		_v[1] = axis[1][0];
		_v[5] = axis[1][1];
		_v[9] = axis[1][2];
		_v[13] = -origin[0] * _v[1] + -origin[1] * _v[5] + -origin[2] * _v[9];

		_v[2] = axis[2][0];
		_v[6] = axis[2][1];
		_v[10] = axis[2][2];
		_v[14] = -origin[0] * _v[2] + -origin[1] * _v[6] + -origin[2] * _v[10];

		_v[3] = 0;
		_v[7] = 0;
		_v[11] = 0;
		_v[15] = 1;
	}
	void transformPoint(const vec3_c &in, vec3_c &out) const {
		out[ 0] = _v[ 0] * in[ 0] + _v[ 4] * in[ 1] + _v[ 8] * in[ 2] + _v[12];
		out[ 1] = _v[ 1] * in[ 0] + _v[ 5] * in[ 1] + _v[ 9] * in[ 2] + _v[13];
		out[ 2] = _v[ 2] * in[ 0] + _v[ 6] * in[ 1] + _v[10] * in[ 2] + _v[14];
	}
	void transformPoint(vec3_c &inout) const {
		vec3_c out;
		transformPoint(inout,out);
		inout = out;
	}
	void transformNormal(const vec3_c &in, vec3_c &out) const {
		out[ 0] = _v[ 0] * in[ 0] + _v[ 4] * in[ 1] + _v[ 8] * in[ 2]; // + _v[12];
		out[ 1] = _v[ 1] * in[ 0] + _v[ 5] * in[ 1] + _v[ 9] * in[ 2]; // + _v[13];
		out[ 2] = _v[ 2] * in[ 0] + _v[ 6] * in[ 1] + _v[10] * in[ 2]; // + _v[14];
	}
	void transformNormal(vec3_c &inout) const {
		vec3_c out;
		transformNormal(inout,out);
		inout = out;
	}
	void transformAABB(const class aabb &in, aabb &out) const;
	void fromQuat(const class quat_c &q);
	void fromQuatAndOrigin(const quat_c &q, const vec3_c &org) {
		// setup rotation
		fromQuat(q);
		// setup origin offset
		_v[12] = org[0];
		_v[13] = org[1];
		_v[14] = org[2];
	}
	void fromAngles(const vec3_c &angles) {
		// pitch
		float sp = sin(DEG2RAD(angles.x));
		float cp = cos(DEG2RAD(angles.x));

		// yaw
		float sy = sin(DEG2RAD(angles.y));
		float cy = cos(DEG2RAD(angles.y));

		// roll
		float sr = sin(DEG2RAD(angles.z));
		float cr = cos(DEG2RAD(angles.z));

		_v[ 0] = cp*cy;  _v[ 4] = (sr*sp*cy+cr*-sy);      _v[ 8] = (cr*sp*cy+-sr*-sy);     _v[12] = 0;
		_v[ 1] = cp*sy;  _v[ 5] = (sr*sp*sy+cr*cy);       _v[ 9] = (cr*sp*sy+-sr*cy);      _v[13] = 0;
		_v[ 2] = -sp;    _v[ 6] = sr*cp;                  _v[10] = cr*cp;                  _v[14] = 0;
		_v[ 3] = 0;      _v[ 7] = 0;                      _v[11] = 0;                      _v[15] = 1;
	}
	void fromAnglesAndOrigin(const vec3_c &a, const vec3_c &org) {
		// setup rotation
		fromAngles(a);
		// setup origin offset
		_v[12] = org[0];
		_v[13] = org[1];
		_v[14] = org[2];
	}
	// radian angles are used in HL2 SMD and Milkshape3D .ms3d
	void fromRadianAnglesAndOrigin(const vec3_c &ra, const vec3_c &org) {
		vec3_c angles( RAD2DEG(ra.y), RAD2DEG(ra.z), RAD2DEG(ra.x) );
		fromAnglesAndOrigin(angles,org);
	}


	void fromOrigin(const vec3_c &origin) {
		identity();
		setOrigin(origin);
	}
	void addOrigin(const vec3_c &ofs) {
		_v[12] += ofs.x;
		_v[13] += ofs.y;
		_v[14] += ofs.z;
	}
	void scaleOriginXYZ(const float scaleXYZ) {
		_v[12] *= scaleXYZ;
		_v[13] *= scaleXYZ;
		_v[14] *= scaleXYZ;
	}
	
	void print() const;
	bool compare(const matrix_c &other) const {
		return !memcmp(this,&other,sizeof(matrix_c));
	}

	inline vec4_t *getVec4() {
		return (vec4_t *)&_m;
	}

	inline operator float *() const {
		return (float*)&_m[0][0];
	}
	inline operator float *() {
		return (float*)&_m[0][0];
	}
};

class matrixExt_c {
	matrix_c mat;
	bool identity;
public:
	matrixExt_c() {
		mat.identity();
		identity = true;
	}
	bool set(const matrix_c &newMat) {
		if(mat.compare(newMat)) {
			return false; // no change
		}
		identity = false;
		mat = newMat;
		return true; // matrix has changed
	}
	bool setIdentity() {
		if(identity) {
			return false; // no change
		}
		identity = true;
		mat.identity();
		return true; // matrix has changed
	}
	bool isIdentity() const {
		return this->identity;
	}
	const matrix_c &getMat() const {
		return mat;
	}
};

#endif // __MATH_MATRIX_H__
