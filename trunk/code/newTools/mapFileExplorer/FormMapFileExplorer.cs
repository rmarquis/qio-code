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
        private TreeNode contextMenuNode;

        public FormMapFileExplorer()
        {
            InitializeComponent();
        }

        private void viewMapFile(string mapName)
        {
            MapFileParser p = new MapFileParser();
            p.loadMapFile(mapName);
            map = p.getMapFile();
            map.setTreeView(treeView1);
            /*
             * MapFileWriter w = new MapFileWriter();
            w.writeMapFile(map, mapName + "_textExport.map");
             * */
            treeView1.BeginUpdate();
            treeView1.Nodes.Clear();
            for (int i = 0; i < map.getNumEntities(); i++)
            {
                MapEntity entity = map.getEntity(i);
                TreeNode nodeEntity = new TreeNode("Entity " + i + " ("+entity.getClassName()+")");
                nodeEntity.Tag = entity;
                entity.setTreeViewNode(nodeEntity);
                TreeNode nodeKeyValues = new TreeNode("Variables");
                KeyValuesList epairs = entity.getKeyValues();
                nodeKeyValues.Tag = epairs;
                for (int j = 0; j < epairs.size(); j++)
                {
                    TreeNode nodeKey = new TreeNode(epairs.getPairKey(j));
                    TreeNode nodeValue = new TreeNode(epairs.getPairValue(j));
                    // save the node so epairs can automatically update GUI
                    epairs.setPairTreeNode(j, nodeKey);
                    nodeKey.Tag = epairs;
                    nodeValue.Tag = epairs;
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
                                nodeBrush.Tag = br;
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
                                nodeBrush.Tag = br;
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
                treeView1.EndUpdate();
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {
         //   viewMapFile("E:/PROJECTS/qio/game/baseqio/maps/mapFormatSamples/oldFormat/singlebrush-64x64x64-fit.map");
        // /   exportObjFile("E:/PROJECTS/qio/game/baseqio/maps/mapFormatSamples/oldFormat/singlebrush-64x64x64-fit.obj");
      //     viewMapFile("E:/PROJECTS/qio/game/baseqio/maps/test_physics.map");
       //    exportObjFile("E:/PROJECTS/qio/game/baseqio/maps/test_physics.obj");
            viewMapFile("E:/PROJECTS/qio/game/baseqio/maps/mapFormatSamples/oldFormat/diamond_for_constraints.map");
            exportObjFile("E:/PROJECTS/qio/game/baseqio/maps/mapFormatSamples/oldFormat/diamond_for_constraints.obj");
        }
        class PolygonVerticesMerger : IVec3ArrayIterator
        {
            // raw arrays, because they are faster
            private Vec3 [] uniquePoints;
            private short[] indices;
            private short[] offsets;
            private short[] counts;
            private short numUniquePoints;
            private short numPolys;
            private short numIndices;

            public PolygonVerticesMerger(int maxBrushFaces = 2048)
            {
                uniquePoints = new Vec3[maxBrushFaces*3];
                indices = new short[maxBrushFaces * 6];
                offsets = new short[maxBrushFaces];
                counts = new short[maxBrushFaces];
                numUniquePoints = 0;
                numPolys = 0;
                numIndices = 0;
            }
            public void reset()
            {
                numUniquePoints = 0;
                numPolys = 0;
                numIndices = 0;
            }
            private short findIndex(Vec3 p)
            {
                for (short i = 0; i < numUniquePoints; i++)
                {
                    if (p.compare(uniquePoints[i]))
                         return i;
                }
                return -1;
            }
            public void addVec3Array(Vec3[] points, int numPoints)
            {
                offsets[numPolys] = numIndices;
                counts[numPolys] = (short)numPoints;
                numPolys++;
                for(int i = 0; i < numPoints; i++)
                {
                    short index = findIndex(points[i]);
                    if (index < 0)
                    {
                        index = numUniquePoints;
                        uniquePoints[numUniquePoints] = points[i];
                        numUniquePoints++;
                    }
                    indices[numIndices] = index;
                    numIndices++;
                }
            }
            public Vec3[]getXYZs()
            {
                return uniquePoints;
            }
            public int getNumUniquePoints()
            {
                return numUniquePoints;
            }
            public short []getFaceIndices()
            {
                return indices;
            }
            public short []getFaceOffsets()
            {
                return offsets;
            }
            public short []getFaceVertsCount()
            {
                return counts;
            }
            public short getNumFaces()
            {
                return numPolys;
            }
        }
        class ConvexVolumeArrayToWaveFrontObjIterator : IConvexVolumeArrayIterator
        {
            private WavefrontOBJ obj;
            private bool bMergeBrushPolygonVertices;
            private PolygonVerticesMerger merger;

            public ConvexVolumeArrayToWaveFrontObjIterator(WavefrontOBJ obj)
            {
                this.obj = obj;
                bMergeBrushPolygonVertices = true;
            }

            public override void addConvexVolume(IConvexVolume v)
            {
                if (bMergeBrushPolygonVertices == false)
                {
                    ConvexVerticesBuilder.buildConvexSides(v, obj);
                }
                else
                {
                    if (merger == null)
                        merger = new PolygonVerticesMerger();
                    else
                        merger.reset();
                    ConvexVerticesBuilder.buildConvexSides(v, merger);

                    obj.addIndexedMesh(merger.getXYZs(), merger.getNumUniquePoints(), merger.getNumFaces(), merger.getFaceIndices(), merger.getFaceOffsets(), merger.getFaceVertsCount());
                }
            }
        }
        private void exportObjFile(string fileName)
        {
            if (map == null)
                return;
            WavefrontOBJ obj = new WavefrontOBJ();
            ConvexVolumeArrayToWaveFrontObjIterator converter = new ConvexVolumeArrayToWaveFrontObjIterator(obj);
            map.iterateBrushes(converter);
            obj.saveObjModel(fileName);
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

        private void treeView1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (treeView1.SelectedNode == treeView1.GetNodeAt(e.Location))
                {
                    if (treeView1.SelectedNode.Nodes.Count == 0)
                    {
                        treeView1.LabelEdit = true;
                        treeView1.SelectedNode.BeginEdit();
                    }
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                if (treeView1.SelectedNode == treeView1.GetNodeAt(e.Location))
                {
                    contextMenuNode = treeView1.GetNodeAt(e.Location);
                    if (contextMenuNode == null)
                        return;
                    contextMenuStrip1.Show(PointToScreen(e.Location));
                }
            }
        }

        private void treeView1_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            if (e.Label == null)
                return;
            KeyValuesList kvList = (KeyValuesList)e.Node.Tag;
            // changing key-value of the entity
            if (kvList != null)
            {
                string keyName = e.Node.Parent.Text;
                kvList.setKeyValue(keyName, e.Label);
                // fix the classname display at entity node
                if (keyName.CompareTo("classname") == 0)
                {
                    TreeNode nodeKeyValues = e.Node.Parent.Parent;
                    TreeNode nodeEntity = nodeKeyValues.Parent;
                    MapEntity mapEntity = (MapEntity)nodeEntity.Tag;
                    if (mapEntity != null)
                    {
                        nodeEntity.Text = ("Entity " + mapEntity.getIndex() + " (" + mapEntity.getClassName() + ")");
                    }
                }
            }
            MapBrushBase brushBase = (MapBrushBase)e.Node.Tag;
            // changing the material name of brushside
            if (brushBase != null)
            {
                // TODO
            }
        }

   
        private void classUsageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
            {
                MessageBox.Show("You must load map file first.", "No map loaded.", MessageBoxButtons.OK);
                return;
            }
            ValueUsersLists classUsers = map.generateClassUsersList();
            FormClassUsersStats f = new FormClassUsersStats(classUsers);
            f.ShowDialog();
        }

        private void replaceClassnameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
            {
                MessageBox.Show("You must load map file first.", "No map loaded.", MessageBoxButtons.OK);
                return;
            }
            FormReplaceClassName f = new FormReplaceClassName(map);
            f.ShowDialog();
        }

        private void replaceMaterialToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
            {
                MessageBox.Show("You must load map file first.", "No map loaded.", MessageBoxButtons.OK);
                return;
            }
            FormReplaceMaterial f = new FormReplaceMaterial(map);
            f.ShowDialog();
        }

        private void modelsUsageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
            {
                MessageBox.Show("You must load map file first.", "No map loaded.", MessageBoxButtons.OK);
                return;
            }
            ValueUsersLists modelUsers = map.generateModelUsersList();
            FormModelUsersStats f = new FormModelUsersStats(modelUsers);
            f.ShowDialog();
        }

        private void copyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (contextMenuNode == null)
                return;
            // allow user to copy entire entity (along with key valeus and brushes)
            MapEntity entity = contextMenuNode.Tag as MapEntity;
            if (entity != null)
            {
                Clipboard.SetText(entity.ToString());
                return;
            }
            // allow user to copy single brushdef block
            MapBrushBase brush = contextMenuNode.Tag as MapBrushBase;
            if (brush != null)
            {
                Clipboard.SetText(brush.ToString());
                return;
            }
            // allow user to copy single keyvalues list
            KeyValuesList epairs = contextMenuNode.Tag as KeyValuesList;
            if (epairs != null)
            {
                Clipboard.SetText(epairs.ToString());
                return;
            }
        }

        private void wavefrontOBJToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (map == null)
            {
                MessageBox.Show("You must load map file first.", "No map loaded.", MessageBoxButtons.OK);
                return;
            }
            saveFileDialog1.Filter = "Wavefront .obj files|*.obj";
            saveFileDialog1.Title = "Export map geometry to .obj";
            if (saveFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                exportObjFile(saveFileDialog1.FileName);
            }
        }
    }
}
