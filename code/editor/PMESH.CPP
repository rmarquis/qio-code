/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// 
// Preliminary patch stuff
//
// 

#include "stdafx.h"
#include "qe3.h"
#include "DialogInfo.h"
#include "CapDialog.h"

#include <math/math.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/textureAPI.h>

// externs
extern void MemFile_fprintf(CMemFile* pMemFile, const char* pText, ...);
extern face_s *Face_Alloc( void );
void _Write3DMatrix (FILE *f, int y, int x, int z, float *m);
void _Write3DMatrix (CMemFile *f, int y, int x, int z, float *m);



#define	CBLOCK_SUBDIVISIONS	6 


patchMesh_c* MakeNewPatch()
{
	return new patchMesh_c();
}

// used for a save spot
patchMesh_c patchSave;

// HACK: for tracking which view generated the click
// as we dont want to deselect a point on a same point
// click if it is from a different view
int  g_nPatchClickedView = -1;
bool g_bSameView = false;


// globals
bool g_bPatchShowBounds = true;
bool g_bPatchWireFrame = false;
bool g_bPatchWeld = true;
bool g_bPatchDrillDown = true;
bool g_bPatchInsertMode = false;
bool g_bPatchBendMode = false;
int  g_nPatchBendState = -1;
int  g_nPatchInsertState = -1;
int  g_nBendOriginIndex = 0;
vec3_c g_vBendOrigin;

bool g_bPatchAxisOnRow = true;
int  g_nPatchAxisIndex = 0;
bool g_bPatchLowerEdge = true;

// BEND states
enum
{
	BEND_SELECT_ROTATION = 0,
	BEND_SELECT_ORIGIN,
	BEND_SELECT_EDGE,
	BEND_BENDIT,
	BEND_STATE_COUNT
};

const char *g_pBendStateMsg[] =
{
  "Use TAB to cycle through available bend axis. Press ENTER when the desired one is highlighted.",
  "Use TAB to cycle through available rotation axis. This will LOCK around that point. You may also use Shift + Middle Click to select an arbitrary point. Press ENTER when the desired one is highlighted",
  "Use TAB to choose which side to bend. Press ENTER when the desired one is highlighted.",
  "Use the MOUSE to bend the patch. It uses the same ui rules as Free Rotation. Press ENTER to accept the bend, press ESC to abandon it and exit Bend mode",
  ""
};

// INSERT states
enum
{
	INSERT_SELECT_EDGE = 0,
	INSERT_STATE_COUNT
};

const char* g_pInsertStateMsg[] =
{
	"Use TAB to cycle through available rows/columns for insertion/deletion. Press INS to insert at the highlight, DEL to remove the pair"
};


float *g_InversePoints[1024];

const float fFullBright = 1.0;
const float fLowerLimit = .50;
const float fDec = .05;

vec_t __VectorNormalize (vec3_t in, vec3_c &out)
{
	vec_t	length, ilength;

	length = sqrt (in[0]*in[0] + in[1]*in[1] + in[2]*in[2]);
	if (length == 0)
	{
		out.clear();
		return 0;
	}

	ilength = 1.0/length;
	out[0] = in[0]*ilength;
	out[1] = in[1]*ilength;
	out[2] = in[2]*ilength;

	return length;
}



/*
==================
Patch_MemorySize
==================
*/
int Patch_MemorySize(patchMesh_c *p)
{
	return _msize(p);
}



/*
===============
InterpolateInteriorPoints
===============
*/
void patchMesh_c::interpolateInteriorPoints() 
{
	int		i, j, k;
	int		next, prev;

	for ( i = 0 ; i < this->width ; i += 2 ) 
	{
		next = ( i == this->width - 1 ) ? 1 : ( i + 1 ) % this->width;
		prev = ( i == 0 ) ? this->width - 2 : i - 1;
#if 0
		if ( i == 0 ) 
		{
			next = ( i + 1 ) % this->width;
			prev = this->width - 2;		      // joined wrap case
		} 
		else if ( i == this->width - 1 ) 
		{
			next = 1;
			prev = i - 1;
		} 
		else 
		{
			next = ( i + 1 ) % this->width;
			prev = i - 1;
		}
#endif
		for ( j = 0 ; j < this->height ; j++ ) 
		{
			for ( k = 0 ; k < 3 ; k++ ) 
			{
				this->ctrl[i][j].xyz[k] = ( this->ctrl[next][j].xyz[k] + this->ctrl[prev][j].xyz[k] ) * 0.5;
			}
		}
	}
}

/*
=================
MakeMeshNormals

=================
*/
int	neighbors[8][2] = {
	{0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}
};

void patchMesh_c::meshNormals() 
{
	int		i, j, k, dist;
	vec3_c	normal;
	vec3_c	sum;
	int		count;
	vec3_c	base;
	vec3_c	delta;
	int		x, y;
	drawVert_t	*dv;
	vec3_c		around[8], temp;
	bool	good[8];
	bool	wrapWidth, wrapHeight;
	float		len;

	wrapWidth = false;
	for ( i = 0 ; i < this->height ; i++ ) 
	{

		delta = this->ctrl[0][i].xyz - this->ctrl[this->width-1][i].xyz;
		len = delta.vectorLength();
		if ( len > 1.0 ) 
		{
			break;
		}
	}
	if ( i == this->height ) 
	{
		wrapWidth = true;
	}

	wrapHeight = false;
	for ( i = 0 ; i < this->width ; i++ ) 
	{
		delta = this->ctrl[i][0].xyz - this->ctrl[i][this->height-1].xyz;
		len = delta.vectorLength();
		if ( len > 1.0 ) 
		{
			break;
		}
	}
	if ( i == this->width) 
	{
		wrapHeight = true;
	}


	for ( i = 0 ; i < this->width ; i++ ) 
	{
		for ( j = 0 ; j < this->height ; j++ ) 
		{
			count = 0;
			//--dv = reinterpret_cast<drawVert_t*>(this.ctrl[j*this.width+i]);
			dv = &this->ctrl[i][j];
			base = dv->xyz;
			for ( k = 0 ; k < 8 ; k++ ) 
			{
				around[k].clear();
				good[k] = false;

				for ( dist = 1 ; dist <= 3 ; dist++ ) 
				{
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) 
					{
						if ( x < 0 ) 
						{
							x = this->width - 1 + x;
						} 
						else if ( x >= this->width ) 
						{
							x = 1 + x - this->width;
						}
					}
					if ( wrapHeight ) 
					{
						if ( y < 0 ) 
						{
							y = this->height - 1 + y;
						} 
						else if ( y >= this->height ) 
						{
							y = 1 + y - this->height;
						}
					}

					if ( x < 0 || x >= this->width || y < 0 || y >= this->height ) 
					{
						break;					// edge of patch
					}
					//--VectorSubtract( this.ctrl[y*this.width+x]->xyz, base, temp );
					temp = this->ctrl[x][y].xyz - base;
					if ( __VectorNormalize( temp, temp ) == 0 ) 
					{
						continue;				// degenerate edge, get more dist
					} 
					else                 
					{
						good[k] = true;
						around[k] = temp;
						break;					// good edge
					}
				}
			}

			sum.clear();
			for ( k = 0 ; k < 8 ; k++ ) 
			{
				if ( !good[k] || !good[(k+1)&7] ) 
				{
					continue;	// didn't get two points
				}
				normal.crossProduct( around[(k+1)&7], around[k] );
				if ( __VectorNormalize( normal, normal ) == 0 ) 
				{
					continue;
				}
				normal += sum;
				count++;
			}
			if ( count == 0 ) 
			{
				//printf("bad normal\n");
				count = 1;
				//continue;
			}
			__VectorNormalize( sum, dv->normal );
		}
	}
}




/*
==================
Patch_CalcBounds
==================
*/
void patchMesh_c::calcPatchBounds(vec3_c& vMin, vec3_c& vMax)
{
	vMin[0] = vMin[1] = vMin[2] = 99999;
	vMax[0] = vMax[1] = vMax[2] = -99999;

	this->bDirty = true;
	for (int w = 0; w < this->width; w++)
	{
		for (int h = 0; h < this->height; h++)
		{
			for (int j = 0; j < 3; j++)
			{
				float f = this->ctrl[w][h].xyz[j];
				if (f < vMin[j])
					vMin[j] = f;
				if (f > vMax[j])
					vMax[j] = f;
			}
		}
	}
}

/*
==================
Brush_RebuildBrush
==================
*/
void Brush_RebuildBrush(brush_s *b, vec3_t vMins, vec3_t vMaxs)
{
	//
	// Total hack job 
	// Rebuilds a brush
	int		i, j;
	face_s	*f, *next;
	vec3_t	pts[4][2];
	texdef_t	texdef;
	// free faces

	for (j = 0; j < 3; j++)
	{
		if ((int)vMins[j] == (int)vMaxs[j])
		{
			vMins[j] -= 4;
			vMaxs[j] += 4;
		}
	}


	for (f=b->brush_faces ; f ; f=next)
	{
		next = f->next;
		if (f)
			texdef = f->texdef;
		Face_Free( f );
	}

	b->brush_faces = NULL;

	// left the last face so we can use its texdef

	for (i=0 ; i<3 ; i++)
		if (vMaxs[i] < vMins[i])
			Error ("Brush_RebuildBrush: backwards");

	pts[0][0][0] = vMins[0];
	pts[0][0][1] = vMins[1];

	pts[1][0][0] = vMins[0];
	pts[1][0][1] = vMaxs[1];

	pts[2][0][0] = vMaxs[0];
	pts[2][0][1] = vMaxs[1];

	pts[3][0][0] = vMaxs[0];
	pts[3][0][1] = vMins[1];

	for (i=0 ; i<4 ; i++)
	{
		pts[i][0][2] = vMins[2];
		pts[i][1][0] = pts[i][0][0];
		pts[i][1][1] = pts[i][0][1];
		pts[i][1][2] = vMaxs[2];
	}

	for (i=0 ; i<4 ; i++)
	{
		f = Face_Alloc();
		f->texdef = texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		if ( b->patchBrush )
		{
			f->texdef.flags |= SURF_PATCH; 
		}
		f->next = b->brush_faces;
		b->brush_faces = f;
		j = (i+1)%4;

		f->planepts[0] = pts[j][1];
		f->planepts[1] = pts[i][1];
		f->planepts[2] = pts[i][0];
	}

	f = Face_Alloc();
	f->texdef = texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	if ( b->patchBrush )
	{
		f->texdef.flags |= SURF_PATCH; 
	}
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0] = pts[0][1];
	f->planepts[1] = pts[1][1];
	f->planepts[2] = pts[2][1];

	f = Face_Alloc();
	f->texdef = texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	if ( b->patchBrush )
	{
		f->texdef.flags |= SURF_PATCH; 
	}
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0] = pts[2][0];
	f->planepts[1] = pts[1][0];
	f->planepts[2] = pts[0][0];

	Brush_Build(b);
}

void patchMesh_c::rebuildPatch()
{
	vec3_c vMin, vMax;
	this->calcPatchBounds(vMin, vMax);
	Brush_RebuildBrush(this->pSymbiot, vMin, vMax);
	this->bDirty = true;
}

/*
==================
AddBrushForPatch
==================
 adds a patch brush and ties it to this patch id
*/
brush_s* AddBrushForPatch(patchMesh_c *pm, bool bLinkToWorld )
{
  // find the farthest points in x,y,z
  vec3_c vMin, vMax;
  pm->calcPatchBounds(vMin, vMax);

  for (int j = 0; j < 3; j++)
  {
    if (vMin[j] == vMax[j])
    {
      vMin[j] -= 4;
      vMax[j] += 4;
    }
  }

  brush_s *b = Brush_Create(vMin, vMax, &g_qeglobals.d_texturewin.texdef);
	face_s		*f;
	for (f=b->brush_faces ; f ; f=f->next) 
  {
		f->texdef.flags |= SURF_PATCH; 
	}

  // FIXME: this entire type of linkage needs to be fixed
  b->pPatch = pm;
  pm->pSymbiot = b;
  pm->bSelected = false;
  pm->bOverlay = false;
  pm->bDirty = true;
  pm->nListID = -1;

  if (bLinkToWorld)
  {
    Brush_AddToList (b, &active_brushes);
	  Entity_LinkBrush (world_entity, b);
    Brush_Build(b);
  }

  return b;
}

// very approximate widths and heights

/*
==================
Patch_Width
==================
*/
float patchMesh_c::calcPatchWidth()
{
	float f = 0;
	for (int i = 0; i < this->width-1; i++)
	{
		vec3_c vTemp;
		vTemp = this->ctrl[i][0].xyz - this->ctrl[i+1][0].xyz;
		f += vTemp.vectorLength();
	}
	return f;
}

float patchMesh_c::calcPatchWidthDistanceTo(int j)
{
	float f = 0;
	for (int i = 0; i < j; i++)
	{
		vec3_c vTemp;
		vTemp = this->ctrl[i][0].xyz - this->ctrl[i+1][0].xyz;
		f += vTemp.vectorLength();
	}
	return f;
}



/*
==================
Patch_Height
==================
*/
float patchMesh_c::calcPatchHeight()
{
	float f = 0;
	for (int i = 0; i < this->height-1; i++)
	{
		vec3_c vTemp;
		vTemp = this->ctrl[0][i].xyz - this->ctrl[0][i+1].xyz;
		f += vTemp.vectorLength();
	}
	return f;
}

float patchMesh_c::calcPatchHeightDistanceTo(int j) 
{
	float f = 0;
	for (int i = 0; i < j; i++)
	{
		vec3_c vTemp;
		vTemp = this->ctrl[0][i].xyz - this->ctrl[0][i+1].xyz;
		f += vTemp.vectorLength();
	}
	return f;
}



/*
==================
Patch_Naturalize
==================
texture = TotalTexture * LengthToThisControlPoint / TotalControlPointLength

dist( this control point to first control point ) / dist ( last control pt to first)
*/
void patchMesh_c::naturalizePatch()
{
	int nWidth = this->d_texture->getImageWidth() * 0.5;
	int nHeight = this->d_texture->getImageHeight() * 0.5 ;
	float fPWidth = this->calcPatchWidth();
	float fPHeight = this->calcPatchHeight();
	float xAccum = 0;
	for ( int i = 0 ; i < this->width ; i++ ) 
	{
		float yAccum = 0;
		for ( int j = 0 ; j < this->height ; j++ ) 
		{
			this->ctrl[i][j].st[0] = (fPWidth / nWidth) * xAccum / fPWidth;
			this->ctrl[i][j].st[1] = (fPHeight / nHeight) * yAccum / fPHeight;
			yAccum = this->calcPatchHeightDistanceTo(j+1);
			//this->ctrl[i][j][3] = (fPWidth / nWidth) * (float)i / (this->width - 1);
			//this->ctrl[i][j][4] = (fPHeight/ nHeight) * (float)j / (this->height - 1);
		}
		xAccum = this->calcPatchWidthDistanceTo(i+1);
	}
	this->bDirty = true;
}

int Index3By[][2] =
{
  {0,0},
  {1,0},
  {2,0},
  {2,1},
  {2,2},
  {1,2},
  {0,2},
  {0,1},
  {0,0},
  {0,0},
  {0,0},
  {0,0},
  {0,0},
  {0,0},
  {0,0}
};

int Index5By[][2] =
{
  {0,0},
  {1,0},
  {2,0},
  {3,0},
  {4,0},
  {4,1},
  {4,2},
  {4,3},
  {4,4},
  {3,4},
  {2,4},
  {1,4},
  {0,4},
  {0,3},
  {0,2},
  {0,1}
};



int Interior3By[][2] =
{
  {1,1}
};

int Interior5By[][2] =
{
  {1,1},
  {2,1},
  {3,1},
  {1,2},
  {2,2},
  {3,2},
  {1,3},
  {2,3},
  {3,3}
};

int Interior3ByCount = sizeof(Interior3By) / sizeof(int[2]);
int Interior5ByCount = sizeof(Interior5By) / sizeof(int[2]);

