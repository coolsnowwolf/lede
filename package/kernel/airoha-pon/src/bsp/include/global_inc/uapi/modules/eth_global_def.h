/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef _ETH_GLOBAL_API_H_
#define _ETH_GLOBAL_API_H_

/**
* \file  eth_global_def.h 
* \brief This file is eth global def header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#include <linux/types.h>

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/
#define RAETH_GSW_CTLAPI		0x89FB
#if defined(TCSUPPORT_SEPERATED_ETH_ITF)
#define RAETH_GSW_SETPORTMAP		0x89FC
#endif /* TCSUPPORT_SEPERATED_ETH_ITF */
/* define  */
typedef enum
{
	CMD_RSV = 0,
	// user API start from ID 0x0010
	CMD_USRAPI_BEGIN = 0x0010,
	CMD_GET_BRGLEARNINGIND,
	CMD_SET_BRGLEARNINGIND,
	CMD_GET_PORTBRGIND,
	CMD_SET_PORTBRGIND,
	CMD_GET_DISCARDUNKNOWNMACIND,
	CMD_SET_DISCARDUNKNOWNMACIND,
	CMD_GET_AGETIME,
	CMD_SET_AGETIME,
	CMD_GET_PORTMAC,
	CMD_GET_PORTSTATUS,
	CMD_GET_MIBCNT,

	CMD_SET_QUEUE_PRIORITY,
	CMD_GET_QUEUE_MAX_SIZE,
	CMD_SET_ALLOC_QUEUE_SIZE,
	CMD_GET_ALLOC_QUEUE_SIZE,
	CMD_SET_WEIGHT,
	CMD_GET_WEIGHT,
	CMD_SET_BACK_PRESSURE,
	CMD_GET_BACK_PRESSURE,
	CMD_SET_DROP_POLICY,
	CMD_GET_DROP_POLICY,
	CMD_SET_TRAFFIC_DESCRIPTOR,
	CMD_GET_TRAFFIC_DESCRIPTOR,
	CMD_CLR_TRAFFIC_DESCRIPTOR,

	CMD_SET_AUTO_DETECT,
	CMD_GET_AUTO_DETECT,
	CMD_SET_LOOPBACK_CONF,
	CMD_GET_LOOPBACK_CONF,
	CMD_GET_CONFIG_STAT,
	CMD_SET_MAX_FRAME_SIZE,
	CMD_GET_MAX_FRAME_SIZE,
	CMD_SET_DTEDCE_Ind,
	CMD_GET_DTEDCE_Ind,
	CMD_SET_PAUSE_TIME,
	CMD_GET_PAUSE_TIME,
	CMD_SET_PORT_ADMIN,
	CMD_GET_PORT_ADMIN,

	CMD_GET_PORT_CHANGEDCNT,
	CMD_GET_PORT_MACLMT,
	CMD_SET_PORT_MACLMT,
	CMD_SET_PORT_SPDMD,
	CMD_GET_PORT_PAUSE,
	CMD_SET_PORT_PAUSE,
	CMD_GET_PORT_POLEN,
	CMD_SET_PORT_POLEN,
	CMD_GET_PORT_POL,
	CMD_SET_PORT_POL,
	CMD_GET_PORT_RATELMTEN,
	CMD_SET_PORT_RATELMTEN,
	CMD_GET_PORT_RATELMT,
	CMD_SET_PORT_RATELMT,
	CMD_GET_PORT_LOOPDET,
	CMD_SET_PORT_LOOPDET,
	CMD_SET_PORT_DISLOOPED,
	CMD_GET_PORT_ACT,
	CMD_SET_PORT_ACT,
	CMD_GET_PORT_AN,
	CMD_SET_PORT_AN,
	CMD_SET_PORT_AN_RESTART,
	CMD_GET_PORT_AN_FAIL,
	CMD_GET_PORT_LINK_LOSS,
	CMD_GET_PORT_FAIL,
	CMD_GET_PORT_CONGESTION,
	CMD_SET_PORT_MAX_BIT_RATE,
	CMD_GET_PORT_MAX_BIT_RATE,
	CMD_SET_PORT_DUPLEX_MODE,
	CMD_GET_PORT_DUPLEX_MODE,
	CMD_GET_PORT_RX_BYTE,
	CMD_GET_PORT_TX_BYTE,

	CMD_GET_TX_TIMESTAMP,
	CMD_GET_RX_TIMESTAMP,
	CMD_GET_CURRTIME,
	CMD_SET_CURRTIME,
	CMD_SET_OFFSET,
	CMD_GET_PTPSTATE,
	CMD_SET_PTPSTATE,
	CMD_SET_DS_TRTCM_ENABLE,
	CMD_GET_DS_TRTCM,
	CMD_SET_DS_TRTCM,
	CMD_SET_DSP5_RATELIMIT_ENABLE,		
	CMD_SET_DSP5_RATELIMIT,	
	CMD_SET_DSP5_QOS,

	CMD_SET_VLAN_ENTRY_ENABLE,
	CMD_SET_VLAN_ENTRY_EGTAG,
	CMD_GET_VLAN_ENTRY_EGTAG,
	CMD_SET_VLAN_ENTRY_STAG,
	CMD_GET_VLAN_ENTRY_STAG,
	CMD_ADD_VLAN_PORT_STAG,
	CMD_SET_VLAN_ENTRY_ETAGMODE,

	CMD_SET_STORM_CTRL_PORT,	
	CMD_SET_STORM_CTRL_UNIT,
	CMD_SET_STORM_CTRL_TYPE,	
	CMD_SET_STORM_CTRL_MODE,
	CMD_SET_STORM_CTRL_PERIOD,
    CMD_GET_PORT_UP_DOWN_STATE,
    CMD_SET_PORT_UP_DOWN,
	CMD_GET_CHIP_TYPE,
	CMD_GET_PORT_ALL_MAC,
	CMD_GET_PORT_BY_MAC,
	CMD_ADD_VLAN_BIND_MODE,
	CMD_DEL_VLAN_BIND_MODE,
    CMD_SET_PORTSTATUS,

	CMD_SET_ACL_RULE,

	CMD_GET_PORT_RATE_LIMIT,
	CMD_SET_PORT_RATE_LIMIT,
	CMD_GET_ETHERWAN_PORT_RATE_LIMIT,
	CMD_SET_ETHERWAN_PORT_RATE_LIMIT,
	CMD_SET_PORT_QOS_ENABLE,
	CMD_SET_PORT_QOS_PARAM,
	CMD_SET_PORT_QOS_MODE,
	CMD_SET_VLAN_ACTIVE,
    CMD_SET_MAC_TX_RX,

	CMD_SET_LOOPBACK_REMOTE,
	CMD_GET_LOOPBACK_REMOTE,
	CMD_SET_PORT_SPEEDMODE,
	CMD_GET_PORT_SPEEDMODE,
	CMD_GET_PORT_LINKSTATE,
	CMD_SET_PORT_FLOWCONTROL_GLOBAL,
	CMD_GET_PORT_FLOWCONTROL_GLOBAL,
	CMD_SET_PORT_FLOWCONTROL,
	CMD_GET_PORT_FLOWCONTROL,
	CMD_SET_LAN2LAN_CLEAR,
	CMD_SET_LAN2LAN,
	CMD_GET_LAN2LAN,
	CMD_GET_PORT_STATISTICS,
	CMD_SET_PORT_STATISTICS_CLEAR,
	CMD_SET_PORT_ISOLATE,
	CMD_SET_PORT_MATRIX,
	CMD_SET_LAN2LAN_SWITCH,
	CMD_CLR_MAC_TABLE,
	CMD_SET_PORT_CLEAN_MAC,
	CMD_GET_BR_TABLE,

	CMD_SET_RGMII_RX_CTRL_DELAY,
	CMD_SET_RGMII_RX_DATA_DELAY,
	CMD_SET_RGMII_RX_CLK_DELAY,
	CMD_SET_RGMII_TX_EN_DELAY,
	CMD_SET_RGMII_TX_DATA_DELAY,
	CMD_SET_RGMII_TX_CLK_DELAY,
	CMD_SET_RGMII_RX_CLK_EN,
	CMD_SET_RGMII_RX_CLK_ALIGN,
	CMD_SET_RGMII_RX_CLK_INV,
	CMD_SET_RGMII_TX_CLK_EN,
	CMD_SET_RGMII_TX_CLK_ALIGN,
	CMD_SET_RGMII_TX_CLK_INV,
	// internal API start from ID 0x1000
	CMD_SET_MIBCNT_EN = 0x1000,
	CMD_SET_MIBCNT_CLR,
	CMD_SET_RXOCT_MODE,
	CMD_SET_TXOCT_MODE,
	CMD_SET_BXPKT_MODE,
	// debug API start from ID 0x7F00
	CMD_APIDBGDUMP_EN = 0x7F00,
	CMD_DO_P6Cal,
	CMD_SET_DEV_TRTCM_RULE,
	
	// ether general API start from ID 0x8000
	CMD_ETH_GENERAL_API_CTL = 0x8000,
    CMD_TRAFFIC_SET,
    CMD_TRAFFIC_RGMII_MODE,
    CMD_TRAFFIC_DISPLAY,
    CMD_SET_PER_VLAN_ACTION,
    CMD_SET_PER_PORT_VLAN_ACTION,
    CMD_SET_VIP_PKT,
    CMD_WHITE_LIST_MODE,
    CMD_WHITE_LIST_PER_PORT_MODE,
    CMD_WHITE_LIST_ADD,
    CMD_WHITE_LIST_DEL,
    CMD_SET_ARL_AGING,
    CMD_SET_ARL_MACTBLADD,
    CMD_SET_ARL_ADDRTBLDISP,
    CMD_SET_ARL_ADDRTBLDISP2,
	CMD_SET_ARL_ADDRTBLCLR,
	CMD_SET_ARL_ADDRTBLCLR2,
	CMD_SET_VLAN_PVID,
	CMD_SET_VLAN_VID,
	CMD_SET_VLAN_DISP,
	CMD_SET_VLAN_PORT_ATTR,
	CMD_SET_VLAN_PORT_MODE,
	CMD_SET_VLAN_EGRESS_TAG_PVC,
	CMD_SET_VLAN_EGRESS_TAG_PCR,
	CMD_SET_VLAN_ACC_FRM,
	CMD_SET_PORT_MIRROR_ENABLE,
	CMD_SET_PORT_MIRROR_PORT_BASED,
	CMD_SET_DSTQ,
	CMD_SET_ACL_PORT_ENABLE,
	CMD_SET_ACL_MULTI_HIT,
	CMD_SET_ACL_TBL_CLR,
	CMD_SET_ACL_TBL_DISP,
	CMD_SET_ACL_TBL_ADD,
	CMD_SET_ACL_MASK_TBL_ADD,
	CMD_SET_ACL_RULE_TBL_ADD,
	CMD_SET_ACL_RATE_TBL_ADD,
	CMD_SET_QOS_BASE,
	CMD_SET_QOS_PRIO_QMAP,
	CMD_SET_QOS_PRIO_TAGMAP,
	CMD_SET_QOS_PRIO_DSCPMAP,
	CMD_SET_QOS_PORT_PRIO,
	CMD_SET_QOS_PORT_WEIGHT,
	CMD_SET_QOS_DSCP_PRIO,
	CMD_GET_GSWR,
	CMD_SET_GSWW,
	CMD_GET_GSWR2,
	CMD_SET_GSWW2,
	CMD_MIIR,
	CMD_MIIW,
	CMD_MIIR2,
	CMD_MIIW2,
    CMD_SET_GSW_SETETHPORTMAP,
    CMD_GET_PORT_JABBER,
    CMD_GET_PORT_REMOTE_FAULT,
    CMD_GET_PORT_MDI_CROSSOVER,
    CMD_GET_PORT_FLOWCONTROL_CFG,
    CMD_SET_MUL_IPTBL_ADD,
    CMD_SET_ARL_DIPTBL_CLEAN_BY_DIP,
    CMD_SET_ARL_SIPTBL_CLEAN_BY_DIP,
    CMD_SET_ARL_SIPTBL_CLEAN_BY_SIP,
    CMD_SET_ARL_SIPTBL_CLEAN_BY_DIPSIP,
} mt7530_switch_api_cmdid;

