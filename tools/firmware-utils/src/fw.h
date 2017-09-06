/*
 *  * Copyright (C) 2007 Ubiquiti Networks, Inc.
 *   *
 *    * This program is free software; you can redistribute it and/or
 *     * modify it under the terms of the GNU General Public License as
 *      * published by the Free Software Foundation; either version 2 of the
 *       * License, or (at your option) any later version.
 *        *
 *         * This program is distributed in the hope that it will be useful, but
 *          * WITHOUT ANY WARRANTY; without even the implied warranty of
 *           * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *            * General Public License for more details.
 *             *
 *              * You should have received a copy of the GNU General Public License
 *               * along with this program; if not, write to the Free Software
 *                * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *                 */

#ifndef FW_INCLUDED
#define FW_INCLUDED

#include <sys/types.h>

#define MAGIC_HEADER	"OPEN"
#define MAGIC_PART	"PART"
#define MAGIC_END	"END."

#define MAGIC_LENGTH	4

typedef struct header {
	char magic[MAGIC_LENGTH];
	char version[256];
	u_int32_t crc;
	u_int32_t pad;
} __attribute__ ((packed)) header_t;

typedef struct part {
	char magic[MAGIC_LENGTH];
	char name[16];
	char pad[12];
	u_int32_t memaddr;
	u_int32_t index;
	u_int32_t baseaddr;
	u_int32_t entryaddr;
	u_int32_t data_size;
	u_int32_t part_size;
} __attribute__ ((packed)) part_t;

typedef struct part_crc {
	u_int32_t crc;
	u_int32_t pad;
} __attribute__ ((packed)) part_crc_t;

typedef struct signature {
	char magic[MAGIC_LENGTH];
	u_int32_t crc;
	u_int32_t pad;
} __attribute__ ((packed)) signature_t;

#define VERSION "1.2"

#define INFO(...) fprintf(stdout, __VA_ARGS__)
#define ERROR(...) fprintf(stderr, "ERROR: "__VA_ARGS__)
#define WARN(...) fprintf(stderr, "WARN: "__VA_ARGS__)
#define DEBUG(...) do {\
        if (debug) \
                fprintf(stdout, "DEBUG: "__VA_ARGS__); \
} while (0);

#endif
