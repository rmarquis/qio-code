namespace mapFileExplorer
{
    partial class FormAddNewKeyValue
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
            this.label1 = new System.Windows.Forms.Label();
            this.tbKey = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.tbVal = new System.Windows.Forms.TextBox();
            this.btCancel = new System.Windows.Forms.Button();
            this.btSet = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(28, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Key:";
            // 
            // tbKey
            // 
            this.tbKey.Location = new System.Drawing.Point(45, 6);
            this.tbKey.Name = "tbKey";
            this.tbKey.Size = new System.Drawing.Size(286, 20);
            this.tbKey.TabIndex = 1;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(14, 33);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(25, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Val:";
            // 
            // tbVal
            // 
            this.tbVal.Location = new System.Drawing.Point(45, 32);
            this.tbVal.Name = "tbVal";
            this.tbVal.Size = new System.Drawing.Size(286, 20);
            this.tbVal.TabIndex = 3;
            // 
            // btCancel
            // 
            this.btCancel.Location = new System.Drawing.Point(17, 64);
            this.btCancel.Name = "btCancel";
            this.btCancel.Size = new System.Drawing.Size(139, 23);
            this.btCancel.TabIndex = 4;
            this.btCancel.Text = "Cancel";
            this.btCancel.UseVisualStyleBackColor = true;
            // 
            // btSet
            // 
            this.btSet.Location = new System.Drawing.Point(162, 64);
            this.btSet.Name = "btSet";
            this.btSet.Size = new System.Drawing.Size(169, 23);
            this.btSet.TabIndex = 5;
            this.btSet.Text = "Set";
            this.btSet.UseVisualStyleBackColor = true;
            this.btSet.Click += new System.EventHandler(this.btSet_Click);
            // 
            // FormAddNewKeyValue
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(346, 93);
            this.Controls.Add(this.btSet);
            this.Controls.Add(this.btCancel);
            this.Controls.Add(this.tbVal);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.tbKey);
            this.Controls.Add(this.label1);
            this.Name = "FormAddNewKeyValue";
            this.Text = "FormAddNewKeyValue";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbKey;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbVal;
        private System.Windows.Forms.Button btCancel;
        private System.Windows.Forms.Button btSet;
    }
}