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
    class MaterialsSystem
    {
        private List<MtrFile> matFiles;

        public MaterialsSystem()
        {
            clearAll();
        }
        public MaterialDef findMaterialDef(string name)
        {
            foreach (MtrFile f in matFiles)
            {
                MaterialDef md = f.findMaterialDef(name);
                if (md != null)
                    return md;
            }
            return null;
        }
        public void clearAll()
        {
            matFiles = new List<MtrFile>();
        }
        public bool materialExists(string name)
        {
            if (findMaterialDef(name) != null)
                return true;
            return false;
        }
        public MtrFile findMtrFile(string name)
        {
            if (matFiles == null)
                return null;
            foreach (MtrFile f in matFiles)
            {
                if (name.CompareTo(f.getName()) == 0)
                {
                    return f;
                }
            }
            return null;
        }
        public void loadOrReloadMaterialFile(string name)
        {
            // see if its loaded
            MtrFile f = findMtrFile(name);
            if (f != null)
            {
                f.precacheMtrFile();
                return;
            }
            // if not, load new
            loadNewMtrFile(name);
        }
        public void loadNewMtrFile(string path)
        {
            try
            {
                MtrFile mf;
                mf = new MtrFile(path);
                matFiles.Add(mf);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Precache failed.", "Failed to precache material file " + path,
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Exclamation,
                        MessageBoxDefaultButton.Button1);
            }
        }
    }
}
