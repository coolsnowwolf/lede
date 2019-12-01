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
	pse.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __PSE_H__
#define __PSE_H__

#define PSE_BASE		0xc000

/* PSE Reset Control Register */
#define PSE_RESET (PSE_BASE + 0x00)
#define PSE_ALL_RST (1 << 0)
#define PSE_LOGICAL_RST (1 << 1)
#define PSE_INIT_DONE (1 << 2)
#define GET_PSE_INIT_DONE(p) (((p) & PSE_INIT_DONE) >> 2)

/* PSE Buffer Control Register */
#define PSE_PBUF_CTRL (PSE_BASE + 0x14)
#define PSE_TOTAL_PAGE_NUM_MASK (0xfff)
#define PSE_GET_TOTAL_PAGE_NUM(p) (((p) & 0xfff))
#define PSE_TOTAL_PAGE_CFG_MASK (0xf << 16)
#define PSE_GET_TOTAL_PAGE_CFG(p) (((p) & PSE_TOTAL_PAGE_CFG_MASK) >> 16)

#define PSE_PBUF_OFFSET_MASK (0xf << 20)
#define GET_PSE_PBUF_OFFSET(p) (((p) & PSE_PBUF_OFFSET_MASK) >> 20)

/* Queue Empty */
#define PSE_QUEUE_EMPTY			(PSE_BASE + 0xb0)

/* Page Flow Control */
#define PSE_FREEPG_CNT				(PSE_BASE + 0x100)

#define PSE_FREEPG_HEAD_TAIL		(PSE_BASE + 0x104)

#define PSE_PG_HIF0_GROUP			(PSE_BASE + 0x110)
#define PSE_HIF0_PG_INFO			(PSE_BASE + 0x114)
#define PSE_PG_HIF1_GROUP			(PSE_BASE + 0x118)
#define PSE_HIF1_PG_INFO			(PSE_BASE + 0x11c)

#define PSE_PG_CPU_GROUP			(PSE_BASE + 0x150)
#define PSE_CPU_PG_INFO			    (PSE_BASE + 0x154)

#define PSE_PG_LMAC0_GROUP			(PSE_BASE + 0x170)
#define PSE_LMAC0_PG_INFO			(PSE_BASE + 0x174)
#define PSE_PG_LMAC1_GROUP			(PSE_BASE + 0x178)
#define PSE_LMAC1_PG_INFO			(PSE_BASE + 0x17c)
#define PSE_PG_LMAC2_GROUP			(PSE_BASE + 0x180)
#define PSE_LMAC2_PG_INFO			(PSE_BASE + 0x184)
#define PSE_PG_PLE_GROUP			(PSE_BASE + 0x190)
#define PSE_PLE_PG_INFO			    (PSE_BASE + 0x194)

/* Indirect path for read/write */
#define PSE_FL_QUE_CTRL_0			(PSE_BASE + 0x1b0)
#define PSE_FL_QUE_CTRL_1			(PSE_BASE + 0x1b4)
#define PSE_FL_QUE_CTRL_2			(PSE_BASE + 0x1b8)
#define PSE_FL_QUE_CTRL_3			(PSE_BASE + 0x1bc)
#define PSE_PL_QUE_CTRL_0			(PSE_BASE + 0x1c0)

/* PSE spare dummy CR */
#define PSE_SPARE_DUMMY_CR1			(PSE_BASE +  0x1e4)
#define PSE_SPARE_DUMMY_CR2			(PSE_BASE +  0x1e8)
#define PSE_SPARE_DUMMY_CR3			(PSE_BASE +  0x2e8)
#define PSE_SPARE_DUMMY_CR4			(PSE_BASE +  0x2ec)

