/*
============================================================================
Copyright (C) 2015 V.

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
// texturedWinding.h
#ifndef __SHARED_TEXTUREDWINDING_H__
#define __SHARED_TEXTUREDWINDING_H__

#include <shared/array.h>
#include <shared/texturedVertex.h>


#define	NORMAL_EPSILON	0.0001
#define	DIST_EPSILON	0.02
#define	ON_EPSILON	0.01

class edPlane_c {
public:
    vec3_c normal;
    double dist;
public:
	int isPlaneEqual(const edPlane_c &b, int flip) const {
		vec3_c tempNormal;
		double tempDist;

		if (flip) {
			tempNormal = -b.normal;
			tempDist = -b.dist;
		} else {
			tempNormal = b.normal;
			tempDist = b.dist;
		}
		if (
		   fabs(this->normal[0] - tempNormal[0]) < NORMAL_EPSILON
		&& fabs(this->normal[1] - tempNormal[1]) < NORMAL_EPSILON
		&& fabs(this->normal[2] - tempNormal[2]) < NORMAL_EPSILON
		&& fabs(this->dist - tempDist) < DIST_EPSILON )
			return true;
		return false;
	}
	int fromPoints(const vec3_c &p1, const vec3_c &p2, const vec3_c &p3)
	{
		vec3_c v1 = p2 - p1;
		vec3_c v2 = p3 - p1;
		this->normal.crossProduct(v1,v2);
		if (this->normal.normalize2() < 0.1) 
			return false;
		this->dist = this->normal.dotProduct(p1);
		return true;
	}
	// project on normal plane
	// along ez 
	// assumes plane normal is normalized
	void projectOnPlane(const vec3_c &ez, vec3_c &p) {
		if (fabs(ez[0]) == 1)
			p[0] = (dist - normal[1] * p[1] - normal[2] * p[2]) / normal[0];
		else if (fabs(ez[1]) == 1)
			p[1] = (dist - normal[0] * p[0] - normal[2] * p[2]) / normal[1];
		else
			p[2] = (dist - normal[0] * p[0] - normal[1] * p[1]) / normal[2];
	}
	const vec3_c &getNormal() const {
		return normal;
	}
	float getDist() const {
		return dist;
	}
};

class texturedWinding_c {
	arraySTD_c<texturedVertex_c> points;
public:
	texturedWinding_c();
	texturedWinding_c(const class edPlane_c &pl);
	texturedWinding_c(u32 numPoints);
	texturedWinding_c *cloneWinding();
	texturedWinding_c *clip(const class edPlane_c &split, bool keepon);
	void splitEpsilon(vec3_t normal, double dist, float epsilon, texturedWinding_c **front, texturedWinding_c **back);
	texturedWinding_c *tryMerge(const texturedWinding_c *f2, vec3_t planenormal, int keep);
	bool isTiny() const;
	bool removePoint(int point);
	texturedWinding_c *insertPoint(vec3_t point, int spot);
	static int planesConcave(texturedWinding_c *w1, texturedWinding_c *w2, const vec3_c &normal1, const vec3_c &normal2, float dist1, float dist2);


	void removeLastPoint() {
		points.resize(points.size()-1);
	}
	u32 size() const {
		return points.size();
	}
	void addPointXYZ(const vec3_c &xyz) {
		texturedVertex_c tv;
		tv.xyz = xyz;
		points.push_back(tv);
	}
	void addPoint(const texturedVertex_c &tv) {
		points.push_back(tv);
	}
	const vec3_c &getXYZ(u32 i) const {
		return points[i].xyz;
	}
	texturedVertex_c &getPoint(u32 i) {
		return points[i];
	}
	const texturedVertex_c &getPoint(u32 i) const {
		return points[i];
	}
	const float *getTC(u32 i) const {
		return points[i].st;
	}
	void setTC(u32 i, float s, float t) {
		points[i].setTC(s,t);
	}
	void setXYZ(u32 i, const vec3_c &v) {
		points[i].xyz = v;
	}
	void setPoint(u32 i, const texturedVertex_c &v) {
		points[i] = v;
	}
};


#endif // __SHARED_TEXTUREDWINDING_H__
