/**********************************************************************
*F: ctb
* Coded at: 2008.10.20 by T. Abi
* Debugged: 2009.2.17 fixed the minimum length as correct (maxline)
* Modified: 2010.7.27 to save b64 result into a file
* Modified: 2013.6.27 add list7z
* Modified: 2014.4.9 u8h2u utf-8 hex to U+nnnn, u2h8u vice versa
* Modifyed: 2015.6.18 added b2url url2b
* Debugged: 2015.6.19 added fflag=0 for b2url and url2b
* Modified: 2015.8.27 added nullitems
* Modified: 2016.3.3 added b62 B62
* Modified: 2017.8.22 adding cut
* Modified: 2017.9.27 added genx0
* Modified: 2020.9.2 added chkbom
* Modified: 2021.2.1 changed (unsigned char *) to (char *) for buf
* Modified: 2021.11.11 added cutl
* Modified: 2021.11.17 added gather
* Modified: 2021.11.25 added ebgcl
* Modified: 2022.2.23 c11 style
* Modified: 2022.8.6 added genpw xlsdt
**********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>	/* fstat, genx0 */
#include <ctype.h>	/* for crlf, maxline */
#include <fcntl.h>
#include <unistd.h>

/**********************************************************************
*F: vsep
*	ctb vsep k hfile src L R
* Coded at: 2017.8.23 by T. Abi
**********************************************************************/
struct VSEPBASE {
char *hf;
FILE *fh;
char *sf;
FILE *fs;
char *lf;
FILE *fl;
char *rf;
FILE *fr;
int32_t k;
int32_t form;	/* 0:noKey 1:withKey */
};

int32_t vsepman( struct VSEPBASE *vb, int32_t cd, int argc, char *argv[] )
{
    int32_t ierr = 0;
    switch(cd){
    case 0:	/* set */
	ierr = 1;
	vb->k = atoi(argv[2]);
	vb->form = atoi(argv[7]);
	int32_t n = 0;
	for (int32_t i=3; i<=6; i++){
	    n += strlen(argv[i])+5;
	}
	vb->hf = malloc(n);
	if (vb->hf != NULL){
	    int i0,i1;
	    strcpy(vb->hf,argv[3]);
	    vb->fh = fopen(vb->hf,"rb");
	    if (vb->fh != NULL){
		i0 = strlen(vb->hf)+1;
		if ((i1=(i0 & 3)) != 0) i0 += 4 - i1;
		vb->sf = &vb->hf[i0];
		strcpy(vb->sf,argv[4]);
		vb->fs = fopen(vb->sf,"rb");
		if (vb->fs != NULL){
		    i0 = strlen(vb->sf)+1;
		    if ((i1=(i0 & 3)) != 0) i0 += 4 - i1;
		    vb->lf = &vb->sf[i0];
		    strcpy(vb->lf,argv[5]);
		    vb->fl = fopen(vb->lf,"wb");
		    if (vb->fl != NULL){
			i0 = strlen(vb->lf)+1;
			if ((i1=(i0 & 3)) != 0) i0 += 4 - i1;
			vb->rf = &vb->lf[i0];
			strcpy(vb->rf,argv[6]);
			vb->fr = fopen(vb->rf,"wb");
			if (vb->fr != NULL){
			    ierr = 0;
/*
fprintf(stderr,"%d %s %s %s %s\n",vb->k,vb->hf,vb->sf,vb->lf,vb->rf);
*/
			}
		    }
		}
	    }

	}
	if (ierr == 1){
	    free(vb->hf);
	}
	break;
    case 1:	/* close */
	fclose(vb->fr);
	fclose(vb->fl);
	fclose(vb->fs);
	fclose(vb->fh);
	break;
    }
    return ierr;
}

int32_t vsep( int argc, char *argv[] )
{
    struct VSEPBASE vbs;
    int32_t ierr = vsepman(&vbs,0,argc,argv);	/* set */
    if (ierr == 0){
	char hbuf[16],sbuf[4096],*ph;
	int32_t bsz = 4000;
	int32_t rsz = fread(sbuf,1,bsz,vbs.fs);
	int32_t ns = 0;	/* number of separators */
	int32_t j0 = 0;
	int32_t j1,h0,h1;
	int32_t sta = 0;	/* 0:left 1:right */
	ph = fgets(hbuf,16,vbs.fh);
	while (ph != NULL){
	    h0 = strlen(ph);
	    for (h1=h0; (h1>0)&&(ph[h1-1]<' '); h1--);
	    if (sta == 0){
		for (int32_t j=j0; (j<rsz)&&(sta==0); j++){
		    if (sbuf[j] == ','){
			ns++;
/*
fprintf(stderr,"%d/%d ",ns,vbs.k);
*/
			if (ns == vbs.k){
			    j1 = j+1;
			    if (vbs.form == 1){
				fwrite(&sbuf[j0],1,j1-j0,vbs.fl);
				fwrite(ph,1,h0,vbs.fl);
			    } else {
				fwrite(&sbuf[j0],1,j-j0,vbs.fl);
				fwrite("\r\n",1,2,vbs.fl);
			    }
			    sta = 1;
			    if (vbs.form == 1){
				fwrite(ph,1,h1,vbs.fr);
				fputc(',',vbs.fr);
			    }
			    j0 = j1;
			}
		    }
		}
		if (sta == 0){
		    fwrite(&sbuf[j0],1,rsz-j0,vbs.fl);
		    j0 = rsz;
		}
		if (j0 >= rsz){
		    rsz = fread(sbuf,1,bsz,vbs.fs);
		    j0 = 0;
		}
	    }
	    if (sta == 1){
		for (int32_t j=j0; (j<rsz)&&(sta==1); j++){
		    if (sbuf[j] == 10){
			j1 = j+1;
			fwrite(&sbuf[j0],1,j1-j0,vbs.fr);
			ns = 0;
			j0 = j1;
			sta = 0;
			ph = fgets(hbuf,16,vbs.fh);
		    }
		}
		if (sta == 1){
		    fwrite(&sbuf[j0],1,rsz-j0,vbs.fr);
		    j0 = rsz;
		}
		if (j0 >= rsz){
		    rsz = fread(sbuf,1,bsz,vbs.fs);
		    j0 = 0;
		}
	    }
	}
	ierr = vsepman(&vbs,1,0,NULL);	/* close */
    }
    return ierr;
}

int32_t vjoin( int argc, char *argv[] )
{
    char buf[8192];
    int32_t bsz = 8000;
    FILE *fa = fopen(argv[2],"rb");
    FILE *fb = fopen(argv[3],"rb");
    FILE *fc = fopen(argv[4],"wb");
    char *p = fgets(buf,bsz,fa);
    char *q;
    while (p != NULL){
	int32_t n = strlen(p);
	while (p[n-1] < ' ') n--;
	p[n++] = ',';
	q = fgets(&p[n],bsz,fb);
	if (q != NULL){
	    fwrite(p,1,strlen(p),fc);
	}
	p = fgets(buf,bsz,fa);
    }
    fclose(fc);
    fclose(fb);
    fclose(fa);
    return 0;
}

/**********************************************************************
*F: list7z
*	ctb list7z [source [result]]
* Coded at: 2013.6.27 by T. Abi
**********************************************************************/
int32_t list7z( int argc, char *argv[] )
{
    int32_t ierr = 0;
    if (argc < 3){
	char buf[256],buf2[256],*p,*q;
	int32_t st = 0;
	p = fgets(buf,256,stdin);
	while ((st == 0) && (feof(stdin) == 0)){
/*
	    if (strstr(buf,"   Date") == buf){
*/
	    if (strstr(buf,"Physical Size") == buf){
		st = 1;
	    }
/*
	    printf("%s",buf);
*/
	    p = fgets(buf,256,stdin);
	}
	while (feof(stdin) == 0){
	    p = buf;
	    q = buf2;
	    while (*p != '\0'){		/* utf-8 11 bit */
		if (((p[0] & 0xe0) == 0xc0) && ((p[1] & 0xc0) == 0x80)){
		    *(q++) = ((p[0] & 3) << 6) | (p[1] & 0x3f);
		    if ((p[0] & 0x1c) != 0){
			*(q++) = (p[0] & 0x1c) >> 2;
		    }
		    p += 2;
		} else {
		    *(q++) = *(p++);
		}
	    }
	    *q = '\0';
	    printf("%s",buf2);
	    p = fgets(buf,256,stdin);
	}
    } else {
	printf("%s",argv[2]);
    }
    return ierr;
}

/**********************************************************************
*F: portion
*	ctb portion range [source [result]]
*	range = skip,(end+1|+size|-size)
* Coded at: 2009.4.24 by T. Abi
**********************************************************************/
struct PORTIONBASE {
char *src;
char *dst;
int32_t spos;
int32_t npos;	/* next block top */
int32_t bsiz;
};

