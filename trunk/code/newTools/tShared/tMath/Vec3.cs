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
// newTools/tShared/tMath/Vec3.cs
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace tMath
{
    public struct Vec3
    {
        private double x, y, z;

        public Vec3(double nX, double nY, double nZ)
        {
            x = nX;
            y = nY;
            z = nZ;
        }
        public void set(double nX, double nY, double nZ)
        {
            x = nX;
            y = nY;
            z = nZ;
        }
        public int findSmallestAxis()
        {
            double absX = Math.Abs(x);
            double absY = Math.Abs(y);
            double absZ = Math.Abs(z);
            if (absX < absY)
            {
                if (absX < absZ)
                    return 0;
                return 2;
            }
            if (absY < absZ)
                return 1;
            return 2;
        }
        public void swapYZ()
        {
            double tmp = y;
            y = z;
            z = tmp;
        }
        /*public int findLargestAxis()
        {
            double absX = Math.Abs(x);
            double absY = Math.Abs(y);
            double absZ = Math.Abs(z);
            if (absX > absY)
            {
                if (absX > absZ)
                    return 0;
                return 2;
            }
            if (absY > absZ)
                return 1;
            return 2;
        }*/
        public void setupAxis(int ax)
        {
            if (ax == 0)
            {
                x = 1;
                y = z = 0;
            }
            else if (ax == 1)
            {
                y = 1;
                x = z = 0;
            }
            else if (ax == 2)
            {
                z = 1;
                y = x = 0;
            }
        }
        // assumes that 'this' is a normal
        public void getPerpendicular(ref Vec3 v)
        {
#if false
            int smallest = findSmallestAxis();
            double invDenom = 1.0f / (x * x + y * y + z * z);
            double invDenomSq = invDenom * invDenom;
            if (smallest == 0)
            {
                v.set(1 - x * x * invDenomSq,
                    - x * y * invDenomSq,
                    - x * z * invDenomSq);
            }
            else if (smallest == 1)
            {
                v.set(- y * x * invDenomSq,
                   1 - y * y * invDenomSq,
                    - y * z * invDenomSq);
            }
            else if (smallest == 2)
            {
                v.set(- z * x * invDenomSq,
                    -z * y * invDenomSq,
                   1 - z * z * invDenomSq);
            }
#else
            // get the squared length of XY subvector
            double lenSquared = x*x + y*y;
            if (lenSquared == 0)
            {
                v.set(1, 0, 0);
            }
            else
            {
                // normalize the vector so it's unit lenght
                double len = (double)Math.Sqrt(lenSquared);
                v.set(-y / len,x / len,0);
            }	
#endif
        }
        public void crossProduct(ref Vec3 a, ref Vec3 b)
        {
            x = a.y * b.z - a.z * b.y;
            y = a.z * b.x - a.x * b.z;
            z = a.x * b.y - a.y * b.x;
        }
        public Vec3 crossProduct(Vec3 b)
        {
            return new Vec3(
                 this.y * b.z - this.z * b.y,
                 this.z * b.x - this.x * b.z,
                 this.x * b.y - this.y * b.x
            );
        }
        public void getPerpendicular(ref Vec3 a, ref Vec3 b)
        {
            getPerpendicular(ref a);
            b.crossProduct(ref this, ref a);
        }
        public Vec3 getPerpendicular()
        {
            Vec3 p = new Vec3();
            getPerpendicular(ref p);
            return p;
        }
        public static Vec3 operator + (Vec3 a, Vec3 b)
        {
            return new Vec3(a.x+b.x,a.y+b.y,a.z+b.z);
        }
        public static Vec3 operator -(Vec3 a, Vec3 b)
        {
            return new Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
        }
        public static Vec3 operator -(Vec3 b)
        {
            return new Vec3(- b.x, - b.y, - b.z);
        }
        public static Vec3 operator *(Vec3 a, double f)
        {
            return new Vec3(a.x * f, a.y * f, a.z * f);
        }
        public static void test_getPerpendicular()
        {
            Vec3 ax_pos_x = new Vec3(1, 0, 0);
            Vec3 ax_pos_x_perp = ax_pos_x.getPerpendicular();
            Vec3 ax_pos_y = new Vec3(0, 1, 0);
            Vec3 ax_pos_y_perp = ax_pos_y.getPerpendicular();
            Vec3 ax_pos_z = new Vec3(0, 0, 1);
            Vec3 ax_pos_z_perp = ax_pos_z.getPerpendicular();
            Vec3 ax_neg_x = new Vec3(-1, 0, 0);
            Vec3 ax_neg_x_perp = ax_neg_x.getPerpendicular();
            Vec3 ax_neg_y = new Vec3(0, -1, 0);
            Vec3 ax_neg_y_perp = ax_neg_y.getPerpendicular();
            Vec3 ax_neg_z = new Vec3(0, 0, -1);
            Vec3 ax_neg_z_perp = ax_neg_z.getPerpendicular();

            Vec3 randomVec0 = (new Vec3(1, 2, 3)).getNormalized();
            Vec3 randomVec0_perp = randomVec0.getPerpendicular();
            double randomVec0_dot = randomVec0.dotProduct(randomVec0_perp);

            Vec3 randomVec1 = (new Vec3(4, -2, 3)).getNormalized();
            Vec3 randomVec1_perp = randomVec1.getPerpendicular();
            double randomVec1_dot = randomVec1.dotProduct(randomVec1_perp);

            Vec3 randomVec2 = (new Vec3(12, -2, 8)).getNormalized();
            Vec3 randomVec2_perp = randomVec2.getPerpendicular();
            double randomVec2_dot = randomVec2.dotProduct(randomVec2_perp);


        }

        public double dotProduct(Vec3 b)
        {
            return x * b.x + y * b.y + z * b.z;
        }

        public double calcLen()
        {
            return (double)Math.Sqrt(x * x + y * y + z * z);
        }
        public void clear()
        {
            x = y = z = 0;
        }
        public void normalize()
        {
            double len = calcLen();
            double inv = 1.0f / len;
            x *= inv;
            y *= inv;
            z *= inv;
        }
        public void scale(double f)
        {
            x *= f;
            y *= f;
            z *= f;
        }
        public Vec3 getNormalized()
        {
            double len = calcLen();
            double inv = 1.0f / len;
            return new Vec3(x * inv, y * inv, z * inv);
        }
        public override string ToString()
        {
            return x.ToString(CultureInfo.InvariantCulture.NumberFormat) + " " + y.ToString(CultureInfo.InvariantCulture.NumberFormat) + " " + z;
        }
        public string ToStringBraced()
        {
            return "( " + x.ToString(CultureInfo.InvariantCulture.NumberFormat) + " " + y.ToString(CultureInfo.InvariantCulture.NumberFormat) + " " + z + " )";
        }





        public Vec3 lerp(Vec3 b, double fraction)
        {
            return this + (b - this) * fraction;
        }

        public bool compare(Vec3 o)
        {
            double eps = 0.0001;
            if (Math.Abs(x - o.x) > eps)
                return false;
            if (Math.Abs(y - o.y) > eps)
                return false;
            if (Math.Abs(z - o.z) > eps)
                return false;
            return true;
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

        public double getX()
        {
            return x;
        }
        public double getY()
        {
            return y;
        }
        public double getZ()
        {
            return z;
        }
    };
}
