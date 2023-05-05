/*
*F: c0.cs
* Modified: 2023.5.5 allowed gzip
* Coded at: 2023.3.18 by T. Abi from t0w.cs
$fw32 = "C:\Windows\Microsoft.NET\Framework"
$fw64 = "C:\Windows\Microsoft.NET\Framework64"
$cs2 = "$fw64\v2.0.50727\csc.exe"
$cs3 = "$fw64\v3.5\csc.exe"
$cs4 = "$fw64\v4.0.30319\csc.exe"
$mb4 = "$fw64\v4.0.30319\MSBuild.exe"
& $cs4 foo.cs
*/
using System;
using System.IO;
using System.IO.Compression;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Collections;

namespace nsHW {

  class KiKbd {
    string flnm;
    int[,] kky;
    int[] sc2kn;
    string[] qch = {
      "  1234567890-^  qwertyuiop@[  asdfghjkl;:  ]zxcvbnm,./      \\\\  ",
      "  !"+'"'+"#$%&'() =~  QWERTYUIOP`{  ASDFGHJKL+*  }ZXCVBNM<>?      _|  " };
    string[] dch = {
      "  1234567890[]  ',.pyfgcrl/=  aoeuidhtns-` \\;qjkxbmwvz      \\`  ",
      "  !@#$%^&*(){}  "+'"'+"<>PYFGCRL?+  AOEUIDHTNS_~ |:QJKXBMWVZ      _~  " };
    string[] kch;
/*
 0123456789abcdef1...4...8...c...2...4...8...c...3...4...8...c...
0  1234567890-^  qwertyuiop@[  asdfghjkl;:  ]zxcvbnm,./      \\  
   !"#$%&'() =~  QWERTYUIOP`{  ASDFGHJKL+*  }ZXCVBNM<>?      _|  
0  1234567890-^B qwertyuiop@[ECasdfghjkl;:hS]zxcvbnm,./R ASC
   !"#$%&'() =~S QWERTYUIOP`{noASDFGHJKL+*zh}ZXCVBNM<>?S lPa
 0123456789abcdef5...4...8...c...6...4...8...c...7...4...8...c...
4        u  l r  d                               k  \     h m \
                                                    _         |
 0123456789abcdef9...4...8...c...A...4...8...c...B...4...8...c...
8
 
 0123456789abcdefD...4...8...c...E...4...8...c...F...4...8...c...
C
 

 0123456789abcdef1...4...8...c...2...4...8...c...3...4...8...c...
0  1234567890[]  ',.pyfgcrl/=  aoeuidhtns-` \;qjkxbmwvz      \`  
   !@#$%^&*(){}  "<>PYFGCRL?+  AOEUIDHTNS_~ |:QJKXBMWVZ      _~  
 0123456789abcdef5...4...8...c...6...4...8...c...7...4...8...c...
4                                                   \         \
                                                    _         |
 0123456789abcdef9...4...8...c...A...4...8...c...B...4...8...c...
8
 
 0123456789abcdefD...4...8...c...E...4...8...c...F...4...8...c...
C
 
*/
    int nKeys;

    public KiKbd( string flnm ){
      this.flnm = flnm;
      kky = new int[72, 4];
      for (int i=0; i<72; i++){
        for (int j=0; j<4; j++){
          kky[i, j] = 0;
        }
      }
      sc2kn = new int[128];
      for (int i=0; i<128; i++){
        sc2kn[i] = 0;
      }
      nKeys = 0;
      getKeyAssign();
      for (int i=0; i<72; i++){
        if ((kky[i, 1] > 0) && (kky[i, 1] < 0x80)){
          sc2kn[kky[i, 1]] = i;
        }
      }
      kch = dch;
    }

    int hex2( string hex ){
      const string hexStr = "0123456789abcdef";
      int v = 0;
      if ((hex != null) && (hex.Length > 0)){
        v = hexStr.IndexOf(hex.Substring(0, 1).ToLower());
        if ((v >= 0) && (hex.Length > 1)){
          v = v*16 + hexStr.IndexOf(hex.Substring(1, 1).ToLower());
        }
      }
      return v;
    }

