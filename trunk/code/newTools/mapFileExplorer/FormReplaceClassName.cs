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
    public partial class FormReplaceClassName : Form
    {
        private fileFormats.MapFile map;

        public FormReplaceClassName()
        {
            InitializeComponent();
        }

        public FormReplaceClassName(fileFormats.MapFile map)
        {
            InitializeComponent();
            this.map = map;
            HashSet<string> classNames = map.getUsedClassNames();
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
                MessageBox.Show("Please select or enter old classname.", "Can't replace.", MessageBoxButtons.OK);
                return;
            }
            if(tb_replaceWith.Text == null || tb_replaceWith.Text.Length < 2) 
            {
                MessageBox.Show("Please enter new classname.", "Can't replace.", MessageBoxButtons.OK);
                return;
            }
            int count = map.replaceClassName(cb_findWhat.Text, tb_replaceWith.Text);
            MessageBox.Show("Replaced classname of " + count + " entities.", "Done.", MessageBoxButtons.OK);
            this.Close();
        }
    }
}
