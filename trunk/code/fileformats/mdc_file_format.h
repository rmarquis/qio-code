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
// mdc_file_format.h - RTCW .mdc model file structures
#ifndef __MDC_FILE_FORMAT_H__
#define __MDC_FILE_FORMAT_H__

#include "md3_file_format.h"

#define MDC_IDENT           ( ( 'C' << 24 ) + ( 'P' << 16 ) + ( 'D' << 8 ) + 'I' )
#define MDC_VERSION         2

#define MDC_TAG_ANGLE_SCALE ( 360.0 / 32700.0 )

#define MDC_DIST_SCALE      0.05    // lower for more accuracy, but less range
#define MDC_MAX_OFS         127.0   // to be safe

#define R_MDC_DecodeXyzCompressed( ofsVec, out ) \
	( out )[0] = ( (float)( ( ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[1] = ( (float)( ( ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[2] = ( (float)( ( ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;


// version history:
// 1 - original
// 2 - changed tag structure so it only lists the names once

struct mdcXyzCompressed_s {
	unsigned int ofsVec;                    // offset direction from the last base frame
//	unsigned short	ofsVec;
};

struct mdcTagName_s {
	char name[64];           // tag name
};

struct mdcTag_s {
	short xyz[3];
	short angles[3];
};

/*
** mdcSurface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numBaseFrames
** XyzCompressed	sizeof( mdcXyzCompressed ) * numVerts * numCompFrames
** frameBaseFrames	sizeof( short ) * numFrames
** frameCompFrames	sizeof( short ) * numFrames (-1 if frame is a baseFrame)
*/
struct mdcSurface_s {
	int ident;                  //

	char name[64];       // polyset name

	int flags;
	int numCompFrames;          // all surfaces in a model should have the same
	int numBaseFrames;          // ditto

	int numShaders;             // all surfaces in a model should have the same
	int numVerts;

	int numTriangles;
	int ofsTriangles;

	int ofsShaders;             // offset from start of md3Surface_t
	int ofsSt;                  // texture coords are common for all frames
	int ofsXyzNormals;          // numVerts * numBaseFrames
	int ofsXyzCompressed;       // numVerts * numCompFrames

	int ofsFrameBaseFrames;     // numFrames
	int ofsFrameCompFrames;     // numFrames

	int ofsEnd;                 // next surface follows

	const short *getFrameBaseFrames() const {
		return (const short*)(((const byte*)this)+ofsFrameBaseFrames);
	}
	const short *getFrameCompFrames() const {
		return (const short*)(((const byte*)this)+ofsFrameCompFrames);
	}

	const u32 *getFirstIndex() const {
		return (const u32*)(((const byte*)this)+ofsTriangles);
	}
	const md3St_s *getSt(u32 stIndex) const {
		return (const md3St_s*)(((const byte*)this)+ofsSt+sizeof(md3St_s)*stIndex);
	}
	const md3XyzNormal_s *getXYZNormal(u32 xyzNormalIndex) const {
		return ((const md3XyzNormal_s*)(((const byte*)this)+ofsXyzNormals)+xyzNormalIndex);
	}
	const mdcXyzCompressed_s *getXYZCompressed(u32 xyzNormalIndex) const {
		return ((const mdcXyzCompressed_s*)(((const byte*)this)+ofsXyzCompressed)+xyzNormalIndex);
	}
	const md3Shader_s *getShader(u32 localShaderIndex) const {
		return ((const md3Shader_s*)(((const byte*)this)+ofsShaders)+localShaderIndex);
	}
};

struct mdcHeader_s {
	int ident;
	int version;

	char name[64];           // model name

	int flags;

	int numFrames;
	int numTags;
	int numSurfaces;

	int numSkins;

	int ofsFrames;                  // offset for first frame, stores the bounds and localOrigin
	int ofsTagNames;                // numTags
	int ofsTags;                    // numFrames * numTags
	int ofsSurfaces;                // first surface, others follow

	int ofsEnd;                     // end of file

	const md3Frame_s *pFrame(u32 frameIndex) const {
		return (((const md3Frame_s*)(((const byte*)this)+ofsFrames))+frameIndex);
	}
	const mdcSurface_s *pSurf(u32 surfIndex) const {
		const mdcSurface_s *sf = (const mdcSurface_s*)(((const byte*)this)+ofsSurfaces);
		while(surfIndex) {
			sf = (const mdcSurface_s*)(((const byte*)sf)+sf->ofsEnd);
			surfIndex--;
		}
		return sf;
	}
};

#endif // __MDC_FILE_FORMAT_H__
