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
	wf_ple.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_PLE_H__
#define __WF_PLE_H__

#define PLE_BASE		0x8000


/* PLE Buffer Control Register */
#define PLE_PBUF_CTRL				(PLE_BASE + 0x14)
#define PLE_TOTAL_PAGE_NUM_MASK	(0xfff)
#define PLE_GET_TOTAL_PAGE_NUM(p) (((p) & 0xfff))
#define PLE_TOTAL_PAGE_CFG_MASK	(0xf << 16)
#define PLE_GET_TOTAL_PAGE_CFG(p) (((p) & PSE_TOTAL_PAGE_CFG_MASK) >> 16)
#define PLE_PBUF_OFFSET_MASK		(0xf << 20)
#define GET_PLE_PBUF_OFFSET(p)	(((p) & PSE_PBUF_OFFSET_MASK) >> 20)


/* Release Control */
#define PLE_RELEASE_CTRL				(PLE_BASE + 0x30)
#define PLE_NORMAL_TX_RLS_QID_MASK	(0x1f)
#define GET_PLE_NORMAL_TX_RLS_QID(p)	(((p) & GET_FIRST_FID_MASK))
#define PLE_NORMAL_TX_RLS_PID_MASK	(0x3 << 6)
#define GET_PLE_NORMAL_TX_RLS_PID(p) (((p) & PLE_NORMAL_TX_RLS_PID_MASK) >> 6)

#define BCNx_RLS_QID_MASK    (0x1f)
#define BCNx_RLS_PID_MASK    (0x3)
#define SET_BCN0_RLS_QID(p)  (((p) & BCNx_RLS_QID_MASK) << 16)
#define SET_BCN0_RLS_PID(p)  (((p) & BCNx_RLS_PID_MASK) << 22)
#define SET_BCN1_RLS_QID(p)  (((p) & BCNx_RLS_QID_MASK) << 24)
#define SET_BCN1_RLS_PID(p)  (((p) & BCNx_RLS_PID_MASK) << 30)

/* HIF Report Control */
#define PLE_HIF_REPORT				(PLE_BASE + 0x34)

/* CPU Interface get frame ID Control */
#define PLE_C_GET_FID_0				(PLE_BASE + 0x40)

/* CPU Interface get frame ID Control */
#define PLE_C_GET_FID_1				(PLE_BASE + 0x44)

#define PLE_C_EN_QUEUE_0                (PLE_BASE + 0x60)
#define PLE_C_EN_QUEUE_1                (PLE_BASE + 0x64)
#define PLE_C_EN_QUEUE_2                (PLE_BASE + 0x68)

#define PLE_C_DE_QUEUE_0                (PLE_BASE + 0x80)
#define PLE_C_DE_QUEUE_1                (PLE_BASE + 0x84)
#define PLE_C_DE_QUEUE_2                (PLE_BASE + 0x88)
#define PLE_C_DE_QUEUE_3                (PLE_BASE + 0x8c)

#define PLE_TO_N9_INT                   (PLE_BASE + 0xf0)
#define PLE_TO_N9_INT_TOGGLE_MASK 0x80000000



/* Queue Empty */
#define PLE_QUEUE_EMPTY			(PLE_BASE + 0xb0)
#define PLE_AC0_QUEUE_EMPTY_0		(PLE_BASE + 0x300)
#define PLE_AC0_QUEUE_EMPTY_1		(PLE_BASE + 0x304)
#define PLE_AC0_QUEUE_EMPTY_2		(PLE_BASE + 0x308)
#define PLE_AC0_QUEUE_EMPTY_3		(PLE_BASE + 0x30c)

#define PLE_AC1_QUEUE_EMPTY_0		(PLE_BASE + 0x310)
#define PLE_AC1_QUEUE_EMPTY_1		(PLE_BASE + 0x314)
#define PLE_AC1_QUEUE_EMPTY_2		(PLE_BASE + 0x318)
#define PLE_AC1_QUEUE_EMPTY_3		(PLE_BASE + 0x31c)

