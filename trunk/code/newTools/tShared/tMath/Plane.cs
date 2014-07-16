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
// newTools/tShared/tMath/Plane.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace tMath
{
    public enum PlaneSide
    {
        FRONT,
        BACK,
        ON,
    };
    public struct PointOnPlaneSideResult
    {
        public float distance;
        public PlaneSide side;
    };
    public struct Plane
    {
        private Vec3 normal;
        private float distance;

        public Plane(Vec3 p1, float p2)
        {
            normal = p1;
            distance = p2;
        }

        public void setupFromPointAndNormal(Vec3 point, Vec3 pointNormal)
        {
            normal = pointNormal;
            distance = -point.dotProduct(normal);
        }
        public void setupFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
        {
            Vec3 edgeA = b - a;
            Vec3 edgeB = c - b;
            edgeA.normalize();
            edgeB.normalize();
            // calculate triangle normal
            Vec3 normal = edgeA.crossProduct(edgeB);
            normal.normalize();
            this.setupFromPointAndNormal(a, normal);
        }

        public float calcDistanceToPoint(Vec3 p)
        {
            return distance + normal.dotProduct(p);
        }
        public void classifyPoint(Vec3 p, out PointOnPlaneSideResult res, float epsilon)
        {
            res.distance = calcDistanceToPoint(p);
            // assume that point is on the plane if the distance value is smaller than given epsilon
            if (Math.Abs(res.distance) < epsilon)
            {
                res.side = PlaneSide.ON;
            }
            else if (res.distance < 0)
            {
                res.side = PlaneSide.BACK;
            }
            else
            {
                res.side = PlaneSide.FRONT;
            }
        }
        public void setNormal(Vec3 nn)
        {
            normal = nn;
        }
        public void setDistance(float nd)
        {
            distance = nd;
        }
        public Plane getOpposite()
        {
            return new Plane(-normal, -distance);
        }
        public Vec3 getNormal()
        {
            return normal;
        }
        public float getDistance()
        {
            return distance;
        }
        public Vec3 getCenter()
        {
            return normal * -distance;
        }
        public override string ToString()
        {
            return normal.ToString() + " " + distance.ToString(System.Globalization.CultureInfo.InvariantCulture);
        }
        public string ToStringBraced()
        {
            return "( " + normal.ToString() + " " + distance.ToString(System.Globalization.CultureInfo.InvariantCulture) + " )";
        }
    };
}
