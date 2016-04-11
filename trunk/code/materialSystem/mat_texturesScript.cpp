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
// mat_texturesScript.cpp - Doom3 textures script execution
#include "mat_local.h"
#include <api/imgAPI.h>
#include <shared/parser.h>
#include <shared/textureWrapMode.h>
#include <math/vec3.h>

byte *IMG_Dropsample( const byte *in, int inwidth, int inheight,  
							int outwidth, int outheight ) {
	int		i, j, k;
	const byte	*inrow;
	const byte	*pix1;
	byte		*out, *out_p;

	out = (byte *)malloc( outwidth * outheight * 4 );
	out_p = out;

	for (i=0 ; i<outheight ; i++, out_p += outwidth*4 ) {
		inrow = in + 4*inwidth*(int)((i+0.25)*inheight/outheight);
		for (j=0 ; j<outwidth ; j++) {
			k = j * inwidth / outwidth;
			pix1 = inrow + k * 4;
			out_p[j*4+0] = pix1[0];
			out_p[j*4+1] = pix1[1];
			out_p[j*4+2] = pix1[2];
			out_p[j*4+3] = pix1[3];
		}
	}

	return out;
}
static void IMG_AddNormalMaps(byte *data1, int width1, int height1, byte *data2, int width2, int height2) {
	int		i, j;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if ( width2 != width1 || height2 != height1 ) {
		newMap = IMG_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	} else {
		newMap = NULL;
	}

	// add the normal change from the second and renormalize
	for ( i = 0 ; i < height1 ; i++ ) {
		for ( j = 0 ; j < width1 ; j++ ) {
			byte	*d1, *d2;
			vec3_c	n;
			float   len;

			d1 = data1 + ( i * width1 + j ) * 4;
			d2 = data2 + ( i * width1 + j ) * 4;

			n[0] = ( d1[0] - 128 ) / 127.0;
			n[1] = ( d1[1] - 128 ) / 127.0;
			n[2] = ( d1[2] - 128 ) / 127.0;

			// There are some normal maps that blend to 0,0,0 at the edges
			// this screws up compression, so we try to correct that here by instead fading it to 0,0,1
			len = n.len();
			if ( len < 1.0f ) {
				n[2] = sqrt(1.0 - (n[0]*n[0]) - (n[1]*n[1]));
			}

			n[0] += ( d2[0] - 128 ) / 127.0;
			n[1] += ( d2[1] - 128 ) / 127.0;
			n.normalize();

			d1[0] = (byte)(n[0] * 127 + 128);
			d1[1] = (byte)(n[1] * 127 + 128);
			d1[2] = (byte)(n[2] * 127 + 128);
			d1[3] = 255;
		}
	}

	if ( newMap ) {
		free( newMap );
	}
}
static void IMG_AddImage( byte *data1, int width1, int height1, byte *data2, int width2, int height2 ) {
	int		i, j;
	int		c;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if ( width2 != width1 || height2 != height1 ) {
		newMap = IMG_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	} else {
		newMap = NULL;
	}


	c = width1 * height1 * 4;

	for ( i = 0 ; i < c ; i++ ) {
		j = data1[i] + data2[i];
		if ( j > 255 ) {
			j = 255;
		}
		data1[i] = j;
	}

	if ( newMap ) {
		free( newMap );
	}
}