#define MT7530_ACL_RULE_NUM	(64)
#define MT7530_ACL_ACTION_NUM	(64)	// MT7530 new design 32-->64

#define MT7530_ACL_RULE_TBL_READ	(4)
#define MT7530_ACL_RULE_TBL_WRITE	(5)
#define MT7530_ACL_MASK_TBL_READ	(8)
#define MT7530_ACL_MASK_TBL_WRITE	(9)
#define MT7530_ACL_CTRL_TBL_READ	(10)
#define MT7530_ACL_CTRL_TBL_WRITE	(11)
#define MT7530_ACL_RATE_TBL_READ	(12)
#define MT7530_ACL_RATE_TBL_WRITE	(13)
#define MT7530_ACL_TRTCM_TBL_READ	(6)
#define MT7530_ACL_TRTCM_TBL_WRITE	(7)

#define MT7530_FC_BLK_UNIT_SIZE		(256 * 2)	// unit=256 bytes*512blocks, but register can set 512bytes*8_bits_register
#define MT7530_FC_BLK_UNIT_SIZE_SHRINK	(128 * 2)	// unit=128 bytes*512blocks, but register can set 256bytes*8_bits_register

typedef enum
{
    MT7530_PORT_LINK_DOWN = 0,
    MT7530_PORT_LINK_UP = 1,
}e_mt7530_port_link_state;
/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
/*
#ifndef uint32
typedef unsigned long		uint32;
//#define uint32 (unsigned int)
#endif
#ifndef uint16
typedef unsigned short		uint16;
//#define uint16 (unsigned short)
#endif
#ifndef uint8
typedef unsigned char		uint8;
//#define uint8 (unsigned char)
#endif
*/