/* CPU Interface Get First Frame ID Control Regitser */
#define C_GFF (PSE_BASE + 0x24)
#define GET_FIRST_FID_MASK (0xfff)
#define GET_FIRST_FID(p) (((p) & GET_FIRST_FID_MASK))
#define GET_QID_MASK (0x1f << 16)
#define SET_GET_QID(p) (((p) & 0x1f) << 16)
#define GET_QID(p) (((p) & GET_QID_MASK) >> 16)
#define GET_PID_MASK (0x3 << 21)
#define SET_GET_PID(p) (((p) & 0x3) << 21)
#define GET_PID(p) (((p) & GET_PID_MASK) >> 21)
#define GET_RVSE_MASK (1 << 23)
#define SET_GET_RVSE(p) (((p) & 0x1) << 23)
#define GET_RVSE(p) (((p) & GET_RVSE_MASK) >> 23)

/* CPU Interface Get Frame ID Control Register */
#define C_GF (PSE_BASE + 0x28)
#define GET_RETURN_FID_MASK (0xfff)
#define GET_RETURN_FID(p) (((p) & GET_RETURN_FID_MASK))
#define CURR_FID_MASK (0xfff << 16)
#define SET_CURR_FID(p) (((p) & 0xfff) << 16)
#define GET_CURR_FID(p) (((p) & CURR_FID_MASK) >> 16)
#define GET_PREV_MASK (1 << 28)
#define SET_GET_PREV(p) (((p) & 0x1) << 28)
#define GET_PREV(p) (((p) & GET_PREV_MASK) >> 28)
#define GET_AUTO_MASK (1 << 29)
#define SET_GET_AUTO(p) (((p) & 0x1) >> 29)
#define GET_AUTO(p) (((p) & GET_AUTO_MASK) >> 29)

/* Get Queue Length Control Register */
#define GQC (PSE_BASE + 0x118)
#define QLEN_RETURN_VALUE_MASK (0xfff)
#define GET_QLEN_RETURN_VALUE(p) (((p) & QLEN_RETURN_VALUE_MASK))
#define GET_QLEN_QID_MASK (0x1f << 16)
#define SET_GET_QLEN_QID(p) (((p) & 0x1f) << 16)
#define GET_QLEN_QID(p) (((p) & GET_QLEN_QID_MASK) >> 16)
#define GET_QLEN_PID_MASK (0x3 << 21)
#define SET_GET_QLEN_PID(p) (((p) & 0x3) << 21)
#define GET_QLEN_PID(p) (((p) & GET_QLEN_PID_MASK) >> 21)
#define QLEN_IN_PAGE (1 << 23)
#define GET_QLEN_IN_PAGE(p) (((p) & QLEN_IN_PAGE) >> 23)

/* Flow Control P0 Control Register */
#define FC_P0 (PSE_BASE + 0x120)
#define MIN_RSRV_P0_MASK (0xfff)
#define MIN_RSRV_P0(p) (((p) & 0xfff))
#define GET_MIN_RSRV_P0(p) (((p) & MIN_RSRV_P0_MASK))
#define MAX_QUOTA_P0_MASK (0xfff << 16)
#define MAX_QUOTA_P0(p) (((p) & 0xfff) << 16)
#define GET_MAX_QUOTA_P0(p) (((p) & MAX_QUOTA_P0_MASK) >> 16)

/* Flow Control P1 Control Register */
#define FC_P1 (PSE_BASE + 0x124)
#define MIN_RSRV_P1_MASK (0xfff)
#define MIN_RSRV_P1(p) (((p) & 0xfff))
#define GET_MIN_RSRV_P1(p) (((p) & MIN_RSRV_P1_MASK))
#define MAX_QUOTA_P1_MASK (0xfff << 16)
#define MAX_QUOTA_P1(p) (((p) & 0xfff) << 16)
#define GET_MAX_QUOTA_P1(p) (((p) & MAX_QUOTA_P1_MASK) >> 16)

/* Flow Control P2_RQ0 Control Register */
#define FC_P2Q0 (PSE_BASE + 0x128)
#define MIN_RSRV_P2_RQ0_MASK (0xfff)
#define MIN_RSRV_P2_RQ0(p) (((p) & 0xfff))
#define GET_MIN_RSRV_P2_RQ0(p) (((p) & MIN_RSRV_P2_RQ0_MASK))
#define MAX_QUOTA_P2_RQ0_MASK (0xfff << 16)
#define MAX_QUOTA_P2_RQ0(p) (((p) & 0xfff) << 16)
#define GET_MAX_QUOTA_P2_RQ0(p) (((p) & MAX_QUOTA_P2_RQ0_MASK) >> 16)

