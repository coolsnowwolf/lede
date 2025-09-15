/******************************************************************************
 *	fbtest - fbtest.c
 *	test program for the tuxbox-framebuffer device
 *	tests all GTX/eNX supported modes
 *                                                                            
 *	(c) 2003 Carsten Juttner (carjay@gmx.net)
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	The Free Software Foundation; either version 2 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program; if not, write to the Free Software
 * 	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *  									      
 ******************************************************************************
 * $Id: fbtest.c,v 1.5 2005/01/14 23:14:41 carjay Exp $
 ******************************************************************************/

// TODO: - should restore the colour map and mode to what it was before
//	 - is colour map handled correctly?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <linux/fb.h>

#define FBDEV "/dev/fb0"

struct vidsize{
	int width;
	int height;
};
static
const struct vidsize vidsizetable[]={	// all supported sizes
	{720,576},{720,480},{720,288},{720,240},
	{640,576},{640,480},{640,288},{640,240},
	{360,576},{360,480},{360,288},{360,240},
	{320,576},{320,480},{320,288},{320,240}
};
#define VIDSIZENUM (sizeof(vidsizetable)/sizeof(struct vidsize))

enum pixenum{	// keep in sync with pixname !
	CLUT4=0,
	CLUT8,
	RGB565,
	ARGB1555,
	ARGB
};
const char *pixname[] = {
	"CLUT4",
	"CLUT8",
	"RGB565",
	"ARGB1555",
	"ARGB"
};

struct pixelformat{
	char *name;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
	char bpp;
	char pixenum;
};

static		// so far these are all modes supported by the eNX (only partially by GTX)
const struct pixelformat pixelformattable[] = {
	{ .name = "CLUT4 ARGB8888", 	// CLUT4 (ARGB8888)
		.bpp = 4, .pixenum = CLUT4,
		.red = 	 { .offset = 0, .length=8, .msb_right =0 },
		.green = { .offset = 0, .length=8, .msb_right =0 },
		.blue =  { .offset = 0, .length=8, .msb_right =0 },
		.transp=  { .offset = 0, .length=8, .msb_right =0 }
	},
	{ .name = "CLUT4 ARGB1555", 	// CLUT4 (ARGB1555)
		.bpp = 4, .pixenum = CLUT4,
		.red = 	 { .offset = 0, .length=5, .msb_right =0 },
		.green = { .offset = 0, .length=5, .msb_right =0 },
		.blue =  { .offset = 0, .length=5, .msb_right =0 },
		.transp=  { .offset = 0, .length=1, .msb_right =0 }
	},
	{ .name = "CLUT8 ARGB8888",	// CLUT8 (ARGB8888)
		.bpp = 8, .pixenum = CLUT8,
		.red = 	 { .offset = 0, .length=8, .msb_right =0 },
		.green = { .offset = 0, .length=8, .msb_right =0 },
		.blue =  { .offset = 0, .length=8, .msb_right =0 },
		.transp=  { .offset = 0, .length=8, .msb_right =0 }
	},
	{ .name = "CLUT8 ARGB1555",	// CLUT8 (ARGB1555)
		.bpp = 8, .pixenum = CLUT8,
		.red = 	 { .offset = 0, .length=5, .msb_right =0 },
		.green = { .offset = 0, .length=5, .msb_right =0 },
		.blue =  { .offset = 0, .length=5, .msb_right =0 },
		.transp=  { .offset = 0, .length=1, .msb_right =0 }
	},
	{ .name = "ARGB1555", 	// ARGB1555
		.bpp = 16, .pixenum = ARGB1555,
		.red = 	 { .offset = 10, .length=5, .msb_right =0 },
		.green = { .offset = 5,  .length=5, .msb_right =0 },
		.blue =  { .offset = 0,  .length=5, .msb_right =0 },
		.transp=  { .offset = 15, .length=1, .msb_right =0 }
	},
	{ .name = "RGB565", 		// RGB565
		.bpp = 16, .pixenum = RGB565,
		.red = 	 { .offset = 11, .length=5, .msb_right =0 },
		.green = { .offset = 5,  .length=6, .msb_right =0 },
		.blue =  { .offset = 0,  .length=5, .msb_right =0 },
		.transp=  { .offset = 0,  .length=0, .msb_right =0 }
	},
	{ .name = "ARGB",	// 32 f*cking bits, the real McCoy :)
		.bpp = 32, .pixenum = ARGB,
		.red = 	 { .offset = 16, .length=8, .msb_right =0 },
		.green = { .offset = 8,  .length=8, .msb_right =0 },
		.blue =  { .offset = 0,  .length=8, .msb_right =0 },
		.transp=  { .offset = 24, .length=8, .msb_right =0 }
	}
};
#define PIXELFORMATNUM (sizeof(pixelformattable)/sizeof(struct pixelformat))

