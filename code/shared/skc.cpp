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
#include "skc.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <fileFormats/skc_file_format.h>

u32 skc_c::calculateSKCFileOfsChannels() const {
	u32 size = sizeof(skcHeader_s);
	size += sizeof(skcFrame_s) * frames.size();
	size += sizeof(skcChannelValue_s) * channelNames.size() * frames.size();
	return size;
}
u32 skc_c::calculateSKCFileFrameOfsChannelValues(u32 frameNum) const {
	u32 size = sizeof(skcHeader_s);
	size += sizeof(skcFrame_s) * frames.size();
	size += sizeof(skcChannelValue_s) * channelNames.size() * frameNum;
	return size;
}
u32 skc_c::calculateSKCFileSize() const {
	u32 size = sizeof(skcHeader_s);
	size += SKC_MAX_CHANNEL_CHARS * channelNames.size();
	size += sizeof(skcFrame_s) * frames.size();
	size += sizeof(skcChannelValue_s) * channelNames.size() * frames.size();
	return size;
}
bool skc_c::loadSKC(const char *fname) {
	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}
	const skcHeader_s *h = (const skcHeader_s*)fileData;
	if(h->ident != SKC_IDENT) {
		g_vfs->FS_FreeFile(fileData);
		return true;
	}
	if(h->version != SKC_VERSION) {
		g_vfs->FS_FreeFile(fileData);
		return true;
	}
	this->frameTime = h->frameTime;
	channelNames.resize(h->numChannels);
	for(u32 i = 0; i < channelNames.size(); i++) {
		channelNames[i] = h->pChannel(i);
	}
	frames.resize(h->numFrames);
	for(u32 i = 0; i < frames.size(); i++) {
		const skcFrame_s *f = h->pFrame(i);
		skcFrame_c &of = frames[i];
		of.values.resize(channelNames.size());
		for(u32 j = 0; j < channelNames.size(); j++) {
			of.values[j].set(f->pValues(j,h));
		}
	}

	// VALIDATE
	if(h->ofsEnd != calculateSKCFileSize()) {
		g_core->RedWarning("SKC validation error for %s - ofsEnd %i, should be %i\n",
			fname,h->ofsEnd,calculateSKCFileSize());
	}
	if(h->ofsChannels != calculateSKCFileOfsChannels()) {
		g_core->RedWarning("SKC validation error for %s - ofsChannels %i, should be %i\n",
			fname,h->ofsChannels,calculateSKCFileOfsChannels());
	}
	for(u32 i = 0; i < h->numFrames; i++) {
		if(h->pFrame(i)->ofsChannelValues != calculateSKCFileFrameOfsChannelValues(i)) {
			g_core->RedWarning("SKC validation error for %s frame %i - ofsChannelValues is %i, should be %i\n",
				fname,i,h->pFrame(i)->ofsChannelValues,calculateSKCFileFrameOfsChannelValues(i));
		}
	}
	// can't really tell whether it's int or float
	//for(u32 i = 0; i < h->numFrames; i++) {
	//	if(h->pFrame(i)->unknown != 0.f) {
	//		g_core->RedWarning("SKC validation warning for %s - unknown value of frame %i is %f\n",
	//			fname,i,h->pFrame(i)->unknown);
	//	}
	//}

	g_vfs->FS_FreeFile(fileData);
	return false;
}
