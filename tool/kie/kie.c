/**********************************************************************
*F: kie.c
* Modifed@: 2022.2.23 c11 style
* Modifed@: 2021.8.18 from k0.c
* Coded at: 2021.8.8 by T. Abi
**********************************************************************/

#define UNICODE

#include <windows.h>
#include <stdint.h>

#define BUFSZ	1024
#define CRTBUFSZ 64000

struct GBUF {
HANDLE hFile;
char buf[BUFSZ];
BOOL bResult;
DWORD nBytesRead;
};

struct KIBASE {
int32_t nkeys;
BYTE kky[80][4];	/* 0kno 1kcd 2ypos 3xpos */
TCHAR tf[10000][2];
BYTE k1;
int32_t ks[2];	/* 0control 1shift */
int32_t showkb;	/* 0not 1show */
int32_t kimode;	/* 0raw 1ki */
};

struct TBOXBASE {
RECT fr;
RECT br;
DWORD dwCharX;		/* average width of characters */
DWORD dwCharY;		/* height of characters */
DWORD dwClientX;	/* width of client area */
DWORD dwClientY;	/* height of client area */
DWORD dwLineLen;	/* line length */
DWORD dwLines;		/* text lines in client area */
int32_t nCaretPosX;		/* horizontal position of caret */
int32_t nCaretPosY;		/* vertical position of caret */
int32_t nCharWidth;		/* width of a character */
int32_t cch;		/* characters in buffer */
int32_t nCurChar;		/* index of current character */
LPTSTR pchInputBuf;	/* address of input buffer */
} tbx;

struct COMBASE {
TCHAR szWindowClass[32];
TCHAR szTitle[80];
HINSTANCE hInst;
struct KIBASE kbs;
} cmn;

int32_t myDrawText(
HDC hDC,
LPCTSTR ibuf,
int32_t cch,
RECT *rc,
UINT uform)
{
    int32_t theight = 0;
    if (ibuf != NULL){
	if (cch == -1){
	    cch = lstrlen(ibuf);
	}
	int32_t px = rc->left;
	int32_t py = rc->top;
	for (int32_t i=0; i<cch; i++){
	    if (i == tbx.nCurChar){
		tbx.nCaretPosX = px;
		tbx.nCaretPosY = py;
	    }
	    SIZE ssz;
	    if (GetTextExtentPoint32(hDC,&ibuf[i],1,&ssz)){
		if (ssz.cx+px >= rc->right){
		    px = rc->left;
		    py += tbx.dwCharY;
		}
		TextOut(hDC,px,py,&ibuf[i],1);
		px += ssz.cx;
	    }
	}
	if (tbx.nCurChar == tbx.cch){
	    tbx.nCaretPosX = px;
	    tbx.nCaretPosY = py;
	}
	theight = py-rc->top + tbx.dwCharY;
    }
    return theight;
}

int32_t showKbd( HDC hDC, struct KIBASE *kbs )
{
    if (kbs->nkeys > 0){
	int32_t x,y;
	HPEN hP1 = CreatePen(PS_SOLID,1,RGB(127,127,255));
	HPEN hP0 = SelectObject(hDC,hP1);
	int32_t b0 = (GetKeyState(VK_SHIFT) & 0x8000) != 0 ? 50 : 0;
	for (int32_t i=0; i<kbs->nkeys; i++){
	    if (kbs->kky[i][3] > 0){
		x = kbs->kky[i][3]*4;
		y = kbs->kky[i][2]*20 +110;
		SelectObject(hDC,hP1);
		Rectangle(hDC,x+5,y+5,x+14,y+14);
		if ((kbs->kky[i][0] >= 1) && (kbs->kky[i][0] <= 49)){
		    int32_t m,ii;
		    if (kbs->k1 == 0){
			ii = b0*100+kbs->kky[i][0]*100;
			if (kbs->tf[ii][0] != 0){
			    m = (kbs->tf[ii][1] == 0) ? 1 : 2;
			    SelectObject(hDC,hP0);
			    TextOut(hDC,x,y,&kbs->tf[ii][0],m);
			}
		    } else {
			ii = kbs->k1*100+b0+kbs->kky[i][0];
			if (kbs->k1 == b0+kbs->kky[i][0]){
			    SelectObject(hDC,hP1);
			    Rectangle(hDC,x-1,y-1,x+19,y+19);
			}
			if (kbs->tf[ii][0] != 0){
			    m = (kbs->tf[ii][1] == 0) ? 1 : 2;
			    SelectObject(hDC,hP0);
			    TextOut(hDC,x,y,&kbs->tf[ii][0],m);
			}
		    }
		}
	    }
	}
	x = 36*4;
	y = 3.5*20+110;
	Rectangle(hDC,x-6,y-1,x-3,y+2);
	x = 48*4;
	Rectangle(hDC,x-6,y-1,x-3,y+2);
	SelectObject(hDC,hP0);
	DeleteObject(hP1);
    }
    return 0;
}

