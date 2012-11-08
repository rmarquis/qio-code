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
// heightmapLoader.cpp
#include <api/staticModelCreatorAPI.h>
#include <api/imgAPI.h>

u32 getIndex(u32 x, u32 y, u32 w, u32 h) {
	u32 idx = x + y * w;
	return idx;
}

float CalcPointHeight(u32 x, u32 y, u32 w, u32 h, const byte *data) {
    u32 idx = getIndex(x,y,w,h)*4;
    float r = (float)data[idx];
    float g = (float)data[idx+1];
    float b = (float)data[idx+2];
    return (0.299f * r + 0.587f * g + 0.114f * b);
}
bool MOD_LoadModelFromHeightmap(const char *fname, staticModelCreatorAPI_i *out) {
	byte *data;
	u32 w, h;
	const char *fixedFName = g_img->loadImage(fname,&data,&w,&h);
	if(data == 0) {
		return true; // error
	}
	float tcStep = 1.f/32.f;
	float xyStep = 32.f;
	float halfW = float(w) * xyStep * 0.5f;
	float halfH = float(h) * xyStep * 0.5f;
	out->resizeVerts(w*h);
	for(u32 x = 0; x < w; x++) {
		for(u32 y = 0; y < h; y++) {
			simpleVert_s v;
			v.xyz.x = x * xyStep - halfW;
			v.xyz.y = y * xyStep - halfH;
			//v.tc.x = (float(x)/float(w-1));
			//v.tc.y = (float(y)/float(h-1));
			v.tc.x = tcStep * float(x);
			v.tc.y = tcStep * float(y);
			v.xyz.z = CalcPointHeight(x,y,w,h,data)*4.f;
			out->setVert(getIndex(x,y,w,h),v);
		}
	}
	out->resizeIndices((w-1)*(h-1)*6);
	u32 idx = 0;
	for(u32 x = 1; x < w; x++) {
		for(u32 y = 1; y < h; y++) {
			u32 i0 = getIndex(x,y,w,h);
			u32 i1 = getIndex(x-1,y,w,h);
			u32 i2 = getIndex(x,y-1,w,h);
			u32 i3 = getIndex(x-1,y-1,w,h);

			out->setIndex(idx,i0);
			idx++;
			out->setIndex(idx,i1);
			idx++;
			out->setIndex(idx,i2);
			idx++;

			out->setIndex(idx,i3);
			idx++;
			out->setIndex(idx,i2);
			idx++;
			out->setIndex(idx,i1);
			idx++;
		}
	}

	g_img->freeImageData(data);
	return false;
}