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
// newTools/tShared/tMath/AABB.cs
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace tMath
{
    public struct AABB
    {
        private Vec3 mins, maxs;

        public AABB(Vec3 a, Vec3 b)
        {
            mins = a;
            maxs = a;

            addPoint(b);
        }
        public void addPoint(Vec3 p)
        {
            if (mins.getX() > p.getX())
                mins.setX(p.getX());
            if (mins.getY() > p.getY())
                mins.setY(p.getY());
            if (mins.getZ() > p.getZ())
                mins.setZ(p.getZ());
            if (maxs.getX() < p.getX())
                maxs.setX(p.getX());
            if (maxs.getY() < p.getY())
                maxs.setY(p.getY());
            if (maxs.getZ() < p.getZ())
                maxs.setZ(p.getZ());
        }
        public override string ToString()
        {
            return mins.ToStringBraced() + " " + maxs.ToStringBraced();
        }
    };
}
