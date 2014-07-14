using fileFormats;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace objFileExplorer
{
    public partial class FormObjFileExplorer : Form
    {
        public FormObjFileExplorer()
        {
            InitializeComponent();
        }

        private WavefrontOBJ m;
        private void saveModel(string fileName)
        {
            if (m == null)
                return;
            m.saveObjModel(fileName);
        }
        private void loadModel(string fileName)
        {
            treeView1.Nodes.Clear();
            m = new WavefrontOBJ();
            m.loadObjModel(fileName);

            TreeNode nodeObjects = new TreeNode("Objects ("+m.getNumObjects()+")");
            for (int i = 0; i < m.getNumObjects(); i++)
            {
                ObjObject o = m.getObject(i);
                TreeNode nodeObject = new TreeNode("Object " + i + " - \"" + o.getName() + "\"");
                // object name
                TreeNode nodeObjectName = new TreeNode("Name");
                TreeNode nodeObjectNameValue = new TreeNode(o.getName());
                nodeObjectNameValue.Tag = o;
                nodeObjectName.Nodes.Add(nodeObjectNameValue);
                nodeObject.Nodes.Add(nodeObjectName);
                // first face
                TreeNode nodeObjectFirstFace = new TreeNode("First face");
                TreeNode nodeObjectFirstFaceValue = new TreeNode(o.getFirstFace().ToString());
                nodeObjectFirstFace.Nodes.Add(nodeObjectFirstFaceValue);
                nodeObject.Nodes.Add(nodeObjectFirstFace);
                // face count
                TreeNode nodeObjectNumFaces = new TreeNode("Faces count");
                TreeNode nodeObjectNumFacesValue = new TreeNode(o.getNumFaces().ToString());
                nodeObjectNumFaces.Nodes.Add(nodeObjectNumFacesValue);
                nodeObject.Nodes.Add(nodeObjectNumFaces);

                nodeObjects.Nodes.Add(nodeObject);
            }
            treeView1.Nodes.Add(nodeObjects);
            TreeNode nodeGroups = new TreeNode("Groups (" + m.getNumGroups() + ")");
            for (int i = 0; i < m.getNumGroups(); i++)
            {
                ObjGroup o = m.getGroup(i);
                TreeNode nodeGroup = new TreeNode("Group " + i + " - \"" + o.getName() + "\"");
                // object name
                TreeNode nodeGroupName = new TreeNode("Name");
                TreeNode nodeGroupNameValue = new TreeNode(o.getName());
                nodeGroupNameValue.Tag = o;
                nodeGroupName.Nodes.Add(nodeGroupNameValue);
                nodeGroup.Nodes.Add(nodeGroupName);
                // first face
                TreeNode nodeGroupFirstFace = new TreeNode("First face");
                TreeNode nodeGroupFirstFaceValue = new TreeNode(o.getFirstFace().ToString());
                nodeGroupFirstFace.Nodes.Add(nodeGroupFirstFaceValue);
                nodeGroup.Nodes.Add(nodeGroupFirstFace);
                // face count
                TreeNode nodeGroupNumFaces = new TreeNode("Faces count");
                TreeNode nodeGroupNumFacesValue = new TreeNode(o.getNumFaces().ToString());
                nodeGroupNumFaces.Nodes.Add(nodeGroupNumFacesValue);
                nodeGroup.Nodes.Add(nodeGroupNumFaces);

                nodeGroups.Nodes.Add(nodeGroup);
            }
            treeView1.Nodes.Add(nodeGroups);
            TreeNode nodeFaces = new TreeNode("Faces ("+m.getNumFaces()+")");
            for (int i = 0; i < m.getNumFaces(); i++)
            {
                ObjFace f = m.getFace(i);
                TreeNode nodeFace = new TreeNode("Face " + i);
                // number of vertices
                /*TreeNode nodeFaceNumFaceVerts = new TreeNode("FaceVerts count");
                TreeNode nodeFaceNumFaceVertsValue = new TreeNode(f.getNumVerts().ToString());
                nodeFaceNumFaceVerts.Nodes.Add(nodeFaceNumFaceVertsValue);
                nodeFace.Nodes.Add(nodeFaceNumFaceVerts);*/
                // vertices
                TreeNode nodeFaceFaceVerts = new TreeNode("FaceVerts (" + f.getNumVerts() + ")");
                for (int j = 0; j < f.getNumVerts(); j++)
                {
                    TreeNode nodeFaceFaceVert = new TreeNode("FaceVert " + j + " (abs " + (j + f.getFirstVert()) + ")");
                    nodeFaceFaceVerts.Nodes.Add(nodeFaceFaceVert);
                }
                nodeFace.Nodes.Add(nodeFaceFaceVerts);

                nodeFaces.Nodes.Add(nodeFace);
            }
            treeView1.Nodes.Add(nodeFaces);
            TreeNode nodePositions = new TreeNode("XYZs ("+m.getNumXYZs()+")");
            for (int i = 0; i < m.getNumXYZs(); i++)
            {
                TreeNode nodePosition = new TreeNode("XYZ " + i);
                TreeNode nodePositionValue = new TreeNode(m.getXYZ(i).ToString());
                nodePosition.Nodes.Add(nodePositionValue);
                nodePositions.Nodes.Add(nodePosition);
            }
            treeView1.Nodes.Add(nodePositions);
            if (m.getNumTexCoords() > 0)
            {
                TreeNode nodeTexCoords = new TreeNode("TexCoords ("+m.getNumTexCoords()+")");
                for (int i = 0; i < m.getNumTexCoords(); i++)
                {
                    TreeNode nodeTexCoord = new TreeNode("TexCoord " + i);
                    TreeNode nodeTexCoordValue = new TreeNode(m.getTexCoord(i).ToString());
                    nodeTexCoord.Nodes.Add(nodeTexCoordValue);
                    nodeTexCoords.Nodes.Add(nodeTexCoord);
                }
                treeView1.Nodes.Add(nodeTexCoords);
            };
            if (m.getNumNormals() > 0)
            {
                TreeNode nodeNormals = new TreeNode("Normals ("+m.getNumNormals()+")");
                for (int i = 0; i < m.getNumNormals(); i++)
                {
                    TreeNode nodeNormal = new TreeNode("Normal " + i);
                    TreeNode nodeNormalValue = new TreeNode(m.getNormal(i).ToString());
                    nodeNormal.Nodes.Add(nodeNormalValue);
                    nodeNormals.Nodes.Add(nodeNormal);
                }
                treeView1.Nodes.Add(nodeNormals);
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {
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
        }

        private void treeView1_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            ObjObject obj = (ObjObject)e.Node.Tag;
            if (obj == null)
                return;
            if (e.Label == null)
                return;
            obj.setName(e.Label);
            TreeNode nodeObject = e.Node.Parent.Parent;
            nodeObject.Text = "Object " + m.getObjectIndex(obj).ToString() + " - \"" + obj.getName() + "\"";
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {           
            openFileDialog1.Filter = "Wavefront OBJ|*.obj|All files (*.*)|*.*";
            openFileDialog1.Title = "Open new model.";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                loadModel(openFileDialog1.FileName);
            }             
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog1.Filter = "Wavefront OBJ|*.obj|All files (*.*)|*.*";
            saveFileDialog1.Title = "Save current model.";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                saveModel(saveFileDialog1.FileName);
            }
        }
    }
}
