/*
============================================================================
Copyright (C) 2016 V.

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
// rect.h
#ifndef __SHARED_RECT_H__
#define __SHARED_RECT_H__

class rect_c {
	float min[2];
	float max[2];
public:
	void setMinX(float f) {
		min[0] = f;
	}
	void setMinY(float f) {
		min[1] = f;
	}
	void setMaxX(float f) {
		max[0] = f;
	}
	void setMaxY(float f) {
		max[1] = f;
	}
	void setW(float f) {
		max[0] = min[0]+f;
	}
	void setH(float f) {
		max[1] = min[1]+f;
	}
	float getX() const {
		return min[0];
	}
	float getY() const {
		return min[1];
	}
	float getCenterX() const {
		return (min[0]+max[0])*0.5f;
	}
	float getCenterY() const {
		return (min[1]+max[1])*0.5f;
	}
	float getMaxX() const {
		return max[0];
	}
	float getMaxY() const {
		return max[1];
	}
	float getW() const {
		return max[0]-min[0];
	}
	float getH() const {
		return max[1]-min[1];
	}
	float getXFrac(int ox) const {
		int local = ox - min[0];
		return float(local)/float(getW());
	}	
	bool isInside(int x, int y) const {
		if(x < min[0])
			return false;
		if(y < min[1])
			return false;
		if(x > max[0])
			return false;
		if(y > max[1])
			return false;
		return true;
	}
	void add(const rect_c &r) {
		for(unsigned int i = 0; i < 2; i++) {
			if(min[i] > r.min[i])
				min[i] = r.min[i];
			if(max[i] < r.max[i])
				max[i] = r.max[i];

		}
	}
	void clear() {
		min[0] = min[1] = 9999999;
		max[0] = max[1] = -9999999;
	}	
	void translate(int dX, int dY) {	
		min[0] += dX;
		max[0] += dX;
		min[1] += dY;
		max[1] += dY;
	}
};

#endif // __SHARED_RECT_H__

