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
// newTools/tShared/tMath/Quaternion.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace tMath
{
    public struct Quat
    {
        private double x, y, z, w;

        public Quat(double x, double y, double z, double w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public void calculateW()
        {
            w = -Math.Sqrt(1 - x * x - y * y - z * z);
        }
        public void setXYZAndCalculateW(Vec3 v)
        {
            x = v.getX();
            y = v.getY();
            z = v.getZ();
            calculateW();
        }
        public void normalize()
        {
            double lenSq = x * x + y * y + z * z + w * w;
            double len = Math.Sqrt(lenSq);
            if (len != 0.0)
            {
                double invLen = 1.0 / len;
                x *= invLen;
                y *= invLen;
                z *= invLen;
                w *= invLen;
            }
        }
        public void inverse()
        {
            x = -x;
            y = -y;
            z = -z;
        }
        public Quat getInversed()
        {
            return new Quat(-x, -y, -z, w);
        }
        public Quat multiplyVec3(Vec3 p)
        {
            return new Quat(
                (this.w * p.getX()) + (this.y * p.getZ()) - (this.z * p.getY()),
                (this.w * p.getY()) + (this.z * p.getX()) - (this.x * p.getZ()),
                (this.w * p.getZ()) + (this.x * p.getY()) - (this.y * p.getX()),
                -(this.x * p.getX()) - (this.y * p.getY()) - (this.z * p.getZ()));
        }
        public Quat multiplyQuat(Quat q)
        {
            return new Quat(
                (this.x * q.w) + (this.w * q.x) + (this.y * q.z) - (this.z * q.y),
                (this.y * q.w) + (this.w * q.y) + (this.z * q.x) - (this.x * q.z),
                (this.z * q.w) + (this.w * q.z) + (this.x * q.y) - (this.y * q.x),
                (this.w * q.w) - (this.x * q.x) - (this.y * q.y) - (this.z * q.z));
        }
        public Vec3 rotatePoint(Vec3 p)
        {
            Quat inv = getInversed();
            inv.normalize();
            Quat tmp = this.multiplyVec3(p);
            Quat final = tmp.multiplyQuat(inv);
            return new Vec3(final.x, final.y, final.z);
        }

        public void setX(double f)
        {
            x = f;
        }
        public void setY(double f)
        {
            y = f;
        }
        public void setZ(double f)
        {
            z = f;
        }

        public override string ToString()
        {
            return x.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + y.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + z.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + w.ToString(System.Globalization.CultureInfo.InvariantCulture);
        }
        public string ToStringBraced()
        {
            return "( " + x.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + y.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + z.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + w.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " )";
        }
        public string ToStringBracedXYZ()
        {
            return "( " + x.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + y.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " " + z.ToString(System.Globalization.CultureInfo.InvariantCulture)
                + " )";
        }

    };
}
