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
// hl2MDLReader.cpp
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include "hl2MDLReader.h"
#include <fileFormats/vvdFileFormat.h>
#include <api/staticModelCreatorAPI.h>

#pragma pack(1)

struct mdlHeader_s {
	int ident;
	int version;
};

struct mdlBodyParts_s {
	int	ofsName;
	int	numModels;
	int	base;
	int	ofsModels;

	inline char * const pName() const {
		return ((char *)this) + ofsName;
	}
};
struct mdlModelHeader_s {
	char name[64];
	int type;
	float boundingRadius;
	int	numMeshes;
	int	ofsMeshes;
	int	numVertices;
	int	ofsVertices;
	int	ofsTangents;
	// NOTE: there are more data fields after ofsTangents but they are version-dependent
};
struct mdlMeshHeader_s {
	int matIndex;
	int ofsModel; // negative
	int	numVertices;
	int	ofsVertices;
};
struct mdlV44BoneWeights_s {
	float	weight[3];
	char	bone[3]; 
	byte	numBones;
};
struct mdlV44Vertex_s {
	mdlV44BoneWeights_s boneWeights;
	vec3_t	pos;
	vec3_t	normal;
	vec2_t	tc;
};
// .vtx file structures for version 6 and 7
struct vtxStripHeader_s {
	int numIndices;
	int ofsIndices;
	int numVerts;
	int ofsVerts;
	short numBones;  
	unsigned char flags;
	int numBoneStateChanges;
	int boneStateChangeOffset;
};
struct vtx7Vertex_s {
	unsigned char boneWeightIndex[3];
	unsigned char numBones;
	short origMeshVertID;
	char boneID[3];
};
struct vtxStripGroupHeader_s {
	int numVerts;
	int ofsVerts;
	int numIndices;
	int ofsIndices;
	int numStrips;
	int ofsStrips;
	unsigned char flags;

	inline const vtxStripHeader_s *pStrip(int i) const { 
		return (const vtxStripHeader_s *)(((byte *)this) + ofsStrips) + i; 
	};
	inline const u16 *pIndices() const { 
		return (const u16 *)(((const byte *)this) + ofsIndices); 
	};
	inline const vtx7Vertex_s *pVertexV7(int i) const  { 
		return (const vtx7Vertex_s *)(((const byte *)this) + ofsVerts) + i; 
	};
};
struct vtxMeshHeader_s {
	int numStripGroups;
	int ofsStripGroups;
	unsigned char flags;

	inline const vtxStripGroupHeader_s *pStripGroup(int i) const { 
		return (vtxStripGroupHeader_s *)(((byte *)this) + ofsStripGroups) + i; 
	};
};

struct vtxModelLODHeader_s {
	int numMeshes;
	int ofsMeshes;
	float switchPoint;

	inline const vtxMeshHeader_s *pMesh(int i) const  { 
		return (const vtxMeshHeader_s *)(((const byte *)this) + ofsMeshes) + i; 
	};
};
struct vtxModelHeader_s {
	int numLODs;
	int ofsLODs;

	inline const vtxModelLODHeader_s *pLOD(int i) const { 
		return (const vtxModelLODHeader_s *)(((const byte *)this) + ofsLODs) + i; 
	};
};

struct vtxBodyPartHeader_s {
	int numModels;
	int ofsModels;

	inline const vtxModelHeader_s *pModel(int i) const { 
		return (const vtxModelHeader_s *)(((const byte *)this) + ofsModels) + i;
	};
};
struct vtxFileHeader_s {
	int version;
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerTri;
	int maxBonesPerVert;
	long checkSum;
	int numLODs;
	int ofsMaterialReplacementLists;
	int numBodyParts;
	int ofsBodyParts;

	inline const vtxBodyPartHeader_s *pBodyPart(int i) const {
		return (const vtxBodyPartHeader_s *)(((const byte *)this) + ofsBodyParts) + i;
	};	
};
#pragma pack()
class vtxFile_c {
	readStream_c data;
	u32 version;
	const vtxFileHeader_s *h;
public:
	bool preload(const char *fname) {
		if(data.loadFromFile(fname)) {
			str fixed = fname;
			fixed.stripExtension();
			fixed.append(".dx90.vtx");
			if(data.loadFromFile(fixed)) {
				fixed = fname;
				fixed.stripExtension();
				fixed.append(".dx80.vtx");
				if(data.loadFromFile(fixed)) {
					g_core->RedWarning("vtxFile_c::preload: cannot open %s\n",fname);
					return true; // error
				}
			}
		}
		h = (const vtxFileHeader_s*)data.getDataPtr();

		return false;
	}
	void getMeshIndices(arraySTD_c<u16> &out, u32 bodyPartNum, u32 modelNum, u32 meshNum) const {
		const vtxBodyPartHeader_s *bodyPart = h->pBodyPart(bodyPartNum);
		const vtxModelHeader_s *model = bodyPart->pModel(modelNum);
		const vtxMeshHeader_s *mesh = model->pLOD(0)->pMesh(meshNum);
		for(u32 i = 0; i < mesh->numStripGroups; i++) {
			const vtxStripGroupHeader_s *stripGroup = mesh->pStripGroup(i);
			const u16 *sIndices = stripGroup->pIndices();
			for(u32 j = 0; j < stripGroup->numStrips; j++) {
				const vtxStripHeader_s *strip = stripGroup->pStrip(j);
				for(u32 k = 0; k < strip->numIndices; k+=3) {
					u32 i0 = sIndices[strip->ofsIndices+k];
					u32 i1 = sIndices[strip->ofsIndices+k+1];
					u32 i2 = sIndices[strip->ofsIndices+k+2];
#if 1
					i0 = stripGroup->pVertexV7(i0)->origMeshVertID;
					i1 = stripGroup->pVertexV7(i1)->origMeshVertID;
					i2 = stripGroup->pVertexV7(i2)->origMeshVertID;
#endif
					out.push_back(i0);
					out.push_back(i1);
					out.push_back(i2);
				}
			}
		}
	}
};
hl2MDLReader_c::hl2MDLReader_c() {
	vvd = 0;
}
hl2MDLReader_c::~hl2MDLReader_c() {

}
bool hl2MDLReader_c::beginReading(const char *fname) {
	if(data.loadFromFile(fname)) {
		g_core->RedWarning("hl2MDLReader_c::beginReading: cannot open %s\n",fname);
		return true; // error
	}
	this->name = fname;
	const mdlHeader_s *h = (const mdlHeader_s*)data.getDataPtr();
	this->version = h->version;
	if(readMatNames()) {
		return true;
	}	
	
	str vvdFileName = fname;
	vvdFileName.setExtension("vvd");
	vvdFileLen = g_vfs->FS_ReadFile(vvdFileName,(void**)&vvd);
	if(vvd == 0) {
		g_core->RedWarning("hl2MDLReader_c::beginReading: cannot open %s\n",vvdFileName.c_str());
		return true; // error
	}

	return false; // no error
}

