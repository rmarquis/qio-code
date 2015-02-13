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
// newTools/tShared/tFileFormats/MD5Anim.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using tMath;
using shared;
using System.Globalization;

namespace fileFormats
{
    struct BoneOrientation
    {
        private Quat q;
        private Vec3 v;

        public Quat getQuat()
        {
            return q;
        }
        public Vec3 getPos()
        {
            return v;
        }
        public void setQuat(Quat nq)
        {
            q = nq;
        }
        public void setPos(Vec3 nv)
        {
            v = nv;
        }
    }
    class BoneOrientations : IBoneOrientations
    {
        private BoneOrientation [] ors;

        public BoneOrientations()
        {
        }
        public BoneOrientations(int c)
        {
            this.allocBones(c);
        }
        public Quat getBoneQuat(int boneIndex)
        {
            return ors[boneIndex].getQuat();
        }
        public Vec3 getBonePos(int boneIndex)
        {
            return ors[boneIndex].getPos();
        }
        public void setBoneQuat(int boneIndex, Quat q)
        {
            ors[boneIndex].setQuat(q);
        }
        public void setBonePos(int boneIndex, Vec3 v)
        {
            ors[boneIndex].setPos(v);
        }
        public void allocBones(int newCount)
        {
            ors = new BoneOrientation[newCount];
        }
    }
    class MD5AnimFrame
    {
        private List<double> values;

        public MD5AnimFrame()
        {
            values = new List<double>();
        }
        public void addValue(double d)
        {
            values.Add(d);
        }
        public double getValue(int i)
        {
            return values[i];
        }
    };
    class MD5AnimJoint
    {
        private string name;
        private int componentFlags;
        private int firstComponent;
        private int parentIndex;
        // these are read from "baseframe" section
        private Vec3 localOfs;
        private Quat localQuat;

        public MD5AnimJoint(string name, int parentIndex, int componentFlags, int firstComponent)
        {
            this.name = name;
            this.parentIndex = parentIndex;
            this.componentFlags = componentFlags;
            this.firstComponent = firstComponent;
        }
        public override string ToString()
        {
            return "\"" + name + "\"\t" + parentIndex.ToString() + " " + firstComponent.ToString() + " " + componentFlags.ToString();
        }
        public void setBaseFrameOfs(Vec3 ofs)
        {
            localOfs = ofs;
        }
        public void setBaseFrameQuat(Vec3 q)
        {
            localQuat.setXYZAndCalculateW(q);
        }
        public Quat getLocalRot()
        {
            return localQuat;
        }
        public Vec3 getLocalOfs()
        {
            return localOfs;
        }
        public string getName()
        {
            return this.name;
        }
        public int getParentIndex()
        {
            return parentIndex;
        }
        public int getFirstComponent()
        {
            return firstComponent;
        }
        public int getComponentFlags()
        {
            return componentFlags;
        }
    };
    class MD5Anim
    {
        private string fileName;
        private string commandLine;
        private int frameRate;   
        private int numAnimatedComponents;
        private List<MD5AnimJoint> animJoints;
        // frames.Count == bounds.Count
        private List<MD5AnimFrame> frames;
        private List<AABB> bounds;

