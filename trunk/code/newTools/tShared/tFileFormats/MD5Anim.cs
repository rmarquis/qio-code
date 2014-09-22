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
using md5animFileExplorer;

namespace fileFormats
{
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
        public bool saveMD5AnimFile(string fileName)
        {

            // TODO
            return true;
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
