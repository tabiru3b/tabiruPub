/*
*F: scancode.cs
* Coded at: 2023.4.1 by T. Abi from c0.cs
$fw64 = "C:\Windows\Microsoft.NET\Framework64"
$cs4 = "$fw64\v4.0.30319\csc.exe"
$mb4 = "$fw64\v4.0.30319\MSBuild.exe"
& $cs4 foo.cs
*/
using System;
using System.Drawing;
using System.Windows.Forms;

namespace nsHW {

  class Form1 : Form {
    Font drawFont;
    SolidBrush drawBrush;
    int scanCode;

    public Form1(){
      this.InitializeComponent();
    }

    void InitializeComponent(){
      drawFont = new Font("ＭＳ　ゴシック", 11);
      drawBrush = new SolidBrush(Color.Black);
      this.Paint += new PaintEventHandler(Form1_Paint);
    } /* void InitializeComponent() */

    void Form1_Paint( Object sender, PaintEventArgs e ){
      Graphics g = e.Graphics;
      string s = String.Format("{0:x}", scanCode);
      g.DrawString(s, drawFont, drawBrush, 10, 10);
    }

    int getScanCode( Int32 lParam ){
      int scanCode = (lParam >> 16) & 0x0ff; // extract 16--23
/*
      int ext = (lParam >> 24) & 0x01; // extract bit 24
      if (ext == 1){
        scanCode += 128;
      }
*/
      return scanCode;
    }

    const int WM_KEYDOWN = 0x0100;
    const int WM_KEYUP = 0x0101;
    protected override void WndProc( ref Message m ){
      int reDraw = 1;
      switch (m.Msg){
      case WM_KEYDOWN:
        scanCode = getScanCode(m.LParam.ToInt32());
        if ((m.WParam.ToInt32() == 'Q') && ((Control.ModifierKeys & Keys.Control) != 0)){
          DialogResult rc = MessageBox.Show("Quit?", "Exiting...", MessageBoxButtons.YesNo);
          if (rc == DialogResult.Yes){
            this.Close();
          }
        }
        break;
      default: reDraw = 0;
        break;
      }
      base.WndProc(ref m);
      if (reDraw == 1){
        this.Refresh();
      }
    }

  } /* class Form1 : Form */

  class Program {

    [STAThread]
    static void Main( string[] args ){
      Application.Run( new Form1() );
    }

  } /* class Program */

} /* nsHW */
