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
// vvdFileFormat.h - Half Life 2 .vvd file format
#ifndef __VVD_V4_FILE_FORMAT_H__
#define __VVD_V4_FILE_FORMAT_H__

//
// VVD files are used to store vertices data of .mdl models
//

// little-endian "IDSV"
#define MODEL_VERTEX_FILE_ID		(('V'<<24)+('S'<<16)+('D'<<8)+'I')
#define MODEL_VERTEX_FILE_VERSION	4
				
struct vvd4FileHeader_s {
	int		id;								// MODEL_VERTEX_FILE_ID
	int		version;						// MODEL_VERTEX_FILE_VERSION
	long	checksum;						// same as in mdl file, ensures sync
	int		numLODs;						// num of valid lods
	int		numLODVertexes[8];	// num verts for desired root lod
	int		numFixups;						// num of vertexFileFixup_s
	int		ofsFixupTable;				// offset from base to fixup table
	int		ofsVertexData;				// offset from base to vertex block
	int		ofsTangentData;				// offset from base to tangent block


	byte *getVertexData() {
		return (((byte*)this)+this->ofsVertexData);
	}
	byte *getTangentData() {
		return (((byte*)this)+this->ofsTangentData);
	}
};

// apply sequentially to lod sorted vertex and tangent pools to re-establish mesh order
struct vertexFileFixup_s {
	int		lod;				// used to skip culled root lod
	int		sourceVertexID;		// absolute index from start of vertex/tangent blocks
	int		numVertexes;
};

#endif // __VVD_V4_FILE_FORMAT_H__