int32_t portionparse( struct PORTIONBASE *pb, int argc, char *argv[] )
{
    int32_t ierr = 0;
    if (argc > 2){
	char *p = strchr(argv[2],',');
	if (p != NULL){
	    if (argv[2][0] == '$'){
		pb->spos = 0;
	    } else {
		pb->spos = atoi(argv[2]);
	    }
	    switch(p[1]){
	    case '+':
		pb->bsiz = atoi(&p[2]);
		pb->npos = pb->spos+pb->bsiz;
		break;
	    case '-':
		pb->bsiz = atoi(&p[2]);
		if (pb->spos == 0){
		    pb->npos = -1;
		} else if (pb->bsiz <= pb->spos){
		    pb->npos = pb->spos;
		    pb->spos = pb->npos-pb->bsiz;
		} else {
		    fprintf(stderr,"Illegal range.\n");
		    ierr = 1;
		}
		break;
	    default:
		pb->npos = atoi(&p[1]);
		pb->bsiz = pb->npos-pb->spos;
		break;
	    }
	} else {
	    fprintf(stderr,"Bad range format.\n");
	    ierr = 1;
	}
    } else {
	fprintf(stderr,"Too few parameters.\n");
	ierr = 1;
    }
    return ierr;
}

int32_t aportion( struct PORTIONBASE *pb, char *src, char *dst )
{
    char buf[256];
    int32_t n,sz;
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	if (strcmp(src,"stdin") == 0){
	    if (pb->npos == -1){
		fprintf(stderr,"reverse position setting ignored for stdin.\n");
	    }
	    n = 1;
	    sz = pb->spos;
	    while ((sz >= 256) && (n > 0)){
		n = fread(buf,1,256,fi);
		sz -= n;
	    }
	    if ((sz < 256) && (n > 0)){
		n = fread(buf,1,sz,fi);
	    }
	} else {
	    if (pb->npos == -1){
		fseek(fi,-pb->bsiz,SEEK_END);
	    } else {
		fseek(fi,pb->spos,SEEK_SET);
	    }
	}
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    sz = pb->bsiz;
	    n = fread(buf,1,256,fi);
	    while ((n > 0) && (n < sz)){
		fwrite(buf,1,n,fo);
		sz -= n;
		n = fread(buf,1,256,fi);
	    }
	    if ((n > 0) && (sz > 0)){
		fwrite(buf,1,sz,fo);
	    }
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t portion( int argc, char *argv[] )
{
    struct PORTIONBASE ptb;
    int32_t ierr = portionparse(&ptb,argc,argv);
    if (ierr == 0){
	ptb.src = (argc > 3) ? argv[3] : "stdin";
	ptb.dst = (argc > 4) ? argv[4] : "stdout";
	for (int32_t i=5; i<argc; i++){
	    fprintf(stderr,"argument '%s' ignored.\n",argv[i]);
	}
	aportion(&ptb,ptb.src,ptb.dst);
    }
    return ierr;
}

/**********************************************************************
*F: gather
* Coded at: 2021.11.17 by T. Abi
**********************************************************************/
int32_t gather( int argc, char *argv[] )
{
    int32_t ierr = 0;
    if (argc == 5){
	FILE *fp[3];
	fp[0] = fopen(argv[2],"rb");
	if (fp[0] != NULL){
	    fp[1] = fopen(argv[3],"rb");
	    if (fp[1] != NULL){
		fp[2] = fopen(argv[4],"wb");
		if (fp[2] != NULL){
		    char buf[65536];
		    size_t sz = 65536;
		    size_t n = fread(buf,1,sz,fp[0]);
		    while (n > 0){
			fwrite(buf,1,n,fp[2]);
			n = fread(buf,1,sz,fp[0]);
		    }
		    n = fread(buf,1,sz,fp[1]);
		    while (n > 0){
			fwrite(buf,1,n,fp[2]);
			n = fread(buf,1,sz,fp[1]);
		    }
		    fclose(fp[2]);
		} else {
		    fprintf(stderr,"Failed to create %s\n",argv[4]);
		}
		fclose(fp[1]);
	    } else {
		fprintf(stderr,"Failed to open %s\n",argv[3]);
	    }
	    fclose(fp[0]);
	} else {
	    fprintf(stderr,"Failed to open %s\n",argv[2]);
	}
    } else {
	fprintf(stderr,"usage: ctb gather file1 file2 newfile\n");
    }
    return ierr;
}

/**********************************************************************
*F: cutl
*	ctb cutl [-nN] source [result]
*	N = 72 default
* Coded at: 2021.11.11 by T. Abi from tc0.c
**********************************************************************/
struct CUTLBASE {
FILE *fpi;
char *src;
FILE *fpo;
char *dst;
int32_t mx;
};

int32_t cutl_ini( struct CUTLBASE *cb, int argc, char *argv[] )
{
    cb->fpi = NULL;
    cb->src = NULL;
    cb->fpo = NULL;
    cb->dst = NULL;
    cb->mx = 72;
    for (int32_t i=2; i<argc; i++){
	if (argv[i][0] == '-'){
	    if ((argv[i][1] == 'n') && (isdigit(argv[i][2]))){
		int32_t n = atoi(&argv[i][2]);
		if ((n >= 0) && (n < 255)){
		    cb->mx = n;
		}
	    }
	} else {
	    if (cb->src == NULL){
		cb->src = argv[i];
	    } else if (cb->dst == NULL){
		cb->dst = argv[i];
	    }
	}
    }
    int32_t ierr = 1;
    if ((cb->src == NULL) || (strcmp(cb->src,"stdin") == 0)){
	cb->fpi = stdin;
    } else {
	cb->fpi = fopen(cb->src,"rb");
    }
    if (cb->fpi != NULL){
	if ((cb->dst == NULL) || (strcmp(cb->dst,"stdout") == 0)){
	    cb->fpo = stdout;
	} else {
	    cb->fpo = fopen(cb->dst,"wb");
	}
	if (cb->fpo != NULL){
	    ierr = 0;
	}
    }
    return ierr;
}

int32_t cutl_fin( struct CUTLBASE *cb )
{
    if ((cb->fpo != NULL) && (cb->fpo != stdout)){
	fclose(cb->fpo);
    }
    if ((cb->fpi != NULL) && (cb->fpi != stdin)){
	fclose(cb->fpi);
    }
    return 0;
}

int32_t cutl_body( struct CUTLBASE *cb )
{
    char buf[4096],txt[256];
    int32_t ierr = 0;
    int32_t s = 0;
    int32_t m = 0;
    int32_t n = fread(buf,1,4096,cb->fpi);
    while (n > 0){
	for (int32_t i=0; i<n; i++){
	    if (s == 0){
		if (buf[i] == 10){
		    txt[m] = 0;
		    fprintf(cb->fpo,"%s\n",txt);
		    m = 0;
		} else {
		    txt[m++] = buf[i];
		    if (m >= cb->mx){
			if ((txt[m-1] & 0x80) == 0x80){
			    while ((m > 0) && ((txt[m-1] & 0xc0) == 0x80)){
				m--;
			    }
			    if ((txt[m-1] & 0xc0) == 0xc0){
				m--;
			    } else {
				fprintf(stderr,"### ERROR ###\n");
				ierr = 1;
			    }
			}
			txt[m] = 0;
			fprintf(cb->fpo,"%s\n",txt);
			m = 0;
			s = 1;
		    }
		}
	    } else {
		if (buf[i] == 10){
		    s = 0;
		}
	    }
	}
	n = fread(buf,1,4096,cb->fpi);
    }
    return ierr;
}

int32_t cutl( int argc, char *argv[] )
{
    struct CUTLBASE clb;
    int32_t ierr = cutl_ini(&clb,argc,argv);
    if (ierr == 0){
	ierr = cutl_body(&clb);
	cutl_fin(&clb);
    }
    return ierr;
}

/***********************************************************************
*F: rot13
***********************************************************************/
int32_t arot13( FILE *fp )
{
    static char rot13a[] =
	   " !\"#$%&'()*+,-./01234:ABCDEFGHIJKLMabcdefghijklm";
    static char rot13b[] =
	"\177;<=>?@[\\]^_`{|}56789~NOPQRSTUVWXYZnopqrstuvwxyz";
    char buf[256],*p;
    int32_t n = fread(buf,1,256,fp);
    while (n > 0){
	for (int32_t i=0; i<n; i++){
	    int8_t c = buf[i];
	    if ((c >= ' ') && (c <= 0x7f)){
		if ((p=strchr(rot13a,c)) != NULL){
		    buf[i] = rot13b[p-rot13a];
		} else {
		    buf[i] = rot13a[strchr(rot13b,c)-rot13b];
		}
	    }
	}
	fwrite(buf,1,n,stdout);
	n = fread(buf,1,256,fp);
    }
    return 0;
}

int32_t rot13( int argc, char *argv[] )
{
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    FILE *fp = fopen(argv[i],"rb");
	    if (fp != NULL){
		arot13(fp);
		fclose(fp);
	    } else {
		fprintf(stderr,"Failed to open %s\n",argv[i]);
	    }
	}
    } else {
	arot13(stdin);
    }
    return 0;
}