face_s* Patch_GetAxisFace(patchMesh_c *p)
{
	face_s *f = NULL;
	vec3_c vTemp;
	brush_s *b = p->pSymbiot;

	for (f = b->brush_faces ; f ; f = f->next) 
	{
		vTemp = f->face_winding->points[1].getXYZ() - f->face_winding->points[0].getXYZ();
		int nScore = 0;

		// default edge faces on caps are 8 high so
		// as soon as we hit one that is bigger it should be on the right axis
		for (int j = 0; j < 3; j++)
		{
			if (vTemp[j] > 8)
				nScore++;
		}

		if (nScore > 0)
			break;
	}

	if (f == NULL)
		f = b->brush_faces;
	return f;
}

int g_nFaceCycle = 0;

face_s* nextFace(patchMesh_c *p)
{
  brush_s *b = p->pSymbiot;
  face_s *f = NULL;
  int n = 0;
	for (f = b->brush_faces ; f && n <= g_nFaceCycle; f = f->next) 
  {
    n++;
  }
  g_nFaceCycle++;
  if (g_nFaceCycle > 5)
  {
    g_nFaceCycle =0;
    f = b->brush_faces;
  }

  return f;
}


///extern void EmitTextureCoordinates ( texturedVertex_c & xyzst, mtrAPI_i *q, face_s *f);

void Patch_CapTexture(patchMesh_c *p, bool bFaceCycle = false)
{
	p->meshNormals();
  face_s *f = (bFaceCycle) ? nextFace(p) : Patch_GetAxisFace(p);
  vec3_c vSave = f->plane.normal;
  float fRotate = f->texdef.rotate;
  f->texdef.rotate = 0;
  float fScale[2];
  fScale[0] = f->texdef.scale[0];
  fScale[1] = f->texdef.scale[1];
  f->texdef.scale[0] = 0.5;
  f->texdef.scale[1] = 0.5;
  float fShift[2];
  fShift[0] = f->texdef.shift[0];
  fShift[1] = f->texdef.shift[1];
  f->texdef.shift[0] = 0;
  f->texdef.shift[1] = 0;
  
  for (int i = 0; i < p->width; i++)
  {
    for (int j = 0; j < p->height; j++)
    {
      if (!bFaceCycle)
      {
        f->plane.normal = p->ctrl[i][j].normal;
      }
		  EmitTextureCoordinates( p->ctrl[i][j].asWindingPoint(), f->d_texture, f);
    }
  }
  f->plane.normal = vSave;
  f->texdef.rotate = fRotate;
  f->texdef.scale[0] = fScale[0];
  f->texdef.scale[1] = fScale[1];
  f->texdef.shift[0] = fShift[0];
  f->texdef.shift[1] = fShift[1];
  p->bDirty = true;
}

void patchMesh_c::fillPatch(vec3_t v)
{
	for (int i = 0; i < this->width; i++)
	{
		for (int j = 0; j < this->height; j++)
		{
			this->ctrl[i][j].xyz = v;
		}
	}
}

brush_s* Cap(patchMesh_c *pParent, bool bByColumn, bool bFirst)
{
  brush_s *b;
  patchMesh_c *p;
  vec3_t vMin, vMax;
  int i, j;

  bool bSmall = true;
  // make a generic patch
  if (pParent->width <= 9)
  {
    b = Patch_GenericMesh(3, 3, 2, false);
  }
  else
  {
    b = Patch_GenericMesh(5, 5, 2, false);
    bSmall = false;
  }

  if (!b)
  {
    Sys_Printf("Unable to cap. You may need to ungroup the patch.\n");
    return NULL;
  }

  p = b->pPatch;
  p->type |= PATCH_CAP;

  vMin[0] = vMin[1] = vMin[2] = 9999;
  vMax[0] = vMax[1] = vMax[2] = -9999;

  // we seam the column edge, FIXME: this might need to be able to seem either edge
  // 
  int nSize = (bByColumn) ? pParent->width : pParent->height;
  int nIndex = (bFirst) ? 0 : (bByColumn) ? pParent->height-1 : pParent->width-1;

  p->fillPatch(pParent->ctrl[0][nIndex].xyz);

  for (i = 0; i < nSize; i++)
  {
    if (bByColumn)
    {
      if (bSmall)
      {
        p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz = pParent->ctrl[i][nIndex].xyz;
      }
      else
      {
        p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz = pParent->ctrl[i][nIndex].xyz;
      }
    }
    else
    {
      if (bSmall)
      {
        p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz = pParent->ctrl[nIndex][i].xyz;
      }
      else
      {
        p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz = pParent->ctrl[nIndex][i].xyz;
      }
    }
  
    for (j = 0; j < 3; j++)
    {
      float f = (bSmall) ? p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz[j] : p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz[j];
      if (f < vMin[j])
        vMin[j] = f;
      if (f > vMax[j])
        vMax[j] = f;
    }
  }

  vec3_t vTemp;
  for (j = 0; j < 3; j++)
  {
    vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
  }

  int nCount = (bSmall) ? Interior3ByCount : Interior5ByCount;
  for (j = 0; j < nCount; j++)
  {
    if (bSmall)
    {
      p->ctrl[Interior3By[j][0]][Interior3By[j][1]].xyz = vTemp;
    }
    else
    {
      p->ctrl[Interior5By[j][0]][Interior5By[j][1]].xyz = vTemp;
    }
  }

  if (bFirst)
  {
    drawVert_t vertTemp;
    for (i = 0; i < p->width; i++)
    {
      for (j = 0; j < p->height / 2; j++)
      {
        memcpy(&vertTemp, &p->ctrl[i][p->height - 1- j], sizeof (drawVert_t));
        memcpy(&p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof(drawVert_t));
        memcpy(&p->ctrl[i][j], &vertTemp, sizeof(drawVert_t));
      }
    }
  }

  p->rebuildPatch();
  Patch_CapTexture(p);
  return p->pSymbiot;
}

brush_s* CapSpecial(patchMesh_c *pParent, int nType, bool bFirst)
{
  brush_s *b;
  patchMesh_c *p;
  vec3_c vMin, vMax, vTemp;
  int i, j;

  if (nType == CCapDialog::IENDCAP)
    b = Patch_GenericMesh(5, 3, 2, false);
  else
    b = Patch_GenericMesh(3, 3, 2, false);

  if (!b)
  {
    Sys_Printf("Unable to cap. Make sure you ungroup before re-capping.");
    return NULL;
  }

  p = b->pPatch;
  p->type |= PATCH_CAP;

  vMin[0] = vMin[1] = vMin[2] = 9999;
  vMax[0] = vMax[1] = vMax[2] = -9999;

  int nSize = pParent->width;
  int nIndex = (bFirst) ? 0 : pParent->height-1;

  // parent bounds are used for some things
  pParent->calcPatchBounds(vMin, vMax);

  for (j = 0; j < 3; j++)
  {
    vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
  }

  if (nType == CCapDialog::IBEVEL)
  {
    p->ctrl[0][0].xyz = pParent->ctrl[0][nIndex].xyz;
    p->ctrl[0][2].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[0][1].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[2][2].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[1][0].xyz = pParent->ctrl[1][nIndex].xyz;
   p->ctrl[1][1].xyz = pParent->ctrl[1][nIndex].xyz;
   p->ctrl[1][2].xyz = pParent->ctrl[1][nIndex].xyz;
   p->ctrl[2][0].xyz = pParent->ctrl[1][nIndex].xyz;
   p->ctrl[2][1].xyz = pParent->ctrl[1][nIndex].xyz;
  }
  else if (nType == CCapDialog::BEVEL)
  {
    vec3_c p1, p2, p3, p4, temp, dir;

    p3 = pParent->ctrl[0][nIndex].xyz;
    p1 = pParent->ctrl[1][nIndex].xyz;
    p2 = pParent->ctrl[2][nIndex].xyz;

    dir = p3 - p2;
    dir.normalize();
    temp = p1 - p2;
    vec_t dist = temp.dotProduct(dir);

	temp = dir * dist;

	temp += p2;

	temp -= p1;
	temp *= 2.0;
	p4 = p1 + temp;

    p->ctrl[0][0].xyz = p4;
    p->ctrl[1][0].xyz = p4;
    p->ctrl[0][1].xyz = p4;
    p->ctrl[1][1].xyz = p4;
    p->ctrl[0][2].xyz = p4;
    p->ctrl[1][2].xyz = p4;
    p->ctrl[2][0].xyz = p3;
    p->ctrl[2][1].xyz = p1;
    p->ctrl[2][2].xyz = p2;

  }
  else if (nType == CCapDialog::ENDCAP)
  {
    vTemp = pParent->ctrl[4][nIndex].xyz + pParent->ctrl[0][nIndex].xyz;
    vTemp *= 0.5;
    p->ctrl[0][0].xyz =pParent->ctrl[0][nIndex].xyz;
    p->ctrl[1][0].xyz = vTemp;
    p->ctrl[2][0].xyz = pParent->ctrl[4][nIndex].xyz;

    p->ctrl[0][2].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[1][2].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[2][2].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[1][1].xyz = pParent->ctrl[2][nIndex].xyz;
    
    p->ctrl[0][1].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[2][1].xyz = pParent->ctrl[3][nIndex].xyz;
  }
  else
  {
    p->ctrl[0][0].xyz = pParent->ctrl[0][nIndex].xyz;
    p->ctrl[1][0].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[2][0].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[3][0].xyz = pParent->ctrl[3][nIndex].xyz;
    p->ctrl[4][0].xyz = pParent->ctrl[4][nIndex].xyz;
    
    p->ctrl[0][1].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[1][1].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[2][1].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[3][1].xyz = pParent->ctrl[3][nIndex].xyz;
    p->ctrl[4][1].xyz = pParent->ctrl[3][nIndex].xyz;

    p->ctrl[0][2].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[1][2].xyz = pParent->ctrl[1][nIndex].xyz;
    p->ctrl[2][2].xyz = pParent->ctrl[2][nIndex].xyz;
    p->ctrl[3][2].xyz = pParent->ctrl[3][nIndex].xyz;
    p->ctrl[4][2].xyz = pParent->ctrl[3][nIndex].xyz;
  }


  bool bEndCap = (nType == CCapDialog::ENDCAP || nType == CCapDialog::IENDCAP);
  if ((!bFirst && !bEndCap) || (bFirst && bEndCap))
  {
    drawVert_t vertTemp;
    for (i = 0; i < p->width; i++)
    {
      for (j = 0; j < p->height / 2; j++)
      {
        memcpy(&vertTemp, &p->ctrl[i][p->height - 1- j], sizeof (drawVert_t));
        memcpy(&p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof(drawVert_t));
        memcpy(&p->ctrl[i][j], &vertTemp, sizeof(drawVert_t));
      }
    }
  }

  p->rebuildPatch();
  Patch_CapTexture(p);
  return p->pSymbiot;
}


void Patch_CapCurrent(bool bInvertedBevel, bool bInvertedEndcap)
{
  patchMesh_c *pParent = NULL;
  brush_s *b[4];
  brush_s *pCap = NULL;
  b[0] = b[1] = b[2] = b[3] = NULL;
  int nIndex = 0;

	if (!QE_SingleBrush())
  {
    Sys_Printf("Cannot cap multiple selection. Please select a single patch.\n");
		return;
  }


	for (brush_s *pb = selected_brushes.next ; pb != NULL && pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      pParent = pb->pPatch;
      // decide which if any ends we are going to cap
      // if any of these compares hit, it is a closed patch and as such
      // the generic capping will work.. if we do not find a closed edge 
      // then we need to ask which kind of cap to add
      if (pParent->ctrl[0][0].xyz.vectorCompare(pParent->ctrl[pParent->width-1][0].xyz))
      {
        pCap = Cap(pParent, true, false);
        if (pCap != NULL)
        {
          b[nIndex++] = pCap;
        }
      }
      if (pParent->ctrl[0][pParent->height-1].xyz.vectorCompare(pParent->ctrl[pParent->width-1][pParent->height-1].xyz))
      {
        pCap = Cap(pParent, true, true);
        if (pCap != NULL)
        {
          b[nIndex++] = pCap;
        }
      }
      if (pParent->ctrl[0][0].xyz.vectorCompare(pParent->ctrl[0][pParent->height-1].xyz))
      {
        pCap = Cap(pParent, false, false);
        if (pCap != NULL)
        {
          b[nIndex++] = pCap;
        }
      }
      if (pParent->ctrl[pParent->width-1][0].xyz.vectorCompare(pParent->ctrl[pParent->width-1][pParent->height-1].xyz))
      {
        pCap = Cap(pParent, false, true);
        if (pCap != NULL)
        {
          b[nIndex++] = pCap;
        }
      }
    }
  }

  if (pParent)
  {
    // if we did not cap anything with the above tests
    if (nIndex == 0)
    {
      CCapDialog dlg;
      if (dlg.DoModal() == IDOK)
      {
        b[nIndex++] = CapSpecial(pParent, dlg.getCapType(), false);
        b[nIndex++] = CapSpecial(pParent, dlg.getCapType(), true);
      }
    }

    if (nIndex > 0)
    {
      while (nIndex > 0)
      {
        nIndex--;
        if (b[nIndex])
        {
          Select_Brush(b[nIndex]);
        }
      }
      eclass_s *pecNew = Eclass_ForName("func_group", false);
      if (pecNew)
      {
        entity_s *e = Entity_Create(pecNew);
        SetKeyValue(e, "type", "patchCapped");
      }
    }
  }
}