struct colour {
	__u16 r;
	__u16 g;
	__u16 b;
	__u16 a;
};
static
struct colour colourtable[] = {
	{.r =0xffff, .g = 0xffff, .b=0xffff, .a=0xffff},	// fully transparent white
	{.r =0xffff, .g = 0x0000, .b=0x0000, .a=0x0000},	// red
	{.r =0x0000, .g = 0xffff, .b=0x0000, .a=0x0000},	// green
	{.r =0x0000, .g = 0x0000, .b=0xffff, .a=0x0000},	// blue
	{.r =0x0000, .g = 0x0000, .b=0x0000, .a=0x0000}		// black
};
#define COLOURNUM (sizeof(colourtable)/sizeof(struct colour))

struct rect{
	int x;
	int y;
	int width;
	int height;
	const struct colour *col;
};
struct pixel{		// up to 32 bits of pixel information
	char byte[4];
};

void col2pixel (struct pixel *pix, const struct pixelformat *pixf, const struct colour *col){
	switch (pixf->pixenum){
		case RGB565:
			pix->byte[0]=(col->r&0xf8)|(col->g&0xfc)>>5;
			pix->byte[1]=(col->g&0xfc)<<3|(col->b&0xf8)>>3;
			break;
		case ARGB1555:
			pix->byte[0]=(col->a&0x80)|(col->r&0xf8)>>1|(col->g&0xf8)>>6;
			pix->byte[1]=(col->g&0xf8)<<2|(col->b&0xf8)>>3;
			break;
		case ARGB:
			pix->byte[0]=col->a;
			pix->byte[1]=col->r;
			pix->byte[2]=col->g;
			pix->byte[3]=col->b;
			break;
		default:
			printf ("unknown pixelformat\n");
			exit(1);
	}
}

int setmode(int fbd, const struct pixelformat *pixf,const struct vidsize *vids){
	struct fb_var_screeninfo var;
	int stat;
	stat = ioctl (fbd, FBIOGET_VSCREENINFO,&var);
	if (stat<0) return -2;
	
	var.xres= vids->width;
	var.xres_virtual = vids->width;
	var.yres= vids->height;
	var.yres_virtual = vids->height;
	
	var.bits_per_pixel = pixf->bpp;
	var.red = pixf->red;
	var.green = pixf->green;
	var.blue = pixf->blue;
	var.transp = pixf->transp;

	stat = ioctl (fbd, FBIOPUT_VSCREENINFO,&var);
	if (stat<0) return -1;
	return 0;
}