/* Flow Control P2_RQ1 Control Register */
#define FC_P2Q1 (PSE_BASE + 0x12c)
#define MIN_RSRV_P2_RQ1_MASK (0xfff)
#define MIN_RSRV_P2_RQ1(p) (((p) & 0xfff))
#define GET_MIN_RSRV_P2_RQ1(p) (((p) & MIN_RSRV_P2_RQ1_MASK))
#define MAX_QUOTA_P2_RQ1_MASK (0xfff << 16)
#define MAX_QUOTA_P2_RQ1(p) (((p) & 0xfff) << 16)
#define GET_MAX_QUOTA_P2_RQ1(p) (((p) & MAX_QUOTA_P2_RQ1_MASK) >> 16)

/* Flow Control P2_RQ2 Control Register */
#define FC_P2Q2 (PSE_BASE + 0x130)
#define MIN_RSRV_P2_RQ2_MASK (0xfff)
#define MIN_RSRV_P2_RQ2(p) (((p) & 0xfff))
#define GET_MIN_RSRV_P2_RQ2(p) (((p) & MIN_RSRV_P2_RQ2_MASK))
#define MAX_QUOTA_P2_RQ2_MASK (0xfff << 16)
#define MAX_QUOTA_P2_RQ2(p) (((p) & 0xfff) << 16)
#define GET_MAX_QUOTA_P2_RQ2(p) (((p) & MAX_QUOTA_P2_RQ2_MASK) >> 16)

/* Flow Control Free for All and Free Page Counter Register */
#define FC_FFC (PSE_BASE + 0x134)
#define FREE_PAGE_CNT_MASK (0xfff)
#define GET_FREE_PAGE_CNT(p) (((p) & FREE_PAGE_CNT_MASK))
#define FFA_CNT_MASK (0xfff << 16)
#define GET_FFA_CNT(p) (((p) & FFA_CNT_MASK) >> 16)

/* Flow Control Reserve from FFA priority Control Register */
#define FC_FRP (PSE_BASE + 0x138)
#define RSRV_PRI_P0_MASK (0x7)
#define RSRV_PRI_P0(p) (((p) & 0x7))
#define GET_RSRV_PRI_P0(p) (((p) & RSRV_PRI_P0_MASK))
#define RSRV_PRI_P1_MASK (0x7 << 3)
#define RSRV_PRI_P1(p) (((p) & 0x7) << 3)
#define GET_RSRV_PRI_P1(p) (((p) & RSRV_PRI_P1_MASK) >> 3)
#define RSRV_PRI_P2_RQ0_MASK (0x7 << 6)
#define RSRV_PRI_P2_RQ0(p) (((p) & 0x7) << 6)
#define GET_RSRV_PRI_P2_RQ0(p) (((p) & RSRV_PRI_P2_RQ0_MASK) >> 6)
#define RSRV_PRI_P2_RQ1_MASK (0x7 << 9)
#define RSRV_PRI_P2_RQ1(p) (((p) & 0x7) << 9)
#define GET_RSRV_PRI_P2_RQ1(p) (((p) & RSRV_PRI_P2_RQ1_MASK) >> 9)
#define RSRV_PRI_P2_RQ2_MASK (0x7 << 12)
#define RSRV_PRI_P2_RQ2(p) (((p) & 0x7) << 12)
#define GET_RSRV_PRI_P2_RQ2(p) (((p) & RSRV_PRI_P2_RQ2_MASK) >> 12)

/* Flow Control P0 and P1 Reserve Counter Register */
#define FC_RP0P1 (PSE_BASE + 0x13c)
#define RSRV_CNT_P0_MASK (0xfff)
#define RSRV_CNT_P0(p) (((p) & 0xfff))
#define GET_RSRV_CNT_P0(p) (((p) & RSRV_CNT_P0_MASK))
#define RSRV_CNT_P1_MASK (0xfff << 16)
#define RSRV_CNT_P1(p) (((p) & 0xfff) << 16)
#define GET_RSRV_CNT_P1(p) (((p) & RSRV_CNT_P1_MASK) >> 16)

