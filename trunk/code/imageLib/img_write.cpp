/*
============================================================================
Copyright (C) 2014 V.

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
#include "img_local.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <stdio.h>
#include <string.h>

bool IMG_WriteTGA(const char *fname, byte *pic, u32 width, u32 height, u32 inBPP) {
	u32 bpp;
	if(inBPP == 3 || inBPP == 4) {
		// image will be saved directly
		bpp = inBPP;
	} else if(inBPP == 1) {
		// image will be converted to RGB
		bpp = 3;
	} else {
		g_core->RedWarning("IMG_WriteTGA: invalid bpp %i, can't write to %s\n",fname,inBPP);
		return true;
	}
	u32 fileLen = width*height*bpp+18;
	
	byte *buffer = new byte[fileLen];
	memset(buffer, 0, 18);
	buffer[2] = 2; // uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = bpp*8;	// pixel size
	
	u32 numPixels = width*height;
	if(inBPP == bpp) {
		memcpy(buffer+18, pic, numPixels*bpp);
	} else {
		byte *p = buffer + 18;
		const byte *in = pic;
		for(u32 i = 0; i < numPixels; i++) {
			*p = *in;
			p++;
			*p = *in;
			*p++;
			*p = *in;
			*p++;
			in++;
		}
	}
//	if(bpp >= 3) {
		// swap rgb to bgr
		for(u32 i = 18; i < fileLen; i+=bpp) {
			byte tmp = buffer[i];
			buffer[i] = buffer[i+2];
			buffer[i+2] = tmp;
		}
//	}

	g_vfs->FS_WriteFile(fname, buffer, fileLen);
	
	delete [] buffer;
	return false;
}