// unefficient implementation, do NOT use it for your next ego shooter, please :)
// for 4-Bit only rectangles with even width are supported
// CLUT-modes use value of red component as index
void drawrect(void *videoram, struct rect *r, const struct pixelformat *pixf, const struct vidsize *vids){
	int x,y,corwidth, bpp = 0, tocopy = 1;
	struct pixel pix;
	unsigned char *pmem = videoram;
	corwidth = r->width;	// actually only "corrected" for 4 Bit

	if (pixf->pixenum!=CLUT4&&pixf->pixenum!=CLUT8){
		switch (pixf->pixenum){
			case ARGB1555:
			case RGB565:
				bpp = 16;
				tocopy = 2;
				break;
			case ARGB:
				bpp = 32;
				tocopy = 4;
				break;
			default:
				printf ("drawrect: unknown pixelformat(%d) bpp:%d\n",pixf->pixenum,pixf->bpp);
				exit(1);
		}
		col2pixel(&pix,pixf,r->col);
	} else {
		switch (pixf->pixenum){	// CLUT = Colour LookUp Table (palette)
			case CLUT4:	// take red value as index in this case
				pix.byte[0]=(r->col->r)<<4|(r->col->r&0xf);	// slightly cryptic... "rect->colour->red"
				corwidth>>=1;	// we copy bytes
				bpp=4;
				tocopy=1;
				break;
			case CLUT8:
				pix.byte[0]=(r->col->r&0xff);
				bpp=8;
				tocopy=1;
				break;
		}
	}
	pmem=videoram+((((r->y*vids->width)+r->x)*bpp)>>3);
	for (y=0;y<r->height;y++){
		int offset = 0;
		for (x=0;x<corwidth;x++){
			memcpy (pmem+offset,pix.byte,tocopy);
			offset+=tocopy;
		}
		pmem +=((vids->width*bpp)>>3);	// skip one whole line, actually should be taken from "fix-info"
	}
}
			
// create quick little test image, 4 colours from table
void draw4field(void *videoram, const struct pixelformat *pixf, const struct vidsize *vids){
	struct rect r;
	struct colour c;
	int height, width;
	c.r = 1;	// only used for the indexed modes, r is taken as index
	height = vids->height;
	width = vids->width;

	r.height = height>>1;
	r.width = width>>1;
	r.x = 0;	r.y = 0;
	if (pixf->pixenum==CLUT4||pixf->pixenum==CLUT8) r.col = &c;
	else r.col = &colourtable[1];
	drawrect (videoram, &r, pixf, vids);

	r.x = width/2;	r.y = 0;
	if (pixf->pixenum==CLUT4||pixf->pixenum==CLUT8) c.r = 2;
	else r.col = &colourtable[2];
	drawrect (videoram, &r, pixf, vids);

	r.x = 0;	r.y = height/2;
	if (pixf->pixenum==CLUT4||pixf->pixenum==CLUT8) c.r = 3;
	else r.col = &colourtable[3];
	drawrect (videoram, &r, pixf, vids);

	r.x = width/2;	r.y = height/2;
	if (pixf->pixenum==CLUT4||pixf->pixenum==CLUT8) c.r = 0;
	else r.col = &colourtable[0];
	drawrect (videoram, &r, pixf, vids);
}

void usage(char *name){
 	printf ("Usage: %s [options]\n"
		"Options: -f<pixelformat>\n"
		"            where format is one of:\n"
		"              CLUT4,CLUT8,ARGB1555,RGB565,ARGB\n"
		"         -s<width>x<heigth>\n"
		"            where width is either 720,640,360,320\n"
		"                  and height is either 288,240,480,576\n"
		"         -n\n"
		"            disables clearing the framebuffer after drawing\n"
		"            the testimage. This can be useful to keep the last\n"
		"            drawn image onscreen.\n"
		"\nExample: %s -fRGB322\n",name,name);
	exit(0);
}