int32_t showMode( HDC hDC, struct KIBASE *kbs )
{
    TCHAR bb[8];
    int32_t x = tbx.fr.right-24;
    int32_t y = tbx.fr.bottom-24;
    if (kbs->kimode == 0){
	lstrcpy(bb,TEXT("Ｗ"));
    } else {
	lstrcpy(bb,TEXT("き"));
    }
    TextOut(hDC,x,y,bb,lstrlen(bb));
/*
MoveToEx(hDC,345,215,NULL);
LineTo(hDC,325,215);
LineTo(hDC,325,235);
*/
    return 0;
}

LRESULT WINAPI MsgPaint(
HWND hWnd,
UINT message,
WPARAM wParam,
LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hWnd, &ps);
    if ((cmn.kbs.showkb == 1) && (cmn.kbs.kimode == 1)){
	showKbd(hDC,&cmn.kbs);
    }
    showMode(hDC,&cmn.kbs);
    RECT rc;
    CopyMemory(&rc,&tbx.br,sizeof(RECT));
    Rectangle(hDC,rc.left-1,rc.top-1,rc.right+1,rc.bottom+1);
    SetTextAlign(hDC, TA_TOP | TA_LEFT);
    HideCaret(hWnd);
    if (tbx.cch > 0){
	myDrawText(hDC,tbx.pchInputBuf,-1,&rc,DT_WORDBREAK);
    } else {
	tbx.nCaretPosX = tbx.br.left;
	tbx.nCaretPosY = tbx.br.top;
    }
    SetCaretPos(tbx.nCaretPosX,tbx.nCaretPosY);
    ShowCaret(hWnd);
    EndPaint(hWnd, &ps);
    return 0;
}

int32_t clipCopy( HWND hWnd )
{
    HGLOBAL hg = GlobalAlloc(GHND|GMEM_SHARE,sizeof(TCHAR)*(lstrlen(tbx.pchInputBuf)+1));
    if (hg != NULL){
	LPTSTR strMem = GlobalLock(hg);
	lstrcpy(strMem,tbx.pchInputBuf);
	GlobalUnlock(hg);
	if (OpenClipboard(hWnd)){
	    EmptyClipboard();
	    SetClipboardData(CF_UNICODETEXT,hg);
	    CloseClipboard();
	} else {
	    GlobalFree(hg);
	}
    }
    return 0;
}

int32_t setMpos( HWND hWnd, struct TBOXBASE *tb, int32_t mx, int32_t my )
{
    if ((mx >= tb->br.left) && (mx <= tb->br.right) && (my >= tb->br.top) && (my <= tb->br.bottom)){
	HDC hDC = GetDC(hWnd);
	int32_t px = tb->br.left;
	int32_t py = tb->br.top;
	int32_t ab = 0;
	SIZE ssz;
	for (int32_t i=0; (i<tb->cch)&&(ab==0); i++){
	    if (GetTextExtentPoint32(hDC,&tb->pchInputBuf[i],1,&ssz)){
		if (px+ssz.cx >= tb->br.right){
		    if ((mx >= px) && (mx <= tb->br.right) && (my >= py) && (my < py+tb->dwCharY)){
			tb->nCurChar = i;
			tb->nCaretPosX = px;
			tb->nCaretPosY = py;
			ab = 1;
		    }
		    px = tb->br.left;
		    py += tb->dwCharY;
		}
/*		TextOut(hDC,px,py,&ibuf[i],1); */
		if ((mx >= px) && (mx < px+ssz.cx) && (my >= py) && (my < py+tb->dwCharY)){
		    tb->nCurChar = i;
		    tb->nCaretPosX = px;
		    tb->nCaretPosY = py;
		    ab = 1;
		}
		px += ssz.cx;
	    }
	}
	if ((my >= py+tb->dwCharY) || ((mx >= px) && (my >= py) && (my < py+tb->dwCharY))){
	    tb->nCurChar = tb->cch;
	    tb->nCaretPosX = px;
	    tb->nCaretPosY = py;
	}
	ReleaseDC(hWnd,hDC);
	HideCaret(hWnd);
	SetCaretPos(tb->nCaretPosX,tb->nCaretPosY);
	ShowCaret(hWnd);
    }
    return 0;
}

