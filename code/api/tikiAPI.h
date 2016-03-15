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
// tikiAPI.h - Ritual's .tik file format support
#ifndef __API_TIKI_H__
#define __API_TIKI_H__


#include "iFaceBase.h"
#include <shared/typedefs.h>

#define TIKI_API_IDENTSTR "TIKI0001"

#define TF_ENTRY -1
#define TF_EXIT -2
#define TF_FIRST -3
#define TF_LAST -4

enum tikiCommandSide_e {
	TCS_SERVER,
	TCS_CLIENT
};

class tikiAnim_i {

public:
	virtual const char *getAlias() const = 0;
	virtual class kfModelAPI_i *getKFModel() const = 0;
	virtual class skelAnimAPI_i *getSkelAnim() const = 0;
	virtual int getTotalTimeMs() const = 0;
	virtual int getNumFrames() const = 0;
	virtual int getFrameTimeMs() const = 0;
	virtual void iterateCommands(tikiCommandSide_e side, u32 startTime, u32 endTime, class perStringCallbackListener_i *cb) const = 0;
};

class tiki_i {

public:
	virtual const char *getName() const = 0;

	virtual bool isSkeletal() const = 0;
	virtual bool isKeyframed() const = 0;

	virtual const char *getClassName() const = 0;

	virtual void iterateInitCommands(tikiCommandSide_e side, class perStringCallbackListener_i *cb) const = 0;

	virtual int findAnim(const char *animAlias) const = 0;
	virtual const class tikiAnim_i *getAnim(int animIndex) const = 0;
	virtual int getAnimTotalTimeMs(int animIndex) const = 0;
	virtual void applyMaterialRemapsTo(class modelPostProcessFuncs_i *out) const = 0;
	// for both bones and TAN tags
	virtual int getBoneNumForName(const char *boneName) const = 0;
	// for keyframed TIKI models
	virtual class kfModelAPI_i *getAnimKFModel(u32 animNum) const = 0;
	// for skeletal TIKI models
	virtual class skelModelAPI_i *getSkelModel() const = 0;
	virtual class skelAnimAPI_i *getSkelAnim(u32 animNum) const = 0;
};

class tikiAPI_i : public iFaceBase_i {
public:
	virtual void init() = 0;
	virtual void shutdown() = 0;
	virtual class tiki_i *registerModel(const char *modName) = 0;
};

extern tikiAPI_i *g_tikiMgr;

#endif // __API_TIKI_H__