    void getKeyAssignBody( StreamReader sr ){
      int n = 0;
      string buf = sr.ReadLine();
      while (buf.Length > 9){
        int i = Int32.Parse(buf.Substring(9, 2));
        kky[i, 0] = i;
        kky[i, 1] = hex2(buf.Substring(0, 2));
        kky[i, 2] = (int)buf[3] - '0';
        kky[i, 3] = Int32.Parse(buf.Substring(5, 2));
        n++;
        buf = sr.ReadLine();
      }
      nKeys = n;
    }

    void getKeyAssign(){
      if (flnm.Substring(flnm.Length-3) == ".gz"){
        using (FileStream fsIn = File.Open(flnm, FileMode.Open)){
          using (GZipStream cs = new GZipStream(fsIn, CompressionMode.Decompress)){
            using (StreamReader sr = new StreamReader(cs)){
              getKeyAssignBody(sr);
            }
          }
        }
      } else {
        using (StreamReader sr = new StreamReader(flnm)){
          getKeyAssignBody(sr);
        }
      }
    }

    public int getKNo( int scanCode ){
      return sc2kn[scanCode];
    }

    public char getChar( int scanCode, Keys ModKey ){
      char c = '\0';
      int sc = scanCode;
      if (scanCode == 0x73){ sc = 0x3c; }
      if (scanCode == 0x7d){ sc = 0x3d; }
      if (sc < 0x40){
        string ks = kch[ ((ModKey & Keys.Shift) == 0) ? 0 : 1 ];
        if ((ks[sc] != ' ') || (sc == 0x39)){
          c = ks[sc];
        }
      }
      return c;
    } /* char getChar() */

    public void showKbd( Graphics g, Pen pen1, int k1, int shift, KiMap kiMp ){
      Font drawFont = new Font("ＭＳ　ゴシック", 11);
      SolidBrush drawBrush = new SolidBrush(Color.Black);
      Rectangle rc = new Rectangle(0, 0, 9, 9);
      for (int i=1; i<=nKeys; i++){
        if (kky[i, 3] > 0){
          rc.X = kky[i, 3]*4 +5;
          rc.Y = kky[i, 2]*20 +110 +5;
          if (k1 == kky[i, 0]){
            g.DrawRectangle(pen1, rc.X-6, rc.Y-6, 20, 20);
          }
          if ((kky[i, 0] >= 1) && (kky[i, 0] <= 49)){
            string ss = kiMp.getKeyTop(kky[i, 0], k1, shift);
            g.DrawString(ss, drawFont, drawBrush, rc.X-5, rc.Y-2);
          } else {
            g.DrawRectangle(pen1, rc);
          }
        }
      }
      rc.Width = 3;
      rc.Height = 3;
      rc.X = 36*4 -5;
      rc.Y = (int)(3.5*20 +110) -1;
      g.DrawRectangle(pen1, rc);
      rc.X = 48*4 -4;
      g.DrawRectangle(pen1, rc);
    }

  } /* class KiKbd */

  class KiMap {
    UnicodeEncoding utf16;
    UTF8Encoding utf8;
    string flnm;
    char[,] tf;

    public KiMap( string flnm ){
      this.flnm = flnm;
      utf16 = new UnicodeEncoding();
      utf8 = new UTF8Encoding();
      tf = new char[10000, 2];
      for (int i=0; i<10000; i++){
        tf[i, 0] = '\0';
        tf[i, 1] = '\0';
      }
      getKeyMap();
    }

    void setUtf16( int n, byte[] barr ){
      int d = ((barr[0] & 0xf0) == 0xe0) ? 3 : 2;
      tf[n, 0] = utf8.GetString(barr, 0, d)[0];
      if ((barr.Length > d) && (barr[d] == ' ')){
        tf[n, 1] = ' ';
      }
    }

    void setUtf8( int n, byte[] barr ){
      byte b = (barr.Length >= 2) ? barr[1] : (byte)0;
      if ((b >= (byte)'0') && (b <= (byte)'9')){
        tf[n, 0] = (char)barr[0];
        tf[n, 1] = (char)b;
      }
    }