//
// ISO C Standard C99 update define uint8_t, uint16_t, uint32_t in <stdint.h>
//   but current source code not include this stdint.h yet
/*
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
*/
typedef enum
{
    ETH_API_RSV = 0,
    HAL_API_EXT,
}eth_general_api_type;


#define MT7530_MIBCNT_VER	(2)	// new MT7530 MIB cnt based on SW_team request

typedef enum
{
#if (MT7530_MIBCNT_VER == 1)
	MIB_ID_TX_DROP_CNT = 0x4000,
	MIB_ID_TX_UCAST_CNT = 0x4004,
	MIB_ID_TX_MCAST_CNT = 0x4008,
	MIB_ID_TX_BCAST_CNT = 0x400C,
	MIB_ID_TX_COL_CNT = 0x4010,
	MIB_ID_TX_SCOL_CNT = 0x4014,
	MIB_ID_TX_MCOL_CNT = 0x4018,
	MIB_ID_TX_DEFER_CNT = 0x401C,
	MIB_ID_TX_LCOL_CNT = 0x4020,
	MIB_ID_TX_XCOL_CNT = 0x4024,
	MIB_ID_TX_PAUSE_CNT = 0x4028,
	MIB_ID_RSV1 = 0x402C,
	MIB_ID_RX_DROP_CNT = 0x4030,
	MIB_ID_RX_FILTER_CNT = 0x4034,
	MIB_ID_RX_UCAST_CNT = 0x4038,
	MIB_ID_RX_MCAST_CNT = 0x403C,
	MIB_ID_RX_BCAST_CNT = 0x4040,
	MIB_ID_RX_ALIGN_ERR_CNT = 0x4044,
	MIB_ID_RX_FCS_ERR_CNT = 0x4048,
	MIB_ID_RX_UNDERSIZE_CNT = 0x404C,
	MIB_ID_RX_FRAG_ERR_CNT = 0x4050,
	MIB_ID_RX_OVERSIZE_CNT = 0x4054,
	MIB_ID_RX_JABB_ERR_CNT = 0x4058,
	MIB_ID_RX_PAUSE_CNT = 0x405C,
	MIB_ID_BX_PKT_64_CNT = 0x4060,
	MIB_ID_BX_PKT_65TO127_CNT = 0x4064,
	MIB_ID_BX_PKT_128TO255_CNT = 0x4068,
	MIB_ID_BX_PKT_256TO511_CNT = 0x406C,
	MIB_ID_BX_PKT_512TO1023_CNT = 0x4070,
	MIB_ID_BX_PKT_1024TOMAX_CNT = 0x4074,
	MIB_ID_RSV2 = 0x4078,
	MIB_ID_RSV3 = 0x407C,
	MIB_ID_TX_OCT_CNT_L = 0x4080,	//MIB_ID_TX_OCT_CNT_H/MIB_ID_TX_OCT_CNT_L
	MIB_ID_TX_OCT_CNT_H = 0x4084,	//MIB_ID_TX_OCT_CNT_H/MIB_ID_TX_OCT_CNT_L
	MIB_ID_RX_OCT_CNT_L = 0x4088,	//MIB_ID_RX_OCT_CNT_H/MIB_ID_RX_OCT_CNT_L
	MIB_ID_RX_OCT_CNT_H = 0x408C,	//MIB_ID_RX_OCT_CNT_H/MIB_ID_RX_OCT_CNT_L
	MIB_ID_RX_CTRL_DROP_CNT = 0x4090,
	MIB_ID_RX_ING_DROP_CNT = 0x4094,
	MIB_ID_RX_ARL_DROP_CNT = 0x4098,
	MIB_ID_RSV4 = 0x409C,
#endif
#if (MT7530_MIBCNT_VER == 2)
	MIB_ID_TX_DROP_CNT = 0x4000,
	MIB_ID_TX_CRC_CNT = 0x4004,
	MIB_ID_TX_UCAST_CNT = 0x4008,
	MIB_ID_TX_MCAST_CNT = 0x400C,
	MIB_ID_TX_BCAST_CNT = 0x4010,
	MIB_ID_TX_COL_CNT = 0x4014,
	MIB_ID_TX_SCOL_CNT = 0x4018,
	MIB_ID_TX_MCOL_CNT = 0x401C,
	MIB_ID_TX_DEFER_CNT = 0x4020,
	MIB_ID_TX_LCOL_CNT = 0x4024,
	MIB_ID_TX_XCOL_CNT = 0x4028,
	MIB_ID_TX_PAUSE_CNT = 0x402C,
	MIB_ID_TX_PKT_64_CNT = 0x4030,
	MIB_ID_TX_PKT_65TO127_CNT = 0x4034,
	MIB_ID_TX_PKT_128TO255_CNT = 0x4038,
	MIB_ID_TX_PKT_256TO511_CNT = 0x403C,
	MIB_ID_TX_PKT_512TO1023_CNT = 0x4040,
	MIB_ID_TX_PKT_1024TOMAX_CNT = 0x4044,
	MIB_ID_TX_OCT_CNT_L = 0x4048,
	MIB_ID_TX_OCT_CNT_H = 0x404C,

	MIB_ID_RX_DROP_CNT = 0x4060,
	MIB_ID_RX_FILTER_CNT = 0x4064,
	MIB_ID_RX_UCAST_CNT = 0x4068,
	MIB_ID_RX_MCAST_CNT = 0x406C,
	MIB_ID_RX_BCAST_CNT = 0x4070,
	MIB_ID_RX_ALIGN_ERR_CNT = 0x4074,
	MIB_ID_RX_FCS_ERR_CNT = 0x4078,
	MIB_ID_RX_UNDERSIZE_CNT = 0x407C,
	MIB_ID_RX_FRAG_ERR_CNT = 0x4080,
	MIB_ID_RX_OVERSIZE_CNT = 0x4084,
	MIB_ID_RX_JABB_ERR_CNT = 0x4088,
	MIB_ID_RX_PAUSE_CNT = 0x408C,
	MIB_ID_RX_PKT_64_CNT = 0x4090,
	MIB_ID_RX_PKT_65TO127_CNT = 0x4094,
	MIB_ID_RX_PKT_128TO255_CNT = 0x4098,
	MIB_ID_RX_PKT_256TO511_CNT = 0x409C,
	MIB_ID_RX_PKT_512TO1023_CNT = 0x40A0,
	MIB_ID_RX_PKT_1024TOMAX_CNT = 0x40A4,
	MIB_ID_RX_OCT_CNT_L = 0x40A8,	//MIB_ID_RX_OCT_CNT_H/MIB_ID_RX_OCT_CNT_L
	MIB_ID_RX_OCT_CNT_H = 0x40AC,	//MIB_ID_RX_OCT_CNT_H/MIB_ID_RX_OCT_CNT_L
	MIB_ID_RX_CTRL_DROP_CNT = 0x40B0,
	MIB_ID_RX_ING_DROP_CNT = 0x40B4,
	MIB_ID_RX_ARL_DROP_CNT = 0x40B8,
#endif
} mt7530_switch_api_MibCntType;

