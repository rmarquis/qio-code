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
// newTools/tShared/XYZTrianglesList.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tMath;

namespace shared
{
    class XYZTrianglesList : IXYZTrianglesIterator, ISimpleStaticMeshBuilder
    {
        private List<XYZTriangle> list;

        public XYZTrianglesList()
        {
            list = new List<XYZTriangle>();
        }
        public override void addXYZTriangle(Vec3 a, Vec3 b, Vec3 c)
        {
            list.Add(new XYZTriangle(a, b, c));
        }
        //ISimpleStaticMeshBuilder begin
        public void beginSurface(string name)
        {
        }
        public void addTriangle(Vec3 a, Vec3 b, Vec3 c, Vec2 stA, Vec2 stB, Vec2 stC)
        {
            list.Add(new XYZTriangle(a, b, c));
        }
        public void endSurface()
        {
        }
        // ISimpleStaticMeshBuilder end
        public Vec3 getXYZ(int i)
        {
            int triangle = i / 3;
            int point = i % 3;
            return list[triangle].getXYZ(point);
        }
        public int getTrianglesCount()
        {
            return list.Count;
        }
        public int getVertexCount()
        {
            return list.Count * 3;
        }

        public void calcTriangleNormal(int i, out Vec3 normal)
        {
            list[i].calcTriangleNormal(out normal);
        }

        public void clear()
        {
            list.Clear();
        }
    }
}