//FIXME: Table drive all this crap
//
void GenerateEndCaps(brush_s *brushParent, bool bBevel, bool bEndcap, bool bInverted)
{
  brush_s *b, *b2;
  patchMesh_c *p, *p2, *pParent;
  vec3_c vTemp, vMin, vMax;
  int i, j;

  pParent = brushParent->pPatch;

  pParent->calcPatchBounds(vMin, vMax);
  // basically generate two endcaps, place them, and link the three brushes with a func_group

  if (pParent->width > 9)
    b = Patch_GenericMesh(5, 3, 2, false);
  else
    b = Patch_GenericMesh(3, 3, 2, false);
  p = b->pPatch;

  vMin[0] = vMin[1] = vMin[2] = 9999;
  vMax[0] = vMax[1] = vMax[2] = -9999;

  for (i = 0; i < pParent->width; i++)
  {
    p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz = pParent->ctrl[i][0].xyz;
    for (j = 0; j < 3; j++)
    {
      if (pParent->ctrl[i][0].xyz[j] < vMin[j])
        vMin[j] = pParent->ctrl[i][0].xyz[j];
      if (pParent->ctrl[i][0].xyz[j] > vMax[j])
        vMax[j] = pParent->ctrl[i][0].xyz[j];
    }
  }

  for (j = 0; j < 3; j++)
  {
    vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
  }

  for (i = 0; i < Interior3ByCount; i++)
  {
    p->ctrl[Interior3By[i][0]][Interior3By[i][1]].xyz = vTemp;
  }

  p->calcPatchBounds(vMin, vMax);
  Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
  Select_Brush(p->pSymbiot);
  return;

  bool bCreated = false;

  if (bInverted)
  {
    if (bBevel)
    {
      b = Patch_GenericMesh(3, 3, 2, false);
      p = b->pPatch;
      p->ctrl[1][2].xyz = p->ctrl[2][2].xyz;
      p->ctrl[2][1].xyz = p->ctrl[2][2].xyz;
      p->ctrl[0][1].xyz = p->ctrl[2][2].xyz;
      p->ctrl[1][0].xyz = p->ctrl[2][2].xyz;
      p->ctrl[1][1].xyz = p->ctrl[2][2].xyz;
      p->ctrl[0][0].xyz = p->ctrl[2][0].xyz;

      b2 = Patch_GenericMesh(3, 3, 2, false);
      p2 = b2->pPatch;
      p2->ctrl[1][2].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[2][1].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[0][1].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[1][0].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[1][1].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[0][0].xyz = p2->ctrl[2][0].xyz;

      bCreated = true;

    }
    else if (bEndcap)
    {
      b = Patch_GenericMesh(5, 5, 2, false);
      p = b->pPatch;
      p->ctrl[4][3].xyz = p->ctrl[4][4].xyz;
      p->ctrl[1][4].xyz = p->ctrl[0][4].xyz;
      p->ctrl[2][4].xyz = p->ctrl[0][4].xyz;
      p->ctrl[3][4].xyz = p->ctrl[0][4].xyz;

      p->ctrl[4][1].xyz = p->ctrl[4][0].xyz;
      p->ctrl[1][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[2][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[3][0].xyz = p->ctrl[0][0].xyz;

      for (i = 1; i < 4; i++)
      {
        for (j = 0; j < 4; j++)
        {
          p->ctrl[j][i].xyz = p->ctrl[4][i].xyz;
        }
      }


      b2 = Patch_GenericMesh(5, 5, 2, false);
      p2 = b2->pPatch;
      p2->ctrl[4][3].xyz = p2->ctrl[4][4].xyz;
      p2->ctrl[1][4].xyz = p2->ctrl[0][4].xyz;
      p2->ctrl[2][4].xyz = p2->ctrl[0][4].xyz;
      p2->ctrl[3][4].xyz = p2->ctrl[0][4].xyz;

      p2->ctrl[4][1].xyz = p2->ctrl[4][0].xyz;
      p2->ctrl[1][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[2][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[3][0].xyz = p2->ctrl[0][0].xyz;

      for (i = 1; i < 4; i++)
      {
        for (j = 0; j < 4; j++)
        {
          p2->ctrl[j][i].xyz = p2->ctrl[4][i].xyz;
        }
      }


      bCreated = true;
    }
  }
  else
  {
    if (bBevel)
    {
      b = Patch_GenericMesh(3, 3, 2, false);
      p = b->pPatch;
      p->ctrl[2][1].xyz = p->ctrl[2][0].xyz;
      p->ctrl[1][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[2][0].xyz = p->ctrl[0][0].xyz;

      b2 = Patch_GenericMesh(3, 3, 2, false);
      p2 = b2->pPatch;
      p2->ctrl[2][1].xyz = p2->ctrl[2][0].xyz;
      p2->ctrl[1][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[2][0].xyz = p2->ctrl[0][0].xyz;
      bCreated = true;
    }
    else if (bEndcap)
    {
      b = Patch_GenericMesh(5, 5, 2, false);
      p = b->pPatch;
      p->ctrl[1][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[2][0].xyz = p->ctrl[0][0].xyz;;
      p->ctrl[3][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[4][1].xyz = p->ctrl[4][0].xyz;
      p->ctrl[4][0].xyz = p->ctrl[0][0].xyz;

      p->ctrl[1][4].xyz = p->ctrl[0][4].xyz;
      p->ctrl[2][4].xyz = p->ctrl[0][4].xyz;
      p->ctrl[3][4].xyz = p->ctrl[0][4].xyz;
      p->ctrl[4][3].xyz = p->ctrl[4][4].xyz;
      p->ctrl[4][4].xyz = p->ctrl[0][4].xyz;

      b2 = Patch_GenericMesh(5, 5, 2, false);
      p2 = b2->pPatch;
      p2->ctrl[1][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[2][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[3][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[4][1].xyz = p2->ctrl[4][0].xyz;
      p2->ctrl[4][0].xyz = p2->ctrl[0][0].xyz;

      p2->ctrl[1][4].xyz = p2->ctrl[0][4].xyz;
      p2->ctrl[2][4].xyz = p2->ctrl[0][4].xyz;
      p2->ctrl[3][4].xyz = p2->ctrl[0][4].xyz;
      p2->ctrl[4][3].xyz = p2->ctrl[4][4].xyz;
      p2->ctrl[4][4].xyz = p2->ctrl[0][4].xyz;
      bCreated = true;
    }
    else
    {
      b = Patch_GenericMesh(3, 3, 2, false);
      p = b->pPatch;
      
      vTemp = p->ctrl[0][1].xyz;
      p->ctrl[0][1].xyz = p->ctrl[0][2].xyz;
      p->ctrl[0][2].xyz = p->ctrl[1][2].xyz;
      p->ctrl[1][2].xyz = p->ctrl[2][2].xyz;
      p->ctrl[2][2].xyz = p->ctrl[2][1].xyz;
      p->ctrl[2][1].xyz = p->ctrl[2][0].xyz;
      p->ctrl[2][0].xyz = p->ctrl[1][0].xyz;
      p->ctrl[1][0].xyz = p->ctrl[0][0].xyz;
      p->ctrl[0][0].xyz = vTemp;

      b2 = Patch_GenericMesh(3, 3, 2, false);
      p2 = b2->pPatch;
      vTemp = p2->ctrl[0][1].xyz;
      p2->ctrl[0][1].xyz = p2->ctrl[0][2].xyz;
      p2->ctrl[0][2].xyz = p2->ctrl[1][2].xyz;
      p2->ctrl[1][2].xyz = p2->ctrl[2][2].xyz;
      p2->ctrl[2][2].xyz = p2->ctrl[2][1].xyz;
      p2->ctrl[2][1].xyz = p2->ctrl[2][0].xyz;
      p2->ctrl[2][0].xyz = p2->ctrl[1][0].xyz;
      p2->ctrl[1][0].xyz = p2->ctrl[0][0].xyz;
      p2->ctrl[0][0].xyz = vTemp;
      bCreated = true;
    }
  }

  if (bCreated)
  {
    drawVert_t vertTemp;
    for (i = 0; i < p->width; i++)
    {
      for (j = 0; j < p->height; j++)
      {
        p->ctrl[i][j].xyz[2] = vMin[2];
        p2->ctrl[i][j].xyz[2] = vMax[2];
      }

      for (j = 0; j < p->height / 2; j++)
      {
        memcpy(&vertTemp, &p->ctrl[i][p->height - 1- j], sizeof (drawVert_t));
        memcpy(&p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof(drawVert_t));
        memcpy(&p->ctrl[i][j], &vertTemp, sizeof(drawVert_t));
      }

    }
    //Select_Delete();

    p->calcPatchBounds(vMin, vMax);
    Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
    p2->calcPatchBounds(vMin, vMax);
    Brush_RebuildBrush(p2->pSymbiot, vMin, vMax);
    Select_Brush(p->pSymbiot);
    Select_Brush(p2->pSymbiot);
  }
  else
  {
    Select_Delete();
  }
  //Select_Brush(brushParent);

}


/*
===============
BrushToPatchMesh
===============
*/
void Patch_BrushToMesh(bool bCone, bool bBevel, bool bEndcap, bool bSquare, int nHeight)
{
	brush_s		*b;
	patchMesh_c	*p;
	int			i,j;

	if (!QE_SingleBrush())
		return;

	b = selected_brushes.next;

	p = MakeNewPatch();

	p->d_texture = b->brush_faces->d_texture;

  p->height = nHeight;

  p->type = PATCH_CYLINDER;
  if (bBevel & !bSquare)
  {
    p->type = PATCH_BEVEL;
    p->width = 3;
    int nStep = (b->getMaxs()[2] - b->getMins()[2]) / (p->height-1);
    int nStart = b->getMins()[2];
    for (i = 0; i < p->height; i++)
    {
	    p->ctrl[0][i].xyz[0] =  b->getMins()[0];
	    p->ctrl[0][i].xyz[1] =  b->getMins()[1];
      p->ctrl[0][i].xyz[2] = nStart;

	    p->ctrl[1][i].xyz[0] =  b->getMaxs()[0];
	    p->ctrl[1][i].xyz[1] =  b->getMins()[1];
      p->ctrl[1][i].xyz[2] = nStart;

	    p->ctrl[2][i].xyz[0] =  b->getMaxs()[0];
	    p->ctrl[2][i].xyz[1] =  b->getMaxs()[1];
      p->ctrl[2][i].xyz[2] = nStart;
      nStart += nStep;
    }
  }
  else if (bEndcap & !bSquare)
  {
    p->type = PATCH_ENDCAP;
    p->width = 5;
    int nStep = (b->getMaxs()[2] - b->getMins()[2]) / (p->height-1);
    int nStart = b->getMins()[2];
    for (i = 0; i < p->height; i++)
    {
	    p->ctrl[0][i].xyz[0] =  b->getMins()[0];
	    p->ctrl[0][i].xyz[1] =  b->getMins()[1];
      p->ctrl[0][i].xyz[2] = nStart;

	    p->ctrl[1][i].xyz[0] =  b->getMins()[0];
	    p->ctrl[1][i].xyz[1] =  b->getMaxs()[1];
      p->ctrl[1][i].xyz[2] = nStart;

	    p->ctrl[2][i].xyz[0] =  b->getMins()[0] + ((b->getMaxs()[0] - b->getMins()[0]) * 0.5);
	    p->ctrl[2][i].xyz[1] =  b->getMaxs()[1];
      p->ctrl[2][i].xyz[2] = nStart;
	   
      p->ctrl[3][i].xyz[0] =  b->getMaxs()[0];
	    p->ctrl[3][i].xyz[1] =  b->getMaxs()[1];
      p->ctrl[3][i].xyz[2] = nStart;

	    p->ctrl[4][i].xyz[0] =  b->getMaxs()[0];
	    p->ctrl[4][i].xyz[1] =  b->getMins()[1];
      p->ctrl[4][i].xyz[2] = nStart;
      nStart += nStep;
    }
  }
  else
  {
    p->width = 9;
	  p->ctrl[1][0].xyz[0] =  b->getMins()[0];
	  p->ctrl[1][0].xyz[1] =  b->getMins()[1];

	  p->ctrl[3][0].xyz[0] =  b->getMaxs()[0];
	  p->ctrl[3][0].xyz[1] =  b->getMins()[1];

	  p->ctrl[5][0].xyz[0] =  b->getMaxs()[0];
	  p->ctrl[5][0].xyz[1] =  b->getMaxs()[1];

	  p->ctrl[7][0].xyz[0] =  b->getMins()[0];
	  p->ctrl[7][0].xyz[1] =  b->getMaxs()[1];

	  for ( i = 1 ; i < p->width - 1 ; i += 2 ) 
    {

      p->ctrl[i][0].xyz[2] =  b->getMins()[2];

		  p->ctrl[i][2].xyz = p->ctrl[i][0].xyz;

		  p->ctrl[i][2].xyz[2] =  b->getMaxs()[2];

		  p->ctrl[i][1].xyz[0] = ( p->ctrl[i][0].xyz[0] + p->ctrl[i][2].xyz[0] ) * 0.5;
		  p->ctrl[i][1].xyz[1] = ( p->ctrl[i][0].xyz[1] + p->ctrl[i][2].xyz[1] ) * 0.5;
		  p->ctrl[i][1].xyz[2] = ( p->ctrl[i][0].xyz[2] + p->ctrl[i][2].xyz[2] ) * 0.5;
	  }
	  p->interpolateInteriorPoints();

    if (bSquare)
    {
      if (bBevel || bEndcap)
      {
        if (bBevel)
        {
          for (i = 0; i < p->height; i++)
          {
            p->ctrl[2][i].xyz = p->ctrl[1][i].xyz;
            p->ctrl[6][i].xyz = p->ctrl[7][i].xyz;
          }
        }
        else
        {
          for (i = 0; i < p->height; i++)
          {
            p->ctrl[4][i].xyz = p->ctrl[5][i].xyz;
            p->ctrl[2][i].xyz = p->ctrl[1][i].xyz;
            p->ctrl[6][i].xyz = p->ctrl[7][i].xyz;
            p->ctrl[7][i].xyz = p->ctrl[8][i].xyz;
          }
        }
      }
      else
      {
        for (i = 0; i < p->width-1; i ++)
        {
          for (j = 0; j < p->height; j++)
          {
            p->ctrl[i][j].xyz = p->ctrl[i+1][j].xyz;
          }
        }
        for (j = 0; j < p->height; j++)
        {
          p->ctrl[8][j].xyz = p->ctrl[0][j].xyz;
        }
      }
    }
  }


  p->naturalizePatch();

  if (bCone)
  {
    p->type = PATCH_CONE;
    float xc = (b->getMaxs()[0] + b->getMins()[0]) * 0.5; 
    float yc = (b->getMaxs()[1] + b->getMins()[1]) * 0.5; 

    for ( i = 0 ; i < p->width ; i ++)
    {
      p->ctrl[i][2].xyz[0] = xc;
      p->ctrl[i][2].xyz[1] = yc;
    }
  }
/*
  if (bEndcap || bBevel)
  {
    if (bInverted)
    {
      for ( i = 0 ; i < p->height ; i ++)
      {
        if (bBevel)
        {
          VectorCopy(p->ctrl[7][i], p->ctrl[0][i]);
          VectorCopy(p->ctrl[7][i], p->ctrl[8][i]);
          VectorCopy(p->ctrl[3][i], p->ctrl[2][i]);
          VectorCopy(p->ctrl[5][i], p->ctrl[1][i]);
          VectorCopy(p->ctrl[5][i], p->ctrl[4][i]);
          VectorCopy(p->ctrl[5][i], p->ctrl[6][i]);
        }
        else
        {
          VectorCopy(p->ctrl[4][i], p->ctrl[8][i]);
          VectorCopy(p->ctrl[1][i], p->ctrl[0][i]);
          VectorCopy(p->ctrl[1][i], p->ctrl[10][i]);
          VectorCopy(p->ctrl[3][i], p->ctrl[2][i]);
          VectorCopy(p->ctrl[5][i], p->ctrl[4][i]);
          VectorCopy(p->ctrl[7][i], p->ctrl[6][i]);
          VectorCopy(p->ctrl[5][i], p->ctrl[7][i]);
          VectorCopy(p->ctrl[3][i], p->ctrl[9][i]);
        }
      }
    }
    else
    {
      for ( i = 0 ; i < p->height ; i ++)
      {
        VectorCopy(p->ctrl[1][i], p->ctrl[2][i]);
        VectorCopy(p->ctrl[7][i], p->ctrl[6][i]);
        if (bBevel)
        {
          VectorCopy(p->ctrl[5][i], p->ctrl[4][i]);
        }
      }
    }
  }
*/
  
  b = AddBrushForPatch(p);


#if 1
    Select_Delete();
    Select_Brush(b);
#else
  if (!bCone)
  {
    Select_Delete();
    Select_Brush(b);
    GenerateEndCaps(b, bBevel, bEndcap, bInverted);
    eclass_s *pecNew = Eclass_ForName("func_group", false);
    if (pecNew)
    {
      Entity_Create(pecNew);
    }
  }
  else
  {
    Select_Delete();
    Select_Brush(b);
  }
#endif

}

/*
==================
Patch_GenericMesh
==================
*/
brush_s* Patch_GenericMesh(int nWidth, int nHeight, int nOrientation, bool bDeleteSource, bool bOverride)
{
	int i,j;

	if (nHeight < 3 || nHeight > 15 || nWidth < 3 || nWidth > 15)
	{
		Sys_Printf("Invalid patch width or height.\n");
		return NULL;
	}

	if (! bOverride && !QE_SingleBrush())
	{
		Sys_Printf("Cannot generate a patch from multiple selections.\n");
		return NULL;
	}



	patchMesh_c* p = MakeNewPatch();
	p->d_texture = QERApp_TryTextureForName(g_qeglobals.d_texturewin.texdef.name);

	p->width = nWidth;
	p->height = nHeight;
	p->type = PATCH_GENERIC;

	int nFirst = 0;
	int nSecond = 1;
	if (nOrientation == 0)
	{
		nFirst = 1;
		nSecond = 2;
	}
	else if (nOrientation == 1)
	{
		nSecond = 2;
	}

	brush_s *b = selected_brushes.next;

	int xStep = b->getMins()[nFirst];
	float xAdj = abs((b->getMaxs()[nFirst] - b->getMins()[nFirst]) / (nWidth - 1));
	float yAdj = abs((b->getMaxs()[nSecond] - b->getMins()[nSecond]) / (nHeight - 1));

	for (i = 0; i < nWidth; i++)
	{
		int yStep = b->getMins()[nSecond];
		for (j = 0; j < nHeight; j++)
		{
			p->ctrl[i][j].xyz[nFirst] = xStep;
			p->ctrl[i][j].xyz[nSecond] = yStep;
			p->ctrl[i][j].xyz[nOrientation] = 0;
			yStep += yAdj;
		}
		xStep += xAdj;
	}

	p->naturalizePatch();

	b = AddBrushForPatch(p);
	if (bDeleteSource)
	{
		Select_Delete();
		Select_Brush(b);
	}

	return b;
	//g_qeglobals.d_select_mode = sel_curvepoint;
}

/*
==================
PointInMoveList
==================
*/
int PointInMoveList(float *pf)
{
	for (int i = 0; i < g_qeglobals.d_num_move_points; i++)
	{
		if (pf == &g_qeglobals.d_move_points[i][0])
			return i;
	}
	return -1;
}

/*
==================
PointValueInMoveList
==================
*/
int PointValueInMoveList(const vec3_c &v)
{
	for (int i = 0; i < g_qeglobals.d_num_move_points; i++)
	{
		if (v.vectorCompare(g_qeglobals.d_move_points[i]))
			return i;
	}
	return -1;
}


/*
==================
RemovePointFromMoveList
==================
*/
void RemovePointFromMoveList(const vec3_c &v)
{
	int n;
	while ( (n = PointValueInMoveList(v)) >= 0)
	{
		for (int i = n; i < g_qeglobals.d_num_move_points-1; i++)
		{
			g_qeglobals.d_move_points[i] = g_qeglobals.d_move_points[i+1];
		}
		g_qeglobals.d_num_move_points--;
	}
}

/*
==================
ColumnSelected
==================
*/
bool ColumnSelected(patchMesh_c* p, int nCol)
{
	for (int i = 0; i < p->height; i++)
	{
		if (PointInMoveList(p->ctrl[nCol][i].xyz) == -1)
			return false;
	}
	return true;
}

/*
==================
AddPoint
==================
*/
static int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	int		i;
	
	for (i=0 ; i<3 ; i++)
		if (fabs(v1[i]-v2[i]) > EQUAL_EPSILON)
			return false;
			
	return true;
}
void AddPoint(patchMesh_c* p, vec3_t v, bool bWeldOrDrill = true)
{
  int nDim1 = (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
  int nDim2 = (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;
  g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = v;
  if ((g_bPatchWeld || g_bPatchDrillDown) && bWeldOrDrill)
  {
	  for ( int i = 0 ; i < p->width ; i++ ) 
    {
		  for ( int j = 0 ; j < p->height ; j++ ) 
      {
        if (g_bPatchWeld)
        {
          if (VectorCompare(v,p->ctrl[i][j].xyz)
            && PointInMoveList(p->ctrl[i][j].xyz) == -1)
          {
            g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
            continue;
          }
        }
        if (g_bPatchDrillDown && g_nPatchClickedView != W_CAMERA)
        {
          if ( (fabs(v[nDim1] - p->ctrl[i][j].xyz[nDim1]) <= EQUAL_EPSILON) 
             &&(fabs(v[nDim2] - p->ctrl[i][j].xyz[nDim2]) <= EQUAL_EPSILON)) 
          {
            if (PointInMoveList(p->ctrl[i][j].xyz) == -1)
            {
              g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
              continue;
            }
          }
#if 0
          int l = 0;
	        for ( int k = 0; k < 2; k++ )
          {
		        if (fabs(v[k] - p->ctrl[i][j].xyz[k]) > EQUAL_EPSILON)
              continue;
             l++;
          }
          if (l >= 2 && PointInMoveList(p->ctrl[i][j].xyz) == -1)
          {
            g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
            continue;
          }
#endif
        }
      }
    }
  }
#if 0
  if (g_qeglobals.d_num_move_points == 1)
  {
    // single point selected
    // FIXME: the two loops can probably be reduced to one
	  for ( int i = 0 ; i < p->width ; i++ ) 
    {
		  for ( int j = 0 ; j < p->height ; j++ ) 
      {
        int n = PointInMoveList(v);
        if (n >= 0)
        {
          if (((i & 0x01) && (j & 0x01)) == 0)
          {
            // put any sibling fixed points
            // into the inverse list
            int p1, p2, p3, p4;
            p1 = i + 2;
            p2 = i - 2;
            p3 = j + 2;
            p4 = j - 2;
            if (p1 < p->width)
            {

            }
            if (p2 >= 0)
            {
            }
            if (p3 < p->height)
            {
            }
            if (p4 >= 0)
            {
            }
          }
        }
      }
    }
  }
#endif
}

/*
==================
SelectRow
==================
*/
void SelectRow(patchMesh_c* p, int nRow, bool bMulti)
{
	if (!bMulti)
		g_qeglobals.d_num_move_points = 0;
	for (int i = 0; i < p->width; i++)
	{
		AddPoint(p, p->ctrl[i][nRow].xyz, false);
	}
	//Sys_Printf("Selected Row %d\n", nRow);
}

/*
==================
SelectColumn
==================
*/
void SelectColumn(patchMesh_c* p, int nCol, bool bMulti)
{
	if (!bMulti)
		g_qeglobals.d_num_move_points = 0;
	for (int i = 0; i < p->height; i++)
	{
		AddPoint(p, p->ctrl[nCol][i].xyz, false);
	}
	//Sys_Printf("Selected Col %d\n", nCol);
}


/*
==================
AddPatchMovePoint
==================
*/
void AddPatchMovePoint(const vec3_c &v, bool bMulti, bool bFull)
{
	if (!g_bSameView && !bMulti && !bFull)
	{
		g_bSameView = true;
		return;
	}

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c* p = pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ ) 
			{
				for ( int j = 0 ; j < p->height ; j++ ) 
				{
					if (v.vectorCompare(p->ctrl[i][j].xyz))
					{
						if (PointInMoveList(p->ctrl[i][j].xyz) == -1)
						{
							if (bFull)        // if we want the full row/col this is on
							{
								SelectColumn(p, i, bMulti);
							}
							else
							{
								if (!bMulti)
									g_qeglobals.d_num_move_points = 0;
								AddPoint(p, p->ctrl[i][j].xyz);
							//Sys_Printf("Selected col:row %d:%d\n", i, j);
							}
							//--if (!bMulti)
								return;
						}
						else
						{
							if (bFull)
							{
								if (ColumnSelected(p, i))
								{
									SelectRow(p, j, bMulti);
								}
								else
								{
									SelectColumn(p, i, bMulti);
								}
								return;
							}
							if (g_bSameView)
							{
								RemovePointFromMoveList(v);
								return;
							}
						}
					}
				}
			}
		}
	}
}

/*
==================
Patch_UpdateSelected
==================
*/
void Patch_UpdateSelected(vec3_t vMove)
{
  int i, j;
  for (i=0 ; i < g_qeglobals.d_num_move_points ; i++)
  {
	*((vec3_c*)g_qeglobals.d_move_points[i]) += vMove;
    if (g_qeglobals.d_num_move_points == 1)
    {
    }
  }

	//--patchMesh_c* p = &patchMeshes[g_nSelectedPatch];
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
	    patchMesh_c* p = pb->pPatch;


      g_qeglobals.d_numpoints = 0;
	    for (i = 0 ; i < p->width ; i++ ) 
      {
		    for ( j = 0 ; j < p->height ; j++ ) 
        {
	        g_qeglobals.d_points[g_qeglobals.d_numpoints] = p->ctrl[i][j].xyz;
          if (g_qeglobals.d_numpoints < MAX_POINTS-1)
          {
	          g_qeglobals.d_numpoints++;
          }
		    }
      }
	    
      vec3_c vMin, vMax;
      p->calcPatchBounds(vMin, vMax);
      Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
    }
  }
  //Brush_Free(p->pSymbiot);
  //Select_Brush(AddBrushForPatch(g_nSelectedPatch));
}



/*
===============
SampleSinglePatch
===============
*/
void SampleSinglePatch (float ctrl[3][3][5], float u, float v, float out[5]) {
	float	vCtrl[3][5];
	int		vPoint;
	int		axis;

	// find the control points for the v coordinate
	for (vPoint = 0 ; vPoint < 3 ; vPoint++) 
	{
		for (axis = 0 ; axis < 5 ; axis++) 
		{
			float	a, b, c;
			float	qA, qB, qC;

			a = ctrl[0][vPoint][axis];
			b = ctrl[1][vPoint][axis];
			c = ctrl[2][vPoint][axis];
			qA = a - 2 * b + c;
			qB = 2 * b - 2 * a;
			qC = a;

			vCtrl[vPoint][axis] = qA * u * u + qB * u + qC;
		}
	}

	// interpolate the v value
	for (axis = 0 ; axis < 5 ; axis++) 
	{
		float	a, b, c;
		float	qA, qB, qC;

		a = vCtrl[0][axis];
		b = vCtrl[1][axis];
		c = vCtrl[2][axis];
		qA = a - 2 * b + c;
		qB = 2 * b - 2 * a;
		qC = a;

		out[axis] = qA * v * v + qB * v + qC;
	}
}

/*
===================
DrawSinglePatch
===================
*/
void DrawSinglePatch (float ctrl[3][3][5], bool bPoints) 
{
	int		i, j;
	float	u, v;
	float	verts[CBLOCK_SUBDIVISIONS+1][CBLOCK_SUBDIVISIONS+1][5];

	for (i = 0 ; i <= CBLOCK_SUBDIVISIONS ; i++) 
	{
		for (j = 0 ; j <= CBLOCK_SUBDIVISIONS ; j++) 
		{
			u = (float)i / CBLOCK_SUBDIVISIONS;
			v = (float)j / CBLOCK_SUBDIVISIONS;
			SampleSinglePatch (ctrl, u, v, verts[i][j]);
		}
	}
	// at this point we have 
	for (i = 0 ; i < CBLOCK_SUBDIVISIONS ; i++) 
	{
		glBegin (GL_QUAD_STRIP);

		for (j = 0 ; j <= CBLOCK_SUBDIVISIONS ; j++) 
		{
			glTexCoord2fv( verts[i+1][j] + 3 );
			glVertex3fv( verts[i+1][j] );
			glTexCoord2fv( verts[i][j] + 3 );
			glVertex3fv( verts[i][j] );
		}
		glEnd ();
	}

}

/*
=================
DrawPatchMesh
=================
*/
//FIXME: this routine needs to be reorganized.. should be about 1/4 the size and complexity
void patchMesh_c::drawPatchMesh(bool bPoints, bool bShade) {
	int		i, j, k, l;
	float	ctrl[3][3][5];

	bool bOverlay = this->bOverlay;
	int nDrawMode = g_pParentWnd->GetCamera()->Camera().draw_mode;

	if (g_PrefsDlg.m_bDisplayLists)
	{
	if (this->bDirty || this->nListID <= 0)
	{
	if (this->nListID <= 0)
	this->nListID = glGenLists(1);
	if (this->nListID > 0)
	{
		glNewList(this->nListID, GL_COMPILE_AND_EXECUTE);
	}



	if (this->type != PATCH_TRIANGLE)
	{
		//vec3_t *vMeshData = new vec3_t[this->width * this->height];
		for ( i = 0 ; i + 2 < this->width ; i += 2 ) 
		{
			for ( j = 0 ; j + 2 < this->height ; j += 2 ) 
			{
				for ( k = 0 ; k < 3 ; k++ ) 
				{
					vec3_t vAvg;
					vAvg[0] = vAvg[1] = vAvg[2] = 0;
					for ( l = 0 ; l < 3 ; l++ ) 
					{
						ctrl[k][l][0] = this->ctrl[ i + k ][ j + l ].xyz[ 0 ];
						ctrl[k][l][1] = this->ctrl[ i + k ][ j + l ].xyz[ 1 ];
						ctrl[k][l][2] = this->ctrl[ i + k ][ j + l ].xyz[ 2 ];
						ctrl[k][l][3] = this->ctrl[ i + k ][ j + l ].xyz[ 3 ];
						ctrl[k][l][4] = this->ctrl[ i + k ][ j + l ].xyz[ 4 ];
					}
				}
				DrawSinglePatch( ctrl, bPoints );
			}
		}
	}
	else
	{
		glBegin (GL_TRIANGLES);
		glTexCoord2fv( this->ctrl[0][0].st);
		glVertex3fv( this->ctrl[0][0].xyz);
		glTexCoord2fv( this->ctrl[2][0].st);
		glVertex3fv( this->ctrl[2][0].xyz);
		glTexCoord2fv( this->ctrl[2][2].st);
		glVertex3fv( this->ctrl[2][2].xyz);
		glEnd();
	}

	if (this->nListID > 0)
	{
		glEndList();
		this->bDirty = false;
	}
	}
	else
	{
		glCallList(this->nListID);
	}
	}
	else
	{
		for ( i = 0 ; i + 2 < this->width ; i += 2 ) 
		{
			for ( j = 0 ; j + 2 < this->height ; j += 2 ) 
			{
				for ( k = 0 ; k < 3 ; k++ ) 
				{
					for ( l = 0 ; l < 3 ; l++ ) 
					{
						ctrl[k][l][0] = this->ctrl[ i + k ][ j + l ].xyz[ 0 ];
						ctrl[k][l][1] = this->ctrl[ i + k ][ j + l ].xyz[ 1 ];
						ctrl[k][l][2] = this->ctrl[ i + k ][ j + l ].xyz[ 2 ];
						ctrl[k][l][3] = this->ctrl[ i + k ][ j + l ].xyz[ 3 ];
						ctrl[k][l][4] = this->ctrl[ i + k ][ j + l ].xyz[ 4 ];
					}
				}
				DrawSinglePatch( ctrl, bPoints );
			}
		}
	}

	vec3_t* pSelectedPoints[256];
	int nIndex = 0;

	glPushAttrib(GL_CURRENT_BIT);
	//--glDisable(GL_BLEND);
	//--glDisable (GL_DEPTH_TEST);
	//glDisable (GL_TEXTURE_2D);

	bool bDisabledLighting = glIsEnabled(GL_LIGHTING);
	if (bDisabledLighting)
	{
		glDisable(GL_LIGHTING);
	}


	// FIXME: this bend painting code needs to be rolled up significantly as it is a cluster fuck right now
	if (bPoints && (g_qeglobals.d_select_mode == sel_curvepoint || g_qeglobals.d_select_mode == sel_area || g_bPatchBendMode || g_bPatchInsertMode))
	{
	bOverlay = false;

	// bending or inserting
	if (g_bPatchBendMode || g_bPatchInsertMode)
	{
	glPointSize(6);
	if (g_bPatchAxisOnRow)
	{
	glColor3f(1, 0, 1);
	glBegin(GL_POINTS);
	for (i = 0; i < this->width; i++)
	{
	glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[i][g_nPatchAxisIndex].xyz));
	}
	glEnd();

	// could do all of this in one loop but it was pretty messy
	if (g_bPatchInsertMode)
	{
		glColor3f(0, 0, 1);
		glBegin(GL_POINTS);
		for (i = 0; i < this->width; i++)
		{
			glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[i][g_nPatchAxisIndex].xyz));
			glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[i][g_nPatchAxisIndex+1].xyz));
		}
		glEnd();
	}
	else
	{
	if (g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN)
	{
	glColor3f(0, 0, 1);
	glBegin(GL_POINTS);
	if (g_nPatchBendState == BEND_SELECT_ORIGIN)
	{
		glVertex3fv(g_vBendOrigin);
	}
	else
	{
	for (i = 0; i < this->width; i++)
	{
	if (g_bPatchLowerEdge)
	{
	for (j = 0; j < g_nPatchAxisIndex; j++)
	glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[i][j].xyz));
	}
	else
	{
	for (j = this->height-1; j > g_nPatchAxisIndex; j--)
	glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[i][j].xyz));
	}
	}
	}
	glEnd();
	}
	}
	}
	else
	{
	glColor3f(1, 0, 1);
	glBegin(GL_POINTS);
	for (i = 0; i < this->height; i++)
	{
		glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[g_nPatchAxisIndex][i].xyz));
	}
	glEnd();

	// could do all of this in one loop but it was pretty messy
	if (g_bPatchInsertMode)
	{
		glColor3f(0, 0, 1);
		glBegin(GL_POINTS);
		for (i = 0; i < this->height; i++)
		{
			glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[g_nPatchAxisIndex][i].xyz));
			glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[g_nPatchAxisIndex+1][i].xyz));
		}
		glEnd();
	}
	else
	{
		if (g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN)
		{
			glColor3f(0, 0, 1);
			glBegin(GL_POINTS);
			for (i = 0; i < this->height; i++)
			{
				if (g_nPatchBendState == BEND_SELECT_ORIGIN)
				{
					glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[g_nBendOriginIndex][i].xyz));
				}
				else
				{
					if (g_bPatchLowerEdge)
					{
						for (j = 0; j < g_nPatchAxisIndex; j++)
							glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[j][i].xyz));
					}
						else
						{
							for (j = this->width-1; j > g_nPatchAxisIndex; j--)
								glVertex3fv(reinterpret_cast<float(*)>(&this->ctrl[j][i].xyz));
						}
					}
				}
				glEnd();
			}
		}
	}
	}
	else // just painting the grid for selection
	{
		glPointSize(6);
		for ( i = 0 ; i < this->width ; i++ )
		{
			for ( j = 0 ; j < this->height ; j++ ) 
			{
				glBegin(GL_POINTS);
				// FIXME: need to not do loop lookups inside here
				int n = PointValueInMoveList(this->ctrl[i][j].xyz);
				if (n >= 0)
				{
					pSelectedPoints[nIndex++] = reinterpret_cast<float(*)[3]>(&this->ctrl[i][j].xyz);
				}

				if (i & 0x01 || j & 0x01)
					glColor3f(1, 0, 1);
				else
					glColor3f(0, 1, 0);
				glVertex3fv(this->ctrl[i][j].xyz);
				glEnd();
			}
		}
	}
	if (nIndex > 0)
	{
		glBegin(GL_POINTS);
		glColor3f(0, 0, 1);
		while (nIndex-- > 0)
		{
			glVertex3fv(*pSelectedPoints[nIndex]);
		}
		glEnd();
	}
	}

	if (bOverlay)
	{
		glPointSize(6);
		glColor3f(0.5, 0.5, 0.5);
		for ( i = 0 ; i < this->width ; i++ )
		{
			glBegin(GL_POINTS);
			for ( j = 0 ; j < this->height ; j++ ) 
			{
				if (i & 0x01 || j & 0x01)
					glColor3f(0.5, 0, 0.5);
				else
					glColor3f(0, 0.5, 0);
					glVertex3fv(this->ctrl[i][j].xyz);
			}
			glEnd();
		}
	}

	if (bDisabledLighting)
	{
		glEnable(GL_LIGHTING);
	}

	glPopAttrib();

}

