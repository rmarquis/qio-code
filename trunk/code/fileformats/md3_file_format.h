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
// md3_file_format.h - Quake3 .md3 model file structures

#ifndef __MD3_FILE_FORMAT_H__
#define __MD3_FILE_FORMAT_H__

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

#define MD3_PATH_LEN		64

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

struct md3Frame_s {
	vec3_t bounds[2];
	vec3_t localOrigin;
	float radius;
	char name[16];
};

struct md3Tag_s {
	char name[MD3_PATH_LEN]; // tag name
	vec3_t origin;
	vec3_t axis[3];
};

struct md3Shader_s {
	char name[MD3_PATH_LEN];
	int shaderIndex; // for internal q3 usage
};

struct md3Triangle_s {
	int indexes[3];
};

struct md3St_s {
	float st[2];
};

struct md3XyzNormal_s {
	short xyz[3];
	byte normal[2];

	inline vec3_c getPos() const {
		vec3_c ret;
		ret.x = float(xyz[0]) * MD3_XYZ_SCALE;
		ret.y = float(xyz[1]) * MD3_XYZ_SCALE;
		ret.z = float(xyz[2]) * MD3_XYZ_SCALE;
		return ret;
	}
	inline vec3_c getNormal() const {
		float lng = normal[0] * (2 * M_PI) / 255.0f;
		float lat = normal[1] * (2 * M_PI) / 255.0f;
		return vec3_c (cos(lat) * sin(lng), sin(lat) * sin(lng),cos(lng));
	}
};

struct md3Surface_s {
	int ident;
	char name[MD3_PATH_LEN]; // submesh name
	int flags;
	int numFrames; // all surfaces in a model should have the same

	int numShaders; // all surfaces in a model should have the same
	int numVerts;

	int numTriangles;
	int ofsTriangles;

	int ofsShaders; // offset from start of md3Surface_s
	int ofsSt; // texture coords are shared by all frames
	int ofsXyzNormals; // [numVerts * numFrames]
	int ofsEnd; // next surface follows

	const u32 *getFirstIndex() const {
		return (const u32*)(((const byte*)this)+ofsTriangles);
	}
	const md3St_s *getSt(u32 stIndex) const {
		return (const md3St_s*)(((const byte*)this)+ofsSt+sizeof(md3St_s)*stIndex);
	}
	const md3XyzNormal_s *getXYZNormal(u32 xyzNormalIndex) const {
		return ((const md3XyzNormal_s*)(((const byte*)this)+ofsXyzNormals)+xyzNormalIndex);
	}
	const md3Shader_s *getShader(u32 localShaderIndex) const {
		return ((const md3Shader_s*)(((const byte*)this)+ofsShaders)+localShaderIndex);
	}
};

struct md3Header_s {
	int ident; // IDP3
	int version; // 15
	char name[MD3_PATH_LEN]; // model name
	int flags;
	int numFrames;
	int numTags;			
	int numSurfaces;
	int numSkins;
	int ofsFrames;
	int ofsTags; // [numFrames* numTags]
	int ofsSurfaces;
	int ofsEnd;

	const md3Frame_s *pFrame(u32 frameIndex) const {
		return (((const md3Frame_s*)(((const byte*)this)+ofsFrames))+frameIndex);
	}
	const md3Surface_s *pSurf(u32 surfIndex) const {
		const md3Surface_s *sf = (const md3Surface_s*)(((const byte*)this)+ofsSurfaces);
		while(surfIndex) {
			sf = (const md3Surface_s*)(((const byte*)sf)+sf->ofsEnd);
			surfIndex--;
		}
		return sf;
	}
	const md3Tag_s *getTags() const {
		return (const md3Tag_s*)(((const byte*)this)+ofsTags);
	}
};

#endif // __MD3_FILE_FORMAT_H__