#define PLE_AC2_QUEUE_EMPTY_0		(PLE_BASE + 0x320)
#define PLE_AC2_QUEUE_EMPTY_1		(PLE_BASE + 0x324)
#define PLE_AC2_QUEUE_EMPTY_2		(PLE_BASE + 0x328)
#define PLE_AC2_QUEUE_EMPTY_3		(PLE_BASE + 0x32c)

#define PLE_AC3_QUEUE_EMPTY_0		(PLE_BASE + 0x330)
#define PLE_AC3_QUEUE_EMPTY_1		(PLE_BASE + 0x334)
#define PLE_AC3_QUEUE_EMPTY_2		(PLE_BASE + 0x338)
#define PLE_AC3_QUEUE_EMPTY_3		(PLE_BASE + 0x33c)

#define PLE_STATION_PAUSE0		    (PLE_BASE + 0x360)
#define PLE_STATION_PAUSE1		    (PLE_BASE + 0x364)
#define PLE_STATION_PAUSE2		    (PLE_BASE + 0x368)
#define PLE_STATION_PAUSE3		    (PLE_BASE + 0x36C)

/* Page Flow Control */
#define PLE_FREEPG_CNT				(PLE_BASE + 0x100)

#define PLE_FREEPG_HEAD_TAIL		(PLE_BASE + 0x104)

#define PLE_PG_HIF_GROUP			(PLE_BASE + 0x110)
#define PLE_HIF_PG_INFO				(PLE_BASE + 0x114)

#define PLE_PG_CPU_GROUP			(PLE_BASE + 0x150)
#define PLE_CPU_PG_INFO			(PLE_BASE + 0x154)


/* Indirect path for read/write */
#define PLE_FL_QUE_CTRL_0			(PLE_BASE + 0x1b0)
#define PLE_FL_QUE_CTRL_1			(PLE_BASE + 0x1b4)
#define PLE_FL_QUE_CTRL_2			(PLE_BASE + 0x1b8)
#define PLE_FL_QUE_CTRL_3			(PLE_BASE + 0x1bc)
#define PLE_PL_QUE_CTRL_0			(PLE_BASE + 0x1c0)

/* Disable Station control */
#define DIS_STA_MAP0			(PLE_BASE + 0x260)
#define DIS_STA_MAP1			(PLE_BASE + 0x264)
#define DIS_STA_MAP2			(PLE_BASE + 0x268)
#define DIS_STA_MAP3			(PLE_BASE + 0x26c)

/* Station Pause control register */
#define STATION_PAUSE0			(PLE_BASE + 0x360)
#define STATION_PAUSE1			(PLE_BASE + 0x364)
#define STATION_PAUSE2			(PLE_BASE + 0x368)
#define STATION_PAUSE3			(PLE_BASE + 0x36c)

/* VOW Ctrl */
#define VOW_RESET_DISABLE       (1 << 26)
#define STA_MAX_DEFICIT_MASK    (0x0000FFFF)
#define VOW_DBDC_BW_GROUP_CTRL  (PLE_BASE + 0x2ec)
#define VOW_CONTROL             (PLE_BASE + 0x370)
#define AIRTIME_DRR_SIZE        (PLE_BASE + 0x374)


typedef enum _ENUM_UMAC_PORT_T {
	ENUM_UMAC_HIF_PORT_0         = 0,
	ENUM_UMAC_CPU_PORT_1         = 1,
	ENUM_UMAC_LMAC_PORT_2        = 2,
	ENUM_PLE_CTRL_PSE_PORT_3     = 3,
	ENUM_UMAC_PSE_PLE_PORT_TOTAL_NUM = 4
} ENUM_UMAC_PORT_T, *P_ENUM_UMAC_PORT_T;

