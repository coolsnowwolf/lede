/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wf_mu.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_MU_H__
#define __WF_MU_H__


#define WF_MU_BASE		0x25000

#define MU_MUCR0			(WF_MU_BASE + 0x0000)	/* 0x25000 */
#define MUCR0_MU_ENA_BIT    (0)
#define MUCR0_MU_ENA_MASK    (0x1<<0)
#define MUCR0_MU_DBDC_OFFSET_BIT    (28)
#define MUCR0_MU_DBDC_OFFSET_MASK   (0x3<<28)

#define MU_MUCR1			(WF_MU_BASE + 0x0004)	/* 0x25004 */
#define MUCR1_MU_MAX_GROUP_SEARCH_BIT 0
#define MUCR1_MU_MAX_GROUP_SEARCH_MASK (0x7ff << 0)
#define MUCR1_PFID_PE_VALID_TH_BIT 12
#define MUCR1_PFID_PE_VALID_TH_MASK (0xf << 12)
#define MUCR1_PFID_PE_VALID_ENA_BIT 16
#define MUCR1_PFID_PE_VALID_ENA_MASK (0x1<<16)
#define MUCR1_PFID_TX_COUNT_REMAP_CTRL_BIT  20
#define MUCR1_PFID_TX_COUNT_REMAP_CTRL_MASK  (0x1<<20)
#define MUCR1_CLUSTER_TAB_REMAP_CTRL_BIT  21
#define MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK  (0x1<<21)
#define MUCR1_GPID_RATE_PER_TAB_CTRL_BIT  22
#define MUCR1_GPID_RATE_PER_TAB_CTRL_MASK  (0x1<<21)
#define MUCR1_GRP_TAB_DMCS_WMASK_BIT  24
#define MUCR1_GRP_TAB_DMCS_WMASK_MASK  (0x1<<24)
#define MUCR1_GRP_TAB_REMAP_CTRL_BIT  28
#define MUCR1_GRP_TAB_REMAP_CTRL_MASK  (0x7<<28)


#define MU_MUCR2			(WF_MU_BASE + 0x0008)	/* 0x25008 */
#define MU_MUCR3			(WF_MU_BASE + 0x000c)	/* 0x2500c */
#define MU_MUCR4			(WF_MU_BASE + 0x0010)	/* 0x25010 */
#define MUCR4_AC_LEN_TAB_CLR_BIT        (0)
#define MUCR4_AC_LEN_TAB_CLR_MASK       (1<<0)

#define MU_PFID_COUNT_TABLE         (WF_MU_BASE + 0x0100)	/* 0x25100 ~ 0x253FF */
#define MU_PFID_COUNT_TABLE_LEN 0x300

#define MU_CLUSTER_TABLE_BASE   (WF_MU_BASE + 0x0400)	/* 0x25400 ~ 0x255FF */
#define MU_CLUSTER_TABLE_LEN        0x200

typedef struct _MU_CLID_ENTRY {
	UINT8 GidMembership[8];
	UINT8 GidUserPosition[16];
} MU_CLID_ENTRY;

#define MU_GPID_RATE_PER_TABLE_BASE (WF_MU_BASE + 0x0680)	/* 0x25680 ~ 0x256FF */
#define MU_GPID_RATE_PER_TABLE_LEN        0x80

#define MU_PROFILE_TABLE_BASE    (WF_MU_BASE + 0x0780)	/* 0x25780 ~ 0x257FF */
#define MU_PROFILE_TABLE_LEN            0x80
typedef struct _MU_PROFILE_ID_ENTRY {
#ifdef RT_BIG_ENDIAN
	UINT32 Rsv_17_31:15;
	UINT32 BA_Status_AC3:1;
	UINT32 BA_Status_AC2:1;
	UINT32 BA_Status_AC1:1;
	UINT32 BA_Status_AC0:1;
	UINT32 MuCluster:5;
	UINT32 WlanId:7;
	UINT32 Vld:1;
#else
	UINT32 Vld:1;
	UINT32 WlanId:7;
	UINT32 MuCluster:5;
	UINT32 BA_Status_AC0:1;
	UINT32 BA_Status_AC1:1;
	UINT32 BA_Status_AC2:1;
	UINT32 BA_Status_AC3:1;
	UINT32 Rsv_17_31:15;
#endif /* RT_BIG_ENDIAN */
} MU_PROFILE_ID_ENTRY;


#define MU_GRP_TABLE_RATE_MAP   (WF_MU_BASE + 0x1000)	/* 0x26000 ~ 0x26FFF */
#define MU_GRP_TABLE_RATE_MAP_LEN

#define MU_GRP_TABLE_STAT_MAP   (WF_MU_BASE + 0x1000)	/* 0x26000 */
#endif /* __WF_MU_H__ */


