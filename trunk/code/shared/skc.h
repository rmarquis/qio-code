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
#include <shared/shared.h>
#include <shared/array.h>
#include <shared/str.h>

struct skcChannelValue_s {
	float data[4];

	void set(const float *p) {
		memcpy(data,p,sizeof(data));
	}
};
class skcFrame_c {
friend class skc_c;
	arraySTD_c<skcChannelValue_s> values;
};
class skc_c {
	arraySTD_c<skcFrame_c> frames;
	arraySTD_c<str> channelNames;
public:
	bool loadSKC(const char *fname);

	u32 calculateSKCFileSize() const;
	u32 calculateSKCFileOfsChannels() const;
	u32 calculateSKCFileFrameOfsChannelValues(u32 frameNum) const;

	void resizeFrames(u32 c) {
		frames.resize(c);
		for(u32 i = 0; i < frames.size(); i++) {
			frames[i].values.resize(channelNames.size());
		}
	}
	void resizeChannels(u32 c) {
		channelNames.resize(c);
		for(u32 i = 0; i < frames.size(); i++) {
			frames[i].values.resize(channelNames.size());
		}
	}
	void setChannelName(u32 i, const char *s) {
		channelNames[i] = s;
	}
	void setFrameChannel(u32 f, u32 c, const float *xyzw) {
		frames[f].values[c].set(xyzw);
	}	

	
};