/***********************************************************************
*F: ebgcl
***********************************************************************/
static char sb[] = "2345679ACEFGHJKLMNPRSTUVWXYZabcdefghkmnopqrstuvwxyz";
int32_t sbman( int32_t cd, char *s, int32_t n )
{
    char c,*p;
    int32_t m = strlen(sb);
    switch(cd){
    case 0:	/* string */
	for (int32_t i=0; i<strlen(s); i++){
	    putchar(((p=strchr(sb,s[i])) != NULL) ? sb[((p-sb)+n)%m] : '?');
	}
	break;
    case 1:	/* stdin */
	while ((c=fgetc(stdin)) != EOF){
	    if (c != '\n'){
		putchar(((p=strchr(sb,c)) != NULL) ? sb[((p-sb)+n)%m] : '?');
	    }
	}
	break;
    }
    return 0;
}

int32_t ebgcl( int argc, char *argv[] )
{
    int32_t m = 17;
    if (argc > 2){
	sbman(0,argv[2],m);
    } else {
	sbman(1,NULL,m);
    }
    return 0;
}

/**********************************************************************
*F: genpw
*	ctb genpw [length]
* Coded at: 2022.8.5 by T. Abi
**********************************************************************/
void genpw00( char *d, int32_t n )
{
    int32_t fd = open("/dev/urandom",O_RDONLY);
    if (fd >= 0){
	uint8_t rbuf[512];
	int32_t rc = read(fd,rbuf,sizeof rbuf);
	close(fd);
	if (rc > 0){
	    /* order */
	    memcpy(d,rbuf,n);
	    for (int32_t i=0; i<n; i++){
		rbuf[i] = i;
	    }
	    int32_t j,k;
	    for (int32_t i=0; i<n; i++){
		j = (d[i] & 0xff) % n;
		if (j != i){
		    k = rbuf[i];
		    rbuf[i] = rbuf[j];
		    rbuf[j] = k;
		}
	    }
	    /* assign */
	    int32_t m = strlen(sb);
	    j = n;
	    for (int32_t i=0; i<n; i++){
		k = rbuf[j] & 0xff;
		if (rbuf[i] == 0){	/* number */
		    d[i] = sb[k % 7];
		} else if (rbuf[i] == 1){	/* upper case */
		    d[i] = sb[(k % 21)+7];
		} else if (rbuf[i] == 2){	/* lower case */
		    d[i] = sb[(k % 23)+28];
		} else {	/* any */
		    d[i] = sb[k % m];
		}
		j++;
	    }
	    d[n] = '\0';
	}
    } else {
	fprintf(stderr,"Failed to open urandom.\n");
    }
}

int32_t genpw( int argc, char *argv[] )
{
    int32_t n = 6;
    if (argc > 2){
	int32_t m = atoi(argv[2]);
	if ((m >= 3) && (m < 128)){
	    n = m;
	}
    }
    char buf[256];
    strcpy(buf,"genpw: failed.\n");
    genpw00(buf,n);
    printf("%s",buf);
    return n;
}

/***********************************************************************
*F: en-xor
***********************************************************************/
int32_t aenxor( char *src, char *dst, int32_t ky )
{
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    char buf[256];
	    int32_t n = fread(buf,1,256,fi);
	    while (n > 0){
		for (int32_t i=0; i<n; i++){
		    buf[i] ^= ky;
		}
		fwrite(buf,1,n,fo);
		n = fread(buf,1,256,fi);
	    }
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t enxor( int argc, char *argv[] )
{
    int32_t ky = 0x55;	/* key character */
    int32_t ii = 0;	/* input file index */
    int32_t oi = 0;	/* output file index */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] == '-'){
		ky = atoi(&argv[i][1]);
	    } else {
		if (ii == 0){
		    ii = i;
		} else if (oi == 0){
		    oi = i;
		} else {
		    ii = oi = i;
		}
	    }
	}
    }
    if (oi == 0){
	if (ii > 0){
	    aenxor(argv[ii],"stdout",ky);
	} else {
	    aenxor("stdin","stdout",ky);
	}
    } else {
	if (ii < oi){
	    aenxor(argv[ii],argv[oi],ky);
	} else {
	    for (int32_t i=2; i<argc; i++){
		if (argv[i][0] != '-'){
		    aenxor(argv[i],"stdout",ky);
		}
	    }
	}
    }
    return 0;
}

/***********************************************************************
*F: quoted-printable converter
*
*	=EOL -- truncated
*	=3D -- =
*
* Coded at: 1999.6.16 T. Abi
* Debugged: 2000.3.8 to check fgets result
* Modified: 2003.1.16 added ESC, debugged =3D n-=q-p+2 --> +3
* Modified: 2003.2.26 added any character
* Lastupdt: 2005.9.13 for CVS
* Modified: 2006.3.16 full conversion
* Debugged: 2006.3.17 to join fread() batch
* Modified: 2008.10.30 for ctb
***********************************************************************/
struct QPBASE {
FILE *fi;
FILE *fo;
char ibuf[256];
char obuf[256];
int32_t nout;
int32_t mxbf;
};

int32_t hex2bin( int8_t c )
{
    int8_t i = toupper(c);
    if ((i >= '0') && (i <= '9')){
	i -= '0';
    } else if ((i >= 'A') && (i <= 'F')){
	i -= '7';
    }
    return i;
}

int32_t qpfman( struct QPBASE *qb, int32_t cd, int8_t c )
{
    int32_t ierr = 0;
    switch(cd){
    case 1:		/* put a char */
	if (qb->nout >= qb->mxbf){
	    fwrite(qb->obuf,1,qb->nout,qb->fo);
	    qb->nout = 0;
	}
	qb->obuf[qb->nout++] = c;
	break;
    default:
	break;
    }
    return ierr;
}

int32_t qpconvert(
FILE *fi,
FILE *fo )
{
    int8_t c;
    struct QPBASE qpb;
    qpb.fi = fi;
    qpb.fo = fo;
    qpb.mxbf = 256;
    qpb.nout = 0;
    int32_t i = 0;
    int32_t xbf = 0;
    int32_t ph = 0;		/* 0:normal 1:= 2:=x */
    int32_t n = fread(qpb.ibuf,1,256,qpb.fi);
    while (n > 0){
	i = 0;
	do {
	    c = qpb.ibuf[i++];
	    switch(ph){
	    case 0:		/* normal */
		if (c == '='){
		    ph = 1;
		} else {
		    qpfman(&qpb,1,c);
		}
		break;
	    case 1:		/* = */
		if (c != '\r'){
		    if (c == '\n'){
			ph = 0;
		    } else {
			if (isxdigit(c)){
			    xbf = c;
			    ph = 2;
			} else {
			    qpfman(&qpb,1,'=');
			    qpfman(&qpb,1,c);
			    ph = 0;
			}
		    }
		}
		break;
	    case 2:		/* =x */
		if (isxdigit(c)){
		    qpfman(&qpb,1,hex2bin(xbf)*16+hex2bin(c));
		} else {
		    qpfman(&qpb,1,'=');
		    qpfman(&qpb,1,xbf);
		    qpfman(&qpb,1,c);
		}
		ph = 0;
		break;
	    default:
		break;
	    }
	} while (i < n);
	n = fread(qpb.ibuf,1,256,qpb.fi);
    }
    switch(ph){
    case 1:
	qpfman(&qpb,1,'=');
	break;
    case 2:
	qpfman(&qpb,1,'=');
	qpfman(&qpb,1,xbf);
	break;
    default:
	break;
    }
    if (qpb.nout > 0){
	fwrite(qpb.obuf,1,qpb.nout,qpb.fo);
    }
    return 0;
}

int32_t aqp( char *src, char *dst )
{
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    qpconvert(fi,fo);
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t qp( int argc, char *argv[] )
{
    int32_t ii = 0;	/* input file index */
    int32_t oi = 0;	/* output file index */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] != '-'){
		if (ii == 0){
		    ii = i;
		} else if (oi == 0){
		    oi = i;
		} else {
		    ii = oi = i;
		}
	    } else {
		fprintf(stderr,"option %s ignored\n",argv[i]);
	    }
	}
    }
    if (ii > 0){
	if (ii <  oi){
	    aqp(argv[ii],argv[oi]);
	} else {
	    for (int32_t i=2; i<argc; i++){
		if (argv[i][0] != '-'){
		    fprintf(stderr,"%s -- ",argv[i]);
		    aqp(argv[i],"stdout");
		}
	    }
	}
    } else {
	aqp("stdin","stdout");
    }
    return 0;
}