/* Flow Control P2_RQ0 and P2_RQ1 Reserve Counter Register */
#define FC_RP2Q0Q1 (PSE_BASE + 0x140)
#define RSRV_CNT_P2_RQ0_MASK (0xfff)
#define RSRV_CNT_P2_RQ0(p) (((p) & 0xfff))
#define GET_RSRV_CNT_P2_RQ0(p) (((p) & RSRV_CNT_P2_RQ0_MASK))
#define RSRV_CNT_P2_RQ1_MASK (0xfff << 16)
#define RSRV_CNT_P2_RQ1(p) (((p) & 0xfff) << 16)
#define GET_RSRV_CNT_P2_RQ1(p) (((p) & RSRV_CNT_P2_RQ1_MASK) >> 16)

/* Flow Control P2_RQ2 Reserve Counter Register */
#define FC_RP2Q2 (PSE_BASE + 0x144)
#define RSRV_CNT_P2_RQ2_MASK (0xfff)
#define RSRV_CNT_P2_RQ2(p) (((p) & 0xfff))
#define GET_RSRV_CNT_P2_RQ2(p) (((p) & RSRV_CNT_P2_RQ2_MASK))

/* Flow Control P0 and P1 Source Counter Register */
#define FC_SP0P1 (PSE_BASE + 0x148)
#define SRC_CNT_P0_MASK (0xfff)
#define GET_SRC_CNT_P0(p) (((p) & SRC_CNT_P0_MASK))
#define SRC_CNT_P1_MASK (0xfff << 16)
#define GET_SRC_CNT_P1(p) (((p) & SRC_CNT_P1_MASK) >> 16)

/* FLow Control P2_RQ0 and P2_RQ1 Source Counter Register */
#define FC_SP2Q0Q1 (PSE_BASE + 0x14c)
#define SRC_CNT_P2_RQ0_MASK (0xfff)
#define GET_SRC_CNT_P2_RQ0(p) (((p) & SRC_CNT_P2_RQ0_MASK))
#define SRC_CNT_P2_RQ1_MASK (0xfff << 16)
#define GET_SRC_CNT_P2_RQ1(p) (((p) & SRC_CNT_P2_RQ1_MASK) >> 16)

/* Flow Control P2_RQ2 Source Counter Register */
#define FC_SP2Q2 (PSE_BASE + 0x150)
#define SRC_CNT_P2_RQ2_MASK (0xfff)
#define GET_SRC_CNT_P2_RQ2(p) (((p) & 0xfff))

#define PSE_RTA (PSE_BASE + 0x194)
#define PSE_RTA_RD_RULE_QID_MASK (0x1f)
#define PSE_RTA_RD_RULE_QID(p) (((p) & 0x1f))
#define GET_PSE_RTA_RD_RULE_QID(p) (((p) & PSE_RTA_RD_RULE_QID_MASK))
#define PSE_RTA_RD_RULE_PID_MASK (0x3 << 5)
#define PSE_RTA_RD_RULE_PID(p) (((p) & 0x3) << 5)
#define GET_PSE_RTA_RD_RULE_PID(p) (((p) & PSE_RTA_RD_RULE_PID_MASK) >> 5)
#define PSE_RTA_RD_RULE_F (1 << 7)
#define GET_PSE_RTA_RD_RULE_F(p) (((p) & PSE_RTA_RD_RULE_F) >> 7)
#define PSE_RTA_TAG_MASK (0xff << 8)
#define PSE_RTA_TAG(p) (((p) & 0xff) << 8)
#define GET_PSE_RTA_TAG(p) (((p) & PSE_RTA_TAG_MASK) >> 8)
#define PSE_RTA_RD_RW (1 << 16)
#define PSE_RTA_RD_KICK_BUSY (1 << 31)
#define GET_PSE_RTA_RD_KICK_BUSY(p) (((p) & PSE_RTA_RD_KICK_BUSY) >> 31)

#endif /* _PSE */