typedef enum
{
	OCTMODE_OFF = 0,
	OCTMODE_BAD_ONLY,
	OCTMODE_GOOD_ONLY,
	OCTMODE_BAD_GOOD_BOTH,
} mt7530_switch_api_RxTxOctMode;

typedef enum
{
	BXMODE_OFF = 0,
	BXMODE_TX_ONLY,
	BXMODE_RX_ONLY,
	BXMODE_TX_RX_BOTH,
} mt7530_switch_api_BxPktMode;

typedef enum {
	ACL_SET_MODE_NO_UNICAST = 0,
	ACL_SET_MODE_BROADCAST = 1,
	ACL_SET_MODE_MULTICAST = 2,
	ACL_SET_MODE_DMAC = 3,
	ACL_SET_MODE_SMAC = 4,
	ACL_SET_MODE_VLAN = 5,
}mt7530_switch_api_AclMode;
enum traffic_type{
    WAN_TO_LAN,
    WIFI0_RA_TO_LAN,
    WIFI1_RAIX_TO_LAN,
    TRAFFIC_TYPE_MAX,
};

typedef struct
{
	mt7530_switch_api_cmdid cmdid;
	u32 PortQueueId;	// port id, or queue id, or ...
	void *paramext_ptr;	// pointer to any paramext struct, or just 32-bit variables
	int ret_value;
} mt7530_switch_api_params;

typedef struct
{
	u32 p1;
	u32 p2;
} mt7530_switch_api_paramext2;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
} mt7530_switch_api_paramext4;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
} mt7530_switch_api_paramext6;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
	u32 p7;
	u32 p8;
} mt7530_switch_api_paramext8;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
	u32 p7;
	u32 p8;
	u32 p9;
} mt7530_switch_api_paramext9;


typedef struct
{
	mt7530_switch_api_MibCntType MibCntType;
	u32 Out__Cnt;
}
mt7530_switch_GetMibCnt_param;

