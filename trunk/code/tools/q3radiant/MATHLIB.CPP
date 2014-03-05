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
// mathlib.c -- math primitives

#include "stdafx.h"
#include "cmdlib.h"
#include "mathlib.h"

vec_t Q_rint (vec_t in)
{
  if (g_PrefsDlg.m_bNoClamp)
    return in;
  else
	  return (float)floor (in + 0.5);
}





void VectorRotate (vec3_t vIn, const edVec3_c &vRotation, edVec3_c &out)
{
  edVec3_c va = vIn;
  edVec3_c vWork = va;
  int nIndex[3][2];
  nIndex[0][0] = 1; nIndex[0][1] = 2;
  nIndex[1][0] = 2; nIndex[1][1] = 0;
  nIndex[2][0] = 0; nIndex[2][1] = 1;

  for (int i = 0; i < 3; i++)
  {
    if (vRotation[i] != 0)
    {
      double dAngle = DEG2RAD(vRotation[i]);
	  double c = cos(dAngle);
      double s = sin(dAngle);
      vWork[nIndex[i][0]] = va[nIndex[i][0]] * c - va[nIndex[i][1]] * s;
      vWork[nIndex[i][1]] = va[nIndex[i][0]] * s + va[nIndex[i][1]] * c;
    }
    va = vWork;
  }
  out = vWork;
}

void VectorRotate(const edVec3_c &vIn, const edVec3_c &vRotation, const edVec3_c &vOrigin, edVec3_c &out)
{
	edVec3_c vTemp = vIn - vOrigin;
	edVec3_c vTemp2;
	VectorRotate(vTemp, vRotation, vTemp2);
	out = vTemp2 + vOrigin;
}



void _Vector5Add (vec5_t va, vec5_t vb, vec5_t out)
{
	out[0] = va[0]+vb[0];
	out[1] = va[1]+vb[1];
	out[2] = va[2]+vb[2];
	out[3] = va[3]+vb[3];
	out[4] = va[4]+vb[4];
}

void _Vector5Scale (vec5_t v, vec_t scale, vec5_t out)
{
	out[0] = v[0] * scale;
	out[1] = v[1] * scale;
	out[2] = v[2] * scale;
	out[3] = v[3] * scale;
	out[4] = v[4] * scale;
}

void _Vector53Copy (vec5_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

// NOTE: added these from Ritual's Q3Radiant
void ClearBounds (vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs)
{
	int		i;
	vec_t	val;
	
	for (i=0 ; i<3 ; i++)
	{
		val = v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}


