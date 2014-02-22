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
// img_utils.cpp - image loader interface
#include <shared/typedefs.h>
#include <stdlib.h> // malloc, free
#include <string.h> // memcpy

void IMG_HorizontalFlip(byte *data, u32 width, u32 height) {
	for(u32 i = 0; i < height; i++) {
		for(u32 j = 0; j < width / 2; j++) {
			int temp = *( (int *)data + i * width + j );
			*( (int *)data + i * width + j ) = *( (int *)data + i * width + width - 1 - j );
			*( (int *)data + i * width + width - 1 - j ) = temp;
		}
	}
}

void IMG_VerticalFlip(byte *data, u32 width, u32 height) {
	for(u32 i = 0; i < width; i++) {
		for(u32 j = 0; j < height / 2; j++) {
			int temp = *( (int *)data + j * width + i );
			*( (int *)data + j * width + i ) = *( (int *)data + ( height - 1 - j ) * width + i );
			*( (int *)data + ( height - 1 - j ) * width + i ) = temp;
		}
	}
}

void IMG_RotatePic(byte *data, u32 width) {
	int *temp = (int *)malloc( width * width * 4 );

	for(u32 i = 0 ; i < width; i++) {
		for(u32 j = 0 ; j < width; j++) {
			*( temp + i * width + j ) = *( (int *)data + j * width + i );
		}
	}

	memcpy(data, temp, width * width * 4);

	free(temp);
}


