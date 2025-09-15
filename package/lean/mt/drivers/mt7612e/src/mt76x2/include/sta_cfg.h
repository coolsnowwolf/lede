/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    sta_cfg.h

    Abstract:
    

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef __STA_CFG_H__
#define __STA_CFG_H__

INT RTMPSTAPrivIoctlSet(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING SetProcName,
	IN PSTRING ProcArg);

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
/* set WOW enable */
INT Set_WOW_Enable(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg);
/* set GPIO pin for wake-up signal */
INT Set_WOW_GPIO(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg);
/* set delay time for WOW really enable */
INT Set_WOW_Delay(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg);
/* set wake up hold time */
INT Set_WOW_Hold(
		IN PRTMP_ADAPTER		pAd,
		IN PSTRING				arg);
/* set wakeup signal type */
INT Set_WOW_InBand(
		IN PRTMP_ADAPTER		pAd,
		IN PSTRING				arg);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#endif /* __STA_CFG_H__ */