typedef struct
{
	u8 index_param;
	u16 MII_BMCR_value;		// MII_BMCR(0x00) : Basic mode control register.
	u16 MII_ADVERTISE_value;	// MII_ADVERTISE(0x04) : Advertisement control register.
	u16 MII_CTRL1000_value;		// MII_CTRL1000(0x09) : 1000BASE-T Control register
} mt7530_switch_MIILinkType;

#define TRTCM_RULE_NUM  32
typedef struct mt7530_switch_api_trtcm_s
{
	u8	trtcmId;
	u16 CIR;
	u16 PIR;
	u16 CBS;
	u16 PBS;
} mt7530_switch_api_trtcm_t, *mt7530_switch_api_trtcm_ptr;

typedef struct mt7530_switch_api_matrix_s
{
    int group[6];
    int type;
}mt7530_switch_api_matrix_t, *mt7530_switch_api_matrix_ptr;

typedef struct mt7530_switch_api_backPressure_s
{
	u8 Enable;
	u32 time;
	u16 MaxQueueThreshold;
	u16 MinQueueThreshold;
} mt7530_switch_api_backPressure_t, *mt7530_switch_api_backPressure_ptr;

typedef struct mt7530_switch_DropPolicy_s
{
	u16 greenPacketDropQueueMaxThr;
	u16 greenPacketDropQueueMinThr;
	u16 yellowPacketDropQueueMaxThr;
	u16 yellowPacketDropQueueMinThr;
	u8 greenPacketDropMax_p;
	u8 yellowPacketDropMaxn_p;
	u8 QueueDropW_q;
	u8 DropPrecdenceColourMarking;
} mt7530_switch_DropPolicy_t, *mt7530_switch_DropPolicy_ptr;

typedef struct mt7530_switch_api_trafficDescriptor_s
{
	u32 CIR;
	u32 PIR;
	u32 CBS;
	u32 PBS;
	u8 colourMode;
	u8 ingressColourMarking;
	u8 engressColourMarking;
	u8 meterType;
	u8 direction;	// 0:TX, 1:RX
} mt7530_switch_api_trafficDescriptor_t, *mt7530_switch_api_trafficDescriptor_ptr;

typedef struct
{
	u32 seconds;
	u32 nanoseconds;
} mt7530_switch_api_timestamp;

typedef struct
{
	mt7530_switch_api_timestamp tstamp;
	u8 msg_type;
	u8 seq_id;
} mt7530_switch_api_timestamp_fifo;

typedef struct mt7530_switch_api_trtcm_acl_s
{
	u8 enable;
	struct mt7530_switch_api_trtcm_s trtcm_acl_table[TRTCM_RULE_NUM];
}mt7530_switch_api_trtcm_acl_t, *mt7530_switch_api_trtcm_acl_ptr;

typedef enum
{
	PORT_VLAN_UNTAG = 0,
	PORT_VLAN_SWAP,
	PORT_VLAN_TAG,
	PORT_VLAN_STACK
}ECNT_SWITCH_VLAN_MODE;

typedef struct mt7530_switch_api_vlan_table_s
{
    unsigned char port_id;
    unsigned int o_vid;
    unsigned int n_vid;
    ECNT_SWITCH_VLAN_MODE vlan_mode;
    unsigned char enable;
}mt7530_switch_api_vlan_table_t, *mt7530_switch_api_vlan_table_ptr;

typedef struct
{
    u8 wan_type;
    u8 interface;
    u8 mode;
}rgmii_setting_s;
#define MAX_VAILD_PORT_ENTRY  (16)
typedef enum
{
    E_CHIP_MT7520S = 0,
    E_CHIP_MT7520F = 1,
    E_CHIP_MT7520G = 2,
    E_CHIP_MT7525F = 3,
    E_CHIP_MT7525G = 4,
	E_CHIP_MT7521F = 5, 	 
	E_CHIP_MT7526F = 6,    
    E_CHIP_MT751627 = 7,
    E_CHIP_MT7528 = 8,
    E_CHIP_MT7580 = 10,
}e_chip_type;
typedef struct _port_mac_s
{
    u8 mac[6];
}port_mac;
typedef struct
{    
    int port;
    u8 vaild_entry_num;
    port_mac portmac[MAX_VAILD_PORT_ENTRY];
}mt7530_switch_port_mac;

#define TRTCM_MATCH_MAC_RULE_NUM  		10
#define ACL_MAC_MATCH_TABLE_NUM 		3
#define ACL_MAC_UP_STREAM  				0
#define ACL_MAC_DOWN_STREAM  			1

struct mt7530AclMacSpeedNode_s
{
	int dev_mac[6];
	unsigned char ds_used; /* ds node used; 0 : un-used, 1 : used */
	unsigned char mac_entry_index; /* pair queue index */	
	int     ds_queue_speed; /* pair queue speed */
	int     us_queue_speed; /* pair queue speed */

};

typedef struct mt7530_switch_dev_speed_acl_s
{
	u8 enable;
	struct mt7530AclMacSpeedNode_s trtcm_acl_mac_table[TRTCM_MATCH_MAC_RULE_NUM];
}mt7530_switch_dev_speed_acl_t, *mt7530_switch_dev_speed_acl_ptr;

