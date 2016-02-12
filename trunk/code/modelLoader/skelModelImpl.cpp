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
#include <api/vfsAPI.h>
#include <shared/parser.h>
#include <shared/hashTableTemplate.h>
#include <shared/stringHashTable.h>
#include <shared/extraSurfEdgesData.h>
#include <fileFormats/actorX_file_format.h>
#include <fileFormats/mdm_file_format.h>
#include <fileFormats/mds_file_format.h>
#include <fileFormats/md5r_file_format.h>
#include <math/quat.h>

static stringRegister_c sk_boneNames;

u32 SK_RegisterString(const char *s) {
	return sk_boneNames.registerString(s);
}
const char *SK_GetString(u32 idx) {
	return sk_boneNames.getString(idx);
}

skelSurfIMPL_c::skelSurfIMPL_c() {
	edgesData = 0;
}
skelSurfIMPL_c::~skelSurfIMPL_c() {
	if(edgesData) {
		delete edgesData;
	}
}
bool skelSurfIMPL_c::compareWeights(u32 wi0, u32 wi1) const {
	const skelWeight_s &w0 = weights[wi0];
	const skelWeight_s &w1 = weights[wi1];
	//if(abs(w0.weight-w1.weight) > 0.0001f)
	if(w0.weight != w1.weight)
		return false;
	if(w0.boneIndex != w1.boneIndex)
		return false;
	if(w0.ofs.compare(w1.ofs)==false)
		return false;
	return true;
}
bool skelSurfIMPL_c::compareVertexWeights(u32 i0, u32 i1) const {
	const skelVert_s &v0 = verts[i0];
	const skelVert_s &v1 = verts[i1];
	if(v0.numWeights != v1.numWeights)
		return false;
	for(u32 i = 0; i < v1.numWeights; i++) {
		if(compareWeights(v0.firstWeight + i, v1.firstWeight + i) == false) {
			return false;
		}
	}
	return true;
}
void skelSurfIMPL_c::calcEqualPointsMapping(arraySTD_c<u16> &mapping) {
	mapping.resize(verts.size());
	mapping.setMemory(0xff);
	u32 c_aliases = 0;
	for(u32 i = 0; i < verts.size(); i++) {
		if(mapping[i] != 0xffff)
			continue;
		for(u32 j = i + 1; j < verts.size(); j++) {
			if(compareVertexWeights(i,j)) {
				mapping[j] = i;
				//g_core->Print("skelSurfIMPL_c::calcEqualPointsMapping: vertex %i is an alias of %i\n",j,i);
				c_aliases++;
			}
		}
		mapping[i] = i;
	}
	//g_core->Print("skelSurfIMPL_c::calcEqualPointsMapping: %i verts, %i alias\n",verts.size(),c_aliases);
}
u32 skelSurfIMPL_c::addVertex(const class texturedVertex_c &a) {
	skelVert_s &v = verts.pushBack();
	v.firstWeight = weights.size();
	v.numWeights = 1;
	skelWeight_s &w = weights.pushBack();
	w.boneIndex = 0;
	w.ofs = a.xyz;
	w.weight = 1.f;
	v.tc = a.st;
	return verts.size()-1;
}
void skelSurfIMPL_c::addTriangle(const class texturedVertex_c &a, const class texturedVertex_c &b, const class texturedVertex_c &c) {
	indices.push_back(addVertex(a));
	indices.push_back(addVertex(b));
	indices.push_back(addVertex(c));
}
void skelSurfIMPL_c::calcEdges() {
	arraySTD_c<u16> mapping;
	calcEqualPointsMapping(mapping);
	u32 triNum = 0;
	if(edgesData) {
		delete edgesData;
	}
	edgesData = new extraSurfEdgesData_s;
	for(u32 i = 0; i < indices.size(); i+=3, triNum++) {
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		i0 = mapping[i0];
		i1 = mapping[i1];
		i2 = mapping[i2];
		edgesData->addEdge(triNum,i0,i1);
		edgesData->addEdge(triNum,i1,i2);
		edgesData->addEdge(triNum,i2,i0);
	}
	g_core->Print("skelSurfIMPL_c::calcEdges: %i edges (%i unmatched) from %i triangles\n",edgesData->edges.size(),
		(edgesData->edges.size()-edgesData->c_matchedEdges),indices.size()/3);
}

