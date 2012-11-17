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
#include "skelModelImpl.h"
#include <api/coreAPI.h>
#include <shared/parser.h>
#include <shared/hashTableTemplate.h>
#include <math/quat.h>

class stringRegister_c {
	struct strEntry_s {
		str name;
		u32 index;
		strEntry_s *hashNext;
		inline const char *getName() const {
			return name;
		}
		inline strEntry_s *getHashNext() const {
			return hashNext;
		}
		inline void setHashNext(strEntry_s *newNext) {
			hashNext = newNext;
		}
	};
	hashTableTemplateExt_c<strEntry_s> table;

public:
	u32 registerString(const char *str) {
		strEntry_s *e = table.getEntry(str);
		if(e == 0) {
			e = new strEntry_s;
			e->name = str;
			e->index = table.size();
		}
		table.addObject(e);
		return e->index;
	}
};

static stringRegister_c sk_boneNames;

u32 SK_RegisterString(const char *s) {
	return sk_boneNames.registerString(s);
}

skelModelIMPL_c::skelModelIMPL_c() {
	curScale.set(1.f,1.f,1.f);
}
skelModelIMPL_c::~skelModelIMPL_c() {
	bones.clear();
	baseFrameABS.clear();
	surfs.clear();
}
void skelModelIMPL_c::scaleXYZ(float scale) {
	baseFrameABS.scale(scale);
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->scaleXYZ(scale);
	}
	curScale *= scale;
}
void skelModelIMPL_c::swapYZ() {
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		//sf->swapYZ();
	}
}
void skelModelIMPL_c::translateY(float ofs) {

}
void skelModelIMPL_c::multTexCoordsY(float f) {

}
void skelModelIMPL_c::translateXYZ(const class vec3_c &ofs) {

}
void skelModelIMPL_c::getCurrentBounds(class aabb &out) {

}
void skelModelIMPL_c::setAllSurfsMaterial(const char *newMatName) {
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++) {
		sf->setMaterial(newMatName);
	}
}
bool skelModelIMPL_c::loadMD5Mesh(const char *fname) {
	// md5mesh files are a raw text files, so setup the parsing
	parser_c p;
	if(p.openFile(fname) == true) {
		g_core->RedWarning("skelModelIMPL_c::loadMD5Mesh: cannot open %s\n",fname);
		return true;
	}

	if(p.atWord("MD5Version")==false) {
		g_core->RedWarning("skelModelIMPL_c::loadMD5Mesh: expected \"MD5Version\" string at the beggining of the file, found %s, in file %s\n",
			p.getToken(),fname);
		return true; // error
	}
	u32 version = p.getInteger();
	if(version != 10) {
		g_core->RedWarning("skelModelIMPL_c::loadMD5Mesh: bad MD5Version %i, expected %i, in file %s\n",version,10,fname);
		return true; // error
	}
	u32 numMeshesParsed = 0;
	while(p.atEOF() == false) {
		if(p.atWord("commandline")) {
			const char *commandLine = p.getToken();

		} else if(p.atWord("numJoints")) {
			u32 i = p.getInteger();
			this->bones.resize(i);
			this->baseFrameABS.resize(i);
		} else if(p.atWord("numMeshes")) {
			u32 i = p.getInteger();
			this->surfs.resize(i);
		} else if(p.atWord("joints")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"joints\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
			boneDef_s *b = this->bones.getArray();
			boneOr_s *bfb = this->baseFrameABS.getArray();
			for(u32 i = 0; i < bones.size(); i++, b++, bfb++) {
				const char *s = p.getToken();
				b->nameIndex = SK_RegisterString(s);
				b->parentIndex = p.getInteger();
				vec3_c pos;
				quat_c quat;
				p.getFloatMat_braced(pos,3);
				p.getFloatMat_braced(quat,3); // fourth component, 'W', can be automatically computed
				quat.calcW();
				bfb->mat.fromQuatAndOrigin(quat,pos);
			}
			b = this->bones.getArray();
			//for(u32 i = 0; i < bones.size(); i++, b++) {
			//	if(b->parentIndex != -1) {
			//		b->parentName = bones[b->parentIndex].nameIndex;
			//	} else {
			//		b->parentName = SKEL_INDEX_WORLDBONE;
			//	}
			//}
			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"joints\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else if(p.atWord("mesh")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"mesh\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}

			skelSurfIMPL_c &sf = this->surfs[numMeshesParsed];

			if(p.atWord("shader")) {
				sf.matName = p.getToken();
			} else {
				sf.matName = ("nomaterial");
			}

			//
			//		VERTICES
			//
			if(p.atWord("numVerts") == false) {
				g_core->RedWarning("Expected \"numVerts\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}

			u32 numVerts = p.getInteger();

			sf.verts.resize(numVerts);

			skelVert_s *v = sf.verts.getArray();
			for(u32 i = 0; i < numVerts; i++, v++) {
				if(p.atWord("vert") == false) {
					g_core->RedWarning("Expected \"vert\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				u32 checkIndex = p.getInteger();
				if(checkIndex != i) {
					g_core->RedWarning("Expected vertex index %i at line %i of %s, found %s\n",i,p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				if(p.getFloatMat_braced(v->tc,2)) {
					g_core->RedWarning("Failed to read 2d textcoord vector at line %i of %s\n",p.getCurrentLineNumber(),fname);
					return true; // error
				}
				v->firstWeight = p.getInteger();
				v->numWeights = p.getInteger();
			}
		

			//
			//		TRIANGLES
			//
			if(p.atWord("numtris") == false) {
				g_core->RedWarning("Expected \"numtris\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}

			u32 numTris = p.getInteger();
			u32 numIndexes = numTris * 3;
			sf.indices.resize(numIndexes);
			u32 triNum = 0;
			for(u32 i = 0; i < numIndexes; i+=3, triNum++) {
				// 	tri 0 2 1 0
				if(p.atWord("tri") == false) {
					g_core->RedWarning("Expected \"tri\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				u32 checkIndex = p.getInteger();
				if(checkIndex != triNum) {
					g_core->RedWarning("Expected triangle index %i at line %i of %s, found %s\n",triNum,p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				// read triangle indices
				sf.indices[i+0] = p.getInteger();
				sf.indices[i+1] = p.getInteger();
				sf.indices[i+2] = p.getInteger();
			}

			//
			//		WEIGHTS
			//
			if(p.atWord("numweights") == false) {
				g_core->RedWarning("Expected \"numweights\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}

			u32 numFileWeights = p.getInteger();
			sf.weights.resize(numFileWeights);
			skelWeight_s *w = sf.weights.getArray();
			for(u32 i = 0; i < numFileWeights; i++, w++) {
				// weight 0 42 0.883179605 ( 2.3967983723 0.3203794658 -2.0581412315 )
				if(p.atWord("weight") == false) {
					g_core->RedWarning("Expected \"weight\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				u32 checkIndex = p.getInteger();
				if(checkIndex != i) {
					g_core->RedWarning("Expected triangle index %i at line %i of %s, found %s\n",triNum,p.getCurrentLineNumber(),fname,p.getToken());
					return true; // error
				}
				w->boneIndex = p.getInteger();
				w->boneName = this->bones[w->boneIndex].nameIndex;
				w->weight = p.getFloat();
				p.getFloatMat_braced(w->ofs,3);
			}
		
			numMeshesParsed++;

			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"mesh\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else {
			g_core->RedWarning("Unknown token %s in md5mesh file %s\n",p.getToken(),fname);
		}
	}

	this->name = fname;

	return false; // no error
}