typedef struct mt7530_switch_api_cnt_s
{
    //u64 TxPktsByte;
	u32 TxPktsCnt;
    u32 TxUniPktsCnt;
    u32 TxBroadPktsCnt;
    u32 TxMultiPktsCnt;	
    u32 TxNonUniPktsCnt;
    u32 TxBytesCnt_Lo;
    u32 TxBytesCnt_Hi;
	
    u32 TxUnderSizePktsCnt;
    u32 Tx64BytePktsCnt;
    u32 Tx65_127BytePktsCnt;
    u32 Tx128_255BytePktsCnt;
    u32 Tx256_511BytePktsCnt;
    u32 Tx512_1023BytePktsCnt;
    u32 Tx1024_1518BytePktsCnt;
    u32 TxOverSizePktsCnt;
	
	u32 TxDropFramesCnt;
    u32 TxPauseFramesCnt;
    u32 TxCRCFramesCnt;
    u32 TxSingleCollsionEvent;
    u32 TxMultiCollsionEvent;
    
    //u64 RTxPktsByte;
    u32 RxPktsCnt;
	u32 RxUniPktsCnt;
	u32 RxBroadPktsCnt;
	u32 RxMultiPktsCnt;
    u32 RxNonUniPktsCnt;
    u32 RxBytesCnt_Lo;
    u32 RxBytesCnt_Hi;
	
    u32 RxUnderSizePktsCnt;
    u32 Rx64BytePktsCnt;
    u32 Rx65_127BytePktsCnt;
    u32 Rx128_255BytePktsCnt;
    u32 Rx256_511BytePktsCnt;
    u32 Rx512_1023BytePktsCnt;
    u32 Rx1024_1518BytePktsCnt;
    u32 RxOverSizePktsCnt;
	
    u32 RxDropFramesCnt;
    u32 RxPauseFramesCnt;
    u32 RxCRCFramesCnt;

	
} mt7530_switch_api_cnt;


typedef struct
{
	u8 port;
	u8 tx_enable;
	u8 rx_enable;
}
mt7530_switch_srcports;

typedef struct
{
	u8 enable;
	mt7530_switch_srcports srcport[7];
	u8 srcportnum;
	u8 dstport;
}
mt7530_switch_lan2lan_info;

typedef struct
{
	u8 srcport;
	u8 tx_enable;
	u8 rx_enable;
	u8 dstport;
}
mt7530_switch_lan2lan;

typedef struct
{
	u8 qosmode;
    u8 wrr_queue_cnt;
	u8 wi[4];
}
mt7530_switch_qosparam;


typedef struct
{
	u8 port;
	u8 timer;
	u8 mac[6];
	u16 vid;
}
mt7530_switch_macinfo;

typedef struct
{
	u32 count;
	mt7530_switch_macinfo macinfo[2048];
}mt7530_switch_brtable;


typedef struct{
	union{
		u8 mac[6] ;
		u32 vlan ;
	}pattern_info ;
	u32 rate ;	/*speed for ratelimit*/
	u8 mode ;	/*0:for no unicast; 1: for broadcast; 2:for multicast*/
				/*3: for dmac ; 4:for smac; 5: for vlan*/
	u8 index ;	/*index for mode 3~5*/
	u8 action ;	/*0:ratelimit; 1:drop. Now only support ratelimit*/
	u8 enable ;
}mt7530_switch_acl_info;

typedef struct{
	u8 enable;   
	u8 shift;
	u8 width;
}mt7530_switch_queue_info;

typedef struct mt7530_switch_arl_mactbladd_s
{
	u32 mac[6];
	u32 destportmap;
	u32 leaky_en;
	u32 eg_tag;
	u32 usr_pri;
	u32 sa_mir_en;
	u32 sa_port_fw;
} mt7530_switch_mactbladd_t, *mt7530_switch_arl_mactbladd_ptr;

typedef struct mt7530_switch_arl_diptbladd_s
{
    u32 dip;
    u32 destportmap;
    u32 status;
    u32 leaky_en;
    u32 eg_tag;
    u32 usr_pri;
    u32 is_extend_gsw;
} mt7530_switch_arl_diptbladd_t, *mt7530_switch_arl_diptbladd_ptr;

typedef struct mt7530_switch_arl_siptbladd_s
{
    u32 dip;
    u32 sip;
    u32 destportmap;
    u32 is_extend_gsw;
} mt7530_switch_arl_siptbladd_t, *mt7530_switch_arl_siptbladd_ptr;

typedef struct mt7530_switch_arl_mul_iptbladd_s
{
    u32 dip;
    u32 sip;
    u32 sw_mask;
    u32 is_extend_gsw;
}mt7530_switch_arl_mul_iptbladd_t, *mt7530_switch_arl_mul_iptbladd_ptr;
typedef struct  mt7530_switch_api_vlan_vid_s
{
	u8 index;
	u8 active;
	u8 portMap;
	u8 tagPortMap;
	u16 vid;
	u8 ivl_en;
	u8 fid;
	u16 stag;
}  mt7530_switch_api_vlan_vid_t, * mt7530_switch_api_vlan_vid_ptr;

typedef struct  mt7530_switch_api_port_mirror_port_based_s
{
	u8 port_tx_mir;
	u8 port_rx_mir;
	u8 vlan_mis; 
	u8 acl_mir; 
	u8 igmp_mir;
}  mt7530_switch_api_port_mirror_port_based_t, * mt7530_switch_api_port_mirror_port_based_ptr;

typedef struct  mt7530_switch_api_qos_port_weight_s
{
	u8 port;
	u8 weight[8];
}  mt7530_switch_api_qos_port_weight_t, * mt7530_switch_api_qos_port_weight_ptr;

typedef struct mt7530_switch_config_s{
	int phyaddr;
	int reg;
	u32 value;
	int ext_switch;
}mt7530_switch_config_t, * mt7530_switch_config_ptr;

typedef enum
{
	ARP = 0,
	PPOE,
	IGMP,
	MLD,
	DHCP,
	DEFAULT,
}vip_type;

typedef struct{
	vip_type type;   
	u8 isvip;
}mt7530_switch_vip_info;

typedef struct mt7530_switch_set_mac_lan_dev_bandwidth_s
{
	u8 mac[6];
	u8 action_type;
}mt7530_switch_set_mac_lan_dev_bandwidth_t;

typedef struct mt7530_switch_get_mac_lan_dev_bandwidth_s
{
	u8 mac[6];
	u8 type;
    u32 up_rate;
    u32 down_rate;
}mt7530_switch_get_mac_lan_dev_bandwidth_t;

