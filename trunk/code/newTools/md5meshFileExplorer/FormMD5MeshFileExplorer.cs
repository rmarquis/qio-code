using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using fileFormats;

namespace md5meshFileExplorer
{
    public partial class FormMD5MeshFileExplorer : Form
    {
        private MD5Model md5model;

        public FormMD5MeshFileExplorer()
        {
            InitializeComponent();
        }

        private void refreshTreeView()
        {
            treeView1.Nodes.Clear();
            TreeNode root = new TreeNode("MD5Model");
            TreeNode fileName = new TreeNode("FileName");
            TreeNode fileNameText = new TreeNode(md5model.getName());
            fileName.Nodes.Add(fileNameText);
            root.Nodes.Add(fileName);
            TreeNode commandLine = new TreeNode("commandLine");
            TreeNode commandLineText = new TreeNode(md5model.getCommandLine());
            commandLine.Nodes.Add(commandLineText);
            root.Nodes.Add(commandLine);
            TreeNode meshes = new TreeNode("meshes (" + md5model.getMeshCount() + ")");
            for (int i = 0; i < md5model.getMeshCount(); i++)
            {
                TreeNode mesh = new TreeNode("mesh " + i);
                TreeNode materialName = new TreeNode("material");
                materialName.Nodes.Add(new TreeNode(md5model.getMeshMaterialName(i)));
                mesh.Nodes.Add(materialName);
                TreeNode vertices = new TreeNode("vertices ("+md5model.getMeshVertexCount(i)+")");

                mesh.Nodes.Add(vertices);
                TreeNode triangles = new TreeNode("triangles (" + md5model.getMeshTriangleCount(i) + ")");

                mesh.Nodes.Add(triangles);
                meshes.Nodes.Add(mesh);
            }
            root.Nodes.Add(meshes);
            TreeNode joints = new TreeNode("joints ("+md5model.getJointCount()+")");
            for (int i = 0; i < md5model.getJointCount(); i++)
            {
                MD5Joint j = md5model.getJoint(i);
                TreeNode joint = new TreeNode("joint "+i+" "+j.getName());
                TreeNode jointName = new TreeNode("name");
                TreeNode jointNameValue = new TreeNode(j.getName());
                jointName.Nodes.Add(jointNameValue);
                joint.Nodes.Add(jointName);
                TreeNode jointOrigin = new TreeNode("Ofs");
                TreeNode jointOriginValue = new TreeNode(j.getOfs().ToString());
                jointOrigin.Nodes.Add(jointOriginValue);
                joint.Nodes.Add(jointOrigin);
                TreeNode jointRot = new TreeNode("Rot");
                TreeNode jointRotValue = new TreeNode(j.getRot().ToString());
                jointRot.Nodes.Add(jointRotValue);
                joint.Nodes.Add(jointRot);
                joints.Nodes.Add(joint);
            }
            root.Nodes.Add(joints);
            treeView1.Nodes.Add(root);
        }
        private void viewMd5Mesh(string name)
        {
            md5model = new MD5Model();
            md5model.loadMD5MeshFile(name);

            refreshTreeView();
        }
        private void exportObjFile(string fileName)
        {
            if (md5model == null)
                return;
            WavefrontOBJ obj = new WavefrontOBJ();
            md5model.buildStaticMesh(obj);
            obj.saveObjModel(fileName);
        }

        private void FormMD5MeshFileExplorer_Load(object sender, EventArgs e)
        {
            viewMd5Mesh("E:/PROJECTS/qio/game/baseqio/models/player/shina/body.md5mesh");
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "MD5mesh files|*.md5mesh";
            openFileDialog1.Title = "Select md5mesh file";

            if (openFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                viewMd5Mesh(openFileDialog1.FileName);
            }
        }

        private void wavefrontobjToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (md5model == null)
            {
                MessageBox.Show("You must load model file first.", "No model loaded.", MessageBoxButtons.OK);
                return;
            }
            saveFileDialog1.Filter = "Wavefront .obj files|*.obj";
            saveFileDialog1.Title = "Export model current geometry to .obj";
            if (saveFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                exportObjFile(saveFileDialog1.FileName);
            }
        }
    }
}
