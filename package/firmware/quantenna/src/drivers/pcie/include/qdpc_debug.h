/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef __QDPC_DEBUG_H__
#define __QDPC_DEBUG_H__

/* Debug macros */

#define SUCCESS         0
#define FAILURE        -1

#ifdef  DEBUG
#define PRINT_DBG(format, ...)           printk(KERN_DEBUG format, ##__VA_ARGS__) 
#else
#define PRINT_DBG(format, ...)           do { } while(0);
#endif

#define PRINT_ERROR(format, ...)         printk(KERN_ERR format, ##__VA_ARGS__) 
#define PRINT_INFO(format, ...)          printk(KERN_INFO format, ##__VA_ARGS__) 

#ifdef DEBUG
#define qdpc_print_dump(str_, buf_, len_)	\
{ 					\
	u32 i = 0; 			\
	printk("%s\n", str_); 		\
	printk("0x%04X : ", i*8); 	\
	for (i=0; i < (u32)(len_); i++) {	\
		if (i && ((i%8) == 0)) {	\
			printk( "%s", "\n");	\
			printk("0x%04X : ", (i));\
		}				\
		printk("%02x ", (buf_)[i]);	\
	}					\
	printk("\n%s\n", str_);			\
}
#else
#define qdpc_print_dump(str_, buf_, len_)
#endif
#endif
