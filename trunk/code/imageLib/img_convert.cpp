/*
============================================================================
Copyright (C) 2012-2013 V.

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
// img_convert.cpp - image converting functions
#include "img_local.h"
#include <stdlib.h> // malloc

void IMG_Convert8BitImageToRGBA32(byte **out, u32 *poutWidth, u32 *poutHeight, const byte *in, u32 inWidth, u32 inHeight, const byte *pal) {
	int		i, j;
	int		row1[2048], row2[2048], col1[2048], col2[2048];
	const byte	*pix1, *pix2, *pix3, *pix4;
	byte	*tex;

	int outWidth;
	// convert texture to power of 2
	for (outWidth = 1; outWidth < inWidth; outWidth <<= 1)
		;

	if (outWidth > 256)
		outWidth = 256;

	int outHeight;
	for (outHeight = 1; outHeight < inHeight; outHeight <<= 1)
		;

	if (outHeight > 256)
		outHeight = 256;

	tex = (byte *)malloc( outWidth * outHeight * 4);

	for (i = 0; i < outWidth; i++)
	{
		col1[i] = (i + 0.25) * (inWidth / (float)outWidth);
		col2[i] = (i + 0.75) * (inWidth / (float)outWidth);
	}

	for (i = 0; i < outHeight; i++)
	{
		row1[i] = (int)((i + 0.25) * (inHeight / (float)outHeight)) * inWidth;
		row2[i] = (int)((i + 0.75) * (inHeight / (float)outHeight)) * inWidth;
	}

	// scale down and convert to 32bit RGB
	byte *ptr = tex;
	for (i=0; i < outHeight; i++)
	{
		for (j=0; j < outWidth; j++, ptr += 4)
		{
			pix1 = &pal[in[row1[i] + col1[j]] * 3];
			pix2 = &pal[in[row1[i] + col2[j]] * 3];
			pix3 = &pal[in[row2[i] + col1[j]] * 3];
			pix4 = &pal[in[row2[i] + col2[j]] * 3];

			ptr[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			ptr[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			ptr[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			ptr[3] = 0xFF;
		}
	}

	*out = tex;

	*poutWidth = outWidth;
	*poutHeight = outHeight;
}