/**********************************************************************
*F: find the maximum line length
* Coded at: 2000.11.15 by T. Abi
* Debugged: 2003.10.17 count values were cerrected
* Modified: 2004.9.21 added -v for histogram
* Lastupdt: 2005.9.13 for CVS
* Modified: 2007.5.8 added minimum line
* Modified: 2007.9.20 added -h -? for help
* Modified: 2007.12.27 changed name to job0 from body
* Modified: 2008.10.30 for ctb
**********************************************************************/
int32_t lhist( FILE *fp, long *mxl )
{
    int32_t c,i,j,k,sta;
    long mxh = (mxl[0] < mxl[1]) ? mxl[1] : mxl[0];
    if (mxl[2] > mxh){
	mxh = mxl[2];
    }
    mxh = mxh/10 +1;
    long *hb[3],cc[3];
    hb[0] = malloc(mxh*sizeof(long)*3);
    if (hb[0] != NULL){
	hb[1] = &hb[0][mxh];
	hb[2] = &hb[1][mxh];
	for (i=0; i<mxh; i++){
	    hb[0][i] = hb[1][i] = hb[2][i] = 0L;
	}
	cc[0] = cc[1] = cc[2] = 0L;
	rewind(fp);
	sta = 0;	/* 0:InLine 1:CR 2:LF 3:CR+LF */
	j = 0;
	while ((c=fgetc(fp)) != EOF){
	    cc[0]++;  cc[1]++;  cc[2]++;
	    switch(c){
	    case 13:
		if ((sta >= 1) && (sta <= 3)){
		    i = sta-1;
		    k = (cc[i]-2)/10;
		    hb[i][k]++;
		    cc[i] = (i==1) ? 0 : 1;
		}
		sta = 1;
		j = 0;
		break;
	    case 10:
		switch(sta){
		case 0:
		    sta = 2;
		    break;
		case 1:
		    sta = 3;
		    break;
		case 2:
		case 3:
		    i = sta-1;
		    k = (cc[i]-2)/10;
		    hb[i][k]++;
		    cc[i] = (i==1) ? 1 : 0;
		    sta = 2;
		    break;
		default:
		    break;
		}
		j = 0;
		break;
	    default:
		j++;
		if ((sta >= 1) && (sta <= 3)){
		    i = sta-1;
		    k = (cc[i]-2)/10;
		    hb[i][k]++;
		    cc[i] = 1;
		}
		sta = 0;
		break;
	    }
	}
	if ((sta >= 1) && (sta <= 3)){
	    i = sta-1;
	    k = (cc[i]-2)/10;
	    hb[i][k]++;
	    cc[i] = 0L;
	} else {
	    printf("The last line (%d chars.) is not terminated.\n",j);
	}
	printf("fm -- to\tCR\tLF\tCR+LF\n");
	printf("   -- 10\t%ld\t%ld\t%ld\n",hb[0][0],hb[1][0],hb[2][0]);
	for (i=1; i<mxh; i++){
	    printf("%-3d--%3d\t%ld\t%ld\t%ld\n",i*10,(i+1)*10,
			hb[0][i],hb[1][i],hb[2][i]);
	}
	free(hb[0]);
    }
    return 0;
}

int32_t amaxline( FILE *fp, int32_t hist )
{
    long cc[3];	/* CR LF CR+LF */
    long lc[3];
    long mxl[3];
    long mnl[3];
    int32_t c,i;
    cc[0] = cc[1] = cc[2] = 0L;
    lc[0] = lc[1] = lc[2] = 0L;
    mxl[0] = mxl[1] = mxl[2] = 0L;
    mnl[0] = mnl[1] = mnl[2] = 2147483647L;
    int32_t sta = 0;	/* 0:InLine 1:CR 2:LF 3:CR+LF */
    int32_t j = 0;
    while ((c=fgetc(fp)) != EOF){
	cc[0]++;  cc[1]++;  cc[2]++;
	switch(c){
	case 13:
	    if ((sta >= 1) && (sta <= 3)){
		i = sta-1;
		if (cc[i]-1 > mxl[i]){
		    mxl[i] = cc[i]-1;
		}
		if (cc[i]-1 < mnl[i]){
		    mnl[i] = cc[i]-1;
		}
		lc[i]++;
		cc[i] = 1;
	    }
	    sta = 1;
	    j = 0;
	    break;
	case 10:
	    switch(sta){
	    case 0:
		sta = 2;
		break;
	    case 1:
		sta = 3;
		break;
	    case 2:
	    case 3:
		i = sta-1;
		if (cc[i]-1 > mxl[i]){
		    mxl[i] = cc[i]-1;
		}
		if (cc[i]-1 < mnl[i]){
		    mnl[i] = cc[i]-1;
		}
		lc[i]++;
		cc[i] = 1;
		sta = 2;
		break;
	    default:
		break;
	    }
	    j = 0;
	    break;
	default:
	    j++;
	    if ((sta >= 1) && (sta <= 3)){
		i = sta-1;
		if (cc[i]-1 > mxl[i]){
		    mxl[i] = cc[i]-1;
		}
		if (cc[i]-1 < mnl[i]){
		    mnl[i] = cc[i]-1;
		}
		lc[i]++;
		cc[i] = 1;
	    }
	    sta = 0;
	    break;
	}
    }
    if ((sta >= 1) && (sta <= 3)){
	i = sta-1;
	if (cc[i] > mxl[i]){
	    mxl[i] = cc[i];
	}
	if (cc[i] < mnl[i]){
	    mnl[i] = cc[i];
	}
	lc[i]++;
	cc[i] = 0L;
    } else {
	printf("The last line (%d chars.) is not terminated.\n",j);
    }
    if (hist != 0){
	lhist(fp,mxl);
    }
    if (lc[0] > 0L)
	printf("CR: %ld lines, %ld -- %ld char.\n",lc[0],mnl[0],mxl[0]);
    if (lc[1] > 0L)
	printf("LF: %ld lines, %ld -- %ld char.\n",lc[1],mnl[1],mxl[1]);
    if (lc[2] > 0L)
	printf("CR+LF: %ld lines, %ld -- %ld char.\n",lc[2],mnl[2],mxl[2]);
    return 0;
}

int32_t maxline( int argc, char *argv[] )
{
    int32_t vf = 0;	/* histogram flag */
    int32_t nf = 0;	/* files count */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] == '-'){
		if (argv[i][1] == 'v'){
		    vf = 1;
		} else {
		    fprintf(stderr,"option %s ignored\n",argv[i]);
		}
	    } else {
		nf++;
	    }
	}
    }
    if (nf > 0){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] != '-'){
		FILE *fp = fopen(argv[i],"rb");
		if (fp != NULL){
		    if (nf > 1){
			printf("%s: ",argv[i]);
		    }
		    amaxline(fp,vf);
		    fclose(fp);
		} else {
		    fprintf(stderr,"Failed to open %s\n",argv[i]);
		}
	    }
	}
    } else {
	amaxline(stdin,vf);
    }
    return 0;
}

/**********************************************************************
*F: find the maximum line length
* Coded at: 2000.11.15 by T. Abi
***********************************************************************/
int32_t anullitems( FILE *fp, int32_t hist )
{
/*    int32_t sta = 0;	// 0:InLine 1:CR 2:LF 3:CR+LF */
    int32_t sch = ',';	/* separator char. */
    int32_t nuc = 0;
    int32_t itc = 0;
    int32_t nucmx = 0;
    int32_t itcmx = 0;
    int32_t nlines = 0;
    int32_t c0 = sch;
    int32_t c1;
    while ((c1=fgetc(fp)) != EOF){
	if ((c1 == sch) || (c1 == 10)){
	    if (c0 == sch){
		nuc++;
	    }
	    itc++;
	}
	if (c1 == 10){
	    if (itc > itcmx){
		itcmx = itc;
	    }
	    if (nuc > nucmx){
		nucmx = nuc;
	    }
	    nlines++;
	    c0 = sch;
	    nuc = 0;
	    itc = 0;
	} else {
	    c0 = c1;
	}
    }
    printf("LF: %d %d %d\n",nucmx,itcmx,nlines);
    return 0;
}

int32_t nullitems( int argc, char *argv[] )
{
    int32_t vf = 0;	/* histogram flag */
    int32_t nf = 0;	/* files count */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] == '-'){
		if (argv[i][1] == 'v'){
		    vf = 1;
		} else {
		    fprintf(stderr,"option %s ignored\n",argv[i]);
		}
	    } else {
		nf++;
	    }
	}
    }
    if (nf > 0){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] != '-'){
		FILE *fp = fopen(argv[i],"rb");
		if (fp != NULL){
		    if (nf > 1){
			printf("%s: ",argv[i]);
		    }
		    anullitems(fp,vf);
		    fclose(fp);
		} else {
		    fprintf(stderr,"Failed to open %s\n",argv[i]);
		}
	    }
	}
    } else {
	anullitems(stdin,vf);
    }
    return 0;
}

