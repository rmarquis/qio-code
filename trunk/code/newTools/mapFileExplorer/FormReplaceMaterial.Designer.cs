namespace mapFileExplorer
{
    partial class FormReplaceMaterial
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
            this.tb_replaceWith = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.cb_findWhat = new System.Windows.Forms.ComboBox();
            this.bt_cancel = new System.Windows.Forms.Button();
            this.bt_replaceAll = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tb_replaceWith
            // 
            this.tb_replaceWith.Location = new System.Drawing.Point(13, 74);
            this.tb_replaceWith.Name = "tb_replaceWith";
            this.tb_replaceWith.Size = new System.Drawing.Size(259, 20);
            this.tb_replaceWith.TabIndex = 11;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 57);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(72, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Replace with:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(56, 13);
            this.label1.TabIndex = 9;
            this.label1.Text = "Find what:";
            // 
            // cb_findWhat
            // 
            this.cb_findWhat.FormattingEnabled = true;
            this.cb_findWhat.Location = new System.Drawing.Point(12, 33);
            this.cb_findWhat.Name = "cb_findWhat";
            this.cb_findWhat.Size = new System.Drawing.Size(260, 21);
            this.cb_findWhat.TabIndex = 8;
            // 
            // bt_cancel
            // 
            this.bt_cancel.Location = new System.Drawing.Point(12, 100);
            this.bt_cancel.Name = "bt_cancel";
            this.bt_cancel.Size = new System.Drawing.Size(131, 29);
            this.bt_cancel.TabIndex = 7;
            this.bt_cancel.Text = "Cancel";
            this.bt_cancel.UseVisualStyleBackColor = true;
            this.bt_cancel.Click += new System.EventHandler(this.bt_cancel_Click);
            // 
            // bt_replaceAll
            // 
            this.bt_replaceAll.Location = new System.Drawing.Point(149, 100);
            this.bt_replaceAll.Name = "bt_replaceAll";
            this.bt_replaceAll.Size = new System.Drawing.Size(123, 29);
            this.bt_replaceAll.TabIndex = 6;
            this.bt_replaceAll.Text = "Replace All";
            this.bt_replaceAll.UseVisualStyleBackColor = true;
            this.bt_replaceAll.Click += new System.EventHandler(this.bt_replaceAll_Click);
            // 
            // FormReplaceMaterial
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 134);
            this.Controls.Add(this.tb_replaceWith);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cb_findWhat);
            this.Controls.Add(this.bt_cancel);
            this.Controls.Add(this.bt_replaceAll);
            this.Name = "FormReplaceMaterial";
            this.Text = "FormReplaceMaterial";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tb_replaceWith;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cb_findWhat;
        private System.Windows.Forms.Button bt_cancel;
        private System.Windows.Forms.Button bt_replaceAll;
    }
}