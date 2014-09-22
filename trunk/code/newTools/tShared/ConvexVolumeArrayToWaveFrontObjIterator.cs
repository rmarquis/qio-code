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

namespace shared
{
    public class ConvexVolumeArrayToWaveFrontObjIterator : IConvexVolumeArrayIterator
    {
        private WavefrontOBJ obj;
        private bool bMergeBrushPolygonVertices;
        private PolygonVerticesMerger merger;

        public ConvexVolumeArrayToWaveFrontObjIterator(WavefrontOBJ obj)
        {
            this.obj = obj;
            bMergeBrushPolygonVertices = true;
        }

        public override void addConvexVolume(IConvexVolume v)
        {
            if (bMergeBrushPolygonVertices == false)
            {
                ConvexVerticesBuilder.buildConvexSides(v, obj);
            }
            else
            {
                if (merger == null)
                    merger = new PolygonVerticesMerger();
                else
                    merger.reset();
                ConvexVerticesBuilder.buildConvexSides(v, merger);

                obj.addIndexedMesh(merger.getXYZs(), merger.getNumUniquePoints(), merger.getNumFaces(), merger.getFaceIndices(), merger.getFaceOffsets(), merger.getFaceVertsCount());
            }
        }
    }
}