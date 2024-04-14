/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SIPC_BIG_TO_LITTLE_H
#define __SIPC_BIG_TO_LITTLE_H
//#define CONFIG_SIPC_BIG_TO_LITTLE /* sipc little */

#define BL_READB(addr) \
    ({ unsigned char __v = (*(volatile unsigned char *) (addr)); __v; })
#define BL_WRITEB(b,addr) (void)((*(volatile unsigned int *) (addr)) = (b))

#define BL_GETB(v) ((v))
#define BL_SETB(v, b) ((v) = (b))


#ifdef CONFIG_SIPC_BIG_TO_LITTLE
/* little 0x78563412
    0x12
    0x34
    0x56
    0x78
    read:
    big: 0x12345678==>0x78563412 
    write: 0x78563412 ===> 0x12345678*/
#define BL_READW(addr) \
    ({ unsigned short __t = (*(volatile unsigned short *) (addr)); \
	unsigned short __v = ((__t & 0x00ff) << 8) + ((__t & 0xff00) >> 8); \
	__v; })
#define BL_READL(addr) \
    ({ unsigned int __t = (*(volatile unsigned int *) (addr)); \
	unsigned int __v = ((__t & 0x000000ff) << 24) + ((__t & 0x0000ff00) << 8) + \
	((__t & 0x00ff0000) >> 8) + ((__t & 0xff000000) >> 24); \
	__v; })

#define BL_WRITEW(b,addr) \
    ({ unsigned short __v = (((b) & 0x00ff) << 8) + (((b) & 0xff00) >> 8); \
	(*(volatile unsigned short *) (addr)) = __v; })

#define BL_WRITEL(b,addr) \
    ({ unsigned int __v = (((b) & 0x000000ff) << 24) + (((b) & 0xff00) >> 8) + \
	(((b) & 0x00ff0000) >> 8) + (((b) & 0xff000000) >> 24); \
	(*(volatile unsigned int *) (addr)) = __v; })

#define BL_GETL(v) \
({unsigned int __v = (((v) & 0x000000ff) << 24) + (((v) & 0x0000ff00) << 8) + \
	(((v) & 0x00ff0000) >> 8) + (((v) & 0xff000000) >> 24); \
	__v; })
#define BL_SETL(v, b) \
	((v) = (((b) & 0x000000ff) << 24) + (((b) & 0x0000ff00) << 8) + \
			(((b) & 0x00ff0000) >> 8) + (((b) & 0xff000000) >> 24))
#define BL_GETW(v) \
	({unsigned int __v = (((v) & 0x00ff) << 8) + (((v) & 0xff00) >> 8); \
			__v; })
#define BL_SETW(v, b) \
		((v) = (((b) & 0x00ff) << 8) + (((b) & 0xff00) >> 8))

#else
#define BL_GETW(v)	v
#define BL_GETL(v)	v

#define BL_SETW(v, b)	((v) = (b))
#define BL_SETL(v, b)	((v) = (b))

#define BL_READW(addr) \
    ({ unsigned short __v = (*(volatile unsigned short *) (addr)); __v; })
#define BL_READL(addr) \
    ({ unsigned int __v = (*(volatile unsigned int *) (addr)); __v; })

#define BL_WRITEW(b,addr) (void)((*(volatile unsigned short *) (addr)) = (b))
#define BL_WRITEL(b,addr) (void)((*(volatile unsigned int *) (addr)) = (b))

#endif

#endif