int32_t prockyup( HWND hWnd, struct TBOXBASE *tb, LPARAM lP, WPARAM wP )
{
    switch(wP){
    case VK_CONTROL:
	cmn.kbs.ks[0] = 0;
	if (cmn.kbs.k1 > 0){
	    cmn.kbs.k1 = 0;
	    RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	}
	break;
    case VK_SHIFT:
	cmn.kbs.ks[1] = 0;
	RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	break;
    }
    return 0;
}

int32_t procBS( HWND hWnd, struct TBOXBASE *tb )
{
    if (tb->cch > 0){
	HDC hDC = GetDC(hWnd);
	if (tb->nCurChar == 0) tb->nCurChar++;
	SIZE ssz;
	if (GetTextExtentPoint32(hDC,&tb->pchInputBuf[tb->nCurChar-1],1,&ssz)){
	    if (tb->nCaretPosX > tb->br.left){
		tb->nCaretPosX -= ssz.cx;
	    } else {
		if (tb->nCaretPosY > tb->br.top){
		    tb->nCaretPosY -= tb->dwCharY;
		}
		tb->nCaretPosX = tb->br.right-ssz.cx;
	    }
	}
	ReleaseDC(hWnd,hDC);
	if (tb->nCurChar < tb->cch){
	    MoveMemory(&tb->pchInputBuf[tb->nCurChar-1],&tb->pchInputBuf[tb->nCurChar],sizeof(TCHAR)*(tb->cch-tb->nCurChar));
	}
	tb->nCurChar--;
	tb->pchInputBuf[--tb->cch] = 0;
    }
    return 0;
}

int32_t procDEL( HWND hWnd, struct TBOXBASE *tb )
{
    if (tb->nCurChar < tb->cch){
	MoveMemory(&tb->pchInputBuf[tb->nCurChar],&tb->pchInputBuf[tb->nCurChar+1],sizeof(TCHAR)*(tb->cch-tb->nCurChar));
	tb->cch--;
    }
    return 0;
}

int32_t csrMove( HWND hWnd, struct TBOXBASE *tb, int32_t cd )
{
    HDC hDC = GetDC(hWnd);
    switch(cd){
    case 0:	/* 0L 1R 2U 3D */
	if (tb->nCurChar > 0){
	    tb->nCurChar--;
	    InvalidateRect(hWnd,&tb->br,TRUE);
	}
	break;
    case 1:	/* 0L 1R 2U 3D */
	if (tb->nCurChar < tb->cch){
	    tb->nCurChar++;
	    InvalidateRect(hWnd,&tb->br,TRUE);
	}
	break;
    case 2:	/* 0L 1R 2U 3D */
	if (tb->nCaretPosY >= tb->br.top+tb->dwCharY){
	    setMpos(hWnd,tb,tb->nCaretPosX,tb->nCaretPosY-tb->dwCharY);
	}
	break;
    case 3:	/* 0L 1R 2U 3D */
	if (tb->nCaretPosY+tb->dwCharY < tb->br.bottom){
	    setMpos(hWnd,tb,tb->nCaretPosX,tb->nCaretPosY+tb->dwCharY);
	}
	break;
    }
    ReleaseDC(hWnd,hDC);
    return 0;
}

