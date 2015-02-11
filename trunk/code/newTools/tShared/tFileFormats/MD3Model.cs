using shared;
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
// newTools/tShared/tFileFormats/MD3Model.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using tMath;


namespace fileFormats
{
    interface IReadStream
    {

    }
    class MD3Frame
    {
        private Vec3 mins;
        private Vec3 maxs;
        private Vec3 localOrigin;
        private double radius;
        private string name;

        public bool readMD3Frame(ByteFileReader r)
        {
            mins = r.readVec3();
            maxs = r.readVec3();
            localOrigin = r.readVec3();
            radius = r.readFloat();
            name = r.readFixedString(16);
            return false;
        }
        public bool writeMD3Frame(ByteFileWriter w)
        {
            w.writeVec3(mins);
            w.writeVec3(maxs);
            w.writeVec3(localOrigin);
            w.writeFloat((float)radius);
            w.writeFixedString(name, 16);
            return false;
        }
        public string getName()
        {
            return name;
        }
        public Vec3 getMins()
        {
            return mins;
        }
        public Vec3 getMaxs()
        {
            return maxs;
        }
        public Vec3 getLocalOrigin()
        {
            return localOrigin;
        }
        public double getRadius()
        {
            return radius;
        }
    }
    class MD3Tag
    {
        private string tagName;
        private Vec3 origin;
        private Vec3 f, r, u; // axis

        public string getName()
        {
            return tagName;
        }
        public Vec3 getPos()
        {
            return origin;
        }
        public Vec3 getRotForward()
        {
            return f;
        }
        public Vec3 getRotRight()
        {
            return r;
        }
        public Vec3 getRotUp()
        {
            return u;
        }
        public bool readMD3Tag(ByteFileReader r)
        {
            tagName = r.readFixedString(64);
            origin = r.readVec3();
            this.f = r.readVec3();
            this.r = r.readVec3();
            this.u = r.readVec3();
            return false;
        }
        public bool writeMD3Tag(ByteFileWriter w)
        {
            w.writeFixedString(tagName,64);
            w.writeVec3(origin);
            w.writeVec3(f);
            w.writeVec3(r);
            w.writeVec3(u);
            return false;
        }
    }
    class MD3Vertex
    {
        private Vec3 xyz;
        private Vec3 normal;

        public void writeMD3Vertex(ByteFileWriter w)
        {
            short x = (short)(xyz.getX() / 0.015625);
            short y = (short)(xyz.getY() / 0.015625);
            short z = (short)(xyz.getZ() / 0.015625);
            w.writeShort(x);
            w.writeShort(y);
            w.writeShort(z);
        }
        public void readMD3Vertex(ByteFileReader r)
        {
            short x = r.readShort();
            short y = r.readShort();
            short z = r.readShort();
            xyz = new Vec3(x * 0.015625, y * 0.015625, z * 0.015625);
            byte lat = r.readByte();
            byte lng = r.readByte();
            double aLat = lat * (2.0 * Math.PI) / 255.0;
            double aLng = lng * (2.0 * Math.PI) / 255.0;
            double nX = Math.Cos(aLat) * Math.Sin(aLng);
            double nY = Math.Sin(aLat) * Math.Sin(aLng);
            double nZ = Math.Cos(aLng);
            normal = new Vec3(nX, nY, nZ);
        }
        public Vec3 getPos()
        {
            return xyz;
        }
    };
    class MD3SurfVertsFrame
    {
        private List<MD3Vertex> verts;

        public MD3SurfVertsFrame()
        {
            verts = new List<MD3Vertex>();
        }
        public void readMD3FrameVerts(ByteFileReader r, int vertexCount)
        {
            for (int i = 0; i < vertexCount; i++)
            {
                MD3Vertex v = new MD3Vertex();
                v.readMD3Vertex(r);
                verts.Add(v);
            }
        }
        public List<MD3Vertex> getVerts()
        {
            return verts;
        }
        public MD3Vertex getVertex(int vertexIndex)
        {
            return verts[vertexIndex];
        }
    }
    class MD3Surface
    {
        private string surfName;
        private int flags;
        private List<string> materials;
        private List<int> indices;
        private List<Vec2> texCoords;
        private List<MD3SurfVertsFrame> frameVerts;

