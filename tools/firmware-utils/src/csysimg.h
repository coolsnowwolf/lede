/*
 *
 *  Copyright (C) 2007,2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program was based on the code found in various Linux
 *  source tarballs released by Edimax for it's devices.
 *  Original author: David Hsu <davidhsu@realtek.com.tw>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#define SIG_LEN		4

#define ADM_CODE_ADDR	0x80500000
#define ADM_WEBP_ADDR	0x10000
#define ADM_WEBP_SIZE	0x10000
#define ADM_BOOT_SIZE	0x8000
#define ADM_CONF_SIZE	0x8000
#define ADM_BOOT_SIG	"\x00\x60\x1A\x40"


/*
 * Generic signatures
 */
#define SIG_CSYS	"CSYS"
#define SIG_CONF	"HS\x00\x00"
#define SIG_BOOT_RTL	"\x00\x00\x40\x21"

/*
 * Web page signatures
 */
#define SIG_BR6104K	"WB4K"
#define SIG_BR6104KP	"WBKP"
#define SIG_BR6104Wg	"WBGW"
#define SIG_BR6104IPC	"WBIP"
#define SIG_BR6114WG	SIG_BR6104IPC
#define SIG_BR6524K	"2-K-"
#define SIG_BR6524KP	"2-KP"	/* FIXME: valid? */
#define SIG_BR6524N	"WNRA"
#define SIG_BR6524WG	"2-WG"	/* FIXME: valid? */
#define SIG_BR6524WP	"2-WP"	/* FIXME: valid? */
#define SIG_BR6541K	"4--K"
#define SIG_BR6541KP	"4-KP"	/* FIXME: valid? */
#define SIG_BR6541WP	"4-WP"	/* FIXME: valid? */
#define SIG_C54BSR4	SIG_BR6104IPC
#define SIG_EW7207APg	"EWAS"
#define SIG_PS1205UWg	"4000"
#define SIG_PS3205U	"5010"
#define SIG_PS3205UWg	"5011"
#define SIG_RALINK	"RNRA"
#define SIG_5GXI	"5GXI"	/* fake signature */

#define SIG_H2BR4	SIG_BR6524K
#define SIG_H2WR54G	SIG_BR6524WG

#define SIG_XRT401D	SIG_BR6104K
#define SIG_XRT402D	SIG_BR6524K

/*
 * CSYS image file header
 */
struct csys_header {
	unsigned char sig[SIG_LEN];
	uint32_t addr;
	uint32_t size;
};