/*
=================
R_HeightmapToNormalMap

it is not possible to convert a heightmap into a normal map
properly without knowing the texture coordinate stretching.
We can assume constant and equal ST vectors for walls, but not for characters.
=================
*/
static void IMG_HeightmapToNormalMap( byte *data, int width, int height, float scale ) {
	int		i, j;
	byte	*depth;

	scale = scale / 256;

	// copy and convert to grey scale
	j = width * height;
	depth = (byte *)malloc( j );
	for ( i = 0 ; i < j ; i++ ) {
		depth[i] = ( data[i*4] + data[i*4+1] + data[i*4+2] ) / 3;
	}

	vec3_c	dir, dir2;
	for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < width ; j++ ) {
			int		d1, d2, d3, d4;
			int		a1, a2, a3, a4;

			// FIXME: look at five points?

			// look at three points to estimate the gradient
			a1 = d1 = depth[ ( i * width + j ) ];
			a2 = d2 = depth[ ( i * width + ( ( j + 1 ) & ( width - 1 ) ) ) ];
			a3 = d3 = depth[ ( ( ( i + 1 ) & ( height - 1 ) ) * width + j ) ];
			a4 = d4 = depth[ ( ( ( i + 1 ) & ( height - 1 ) ) * width + ( ( j + 1 ) & ( width - 1 ) ) ) ];

			d2 -= d1;
			d3 -= d1;

			dir[0] = -d2 * scale;
			dir[1] = -d3 * scale;
			dir[2] = 1;
			dir.normalize();

			a1 -= a3;
			a4 -= a3;

			dir2[0] = -a4 * scale;
			dir2[1] = a1 * scale;
			dir2[2] = 1;
			dir2.normalize();
	
			dir += dir2;
			dir.normalize();

			a1 = ( i * width + j ) * 4;
			data[ a1 + 0 ] = (byte)(dir[0] * 127 + 128);
			data[ a1 + 1 ] = (byte)(dir[1] * 127 + 128);
			data[ a1 + 2 ] = (byte)(dir[2] * 127 + 128);
			data[ a1 + 3 ] = 255;
		}
	}


	free( depth );
}
static void IMG_MakeAlpha( byte *data, int width, int height ) {
	// average RGB into alpha, then set RGB to white
	int c = width * height * 4;
	for (int i = 0 ; i < c ; i+=4) {
		data[i+3] = ( data[i+0] + data[i+1] + data[i+2] ) / 3;
		data[i+0] = 
		data[i+1] = 
		data[i+2] = 255;
	}
}
static void IMG_InvertColor( byte *data, int width, int height ) {
	int c = width * height * 4;
	for (int i = 0 ; i < c ; i+=4) {
		data[i+0] = 255 - data[i+0];
		data[i+1] = 255 - data[i+1];
		data[i+2] = 255 - data[i+2];
	}
}
static void IMG_Invert( byte *data, int width, int height ) {
	int c = width * height * 4;
	for (int i = 0 ; i < c ; i+=4) {
		data[i+0] = 255 - data[i+0];
		data[i+1] = 255 - data[i+1];
		data[i+2] = 255 - data[i+2];
		data[i+3] = 255 - data[i+3];
	}
}
static void IMG_MakeIntensity( byte *data, int width, int height ) {
	// copy red to green, blue, and alpha
	int c = width * height * 4;
	for ( int i = 0 ; i < c ; i+=4 ) {
		data[i+1] = 
		data[i+2] = 
		data[i+3] = data[i];
	}
}
// parse Doom3 image script.
// Example:
//	"bumpmap 	addnormals (textures/tech_floor/biotechfloor3b_local.tga, 
//			heightmap (textures/tech_floor/biotechfloor3_h.tga, 6))"
class image_c {
	str fname;
	byte *data;
	u32 w, h;
public:
	image_c() {
		data = 0;
		w = h = 0;
	}
	bool loadFromFile(const char *inName) {
		if(g_img == 0)
			return true;
		this->fname = g_img->loadImage(inName,&data,&w,&h);
		if(data == 0)
			return true; // error
		return false; // ok
	}
	~image_c() {
		if(data) {
			g_img->freeImageData(data);
		}
	}

