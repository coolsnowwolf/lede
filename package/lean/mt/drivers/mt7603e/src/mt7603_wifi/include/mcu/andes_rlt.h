/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	andes_rlt.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_RLT_H__
#define __ANDES_RLT_H__

#include "mcu.h"

#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif

struct _RTMP_ADAPTER;
struct _RXFCE_INFO;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;
struct cmd_msg;

#define CPU_CTL					0x0704
#define CLOCK_CTL				0x0708
#define RESET_CTL				0x070C
#define INT_LEVEL				0x0718
#define COM_REG0				0x0730
#define COM_REG1				0x0734
#define COM_REG2				0x0738
#define COM_REG3				0x073C
#define PCIE_REMAP_BASE1		0x0740
#define PCIE_REMAP_BASE2		0x0744
#define PCIE_REMAP_BASE3		0x0748
#define PCIE_REMAP_BASE4		0x074C
#define LED_CTRL				0x0770
#define LED_TX_BLINK_0			0x0774
#define LED_TX_BLINK_1			0x0778
#define LED0_S0				0x077C
#define LED0_S1				0x0780
#define SEMAPHORE_00			0x07B0
#define SEMAPHORE_01			0x07B4
#define SEMAPHORE_02			0x07B8
#define SEMAPHORE_03			0x07BC

#define MCU_WAIT_ACK_CMD_THRESHOLD 0x0f
#define MCU_RX_CMD_THRESHOLD 0x0f


#ifdef RTMP_MAC_PCI
#define ANDES_FIRMWARE_IMAGE_BASE     0x80000
#endif /* RTMP_MAC_PCI */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_CMD_PKT{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 cmd_type:7;
	UINT32 cmd_seq:4;
	UINT32 pkt_len:16;
}TXINFO_NMAC_CMD_PKT;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_CMD_PKT {
	UINT32 pkt_len:16;
	UINT32 cmd_seq:4;
	UINT32 cmd_type:7;
	UINT32 d_port:3;
	UINT32 info_type:2;
}TXINFO_NMAC_CMD_PKT;
#endif /* RT_BIG_ENDIAN */

#define UPLOAD_PATCH_UNIT 2048
#define PATCH_INFO_SIZE 30
#define FW_INFO_SIZE 32
#define IV_SIZE 0x40
#define GET_SEMAPHORE_RETRY_MAX 600
#define UPLOAD_FW_UNIT 14592
#define UPLOAD_FW_TIMEOUT 1000

/*
 * Calibration ID
 */
enum CALIBRATION_ID {
	R_CALIBRATION = 1,
	RXDCOC_CALIBRATION = 2,
	LC_CALIBRATION = 3,
	LOFT_CALIBRATION = 4,
	TXIQ_CALIBRATION = 5,
	BW_CALIBRATION = 6,
	DPD_CALIBRATION = 7,
	RXIQ_CALIBRATION = 8,
	TXDCOC_CALIBRATION = 9,
	RX_GROUP_DELAY_CALIBRATION = 10,
	TX_GROUP_DELAY_CALIBRATION = 11,
	FULL_CALIBRATION_ID = 0xFF,
};

enum CALIBRATION_ID_7662 {
	R_CALIBRATION_7662 = 1,
	TEMP_SENSOR_CALIBRATION_7662,
	RXDCOC_CALIBRATION_7662,
	RC_CALIBRATION_7662,
	SX_LOGEN_CALIBRATION_7662,
	LC_CALIBRATION_7662,
	TX_LOFT_CALIBRATION_7662,
	TXIQ_CALIBRATION_7662,
	TSSI_CALIBRATION_7662,
	TSSI_COMPENSATION_7662,
	DPD_CALIBRATION_7662,
	RXIQC_FI_CALIBRATION_7662,
	RXIQC_FD_CALIBRATION_7662,
};

enum CALIBRATION_TYPE {
	FULL_CALIBRATION,
	PARTIAL_CALIBRATION,
};

enum SWITCH_CHANNEL_STAGE {
	NORMAL_OPERATING = 0x01,
	SCANNING = 0x02,
	TEMPERATURE_CHANGE = 0x04,
};

/*
 * Function set ID
 */
enum FUN_ID {
	Q_SELECT = 1,
	BW_SETTING = 2,
	USB2_SW_DISCONNECT = 2,
	USB3_SW_DISCONNECT = 3,
	LOG_FW_DEBUG_MSG = 4,
	GET_FW_VERSION = 5,
};


/*
 * FW debug message parameters
 */