int32_t prockydn( HWND hWnd, struct TBOXBASE *tb, LPARAM lP, WPARAM wP )
{
    switch(wP){
    case VK_LEFT:
	csrMove(hWnd,tb,0);	/* 0L 1R 2U 3D */
	break;
    case VK_RIGHT:
	csrMove(hWnd,tb,1);	/* 0L 1R 2U 3D */
	break;
    case VK_UP:
	csrMove(hWnd,tb,2);	/* 0L 1R 2U 3D */
	break;
    case VK_DOWN:
	csrMove(hWnd,tb,3);	/* 0L 1R 2U 3D */
	break;
    case VK_HOME:
	if (tb->nCurChar > 0){
	    tb->nCurChar = 0;
	    InvalidateRect(hWnd,&tb->br,TRUE);
	}
	break;
    case VK_END:
	if (tb->nCurChar < tb->cch){
	    tb->nCurChar = tb->cch;
	    InvalidateRect(hWnd,&tb->br,TRUE);
	}
	break;
    case VK_BACK:
	if (tb->cch > 0){
	    procBS(hWnd,tb);
	    cmn.kbs.k1 = 0;
/*	    InvalidateRect(hWnd,&tb->br,TRUE); */
	    RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	}
	break;
    case VK_DELETE:
	if (tb->nCurChar < tb->cch){
	    procDEL(hWnd,tb);
	    cmn.kbs.k1 = 0;
	    RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	}
	break;
    case VK_CONTROL:
	cmn.kbs.ks[0] = 1;
	break;
    case VK_SHIFT:
	if (cmn.kbs.ks[1] == 0){
	    cmn.kbs.ks[1] = 1;
	    RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	}
	break;
    default:
	if (cmn.kbs.ks[0] == 1){
	    switch(wP & 0xff){
	    case ' ':
		cmn.kbs.kimode = 1-cmn.kbs.kimode;
		break;
	    case 'C':
		clipCopy(hWnd);
		break;
	    case 'U':
		tb->cch = 0;
		tb->pchInputBuf[tb->cch] = '\0';
		tb->nCurChar = tb->cch;
		break;
	    case 'X':
		clipCopy(hWnd);
		tb->cch = 0;
		tb->pchInputBuf[tb->cch] = '\0';
		tb->nCurChar = tb->cch;
		break;
	    case 'H':
		if (tb->cch > 0){
		    procBS(hWnd,tb);
		    cmn.kbs.k1 = 0;
		}
		break;
	    case 'Q':
		DestroyWindow(hWnd);
		break;
	    }
	} else {
if (cmn.kbs.kimode == 1){
/*	    HDC hDC = GetDC(hWnd); */
	    int32_t sc = (lP >> 16) & 0xff;
	    int32_t ik = 0;
	    for (int32_t i=0; i<cmn.kbs.nkeys; i++){
		if (sc == cmn.kbs.kky[i][1]){
		    if ((cmn.kbs.kky[i][0] >= 1) && (cmn.kbs.kky[i][0] < 49)){
			if (cmn.kbs.k1 == 0){
			    cmn.kbs.k1 = cmn.kbs.kky[i][0] + cmn.kbs.ks[1]*50;
			} else {
			    ik = cmn.kbs.k1*100 +cmn.kbs.kky[i][0] +cmn.kbs.ks[1]*50;
			    cmn.kbs.k1 = 0;
			}
		    } else if (cmn.kbs.kky[i][0] == 56){
			RECT rc;
			GetWindowRect(hWnd,&rc);
			cmn.kbs.showkb = 1-cmn.kbs.showkb;
			if (cmn.kbs.showkb == 1){
			    MoveWindow(hWnd,rc.left,rc.top,370,280,TRUE);
			} else {
			    MoveWindow(hWnd,rc.left,rc.top,370,170,TRUE);
			}
		    }
		}
	    }
/*	    ReleaseDC(hWnd,hDC); */
	    if ((ik > 0) && (cmn.kbs.tf[ik][0] != 0)){
		if (tb->nCurChar < tb->cch){
		    MoveMemory(&tb->pchInputBuf[tb->nCurChar+1],&tb->pchInputBuf[tb->nCurChar],sizeof(TCHAR)*(tb->cch-tb->nCurChar));
		}
		tb->pchInputBuf[tb->nCurChar++] = cmn.kbs.tf[ik][0];
		tb->pchInputBuf[++tb->cch] = '\0';
	    }
}
	}
/*	InvalidateRect(hWnd,&tb->br,TRUE); */
	RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE);
	break;
    }
    return 0;
}