/***********************************************************************
*F: CR LF converter
*	a.out cl -- means CR to LF, c=CR l=LF 2=CR+LF
*
* Coded at: '98/07/16 by T. Abi
* Modified: 2005.9.13 for CVS
***********************************************************************/
#define CR 13
#define LF 10

int32_t putacl2( FILE *fp, int32_t ft, int32_t cl2 )
{
    if (ft > 0){
	if ((ft & 12) != cl2){
	    if ((cl2 & 4) != 0){
		fputc(CR,fp);
	    }
	    if ((cl2 & 8) != 0){
		fputc(LF,fp);
	    }
	} else {
	    if ((ft & 1) != 0){
		fputc(CR,fp);
	    }
	    if ((ft & 2) != 0){
		fputc(LF,fp);
	    }
	}
    }
    return 0;
}

int32_t acrlf( char *src, char *dst, int32_t ft )
{
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    int crlf[3];	/* 0:cr 1:lf 2:crlf */
	    int c,ph;
	    crlf[0] = crlf[1] = crlf[2] = 0;
	    ph = 0;	/* 0:non 1:cr */
	    c = fgetc(fi);
	    while (feof(fi) == 0){
		if (c == CR){
		    if (ph == 1){
			crlf[0]++;
			putacl2(fo,ft,4);
		    } else {
			ph = 1;
		    }
		} else if (c == LF){
		    if (ph == 1){
			crlf[2]++;
			putacl2(fo,ft,12);
			ph = 0;
		    } else {
			crlf[1]++;
			putacl2(fo,ft,8);
		    }
		} else {
		    if (ph == 1){
			crlf[0]++;
			putacl2(fo,ft,4);
			ph = 0;
		    }
		    if (ft > 0){
			fputc(c,fo);
		    }
		}
		c = fgetc(fi);
	    }
	    if (ph == 1){
		crlf[0]++;
		putacl2(fo,ft,4);
	    }
	    if (ft == 0){
		fprintf(fo,"cr:%d lf:%d crlf:%d\n",crlf[0],crlf[1],crlf[2]);
	    }
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t chkcl2( int8_t c )
{
    int32_t k;
    switch(toupper(c)){
    case 'C':
	k = 1;
	break;
    case 'L':
	k = 2;
	break;
    case '2':
	k = 3;
	break;
    default:
	k = 0;
	break;
    }
    return k;
}

int32_t crlf( int argc, char *argv[] )
{
    int8_t c;
    int32_t ft = 0;	/* from to */
    int32_t ii = 0;	/* input file index */
    int32_t oi = 0;	/* output file index */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] == '-'){
		if ((ft=(chkcl2(argv[i][1])<<2)) > 0){
		    if ((c=chkcl2(argv[i][2])) > 0){
			ft += c;
		    } else {
			ft = 0;
		    }
		}
	    } else {
		if (ii == 0){
		    ii = i;
		} else if (oi == 0){
		    oi = i;
		} else {
		    ii = oi = i;
		}
	    }
	}
    }
    if (oi == 0){
	if (ii > 0){
	    acrlf(argv[ii],"stdout",ft);
	} else {
	    acrlf("stdin","stdout",ft);
	}
    } else {
	if ((ft > 0) && (ii < oi)){
	    acrlf(argv[ii],argv[oi],ft);
	} else {
	    for (int32_t i=2; i<argc; i++){
		if (argv[i][0] != '-'){
		    if (ft == 0){
			fprintf(stderr,"%s -- ",argv[i]);
		    }
		    acrlf(argv[i],"stdout",ft);
		}
	    }
	}
    }
    return 0;
}

/***********************************************************************
*F: Base64 converter
*
*	****** MIME structure ************
*	MIME string :=	MIME in			-- =?
*			character set string	-- like iso-2022-j
*			MIME body		-- ?B?
*			MIME string body	-- like GyRC....
*			[padding]		-- =
*			MIME out		-- ?=
*
*	***** Base64 character table *****
*	   0 1 2 3 4 5 6 7 8 9 a b c d e f
*	00 A B C D E F G H I J K L M N O P
*	10 Q R S T U V W X Y Z a b c d e f
*	20 g h i j k l m n o p q r s t u v
*	30 w x y z 0 1 2 3 4 5 6 7 8 9 + /
*
*	****** special characters ********
*	MIME in		=?
*	MIME body	?B?
*	padding		=
*	MIME out	?=
*	KANJI in	ESC $ B
*	KANJI out	ESC ( B
*
* Coded at: '95/03/29 by T. Abi
* Lastupdt: '95/03/31
* Debugged: '95/04/03 failed to read the last '?='
* Modified: '97/03/20 for base64 converter from mime.c
* Debugged: '97/10/07 "wb" only, used fwrite
* Modified: 2000.6.7 added encode function
* Lastupdt: 2005.9.13 for CVS
* Modified: 2016.3.3 added b62 B62 9=9A +=9B /=9C
***********************************************************************/
static char base64table[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int32_t b64conv( FILE *fi, FILE *fo, int32_t cd, int32_t b62 )	/* 0:decode 1:encode */
{
    char *p;
    int32_t ph = 0;
    int32_t b = 0;
    int32_t n = 76;	/* characters in a line */
    int32_t m = 0;
    int8_t c = fgetc(fi);
    int8_t x;
    if (cd == 0){
	while (feof(fi) == 0){
	    if ((c == '9') && (b62 == 62)){
		c = ((char *)"9+/")[fgetc(fi)-'A'];
	    }
	    if ((p=strchr(base64table,c)) != NULL){
		b = (b << 6)+(p-base64table);
		switch(ph){
		case 1:
		    fputc(b >> 4,fo);
		    m++;
		    b &= 15;
		    break;
		case 2:
		    fputc(b >> 2,fo);
		    m++;
		    b &= 3;
		    break;
		case 3:
		    fputc(b,fo);
		    m++;
		    b = 0;
		    ph = -1;
		    break;
		}
		ph++;
	    }
	    c = fgetc(fi);
	}
    } else {
	while (feof(fi) == 0){
	    switch(ph){
	    case 0:
		x = c >> 2;
		if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
		} else {
		    fprintf(fo,"9%c",x+4);
		}
		m++;
		b = c & 3;
		ph++;
		break;
	    case 1:
		x = (b << 4) + (c >> 4);
		if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
		} else {
		    fprintf(fo,"9%c",x+4);
		}
		m++;
		b = c & 15;
		ph++;
		break;
	    case 2:
		x = (b << 2) + (c >> 6);
		if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
		} else {
		    fprintf(fo,"9%c",x+4);
		}
		m++;
		if (m >= n){
		    if (fo != stdout){
			fputc(13,fo);
		    }
		    fputc(10,fo);
		    m = 0;
		}
		x = c & 0x3f;
		if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
		} else {
		    fprintf(fo,"9%c",x+4);
		}
		m++;
		ph = 0;
		break;
	    }
	    if (m >= n){
		if (fo != stdout){
		    fputc(13,fo);
		}
		fputc(10,fo);
		m = 0;
	    }
	    c = fgetc(fi);
	}
	switch(ph){
	case 1:
	    x = b << 4;
	    if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
	    } else {
		    fprintf(fo,"9%c",x+4);
	    }
	    m++;
	    if (m >= n){
		if (fo != stdout){
		    fputc(13,fo);
		}
		fputc(10,fo);
		m = 0;
	    }
	    if (b62 == 64){
		fputc('=',fo);
	    }
	    m++;
	    if (m >= n){
		if (fo != stdout){
		    fputc(13,fo);
		}
		fputc(10,fo);
		m = 0;
	    }
	    if (b62 == 64){
		fputc('=',fo);
	    }
	    m++;
	    break;
	case 2:
	    x = b << 2;
	    if ((b62 == 64) || (x < 61)){
		    fputc(base64table[x],fo);
	    } else {
		    fprintf(fo,"9%c",x+4);
	    }
	    m++;
	    if (m >= n){
		if (fo != stdout){
		    fputc(13,fo);
		}
		fputc(10,fo);
		m = 0;
	    }
	    if (b62 == 64){
		fputc('=',fo);
	    }
	    m++;
	    break;
	}
	if (m > 0){
	    if (fo != stdout){
		fputc(13,fo);
	    }
	    fputc(10,fo);
	    m = 0;
	}
    }
    return 0;
}

int32_t ab64( char *src, char *dst, int32_t cd, int32_t b62 )	/* 0:decode 1:encode */
{
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    b64conv(fi,fo,cd,b62);
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t base64( int argc, char *argv[], int32_t cd, int32_t b62 )
{
    int32_t ii = 0;	/* input file index */
    int32_t oi = 0;	/* output file index */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] != '-'){
		if (ii == 0){
		    ii = i;
		} else if (oi == 0){
		    oi = i;
		} else {
		    ii = oi = i;
		}
	    } else {
		fprintf(stderr,"option %s ignored\n",argv[i]);
	    }
	}
    }
    if (ii > 0){
	if (ii <  oi){
	    ab64(argv[ii],argv[oi],cd,b62);
	} else {
	    for (int32_t i=2; i<argc; i++){
		if (argv[i][0] != '-'){
		    fprintf(stderr,"%s -- ",argv[i]);
		    ab64(argv[i],"stdout",cd,b62);
		}
	    }
	}
    } else {
	ab64("stdin","stdout",cd,b62);
    }
    return 0;
}