/*
==================
Patch_DrawXY
==================
*/
void patchMesh_c::drawPatchXY()
{
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	if (this->bSelected)
	{
		glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES]);
	}
	else
		glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES]);

	this->drawPatchMesh(this->bSelected);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

/*
==================
Patch_DrawCam
==================
*/
void patchMesh_c::drawPatchCam()
{
	glColor3f (1,1,1);
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (g_bPatchWireFrame)
	{
		glDisable( GL_CULL_FACE );
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		this->drawPatchMesh(this->bSelected, true );
		glEnable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
		glCullFace(GL_FRONT);
		glBindTexture (GL_TEXTURE_2D, this->d_texture->getFirstColorMapStage()->getTexture(0)->getInternalHandleU32());

		if (this->d_texture->hasEditorTransparency() && this->d_texture->getEditorTransparency() < 1.0)
		{
			//glEnable ( GL_BLEND );
			//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(this->d_texture->getEditorColor()[0], this->d_texture->getEditorColor()[1], this->d_texture->getEditorColor()[2], this->d_texture->getEditorTransparency()); 
		}
		this->drawPatchMesh(this->bSelected, true );

		glCullFace(GL_BACK);
		//glDisable(GL_TEXTURE_2D);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

		glDisable ( GL_BLEND );
		this->drawPatchMesh(this->bSelected, true );
	}

#if 0 // this paints normal indicators on the ctrl points
//--glDisable (GL_DEPTH_TEST);
glDisable (GL_TEXTURE_2D);
glColor3f (1,1,1);

for (int i = 0; i < this->width; i++)
{
for (int j = 0; j < this->height; j++)
{
vec3_t temp;
glBegin (GL_LINES);
glVertex3fv (this->ctrl[i][j].xyz);
VectorMA (this->ctrl[i][j].xyz, 8, this->ctrl[i][j].normal, temp);
glVertex3fv (temp);
glEnd ();
}
}
glEnable (GL_TEXTURE_2D);
//--glEnable (GL_DEPTH_TEST);
#endif


	glPopAttrib();
}