        public MD3Surface()
        {
            materials = new List<string>();
            indices = new List<int>();
            texCoords = new List<Vec2>();
            frameVerts = new List<MD3SurfVertsFrame>();
        }
        public string getMaterialName(int index)
        {
            return materials[index];
        }
        public int getNumMaterials()
        {
            return materials.Count;
        }
        public int getNumVertices()
        {
            return texCoords.Count;
        }
        public int getNumTris()
        {
            return indices.Count / 3;
        }
        public void addToSimpleStaticMeshBuilder(int frameIndex, ISimpleStaticMeshBuilder o)
        {
            o.beginSurface(materials[0]);
            for (int i = 0; i < indices.Count; i += 3)
            {
                int i0 = indices[i + 0];
                int i1 = indices[i + 1];
                int i2 = indices[i + 2];
                MD3Vertex v0 = frameVerts[frameIndex].getVertex(i0);
                MD3Vertex v1 = frameVerts[frameIndex].getVertex(i1);
                MD3Vertex v2 = frameVerts[frameIndex].getVertex(i2);
                Vec2 tc0 = texCoords[i0];
                Vec2 tc1 = texCoords[i1];
                Vec2 tc2 = texCoords[i2];
                o.addTriangle(v0.getPos(), v1.getPos(), v2.getPos(), tc0, tc1, tc2);
            }
            o.endSurface();
        }
        public bool readMD3Surface(ByteFileReader r)
        {
            long surfaceDataStart = r.getPos();

            int ident = r.readInt();
            surfName = r.readFixedString(64);
            flags = r.readInt();
            int numFrames = r.readInt();
            int numMaterials = r.readInt();
            int numVerts = r.readInt();
            int numTris = r.readInt();
            int ofsTris = r.readInt();
            int ofsMaterials = r.readInt();
            int ofsTexCoords = r.readInt();
            int ofsXYZNormals = r.readInt();
            int ofsEnd = r.readInt();

            // read material names
            r.setPos(surfaceDataStart + ofsMaterials);
            for (int i = 0; i < numMaterials; i++)
            {
                string matName = r.readFixedString(64);
                int dummy = r.readInt();
                materials.Add(matName);
            }
            // read indices (triangles)
            r.setPos(surfaceDataStart + ofsTris);
            for (int i = 0; i < numTris; i++)
            {
                int i0 = r.readInt();
                int i1 = r.readInt();
                int i2 = r.readInt();
                indices.Add(i0);
                indices.Add(i1);
                indices.Add(i2);
            }
            // read texture coordinates
            r.setPos(surfaceDataStart + ofsTexCoords);
            for (int i = 0; i < numVerts; i++)
            {
                Vec2 tc = r.readVec2();
                texCoords.Add(tc);
            }
            r.setPos(surfaceDataStart + ofsXYZNormals);
            // read vertices of all frames
            for (int i = 0; i < numFrames; i++)
            {
                MD3SurfVertsFrame sfVerts = new MD3SurfVertsFrame();
                sfVerts.readMD3FrameVerts(r,numVerts);
                frameVerts.Add(sfVerts);
            }

            r.setPos(surfaceDataStart + ofsEnd);
            return false;
        }
        public bool writeMD3Surface(ByteFileWriter w)
        {
            long surfaceDataStart = w.getPos();

            int ident = 0;
            w.writeInt(ident);
            w.writeFixedString(surfName, 64);
            w.writeInt(flags);
            w.writeInt(frameVerts.Count);
            w.writeInt(materials.Count);
            w.writeInt(texCoords.Count);
            w.writeInt(indices.Count / 3);
            long offsetsAt = w.getPos();
            // write offsets
            w.writeInt(0); // ofsTris
            w.writeInt(0); // ofsMaterials
            w.writeInt(0); // ofsTexCoords
            w.writeInt(0); // ofsXYZNormals
            w.writeInt(0); // ofsEnd
            long ofsMaterials = w.getPos();
            // write material names
            foreach (string s in materials)
            {
                w.writeFixedString(s, 64);
                w.writeInt(0);
            }
            long ofsTris = w.getPos();
            foreach (int i in indices)
            {
                w.writeInt(i);
            }
            long ofsXYZNormals = w.getPos();
            foreach (MD3SurfVertsFrame f in frameVerts)
            {
                foreach (MD3Vertex v in f.getVerts())
                {
                    v.writeMD3Vertex(w);
                }
            }
            return false;
        }
    }
    class MD3Skin
    {

    }
    class ByteFileWriter
    {
        private FileStream file;
        private byte[] buffer;

