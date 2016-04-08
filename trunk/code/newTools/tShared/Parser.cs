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
// newTools/tShared/Parser.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tMath;

namespace shared
{
    class Parser
    {
        private string text;
        private int pos;

        private int skipWhiteSpaces()
        {
            int r = pos;
            while (true)
            {
                if (r >= text.Length)
                    break;
                if (Char.IsWhiteSpace(text[r]))
                {
                    r++;
                    continue;
                }
                if (text[r] == '/' && text[r + 1] == '/')
                {
                    // skip comment
                    r += 2;
                    while (r < text.Length && text[r] != '\n')
                    {
                        r++;
                    }
                    if (r >= text.Length)
                        return r;
                    r++; // skip '\n'
                    continue;
                }
                break;
            }
            return r;
        }
        public bool beginParsingFile(string fileName)
        {
            //try
            //{
                StreamReader streamReader = new StreamReader(fileName);
                text = streamReader.ReadToEnd();
                streamReader.Close();
                return false;
            //}
        //    catch(Exception ex)
          //  {

        }
        public bool beginParsingText(string newText)
        {
            this.text = newText;
            return false;
        }
        public bool isAtEOF()
        {
            int i = skipWhiteSpaces();
            if (i + 1 >= text.Length)
                return true;
            return false;
        }
        public bool isAtEOL()
        {
            int i = pos;
            while (i < text.Length)
            {
                if (text[i] == '\n')
                    return true;
                if (i + 1 < text.Length)
                {
                    if (text[i] == '/' && text[i + 1] == '/')
                    {
                        return true; // single line comment means end of the line
                    }
                }
                if (char.IsWhiteSpace(text[i]) == false)
                    return false;
                i++;
            }
            return false;
        }
        public bool isAtToken(string tok, bool bNeedWSAfter = true)
        {
            int p = skipWhiteSpaces();
            if (tok.Length + p > text.Length)
                return false;
            for (int i = 0; i < tok.Length; i++)
            {
                int charAt = p + i;
                if (charAt >= text.Length)
                    return false;
                char at = text[charAt];
                if (char.ToLower(at) != char.ToLower(tok[i]))
                    return false;
            }
            if (bNeedWSAfter)
            {
                // next character must be a whitespace (unless expected token is directly at the end of the buffer)
                if (p + tok.Length < text.Length)
                {
                    if (Char.IsWhiteSpace(text[p + tok.Length]) == false)
                        return false;
                }
            }
            pos = p + tok.Length;
            return true;
        }
        public bool skipToNextLine()
        {
            while (true)
            {
                if (text[pos] == '\n')
                {
                    pos++;
                    return false;
                }
                pos++;
                if (text.Length <= pos)
                    return true;
            }
        }
        public bool readQuotedString(out string s)
        {
            int p = skipWhiteSpaces();
            if (p >= text.Length)
            {
                s = "";
                return true;
            }
            if (text[p] != '"')
            {
                s = "";
                return true;
            }
            p++;
            int start = p;
            while (text[p] != '"')
            {
                p++;
            }
            s = text.Substring(start, p - start);
            p++;
            pos = p;
            return false;
        }
        public bool readString(out string s, string stopSet = null)
        {
            int p = skipWhiteSpaces();
            if (p >= text.Length)
            {
                s = "";
                return true;
            }
            int start = p;
            while (Char.IsWhiteSpace(text[p]) == false)
            {
                if (stopSet != null)
                {
                    if (stopSet.IndexOf(text[p]) != -1)
                        break;
                }
                p++;
            }
            s = text.Substring(start, p - start);
            pos = p;
            return false;
        }
        public bool readToken(out string s)
        {
            int p = skipWhiteSpaces();
            if (p >= text.Length)
            {
                s = "";
                return true;
            }
            if (text[p] == '"')
                return readQuotedString(out s);
            return readString(out s);
        }
        public bool readFloat(out float f)
        {
            string s;
            if (readString(out s))
            {
                f = 0;
                return true;
            }
            s = s.Replace('.', ',');
            if (!float.TryParse(s, out f))
                return true;
            return false;
        }
        public bool readInt(out int i)
        {
            string s;
            if (readString(out s))
            {
                i = 0;
                return true;
            }
            if (!int.TryParse(s, out i))
                return true;
            return false;
        }
        public bool readVec3(out Vec3 o)
        {
            float x, y, z;
            if (readFloat(out x))
            {
                o = new Vec3(0, 0, 0);
                return true;
            }
            if (readFloat(out y))
            {
                o = new Vec3(0, 0, 0);
                return true;
            }
            if (readFloat(out z))
            {
                o = new Vec3(0, 0, 0);
                return true;
            }
            o = new Vec3(x, y, z);
            return false;
        }
        public bool readVec2(out Vec2 o)
        {
            float x, y;
            if (readFloat(out x))
            {
                o = new Vec2(0, 0);
                return true;
            }
            if (readFloat(out y))
            {
                o = new Vec2(0, 0);
                return true;
            }
            o = new Vec2(x, y);
            return false;
        }
        public bool readBracedVec3(out Vec3 o)
        {
            if (isAtToken("(") == false)
            {
                o = new Vec3(0, 0, 0);
                return true;
            }
            if (readVec3(out o))
                return true;
            if (isAtToken(")") == false)
            {
                o = new Vec3(0, 0, 0);
                return true;
            }
            return false;
        }
        public bool readBracedVec2(out Vec2 o)
        {
            if (isAtToken("(") == false)
            {
                o = new Vec2(0, 0);
                return true;
            }
            if (readVec2(out o))
                return true;
            if (isAtToken(")") == false)
            {
                o = new Vec2(0, 0);
                return true;
            }
            return false;
        }
        public void skipCurlyBracedBlock()
        {
            // check for EOF early
            if (pos >= text.Length)
            {
                return;
            }
            int level = 1;
            while (level > 0)
            {
                if (text[pos] == '{')
                    level++;
                if (text[pos] == '}')
                    level--;
                pos++;
                if (pos >= text.Length)
                {
                    return;
                }
            }
        }
        public int getCurrentLineNumber()
        {
            int i = 0;
            int ret = 0;
            while (i < pos)
            {
                if (text[i] == '\n')
                    ret++;
                i++;
            }
            return ret;
        }
        public int getPos()
        {
            return pos;
        }
    }
}
