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
// imgAPI.h - image loader API

#ifndef __IMGAPI_H__
#define __IMGAPI_H__

#include "iFaceBase.h"
#include <shared/typedefs.h>

#define IMG_API_IDENTSTR "ImagesAPI0001"

class imgAPI_i : public iFaceBase_i {
public:
	// default image access (returned data MUST NOT be fried!)
	virtual void getDefaultImage(byte **outData, u32 *outW, u32 *outH) = 0;

	// image loading (with preprocessor)
	virtual const char *loadImage(const char *fname, byte **outData, u32 *outW, u32 *outH) = 0;
	virtual void freeImageData(byte *data) = 0;

	// image processing
	virtual void convert8BitImageToRGBA32(byte **converted, u32 *outWidth, u32 *outHeight, const byte *pixels, u32 width, u32 height, const byte *palette) = 0;
};

extern imgAPI_i *g_img;

#endif // __IMGAPI_H__
