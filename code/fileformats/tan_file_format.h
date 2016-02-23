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
#ifndef __TAN_FILE_FORMAT_H__
#define __TAN_FILE_FORMAT_H__

#define TAN_IDENT    ((' '<<24)+('N'<<16)+('A'<<8)+'T')
#define TAN_VERSION  2

#define TAN_XYZ_SCALE (1.0/64)
#define TAN_MAX_TAGS      16    // per frame

struct tanXyzNormal_s {
	unsigned short xyz[3];
	short    normal;

	inline vec3_c getPos() const {
		vec3_c ret;
		ret.x = float(xyz[0]) * TAN_XYZ_SCALE;
		ret.y = float(xyz[1]) * TAN_XYZ_SCALE;
		ret.z = float(xyz[2]) * TAN_XYZ_SCALE;
		return ret;
	}
};

struct tanSt_s {
	float st[2];
};

struct tanFrame_s {
   vec3_t      bounds[2];
   vec3_t      scale; // multiply by this ( ADDED FOR FAKK )
   vec3_t      offset; // and add this
   vec3_t      delta;
   float       radius;
   float       frametime; // ( ADDED FOR FAKK )
   //	char		name[16]; // it was here in md3, but is not present in tan
};

struct tanSurface_s {
   int      ident;            //

   char     name[64];  // polyset name

   int      numFrames;        // all surfaces in a model should have the same
   int      numVerts;
   int      minLod; // added for FAKK

   int      numTriangles;
   int      ofsTriangles;

   int      ofsCollapseMap;   // numVerts * int // added for FAKK

   int      ofsSt;            // texture coords are common for all frames
   int      ofsXyzNormals;    // numVerts * numFrames

   int      ofsEnd;           // next surface follows


	const u32 *getFirstIndex() const {
		return (const u32*)(((const byte*)this)+ofsTriangles);
	}
	const tanSt_s *getSt(u32 stIndex) const {
		return (const tanSt_s*)(((const byte*)this)+ofsSt+sizeof(tanSt_s)*stIndex);
	}
	const tanXyzNormal_s *getXYZNormal(u32 xyzNormalIndex) const {
		return ((const tanXyzNormal_s*)(((const byte*)this)+ofsXyzNormals)+xyzNormalIndex);
	}
};

struct tanTagData_s {
   vec3_t      origin;
   vec3_t      axis[3];
};

struct tanTag_s {
   char     name[64];  // tag name
};

struct tanHeader_s {
   int      ident;
   int      version;

   char     name[64];  // model name

   int      numFrames;
   int      numTags;
   int      numSurfaces;
   float    totaltime;
   vec3_t   totaldelta;

   int      ofsFrames;        // offset for first frame
   int      ofsSurfaces;      // first surface, others follow
   int      ofsTags[TAN_MAX_TAGS]; // tikiTag_t + numFrames * tikiTagData_t

   int      ofsEnd;           // end of file


	const tanFrame_s *pFrame(u32 frameIndex) const {
		return (((const tanFrame_s*)(((const byte*)this)+ofsFrames))+frameIndex);
	}
	const tanSurface_s *pSurf(u32 surfIndex) const {
		const tanSurface_s *sf = (const tanSurface_s*)(((const byte*)this)+ofsSurfaces);
		while(surfIndex) {
			sf = (const tanSurface_s*)(((const byte*)sf)+sf->ofsEnd);
			surfIndex--;
		}
		return sf;
	}
};

#endif // __TAN_FILE_FORMAT_H__