int32_t procchar( HWND hWnd, struct TBOXBASE *tb, LPARAM lP, WPARAM wP )
{
    if ((cmn.kbs.kimode == 0) && (cmn.kbs.ks[0] == 0)){
	if ((wP >= ' ') && (wP < 0x7f)){
	    if (tb->nCurChar < tb->cch){
		MoveMemory(&tb->pchInputBuf[tb->nCurChar+1],&tb->pchInputBuf[tb->nCurChar],sizeof(TCHAR)*(tb->cch-tb->nCurChar));
	    }
	    tb->pchInputBuf[tb->nCurChar++] = wP & 0xff;
	    tb->pchInputBuf[++tb->cch] = '\0';
	}
    }
    InvalidateRect(hWnd,&tb->br,TRUE);
    return 0;
}

LRESULT CALLBACK WndProc(
HWND hWnd,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
    switch(message){
    case WM_PAINT:
	MsgPaint(hWnd,message,wParam,lParam);
	break;
    case WM_SIZE:
	tbx.dwClientX = LOWORD(lParam);
	tbx.dwClientY = HIWORD(lParam);
	tbx.dwLineLen = tbx.dwClientX-tbx.dwCharX;
	tbx.dwLines = tbx.dwClientY/tbx.dwCharY;
	GetClientRect(hWnd,&tbx.fr);
	SetRect(&tbx.br,10,10,340,100);
	HideCaret(hWnd);
	tbx.nCaretPosX = tbx.br.left;
	tbx.nCaretPosY = tbx.br.top;
	SetCaretPos(tbx.nCaretPosX,tbx.nCaretPosY);
	ShowCaret(hWnd);
	break;
    case WM_SETFOCUS:
	CreateCaret(hWnd,NULL,2,tbx.dwCharY);
	SetCaretPos(tbx.nCaretPosX,tbx.nCaretPosY);
	ShowCaret(hWnd);
	break;
    case WM_KILLFOCUS:
	HideCaret(hWnd);
	DestroyCaret();
	break;
    case WM_CHAR:
	procchar(hWnd,&tbx,lParam,wParam);
	break;
    case WM_KEYDOWN:
	prockydn(hWnd,&tbx,lParam,wParam);
	break;
    case WM_KEYUP:
	prockyup(hWnd,&tbx,lParam,wParam);
	break;
/*
    case WM_SYSKEYDOWN:
	if ((GetKeyState(VK_MENU) & 0x8000) != 0){
	    MessageBox(hWnd,TEXT("ALT"),TEXT("detected"),MB_OK);
	}
	break;
*/
    case WM_LBUTTONDOWN:
	setMpos(hWnd,&tbx,LOWORD(lParam),HIWORD(lParam));
	break;
    case WM_DESTROY:
	PostQuitMessage(0);
	break;
    default:
	return DefWindowProc(hWnd,message,wParam,lParam);
	break;
    }
    return 0;
}

int hex2( char *s )
{
    int32_t b = 0;
    for (int32_t i=0; i<2; i++){
	b *= 16;
	int32_t c = s[i];
	if ((c >= '0') && (c <= '9')){
	    b += c-'0';
	} else if ((c >= 'A') && (c <= 'F')){
	    b += c-'7';
	} else if ((c >= 'a') && (c <= 'f')){
	    b += c-'W';
	}
    }
    return b;
}

