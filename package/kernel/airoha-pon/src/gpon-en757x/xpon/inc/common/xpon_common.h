/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2016, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	xpon common

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Wei.Sun		2016/4/23		Create
*/
#ifndef _XPON_COMMON_H
#define _XPON_COMMON_H
#include <time.h>

/*
***********************************************************************************************************
Time MACRO for GPON/EPON/PMMGR module
***********************************************************************************************************
*/
/** gettimeofday() function may have risk while set the sys time from 1970 to real time **
 ** use this MACRO get the sys uptime to instand of the gettimeofday() for OMCI OAM and PMMGR module **/
#define MACRO_XPON_COMMON_READUPTIME(__TV__)			\
{												\
	struct timespec __TS__;						\
												\
	clock_gettime(CLOCK_MONOTONIC,&__TS__);		\
												\
	(__TV__)->tv_sec = __TS__.tv_sec;			\
	(__TV__)->tv_usec = __TS__.tv_nsec / 1000;	\
}


#endif
