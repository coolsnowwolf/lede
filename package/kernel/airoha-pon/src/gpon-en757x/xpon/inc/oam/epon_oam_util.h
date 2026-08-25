/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	epon_oam_util.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	JQ.Zhu		2012/7/5		Create
*/

#ifndef __EPON_UTIL
#define __EPON_UTIL


#define NO_QMARKS 0
#define QMARKS 1
#define NO_ATTRIBUTE 2
#define MAC_PATH "/etc/mac.conf"
#define MAXGET_PROFILE_SIZE 128


#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAIL
#define FAIL -1
#endif

#ifdef CONFIG_DEBUG
#define eponOamDbg(L, fmt, args...) {if (eponOamCfg.dbgLvl>=L) tcdbg_printf("%s [%04d]: "fmt, __FILE__, __LINE__, ##args); } 
#define eponOamLlidDbg(L, llid, fmt, args...) {if ((eponOamCfg.dbgLvl >= L) && (eponOamCfg.dbgLlidMask & (1<<llid))) tcdbg_printf("%s [%04d]: "fmt, __FILE__, __LINE__, ##args); }
#else
#define eponOamLlidDbg(L, llid, fmt, args...) {}
#define eponOamDbg(L, fmt, args...) {}
#endif

int eponOamGetOnuLanMac(char *out_buf);
u_int get32(u_char *cp);
u_short get16(u_char *cp);
u_int64_t get64(u_char *cp);
u_char *put16(u_char	*cp,u_short	x);
u_char *put32(u_char	*cp, u_int	x);
u_char *put64(u_char	*cp, u_int64_t	x);
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))


#endif //__EPON_UTIL

