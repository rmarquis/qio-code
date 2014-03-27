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

// limits used to validate MDL data
// (they can be freely increased, they dont affect memory allocation at all)
#define MAX_MDL_MATERIALS 1024
#define MAX_MDL_BODYPARTS 512

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
struct mdlV37BoneWeight_s {
	float weight[4];
	short bone[4]; 
	short numBones;
	short material;
	short firstRef;
	short lastRef;
};
struct mdlV37Vertex_s {
	mdlV37BoneWeight_s m_BoneWeights;
	vec3_t	pos;
	vec3_t	normal;
	vec2_t	tc;
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
struct mdlV44VertexData {
	int dummy;
	int numLODVertexes[8];
};
struct mdlMeshHeader_s {
	int matIndex;
	int ofsModel; // negative
	int	numVertices;
	int	ofsVertices;

	const mdlV37Vertex_s *getVertices37() const {
		const byte *modelData = ((const byte*)this) + ofsModel;
		int ofsModelVertices = *((int*)(modelData+84));
		const byte *verticesData = modelData + ofsModelVertices;
		const mdlV37Vertex_s *vertices = (const mdlV37Vertex_s*)verticesData;
		return vertices + ofsVertices;
	}
	const mdlV44VertexData *getVertexData44() const {
		return (const mdlV44VertexData*)(((const byte*)this) + 36 + 12);
	}
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
struct vtx6Vertex_s {
	unsigned char boneWeightIndex[4];
	short boneID[4];
	short origMeshVertID;
	unsigned char numBones;
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
	inline const vtx6Vertex_s *pVertexV6(int i) const  { 
		return (const vtx6Vertex_s *)(((const byte *)this) + ofsVerts) + i; 
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
	void getMeshIndices(arraySTD_c<u16> &out, u32 bodyPartNum, u32 modelNum, u32 meshNum/*, u32 prevVertexCount*/) const {
		const vtxBodyPartHeader_s *bodyPart = h->pBodyPart(bodyPartNum);
		const vtxModelHeader_s *model = bodyPart->pModel(modelNum);
		const vtxMeshHeader_s *mesh = model->pLOD(0)->pMesh(meshNum);
		//for(u32 i = 0; i < meshNum; i++) {
		//	const vtxMeshHeader_s *pm = model->pLOD(0)->pMesh(i);
		//	for(u32 j = 0; j < pm->numStripGroups; j++) {
		//		const vtxStripGroupHeader_s *stripGroup = mesh->pStripGroup(j);			
		//		prevVertexCount += stripGroup->numVerts;
		//	}
		//}
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
					if(this->h->version == 7) {
						i0 = stripGroup->pVertexV7(i0)->origMeshVertID;//+prevVertexCount;
						i1 = stripGroup->pVertexV7(i1)->origMeshVertID;//+prevVertexCount;
						i2 = stripGroup->pVertexV7(i2)->origMeshVertID;//+prevVertexCount;
					} else {
						i0 = stripGroup->pVertexV6(i0)->origMeshVertID;
						i1 = stripGroup->pVertexV6(i1)->origMeshVertID;
						i2 = stripGroup->pVertexV6(i2)->origMeshVertID;
					}
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
	if(vvd) {
		g_vfs->FS_FreeFile(vvd);
		vvd = 0;
	}
}
bool hl2MDLReader_c::beginReading(const char *fname) {
	if(data.loadFromFile(fname)) {
		g_core->RedWarning("hl2MDLReader_c::beginReading: cannot open %s\n",fname);
		return true; // error
	}
	this->name = fname;
	this->fileLen = data.getTotalLen();
	const mdlHeader_s *h = (const mdlHeader_s*)data.getDataPtr();
	this->version = h->version;
	if(readMatNames()) {
		return true;
	}	
	
	if(version >= 44) {
		// .vvd is not present in older .mdl formats
		str vvdFileName = fname;
		vvdFileName.setExtension("vvd");
		vvdFileLen = g_vfs->FS_ReadFile(vvdFileName,(void**)&vvd);
		if(vvd == 0) {
			g_core->RedWarning("hl2MDLReader_c::beginReading: cannot open %s\n",vvdFileName.c_str());
			return true; // error
		}
	}

	return false; // no error
}
bool hl2MDLReader_c::readMatNames() {
	if(version == 44) {
		data.setPos(204);
	} else if(version == 37) {
		data.setPos(224);
	} else {
		data.setPos(208);
	}
	u32 numMaterials = data.readU32();
	// do the sanity check
	if(numMaterials > MAX_MDL_MATERIALS) {
		g_core->RedWarning("hl2MDLReader_c::readMatNames: numMaterials is higher than MAX_MDL_MATERIALS (%i, %i). Mdl file is not valid.\n",
			numMaterials,MAX_MDL_MATERIALS);
		return true; // error
	}
	u32 ofsMaterials = data.readU32();
	// do the sanity check
	if(ofsMaterials > this->fileLen) {
		g_core->RedWarning("hl2MDLReader_c::readMatNames: materials data offset is higher than mdl file length (%i>%i)\n",
			ofsMaterials,this->fileLen);
		return true; // error
	}
	u32 numMaterialPaths = data.readU32();
	// do the sanity check
	if(numMaterialPaths > MAX_MDL_MATERIALS) {
		g_core->RedWarning("hl2MDLReader_c::readMatNames: numMaterialPaths is higher than MAX_MDL_MATERIALS (%i, %i). Mdl file is not valid.\n",
			numMaterialPaths,MAX_MDL_MATERIALS);
		return true; // error
	}
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
		if(this->version == 44) {
			data.setPos(saved + 64);
		} else {
			data.setPos(saved + 32);
		}
	}
	return false; // no error
}
bool hl2MDLReader_c::getStaticModelData(class staticModelCreatorAPI_i *out) {
	if(version == 44) {
		data.setPos(232);
	} else if(version == 37) {
		data.setPos(252);
	} else {
		data.setPos(236);
	}

	u32 numBodyParts = data.readU32();
	if(numBodyParts > MAX_MDL_BODYPARTS) {
		g_core->RedWarning("hl2MDLReader_c::getStaticModelData: bodyParts count %i is higher than MAX_MDL_BODYPARTS (%i). Mdl file is not valid\n",
			numBodyParts,MAX_MDL_BODYPARTS);
		return true; // error
	}

	u32 bodyPartsOfs = data.readU32();
	if(bodyPartsOfs >= this->fileLen) {
		g_core->RedWarning("hl2MDLReader_c::getStaticModelData: bodyPartsOfs %i is higher than mdl file lenght (%i). Mdl file is not valid\n",
			bodyPartsOfs,this->fileLen);
		return true; // error
	}

	vtxFile_c vtx;
	str vtxName = this->name;
	vtxName.setExtension("vtx");
	if(vtx.preload(vtxName)) {
		g_core->RedWarning("hl2MDLReader_c::getStaticModelData: couldn't load VTX file for mdl %s\n",this->name.c_str());
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
			g_core->Print("Model %i of %i, name: %s, numMeshes %i\n",j,bp.numModels,mh->name,mh->numMeshes);
			u32 firstMeshOfs = modelHeaderAt + mh->ofsMeshes;
			data.setPos(firstMeshOfs);
			// for each mesh
			//u32 meshVertexOffset = 0;
			for(u32 k = 0; k < mh->numMeshes; k++) {
				u32 meshHeaderAt = data.getPos();
				const mdlMeshHeader_s *mesh = (const mdlMeshHeader_s*)data.getCurDataPtr();
				const char *meshMaterial = this->getMatName(mesh->matIndex);
				str fullMatName = meshMaterial;
				fullMatName.setExtension("vmt");

				arraySTD_c<u16> indices;
				vtx.getMeshIndices(indices,i,j,k/*,meshVertexOffset*/);
				//if(this->version == 44) {
				//	const mdlV44VertexData *v44VertexData = mesh->getVertexData44();
				//	meshVertexOffset += v44VertexData->numLODVertexes[0];;
				//}

				if(this->version == 44) {
					// MDL V44 (retail Half Life2, Episodes, Portal) path
					// Vertices data is stored in vvd file
					const mdlV44Vertex_s *baseVertices44 = (const mdlV44Vertex_s*)vvd->getVertexData();
					const mdlV44Vertex_s *meshVertices44 = baseVertices44 + mesh->ofsVertices;

					for(u32 l = 0; l < indices.size(); l+= 3) {
						u32 i0 = indices[l+0];
						u32 i1 = indices[l+1];
						u32 i2 = indices[l+2];
						simpleVert_s v0,v1,v2;
						v0.setXYZ(meshVertices44[i0].pos);
						v0.setUV(meshVertices44[i0].tc);
						v1.setXYZ(meshVertices44[i1].pos);
						v1.setUV(meshVertices44[i1].tc);
						v2.setXYZ(meshVertices44[i2].pos);
						v2.setUV(meshVertices44[i2].tc);
						out->addTriangle(fullMatName,v0,v1,v2);
					}
				} else {
					// older mdl format paths (without vvd file)
					// vertices data is stored in mdl file
					const mdlV37Vertex_s *meshVertices37 = mesh->getVertices37();
					//g_core->Print("verize %i\n",sizeof(mdlV37Vertex_s));
					//for(u32 l = 0; l < mesh->numVertices; l++) {
					//	g_core->Print("V %i - %f %f %f\n",l,meshVertices37[l].pos[0],meshVertices37[l].pos[1],meshVertices37[l].pos[2]);
					//}
					for(u32 l = 0; l < indices.size(); l+= 3) {
						u32 i0 = indices[l+0];
						u32 i1 = indices[l+1];
						u32 i2 = indices[l+2];
						simpleVert_s v0,v1,v2;
						v0.setXYZ(meshVertices37[i0].pos);
						v0.setUV(meshVertices37[i0].tc);
						v1.setXYZ(meshVertices37[i1].pos);
						v1.setUV(meshVertices37[i1].tc);
						v2.setXYZ(meshVertices37[i2].pos);
						v2.setUV(meshVertices37[i2].tc);
						out->addTriangle(fullMatName,v0,v1,v2);
					}
				}
		
				if(this->version == 44) {
					data.setPos(meshHeaderAt + 116);
				} else {
					data.setPos(meshHeaderAt + 68);
				}
			}
			if(this->version == 44) {
				data.setPos(modelHeaderAt + 148);
			} else {
				data.setPos(modelHeaderAt + 140);
			}
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