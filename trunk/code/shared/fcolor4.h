/*
============================================================================
Copyright (C) 2013 V.

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
// fcolor4.h
#ifndef __SHARED_FCOLOR4_H__
#define __SHARED_FCOLOR4_H__

class fcolor4_c {
	union {
		struct {
			float r,g,b,a;
		};
		float col[4];
	};
public:
	bool isFullBright() const {
		if(r == 1.f && g == 1.f && b == 1.f && a == 1.f) {
			return true;
		}
		return false;
	}
	void setFullBright() {
		r = b = g = a = 1.f;
	}
	void fromColor3f(const float *rgb) {
		if(rgb) {
			r = rgb[0];
			g = rgb[1];
			b = rgb[2];
			a = 1.f;
		} else {
			setFullBright();
		}
	}
	void fromColor4f(const float *rgba) {
		if(rgba) {
			r = rgba[0];
			g = rgba[1];
			b = rgba[2];
			a = rgba[3];
		} else {
			setFullBright();
		}
	}
	void set(float nr, float ng, float nb, float na)  {
		r = nr;
		g = ng;
		b = nb;
		a = na;
	}
	void scaleRGB(float f) {
		r *= f;
		g *= f;
		b *= f;
	}
	void scaleRGBA(float f) {
		r *= f;
		g *= f;
		b *= f;
		a *= f;
	}
	const float *toPointer() const {
		return &r;
	}
};

#endif // __SHARED_FCOLOR4_H__
