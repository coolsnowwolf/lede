/* vi: set sw=4 ts=4: */
/*
 *	(SEA)ttle i(MA)ge is the image which used in project seattle.
 *
 *	Created by David Hsieh <david_hsieh@alphanetworks.com>
 *	Copyright (C) 2008-2009 Alpha Networks, Inc.
 *
 *	This file is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation; either'
 *	version 2.1 of the License, or (at your option) any later version.
 *
 *	The GNU C Library is distributed in the hope that it will be useful,'
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with the GNU C Library; if not, write to the Free
 *	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *	02111-1307 USA.
 */

#ifndef __SEAMA_HEADER_FILE__
#define __SEAMA_HEADER_FILE__

#include <stdint.h>

#define SEAMA_MAGIC		0x5EA3A417

/*
 *	SEAMA looks like the following map.
 *	All the data of the header should be in network byte order.
 *
 *  +-------------+-------------+------------
 *	| SEAMA magic               |     ^
 *  +-------------+-------------+     |
 *	| reserved    | meta size   |     |
 *  +-------------+-------------+   header
 *	| image size (0 bytes)      |     |
 *  +-------------+-------------+     |
 *	~ Meta data                 ~     v
 *  +-------------+-------------+------------
 *	| SEAMA magic               |   ^     ^
 *  +-------------+-------------+   |     |
 *	| reserved    | meta size   |   |     |
 *  +-------------+-------------+   |     |
 *	| image size                |   |     |
 *  +-------------+-------------+ header  |
 *	|                           |   |     |
 *	| 16 bytes of MD5 digest    |   |     |
 *	|                           |   |     |
 *	|                           |   |     |
 *  +-------------+-------------+   |     |
 *	~ Meta data                 ~   v     |
 *  +-------------+-------------+-------  |
 *	|                           |         |
 *	| Image of the 1st entity   |         |
 *	~                           ~ 1st entity
 *	|                           |         |
 *	|                           |         v
 *  +-------------+-------------+-------------
 *	| SEAMA magic               |   ^     ^
 *  +-------------+-------------+   |     |
 *	| reserved    | meta size   |   |     |
 *  +-------------+-------------+   |     |
 *	| image size                |   |     |
 *  +-------------+-------------+ header  |
 *	|                           |   |     |
 *	| 16 bytes of MD5 digest    |   |     |
 *	|                           |   |     |
 *	|                           |   |     |
 *  +-------------+-------------+   |     |
 *	~ Meta data                 ~   v     |
 *  +-------------+-------------+-------  |
 *	|                           |         |
 *	| Image of the 2nd entity   |         |
 *	~                           ~ 2nd entity
 *	|                           |         |
 *	|                           |         v
 *  +-------------+-------------+-------------
 */


/*
 *	SEAMA header
 *
 *	|<-------- 32 bits -------->|
 *  +-------------+-------------+
 *	| SEAMA magic               |
 *  +-------------+-------------+
 *	| reserved    | meta size   |
 *  +-------------+-------------+
 *	| image size                |
 *  +-------------+-------------+
 */
/* seama header */
typedef struct seama_hdr	seamahdr_t;
struct seama_hdr
{
	uint32_t	magic;			/* should always be SEAMA_MAGIC. */
	uint16_t	reserved;		/* reserved for  */
	uint16_t	metasize;		/* size of the META data */
	uint32_t	size;			/* size of the image */
} __attribute__ ((packed));


#endif
