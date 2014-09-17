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
// newTools/tShared/tFileFormats/MD5Mesh.cs
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
    interface IBoneOrientations
    {
        Quat getBoneQuat(int boneIndex);
        Vec3 getBonePos(int boneIndex);
    }
    class MD5Weight
    {
        private Vec3 offset;
        private int boneIndex;
        private float weight;

        public MD5Weight(int boneIndex, float weight, Vec3 ofs)
        {
            this.boneIndex = boneIndex;
            this.weight = weight;
            this.offset = ofs;
        }
        public float getWeight()
        {
            return weight;
        }
        public int getBoneIndex()
        {
            return boneIndex;
        }
        public Vec3 getOfs()
        {
            return offset;
        }
    }
    class MD5Vertex
    {
        // "currentPosition" is not loaded from .md5*** file, it's computed on the fly
        private Vec3 currentPosition;
        // texCoord and weight indexes are loaded from .md5mesh file
        private Vec2 texCoord;
        private int firstWeight, numWeights;

        public MD5Vertex(Vec2 st, int firstWeight, int numWeights)
        {
            this.texCoord = st;
            this.firstWeight = firstWeight;
            this.numWeights = numWeights;
        }
        public void setCurrentPos(Vec3 np)
        {
            this.currentPosition = np;
        }
        public int getNumWeights()
        {
            return numWeights;
        }
        public int getFirstWeightIndex()
        {
            return firstWeight;
        }
        public Vec3 getCurrentPos()
        {
            return currentPosition;
        }
        public Vec2 getTexCoords()
        {
            return texCoord;
        }
    }
    class MD5Mesh
    {
        private string materialName;
        private List<int> indices;
        private List<MD5Weight> weights;
        private List<MD5Vertex> vertices;

        public MD5Mesh()
        {
            indices = new List<int>();
            vertices = new List<MD5Vertex>();
            weights = new List<MD5Weight>();
        }
        public void addTriangle(int i0, int i1, int i2)
        {
            indices.Add(i0);
            indices.Add(i1);
            indices.Add(i2);
        }
        public void buildVertices(IBoneOrientations currentBones)
        {
            foreach(MD5Vertex v in vertices)
            {
                Vec3 pos = new Vec3();
                pos.set(0, 0, 0);
                for(int i = 0; i < v.getNumWeights(); i++)
                {
                    MD5Weight w = weights[v.getFirstWeightIndex()+i];
                    Quat q = currentBones.getBoneQuat(w.getBoneIndex());
                    Vec3 p = currentBones.getBonePos(w.getBoneIndex());
                    // get weight position rotated by joint quaternion
                    Vec3 ofsRotated = q.rotatePoint(w.getOfs());
                    // get weight position in world coordinates
                    Vec3 ofsWorld = ofsRotated + p;
                    pos += ofsWorld * w.getWeight();
                }
                v.setCurrentPos(pos);
            }
        }
        public void addToSimpleStaticMeshBuilder(ISimpleStaticMeshBuilder o)
        {
            o.beginSurface(this.materialName);
            for (int i = 0; i < indices.Count; i += 3)
            {
                int i0 = indices[i + 0];
                int i1 = indices[i + 1];
                int i2 = indices[i + 2];
                MD5Vertex v0 = vertices[i0];
                MD5Vertex v1 = vertices[i1];
                MD5Vertex v2 = vertices[i2];
                Vec2 st0 = v0.getTexCoords();
                Vec2 st1 = v1.getTexCoords();
                Vec2 st2 = v2.getTexCoords();
#if false
                Vec3 xyz0 = weights[v0.getFirstWeightIndex()].getOfs();
                Vec3 xyz1 = weights[v1.getFirstWeightIndex()].getOfs();
                Vec3 xyz2 = weights[v2.getFirstWeightIndex()].getOfs();
#else
                Vec3 xyz0 = v0.getCurrentPos();
                Vec3 xyz1 = v1.getCurrentPos();
                Vec3 xyz2 = v2.getCurrentPos();    
#endif
                o.addTriangle(xyz0, xyz1, xyz2, st0, st1, st2);
            }
            o.endSurface();
        }
        public void addVertex(MD5Vertex v)
        {
            vertices.Add(v);
        }
        public void setMaterialName(string newName)
        {
            materialName = newName;
        }
        public string getMaterialName()
        {
            return materialName;
        }
        public int getVertexCount()
        {
            return vertices.Count;
        }
        public int getTriangleCount()
        {
            return indices.Count/3;
        }
        public void addWeight(MD5Weight w)
        {
            weights.Add(w);
        }
    }
    class MD5Joint
    {
        private string name;
        private Vec3 ofs;
        private Quat rot;
        private int parentIndex;

        public MD5Joint(string name, int parentIndex, Vec3 ofs, Vec3 rot)
        {
            this.name = name;
            this.parentIndex = parentIndex;
            this.ofs = ofs;
            this.rot.setXYZAndCalculateW(rot);
        }
        public string getName()
        {
            return this.name;
        }
        public Vec3 getOfs()
        {
            return ofs;
        }
        public Quat getRot()
        {
            return rot;
        }
    }
    class MD5Model : IBoneOrientations
    {
        private string fileName;
        private string commandLine;
        private List<MD5Mesh> meshes;
        private List<MD5Joint> joints;

        // IBoneOrientations
        public Quat getBoneQuat(int boneIndex)
        {
            return joints[boneIndex].getRot();
        }
        public Vec3 getBonePos(int boneIndex)
        {
            return joints[boneIndex].getOfs();
        }

        private bool parseJoints(Parser p, int expectedCount)
        {
            if (p.isAtToken("{") == false)
            {
                return true;
            }
            for (int i = 0; i < expectedCount; i++)
            {
                string name;
                if (p.readQuotedString(out name))
                {

                    return true;
                }
                int parentIndex;
                if (p.readInt(out parentIndex))
                {

                    return true;
                }
                Vec3 ofs;
                if (p.readBracedVec3(out ofs))
                {

                    return true;
                }
                Vec3 rot;
                if (p.readBracedVec3(out rot))
                {

                    return true;
                }
                joints.Add(new MD5Joint(name, parentIndex, ofs, rot));
            }
            if (p.isAtToken("}") == false)
            {
                return true;
            }
            return false;
        }
        private bool parseMesh(Parser p)
        {
            if (p.isAtToken("{") == false)
            {
                return true;
            }
            MD5Mesh m = new MD5Mesh();
            int numVerts, numTris, numWeights;
            while(p.isAtToken("}") == false)
            {
                if (p.isAtEOF())
                {

                    return true;
                }
                if (p.isAtToken("shader"))
                {
                    string materialName;
                    if (p.readQuotedString(out materialName))
                    {
                        return true;
                    }
                    m.setMaterialName(materialName);
                }
                else if (p.isAtToken("numVerts"))
                {
                    if (p.readInt(out numVerts))
                    {
                        return true;
                    }
                    for (int i = 0; i < numVerts; i++)
                    {
                        if (p.isAtToken("vert") == false)
                        {
                            return true;
                        }
                        int vertIndex;
                        if (p.readInt(out vertIndex))
                        {
                            return true;
                        }
                        if (vertIndex != i)
                        {
                            return true;
                        }
                        Vec2 st;
                        if (p.readBracedVec2(out st))
                        {
                            return true;
                        }
                        int firstWeight;
                        if (p.readInt(out firstWeight))
                        {
                            return true;
                        }
                        int numVWeights;
                        if (p.readInt(out numVWeights))
                        {
                            return true;
                        }
                        m.addVertex(new MD5Vertex(st, firstWeight, numVWeights));
                    }
                }
                else if (p.isAtToken("numtris"))
                {
                    if (p.readInt(out numTris))
                    {
                        return true;
                    }
                    for (int i = 0; i < numTris; i++)
                    {
                        if (p.isAtToken("tri") == false)
                        {
                            return true;
                        }
                        int triIndex;
                        if (p.readInt(out triIndex))
                        {
                            return true;
                        }
                        if (triIndex != i)
                        {
                            return true;
                        }
                        int i0;
                        if (p.readInt(out i0))
                        {
                            return true;
                        }
                        int i1;
                        if (p.readInt(out i1))
                        {
                            return true;
                        }
                        int i2;
                        if (p.readInt(out i2))
                        {
                            return true;
                        }
                        m.addTriangle(i0, i1, i2);
                    }
                }
                else if (p.isAtToken("numWeights"))
                {
                    if (p.readInt(out numWeights))
                    {
                        return true;
                    }
                    for (int i = 0; i < numWeights; i++)
                    {
                        if (p.isAtToken("weight") == false)
                        {
                            return true;
                        }
                        int weightIndex;
                        if (p.readInt(out weightIndex))
                        {
                            return true;
                        }
                        if (weightIndex != i)
                        {
                            return true;
                        }
                        int boneIndex;
                        if (p.readInt(out boneIndex))
                        {
                            return true;
                        }
                        float weight;
                        if (p.readFloat(out weight))
                        {
                            return true;
                        }
                        Vec3 ofs;
                        if (p.readBracedVec3(out ofs))
                        {
                            return true;
                        }
                        m.addWeight(new MD5Weight(boneIndex, weight, ofs));
                    }
                }
                else
                {
                }
            }
            meshes.Add(m);
            return false;
        }
        public string getMeshMaterialName(int index)
        {
            return meshes[index].getMaterialName();
        }
        public int getMeshVertexCount(int index)
        {
            return meshes[index].getVertexCount();
        }
        public int getMeshTriangleCount(int index)
        {
            return meshes[index].getTriangleCount();
        }
        public string getName()
        {
            return fileName;
        }
        public int getMeshCount()
        {
            return meshes.Count;
        }
        public int getJointCount()
        {
            return joints.Count;
        }
        public string getCommandLine()
        {
            return commandLine;
        }
        public MD5Joint getJoint(int index)
        {
            return joints[index];
        }
        public void buildVertices(IBoneOrientations currentBones)
        {
            foreach (MD5Mesh m in meshes)
            {
                m.buildVertices(currentBones);
            }
        }
        public void buildBaseFrameVertices()
        {
            buildVertices(this);
        }
        public void addToSimpleStaticMeshBuilder(ISimpleStaticMeshBuilder o)
        {
            foreach(MD5Mesh m in meshes)
            {
                m.addToSimpleStaticMeshBuilder(o);
            }
        }
        public bool saveMD5MeshFile(string fileName)
        {

            // TODO
            return true;
        }
        public bool loadMD5MeshFile(string fileName)
        {
            this.fileName = fileName;

            joints = new List<MD5Joint>();
            meshes = new List<MD5Mesh>();

            Parser p = new Parser();
            if (p.beginParsingFile(fileName))
            {
                return true; // error
            }
            if (p.isAtToken("MD5Version") == false)
            {

                return true;
            }
            if (p.isAtToken("10") == false)
            {

                return true;
            }
            int numMeshes = -1;
            int numJoints = -1;
            while (p.isAtEOF() == false)
            {
                if (p.isAtToken("commandLine"))
                {
                    p.readQuotedString(out commandLine);
                }
                else if (p.isAtToken("numMeshes"))
                {
                    p.readInt(out numMeshes);
                }
                else if (p.isAtToken("numJoints"))
                {
                    p.readInt(out numJoints);
                }
                else if (p.isAtToken("joints"))
                {
                    // "numJoints" should be already set
                    if (numJoints == -1)
                    {

                        return true;
                    }
                    if (parseJoints(p, numJoints))
                    {
                        return true;
                    }
                }
                else if (p.isAtToken("mesh"))
                {
                    // "numMeshes" should be already set
                    if (numMeshes == -1)
                    {

                        return true;
                    }
                    if (parseMesh(p))
                    {
                        return true;
                    }
                }
                else
                {
                    return true;
                }
            }
            buildBaseFrameVertices();
            return false;
        }
    }
}