typedef struct mt7530_switch_set_mac_max_wifi_bandwidth_s
{
	u8 mac[6];
    u32 up_rate;
    u32 down_rate;
}mt7530_switch_set_mac_max_wifi_bandwidth_t;

typedef struct {
    int wan_port;
    int lan_portmap[6];
}mt7530_switch_eth_portmap_t;

typedef struct 
{
    char eth_port_state; /* -1 :reserved  , 0 internel switch, 1:external switch*/
    int vlan_id;/*-1 untag,  0~4095 tag*/
}eth_port_vlan_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/




/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/



/************************************************************************
*                    R E G I S T E R   D E F I N E
*************************************************************************
*/

#define REG_MFC_ADDR			(0x0010)
#define REG_MFC_MIRROR_PORT_OFFT	(0)
#define REG_MFC_MIRROR_PORT_LENG	(3)
#define REG_MFC_MIRROR_PORT_RELMASK	(0x00000007)
#define REG_MFC_MIRROR_PORT_MASK	(REG_MFC_MIRROR_PORT_RELMASK << REG_MFC_MIRROR_PORT_OFFT)
#define REG_MFC_MIRROR_EN_OFFT		(3)
#define REG_MFC_MIRROR_EN_LENG		(1)
#define REG_MFC_MIRROR_EN_RELMASK	(0x00000001)
#define REG_MFC_MIRROR_EN_MASK		(REG_MFC_MIRROR_EN_RELMASK << REG_MFC_MIRROR_EN_OFFT)

#define REG_ATA1_ADDR			(0x0074)
#define REG_ATA2_ADDR			(0x0078)

#define REG_ATWD_ADDR			(0x007C)
#define REG_ATWD_STATUS_OFFT		(2)
#define REG_ATWD_STATUS_LENG		(2)
#define REG_ATWD_STATUS_RELMASK		(0x00000003)
#define REG_ATWD_STATUS_MASK		(REG_ATWD_STATUS_RELMASK << REG_ATWD_STATUS_OFFT)
#define REG_ATWD_PORT_OFFT		(4)
#define REG_ATWD_PORT_LENG		(8)
#define REG_ATWD_PORT_RELMASK		(0x000000FF)
#define REG_ATWD_PORT_MASK		(REG_ATWD_PORT_RELMASK << REG_ATWD_PORT_OFFT)
#define REG_ATWD_LEAKY_EN_OFFT		(12)
#define REG_ATWD_LEAKY_EN_LENG		(1)
#define REG_ATWD_LEAKY_EN_RELMASK	(0x00000001)
#define REG_ATWD_LEAKY_EN_MASK		(REG_ATWD_LEAKY_EN_RELMASK << REG_ATWD_LEAKY_EN_OFFT)
#define REG_ATWD_EG_TAG_OFFT		(13)
#define REG_ATWD_EG_TAG_LENG		(3)
#define REG_ATWD_EG_TAG_RELMASK		(0x00000007)
#define REG_ATWD_EG_TAG_MASK		(REG_ATWD_EG_TAG_RELMASK << REG_ATWD_EG_TAG_OFFT)
#define REG_ATWD_USR_PRI_OFFT		(16)
#define REG_ATWD_USR_PRI_LENG		(3)
#define REG_ATWD_USR_PRI_RELMASK	(0x00000007)
#define REG_ATWD_USR_PRI_MASK		(REG_ATWD_USR_PRI_RELMASK << REG_ATWD_USR_PRI_OFFT)
#define REG_ATWD_SA_MIR_EN_OFFT		(19)
#define REG_ATWD_SA_MIR_EN_LENG		(1)
#define REG_ATWD_SA_MIR_EN_RELMASK	(0x00000001)
#define REG_ATWD_SA_MIR_EN_MASK		(REG_ATWD_SA_MIR_EN_RELMASK << REG_ATWD_SA_MIR_EN_OFFT)
#define REG_ATWD_SA_PORT_FW_OFFT	(20)
#define REG_ATWD_SA_PORT_FW_LENG	(3)
#define REG_ATWD_SA_PORT_FW_RELMASK	(0x00000007)
#define REG_ATWD_SA_PORT_FW_MASK	(REG_ATWD_SA_PORT_FW_RELMASK << REG_ATWD_SA_PORT_FW_OFFT)

#define REG_ATC_ADDR			(0x0080)
#define REG_ATC_AC_CMD_OFFT		(0)
#define REG_ATC_AC_CMD_LENG		(3)
#define REG_ATC_AC_CMD_RELMASK		(0x00000007)
#define REG_ATC_AC_CMD_MASK		(REG_ATC_AC_CMD_RELMASK << REG_ATC_AC_CMD_OFFT)
#define REG_ATC_AC_SAT_OFFT		(4)
#define REG_ATC_AC_SAT_LENG		(2)
#define REG_ATC_AC_SAT_RELMASK		(0x00000003)
#define REG_ATC_AC_SAT_MASK		(REG_ATC_AC_SAT_RELMASK << REG_ATC_AC_SAT_OFFT)
#define REG_ATC_AC_MAT_OFFT		(8)
#define REG_ATC_AC_MAT_LENG		(4)
#define REG_ATC_AC_MAT_RELMASK		(0x0000000F)
#define REG_ATC_AC_MAT_MASK		(REG_ATC_AC_MAT_RELMASK << REG_ATC_AC_MAT_OFFT)
#define REG_AT_SRCH_HIT_OFFT		(13)
#define REG_AT_SRCH_HIT_RELMASK		(0x00000001)
#define REG_AT_SRCH_HIT_MASK		(REG_AT_SRCH_HIT_RELMASK << REG_AT_SRCH_HIT_OFFT)
#define REG_AT_SRCH_END_OFFT		(14)
#define REG_AT_SRCH_END_RELMASK		(0x00000001)
#define REG_AT_SRCH_END_MASK		(REG_AT_SRCH_END_RELMASK << REG_AT_SRCH_END_OFFT)
#define REG_ATC_BUSY_OFFT		(15)
#define REG_ATC_BUSY_LENG		(1)
#define REG_ATC_BUSY_RELMASK		(0x00000001)
#define REG_ATC_BUSY_MASK		(REG_ATC_BUSY_RELMASK << REG_ATC_BUSY_OFFT)
#define REG_AT_ADDR_OFFT		(16)
#define REG_AT_ADDR_LENG		(12)
#define REG_AT_ADDR_RELMASK		(0x00000FFF)
#define REG_AT_ADDR_MASK		(REG_AT_ADDR_RELMASK << REG_AT_ADDR_OFFT)

