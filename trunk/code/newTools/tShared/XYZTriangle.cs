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
// newTools/tShared/IXYZTriangle.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tMath;

namespace shared
{
    class XYZTriangle
    {
        private Vec3 a, b, c;

        public XYZTriangle(Vec3 a, Vec3 b, Vec3 c)
        {
            this.a = a;
            this.b = b;
            this.c = c;
        }

        public Vec3 getXYZ(int point)
        {
            if (point == 0)
                return a;
            if (point == 1)
                return b;
            return c;
        }
        public void calcTriangleNormal(out Vec3 normal)
        {
            Vec3 e1 = a - b;
            Vec3 e0 = a - c;
            normal = new Vec3();
            normal.crossProduct(ref e0, ref e1);
            normal.normalize();
        }
    }
}