	void addImage(image_c *other) {
		IMG_AddImage(data,w,h,other->data,other->w,other->h);
	}
	void addNormalsFrom(image_c *other) {
		IMG_AddNormalMaps(data,w,h,other->data,other->w,other->h);
	}
	void convertHeightMapToNormalMap(float level) {
		IMG_HeightmapToNormalMap(data,w,h,level);
	}
	void makeAlpha() {
		IMG_MakeAlpha(data,w,h);
	}
	//void makeRGB(byte r, byte g, byte b) {
	//	w = h = 16;
	//	data = 
	//}
	void invertColor() {
		IMG_InvertColor(data,w,h);
	}
	void invert() {
		IMG_Invert(data,w,h);
	}
	void makeIntensity() {
		IMG_MakeIntensity(data,w,h);
	}
	const byte *getData() const {
		return data;
	}
	u32 getW() const {
		return w;
	}
	u32 getH() const {
		return h;
	}
};
image_c *IMG_LoadImage(const char *fname) {
	image_c *ret = new image_c;
	if(ret->loadFromFile(fname)) {
		delete ret;
		return 0;
	}
	return ret;
}
image_c *MAT_ParseImageScript_r(parser_c &p) {
	if(p.atWord_dontNeedWS("addnormals")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"addnormals\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *img0 = MAT_ParseImageScript_r(p);
		if(p.atChar(',') == false) {
			g_core->RedWarning("Expected ',' inside \"addnormals\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *img1 = MAT_ParseImageScript_r(p);
		image_c *ret = img0;
		if(img0 && img1) {
			img0->addNormalsFrom(img1);
			delete img1;
			img1 = 0;
		} else {
			ret = 0;
			if(img0) {
				delete img0;
				img0 = 0;
			}
			if(img1) {
				delete img1;
				img1 = 0;
			}
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"addnormals\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return ret;
	} else if(p.atWord_dontNeedWS("heightmap")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"heightmap\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *heightMapImage = MAT_ParseImageScript_r(p);
		if(p.atChar(',') == false) {
			g_core->RedWarning("Expected ',' inside \"addnormals\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		float level = atoi(p.getToken(")"));
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"heightmap\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		if(heightMapImage) {
			heightMapImage->convertHeightMapToNormalMap(level);
		}
		return heightMapImage;
	} else if(p.atWord_dontNeedWS("makealpha")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"makealpha\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *alphaImage = MAT_ParseImageScript_r(p);
		if(alphaImage){
			alphaImage->makeAlpha();
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"makealpha\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return alphaImage;	

	} else if(p.atWord_dontNeedWS("invertColor")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"invertColor\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *imageToInvert = MAT_ParseImageScript_r(p);
		if(imageToInvert){
			imageToInvert->invertColor();
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"invertColor\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return imageToInvert;	
	} else if(p.atWord_dontNeedWS("invert")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"invert\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *imageToInvert = MAT_ParseImageScript_r(p);
		if(imageToInvert){
			imageToInvert->invert();
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"invert\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return imageToInvert;	
	} else if(p.atWord_dontNeedWS("makeintensity")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"makeintensity\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *intensityImage = MAT_ParseImageScript_r(p);
		if(intensityImage){
			intensityImage->makeIntensity();
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"makeintensity\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return intensityImage;	
	} else if(p.atWord_dontNeedWS("add")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"add\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *img0 = MAT_ParseImageScript_r(p);
		if(p.atChar(',') == false) {
			g_core->RedWarning("Expected ',' inside \"add\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		image_c *img1 = MAT_ParseImageScript_r(p);
		image_c *ret = img0;
		if(img0 && img1) {
			img0->addImage(img1);
			delete img1;
			img1 = 0;
		} else {
			ret = 0;
			if(img0) {
				delete img0;
				img0 = 0;
			}
			if(img1) {
				delete img1;
				img1 = 0;
			}
		}
		if(p.atChar(')') == false) {
			g_core->RedWarning("Expected ')' after \"add\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		return ret;
	} else if(p.atWord_dontNeedWS("makeRGB")) {
		if(p.atChar('(') == false) {
			g_core->RedWarning("Expected '(' after \"makeRGB\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
			return 0; // error
		}
		// makeRGB(1,1,1)
		float val[3];
		for(u32 i = 0; i < 3; i++) {
			if(i == 2) {		
				const char *s = p.getToken(")");	
				val[i] = atof(s);
				if(!p.atChar(')')) {		
					g_core->RedWarning("Expected ')' after \"makeRGB\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
				}
			} else {
				const char *s = p.getToken(",");
				val[i] = atof(s);
				if(!p.atChar(',')) {
					g_core->RedWarning("Expected ',' inside \"makeRGB\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
				}
			}
		}
		image_c *rgbImage = new image_c;
		if(rgbImage){
			// hack
			char tmp[128];
			sprintf(tmp,"(%f %f %f)",val[0],val[1],val[2]);
			rgbImage->loadFromFile(tmp);
		}
		return rgbImage;	
	} else {
		str texName = p.getToken(",)");
		image_c *newImage = IMG_LoadImage(texName);
		return newImage;
	}
}
class textureAPI_i *MAT_ParseImageScript(parser_c &p) {
	// special case for eg. "(1 1 1)" single-colro images (Qio-specific?)
	if(p.isNextTokenInQuotationMarks()) {
		str texName = p.getToken();
		return MAT_RegisterTexture(texName,TWM_REPEAT);
	}
	const char *startPos = p.getCurDataPtr();
	image_c *finalImage = MAT_ParseImageScript_r(p);
	const char *endPos = p.getCurDataPtr();
	str texName;
	texName.setFromTo(startPos,endPos);
	if(finalImage == 0) {
		// HACK for Dim's obj_mp_office where texure names have (32) inside them
		if(texName.countCharacter('(') != texName.countCharacter(')')) {
			texName.append(p.getNextWordInLine());
		}
		return MAT_RegisterTexture(texName,TWM_REPEAT);
	}
	class textureAPI_i *ret = MAT_CreateTexture(texName, finalImage->getData(), finalImage->getW(), finalImage->getH(),4);
	delete finalImage;
	return ret;
}