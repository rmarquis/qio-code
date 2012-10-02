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

#include <shared/typedefs.h>
#include <string.h> // memset

enum {
	DEFAULT_TEXTURE_SIZE = 32,
};

bool img_defaultTextureReady = false;

// create a default box-texture used to debug texture coordinates
byte img_defaultImage[DEFAULT_TEXTURE_SIZE][DEFAULT_TEXTURE_SIZE][4];
void IMG_CreateDefaultTexture() {
	memset(img_defaultImage, 32, sizeof(img_defaultImage) );
	for(u32 x = 0; x < DEFAULT_TEXTURE_SIZE; x++) {
		img_defaultImage[0][x][0] =
		img_defaultImage[0][x][1] =
		img_defaultImage[0][x][2] =
		img_defaultImage[0][x][3] = 255;

		img_defaultImage[x][0][0] =
		img_defaultImage[x][0][1] =
		img_defaultImage[x][0][2] =
		img_defaultImage[x][0][3] = 255;

		img_defaultImage[DEFAULT_TEXTURE_SIZE-1][x][0] =
		img_defaultImage[DEFAULT_TEXTURE_SIZE-1][x][1] =
		img_defaultImage[DEFAULT_TEXTURE_SIZE-1][x][2] =
		img_defaultImage[DEFAULT_TEXTURE_SIZE-1][x][3] = 255;

		img_defaultImage[x][DEFAULT_TEXTURE_SIZE-1][0] =
		img_defaultImage[x][DEFAULT_TEXTURE_SIZE-1][1] =
		img_defaultImage[x][DEFAULT_TEXTURE_SIZE-1][2] =
		img_defaultImage[x][DEFAULT_TEXTURE_SIZE-1][3] = 255;
	}
}
void IMG_GetDefaultImage(byte **outData, u32 *outW, u32 *outH) {
	if(img_defaultTextureReady == false) {
		IMG_CreateDefaultTexture();
		img_defaultTextureReady = true;
	}
	*outData = (byte*)img_defaultImage;
	*outW = DEFAULT_TEXTURE_SIZE;
	*outH = DEFAULT_TEXTURE_SIZE;
}