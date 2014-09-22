/*
============================================================================
Copyright (C) 2014 V.

This file is part of NewTools source code.

NewTools source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

NewTools source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// newTools/tShared/IConvexVolume.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using fileFormats;
using tMath;

namespace shared
{
    class PolygonVerticesMerger : IVec3ArrayIterator
    {
        // raw arrays, because they are faster
        private Vec3[] uniquePoints;
        private short[] indices;
        private short[] offsets;
        private short[] counts;
        private short numUniquePoints;
        private short numPolys;
        private short numIndices;

        public PolygonVerticesMerger(int maxBrushFaces = 2048)
        {
            uniquePoints = new Vec3[maxBrushFaces * 3];
            indices = new short[maxBrushFaces * 6];
            offsets = new short[maxBrushFaces];
            counts = new short[maxBrushFaces];
            numUniquePoints = 0;
            numPolys = 0;
            numIndices = 0;
        }
        public void reset()
        {
            numUniquePoints = 0;
            numPolys = 0;
            numIndices = 0;
        }
        private short findIndex(Vec3 p)
        {
            for (short i = 0; i < numUniquePoints; i++)
            {
                if (p.compare(uniquePoints[i]))
                    return i;
            }
            return -1;
        }
        public void addVec3Array(Vec3[] points, int numPoints)
        {
            offsets[numPolys] = numIndices;
            counts[numPolys] = (short)numPoints;
            numPolys++;
            for (int i = 0; i < numPoints; i++)
            {
                short index = findIndex(points[i]);
                if (index < 0)
                {
                    index = numUniquePoints;
                    uniquePoints[numUniquePoints] = points[i];
                    numUniquePoints++;
                }
                indices[numIndices] = index;
                numIndices++;
            }
        }
        public Vec3[] getXYZs()
        {
            return uniquePoints;
        }
        public int getNumUniquePoints()
        {
            return numUniquePoints;
        }
        public short[] getFaceIndices()
        {
            return indices;
        }
        public short[] getFaceOffsets()
        {
            return offsets;
        }
        public short[] getFaceVertsCount()
        {
            return counts;
        }
        public short getNumFaces()
        {
            return numPolys;
        }
    }
}