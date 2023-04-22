/*
*F: k1.c
* Coded at: 2022.7.9 by T. Abi
*/
#include <stdint.h>
#include <cairo.h>
#include <gtk/gtk.h>

#define FIBUFSZ 1024
#define FIKIM00 "/opt/ki/bks/M00"
#define FIKIM01 "/opt/ki/bks/M01"
#define KYBUFSZ 64000

struct KIBASE {
int32_t nkeys;
uint8_t kky[80][4];	/* 0kno 1kcd 2ypos 3xpos */
uint8_t tf[10000][4];
uint8_t k1;
uint8_t dmy[3];
int32_t ks[2];		/* 0control 1shift */
int32_t showkb;		/* 0not 1show */
int32_t kimode;		/* 0raw 1ki */
};

struct TBOXBASE {
double fRect[4];	/* frame rectangle x,y,w,h */
double bRect[4];	/* buffer rectangle x,y,w,h */
int32_t fontSize;
int32_t lineHeight;
int32_t nb;		/* number of bytes in iBuf */
uint8_t *iBuf;		/* address of input buffer */
int32_t pcsr;		/* cursor point */
double csrpos[2];
int32_t csrdir;		/* 0L 1R 2U 3D 4- */
};

static struct COMBASE {
GtkWidget *window;
GtkWidget *darea;
struct KIBASE kbs;
struct TBOXBASE tbx;
} cmn;

int32_t get_kikey( struct KIBASE *ki, char *flnm )
{
    int32_t ierr = 0;
    char buf[FIBUFSZ];
    int32_t bsz = FIBUFSZ;
    FILE *fp = fopen(flnm,"rb");
    if (fp != NULL){
	int32_t nR = fread(buf,1,bsz,fp);
	int32_t n = 0;
	int32_t i = 0;
	int32_t j;
	do {
	    j = i+1;
	    while ((j < nR) && (buf[j] != '\n')) j++;
	    if (j >= nR){
		j = nR-i;
		memcpy(buf,&buf[i],j);
		nR = fread(&buf[j],1,bsz-j,fp);
		nR += j;
		i = 0;
		j = i+1;
		while ((j < nR) && (buf[j] != '\n')) j++;
	    }
	    ki->kky[n][0] = atoi(&buf[i+10]);
	    ki->kky[n][1] = atoi(&buf[i]);
	    ki->kky[n][2] = buf[i+4]-'0';
	    ki->kky[n][3] = atoi(&buf[i+6]);
	    i = j+1;
	    n++;
	} while (buf[i] >= '0');
	fclose(fp);
	ki->nkeys = n;
    } else {
	printf("%s not found.\n",flnm);
	ierr = 1;
    }
    return ierr;
}

