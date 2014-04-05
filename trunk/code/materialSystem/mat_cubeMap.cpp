/*
============================================================================
Copyright (C) 2014 V.

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
// mat_cubeMap.cpp
#include <api/cubeMapAPI.h>
#include <api/rbAPI.h>
#include <api/imgAPI.h>
#include <api/coreAPI.h>
#include <shared/str.h>
#include <shared/hashTableTemplate.h>

const char *cameraCubeMapSufixes[6] = { 
	"_forward", "_back",
	"_left", "_right", 
	"_up", "_down" };
const char *generalCubeMapSufixes[6] = { 
	"_px", "_nx",
	"_py", "_ny", 
	"_pz", "_nz" };
enum cubeMapDataType_e {
	CMDT_NOT_SET,
	CMDT_CAMERA,
	CMDT_OTHER,
};

class cubemapIMPL_c : public cubeMapAPI_i {
	str name;
	union {
		u32 handleU32;
		void *handleV;
	};
	cubemapIMPL_c *hashNext;
public:
	cubemapIMPL_c() {
		hashNext = 0;
		handleV = 0;
	}
	~cubemapIMPL_c() {
		if(rb == 0)
			return;
		rb->freeCubeMap(this);
	}

	// returns the path to the texture file (with extension)
	virtual const char *getName() const {
		return name;
	}
	void setName(const char *newName) {
		name = newName;
	}
	virtual void *getInternalHandleV() const {
		return handleV;
	}
	virtual void setInternalHandleV(void *newHandle) {
		handleV = newHandle;
	}
	virtual u32 getInternalHandleU32() const {
		return handleU32;
	}
	virtual void setInternalHandleU32(u32 newHandle) {
		handleU32 = newHandle;
	}

	void loadCubeMap() {
		if(handleV)
			rb->freeCubeMap(this);
		imageData_s imgs[6];
		cubeMapDataType_e type = CMDT_NOT_SET;
		for(u32 i = 0; i < 6; i++) {
			imageData_s &img = imgs[i];
			str fullName = this->name;
			fullName.append(cameraCubeMapSufixes[i]);
			g_img->loadImage(fullName.c_str(),&img.pic,&img.w,&img.h);
			if(img.pic == 0) {
				fullName = this->name;
				fullName.append(generalCubeMapSufixes[i]);
				g_img->loadImage(fullName.c_str(),&img.pic,&img.w,&img.h);
				if(img.pic) {
					type = CMDT_OTHER;
				} else {
					g_core->RedWarning("cubemapIMPL_c::loadCubeMap: cannot find side %i for cubemap %s\n",i,this->name.c_str());
				}
			} else {
				type = CMDT_CAMERA;
			}
		}
		if(type == CMDT_CAMERA) {
			for(u32 i = 0; i < 6; i++) {
				imageData_s &img = imgs[i];
				switch(i) {
					case 0: 
						g_img->rotatePic(img.pic,img.w);
					break;
					case 1:	// back
						g_img->rotatePic(img.pic,img.w);
						g_img->horizontalFlip(img.pic,img.w,img.h);
						g_img->verticalFlip(img.pic,img.w,img.h);
					break;
					case 2:	// left
						g_img->verticalFlip(img.pic,img.w,img.h);
					break;
					case 3:	// right
						g_img->horizontalFlip(img.pic,img.w,img.h);
					break;
					case 4:	// up
						g_img->rotatePic(img.pic,img.w);
					break;
					case 5: // down
						g_img->rotatePic(img.pic,img.w);
					break;
				}
			}
		}
		rb->uploadCubeMap(this,imgs);
		for(u32 i = 0; i < 6; i++) {
			imageData_s &img = imgs[i];
			if(img.pic) {
				g_img->freeImageData(img.pic);
			}
		}
	}
	cubemapIMPL_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(cubemapIMPL_c *p) {
		hashNext = p;
	}
};

static hashTableTemplateExt_c<cubemapIMPL_c> mat_cubeMaps;

class cubeMapAPI_i *MAT_RegisterCubeMap(const char *texName, bool forceReload) {
	cubemapIMPL_c *ret = mat_cubeMaps.getEntry(texName);
	if(ret) {
		if(forceReload) {
			ret->loadCubeMap();
		}
		return ret;
	}
	ret = new cubemapIMPL_c;
	ret->setName(texName);
	ret->loadCubeMap();
	if(mat_cubeMaps.addObject(ret)) {
		mat_cubeMaps.addObject(ret,true);
	}
	return ret;
}
void MAT_FreeAllCubeMaps() {
	for(u32 i = 0; i < mat_cubeMaps.size(); i++) {
		cubemapIMPL_c *t = mat_cubeMaps[i];
		delete t;
		mat_cubeMaps[i] = 0;
	}
	mat_cubeMaps.clear();
}