    void updt( int n, string s ){
      byte[] barr = utf8.GetBytes(s);
      int nb = barr.Length;
      byte b = (nb == 0) ? (byte)0 : barr[0];
      if ((b == 0) || (b == (byte)'#') || (b == (byte)'\t')){
        tf[n, 0] = '\0';
        tf[n, 1] = '\0';
      } else if (((b & 0xf0) == 0xe0) || ((b & 0xe0) == 0xc0)){
        setUtf16(n, barr);
      } else if ((b >= '0') || (b <= '9')){
        setUtf8(n, barr);
      }
    }

    void getKeyMapBody( StreamReader sr ){
      int n = 0;
      tf[n, 0] = '\0';
      tf[n, 1] = '\0';
      n++;
      string buf = sr.ReadLine();
      while (buf != null){
        updt(n, buf);
        n++;
        buf = sr.ReadLine();
      }
    }

    void getKeyMap(){
      if (flnm.Substring(flnm.Length-3) == ".gz"){
        using (FileStream fsIn = File.Open(flnm, FileMode.Open)){
          using (GZipStream cs = new GZipStream(fsIn, CompressionMode.Decompress)){
            using (StreamReader sr = new StreamReader(cs)){
              getKeyMapBody(sr);
            }
          }
        }
      } else {
        using (StreamReader sr = new StreamReader(flnm)){
          getKeyMapBody(sr);
        }
      }
    }

    public char getChar( int ik ){
      return tf[ik, 0];
    }

    string getString( int ii ){
      string s = "";
      if (tf[ii, 0] != 0){
        if (tf[ii, 1] == 0){
          s = tf[ii, 0].ToString();
        } else {
          char[] chs = new char[2];
          chs[0] = tf[ii, 0];
          chs[1] = tf[ii, 1];
          s = new string(chs);
        }
      }
      return s;
    }

    public string getKeyTop( int kno, int k1, int shift ){
      string s = "";
      int b0 = (shift == 1) ? 50 : 0;
      int ii = 0;
      if (k1 == 0){
        ii = b0*100 +kno*100;
      } else {
        ii = k1*100 +b0 +kno;
      }
      s = getString(ii);
      return s;
    }

  } /* class KiMap */

  class KiBase {
    string kiDir;
    const string KIKEY = "kiKey_106.gz";
    const string KIMAP = "kiMap.gz";
    KiKbd kiKy;
    KiMap kiMp;
    int k1;
    int[] ks;	/* 0control 1shift */

