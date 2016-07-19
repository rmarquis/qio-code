/*
============================================================================
Copyright (C) 2010-2016 V.

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
#ifndef __SKC_FILE_FORMAT_H__
#define __SKC_FILE_FORMAT_H__

/*
==============================================================================

SKC file format

==============================================================================
*/

#define SKC_IDENT			(('N'<<24)+('A'<<16)+('K'<<8)+'S')
#define SKC_VERSION			13
#define SKC_VERSION_SH			14
#define SKC_MAX_CHANNEL_CHARS	32

struct skcBone_s {
	float		floatVal[4];
};

struct skcFrame_s {
	float		bounds[2][3];
	float		radius;
	float		delta[3];
	float		unknown;
	int			ofsChannelValues;
	
	skcFrame_s() {
		clear();
	}
	void clear() {
		memset(this,0,sizeof(skcFrame_s));
	}
	const float *pValues(int c, const void *h) const {
		return (const float*)(((const byte*)h) + ofsChannelValues + c * sizeof(skcBone_s));
	}
};

struct skcHeader_s {
	int			ident;
	int			version;

	int			type;

	int			ofsEnd;

	float		frameTime;

	int			i3;
	// non zero in allied_airborne/airborne.skc
	float			i4;
	int			i5;
	int			i6;
	int			numChannels;
	int			ofsChannels;
	int			numFrames;
	
	skcHeader_s() {
		clear();
		frameTime = 1.f;
	}
	void clear() {
		memset(this,0,sizeof(skcHeader_s));
	}
	float getTotalAnimTime() const {
		return frameTime * numFrames;
	}
	const char *pChannel(int c) const {
		return (const char*)(((const byte*)this) + ofsChannels + c * SKC_MAX_CHANNEL_CHARS);
	}

	const skcFrame_s *pFrame(int f) const {
		return (const skcFrame_s*)(((const byte*)this) + sizeof(*this) + f * sizeof(skcFrame_s));
	}
};

#endif // __SKC_FILE_FORMAT_H__
