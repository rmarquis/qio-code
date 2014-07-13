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
// newTools/mapFileExplorer/FormMapFileExplorer.cs
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using tMath;
using fileFormats;
using shared;

namespace mapFileExplorer
{
    public partial class FormMapFileExplorer : Form
    {
        private MapFile map;

        public FormMapFileExplorer()
        {
            InitializeComponent();
        }

        private void viewMapFile(string mapName)
        {
            MapFileParser p = new MapFileParser();
            p.loadMapFile(mapName);
            map = p.getMapFile();
            /*
             * MapFileWriter w = new MapFileWriter();
            w.writeMapFile(map, mapName + "_textExport.map");
             * */
            treeView1.Nodes.Clear();
            for (int i = 0; i < map.getNumEntities(); i++)
            {
                MapEntity entity = map.getEntity(i);
                TreeNode nodeEntity = new TreeNode("Entity " + i + " ("+entity.getClassName()+")");
                TreeNode nodeKeyValues = new TreeNode("Variables");
                KeyValuesList epairs = entity.getKeyValues();
                for (int j = 0; j < epairs.size(); j++)
                {
                    KeyValue kv = epairs.getKeyValue(j);
                    TreeNode nodeKey = new TreeNode(kv.getKey());
                    TreeNode nodeValue = new TreeNode(kv.getValue());
                    nodeKey.Nodes.Add(nodeValue);
                    nodeKeyValues.Nodes.Add(nodeKey);
                }
                nodeEntity.Nodes.Add(nodeKeyValues);
                if (entity.getNumBrushes() != 0 || entity.getNumPatches() != 0)
                {
                    TreeNode nodeGeometry = new TreeNode("Geometry");
                    if (entity.getNumBrushes() != 0)
                    {
                        TreeNode nodeBrushes = new TreeNode("Brushes");
                        for (int j = 0; j < entity.getNumBrushes(); j++)
                        {
                            MapBrushBase brBase = entity.getBrush(j);
                            if (brBase.isOldBrush())
                            {
                                MapBrushOld br = (MapBrushOld)brBase;
                                TreeNode nodeBrush = new TreeNode("Brush (old format) " + j);
                                for (int k = 0; k < br.getNumSides(); k++)
                                {
                                    MapBrushSideOld side = br.getSide(k);
                                    TreeNode nodeSide = new TreeNode("Side " + k);
                                    // plane points
                                    TreeNode nodePlanePoints = new TreeNode("Plane points");
                                    nodePlanePoints.Nodes.Add(side.getPlanePointA().ToStringBraced()
                                        + " " + side.getPlanePointB().ToStringBraced()
                                        + " " + side.getPlanePointB().ToStringBraced());
                                    nodeSide.Nodes.Add(nodePlanePoints);
                                    // material
                                    TreeNode nodeMaterial = new TreeNode("Material");
                                    nodeMaterial.Nodes.Add(side.getMatName());
                                    nodeSide.Nodes.Add(nodeMaterial);
                                    // tex scale
                                    TreeNode nodeTexScale = new TreeNode("TexScale");
                                    nodeTexScale.Nodes.Add(side.getTexScale().ToString());
                                    nodeSide.Nodes.Add(nodeTexScale);
                                    // tex shift
                                    TreeNode nodeTexShift = new TreeNode("TexShift");
                                    nodeTexShift.Nodes.Add(side.getTexShift().ToString());
                                    nodeSide.Nodes.Add(nodeTexShift);
                                    // tex rotation
                                    TreeNode nodeTexRotation = new TreeNode("TexRotation");
                                    nodeTexRotation.Nodes.Add(side.getTexRotation().ToString());
                                    nodeSide.Nodes.Add(nodeTexRotation);
                                    // contents (eg. detail)
                                    TreeNode nodeContentFlags = new TreeNode("ContentFlags");
                                    nodeContentFlags.Nodes.Add(side.getContentFlags().ToString());
                                    nodeSide.Nodes.Add(nodeContentFlags);
                                    nodeBrush.Nodes.Add(nodeSide);
                                }
                                nodeBrushes.Nodes.Add(nodeBrush);
                            }
                            else
                            {
                                MapBrushDef3 br = (MapBrushDef3)brBase;
                                TreeNode nodeBrush = new TreeNode("Brush (Def3) " + j);
                                for (int k = 0; k < br.getNumSides(); k++)
                                {
                                    MapBrushSide4 side = br.getSide(k);
                                    TreeNode nodeSide = new TreeNode("Side " + k);
                                    // plane points
                                    TreeNode nodePlanePoints = new TreeNode("Plane");
                                    nodePlanePoints.Nodes.Add(side.getPlane().ToString());
                                    nodeSide.Nodes.Add(nodePlanePoints);
                                    // material
                                    TreeNode nodeMaterial = new TreeNode("Material");
                                    nodeMaterial.Nodes.Add(side.getMatName());
                                    nodeSide.Nodes.Add(nodeMaterial);
                                    // tex matrix
                                     TreeNode nodeTexAxis0 = new TreeNode("TexAxis0");
                                     nodeTexAxis0.Nodes.Add(side.getTextureAxis0().ToString());
                                     nodeSide.Nodes.Add(nodeTexAxis0);
                                     TreeNode nodeTexAxis1 = new TreeNode("TexAxis1");
                                     nodeTexAxis1.Nodes.Add(side.getTextureAxis1().ToString());
                                     nodeSide.Nodes.Add(nodeTexAxis1);
                                     nodeBrush.Nodes.Add(nodeSide);
                                }
                                nodeBrushes.Nodes.Add(nodeBrush);
                            }
                        }
                        nodeGeometry.Nodes.Add(nodeBrushes);
                    }
                    if (entity.getNumPatches() != 0)
                    {
                        TreeNode nodePatches = new TreeNode("Patches");
                        for (int j = 0; j < entity.getNumPatches(); j++)
                        {
                            MapPatch patch = entity.getPatch(j);
                            TreeNode nodePatch = new TreeNode("Patch " + j);
                            // material
                            TreeNode nodeMaterial = new TreeNode("Material");
                            nodeMaterial.Nodes.Add(patch.getMatName());
                            nodePatch.Nodes.Add(nodeMaterial);
                            // sizes
                            TreeNode nodeSizes = new TreeNode("Size");
                            nodeSizes.Nodes.Add(patch.getSizeW().ToString() + " " + patch.getSizeH().ToString());
                            nodePatch.Nodes.Add(nodeSizes);
                            // verts
                            TreeNode nodeVerts = new TreeNode("Verts");
                            for (int k = 0; k < patch.getSizeH(); k++)
                            {
                                TreeNode nodeRowK = new TreeNode("Row " + k);
                                for (int l = 0; l < patch.getSizeW(); l++)
                                {
                                    TreeNode nodeVertex = new TreeNode("Vertex " + l + " (abs " + k + "/" + l + ")");
                                    nodeVertex.Nodes.Add(patch.getVertex(l, k).ToString());
                                    nodeRowK.Nodes.Add(nodeVertex);
                                }
                                nodeVerts.Nodes.Add(nodeRowK);
                            }
                            nodePatch.Nodes.Add(nodeVerts);

                            nodePatches.Nodes.Add(nodePatch);
                        }
                        nodeGeometry.Nodes.Add(nodePatches);
                    }
                    nodeEntity.Nodes.Add(nodeGeometry);
                }
                treeView1.Nodes.Add(nodeEntity);
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Map files|*.map";
            openFileDialog1.Title = "Select a map file";

            if (openFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                viewMapFile(openFileDialog1.FileName);
            }
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            treeView1.Nodes.Clear();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
                return;
            saveFileDialog1.Filter = "Map files|*.map";
            saveFileDialog1.Title = "Write a map file";
            if (saveFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                MapFileWriter writer = new MapFileWriter();
                writer.writeMapFile(map,saveFileDialog1.FileName);
            }
        }
    }
}