//
//================
//Patch_BuildPoints
//================
void Patch_BuildPoints (brush_s *b) 
{
	face_s		*f;
	b->patchBrush = false;
	for (f=b->brush_faces ; f ; f=f->next) 
  {
		if (f->texdef.flags & SURF_PATCH) 
    {
			b->patchBrush = true;
      //vec3_t vMin, vMax;
      //Patch_CalcBounds(&patchMeshes[b->nPatchID], vMin, vMax);
      //VectorCopy(vMin, b->mins);
      //VectorCopy(vMax, b->maxs);
			break;
		}
	}
}



//
//===============
//Patch_WriteFile
//===============
//
#if 0
void Patch_WriteFile (char *name) 
{
	char patchName[1024];
  time_t ltime;
	strcpy(patchName, name);
	StripExtension (patchName);
	strcat (patchName, ".patch");

  FILE *file = fopen(patchName, "w");
  if (file)
  {
	  time(&ltime);
	  fprintf (file, "// %s saved on %s\n", name, ctime(&ltime) );

    for (int i = 0; i < numPatchMeshes; i++)
    {
		  fprintf(file, "{\n");
      fprintf(file, " %s\n", patchMeshes[i].d_texture->getName());
      fprintf(file, " ( %i %i %i ) \n", patchMeshes[i].width, patchMeshes[i].height, patchMeshes[i].negative);
      for (int w = 0; w < patchMeshes[i].width; w++)
      {
        for (int h = 0; h < patchMeshes[i].height; h++)
        {
          fprintf(file, " ( ");
          for (int k = 0; k < 5; k++)
          {
            float f = patchMeshes[i].ctrl[w][h][k];
            if (f == int(f))
              fprintf(file, "%i ", (int)f);
            else
              fprintf(file, "%f ", f);
          }
          fprintf(file, ")\n");
        }
      }
      fprintf(file, "}\n");
    }
    fclose(file);
  }
}
#endif

/*
==================
Patch_Move
==================
*/
void patchMesh_c::movePatch(const vec3_t vMove, bool bRebuild)
{
	this->bDirty = true;
	for (int w = 0; w < this->width; w++)
	{
		for (int h = 0; h < this->height; h++)
		{
			this->ctrl[w][h].xyz += vMove;
		}
	}
	if (bRebuild)
	{
		vec3_c vMin, vMax;
		this->calcPatchBounds(vMin, vMax);
		//Brush_RebuildBrush(patchMeshes[n].pSymbiot, vMin, vMax);
	}
	UpdatePatchInspector();
}

/*
==================
Patch_ApplyMatrix
==================
*/
void Patch_ApplyMatrix(patchMesh_c *p, const class vec3_c &vOrigin, const vec3_c vMatrix[3], bool bSnap)
{
  vec3_c vTemp;

  for (int w = 0; w < p->width; w++)
  {
    for (int h = 0; h < p->height; h++)
    {
      if ( (g_qeglobals.d_select_mode == sel_curvepoint || g_bPatchBendMode)
        && PointInMoveList(p->ctrl[w][h].xyz) == -1)
        continue;
			vTemp = p->ctrl[w][h].xyz - vOrigin;
			for (int j = 0; j < 3; j++)
      {
        p->ctrl[w][h].xyz[j] = vTemp.dotProduct(vMatrix[j]) + vOrigin[j];
        if (bSnap)
        {
          p->ctrl[w][h].xyz[j] = floor(p->ctrl[w][h].xyz[j] + 0.5);
        }
      }
    }
  }
  vec3_c vMin, vMax;
  p->calcPatchBounds(vMin, vMax);
  Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
}

/*
==================
Patch_EditPatch
==================
*/
void Patch_EditPatch()
{
	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_num_move_points = 0;

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c* p = pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ ) 
			{
				for ( int j = 0 ; j < p->height ; j++ ) 
				{
					g_qeglobals.d_points[g_qeglobals.d_numpoints] = p->ctrl[i][j].xyz;
					if (g_qeglobals.d_numpoints < MAX_POINTS-1)
					{
						g_qeglobals.d_numpoints++;
					}
				}
			}
		}
	}
	g_qeglobals.d_select_mode = sel_curvepoint;
	//--g_nSelectedPatch = n;
}



/*
==================
Patch_Deselect
==================
*/
//FIXME: need all sorts of asserts throughout a lot of this crap
void Patch_Deselect()
{
	//--g_nSelectedPatch = -1;
	clearSelection();

	for (brush_s *b = selected_brushes.next ; b != 0 && b != &selected_brushes ; b=b->next)
	{
		if (b->patchBrush)
		{
			b->pPatch->bSelected = false;
		}
	}

	//for (int i = 0; i < numPatchMeshes; i++)
	//  patchMeshes[i].bSelected = false;

	if (g_bPatchBendMode)
		Patch_BendToggle();
	if (g_bPatchInsertMode)
		Patch_InsDelToggle();
}


/*
==================
Patch_Select
==================
*/
void Patch_Select(patchMesh_c *p)
{
	// maintained for point manip.. which i need to fix as this 
	// is pf error prone
	//--g_nSelectedPatch = n;
	p->bSelected = true;
}


/*
==================
Patch_Deselect
==================
*/
void Patch_Deselect(patchMesh_c *p)
{
	p->bSelected = false;
}


/*
==================
Patch_Delete
==================
*/
void Patch_Delete(patchMesh_c *p)
{
	p->pSymbiot->pPatch = NULL;
	p->pSymbiot->patchBrush = false;

	delete p;

	UpdatePatchInspector();
}


/*
==================
Patch_Scale
==================
*/
void Patch_Scale(patchMesh_c *p, const vec3_t vOrigin, const vec3_t vAmt, bool bRebuild)
{

	for (int w = 0; w < p->width; w++)
	{
		for (int h = 0; h < p->height; h++)
		{
			if (g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(p->ctrl[w][h].xyz) == -1)
				continue;
			for (int i=0 ; i<3 ; i++)
			{
				p->ctrl[w][h].xyz[i] -= vOrigin[i];
				p->ctrl[w][h].xyz[i] *= vAmt[i];
				p->ctrl[w][h].xyz[i] += vOrigin[i];
			}
		}
	}
	if (bRebuild)
	{
		vec3_c vMin, vMax;
		p->calcPatchBounds(vMin, vMax);
		Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	}
	UpdatePatchInspector();
}


/*
==================
Patch_Cleanup
==================
*/
void Patch_Cleanup()
{
	//--g_nSelectedPatch = -1;
	//numPatchMeshes = 0;
}



/*
==================
Patch_SetView
==================
*/
void Patch_SetView(int n)
{
	g_bSameView = (n == g_nPatchClickedView);
	g_nPatchClickedView = n;
}


/*
==================
Patch_SetTexture
==================
*/
// FIXME: need array validation throughout
void Patch_SetTexture(patchMesh_c *p, texdef_t *tex_def)
{
	p->d_texture = QERApp_TryTextureForName(tex_def->name);
	UpdatePatchInspector();
}


/*
==================
Patch_DragScale
==================
*/
bool Patch_DragScale(patchMesh_c *p, const vec3_c &vAmt, const vec3_c &vMove)
{
  vec3_c vMin, vMax, vScale, vTemp, vMid;
  int i;

  p->calcPatchBounds(vMin, vMax);

  vTemp = vMax - vMin;

  // if we are scaling in the same dimension the patch has no depth
  for (i = 0; i < 3; i ++)
  {
    if (vTemp[i] == 0 && vMove[i] != 0)
    {
      //Patch_Move(n, vMove, true);
      return false;
    }
  }
  
  for (i=0 ; i<3 ; i++)
    vMid[i] = (vMin[i] + ((vMax[i] - vMin[i]) / 2));

  for (i = 0; i < 3; i++)
  {
    if (vAmt[i] != 0)
    {
      vScale[i] = 1.0 + vAmt[i] / vTemp[i];
    }
    else
    {
      vScale[i] = 1.0;
    }
  }

  Patch_Scale(p, vMid, vScale, false);

  vTemp = vMax - vMin;

  p->calcPatchBounds(vMin, vMax);
  
  vMid = vMax - vMin;

  vTemp = vMid - vTemp;

  vTemp *= 0.5;

  // abs of both should always be equal
  if (!vMove.vectorCompare(vAmt))
  {
    for (i = 0; i < 3; i++)
    {
      if (vMove[i] != vAmt[i])
        vTemp[i] = -(vTemp[i]);
    }
  }

  p->movePatch(vTemp);
  return true;
}


/*
==================
Patch_AddRow
==================
*/
void Patch_AddRow(patchMesh_c *p)
{
  vec3_c vMin, vMax, vTemp;
  int i, j;


  if (p->height+2 < MAX_PATCH_HEIGHT)
  {
    p->calcPatchBounds(vMin, vMax);
    vTemp = vMax - vMin;
    for (i = 0; i < 3; i++)
    {
      vTemp[i] /= p->height + 2;
    }

    for (j = 0; j < p->width; j++)
    {
      p->ctrl[j][p->height+1].xyz = p->ctrl[j][p->height].xyz;
      p->ctrl[j][p->height+2].xyz = p->ctrl[j][p->height].xyz;
      p->height += 2;
      i = 1;
      while (i < p->height)
      {
        p->ctrl[j][i].xyz += vTemp;
        i++;
      }
    }
    
    p->calcPatchBounds(vMin, vMax);
    Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
  }
  UpdatePatchInspector();
}

/*
==================
Patch_InsertColumn
==================
*/
void Patch_InsertColumn(patchMesh_c *p, bool bAdd)
{
  int h, w, i, j;
  vec3_c vTemp;
  
  if (p->width + 2 >= MAX_PATCH_WIDTH)
    return;

  if (bAdd) // add column?
  {
    for (h = 0; h < p->height; h++)
    {
      j = p->width-1;
      vTemp = p->ctrl[j][h].xyz - p->ctrl[j-1][h].xyz;
      for (i = 0; i < 3; i++)
        vTemp[i] /= 3;

      memcpy(&p->ctrl[j+2][h],&p->ctrl[j][h], sizeof(drawVert_t));
      memcpy(&p->ctrl[j][h],&p->ctrl[j-1][h], sizeof(drawVert_t));

      p->ctrl[j][h].xyz += vTemp;
      memcpy(&p->ctrl[j+1][h], &p->ctrl[j][h], sizeof(drawVert_t));
      p->ctrl[j+1][h].xyz += vTemp;
    }
  }
  else
  {
    for (h = 0; h < p->height; h++)
    {
      w = p->width-1;
      while (w >= 0)
      {
        memcpy(&p->ctrl[w+2][h],&p->ctrl[w][h], sizeof(drawVert_t));
        w--;
      }
      vTemp = p->ctrl[1][h].xyz - p->ctrl[0][h].xyz;
      for (i = 0; i < 3; i++)
        vTemp[i] /= 3;
      p->ctrl[1][h].xyz = p->ctrl[0][h].xyz;
      p->ctrl[1][h].xyz += vTemp;
      p->ctrl[2][h].xyz = p->ctrl[1][h].xyz;
      p->ctrl[2][h].xyz+= vTemp;
    }
  }
  p->width += 2;
  UpdatePatchInspector();
}


/*
==================
Patch_InsertRow
==================
*/
void Patch_InsertRow(patchMesh_c *p, bool bAdd)
{
  int h, w, i, j;
  vec3_c vTemp;
  
  if (p->height + 2 >= MAX_PATCH_HEIGHT)
    return;

  if (bAdd) // add column?
  {
    for (w = 0; w < p->width; w++)
    {
      j = p->height-1;
      vTemp = p->ctrl[w][j].xyz - p->ctrl[w][j-1].xyz;
      for (i = 0; i < 3; i++)
        vTemp[i] /= 3;

      memcpy(&p->ctrl[w][j+2],&p->ctrl[w][j], sizeof(drawVert_t));
      memcpy(&p->ctrl[w][j],&p->ctrl[w][j-1], sizeof(drawVert_t));

      p->ctrl[w][j].xyz += vTemp;
      memcpy(&p->ctrl[w][j+1], &p->ctrl[w][j], sizeof(drawVert_t));
      p->ctrl[w][j+1].xyz += vTemp;
    }
  }
  else
  {
    for (w = 0; w < p->width; w++)
    {
      h = p->height-1;
      while (h >= 0)
      {
        memcpy(&p->ctrl[w][h+2],&p->ctrl[w][h], sizeof(drawVert_t));
        h--;
      }
      vTemp = p->ctrl[w][1].xyz - p->ctrl[w][0].xyz;
      for (i = 0; i < 3; i++)
        vTemp[i] /= 3;
      p->ctrl[w][1].xyz = p->ctrl[w][0].xyz;
      p->ctrl[w][1].xyz += vTemp;
      p->ctrl[w][2].xyz = p->ctrl[w][1].xyz;
      p->ctrl[w][2].xyz += vTemp;
    }
  }
  p->height += 2;
  UpdatePatchInspector();
}