/**********************************************************************
*F: chist
* Coded at: 2000.1.17 by T. Abi
* Modified: 2003.3.18 added -z option for zero character
* Debugged: 2003.3.21 K=4096 --> K=1024, and removed the 65535 limit
* Modified: 2003.9.1 for linux
* Modified: 2005.9.13 for CVS
* Modified: 2008.10.21 for ctb
**********************************************************************/
struct CHISTBASE {
char *flnm;
FILE *fp;
int32_t zch;
};

int32_t chistparse( struct CHISTBASE *cb, int argc, char *argv[] )
{
    for (int32_t i=2; i<argc; i++){
	if (argv[i][0] == '-'){
	    if (toupper(argv[i][1]) == 'Z'){
		cb->zch = (argv[i][2] < ' ') ? ' ' : argv[i][2];
	    }
	} else {
	    if (cb->flnm == NULL){
		cb->flnm = argv[i];
	    } else {
		fprintf(stderr,"%s ignored\n",argv[i]);
	    }
	}
    }
    return 0;
}

int32_t chistbody( struct CHISTBASE *cb )
{
    unsigned char buf[256];
    unsigned long chist[256];
    unsigned long k;
    for (int32_t i=0; i<256; i++){
	chist[i] = 0;
    }
    int32_t bfsz = 256;
    int32_t n = fread(buf,1,bfsz,cb->fp);
    while (n > 0){
	for (int32_t i=0; i<n; i++){
	    int32_t j = buf[i];
	    chist[j]++;
	}
	n = fread(buf,1,bfsz,cb->fp);
    }
    printf("x   x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf\n");
    for (int32_t i=0; i<16; i++){
	printf("%x ",i);
	for (int32_t j=0; j<16; j++){
	    k = chist[j+i*16];
	    if (k == 0){
		printf("   %c",cb->zch);
	    } else if (k < 65536){
		printf("%4lx",k);
	    } else if (k < 4194304){
		printf("%3lxK",k/1024);
	    } else {
		printf("%3lxM",k/1048576);
	    }
	}
	printf("\n");
    }
    return 0;
}

int32_t chist( int argc, char *argv[] )
{
    struct CHISTBASE chb;
    chb.flnm = NULL;
    chb.fp = stdin;
    chb.zch = '0';
    chistparse(&chb,argc,argv);
    chb.fp = (chb.flnm == NULL) ? stdin : fopen(chb.flnm,"rb");
    if (chb.fp != NULL){
	chistbody(&chb);
	if (chb.fp != stdin){
	    fclose(chb.fp);
	}
    } else {
	fprintf(stderr,"Failed to open %s\n",chb.flnm);
    }
    return 0;
}

/***********************************************************************
*F: h2b
* Coded at: 1998.6.25 by T. Abi
* Lastupdt: 1998.6.26 to remove the last character
* Lastupdt: 2005.9.13 for CVS
* Modified: 2008.10.21 for ctb
* Modified: 2009.7.16 to take an output file name
***********************************************************************/
int32_t chkhexdec( int8_t c )
{
    int32_t f = 0;
    if ((c >= '0') && (c <= '9'))  f = 1;
    if (c >= 'a')  c -= ' ';
    if ((c >= 'A') && (c <= 'F'))  f = 2;
    return f;
}

int32_t h2bconv( FILE *fi, FILE *fo )
{
    int32_t b,c0,c1,d0,d1,f;
    while (feof(fi) == 0){
	do {
	    c0 = fgetc(fi);
	    f = chkhexdec(c0);
	} while ((f == 0) && (feof(fi) == 0));
	d0 = c0;
	if (f == 1)  d0 -= '0';
	if (f == 2)  d0 -= ((c0 > 'Z') ? 0x57 : 0x37);
	do {
	    c1 = fgetc(fi);
	    f = chkhexdec(c1);
	} while ((f == 0) && (feof(fi) == 0));
	d1 = c1;
	if (f == 1)  d1 -= '0';
	if (f == 2)  d1 -= ((c1 > 'Z') ? 0x57 : 0x37);
	if (feof(fi) == 0){
	    b = d0*16+d1;
	    fwrite(&b,1,1,fo);
	}
    }
    return 0;
}

int32_t ah2b( char *src, char *dst )
{
    FILE *fi = (strcmp(src,"stdin") == 0) ? stdin : fopen(src,"rb");
    if (fi != NULL){
	FILE *fo = (strcmp(dst,"stdout") == 0) ? stdout : fopen(dst,"wb");
	if (fo != NULL){
	    h2bconv(fi,fo);
	    if (fo != stdout){
		fclose(fo);
	    }
	} else {
	    fprintf(stderr,"Failed to create %s.\n",dst);
	}
	if (fi != stdin){
	    fclose(fi);
	}
    } else {
	fprintf(stderr,"Failed to open %s.\n",src);
    }
    return 0;
}

int32_t h2b( int argc, char *argv[] )
{
    int32_t ii = 0;	/* input file index */
    int32_t oi = 0;	/* output file index */
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    if (argv[i][0] != '-'){
		if (ii == 0){
		    ii = i;
		} else if (oi == 0){
		    oi = i;
		} else {
		    ii = oi = i;
		}
	    } else {
		fprintf(stderr,"option %s ignored\n",argv[i]);
	    }
	}
    }
    if (ii > 0){
	if (ii <  oi){
	    ah2b(argv[ii],argv[oi]);
	} else {
	    for (int32_t i=2; i<argc; i++){
		if (argv[i][0] != '-'){
		    fprintf(stderr,"%s -- ",argv[i]);
		    ah2b(argv[i],"stdout");
		}
	    }
	}
    } else {
	ah2b("stdin","stdout");
    }
    return 0;
}

/***********************************************************************
*F: b2h
* Coded at: 1998.6.25 by T. Abi
* Lastupdt: 1998.6.26 to remove the last character
* Modified: 2000.1.6 from h2b
* Debugged: 2000.1.21 fixed the last '\n'
* Modified: 2005.9.13 for CVS
* Modified: 2008.10.21 for ctb
***********************************************************************/
int32_t ab2h( FILE *fp )
{
    char ibuf[16],obuf[80];
    int32_t n = fread(ibuf,1,16,fp);
    while (n > 0){
	char *p = obuf;
	sprintf(p, "%02x",ibuf[0]&0xff);
	p += 2;
	for (int32_t i=1; i<n; i++){
	    sprintf(p," %02x",ibuf[i]&0xff);
	    p += 3;
	}
	*p = '\0';
	printf("%s\n",obuf);
	n = fread(ibuf,1,16,fp);
    }
    return 0;
}

int32_t b2h( int argc, char *argv[] )
{
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    FILE *fp = fopen(argv[i],"rb");
	    if (fp != NULL){
		ab2h(fp);
		fclose(fp);
	    } else {
		fprintf(stderr,"Failed to open %s\n",argv[i]);
	    }
	}
    } else {
	ab2h(stdin);
    }
    return 0;
}

int32_t u8enc( int argc, char *argv[], int32_t cd )
{
    int b[5];
    char *p = argv[2];
    if (cd == 0){
	b[0] = hex2bin(p[0])*16 + hex2bin(p[1]);
	b[1] = hex2bin(p[2])*16 + hex2bin(p[3]);
	b[2] = hex2bin(p[4])*16 + hex2bin(p[5]);
	b[3] = ((b[0] & 0xf) << 4) + ((b[1] & 0x3c) >> 2);
	b[4] = ((b[1] & 3) << 6) + (b[2] & 0x3f);
	printf("U+%02x%02x\n",b[3],b[4]);
    } else {
	b[3] = hex2bin(p[2])*16 + hex2bin(p[3]);
	b[4] = hex2bin(p[4])*16 + hex2bin(p[5]);
	b[0] = 0xe0 + ((b[3] & 0xf0) >> 4);
	b[1] = 0x80 + ((b[3] & 0xf) << 2) + ((b[4] & 0xc0) >> 6);
	b[2] = 0x80 + (b[4] & 0x3f);
	printf("%02x%02x%02x\n",b[0],b[1],b[2]);
    }
    return 0;
}