int32_t ki_init( struct KIBASE *kbs )
{
    struct GBUF gbf;
/*
struct GBUF gout;
*/
    TCHAR flnm[256];
    lstrcpy(flnm,TEXT("C:\\Users\\user\\ki\\kiKey_106"));
    gbf.hFile = CreateFile(flnm,GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
    if (gbf.hFile != INVALID_HANDLE_VALUE){
	gbf.bResult = ReadFile(gbf.hFile,gbf.buf,BUFSZ,&gbf.nBytesRead,NULL);
	int32_t n = 0;
	int32_t i = 0;
	int32_t j;
	do {
	    j = i+1;
	    while ((j < gbf.nBytesRead) && (gbf.buf[j] != '\n')) j++;
	    if (j >= gbf.nBytesRead){
		j = gbf.nBytesRead-i;
		CopyMemory(gbf.buf,&gbf.buf[i],j);
		gbf.bResult = ReadFile(gbf.hFile,&gbf.buf[j],BUFSZ-j,&gbf.nBytesRead,NULL);
		gbf.nBytesRead += j;
		i = 0;
		j = i+1;
		while ((j < gbf.nBytesRead) && (gbf.buf[j] != '\n')) j++;
	    }
	    kbs->kky[n][0] = atoi(&gbf.buf[i+9]);
	    kbs->kky[n][1] = hex2(&gbf.buf[i]);
	    kbs->kky[n][2] = gbf.buf[i+3]-'0';
	    kbs->kky[n][3] = atoi(&gbf.buf[i+5]);
	    i = j+1;
	    n++;
	} while (gbf.buf[i] >= '0');
	CloseHandle(gbf.hFile);
	kbs->nkeys = n;
/*
	TCHAR bb[16];
	wsprintf(bb,TEXT("%d"),n);
	CopyMemory(&tbx.pchInputBuf[tbx.cch],bb,sizeof(TCHAR)*lstrlen(bb));
	tbx.cch += lstrlen(bb);
*/
    }
    lstrcpy(flnm,TEXT("C:\\Users\\user\\ki\\kiMap"));
    gbf.hFile = CreateFile(flnm,GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
    if (gbf.hFile != INVALID_HANDLE_VALUE){
	int hist[8];
	for (int32_t i=0; i<8; i++){
	    hist[i] = 0;
	}
/*
gout.hFile = CreateFile(TEXT("kie.log"),GENERIC_WRITE,0,NULL,
			OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,NULL);
SetFilePointer(gout.hFile,0L,NULL,FILE_END);
*/
	gbf.bResult = ReadFile(gbf.hFile,gbf.buf,BUFSZ,&gbf.nBytesRead,NULL);
	int32_t n = 0;
	kbs->tf[n][0] = 0;
	kbs->tf[n][1] = 0;
	n++;
	int8_t c;
	int32_t i = 0;
	int32_t j = i;
	do {
	    j = i;
	    while ((j < gbf.nBytesRead) && (gbf.buf[j] != '\n')) j++;
	    if (j >= gbf.nBytesRead){
		j -= i;
		CopyMemory(gbf.buf,&gbf.buf[i],j);
		i = 0;
		gbf.bResult = ReadFile(gbf.hFile,&gbf.buf[j],BUFSZ-j,&gbf.nBytesRead,NULL);
		if (gbf.nBytesRead > 0){
		    gbf.nBytesRead += j;
		    j = i;
		    while ((j < gbf.nBytesRead) && (gbf.buf[j] != '\n')) j++;
		}
	    }
	    c = gbf.buf[i];
	    if (c == '\n'){
		kbs->tf[n][0] = 0;
		kbs->tf[n][1] = 0;
		hist[0]++;
	    } else if (c == '#'){
		kbs->tf[n][0] = 0;
		kbs->tf[n][1] = 0;
		hist[1]++;
	    } else if (c == '\t'){
		kbs->tf[n][0] = 0;
		kbs->tf[n][1] = 0;
		hist[2]++;
	    } else if (((c & 0xf0) == 0xe0) || ((c & 0xe0) == 0xc0)){
		int d = ((c & 0xf0) == 0xe0) ? 3 : 2;
		MultiByteToWideChar(CP_UTF8,MB_PRECOMPOSED,&gbf.buf[i],d,&kbs->tf[n][0],2);
		if ((gbf.buf[i+d] == '\t') || (gbf.buf[i+d] == '\n')){
		    kbs->tf[n][1] = 0;
		    hist[3]++;
		} else if (gbf.buf[i+d] == ' '){
		    kbs->tf[n][1] = ' ';
		    hist[4]++;
		} else {
/* gout.bResult = WriteFile(gout.hFile,&gbf.buf[i],j-i+1,&gout.nBytesRead,NULL); */
		    hist[7]++;
		}
	    } else if ((c >= '0') && (c <= '9')){
		if ((gbf.buf[i+1] >= '0') && (gbf.buf[i+1] <= '9')){
		    kbs->tf[n][0] = gbf.buf[i];
		    kbs->tf[n][1] = gbf.buf[i+1];
		    hist[5]++;
		} else {
		    hist[7]++;
		}
	    } else {
		hist[6]++;
	    }
	    if (j < gbf.nBytesRead){
		i = j+1;
		if (i >= gbf.nBytesRead){
		    gbf.bResult = ReadFile(gbf.hFile,gbf.buf,BUFSZ,&gbf.nBytesRead,NULL);
		    if (gbf.nBytesRead > 0){
			i = 0;
		    } else {
			i = -1;
		    }
		}
	    } else {
		i = -1;
	    }
	    n++;
	} while ((i >= 0) && (n < 10010));
/*
CloseHandle(gout.hFile);
*/
	CloseHandle(gbf.hFile);
/*
	TCHAR bb[64];
	wsprintf(bb,TEXT("0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d n:%d"),
		hist[0],hist[1],hist[2],hist[3],hist[4],hist[5],hist[6],hist[7],n);
	CopyMemory(&tbx.pchInputBuf[tbx.cch],bb,sizeof(TCHAR)*lstrlen(bb));
	tbx.cch += lstrlen(bb);
*/
    }
/*
    tbx.pchInputBuf[tbx.cch] = 0;
*/
    kbs->k1 = 0;
    kbs->showkb = 0;
    kbs->kimode = 0;
    return 0;
}

int32_t t_init( HWND hWnd )
{
    tbx.nCaretPosX = 0;
    tbx.nCaretPosY = 0;
    tbx.nCharWidth = 0;
    tbx.cch = 0;
    tbx.nCurChar = 0;
    HDC hDC = GetDC(hWnd);
    TEXTMETRIC tm;
    GetTextMetrics(hDC,&tm);
    ReleaseDC(hWnd,hDC);
    tbx.dwCharX = tm.tmAveCharWidth;
    tbx.dwCharY = tm.tmHeight;
    tbx.pchInputBuf = malloc(sizeof(TCHAR)*CRTBUFSZ);
    tbx.pchInputBuf[0] = 0;
    tbx.cch = 0;
/*
    lstrcpy(tbx.pchInputBuf,TEXT("Hello, 彩音 wonder!"));
    tbx.cch = lstrlen(tbx.pchInputBuf);
*/
    ki_init(&cmn.kbs);
    return 0;
}

int32_t t_fin()
{
    if (tbx.pchInputBuf != NULL){
	free(tbx.pchInputBuf);
    }
    return 0;
}

int WINAPI WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow)
{
    lstrcpy(cmn.szWindowClass,TEXT("DesktopApp"));
    lstrcpy(cmn.szTitle,TEXT("Windows Desktop Guided Tour Application"));
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance,IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL,IDC_ARROW);
    wcex.hbrBackground = GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = cmn.szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance,IDI_APPLICATION);
    if (RegisterClassEx(&wcex)){
	cmn.hInst = hInstance;
	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		cmn.szWindowClass, cmn.szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 370, 170,
		NULL, NULL, hInstance, NULL);
	if (hWnd){
	    t_init(hWnd);
	    ShowWindow(hWnd,nCmdShow);
	    UpdateWindow(hWnd);
	    MSG msg;
	    while (GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	    t_fin();
	    return (int)msg.wParam;
	} else {
	    MessageBox(NULL,TEXT("Call to CreateWindow failed!"),TEXT("Windows Desktop Guided Tour"),0);
	    return 0;
	}
    } else {
	MessageBox(NULL,TEXT("Call to RegisterClassEx failed!"),TEXT("Windows Desktop Guided Tour"),0);
	return 0;
    }
}