/*
==================
Patch_RemoveRow
==================
*/
void Patch_RemoveRow(patchMesh_c *p, bool bFirst)
{
	if (p->height <= MIN_PATCH_HEIGHT)
		return;

	p->height -= 2;

	if (bFirst)
	{
		for (int w = 0; w < p->width; w++)
		{
			for (int h = 0; h < p->height; h++)
			{
				memcpy(&p->ctrl[w][h], &p->ctrl[w][h+2], sizeof(drawVert_t));
			}
		}
	}
	UpdatePatchInspector();
}


/*
==================
Patch_RemoveColumn
==================
*/
void patchMesh_c::removePatchColumn(bool bFirst)
{
	if (this->width <= MIN_PATCH_WIDTH)
		return;

	this->width -= 2;

	if (bFirst)
	{
		for (int h = 0; h < this->height; h++)
		{
			for (int w = 0; w < this->width; w++)
			{
				memcpy(&this->ctrl[w][h], &this->ctrl[w+2][h], sizeof(drawVert_t));
			}
		}
	}
	UpdatePatchInspector();
}


/*
==================
Patch_AdjustColumns
==================
*/
void patchMesh_c::adjustPatchColumns(int nCols)
{
	vec3_c vTemp, vTemp2;
	int i, w, h;

	if (nCols & 0x01 || this->width + nCols < 3 || this->width + nCols > MAX_PATCH_WIDTH)
		return;

	// add in column adjustment
	this->width += nCols;

	for (h = 0; h < this->height; h++)
	{
	// for each column, we need to evenly disperse this->width number 
	// of points across the old bounds

	// calc total distance to interpolate 
	vTemp = this->ctrl[this->width - 1 - nCols][h].xyz - this->ctrl[0][h].xyz;

	// amount per cycle
	for (i = 0; i < 3; i ++)
	{
		vTemp2[i] = vTemp[i] / (this->width - 1);
	}

	// move along
	for (w = 0; w < this->width-1; w++)
	{
		this->ctrl[w+1][h].xyz = this->ctrl[w][h].xyz + vTemp2;
	}

	}
	for ( w = 0 ; w < this->width ; w++ ) 
	{
		for ( h = 0 ; h < this->height ; h++ ) 
		{
			this->ctrl[w][h].st[0] = 4 * (float)w / (this->width - 1);
			this->ctrl[w][h].st[1] = 4 * (float)h / (this->height - 1);
		}
	}
	UpdatePatchInspector();
}


/*
==================
Patch_AdjustRows
==================
*/
void patchMesh_c::adjustPatchRows(int nRows)
{
	vec3_c vTemp, vTemp2;
	int i, w, h;

	if (nRows & 0x01 || this->height + nRows < 3 || this->height + nRows > MAX_PATCH_HEIGHT)
		return;

	// add in column adjustment
	this->height += nRows;

	for (w = 0; w < this->width; w++)
	{
		// for each row, we need to evenly disperse this->height number 
		// of points across the old bounds

		// calc total distance to interpolate 
		vTemp = this->ctrl[w][this->height - 1 - nRows].xyz - this->ctrl[w][0].xyz;

		//vTemp[0] = vTemp[1] = vTemp[2] = 0;
		//for (h = 0; h < this->height - nRows; h ++)
		//{
		//  VectorAdd(vTemp, this->ctrl[w][h], vTemp);
		//}

		// amount per cycle
		for (i = 0; i < 3; i ++)
		{
			vTemp2[i] = vTemp[i] / (this->height - 1);
		}

		// move along
		for (h = 0; h < this->height-1; h++)
		{
			this->ctrl[w][h+1].xyz = this->ctrl[w][h].xyz + vTemp2;
		}

	}
	for ( w = 0 ; w < this->width ; w++ ) 
	{
		for ( h = 0 ; h < this->height ; h++ ) 
		{
			this->ctrl[w][h].st[0] = 4 * (float)w / (this->width - 1);
			this->ctrl[w][h].st[1] = 4 * (float)h / (this->height - 1);
		}
	}
	UpdatePatchInspector();
}


void Patch_DisperseRows()
{
	vec3_c vTemp, vTemp2;
	int i, w, h;


	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c *p = pb->pPatch;
			p->rebuildPatch();
			for (w = 0; w < p->width; w++)
			{
				// for each row, we need to evenly disperse p->height number 
				// of points across the old bounds

				// calc total distance to interpolate 
				vTemp = p->ctrl[w][p->height - 1].xyz - p->ctrl[w][0].xyz;

				//vTemp[0] = vTemp[1] = vTemp[2] = 0;
				//for (h = 0; h < p->height - nRows; h ++)
				//{
				//  VectorAdd(vTemp, p->ctrl[w][h], vTemp);
				//}

				// amount per cycle
				for (i = 0; i < 3; i ++)
				{
					vTemp2[i] = vTemp[i] / (p->height - 1);
				}

				// move along
				for (h = 0; h < p->height-1; h++)
				{
					p->ctrl[w][h+1].xyz = p->ctrl[w][h].xyz + vTemp2;
				}
				p->naturalizePatch();

			}
		}
	}
	UpdatePatchInspector();

}

/*
==================
Patch_AdjustColumns
==================
*/
void Patch_DisperseColumns()
{
	vec3_c vTemp, vTemp2;
	int i, w, h;

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c *p = pb->pPatch;
			p->rebuildPatch();

			for (h = 0; h < p->height; h++)
			{
			// for each column, we need to evenly disperse p->width number 
			// of points across the old bounds

			// calc total distance to interpolate 
			vTemp = p->ctrl[p->width - 1][h].xyz - p->ctrl[0][h].xyz;

			// amount per cycle
			for (i = 0; i < 3; i ++)
			{
				vTemp2[i] = vTemp[i] / (p->width - 1);
			}

			// move along
			for (w = 0; w < p->width-1; w++)
			{
				p->ctrl[w+1][h].xyz = p->ctrl[w][h].xyz + vTemp2;
			}

			}
			p->naturalizePatch();
		}
	}
	UpdatePatchInspector();
}



/*
==================
Patch_AdjustSelected
==================
*/
void Patch_AdjustSelected(bool bInsert, bool bColumn, bool bFlag)
{
	bool bUpdate = false;
	for (brush_s* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
	if (pb->patchBrush)
	{
	if (bInsert)
	{
	if (bColumn)
	{
	Patch_InsertColumn(pb->pPatch, bFlag);
	}
	else
	{
	Patch_InsertRow(pb->pPatch, bFlag);
	}
	}
	else
	{
	if (bColumn)
	{
	pb->pPatch->removePatchColumn(bFlag);
	}
	else
	{
	Patch_RemoveRow(pb->pPatch, bFlag);
	}
	}
	bUpdate = true;
	vec3_c vMin, vMax;
	patchMesh_c *p = pb->pPatch;
	p->calcPatchBounds(vMin, vMax);
	Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	}
	}
	if (bUpdate)
	{
		Sys_UpdateWindows(W_ALL);
	}
}


/*
==================
Patch_AdjustSelectedRowCols
==================
*/
void Patch_AdjustSelectedRowCols(int nRows, int nCols)
{
	for (brush_s* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			Patch_InsertColumn(pb->pPatch, false);
			if (nRows != 0)
			{
				pb->pPatch->adjustPatchRows(nRows);
			}

			if (nCols != 0)
			{
				pb->pPatch->adjustPatchColumns(nCols);
			}
		}
	}
	UpdatePatchInspector();
}



void Parse1DMatrix(int x, float* p)
{
  GetToken(true); // (
  for (int i = 0; i < x; i++)
  {
    GetToken(false);
    p[i] = atof(token);
  }
  GetToken(true); // )
}

void Parse2DMatrix(int y, int x, float* p)
{
  GetToken(true); // (
  for (int i = 0; i < y; i++)
  {
    Parse1DMatrix(x, p + i*x);
  }
  GetToken(true); // )
}

void Parse3DMatrix(int z, int y, int x, float* p)
{
  GetToken(true); // (
  for (int i = 0; i < z; i++)
  {
    Parse2DMatrix(y, x, p + i*(x*MAX_PATCH_HEIGHT));
  }
  GetToken(true); // )
}

// parses a patch
brush_s* Patch_Parse(bool bOld)
{
    //--if (bOld)
    //--{
    //--  return Patch_ParseOld();
    //--}

  GetToken(true);
    
  if (strcmp(token, "{"))
    return NULL;

	patchMesh_c *pm = MakeNewPatch();


	{
		// texture def
		GetToken(true);
		
		// band-aid 
		if (strcmp(token, "("))
		{
			pm->d_texture = QERApp_TryTextureForName(token);
			GetToken(true);
		}
		else
		{
			pm->d_texture = QERApp_TryTextureForName("notexture");
			Sys_Printf("Warning: Patch read with no texture, using notexture... \n");
		}               
		
		if (strcmp(token, "("))
			return NULL;
		
		// width, height, flags (currently only negative)
		GetToken(false);
		pm->width = atoi(token);
		
		GetToken(false);
		pm->height = atoi(token);
		
		GetToken(false);
		pm->contents = atoi(token);
		
		GetToken(false);
		pm->flags = atoi(token);
		
		GetToken(false);
		pm->value = atoi(token);
		
		if (!bOld)
		{
			GetToken(false);
			pm->type = atoi(token);
		}
		
		GetToken(false);
		if (strcmp(token, ")"))
			return NULL;
		
	}



  float ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];
  Parse3DMatrix(pm->width, pm->height, 5, reinterpret_cast<float*>(&ctrl));

  int w, h;

  for (w = 0; w < pm->width; w++)
  {
    for (h = 0; h < pm->height; h++)
    {
      pm->ctrl[w][h].xyz[0] = ctrl[w][h][0];  
      pm->ctrl[w][h].xyz[1] = ctrl[w][h][1];  
      pm->ctrl[w][h].xyz[2] = ctrl[w][h][2];  
      pm->ctrl[w][h].st[0] = ctrl[w][h][3]; 
      pm->ctrl[w][h].st[1] = ctrl[w][h][4];  
    }
  }

  GetToken(true);

	//if (g_qeglobals.m_bBrushPrimitMode)
 // {
 //   // we are in brush primit mode, but maybe it's a classic patch that needs converting, test "}"
 //   if (strcmp(token, "}") && strcmp (token, "(") )
	//  {
	//	  epair_s *ep = ParseEpair();
	//	  ep->next = pm->epairs;
	//	  pm->epairs = ep;
 //     GetToken(true);
	//  }
 // }

  if (strcmp(token, "}"))
    return NULL;

  brush_s *b = AddBrushForPatch(pm, false);

  return b;
}


/*
==================
Patch_Write 
==================
*/
void Patch_Write (patchMesh_c *p, CMemFile *file)
{
  //--MemFile_fprintf(file, " {\n  patchDef3\n  {\n");
  MemFile_fprintf(file, " {\n  patchDef2\n  {\n");

  MemFile_fprintf(file, "   %s\n",p->d_texture->getName());
  //--MemFile_fprintf(file, "   ( %i %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value, p->type);
  MemFile_fprintf(file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value);


	float		ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

  int w, h;
  for (w = 0; w < p->width; w++)
  {
    for (h = 0; h < p->height; h++)
    {
      ctrl[w][h][0] = p->ctrl[w][h].xyz[0];
      ctrl[w][h][1] = p->ctrl[w][h].xyz[1];
      ctrl[w][h][2] = p->ctrl[w][h].xyz[2];
      ctrl[w][h][3] = p->ctrl[w][h].st[0];
      ctrl[w][h][4] = p->ctrl[w][h].st[1];
    }
  }

  _Write3DMatrix(file, p->width, p->height, 5, reinterpret_cast<float*>(&ctrl));

  MemFile_fprintf(file, "  }\n }\n");
}

void Patch_Write (patchMesh_c *p, FILE *file)
{
	fprintf(file, " {\n  patchDef2\n  {\n");
	
	{
		fprintf(file, "   %s\n",p->d_texture->getName());
		fprintf(file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value);
	}

	float		ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

  int w, h;
  for (w = 0; w < p->width; w++)
  {
    for (h = 0; h < p->height; h++)
    {
      ctrl[w][h][0] = p->ctrl[w][h].xyz[0];
      ctrl[w][h][1] = p->ctrl[w][h].xyz[1];
      ctrl[w][h][2] = p->ctrl[w][h].xyz[2];
      ctrl[w][h][3] = p->ctrl[w][h].st[0];
      ctrl[w][h][4] = p->ctrl[w][h].st[1];
    }
  }

  _Write3DMatrix(file, p->width, p->height, 5, reinterpret_cast<float*>(&ctrl));


  fprintf(file, "  }\n }\n");
}


/*
==================
Patch_RotateTexture
==================
*/
void Patch_RotateTexture(patchMesh_c *p, float fAngle)
{
  vec3_c vMin, vMax;
  p->calcPatchBounds(vMin, vMax);
  p->bDirty = true;
  for (int w = 0; w < p->width; w++)
  {
    for (int h = 0; h < p->height; h++)
    {
      if (g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(p->ctrl[w][h].xyz) == -1)
        continue;

      float x = p->ctrl[w][h].st[0];
      float y = p->ctrl[w][h].st[1];
      p->ctrl[w][h].st[0] = x * cos(DEG2RAD(fAngle)) - y * sin(DEG2RAD(fAngle));
      p->ctrl[w][h].st[1] = y * cos(DEG2RAD(fAngle)) + x * sin(DEG2RAD(fAngle));
    }
  }
}


/*
==================
Patch_ScaleTexture
==================
*/
void Patch_ScaleTexture(patchMesh_c *p, float fx, float fy, bool bFixup)
{
  // FIXME:
  // this hack turns scales into 1.1 or 0.9
  if (bFixup)
  {
    fx = (fx == 0) ? 1.0 : (fx > 0) ? 0.9 : 1.10;
    fy = (fy == 0) ? 1.0 : (fy > 0) ? 0.9 : 1.10;
  }
  else
  {
    if (fx == 0)
      fx = 1.0;
    if (fy == 0)
      fy = 1.0;
  }
  
  for (int w = 0; w < p->width; w++)
  {
    for (int h = 0; h < p->height; h++)
    {
      if (g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(p->ctrl[w][h].xyz) == -1)
        continue;

      p->ctrl[w][h].st[0] *= fx;
      p->ctrl[w][h].st[1] *= fy;
    }
  }
  p->bDirty = true;
}


/*
==================
Patch_ShiftTexture
==================
*/
void Patch_ShiftTexture(patchMesh_c *p, float fx, float fy)
{
  //if (fx)
  //  fx = (fx > 0) ? 0.1 : -0.1;
  //if (fy)
  //  fy = (fy > 0) ? 0.1 : -0.1;

  fx = (abs(fx) >= 1) ? fx / 10 : fx;
  fy = (abs(fy) >= 1) ? fy / 10 : fy;

  for (int w = 0; w < p->width; w++)
  {
    for (int h = 0; h < p->height; h++)
    {
      if (g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(p->ctrl[w][h].xyz) == -1)
        continue;

      p->ctrl[w][h].st[0] += fx;
      p->ctrl[w][h].st[1] += fy;
    }
  }
  p->bDirty = true;
}

void patchMesh_c::invertPatch()
{
	drawVert_t vertTemp;
	this->bDirty = true;
	for ( int i = 0 ; i < this->width ; i++ ) 
	{
		for (int j = 0; j < this->height / 2; j++)
		{
			memcpy(&vertTemp, &this->ctrl[i][this->height - 1- j], sizeof (drawVert_t));
			memcpy(&this->ctrl[i][this->height - 1 - j], &this->ctrl[i][j], sizeof(drawVert_t));
			memcpy(&this->ctrl[i][j], &vertTemp, sizeof(drawVert_t));
		}
	}
}