int main (int argc,char **argv){
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct fb_cmap cmap;
	struct rect r;
	int fbd;
	unsigned char *pfb;
	int stat;
	int optchar,fmode=-1,smode=-1,clear=1;
	int i_cmap,i_size,i_pix;
	extern char *optarg;
	
	if (argc!=0&&argc>4) usage(argv[0]);
	while ( (optchar = getopt (argc,argv,"f:s:n"))!= -1){
		int i,height,width;
		switch (optchar){
			case 'f':
				for (i=0;i<(sizeof(pixname)/sizeof(char*));i++){
					if (!strncmp (optarg,pixname[i],strlen(pixname[i]))){
						fmode=i;
						printf ("displaying only %s-modes\n",pixname[i]);
						break;
					}
				}
				if (fmode==-1){
					printf ("unknown pixelformat\n");
					exit(0);
				}
				break;
			case 's':
				if (sscanf (optarg,"%dx%d",&width,&height)!=2){
					printf ("parsing size failed\n");
					exit(0);
				} else {
					printf ("requested size %dx%d\n",width,height);
					for (i=0;i<VIDSIZENUM;i++){
						if (vidsizetable[i].width == width &&
							vidsizetable[i].height == height){
							smode = i;
							break;
						}
					}
					if (smode==-1){
						printf ("this size is not supported\n");
						exit(0);
					}
				}
				break;
			case 'n':
				clear = 0;
				printf ("clearing framebuffer after drawing is disabled\n");
				break;
			case '?':
				usage (argv[0]);
		}
	}
	
	fbd = open (FBDEV, O_RDWR);
	if (fbd<0){
		perror ("Error opening framebuffer device");
		return 1;
	}
	stat = ioctl (fbd, FBIOGET_FSCREENINFO,&fix);
	if (stat<0){
		perror ("Error getting fix screeninfo");
		return 1;
	}
	stat = ioctl (fbd, FBIOGET_VSCREENINFO,&var);
	if (stat<0){
		perror ("Error getting var screeninfo");
		return 1;
	}
	stat = ioctl (fbd, FBIOPUT_VSCREENINFO,&var);
	if (stat<0){
		perror ("Error setting mode");
		return 1;
	}
	pfb = mmap (0, fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fbd, 0);
	if (pfb == MAP_FAILED){
		perror ("Error mmap'ing framebuffer device");
		return 1;
	}

	// iterate over all modes
	for (i_pix=0;i_pix<PIXELFORMATNUM;i_pix++){
		if (fmode!=-1 && pixelformattable[i_pix].pixenum != fmode) continue;
		printf ("testing: %s",pixelformattable[i_pix].name);
		printf (" for sizes: \n");
		for (i_size=0;i_size<VIDSIZENUM;i_size++){
			if (smode!=-1 && i_size!=smode) continue;
			printf ("%dx%d ",vidsizetable[i_size].width,vidsizetable[i_size].height);
			fflush(stdout);
			if ((i_size%4)==3) printf ("\n");
			
			// try to set mode
			stat = setmode(fbd,&pixelformattable[i_pix],&vidsizetable[i_size]);
			if (stat==-2) perror ("fbtest: could not get fb_var-screeninfo from fb-device");
			else if (stat==-1){
				printf ("\nCould not set mode %s (%dx%d), possible reasons:\n"
					"- you have a GTX (soz m8)\n"
					"- your configuration does not have enough graphics RAM\n"
					"- you found a bug\n"
					"choose your poison accordingly...\n",
					pixelformattable[i_pix].name,vidsizetable[i_size].width,vidsizetable[i_size].height);
					continue;
			}
			// fill cmap;
			cmap.len = 1;
			if ((pixelformattable[i_pix].bpp==4)||
				((pixelformattable[i_pix].bpp==8)&&(pixelformattable[i_pix].red.length!=3))){
				for (i_cmap=0;i_cmap<COLOURNUM;i_cmap++){
					cmap.start=i_cmap;
					cmap.red=&colourtable[i_cmap].r;
					cmap.green=&colourtable[i_cmap].g;
					cmap.blue=&colourtable[i_cmap].b;
					cmap.transp=&colourtable[i_cmap].a;
					stat = ioctl (fbd, FBIOPUTCMAP, &cmap);
					if (stat<0) printf ("setting colourmap failed\n");
				}
			}
			// create the test image
			draw4field(pfb,&pixelformattable[i_pix],&vidsizetable[i_size]);
			usleep (500000);
			// clear screen
			if (clear){
				r.x=r.y=0;r.width = vidsizetable[i_size].width; r.height = vidsizetable[i_size].height;
				r.col = &colourtable[4];
				drawrect(pfb,&r,&pixelformattable[i_pix],&vidsizetable[i_size]);
			}
		}
		printf ("\n");
	}

	stat = munmap (pfb,fix.smem_len);
	if (stat<0){
		perror ("Error munmap'ing framebuffer device");
		return 1;
	}
	close (fbd);
	return 0;
}