enum FW_DEBUG_SETTING {
	DISABLE_DEBUG_MSG = 0,
	DEBUG_MSG_TO_UART,
	DEBUG_MSG_TO_HOST,
};

enum CR_TYPE {
	RF_CR,
	BBP_CR,
	RF_BBP_CR,
	HL_TEMP_CR_UPDATE,
};

enum TEMPERATURE_LEVEL_7662 {
	NORMAL_TEMP_7662, /* 0~60 */
	LOW_TEMP_7662, /* < 0 */
	HIGH_TEMP_7662, /* > 60 */
};

#define LOAD_CR_MODE_MASK (0xff)
#define LOAD_CR_MODE(p) (((p) & 0xff))
#define LOAD_CR_TEMP_LEVEL_MASK (0xff << 8)
#define LOAD_CR_TEMP_LEVEL(p) (((p) & 0xff) << 8)
#define LOAD_CR_CHL_MASK (0xff << 16)
#define LOAD_CR_CHL(p) (((p) & 0xff) << 16)

#define SC_PARAM1_CHL_MASK (0xff)
#define SC_PARAM1_CHL(p) (((p) & 0xff))
#define SC_PARAM1_SCAN_MASK (0xff << 8)
#define SC_PARAM1_SCAN(p) (((p) & 0xff) << 8)
#define SC_PARAM1_BW_MASK (0xff << 16)
#define SC_PARAM1_BW(p) (((p) & 0xff) << 16)

#define SC_PARAM2_TR_SETTING_MASK (0xffff)
#define SC_PARAM2_TR_SETTING(p) (((p) & 0xffff))
#define SC_PARAM2_EXTENSION_CHL_MASK (0xff << 16)
#define SC_PARAM2_EXTENSION_CHL(p) (((p) & 0xff) << 16)

#define TSSI_PARAM2_SLOPE0_MASK (0xff)
#define TSSI_PARAM2_SLOPE0(p) (((p) & 0xff))
#define TSSI_PARAM2_SLOPE1_MASK (0xff << 8)
#define TSSI_PARAM2_SLOPE1(p) (((p) & 0xff) << 8)
#define TSSI_PARAM2_OFFSET0_MASK (0xff << 16)
#define TSSI_PARAM2_OFFSET0(p) (((p) & 0xff) << 16)
#define TSSI_PARAM2_OFFSET1_MASK (0xff << 24)
#define TSSI_PARAM2_OFFSET1(p) (((p) & 0xff) << 24)

/*
 * Command type table 
 */
enum RLT_CMD_TYPE {
	CMD_FUN_SET_OP = 1,
	CMD_LOAD_CR,
	CMD_BURST_WRITE = 8,
	CMD_READ_MODIFY_WRITE,
	CMD_RANDOM_READ,
	CMD_BURST_READ,
	CMD_RANDOM_WRITE = 12,
	CMD_LED_MODE_OP = 16,
	CMD_POWER_SAVING_OP = 20,
	CMD_WOW_CONFIG,
	CMD_WOW_QUERY,
	CMD_WOW_FEATURE = 24,
	CMD_CARRIER_DETECT_OP = 28,
	CMD_RADOR_DETECT_OP,
	CMD_SWITCH_CHANNEL_OP,
	CMD_CALIBRATION_OP,
	CMD_BEACON_OP,
	CMD_ANTENNA_OP
};

/*
 * Event type table
 */
enum RLT_EVENT_TYPE {
	CMD_DONE,
	CMD_ERROR,
	CMD_RETRY,
	EVENT_PWR_RSP,
	EVENT_WOW_RSP,
	EVENT_CARRIER_DETECT_RSP,
	EVENT_DFS_DETECT_RSP,
};

typedef	union _ANDES_CALIBRATION_PARAM {
	UINT32 generic;
} ANDES_CALIBRATION_PARAM;

enum CALIBRATION_TEST_TYPE {
	CAL_ROBUST_TEST=0,

};

