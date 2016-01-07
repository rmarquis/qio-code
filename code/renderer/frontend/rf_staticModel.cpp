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
#include <api/materialSystemAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>

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
	virtual void buildEditorLightDiamondShape(const class aabb &inside, const vec3_c *color) {
		vec3_t corners[4];
		float midZ = inside.getMins()[2] + (inside.getMaxs()[2] - inside.getMins()[2]) / 2;

		corners[0][0] = inside.getMins()[0];
		corners[0][1] = inside.getMins()[1];
		corners[0][2] = midZ;

		corners[1][0] = inside.getMins()[0];
		corners[1][1] = inside.getMaxs()[1];
		corners[1][2] = midZ;

		corners[2][0] = inside.getMaxs()[0];
		corners[2][1] = inside.getMaxs()[1];
		corners[2][2] = midZ;

		corners[3][0] = inside.getMaxs()[0];
		corners[3][1] = inside.getMins()[1];
		corners[3][2] = midZ;

		vec3_c top, bottom;

		top[0] = inside.getMins()[0] + ((inside.getMaxs()[0] - inside.getMins()[0]) / 2);
		top[1] = inside.getMins()[1] + ((inside.getMaxs()[1] - inside.getMins()[1]) / 2);
		top[2] = inside.getMaxs()[2];

		bottom = top;
		bottom[2] = inside.getMins()[2];
		rVertexBuffer_c verts;
		vec3_c curColor(1,1,1);
		if(color)
			curColor = *color;
		vec3_c vSave = curColor;
		u16 indices[] = {
			// top sides
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 1,
			// bottom sides
			2, 1, 5,
			3, 2, 5,
			4, 3, 5,
			1, 4, 5
		};
		verts.addVertexXYZColor3f(top,curColor);
		for (u32 i = 0; i < 4; i++) {
			curColor *= 0.95;
			verts.addVertexXYZColor3f(corners[i],curColor);
		}
		curColor *= 0.95;
		verts.addVertexXYZColor3f(bottom,curColor);

		rIndexBuffer_c pIndices;
		pIndices.addU16Array(indices,sizeof(indices)/sizeof(indices[0]));
		// get an unique name so this material is used only here
		mtrAPI_i *mat = g_ms->registerMaterial("(1 0.92016 0.80106)");
		// HACK, so vertex colors are working...
		((mtrStageAPI_i*)mat->getFirstColorMapStage())->setRGBGenVerex();

		data.addSurface(mat,verts,pIndices);
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

