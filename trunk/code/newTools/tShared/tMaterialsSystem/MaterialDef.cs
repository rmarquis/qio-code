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
using tMath;

namespace shared
{
       class MaterialDef
        {
            private string matName;
            // after mat name, before {
            private int matDefStart;
            private int matDefEnd;

            public MaterialDef(string matName, int start, int end)
            {
                this.matName = matName;
                this.matDefStart = start;
                this.matDefEnd = end;

              //  MessageBox.Show("New material name " + matName);
            }
            public int getStart()
            {
                return matDefStart;
            }
            public int getEnd()
            {
                return matDefEnd;
            }
            public string getName()
            {
                return matName;
            }
        }
}