        public bool beginWriting(string fileName)
        {
            try
            {
                file = new FileStream(fileName, FileMode.Create, FileAccess.Write);
            }
            catch
            {
                return true;
            }
            buffer = new byte[8];
            return false;
        }
        public void setPos(long newOfs)
        {
            file.Seek(newOfs, SeekOrigin.Begin);
        }
        public long getPos()
        {
            return file.Position;
        }
        public void writeFixedString(string s, int maxCount)
        {
            int i;
            for (i = 0; i < s.Length; i++)
            {
                if (i >= maxCount)
                    return;
                char ch = s[i];
                file.WriteByte((byte)ch);
            }
            for (; i < maxCount; i++)
            {
                file.WriteByte(0);
            }
        }
        public void writeInt(int i)
        {
            file.Write(BitConverter.GetBytes(i), 0, 4);
        }
        public void writeShort(short s)
        {
            file.Write(BitConverter.GetBytes(s), 0, 2);
        }
        public void writeFloat(float f)
        {
            file.Write(BitConverter.GetBytes(f), 0, 4);
        }
        public void writeVec3(Vec3 v)
        {
            writeFloat((float)v.getX());
            writeFloat((float)v.getY());
            writeFloat((float)v.getZ());
        }
    }
    class ByteFileReader
    {
        private FileStream file;
        private byte[] buffer;

        public bool beginReading(string fileName)
        {
            try
            {
                file = new FileStream(fileName, FileMode.Open, FileAccess.Read);
            }
            catch
            {
                return true;
            }
            buffer = new byte[8];
            return false;
        }
        public void setPos(long newOfs)
        {
            file.Seek(newOfs, SeekOrigin.Begin);
        }
        public long getPos()
        {
            return file.Position;
        }
        public int readInt()
        {
            file.Read(buffer, 0, 4);
            return BitConverter.ToInt32(buffer, 0);
        }
        public short readShort()
        {
            file.Read(buffer, 0, 2);
            return BitConverter.ToInt16(buffer, 0);
        }
        public byte readByte()
        {
            file.Read(buffer, 0, 1);
            return buffer[0];
        }
        public float readFloat()
        {
            file.Read(buffer, 0, 4);
            return BitConverter.ToSingle(buffer, 0);
        }
        public void readCharacters(char [] s, int count)
        {
            for (int i = 0; i < count; i++)
            {
                file.Read(buffer, 0, 1);
                s[i] = (char)buffer[0];
            }
        }
        public string readFixedString(int maxSize)
        {
            string s = "";
            for (int i = 0; i < maxSize; i++)
            {
                file.Read(buffer, 0, 1);
                char ch = (char)buffer[0];
                if (ch != 0)
                {
                    s += ch;
                }
            }
            return s;
        }
        public Vec3 readVec3()
        {
            float x = readFloat();
            float y = readFloat();
            float z = readFloat();
            return new Vec3(x, y, z);
        }
        public Vec2 readVec2()
        {
            float x = readFloat();
            float y = readFloat();
            return new Vec2(x, y);
        }

    }
    class MD3Model
    {
        private string fileName;
        private string internalName;
        private int flags;
        private List<MD3Frame> frames;
        private List<MD3Surface> surfaces;
        private List<MD3Skin> skins;
        private List<MD3Tag> tags;

