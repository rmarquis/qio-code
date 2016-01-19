/*
============================================================================
Copyright (C) 2010 V.

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
#ifndef __MDX_FILE_FORMAT_H__
#define __MDX_FILE_FORMAT_H__

/*
==============================================================================

MDX file format (Wolfenstein Skeletal Data)

version history:
	1 - initial version
	2 - moved parentOffset from the mesh to the skeletal data file

==============================================================================
*/

#define MDX_IDENT           ( ( 'W' << 24 ) + ( 'X' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDX_VERSION         2

#define MDX_ROTATION_SCALE   ( 360.0 / 65536.0 )

struct mdxBoneFrameCompressed_t {
	short           angles[4];	// to be converted to axis at run-time (this is also better for lerping)
	short           ofsAngles[2];	// PITCH/YAW, head in this direction from parent to go to the offset position

	vec3_c getAngles() const {
		vec3_c ret;
		ret.x = double(angles[0]) * MDX_ROTATION_SCALE;
		ret.y = double(angles[1]) * MDX_ROTATION_SCALE;
		ret.z = double(angles[2]) * MDX_ROTATION_SCALE;
		return ret;
	}
	vec3_c getOfsAngles() const {
		vec3_c ret;
		ret.x = double(ofsAngles[0]) * MDX_ROTATION_SCALE;
		ret.y = double(ofsAngles[1]) * MDX_ROTATION_SCALE;
		ret.z = 0;
		return ret;
	}
};

struct mdxFrame_t {
	vec3_t          bounds[2];	// bounds of this frame
	vec3_t          localOrigin;	// midpoint of bounds, used for sphere cull
	float           radius;		// dist from localOrigin to corner
	vec3_t          parentOffset;	// one bone is an ascendant of all other bones, it starts the hierachy at this position

	const mdxBoneFrameCompressed_t *pBone(u32 boneNum) const {
		return (mdxBoneFrameCompressed_t*)(((byte*)this)+sizeof(*this)+sizeof(mdxBoneFrameCompressed_t)*boneNum);
	}
};

struct mdxBoneInfo_t {
	char            name[64];	// name of bone
	int             parent;		// not sure if this is required, no harm throwing it in
	float           torsoWeight;	// scale torso rotation about torsoParent by this
	float           parentDist;
	int             flags;
};

struct mdxHeader_t {
	int             ident;
	int             version;

	char            name[64];	// model name

	// bones are shared by all levels of detail
	int             numFrames;
	int             numBones;
	int             ofsFrames;	// (mdxFrame_t + mdxBoneFrameCompressed_t[numBones]) * numframes
	int             ofsBones;	// mdxBoneInfo_t[numBones]
	int             torsoParent;	// index of bone that is the parent of the torso

	int             ofsEnd;		// end of file

	const mdxBoneInfo_t *pBone(u32 boneNum) const {
		return (const mdxBoneInfo_t *)(((byte*)this)+this->ofsBones+boneNum*sizeof(mdxBoneInfo_t));
	}
	const mdxFrame_t *pFrame(u32 frameNum) const {
		return (const mdxFrame_t *)(((byte*)this)+this->ofsFrames+frameNum*(sizeof(mdxFrame_t)+this->numBones*sizeof(mdxBoneFrameCompressed_t)));
	}
};

#endif // __MDX_FILE_FORMAT_H__
