using Microsoft.DirectX;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using tMath;
using shared;
using System.Drawing;
using Microsoft.DirectX.Direct3D;

namespace tDirectx
{
    class DXVertexBufferXYZNormals
    {
        private VertexBuffer vertexBuffer = null;
        private int primitiveCount;

        public void create(XYZTrianglesList l, Device device)
        {
            // free previous buffer
            if (vertexBuffer != null)
            {
                vertexBuffer.Dispose();
                vertexBuffer = null;
            }

            primitiveCount = l.getTrianglesCount();
            if (primitiveCount == 0)
            {
                // "new VertexBuffer" causes exception if vertex count is 0
                return;
            }
            vertexBuffer = new VertexBuffer(typeof(CustomVertex.PositionNormalColored),
                        l.getTrianglesCount() * 3, device, 0, CustomVertex.PositionNormalColored.Format, Pool.Default);

            GraphicsStream stm = vertexBuffer.Lock(0, 0, 0);
            CustomVertex.PositionNormalColored[] verts = new CustomVertex.PositionNormalColored[l.getTrianglesCount() * 3];

            // set triangle points
            for (int i = 0; i < l.getVertexCount(); i++)
            {
                Vec3 pos = l.getXYZ(i);
                verts[i].X = (float)pos.getX();
                verts[i].Y = (float)pos.getY();
                verts[i].Z = (float)pos.getZ();
                Vec3 normal;
                l.calcTriangleNormal(i / 3, out normal);
                verts[i].Nx = (float)normal.getX();
                verts[i].Ny = (float)normal.getY();
                verts[i].Nz = (float)normal.getZ();
                verts[i].Color = Color.White.ToArgb();
            }
            stm.Write(verts);
            vertexBuffer.Unlock();
        }

        public void drawBuffer(Device device)
        {
            // used vertices has only position and color (no texture coordinates)
            device.VertexFormat = CustomVertex.PositionNormalColored.Format;
            // draw example vertex buffer (single triangle)
            device.SetStreamSource(0, vertexBuffer, 0);
            device.DrawPrimitives(PrimitiveType.TriangleList, 0, primitiveCount);
        }
        public void destroyBuffer()
        {
            vertexBuffer.Dispose();
        }
    }
}
