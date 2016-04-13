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
    public partial class FormModelUsersStats : Form
    {
        private fileFormats.ValueUsersLists modelUsers;

        public FormModelUsersStats()
        {
            InitializeComponent();
        }

        public FormModelUsersStats(fileFormats.ValueUsersLists modelUsers)
        {
            InitializeComponent();
            this.modelUsers = modelUsers;
            for (int i = 0; i < modelUsers.size(); i++)
            {
                TreeNode nodeClass = new TreeNode("Model " + modelUsers.getValueName(i) + " has " + modelUsers.getValueNameUsersCount(i) + " users.");
                for (int j = 0; j < modelUsers.getValueNameUsersCount(i); j++)
                {
                    int classUserIndex = modelUsers.getValueUserIndex(i, j);

                }
                treeView1.Nodes.Add(nodeClass);
            }
        }

        private void FormModelUsersStats_Load(object sender, EventArgs e)
        {

        }
    }
}
