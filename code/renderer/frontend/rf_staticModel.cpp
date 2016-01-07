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
// rf_staticModel.cpp
#include "rf_local.h"
#include "rf_surface.h"
#include <api/rStaticModelAPI.h>

class rStaticModelImpl_c : public rStaticModelAPI_i {
	r_model_c data;
	float color[4];
	bool bHasCustomColor;
public:
	rStaticModelImpl_c() {
		bHasCustomColor = false;
	}
	virtual void clearStaticModelData() {
		data.clear();
	}
	virtual void setColor(const float *rgba) {
		if(rgba) {
			memcpy(color,rgba,sizeof(color));
			// hack
			color[0] *= 255.f;
			color[1] *= 255.f;
			color[2] *= 255.f;
			color[3] *= 255.f;
			bHasCustomColor = true;
		} else {
			bHasCustomColor = false;
		}
	}
	virtual staticModelCreatorAPI_i *getStaticModelCreator() {
		return &data;
	}
	virtual void buildVBOsAndIBOs() { 
		data.createVBOsAndIBOs();
	}
	void addDrawCalls() {


		if(rf_camera.getFrustum().cull(data.getBounds()) == CULL_OUT) {
			return;
		}


		if(bHasCustomColor) {
			data.addDrawCalls(0,false,(vec3_c*)color);
		} else {
			data.addDrawCalls();
		}
	}
};


static arraySTD_c<rStaticModelImpl_c*> rf_staticModels;


class rStaticModelAPI_i *RF_AllocStaticModel() {
	rStaticModelImpl_c *ent = new rStaticModelImpl_c;
	rf_staticModels.push_back(ent);
	return ent;
}
void RF_RemoveStaticModel(class rStaticModelAPI_i *ent) {
	rStaticModelImpl_c *rent = (rStaticModelImpl_c*)ent;
	rf_staticModels.remove(rent);
	delete rent;
}

void RF_AddStaticModelDrawCalls() {
	for(u32 i = 0; i < rf_staticModels.size(); i++) {
		rf_staticModels[i]->addDrawCalls();
	}
}

