using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace mapFileExplorer
{
    public partial class FormReplaceMaterial : Form
    {
        private fileFormats.MapFile map;

        public FormReplaceMaterial()
        {
            InitializeComponent();
        }

        public FormReplaceMaterial(fileFormats.MapFile map)
        {
            InitializeComponent();
            this.map = map;
            HashSet<string> classNames = map.getUsedMaterialNames();
            foreach (string s in classNames)
            {
                cb_findWhat.Items.Add(s);
            }
        }
        private void bt_cancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void bt_replaceAll_Click(object sender, EventArgs e)
        {
            if (tb_replaceWith.Text == null || tb_replaceWith.Text.Length < 2)
            {
                MessageBox.Show("Please select or enter old material name.", "Can't replace.", MessageBoxButtons.OK);
                return;
            }
            if (tb_replaceWith.Text == null || tb_replaceWith.Text.Length < 2)
            {
                MessageBox.Show("Please enter new material name.", "Can't replace.", MessageBoxButtons.OK);
                return;
            }
            int count = map.replaceMaterialName(cb_findWhat.Text, tb_replaceWith.Text);
            MessageBox.Show("Replaced materialname of " + count + " primitives.", "Done.", MessageBoxButtons.OK);
            this.Close();
        }
    }
}