        public MD3Model()
        {
            frames = new List<MD3Frame>();
            surfaces = new List<MD3Surface>();
            skins = new List<MD3Skin>();
            tags = new List<MD3Tag>();
        }
        public string getFileName()
        {
            return fileName;
        }
        public string getInternalName()
        {
            return internalName;
        }
        public int getNumSurfaces()
        {
            return surfaces.Count;
        }
        public int getNumTags()
        {
            return tags.Count;
        }
        public MD3Tag getTag(int tagNum)
        {
            return tags[tagNum];
        }
        public MD3Surface getSurface(int surfNum)
        {
            return surfaces[surfNum];
        }
        public int getNumFrames()
        {
            return frames.Count;
        }
        public MD3Frame getFrame(int index)
        {
            return frames[index];
        }
        public void addToSimpleStaticMeshBuilder(int frameNum, ISimpleStaticMeshBuilder o)
        {
            foreach (MD3Surface s in surfaces)
            {
                s.addToSimpleStaticMeshBuilder(frameNum,o);
            }
        }
        public bool writeMD3Model(string outFileName)
        {
            ByteFileWriter w = new ByteFileWriter();
            if (w.beginWriting(outFileName))
            {
                return true;
            }
            w.writeFixedString("IDP3", 4);
            w.writeInt(10);
            w.writeFixedString(internalName, 64);
            w.writeInt(flags);
            w.writeInt(frames.Count);
            w.writeInt(tags.Count);
            w.writeInt(surfaces.Count);
            // skins count
            w.writeInt(0);
            int ofsFrames = 108;
            int ofsTags = ofsFrames + frames.Count * 52;
            int ofsSurfaces = ofsTags + tags.Count * 110;
            // sanity check
            if (ofsTags != w.getPos())
            {
                MessageBox.Show("OfsTags mismatch.",
                "MD3 write error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1);
                return true;
            }
            // offset to first frame
            w.writeInt(ofsFrames);
            // offset to first tag
            w.writeInt(ofsTags);
            // write offset to surface
            w.writeInt(ofsSurfaces);
            // ofs ends - will be set after writing the surfaces
            w.writeInt(0);

            // sanity check
            if (ofsFrames != w.getPos())
            {
                MessageBox.Show("Ofsframes mismatch.",
                "MD3 write error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1);
                return true;
            }
            foreach (MD3Frame f in frames)
            {
                f.writeMD3Frame(w);
            }
            // sanity check
            if (ofsTags != w.getPos())
            {
                MessageBox.Show("OfsTags mismatch.",
                "MD3 write error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1);
                return true;
            }
            foreach (MD3Tag t in tags)
            {
                t.writeMD3Tag(w);
            }
            // sanity check
            if (ofsSurfaces != w.getPos())
            {
                MessageBox.Show("OfsSurfaces mismatch.",
                "MD3 write error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1);
                return true;
            }
            foreach (MD3Surface sf in surfaces)
            {
               sf.writeMD3Surface(w);
            }



            return false;
        }
        public bool loadMD3Model(string fileName)
        {
            this.fileName = fileName;
            ByteFileReader r = new ByteFileReader();
            if (r.beginReading(fileName))
            {
                return true;
            }
            char []ident = new char[4];
            r.readCharacters(ident, 4);
            
            int version = r.readInt();
            internalName = r.readFixedString(64);
            flags = r.readInt();
            int numFrames = r.readInt();
            int numTags = r.readInt();
            int numSurfs = r.readInt();
            int numSkins = r.readInt();
            int ofsFrames = r.readInt();
            int ofsTags = r.readInt();
            int ofsSurfs = r.readInt();
            int ofsEnd = r.readInt();

            r.setPos(ofsFrames);
            for (int i = 0; i < numFrames; i++)
            {
                MD3Frame f = new MD3Frame();
                f.readMD3Frame(r);
                frames.Add(f);
            }
            r.setPos(ofsTags);
            for (int i = 0; i < numTags; i++)
            {
                MD3Tag t = new MD3Tag();
                t.readMD3Tag(r);
                tags.Add(t);
            }
            r.setPos(ofsSurfs);
            for (int i = 0; i < numSurfs; i++)
            {
                MD3Surface s = new MD3Surface();
                s.readMD3Surface(r);
                surfaces.Add(s);
            }
            return false;
        }
    
    }
}
