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
// newTools/tShared/ConvexVerticesBuilder.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tMath;

namespace shared
{
    class Polygon
    {
        private Vec3[] points;
        private int numPoints;
        private const int MAX_POLYGON_POINTS = 256;

        public Polygon(Plane basePlane, int maxPoints)
        {
            points = new Vec3[maxPoints];
            numPoints = 4;
            if (maxPoints > MAX_POLYGON_POINTS)
            {
                throw new ArgumentException( "Too many points on polygon." );
            }
            Vec3 a = new Vec3(), b = new Vec3();
            basePlane.getNormal().getPerpendicular(ref a, ref b);
            float halfSize = 100000.0f;
            a.scale(halfSize);
            b.scale(halfSize);
            Vec3 center = basePlane.getCenter();
            points[0] = center - a + b;
            points[1] = center + a + b;
            points[2] = center + a - b;
            points[3] = center - a - b;

            // precision fix
            for (int i = 0; i < numPoints; i++)
            {
                float distance = basePlane.calcDistanceToPoint(points[i]);
                Vec3 better = points[i] - basePlane.getNormal() * distance;
                float newDistance = basePlane.calcDistanceToPoint(better);
                if (Math.Abs(distance) > Math.Abs(newDistance))
                {
                    points[i] = better;
                }
            }
        }
        // http://www.flipcode.com/archives/Real-time_3D_Clipping_Sutherland-Hodgeman.shtml
        // http://www.terathon.com/code/clipping.html
        // http://ezekiel.vancouver.wsu.edu/~cs442/lectures/bsp-trees/poly-split.pdf
        static PointOnPlaneSideResult[] rels = new PointOnPlaneSideResult[MAX_POLYGON_POINTS];
        static Vec3[] newPoints = new Vec3[MAX_POLYGON_POINTS];
        public bool clipByPlane(Plane pl)
        {
            // first classify polygon points against plane
            int front = 0, back = 0, on = 0;
            for (int i = 0; i < numPoints; i++)
            {
                pl.classifyPoint(points[i], out rels[i], 0.01f);
                if (rels[i].side == PlaneSide.FRONT) front++;
                else if (rels[i].side == PlaneSide.BACK) back++;
                else if (rels[i].side == PlaneSide.ON) on++;
            }
            if (on == numPoints)
                return false; //
            if (front == 0)
            {
                // no intersection
                // entire polygon clipped away
                numPoints = 0;
                return true;
            }
            if (back == 0)
            {
                // no intersection, no clipping
                return false;
            }
            // iterate points again and generate splits
            int newCount = 0;
            for (int i = 0; i < numPoints; i++)
            {
                int next = (i + 1) % numPoints;
                if (rels[i].side == PlaneSide.ON)
                {
                    newPoints[newCount] = points[i];
                    newCount++;
                    continue;
                }
                if (rels[i].side == PlaneSide.FRONT)
                {
                    newPoints[newCount] = points[i];
                    newCount++;
                }
                if (rels[next].side == PlaneSide.ON || rels[next].side == rels[i].side)
                    continue;

                // ccalculate where the point is relative to the two distances of the vertices of the plane:
                // This is always a number between 0 and 1.
                float fraction = rels[i].distance / (rels[i].distance - rels[next].distance);
                newPoints[newCount] = points[i].lerp(points[next], fraction);
                newCount++;
            }
            for (int i = 0; i < newCount; i++)
            {
                points[i] = newPoints[i];
            }
            numPoints = newCount;
            return false;
        }
        public int getNumPoints()
        {
            return numPoints;
        }
        public Vec3[] getPoints()
        {
            return points;
        }
    }
    class ConvexVerticesBuilder
    {
        public static bool buildConvexSides(IConvexVolume c, IVec3ArrayIterator op)
        { 
            for (int i = 0; i < c.getNumPlanes(); i++)
            {
                Plane sidePlane = c.getPlane(i).getOpposite();
                Polygon p = new Polygon(sidePlane, c.getNumPlanes());
                for (int j = 0; j < c.getNumPlanes(); j++)
                {
                    if (i == j)
                        continue;
                    p.clipByPlane(c.getPlane(j));
                }
                if (p.getNumPoints()>2)
                {
                    op.addVec3Array(p.getPoints(), p.getNumPoints());
                }
            }
            return false;
        }
    }
}
