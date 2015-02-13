using fileFormats;
using shared;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using tDirectx;

namespace md5Viewer
{
    public partial class FormMD5Viewer : Form
    {
        private MD5Model md5model;
        private MD5Anim md5anim;
        private Camera camera = new Camera();
        private Device device = null;
        private PresentParameters presentParams = null;
        private XYZTrianglesList rawMesh;
        private DXVertexBufferXYZNormals gpuMesh;
        private bool bMouseDown = false;
        private int mouseX;
        private int mouseY;
        private System.Windows.Forms.MouseButtons mouseButton;
        private int curFrame;
        private BoneOrientations bones;

        public FormMD5Viewer()
        {
            InitializeComponent();
            InitializeDirectX(this.pb_3dView);
        }
        // handle should be the handle to PictureBox
        private void InitializeDirectX(Control handle)
        {
            presentParams = new PresentParameters();
            try
            {
                presentParams.Windowed = true;
                presentParams.SwapEffect = SwapEffect.Discard;
                presentParams.EnableAutoDepthStencil = true;
                presentParams.AutoDepthStencilFormat = DepthFormat.D16;
                device = new Device(0, DeviceType.Hardware, handle, CreateFlags.HardwareVertexProcessing, presentParams);
            }
            catch (DirectXException e)
            {
                MessageBox.Show("DirectX exception caught during init! " + e.ToString());
            }
        }
        // renders a single frame in the picturebox
        // (should be called when screen must be refreshed eg. by button or timer)
        private void UpdateDirectXDisplay()
        {
            if (device == null)
                return;
            try
            {
                device.SetRenderState(RenderStates.ShadeMode, 1);
                device.RenderState.Lighting = false;
                device.RenderState.CullMode = Cull.None;
                device.SetRenderState(RenderStates.ZEnable, true);

                device.Clear(ClearFlags.Target, System.Drawing.Color.White, 1.0f, 0);
                device.Clear(ClearFlags.ZBuffer, System.Drawing.Color.White, 1.0f, 0);
                device.BeginScene();


                device.RenderState.Lighting = true;
                // add two directional lights so it's more easy to distinguish between model sides
                device.Lights[0].Type = LightType.Directional;
                device.Lights[0].Diffuse = System.Drawing.Color.DarkTurquoise;
                device.Lights[0].Direction = new Vector3(1, 2, 3);
                device.Lights[0].Enabled = true; // Turn it on

                device.Lights[1].Type = LightType.Directional;
                device.Lights[1].Diffuse = System.Drawing.Color.Yellow;
                device.Lights[1].Direction = new Vector3(-2, -1, -3);
                device.Lights[1].Enabled = true; // Turn it on

                // Finally, turn on some ambient light.
                // Ambient light is light that scatters and lights all objects evenly.
                device.RenderState.Ambient = System.Drawing.Color.FromArgb(0x202020);

                device.Transform.Projection = Matrix.PerspectiveFovLH((float)Math.PI / 4, pb_3dView.Width / pb_3dView.Height, 1f, 5000f);

                Vector3 at;
                at.X = (float)camera.getPosition().getX();
                at.Y = (float)camera.getPosition().getY();
                at.Z = (float)camera.getPosition().getZ();
                device.Transform.View = Matrix.LookAtLH(at, new Vector3(0, 0, 0), new Vector3(0, 0, 1));

                // drawing primitive without texture
                device.SetTexture(0, null);

                if (gpuMesh != null)
                {
                     gpuMesh.drawBuffer(device);
                }

                device.EndScene();
                device.Present();

            }
            catch (DirectXException e)
            {
                MessageBox.Show("DirectX exception caught during init! " + e.ToString());
            }
        }
        private void updateAnimStatusLabel()
        {
            if (md5anim == null)
            {
                trackBarFrame.Enabled = false;
                lb_animInfo.Text = "No animation loaded.";
                return;
            }
            lb_animInfo.Text = "Frame " + curFrame + " of " + md5anim.getFrameCount() + ".";
            trackBarFrame.Maximum = md5anim.getFrameCount();
            trackBarFrame.Minimum = 0;
            if (trackBarFrame.Value >= trackBarFrame.Maximum)
            {
                trackBarFrame.Value = 0;
                curFrame = 0;
            }
            trackBarFrame.Enabled = true;
            bones = new BoneOrientations(md5anim.getJointCount());
        }
        private bool loadMD5Anim(string fileName)
        {
            md5anim = new MD5Anim();
            if (md5anim.loadMD5AnimFile(fileName))
            {
                md5anim = null;
                return true;
            }
            lb_md5AnimName.Text = fileName;
            if (curFrame >= md5anim.getFrameCount())
                curFrame = 0;
            updateAnimStatusLabel();
            return false;
        }
        private bool loadMD5Model(string fileName)
        {
            md5model = new MD5Model();
            if (md5model.loadMD5MeshFile(fileName))
            {
                md5model = null;
                return true;
            }
            lb_md5MeshName.Text = fileName;

            md5model.buildBaseFrameVertices();
            rawMesh = new XYZTrianglesList();
            md5model.addToSimpleStaticMeshBuilder(rawMesh);
            gpuMesh = new DXVertexBufferXYZNormals();
            gpuMesh.create(rawMesh, device);
            UpdateDirectXDisplay();
            return false;
        }
        private void buildCurrentFrameMesh()
        {
            md5anim.buildFrameABSBones(curFrame, bones);
            rawMesh.clear();
            md5model.buildVertices(bones);
            md5model.addToSimpleStaticMeshBuilder(rawMesh);
            gpuMesh.create(rawMesh, device);
        }
        private void FormMD5Viewer_Load(object sender, EventArgs e)
        {
            InitializeDirectX(pb_3dView);
            loadMD5Model("E:/PROJECTS/qio/game/baseqio/models/player/shina/body.md5mesh");
            loadMD5Anim("E:/PROJECTS/qio/game/baseqio/models/player/shina/run.md5anim");
        }

        private void openModelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "MD5mesh files|*.md5mesh";
            openFileDialog1.Title = "Select md5mesh file";

            if (openFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                loadMD5Model(openFileDialog1.FileName);
            }
        }

        private void openAnimationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "MD5anim files|*.md5anim";
            openFileDialog1.Title = "Select md5anim file";

            if (openFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                loadMD5Anim(openFileDialog1.FileName);
            }
        }

        private void pb_3dView_MouseUp(object sender, MouseEventArgs e)
        {
            bMouseDown = false;
        }

        private void pb_3dView_MouseMove(object sender, MouseEventArgs e)
        {
            if (bMouseDown)
            {
                // calculate mouse cursor delta
                float deltaX = mouseX - e.X;
                float deltaY = mouseY - e.Y;

                mouseX = e.X;
                mouseY = e.Y;

                if (mouseButton == MouseButtons.Left)
                {
                    // change camera angles
                    camera.rotateTheta(deltaX);
                    camera.rotatePhi(deltaY);
                }
                else
                {
                    camera.deltaRadius(deltaY);
                    //camera.rotateYaw(deltaX);
                }
                UpdateDirectXDisplay();
            }
        }

        private void pb_3dView_MouseDown(object sender, MouseEventArgs e)
        {
            bMouseDown = true;
            mouseButton = e.Button;
            mouseX = e.X;
            mouseY = e.Y;
        }

        private void trackBarFrame_Scroll(object sender, EventArgs e)
        {
            curFrame = trackBarFrame.Value;
            buildCurrentFrameMesh();
            updateAnimStatusLabel();
            UpdateDirectXDisplay();
        }
    }
}
