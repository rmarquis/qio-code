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

namespace mapFileExplorer
{
    public partial class FormAddNewKeyValue : Form
    {
        private FormMapFileExplorer formMapFileExplorer;
        private MapEntity lastClickedNodeMapEntity;

        public FormAddNewKeyValue(FormMapFileExplorer formMapFileExplorer, fileFormats.MapEntity lastClickedNodeMapEntity)
        {
            this.formMapFileExplorer = formMapFileExplorer;
            this.lastClickedNodeMapEntity = lastClickedNodeMapEntity;
            InitializeComponent();
        }

        private void btSet_Click(object sender, EventArgs e)
        {
            if (tbKey.Text.Length == 0)
            {
                MessageBox.Show("Key name must not be empty!");
                return;
            }
            if (tbKey.Text.IndexOf('"')!= -1)
            {
                MessageBox.Show("\" cannot be used in key values!");
                return;
            }
            if (tbVal.Text.IndexOf('"') != -1)
            {
                MessageBox.Show("\" cannot be used in key values!");
                return;
            }
            lastClickedNodeMapEntity.setKeyValue(tbKey.Text, tbVal.Text);
        }
    }
}
