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
// lwoLoader.cpp
#include <shared/array.h>
#include <shared/str.h>
#include <shared/readStream.h>
#include <math/aabb.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>

#define LWO_IDENT (('M'<<24) + ('R'<<16) + ('O'<<8) + 'F')
#define LWO_LWO2 (('2'<<24) + ('O'<<16) + ('W'<<8) + 'L')
#define LWO_TAGS (('S'<<24) + ('G'<<16) + ('A'<<8) + 'T')
#define LWO_LAYR (('R'<<24) + ('Y'<<16) + ('A'<<8) + 'L')
#define LWO_POINTS (('S'<<24) + ('T'<<16) + ('N'<<8) + 'P')
#define LWO_BBOX (('X'<<24) + ('O'<<16) + ('B'<<8) + 'B')
#define LWO_POLS (('S'<<24) + ('L'<<16) + ('O'<<8) + 'P')
#define LWO_FACE (('E'<<24) + ('C'<<16) + ('A'<<8) + 'F')
#define LWO_PTAG (('G'<<24) + ('A'<<16) + ('T'<<8) + 'P')
#define LWO_SURF (('F'<<24) + ('R'<<16) + ('U'<<8) + 'S')
#define LWO_VMAP (('P'<<24) + ('A'<<16) + ('M'<<8) + 'V')
#define LWO_VMAD (('D'<<24) + ('A'<<16) + ('M'<<8) + 'V')
#define LWO_TXUV (('V'<<24) + ('U'<<16) + ('X'<<8) + 'T')


void LWO_ReadLWOName(class readStream_c &r, str &out) {
	r.readByteString(out);
	if(r.getPos() & 1) {
		r.skipBytes(1);
	}
}
#define SWAP16(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP32(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )

s32 SwapS32(s32 i) {
	return SWAP32(i);
}
s16 SwapS16(s16 i) {
	return SWAP16(i);
}
u32 LWO_ReadLWOVertIndex(class readStream_c &r) {
	int i = r.readChar();
	int ret;
	if(i == 0xff) {
		i += (r.readChar() << 8);
		i += (r.readChar() << 16);
		i += (r.readChar() << 24);

		ret = SwapS32(i);
	} else {
		i += (r.readChar() << 8);

		ret = SwapS16(i);
	}
	return ret;
}
struct lwoTag_s {
	int polyNum;
	int tagNum;
};
struct lwoVMap_s {
	int vertNum;
	float tc[2];
};
struct lwoVMad_s {
	int vertNum;
	int polyNum;
	float tc[2];
};
struct lwoPoly_s {
	u32 flags;
	u32 numVerts;
	u32 vertices[6];
};

