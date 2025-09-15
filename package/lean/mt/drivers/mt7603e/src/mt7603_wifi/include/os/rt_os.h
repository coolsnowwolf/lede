/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt_os.h

	Abstract:
	Put all OS related definition/structure/MACRO here.

	Note:
	Used in UTIL/NETIF module.

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef _RT_OS_H_
#define _RT_OS_H_


#ifdef LINUX
#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE                              0x8BE0
#endif
#define SIOCIWFIRSTPRIV								SIOCDEVPRIVATE
#endif
#endif /* LINUX */



#ifdef CONFIG_AP_SUPPORT
/* Ralink defined OIDs */
#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET							(SIOCIWFIRSTPRIV + 0x02)
#define RT_PRIV_IOCTL_EXT							(SIOCIWFIRSTPRIV + 0x0E) /* Sync. with RT61 (for wpa_supplicant) */				
#ifdef DBG
#define RTPRIV_IOCTL_BBP                            (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC                            (SIOCIWFIRSTPRIV + 0x05)

#ifdef RTMP_RF_RW_SUPPORT
#define RTPRIV_IOCTL_RF                             (SIOCIWFIRSTPRIV + 0x13)
#endif /* RTMP_RF_RW_SUPPORT */

#endif /* DBG */
#define RTPRIV_IOCTL_E2P                            (SIOCIWFIRSTPRIV + 0x07)

#define RTPRIV_IOCTL_ATE							(SIOCIWFIRSTPRIV + 0x08)

#define RTPRIV_IOCTL_STATISTICS                     (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE                (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA                    (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY                    (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE					(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)	/* modified by Red@Ralink, 2009/09/30 */
#define RTPRIV_IOCTL_STATIC_WEP_COPY                (SIOCIWFIRSTPRIV + 0x10)

#define RTPRIV_IOCTL_SHOW							(SIOCIWFIRSTPRIV + 0x11)
#define RTPRIV_IOCTL_WSC_PROFILE                    (SIOCIWFIRSTPRIV + 0x12)
#define RTPRIV_IOCTL_QUERY_BATABLE                  (SIOCIWFIRSTPRIV + 0x16)
#ifdef INF_AR9
#define RTPRIV_IOCTL_GET_AR9_SHOW   (SIOCIWFIRSTPRIV + 0x17)
#endif/* INF_AR9 */
#define RTPRIV_IOCTL_SET_WSCOOB	(SIOCIWFIRSTPRIV + 0x19)
#define RTPRIV_IOCTL_WSC_CALLBACK	(SIOCIWFIRSTPRIV + 0x1A)
#endif /* CONFIG_AP_SUPPORT */


#define RTPRIV_IOCTL_GET_DRIVER_INFO				(SIOCIWFIRSTPRIV + 0x1D)

#ifdef WIFI_DIAG
#define RTPRIV_IOCTL_GET_PROCESS_INFO				(SIOCIWFIRSTPRIV + 0x1E)
#endif

#endif /* _RT_OS_H_ */
