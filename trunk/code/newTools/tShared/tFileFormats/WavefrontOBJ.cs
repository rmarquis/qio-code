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
// newTools/tShared/tFileFormats/WavefrontOBJ.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using tMath;
using shared;

namespace fileFormats
{
    struct ObjFaceVertex
    {
        private int xyzIndex, stIndex, nIndex;

        public ObjFaceVertex(int newXyzIndex)
        {
            xyzIndex = newXyzIndex + 1;
            stIndex = nIndex = 0;
        }

        public override string ToString()
        {
            if (stIndex == 0 && nIndex == 0)
            {
                return xyzIndex.ToString();
            }
            string r = xyzIndex.ToString() + "/";
            if (stIndex != 0)
            {
                r += stIndex.ToString();
            }
            if (nIndex == 0)
                return r;
            r += "/";
            r += nIndex.ToString();
            return r;
        }
        public bool setFromString(string s)
        {
            int p = s.IndexOf('/');
            if (p < 0)
            {
                if (int.TryParse(s, out xyzIndex) == false)
                {
                    return true;
                }
                return false;
            }
            string xyzString = s.Substring(0, p);
            if (int.TryParse(xyzString, out xyzIndex) == false)
            {
                return true;
            }
            int p2 = s.IndexOf('/', p + 1);
            if (p2 < 0)
            {
                string stStr = s.Substring(p + 1);
                if (int.TryParse(stStr, out stIndex) == false)
                {
                    return true;
                }
                return false;
            }
            int stStringLen = p2 - p - 1;
            if (stStringLen > 0)
            {
                string stString = s.Substring(p + 1, stStringLen);
                if (int.TryParse(stString, out stIndex) == false)
                {
                    return true;
                }
            }
            string normString = s.Substring(p2+1);
            if (int.TryParse(normString, out nIndex) == false)
            {
                return true;
            }
            return false;
        }
    }
    struct ObjFace
    {
        private int firstVertex;
        private int numVerts;

        public void setFirstVertex(int i)
        {
            firstVertex = i;
        }
        public void setNumVerts(int i)
        {
            numVerts = i;
        }
        public int getNumVerts()
        {
            return numVerts;
        }
        public int getFirstVert()
        {
            return firstVertex;
        }
    }
    class ObjObject
    {
        private string objectName;
        private int firstFace;
        private int numFaces;

        public void setName(string s)
        {
            objectName = s;
        }
        public void setFirstFace(int i)
        {
            firstFace = i;
        }
        public void setNumFaces(int i)
        {
            numFaces = i;
        }
        public int getFirstFace()
        {
            return firstFace;
        }
        public int getNumFaces()
        {
            return numFaces;
        }
        public string getName()
        {
            return objectName;
        }
    }
    class ObjGroup : ObjObject
    {

    };
    class WavefrontOBJ : IVec3ArrayIterator
    {
        private List<Vec3> xyzs;
        private List<Vec2> texCoords;
        private List<Vec3> normals;
        private List<ObjFaceVertex> faceVerts;
        private List<ObjFace> faces;
        private List<ObjObject> objects;
        private List<ObjGroup> groups;
        private static readonly char[] lineSplitChars = { ' ' };