/*
==================
Patch_ToggleInverted
==================
*/
void Patch_ToggleInverted()
{
  bool bUpdate = false;

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      bUpdate = true;
      pb->pPatch->invertPatch();
    }
  }

  if (bUpdate)
  {
    Sys_UpdateWindows(W_ALL);
  }
  UpdatePatchInspector();
}

/*
==================
Patch_ToggleInverted
==================
*/
void Patch_InvertTexture(bool bY)
{
  bool bUpdate = false;

  float fTemp[2];
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      bUpdate = true;
      patchMesh_c *p = pb->pPatch;
      p->bDirty = true;
      if (bY)
      {
	      for ( int i = 0 ; i < p->height ; i++ ) 
        {
          for (int j = 0; j < p->width / 2; j++)
          {
            memcpy(fTemp, &p->ctrl[p->width - 1- j][i].st[0], sizeof (float[2]));
            memcpy(&p->ctrl[p->width - 1- j][i].st[0], &p->ctrl[j][i].st[0], sizeof(float[2]));
            memcpy(&p->ctrl[j][i].st[0], fTemp, sizeof(float[2]));
		      }
	      }
      }
      else
      {
	      for ( int i = 0 ; i < p->width ; i++ ) 
        {
          for (int j = 0; j < p->height / 2; j++)
          {
            memcpy(fTemp, &p->ctrl[i][p->height - 1- j].st[0], sizeof (float[2]));
            memcpy(&p->ctrl[i][p->height - 1 - j].st[0], &p->ctrl[i][j].st[0], sizeof(float[2]));
            memcpy(&p->ctrl[i][j].st[0], fTemp, sizeof(float[2]));
		      }
	      }
      }
    }
  }

  if (bUpdate)
  {
    Sys_UpdateWindows(W_ALL);
  }
  UpdatePatchInspector();
}




/*
==================
Patch_Save
==================
 Saves patch ctrl info (originally to deal with a 
 cancel in the surface dialog
*/
void Patch_Save(patchMesh_c *p)
{
	patchSave.width = p->width;
	patchSave.height = p->height;
	memcpy(patchSave.ctrl, p->ctrl, sizeof(p->ctrl));
}


/*
==================
Patch_Restore
==================
*/
void Patch_Restore(patchMesh_c *p)
{
	p->width = patchSave.width;
	p->height = patchSave.height;
	memcpy(p->ctrl, patchSave.ctrl, sizeof(p->ctrl));
}

void Patch_ResetTexturing(float fx, float fy)
{
	for (brush_s* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c *p = pb->pPatch;
			p->bDirty = true;
			for ( int i = 0 ; i < p->width ; i++ ) 
			{
				for ( int j = 0 ; j < p->height ; j++ ) 
				{
					p->ctrl[i][j].st[0] = fx * (float)i / (p->width - 1);
					p->ctrl[i][j].st[1] = fy * (float)j / (p->height - 1);
				}
			}
		}
	}
}


void Patch_FitTexturing()
{
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			patchMesh_c *p = pb->pPatch;
			p->bDirty = true;
			for ( int i = 0 ; i < p->width ; i++ ) 
			{
				for ( int j = 0 ; j < p->height ; j++ ) 
				{
					p->ctrl[i][j].st[0] = 1 * (float)i / (p->width - 1);
					p->ctrl[i][j].st[1] = 1 * (float)j / (p->height - 1);
				}
			}
		}
	}
}


void Patch_SetTextureInfo(texdef_t *pt)
{
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			if (pt->rotate)
				Patch_RotateTexture(pb->pPatch, pt->rotate);

			if (pt->shift[0] || pt->shift[1])
				Patch_ShiftTexture(pb->pPatch, pt->shift[0], pt->shift[1]);

			if (pt->scale[0] || pt->scale[1])
				Patch_ScaleTexture(pb->pPatch, pt->scale[0], pt->scale[1], false);

			patchMesh_c *p = pb->pPatch;
			p->contents = pt->contents;
			p->flags = pt->flags;
			p->value = pt->value;
		}
	}
}

bool WINAPI OnlyPatchesSelected()
{
	if (g_ptrSelectedFaces.GetSize() > 0 || selected_brushes.next == &selected_brushes)
		return false;
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (!pb->patchBrush)
		{
			return false;
		}
	}
	return true;
}

bool WINAPI AnyPatchesSelected()
{
	if (g_ptrSelectedFaces.GetSize() > 0  || selected_brushes.next == &selected_brushes)
		return false;
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			return true;
		}
	}
	return false;
}

patchMesh_c* SinglePatchSelected()
{
	if (selected_brushes.next->patchBrush)
	{
		return selected_brushes.next->pPatch;
	}
	return NULL;
}

void Patch_BendToggle()
{
	if (g_bPatchBendMode)
	{
		g_bPatchBendMode = false;
		HideInfoDialog();
		g_pParentWnd->UpdatePatchToolbarButtons() ;
		return;
	}

	brush_s* b = selected_brushes.next;

	if (!QE_SingleBrush() || !b->patchBrush)
	{
		Sys_Printf("Must bend a single patch");
		return;
	}

	Patch_Save(b->pPatch);
	g_bPatchBendMode = true;
	g_nPatchBendState = BEND_SELECT_ROTATION;
	g_bPatchAxisOnRow = true;
	g_nPatchAxisIndex = 1;
	ShowInfoDialog(g_pBendStateMsg[BEND_SELECT_ROTATION]);
}

void Patch_BendHandleTAB()
{
  if (!g_bPatchBendMode)
  {
    return;
  }

	brush_s* b = selected_brushes.next;
  if (!QE_SingleBrush() || !b->patchBrush)
  {
    Patch_BendToggle();
    Sys_Printf("No patch to bend!");
		return;
  }

  patchMesh_c *p = b->pPatch;

  bool bShift = (GetKeyState(VK_SHIFT) & 0x8000);

  if (g_nPatchBendState == BEND_SELECT_ROTATION)
  {
    // only able to deal with odd numbered rows/cols
    g_nPatchAxisIndex += (bShift) ? -2 : 2;
    if (g_bPatchAxisOnRow)
    {
      if ((bShift) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->height)
      {
        g_bPatchAxisOnRow = false;
        g_nPatchAxisIndex = (bShift) ? p->width-1 : 1;
      }
    }
    else
    {
      if ((bShift) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->width)
      {
        g_bPatchAxisOnRow = true;
        g_nPatchAxisIndex = (bShift) ? p->height-1 : 1;
      }
    }
  }
  else
  if (g_nPatchBendState == BEND_SELECT_ORIGIN)
  {
    g_nBendOriginIndex += (bShift) ? -1 : 1;
    if (g_bPatchAxisOnRow)
    {
      if (bShift)
      {
        if (g_nBendOriginIndex < 0)
          g_nBendOriginIndex = p->width-1;
      }
      else
      {
        if (g_nBendOriginIndex > p->width-1)
          g_nBendOriginIndex = 0;
      }
      g_vBendOrigin = p->ctrl[g_nBendOriginIndex][g_nPatchAxisIndex].xyz;
    }
    else
    {
      if (bShift)
      {
        if (g_nBendOriginIndex < 0)
          g_nBendOriginIndex = p->height-1;
      }
      else
      {
        if (g_nBendOriginIndex > p->height-1)
          g_nBendOriginIndex = 0;
      }
      g_vBendOrigin = p->ctrl[g_nPatchAxisIndex][g_nBendOriginIndex].xyz;
    }
  }
  else
  if (g_nPatchBendState == BEND_SELECT_EDGE)
  {
    g_bPatchLowerEdge ^= 1;
  }
  Sys_UpdateWindows(W_ALL);
}

void Patch_BendHandleENTER()
{
  if (!g_bPatchBendMode)
  {
    return;
  }

  if (g_nPatchBendState  < BEND_BENDIT)
  {
    g_nPatchBendState++;
    ShowInfoDialog(g_pBendStateMsg[g_nPatchBendState]);
    if (g_nPatchBendState == BEND_SELECT_ORIGIN)
    {
      g_vBendOrigin[0] = g_vBendOrigin[1] = g_vBendOrigin[2] = 0;
      g_nBendOriginIndex = 0;
      Patch_BendHandleTAB();
    }
    else
    if (g_nPatchBendState == BEND_SELECT_EDGE)
    {
      g_bPatchLowerEdge = true;
    }
    else
    if (g_nPatchBendState == BEND_BENDIT)
    {
      // basically we go into rotation mode, set the axis to the center of the 
    }
  }
  else
  {
    // done
    Patch_BendToggle();
  }
  Sys_UpdateWindows(W_ALL);

}


void Patch_BendHandleESC()
{
  if (!g_bPatchBendMode)
  {
    return;
  }
  Patch_BendToggle();
	brush_s* b = selected_brushes.next;
  if (QE_SingleBrush() && b->patchBrush)
  {
    Patch_Restore(b->pPatch);
  }
  Sys_UpdateWindows(W_ALL);
}

void Patch_SetBendRotateOrigin(patchMesh_c *p)
{
#if 1
  int nType = g_pParentWnd->ActiveXY()->GetViewType();
  int nDim3 = (nType == XY) ? 2 : (nType == YZ) ? 0 : 1;

  g_vBendOrigin[nDim3] = 0;
  g_pParentWnd->ActiveXY()->RotateOrigin() = g_vBendOrigin;
  return;
#else
  int nDim1 = (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
  int nDim2 = (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;

  float fxLo, fyLo, fxHi, fyHi;
  fxLo = fyLo = 9999;
  fxHi = fyHi = -9999;

  if (g_bPatchAxisOnRow)
  {
    for (int i = 0; i < p->width; i++)
    {
      if (p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1] < fxLo)
        fxLo = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1];

      if (p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1] > fxHi)
        fxHi = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1];

      if (p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2] < fyLo)
        fyLo = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2];

      if (p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2] > fyHi)
        fyHi = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2];
    }
  }
  else
  {
    for (int i = 0; i < p->height; i++)
    {
      if (p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1] < fxLo)
        fxLo = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1];

      if (p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1] > fxHi)
        fxHi = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1];

      if (p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2] < fyLo)
        fyLo = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2];

      if (p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2] > fyHi)
        fyHi = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2];
    }
  }

  g_pParentWnd->ActiveXY()->RotateOrigin()[0] = g_pParentWnd->ActiveXY()->RotateOrigin()[1] = g_pParentWnd->ActiveXY()->RotateOrigin()[2] = 0.0;
  g_pParentWnd->ActiveXY()->RotateOrigin()[nDim1] = (fxLo + fxHi) * 0.5;
  g_pParentWnd->ActiveXY()->RotateOrigin()[nDim2] = (fyLo + fyHi) * 0.5;
#endif
}

// also sets the rotational origin
void Patch_SelectBendAxis()
{
	brush_s* b = selected_brushes.next;
  if (!QE_SingleBrush() || !b->patchBrush)
  {
    // should not ever happen
    Patch_BendToggle();
		return;
  }

  patchMesh_c *p = b->pPatch;
  if (g_bPatchAxisOnRow)
  {
    SelectRow(p, g_nPatchAxisIndex, false);
  }
  else
  {
    SelectColumn(p, g_nPatchAxisIndex, false);
  }

  //FIXME: this only needs to be set once... 
  Patch_SetBendRotateOrigin(p);

}

void Patch_SelectBendNormal()
{
	brush_s* b = selected_brushes.next;
  if (!QE_SingleBrush() || !b->patchBrush)
  {
    // should not ever happen
    Patch_BendToggle();
		return;
  }

  patchMesh_c *p = b->pPatch;

  g_qeglobals.d_num_move_points = 0;
  if (g_bPatchAxisOnRow)
  {
    if (g_bPatchLowerEdge)
    {
      for (int j = 0; j < g_nPatchAxisIndex; j++)
        SelectRow(p, j, true);
    }
    else
    {
      for (int j = p->height-1; j > g_nPatchAxisIndex; j--)
        SelectRow(p, j, true);
    }
  }
  else
  {
    if (g_bPatchLowerEdge)
    {
      for (int j = 0; j < g_nPatchAxisIndex; j++)
        SelectColumn(p, j, true);
    }
    else
    {
      for (int j = p->width-1; j > g_nPatchAxisIndex; j--)
        SelectColumn(p, j, true);
    }
  }
  Patch_SetBendRotateOrigin(p);
}



void Patch_InsDelToggle()
{
  if (g_bPatchInsertMode)
  {
    g_bPatchInsertMode = false;
    HideInfoDialog();
    g_pParentWnd->UpdatePatchToolbarButtons() ;
    return;
  }

	brush_s* b = selected_brushes.next;

  if (!QE_SingleBrush() || !b->patchBrush)
  {
    Sys_Printf("Must work with a single patch");
		return;
  }

  Patch_Save(b->pPatch);
	g_bPatchInsertMode = true;
  g_nPatchInsertState = INSERT_SELECT_EDGE;
  g_bPatchAxisOnRow = true;
  g_nPatchAxisIndex = 0;
  ShowInfoDialog(g_pInsertStateMsg[INSERT_SELECT_EDGE]);

}

void Patch_InsDelESC()
{
  if (!g_bPatchInsertMode)
  {
    return;
  }
  Patch_InsDelToggle();
  Sys_UpdateWindows(W_ALL);
}


void Patch_InsDelHandleENTER()
{
}

void Patch_InsDelHandleTAB()
{
  if (!g_bPatchInsertMode)
  {
    Patch_InsDelToggle();
    return;
  }

	brush_s* b = selected_brushes.next;
  if (!QE_SingleBrush() || !b->patchBrush)
  {
    Patch_BendToggle();
    Sys_Printf("No patch to bend!");
		return;
  }

  patchMesh_c *p = b->pPatch;

  // only able to deal with odd numbered rows/cols
  g_nPatchAxisIndex += 2;
  if (g_bPatchAxisOnRow)
  {
    if (g_nPatchAxisIndex >= p->height-1)
    {
      g_bPatchAxisOnRow = false;
      g_nPatchAxisIndex = 0;
    }
  }
  else
  {
    if (g_nPatchAxisIndex >= p->width-1)
    {
      g_bPatchAxisOnRow = true;
      g_nPatchAxisIndex = 0;
    }
  }
  Sys_UpdateWindows(W_ALL);
}


void _Write1DMatrix (FILE *f, int x, float *m) {
	int		i;

	fprintf (f, "( ");
	for (i = 0 ; i < x ; i++) {
		if (m[i] == (int)m[i] ) {
			fprintf (f, "%i ", (int)m[i]);
		} else {
			fprintf (f, "%f ", m[i]);
		}
	}
	fprintf (f, ")");
}

void _Write2DMatrix (FILE *f, int y, int x, float *m) {
	int		i;

	fprintf (f, "( ");
	for (i = 0 ; i < y ; i++) {
		_Write1DMatrix (f, x, m + i*x);
		fprintf (f, " ");
	}
	fprintf (f, ")\n");
}


void _Write3DMatrix (FILE *f, int z, int y, int x, float *m) {
	int		i;

	fprintf (f, "(\n");
	for (i = 0 ; i < z ; i++) {
		_Write2DMatrix (f, y, x, m + i*(x*MAX_PATCH_HEIGHT) );
	}
	fprintf (f, ")\n");
}

void _Write1DMatrix (CMemFile *f, int x, float *m) {
	int		i;

	MemFile_fprintf (f, "( ");
	for (i = 0 ; i < x ; i++) {
		if (m[i] == (int)m[i] ) {
			MemFile_fprintf (f, "%i ", (int)m[i]);
		} else {
			MemFile_fprintf (f, "%f ", m[i]);
		}
	}
	MemFile_fprintf (f, ")");
}

void _Write2DMatrix (CMemFile *f, int y, int x, float *m) {
	int		i;

	MemFile_fprintf (f, "( ");
	for (i = 0 ; i < y ; i++) {
		_Write1DMatrix (f, x, m + i*x);
		MemFile_fprintf (f, " ");
	}
	MemFile_fprintf (f, ")\n");
}


void _Write3DMatrix (CMemFile *f, int z, int y, int x, float *m) {
	int		i;

	MemFile_fprintf (f, "(\n");
	for (i = 0 ; i < z ; i++) {
		_Write2DMatrix (f, y, x, m + i*(x*MAX_PATCH_HEIGHT) );
	}
	MemFile_fprintf (f, ")\n");
}


