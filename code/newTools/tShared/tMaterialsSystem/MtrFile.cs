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
// newTools/tShared/MaterialDef.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using tMath;

namespace shared
{
  
        class MtrFile
        {
            private string name;
            private Parser p;
            private List<MaterialDef> materials;

            public string getName()
            {
                return name;
            }
            public List<MaterialDef> getDefs()
            {
                return materials;
            }
            public string getMaterialDefText(string name)
            {
                MaterialDef md = findMaterialDef(name);
                if (md == null)
                    return null;
                string d = p.getData();
                return d.Substring(md.getStart(), md.getEnd() - md.getStart());
            }
            public string getRawText()
            {
                return p.getData();
            }
            public MaterialDef findMaterialDef(string name)
            {
                foreach (MaterialDef md in materials)
                {
                    if (name.Equals(md.getName()))
                    {
                        return md;
                    }
                }
                return null;
            }
            public void precacheMtrFile()
            {
                materials = new List<MaterialDef>();
                p = new Parser();
                p.beginParsingFile(name);
                while (p.isAtEOF() == false)
                {
                    if (p.isAtToken("table"))
                    {
                        string tabName;
                        p.readString(out tabName,"{");
                        if (p.isAtToken("{", false) == false)
                        {
                            MessageBox.Show("Material file parse error.",
                                "Expected '{' to follow table name " + tabName + " in mat file " + name + " at line " + p.getCurrentLineNumber(),
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                            return; // error
                        }
                        p.skipCurlyBracedBlock();
                    }
                    else
                    {
                        string matName;
                        p.skipWhiteSpaces();
                        int start = p.getPos();
                        p.readString(out matName, "{");
                        if (p.isAtToken("{", false) == false)
                        {
                            MessageBox.Show("Material file parse error!",
                                "Expected '{' to follow material name " + matName + " in mat file " + name + " at line " + p.getCurrentLineNumber(),
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Exclamation,
                            MessageBoxDefaultButton.Button1);
                            return; // error
                        }
                        p.skipCurlyBracedBlock();
                        int end = p.getPos();
                        materials.Add(new MaterialDef(matName, start, end));
                    }
                }
            }
            static int STR_SkipOf(string s, int at, string skip)
            {
                while(at < s.Length)
                {
                    char ch = s[at];
                    if (skip.IndexOf(ch) == -1)
                        return at;
                    at++;
                }
                return at;
            }
            private int autoGenerateQerEditorImageLineForMaterial(String matName, int start, int end)
            {
                // get material text
                String mtrText = p.getData().Substring(start, end - start);
                // check is there already a qer_editorImage
                if(mtrText.IndexOf("qer_editorImage", StringComparison.InvariantCultureIgnoreCase) != -1)
                {
                    return 0;
                }
                int br = mtrText.IndexOf('{');
                if(br == -1)
                {
                    MessageBox.Show("Material parse error","Unexpected error - '{' not found in material text",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                    return 0;
                }
                br++; // skip '{'
                int at = STR_SkipOf(mtrText,br," \t");
                at = STR_SkipOf(mtrText, at, "\n\r");
                //string editorImageName = matName;
                string editorImageName = findDiffuseMapValue(mtrText);
                if(editorImageName == null)
                {
                    editorImageName = matName;
                }
                else
                {
                    int d = editorImageName.LastIndexOf('.');
                    if(d != -1)
                    {
                        editorImageName = editorImageName.Substring(0, d);
                    }
                }
                string textToInsert = "\tqer_editorImage " + editorImageName + "\r\n";
                int insertedLen = textToInsert.Length;
                p.setData(p.getData().Insert(start+at, textToInsert));
                return insertedLen;
            }
            public static string findDiffuseMapValue(String materialText)
            {
                string s;
                Parser p = new Parser();
                p.beginParsingText(materialText);
                while(p.isAtEOF()== false)
                {
                    if(p.isAtToken("diffusemap") || p.isAtToken("colormap"))
                    {
                        p.readToken(out s);
                        return s;
                    }
                    else
                    {
                        p.readToken(out s);
                    }
                }
                return null;
            }
            public void autoGenerateQerEditorImageLines()
            {
                int changedMaterialDefs = 0;
                p = new Parser();
                p.beginParsingFile(name);
                while (p.isAtEOF() == false)
                {
                    if (p.isAtToken("table"))
                    {
                        string tabName;
                        p.readString(out tabName, "{");
                        if (p.isAtToken("{", false) == false)
                        {
                            MessageBox.Show("Material file parse error.","Expected '{' to follow table name " + tabName + " in mat file " + name,
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Exclamation,
                            MessageBoxDefaultButton.Button1);
                            return; // error
                        }
                        p.skipCurlyBracedBlock();
                    }
                    else
                    {
                        string matName;
                        p.skipWhiteSpaces();
                        int start = p.getPos();
                        p.readString(out matName, "{");
                        if (p.isAtToken("{", false) == false)
                        {
                            MessageBox.Show("Material file parse error", "Expected '{' to follow material name " + matName + " in mat file " + name,
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                            return; // error
                        }

                        p.skipCurlyBracedBlock();
                        int end = p.getPos();
                        int insertedLen = autoGenerateQerEditorImageLineForMaterial(matName, start, end);
                        if(insertedLen != 0)
                        {
                            changedMaterialDefs++;
                        }
                        // after generating end offset might have changed
                        p.setPos(start);
                        p.readString(out matName, "{");
                        if (p.isAtToken("{", false) == false)
                        {
                            MessageBox.Show("Material file parse error.","Expected '{' to follow material name " + matName + " in mat file " + name,
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
                            return; // error
                        }
                        p.skipCurlyBracedBlock();
                    }
                }
                // reload all material definitions
                precacheMtrFile();
                MessageBox.Show("Autogeneration done.",
                    "Updated " + changedMaterialDefs + " material definitions out of " + materials.Count + " total.",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
            }
            public MtrFile(String fname)
            {
                this.name = fname;
                precacheMtrFile();
            }
        }
}
