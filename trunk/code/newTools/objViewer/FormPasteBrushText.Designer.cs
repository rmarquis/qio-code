namespace objViewer
{
    partial class FormPasteBrushText
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
            this.tb_brushText = new System.Windows.Forms.TextBox();
            this.bt_importBrush = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tb_brushText
            // 
            this.tb_brushText.Dock = System.Windows.Forms.DockStyle.Top;
            this.tb_brushText.Location = new System.Drawing.Point(0, 0);
            this.tb_brushText.Multiline = true;
            this.tb_brushText.Name = "tb_brushText";
            this.tb_brushText.Size = new System.Drawing.Size(284, 223);
            this.tb_brushText.TabIndex = 0;
            // 
            // bt_importBrush
            // 
            this.bt_importBrush.Location = new System.Drawing.Point(181, 230);
            this.bt_importBrush.Name = "bt_importBrush";
            this.bt_importBrush.Size = new System.Drawing.Size(75, 23);
            this.bt_importBrush.TabIndex = 1;
            this.bt_importBrush.Text = "Import brush";
            this.bt_importBrush.UseVisualStyleBackColor = true;
            this.bt_importBrush.Click += new System.EventHandler(this.bt_importBrush_Click);
            // 
            // FormPasteBrushText
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.bt_importBrush);
            this.Controls.Add(this.tb_brushText);
            this.Name = "FormPasteBrushText";
            this.Text = "FormPasteBrushText";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tb_brushText;
        private System.Windows.Forms.Button bt_importBrush;
    }
}