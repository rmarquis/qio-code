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

namespace md3FileExplorer
{
    public partial class FormMD3FileExplorer : Form
    {
        private MD3Model md3Model;

        public FormMD3FileExplorer()
        {
            InitializeComponent();

        }

        private void refreshTreeView()
        {
            treeView1.Nodes.Clear();
            TreeNode root = new TreeNode("MD3Model");
            TreeNode fileName = new TreeNode("FileName");
            TreeNode fileNameText = new TreeNode(md3Model.getFileName());
            fileName.Nodes.Add(fileNameText);
            root.Nodes.Add(fileName);
            TreeNode commandLine = new TreeNode("InternalName");
            TreeNode commandLineText = new TreeNode(md3Model.getInternalName());
            commandLine.Nodes.Add(commandLineText);
            root.Nodes.Add(commandLine);
            // display meshes
            TreeNode meshes = new TreeNode("meshes (" + md3Model.getNumSurfaces() + ")");
            for (int i = 0; i < md3Model.getNumSurfaces(); i++)
            {
                MD3Surface sf = md3Model.getSurface(i);
                TreeNode mesh = new TreeNode("surface " + i);
                TreeNode materials = new TreeNode("materials");
                for(int j = 0; j < sf.getNumMaterials(); j++) 
                {
                    TreeNode materialName = new TreeNode("material "+j);
                    materialName.Nodes.Add(new TreeNode(sf.getMaterialName(j)));
                    materials.Nodes.Add(materialName);
                }
                mesh.Nodes.Add(materials);
                TreeNode vertices = new TreeNode("vertices (" + sf.getNumVertices() + ")");

                mesh.Nodes.Add(vertices);
                TreeNode triangles = new TreeNode("triangles (" + sf.getNumTris() + ")");

                mesh.Nodes.Add(triangles);
                meshes.Nodes.Add(mesh);
            }
            root.Nodes.Add(meshes);
            // display tags
            TreeNode joints = new TreeNode("tags (" + md3Model.getNumTags() + ")");
            for (int i = 0; i < md3Model.getNumTags(); i++)
            {
                MD3Tag j = md3Model.getTag(i);
                TreeNode joint = new TreeNode("joint " + i + " " + j.getName());
                TreeNode jointName = new TreeNode("name");
                TreeNode jointNameValue = new TreeNode(j.getName());
                jointName.Nodes.Add(jointNameValue);
                joint.Nodes.Add(jointName);
                TreeNode jointOrigin = new TreeNode("Pos");
                TreeNode jointOriginValue = new TreeNode(j.getPos().ToString());
                jointOrigin.Nodes.Add(jointOriginValue);
                joint.Nodes.Add(jointOrigin);
                TreeNode jointRot = new TreeNode("Axis");
                TreeNode rotForward = new TreeNode("forward");
                rotForward.Nodes.Add(j.getRotForward().ToString());
                jointRot.Nodes.Add(rotForward);
                TreeNode rotRight = new TreeNode("right");
                rotRight.Nodes.Add(j.getRotRight().ToString());
                jointRot.Nodes.Add(rotRight);
                TreeNode rotUp = new TreeNode("up");
                rotUp.Nodes.Add(j.getRotUp().ToString());
                jointRot.Nodes.Add(rotUp);
                joint.Nodes.Add(jointRot);
                joints.Nodes.Add(joint);
            }
            root.Nodes.Add(joints);
            // display frames
            TreeNode frames = new TreeNode("frames (" + md3Model.getNumFrames() + ")");
            for (int i = 0; i < md3Model.getNumFrames(); i++)
            {
                MD3Frame f = md3Model.getFrame(i);
                TreeNode frame = new TreeNode("frame " + i);
                TreeNode name = new TreeNode("name");
                name.Nodes.Add(f.getName());
                frame.Nodes.Add(name);
                TreeNode mins = new TreeNode("mins");
                mins.Nodes.Add(f.getMins().ToString());
                frame.Nodes.Add(mins);
                TreeNode maxs = new TreeNode("maxs");
                maxs.Nodes.Add(f.getMaxs().ToString());
                frame.Nodes.Add(maxs);
                TreeNode radius = new TreeNode("radius");
                radius.Nodes.Add(f.getRadius().ToString());
                frame.Nodes.Add(radius);
                TreeNode localOrigin = new TreeNode("localOrigin");
                localOrigin.Nodes.Add(f.getLocalOrigin().ToString());
                frame.Nodes.Add(localOrigin);

                frames.Nodes.Add(frame);
            }
            root.Nodes.Add(frames);
            treeView1.Nodes.Add(root);
        }
        private void viewMD3Model(string name)
        {
            md3Model = new MD3Model();
            if (md3Model.loadMD3Model(name))
            {
                md3Model = null;
                return;
            }

            refreshTreeView();
        }
        private void FormMD3FileExplorer_Load(object sender, EventArgs e)
        {

        }
    }
}
