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
// wavefrontOBJModelLoader.cpp
#include <shared/parser.h>
#include <shared/array.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include "mtlFile.h"
#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <api/rAPI.h>

bool MOD_LoadOBJ(const char *fname, staticModelCreatorAPI_i *out) {
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("MOD_LoadOBJ: cannot open %s\n",fname);
		return true;
	}
	arraySTD_c<vec3_c> XYZs;
	arraySTD_c<vec2_c> texCoords;
	u32 unnamedGroupNumber = 0;
	str lastMtl;
	mtlFile_c mtlFile;
	str currentMaterialName = "missingObjMaterial";
	while(p.atEOF() == false) {
		if(p.atWord("#")) {
			p.skipLine();
		} else if(p.atWord("o")) {
			str objectName = p.getToken();
		} else if(p.atWord("g")) {
			str groupName;
			if(p.isAtEOL()) {
				g_core->RedWarning("rModel_c::loadOBJ: warning: missing group name after 'g' token at line %i\n",p.getCurrentLineNumber());
				groupName = va("unnamedGroupNumber%i",unnamedGroupNumber);
				unnamedGroupNumber++;
			} else {
				groupName = p.getToken();
			}
		} else if(p.atWord("s")) {
			p.getInteger();
		} else if(p.atWord("v")) {
			vec3_c &newXYZ = XYZs.pushBack();
			p.getFloatMat(newXYZ,3);
		} else if(p.atWord("vt")) {
			vec2_c &newTC = texCoords.pushBack();
			p.getFloatMat(newTC,2);
#if 1
			// it might break some models texcorods instead of fixing them..?
			// I was right, it breaks texcoords of barrel_c model from
			// http://thefree3dmodels.com/stuff/accessories/radioactive_barrel/21-1-0-1481
			newTC.y *= -1;
#endif
			if(p.isAtEOL() == false) {
				p.skipLine();
			}
		} else if(p.atWord("vn")) {
			vec3_c norm;
			p.getFloatMat(norm,3);
		} else if(p.atWord("f")) {
			u32 vertIndexes[512];
			u32 texVertIndexes[512];
			u32 numPoints = 0;
			//int l = p.getCurrentLineNumber();
			bool hasTexVerts = false;
			while(p.isAtEOL() == false) {
				const char *w = p.getToken();
				const char *p = strchr(w,'/');
				if(p) {
					str tmp;
					tmp.setFromTo(w,p);
					vertIndexes[numPoints] = atoi(tmp);
					p++;
					hasTexVerts = true;
					const char *p2 = strchr(p,'/');
					if(p2 == 0) {
						texVertIndexes[numPoints] = atoi(p);
					} else {
						tmp.setFromTo(p,p2);
						texVertIndexes[numPoints] = atoi(tmp);
					}
				} else {
					vertIndexes[numPoints] = atoi(w);
				}
				numPoints++;
			}
			if(vertIndexes[0]-1 >= XYZs.size()) {
				// this should NEVER happen
				g_core->RedWarning("rModel_c::loadOBJ: vertex index out of range\n");
			} else {
				simpleVert_s baseVert;
				baseVert.xyz = XYZs[vertIndexes[0]-1];
				if(hasTexVerts) {
					int baseTexIdx = texVertIndexes[0];
					if(baseTexIdx > 0) {
						baseVert.tc = texCoords[baseTexIdx-1];
					}
				}
				for(u32 i = 2; i < numPoints; i++) {
					simpleVert_s first, second;
					int firstIdx = vertIndexes[i-1];
					first.xyz = XYZs[firstIdx-1];
					if(hasTexVerts) {
						int firstTexIdx = texVertIndexes[i-1];
						if(firstTexIdx > 0) {
							first.tc = texCoords[firstTexIdx-1];
						}
					}
					int secondIdx = vertIndexes[i];
					second.xyz = XYZs[secondIdx-1];
					if(hasTexVerts) {
						int secondTexIdx = texVertIndexes[i];
						if(secondTexIdx > 0) {
							second.tc = texCoords[secondTexIdx-1];
						}
					}
					//sf->addTriangleV(baseVert,first,second);
					out->addTriangle(currentMaterialName,baseVert,first,second);
				}
			}
		} else if(p.atWord("usemtl")) {
			str mtlName = p.getToken();
			g_core->Print("rModel_c::loadOBJ: usemtl %s\n",mtlName.c_str());
			const struct mtlEntry_s *me = mtlFile.findEntry(mtlName);
			if(me) {
				str mtlImageName;
				if(me->map_Ka.length()) { 
					mtlImageName.append(me->map_Ka);
				} else if(me->map_refl.length()) {
					mtlImageName.append(me->map_refl);
				} else if(me->map_Kd.length()) {
					mtlImageName.append(me->map_Kd);
				}
				if(rf && rf->isMaterialOrImagePresent(mtlImageName)) {
					currentMaterialName = mtlImageName;
				} else {
					str imageFile = fname;
					imageFile.toDir();
					imageFile.append(mtlImageName);
					currentMaterialName = imageFile;
					//sf->setMaterial(imageFile);
				}
			} else {
				currentMaterialName = mtlName;
				//sf->setMaterial(mtlName);
			}
			lastMtl = mtlName;
		} else if(p.atWord("mtllib")) {
			str mtlName = p.getToken();
			str fullPath = fname;
			fullPath.toDir();
			fullPath.append(mtlName);
			if(mtlFile.loadMTL(fullPath)) {
				g_core->RedWarning("rModel_c::loadOBJ: cannot load mtl file %s\n",fullPath.c_str());
			}
		} else {
			g_core->RedWarning("rModel_c::loadOBJ: unknown token %s in .OBJ file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
		}
	}
	return false;
}