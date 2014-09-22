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

namespace md5animFileExplorer
{
    public partial class FormMD5AnimFileExplorer : Form
    {
        private MD5Anim md5anim;

        public FormMD5AnimFileExplorer()
        {
            InitializeComponent();
        }

        private void FormMD5AnimFileExplorer_Load(object sender, EventArgs e)
        {
            viewMd5Anim("E:/PROJECTS/qio/game/baseqio/models/player/shina/run.md5anim");
        }
        private void viewMd5Anim(string name)
        {
            md5anim = new MD5Anim();
            if (md5anim.loadMD5AnimFile(name))
            {
                md5anim = null;
                return;
            }

            refreshTreeView();
        }
        private void refreshTreeView()
        {
            treeView1.Nodes.Clear();
            TreeNode root = new TreeNode("MD5Anim");
            TreeNode fileName = new TreeNode("FileName");
            TreeNode fileNameText = new TreeNode(md5anim.getName());
            fileName.Nodes.Add(fileNameText);
            root.Nodes.Add(fileName);
            TreeNode commandLine = new TreeNode("commandLine");
            TreeNode commandLineText = new TreeNode(md5anim.getCommandLine());
            commandLine.Nodes.Add(commandLineText);
            root.Nodes.Add(commandLine);
            TreeNode frameRate = new TreeNode("frameRate");
            TreeNode frameRateText = new TreeNode(md5anim.getFrameRate().ToString());
            frameRate.Nodes.Add(frameRateText);
            root.Nodes.Add(frameRate);
            TreeNode numAnimatedComponents = new TreeNode("numAnimatedComponents");
            TreeNode numAnimatedComponentsText = new TreeNode(md5anim.getAnimatedComponentsCount().ToString());
            numAnimatedComponents.Nodes.Add(numAnimatedComponentsText);
            root.Nodes.Add(numAnimatedComponents);

            TreeNode frames = new TreeNode("frames (" + md5anim.getFrameCount() + ")");
            for (int i = 0; i < md5anim.getFrameCount(); i++)
            {
                TreeNode frame = new TreeNode("frame " + i);
                TreeNode bounds = new TreeNode("Bounds");
                bounds.Nodes.Add(md5anim.getBounds(i).ToString());
                frame.Nodes.Add(bounds);
                TreeNode values = new TreeNode("Values");
                for (int j = 0; j < md5anim.getAnimatedComponentsCount(); j++)
                {
                    values.Nodes.Add(new TreeNode(md5anim.getFrame(i).getValue(j).ToString()));
                }
                frame.Nodes.Add(values);
                frames.Nodes.Add(frame);
            }
            root.Nodes.Add(frames);
            TreeNode joints = new TreeNode("joints (" + md5anim.getJointCount() + ")");
            for (int i = 0; i < md5anim.getJointCount(); i++)
            {
                MD5AnimJoint j = md5anim.getJoint(i);
                TreeNode joint = new TreeNode("joint " + i + " " + j.getName());
                TreeNode jointName = new TreeNode("name");
                TreeNode jointNameValue = new TreeNode(j.getName());
                jointName.Nodes.Add(jointNameValue);
                joint.Nodes.Add(jointName);
                TreeNode parent = new TreeNode("parent");
                TreeNode parentValue = new TreeNode(j.getParentIndex().ToString());
                parent.Nodes.Add(parentValue);
                joint.Nodes.Add(parent);
                TreeNode jointOrigin = new TreeNode("Ofs");
                TreeNode jointOriginValue = new TreeNode(j.getLocalOfs().ToString());
                jointOrigin.Nodes.Add(jointOriginValue);
                joint.Nodes.Add(jointOrigin);
                TreeNode jointRot = new TreeNode("Rot");
                TreeNode jointRotValue = new TreeNode(j.getLocalRot().ToString());
                jointRot.Nodes.Add(jointRotValue);
                joint.Nodes.Add(jointRot);
                joints.Nodes.Add(joint);
            }
            root.Nodes.Add(joints);
            treeView1.Nodes.Add(root);
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "D3/Q4 md5anim|*.md5anim|All files (*.*)|*.*";
            openFileDialog1.Title = "Open new animation.";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                viewMd5Anim(openFileDialog1.FileName);
            }  
        }
    }
}