void Patch_NaturalizeSelected(bool bCap, bool bCycleCap)
{
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      if (bCap)
        Patch_CapTexture(pb->pPatch, bCycleCap);
      else
        pb->pPatch->naturalizePatch();
    }
  }
}

bool within(vec3_t vTest, vec3_t vTL, vec3_t vBR)
{
  int nDim1 = (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
  int nDim2 = (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;
  if ((vTest[nDim1] > vTL[nDim1] && vTest[nDim1] < vBR[nDim1]) ||
      (vTest[nDim1] < vTL[nDim1] && vTest[nDim1] > vBR[nDim1]))
  {
    if ((vTest[nDim2] > vTL[nDim2] && vTest[nDim2] < vBR[nDim2]) ||
        (vTest[nDim2] < vTL[nDim2] && vTest[nDim2] > vBR[nDim2]))
      return true;
  }
  return false;
}


void Patch_SelectAreaPoints()
{
  g_qeglobals.d_num_move_points = 0;
  g_nPatchClickedView = -1;

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      patchMesh_c *p = pb->pPatch;
      for (int i = 0; i < p->width; i++)
      {
        for (int j = 0; j < p->height; j++)
        {
          if (within(p->ctrl[i][j].xyz, g_qeglobals.d_vAreaTL, g_qeglobals.d_vAreaBR))
          {
            g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
          }
        }
      }
    }
  }
}

const char* Patch_GetTextureName()
{
	brush_s* b = selected_brushes.next;
  if (b->patchBrush)
  {
    patchMesh_c *p = b->pPatch;
    if (p->d_texture->getName())
      return p->d_texture->getName();
  }
  return "";
}

patchMesh_c* Patch_Duplicate(patchMesh_c *pFrom)
{
  patchMesh_c* p = MakeNewPatch();
  memcpy(p, pFrom , sizeof(patchMesh_c));
  p->bSelected = false;
  p->bDirty = true;
  p->bOverlay = false;
  p->nListID = -1;
  // surface plugin
  
  AddBrushForPatch(p);
  return p;
}


void Patch_Thicken(int nAmount, bool bSeam)
{
  int i, j, h, w;
  brush_s *b;
  patchMesh_c *pSeam;
  vec3_c vMin, vMax;
  CPtrArray brushes;

  nAmount = -nAmount;


	if (!QE_SingleBrush())
  {
    Sys_Printf("Cannot thicken multiple patches. Please select a single patch.\n");
		return;
  }

	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      patchMesh_c *p = pb->pPatch;
      p->meshNormals();
      patchMesh_c *pNew = Patch_Duplicate(p);
      for (i = 0; i < p->width; i++)
      {
        for (j = 0; j < p->height; j++)
        {
		      pNew->ctrl[i][j].xyz.vectorMA (p->ctrl[i][j].xyz, nAmount, p->ctrl[i][j].normal);
        }
      }

      pNew->rebuildPatch();
      pNew->type |= PATCH_THICK;
      brushes.Add(pNew->pSymbiot);

      if (bSeam)
      {

        // FIXME: this should detect if any edges of the patch are closed and act appropriately
        // 
        if (!(p->type & PATCH_CYLINDER))
        {
          b = Patch_GenericMesh(3, p->height, 2, false, true);
          pSeam = b->pPatch;
          pSeam->type |= PATCH_SEAM;
          for (i = 0; i < p->height; i++)
          {
            pSeam->ctrl[0][i].xyz = p->ctrl[0][i].xyz;
            pSeam->ctrl[2][i].xyz = pNew->ctrl[0][i].xyz;
            pSeam->ctrl[1][i].xyz = pSeam->ctrl[0][i].xyz + pSeam->ctrl[2][i].xyz;
            pSeam->ctrl[1][i].xyz *= 0.5;
          }


          pSeam->calcPatchBounds(vMin, vMax);
          Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
          //--Patch_CapTexture(pSeam);
          pSeam->naturalizePatch();
          pSeam->invertPatch();
          brushes.Add(b);

          w = p->width - 1;
          b = Patch_GenericMesh(3, p->height, 2, false, true);
          pSeam = b->pPatch;
          pSeam->type |= PATCH_SEAM;
          for (i = 0; i < p->height; i++)
          {
            pSeam->ctrl[0][i].xyz = p->ctrl[w][i].xyz;
            pSeam->ctrl[2][i].xyz = pNew->ctrl[w][i].xyz;
            pSeam->ctrl[1][i].xyz = pSeam->ctrl[0][i].xyz + pSeam->ctrl[2][i].xyz;
            pSeam->ctrl[1][i].xyz *= 0.5;
          }
          pSeam->calcPatchBounds(vMin, vMax);
          Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
          //--Patch_CapTexture(pSeam);
          pSeam->naturalizePatch();
          brushes.Add(b);
        }
    
        //--{
          // otherwise we will add one per end
          b = Patch_GenericMesh(p->width, 3, 2, false, true);
          pSeam = b->pPatch;
          pSeam->type |= PATCH_SEAM;
          for (i = 0; i < p->width; i++)
          {
            pSeam->ctrl[i][0].xyz = p->ctrl[i][0].xyz;
            pSeam->ctrl[i][2].xyz = pNew->ctrl[i][0].xyz;
            pSeam->ctrl[i][1].xyz = pSeam->ctrl[i][0].xyz + pSeam->ctrl[i][2].xyz;
            pSeam->ctrl[i][1].xyz *= 0.5;
          }


          pSeam->calcPatchBounds(vMin, vMax);
          Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
          //--Patch_CapTexture(pSeam);
          pSeam->naturalizePatch();
          pSeam->invertPatch();
          brushes.Add(b);

          h = p->height - 1;
          b = Patch_GenericMesh(p->width, 3, 2, false, true);
          pSeam = b->pPatch;
          pSeam->type |= PATCH_SEAM;
          for (i = 0; i < p->width; i++)
          {
            pSeam->ctrl[i][0].xyz = p->ctrl[i][h].xyz;
           pSeam->ctrl[i][2].xyz = pNew->ctrl[i][h].xyz;
            pSeam->ctrl[i][1].xyz = pSeam->ctrl[i][0].xyz + pSeam->ctrl[i][2].xyz;
            pSeam->ctrl[i][1].xyz *= 0.5;
          }
          pSeam->calcPatchBounds(vMin, vMax);
          Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
          //--Patch_CapTexture(pSeam);
          pSeam->naturalizePatch();
          brushes.Add(b);

          eclass_s *pecNew = Eclass_ForName("func_group", false);
          if (pecNew)
          {
            entity_s *e = Entity_Create(pecNew);
            SetKeyValue(e, "type", "patchThick");
          }

        
        //--}
      }
      pNew->invertPatch();
    }
  }

  for (i = 0; i < brushes.GetSize(); i++)
  {
    Select_Brush(reinterpret_cast<brush_s*>(brushes.GetAt(i)));
  }

  UpdatePatchInspector();
}


/*
lets get another list together as far as necessities..

*snapping stuff to the grid (i will only snap movements by the mouse to the grid.. snapping the rotational bend stuff will fubar everything)

capping bevels/endcaps

hot keys

texture fix for caps

clear clipboard

*region fix

*surface dialog

*/

void Patch_SetOverlays()
{
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			pb->pPatch->bOverlay = true;
		}
	}
}



void Patch_ClearOverlays()
{
	brush_s *pb;
	for (pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			pb->pPatch->bOverlay = false;
		}
	}
	for (pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next)
	{
		if (pb->patchBrush)
		{
			pb->pPatch->bOverlay = false;
		}
	}
}

// freezes selected vertices
void Patch_Freeze()
{
  brush_s *pb;
	for (pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      pb->pPatch->bOverlay = false;
    }
  }

	for (pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      pb->pPatch->bOverlay = false;
    }
  }

}

void Patch_UnFreeze(bool bAll)
{
}



void Patch_Transpose()
{
	int		i, j, w;
  drawVert_t dv;
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      patchMesh_c *p = pb->pPatch;

    	if ( p->width > p->height ) 
      {
		    for ( i = 0 ; i < p->height ; i++ ) 
        {
    			for ( j = i + 1 ; j < p->width ; j++ ) 
          {
    				if ( j < p->height ) 
            {
    					// swap the value
		    			memcpy(&dv,&p->ctrl[j][i],sizeof(drawVert_t));
				    	memcpy(&p->ctrl[j][i],&p->ctrl[i][j], sizeof(drawVert_t));
    					memcpy(&p->ctrl[i][j],&dv, sizeof(drawVert_t));
    				} 
            else 
            {
    					// just copy
		    			memcpy(&p->ctrl[j][i],&p->ctrl[i][j], sizeof(drawVert_t));
		    		}
    			}
  		  }
	    } 
      else 
      {
    		for ( i = 0 ; i < p->width ; i++ ) 
        {
		  	  for ( j = i + 1 ; j < p->height ; j++ ) 
          {
    				if ( j < p->width ) 
            {
					    // swap the value
					    memcpy(&dv,&p->ctrl[i][j], sizeof(drawVert_t));
              memcpy(&p->ctrl[i][j],&p->ctrl[j][i], sizeof(drawVert_t));
    					memcpy(&p->ctrl[j][i],&dv, sizeof(drawVert_t));
		    		} 
            else 
            {
    					// just copy
    					memcpy(&p->ctrl[i][j],&p->ctrl[j][i], sizeof(drawVert_t));
		    		}
    			}
    		}
    	}

      w = p->width;
      p->width = p->height;
      p->height = w;
      p->invertPatch();
      p->rebuildPatch();
		}
	}
}



void Select_SnapToGrid()
{
	int i,j, k;
	for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
	{
    if (pb->patchBrush)
    {
      patchMesh_c *p = pb->pPatch;
#if 0
	    float		ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];
      memcpy(ctrl, p->ctrl, sizeof(p->ctrl));
      i = p->width;
      p->width = p->height;
      p->height = i;
      for (i = 0; i < p->width; i++)
      {
        int l = p->height-1;
        for (j = 0; j < p->height; j++)
        {
          for (k = 0; k < 5; k++)
          {
            p->ctrl[i][l][k] = ctrl[j][i][k];
          }
          l--;
        }
      }
#else
      for (i = 0; i < p->width; i++)
      {
        for (j = 0; j < p->height; j++)
        {
          for (k = 0; k < 3; k++)
          {
            p->ctrl[i][j].xyz[k] = floor(p->ctrl[i][j].xyz[k] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
          }
        }
      }
#endif
      vec3_c vMin, vMax;
      p->calcPatchBounds(vMin, vMax);
      Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
    }
    else
    {
      Brush_SnapToGrid(pb);
    }
  }
}


void Patch_FindReplaceTexture(brush_s *pb, const char *pFind, const char *pReplace, bool bForce)
{
	if (pb->patchBrush)
	{
		patchMesh_c *p = pb->pPatch;
		if (bForce || _strcmpi(p->d_texture->getName(), pFind) == 0)
		{
			p->d_texture = QERApp_TryTextureForName(pReplace);
			//strcpy(p->d_texture->getName(), pReplace);
		}
	}
}

void Patch_ReplaceQTexture(brush_s *pb, mtrAPI_i *pOld, mtrAPI_i *pNew)
{
	if (pb->patchBrush)
	{
		patchMesh_c *p = pb->pPatch;
		if (p->d_texture == pOld)
		{
			p->d_texture = pNew;
		}
	}
}


void Patch_FromTriangle(vec5_t vx, vec5_t vy, vec5_t vz)
{
  patchMesh_c* p = MakeNewPatch();
	p->d_texture = QERApp_TryTextureForName(g_qeglobals.d_texturewin.texdef.name);
	p->width = 3;
	p->height = 3;
  p->type = PATCH_TRIANGLE;

  // 0 0 goes to x
  // 0 1 goes to x
  // 0 2 goes to x

  // 1 0 goes to mid of x and z
  // 1 1 goes to mid of x y and z
  // 1 2 goes to mid of x and y

  // 2 0 goes to z
  // 2 1 goes to mid of y and z
  // 2 2 goes to y

  vec5_t vMidXZ;
  vec5_t vMidXY;
  vec5_t vMidYZ;
int j;

  for ( j = 0; j < 3; j++)
  {
    _Vector5Add(vx, vz, vMidXZ);
    _Vector5Scale(vMidXZ, 0.5, vMidXZ);
    //vMidXZ[j] = vx[j] + abs((vx[j] - vz[j]) * 0.5);
  }

  for (j = 0; j < 3; j++)
  {
    _Vector5Add(vx, vy, vMidXY);
    _Vector5Scale(vMidXY, 0.5, vMidXY);
    //vMidXY[j] = vx[j] + abs((vx[j] - vy[j]) * 0.5);
  }

  for (j = 0; j < 3; j++)
  {
    _Vector5Add(vy, vz, vMidYZ);
    _Vector5Scale(vMidYZ, 0.5, vMidYZ);
    //vMidYZ[j] = vy[j] + abs((vy[j] - vz[j]) * 0.5);
  }

  _Vector53Copy(vx, p->ctrl[0][0].xyz);
  _Vector53Copy(vx, p->ctrl[0][1].xyz);
  _Vector53Copy(vx, p->ctrl[0][2].xyz);
  p->ctrl[0][0].st[0] = vx[3];
  p->ctrl[0][0].st[1] = vx[4];
  p->ctrl[0][1].st[0] = vx[3];
  p->ctrl[0][1].st[1] = vx[4];
  p->ctrl[0][2].st[0] = vx[3];
  p->ctrl[0][2].st[1] = vx[4];

  _Vector53Copy(vMidXY, p->ctrl[1][0].xyz);
  _Vector53Copy(vx, p->ctrl[1][1].xyz);
  _Vector53Copy(vMidXZ, p->ctrl[1][2].xyz);
  p->ctrl[1][0].st[0] = vMidXY[3];
  p->ctrl[1][0].st[1] = vMidXY[4];
  p->ctrl[1][1].st[0] = vx[3];
  p->ctrl[1][1].st[1] = vx[4];
  p->ctrl[1][2].st[0] = vMidXZ[3];
  p->ctrl[1][2].st[1] = vMidXZ[4];

  _Vector53Copy(vy, p->ctrl[2][0].xyz);
  _Vector53Copy(vMidYZ, p->ctrl[2][1].xyz);
  _Vector53Copy(vz, p->ctrl[2][2].xyz);
  p->ctrl[2][0].st[0] = vy[3];
  p->ctrl[2][0].st[1] = vy[4];
  p->ctrl[2][1].st[0] = vMidYZ[3];
  p->ctrl[2][1].st[1] = vMidYZ[4];
  p->ctrl[2][2].st[0] = vz[3];
  p->ctrl[2][2].st[1] = vz[4];


  //p->naturalizePatch();

  brush_s *b = AddBrushForPatch(p);

}



//Real nitpicky, but could you make CTRL-S save the current map with the current name? (ie: File/Save)
/*
Feature addition.
When reading in textures, please check for the presence of a file called "textures.link" or something, which contains one line such as;

g:\quake3\baseq3\textures\common

 So that, when I'm reading in, lets say, my \eerie directory, it goes through and adds my textures to the palette, along with everything in common.

  Don't forget to add "Finer texture alignment" to the list. I'd like to be able to move in 0.1 increments using the Shift-Arrow Keys.

  No. Sometimes textures are drawn the wrong way on patches. We'd like the ability to flip a texture. Like the way X/Y scale -1 used to worked.

  1) Easier way of deleting rows, columns
2) Fine tuning of textures on patches (X/Y shifts other than with the surface dialog)
2) Patch matrix transposition

  1) Actually, bump texture flipping on patches to the top of the list of things to do.
2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
3) Brandon has a wierd anomoly. He fine-tunes a patch with caps. It looks fine when the patch is selected, but as soon as he escapes out, it reverts to it's pre-tuned state. When he selects the patch again, it looks tuned


*1) Flipping textures on patches
*2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
3) Easier way of deleting rows columns
*4) Thick Curves
5) Patch matrix transposition
6) Inverted cylinder capping
*7) bugs
*8) curve speed

  Have a new feature request. "Compute Bounding Box" for mapobjects (md3 files). This would be used for misc_mapobject (essentially, drop in 3DS Max models into our maps)

  Ok, Feature Request. Load and draw MD3's in the Camera view with proper bounding boxes. This should be off misc_model

  Feature Addition: View/Hide Hint Brushes -- This should be a specific case.
*/
