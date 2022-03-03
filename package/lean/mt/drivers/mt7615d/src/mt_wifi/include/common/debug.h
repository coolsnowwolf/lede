/****************************************************************************
 * Mediatek Inc.
 * 5F., No.5, Taiyuan 1st St., Zhubei City,
 * Hsinchu County 302, Taiwan, R.O.C.
 * (c) Copyright 2014, Mediatek, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
	****************************************************************************

    Module Name:
	debug.h

    Abstract:
	All function prototypes and macro are provided from debug message.

    Revision History:
    Who             When            What
    ---------    ----------    ----------------------------------------------
    Name           Date              Modification logs
    UnifyLOGTF   2014.07.11     Initial version

***************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__


/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */

/* Debug Level */
#define DBG_LVL_OFF		0
#define DBG_LVL_ERROR	1
#define DBG_LVL_WARN	2
#define DBG_LVL_TRACE	3
#define DBG_LVL_INFO	4
#define DBG_LVL_LOUD	5
#define DBG_LVL_NOISY	6
#define DBG_LVL_MAX		DBG_LVL_NOISY
#if !defined(EVENT_TRACING)
/* Debug Category */
/* if change the definition of below category or update new category, please update cat_str and sub_cat_str in cmm_info.c */
#define DBG_CAT_MISC    0 /* misc */
#define DBG_CAT_INIT    1 /* initialization/shutdown */
#define DBG_CAT_HW      2 /* MAC/BBP/RF/Chip */
#define DBG_CAT_FW      3 /* FW related command, response, CR that FW care about */
#define DBG_CAT_HIF     4 /* Host interface: usb/sdio/pcie/rbus */
#define DBG_CAT_FPGA    5 /* FPGA Chip verify, DVT */
#define DBG_CAT_TEST    6 /* ATE, QA, UT, FPGA?, TDT, SLT, WHQL, and other TEST */
#define DBG_CAT_RA      7 /* Rate Adaption/Throughput related */
#define DBG_CAT_AP      8 /* AP, MBSS, WDS */
#define DBG_CAT_CLIENT  9 /* STA, ApClient, AdHoc, Mesh */
#define DBG_CAT_TX      10 /* Tx data path */
#define DBG_CAT_RX      11 /* Rx data path */
#define DBG_CAT_CFG     12 /* ioctl/oid/profile/cfg80211/Registry */
#define DBG_CAT_MLME    13 /* 802.11 fundamental connection flow, auth, assoc, disconnect, etc */
#define DBG_CAT_PROTO   14 /* protocol, ex. TDLS */
#define DBG_CAT_SEC     15 /* security/key/WPS/WAPI/PMF/11i related*/
#define DBG_CAT_PS      16 /* power saving/UAPSD */
#define DBG_CAT_POWER   17 /* power Setting, Single Sku, Temperature comp, etc */
#define DBG_CAT_COEX    18 /* BT, BT WiFi Coex, LTE, TVWS*/
#define DBG_CAT_P2P     19 /* P2P, Miracast */
#define DBG_CAT_TOKEN	20
#define DBG_CAT_CMW     21 /* CMW Link Test related */
#define DBG_CAT_RSV1    30 /* reserved index for code development */
#define DBG_CAT_RSV2    31 /* reserved index for code development */
#define DBG_CAT_MAX     31
#define DBG_CAT_ALL     DBG_CAT_MISC
#define DBG_CAT_EN_ALL_MASK 0xFFFFFFFFu
#endif

/* Debug SubCategory */
/* if change the definition of below subcategory or update new subcategory, please update cat_str and sub_cat_str in cmm_info.c */

#define DBG_SUBCAT_ALL	DBG_SUBCAT_MISC

#define DBG_SUBCAT_EN_ALL_MASK 0xFFFFFFFFu

#define DBG_SUBCAT_MISC    0x00000001u /* misc for all category */

/* Sub-Category of  DBG_CAT_HW */
#define CATHW_SA		0x00000002u	/* debug flag for smart antenna */

/* Sub-Category of  DBG_CAT_HIF */
#define CATHIF_PCI		0x00000002u
#define CATHIF_USB		0x00000004u
#define CATHIF_SDIO		0x00000008u

/* Sub-Category of  DBG_CAT_AP */
#define CATAP_MBSS		0x00000002u
#define CATAP_WDS		0x00000004u

/* Sub-Category of  DBG_CAT_CLIENT */
#define CATCLIENT_ADHOC	0x00000002u
#define CATCLIENT_APCLI	0x00000004u
#define CATCLIENT_MESH	0x00000008u

/* Sub-Category of  DBG_CAT_TX */
#define CATTX_TMAC		0x00000002u	/* debug flag for tmac info dump */

/*  Sub-Category of DBG_CAT_TOKEN */
#define TOKEN_INFO		0x00000002u
#define TOKEN_PROFILE	0x00000004u
#define TOKEN_TRACE		0x00000008u

/* Sub-Category of  DBG_CAT_PROTO */
#define CATPROTO_ACM	0x00000002u
#define CATPROTO_BA	0x00000004u
#define CATPROTO_TDLS	0x00000008u
#define CATPROTO_WNM	0x00000010u
#define CATPROTO_IGMP	0x00000020u
#define CATPROTO_MAT	0x00000040u
#define CATPROTO_RRM	0x00000080u
#define CATPROTO_DFS	0x00000100u
#define CATPROTO_FT	0x00000200u
#define CATPROTO_SCAN	0x00000400u
#define CATPROTO_FTM    0x00000800u

/* Sub-Category of  DBG_CAT_SEC */
#define CATSEC_KEY	    0x00000002u
#define CATSEC_WPS	    0x00000004u
#define CATSEC_WAPI	    0x00000008u
#define CATSEC_PMF	    0x00000010u
#define CATSEC_SAE	    0x00000020u
#define CATSEC_SUITEB	    0x00000040u
#define CATSEC_OWE	    0x00000080u
#define CATSEC_ECC	    0x00000100u


/* Sub-Category of  DBG_CAT_PS */
#define CATPS_UAPSD		0x00000002u


/***********************************************************************************
 *	Debugging and printing related definitions and prototypes
 ***********************************************************************************/
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#ifdef DBG
extern int			DebugLevel;
extern UINT32		DebugCategory;
extern UINT32		DebugSubCategory[DBG_LVL_MAX + 1][32];


#define MTWF_LOG(Category, SubCategory, Level, Fmt)	\
	do {	\
		if ((0x1 << Category) & (DebugCategory))	\
			if ((SubCategory) & (DebugSubCategory[Level][Category])) \
				MTWF_PRINT Fmt; \
	} while (0)

#else
#define MTWF_LOG(Category, SubCategory, Level, Fmt)
#endif

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump_with_lvl(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen, int dbglvl);
#endif /* __DEBUG_H__ */