    public KiBase( string kiDir ){
      this.kiDir = kiDir;
      kiKy = new KiKbd(kiDir+@"\"+KIKEY);
      kiMp = new KiMap(kiDir+@"\"+KIMAP);
      k1 = 0;
      ks = new int[2];
      ks[0] = 0;
      ks[1] = 0;
    }

    public int getKNo( int scanCode ){
      return kiKy.getKNo(scanCode);
    }

    public char getChar( int scanCode, Keys ModKey ){
      return kiKy.getChar(scanCode, ModKey);
    }

    public void showKbd( Graphics g, Pen pen1 ){
      kiKy.showKbd(g, pen1, k1, ks[1], kiMp);
    }

    public void clearStatus(){
      k1 = 0;
    }

    public void setCtrl( int cd ){
      if (ks[0] != cd){
        ks[0] = cd;
/*
        if (cd == 0){
          k1 = 0;
        }
*/
      }
k1 = 0;
    }

    public void setShift( int cd ){
      if (ks[1] != cd){
        ks[1] = cd;
      }
    }

    const int VK_SHIFT = 0x010;
    const int VK_CONTROL = 0x011;
    public int keyUp( int wParam ){
      int reDraw = 1;
      switch (wParam){
      case VK_CONTROL: setCtrl(0); break;
      case VK_SHIFT: ks[1] = 0; break;
      default: reDraw = 0; break;
      }
      return reDraw;
    }

    public char convChar( int kno, char c ){
      char c0 = '\0';
      if (kno == 62){	/* BS */
        clearStatus();
      } else if (kno == 52){	/* Ctrl */
        setCtrl(1);
      } else if ((kno == 50) || (kno == 64)){	/* Shift */
        setShift(1);
      } else {
        int ik = 0;
        if ((kno >= 1) && (kno < 49)){
          if (k1 == 0){
            k1 = kno +ks[1]*50;
          } else {
            ik = k1*100 +kno +ks[1]*50;
            k1 = 0;
          }
        }
        if (ik > 0){
          c0 = kiMp.getChar(ik);
        }
      }
      return c0;
    }

  } /* class KiBase */

  class CARETBASE {
    Pen[] drawPen;
    PointF cpos;
    int sta;	/* 0hide 1show */

    public CARETBASE( Color bg ){
      drawPen = new Pen[2];
      drawPen[0] = new Pen(bg, 1);
      drawPen[1] = new Pen(Color.Blue, 1);
      cpos.X = 0;
      cpos.Y = 0;
      sta = 0;	/* 0hide 1show */
    }

    public void move( float x, float y ){
      cpos.X = x;
      cpos.Y = y;
    }

    public void rmove( int dx, int dy ){
      cpos.X += dx;
      cpos.Y += dy;
    }

    public void setX( float x ){
      cpos.X = x;
    }

    void drawCursor( Graphics g, int sta ){
      this.sta = sta;
      g.DrawLine(drawPen[sta], cpos.X, cpos.Y+3, cpos.X, cpos.Y+8);
    }

    public void hide( Graphics g ){
      drawCursor(g, 0);
    }

    public void show( Graphics g ){
      drawCursor(g, 1);
    }

    public PointF getPos(){
      return new PointF(cpos.X, cpos.Y);
    }

    public bool chkPos( PointF pf, SizeF sf, int ht ){
      return ((cpos.X >= pf.X) && (cpos.X < pf.X+sf.Width) && (cpos.Y >= pf.Y) && (cpos.Y < pf.Y+ht));
    }

  }

  class TBOXBASE {
    Graphics g;
    Color bg;
    KiBase kiBs;
    Pen[] drawPen;
    Font drawFont;
    SolidBrush drawBrush;
    const int CRTBUFSZ = 64000;
    Size sz;
    Rectangle br;
    CARETBASE caret;
    int dwCharX;	/* average width of characters */
    int dwCharY;	/* height of characters */
    int dwClientX;	/* width of client area */
    int dwClientY;	/* height of characters */
    int dwLineLen;	/* line length */
    int dwLines;	/* text lines in client area */
    int nCharWidth;	/* width of a character */
    int cch;	/* characters in buffer */
    int nCurChar;	/* index of current character */
    char[] pchInputBuf;	/* address of input buffer */
    int showKbdFlag;
    int kiModeFlag;

    public TBOXBASE( string kiDir, Color bg ){
      kiBs = new KiBase(kiDir);
      g = null;
      this.bg = bg;
      drawPen = new Pen[2];
      drawPen[0] = new Pen(Color.Black, 1);
/*      drawPen[1] = new Pen(Color.Yellow, 1); */
      drawPen[1] = new Pen(Color.FromArgb(127,127,255), 1);
      drawFont = new Font("ＭＳ　ゴシック", 11);
      drawBrush = new SolidBrush(Color.Black);
      this.sz = new Size(380, 180);
      this.br = new Rectangle(10, 10, 340, 100);
      this.caret = new CARETBASE(bg);
      nCharWidth = 0;
      cch = 0;
      nCurChar = 0;
      dwCharX = (int)drawFont.Size;
      dwCharY = drawFont.Height;
      pchInputBuf = new char[CRTBUFSZ];
      pchInputBuf[0] = '\0';
      showKbdFlag = 0;
      kiModeFlag = 0;
/*
char[] carr = "Hello, 彩音 wonder!".ToCharArray();
cch = carr.Length;
for (int i=0; i<cch; i++){
  pchInputBuf[i] = carr[i];
}
nCurChar = cch;
pchInputBuf[cch] = '\0';
*/
    }

    public void showMode(){
      StringFormat sf = new StringFormat();
      sf.LineAlignment = StringAlignment.Near;
      sf.Alignment = StringAlignment.Far;
      int px = br.X+br.Width;
      int py = br.Y+br.Height +3;
      string s = (kiModeFlag == 0) ? "Ｗ" : "き";
      g.DrawString(s, drawFont, drawBrush, px, py, sf);
    }

    void myDrawText(){
/*      caret.hide(g); */
      PointF pf = new PointF();
      pf.X = br.X;
      pf.Y = br.Y;
      for (int i=0; i<cch; i++){
        string c = pchInputBuf[i].ToString();
        SizeF szf = g.MeasureString(c, drawFont);
        if (pf.X+szf.Width >= br.X+br.Width){
          if ((nCurChar == -1) && caret.chkPos(pf, szf, drawFont.Height)){
            nCurChar = i;
            caret.move(pf.X, pf.Y);
          }
          pf.X = br.X;
          pf.Y += drawFont.Height;
        }
        g.DrawString(c, drawFont, drawBrush, pf);
        if ((nCurChar == -1) && caret.chkPos(pf, szf, drawFont.Height)){
          nCurChar = i;
          caret.move(pf.X, pf.Y);
        } else if (i == nCurChar){
          caret.move(pf.X, pf.Y);
        }
        pf.X += szf.Width;
      }
      if ((nCurChar == -1) || (nCurChar == cch)){
        nCurChar = cch;
        caret.move(pf.X, pf.Y);
      }
      caret.show(g);
    }

    public void Paint( Graphics g ){
      this.g = g;
      g.DrawRectangle(drawPen[0], br);
      if ((showKbdFlag == 1) && (kiModeFlag == 1)){
        kiBs.showKbd(g, drawPen[1]);
      }
      showMode();
      myDrawText();
    }

    public void putChar( char c ){
      if (nCurChar == -1){
        nCurChar = cch;
      }
      if (nCurChar < cch){
        for (int i=cch; i>nCurChar; i--){
          pchInputBuf[i] = pchInputBuf[i-1];
        }
      }
      pchInputBuf[nCurChar++] = c;
      pchInputBuf[++cch] = '\0';
    }

    void clearBuffer(){
      cch = 0;
      nCurChar = 0;
    }

    void procBS(){
      if (nCurChar > 0){
        nCurChar--;
        cch--;
        if (cch > nCurChar){
          for (int i=nCurChar; i<cch; i++){
            pchInputBuf[i] = pchInputBuf[i+1];
          }
        }
      }
    }

    void procUP(){
      caret.rmove(0, -1);
      nCurChar = -1;
    }

    void procDOWN(){
      caret.rmove(0, drawFont.Height);
      nCurChar = -1;
    }

    void procChar( int kn, char c ){
      if (kn == 62){	/* BS */
        procBS();
        kiBs.clearStatus();
      } else if (kn == 52){	/* Ctrl */
        kiBs.setCtrl(1);
      } else if ((kn == 50) || (kn == 64)){	/* Shift */
        kiBs.setShift(1);
      } else if (kn == 56){	/* henkan */
        showKbdFlag = 1-showKbdFlag;
      } else if (kn == 58){	/* Up */
        procUP();
      } else if (kn == 59){	/* Left */
        if (nCurChar > 0) nCurChar--;
      } else if (kn == 60){	/* Right */
        if ((nCurChar < cch) && (nCurChar >= 0)) nCurChar++;
      } else if (kn == 61){	/* Down */
        procDOWN();
      } else {
        char c0 = (kiModeFlag == 0) ? c : kiBs.convChar(kn, c);
        if (c0 != '\0'){
          putChar(c0);
        }
      }
    }

    void procCtrl( int kn, char c ){
      char c0 = Char.ToLower(c);
      if (c0 == ' '){
        kiModeFlag = 1-kiModeFlag;
      } else if (c0 == 'c'){
        Clipboard.SetText(new string(pchInputBuf, 0, cch));
      } else if (c0 == 'h'){
        procBS();
        kiBs.clearStatus();
      } else if (c0 == 'u'){
        clearBuffer();
      } else if (c0 == 'x'){
        Clipboard.SetText(new string(pchInputBuf, 0, cch));
        clearBuffer();
      }
    }

    public int keyDown( int scanCode, Keys ModKey ){
      int reDraw = 1;
      if (scanCode == 0x47){	/* HOME */
        if ((ModKey & Keys.Control) == 0){
          caret.setX(br.X);
          nCurChar = -1;
        } else {
          nCurChar = 0;
        }
      } else if (scanCode == 0x4f){	/* END */
        if ((ModKey & Keys.Control) == 0){
          caret.setX(br.X+br.Width);
          nCurChar = -1;
        } else {
          nCurChar = cch;
        }
      } else if (scanCode == 0x53){	/* DEL */
        if (nCurChar < cch){
          nCurChar++;
          procBS();
        }
      } else {
        int kn = kiBs.getKNo(scanCode);
        char c = kiBs.getChar(scanCode, ModKey);
        if ((ModKey & Keys.Control) == 0){
          int prev = showKbdFlag;
          procChar(kn, c);
          if (showKbdFlag != prev){
            reDraw += 2*(1+showKbdFlag);
          }
        } else {
          procCtrl(kn, c);
        }
      }
      return reDraw;
    }

    public int keyUp( int wParam ){
      return kiBs.keyUp(wParam);
    }

    public int lbDown( int lParam ){
      caret.move(lParam & 0xffff, (lParam >> 16) & 0xffff);
      nCurChar = -1;
      return 1;
    }

  } /* class TBOXBASE */

  class Form1 : Form {
    string kiDir;
    TBOXBASE tbxb;

    public Form1(){
      kiDir = Environment.GetEnvironmentVariable("USERPROFILE", EnvironmentVariableTarget.Process);
      kiDir += @"\ki";
      tbxb = new TBOXBASE(kiDir, this.BackColor);
      this.InitializeComponent();
    }

    void InitializeComponent(){
      this.Paint += new PaintEventHandler(Form1_Paint);
      this.Resize += new EventHandler(Form1_Resize);
      this.Size = new Size(378, 178);
/*      this.SetStyle(ControlStyles.ResizeRedraw, true); */
      this.SetStyle(ControlStyles.DoubleBuffer, true);
      this.SetStyle(ControlStyles.UserPaint, true);
      this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
    } /* void InitializeComponent() */

    void Form1_Resize( Object sender, EventArgs e ){
/*
      Size sz0 = this.ClientSize;
*/
    }

    void Form1_Paint( Object sender, PaintEventArgs e ){
      Graphics g = e.Graphics;
      tbxb.Paint(g);
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

    void postKeyDown( int reDraw ){
      if ((reDraw & 1) == 1){
        if ((reDraw & 2) == 2){
          this.Size = new Size(378, 178);
        } else if ((reDraw & 4) == 4){
          this.Size = new Size(378, 278);
        }
        this.Refresh();
      }
    }

    const int WM_KEYDOWN = 0x0100;
    const int WM_KEYUP = 0x0101;
    const int WM_LBUTTONDOWN = 0x0201;
    protected override void WndProc( ref Message m ){
      int reDraw = 1;
      int scanCode;
      switch (m.Msg){
      case WM_KEYDOWN:
        scanCode = getScanCode(m.LParam.ToInt32());
        if ((m.WParam.ToInt32() == 'Q') && ((Control.ModifierKeys & Keys.Control) != 0)){
          DialogResult rc = MessageBox.Show("Quit?", "Exiting...", MessageBoxButtons.YesNo);
          if (rc == DialogResult.Yes){
            this.Close();
          }
        } else {
          reDraw = tbxb.keyDown(scanCode, Control.ModifierKeys);
        }
        break;
      case WM_KEYUP:
        reDraw = tbxb.keyUp(m.WParam.ToInt32());
        break;
      case WM_LBUTTONDOWN:
        reDraw = tbxb.lbDown(m.LParam.ToInt32());
        break;
      default: reDraw = 0;
        break;
      }
      base.WndProc(ref m);
      if ((reDraw & 1) == 1){
        postKeyDown(reDraw);
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