/* N9 MCU QUEUE LIST */
typedef enum _ENUM_UMAC_CPU_P_QUEUE_T {
	ENUM_UMAC_CTX_Q_0 = 0,
	ENUM_UMAC_CTX_Q_1 = 1,
	ENUM_UMAC_CTX_Q_2 = 2,
	ENUM_UMAC_CTX_Q_3 = 3,
	ENUM_UMAC_CRX     = 0,
	ENUM_UMAC_CIF_QUEUE_TOTAL_NUM = 4
} ENUM_UMAC_CPU_P_QUEUE_T, *P_ENUM_UMAC_CPU_P_QUEUE_T;


/* LMAC PLE TX QUEUE LIST */
typedef enum _ENUM_UMAC_LMAC_PLE_TX_P_QUEUE_T {
	ENUM_UMAC_LMAC_PLE_TX_Q_00           = 0x00,
	ENUM_UMAC_LMAC_PLE_TX_Q_01           = 0x01,
	ENUM_UMAC_LMAC_PLE_TX_Q_02           = 0x02,
	ENUM_UMAC_LMAC_PLE_TX_Q_03           = 0x03,

	ENUM_UMAC_LMAC_PLE_TX_Q_10           = 0x04,
	ENUM_UMAC_LMAC_PLE_TX_Q_11           = 0x05,
	ENUM_UMAC_LMAC_PLE_TX_Q_12           = 0x06,
	ENUM_UMAC_LMAC_PLE_TX_Q_13           = 0x07,

	ENUM_UMAC_LMAC_PLE_TX_Q_20           = 0x08,
	ENUM_UMAC_LMAC_PLE_TX_Q_21           = 0x09,
	ENUM_UMAC_LMAC_PLE_TX_Q_22           = 0x0a,
	ENUM_UMAC_LMAC_PLE_TX_Q_23           = 0x0b,

	ENUM_UMAC_LMAC_PLE_TX_Q_30           = 0x0c,
	ENUM_UMAC_LMAC_PLE_TX_Q_31           = 0x0d,
	ENUM_UMAC_LMAC_PLE_TX_Q_32           = 0x0e,
	ENUM_UMAC_LMAC_PLE_TX_Q_33           = 0x0f,

	ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0      = 0x10,
	ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0       = 0x11,
	ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0       = 0x12,
	ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0      = 0x13,

	ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1      = 0x14,
	ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1       = 0x15,
	ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1       = 0x16,
	ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1      = 0x17,
	ENUM_UMAC_LMAC_PLE_TX_Q_NAF         = 0x18,
	ENUM_UMAC_LMAC_PLE_TX_Q_NBCN        = 0x19,
	ENUM_UMAC_LMAC_PLE_TX_Q_RELEASE     = 0x1f, /* DE suggests not to use 0x1f, it's only for hw free queue */
	ENUM_UMAC_LMAC_QUEUE_TOTAL_NUM      = 24,

} ENUM_UMAC_LMAC_TX_P_QUEUE_T, *P_ENUM_UMAC_LMAC_TX_P_QUEUE_T;

/* LMAC PLE For PSE Control P3 */
typedef enum _ENUM_UMAC_PLE_CTRL_P3_QUEUE_T {
	ENUM_UMAC_PLE_CTRL_P3_Q_0X1E            = 0x1e,
	ENUM_UMAC_PLE_CTRL_P3_Q_0X1F            = 0x1f,
	ENUM_UMAC_PLE_CTRL_P3_TOTAL_NUM         = 2
} ENUM_UMAC_PLE_CTRL_P3_QUEUE_T, *P_ENUM_UMAC_PLE_CTRL_P3_QUEUE_T;



typedef struct _EMPTY_QUEUE_INFO_T {
	PCHAR QueueName;
	UINT32 Portid;
	UINT32 Queueid;
} EMPTY_QUEUE_INFO_T, *P_EMPTY_QUEUE_INFO_T;


#endif /* __WF_PLE_H__ */

