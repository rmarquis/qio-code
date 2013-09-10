/*
============================================================================
Copyright (C) 2013 V.

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
// cg_testMaterial.cpp - material testing code
#include "cg_local.h"
#include <api/customRenderObjectAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <api/materialSystemAPI.h>
#include <api/rAPI.h>
#include <api/mtrAPI.h>
#include <math/vec3.h>
#include <shared/autoCvar.h>
#include <shared/simpleVert.h>

static aCvar_c cg_testMaterial_shapeType("cg_testMaterial_shapeType","0");
static aCvar_c cg_testMaterial_shapeSize("cg_testMaterial_shapeSize","64");
static aCvar_c cg_testMaterial_shapeTexCoordScale("cg_testMaterial_shapeTexCoordScale","1.0");
static aCvar_c cg_testMaterial("cg_testMaterial","0");

// material tester class
class cgMaterialTester_c : public customRenderObjectAPI_i {
	vec3_c origin;
	mtrAPI_i *mat;


	// customRenderObjectAPI_i impl
	// this is called every frame from renderer 
	virtual void instanceModel(class staticModelCreatorAPI_i *out, const class axis_c &viewerAxis) {
		out->clear();
		const char *matName = mat->getName();
		int shapeType = cg_testMaterial_shapeType.getInt();
		float size = cg_testMaterial_shapeSize.getFloat();
		if(shapeType == 0) {
			// quad lying on Z plane
			simpleVert_s verts[4];
			verts[3].setUV(0,0);
			verts[3].setXYZ(origin + vec3_c(-size,-size,0));
			verts[2].setUV(1,0);
			verts[2].setXYZ(origin + vec3_c(size,-size,0));
			verts[1].setUV(1,1);
			verts[1].setXYZ(origin + vec3_c(size,size,0));
			verts[0].setUV(0,1);
			verts[0].setXYZ(origin + vec3_c(-size,size,0));
			// quad indices: i0, i1, i2,  i2, i3, i0
			out->addTriangle(matName,verts[0],verts[1],verts[2]);
			out->addTriangle(matName,verts[2],verts[3],verts[0]);
		} else {
			// sprite rotating towards viewer
			simpleVert_s verts[4];
			verts[3].setUV(0,0);
			verts[3].setXYZ(origin - viewerAxis.getUp() * size - viewerAxis.getLeft() * size);
			verts[2].setUV(1,0);
			verts[2].setXYZ(origin + viewerAxis.getUp() * size - viewerAxis.getLeft() * size);
			verts[1].setUV(1,1);
			verts[1].setXYZ(origin + viewerAxis.getUp() * size + viewerAxis.getLeft() * size);
			verts[0].setUV(0,1);
			verts[0].setXYZ(origin - viewerAxis.getUp() * size + viewerAxis.getLeft() * size);
			// quad indices: i0, i1, i2,  i2, i3, i0
			out->addTriangle(matName,verts[0],verts[1],verts[2]);
			out->addTriangle(matName,verts[2],verts[3],verts[0]);
		}
		out->scaleTexCoords(cg_testMaterial_shapeTexCoordScale.getFloat());
		out->recalcTBNs();
	}
public:
	void setMaterial(class mtrAPI_i *newMat) {
		mat = newMat;
	}
	void setOrigin(const vec3_c &newPos) {
		origin = newPos;
	}
};

static cgMaterialTester_c *cg_materialTester = 0;

static void CG_FreeTestMaterialClass() {
	if(cg_materialTester) {
		rf->removeCustomRenderObject(cg_materialTester);
		delete cg_materialTester;
		cg_materialTester = 0;
	}
}

void CG_RunTestMaterial() {
	const char *matName = cg_testMaterial.getStr();
	if(matName[0] == 0 || (matName[0] == '0' && matName[1] == 0)) {
		CG_FreeTestMaterialClass();
		return;
	}
	class mtrAPI_i *mat = g_ms->registerMaterial(matName);
	if(cg_materialTester == 0) {
		cg_materialTester = new cgMaterialTester_c;
		rf->addCustomRenderObject(cg_materialTester);
		cg_materialTester->setOrigin(cg.refdefViewOrigin-vec3_c(0,0,48.f));
	}
	cg_materialTester->setMaterial(mat);
}


