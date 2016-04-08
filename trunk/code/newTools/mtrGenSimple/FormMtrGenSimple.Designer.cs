namespace mtrGenSimple
{
    partial class FormMtrGenSimple
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
            this.components = new System.ComponentModel.Container();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbBasePath = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.tbMatName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.cbMatFile = new System.Windows.Forms.ComboBox();
            this.button2 = new System.Windows.Forms.Button();
            this.cbType1 = new System.Windows.Forms.ComboBox();
            this.cbType2 = new System.Windows.Forms.ComboBox();
            this.cbType3 = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.btViewRawMtrFile = new System.Windows.Forms.Button();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.pasteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.btUpdateExistingMaterial = new System.Windows.Forms.Button();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.cbMaterialsFromMatFile = new System.Windows.Forms.ComboBox();
            this.tbMaterialText = new System.Windows.Forms.TextBox();
            this.tbSaveChanges = new System.Windows.Forms.Button();
            this.btPreviewChanges = new System.Windows.Forms.Button();
            this.btViewCreatedMaterialInQio = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            this.contextMenuStrip1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ButtonShadow;
            this.pictureBox1.Location = new System.Drawing.Point(4, 72);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(128, 128);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Click += new System.EventHandler(this.pictureBox1_Click);
            this.pictureBox1.MouseClick += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseClick);
            this.pictureBox1.MouseEnter += new System.EventHandler(this.pictureBox1_MouseEnter);
            // 
            // pictureBox2
            // 
            this.pictureBox2.BackColor = System.Drawing.SystemColors.ButtonShadow;
            this.pictureBox2.Location = new System.Drawing.Point(138, 72);
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.Size = new System.Drawing.Size(128, 128);
            this.pictureBox2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox2.TabIndex = 1;
            this.pictureBox2.TabStop = false;
            this.pictureBox2.MouseClick += new System.Windows.Forms.MouseEventHandler(this.pictureBox2_MouseClick);
            this.pictureBox2.MouseEnter += new System.EventHandler(this.pictureBox2_MouseEnter);
            // 
            // pictureBox3
            // 
            this.pictureBox3.BackColor = System.Drawing.SystemColors.ButtonShadow;
            this.pictureBox3.Location = new System.Drawing.Point(272, 72);
            this.pictureBox3.Name = "pictureBox3";
            this.pictureBox3.Size = new System.Drawing.Size(128, 128);
            this.pictureBox3.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox3.TabIndex = 2;
            this.pictureBox3.TabStop = false;
            this.pictureBox3.MouseClick += new System.Windows.Forms.MouseEventHandler(this.pictureBox3_MouseClick);
            this.pictureBox3.MouseEnter += new System.EventHandler(this.pictureBox3_MouseEnter);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(55, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Basepath:";
            // 
            // tbBasePath
            // 
            this.tbBasePath.Location = new System.Drawing.Point(71, 12);
            this.tbBasePath.Name = "tbBasePath";
            this.tbBasePath.Size = new System.Drawing.Size(274, 20);
            this.tbBasePath.TabIndex = 4;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(352, 9);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(61, 23);
            this.button1.TabIndex = 5;
            this.button1.Text = "Change";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 7);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(57, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Mat name:";
            // 
            // tbMatName
            // 
            this.tbMatName.Location = new System.Drawing.Point(69, 4);
            this.tbMatName.Name = "tbMatName";
            this.tbMatName.Size = new System.Drawing.Size(270, 20);
            this.tbMatName.TabIndex = 7;
            this.tbMatName.Text = "textures/mtrGenTests/test123";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 39);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "Mat file:";
            // 
            // cbMatFile
            // 
            this.cbMatFile.FormattingEnabled = true;
            this.cbMatFile.Location = new System.Drawing.Point(71, 39);
            this.cbMatFile.Name = "cbMatFile";
            this.cbMatFile.Size = new System.Drawing.Size(274, 21);
            this.cbMatFile.TabIndex = 9;
            this.cbMatFile.SelectedIndexChanged += new System.EventHandler(this.cbMatFile_SelectedIndexChanged);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(4, 236);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(192, 36);
            this.button2.TabIndex = 10;
            this.button2.Text = "Copy/convert image files and create material text.";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // cbType1
            // 
            this.cbType1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbType1.FormattingEnabled = true;
            this.cbType1.Location = new System.Drawing.Point(7, 207);
            this.cbType1.Name = "cbType1";
            this.cbType1.Size = new System.Drawing.Size(121, 21);
            this.cbType1.TabIndex = 11;
            // 
            // cbType2
            // 
            this.cbType2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbType2.FormattingEnabled = true;
            this.cbType2.Location = new System.Drawing.Point(138, 206);
            this.cbType2.Name = "cbType2";
            this.cbType2.Size = new System.Drawing.Size(121, 21);
            this.cbType2.TabIndex = 12;
            // 
            // cbType3
            // 
            this.cbType3.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbType3.FormattingEnabled = true;
            this.cbType3.Location = new System.Drawing.Point(272, 206);
            this.cbType3.Name = "cbType3";
            this.cbType3.Size = new System.Drawing.Size(121, 21);
            this.cbType3.TabIndex = 13;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(7, 34);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(366, 26);
            this.label4.TabIndex = 14;
            this.label4.Text = "Drag and drop image files here. NOTE: Do not put them in baseqio directory.\r\nTool" +
    " will automatically copy and rename them.";
            // 
            // btViewRawMtrFile
            // 
            this.btViewRawMtrFile.Location = new System.Drawing.Point(352, 39);
            this.btViewRawMtrFile.Name = "btViewRawMtrFile";
            this.btViewRawMtrFile.Size = new System.Drawing.Size(61, 23);
            this.btViewRawMtrFile.TabIndex = 15;
            this.btViewRawMtrFile.Text = "View raw";
            this.btViewRawMtrFile.UseVisualStyleBackColor = true;
            this.btViewRawMtrFile.Click += new System.EventHandler(this.btViewRawMtrFile_Click);
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.pasteToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(230, 26);
            this.contextMenuStrip1.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip1_Opening);
            // 
            // pasteToolStripMenuItem
            // 
            this.pasteToolStripMenuItem.Name = "pasteToolStripMenuItem";
            this.pasteToolStripMenuItem.Size = new System.Drawing.Size(229, 22);
            this.pasteToolStripMenuItem.Text = "Paste (from path or web URL)";
            this.pasteToolStripMenuItem.Click += new System.EventHandler(this.pasteToolStripMenuItem_Click);
            // 
            // btUpdateExistingMaterial
            // 
            this.btUpdateExistingMaterial.Location = new System.Drawing.Point(201, 236);
            this.btUpdateExistingMaterial.Name = "btUpdateExistingMaterial";
            this.btUpdateExistingMaterial.Size = new System.Drawing.Size(192, 36);
            this.btUpdateExistingMaterial.TabIndex = 16;
            this.btUpdateExistingMaterial.Text = "Update existing material";
            this.btUpdateExistingMaterial.UseVisualStyleBackColor = true;
            this.btUpdateExistingMaterial.Click += new System.EventHandler(this.btUpdateExistingMaterial_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Location = new System.Drawing.Point(15, 68);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(426, 342);
            this.tabControl1.TabIndex = 17;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.btViewCreatedMaterialInQio);
            this.tabPage1.Controls.Add(this.label2);
            this.tabPage1.Controls.Add(this.pictureBox1);
            this.tabPage1.Controls.Add(this.btUpdateExistingMaterial);
            this.tabPage1.Controls.Add(this.pictureBox2);
            this.tabPage1.Controls.Add(this.pictureBox3);
            this.tabPage1.Controls.Add(this.label4);
            this.tabPage1.Controls.Add(this.tbMatName);
            this.tabPage1.Controls.Add(this.cbType3);
            this.tabPage1.Controls.Add(this.button2);
            this.tabPage1.Controls.Add(this.cbType2);
            this.tabPage1.Controls.Add(this.cbType1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(418, 316);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Create...";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.btPreviewChanges);
            this.tabPage2.Controls.Add(this.tbSaveChanges);
            this.tabPage2.Controls.Add(this.tbMaterialText);
            this.tabPage2.Controls.Add(this.cbMaterialsFromMatFile);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(418, 316);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Edit...";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // cbMaterialsFromMatFile
            // 
            this.cbMaterialsFromMatFile.FormattingEnabled = true;
            this.cbMaterialsFromMatFile.Location = new System.Drawing.Point(7, 7);
            this.cbMaterialsFromMatFile.Name = "cbMaterialsFromMatFile";
            this.cbMaterialsFromMatFile.Size = new System.Drawing.Size(405, 21);
            this.cbMaterialsFromMatFile.TabIndex = 0;
            this.cbMaterialsFromMatFile.SelectedIndexChanged += new System.EventHandler(this.cbMaterialsFromMatFile_SelectedIndexChanged);
            // 
            // tbMaterialText
            // 
            this.tbMaterialText.Location = new System.Drawing.Point(7, 35);
            this.tbMaterialText.Multiline = true;
            this.tbMaterialText.Name = "tbMaterialText";
            this.tbMaterialText.Size = new System.Drawing.Size(405, 228);
            this.tbMaterialText.TabIndex = 1;
            // 
            // tbSaveChanges
            // 
            this.tbSaveChanges.Location = new System.Drawing.Point(7, 269);
            this.tbSaveChanges.Name = "tbSaveChanges";
            this.tbSaveChanges.Size = new System.Drawing.Size(90, 23);
            this.tbSaveChanges.TabIndex = 2;
            this.tbSaveChanges.Text = "Save changes";
            this.tbSaveChanges.UseVisualStyleBackColor = true;
            // 
            // btPreviewChanges
            // 
            this.btPreviewChanges.Location = new System.Drawing.Point(103, 269);
            this.btPreviewChanges.Name = "btPreviewChanges";
            this.btPreviewChanges.Size = new System.Drawing.Size(90, 23);
            this.btPreviewChanges.TabIndex = 3;
            this.btPreviewChanges.Text = "Preview in Qio";
            this.btPreviewChanges.UseVisualStyleBackColor = true;
            this.btPreviewChanges.Click += new System.EventHandler(this.btPreviewChanges_Click);
            // 
            // btViewCreatedMaterialInQio
            // 
            this.btViewCreatedMaterialInQio.Location = new System.Drawing.Point(6, 274);
            this.btViewCreatedMaterialInQio.Name = "btViewCreatedMaterialInQio";
            this.btViewCreatedMaterialInQio.Size = new System.Drawing.Size(192, 36);
            this.btViewCreatedMaterialInQio.TabIndex = 17;
            this.btViewCreatedMaterialInQio.Text = "View created material in Qio";
            this.btViewCreatedMaterialInQio.UseVisualStyleBackColor = true;
            this.btViewCreatedMaterialInQio.Click += new System.EventHandler(this.btViewCreatedMaterialInQio_Click);
            // 
            // FormMtrGenSimple
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(451, 422);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.btViewRawMtrFile);
            this.Controls.Add(this.cbMatFile);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.tbBasePath);
            this.Controls.Add(this.label1);
            this.Name = "FormMtrGenSimple";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.Form1_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.Form1_DragEnter);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            this.contextMenuStrip1.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.PictureBox pictureBox3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbBasePath;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbMatName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox cbMatFile;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ComboBox cbType1;
        private System.Windows.Forms.ComboBox cbType2;
        private System.Windows.Forms.ComboBox cbType3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button btViewRawMtrFile;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem pasteToolStripMenuItem;
        private System.Windows.Forms.Button btUpdateExistingMaterial;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.ComboBox cbMaterialsFromMatFile;
        private System.Windows.Forms.TextBox tbMaterialText;
        private System.Windows.Forms.Button tbSaveChanges;
        private System.Windows.Forms.Button btPreviewChanges;
        private System.Windows.Forms.Button btViewCreatedMaterialInQio;
    }
}

