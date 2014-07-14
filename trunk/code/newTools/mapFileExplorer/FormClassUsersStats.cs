using fileFormats;
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
    public partial class FormClassUsersStats : Form
    {
        private fileFormats.ValueUsersLists classUsers;

        public FormClassUsersStats()
        {
            InitializeComponent();
        }

        public FormClassUsersStats(ValueUsersLists classUsers)
        {
            InitializeComponent();
            this.classUsers = classUsers;
            for (int i = 0; i < classUsers.size(); i++)
            {
                TreeNode nodeClass = new TreeNode("Class " + classUsers.getValueName(i) + " has " + classUsers.getValueNameUsersCount(i) + " users.");
                for (int j = 0; j < classUsers.getValueNameUsersCount(i); j++)
                {
                    int classUserIndex = classUsers.getValueUserIndex(i, j);

                }
                treeView1.Nodes.Add(nodeClass);
            }
        }
    }
}