#ifdef RTMP_PCI_SUPPORT
INT32 AndesRltPciLoadRomPatch(struct _RTMP_ADAPTER *pAd);
INT32 AndesRltPciEraseRomPatch(struct _RTMP_ADAPTER *pAd);
INT32 AndesRltPciLoadFw(struct _RTMP_ADAPTER *pAd);
INT32 AndesRltPciEraseFw(struct _RTMP_ADAPTER *pAd);
VOID PciRxCmdMsgComplete(struct _RTMP_ADAPTER *pAd, struct _RXFCE_INFO *fce_info);
VOID AndesRltPciFwInit(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_PCI_SUPPORT */
INT32 AndesRltBurstWrite(struct _RTMP_ADAPTER *pAd, UINT32 offset, UINT32 *data, UINT32 cnt);
INT32 AndesRltBurstRead(struct _RTMP_ADAPTER *pAd, UINT32 offset, UINT32 cnt, UINT32 *data);
INT32 AndesRltRandomRead(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *reg_pair, UINT32 num);
INT32 AndesRltRfRandomRead(struct _RTMP_ADAPTER *pAd, struct _BANK_RF_REG_PAIR *reg_pair, UINT32 num);
INT32 AndesRltReadModifyWrite(struct _RTMP_ADAPTER *pAd, struct _R_M_W_REG *reg_pair, UINT32 num);
INT32 AndesRltRfReadModifyWrite(struct _RTMP_ADAPTER *pAd, struct _RF_R_M_W_REG *reg_pair, UINT32 num);
INT32 AndesRltRandomWrite(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *reg_pair, UINT32 num);
INT32 AndesRltRfRandomWrite(struct _RTMP_ADAPTER *pAd, struct _BANK_RF_REG_PAIR *reg_pair, UINT32 num);
#ifdef CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT
INT32 AndesBbpRandomWrite(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *reg_pair, UINT32 num);
#endif /* CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT */
INT32 AndesRltScRandomWrite(struct _RTMP_ADAPTER *pAd, CR_REG *table, UINT32 nums, UINT32 flags);
INT32 AndesRltScRfRandomWrite(struct _RTMP_ADAPTER *pAd, BANK_RF_CR_REG *table, UINT32 nums, UINT32 flags);
INT32 AndesRltFunSet(struct _RTMP_ADAPTER *pAd, UINT32 fun_id, UINT32 param);
INT32 AndesRltPwrSaving(struct _RTMP_ADAPTER *pAd, UINT32 op, UINT32 level, 
						 UINT32 listen_interval, UINT32 pre_tbtt_lead_time,
						 UINT8 tim_byte_offset, UINT8 tim_byte_pattern);
INT32 AndesRltCalibration(struct _RTMP_ADAPTER *pAd, UINT32 cal_id, ANDES_CALIBRATION_PARAM *param);
VOID PciKickOutCmdMsgComplete(PNDIS_PACKET net_pkt);
VOID AndesRltFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
INT32 AndesRltPciKickOutCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
INT32 AndesRltLoadCr(struct _RTMP_ADAPTER *pAd, UINT32 cr_type, UINT8 temp_level, UINT8 channel);
INT32 AndesRltSwitchChannel(struct _RTMP_ADAPTER *pAd, u8 channel, BOOLEAN scan, UINT32 bw, UINT32 tx_rx_setting, UINT8 bbp_ch_idx);
struct cmd_msg *andes_alloc_cmd_msg(struct _RTMP_ADAPTER *pAd, UINT32 length);

VOID AndesRltRxProcessCmdMsg(struct _RTMP_ADAPTER *ad, struct cmd_msg *rx_msg);

#define MAX_CALIBRATION_WAIT_TIME						100
#ifdef RTMP_PCI_SUPPORT
#define ANDES_CALIBRATION_START(_pAd)						\
{															\
	UINT32 _value;											\
	RTMP_IO_READ32(_pAd, COM_REG0, &_value);				\
	_value &= 0x7FFFFFFF;										\
	_RTMP_IO_WRITE32(_pAd, COM_REG0, _value);				\
}


#define ANDES_WAIT_CALIBRATION_DONE(_pAd)				\
{															\
	UINT32 _value;											\
	INT32 _i;													\
	RtmpOsMsDelay(1);										\
	for ( _i = 0 ; _i < MAX_CALIBRATION_WAIT_TIME; _i ++ ) {		\
		RTMP_IO_READ32(_pAd, COM_REG0, &_value);			\
		if ( _value & 0x80000000 )								\
			break;											\
		RtmpOsMsDelay(5);									\
	}														\
	if ( _i == MAX_CALIBRATION_WAIT_TIME )					\
		DBGPRINT(RT_DEBUG_ERROR,("ANDES_WAIT_CALIBRATION_DONE timeout!\n"));	\
	else														\
		DBGPRINT(RT_DEBUG_INFO,("ANDES_WAIT_CALIBRATION_DONE %d\n", _i));	\
}
#endif /* RTMP_PCI_SUPPORT */

#endif /* __ANDES_RLT_H__ */