int32_t get_kimap( struct KIBASE *ki, char *flnm )
{
    int32_t ierr = 0;
    char buf[FIBUFSZ];
    int32_t bsz = FIBUFSZ;
    FILE *fp = fopen(flnm,"rb");
    if (fp != NULL){
/*
FILE *fOut = fopen("kie.log","a");
*/
	int32_t nR = fread(buf,1,bsz,fp);
	int32_t n = 0;
	ki->tf[n][0] = 0;
	ki->tf[n][1] = 0;
	ki->tf[n][2] = 0;
	ki->tf[n][3] = 0;
	n++;
	int8_t c;
	int32_t i = 0;
	int32_t j = i;
	do {
	    j = i;
	    while ((j < nR) && (buf[j] != '\n')) j++;
	    if (j >= nR){
		j -= i;
		memcpy(buf,&buf[i],j);
		i = 0;
		nR = fread(&buf[j],1,bsz-j,fp);
		if (nR > 0){
		    nR += j;
		    j = i;
		    while ((j < nR) && (buf[j] != '\n')) j++;
		}
	    }
	    c = buf[i];
	    if ((c == '\n') || (c == '#') || (c == '\t')){
		ki->tf[n][0] = 0;
		ki->tf[n][1] = 0;
		ki->tf[n][2] = 0;
		ki->tf[n][3] = 0;
	    } else if ((c & 0xf0) == 0xe0){
		ki->tf[n][0] = buf[i];
		ki->tf[n][1] = buf[i+1];
		ki->tf[n][2] = buf[i+2];
		ki->tf[n][3] = (buf[i+3] == ' ') ? ' ' : 0;
	    } else if ((c & 0xe0) == 0xc0){
		ki->tf[n][0] = buf[i];
		ki->tf[n][1] = buf[i+1];
		ki->tf[n][2] = 0;
		ki->tf[n][3] = 0;
	    } else if ((c >= '0') && (c <= '9')){
		if ((buf[i+1] >= '0') && (buf[i+1] <= '9')){
		    ki->tf[n][0] = buf[i];
		    ki->tf[n][1] = buf[i+1];
		    ki->tf[n][2] = 0;
		    ki->tf[n][3] = 0;
		}
	    } else if (strncmp(&buf[i],"MR",2) == 0){
		ki->tf[n][0] = buf[i];
		ki->tf[n][1] = buf[i+1];
		ki->tf[n][2] = 0;
		ki->tf[n][3] = 0;
	    }
	    if (j < nR){
		i = j+1;
		if (i >= nR){
		    nR = fread(buf,1,bsz,fp);
		    if (nR > 0){
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
fclose(fOut);
*/
	fclose(fp);
    } else {
	printf("%s not found.\n",flnm);
	ierr = 1;
    }
    return ierr;
}

int32_t ki_init( struct KIBASE *ki )
{
    int32_t ierr = get_kikey(ki,FIKIM00);
    if (ierr == 0){
	ierr = get_kimap(ki,FIKIM01);
    }
    ki->k1 = 0;
    ki->ks[0] = 0;	/* Ctrl */
    ki->ks[1] = 0;	/* Shift */
    ki->showkb = 0;
    ki->kimode = 0;
    return ierr;
}

int32_t kiman(
struct KIBASE *kb,
int32_t cd,
uint8_t *s,
int32_t n )
{
    int32_t ierr = 0;
    switch(cd){
    case 0:	/* init */
	ki_init(kb);
	break;
    case 1:	/* fin */
	break;
    default:
	break;
    }
    return ierr;
}

int32_t tbxman(
struct TBOXBASE *tb,
int32_t cd,
uint8_t *s,
int32_t n )
{
    int32_t ierr = 0;
    switch(cd){
    case 0:	/* init */
	memset(tb->fRect,0,sizeof(tb->fRect));
	double *rc = tb->bRect;	/* x,y,w,h */
	rc[0] = 10; rc[1] = 10; rc[2] = 350; rc[3] = 100;
	tb->fontSize = 16;
	tb->lineHeight = tb->fontSize*1.2;
	tb->pcsr = -1;
	tb->csrpos[0] = rc[0];
	tb->csrpos[1] = rc[1];
	tb->csrdir = 4;
	tb->iBuf = malloc(KYBUFSZ);
	if (tb->iBuf != NULL){
	    tb->nb = 0;
	    tb->iBuf[tb->nb] = '\0';
/*
	    memcpy(tb->iBuf,"bag",3);
	    tb->iBuf[3] = '\0';
	    tb->nb = 3;
*/
	} else {
	    ierr = 1;
	}
	break;
    case 1:	/* fin */
	if (tb->iBuf != NULL){
	    free(tb->iBuf);
	}
	break;
    default:
	break;
    }
    return ierr;
}

static void destroy( GtkWidget *widget, gpointer data )
{
    gtk_main_quit();
}

static void showKbd(
cairo_t *cr,
struct TBOXBASE *tb,
struct KIBASE *ki )
{
    char cbuf[8];
    int32_t x,y;
    int32_t b0 = (ki->ks[1] == 1) ? 50 : 0;
    for (int32_t i=0; i<ki->nkeys; i++){
	if (ki->kky[i][3] > 0){
	    x = ki->kky[i][3]*8 +8;
	    y = ki->kky[i][2]*20 +110;
	    if ((ki->kky[i][0] >= 1) && (ki->kky[i][0] <= 49)){
		int32_t ii;
		if (ki->k1 == 0){
		    ii = b0*100+ki->kky[i][0]*100;
		    if (ki->tf[ii][0] != 0){
			memcpy(cbuf,ki->tf[ii],4);
			cbuf[4] = '\0';
/*
			if (((ki->tf[ii][0] & 0xf0) == 0xe0) && (ki->tf[ii][3] == ' ')){
			    memcpy(cbuf,ki->tf[ii],4);
			    cbuf[4] = '\0';
			} else {
			    memcpy(cbuf,&ki->tf[ii],4);
			}
*/
			cairo_move_to(cr,x,y+tb->fontSize);
			cairo_show_text(cr,cbuf);
		    } else {
			cairo_rectangle(cr,x+5,y+5,9,9);
			cairo_stroke(cr);
		    }
		} else {
		    ii = ki->k1*100+b0+ki->kky[i][0];
		    if (ki->k1 == b0+ki->kky[i][0]){
			cairo_rectangle(cr,x-1,y-1,20,20);
			cairo_stroke(cr);
		    }
		    if (ki->tf[ii][0] != 0){
			memcpy(cbuf,ki->tf[ii],4);
			cbuf[4] = '\0';
/*
			if (((ki->tf[ii][0] & 0xf0) == 0xe0) && (ki->tf[ii][3] == ' ')){
			    memcpy(cbuf,ki->tf[ii],4);
			    cbuf[4] = '\0';
			} else {
			    memcpy(cbuf,&ki->tf[ii],4);
			}
*/
			cairo_move_to(cr,x,y+tb->fontSize);
			cairo_show_text(cr,cbuf);
		    } else {
			cairo_rectangle(cr,x+5,y+5,9,9);
			cairo_stroke(cr);
		    }
		}
	    } else {
		cairo_rectangle(cr,x+5,y+5,9,9);
		cairo_stroke(cr);
	    }
	}
    }
    x = 18*8;
    y = 3.5*20+110;
    cairo_rectangle(cr,x+3,y-1,3,3);
    cairo_stroke(cr);
    x = 24*8;
    cairo_rectangle(cr,x+2,y-1,3,3);
    cairo_stroke(cr);
}

static void showMode(
cairo_t *cr,
struct TBOXBASE *tb,
struct KIBASE *ki )
{
    char bb[8];
    double *rc = tb->bRect;
    double x = rc[0]+rc[2]-tb->fontSize;
    double y = rc[1]+rc[3]+tb->fontSize;
    if (ki->kimode == 0){
	strcpy(bb,"ki");
    } else {
	strcpy(bb,"き");
    }
    cairo_move_to(cr,x,y);
    cairo_show_text(cr,bb);
}

static void showCaret(
cairo_t *cr,
struct TBOXBASE *tb )
{
    double x = tb->csrpos[0];
    double y = tb->csrpos[1]+tb->fontSize;
    double h = tb->fontSize/2;
    cairo_rectangle(cr,x,y,h,3);
    cairo_stroke(cr);
}

static void myDrawText(
cairo_t *cr,
struct TBOXBASE *tb )
{
    char cbuf[8];
    cairo_text_extents_t te;
    int32_t pb = 0;
    int32_t pb0 = 0;
    double cx = tb->bRect[0];
    double cy = tb->bRect[1];
    double tx = tb->bRect[0]+tb->bRect[2];
    double ty = tb->bRect[1]+tb->bRect[3]*2;
    if (tb->pcsr >= 0){
	if ((tb->csrdir == 2) || (tb->csrdir == 3)){
	    tx = tb->csrpos[0];
	    ty = tb->csrpos[1];
	    if ((tb->csrdir == 2) && (ty > cy)){
		ty -= tb->lineHeight;
		tb->pcsr = -1;
	    } else if (tb->csrdir == 3){
		ty += tb->lineHeight;
		tb->pcsr = -1;
	    }
	}
    }
    uint8_t *p;
    while (tb->iBuf[pb] != '\0'){
	pb0 = pb;
	p = &tb->iBuf[pb];
	if ((*p & 0xf0) == 0xe0){
	    memcpy(cbuf,p,3);
	    cbuf[3] = '\0';
	    pb += 3;
	} else if ((*p & 0xe0) == 0xc0){
	    cbuf[0] = p[0];
	    cbuf[1] = p[1];
	    cbuf[2] = '\0';
	    pb += 2;
	} else if ((*p & 0x80) == 0){
	    cbuf[0] = p[0];
	    cbuf[1] = '\0';
	    pb++;
	}
	cairo_text_extents(cr,cbuf,&te);
	if (cx+te.x_bearing+te.width > tb->bRect[0]+tb->bRect[2]){
	    cx = tb->bRect[0];
	    cy += tb->lineHeight;
	}
	cairo_move_to(cr,cx,cy+tb->fontSize);
	cairo_show_text(cr,cbuf);
	if ((tb->pcsr == -1) && (ty < cy+tb->lineHeight) && (cy <= ty)){
	    if ((cx <= tx) && (tx < cx+te.x_advance)){
		tb->csrpos[0] = cx;
		tb->csrpos[1] = cy;
		if (tx-cx < te.x_advance/2){
		    tb->pcsr = pb0;
		} else {
		    tb->pcsr = pb;
		    tb->csrpos[0] += te.x_advance;
		}
	    }
	}
	cx += te.x_advance;
	if (cx > tb->bRect[0]+tb->bRect[2]){
	    cx = tb->bRect[0];
	    cy += tb->lineHeight;
	}
	if (pb == tb->pcsr){
	    tb->csrpos[0] = cx;
	    tb->csrpos[1] = cy;
	}
    }
    if (tb->pcsr == -1){
	tb->csrpos[0] = cx;
	tb->csrpos[1] = cy;
    }
    tb->csrdir = 4;
}

static gboolean on_draw_event(
GtkWidget *widget,
cairo_t *cr,
gpointer data )
{
    struct TBOXBASE *tb = &cmn.tbx;
    cairo_set_source_rgb(cr,0.1,0,0);
    cairo_set_line_width(cr,1.0);
    cairo_set_font_size(cr,tb->fontSize);
    double *rc = tb->bRect;
    cairo_rectangle(cr,rc[0],rc[1],rc[2],rc[3]);
    cairo_stroke(cr);
    showMode(cr,tb,&cmn.kbs);
    if ((cmn.kbs.showkb == 1) && (cmn.kbs.kimode == 1)){
	showKbd(cr,tb,&cmn.kbs);
    }
    myDrawText(cr,tb);
    showCaret(cr,tb);
    return FALSE;
}

static int32_t prockyrelease(
GtkWidget *widget,
GdkEventKey *event )
{
    int32_t rdrw = 0;
    struct KIBASE *ki = &cmn.kbs;
    guint16 hwkc = event->hardware_keycode;
    if ((hwkc == 37) || (hwkc == 109)){	/* LCTL or RCTL */
	ki->ks[0] = 0;
    } else if ((hwkc == 50) || (hwkc == 62)){	/* LFSH or RTSH */
	if (ki->ks[1] == 1){
	    ki->ks[1] = 0;
	    rdrw = 1;
	}
    }
    return rdrw;
}

static void csrMove( struct TBOXBASE *tb, int32_t cd )
{
    switch(cd){
    case 0:	/* 0L 1R 2U 3D */
	if (tb->nb > 0){
	    int32_t pt = (tb->pcsr == -1) ? tb->nb : tb->pcsr;
	    if (pt > 0){
		do {
		    pt--;
		} while ((pt > 0) && ((tb->iBuf[pt] & 0xc0) == 0x80));
		tb->pcsr = pt;
	    }
	}
	break;
    case 1:	/* 0L 1R 2U 3D */
	if (tb->nb > 0){
	    int32_t pt = (tb->pcsr == -1) ? tb->nb : tb->pcsr;
	    if (pt < tb->nb){
		uint8_t c = tb->iBuf[pt];
		if ((c & 0xf0) == 0xe0){
		    pt += 3;
		} else if ((c & 0xe0) == 0xc0){
		    pt += 2;
		} else if ((c & 0x80) == 0){
		    pt++;
		} 
		tb->pcsr = (pt == tb->nb) ? -1 : pt;
	    }
	}
	break;
    case 2:	/* 0L 1R 2U 3D */
	if (tb->csrpos[1] > tb->bRect[1]+tb->fontSize){
	    tb->csrdir = 2;
	}
	break;
    case 3:	/* 0L 1R 2U 3D */
	if (tb->csrpos[1] < tb->bRect[1]+tb->bRect[3]-tb->fontSize){
	    tb->csrdir = 3;
	}
	break;
    }
}

static void procDEL( struct TBOXBASE *tb )
{
    if (tb->nb > 0){
	if (tb->pcsr == -1){
	    csrMove(tb,0);
	}
	int32_t pt = tb->pcsr;
	int32_t pf = pt;
	uint8_t c = tb->iBuf[pt];
	if ((c & 0xf0) == 0xe0){
	    pf += 3;
	} else if ((c & 0xe0) == 0xc0){
	    pf += 2;
	} else if ((c & 0x80) == 0){
	    pf++;
	} 
	if (pf == tb->nb){
	    tb->nb = pt;
	    tb->iBuf[pt] = '\0';
	    tb->pcsr = -1;
	} else {
	    memmove(&tb->iBuf[pt],&tb->iBuf[pf],tb->nb-pf);
	    tb->nb -= pf-pt;
	    tb->iBuf[tb->nb] = '\0';
	    tb->pcsr = pt;
	}
    }
}

static void procBS( struct TBOXBASE *tb )
{
    if (tb->nb > 0){
	if (tb->pcsr == -1){
	    if ((tb->iBuf[tb->nb-1] & 0xc0) == 0x80){
		while ((tb->nb > 0) && ((tb->iBuf[tb->nb-1] & 0xc0) == 0x80)){
		    tb->nb--;
		}
	    }
	    tb->iBuf[--tb->nb] = '\0';
	} else {
	    if (tb->pcsr > 0){
		csrMove(tb,0);
	    }
	    procDEL(tb);
	}
    }
}

static void insChar(
struct TBOXBASE *tb, 
uint8_t *ch0 )
{
    int32_t pt = (tb->pcsr == -1) ? tb->nb : tb->pcsr;
    int32_t sz = 0;
    if ((*ch0 & 0xf0) == 0xe0){
	sz = 3;
    } else if ((*ch0 & 0xe0) == 0xc0){
	sz = 2;
    } else if ((*ch0 & 0x80) == 0){
	sz = 1;
    }
    if (sz > 0){
	if (pt < tb->nb){
	    memmove(&tb->iBuf[pt+sz],&tb->iBuf[pt],tb->nb-pt);
	    tb->pcsr += sz;
	}
	memcpy(&tb->iBuf[pt],ch0,sz);
	tb->nb += sz;
	tb->iBuf[tb->nb] = '\0';
    }
}

static void clipCopy( int32_t cd )
{
    GtkClipboard *gcb = gtk_widget_get_clipboard(GTK_WIDGET(cmn.window),GDK_SELECTION_CLIPBOARD);
    struct TBOXBASE *tb = &cmn.tbx;
    gtk_clipboard_set_text(gcb,(char *)tb->iBuf,tb->nb);
    if (cd == 1){
	tb->nb = 0;
	tb->iBuf[tb->nb] = '\0';
	tb->pcsr = -1;
    }
}

static int32_t prockypress(
GtkWidget *widget,
GdkEventKey *event )
{
    int32_t rdrw = 0;
    struct TBOXBASE *tb = &cmn.tbx;
    struct KIBASE *ki = &cmn.kbs;
    guint16 hwkc = event->hardware_keycode;
    if (hwkc == 113){	/* LEFT */
	csrMove(tb,0);
	rdrw = 1;
    } else if (hwkc == 114){	/* RGHT */
	csrMove(tb,1);
	rdrw = 1;
    } else if (hwkc == 111){	/* UP */
	csrMove(tb,2);
	rdrw = 1;
    } else if (hwkc == 116){	/* DOWN */
	csrMove(tb,3);
	rdrw = 1;
    } else if (hwkc == 110){	/* HOME */
	if (tb->nb > 0){
	    tb->pcsr = 0;
	}
	rdrw = 1;
    } else if (hwkc == 115){	/* END */
	tb->pcsr = -1;
	rdrw = 1;
    } else if (hwkc == 22){	/* BKSP */
	procBS(tb);
	ki->k1 = 0;
	rdrw = 1;
    } else if (hwkc == 119){	/* DEL */
	procDEL(tb);
	ki->k1 = 0;
	rdrw = 1;
    } else if ((hwkc == 37) || (hwkc == 109)){	/* LCTL or RCTL */
	ki->ks[0] = 1;
	if (ki->k1 != 0){
	    ki->k1 = 0;
	    rdrw = 1;
	}
    } else if ((hwkc == 50) || (hwkc == 62)){	/* LFSH or RTSH */
	if (ki->ks[1] == 0){
	    ki->ks[1] = 1;
	    rdrw = 1;
	}
    } else if (hwkc == 100){	/* K56 XFER */
	ki->showkb = 1-ki->showkb;
	if (ki->showkb == 1){
	    gtk_widget_set_size_request(GTK_WIDGET(cmn.darea),370,240);
/*	    gtk_window_set_default_size(GTK_WINDOW(cmn.window),370,240); */
	} else {
	    gtk_widget_set_size_request(GTK_WIDGET(cmn.darea),370,140);
/*	    gtk_window_set_default_size(GTK_WINDOW(cmn.window),370,140); */
	}
	rdrw = 1;
    } else {
	guint kv = event->keyval;
	if ((event->state & GDK_CONTROL_MASK) != 0){
	    if (kv == GDK_KEY_space){
		ki->kimode = 1-ki->kimode;
		rdrw = 1;
	    } else if ((kv == GDK_KEY_C) || (kv == GDK_KEY_c)){
		clipCopy(0);	/* leave */
	    } else if ((kv == GDK_KEY_X) || (kv == GDK_KEY_x)){
		clipCopy(1);	/* delete */
		rdrw = 1;
	    } else if ((kv == GDK_KEY_U) || (kv == GDK_KEY_u)){
		tb->nb = 0;
		tb->iBuf[0] = '\0';
		rdrw = 1;
	    } else if ((kv == GDK_KEY_Q) || (kv == GDK_KEY_q)){
		gtk_main_quit();
	    }
	} else {
	    if (ki->kimode == 1){
		int32_t ik = 0;
		for (int32_t i=0; i<ki->nkeys; i++){
		    if (hwkc == ki->kky[i][1]){
			if ((ki->kky[i][0] >= 1) && (ki->kky[i][0] < 49)){
			    if (ki->k1 == 0){
				ki->k1 = ki->kky[i][0] + ki->ks[1]*50;
			    } else {
				ik = ki->k1*100 +ki->kky[i][0] +ki->ks[1]*50;
				ki->k1 = 0;
			    }
			    rdrw = 1;
			} else if (ki->kky[i][0] == 56){	/* K56 XFER */
			    ki->showkb = 1-ki->showkb;
			    if (ki->showkb == 1){
				gtk_widget_set_size_request(GTK_WIDGET(cmn.darea),370,240);
			    } else {
				gtk_widget_set_size_request(GTK_WIDGET(cmn.darea),370,140);
			    }
			    rdrw = 1;
			}
		    }
		}
		if ((ik > 0) && (ki->tf[ik][0] != 0)){
		    insChar(tb,ki->tf[ik]);
		    rdrw = 1;
		}
	    } else {
		if ((kv >= ' ') && (kv <= '~')){
		    int32_t pt = (tb->pcsr == -1) ? tb->nb : tb->pcsr;
		    if (pt < tb->nb){
			memmove(&tb->iBuf[pt+1],&tb->iBuf[pt],tb->nb-pt);
			tb->pcsr++;
		    }
		    tb->iBuf[pt] = kv;
		    tb->iBuf[++tb->nb] = '\0';
		    rdrw = 1;
		}
	    }
	}
    }
    return rdrw;
}

static gboolean on_keyrelease_event(
GtkWidget *widget,
GdkEventKey *event,
gpointer data )
{
    int32_t rdrw = prockyrelease(widget,event);
    if (rdrw == 1){
	gtk_widget_queue_draw(widget);
	return TRUE;
    }
    return FALSE;
}

static gboolean on_keypress_event(
GtkWidget *widget,
GdkEventKey *event,
gpointer data )
{
    int32_t rdrw = 0;
    rdrw = prockypress(widget,event);
    if (rdrw == 1){
	gtk_widget_queue_draw(widget);
	return TRUE;
    }
    return FALSE;
}

int32_t appini( struct COMBASE *cm )
{
    int32_t ierr = tbxman(&cm->tbx,0,NULL,0);
    if (ierr == 0){
	ierr = kiman(&cm->kbs,0,NULL,0);
    }
    if (ierr == 0){
	cm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(cm->window),"k0win");
	cm->darea = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(cm->darea),370,140);
	gtk_container_add(GTK_CONTAINER(cm->window),cm->darea);
/*	g_object_set_property(G_OBJECT(cm->darea),"gtk-cursor-blink",TRUE); */
	g_signal_connect(G_OBJECT(cm->darea),"draw",G_CALLBACK(on_draw_event),NULL);
	gtk_widget_add_events(cm->window,GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(cm->window),"key_press_event",G_CALLBACK(on_keypress_event),NULL);
	gtk_widget_add_events(cm->window,GDK_KEY_RELEASE_MASK);
	g_signal_connect(G_OBJECT(cm->window),"key_release_event",G_CALLBACK(on_keyrelease_event),NULL);
	g_signal_connect(cm->window,"destroy",G_CALLBACK(destroy),NULL);
/*	gtk_widget_set_size_request(GTK_WIDGET(cm->window),370,140); */
/*	gtk_window_set_default_size(GTK_WINDOW(cmn.window),370,140); */
	gtk_widget_show_all(GTK_WIDGET(cm->window));
    }
    return ierr;
}

int32_t appfin( struct COMBASE *cm )
{
    int32_t ierr = tbxman(&cm->tbx,1,NULL,0);
    return ierr;
}

int main( int argc, char *argv[] )
{
    gtk_init(&argc,&argv);
    int32_t ierr = appini(&cmn);
    if (ierr == 0){
	gtk_main();
	appfin(&cmn);
    }
    return 0;
}