#define REG_TSRA1_ADDR			(0x0084)
#define REG_TSRA2_ADDR			(0x0088)
#define REG_ATRD_ADDR			(0x008C)

#define REG_GFCCR0_ADDR			(0x1FE0)
#define REG_FC_EN_OFFT			(31)
#define REG_FC_EN_RELMASK		(0x00000001)
#define REG_FC_EN_MASK			(REG_FC_EN_RELMASK << REG_FC_EN_OFFT)

#define REG_SSC_P0_ADDR			(0x2000)

#define REG_PCR_P0_ADDR			(0x2004)
#define REG_PCR_VLAN_MIS_OFFT		(2)
#define REG_PCR_VLAN_MIS_LENG		(1)
#define REG_PCR_VLAN_MIS_RELMASK	(0x00000001)
#define REG_PCR_VLAN_MIS_MASK		(REG_PCR_VLAN_MIS_RELMASK << REG_PCR_VLAN_MIS_OFFT)
#define REG_PCR_ACL_MIR_OFFT		(7)
#define REG_PCR_ACL_MIR_LENG		(1)
#define REG_PCR_ACL_MIR_RELMASK		(0x00000001)
#define REG_PCR_ACL_MIR_MASK		(REG_PCR_ACL_MIR_RELMASK << REG_PCR_ACL_MIR_OFFT)
#define REG_PORT_RX_MIR_OFFT		(8)
#define REG_PORT_RX_MIR_LENG		(1)
#define REG_PORT_RX_MIR_RELMASK		(0x00000001)
#define REG_PORT_RX_MIR_MASK		(REG_PORT_RX_MIR_RELMASK << REG_PORT_RX_MIR_OFFT)
#define REG_PORT_TX_MIR_OFFT		(9)
#define REG_PORT_TX_MIR_LENG		(1)
#define REG_PORT_TX_MIR_RELMASK		(0x00000001)
#define REG_PORT_TX_MIR_MASK		(REG_PORT_TX_MIR_RELMASK << REG_PORT_TX_MIR_OFFT)
#define REG_PCR_EG_TAG_OFFT		(28)
#define REG_PCR_EG_TAG_LENG		(2)
#define REG_PCR_EG_TAG_RELMASK		(0x00000003)
#define REG_PCR_EG_TAG_MASK		(REG_PCR_EG_TAG_RELMASK << REG_PCR_EG_TAG_OFFT)

#define REG_PIC_P0_ADDR			(0x2008)
#define REG_PIC_IGMP_MIR_OFFT		(19)
#define REG_PIC_IGMP_MIR_LENG		(1)
#define REG_PIC_IGMP_MIR_RELMASK	(0x00000001)
#define REG_PIC_IGMP_MIR_MASK		(REG_PIC_IGMP_MIR_RELMASK << REG_PIC_IGMP_MIR_OFFT)

#define REG_PSC_P0_ADDR			(0x200C)

#define REG_PVC_P0_ADDR			(0x2010)
#define REG_PVC_ACC_FRM_OFFT		(0)
#define REG_PVC_ACC_FRM_LENG		(2)
#define REG_PVC_ACC_FRM_RELMASK		(0x00000003)
#define REG_PVC_ACC_FRM_MASK		(REG_PVC_ACC_FRM_RELMASK << REG_PVC_ACC_FRM_OFFT)
#define REG_PVC_EG_TAG_OFFT		(8)
#define REG_PVC_EG_TAG_LENG		(3)
#define REG_PVC_EG_TAG_RELMASK		(0x00000007)
#define REG_PVC_EG_TAG_MASK		(REG_PVC_EG_TAG_RELMASK << REG_PVC_EG_TAG_OFFT)

#define REG_PPBV1_P0_ADDR		(0x2014)
#define REG_PPBV2_P0_ADDR		(0x2018)
#define REG_BSR_P0_ADDR			(0x201C)
#define REG_STAG01_P0_ADDR		(0x2020)
#define REG_STAG23_P0_ADDR		(0x2024)
#define REG_STAG45_P0_ADDR		(0x2028)
#define REG_STAG67_P0_ADDR		(0x202C)

#define REG_CMACCR_ADDR			(0x30E0)
#define REG_MTCC_LMT_OFFT		(9)
#define REG_MTCC_LMT_LENG		(4)
#define REG_MTCC_LMT_RELMASK		(0x0000000F)
#define REG_MTCC_LMT_MASK		(REG_MTCC_LMT_RELMASK << REG_MTCC_LMT_OFFT)

#define REG_PORT_RX_BYTE_LOW_CNT	(0x40A8)
#define REG_PORT_RX_BYTE_HIGH_CNT	(0x40AC)
#define REG_PORT_TX_BYTE_LOW_CNT	(0x4048)
#define REG_PORT_TX_BYTE_HIGH_CNT	(0x404C)

#define RGMII 1
#define TRGMII 0

#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef ENABLE
#define ENABLE  1
#endif

#define MAC_TX 0
#define MAC_RX 1
#endif // #ifndef _ETH_GLOBAL_API_H_