bool hl2MDLReader_c::readMatNames() {
	data.setPos(204);
	u32 numMaterials = data.readU32();
	u32 ofsMaterials = data.readU32();
	u32 numMaterialPaths = data.readU32();
	u32 ofsMaterialPaths = data.readU32();
	data.setPos(ofsMaterialPaths);
	u32 ofsPathString = data.readU32();
	const char *materialPath = ((const char*)data.getDataPtr()) + ofsPathString;
	data.setPos(ofsMaterials);
	matNames.resize(numMaterials);
	for(u32 i = 0; i < numMaterials; i++) {
		u32 saved = data.getPos();
		u32 localMatNameOfs = data.readU32();
		data.setPos(saved + localMatNameOfs);
		const char *matName = (const char*)data.getCurDataPtr();
		matNames[i] = materialPath;
		matNames[i].append(matName);
		matNames[i].backSlashesToSlashes();
		data.setPos(saved + 64);
	}
	return false; // no error
}
bool hl2MDLReader_c::getStaticModelData(class staticModelCreatorAPI_i *out) {
	data.setPos(232);

	u32 numBodyParts = data.readU32();
	u32 bodyPartsOfs = data.readU32();

	vtxFile_c vtx;
	str vtxName = this->name;
	vtxName.setExtension("vtx");
	if(vtx.preload(vtxName)) {
		return true; // error
	}

	data.setPos(bodyPartsOfs);
	const mdlBodyParts_s *bodyParts = (const mdlBodyParts_s *)data.getCurDataPtr();
	// for each bodypart
	for(u32 i = 0; i < numBodyParts; i++) {
		const mdlBodyParts_s &bp = bodyParts[i];
		g_core->Print("Bodyparts name: %s\n",bp.pName());
		u32 bpOfs = data.pointerToOfs(&bp);
		const char *name = bp.pName();
		u32 firstModelOfs = bpOfs + bp.ofsModels;
		data.setPos(firstModelOfs);
		// for each model
		for(u32 j = 0; j < bp.numModels; j++) {
			u32 modelHeaderAt = data.getPos();
			const mdlModelHeader_s *mh = (const mdlModelHeader_s *)data.getCurDataPtr();
			g_core->Print("ModelName: %s, numMeshes %i\n",mh->name,mh->numMeshes);
			u32 firstMeshOfs = modelHeaderAt + mh->ofsMeshes;
			data.setPos(firstMeshOfs);
			// for each mesh
			for(u32 k = 0; k < mh->numMeshes; k++) {
				u32 meshHeaderAt = data.getPos();
				const mdlMeshHeader_s *mesh = (const mdlMeshHeader_s*)data.getCurDataPtr();
				const char *meshMaterial = this->getMatName(mesh->matIndex);
				str fullMatName = meshMaterial;
				fullMatName.setExtension("vmt");
				
				const mdlV44Vertex_s *baseVertices = (const mdlV44Vertex_s*)vvd->getVertexData();
				const mdlV44Vertex_s *meshVertices = baseVertices + mesh->ofsVertices;
				const mdlV44Vertex_s *v = meshVertices;

				arraySTD_c<u16> indices;
				vtx.getMeshIndices(indices,i,j,k);
				for(u32 l = 0; l < indices.size(); l+= 3) {
					u32 i0 = indices[l+0];
					u32 i1 = indices[l+1];
					u32 i2 = indices[l+2];
					simpleVert_s v0,v1,v2;
					v0.setXYZ(meshVertices[i0].pos);
					v0.setUV(meshVertices[i0].tc);
					v1.setXYZ(meshVertices[i1].pos);
					v1.setUV(meshVertices[i1].tc);
					v2.setXYZ(meshVertices[i2].pos);
					v2.setUV(meshVertices[i2].tc);
					out->addTriangle(fullMatName,v0,v1,v2);
				}

				data.setPos(meshHeaderAt + 116);
			}
			data.setPos(modelHeaderAt + 148);
		}
	}
	return false; // no error
}

u32 hl2MDLReader_c::getNumMaterials() const {
	return matNames.size();
}
const char *hl2MDLReader_c::getMatName(u32 matIndex) const {
	return matNames[matIndex];
}