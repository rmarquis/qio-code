/*
============================================================================
Copyright (C) 2012-2013 V.

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
// md2_file_format.h - QuakeII model file format
#ifndef __MD2_FILE_FORMAT_H__
#define __MD2_FILE_FORMAT_H__

#define MD2_IDENT (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I')
#define MD2_VERSION 8

/* Texture name */
struct md2Skin_s {
	char name[64];              /* texture file name */
};

/* Texture coords */
struct md2TexCoord_s {
	short s;
	short t;
};

/* Triangle info */
struct md2Triangle_s {
	unsigned short vertex[3];   /* vertex indices of the triangle */
	unsigned short st[3];       /* tex. coord. indices */
};

/* Compressed vertex */
struct md2Vertex_s {
	unsigned char v[3];         /* position */
	unsigned char normalIndex;  /* normal vector index */
};

/* Model frame */
struct md2Frame_s {
	vec3_t scale;               /* scale factor */
	vec3_t translate;           /* translation vector */
	char name[16];              /* frame name */

	const md2Vertex_s *pVertex(u32 vertNum) const {
		return (const md2Vertex_s*)(((byte*)this)+sizeof(*this)+vertNum*sizeof(md2Vertex_s));
	}
};
struct md2Header_s {
	int ident;                  /* magic number: "IDP2" */
	int version;                /* version: must be 8 */

	int skinWidth;              /* texture width */
	int skinHeight;             /* texture height */

	int frameSize;              /* size in bytes of a frame */

	int numSkins;              /* number of skins */
	int numVertices;           /* number of vertices per frame */
	int numST;                 /* number of texture coordinates */
	int numTris;               /* number of triangles */
	int numGLCmds;             /* number of opengl commands */
	int numFrames;             /* number of frames */

	int ofsSkins;           /* offset skin data */
	int ofsSt;              /* offset texture coordinate data */
	int ofsTris;            /* offset triangle data */
	int ofsFrames;          /* offset frame data */
	int ofsGLCmds;          /* offset OpenGL command data */
	int ofsEnd;             /* offset end of file */


	const md2Frame_s *pFrame(u32 frameNum) const {
		return (const md2Frame_s*)(((byte*)this)+ofsFrames+frameNum*(sizeof(md2Frame_s)+sizeof(md2Vertex_s)*numVertices));
	}
	const md2Triangle_s *pTri(u32 triNum) const {
		return (const md2Triangle_s*)(((byte*)this)+ofsTris+triNum*sizeof(md2Triangle_s));
	}
	const md2TexCoord_s *pTC(u32 tcNum) const {
		return (const md2TexCoord_s*)(((byte*)this)+ofsSt+tcNum*sizeof(md2TexCoord_s));
	}
	const md2Skin_s *pSkin(u32 skinNum) const {
		return (const md2Skin_s*)(((byte*)this)+ofsSkins+skinNum*sizeof(md2Skin_s));
	}
};

#endif // __MD2_FILE_FORMAT_H__
