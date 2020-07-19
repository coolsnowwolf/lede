/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifndef __WNM_CONFIG_H__
#define __WNM_CONFIG_H__

#ifdef DOT11V_WNM_SUPPORT
#include "rtmp_type.h"
#include "dot11v_wnm.h"


#define WNM_DEFAULT_QUIET_PERIOD	200


typedef struct _WNM_CONFIG
{
	BOOLEAN bDot11vWNM_BSSEnable;
	BOOLEAN bDot11vWNM_DMSEnable;
	BOOLEAN bDot11vWNM_FMSEnable;
	BOOLEAN bDot11vWNM_SleepModeEnable;
	BOOLEAN bDot11vWNM_TFSEnable;
#ifdef CONFIG_AP_SUPPORT
	VOID *DMSEntry[10];
#endif /* CONFIG_AP_SUPPORT */

} WNM_CONFIG;



#endif /* DOT11V_WNM_SUPPORT */

#endif /* __WNM_CONFIG_H__ */



