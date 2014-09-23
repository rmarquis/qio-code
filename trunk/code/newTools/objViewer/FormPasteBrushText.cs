using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace objViewer
{
    public partial class FormPasteBrushText : Form
    {
        private FormObjViewer objView;
        public FormPasteBrushText()
        {
            InitializeComponent();
        }

        public void setOBJViewerForm(FormObjViewer formObjViewer)
        {
            objView = formObjViewer;
        }

        private void bt_importBrush_Click(object sender, EventArgs e)
        {
            objView.importBrushFromText(tb_brushText.Text);
            this.Close();
        }
    }
}