skelModelIMPL_c::skelModelIMPL_c() {
	curScale.set(1.f,1.f,1.f);
}
skelModelIMPL_c::~skelModelIMPL_c() {
	bones.clear();
	baseFrameABS.clear();
	surfs.clear();
	g_core->Print("Freeing skelModel %s\n",this->name.c_str());
}
void skelModelIMPL_c::printBoneNames() const {
	for(u32 i = 0; i < bones.size(); i++) {
		g_core->Print("%i/%i: %s (%i)\n",i,bones.size(),SK_GetString(bones[i].nameIndex),bones[i].nameIndex);
	}
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
void skelModelIMPL_c::swapIndexes() {
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->swapIndexes();
	}
}
void skelModelIMPL_c::translateY(float ofs) {

}
void skelModelIMPL_c::multTexCoordsY(float f) {

}
void skelModelIMPL_c::multTexCoordsXY(float f) {

}
void skelModelIMPL_c::translateXYZ(const class vec3_c &ofs) {

}
void skelModelIMPL_c::transform(const class matrix_c &mat) {

}
void skelModelIMPL_c::getCurrentBounds(class aabb &out) {

}
void skelModelIMPL_c::setAllSurfsMaterial(const char *newMatName) {
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++) {
		sf->setMaterial(newMatName);
	}
}
void skelModelIMPL_c::setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName) {
	for(u32 i = 0; i < numSurfIndexes; i++) {
		u32 sfNum = surfIndexes[i];
		surfs[sfNum].setMaterial(newMatName);
	}
}
void skelModelIMPL_c::recalcEdges() {
	skelSurfIMPL_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->calcEdges();
	}
}

void CalcTriangleTN(const vec3_c &v0xyz, const vec3_c &v1xyz, const vec3_c &v2xyz, 
					const vec2_c &v0tc, const vec2_c &v1tc, const vec2_c &v2tc, vec3_c &normal, vec3_c &tangent) {	
	// normal part
	vec3_c edge0 = v2xyz - v0xyz;
	vec3_c edge1 = v1xyz - v0xyz;
	normal = edge0.crossProduct(edge1);

	vec2_c st0 = v2tc - v0tc;
	vec2_c st1 = v1tc - v0tc;

	float coef = 1.f / (st0.getX() * st1.getY() - st1.getX() * st0.getY());

	tangent.setX(coef * ((edge0.getX() * st1.getY())  + (edge1.getX() * -st0.getY())));
	tangent.setY(coef * ((edge0.getY() * st1.getY())  + (edge1.getY() * -st0.getY())));
	tangent.setZ(coef * ((edge0.getZ() * st1.getY())  + (edge1.getZ() * -st0.getY())));
}
skelSurfIMPL_c *skelModelIMPL_c::registerSurface(const char *matName) {
	for(u32 i = 0; i < surfs.size(); i++) {
		if(stricmp(surfs[i].matName,matName)==0)
			return &surfs[i];
	}
	skelSurfIMPL_c ns ;
	ns.setMaterial(matName);
	surfs.push_back(ns);
	return &surfs[surfs.size()-1];
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
//			w->boneName = this->bones[w->boneIndex].nameIndex;
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

	// precalc TBN
	for(u32 i = 0; i < surfs.size(); i++) {
		skelSurfIMPL_c &sf = surfs[i];
		arraySTD_c<vec3_c> positions;
		positions.resize(sf.verts.size());
		// calc positions
		for(u32 j = 0; j < sf.verts.size(); j++) {
			skelVert_s &v = sf.verts[j];
			v.n.zero();
			v.t.zero();
			vec3_c &xyz = positions[j];
			for(u32 k = 0; k < v.numWeights; k++) {
				const skelWeight_s &w = sf.weights[v.firstWeight+k];
				vec3_c p;
				baseFrameABS[w.boneIndex].mat.transformPoint(w.ofs,p);
				xyz += p * w.weight;
			}
		}
		// calc tbn
		for(u32 j = 0; j < sf.indices.size(); j+=3) {
			u32 i0 = sf.indices[j+0];
			u32 i1 = sf.indices[j+1];
			u32 i2 = sf.indices[j+2];
			const vec3_c &pA = positions[i0];
			const vec3_c &pB = positions[i1];
			const vec3_c &pC = positions[i2];
			skelVert_s &vA = sf.verts[i0];
			skelVert_s &vB = sf.verts[i1];
			skelVert_s &vC = sf.verts[i2];
			vec3_c n, t;
			CalcTriangleTN(pA,pB,pC,vA.tc,vB.tc,vC.tc,n,t);
			vA.n += n;
			vB.n += n;
			vC.n += n;
			vA.t += t;
			vB.t += t;
			vC.t += t;
		}
		//arraySTD_c<matrix_c> mats;
		//mats.resize(baseFrameABS.size());
		//for(u32 i = 0; i < mats.size(); i++) {
		//	matrix_c &m = mats[i];
		//	const boneOr_s &o = baseFrameABS[i];
		//	m.fromQuatAndOrigin(b
		//}
		boneOrArray_c inv = baseFrameABS.getInversed();
		// calc locals
		for(u32 j = 0; j < sf.verts.size(); j++) {
			skelVert_s &v = sf.verts[j];
			v.t.normalize();
			v.n.normalize();
			const skelWeight_s &fw = sf.weights[v.firstWeight];
			const matrix_c &im = inv[fw.boneIndex].mat;
			im.transformNormal(v.t);
			im.transformNormal(v.n);
		}
	}
	this->name = fname;

	return false; // no error
}
struct smdLink_s {
	int bone;
	float weight;
};


