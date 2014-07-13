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
// newTools/tShared/tFileFormats/MapFile.cs
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

    class KeyValue
    {
        private string key, value;

        public KeyValue(string nk, string nv)
        {
            key = nk;
            value = nv;
        }
        public void setValue(string s)
        {
            value = s;
        }
        public string getKey()
        {
            return key;
        }
        public string getValue()
        {
            return value;
        }
        public override string ToString()
        {
            return "\"" + key + "\" \"" + value + "\"\n";
        }
    }
    class KeyValuesList
    {
        private List<KeyValue> data;

        public KeyValuesList()
        {
            data = new List<KeyValue>();
        }
        public int findKeyIndex(string key)
        {
            for (int i = 0; i < data.Count; i++)
            {
                if (string.Compare(data[i].getKey(), key, true) == 0)
                    return i;
            }
            return -1;
        }
        public void setKeyValue(string key, string value)
        {
            int index = findKeyIndex(key);
            if (index >= 0)
            {
                data[index].setValue(value);
            }
            else
            {
                data.Add(new KeyValue(key, value));
            }
        }
        public string getKeyValue(string key)
        {
            int index = findKeyIndex(key);
            if (index >= 0)
            {
                return data[index].getValue();
            }
            return "";
        }
        public KeyValue getKeyValue(int i)
        {
            return data[i];
        }
        public override string ToString()
        {
            string r = "";
            for (int i = 0; i < data.Count; i++)
            {
                r += data[i].ToString();
            }
            return r;
        }
        public int size()
        {
            return data.Count;
        }
    }
    struct MapPatchVertex
    {
        private Vec3 xyz;
        private Vec2 st;

        public void setPos(Vec3 np)
        {
            xyz = np;
        }
        public void setTexCoords(Vec2 nt)
        {
            st = nt;
        }
        public override string ToString()
        {
            return xyz.ToString() + " " + st.ToString();
        }
    }
    class MapPatch
    {
        private string matName;
        private int w, h;
        private MapPatchVertex[] verts;

        private int calcPointIndex(int i, int j)
        {
            return i * h + j;
        }
        public void setVertex(int i, int j, Vec3 position, Vec2 texCoords)
        {
            int pointIndex = calcPointIndex(i, j);
            verts[pointIndex].setPos(position);
            verts[pointIndex].setTexCoords(texCoords);
        }
        public MapPatchVertex getVertex(int i, int j)
        {
            int pointIndex = calcPointIndex(i, j);
            return verts[pointIndex];
        }
        public int getSizeW()
        {
            return w;
        }
        public int getSizeH()
        {
            return h;
        }
        public void setSizes(int iW, int iH)
        {
            w = iW;
            h = iH;
            verts = new MapPatchVertex[w * h];
        }
        public void setMatName(string newMatName)
        {
            matName = newMatName;
        }
        public string getMatName()
        {
            return matName;
        }

    };
    class MapBrushSideOld
    {
        private string matName;
        private Vec3 a, b, c;
        private Vec2 texShift, texScale;
        private float texRotation;
        private int contentFlags;

        public MapBrushSideOld()
        {
        }
        public void setPlanePoints(Vec3 nA, Vec3 nB, Vec3 nC)
        {
            a = nA;
            b = nB;
            c = nC;
        }
        public void setMatName(string newMatName)
        {
            matName = newMatName;
        }
        public string getMatName()
        {
            return matName;
        }
        public void setTexShift(Vec2 ns)
        {
            texShift = ns;
        }
        public void setTexScale(Vec2 ns)
        {
            texScale = ns;
        }
        public void setTexRotation(float nr)
        {
            texRotation = nr;
        }
        public void setContentFlags(int nc)
        {
            contentFlags = nc;
        }
        public Vec2 getTexShift()
        {
            return texShift;
        }
        public Vec2 getTexScale()
        {
            return texScale;
        }
        public float getTexRotation()
        {
            return texRotation;
        }
        public Vec3 getPlanePointA()
        {
            return a;
        }
        public Vec3 getPlanePointB()
        {
            return b;
        }
        public Vec3 getPlanePointC()
        {
            return c;
        }
        public int getContentFlags()
        {
            return contentFlags;
        }
        public override string ToString()
        {
            string r = a.ToStringBraced() + " " + b.ToStringBraced() + " " + c.ToStringBraced();
            r += " ";
            r += matName;
            r += " ";
            r += texShift.ToString();
            r += " ";
            r += texRotation.ToString(System.Globalization.CultureInfo.InvariantCulture);
            r += " ";
            r += texScale.ToString();
            r += " ";
            r += contentFlags;
            r += " 0";
            r += " 0";
            r += "\n";
            return r;
        }
    }
    abstract class MapBrushBase
    {
        public abstract bool isOldBrush();

    }
    class MapBrushOld : MapBrushBase
    {
        private List<MapBrushSideOld> sides;

        public MapBrushOld()
        {
            sides = new List<MapBrushSideOld>();
        }
        public override bool isOldBrush()
        {
            return true;
        }
        public void addSide(MapBrushSideOld ns)
        {
            sides.Add(ns);
        }
        public MapBrushSideOld getSide(int i)
        {
            return sides[i];
        }
        public int getNumSides()
        {
            return sides.Count;
        }
        public override string ToString()
        {
            string r = "{\n";
            for (int i = 0; i < sides.Count; i++)
            {
                r += sides[i].ToString();
            }
            r += "}\n";
            return r;
        }
    }
    class MapBrushSide4
    {
        private string matName;
        private Plane plane;
        private Vec3 texAxis0;
        private Vec3 texAxis1;

        public void setMatName(string newMatName)
        {
            matName = newMatName;
        }
        public string getMatName()
        {
            return matName;
        }
        public void setPlane(Plane newPlane)
        {
            plane = newPlane;
        }
        public Plane getPlane()
        {
            return plane;
        }
        public void setTextureMatrix(Vec3 newTexAxis0, Vec3 newTexAxis1)
        {
            texAxis0 = newTexAxis0;
            texAxis1 = newTexAxis1;
        }
        public Vec3 getTextureAxis0()
        {
            return texAxis0;
        }
        public Vec3 getTextureAxis1()
        {
            return texAxis1;
        }
        public override string ToString()
        {
            string r = "   " + plane.ToStringBraced() +
                " ( " + texAxis0.ToStringBraced()
                + texAxis1.ToStringBraced() + " ) \"" + matName + "\" " + "0 0 0\n";
            return r;
        }
    }
    class MapBrushDef3 : MapBrushBase
    {
        private List<MapBrushSide4> sides;

        public MapBrushDef3()
        {
            sides = new List<MapBrushSide4>();
        }
        public override bool isOldBrush()
        {
            return false;
        }
        public void addSide(MapBrushSide4 ns)
        {
            sides.Add(ns);
        }
        public MapBrushSide4 getSide(int i)
        {
            return sides[i];
        }
        public int getNumSides()
        {
            return sides.Count;
        }
        public override string ToString()
        {
            string r = "{\n brushDef3\n {\n";
            for (int i = 0; i < sides.Count; i++)
            {
                r += sides[i].ToString();
            }
            r += " }\n}\n";
            return r;
        }
    }
    class MapEntity
    {
        private KeyValuesList keyValues;
        private List<MapBrushBase> brushes;
        private List<MapPatch> patches;

        public MapEntity()
        {
            keyValues = new KeyValuesList();
            brushes = new List<MapBrushBase>();
            patches = new List<MapPatch>();
        }
        public void setKeyValue(string key, string value)
        {
            keyValues.setKeyValue(key, value);
        }
        public KeyValuesList getKeyValues()
        {
            return keyValues;
        }
        public string getClassName()
        {
            return keyValues.getKeyValue("classname");
        }
        public void addBrush(MapBrushBase b)
        {
            brushes.Add(b);
        }
        public MapBrushBase getBrush(int i)
        {
            return brushes[i];
        }
        public int getNumBrushes()
        {
            return brushes.Count;
        }
        public void addPatch(MapPatch b)
        {
            patches.Add(b);
        }
        public MapPatch getPatch(int i)
        {
            return patches[i];
        }
        public int getNumPatches()
        {
            return patches.Count;
        }
        public override string ToString()
        {
            string r = "{\n";
            r += keyValues.ToString();
            for (int i = 0; i < brushes.Count; i++)
            {
                r += "// brush " + i + "\n";
                r += brushes[i].ToString();
            }
            r += "}\n";
            return r;
        }
    }
    enum EMapVersion
    {
        EMV_OLD, // Quake3, etc
        EMV_VERSION2, // "Version 2" ident, Doom3
        EMV_VERSION3, // "Version 3" ident, Quake4
    };
    class MapFile
    {
        private string fileName;
        private EMapVersion version;
        private List<MapEntity> entities;

        public MapFile()
        {
            entities = new List<MapEntity>();
        }
        public void setVersion(EMapVersion nv)
        {
            version = nv;
        }
        public void setFileName(string newFileName)
        {
            fileName = newFileName;
        }
        public void addEntity(MapEntity ne)
        {
            entities.Add(ne);
        }
        public MapEntity getEntity(int i)
        {
            return entities[i];
        }
        public int getNumEntities()
        {
            return entities.Count;
        }
        public override string ToString()
        {
            string r = "";
            if (version == EMapVersion.EMV_VERSION2)
            {
                r += "Version 2\n";
            }
            else if (version == EMapVersion.EMV_VERSION3)
            {
                r += "Version 3\n";
            }
            for (int i = 0; i < entities.Count; i++)
            {
                r += "// entity " + i + "\n";
                r += entities[i].ToString();
            }
            return r;
        }
    }
    class MapFileParser
    {
        private Parser p;
        private MapFile m;

        private void showParseError(string msg)
        {
            MessageBox.Show(msg,
                "Map parser error",
                MessageBoxButtons.OK,
                MessageBoxIcon.Exclamation,
                MessageBoxDefaultButton.Button1
            );
        }
        private bool parseMapBrush(MapEntity e)
        {
            MapBrushOld br = new MapBrushOld();
            while (p.isAtToken("}") == false)
            {
                Vec3 a, b, c;
                if (p.readBracedVec3(out a))
                {
                    showParseError("Failed to read point A of plane equation of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                if (p.readBracedVec3(out b))
                {
                    showParseError("Failed to read point B of plane equation of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                if (p.readBracedVec3(out c))
                {
                    showParseError("Failed to read point C of plane equation of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                string matName;
                if (p.readToken(out matName))
                {
                    showParseError("Failed to read material name of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Vec2 texShift;
                if (p.readVec2(out texShift))
                {
                    showParseError("Failed to texture shift of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                float texRotation;
                if (p.readFloat(out texRotation))
                {
                    showParseError("Failed to texture rotation of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Vec2 texScale;
                if (p.readVec2(out texScale))
                {
                    showParseError("Failed to texture scale of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                // used for detail flag, this is 0 for structural brushes
                int contents;
                if (p.readInt(out contents))
                {
                    showParseError("Failed to contentFlags of brushside " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                // now texDef->flags and texDef->value
                p.skipToNextLine();
                MapBrushSideOld bs = new MapBrushSideOld();
                bs.setPlanePoints(a, b, c);
                bs.setMatName(matName);
                bs.setTexShift(texShift);
                bs.setTexScale(texScale);
                bs.setTexRotation(texRotation);
                bs.setContentFlags(contents);
                br.addSide(bs);
            }
            e.addBrush(br);
            return false;
        }
        private bool parseMapPatchDef(MapEntity e, bool bPatchDef3)
        {
            if (!p.isAtToken("{"))
            {
                showParseError("Expected '{' at the beginning of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            // patch info
            string matName;
            if (p.readToken(out matName))
            {
                showParseError("Failed to read material name of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            int w, h;
            if (!p.isAtToken("("))
            {
                showParseError("Expected '(' at the beginning of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            if (p.readInt(out w))
            {
                showParseError("Failed to read width of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            if (p.readInt(out h))
            {
                showParseError("Failed to read height of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            int a, b, c;
            if (p.readInt(out a))
            {
                showParseError("Failed to read third parm of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            if (p.readInt(out b))
            {
                showParseError("Failed to read 4th parm of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            if (p.readInt(out c))
            {
                showParseError("Failed to read 5th parm of patchDef2 at line " + p.getCurrentLineNumber());
                return true;
            }
            if (bPatchDef3)
            {
                int g, f;
                if (p.readInt(out f))
                {
                    showParseError("Failed to read 6th parm of patchDef2 at line " + p.getCurrentLineNumber());
                    return true;
                }
                if (p.readInt(out g))
                {
                    showParseError("Failed to read 7th parm of patchDef2 at line " + p.getCurrentLineNumber());
                    return true;
                }
            }
            while (true)
            {
                if (p.isAtToken("+surfaceParm"))
                {
                    string parmName;
                    p.readString(out parmName);
                }
                else
                {
                    break;
                }
            }
            if (!p.isAtToken(")"))
            {
                showParseError("Expected ')' after patchDef2 info block at line " + p.getCurrentLineNumber());
                return true;
            }
            if (!p.isAtToken("("))
            {
                showParseError("Expected '(' at the beginning of patchDef2 vertices block at line " + p.getCurrentLineNumber());
                return true;
            }
            MapPatch patch = new MapPatch();
            patch.setSizes(w, h);
            patch.setMatName(matName);
            for (int i = 0; i < w; i++)
            {
                if (!p.isAtToken("("))
                {
                    showParseError("Expected '(' at the beginning of patchDef2 vertices row " + i + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                for (int j = 0; j < h; j++)
                {
                    if (!p.isAtToken("("))
                    {
                        showParseError("Expected '(' at the beginning of patchDef2 vertex " + i + "/" + j + " at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    Vec3 position;
                    if (p.readVec3(out position))
                    {
                        showParseError("Failed to read position of patchDef2 vertex " + i + "/" + j + " at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    Vec2 texCoords;
                    if (p.readVec2(out texCoords))
                    {
                        showParseError("Failed to read texCoord of patchDef2 vertex " + i + "/" + j + " at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    if (!p.isAtToken(")"))
                    {
                        showParseError("Expected ')' at the end of patchDef2 vertex " + i + "/" + j + " at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    patch.setVertex(i, j, position, texCoords);
                }
                if (!p.isAtToken(")"))
                {
                    showParseError("Expected ')' at the end of patchDef2 vertices row " + i + " at line " + p.getCurrentLineNumber());
                    return true;
                }
            }
            if (!p.isAtToken(")"))
            {
                showParseError("Expected ')' after patchDef2 vertices block at line " + p.getCurrentLineNumber());
                return true;
            }
            if (!p.isAtToken("}"))
            {
                showParseError("Expected '}' at the end of patchDef2 block at line " + p.getCurrentLineNumber());
                return true;
            }
            if (!p.isAtToken("}"))
            {
                showParseError("Expected '}' at the end of patchDef2 block at line " + p.getCurrentLineNumber());
                return true;
            }
            e.addPatch(patch);
            return false;
        }
        private bool parseMapBrushDef3(MapEntity e)
        {
            if (!p.isAtToken("{"))
            {
                showParseError("Expected '{' at the beginning of brushDef3 at line " + p.getCurrentLineNumber());
                return true;
            }
            MapBrushDef3 br = new MapBrushDef3();
            while (p.isAtToken("}") == false)
            {
                if (!p.isAtToken("("))
                {
                    showParseError("Expected '(' at the beginning of brushDef3 plane equation " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Vec3 normal;
                if (p.readVec3(out normal))
                {
                    showParseError("Failed to read brushDef3 plane equation normal of side " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                float distance;
                if (p.readFloat(out distance))
                {
                    showParseError("Failed to read brushDef3 plane equation distance of side " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Plane pl = new Plane();
                pl.setDistance(distance);
                pl.setNormal(normal);
                if (!p.isAtToken(")"))
                {
                    showParseError("Expected ')' after the brushDef3 plane equation " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                if (!p.isAtToken("("))
                {
                    showParseError("Expected '(' at the beginning of brushDef3 texture matrix " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Vec3 texAxis0;
                if (p.readBracedVec3(out texAxis0))
                {
                    showParseError("Failed to read first vec3 of brushDef3 texture matrix " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                Vec3 texAxis1;
                if (p.readBracedVec3(out texAxis1))
                {
                    showParseError("Failed to read second vec3 of brushDef3 texture matrix " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                if (!p.isAtToken(")"))
                {
                    showParseError("Expected ')' after the brushDef3 texture matrix " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                string matName;
                if (p.readToken(out matName))
                {
                    showParseError("Failed to read material name of brushDef3 side " + br.getNumSides() + " at line " + p.getCurrentLineNumber());
                    return true;
                }
                MapBrushSide4 bs = new MapBrushSide4();
                bs.setMatName(matName);
                bs.setPlane(pl);
                bs.setTextureMatrix(texAxis0, texAxis1);
                br.addSide(bs);
                p.skipToNextLine();
            }
            if (!p.isAtToken("}"))
            {
                showParseError("Expected '}' at the end of brushDef3 at line " + p.getCurrentLineNumber());
                return true;
            }
            e.addBrush(br);
            return false;
        }
        private bool parseMapEntity()
        {
            MapEntity e = new MapEntity();
            while (p.isAtToken("}") == false)
            {
                if (p.isAtEOF())
                { //should never happen                
                    showParseError("Unexpected EOF hit while parsing entity " + m.getNumEntities());
                    break;
                }
                else if (p.isAtToken("{"))
                {
                    if (p.isAtToken("patchDef2"))
                    {
#if false
                        if (!p.isAtToken("{"))
                        { 
                            return true;
                        }
                        p.skipCurlyBracedBlock();
                        if (!p.isAtToken("}"))
                        {
                            return true;
                        }
#else
                        if (parseMapPatchDef(e, false))
                        {
                            return true;
                        }
#endif
                    }
                    else if (p.isAtToken("brushDef3"))
                    {
#if false
                        if (!p.isAtToken("{"))
                        { 
                            return true;
                        }
                        p.skipCurlyBracedBlock();
                        if (!p.isAtToken("}"))
                        {
                            return true;
                        }
#else
                        if (parseMapBrushDef3(e))
                        {
                            return true;
                        }
#endif
                    }
                    else if (p.isAtToken("patchDef3"))
                    {
#if false
                        if (!p.isAtToken("{"))
                        { 
                            return true;
                        }
                        p.skipCurlyBracedBlock();
                        if (!p.isAtToken("}"))
                        {
                            return true;
                        }
#else
                        if (parseMapPatchDef(e, true))
                        {
                            return true;
                        }
#endif
                    }
                    else if (p.isAtToken("terrainDef"))
                    {
                        if (!p.isAtToken("{"))
                        {
                            return true;
                        }
                        p.skipCurlyBracedBlock();
                        if (!p.isAtToken("}"))
                        {
                            return true;
                        }
                    }
                    else
                    {
#if false
                        p.skipCurlyBracedBlock();
#else
                        if (parseMapBrush(e))
                        {
                            return true;
                        }
#endif
                    }
                }
                else
                {
                    string key, value;
                    if (p.readQuotedString(out key))
                    {
                        showParseError("Failed to read entity " + m.getNumEntities() + " epair key at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    if (p.readQuotedString(out value))
                    {
                        showParseError("Failed to read entity " + m.getNumEntities() + " epair value at line " + p.getCurrentLineNumber());
                        return true;
                    }
                    e.setKeyValue(key, value);
                }
            }
            m.addEntity(e);
            return false;
        }
        private bool parseMapFile()
        {
            if (p.isAtToken("Version"))
            {
                if (p.isAtToken("2"))
                {
                    m.setVersion(EMapVersion.EMV_VERSION2);
                }
                else if (p.isAtToken("3"))
                {
                    m.setVersion(EMapVersion.EMV_VERSION3);
                }
                else
                {
                    showParseError("Map has unknown version (should be 2 or 3)");
                    m.setVersion(EMapVersion.EMV_OLD);
                }
            }
            else
            {
                m.setVersion(EMapVersion.EMV_OLD);
            }
            while (p.isAtEOF() == false)
            {
                if (p.isAtToken("{"))
                {
                    if (parseMapEntity())
                    {
                        showParseError("Failed to parse entity at " + p.getCurrentLineNumber());
                        return true;
                    }
                }
                else
                {
                    showParseError("Unknown token at line " + p.getCurrentLineNumber());
                    return true;
                }
            }
            return false;
        }
        public MapFile getMapFile()
        {
            return m;
        }
        public bool loadMapFile(string fileName)
        {
            p = new Parser();
            if (p.beginParsingFile(fileName))
                return true;
            m = new MapFile();
            m.setFileName(fileName);
            if (parseMapFile())
                return true;
            return false;
        }
    }
    class MapFileWriter
    {

        public bool writeMapFile(MapFile map, string fileName)
        {
            // get map text
            string mapFileText = map.ToString();
            // write to file
            System.IO.StreamWriter file = new System.IO.StreamWriter(fileName);
            file.WriteLine(mapFileText);
            file.Close();
            return false;
        }
    }
}
