namespace md5Viewer
{
    partial class FormMD5Viewer
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
            this.pb_3dView = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openModelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openAnimationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lb_md5MeshName = new System.Windows.Forms.Label();
            this.lb_md5AnimName = new System.Windows.Forms.Label();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.trackBarFrame = new System.Windows.Forms.TrackBar();
            this.lb_animInfo = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pb_3dView)).BeginInit();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarFrame)).BeginInit();
            this.SuspendLayout();
            // 
            // pb_3dView
            // 
            this.pb_3dView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pb_3dView.Location = new System.Drawing.Point(12, 27);
            this.pb_3dView.Name = "pb_3dView";
            this.pb_3dView.Size = new System.Drawing.Size(500, 239);
            this.pb_3dView.TabIndex = 0;
            this.pb_3dView.TabStop = false;
            this.pb_3dView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pb_3dView_MouseDown);
            this.pb_3dView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pb_3dView_MouseMove);
            this.pb_3dView.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pb_3dView_MouseUp);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 278);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(58, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "MD5mesh:";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 301);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(55, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "MD5anim:";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(524, 24);
            this.menuStrip1.TabIndex = 6;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openModelToolStripMenuItem,
            this.openAnimationToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openModelToolStripMenuItem
            // 
            this.openModelToolStripMenuItem.Name = "openModelToolStripMenuItem";
            this.openModelToolStripMenuItem.Size = new System.Drawing.Size(169, 22);
            this.openModelToolStripMenuItem.Text = "Open model...";
            this.openModelToolStripMenuItem.Click += new System.EventHandler(this.openModelToolStripMenuItem_Click);
            // 
            // openAnimationToolStripMenuItem
            // 
            this.openAnimationToolStripMenuItem.Name = "openAnimationToolStripMenuItem";
            this.openAnimationToolStripMenuItem.Size = new System.Drawing.Size(169, 22);
            this.openAnimationToolStripMenuItem.Text = "Open animation...";
            this.openAnimationToolStripMenuItem.Click += new System.EventHandler(this.openAnimationToolStripMenuItem_Click);
            // 
            // lb_md5MeshName
            // 
            this.lb_md5MeshName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lb_md5MeshName.AutoSize = true;
            this.lb_md5MeshName.Location = new System.Drawing.Point(67, 278);
            this.lb_md5MeshName.Name = "lb_md5MeshName";
            this.lb_md5MeshName.Size = new System.Drawing.Size(31, 13);
            this.lb_md5MeshName.TabIndex = 7;
            this.lb_md5MeshName.Text = "none";
            // 
            // lb_md5AnimName
            // 
            this.lb_md5AnimName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lb_md5AnimName.AutoSize = true;
            this.lb_md5AnimName.Location = new System.Drawing.Point(67, 301);
            this.lb_md5AnimName.Name = "lb_md5AnimName";
            this.lb_md5AnimName.Size = new System.Drawing.Size(31, 13);
            this.lb_md5AnimName.TabIndex = 8;
            this.lb_md5AnimName.Text = "none";
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // trackBarFrame
            // 
            this.trackBarFrame.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.trackBarFrame.Enabled = false;
            this.trackBarFrame.Location = new System.Drawing.Point(12, 341);
            this.trackBarFrame.Name = "trackBarFrame";
            this.trackBarFrame.Size = new System.Drawing.Size(500, 43);
            this.trackBarFrame.TabIndex = 9;
            this.trackBarFrame.Scroll += new System.EventHandler(this.trackBarFrame_Scroll);
            // 
            // lb_animInfo
            // 
            this.lb_animInfo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lb_animInfo.AutoSize = true;
            this.lb_animInfo.Location = new System.Drawing.Point(12, 325);
            this.lb_animInfo.Name = "lb_animInfo";
            this.lb_animInfo.Size = new System.Drawing.Size(107, 13);
            this.lb_animInfo.TabIndex = 10;
            this.lb_animInfo.Text = "No animation loaded.";
            // 
            // FormMD5Viewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(524, 382);
            this.Controls.Add(this.lb_animInfo);
            this.Controls.Add(this.trackBarFrame);
            this.Controls.Add(this.lb_md5AnimName);
            this.Controls.Add(this.lb_md5MeshName);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.pb_3dView);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "FormMD5Viewer";
            this.Text = "MD5 model viewer";
            this.Load += new System.EventHandler(this.FormMD5Viewer_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pb_3dView)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarFrame)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pb_3dView;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openModelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openAnimationToolStripMenuItem;
        private System.Windows.Forms.Label lb_md5MeshName;
        private System.Windows.Forms.Label lb_md5AnimName;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TrackBar trackBarFrame;
        private System.Windows.Forms.Label lb_animInfo;
    }
}

