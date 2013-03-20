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
// img_devil.cpp - loading images trough DEVIL library
#include "img_local.h"
#include <shared/str.h>
#include <api/coreAPI.h>
#include <qcommon/q_shared.h>
#include <api/vfsAPI.h>


struct tgaHeader_s {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
};

void IMG_LoadTGA(const char *name, byte **pic, byte *buffer, u32 *width, u32 *height) {
	*pic = 0;

	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	tgaHeader_s	targa_header;
	byte		*targa_rgba;

	byte *buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	targa_header.colormap_index =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_length =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.y_origin =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.width =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.height =  ( *(short *)buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10
		&& targa_header.image_type != 3 ) 
	{
		g_core->Print("IMG_LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n");
		*pic = 0;
		return;
	}

	if ( targa_header.colormap_type != 0 )
	{
		g_core->Print("IMG_LoadTGA: colormaps not supported\n" );
		*pic = 0;
		return;
	}

	if ( ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) && targa_header.image_type != 3 )
	{
		g_core->Print("IMG_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		*pic = 0;
		return;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = (byte *)malloc (numPixels*4); 
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if ( targa_header.image_type==2 || targa_header.image_type == 3 )
	{ 
		// Uncompressed RGB or gray scale image
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) 
				{
					
				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					g_core->Print("IMG_LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
					*pic = 0;
					return;
					break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
						default:
							g_core->Print("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
							*pic = 0;
							return;
							break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
							default:
								g_core->Print("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
								*pic = 0;
								return;
								break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	// instead we just print a warning
	if (targa_header.attributes & 0x20) {
		g_core->Print("IMG_LoadTGA: %s TGA file header declares top-down image, ignoring\n", name);
	}
}

#if 1
#define IMAGE_USE_VTF_LIB
#endif 

#ifdef IMAGE_USE_VTF_LIB
#include <VTFLib.h>
#pragma comment( lib, "VTFLib.lib" )

static bool vl_ready = false;
bool IMG_LoadVTF(const char *fname, const byte *buffer, const u32 bufferLen, byte **pic, u32 *width, u32 *height) {
	if(vl_ready==false) {
		vlInitialize();
		vl_ready = true;
	}
	vlUInt img = 1;
	vlCreateImage(&img);
	vlBindImage(img);
	bool res = vlImageLoadLump(buffer,bufferLen,false);
	bool loaded = vlImageIsLoaded();
	vlUInt w = vlImageGetWidth();
	vlUInt h = vlImageGetHeight();
	vlUInt d = vlImageGetDepth();
	vlUInt faceCount = vlImageGetFaceCount();
	vlBool hasThumbnail = vlImageGetHasThumbnail();
	vlUInt frameCount = vlImageGetFrameCount();
	*width = w;
	*height = h;
	VTFImageFormat format = vlImageGetFormat();
	vlByte *data = vlImageGetData(0, 0, 0, 0);
	byte *out = *pic = (byte*)malloc(w*h*4);
	vlImageConvertToRGBA8888(data,out,w,h,format);
	vlDeleteImage(img);
	return false; // ok
}

#endif // IMAGE_USE_VTF_LIB

#include <IL/il.h>

void IMG_InitDevil() {
	//initialize devIL
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	ilEnable(IL_TYPE_SET);
	ilTypeFunc(IL_UNSIGNED_BYTE);
}

struct imgType_s {
	const char *ext;
};
imgType_s img_types [] = {
	"tga",
	"jpg",
	"bmp",
	"png",
	"dds",
	"ftx",
	"vtf",
};

int ILTypeForExt(const char *s)
{
	if(!stricmp(s,"tga"))
		return IL_TGA;
	if(!stricmp(s,"jpg"))
		return IL_JPG;
	if(!stricmp(s,"bmp"))
		return IL_BMP;
	if(!stricmp(s,"dds"))
		return IL_DDS;
	if(!stricmp(s,"ftx"))
		return IL_FTX;
	if(!stricmp(s,"png"))
		return IL_PNG;
	if(!stricmp(s,"vtf"))
		return IL_VTF;
	if(!stricmp(s,"wal"))
		return IL_WAL;
	if(!stricmp(s,"gif"))
		return IL_GIF;
	g_core->Print("WARNING: unknown image file extension %s \n",s);
	return IL_TGA;
}
u32 img_numSupportedImageTypes = sizeof(img_types) / sizeof(img_types[0]);

char lastValidFName[256];
const char *IMG_LoadImageInternal( const char *fname, byte **imageData, u32 *width, u32 *height ) {
	*imageData = 0;
	*width = 0;
	*height = 0;

	if(fname == 0 || fname[0] == 0)
		return 0;

	const char *ext = G_strgetExt(fname);

	str s = fname;
	s.defaultExtension("tga");

	byte *buf = 0;
	u32 len;
	len = g_vfs->FS_ReadFile(s,(void**)&buf);
	if(buf == 0) {
		for(u32 i = 0; i < img_numSupportedImageTypes; i++) {
			imgType_s *t = &img_types[i];
			s.setExtension(t->ext);
			len = g_vfs->FS_ReadFile(s,(void**)&buf);
			if(buf)
				break;
		}
	}
	if(buf == 0)
		return 0; // cannot open image file

	ext = s.getExt();

	if(1 && !stricmp(ext,"tga")) {
		// devil tga loader swaps colors....
		IMG_LoadTGA( s, imageData, buf, width, height ); 
		g_vfs->FS_FreeFile(buf);
		strcpy(lastValidFName,s.c_str());
		return lastValidFName;
	}
#ifdef IMAGE_USE_VTF_LIB
	if(1 && !stricmp(ext,"vtf")) {
		// we have a better loader for vtf...
		// Devil VTF loader cant load SOME OF vtf types
		IMG_LoadVTF(s,buf,len,imageData,width,height);
		g_vfs->FS_FreeFile(buf);
		strcpy(lastValidFName,s.c_str());
		return lastValidFName;
	}
#endif // IMAGE_USE_VTF_LIB
	
    ILuint ilTexture;
    ilGenImages(1, &ilTexture);
    ilBindImage(ilTexture);


	int type = ILTypeForExt(ext); // get IL_***
	//g_core->Print("Calling ilLoadL for %s with len %i\n",s.c_str(),len);
	ILboolean done = 0;
	done = ilLoadL(type,buf,len);
	//g_core->Print("result %i\n",done);
	g_vfs->FS_FreeFile(buf);
	if(!done) {
		ilBindImage(0);
		ilDeleteImages(1, &ilTexture);
		g_core->Print("IMG_LoadImageInternal: error while reading image file %s\n",s.c_str());
		return 0;
	}
	strcpy(lastValidFName,s.c_str());
	*width = ilGetInteger(IL_IMAGE_WIDTH);
	*height = ilGetInteger(IL_IMAGE_HEIGHT);
	int d = ilGetInteger(IL_IMAGE_BPP);
	if(d!=4) {
		d = 4;
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	}
	*imageData = (unsigned char*)malloc((*width) * (*height) * (d));
	memcpy(*imageData, ilGetData(), (*width) * (*height) * (d));
    ilBindImage(0);
    ilDeleteImages(1, &ilTexture);
	return lastValidFName;
}