bool skelModelIMPL_c::loadSMD(const char *fname) {
	this->name = fname;

	// .smd files are a raw text files, so setup the parsing
	parser_c p;
	p.openFile(fname);

	if(p.atWord("version")==false) {
		g_core->Print("skelModel_c::loadSMD: expected \"version\" string at the beggining of the file, found %s, in file %s\n",
			p.getToken(),fname);
		return true; // error
	}
	u32 version = p.getInteger();
	if(version != 1) {
		g_core->Print("skelModel_c::loadSMD: bad version %i, expected %i, in file %s\n",version,1,fname);
		return true; // error
	}
	boneOrArray_c baseFrameInv;
	u32 numNodesParsed = 0;
	while(p.atEOF()==false) {
		if(p.atWord("nodes")) {
			while(p.atWord("end") == false && p.atEOF() == false) {
				u32 checkIndex = p.getInteger();
				if(checkIndex != numNodesParsed) {
					g_core->Print("Expected node index %i at line %i of %s, found %s\n",numNodesParsed,p.getCurrentLineNumber(),fname,p.getToken());
				}
				str nodeName = p.getToken();
				int parentIndex = p.getInteger();

				boneDef_s b;
				b.nameIndex = SK_RegisterString(nodeName);
				b.parentIndex = parentIndex;

				numNodesParsed++;

				this->bones.push_back(b);
			}
			g_core->Print("%i smd nodes (bones) loaded\n",this->bones.size());
		} else if(p.atWord("skeleton") && p.atEOF() == false) {
			bool stop = false;
			boneOrArray_c baseFrameRelative;
			while(stop == false) {
				if(p.atWord("time") == false) {
					g_core->Print("Expected \"time\" to follow \"skeleton\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
					return true;
				}
				atTimeToken:
				float timeVal = p.getFloat();
				baseFrameRelative.resize(this->bones.size());
				while(1) {
					// <int|bone ID> <float|PosX PosY PosZ> <float|RotX RotY RotZ>
					u32 id = p.getInteger();
					vec3_c pos;
					p.getFloatMat(pos,3);
					vec3_c rot;
					p.getFloatMat(rot,3);
					if(timeVal == 0) {
						vec3_c angles( RAD2DEG(rot.y), RAD2DEG(rot.z), RAD2DEG(rot.x) );
					//	angles *= -1;
						baseFrameRelative[id].mat.fromAnglesAndOrigin(angles,pos);
					}
					if(p.atWord("time")) {
						goto atTimeToken; 
					} else if(p.atWord("end")) {
						stop = true;
						break;
					}
				}
			}
			baseFrameABS = baseFrameRelative;
			baseFrameABS.localBonesToAbsBones(&bones);
			baseFrameInv = baseFrameABS;
			baseFrameInv.inverse();
		} else if(p.atWord("triangles")) {
			while(p.atWord("end") == false && p.atEOF() == false) {
				str materialName = p.getToken();
				skelSurfIMPL_c *sf = this->registerSurface(materialName.c_str());

				skelVert_s tri[3];
				// 0 -328.499207 -69.337677 1692.571899 -0.216319 1.422574 1.000000 0.000000 0.000000 0
				// <int|Parent bone> <float|PosX PosY PosZ> <normal|NormX NormY NormZ> <normal|U V> <int|links> <int|Bone ID> <normal|Weight> [...]
				// The final three values are optional: they override
				// <Parent bone> to define a series of weightmap links. 
				// Bone ID and Weight are repeated for each link.
				// If the weights do not add up to 1,
				// any remaining value is placed on <Parent bone>.
				for(u32 i = 0; i < 3; i++) {
					skelVert_s &ov = tri[i];
					ov.firstWeight = sf->weights.size();
					int parentBone = p.getInteger();
					vec3_c pos;
					p.getFloatMat(pos,3);
					vec3_c norm;
					p.getFloatMat(norm,3);
					vec2_c tc;
					p.getFloatMat(tc,2);
					// there is no "numLinks" token in airboat_LOD_1.smd from source sdk,
					// it looks like that keyword is optional
					u32 numLinks = 0;
					arraySTD_c<smdLink_s> links;
					float linksWSum = 0;
					if(p.isAtEOL() == false) {
						numLinks = p.getInteger();
						if(numLinks) {
							for(u32 l = 0; l < numLinks; l++) {
								smdLink_s link;
								link.bone = p.getInteger();
								link.weight = p.getFloat();
								linksWSum += link.weight;
								links.push_back(link);
							}
						}
					}
					if(numLinks == 0) {
						smdLink_s missing;
						missing.weight = 1.f;
						missing.bone = parentBone;
						links.push_back(missing);
						numLinks++;
					} else if(abs(linksWSum-1.f) > 0.05) {
						smdLink_s missing;
						missing.weight = 1.f - linksWSum;
						missing.bone = parentBone;
						links.push_back(missing);
						numLinks++;
					}
					for(u32 l = 0; l < numLinks; l++) {
						int boneIndex = links[l].bone;
						vec3_c posLocal;
						const matrix_c &m = baseFrameInv[boneIndex].mat;
						posLocal = pos;
						m.transformPoint(posLocal);

						skelWeight_s sw;
						sw.boneIndex = boneIndex;
						sw.ofs = posLocal;
						sw.weight = links[l].weight;
						if(sw.weight < 0.0001) {
							g_core->Print("skelModel_c::loadSMD: bone weight lower than 0.00001, ignoring\n");
							continue;
						}
						sf->weights.push_back(sw);
					}
					// calc normal
				
					int boneIndex = links[0].bone;
					vec3_c normLocal;
					const matrix_c &m = baseFrameInv[boneIndex].mat;
					m.transformNormal(norm,normLocal);
					ov.n = normLocal;

					ov.tc = tc;
					tri[i].numWeights = sf->weights.size()-tri[i].firstWeight;
				}
				for(int idx = 2; idx >= 0; idx--) {
					sf->indices.push_back(sf->verts.size());
					sf->verts.push_back(tri[idx]);
				}
			}
		} else {
			g_core->Print("Unknown token %s in smd file %s\n",p.getToken(),fname);
		}
	}
	for(u32 i = 0; i < bones.size(); i++) {
		baseFrameABS[i].boneName = bones[i].nameIndex;
	}
	g_core->Print("SMD file %s has %i surfaces\n",fname,surfs.size());
	return false;
}

bool skelModelIMPL_c::loadMD5R(const char *fname) {
	this->name = fname;
	parser_c p;
	p.openFile(fname);

	this->name = fname;

	u32 memSize = 0;
	u32 MD5RVersion = 0;
	
	md5rArray_c<md5rVertexBuffer_c> vertexBuffers;
	md5rArray_c<md5rIndexBuffer_c> indexBuffers;
	md5rArray_c<md5rMesh_c> meshes;
	md5rSilhouetteEdgesArray_c silhouetteEdges;

	while(p.atEOF() == false) {
		if(p.atWord("MemSize")) {
			memSize = p.getInteger();
		} else if(p.atWord("MD5RVersion")) {
			MD5RVersion = p.getInteger();
		} else if(p.atWord_dontNeedWS("Joint")) {
			if(p.atWord_dontNeedWS("[")) {

			}
			const char *s = p.getToken("]");
			u32 numJoints = atoi(s);
			if(p.atWord_dontNeedWS("]")) {

			}
			g_core->RedWarning("skelModel_c::loadMD5R: %i joints\n",numJoints);
			if(p.atWord_dontNeedWS("{") == false) {
				g_core->RedWarning("skelModel_c::loadMD5R: expected '{' at the beginning of Joints block at line %i of %s, found %s\n",
					p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}
			bones.resize(numJoints);
			baseFrameABS.resize(numJoints);
			boneDef_s *b = this->bones.getArray();
			boneOr_s *bfb = this->baseFrameABS.getArray();
			for(u32 i = 0; i < numJoints; i++, b++, bfb++) {
				// "origin" -1  0 0 0  -0.5 -0.5 -0.5
				// boneName, parentIndex, ofs, quatXYZ
				const char *s = p.getToken();
				g_core->Print("MD5R joint name: %s\n",s);
				b->nameIndex = SK_RegisterString(s);
				b->parentIndex = p.getInteger();
				vec3_c pos;
				quat_c quat;
				p.getFloatMat(pos,3);
				p.getFloatMat(quat,3); // fourth component, 'W', can be automatically computed
				quat.calcW();
				bfb->mat.fromQuatAndOrigin(quat,pos);
			}
			if(p.atWord_dontNeedWS("}") == false) {
				g_core->RedWarning("skelModel_c::loadMD5R: expected '}' after the Joints block at line %i of %s, found %s\n",
					p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("VertexBuffer")) {
			if(vertexBuffers.parse(p)) {
				g_core->RedWarning("skelModel_c::loadMD5R: failed to parse VertexBufferArray. Cannot load %s\n",fname);
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("LevelOfDetail")) {
			if(p.atWord_dontNeedWS("[")) {
			}
			const char *s = p.getToken("]");
			u32 numLODs = atoi(s);
			if(p.atWord_dontNeedWS("]")) {

			}
			if(p.atWord_dontNeedWS("{") == false) {
				g_core->RedWarning("skelModel_c::loadMD5R: expected '{' at the beginning of LevelOfDetail block at line %i of %s, found %s\n",
					p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}
			for(u32 i = 0; i < numLODs; i++) {
				p.getFloat();
			}
			if(p.atWord_dontNeedWS("}") == false) {
				g_core->RedWarning("skelModel_c::loadMD5R: expected '}' after the VertexBffers block at line %i of %s, found %s\n",
					p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("IndexBuffer")) {
			if(indexBuffers.parse(p)) {
				g_core->RedWarning("skelModel_c::loadMD5R: failed to parse IndexBufferArray. Cannot load %s\n",fname);
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("SilhouetteEdge")) {
			if(silhouetteEdges.parse(p)) {	
				g_core->RedWarning("skelModel_c::loadMD5R: failed to parse SilhouetteEdges. Cannot load %s\n",fname);
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("Mesh")) {
			if(meshes.parse(p)) {
				g_core->RedWarning("skelModel_c::loadMD5R: failed to parse MeshArray. Cannot load %s\n",fname);
				return true; // error
			}
		} else if(p.atWord_dontNeedWS("Bounds")) {
			break;
		}
	}

	surfs.resize(meshes.size());
	for(u32 i = 0; i < meshes.size(); i++) {
		const md5rMesh_c &in = meshes[i];
		skelSurfIMPL_c &sf = surfs[i];
		sf.setMaterial(in.getMatName());
		for(u32 j = 0; j < in.getNumPrimBatches(); j++) {
			const md5rPrimBatch_c &b = in.getPrimBatch(j);
			const md5rVertexBuffer_c &verts = vertexBuffers[in.getDrawBuffers(0)];
			const md5rIndexBuffer_c &indices = indexBuffers[in.getDrawBuffers(1)];
			for(u32 k = 0; k < b.DrawIndexedTriList[3]; k++) {
				u32 i0 = indices.getIndex(k*3+0+b.DrawIndexedTriList[2]);
				u32 i1 = indices.getIndex(k*3+1+b.DrawIndexedTriList[2]);
				u32 i2 = indices.getIndex(k*3+2+b.DrawIndexedTriList[2]);
				if(i0 >= verts.size()) {
					continue;
				}
				if(i1 >= verts.size()) {
					continue;
				}
				if(i2 >= verts.size()) {
					continue;
				}
				texturedVertex_c v0, v1,v2;
				verts.getVertex(i0,v0);
				verts.getVertex(i1,v1);
				verts.getVertex(i2,v2);
				sf.addTriangle(v0,v1,v2);
			}
		}
	}
	return false;
}
bool skelModelIMPL_c::loadMDS(const char *fname) {
	this->name = fname;

	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}
	const mdsHeader_t *h = (const mdsHeader_t *)fileData;
	if(h->ident != MDS_IDENT) {
		g_core->RedWarning("MDM file %s header has bad ident\n",fname);
		return true;
	}
	if(h->version != MDS_VERSION) {
		g_core->RedWarning("MDM file %s has bad version %i, should be %i\n",fname,h->version,MDM_VERSION);
		return true;
	}

	this->surfs.resize(h->numSurfaces);
	skelSurfIMPL_c *s = this->surfs.getArray();
	for(u32 i = 0; i < h->numSurfaces; i++, s++) {
		const mdsSurface_t *sf = h->pSurface(i);

		s->surfName = sf->name;
		s->matName = sf->shader;

		s->indices.resize(sf->numTriangles*3);
		for(u32 j = 0; j < sf->numTriangles; j++) {
			const mdsTriangle_t *tri = sf->pTri(j);
			s->indices[j*3+0] = tri->indexes[0];
			s->indices[j*3+1] = tri->indexes[1];
			s->indices[j*3+2] = tri->indexes[2];
		}

		const mdsVertex_t *iv = sf->pFirstVert();
		s->verts.resize(sf->numVerts);
		skelVert_s *v = s->verts.getArray();
		for(u32 j = 0; j < sf->numVerts; j++, v++) {
			v->n = iv->normal;
			v->tc = iv->texCoords;
			v->numWeights = (iv->numWeights);
			v->firstWeight = s->weights.size();
			s->weights.resize(v->firstWeight + v->numWeights);
			skelWeight_s *w = &s->weights[v->firstWeight];
			for(u32 k = 0; k < iv->numWeights; k++, w++) {
				const mdsWeight_t *wi = iv->pWeight(k);
				w->boneIndex = wi->boneIndex;
				w->ofs = wi->offset;
				w->weight = wi->boneWeight;
			}
			iv = iv->pNextVert();
		}
	}

#if 0
	this->bones.resize(h->numBones);
	boneOr_s *bfb = this->baseFrameABS.getArray();
	boneDef_s *b = this->bones.getArray();
	for(u32 i = 0; i < h->numBones; i++, b++) {
		const mdsBoneInfo_t *bi = h->pBone(i);

		b->name = SK_RegisterString(bi->name);
		b->parentIndex = bi->parent;
	}
#endif

	return false; // no error


	return false;
}
bool skelModelIMPL_c::loadMDM(const char *fname) {
	this->name = fname;

	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}
	const mdmHeader_t *h = (const mdmHeader_t *)fileData;
	if(h->ident != MDM_IDENT) {
		g_core->RedWarning("MDM file %s header has bad ident\n",fname);
		return true;
	}
	if(h->version != MDM_VERSION) {
		g_core->RedWarning("MDM file %s has bad version %i, should be %i\n",fname,h->version,MDM_VERSION);
		return true;
	}

	this->surfs.resize(h->numSurfaces);
	skelSurfIMPL_c *s = this->surfs.getArray();
	for(u32 i = 0; i < h->numSurfaces; i++, s++) {
		const mdmSurface_t *sf = h->pSurface(i);

		s->surfName = sf->name;
		s->matName = sf->shader;

		s->indices.resize(sf->numTriangles*3);
		for(u32 j = 0; j < sf->numTriangles; j++) {
			const mdmTriangle_t *tri = sf->pTri(j);
			s->indices[j*3+0] = tri->indexes[0];
			s->indices[j*3+1] = tri->indexes[1];
			s->indices[j*3+2] = tri->indexes[2];
		}

		const mdmVertex_t *iv = sf->pFirstVert();
		s->verts.resize(sf->numVerts);
		skelVert_s *v = s->verts.getArray();
		for(u32 j = 0; j < sf->numVerts; j++, v++) {
			v->n = iv->normal;
			v->tc = iv->texCoords;
			v->numWeights = (iv->numWeights);
			v->firstWeight = s->weights.size();
			s->weights.resize(v->firstWeight + v->numWeights);
			skelWeight_s *w = &s->weights[v->firstWeight];
			for(u32 k = 0; k < iv->numWeights; k++, w++) {
				const mdmWeight_t *wi = iv->pWeight(k);
				w->boneIndex = wi->boneIndex;
				w->ofs = wi->offset;
				w->weight = wi->boneWeight;
			}
			iv = iv->pNextVert();
		}
	}

	//
	// There is NO bone data in mdm files!
	//
	//this->bones.resize(h->numBones);
	//boneData_s *b = this->bones.getArray();
	//for(u32 i = 0; i < h->numBones; i++, b++) {
	//	const mdmBoneInfo_t *bi = h->pBone(i);

	//	b->name = SK_RegisterString(bi->name);
	//	b->channelIndex[CHANNEL_POS] = SK_RegisterPOSChannelForBone(bi->name);
	//	b->channelIndex[CHANNEL_ROT] = SK_RegisterROTChannelForBone(bi->name);
	//	b->parentIndex = bi->parent;
	//}

	this->name = fname;

	return false; // no error

	return false;
}
bool skelModelIMPL_c::loadPSK(const char *fname) {	
	this->name = fname;

	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}
	
	const axChunkHeader_t *h = (const axChunkHeader_t*)fileData;
	if(h->hasIdent(PSK_IDENT_HEADER)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong ident %s, should be %s\n",fname,h->ident,PSK_IDENT_HEADER);
		return true; // error
	}
	const axChunkHeader_t *s;

	s = h->getNextHeader();
	if(s->hasIdent(PSK_IDENT_POINTS)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_POINTS);
		return true; // error
	}
	if(s->dataSize != sizeof(axPoint_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axPoint_t));
		return true; // error
	}
	const axChunkHeader_t *pSection = s;
	u32 numPoints = s->numData;
	arraySTD_c<vec3_c> points;
	points.resize(numPoints);
	vec3_c *p = points.getArray();
	for(u32 i = 0; i < numPoints; i++, p++) {
		const axPoint_t *pi = s->getPoint(i);
		p->set(pi->point[0],pi->point[1],pi->point[2]);
	}
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_VERTS)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_VERTS);
		return true; // error
	}
	if(s->dataSize != sizeof(axVertex_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axVertex_t));
		return true; // error
	}
	const axChunkHeader_t *vertSection = s;
	u32 numVerts = s->numData;

	for(u32 i = 0; i < numVerts; i++) {
		const axVertex_t *vi = s->getVertex(i);
		if(vi->pointIndex < 0 || vi->pointIndex >= numPoints) {
			g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has vertex with point index %i out of range <0,%i)\n",fname,vi->pointIndex,numPoints);
			return true; // error
		}
	}
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_FACES)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_FACES);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axTriangle_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axTriangle_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	const axChunkHeader_t *triSection = s;
	u32 numTris = s->numData;
	for(u32 i = 0; i < numTris; i++) {
		const axTriangle_t *ti = s->getTri(i);
		for(u32 j = 0; j < 3; j++) {
			if(ti->indexes[j] >= numVerts) {
				g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has triangle with vertex index %i (%i) out of range <0,%i)\n",fname,ti->indexes[j],j,numVerts);
				g_vfs->FS_FreeFile(fileData);
				return true; // error
			}
		}
	}
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_MATS)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_MATS);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axMaterial_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axMaterial_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	const axChunkHeader_t *matSection = s;
	u32 numMaterials = s->numData;
	if(0) {
		for(u32 i = 0; i < numMaterials; i++) {
			const axMaterial_t *ti = s->getMat(i);
			g_core->RedWarning("Mat %i name %s\n",i,ti->name);
		}
	}
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_REFS)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_REFS);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axReferenceBone_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axReferenceBone_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	u32 nj = s->numData;
	this->bones.resize(nj);
	this->baseFrameABS.resize(nj);
	boneOrArray_c baseFrameRelative;
	baseFrameRelative.resize(nj);
	for(u32 i = 0; i < nj; i++) {
		const axReferenceBone_t *bi = s->getBone(i);
		if(0) {
			g_core->RedWarning("Bone %i name %s\n",i,bi->name);
		}

		bones[i].nameIndex = SK_RegisterString(bi->name);
		bones[i].parentIndex = bi->parentIndex;
		if(i == 0)
			bones[i].parentIndex--;

		const axBone_t *or = &bi->bone;
		quat_c q;
		q = or->quat;
		if(i == 0) {
			// negate y only
			//q.setY(-q.getY());
		} else {
			// negate x,y,z
			q.conjugate();
		}
		vec3_c pos = or->position;
		baseFrameRelative[i].mat.fromQuatAndOrigin(q,pos);
		baseFrameRelative[i].boneName = bones[i].nameIndex;
	}
	baseFrameABS = baseFrameRelative;
	baseFrameABS.localBonesToAbsBones(&bones);
	boneOrArray_c baseFrameABSInverse = baseFrameABS.getInversed();
	
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_WEIGHTS)==false) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_WEIGHTS);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axBoneWeight_t)) {
		g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axBoneWeight_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	const axChunkHeader_t *wSection = s;
	u32 numBoneWeights = s->numData;
	for(u32 i = 0; i < numBoneWeights; i++) {
		const axBoneWeight_t *wi = s->getWeight(i);
		if(wi->pointIndex >= numPoints) {
			g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has boneWeight with point index %i out of range <0,%i)\n",fname,wi->pointIndex,numPoints);
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}		
		if(wi->boneIndex >= nj) {
			g_core->RedWarning("skelModelIMPL_c::loadPSK: psk file %s has boneWeight with bone index %i out of range <0,%i)\n",fname,wi->boneIndex,nj);
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}		
	}
	surfs.resize(numMaterials);
	for(u32 i = 0; i < numMaterials; i++) {
		const axMaterial_t *ti = matSection->getMat(i);
		surfs[i].matName = ti->name;
	}
	for(u32 i = 0; i < numTris; i++) {
		const axTriangle_t *ti = triSection->getTri(i);
		const axMaterial_t *mat = matSection->getMat(ti->materialIndex);
		skelSurfIMPL_c &sf = surfs[ti->materialIndex];
		const axVertex_t *v[3];

		// load vertex info
		arraySTD_c<const axBoneWeight_t*> weights[3];
		for(u32 j = 0; j < 3; j++) {
			v[j] = vertSection->getVertex(ti->indexes[j]);
			wSection->getPointWeights(v[j]->pointIndex,weights[j]);
		}
	
		vec3_c pointsABS[3];
		// convert it to our CSkelVert
		skelVert_s tri[3];
		for(u32 j = 0; j < 3; j++) {
			skelVert_s &ov = tri[j];

			ov.tc = v[j]->st;

			if(weights[0].size() == 0 && weights[0].size() == 0 && weights[0].size() == 0) {
				// this is for PSKX loading (static models psk - no weights...)
				vec3_c pXYZ = points[v[j]->pointIndex];
				ov.firstWeight = sf.weights.size();
				ov.numWeights = 1;
				skelWeight_s w;
				w.boneIndex = 0;
//			w.boneName = 0;
				w.ofs = pXYZ;
				w.weight = 1.f;
				sf.weights.push_back(w);
				pointsABS[j] = pXYZ;
			} else {
				ov.firstWeight = sf.weights.size();
				ov.numWeights = weights[j].size();
				sf.weights.resize(ov.firstWeight+weights[j].size());
				for(u32 k = 0; k < weights[j].size(); k++)
				{
					skelWeight_s &w = sf.weights[ov.firstWeight+k];
					const axBoneWeight_t *wi = weights[j][k];
					w.boneIndex = wi->boneIndex;
				///	w.boneName = bones[wi->boneIndex].nameIndex;
					w.weight = wi->weight;
					const axPoint_t *wPoint = pSection->getPoint(wi->pointIndex);
					pointsABS[j] = wPoint->point;
					const matrix_c &m = baseFrameABSInverse[w.boneIndex].mat;
					m.transformPoint(pointsABS[j],w.ofs);
				}
			}
		}
		vec3_c n, t;
		CalcTriangleTN(pointsABS[0],pointsABS[1],pointsABS[2],tri[0].tc,tri[1].tc,tri[2].tc,n,t);
		for(u32 j = 0; j < 3; j++) {
			skelWeight_s &w = sf.weights[tri[j].firstWeight];
			if(w.boneIndex < baseFrameABSInverse.size()) {
				const matrix_c &m = baseFrameABSInverse[w.boneIndex].mat;
				m.transformNormal(n,tri[j].n);
				m.transformNormal(t,tri[j].t);
			} else {
				tri[j].n = n;
				tri[j].t = t;
			}
		}
		// add to model
		sf.indices.push_back(sf.verts.size());
		sf.verts.push_back(tri[0]);
		sf.indices.push_back(sf.verts.size());
		sf.verts.push_back(tri[1]);
		sf.indices.push_back(sf.verts.size());
		sf.verts.push_back(tri[2]);
	}

	g_vfs->FS_FreeFile(fileData);

	return false; // no error
}