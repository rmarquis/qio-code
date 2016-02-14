/*
============================================================================
Copyright (C) 2015 V.

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
// brushside.cpp
#include "brushside.h"
#include <api/mtrAPI.h>
#include <api/coreAPI.h>
#include <shared/textureAxisFromNormal.h>

#define ZERO_EPSILON 1.0E-6

// used only for debugging
int g_allocatedCounter_face = 0;

face_s::face_s() {
	next = 0;
	original = 0;
	face_winding = 0;
	d_texture = 0;
	g_allocatedCounter_face++;
}
face_s::~face_s() {
	if (this->face_winding) {
		delete this->face_winding;
		this->face_winding = 0;
	}

	// allocated counter is only used for debugging
	g_allocatedCounter_face--;
}
void face_s::calculatePlaneFromPoints(){
	plane.fromPoints(planepts[2],planepts[1],planepts[0]);
}	
face_s *face_s::cloneFace() {
	face_s *n = new face_s();
	n->texdef = this->texdef;

	memcpy (n->planepts, this->planepts, sizeof(n->planepts));
	n->plane = this->plane;

	// all other fields are derived, and will be set by Brush_Build
	return n;
}
// TTimo: NOTE: this is never to get called while in brush primitives mode
void face_s::calcTextureVectors(float STfromXYZ[2][4]) const  {
	vec3_c		pvecs[2];
	int			sv, tv;
	float		ang, sinv, cosv;
	float		ns, nt;
	int			i,j;

	 texdef_t *td = (texdef_t*)&this->texdef;
	mtrAPI_i *q = this->d_texture;

	memset (STfromXYZ, 0, 8*sizeof(float));

	//assert(td->scale[0] != 0.f);
	//assert(td->scale[1] != 0.f);
	if (!td->scale[0])
		td->scale[0] =  0.5 ;
	if (!td->scale[1])
		td->scale[1] = 0.5 ;

	// get natural texture axis
	MOD_TextureAxisFromNormal(this->plane.getNormal(), pvecs[0], pvecs[1]);

	// rotate axis
	if (td->rotate == 0)
		{ sinv = 0 ; cosv = 1; }
	else if (td->rotate == 90)
		{ sinv = 1 ; cosv = 0; }
	else if (td->rotate == 180)
		{ sinv = 0 ; cosv = -1; }
	else if (td->rotate == 270)
		{ sinv = -1 ; cosv = 0; }
	else
	{	
		ang = DEG2RAD(td->rotate);
		sinv = sin(ang);
		cosv = cos(ang);
	}

	if (pvecs[0][0])
		sv = 0;
	else if (pvecs[0][1])
		sv = 1;
	else
		sv = 2;
				
	if (pvecs[1][0])
		tv = 0;
	else if (pvecs[1][1])
		tv = 1;
	else
		tv = 2;
					
	for (i=0 ; i<2 ; i++) {
		ns = cosv * pvecs[i][sv] - sinv * pvecs[i][tv];
		nt = sinv * pvecs[i][sv] +  cosv * pvecs[i][tv];
		STfromXYZ[i][sv] = ns;
		STfromXYZ[i][tv] = nt;
	}

	// scale
	for (i=0 ; i<2 ; i++)
		for (j=0 ; j<3 ; j++)
			STfromXYZ[i][j] = STfromXYZ[i][j] / td->scale[i];

	// shift
	STfromXYZ[0][3] = td->shift[0];
	STfromXYZ[1][3] = td->shift[1];

	for (j=0 ; j<4 ; j++) {
		STfromXYZ[0][j] /= q->getImageWidth();
		STfromXYZ[1][j] /= q->getImageHeight();
	}
}

u32 face_s::getMemorySize() const {
	int size = sizeof(face_s);
	if (this->face_winding){
	}
	return size;
}
face_s *face_s::cloneFace() const {
	face_s *n = new face_s();
	n->texdef = this->texdef;
	memcpy(n->planepts, this->planepts, sizeof(n->planepts));
	n->plane = this->plane;
	if (this->face_winding)
		n->face_winding = this->face_winding->cloneWinding();
	else
		n->face_winding = NULL;
	n->d_texture = this->d_texture;
	return n;
}
// compute a determinant using Sarrus rule
//++timo "inline" this with a macro
// NOTE : the three vec3_t are understood as columns of the matrix
float SarrusDet(const vec3_c &a, const vec3_c &b, const vec3_c &c)
{
	return a[0]*b[1]*c[2]+b[0]*c[1]*a[2]+c[0]*a[1]*b[2]
		-c[0]*b[1]*a[2]-a[1]*b[0]*c[2]-a[0]*b[2]*c[1];
}

//++timo replace everywhere texX by texS etc. ( ----> and in q3map !) 
// NOTE : ComputeAxisBase here and in q3map code must always BE THE SAME !
// WARNING : special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere when x == 0
// rotation by (0,RotY,RotZ) assigns X to normal
void ComputeAxisBase(const vec3_c &inNormal, vec3_c &texS, vec3_c &texT)
{
	float RotY,RotZ;
	vec3_c normal = inNormal;
	// do some cleaning
	if (fabs(normal[0])<1e-6)
		normal[0]=0.0f;
	if (fabs(normal[1])<1e-6)
		normal[1]=0.0f;
	if (fabs(normal[2])<1e-6)
		normal[2]=0.0f;
	RotY=-atan2(normal[2],sqrt(normal[1]*normal[1]+normal[0]*normal[0]));
	RotZ=atan2(normal[1],normal[0]);
	// rotate (0,1,0) and (0,0,1) to compute texS and texT
	texS[0]=-sin(RotZ);
	texS[1]=cos(RotZ);
	texS[2]=0;
	// the texT vector is along -Z ( T texture coorinates axis )
	texT[0]=-sin(RotY)*cos(RotZ);
	texT[1]=-sin(RotY)*sin(RotZ);
	texT[2]=-cos(RotY);
}

void face_s::convertFaceToBrushPrimitFace() {
	vec3_c texX,texY;
	vec3_c proj;
	// ST of (0,0) (1,0) (0,1)
	texturedVertex_c ST[3]; // [ point index ] [ xyz ST ]
	// compute axis base
	ComputeAxisBase(this->plane.norm,texX,texY);
	// compute projection vector
	proj = this->plane.norm * this->plane.dist;
	// (0,0) in plane axis base is (0,0,0) in world coordinates + projection on the affine plane
	// (1,0) in plane axis base is texX in world coordinates + projection on the affine plane
	// (0,1) in plane axis base is texY in world coordinates + projection on the affine plane
	// use old texture code to compute the ST coords of these points
	ST[0].setXYZ(proj);
	this->calcTextureCoordinates(ST[0]);
	ST[1].setXYZ(texX);
	ST[1].xyz += proj;
	this->calcTextureCoordinates(ST[1]);
	ST[2].setXYZ(texY);
	ST[2].xyz += proj;
	this->calcTextureCoordinates(ST[2]);
	// compute texture matrix
	this->brushprimit_texdef.coords[0][2]=ST[0][3];
	this->brushprimit_texdef.coords[1][2]=ST[0][4];
	this->brushprimit_texdef.coords[0][0]=ST[1][3]-this->brushprimit_texdef.coords[0][2];
	this->brushprimit_texdef.coords[1][0]=ST[1][4]-this->brushprimit_texdef.coords[1][2];
	this->brushprimit_texdef.coords[0][1]=ST[2][3]-this->brushprimit_texdef.coords[0][2];
	this->brushprimit_texdef.coords[1][1]=ST[2][4]-this->brushprimit_texdef.coords[1][2];
}
// convert a texture matrix between two mtrAPI_i
// if NULL for mtrAPI_i, basic 2x2 texture is assumed ( straight mapping between s/t coordinates and geometric coordinates )
void ConvertTexMatWithQTexture( brushprimit_texdef_s *texMat1, mtrAPI_i *qtex1, brushprimit_texdef_s *texMat2, mtrAPI_i *qtex2 )
{
	float s1,s2;
	s1 = ( qtex1 ? static_cast<float>( qtex1->getImageWidth() ) : 2.0f ) / ( qtex2 ? static_cast<float>( qtex2->getImageWidth() ) : 2.0f );
	s2 = ( qtex1 ? static_cast<float>( qtex1->getImageHeight() ) : 2.0f ) / ( qtex2 ? static_cast<float>( qtex2->getImageHeight() ) : 2.0f );
	texMat2->coords[0][0]=s1*texMat1->coords[0][0];
	texMat2->coords[0][1]=s1*texMat1->coords[0][1];
	texMat2->coords[0][2]=s1*texMat1->coords[0][2];
	texMat2->coords[1][0]=s2*texMat1->coords[1][0];
	texMat2->coords[1][1]=s2*texMat1->coords[1][1];
	texMat2->coords[1][2]=s2*texMat1->coords[1][2];
}

// compute a fake shift scale rot representation from the texture matrix
// these shift scale rot values are to be understood in the local axis base
void TexMatToFakeTexCoords( float texMat[2][3], float shift[2], float *rot, float scale[2] )
{
	// check this matrix is orthogonal
	if (fabs(texMat[0][0]*texMat[0][1]+texMat[1][0]*texMat[1][1])>ZERO_EPSILON)
		g_core->RedWarning("Warning : non orthogonal texture matrix in TexMatToFakeTexCoords\n");
	scale[0]=sqrt(texMat[0][0]*texMat[0][0]+texMat[1][0]*texMat[1][0]);
	scale[1]=sqrt(texMat[0][1]*texMat[0][1]+texMat[1][1]*texMat[1][1]);
	if (scale[0]<ZERO_EPSILON || scale[1]<ZERO_EPSILON)
		g_core->RedWarning("Warning : unexpected scale==0 in TexMatToFakeTexCoords\n");
	// compute rotate value
	if (fabs(texMat[0][0])<ZERO_EPSILON)
	{
		// check brushprimit_texdef[1][0] is not zero
		if (fabs(texMat[1][0])<ZERO_EPSILON)
			g_core->RedWarning("Warning : unexpected texdef[1][0]==0 in TexMatToFakeTexCoords\n");
		// rotate is +-90
		if (texMat[1][0]>0)
			*rot=90.0f;
		else
			*rot=-90.0f;
	}
	else
	*rot = RAD2DEG( atan2( texMat[1][0], texMat[0][0] ) );
	shift[0] = -texMat[0][2];
	shift[1] = texMat[1][2];
}

// compute back the texture matrix from fake shift scale rot
// the matrix returned must be understood as a mtrAPI_i with width=2 height=2 ( the default one )
void FakeTexCoordsToTexMat( float shift[2], float rot, float scale[2], float texMat[2][3] )
{
	texMat[0][0] = scale[0] * cos( DEG2RAD( rot ) );
	texMat[1][0] = scale[0] * sin( DEG2RAD( rot ) );
	texMat[0][1] = -1.0f * scale[1] * sin( DEG2RAD( rot ) );
	texMat[1][1] = scale[1] * cos( DEG2RAD( rot ) );
	texMat[0][2] = -shift[0];
	texMat[1][2] = shift[1];
}



// texture locking
void face_s::moveTexture_BrushPrimit(const vec3_c &delta)
{
	vec3_c texS,texT;
	float tx,ty;
	vec3_t M[3]; // columns of the matrix .. easier that way
	float det;
	vec3_t D[2];
	// compute plane axis base ( doesn't change with translation )
	ComputeAxisBase( this->plane.norm, texS, texT );
	// compute translation vector in plane axis base
	tx = delta.dotProduct( texS );
	ty = delta.dotProduct( texT );
	// fill the data vectors
	M[0][0]=tx; M[0][1]=1.0f+tx; M[0][2]=tx;
	M[1][0]=ty; M[1][1]=ty; M[1][2]=1.0f+ty;
	M[2][0]=1.0f; M[2][1]=1.0f; M[2][2]=1.0f;
	D[0][0]=this->brushprimit_texdef.coords[0][2];
	D[0][1]=this->brushprimit_texdef.coords[0][0]+this->brushprimit_texdef.coords[0][2];
	D[0][2]=this->brushprimit_texdef.coords[0][1]+this->brushprimit_texdef.coords[0][2];
	D[1][0]=this->brushprimit_texdef.coords[1][2];
	D[1][1]=this->brushprimit_texdef.coords[1][0]+this->brushprimit_texdef.coords[1][2];
	D[1][2]=this->brushprimit_texdef.coords[1][1]+this->brushprimit_texdef.coords[1][2];
	// solve
	det = SarrusDet( M[0], M[1], M[2] );
	this->brushprimit_texdef.coords[0][0] = SarrusDet( D[0], M[1], M[2] ) / det;
	this->brushprimit_texdef.coords[0][1] = SarrusDet( M[0], D[0], M[2] ) / det;
	this->brushprimit_texdef.coords[0][2] = SarrusDet( M[0], M[1], D[0] ) / det;
	this->brushprimit_texdef.coords[1][0] = SarrusDet( D[1], M[1], M[2] ) / det;
	this->brushprimit_texdef.coords[1][1] = SarrusDet( M[0], D[1], M[2] ) / det;
	this->brushprimit_texdef.coords[1][2] = SarrusDet( M[0], M[1], D[1] ) / det;
}

// call Face_MoveTexture_BrushPrimit after vec3_t computation
void face_s::shiftTexture_BrushPrimit(int x, int y)
{
	vec3_c texS,texT;
	vec3_c delta;
	ComputeAxisBase( this->plane.norm, texS, texT );
	texS *= x;
	texT *= y;
	delta = texS + texT;
	this->moveTexture_BrushPrimit(delta);
}

// texture locking
// called before the points on the face are actually rotated
void face_s::rotateFaceTexture_BrushPrimit(int nAxis, float fDeg, const vec3_c &vOrigin)
{
	vec3_c texS,texT;			// axis base of the initial plane
	vec3_c vRotate;				// rotation vector
	vec3_c Orig;
	vec3_c rOrig,rvecS,rvecT;	// (0,0) (1,0) (0,1) ( initial plane axis base ) after rotation ( world axis base )
	vec3_c rNormal;				// normal of the plane after rotation
	vec3_c rtexS,rtexT;			// axis base of the rotated plane
	vec3_c lOrig,lvecS,lvecT;	// [2] are not used ( but usefull for debugging )
	vec3_t M[3];
	float det;
	vec3_t D[2];
	// compute plane axis base
	ComputeAxisBase( this->plane.norm, texS, texT );
	// compute coordinates of (0,0) (1,0) (0,1) ( initial plane axis base ) after rotation
	// (0,0) (1,0) (0,1) ( initial plane axis base ) <-> (0,0,0) texS texT ( world axis base )
	// rotation vector
	vRotate.clear();
	vRotate[nAxis]=fDeg;
	Orig.clear();
	vec3_c::vectorRotate( Orig, vRotate, vOrigin, rOrig );
	vec3_c::vectorRotate( texS, vRotate, vOrigin, rvecS );
	vec3_c::vectorRotate( texT, vRotate, vOrigin, rvecT );
	// compute normal of plane after rotation
	vec3_c::vectorRotate( this->plane.norm, vRotate, rNormal );
	// compute rotated plane axis base
	ComputeAxisBase( rNormal, rtexS, rtexT );
	// compute S/T coordinates of the three points in rotated axis base ( in M matrix )
	lOrig[0] = rOrig.dotProduct( rtexS );
	lOrig[1] = rOrig.dotProduct( rtexT );
	lvecS[0] = rvecS.dotProduct( rtexS );
	lvecS[1] = rvecS.dotProduct( rtexT );
	lvecT[0] = rvecT.dotProduct( rtexS );
	lvecT[1] = rvecT.dotProduct( rtexT );
	M[0][0] = lOrig[0]; M[1][0] = lOrig[1]; M[2][0] = 1.0f;
	M[0][1] = lvecS[0]; M[1][1] = lvecS[1]; M[2][1] = 1.0f;
	M[0][2] = lvecT[0]; M[1][2] = lvecT[1]; M[2][2] = 1.0f;
	// fill data vector
	D[0][0]=this->brushprimit_texdef.coords[0][2];
	D[0][1]=this->brushprimit_texdef.coords[0][0]+this->brushprimit_texdef.coords[0][2];
	D[0][2]=this->brushprimit_texdef.coords[0][1]+this->brushprimit_texdef.coords[0][2];
	D[1][0]=this->brushprimit_texdef.coords[1][2];
	D[1][1]=this->brushprimit_texdef.coords[1][0]+this->brushprimit_texdef.coords[1][2];
	D[1][2]=this->brushprimit_texdef.coords[1][1]+this->brushprimit_texdef.coords[1][2];
	// solve
	det = SarrusDet( M[0], M[1], M[2] );
	this->brushprimit_texdef.coords[0][0] = SarrusDet( D[0], M[1], M[2] ) / det;
	this->brushprimit_texdef.coords[0][1] = SarrusDet( M[0], D[0], M[2] ) / det;
	this->brushprimit_texdef.coords[0][2] = SarrusDet( M[0], M[1], D[0] ) / det;
	this->brushprimit_texdef.coords[1][0] = SarrusDet( D[1], M[1], M[2] ) / det;
	this->brushprimit_texdef.coords[1][1] = SarrusDet( M[0], D[1], M[2] ) / det;
	this->brushprimit_texdef.coords[1][2] = SarrusDet( M[0], M[1], D[1] ) / det;
}

// compute texture coordinates for the winding points
void face_s::calcBrushPrimitTextureCoordinates(texturedWinding_c *w)
{
	vec3_c texX,texY;
	float x,y;
	// compute axis base
	ComputeAxisBase(this->plane.norm,texX,texY);
	// in case the texcoords matrix is empty, build a default one
	// same behaviour as if scale[0]==0 && scale[1]==0 in old code
	if (this->brushprimit_texdef.coords[0][0]==0 && this->brushprimit_texdef.coords[1][0]==0 && this->brushprimit_texdef.coords[0][1]==0 && this->brushprimit_texdef.coords[1][1]==0)
	{
		this->brushprimit_texdef.coords[0][0] = 1.0f;
		this->brushprimit_texdef.coords[1][1] = 1.0f;
		ConvertTexMatWithQTexture( &this->brushprimit_texdef, NULL, &this->brushprimit_texdef, this->d_texture );
	}
	int i;
    for (i=0 ; i<w->size() ; i++)
	{
		x=w->getXYZ(i).dotProduct(texX);
		y=w->getXYZ(i).dotProduct(texY);

		w->setTC(i,this->brushprimit_texdef.coords[0][0]*x+this->brushprimit_texdef.coords[0][1]*y+this->brushprimit_texdef.coords[0][2],this->brushprimit_texdef.coords[1][0]*x+this->brushprimit_texdef.coords[1][1]*y+this->brushprimit_texdef.coords[1][2]);
	}
}

void face_s::calcTextureCoordinates (texturedVertex_c &out) const {
	// out layout is: xyz st
	float STfromXYZ[2][4];
	this->calcTextureVectors(STfromXYZ);
	out[3] = out.dotProduct(STfromXYZ[0]) + STfromXYZ[0][3];
	out[4] = out.dotProduct(STfromXYZ[1]) + STfromXYZ[1][3];
}
void face_s::fitTexture(int nHeight, int nWidth) {
	aabb bounds;
	int i;
	float width, height, temp;
	float rot_width, rot_height;
	float cosv,sinv,ang;
	float min_t, min_s, max_t, max_s;
	float s,t;
	vec3_c	vecs[2];
	vec3_t   coords[4];

	if (nHeight < 1)
	{
		nHeight = 1;
	}
	if (nWidth < 1)
	{
		nWidth = 1;
	}

	bounds.clear();

	texdef_t *td = &this->texdef;
	texturedWinding_c *w = this->face_winding;
	if (!w)
	{
		return;
	}
	for (i=0 ; i<w->size() ; i++)
	{
		bounds.addPoint(w->getXYZ(i));
	}
	// 
	// get the current angle
	//
	ang = DEG2RAD(td->rotate);
	sinv = sin(ang);
	cosv = cos(ang);

	// get natural texture axis
	MOD_TextureAxisFromNormal(this->plane.getNormal(), vecs[0], vecs[1]);

	min_s = bounds.getMins().dotProduct(vecs[0]);
	min_t = bounds.getMins().dotProduct(vecs[1]);
	max_s = bounds.getMaxs().dotProduct(vecs[0]);
	max_t = bounds.getMaxs().dotProduct(vecs[1]);
	width = max_s - min_s;
	height = max_t - min_t;
	coords[0][0] = min_s;
	coords[0][1] = min_t;
	coords[1][0] = max_s;
	coords[1][1] = min_t;
	coords[2][0] = min_s;
	coords[2][1] = max_t;
	coords[3][0] = max_s;
	coords[3][1] = max_t;
	min_s = min_t = 99999;
	max_s = max_t = -99999;
	for (i=0; i<4; i++)
	{
		s = cosv * coords[i][0] - sinv * coords[i][1];
		t = sinv * coords[i][0] + cosv * coords[i][1];
		if (i&1)
		{
			if (s > max_s) 
			{
				max_s = s;
			}
		}
		else
		{
			if (s < min_s) 
			{
				min_s = s;
			}
			if (i<2)
			{
				if (t < min_t) 
				{
					min_t = t;
				}
			}
			else
			{
				if (t > max_t) 
				{
					max_t = t;
				}
			}
		}
	}
	rot_width =  (max_s - min_s);
	rot_height = (max_t - min_t);
	td->scale[0] = -(rot_width/((float)(this->d_texture->getImageWidth()*nWidth)));
	td->scale[1] = -(rot_height/((float)(this->d_texture->getImageHeight()*nHeight)));

	td->shift[0] = min_s/td->scale[0];
	temp = (int)(td->shift[0] / (this->d_texture->getImageWidth()*nWidth));
	temp = (temp+1)*this->d_texture->getImageWidth()*nWidth;
	td->shift[0] = (int)(temp - td->shift[0])%(this->d_texture->getImageWidth()*nWidth);

	td->shift[1] = min_t/td->scale[1];
	temp = (int)(td->shift[1] / (this->d_texture->getImageHeight()*nHeight));
	temp = (temp+1)*(this->d_texture->getImageHeight()*nHeight);
	td->shift[1] = (int)(temp - td->shift[1])%(this->d_texture->getImageHeight()*nHeight);
}