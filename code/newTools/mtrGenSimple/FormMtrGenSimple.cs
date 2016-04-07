using shared;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace mtrGenSimple
{
    public partial class FormMtrGenSimple : Form
    {
        private bool validData;
        private string path;
        private Image image;
        private Thread getImageThread;
        private PictureBox curPb;
        private Random r = new Random();

        public FormMtrGenSimple()
        {
            InitializeComponent();
        }

        private void StartImageLoading(String filename)
        {
            path = filename;
            getImageThread = new Thread(new ThreadStart(LoadImage));
            getImageThread.Start();
        }
        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            if (curPb == null)
                return;
            string filename;
            validData = GetFilename(out filename, e);
            if (validData)
            {
                StartImageLoading(filename);
                e.Effect = DragDropEffects.Copy;
            }
            else
                e.Effect = DragDropEffects.None;
        }
        private bool GetFilename(out string filename, DragEventArgs e) 
        {
            bool ret = false;
            filename = String.Empty;
            if ((e.AllowedEffect & DragDropEffects.Copy) == DragDropEffects.Copy)
            {
                Array data = ((IDataObject)e.Data).GetData("FileDrop") as Array;
                if (data != null)
                {
                    if ((data.Length == 1) && (data.GetValue(0) is String))
                    {
                        filename = ((string[])data)[0];
                        string ext = Path.GetExtension(filename).ToLower();
                        if ((ext == ".jpg") || (ext == ".png") || (ext == ".bmp"))
                        {
                            ret = true;
                        }
                    }
                }
            }
            return ret;
        }
        public void Form1_DragDrop(object sender, DragEventArgs e)
        {
            if (curPb == null)
                return;
            PictureBox savedPb = curPb;
            if (validData)
            {
                while (getImageThread.IsAlive)
                {
                    Application.DoEvents();
                    Thread.Sleep(0);
                }
                curPb.Image = image;
                curPb.ImageLocation = path;
            }
        }
        protected void LoadImage()
        {
            try
            {
                image = new Bitmap(path);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Failed to load image from path '" + path + "'");
            }
        }

        private void AutoFindBasePath()
        {
            String cur = Directory.GetCurrentDirectory();
            //MessageBox.Show("Current working irectory: " + cur);
            int at = cur.IndexOf("Qio\\");
            if (at == -1)
                return;
            String s = cur.Substring(0, at+4);
            s += "game\\baseqio\\";
            if (Directory.Exists(s) == false)
                return;
            tbBasePath.Text = s;
            //CreateDirs(tbBasePath.Text);
        }
        private void FindMaterialFiles(string path, string ext)
        {
            if (Directory.Exists(path) == false)
                return;
            string [] files = Directory.GetFiles(path, "*."+ext, SearchOption.AllDirectories);
            for (int i = 0; i < files.Length; i++)
            {
                string fullPath = files[i];
                string matFileName = fullPath.Substring(path.Length);
                cbMatFile.Items.Add(matFileName);
            }
            cbMatFile.SelectedIndex = 0;
        }
        class MtrFile
        {

            public MtrFile(String fname)
            {
                Parser p = new Parser();
                p.beginParsingFile(fname);
                while (p.isAtEOF() == false)
                {
                    if (p.isAtToken("table"))
                    {
                        string tabName;
                        p.readString(out tabName);
                    }
                    else
                    {
                        string matName;
                        p.readString(out matName);

                    }
                }
            }
        }
        private void PrecacheMaterialFiles()
        {
            for (int i = 0; i < cbMatFile.Items.Count; i++)
            {
                String name = cbMatFile.Items[i].ToString();
                String path = getAbsoluteMaterialFilePath(name);
                MtrFile mf;
                mf = new MtrFile(path);
            }
        }
        // All cases should be supported:
        // C:/Qio/baseqio + textures/test = C:/Qio/baseqio/textures/test
        // C:/Qio/baseqio/ + textures/test = C:/Qio/baseqio/textures/test
        // C:/Qio/baseqio/ + /textures/test = C:/Qio/baseqio/textures/test
        // C:/Qio/baseqio + /textures/test = C:/Qio/baseqio/textures/test
        private string MergePaths(String a, String b)
        {
            // remove slash from b
            if (b.Length != 0)
            {
                if (b[0] == '/' || b[0] == '\\')
                    b = b.Substring(1);
            }
            if (a.Length != 0)
            {
                // add to a
                if (a[a.Length - 1] != '/' && a[a.Length - 1] != '\\')
                {
                    a += "/";
                }
            }
            return a + b;
        }
        private void FindMaterialFiles()
        {
            cbMatFile.SelectedIndex = -1;
            cbMatFile.Items.Clear();
            FindMaterialFiles(MergePaths(tbBasePath.Text,"scripts\\"), "shader");
            FindMaterialFiles(MergePaths(tbBasePath.Text,"materials\\"), "mtr");
            PrecacheMaterialFiles();
        }
        private void FillImageTypes(ComboBox cb)
        {
            cb.Items.Clear();
            cb.Items.Add("DiffuseMap");
            cb.Items.Add("NormalMap");
            cb.Items.Add("SpecularMap");
            cb.Items.Add("HeightMap");
            cb.SelectedIndex = 0;
        }
        static void SendUdp(int srcPort, string dstIp, int dstPort, byte[] data)
        {
            using (UdpClient c = new UdpClient(srcPort))
                c.Send(data, data.Length, dstIp, dstPort);
        }
        private void sendCommandToGame(string command)
        {
            MemoryStream m = new MemoryStream();
            m.WriteByte(0xff);
            m.WriteByte(0xff);
            m.WriteByte(0xff);
            m.WriteByte(0xff);
            byte[] data = Encoding.ASCII.GetBytes("stufftext " + command);
            m.Write(data,0,data.Length);
            SendUdp(22000, "127.0.0.1", 27960, m.GetBuffer());
            //Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram,
            //ProtocolType.Udp);

            //IPAddress serverAddr = IPAddress.Parse("127.0.0.1");

            //IPEndPoint endPoint = new IPEndPoint(serverAddr, 11000);

            //string text = "Hello";
            //byte[] send_buffer = Encoding.ASCII.GetBytes(text);

            //sock.SendTo(send_buffer, endPoint);


        }
        void setBasePath(String s)
        {
            tbBasePath.Text = s;
            FindMaterialFiles();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            AutoFindBasePath();
            FindMaterialFiles();

            FillImageTypes(cbType1);
            FillImageTypes(cbType2);
            FillImageTypes(cbType3);
            cbType1.SelectedIndex = 0;
            cbType2.SelectedIndex = 1;
            cbType3.SelectedIndex = 2;
            tbMatName.Text = "textures/mtrGenTests/testMat" + r.Next() % 100000 + "" + r.Next() % 100000;

            // test
           // sendCommandToGame("devmap test_physics");
        }

        private void pictureBox1_MouseEnter(object sender, EventArgs e)
        {
            curPb = pictureBox1;
        }

        private void pictureBox2_MouseEnter(object sender, EventArgs e)
        {
            curPb = pictureBox2;
        }

        private void pictureBox3_MouseEnter(object sender, EventArgs e)
        {
            curPb = pictureBox3;
        }
        private int FirstSlash(string s, int ofs)
        {
            int a = s.IndexOf('/', ofs);
            int b = s.IndexOf('\\', ofs);
            if (a == -1)
                return b;
            if (b == -1)
                return a;
            if (a < b)
                return a;
            return b;
        }
        private void CreateDirs(string s)
        {
            int at = 0;
            while (true)
            {
                at = FirstSlash(s, at);
                if (at < 0)
                    break;
                string dir = s.Substring(0, at);
                if (Directory.Exists(dir) == false)
                {
                    Directory.CreateDirectory(dir);
                }
                at++;
            }
        }
        private string FilePath2DirPath(string s)
        {
            int at = 0;
            int last = -1;
            while (true)
            {
                at = FirstSlash(s, at);
                if (at < 0)
                    break;
                last = at;
                at++;
            }
            if (last != -1)
            {
                return s.Substring(0, last+1);
            }
            return s;
        }
        enum MaterialImageRole
        {
            DIFFUSE,
            NORMAL,
            SPECULAR,
            HEIGHT,
        }
        class MaterialImage
        {
            public MaterialImageRole role;
            public string sourcePath;
            public string targetPath;
        }
        private MaterialImageRole Combo2Role(ComboBox cb)
        {
            if (cb.SelectedValue == null)
                return MaterialImageRole.DIFFUSE;
            String s = cb.SelectedValue.ToString().ToLower();
            if(s.IndexOf("diffuse")!=-1 || s.IndexOf("color") != -1)
                return MaterialImageRole.DIFFUSE;
            if(s.IndexOf("specular")!=-1)
                return MaterialImageRole.SPECULAR;
            if(s.IndexOf("normal")!=-1 || s.IndexOf("bump") != -1)
                return MaterialImageRole.NORMAL;
            if (s.IndexOf("height") != -1)
                return MaterialImageRole.HEIGHT;
            return MaterialImageRole.DIFFUSE;
        }
        private MaterialImage getPictureBoxImage(PictureBox pb,ComboBox imgType)
        {
            if (pb.Image == null)
                return null;
            MaterialImage mi = new MaterialImage();
            mi.role = Combo2Role(imgType); ;
            mi.sourcePath = pb.ImageLocation;
            return mi;
        }
        private MaterialImage findImageOfRole(List<MaterialImage> l, MaterialImageRole r)
        {
            foreach (MaterialImage mi in l)
            {
                if (mi.role == r)
                    return mi;
            }
            return null;
        }
        private string getCurrentMatFileComboText()
        {
            if (cbMatFile.SelectedItem != null)
            {
                return cbMatFile.SelectedItem.ToString(); ;
            }
            return cbMatFile.Text;
        }
        private string getAbsoluteMaterialFilePath(String name)
        {
            // dirty solution
            String ret = "";
            string[] dirs = { "scripts/", "materials/" };
            for (int i = 0; i < 2; i++)
            {
                ret = MergePaths(tbBasePath.Text,dirs[i]);
                ret += name;
                if (File.Exists(ret))
                    return ret;
            }
            return ret;
        }
        private string getCurrentMatFileNamePath()
        {
            String name = getCurrentMatFileComboText();
            return getAbsoluteMaterialFilePath(name); ;
        }
        private string getLocalPath(string s)
        {
            return s.Substring(tbBasePath.Text.Length);
        }
        private string getCurDateTimeStringForFileName()
        {
            String dateStr = DateTime.Now.ToString("yyyy-M-d_HH-mm-ss");
            return dateStr;
        }
        private void button2_Click(object sender, EventArgs e)
        {
            // for material name like "textures/testGen/testMat123"
            // we want to create:
            // 1. "textures/" dir if not exists
            // 2. "textures/testgen/" dir if not exists
            // 3. textures/testgen/testMat123.png (diffuse map)
            // 4. textures/testgen/testMat123_n.png (normal map)
            // 5. textures/testgen/testMat123_s.png (specular map)
            // 6. textures/testgen/testMat123_h.png (height map)
            // 7. material text inside .mtr file

            string mtrFile = getCurrentMatFileNamePath();
            String dateStr = getCurDateTimeStringForFileName();

            if (File.Exists(mtrFile))
            {
                // backup material file first
                String bpName = mtrFile + dateStr + ".bak";
                try
                {
                    File.Copy(mtrFile, bpName);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Failed to create material file backup ('" + bpName + "'). Aborting operation...");
                    return;
                }
            }

            // create path for images if not exist
            string matDir = FilePath2DirPath(tbMatName.Text);
            string fullPath = MergePaths(tbBasePath.Text,matDir);
            CreateDirs(fullPath);
            // copy and rename images
            List<MaterialImage> images = new List<MaterialImage>();
            MaterialImage img = getPictureBoxImage(pictureBox1,cbType1);
            if (img != null)
            {
                images.Add(img);
            }
            img = getPictureBoxImage(pictureBox2,cbType2);
            if (img != null)
            {
                images.Add(img);
            }
            img = getPictureBoxImage(pictureBox3,cbType3);
            if (img != null)
            {
                images.Add(img);
            }
         //   img = getPictureBox3Image();
           // if (img != null)
            {
             //   images.Add(img);
            }

            // convert or copy images
            foreach (MaterialImage mi in images)
            {
                String suffix;
                switch (mi.role)
                {
                    case MaterialImageRole.DIFFUSE:
                    {
                        suffix = "";
                    }
                    break;
                    case MaterialImageRole.NORMAL:
                    {
                        suffix = "_n";
                    }
                    break;
                    case MaterialImageRole.HEIGHT:
                    {
                        suffix = "_h";
                    }
                    break;
                    case MaterialImageRole.SPECULAR:
                    {
                        suffix = "_s";
                    }
                    break;
                    default:
                        {
                            suffix="_error"; // TODO
                        }
                        break;
                }
                string ext = Path.GetExtension(mi.sourcePath);
                mi.targetPath = MergePaths(tbBasePath.Text,tbMatName.Text) + suffix + ext;
                if (IsURL(mi.sourcePath))
                {
                    try
                    {
                        using (WebClient wc = new WebClient())
                            wc.DownloadFile(mi.sourcePath, mi.targetPath);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Failed to download '" + mi.sourcePath + "' to '" + mi.targetPath + "'");
                        return; // abort
                    }
                }
                else
                {
                    try
                    {
                        File.Copy(mi.sourcePath, mi.targetPath);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Failed to copy '" + mi.sourcePath + "' to '" + mi.targetPath + "'");
                        return; // abort
                    }
                }
            }

            

            MaterialImage diffuseMap = findImageOfRole(images, MaterialImageRole.DIFFUSE);
            MaterialImage specularMap = findImageOfRole(images, MaterialImageRole.SPECULAR);
            MaterialImage normalMap = findImageOfRole(images, MaterialImageRole.NORMAL);
            MaterialImage heightMap = findImageOfRole(images, MaterialImageRole.HEIGHT);

            string mtrText = tbMatName.Text + Environment.NewLine;
            mtrText += "{" + Environment.NewLine;
            mtrText += "\t// generated on " + dateStr + Environment.NewLine;
            if (diffuseMap != null)
            {
                mtrText += "\tdiffuseMap " + getLocalPath(diffuseMap.targetPath) + Environment.NewLine;
            }
            if (specularMap != null)
            {
                mtrText += "\tspecularMap " + getLocalPath(specularMap.targetPath) + Environment.NewLine;
            }
            if (specularMap != null)
            {
                mtrText += "\tnormalMap " + getLocalPath(normalMap.targetPath) + Environment.NewLine;
            }
            if (specularMap != null)
            {
                mtrText += "\theightMap " + getLocalPath(heightMap.targetPath) + Environment.NewLine;
            }
            mtrText += "}" + Environment.NewLine;

            // append text
            using (StreamWriter sw = File.AppendText(mtrFile))
            {
                sw.Write(mtrText);
            }
            MessageBox.Show("Generated text " + mtrText);



            MessageBox.Show("Successfully added new material text to " + mtrFile);

            sendCommandToGame("mat_refreshMaterialSourceFile " + getCurrentMatFileComboText());
            sendCommandToGame("cg_testMaterial " + tbMatName.Text);

        }

        private void btViewRawMtrFile_Click(object sender, EventArgs e)
        {
            String fname = getCurrentMatFileNamePath();
            Process.Start(fname);
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {

        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                contextMenuStrip1.Show(pictureBox1, e.Location);
            }
        }
        private bool IsURL(String s)
        {
            Uri uriResult;
            bool result = Uri.TryCreate(s, UriKind.Absolute, out uriResult)
                && (uriResult.Scheme == Uri.UriSchemeHttp || uriResult.Scheme == Uri.UriSchemeHttps);
            return result;
        }
        private void pasteImage(PictureBox pb)
        {
            if (Clipboard.ContainsText(TextDataFormat.Text))
            {
                String txt = Clipboard.GetText(TextDataFormat.Text);
                bool result = IsURL(txt);
                if (result)
                {
                    pb.ImageLocation = txt;
                }
                else
                {
                    StartImageLoading(txt);
                }
            }
            else if (Clipboard.ContainsImage())
            {
                pb.Image = Clipboard.GetImage();
                // also save in temp path
                String p = Path.GetTempPath();
                p += "clipboardImage" + getCurDateTimeStringForFileName() + " " + r.Next() % 1000 + ".png";
                pb.Image.Save(p);
                pb.ImageLocation = p;
            }
        }
        private void pasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            pasteImage(curPb);
        }

        private void pictureBox2_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                contextMenuStrip1.Show(pictureBox2, e.Location);
            }
        }

        private void pictureBox3_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                contextMenuStrip1.Show(pictureBox3, e.Location);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            DialogResult result = fbd.ShowDialog();
            if (result == System.Windows.Forms.DialogResult.OK)
            {
                setBasePath(fbd.SelectedPath);
            }
        }
    }
}
