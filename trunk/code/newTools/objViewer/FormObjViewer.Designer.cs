namespace objViewer
{
    partial class FormObjViewer
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.importToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mD3ModelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mD5ModelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mAPFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.singleBrushToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.swapTrianglesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.swapYZToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scaleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.mD5ModelAnimationPoseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox1.Location = new System.Drawing.Point(12, 27);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(260, 223);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.SizeChanged += new System.EventHandler(this.pictureBox1_SizeChanged);
            this.pictureBox1.Click += new System.EventHandler(this.pictureBox1_Click);
            this.pictureBox1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseDown);
            this.pictureBox1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseMove);
            this.pictureBox1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseUp);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(284, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.importToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.openToolStripMenuItem.Text = "Open OBJ...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.saveToolStripMenuItem.Text = "Save OBJ..";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // importToolStripMenuItem
            // 
            this.importToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mD3ModelToolStripMenuItem,
            this.mD5ModelToolStripMenuItem,
            this.mD5ModelAnimationPoseToolStripMenuItem,
            this.mAPFileToolStripMenuItem,
            this.singleBrushToolStripMenuItem});
            this.importToolStripMenuItem.Name = "importToolStripMenuItem";
            this.importToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.importToolStripMenuItem.Text = "Import";
            // 
            // mD3ModelToolStripMenuItem
            // 
            this.mD3ModelToolStripMenuItem.Name = "mD3ModelToolStripMenuItem";
            this.mD3ModelToolStripMenuItem.Size = new System.Drawing.Size(230, 22);
            this.mD3ModelToolStripMenuItem.Text = "MD3 model..";
            this.mD3ModelToolStripMenuItem.Click += new System.EventHandler(this.mD3ModelToolStripMenuItem_Click);
            // 
            // mD5ModelToolStripMenuItem
            // 
            this.mD5ModelToolStripMenuItem.Name = "mD5ModelToolStripMenuItem";
            this.mD5ModelToolStripMenuItem.Size = new System.Drawing.Size(230, 22);
            this.mD5ModelToolStripMenuItem.Text = "MD5 model basepose...";
            this.mD5ModelToolStripMenuItem.Click += new System.EventHandler(this.mD5ModelToolStripMenuItem_Click);
            // 
            // mAPFileToolStripMenuItem
            // 
            this.mAPFileToolStripMenuItem.Name = "mAPFileToolStripMenuItem";
            this.mAPFileToolStripMenuItem.Size = new System.Drawing.Size(230, 22);
            this.mAPFileToolStripMenuItem.Text = "MAP file...";
            this.mAPFileToolStripMenuItem.Click += new System.EventHandler(this.mAPFileToolStripMenuItem_Click);
            // 
            // singleBrushToolStripMenuItem
            // 
            this.singleBrushToolStripMenuItem.Name = "singleBrushToolStripMenuItem";
            this.singleBrushToolStripMenuItem.Size = new System.Drawing.Size(230, 22);
            this.singleBrushToolStripMenuItem.Text = "Single brush...";
            this.singleBrushToolStripMenuItem.Click += new System.EventHandler(this.singleBrushToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.swapTrianglesToolStripMenuItem,
            this.swapYZToolStripMenuItem,
            this.scaleToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // swapTrianglesToolStripMenuItem
            // 
            this.swapTrianglesToolStripMenuItem.Name = "swapTrianglesToolStripMenuItem";
            this.swapTrianglesToolStripMenuItem.Size = new System.Drawing.Size(150, 22);
            this.swapTrianglesToolStripMenuItem.Text = "Swap triangles";
            this.swapTrianglesToolStripMenuItem.Click += new System.EventHandler(this.swapTrianglesToolStripMenuItem_Click);
            // 
            // swapYZToolStripMenuItem
            // 
            this.swapYZToolStripMenuItem.Name = "swapYZToolStripMenuItem";
            this.swapYZToolStripMenuItem.Size = new System.Drawing.Size(150, 22);
            this.swapYZToolStripMenuItem.Text = "Swap YZ";
            this.swapYZToolStripMenuItem.Click += new System.EventHandler(this.swapYZToolStripMenuItem_Click);
            // 
            // scaleToolStripMenuItem
            // 
            this.scaleToolStripMenuItem.Name = "scaleToolStripMenuItem";
            this.scaleToolStripMenuItem.Size = new System.Drawing.Size(150, 22);
            this.scaleToolStripMenuItem.Text = "Scale...";
            this.scaleToolStripMenuItem.Click += new System.EventHandler(this.scaleToolStripMenuItem_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // mD5ModelAnimationPoseToolStripMenuItem
            // 
            this.mD5ModelAnimationPoseToolStripMenuItem.Name = "mD5ModelAnimationPoseToolStripMenuItem";
            this.mD5ModelAnimationPoseToolStripMenuItem.Size = new System.Drawing.Size(230, 22);
            this.mD5ModelAnimationPoseToolStripMenuItem.Text = "MD5 model animation pose...";
            this.mD5ModelAnimationPoseToolStripMenuItem.Click += new System.EventHandler(this.mD5ModelAnimationPoseToolStripMenuItem_Click);
            // 
            // FormObjViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "FormObjViewer";
            this.Text = "OBJ Model Viewer";
            this.Load += new System.EventHandler(this.FormObjViewer_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem swapTrianglesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem swapYZToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.ToolStripMenuItem scaleToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem importToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mD3ModelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mD5ModelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem singleBrushToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mAPFileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mD5ModelAnimationPoseToolStripMenuItem;
    }
}

