using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
// application is using DirectX libraries
// (they should be adde to project "References" list)
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using fileFormats;
using shared;
using tMath;

namespace objViewer
{
    public partial class FormObjViewer : Form
    {
        // directX variables
        private Device device = null;
        private PresentParameters presentParams = null;
        private WavefrontOBJ model = null;
        private DXVertexBufferXYZNormals vertexBuffer = null;
        private Camera camera = null;
        private bool bMouseDown = false;
        private int mouseX;
        private int mouseY;
        private System.Windows.Forms.MouseButtons mouseButton;

        class DXVertexBufferXYZNormals
        {
            private VertexBuffer vertexBuffer = null;
            private int primitiveCount;

            public void create(XYZTrianglesList l, Device device)
            {
                primitiveCount = l.getTrianglesCount();
                vertexBuffer = new VertexBuffer(typeof(CustomVertex.PositionNormalColored),
                            l.getTrianglesCount() * 3, device, 0, CustomVertex.PositionNormalColored.Format, Pool.Default);

                GraphicsStream stm = vertexBuffer.Lock(0, 0, 0);
                CustomVertex.PositionNormalColored[] verts = new CustomVertex.PositionNormalColored[l.getTrianglesCount() * 3];

                // set triangle points
                for (int i = 0; i < l.getVertexCount(); i++)
                {
                    Vec3 pos = l.getXYZ(i);
                    verts[i].X = (float)pos.getX();
                    verts[i].Y = (float)pos.getY();
                    verts[i].Z = (float)pos.getZ();
                    Vec3 normal;
                    l.calcTriangleNormal(i/3,out normal);
                    verts[i].Nx = (float)normal.getX();
                    verts[i].Ny = (float)normal.getY();
                    verts[i].Nz = (float)normal.getZ();
                    verts[i].Color = Color.White.ToArgb();
                }
                stm.Write(verts);
                vertexBuffer.Unlock();
            }

            public void drawBuffer(Device device)
            {
                // used vertices has only position and color (no texture coordinates)
                device.VertexFormat = CustomVertex.PositionNormalColored.Format;
                // draw example vertex buffer (single triangle)
                device.SetStreamSource(0, vertexBuffer, 0);
                device.DrawPrimitives(PrimitiveType.TriangleList, 0, primitiveCount);
            }
        }



        public FormObjViewer()
        {
            InitializeComponent();
            camera = new Camera();
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
                device = new Device(0, DeviceType.Hardware, handle, CreateFlags.SoftwareVertexProcessing, presentParams);


                //MessageBox.Show("DirectX device for PictureBox successfuly initialized. Click button to draw the triangle.");
            }
            catch (DirectXException e)
            {
                MessageBox.Show("DirectX exception caught during init! "+e.ToString());
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

                device.Transform.Projection = Matrix.PerspectiveFovLH((float)Math.PI / 4, pictureBox1.Width / pictureBox1.Height, 1f, 500f);

                Vector3 at;
                at.X =(float) camera.getPosition().getX();
                at.Y = (float)camera.getPosition().getY();
                at.Z = (float)camera.getPosition().getZ();
                device.Transform.View = Matrix.LookAtLH(at, new Vector3(0,0,0), new Vector3(0, 0, 1));
                
                // drawing primitive without texture
                device.SetTexture(0, null);

                if (vertexBuffer != null)
                {
                    vertexBuffer.drawBuffer(device);
                }

                device.EndScene();
                device.Present();

            }
            catch (DirectXException e)
            {
                MessageBox.Show("DirectX exception caught during init! " + e.ToString());
            }
        }
        private void FormObjViewer_Load(object sender, EventArgs e)
        {
            InitializeDirectX(pictureBox1);
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

        private void recreateGPUBuffers()
        {
            if (model == null)
            {
                vertexBuffer = null;
                return;
            }
            XYZTrianglesList l = new XYZTrianglesList();
            model.iterateXYZTriangles(l);

            vertexBuffer = new DXVertexBufferXYZNormals();
            vertexBuffer.create(l, device);
        }
        private void loadModel(string name)
        {
            model = new WavefrontOBJ();
            if (model.loadObjModel(name))
            {
                model = null;
            }
            recreateGPUBuffers();
            UpdateDirectXDisplay();
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            UpdateDirectXDisplay();
        }

        private void pictureBox1_SizeChanged(object sender, EventArgs e)
        {
            recreateGPUBuffers();
            UpdateDirectXDisplay();
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            bMouseDown = false;
        }

        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
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

        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            bMouseDown = true;
            mouseButton = e.Button;
            mouseX = e.X;
            mouseY = e.Y;
        }
    }
}