        public WavefrontOBJ()
        {
            xyzs = new List<Vec3>();
            texCoords = new List<Vec2>();
            normals = new List<Vec3>();
            faceVerts = new List<ObjFaceVertex>();
            faces = new List<ObjFace>();
            objects = new List<ObjObject>();
            groups = new List<ObjGroup>();
        }
        public override void addVec3Array(Vec3[] points, int numPoints)
        {
            ObjFace f = new ObjFace();
            f.setFirstVertex(faceVerts.Count);
            f.setNumVerts(numPoints);
            faces.Add(f);
            for (int i = 0; i < numPoints; i++)
            {
                faceVerts.Add(new ObjFaceVertex(xyzs.Count+i));
            }
            for (int i = 0; i < numPoints; i++)
            {
                xyzs.Add(points[i]);
            }
        }
        public void addIndexedMesh(Vec3[] mPoints, int numPoints, short numFaces, short[] fIndices, short[] fOffsets, short[] fCounts)
        {
            int firstXyz = xyzs.Count;
            for (int i = 0; i < numPoints; i++)
            {
                xyzs.Add(mPoints[i]);
            }
            for (int i = 0; i < numFaces; i++)
            {
                int numFaceVerts = fCounts[i];
                ObjFace f = new ObjFace();
                f.setFirstVertex(faceVerts.Count);
                f.setNumVerts(numFaceVerts);
                for (int j = 0; j < numFaceVerts; j++)
                {
                    int xyzIndex = firstXyz + fIndices[fOffsets[i] + j];
                    faceVerts.Add(new ObjFaceVertex(xyzIndex));
                }
                faces.Add(f);
            }
        }
        private void showOBJParseError(string msg)
        {
            MessageBox.Show(msg,
                "OBJ parser error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1
            );
        }
        public bool loadObjModel(string fileName)
        {

            StreamReader r = new StreamReader(fileName);
            string fileData = r.ReadToEnd();
            int lineNumber = 0;
            ObjObject lastObject = null;
            ObjGroup lastGroup = null;
            foreach (string line in fileData.Split('\n'))
            {
                lineNumber++;
                // skip comments
                int commentStart = line.IndexOf('#');
                string sl;
                if (commentStart >= 0)
                {
                    sl = line.Substring(0, commentStart);
                }
                else
                {
                    sl = line;
                }
                // remove extra whitespaces
                sl.Trim();
                // tokenize
                string[] tokens = sl.Split(lineSplitChars, StringSplitOptions.RemoveEmptyEntries);
                if (tokens == null || tokens.Length == 0)
                    continue;
                switch (tokens[0])
                {
                    case "mtllib":
                    {
                        if (tokens.Length < 2)
                        {
                            showOBJParseError("Missing materialName after 'mtllib' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            string mtlLibName = tokens[1];
                        }
                    }
                    break;
                    case "o":
                    {
                        if (tokens.Length < 2)
                        {
                            showOBJParseError("Missing objectName after 'o' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            if (lastObject != null)
                            {
                                lastObject.setNumFaces(faces.Count - lastObject.getFirstFace());
                            }
                            string objectName = tokens[1];
                            ObjObject no = new ObjObject();
                            no.setName(objectName);
                            no.setFirstFace(faces.Count);
                            objects.Add(no);
                            lastObject = no;
                        }
                    }
                    break;
                    case "g":
                    {
                        if (tokens.Length < 2)
                        {
                            showOBJParseError("Missing groupName after 'g' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            if (lastGroup != null)
                            {
                                lastGroup.setNumFaces(faces.Count - lastGroup.getFirstFace());
                            }
                            string groupName = tokens[1];
                            ObjGroup no = new ObjGroup();
                            no.setName(groupName);
                            no.setFirstFace(faces.Count);
                            groups.Add(no);
                            lastObject = no;
                        }
                    }
                    break;
                    case "v":
                    {
                        if (tokens.Length < 4)
                        {
                            showOBJParseError("Missing groupName after 'g' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            float x, y, z;
                            if (float.TryParse(tokens[1].Replace('.',','), out x) == false)
                            {
                                showOBJParseError("'X' component of 'v' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            if (float.TryParse(tokens[2].Replace('.', ','), out y) == false)
                            {
                                showOBJParseError("'Y' component of 'v' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            if (float.TryParse(tokens[3].Replace('.', ','), out z) == false)
                            {
                                showOBJParseError("'Z' component of 'v' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            xyzs.Add(new Vec3(x, y, z));
                        }
                    }
                    break;
                    case "vt":
                    {
                        if (tokens.Length < 3)
                        {
                            showOBJParseError("Missing texcoord values after 'vt' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            float x, y;
                            if (float.TryParse(tokens[1].Replace('.', ','), out x) == false)
                            {
                                showOBJParseError("'X' component of 'vt' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            if (float.TryParse(tokens[2].Replace('.', ','), out y) == false)
                            {
                                showOBJParseError("'Y' component of 'v' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            texCoords.Add(new Vec2(x, y));
                        }
                    }
                    break;
                    case "vn":
                    {
                        if (tokens.Length < 4)
                        {
                            showOBJParseError("Missing normal values after 'vn' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            float x, y, z;
                            if (float.TryParse(tokens[1].Replace('.', ','), out x) == false)
                            {
                                showOBJParseError("'X' component of 'vn' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            if (float.TryParse(tokens[2].Replace('.', ','), out y) == false)
                            {
                                showOBJParseError("'Y' component of 'vn' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            if (float.TryParse(tokens[3].Replace('.', ','), out z) == false)
                            {
                                showOBJParseError("'Z' component of 'vn' is not a valid float at line " + lineNumber);
                                return true;
                            }
                            normals.Add(new Vec3(x, y, z));
                        }
                    }
                    break;
                    case "f":
                    {
                        if (tokens.Length < 4)
                        {
                            showOBJParseError("Missing face vertex indices after 'f' token at line " + lineNumber);
                            return true;
                        }
                        else
                        {
                            int numFaceVerts = tokens.Length - 1;
                            ObjFace face = new ObjFace();
                            face.setFirstVertex(faceVerts.Count);
                            face.setNumVerts(numFaceVerts);
                            for (int i = 0; i < numFaceVerts; i++)
                            {
                                string faceVertexString = tokens[1 + i];
                                ObjFaceVertex faceVert = new ObjFaceVertex();
                                if (faceVert.setFromString(faceVertexString))
                                {
                                    showOBJParseError("Vertex string '"+faceVertexString+"' is not a valid at line " + lineNumber);
                                    return true;
                                }
                                faceVerts.Add(faceVert);
                            }
                            faces.Add(face);
                        }
                    }
                    break;
                    default:
                    {

                    }
                    break;
                }
            }
            if (lastObject != null)
            {
                lastObject.setNumFaces(faces.Count - lastObject.getFirstFace());
            }
            if (lastGroup != null)
            {
                lastGroup.setNumFaces(faces.Count - lastGroup.getFirstFace());
            }
           /* if (objects.Count == 0)
            {
                ObjObject obj = new ObjObject();
                obj.setName("default_object");
                obj.setFirstFace(0);
                obj.setNumFaces(faces.Count);
                objects.Add(obj);
            }
            if (groups.Count == 0)
            {
                ObjGroup obj = new ObjGroup();
                obj.setName("default_group");
                obj.setFirstFace(0);
                obj.setNumFaces(faces.Count);
                groups.Add(obj);
            }*/
            return false;
        }
        public bool saveObjModel(string fileName)
        {
            System.IO.StreamWriter file = new System.IO.StreamWriter(fileName);
            file.WriteLine("# obj file exported by objFileExplorer");
            for (int i = 0; i < xyzs.Count; i++)
            {
                file.WriteLine("v "+xyzs[i].ToString().Replace(',','.'));
            }
            for (int i = 0; i < texCoords.Count; i++)
            {
                file.WriteLine("vt " + texCoords[i].ToString().Replace(',', '.'));
            }
            for (int i = 0; i < normals.Count; i++)
            {
                file.WriteLine("vn " + normals[i].ToString().Replace(',', '.'));
            }
            for (int i = 0; i < objects.Count; i++)
            {
                ObjObject obj = objects[i];
                file.WriteLine("o " + obj.getName());
                for (int j = 0; j < obj.getNumFaces(); j++)
                {
                    ObjFace face = faces[obj.getFirstFace()+j];
                    file.Write("f");
                    for (int k = 0; k < face.getNumVerts(); k++)
                    {
                        int faceVertIndex = k + face.getFirstVert();
                        ObjFaceVertex faceVertex = faceVerts[faceVertIndex];
                        string faceVertString = " "+faceVertex.ToString();
                        file.Write(faceVertString);
                    }
                    file.WriteLine();
                }
            }
            for (int i = 0; i < groups.Count; i++)
            {
                ObjGroup g = groups[i];
                file.WriteLine("g " + g.getName());
                for (int j = 0; j < g.getNumFaces(); j++)
                {
                    ObjFace face = faces[g.getFirstFace() + j];
                    file.Write("f");
                    for (int k = 0; k < face.getNumVerts(); k++)
                    {
                        int faceVertIndex = k + face.getFirstVert();
                        ObjFaceVertex faceVertex = faceVerts[faceVertIndex];
                        string faceVertString = " " + faceVertex.ToString();
                        file.Write(faceVertString);
                    }
                    file.WriteLine();
                }
            }
            if (groups.Count == 0 && objects.Count == 0)
            {
                for (int j = 0; j < faces.Count; j++)
                {
                    ObjFace face = faces[j];
                    file.Write("f");
                    for (int k = 0; k < face.getNumVerts(); k++)
                    {
                        int faceVertIndex = k + face.getFirstVert();
                        ObjFaceVertex faceVertex = faceVerts[faceVertIndex];
                        string faceVertString = " " + faceVertex.ToString();
                        file.Write(faceVertString);
                    }
                    file.WriteLine();
                }
            }
            file.Close();
            return false;
        }
        public int getNumObjects()
        {
            return objects.Count;
        }
        public ObjObject getObject(int i)
        {
            return objects[i];
        }
        public int getNumGroups()
        {
            return groups.Count;
        }
        public ObjGroup getGroup(int i)
        {
            return groups[i];
        }
        public int getNumFaces()
        {
            return faces.Count;
        }
        public ObjFace getFace(int i)
        {
            return faces[i];
        }
        public int getNumXYZs()
        {
            return xyzs.Count;
        }
        public Vec3 getXYZ(int i)
        {
            return xyzs[i];
        }
        public int getNumNormals()
        {
            return normals.Count;
        }
        public Vec3 getNormal(int i)
        {
            return normals[i];
        }
        public int getNumTexCoords()
        {
            return texCoords.Count;
        }
        public Vec2 getTexCoord(int i)
        {
            return texCoords[i];
        }
        public int getObjectIndex(ObjObject o)
        {
            return objects.IndexOf(o);
        }
    }
}
