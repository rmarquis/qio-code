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
// newTools/tShared/KeyValuesList.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tMath;

namespace shared
{
#if false
    class KeyValuesList
    {
        private Dictionary<string, string> data;

        public KeyValuesList()
        {
            data = new Dictionary<string, string>();
        }
        public void setKeyValue(string key, string value)
        {
            data.Add(key, value);
        }
        public string getKeyValue(string key)
        {
            string r;
            if (data.TryGetValue(key, out r))
                return r;
            return "";
        }
        public string getPairKey(int i)
        {
            return data.ElementAt(i).Key;
        }
        public string getPairValue(int i)
        {
            return data.ElementAt(i).Value;
        }
        public override string ToString()
        {
            string r = "";
            for (int i = 0; i < data.Count; i++)
            {
                var elem = data.ElementAt(i);
                r += ("\"" + elem.Key + "\"");
                r += ("\"" + elem.Value + "\"");
            }
            return r;
        }
        public int size()
        {
            return data.Count;
        }
    }
#else
    public class KeyValue
    {
        private string key, value;
        // only for GUI
        private System.Windows.Forms.TreeNode nodeKey;

        public KeyValue(string nk, string nv)
        {
            key = nk;
            value = nv;
        }
        public void setValue(string s)
        {
            value = s;
            // automatically update GUI
            if (nodeKey != null)
            {
                if (nodeKey.Nodes.Count != 0)
                {
                    System.Windows.Forms.TreeNode nodeValue = nodeKey.Nodes[0];       
                    nodeValue.Text = s;
                }
            }
        }
        public string getKey()
        {
            return key;
        }
        public string getValue()
        {
            return value;
        }
        public void setNodeKey(System.Windows.Forms.TreeNode pNodeKey)
        {
            nodeKey = pNodeKey;
        }
        public System.Windows.Forms.TreeNode getNodeKey()
        {
            return nodeKey;
        }
        public override string ToString()
        {
            return "\"" + key + "\" \"" + value + "\"\n";
        }
    }
    public class KeyValuesList
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
        public string getPairKey(int i)
        {
            return data[i].getKey();
        }
        public string getPairValue(int i)
        {
            return data[i].getValue();
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
        // only for GUI
        public void setPairTreeNode(int index, System.Windows.Forms.TreeNode pNodeKey)
        {
            data[index].setNodeKey(pNodeKey);
        }

        public bool hasKey(string key)
        {
            int index = findKeyIndex(key);
            if (index >= 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
#endif
}