        public string getName()
        {
            return fileName;
        }
        public string getCommandLine()
        {
            return commandLine;
        }
        public int getFrameCount()
        {
            return frames.Count;
        }
        public int getJointCount()
        {
            return animJoints.Count;
        }
        public int getFrameRate()
        {
            return frameRate;
        }
        public int getAnimatedComponentsCount()
        {
            return numAnimatedComponents;
        }
        public MD5AnimFrame getFrame(int index)
        {
            return frames[index];
        }
        public MD5AnimJoint getJoint(int index)
        {
            return animJoints[index];
        }
        public AABB getBounds(int index)
        {
            return bounds[index];
        }
        public bool buildFrameLocalBone(int frameIndex, int jointIndex, out Quat q, out Vec3 v)
        {
            MD5AnimFrame f = frames[frameIndex];
            MD5AnimJoint j = animJoints[jointIndex];
            v = j.getLocalOfs();
            q = j.getLocalRot();
            int flags = j.getComponentFlags();
            int componentOfs = j.getFirstComponent();
            if ((flags & 1) != 0)
            { // animated X component
                v.setX(f.getValue(componentOfs));
                componentOfs++;
            }
            if ((flags & 2) != 0)
            { // animated Y component
                v.setY(f.getValue(componentOfs));
                componentOfs++;
            }
            if ((flags & 4) != 0)
            { // animated Z component
                v.setZ(f.getValue(componentOfs));
                componentOfs++;
            }
            if ((flags & 8) != 0)
            { // animated QUAT X component
                q.setX(f.getValue(componentOfs));
                componentOfs++;
            }
            if ((flags & 16) != 0)
            { // animated QUAT Y component
                q.setY(f.getValue(componentOfs));
                componentOfs++;
            }
            if ((flags & 32) != 0)
            { // animated QUAT Z component
                q.setZ(f.getValue(componentOfs));
                componentOfs++;
            }
            q.calculateW();
            return false;
        }
        public bool buildFrameLocalBones(int frameIndex, BoneOrientations bones)
        {
            for (int i = 0; i < animJoints.Count; i++)
            {
                Quat q;
                Vec3 v;
                buildFrameLocalBone(frameIndex, i, out q, out v);
                bones.setBonePos(i, v);
                bones.setBoneQuat(i, q);
            }
            return false;
        }
        public bool buildFrameABSBones(int frameIndex, BoneOrientations bones)
        {
            if (buildFrameLocalBones(frameIndex, bones))
            {
                return true;
            }
            for (int i = 0; i < animJoints.Count; i++)
            {
                int parent = animJoints[i].getParentIndex();
                if (parent != -1)
                {
                    Vec3 rotatedPos = bones.getBoneQuat(parent).rotatePoint(bones.getBonePos(i));
                    Vec3 newPos = rotatedPos + bones.getBonePos(parent);
                    Quat newRot = bones.getBoneQuat(parent).multiplyQuat(bones.getBoneQuat(i));
                    newRot.normalize();
                    bones.setBonePos(i,newPos);
                    bones.setBoneQuat(i,newRot);
                }
            }
            return false;
        }
        public bool saveMD5AnimFile(string fileName)
        {
            // get md5anim text
            string md5animFileText = this.ToString();
            // write to file
            System.IO.StreamWriter file = new System.IO.StreamWriter(fileName);
            file.WriteLine(md5animFileText);
            file.Close();
            return false;
        }
        public string getJointName(int index)
        {
            if (index < 0)
                return "";
            return animJoints[index].getName();
        }
        public override string ToString()
        {
            string r = "MD5Version 10\n";
            r += "commandline \""+commandLine+"\"\n";
            r += "\n";
            r += "numFrames "+frames.Count.ToString()+"\n";
            r += "numJoints "+animJoints.Count.ToString()+"\n";
            r += "frameRate "+frameRate+"\n";
            r += "numAnimatedComponents " +numAnimatedComponents.ToString() +"\n";
            r += "\n";
            // write skeleton
            r += "hierarchy {\n";
            foreach(MD5AnimJoint j in animJoints)
            {
                r += "\t" + j.ToString() + "\t// " + getJointName(j.getParentIndex()).ToString() + "\n";
            }
            r += "}\n\n";
            // write frame bounds
            r += "bounds {\n";
            foreach (AABB bb in bounds)
            {
                r += "\t" + bb.ToString() + "\n";
            }
            r += "}\n\n";
            // write base frame
            r += "baseframe {\n";
            foreach (MD5AnimJoint j in animJoints)
            {
                r += "\t" + j.getLocalOfs().ToStringBraced() + " " + j.getLocalRot().ToStringBracedXYZ() + "\n";
            }
            r += "}\n\n";
            for (int i = 0; i < frames.Count; i++)
            {
                r += "frame " + i.ToString() + " {\n\t";
                int j;
                for (j = 0; j < numAnimatedComponents; j++)
                {
                    r += frames[i].getValue(j).ToString(CultureInfo.InvariantCulture.NumberFormat);
                    if ((j + 1) % 6 == 0)
                    {
                        r += "\n\t";
                    }
                    else
                    {
                        r += " ";
                    }
                }
                if ((j + 1) % 6 != 0)
                {
                    r += "\n";
                }
                r += "}\n";
            }
            return r;
        }
        private bool parseFrame(Parser p)
        {
            int frameIndex;
            if (p.readInt(out frameIndex))
            {

                return true;
            }
            if (frameIndex != frames.Count)
            {

                return true;
            }
            if (p.isAtToken("{") == false)
            {
                return true;
            }
            MD5AnimFrame f = new MD5AnimFrame();
            for (int i = 0; i < numAnimatedComponents; i++)
            {
                float val;
                if (p.readFloat(out val))
                {
                    return true;
                }
                f.addValue(val);
            }
            if (p.isAtToken("}") == false)
            {
                return true;
            }
            frames.Add(f);
            return false;
        }
        private bool parseBounds(Parser p, int expectedCount)
        {
            if (p.isAtToken("{") == false)
            {
                return true;
            }
            for (int i = 0; i < expectedCount; i++)
            {
                Vec3 a, b;
                if (p.readBracedVec3(out a))
                {
                    return true;
                }
                if (p.readBracedVec3(out b))
                {
                    return true;
                }
                bounds.Add(new AABB(a, b));
            }
            if (p.isAtToken("}") == false)
            {
                return true;
            }
            return false;
        }
        private bool parseBaseFrame(Parser p, int expectedCount)
        {
            if (p.isAtToken("{") == false)
            {
                return true;
            }
            for (int i = 0; i < expectedCount; i++)
            {
                Vec3 ofs, quat;
                if (p.readBracedVec3(out ofs))
                {
                    return true;
                }
                if (p.readBracedVec3(out quat))
                {
                    return true;
                }
                animJoints[i].setBaseFrameOfs(ofs);
                animJoints[i].setBaseFrameQuat(quat);
            }
            if (p.isAtToken("}") == false)
            {
                return true;
            }
            return false;
        }
        private bool parseHierarchy(Parser p, int expectedCount)
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
                int componentBits;
                if (p.readInt(out componentBits))
                {

                    return true;
                }
                int firstComponent;
                if (p.readInt(out firstComponent))
                {

                    return true;
                }
                animJoints.Add(new MD5AnimJoint(name, parentIndex, componentBits, firstComponent));
            }
            if (p.isAtToken("}") == false)
            {
                return true;
            }
            return false;
        }
        public bool loadMD5AnimFile(string fileName)
        {
            this.fileName = fileName;
            this.animJoints = new List<MD5AnimJoint>();
            this.frames = new List<MD5AnimFrame>();
            this.bounds = new List<AABB>();

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
            int numFrames = -1;
            int numJoints = -1;
            numAnimatedComponents = -1;
            while (p.isAtEOF() == false)
            {
                if (p.isAtToken("commandLine"))
               {
                    p.readQuotedString(out commandLine);
                }
                else if (p.isAtToken("numFrames"))
                {
                    if (p.readInt(out numFrames))
                    {
                        MessageBox.Show("Failed to parse 'numFrames' value of md5anim file " + fileName + ".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else if (p.isAtToken("numJoints"))
                {
                    if (p.readInt(out numJoints))
                    {
                        MessageBox.Show("Failed to parse 'numJoints' value of md5anim file " + fileName + ".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else if (p.isAtToken("numAnimatedComponents"))
                {
                    p.readInt(out numAnimatedComponents);
                }
                else if (p.isAtToken("frameRate"))
                {
                    p.readInt(out frameRate);
                }
                else if (p.isAtToken("hierarchy"))
                {
                    // "numJoints" should be already set
                    if (numJoints == -1)
                    {

                        return true;
                    }
                    if (parseHierarchy(p, numJoints))
                    {
                        MessageBox.Show("Failed to parse 'hierarhy' section of md5anim file " + fileName + ".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else if (p.isAtToken("frame"))
                {
                    // "numMeshes" should be already set
                    if (numFrames == -1)
                    {

                        return true;
                    }
                    if (parseFrame(p))
                    {
                        MessageBox.Show("Failed to parse 'frame "+frames.Count+"' section of md5anim file " + fileName + ".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else if (p.isAtToken("baseframe"))
                {
                    if (parseBaseFrame(p, numJoints))
                    {
                        MessageBox.Show("Failed to parse 'baseframe' section of md5anim file "+fileName+".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else if (p.isAtToken("bounds"))
                {
                    if (parseBounds(p,numFrames))
                    {
                        MessageBox.Show("Failed to parse 'bounds' section of md5anim file " + fileName + ".",
                        "Parse error.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                        return true;
                    }
                }
                else
                {
                    string s;
                    p.readString(out s);
                    MessageBox.Show("Unknown token '" +s+"' in file "+ fileName + ".",
                    "Parse error.",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Exclamation,
                    MessageBoxDefaultButton.Button1);
                    return true;
                }
            }
            return false;
        }
    }
}
