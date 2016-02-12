/*
============================================================================
Copyright (C) 2012-2016 V.

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
// md5r_file_format.h
#ifndef __MD5R_FILE_FORMAT_H__
#define __MD5R_FILE_FORMAT_H__

#include <math/aabb.h>
#include <shared/texturedVertex.h>

// vertex component type
enum md5rVertexComponentType_e {
	MD5RVCT_POS,
	MD5RVCT_BLENDINDEX,
	MD5RVCT_BLENDINDEXSMALL,
	MD5RVCT_NORMAL,
	MD5RVCT_TANGENT,
	MD5RVCT_BINORMAL,
	MD5RVCT_DIFFUSECOLOR,
	MD5RVCT_TEXCOORD,
	MD5RVCT_BLENDWEIGHT,
};
// vertex component data type
enum md5rVertexComponentDataType_e {
	MD5RVCDT_FLOAT2, // 8 bytes
	MD5RVCDT_FLOAT3, // 12 bytes
	MD5RVCDT_FLOAT4, // 16 bytes
	MD5RVCDT_DEC_11_11_10N, // 4 bytes
	MD5RVCDT_DEC_10_10_10N,
	MD5RVCDT_RGBA, // 4 bytes
	MD5RVCDT_INTEGER, // 4 bytes
};
// Position 3 BlendIndex Normal Tangent Binormal DiffuseColor TexCoord 2 0
// 64 bytes
// 12 + 4 + 12 + 12 + 12 + 4 + 8
struct testVert_s {
	float pos[3];
	int blendIndex;
	float normal[3];
	float tangent[3];
	float binormal[3];
	byte color[4];
	float tc[2];
};
// Vertex format with two texcoords example:
// 	VertexFormat { Position 4 Normal Tangent Binormal DiffuseColor TexCoord 2 0 TexCoord 2 1 }
class md5rVertexComponent_c {
friend class md5rVertexFormat_c;
friend class md5rVertexBuffer_c;
	byte type;
	byte dataType;
	byte offset;
	byte size;

	bool parse(class parser_c &p) {
		if(p.atWord_dontNeedWS("Position")) {
			type = MD5RVCT_POS;
			dataType = MD5RVCDT_FLOAT3; // set default dataType
		} else if(p.atWord_dontNeedWS("BlendIndexSmall")) {
			type = MD5RVCT_BLENDINDEXSMALL;
		} else if(p.atWord_dontNeedWS("BlendIndex")) {
			type = MD5RVCT_BLENDINDEX;
			dataType = MD5RVCDT_INTEGER; // set default dataType
		} else if(p.atWord_dontNeedWS("Normal")) {
			type = MD5RVCT_NORMAL;
			dataType = MD5RVCDT_FLOAT3; // set default dataType
		} else if(p.atWord_dontNeedWS("Tangent")) {
			type = MD5RVCT_TANGENT;
			dataType = MD5RVCDT_FLOAT3; // set default dataType
		} else if(p.atWord_dontNeedWS("Binormal")) {
			type = MD5RVCT_BINORMAL;
			dataType = MD5RVCDT_FLOAT3; // set default dataType
		} else if(p.atWord_dontNeedWS("DiffuseColor")) {
			type = MD5RVCT_DIFFUSECOLOR;
			dataType = MD5RVCDT_RGBA; // set default dataType
		} else if(p.atWord_dontNeedWS("TexCoord")) {
			type = MD5RVCT_TEXCOORD;
		} else if(p.atWord_dontNeedWS("BlendWeight")) {
			type = MD5RVCT_BLENDWEIGHT;
		} else {
			g_core->RedWarning("md5rVertexComponent_c::parse: unknown vertex format component type %s at line %i of %s\n",p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}
		if(p.atWord("4")) {
			if(p.atWord("4")) {
			// VertexFormat { Position 3 BlendIndex BlendWeight 4 4 }

			} else {			
				dataType = MD5RVCDT_FLOAT4;
			}
		} else if(p.atWord("3")) {
			if(p.atWord("4")) {
			// VertexFormat { Position 3 BlendIndex BlendWeight 3 4 Normal Tangent DiffuseColor TexCoord 2 0 }

			} else {			
				dataType = MD5RVCDT_FLOAT3;
			}
		} else if(p.atWord("2")) {
			if(p.atWord("0")) {
				dataType = MD5RVCDT_FLOAT2;
			} else if(p.atWord("1")) {
				dataType = MD5RVCDT_FLOAT2;
			} else {	
				g_core->RedWarning("md5rVertexComponent_c::parse: unknown texcoord format component data type %s at line %i of %s\n",p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
				return true;
			}
		} else if(p.atWord("Dec_10_10_10N")) {
			dataType = MD5RVCDT_DEC_10_10_10N;
		} else if(p.atWord("Dec_11_11_10N")) {
			dataType = MD5RVCDT_DEC_11_11_10N;
		} else {
			// use default dataType
		}
		// set size field
		if(dataType == MD5RVCDT_FLOAT3) {
			size = 12;
		} else if(dataType == MD5RVCDT_FLOAT4) {
			size = 16;
		} else if(dataType == MD5RVCDT_FLOAT2) {
			size = 8;
		} else if(dataType == MD5RVCDT_RGBA) {
			size = 4;
		} else if(dataType == MD5RVCDT_DEC_10_10_10N) {
			size = 4;
		} else if(dataType == MD5RVCDT_DEC_11_11_10N) {
			size = 4;
		} else if(dataType == MD5RVCDT_INTEGER) {
			size = 4;
		}
		return false;
	}
};
class md5rVertexFormat_c {
	arraySTD_c<md5rVertexComponent_c> components;
public:
	bool parse(parser_c &p) {
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rVertexBuffer_c::parse: expected '{' at the beginning of VertexFormat block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord("}") == false) {
			md5rVertexComponent_c &c = components.pushBack();
			if(c.parse(p)) {
				return true;
			}
		}
		// calculate offsets
		u32 ofs = 0;
		for(u32 i = 0; i < components.size(); i++) {
			md5rVertexComponent_c &c = components[i];
			c.offset = ofs;
			ofs += c.size;
		}
		return false; // no error
	}
	u32 size() const {
		return components.size();
	}
	const md5rVertexComponent_c *findComponentOfType(md5rVertexComponentType_e type) const {
		for(u32 i = 0; i < components.size(); i++) {
			if(components[i].type == type) {
				return &components[i];
			}
		}
		return 0;
	}
};
class md5rRawBytesBuffer_c {
	arraySTD_c<byte> data;
	// name of file from where data is loaded
	str dataFileName;
	// name of "parent" source file
	str sourceFileName;

	bool loadDataFromExternalFile() {
		byte *fileData;
		u32 fileLen;
		fileLen = g_vfs->FS_ReadFile(dataFileName,(void**)&fileData);
		if(fileData == 0) {
			str fixed = sourceFileName;
			fixed.toDir();
			fixed.append(dataFileName);
			fileLen = g_vfs->FS_ReadFile(fixed,(void**)&fileData);
			if(fileData == 0) {
				g_core->RedWarning("md5rRawBytesBuffer_c::loadDataFromExternalFile: cannot open %s\n",dataFileName.c_str());
				return true; // error
			}
		}
		data.resize(fileLen);
		memcpy(data.getArray(),fileData,fileLen);
		g_vfs->FS_FreeFile(fileData);
		return false;
	}
public:
	bool parse(class parser_c &p) {
		this->sourceFileName = p.getDebugFileName();

		if(p.atWord("bin")) {
			// binary data is stored in this file
			u32 numBytes = p.getInteger();
			// skip '\n'
			p.skipBytes(1);
			data.resize(numBytes);
			p.readBytes(data.getArray(),numBytes);
		} else if(p.atWord("file")) {
			// binary data is stored in separate file
			dataFileName = p.getToken();
			loadDataFromExternalFile();
		} else {
			// there is no data stored
			// (happens very often)
		}
		return false;
	}
	u32 size() const {
		return data.size();
	}
	byte operator[] (u32 idx) const {
		return data[idx];
	}
	const byte *getArray() const {
		return data.getArray();
	}
};
template <typename _Ty> 
class md5rArray_c {
	arraySTD_c<_Ty> data;
public:
	const char *getTypeIdent() const {
		return "TODO";
	}
	bool parse(class parser_c &p) {
		if(p.atWord_dontNeedWS("[")) {

		}
		const char *s = p.getToken("]");
		u32 numObjects = atoi(s);
		data.resize(numObjects);
		if(p.atWord_dontNeedWS("]")) {

		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rArray_c<%s>::parse: expected '{' at the beginning of %s block at line %i of %s, found %s\n",
			getTypeIdent(), getTypeIdent(), p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		for(u32 i = 0; i < numObjects; i++) {
			if(data[i].parse(p)) {
				g_core->RedWarning("md5rArray_c<%s>::parse: failed to parse object %i of %i\n",getTypeIdent(),i,numObjects);
				return true; // error
			}
		}
		if(p.atWord_dontNeedWS("}") == false) {
			g_core->RedWarning("md5rArray_c<%s>::parse: expected '}' after the array data block at line %i of %s, found %s\n",
				getTypeIdent(),p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		return false; // no error
	}
	const _Ty &operator[] (u32 i) const {
		return data[i];
	}
	u32 size() const {
		return data.size();
	}
};
class md5rVertexBuffer_c {
	u32 numVertices;
	u32 stride;
	md5rRawBytesBuffer_c data;
	bool bSystemMemory;
	bool bVideoMemory;
	bool bKeepUncompressed;
	md5rVertexFormat_c vertexFormat;
	md5rVertexFormat_c loadVertexFormat;
public:
	md5rVertexBuffer_c() {
		bSystemMemory = false;
		bVideoMemory = false;
		bKeepUncompressed = false;
		stride = 0;
		numVertices = 0;
	}
	bool parse(class parser_c &p) {
		if(p.atWord("VertexBuffer") == false) {
			g_core->RedWarning("skelModel_c::loadMD5R: expected VertexBuffer keyword at the beginning of VertexBuffer block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rVertexBuffer_c::prase: expected '{' at the beginning of VertexBuffer block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord_dontNeedWS("}")==false) {
			if(p.atWord("VertexFormat")) {
				if(vertexFormat.parse(p)) {
					return true;
				}
			} else if(p.atWord("LoadVertexFormat")) {
				if(loadVertexFormat.parse(p)) {
					return true;
				}
			} else if(p.atWord("SystemMemory")) {
				bSystemMemory = true;
			} else if(p.atWord("VideoMemory")) {
				bVideoMemory = true;	
			} else if(p.atWord("KeepUncompressed")) {
				bKeepUncompressed = true;
			} else if(p.atWord_dontNeedWS("Vertex")) {
				if(p.atWord_dontNeedWS("[")) {

				}
				const char *s = p.getToken("]");
				numVertices = atoi(s);
				if(p.atWord_dontNeedWS("]")) {

				}
				if(p.atWord_dontNeedWS("{") == false) {
					g_core->RedWarning("skelModel_c::loadMD5R: expected '{' at the beginning of Vertexes block at line %i of %s, found %s\n",
						p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
					return true; // error
				}
				// load binary data directly from current file
				// or from external md5rbin file
				data.parse(p);
				
				stride = data.size() / numVertices;

				if(p.atWord_dontNeedWS("}") == false) {
					g_core->RedWarning("skelModel_c::loadMD5R: expected '}' after the Vertexes block at line %i of %s, found %s\n",
						p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
					return true; // error
				}
			}
		}
		return false;
	}
	u32 getVertexOffset(u32 index) const {
		return this->stride * index;
	}
	const byte *getVertexData(u32 index) const {
		if(this->data.size() == 0)
			return 0;
		return this->data.getArray() + getVertexOffset(index);
	}
	const md5rVertexComponent_c *findComponentOfType(md5rVertexComponentType_e type) const {
		if(loadVertexFormat.size()) {
			return loadVertexFormat.findComponentOfType(type);
		}
		return vertexFormat.findComponentOfType(type);
	}
	void getVertexVec3(int index, md5rVertexComponentType_e type, float *out) const {
		const byte *data = getVertexData(index);
		const md5rVertexComponent_c *c = findComponentOfType(type);
		if(c == 0)
			return;
		if(c->dataType == MD5RVCDT_FLOAT3) {
			memcpy(out,data+c->offset,3*sizeof(float));
		} else if(c->dataType == MD5RVCDT_FLOAT4) {
			memcpy(out,data+c->offset,3*sizeof(float));
		}
	}
	void getVertexVec2(int index, md5rVertexComponentType_e type, float *out) const {
		const byte *data = getVertexData(index);
		const md5rVertexComponent_c *c = findComponentOfType(type);
		if(c == 0)
			return;
		if(c->dataType == MD5RVCDT_FLOAT2) {
			memcpy(out,data+c->offset,2*sizeof(float));
		} else if(c->dataType == MD5RVCDT_FLOAT3) {
			memcpy(out,data+c->offset,2*sizeof(float));
		} else if(c->dataType == MD5RVCDT_FLOAT4) {
			memcpy(out,data+c->offset,2*sizeof(float));
		}
	}
	void getVertexPos(u32 index, float *out) const {
		getVertexVec3(index,MD5RVCT_POS,out);
	}
	void getVertexTC(u32 index, float *out) const {
		getVertexVec2(index,MD5RVCT_TEXCOORD,out);
	}
	void getVertex(u32 index, texturedVertex_c &v) const {
		getVertexPos(index,v.xyz);
		getVertexTC(index,v.st);
	}
	u32 size() const {
		return numVertices;
	}
};
class md5rIndexBuffer_c {
	u32 numIndices;
	md5rRawBytesBuffer_c data;
	u32 bitDepth;
public:
	bool parse(class parser_c &p) {
		if(p.atWord("IndexBuffer") == false) {
			g_core->RedWarning("md5rIndexBuffer_c::parse: expected IndexBuffer keyword at the beginning of IndexBuffer block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rIndexBuffer_c::parse: expected '{' at the beginning of IndexBuffer block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord_dontNeedWS("Index[")==false) {
			if(p.atWord("IndexFormat")) {
				p.getToken(); // get first {
				p.skipCurlyBracedBlock();
			} else if(p.atWord("LoadIndexFormat")) {
				p.getToken(); // get first {
				p.skipCurlyBracedBlock();
			} else if(p.atWord("SystemMemory")) {
						
			} else if(p.atWord("VideoMemory")) {
						
			} else if(p.atWord("BitDepth")) {
				bitDepth = p.getInteger();
			}
		}
		if(p.atWord_dontNeedWS("[")) {

		}
		const char *s = p.getToken("]");
		numIndices = atoi(s);
		if(p.atWord_dontNeedWS("]")) {

		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rIndexBuffer_c::parse: expected '{' at the beginning of Indexes block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		// load binary data directly from current file
		// or from external md5rbin file
		data.parse(p);
				
		if(p.atWord_dontNeedWS("}") == false) {
			g_core->RedWarning("md5rIndexBuffer_c::parse: expected '}' after the Indexes block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		if(p.atWord_dontNeedWS("}") == false) {
			g_core->RedWarning("md5rIndexBuffer_c::parse: expected '{' at the end of IndexBuffer block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}		
		return false;
	}
	u32 getIndex(u32 i) const {
		if(data.size() == 0)
			return 0;
		if(i >= numIndices) {
			return 0;
		}
		if(bitDepth == 16) {
			return ((const u16 *)data.getArray())[i];
		} else if(bitDepth == 32) {
			return ((const u32 *)data.getArray())[i];
		} else { 
			return 0;
		}
	}
	u32 size() const {
		return numIndices;
	}
};
class md5rPrimBatch_c {
public:
	int SilTraceIndexedTriList[4];
	int DrawIndexedTriList[4];
	int ShadowVerts;
	int SilhouetteEdge[2];
	int ShadowIndexedTriList[6];
	arraySTD_c<int> transforms;
public:
	md5rPrimBatch_c() {
		DrawIndexedTriList[0] = DrawIndexedTriList[1] = DrawIndexedTriList[2] = DrawIndexedTriList[3] = -1;
	}
	bool parse(class parser_c &p) {
		if(p.atWord("PrimBatch") == false) {
			g_core->RedWarning("md5rPrimBatch_c::parse: expected PrimBatch keyword at the beginning of Mesh block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rPrimBatch_c::parse: expected '{' at the beginning of PrimBatch block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord_dontNeedWS("}")==false) {
			if(p.atWord("SilTraceIndexedTriList")) {
				SilTraceIndexedTriList[0] = p.getInteger();
				SilTraceIndexedTriList[1] = p.getInteger();
				SilTraceIndexedTriList[2] = p.getInteger();
				SilTraceIndexedTriList[3] = p.getInteger();
			} else if(p.atWord("DrawIndexedTriList")) {
				// DrawIndexedTriList 0 651 0 610
				// 610 is numIndices / 3
				DrawIndexedTriList[0] = p.getInteger();
				DrawIndexedTriList[1] = p.getInteger();
				DrawIndexedTriList[2] = p.getInteger();
				DrawIndexedTriList[3] = p.getInteger();
			} else if(p.atWord("ShadowVerts")) {
				ShadowVerts = p.getInteger();
			} else if(p.atWord("SilhouetteEdge")) {
				SilhouetteEdge[0] = p.getInteger();
				SilhouetteEdge[1] = p.getInteger();
			} else if(p.atWord("ShadowIndexedTriList")) {
			//	ShadowIndexedTriList 0 688 0 524 288 63
				ShadowIndexedTriList[0] = p.getInteger();
				ShadowIndexedTriList[1] = p.getInteger();
				ShadowIndexedTriList[2] = p.getInteger();
				ShadowIndexedTriList[3] = p.getInteger();
				ShadowIndexedTriList[4] = p.getInteger();
				ShadowIndexedTriList[5] = p.getInteger();
			} else if(p.atWord_dontNeedWS("Transform")) {
				if(p.atWord_dontNeedWS("[")) {

				}
				const char *s = p.getToken("]");
				u32 numTransforms = atoi(s);
				if(p.atWord_dontNeedWS("]")) {

				}
				if(p.atWord_dontNeedWS("{") == false) {
					g_core->RedWarning("md5rPrimBatch_c::parse: expected '{' at the beginning of Transform block at line %i of %s, found %s\n",
						p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
					return true; // error
				}
				transforms.resize(numTransforms);
				for(u32 i = 0; i < numTransforms; i++) {
					transforms[i] = p.getInteger();
				}
				if(p.atWord_dontNeedWS("}") == false) {
					g_core->RedWarning("md5rPrimBatch_c::parse: expected '}' at the end of Transform block at line %i of %s, found %s\n",
						p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
					return true; // error
				}
			} else {

			}
		}
		return false;
	}
};
class md5rMesh_c {
friend class skelModel_c;
	str matName;
	int silTraceBuffers[2];
	int drawBuffers[2];
	int shadowVolumeBuffers[2];
	aabb bb;
	int levelOfDetail;
	md5rArray_c<md5rPrimBatch_c> primBatches;
public:
	md5rMesh_c() {
		drawBuffers[0] = -1;
		drawBuffers[1] = -1;
	}
	int getDrawBuffers(int i) const {
		return drawBuffers[i];
	}
	bool parse(class parser_c &p) {
		if(p.atWord("Mesh") == false) {
			g_core->RedWarning("md5rMesh_c::parse: expected Mesh keyword at the beginning of Mesh block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rMesh_c::parse: expected '{' at the beginning of Mesh block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord_dontNeedWS("}")==false) {
			if(p.atWord("Material")) {
				matName = p.getToken();
			} else if(p.atWord("SilTraceBuffers")) {
				silTraceBuffers[0] = p.getInteger();
				silTraceBuffers[1] = p.getInteger();
			} else if(p.atWord("DrawBuffers")) {
				drawBuffers[0] = p.getInteger();
				drawBuffers[1] = p.getInteger();
			} else if(p.atWord("ShadowVolumeBuffers")) {
				shadowVolumeBuffers[0] = p.getInteger();
				shadowVolumeBuffers[1] = p.getInteger();
			} else if(p.atWord("LevelOfDetail")) {
				levelOfDetail = p.getInteger();
			} else if(p.atWord_dontNeedWS("PrimBatch")) {
				if(primBatches.parse(p)) {

				}
			} else if(p.atWord("Bounds")) {
				p.getFloatMat(bb.mins,3);
				p.getFloatMat(bb.maxs,3);
			} else {

			}
		}
		return false;
	}
	
	
	u32 getNumPrimBatches() const {
		return primBatches.size();
	}
	const md5rPrimBatch_c &getPrimBatch(u32 index) const {
		return primBatches[index];
	}
	const char *getMatName() const {
		return matName;
	}
};
class md5rModel_c {
	str modelName;
	aabb bb;
	md5rArray_c<md5rMesh_c> meshes;
public:
	bool parse(class parser_c &p) {
		if(p.atWord("Model") == false) {
			g_core->RedWarning("md5rModel_c::parse: expected Model keyword at the beginning of Mesh block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		modelName = p.getToken();
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rModel_c::parse: expected '{' at the beginning of Mesh block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		while(p.atWord_dontNeedWS("}")==false) {
			if(p.atWord_dontNeedWS("Mesh")) {
				if(meshes.parse(p)) {
					g_core->RedWarning("md5rModel_c::parse: cannot parse meshes\n",p.getDebugFileName());
					return true; // error
				}
			} else if(p.atWord("Bounds")) {
				p.getFloatMat(bb.mins,3);
				p.getFloatMat(bb.maxs,3);
			} else {

			}
		}
		return false;
	}
};
class md5rSilhouetteEdgesArray_c {
	md5rRawBytesBuffer_c data;
public:
	bool parse(class parser_c &p) {
		if(p.atWord_dontNeedWS("[")) {
		}
		const char *s = p.getToken("]");
		u32 numIndexBuffers = atoi(s);
		if(p.atWord_dontNeedWS("]")) {

		}
		if(p.atWord_dontNeedWS("{") == false) {
			g_core->RedWarning("md5rSilhouetteEdgesArray_c::parse: expected '{' at the beginning of SilhouetteEdges block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		// load binary data directly from current file
		// or from external md5rbin file
		data.parse(p);

		if(p.atWord_dontNeedWS("}") == false) {
			g_core->RedWarning("md5rSilhouetteEdgesArray_c::parse: expected '}' after the SilhouetteEdges block at line %i of %s, found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return true; // error
		}
		return false;
	}
};

#endif // __MD5R_FILE_FORMAT_H__