/***********************************************************************
*F: b2url
* Coded at: 2015.6.18 by T. Abi
***********************************************************************/
int32_t as2url( char *s )
{
    static char uRC[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    int32_t sz = strlen(s)*3+1;
    char *obuf = malloc(sz);
    int32_t m = 0;
    int8_t c;
    while ((c=(*(s++)&0xff)) != '\0'){
	if (strchr(uRC,c) != NULL){
	    obuf[m++] = c;
	} else {
	    sprintf(&obuf[m],"%%%02x",c);
	    m += 3;
	}
    }
    if (m > 0){
	obuf[m] = '\0';
	printf("%s",obuf);
    }
    free(obuf);
    return 0;
}

int32_t af2url( FILE *fp )
{
    static char uRC[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    char ibuf[16],*obuf;
    int8_t c;
    int32_t sz = 1024;
    obuf = malloc(sz);
    int32_t m = 0;
    int32_t n = fread(ibuf,1,16,fp);
    while (n > 0){
	for (int32_t i=0; (i<n)&&(m<1020); i++){
	    c = ibuf[i] & 0xff;
	    if (strchr(uRC,c) != NULL){
		obuf[m++] = c;
	    } else {
		sprintf(&obuf[m],"%%%02x",c);
		m += 3;
	    }
	}
	n = fread(ibuf,1,16,fp);
    }
    if (m > 0){
	obuf[m] = '\0';
	printf("%s",obuf);
    }
    free(obuf);
    return 0;
}

int32_t ff2url( char *flnm )
{
    FILE *fp = fopen(flnm,"rb");
    if (fp != NULL){
	af2url(fp);
	fclose(fp);
    } else {
	fprintf(stderr,"Failed to open %s\n",flnm);
    }
    return 0;
}

int32_t b2url( int argc, char *argv[] )
{
    if (argc > 2){
	int fflag = 0;	/* 0:string 1:file */
	for (int32_t i=2; i<argc; i++){
	    if (strncmp(argv[i],"-f",2) == 0){
		if (argv[i][2] > ' '){
		    ff2url(&argv[i][2]);
		    fflag = 0;
		} else {
		    fflag = 1;
		}
	    } else {
		if (fflag == 1){
		    ff2url(argv[i]);
		    fflag = 0;
		} else {
		    as2url(argv[i]);
		}
	    }
	}
    } else {
	af2url(stdin);
    }
    return 0;
}

/***********************************************************************
*F: url2b
* Coded at: 2015.6.18 by T. Abi
***********************************************************************/
int32_t asurl2b( char *s )
{
    int32_t sz = strlen(s);
    char *obuf = malloc(sz+1);
    int32_t m = 0;
    int8_t c;
    while ((c=*(s++)) > '\0'){
	if (c == '%'){
	    if ((isxdigit(s[0]) > 0) && (isxdigit(s[1]) > 0)){
		c = hex2bin(s[0])*16+hex2bin(s[1]);
		s += 2;
	    }
	}
	obuf[m++] = c;
    }
    if (m > 0){
	obuf[m] = '\0';
	printf("%s",obuf);
    }
    free(obuf);
    return 0;
}

int32_t afurl2b( FILE *fp )
{
    char ibuf[16];
    int32_t sz = 1024;
    char *obuf = malloc(sz);
    int32_t m = 0;
    int32_t n = fread(ibuf,1,16,fp);
    int32_t i = 0;
    int8_t c;
    while (n > 0){
	c = ibuf[i++];
	if (c == '%'){
	    if (i > n-2){
		ibuf[0] = ibuf[n-1];
		i = n-i;
		n = fread(&ibuf[i],1,16-i,fp) + i;
		i = 0;
	    }
	    if ((isxdigit(ibuf[0]) > 0) && (isxdigit(ibuf[1]) > 0)){
		c = hex2bin(ibuf[0])*16+hex2bin(ibuf[1]);
		i += 2;
	    }
	}
	obuf[m++] = c;
	if (i >= n){
	    n = fread(ibuf,1,16,fp);
	}
    }
    if (m > 0){
	obuf[m] = '\0';
	printf("%s",obuf);
    }
    free(obuf);
    return 0;
}

int32_t furl2b( char *flnm )
{
    FILE *fp = fopen(flnm,"rb");
    if (fp != NULL){
	afurl2b(fp);
	fclose(fp);
    } else {
	fprintf(stderr,"Failed to open %s\n",flnm);
    }
    return 0;
}

int32_t url2b( int argc, char *argv[] )
{
    if (argc > 2){
	int fflag = 0;	/* 0:string 1:file */
	for (int32_t i=2; i<argc; i++){
	    if (strncmp(argv[i],"-f",2) == 0){
		if (argv[i][2] > ' '){
		    furl2b(&argv[i][2]);
		    fflag = 0;
		} else {
		    fflag = 1;
		}
	    } else {
		if (fflag == 1){
		    furl2b(argv[i]);
		    fflag = 0;
		} else {
		    asurl2b(argv[i]);
		}
	    }
	}
    } else {
	afurl2b(stdin);
    }
    return 0;
}

/**********************************************************************
*F: hdmp
* Up-dated: '88/06/28 by T. Abi
* Lastupdt: '93/07/20 for 730
* Debugged: 2000.1.26 binary open
* Debugged: 2004.1.26 void main() --> main(), fp <= NULL --> fp == NULL
* Modified: 2005.9.2 changed file name from dump.c to hdmp.c
* Modified: 2008.10.20 for ctb
**********************************************************************/
int32_t ahdmp( FILE *fp )
{
    char ibuf[16],obuf[80],*p;
    int8_t c;
    printf(" addr   +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F    0123456789ABCDEF\n");
    int32_t ad = 0;
    int32_t n = fread(ibuf,1,16,fp);
    while (n > 0){
	sprintf(obuf,"%05x0   ",ad++);
	p = &obuf[8];
	for (int32_t i=0; i<16; i++){
	    if (i < n){
		sprintf(p,"%02x ",ibuf[i]&0xff);
	    } else {
		sprintf(p,"   ");
	    }
	    p += 3;
	}
	sprintf(p,"   ");
	p += 3;
	for (int32_t i=0; i<16; i++){
	    if (i < n){
		c = ibuf[i]&0xff;
		sprintf(p,"%c",((c >= 0x20) && (c < 0x7f)) ? c : '.');
	    } else {
		sprintf(p," ");
	    }
	    p++;
	}
	printf("%s\n",obuf);
	n = fread(ibuf,1,16,fp);
    }
    return 0;
}

int32_t hdmp( int argc, char *argv[] )
{
    if (argc > 2){
	for (int32_t i=2; i<argc; i++){
	    FILE *fp = fopen(argv[i],"rb");
	    if (fp != NULL){
		ahdmp(fp);
		fclose(fp);
	    } else {
		fprintf(stderr,"Failed to open %s\n",argv[i]);
	    }
	}
    } else {
	ahdmp(stdin);
    }
    return 0;
}

/**********************************************************************
*F: genx0
*	ctb genx0 source [result]
* Coded at: 2017.9.27 by T. Abi
**********************************************************************/
int32_t genx0( int argc, char *argv[] )
{
    int32_t ierr = 0;
    if (argc > 2){
	FILE *f0 = fopen(argv[2],"rb");
	if (f0 != NULL){
	    struct stat sb;
/*	    fstat(fileno(f0),&sb); // needs -posix */
	    stat(argv[2],&sb);
	    size_t sz = sb.st_size;
	    FILE *f1 = fopen("x0","wb");
	    if (f1 != NULL){
		char buf[4096];
		int32_t m;
		int32_t b = 4096;
		int32_t n = sz-0x194;
		while (n >= b){
		    m = fread(buf,1,b,f0);
		    fwrite(buf,1,m,f1);
		    n -= m;
		}
		if (n > 0){
		    m = fread(buf,1,n,f0);
		    fwrite(buf,1,m,f1);
		}
		m = fread(buf,1,8,f0);
		memset(buf,0,8);
		fwrite(buf,1,8,f1);
		m = fread(buf,1,0x18c,f0);
		fwrite(buf,1,m,f1);
		fclose(f1);
	    }
	    fclose(f0);
	}
    } else {
	fprintf(stderr,"usage: ctb genx0 source\n");
    }
    return ierr;
}

int32_t leap0( int32_t y )
{
    return ((((y%4)==0)&&(((y%100)!=0)||((y%400)==0)))?1:0);
}

void dspxdt( size_t ad, void *s )
{
    uint64_t *ns = s;
    double d1 = (*ns)/10000000.0;	/* [sec] */
    double d2 = d1/3600/24;	/* [days] */
    int32_t tm_year = 1601 -1900;
    int32_t tm_mon = 0;
    int32_t tm_mday = 0;
    int32_t tm_hour = 0;
    int32_t tm_min = 0;
    int32_t tm_sec = 0;
    double d3 = d2;
    int32_t y0 = tm_year+1900;
    int32_t lp = leap0(y0);
    int32_t y0d = 365+lp;
    while (d3 > y0d){
	d3 -= y0d;
	tm_year++;
	y0++;
	lp = leap0(y0);
	y0d = 365+lp;
    }
    int32_t mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int32_t m0 = 0;
    int32_t m0d = mdays[m0];
    while (d3 > m0d){
	d3 -= m0d;
	tm_mon++;
	m0++;
	m0d = mdays[m0];
	if ((lp == 1) && (m0 == 1)){
	    m0d++;
	}
    }
    tm_mday = d3;
    d3 -= tm_mday;
    tm_mday++;
    d3 *= 24;
    tm_hour = d3;
    d3 -= tm_hour;
    d3 *= 60;
    tm_min = d3;
    d3 -= tm_min;
    d3 *= 60;
    tm_sec = d3;
    printf("0x%lx : %d/%02d/%02d %02d:%02d:%02d UTC\n",ad,tm_year+1900,tm_mon+1,tm_mday,tm_hour,tm_min,tm_sec);
}

int32_t xlsdt( int argc, char *argv[] )
{
    static uint8_t rent[] = { 'R',0,'o',0,'o',0,'t',0,' ',0,'E',0,'n',0,'t',0,'r',0,'y',0 };
    int32_t ierr = 0;
    if (argc > 2){
	FILE *f0 = fopen(argv[2],"rb");
	if (f0 != NULL){
	    char buf[4096];
	    fseek(f0,0x400,SEEK_SET);
	    int32_t m = fread(buf,1,0x200,f0);
	    int32_t k = memcmp(buf,rent,20);
	    if ((k == 0) && (m > 0)){
		dspxdt(0x400,&buf[0x6c]);
	    } else {
		struct stat sb;
/*		fstat(fileno(f0),&sb); // needs -posix */
		stat(argv[2],&sb);
		size_t sz = sb.st_size;
		size_t iz = sz-0x10;
		iz -= iz & 0xff;
		fseek(f0,iz,SEEK_SET);
		do {
		    m = fread(buf,1,0x200,f0);
/*
fprintf(stderr,"0x%lx : %c %c %c %c %c %c %c %c %c %c\n",iz,buf[0],buf[2],buf[4],buf[6],buf[8],buf[10],buf[12],buf[14],buf[16],buf[18]);
*/
		    k = memcmp(buf,rent,20);
		    if ((k != 0) && (m > 0)){
/*			fseek(f0,-256L,SEEK_CUR); */
			iz -= 0x100;
			fseek(f0,iz,SEEK_SET);
		    }
		} while ((iz > 0x180) && (k != 0));
		if (k == 0){
		    dspxdt(iz,&buf[0x6c]);
		}
	    }
	    fclose(f0);
	}
    } else {
	fprintf(stderr,"usage: ctb genx0 source\n");
    }
    return ierr;
}

/**********************************************************************
*F: chkbom
*	ctb chkbom source
* Coded at: 2020.9.2 by T. Abi
**********************************************************************/
int32_t chkbom( int argc, char *argv[] )
{
    int32_t ierr = 0;
    if (argc > 2){
	FILE *fp = fopen(argv[2],"rb");
	if (fp != NULL){
	    unsigned char buf[8];
	    int32_t n = fread(buf,1,3,fp);
	    if (n == 3){
		if ((buf[0] == 0xef) && (buf[1] == 0xbb) && (buf[2] == 0xbf)){
		    printf("BOM exists.\n");
		} else {
		    printf("No BOM.\n");
		}
	    } else {
		fprintf(stderr,"Couldn't read 3 bytes from %s\n",argv[2]);
	    }
	    fclose(fp);
	} else {
	    fprintf(stderr,"Failed to open %s\n",argv[2]);
	}
    } else {
	fprintf(stderr,"usage: ctb chkbom source\n");
    }
    return ierr;
}

/**********************************************************************
*F: main
**********************************************************************/
int main( int argc, char *argv[] )
{
    if (argc < 2){
	fprintf(stderr,"usage: ctb <key> [args]\n");
	fprintf(stderr,"\thdmp [source]\n");
	fprintf(stderr,"\tb2h [source]\n");
	fprintf(stderr,"\th2b [source [result]]\n");
	fprintf(stderr,"\tb2url [-f] [source]\n");
	fprintf(stderr,"\turl2b [-f] [source]\n");
	fprintf(stderr,"\tchist [-z[C]] [source]\n");
	fprintf(stderr,"\tb64 [source [result]] #decoding\n");
	fprintf(stderr,"\tB64 [source [result]] #encoding\n");
	fprintf(stderr,"\tb62 [source [result]] #decoding\n");
	fprintf(stderr,"\tB62 [source [result]] #encoding\n");
	fprintf(stderr,"\tu8h2u hex\n");
	fprintf(stderr,"\tu2h8u U+nnnn\n");
	fprintf(stderr,"\tcrlf [-ft] [source [result]]\n");
	fprintf(stderr,"\t\tf := c|l|2, from, c=CR l=LF 2=CR+LF\n");
	fprintf(stderr,"\t\tt := c|l|2, to\n");
	fprintf(stderr,"\tmaxline [-v] [source]\n");
	fprintf(stderr,"\tnullitems [source]\n");
	fprintf(stderr,"\tqp [source [result]]\n");
	fprintf(stderr,"\txor [-ky] [source [result]]\n");
	fprintf(stderr,"\t\tky = 0..255\n");
	fprintf(stderr,"\trot13 [source]\n");
	fprintf(stderr,"\tportion range [source]\n");
	fprintf(stderr,"\t\trange := skip,(end+1|+size|-size)\n");
	fprintf(stderr,"\tgather file1 file2 newfile\n");
	fprintf(stderr,"\tcutl [-nN] source [result]\n");
	fprintf(stderr,"\t\tN = 0..255 (72 default)\n");
	fprintf(stderr,"\tlist7z [source]\n");
	fprintf(stderr,"\tvsep k hfile src L R form\n");
	fprintf(stderr,"\tvjoin ha hb hc\n");
	fprintf(stderr,"\tgenx0 source [result]\n");
	fprintf(stderr,"\txlsdt source\n");
	fprintf(stderr,"\tchkbom source\n");
	fprintf(stderr,"\tgenpw [length]\n");
    } else if (strcmp(argv[1],"hdmp") == 0){
	hdmp(argc,argv);
    } else if (strcmp(argv[1],"b2h") == 0){
	b2h(argc,argv);
    } else if (strcmp(argv[1],"h2b") == 0){
	h2b(argc,argv);
    } else if (strcmp(argv[1],"b2url") == 0){
	b2url(argc,argv);
    } else if (strcmp(argv[1],"url2b") == 0){
	url2b(argc,argv);
    } else if (strcmp(argv[1],"chist") == 0){
	chist(argc,argv);
    } else if (strcmp(argv[1],"b64") == 0){
	base64(argc,argv,0,64);
    } else if (strcmp(argv[1],"B64") == 0){
	base64(argc,argv,1,64);
    } else if (strcmp(argv[1],"b62") == 0){
	base64(argc,argv,0,62);
    } else if (strcmp(argv[1],"B62") == 0){
	base64(argc,argv,1,62);
    } else if (strcmp(argv[1],"u8h2u") == 0){
	u8enc(argc,argv,0);
    } else if (strcmp(argv[1],"u2h8u") == 0){
	u8enc(argc,argv,1);
    } else if (strcmp(argv[1],"crlf") == 0){
	crlf(argc,argv);
    } else if (strcmp(argv[1],"nullitems") == 0){
	nullitems(argc,argv);
    } else if (strcmp(argv[1],"maxline") == 0){
	maxline(argc,argv);
    } else if (strcmp(argv[1],"qp") == 0){
	qp(argc,argv);
    } else if (strcmp(argv[1],"xor") == 0){
	enxor(argc,argv);
    } else if (strcmp(argv[1],"rot13") == 0){
	rot13(argc,argv);
    } else if (strcmp(argv[1],"ebgcl") == 0){
	ebgcl(argc,argv);
    } else if (strcmp(argv[1],"portion") == 0){
	portion(argc,argv);
    } else if (strcmp(argv[1],"gather") == 0){
	gather(argc,argv);
    } else if (strcmp(argv[1],"cutl") == 0){
	cutl(argc,argv);
    } else if (strcmp(argv[1],"list7z") == 0){
	list7z(argc,argv);
    } else if (strcmp(argv[1],"vsep") == 0){
	vsep(argc,argv);
    } else if (strcmp(argv[1],"vjoin") == 0){
	vjoin(argc,argv);
    } else if (strcmp(argv[1],"genx0") == 0){
	genx0(argc,argv);
    } else if (strcmp(argv[1],"xlsdt") == 0){
	xlsdt(argc,argv);
    } else if (strcmp(argv[1],"chkbom") == 0){
	chkbom(argc,argv);
    } else if (strcmp(argv[1],"genpw") == 0){
	genpw(argc,argv);
    } else {
	fprintf(stderr,"[%s] unknown.\n",argv[1]);
    }
    return 0;
}