void LWO_VerbosePrintf(const char *fmt, ...) {
	//if(lwo_verbose.getInteger() == 0)
	//	return;

	// TODO
}
bool MOD_LoadLWO(const char *fname, class staticModelCreatorAPI_i *out) {
	readStream_c r;
	if(r.loadFromFile(fname)) {
		g_core->RedWarning("MOD_LoadLWO: cannot open %s\n",fname);
		return true;
	}
	int ident = r.readS32();
	if(ident != LWO_IDENT) {
		g_core->RedWarning("MOD_LoadLWO: %s has bad ident\n",fname);
		return true;
	}
	u32 streamLen = r.getTotalLen();
	u32 fileDataLen = r.swReadU32();
	if(streamLen != fileDataLen+8) {
		g_core->RedWarning("MOD_LoadLWO: WARNING : %s has bad filelen (%i, should be %i)\n",fname,fileDataLen,streamLen-8);
	}
	int id = r.readS32();
	if(id != LWO_LWO2) {
		g_core->RedWarning("MOD_LoadLWO: %s has bad second ident (LWO2 expected)\n",fname);
		return true;
	}
	arraySTD_c<vec3_c> points;
	arraySTD_c<lwoTag_s> tags;
	arraySTD_c<lwoVMap_s> vMaps;
	arraySTD_c<lwoVMad_s> vMads;
	arraySTD_c<lwoPoly_s> polys;
	arraySTD_c<str> matNames;
	aabb pointsBB;
	while(r.isAtEOF() == false) {
		u32 secStartOfs = r.getPos();
		int secIdent = r.readS32();
		u32 secLen = r.swReadU32();
		u32 secDataStartPos = r.getPos();
		u32 secDataEndPos = r.getPos() + secLen;
		if(secIdent == LWO_TAGS) {
			while(r.getPos() != secDataEndPos) {
				str tagName;
				LWO_ReadLWOName(r,tagName);
				LWO_VerbosePrintf("LWO tagName: %s\n",tagName.c_str());
				matNames.push_back(tagName);
			}
		} else if(secIdent == LWO_LAYR) {
			u16 flags[2];
			flags[0] = r.swReadU16();
			flags[1] = r.swReadU16();
			vec3_c pivot;
			r.swReadVec3(pivot);
			str layerName;
			LWO_ReadLWOName(r,layerName);
			int parent = -1;
			if(r.getPos() != secDataEndPos) {
				parent = r.swReadU16();
			}
			LWO_VerbosePrintf("LWO layer: %s with pivot %f %f %f and parent %i\n",layerName.c_str(),pivot.x,pivot.y,pivot.z,parent);
		} else if(secIdent == LWO_POINTS) {
			if(points.size()) {
				g_core->Print("MOD_LoadLWO: LWO_POINTS section found more than once! Part of model geometry might be missing in %s\n",fname);
			}
			u32 numPoints = secLen / 12;
			points.resize(numPoints);
			for(u32 i = 0; i < numPoints; i++) {
				r.swReadVec3(points[i]);
				// swap YZ - the same thing is done by Doom3
				points[i].swapYZ();
				pointsBB.addPoint(points[i]);
				//g_core->Print("LWO points %i of %i - %f %f %f\n",i,numPoints,points[i].x,points[i].y,points[i].z);
			}
		} else if(secIdent == LWO_BBOX) {
			aabb bb;
			r.swReadVec3(bb.mins);
			r.swReadVec3(bb.maxs);
		} else if(secIdent == LWO_POLS) {
			int faceIdent = r.readS32();
			if(faceIdent != LWO_FACE) {
				g_core->Print("MOD_LoadLWO: expected FACE ident in %s at %i\n",fname,r.getPos()-4);
			}
			u32 readFaceNum = 0;
			while(r.getPos() != secDataEndPos) {
				u32 numVerts = r.swReadU16();
				u32 flags = (0xFC00 & numVerts) >> 10;
				numVerts =  0x03FF & numVerts;
				assert(numVerts == 3);
				lwoPoly_s newPoly;
				newPoly.numVerts = numVerts;
				newPoly.flags = flags;
				for(u32 i = 0; i < numVerts; i++) {
					u32 vertIdx = LWO_ReadLWOVertIndex(r);
					newPoly.vertices[i] = vertIdx;
					//g_core->Print("Face %i, vert %i of %i - %i\n",readFaceNum,i,numVerts,vertIdx);
				}
				polys.push_back(newPoly);
				readFaceNum++;
			}	
		} else if(secIdent == LWO_PTAG) {
			int tagID = r.readS32();
			while(r.getPos() != secDataEndPos) {
				lwoTag_s nt;
				nt.polyNum = LWO_ReadLWOVertIndex(r);
				nt.tagNum = r.swReadU16();
				if(tagID == LWO_SURF) {
					tags.push_back(nt);
				}
			}
		} else if(secIdent == LWO_VMAP) {
			int mapID = r.readS32();
			int numDim = r.swReadU16();
			str mapName;
			LWO_ReadLWOName(r,mapName);
			LWO_VerbosePrintf("LWO_VMAP: id %i, numDims %i, name %s\n",mapID,numDim,mapName);
			if(mapID == LWO_TXUV) {
				while(r.getPos() != secDataEndPos) {
					lwoVMap_s newMap;
					newMap.vertNum = LWO_ReadLWOVertIndex(r);
					newMap.tc[0] = r.swReadFloat();
					newMap.tc[1] = -r.swReadFloat();
					vMaps.push_back(newMap);
				}
			} else {
				r.skipBytes(secLen);
			}
		} else if(secIdent == LWO_VMAD) {
			int mapID = r.readS32();
			int numDim = r.swReadU16();
			str mapName;
			LWO_ReadLWOName(r,mapName);
			LWO_VerbosePrintf("LWO_VMAD: id %i, numDims %i, name %s\n",mapID,numDim,mapName);
			if(mapID == LWO_TXUV) {
				while(r.getPos() != secDataEndPos) {
					lwoVMad_s newMad;
					newMad.vertNum = LWO_ReadLWOVertIndex(r);
					newMad.polyNum = LWO_ReadLWOVertIndex(r);
					newMad.tc[0] = r.swReadFloat();
					newMad.tc[1] = -r.swReadFloat();
					vMads.push_back(newMad);
				}
			} else {
				r.skipBytes(secLen);
			}
		} else {
			r.skipBytes(secLen);
		}
		if(r.getPos() != secDataEndPos) {
			r.setPos(secDataEndPos);
		}
	}

	// build model from loaded data
	lwoTag_s *t = tags.getArray();
	for(u32 i = 0; i < tags.size(); i++, t++) {
		if(t->polyNum >= polys.size()) {
			g_core->Print("MOD_LoadLWO: t->polyNum >= polys.size() ! (%i >= %i) in model %s, skipping polygon\n",
				t->polyNum,polys.size(),fname);
			continue;
		}
		lwoPoly_s &p = polys[t->polyNum];
		simpleVert_s verts[6];
		for(u32 j = 0; j < p.numVerts; j++) {
			u32 vertNum = p.vertices[j];
			// assign position
			verts[j].setXYZ(points[vertNum]);
			// assign texCoord
			bool foundTC = false;
			lwoVMad_s *vMad = vMads.getArray();
			for(u32 k = 0; k < vMads.size(); k++, vMad++) {
				if(t->polyNum != vMad->polyNum) {
					continue;
				}
				if(vMad->vertNum != vertNum)
					continue;
				verts[j].setUV(vMad->tc);
				foundTC = true;
				break;
			}
			if(foundTC == false) {
				lwoVMap_s *vMap = vMaps.getArray();
				for(u32 k = 0; k < vMaps.size(); k++, vMap++) {
					if(vMap->vertNum != vertNum)
						continue;
					verts[j].setUV(vMap->tc);
					foundTC = true;
					break;
				}
				if(foundTC == false) {
					//g_core->RedWarning
					// it happens often on Prey maps 
					// (and I think it's the same for Doom3)
					LWO_VerbosePrintf("LWO texcoords not found!\n");
				}
			}
		}
		out->addTriangle(matNames[t->tagNum],verts[0],verts[1],verts[2]);
	}
	return false;
}
