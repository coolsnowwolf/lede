/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ap_vow.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"
#include "hdev/hdev.h"

#ifdef VOW_SUPPORT
#define UMAC_DRR_TABLE_CTRL0            (0x00008388)

#define UMAC_DRR_TABLE_WDATA0           (0x00008340)
#define UMAC_DRR_TABLE_WDATA1           (0x00008344)
#define UMAC_DRR_TABLE_WDATA2           (0x00008348)
#define UMAC_DRR_TABLE_WDATA3           (0x0000834C)

/* Charge mode control control operation (0x8x : charge tx time & length) */

#define UMAC_CHARGE_BW_TOKEN_BIT_MASK                       BIT(0)
#define UMAC_CHARGE_BW_DRR_BIT_MASK                         BIT(1)
#define UMAC_CHARGE_AIRTIME_DRR_BIT_MASK                    BIT(2)
#define UMAC_CHARGE_ADD_MODE_BIT_MASK                       BIT(3)

#define UMAC_CHARGE_OP_BASE                                 0x80
#define UMAC_CHARGE_BW_TOKEN_OP_MASK                        (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_BW_TOKEN_BIT_MASK)
#define UMAC_CHARGE_BW_DRR_OP_MASK                          (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_BW_DRR_BIT_MASK)
#define UMAC_CHARGE_AIRTIME_DRR_OP_MASK                     (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_AIRTIME_DRR_BIT_MASK)

#define UMAC_CHARGE_MODE_STA_ID_MASK                        BITS(0, 7)
#define UMAC_CHARGE_MODE_STA_ID_OFFSET                      0
#define UMAC_CHARGE_MODE_QUEUE_ID_MASK                      BITS(8, 11)
#define UMAC_CHARGE_MODE_QUEUE_ID_OFFSET                    8

#define UMAC_CHARGE_MODE_BSS_GROUP_MASK                     BITS(0, 3)
#define UMAC_CHARGE_MODE_BSS_GROUP_OFFSET                   0


#define UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK           BITS(0, 15)
#define UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET         0

#define UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK             BITS(16, 31)
#define UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET           16


/* Change MODE Ctrl operation */
#define UMAC_DRR_TABLE_CTRL0_CHANGE_MODE_MASK               BIT(23)
#define UMAC_DRR_TABLE_CTRL0_CHANGE_MODE_OFFSET             23


/* 00000340 DRR_Table_WData DRR table Wdata register register   00000000 */

#define UMAC_DRR_TABLE_WDATA0_STA_MODE_MASK                  BITS(0, 15)
#define UMAC_DRR_TABLE_WDATA0_STA_MODE_OFFSET                0

/* 00000350 DRR_Table_Rdata DRR table control register read data    00000000 */
#define UMAC_DRR_TABLE_RDATA0_STA_MODE_MASK                  BITS(0, 15)
#define UMAC_DRR_TABLE_RDATA0_STA_MODE_OFFSET                0

/* 00000388 DRR_Table_ctrl0     DRR table control register register 0   00000000 */

#define UMAC_DRR_TABLE_CTRL0_EXEC_MASK                      BIT(31)
#define UMAC_DRR_TABLE_CTRL0_EXEC_OFFSET                    31
#define UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET                 16


#define UMAC_BSS_GROUP_NUMBER               16
#define UMAC_WLAN_ID_MAX_VALUE          127

#ifndef _LINUX_BITOPS_H
#define BIT(n)                          ((UINT32) 1 << (n))
#endif /* BIT */
#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))




#define VOW_DEF_AVA_AIRTIME (1000000)  /* us */

#define VOW_BSS_SETTING_BEGIN   16
#define VOW_BSS_SETTING_END     (VOW_BSS_SETTING_BEGIN + 16)

/* global variables */
PRTMP_ADAPTER pvow_pad;
UINT32 vow_tx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_rx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_tx_ok[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_tx_fail[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_sum_tx_rx_time;
UINT32 vow_avg_sum_time;
UINT32 vow_last_tx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_last_rx_time[MAX_LEN_OF_MAC_TABLE];
UINT16 vow_idx;
UINT32 vow_tx_bss_byte[WMM_NUM_OF_AC];
UINT32 vow_rx_bss_byte[WMM_NUM_OF_AC];
UINT32 vow_tx_mbss_byte[VOW_MAX_GROUP_NUM];
UINT32 vow_rx_mbss_byte[VOW_MAX_GROUP_NUM];
UINT32 vow_ampdu_cnt;
UINT32 vow_interval;
UINT32 vow_last_free_cnt;

/* VOW internal commands */
/***********************************************************/
/*      EXT_CMD_ID_DRR_CTRL = 0x36                         */
/***********************************************************/
/* for station DWRR configration */
INT32 vow_set_sta(PRTMP_ADAPTER pad, UINT8 sta_id, UINT32 subcmd)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	UINT32 Setting = 0;
	INT32 ret;

	if (sta_id >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = subcmd;
	sta_ctrl.ucStaID = sta_id;

	switch (subcmd) {
	case ENUM_VOW_DRR_CTRL_FIELD_STA_ALL:
		/* station configration */
		Setting |= pad->vow_sta_cfg[sta_id].group;
		Setting |= (pad->vow_sta_cfg[sta_id].ac_change_rule << pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK] << pad->vow_gen.VOW_STA_WMM_AC0_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE] << pad->vow_gen.VOW_STA_WMM_AC1_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI] << pad->vow_gen.VOW_STA_WMM_AC2_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO] << pad->vow_gen.VOW_STA_WMM_AC3_OFFSET);
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = Setting;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, Setting));
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].group;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].ac_change_rule;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC1_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC2_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC3_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO];
		break;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L1:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L2:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L3:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L4:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L5:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L6:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L7:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n",
				 __func__, subcmd, pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0]));
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL: {
		UINT32 i;

		/* station quantum configruation */
		for (i = 0; i < VOW_MAX_STA_DWRR_NUM; i++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(vow_sta_dwrr_quantum[%d] = 0x%x)\n", __func__, i, pad->vow_cfg.vow_sta_dwrr_quantum[i]));
			sta_ctrl.rAirTimeCtrlValue.rAirTimeQuantumAllField.ucAirTimeQuantum[i] = pad->vow_cfg.vow_sta_dwrr_quantum[i];
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, Setting));
	}
	break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING: {
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].paused;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, pad->vow_sta_cfg[sta_id].paused));
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
		break;
	}

	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T)));
	return ret;
}

/* for DWRR max wait time configuration */
INT vow_set_sta_DWRR_max_time(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	INT32 ret;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND;
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.sta_max_wait_time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(sta_max_wait_time = 0x%x)\n", __func__, pad->vow_cfg.sta_max_wait_time));
	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T)));
	return ret;
}
/***********************************************************/
/*      EXT_CMD_ID_BSSGROUP_CTRL = 0x37                    */
/***********************************************************/
VOID vow_fill_group_all(PRTMP_ADAPTER pad, UINT8 group_id, EXT_CMD_BSS_CTRL_T *group_ctrl)
{
	/* DW0 */
	group_ctrl->arAllBssGroupMultiField[group_id].u2MinRateToken = pad->vow_bss_cfg[group_id].min_rate_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u2MaxRateToken = pad->vow_bss_cfg[group_id].max_rate_token;
	/* DW1 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinTokenBucketTimeSize = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinAirTimeToken = pad->vow_bss_cfg[group_id].min_airtime_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinTokenBucketLengSize = pad->vow_bss_cfg[group_id].min_ratebucket_size;
	/* DW2 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxTokenBucketTimeSize = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxAirTimeToken = pad->vow_bss_cfg[group_id].max_airtime_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxTokenBucketLengSize = pad->vow_bss_cfg[group_id].max_ratebucket_size;
	/* DW3 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxWaitTime = pad->vow_bss_cfg[group_id].max_wait_time;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxBacklogSize = pad->vow_bss_cfg[group_id].max_backlog_size;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Group id = 0x%x, min_rate %d, max_rate %d, min_ratio %d, max_ratio %d)\n",
			 __func__, group_id,
			 pad->vow_bss_cfg[group_id].min_rate,
			 pad->vow_bss_cfg[group_id].max_rate,
			 pad->vow_bss_cfg[group_id].min_airtime_ratio,
			 pad->vow_bss_cfg[group_id].max_airtime_ratio));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(min rate token = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].min_rate_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max rate token = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_rate_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(min airtime token = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].min_airtime_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max airtime token = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_airtime_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(min rate bucket = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].min_ratebucket_size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max rate bucket = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_ratebucket_size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(min airtime bucket = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].min_airtimebucket_size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max airtime bucket = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_airtimebucket_size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max baclog size = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_backlog_size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(max wait time = 0x%x)\n", __func__, pad->vow_bss_cfg[group_id].max_wait_time));
}
/* for group configuration */
INT vow_set_group(PRTMP_ADAPTER pad, UINT8 group_id, UINT32 subcmd)
{
	EXT_CMD_BSS_CTRL_T group_ctrl;
	INT32 ret;

	NdisZeroMemory(&group_ctrl, sizeof(group_ctrl));
	group_ctrl.u4CtrlFieldID = subcmd;
	group_ctrl.ucBssGroupID = group_id;

	switch (subcmd) {
	/* group configuration */
	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_1_GROUP:
		vow_fill_group_all(pad, group_id, &group_ctrl);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_rate_token;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_rate_token;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtime_token;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_ratebucket_size;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtime_token;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_ratebucket_size;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_wait_time;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_backlog_size;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(SubCmd %x, Value = 0x%x)\n", __func__, subcmd, group_ctrl.u4SingleFieldIDValue));
		break;

	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP: {
		UINT32 i;

		for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
			vow_fill_group_all(pad, i, &group_ctrl);
	}
	break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_01:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_02:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_03:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_04:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_05:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_06:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_07:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_08:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_09:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0A:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0B:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0C:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0D:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0E:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0F:
		/* Group DWRR quantum */
		group_ctrl.ucBssGroupQuantumTime[group_id] = pad->vow_bss_cfg[group_id].dwrr_quantum;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(group %d DWRR quantum = 0x%x)\n", __func__, group_id, pad->vow_bss_cfg[group_id].dwrr_quantum));
		break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL: {
		UINT32 i;

		for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
			group_ctrl.ucBssGroupQuantumTime[i] = pad->vow_bss_cfg[i].dwrr_quantum;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(group %d DWRR quantum = 0x%x)\n", __func__, i, pad->vow_bss_cfg[i].dwrr_quantum));
		}
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
		break;
	}

	ret = MtCmdSetVoWGroupCtrl(pad, &group_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_BSS_CTRL_T)));
	return ret;
}

/* for DWRR max wait time configuration */
INT vow_set_group_DWRR_max_time(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	INT32 ret;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_BW_DEFICIT_BOUND;
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.group_max_wait_time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(group_max_wait_time = 0x%x)\n", __func__, pad->vow_cfg.group_max_wait_time));
	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T)));
	return ret;
}

/***********************************************************/
/*      EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38                 */
/***********************************************************/
/* for group configuration */
INT vow_set_feature_all(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_FEATURE_CTRL_T feature_ctrl;
	INT32 ret, i;

	NdisZeroMemory(&feature_ctrl, sizeof(feature_ctrl));
	/* DW0 - flags */
	feature_ctrl.u2IfApplyBss_0_to_16_CtrlFlag = 0xFFFF; /* 16'b */
	feature_ctrl.u2IfApplyRefillPerildFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyDbdc1SearchRuleFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyDbdc0SearchRuleFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnTxopNoChangeBssFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyAirTimeFairnessFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyWeightedAirTimeFairnessFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnbwrefillFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnbwCtrlFlag = TRUE; /* 1'b */
	/* DW1 - flags */
	feature_ctrl.u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW2 - flags */
	feature_ctrl.u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW5 - ctrl values */
	feature_ctrl.u2Bss_0_to_16_CtrlValue = pad->vow_cfg.per_bss_enable; /* 16'b */
	feature_ctrl.u2RefillPerildValue = pad->vow_cfg.refill_period; /* 8'b */
	feature_ctrl.u2Dbdc1SearchRuleValue = pad->vow_cfg.dbdc1_search_rule; /* 1'b */
	feature_ctrl.u2Dbdc0SearchRuleValue = pad->vow_cfg.dbdc0_search_rule; /* 1'b */
	feature_ctrl.u2WeightedAirTimeFairnessValue = pad->vow_watf_en; /* 1'b */
	feature_ctrl.u2EnTxopNoChangeBssValue = pad->vow_cfg.en_txop_no_change_bss; /* 1'b */
	feature_ctrl.u2AirTimeFairnessValue = pad->vow_cfg.en_airtime_fairness; /* 1'b */
	feature_ctrl.u2EnbwrefillValue = pad->vow_cfg.en_bw_refill; /* 1'b */
	feature_ctrl.u2EnbwCtrlValue = pad->vow_cfg.en_bw_ctrl; /* 1'b */

	/* DW6 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		feature_ctrl.u2BssCheckTimeToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].at_on << i);

	/* DW7 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		feature_ctrl.u2BssCheckLengthToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].bw_on << i);

	if (pad->vow_gen.VOW_GEN == VOW_GEN_2) {
		/* DW8 - misc */
		feature_ctrl.u4IfApplyStaLockForRtsFlag = TRUE; /* 1'b */
		feature_ctrl.u4RtsStaLockValue = pad->vow_misc_cfg.rts_sta_lock; /* 1'b */
		/* VOW is disabled, skip all setting */
		if (vow_is_enabled(pad)) {
			feature_ctrl.u4IfApplyKeepQuantumFlag = TRUE; /* 1'b */
			feature_ctrl.u4KeepQuantumValue = pad->vow_misc_cfg.keep_quantum; /* 1'b */
			feature_ctrl.u4IfApplyTxCountModeFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxCountValue = pad->vow_misc_cfg.tx_rr_count; /* 4'b */
			feature_ctrl.u4IfApplyTxMeasurementModeFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxMeasurementModeValue = pad->vow_misc_cfg.measurement_mode; /* 1'b */
			feature_ctrl.u4IfApplyTxBackOffBoundFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxBackOffBoundEnable = pad->vow_misc_cfg.max_backoff_bound_en; /* 1'b */
			feature_ctrl.u4TxBackOffBoundValue = pad->vow_misc_cfg.max_backoff_bound; /* 4'b */
			feature_ctrl.u4IfApplyRtsFailedChargeDisFlag = TRUE; /* 1'b */
			feature_ctrl.u4RtsFailedChargeDisValue = pad->vow_misc_cfg.rts_failed_charge_time_en; /* 1'b */
			feature_ctrl.u4IfApplyRxEifsToZeroFlag = TRUE; /* 1'b */
			feature_ctrl.u4ApplyRxEifsToZeroValue =	pad->vow_misc_cfg.zero_eifs_time; /* 1'b */
			feature_ctrl.u4IfApplyRxRifsModeforCckCtsFlag = TRUE; /* 1'b */
			feature_ctrl.u4RxRifsModeforCckCtsValue = pad->vow_misc_cfg.rx_rifs_mode; /* 1'b */
			feature_ctrl.u4IfApplyKeepVoWSettingForSerFlag = TRUE; /* 1'b */
			feature_ctrl.u4VowKeepSettingValue = pad->vow_misc_cfg.keep_vow_sram_setting; /* 1'b */
			feature_ctrl.u4VowKeepSettingBit = pad->vow_misc_cfg.keep_vow_sram_setting_bit; /* 1'b */
		}
	}


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2Bss_0_to_16_CtrlValue  = 0x%x)\n", __func__, pad->vow_cfg.per_bss_enable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2RefillPerildValue = 0x%x)\n", __func__, pad->vow_cfg.refill_period));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2Dbdc1SearchRuleValue = 0x%x)\n", __func__, pad->vow_cfg.dbdc1_search_rule));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2Dbdc0SearchRuleValue = 0x%x)\n", __func__, pad->vow_cfg.dbdc0_search_rule));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2EnTxopNoChangeBssValue = 0x%x)\n", __func__, pad->vow_cfg.en_txop_no_change_bss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2AirTimeFairnessValue = 0x%x)\n", __func__, pad->vow_cfg.en_airtime_fairness));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2EnbwrefillValue = 0x%x)\n", __func__, pad->vow_cfg.en_bw_refill));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2EnbwCtrlValue = 0x%x)\n", __func__, pad->vow_cfg.en_bw_ctrl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2WeightedAirTimeFairnessValue = 0x%x)\n", __func__, pad->vow_watf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2BssCheckTimeToken_0_to_16_CtrlValue = 0x%x)\n", __func__, feature_ctrl.u2BssCheckTimeToken_0_to_16_CtrlValue));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(u2BssCheckLengthToken_0_to_16_CtrlValue = 0x%x)\n", __func__, feature_ctrl.u2BssCheckLengthToken_0_to_16_CtrlValue));
	ret = MtCmdSetVoWFeatureCtrl(pad, &feature_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_VOW_FEATURE_CTRL_T)));
	return ret;
}

/***********************************************************/
/*      EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a                  */
/***********************************************************/
/* for RX airtime configuration */
INT vow_set_rx_airtime(PRTMP_ADAPTER pad, UINT8 cmd, UINT32 subcmd)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = cmd;
	rx_at_ctrl.u4CtrlSubFieldID = subcmd;

	switch (cmd) {
	/* RX airtime feature control */
	case ENUM_RX_AT_FEATURE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN:
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.fgRxAirTimeEn = pad->vow_rx_time_cfg.rx_time_en;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 __func__, cmd, subcmd, pad->vow_rx_time_cfg.rx_time_en));
			break;

		case ENUM_RX_AT_FEATURE_SUB_TYPE_MIBTIME_EN:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Not implemented yet = 0x%x)\n", __func__, subcmd));
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such sub command = 0x%x)\n", __func__, subcmd));
		}

		break;

	case ENUM_RX_AT_BITWISE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR: /* clear all RX airtime counters */
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgRxAirTimeClrEn = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 __func__, cmd, subcmd, rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgRxAirTimeClrEn));
			break;

		case ENUM_RX_AT_BITWISE_SUB_TYPE_MIBTIME_CLR:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Not implemented yet = 0x%x)\n", __func__, subcmd));
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such sub command = 0x%x)\n", __func__, subcmd));
		}

		break;

	case ENUM_RX_AT_TIMER_VALUE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL:
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.ucEdOffsetValue = pad->vow_rx_time_cfg.ed_offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd =  0x%x, value = 0x%x)\n",
					 __func__, cmd, subcmd, pad->vow_rx_time_cfg.ed_offset));
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such sub command = 0x%x)\n", __func__, subcmd));
		}

		break;

	case EMUM_RX_AT_REPORT_CTRL:
		switch (subcmd) {

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such sub command = 0x%x)\n", __func__, subcmd));
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
		break;
	}

	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_RX_AT_CTRL_T)));
	return ret;
}

/* select RX WMM backoff time for 4 OM */
INT vow_set_wmm_selection(PRTMP_ADAPTER pad, UINT8 om)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_BITWISE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_BITWISE_SUB_TYPE_STA_WMM_CTRL;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucOwnMacID = om;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgtoApplyWm00to03MibCfg = pad->vow_rx_time_cfg.wmm_backoff_sel[om];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, OM = 0x%x, Map = 0x%x)\n",
			 __func__, rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucOwnMacID, ENUM_RX_AT_BITWISE_SUB_TYPE_STA_WMM_CTRL, om,
			 pad->vow_rx_time_cfg.wmm_backoff_sel[om]));
	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_RX_AT_CTRL_T)));
	return ret;
}

/* set 16 MBSS  mapping to 4 RX backoff time configurations */
INT vow_set_mbss2wmm_map(PRTMP_ADAPTER pad, UINT8 bss_idx)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_BITWISE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_BITWISE_SUB_TYPE_MBSS_WMM_CTRL;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucMbssGroup = bss_idx;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucWmmGroup = pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, bss_idx = 0x%x, Map = 0x%x)\n",
			 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, bss_idx,
			 pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx]));
	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_RX_AT_CTRL_T)));
	return ret;
}

/* set backoff time for RX*/
INT vow_set_backoff_time(PRTMP_ADAPTER pad, UINT8 target)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_TIMER_VALUE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_TIME_VALUE_SUB_TYPE_BACKOFF_TIMER;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackoffWmmGroupIdx = target;

	switch (target) {
	case ENUM_RX_AT_WMM_GROUP_0:
	case ENUM_RX_AT_WMM_GROUP_1:
	case ENUM_RX_AT_WMM_GROUP_2:
	case ENUM_RX_AT_WMM_GROUP_3:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO]));
		break;

	case ENUM_RX_AT_WMM_GROUP_PEPEATER:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO]));
		break;

	case ENUM_RX_AT_WMM_GROUP_STA:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO]));
		break;

	case ENUM_RX_AT_NON_QOS:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.non_qos_backoff;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, backoff time = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.non_qos_backoff));
		break;

	case ENUM_RX_AT_OBSS:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.obss_backoff;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, backoff time = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.obss_backoff));
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, target));
		break;
	}

	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_RX_AT_CTRL_T)));
	return ret;
}

/* set backoff time for RX*/
INT vow_get_rx_time_counter(PRTMP_ADAPTER pad, UINT8 target, UINT8 band_idx)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = EMUM_RX_AT_REPORT_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = target;

	switch (target) {
	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxNonWiFiBandIdx = band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, target = 0x%x, band_idx = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target, band_idx));
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxObssBandIdx = band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, target = 0x%x, band_idx = 0x%x)\n",
				 __func__, rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target, band_idx));
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Not implemented yet = 0x%x)\n", __func__, target));
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, target));
	}

	ret = MtCmdGetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_RX_AT_CTRL_T)));

	if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer;
	else if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer;
	else if (target == ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer;
	else
		return -1;
}
/***********************************************************/
/*      EXT_CMD_ID_AT_PROC_MODULE = 0x4b                   */
/***********************************************************/

/* for airtime estimator module */
INT vow_set_at_estimator(PRTMP_ADAPTER pad, UINT32 subcmd)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	at_proc.u4CtrlFieldID = ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL;
	at_proc.u4CtrlSubFieldID = cpu2le16(subcmd);
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(at_proc.u4CtrlFieldID);
#endif

	switch (subcmd) {
	case ENUM_AT_PROC_EST_FEATURE_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.fgAtEstimateOnOff = pad->vow_at_est.at_estimator_en;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_at_est.at_estimator_en));
		break;

	case ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2AtEstMonitorPeriod = cpu2le16(pad->vow_at_est.at_monitor_period);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_at_est.at_monitor_period));
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T)));
	return ret;
}

INT vow_set_at_estimator_group(PRTMP_ADAPTER pad, UINT32 subcmd, UINT8 group_id)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	at_proc.u4CtrlFieldID = ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL;
	at_proc.u4CtrlSubFieldID = subcmd;
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(at_proc.u4CtrlFieldID);
	at_proc.u4CtrlSubFieldID = cpu2le16(at_proc.u4CtrlSubFieldID);
#endif

	switch (subcmd) {
	case ENUM_AT_PROC_EST_GROUP_RATIO_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask |= (1UL << group_id);
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2GroupMaxRatioValue[group_id] = cpu2le16(pad->vow_bss_cfg[group_id].max_airtime_ratio);
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2GroupMinRatioValue[group_id] = cpu2le16(pad->vow_bss_cfg[group_id].min_airtime_ratio);
#ifdef RT_BIG_ENDIAN
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask =
			cpu2le32(at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask);
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group %d, val = 0x%x/0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, group_id,
				 pad->vow_bss_cfg[group_id].max_airtime_ratio,
				 pad->vow_bss_cfg[group_id].min_airtime_ratio));
		break;

	case ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.ucGrouptoSelectBand = group_id;
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.ucBandSelectedfromGroup = pad->vow_bss_cfg[group_id].band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, group %d, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, group_id, pad->vow_bss_cfg[group_id].band_idx));
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T)));
	return ret;
}

/* for bad node detector */
INT vow_set_bad_node(PRTMP_ADAPTER pad, UINT32 subcmd)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	at_proc.u4CtrlFieldID = ENUM_AT_RPOCESS_BAD_NODE_MODULE_CTRL;
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(at_proc.u4CtrlFieldID);
#endif
	at_proc.u4CtrlSubFieldID = cpu2le16(subcmd);

	switch (subcmd) {
	case ENUM_AT_PROC_BAD_NODE_FEATURE_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.fgAtBadNodeOnOff = pad->vow_badnode.bn_en;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_badnode.bn_en));
		break;

	case ENUM_AT_PROC_BAD_NODE_MONITOR_PERIOD_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.u2AtBadNodeMonitorPeriod = cpu2le16(pad->vow_badnode.bn_monitor_period);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_badnode.bn_monitor_period));
		break;

	case ENUM_AT_PROC_BAD_NODE_FALLBACK_THRESHOLD:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.ucFallbackThreshold = pad->vow_badnode.bn_fallback_threshold;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_badnode.bn_fallback_threshold));
		break;

	case ENUM_AT_PROC_BAD_NODE_PER_THRESHOLD:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.ucTxPERThreshold = pad->vow_badnode.bn_per_threshold;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 __func__, at_proc.u4CtrlFieldID, at_proc.u4CtrlSubFieldID, pad->vow_badnode.bn_per_threshold));
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(No such command = 0x%x)\n", __func__, subcmd));
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d), sizeof %zu\n", __func__, ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T)));
	return ret;
}

void vow_dump_umac_CRs(PRTMP_ADAPTER pad)
{
	int i;

	for (i = 0x8340; i < 0x83c0; i += 4) {
		UINT32 val;

		RTMP_IO_READ32(pad, i, &val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%0x -> 0x%0x\n", i, val));
	}
}
/* ---------------------- end -------------------------------*/

BOOLEAN vow_is_enabled(PRTMP_ADAPTER pad)
{
	return (pad->vow_cfg.en_bw_ctrl || pad->vow_cfg.en_airtime_fairness);
}

BOOLEAN vow_watf_is_enabled(
	IN PRTMP_ADAPTER pad)
{
	return pad->vow_watf_en;
}

VOID vow_init(PRTMP_ADAPTER pad)
{
	BOOLEAN ret;
	/* for M2M test */
	pvow_pad = pad;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[31m%s: start ...\x1b[m\n", __func__));
	/* vow CR address init */
	vow_init_CR_offset_gen_1_gen_2(pad);
	/* vow_dump_umac_CRs(pad); */
	/* vow station init */
	vow_init_sta(pad);
	/* vow group init */
	vow_init_group(pad);
	/* vow rx init */
	vow_init_rx(pad);
	/* vow misc init */
	vow_init_misc(pad);
	/* feature control */
	ret = vow_set_feature_all(pad);
	/* configure badnode detector */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[31m%s: end ...\x1b[m\n", __func__));
}

VOID vow_init_CR_offset_gen_1_gen_2(PRTMP_ADAPTER pad)
{
	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* VOW debug command 0x22/0x44 */
		pad->vow_gen.VOW_STA_SETTING_BEGIN = 0;
		pad->vow_gen.VOW_STA_SETTING_END = pad->vow_gen.VOW_STA_SETTING_BEGIN + 16;
		pad->vow_gen.VOW_STA_BITMAP_BEGIN = 0;
		pad->vow_gen.VOW_STA_BITMAP_END = pad->vow_gen.VOW_STA_BITMAP_BEGIN + 16;
		pad->vow_gen.VOW_BSS_TOKEN_OFFSET = 32;
		pad->vow_gen.VOW_STA_SETTING_FACTOR = 3;

		/* STA setting */
		pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET = 4;
		pad->vow_gen.VOW_STA_WMM_AC0_OFFSET = 6;
		pad->vow_gen.VOW_STA_WMM_AC1_OFFSET = 8;
		pad->vow_gen.VOW_STA_WMM_AC2_OFFSET = 10;
		pad->vow_gen.VOW_STA_WMM_AC3_OFFSET = 12;
		pad->vow_gen.VOW_STA_WMM_ID_OFFSET = 14;
	} else {
		/* VOW debug command 0x22/0x44 */
		pad->vow_gen.VOW_STA_SETTING_BEGIN = 68;
		pad->vow_gen.VOW_STA_SETTING_END = pad->vow_gen.VOW_STA_SETTING_BEGIN + 32;
		pad->vow_gen.VOW_STA_BITMAP_BEGIN = 52;
		pad->vow_gen.VOW_STA_BITMAP_END = pad->vow_gen.VOW_STA_BITMAP_BEGIN + 16;
		pad->vow_gen.VOW_BSS_TOKEN_OFFSET = 0;
		pad->vow_gen.VOW_STA_SETTING_FACTOR = 2;

		/* STA setting */
		pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET = 4;
		pad->vow_gen.VOW_STA_WMM_AC0_OFFSET = 8;
		pad->vow_gen.VOW_STA_WMM_AC1_OFFSET = 12;
		pad->vow_gen.VOW_STA_WMM_AC2_OFFSET = 16;
		pad->vow_gen.VOW_STA_WMM_AC3_OFFSET = 20;
		pad->vow_gen.VOW_STA_WMM_ID_OFFSET = 6;
	}
}


VOID vow_init_sta(PRTMP_ADAPTER pad)
{
	UINT8 i;
	BOOLEAN ret;

	/* if ATF is disabled, the default max DWRR wait time is configured as 256us to force STA round-robin */
	if (pad->vow_cfg.en_airtime_fairness == FALSE)
		pad->vow_cfg.sta_max_wait_time = 1; /* 256us */

	/* set max wait time for DWRR station */
	ret = vow_set_sta_DWRR_max_time(pad);

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	/* station DWRR quantum */
	ret = vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

	/* per station DWRR configuration */
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_ALL);
		ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP);
		/* set station pause status */
		ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
	}
}

VOID vow_init_group(PRTMP_ADAPTER pad)
{
	BOOLEAN ret;

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	/* group DWRR quantum */
	ret = vow_set_group(pad, VOW_ALL_GROUP, ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL);

	/* set group configuration */
	ret = vow_set_group(pad, VOW_ALL_GROUP, ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP);

	/* set max BSS wait time and sta wait time */
	/* RTMP_IO_WRITE32(pad, 0x8374, 0x00200020); */
	vow_set_group_DWRR_max_time(pad);
	/*set Airtime estimator enable*/
	pad->vow_at_est.at_estimator_en = TRUE;
	vow_set_at_estimator(pad, ENUM_AT_PROC_EST_FEATURE_CTRL);
}

VOID vow_init_rx(PRTMP_ADAPTER pad)
{
	UINT8 i;
	BOOLEAN ret;

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE) {
		pad->vow_rx_time_cfg.rx_time_en = FALSE;
		vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
		return;
	}

	/* reset all RX counters */
	ret = vow_set_rx_airtime(pad, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
	/* RX airtime feature enable */
	ret = vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
	/* set ED offset */
	ret = vow_set_rx_airtime(pad, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);
	/* set OBSS backoff time - 1 set*/
	ret = vow_set_backoff_time(pad, ENUM_RX_AT_OBSS);
	/* set non QOS backoff time - 1 set */
	ret = vow_set_backoff_time(pad, ENUM_RX_AT_NON_QOS);
	/* set repeater backoff time - 1 set */
	ret = vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_PEPEATER);
	/* set OM backoff time */
	ret = vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_STA);

	/* set WMM AC backoff time */
	for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++)
		ret = vow_set_backoff_time(pad, i);

	/* set BSS belogs to which WMM set */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		ret = vow_set_mbss2wmm_map(pad, i);

	/* select RX WMM backoff time for 4 OM */
	for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++)
		ret = vow_set_wmm_selection(pad, i);

	/* configure airtime estimator */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
		vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, i);
		/* vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, i); */
	}
}


VOID vow_init_misc(PRTMP_ADAPTER pad)
{
	UINT32 reg32, i;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* disable RTS failed airtime charge for RTS deadlock */
		HW_IO_READ32(pad, AGG_SCR, &reg32);
		reg32 |= RTS_FAIL_CHARGE_DIS;
		HW_IO_WRITE32(pad, AGG_SCR, reg32);
	}

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* RX_RIFS_MODE enable, 820F4000 bit[23] set to 1 */
		/* detect ED signal down for CCK CTS */
		HW_IO_READ32(pad, TMAC_TCR, &reg32);
		reg32 |= RX_RIFS_MODE;
		HW_IO_WRITE32(pad, TMAC_TCR, reg32);

		/* Configure 1 to force rmac_cr_eifs_time=0 for VOW OBSS counter, 820f52e0 bit[21] set to 1 */
		HW_IO_READ32(pad, RMAC_RSVD0, &reg32);
		reg32 |= RX_EIFS_TIME_ZERO;
		HW_IO_WRITE32(pad, RMAC_RSVD0, reg32);

		/* for SER (0x82060370 bit[26]=1 --> keep all VOW setting) */
		HW_IO_READ32(pad, VOW_CONTROL, &reg32);
		reg32 |= VOW_RESET_DISABLE;
		HW_IO_WRITE32(pad, VOW_CONTROL, reg32);
	}

	if (vow_watf_is_enabled(pad)) {
		pad->vow_cfg.vow_sta_dwrr_quantum[0] = pad->vow_watf_q_lv0;
		pad->vow_cfg.vow_sta_dwrr_quantum[1] = pad->vow_watf_q_lv1;
		pad->vow_cfg.vow_sta_dwrr_quantum[2] = pad->vow_watf_q_lv2;
		pad->vow_cfg.vow_sta_dwrr_quantum[3] = pad->vow_watf_q_lv3;
	} else {
		pad->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
		pad->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
		pad->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
		pad->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
	}

	for (i = 0; i < VOW_WATF_LEVEL_NUM; i++)
		vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + i);
}

VOID vow_reset(PRTMP_ADAPTER pad)
{
	vow_reset_watf(pad);
	vow_reset_dvt(pad);
}

VOID vow_update_om_wmm(PRTMP_ADAPTER pad, struct wifi_dev *wdev, PEDCA_PARM pApEdcaParm)
{
	UCHAR wmm_idx;
	UCHAR st;
	UINT16 cw;
	UCHAR ac_idx;
	UCHAR ac_map[] = {WMM_AC_BE, WMM_AC_BK, WMM_AC_VI, WMM_AC_VO};
	struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	if (WMODE_CAP_5G(wdev->PhyMode))
		st = SLOT_TIME_5G;
	else if (pad->CommonCfg.bUseShortSlotTime)
		st = SLOT_TIME_24G_SHORT;
	else
		st = SLOT_TIME_24G_LONG;

	if (pBssEdca) {
		/* invalid */
		if (pBssEdca->bValid == FALSE)
			return;

		if ((pApEdcaParm == NULL) || (pApEdcaParm->bValid == FALSE))
			return;

		wmm_idx = pApEdcaParm->WmmSet;

		for (ac_idx = 0; ac_idx < WMM_NUM_OF_AC; ac_idx++) {
			cw = (1 << pBssEdca->Cwmin[ac_map[ac_idx]]) - 1;
			pad->vow_rx_time_cfg.wmm_backoff[wmm_idx][ac_idx] =
				(WMODE_CAP_5G(wdev->PhyMode) ? SIFS_TIME_5G : SIFS_TIME_24G) +
				pBssEdca->Aifsn[ac_map[ac_idx]]*st + cw*st;
		}

		vow_set_backoff_time(pad, wmm_idx);
	}
}

VOID vow_mbss_init(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	if (wdev) {
		vow_mbss_grp_band_map(pad, wdev);
		vow_mbss_wmm_map(pad, wdev);
		/* configure BCMC entry */
		vow_set_client(pad, wdev->wdev_idx, wdev->tr_tb_idx);
	}
}

VOID vow_group_band_map(PRTMP_ADAPTER pad, UCHAR band_idx, UCHAR group_idx)
{
	UINT32 reg32;

	HW_IO_READ32(pad, VOW_DBDC_BW_GROUP_CTRL, &reg32);
	reg32 &= ~(1 << group_idx);
	reg32 |= (band_idx << group_idx);
	HW_IO_WRITE32(pad, VOW_DBDC_BW_GROUP_CTRL, reg32);
}

/* do bss(group) and band mapping */
VOID vow_mbss_grp_band_map(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	UCHAR band_idx;
	UCHAR wdev_idx;

	if (wdev) {
		band_idx = HcGetBandByWdev(wdev);
		wdev_idx = wdev->wdev_idx;
		/* MBSS <--> group 1 to 1 mapping, ex: SSID0 --> Group0 */
		vow_group_band_map(pad, band_idx, wdev_idx);
		pad->vow_bss_cfg[wdev_idx].band_idx = band_idx;
		vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, wdev_idx);
	}
}

/* do bss and wmm mapping for RX */
VOID vow_mbss_wmm_map(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	UCHAR wmm_idx;

	if (wdev) {
		wmm_idx = HcGetWmmIdx(pad, wdev);
		pad->vow_rx_time_cfg.bssid2wmm_set[wdev->wdev_idx] = wmm_idx;
		vow_set_mbss2wmm_map(pad, wdev->wdev_idx);
	}
}

static UINT32 vow_get_availabe_airtime(VOID)
{
	return VOW_DEF_AVA_AIRTIME;
}

/* get rate token */
UINT16 vow_convert_rate_token(PRTMP_ADAPTER pad, UINT8 type, UINT8 group_id)
{
	UINT16 period, rate, token = 0;

	period = (1 << pad->vow_cfg.refill_period);

	if (type == VOW_MAX) {
		rate = pad->vow_bss_cfg[group_id].max_rate;
		token = (period * rate);
	} else {
		rate = pad->vow_bss_cfg[group_id].min_rate;
		token = (period * rate);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: period %dus, rate %u, token %u\n", __func__, period, rate, token));
	return token;
}

/* get airtime token */
UINT16 vow_convert_airtime_token(PRTMP_ADAPTER pad, UINT8 type, UINT8 group_id)
{
	UINT16 period, ratio, token = 0;
	UINT32 atime = vow_get_availabe_airtime();
	UINT64 tmp;

	period = (1 << pad->vow_cfg.refill_period);

	if (type == VOW_MAX)
		ratio = pad->vow_bss_cfg[group_id].max_airtime_ratio;
	else
		ratio = pad->vow_bss_cfg[group_id].min_airtime_ratio;

	/* shift 3 --> because unit is 1/8 us,
	   10^8 --> ratio needs to convert from integer to %, preiod needs to convert from us to s
	*/
	tmp = ((UINT64)period * atime * ratio) << 3;
	/* printk("%s: tmp %llu\n", __FUNCTION__, tmp); */
	token = div64_u64(tmp, 100000000);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: period %dus, ratio %u, available time %u, token %u\n", __func__, period, ratio, atime, token));
	return token;
}


/* add client(station) */

VOID vow_set_client(PRTMP_ADAPTER pad, UINT8 group, UINT8 sta_id)
{
	BOOLEAN ret;
	/* set group for station */
	pad->vow_sta_cfg[sta_id].group = group;
	/* update station bitmap */
	/* don't change command sequence - STA_BSS_GROUP will refer to STA_ALL's old setting */
	ret = vow_set_sta(pad, sta_id, ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP);
	ret = vow_set_sta(pad, sta_id, ENUM_VOW_DRR_CTRL_FIELD_STA_ALL);
	/* set station pause status */
	ret = vow_set_sta(pad, sta_id, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
}

INT set_vow_min_rate_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_rate_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_rate_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_airtime_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtime_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_airtime_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtime_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_rate_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_ratebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_ratebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_airtime_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtimebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_airtime_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtimebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_backlog_size(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_backlog_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_wait_time = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_group_dwrr_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pad->vow_cfg.group_max_wait_time = val;
			ret = vow_set_group_DWRR_max_time(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_pause(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			pad->vow_sta_cfg[sta].paused = val;
			vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: sta %d set %u.\n", __func__, sta, val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_sta_group(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			INT ret;

			pad->vow_sta_cfg[sta].group = val;
			ret = vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: sta %d group %u.\n", __func__, sta, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bw_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_bw_ctrl = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_refill_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_bw_refill = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_airtime_fairness_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_airtime_fairness = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));
#if defined (FQ_SCH_SUPPORT)
			if (pad->vow_cfg.en_airtime_fairness == 0)
				set_fq_enable(pad, "0-0");
#endif
			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_txop_switch_bss_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_txop_no_change_bss = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dbdc_search_rule(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, band;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &band, &val);

		if ((rv > 1)) {
			INT ret;

			if (band == 0)
				pad->vow_cfg.dbdc0_search_rule = val;
			else
				pad->vow_cfg.dbdc1_search_rule = val;

			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_refill_period(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.refill_period = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bss_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < 16)) {
			INT ret;

			pad->vow_cfg.per_bss_enable &= ~(1 << group);
			pad->vow_cfg.per_bss_enable |= (val << group);
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_dwrr_quantum(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, id;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &id, &val);

		if ((rv > 1) && (id < 8)) {
			INT ret;

			pad->vow_cfg.vow_sta_dwrr_quantum[id] = val;
			ret = vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + id);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set quantum id %u, val %d.\n", __func__, id, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_frr_quantum(
		IN	PRTMP_ADAPTER pad,
		IN	RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);
		if ((rv > 0) && (val <= 0xff)) {
			pad->vow_sta_frr_quantum = val;
			pad->vow_sta_frr_flag = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set FRR quantum %d.\n", __FUNCTION__, val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_airtime_ctrl_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < 16)) {
			INT ret;

			pad->vow_bss_cfg[group].at_on = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bw_ctrl_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < 16)) {
			INT ret;

			pad->vow_bss_cfg[group].bw_on = val;
			ret = vow_set_feature_all(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_ac_priority(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, sta;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (sta < MAX_LEN_OF_MAC_TABLE) && (val < 4)) {
			BOOLEAN ret;
			/* set AC change rule */
			pad->vow_sta_cfg[sta].ac_change_rule = val;
			ret = vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY);

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: sta %d W ENUM_VOW_DRR_PRIORITY_CFG_ITEM failed.\n", __func__, sta));
				return FALSE;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: sta %d W AC change rule %d.\n", __func__, sta, pad->vow_sta_cfg[sta].ac_change_rule));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_dwrr_quantum_id(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, sta, ac;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &sta, &ac, &val);

		if ((rv > 2) && (sta < MAX_LEN_OF_MAC_TABLE) && (ac < 4) && (val < 8)) {
			INT ret;

			pad->vow_sta_cfg[sta].dwrr_quantum[ac] = val;
			ret = vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID + ac);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set sta %d, ac %d, quantum id %u.\n", __func__, sta, ac, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bss_dwrr_quantum(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < 16)) {
			INT ret;

			pad->vow_bss_cfg[group].dwrr_quantum = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00 + group);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set group %d, quantum id %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dwrr_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.sta_max_wait_time = val;
			/* ret = vow_set_sta(pad, 0xFF, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND); */
			ret = vow_set_sta_DWRR_max_time(pad);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_rate(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_rate = val;
			pad->vow_bss_cfg[group].min_rate_token = vow_convert_rate_token(pad, VOW_MIN, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set rate %u\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_rate = val;
			pad->vow_bss_cfg[group].max_rate_token = vow_convert_rate_token(pad, VOW_MAX, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d set %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_ratio(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtime_ratio = val;
			pad->vow_bss_cfg[group].min_airtime_token = vow_convert_airtime_token(pad, VOW_MIN, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM);
			if (ret == 0) {
				ret = vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: group %d set %u.\n", __func__, group, val));
			}
			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_ratio(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtime_ratio = val;
			pad->vow_bss_cfg[group].max_airtime_token = vow_convert_airtime_token(pad, VOW_MAX, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM);
			if (ret == 0) {
				ret = vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: group %d set %u.\n", __func__, group, val));
			}
			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_counter_clr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_airtime_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pAd->vow_rx_time_cfg.rx_time_en = val;
			ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_ed_offset(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val < 32)) {
			INT ret;

			pAd->vow_rx_time_cfg.ed_offset = val;
			ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_rx_obss_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val <= 0xFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.obss_backoff = val;
			ret = vow_set_backoff_time(pAd, ENUM_RX_AT_OBSS);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_rx_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 wmm, ac, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &wmm, &ac, &val);

		if ((rv > 2) && (wmm < VOW_MAX_WMM_SET_NUM) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.wmm_backoff[wmm][ac] = val;
			ret = vow_set_backoff_time(pAd, wmm);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: wmm %d ac %d set %u.\n", __func__, wmm, ac, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_non_qos_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.non_qos_backoff = val;
			ret = vow_set_backoff_time(pAd, ENUM_RX_AT_NON_QOS);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_om_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 ac, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &ac, &val);

		if ((rv > 0) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.om_wmm_backoff[ac] = val;
			ret = vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_STA);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set ac %d, val = %u.\n", __func__, ac, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_repeater_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 ac, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &ac, &val);

		if ((rv > 0) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.repeater_wmm_backoff[ac] = val;
			ret = vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_PEPEATER);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set ac %d, val = %u.\n", __func__, ac, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_bss_wmmset(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss_idx, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &bss_idx, &val);

		if ((rv > 1) && (val < VOW_MAX_WMM_SET_NUM) && (bss_idx < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pAd->vow_rx_time_cfg.bssid2wmm_set[bss_idx] = val;
			ret = vow_set_mbss2wmm_map(pAd, bss_idx);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: bss_idx %d set %u.\n", __func__, bss_idx, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_om_wmm_select(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 om_idx, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &om_idx, &val);

		if ((rv > 1) && (om_idx < 4)) { /* FIXME: enum --> 4 */
			INT ret;

			pAd->vow_rx_time_cfg.wmm_backoff_sel[om_idx] = val;
			ret = vow_set_wmm_selection(pAd, om_idx);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: OM MAC index %d set %u.\n", __func__, om_idx, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

BOOLEAN halUmacVoWChargeBwToken(
	IN UINT_8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT_16 u2ChargeLenValue,
	IN UINT_16 u2ChargeTimeValue
)
{
	UINT32 reg;

	if ((ucBssGroup >= UMAC_BSS_GROUP_NUMBER) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = ((((UINT32)u2ChargeLenValue << UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK) |
		   (((UINT32)u2ChargeTimeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK));
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_BW_TOKEN_OP_MASK | (((UINT32)fgChargeMode << (ffs(UMAC_CHARGE_ADD_MODE_BIT_MASK) - 1)) & UMAC_CHARGE_ADD_MODE_BIT_MASK)) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((ucBssGroup << UMAC_CHARGE_MODE_BSS_GROUP_OFFSET) & UMAC_CHARGE_MODE_BSS_GROUP_MASK));
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}

BOOLEAN halUmacVoWChargeBwTokenLength(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue
)
{
	return halUmacVoWChargeBwToken(ucBssGroup, fgChargeMode, u2ChargeLenValue, 0);
}



BOOLEAN halUmacVoWChargeBwTokenTime(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeTimeValue
)
{
	return halUmacVoWChargeBwToken(ucBssGroup, fgChargeMode, 0, u2ChargeTimeValue);
}

BOOLEAN halUmacVoWChargeBwDrr(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue,
	IN UINT16 u2ChargeTimeValue
)
{
	UINT32 reg;

	if ((ucBssGroup >= UMAC_BSS_GROUP_NUMBER) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = ((((UINT32)u2ChargeLenValue << UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK) |
		   (((UINT32)u2ChargeTimeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK));
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_BW_DRR_OP_MASK | (fgChargeMode << UMAC_CHARGE_ADD_MODE_BIT_MASK)) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((ucBssGroup << UMAC_CHARGE_MODE_BSS_GROUP_OFFSET) & UMAC_CHARGE_MODE_BSS_GROUP_MASK));
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}

BOOLEAN halUmacVoWChargeBwDrrLength(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue
)
{
	return halUmacVoWChargeBwDrr(ucBssGroup, fgChargeMode, u2ChargeLenValue, 0);
}

BOOLEAN halUmacVoWChargeBwDrrTime(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeTimeValue
)
{
	return halUmacVoWChargeBwDrr(ucBssGroup, fgChargeMode, 0, u2ChargeTimeValue);
}


BOOLEAN halUmacVoWChargeAitTimeDRR(
	IN UINT8 ucStaID,
	IN UINT8 ucAcId,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeValue
)
{
	UINT32 reg;

	if ((ucStaID > UMAC_WLAN_ID_MAX_VALUE) ||
		(ucAcId >= 4) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = (((UINT32)u2ChargeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK);
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_AIRTIME_DRR_OP_MASK | (((UINT32)fgChargeMode << (ffs(UMAC_CHARGE_ADD_MODE_BIT_MASK)-1) & UMAC_CHARGE_ADD_MODE_BIT_MASK))) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((ucStaID << UMAC_CHARGE_MODE_STA_ID_OFFSET) & UMAC_CHARGE_MODE_STA_ID_MASK) |
		   ((ucAcId << UMAC_CHARGE_MODE_QUEUE_ID_OFFSET) & UMAC_CHARGE_MODE_QUEUE_ID_MASK));
	RTMP_IO_WRITE32(pvow_pad, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}



INT set_vow_charge_sta_dwrr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, ac, mode, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d-%d", &sta, &mode, &ac, &val);

		if ((rv > 3) && (sta < MAX_LEN_OF_MAC_TABLE) && (ac < WMM_NUM_OF_AC)) {
			halUmacVoWChargeAitTimeDRR(sta, ac, mode, val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: sta%d/ac%d %c charge--> %u.\n", __func__, sta, ac, mode == 0 ? 'd' : 'a', val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_time(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwTokenTime(group, mode, val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_len(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwTokenLength(group, mode, val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_dwrr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwDrrTime(group, mode, val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_sta_psm(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, psm, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &psm);

		if ((rv > 1) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			UINT32 offset, reg;
			/* clear PSM update mask bit31 */
			RTMP_IO_WRITE32(pAd, 0x23000, 0x80000000);
			/* set PSM bit in WTBL DW3 bit 30 */
			offset = (sta << 8) | 0x3000c;
			RTMP_IO_READ32(pAd, offset, &reg);

			if (psm) {
				reg |= 0x40000000;
				RTMP_IO_WRITE32(pAd, offset, reg);
			} else {
				reg &= ~0x40000000;
				RTMP_IO_WRITE32(pAd, offset, reg);
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: sta%d psm--> %u.\n", __func__, sta, psm));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}



INT set_vow_monitor_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			pAd->vow_monitor_sta = sta;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: monitor sta%d.\n", __func__, sta));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_monitor_bss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss <= 16)) {
			pAd->vow_monitor_bss = bss;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: monitor bss%d.\n", __func__, bss));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_monitor_mbss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss < 16)) {
			pAd->vow_monitor_mbss = bss;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: monitor mbss%d.\n", __func__, bss));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_avg_num(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 num, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &num);

		if ((rv > 0) && (num < 1000)) {
			pAd->vow_avg_num = num;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: average numer %d.\n", __func__, num));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dvt_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 en, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if (rv > 0) {
			pAd->vow_dvt_en = en;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: DVT enable %d.\n", __func__, pAd->vow_dvt_en));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_help(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== Group table =========\n"
			  "vow_min_rate_token = <group>-<token>\n"
			  "vow_max_rate_token = <group>-<token>\n"
			  "vow_min_airtime_token = <group>-<token>\n"
			  "vow_max_airtime_token = <group>-<token>\n"
			  "vow_min_rate_bucket = <group>-<byte> 1K\n"
			  "vow_max_rate_bucket = <group>-<byte> 1K\n"
			  "vow_min_airtime_bucket = <group>-<time> 1.024\n"
			  "vow_max_airtime_bucket = <group>-<time> 1.024\n"
			  "vow_max_wait_time = <group>-<time> 1.024\n"
			  "vow_max_backlog_size = <group>-<byte> 1K\n"
			  "======== Control =============\n"
			  "vow_bw_enable = <0/1> 0:disable, 1:enable\n"
			  "vow_refill_en = <0/1> 0:disable, 1:enable\n"
			  "vow_airtime_fairness_en = <0/1> 0:disable, 1:enable\n"
			  "vow_txop_switch_bss_en = <0/1> 0:disable, 1:enable\n"
			  "vow_dbdc_search_rule = <band>-<0/1> 0:WMM AC, 1:WMM set\n"
			  "vow_refill_period = <n> 2^n\n"
			  "vow_bss_enable = <group>-<0/1> 0:disable, 1:enable\n"
			  "vow_airtime_control_en = <group>-<0/1> 0:disable, 1:enable\n"
			  "vow_bw_control_en = <group>-<0/1> 0:disable, 1:enable\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== Group others =============\n"
			  "vow_bss_dwrr_quantum = <group>-<time> 256us\n"
			  "vow_group_dwrr_max_wait_time = <time> 256us\n"
			  "vow_group2band_map = <group>-<band>\n"
			  "======== Station table =============\n"
			  "vow_sta_dwrr_quantum = <Qid>-<val> 256us\n"
			  "vow_sta_dwrr_quantum_id = <wlanidx>-<WMMA AC>-<Qid>\n"
			  "vow_sta_ac_priority = <wlanidx>-<0/1/2> 0:disable, 1:BE, 2:BK\n"
			  "vow_sta_pause = <wlanidx>-<0/1> 0: normal, 1: pause\n"
			  "vow_sta_psm = <wlanidx>-<0/1> 0: normal, 1: power save\n"
			  "vow_sta_group = <wlanidx>-<group>\n"
			  "vow_dwrr_max_wait_time = <time> 256us\n"
			  "======== User Config =============\n"
			  "vow_min_rate = <group>-<Mbps>\n"
			  "vow_max_rate = <group>-<Mbps>\n"
			  "vow_min_ratio = <group>-<%%>\n"
			  "vow_max_ratio = <group>-<%%>\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== Rx Config =============\n"
			  "vow_rx_counter_clr = <n>\n"
			  "vow_rx_airtime_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_rx_ed_offset = <val> 1.024(5b)\n"
			  "vow_rx_obss_backoff = <val> 1.024(16b)\n"
			  "vow_rx_wmm_backoff = <WMM set>-<WMM AC>-<val>\n"
			  "vow_om_wmm_backoff = <WMM AC>-<val>\n"
			  "vow_repeater_wmm_backoff = <WMM AC>-<val>\n"
			  "vow_rx_non_qos_backoff = <val>\n"
			  "vow_rx_bss_wmmset = <MBSS idx>-<0/1/2/3>\n"
			  "vow_rx_om_wmm_sel = <OM idx>-<val> 0:RX WMM(1to1), 1:OM wmm\n"
			  "======== Airtime estimator =============\n"
			  "vow_at_est_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_at_mon_period = <period> ms\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== Badnode detector =============\n"
			  "vow_bn_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_bn_mon_period = <period> ms\n"
			  "vow_bn_fallback_th = <count>\n"
			  "vow_bn_per_th = <TX PER>\n"
			  "======== Airtime counter test =============\n"
			  "vow_counter_test = <0/1> 0:dieable, 1:enable\n"
			  "vow_counter_test_period = <period> ms\n"
			  "vow_counter_test_band = <band>\n"
			  "vow_counter_test_avgcnt = <average num> sec\n"
			  "vow_counter_test_target = <wlanidx>\n"
			  "======== DVT =============\n"
			  "vow_dvt_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_monitor_sta = <STA num>\n"
			  "vow_show_sta = <STA num>\n"
			  "vow_monitor_bss = <BSS num>\n"
			  "vow_monitor_mbss = <MBSS num>\n"
			  "vow_show_mbss = <MBSS num>\n"
			  "vow_avg_num = <average num> sec\n"
			  "======== RED ===========\n"
			  "vow_set_red_en = <0/1> 0:disable, 1:enable\n"
			  "vow_set_red_show_sta = <STA num>\n"
			  "vow_set_red_tar_delay = <tarDelay> us\n"
			  "======== WATF ===========\n"
			  "vow_watf_en = <0/1> 0:disable, 1:enable\n"
			  "vow_watf_q = <level>-<quantum> unit 256us\n"
			  "vow_watf_add_entry = <level>-<Addr>\n"
			  "vow_watf_del_entry = <Addr>\n"
			 ));
	return TRUE;
}


/* commands for show */
INT set_vow_show_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (sta < 127)) {
			pAd->vow_show_sta = sta;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: show station up to %d.\n", __func__, pAd->vow_show_sta));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_show_mbss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 mbss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &mbss);

		if ((rv > 0) && (mbss <= 16)) {
			pAd->vow_show_mbss = mbss;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: show MBSS up to %d.\n", __func__, pAd->vow_show_mbss));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_dump_vow(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	vow_dump_umac_CRs(pAd);
	return TRUE;
}

INT show_vow_dump_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val1, val2, val3, val4, val;

	for (sta = pAd->vow_gen.VOW_STA_SETTING_BEGIN; sta < pAd->vow_gen.VOW_STA_SETTING_END; sta++) {
		val = 0x80220000;
		val |= sta;
		RTMP_IO_WRITE32(pAd, 0x8388, val);
		RTMP_IO_READ32(pAd, 0x8350, &val1);
		RTMP_IO_READ32(pAd, 0x8354, &val2);
		RTMP_IO_READ32(pAd, 0x8358, &val3);
		RTMP_IO_READ32(pAd, 0x835c, &val4);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("STA%d~%d: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n",
					(sta - pAd->vow_gen.VOW_STA_SETTING_BEGIN) << pAd->vow_gen.VOW_STA_SETTING_FACTOR,
					((sta - pAd->vow_gen.VOW_STA_SETTING_BEGIN + 1) << pAd->vow_gen.VOW_STA_SETTING_FACTOR) - 1,
					val1, val2, val3, val4));
	}

	return TRUE;
}

INT show_vow_dump_bss_bitmap(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1, val2, val3, val4, val;

	for (group = pAd->vow_gen.VOW_STA_BITMAP_BEGIN; group < pAd->vow_gen.VOW_STA_BITMAP_END; group++) {
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
			val = 0x80440000;
		else
			val = 0x80220000; /* move from BSS sysram to STA sysram */

		val |= group;
		RTMP_IO_WRITE32(pAd, 0x8388, val);
		RTMP_IO_READ32(pAd, 0x8350, &val1);
		RTMP_IO_READ32(pAd, 0x8354, &val2);
		RTMP_IO_READ32(pAd, 0x8358, &val3);
		RTMP_IO_READ32(pAd, 0x835c, &val4);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Group%d BitMap: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n", group, val1, val2, val3, val4));
	}

	return TRUE;
}

INT show_vow_dump_bss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1, val2, val3, val4, val;

	for (group = VOW_BSS_SETTING_BEGIN; group < VOW_BSS_SETTING_END; group++) {
		val = 0x80440000;
		val |= group;
		RTMP_IO_WRITE32(pAd, 0x8388, val);
		RTMP_IO_READ32(pAd, 0x8350, &val1);
		RTMP_IO_READ32(pAd, 0x8354, &val2);
		RTMP_IO_READ32(pAd, 0x8358, &val3);
		RTMP_IO_READ32(pAd, 0x835c, &val4);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Group%d config: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n", group - 16, val1, val2, val3, val4));
	}

	return TRUE;
}

INT vow_show_bss_atoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1, val2, val3, val4, val;
	UINT32 rv;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < 16)) {
			val = 0x80440000;
			val |= (group + pAd->vow_gen.VOW_BSS_TOKEN_OFFSET);
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8350, &val1);
			RTMP_IO_READ32(pAd, 0x8354, &val2);
			RTMP_IO_READ32(pAd, 0x8358, &val3);
			RTMP_IO_READ32(pAd, 0x835c, &val4);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Group%d airtime token: max 0x%08X, min 0x%08X.\n", group, val2, val1));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_bss_ltoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1, val2, val3, val4, val;
	UINT32 rv;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < 16)) {
			val = 0x80440000;
			val |= (group + pAd->vow_gen.VOW_BSS_TOKEN_OFFSET);
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8350, &val1);
			RTMP_IO_READ32(pAd, 0x8354, &val2);
			RTMP_IO_READ32(pAd, 0x8358, &val3);
			RTMP_IO_READ32(pAd, 0x835c, &val4);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Group%d length token: max 0x%08X, min 0x%08X\n", group, val4, val3));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_bss_dtoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1_ori, val1_sign_ext, val2_ori, val2_sign_ext, val;
	UINT32 rv;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < 16)) {
			if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
				val = 0x80460000;
			else
				val = 0x80480000;
			val |= group;
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8358, &val1_ori);
			RTMP_IO_READ32(pAd, 0x835c, &val2_ori);

			val1_sign_ext = val1_ori;
			val2_sign_ext = val2_ori;

			if (val1_ori >> 17)
				val1_sign_ext = ~(0x3FFFF - val1_ori) + 1;
			if (val2_ori >> 17)
				val2_sign_ext = ~(0x3FFFF - val2_ori) + 1;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Group%d airtime token: max %d(0x%08X), min %d(0x%08X)\n",
				group, val1_sign_ext, val1_ori, val2_sign_ext, val2_ori));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_sta_dtoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val1, val2, val3, val4, val;
	UINT32 val1_sign_ext, val2_sign_ext, val3_sign_ext, val4_sign_ext;
	UINT32 rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			val = 0x80000000;
			val |= (sta << 2);
			/* BK */
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8358, &val1);
			/* BE */
			val |= 1;
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8358, &val2);
			/* VI */
			val |= 2;
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8358, &val3);
			/* VO */
			val |= 3;
			RTMP_IO_WRITE32(pAd, 0x8388, val);
			RTMP_IO_READ32(pAd, 0x8358, &val4);

			val1_sign_ext = val1;
			val2_sign_ext = val2;
			val3_sign_ext = val3;
			val4_sign_ext = val4;

			if (val1 >> 17)
				val1_sign_ext = ~(0x3FFFF - val1) + 1;
			if (val2 >> 17)
				val2_sign_ext = ~(0x3FFFF - val2) + 1;
			if (val3 >> 17)
				val3_sign_ext = ~(0x3FFFF - val3) + 1;
			if (val4 >> 17)
				val4_sign_ext = ~(0x3FFFF - val4) + 1;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Sta%d deficit token: ac0 %d(0x%08X), ac1 %d(0x%08X), ac2 %d(0x%08X), ac3 %d(0x%08X)\n",
				sta, val1_sign_ext, val1, val2_sign_ext, val2, val3_sign_ext, val3, val4_sign_ext, val4));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT show_vow_rx_time(
	IN  RTMP_ADAPTER * pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 counter[4];

	counter[0] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 0);
	counter[1] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 1);
	counter[2] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 0);
	counter[3] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 1);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: nonwifi %u/%u, obss %u/%u.\n", __func__, counter[0], counter[1], counter[2], counter[3]));
	return TRUE;
}

INT show_vow_sta_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (sta < MAX_LEN_OF_MAC_TABLE)) {
			UINT32 pri, q;
			CHAR * pri_str[] = {"No change.", "To BE.", "To BK."};

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: ************ sta%d ***********\n", __func__, sta));
			/* group */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Group --> %u\n", pAd->vow_sta_cfg[sta].group));
			/* priority */
			pri = pAd->vow_sta_cfg[sta].ac_change_rule;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Priority --> %s(%u)\n", pri_str[pri], pri));
			/* airtime quantum for AC */
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_BK];
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Ac0 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q));
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_BE];
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Ac1 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q));
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_VI];
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Ac2 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q));
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_VO];
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Ac3 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_all_sta_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 i;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		CHAR str[4];

		sprintf(str, "%d", i);
		show_vow_sta_conf(pAd, str);
	}

	return TRUE;
}

INT show_vow_bss_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss < VOW_MAX_GROUP_NUM)) {
			UINT32 val;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: ************** Group%d **********\n", __func__, bss));
			/* per BSS bw control */
			val = pAd->vow_cfg.per_bss_enable & (1UL << bss);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("BW control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val));
			/* per BSS airtime control */
			val = pAd->vow_bss_cfg[bss].at_on;
			/* val = halUmacVoWGetDisablePerBssCheckTimeTokenFeature(bss); */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Airtime control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val));
			/* per BSS TP control */
			/* val = halUmacVoWGetDisablePerBssCheckLengthTokenFeature(bss); */
			val = pAd->vow_bss_cfg[bss].bw_on;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Rate control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val));
			/* Rate Setting */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Rate --> %u/%uMbps\n", pAd->vow_bss_cfg[bss].max_rate, pAd->vow_bss_cfg[bss].min_rate));
			/* Airtime Setting */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Airtime ratio --> %u/%u %%\n", pAd->vow_bss_cfg[bss].max_airtime_ratio, pAd->vow_bss_cfg[bss].min_airtime_ratio));
			/* Rate token */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Rate token --> %u Byte(%u)/%u Byte(%u)\n",
					  (pAd->vow_bss_cfg[bss].max_rate_token >> 3),
					  pAd->vow_bss_cfg[bss].max_rate_token,
					  (pAd->vow_bss_cfg[bss].min_rate_token >> 3),
					  pAd->vow_bss_cfg[bss].min_rate_token));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Rate bucket --> %u Byte/%u Byte\n",
					  pAd->vow_bss_cfg[bss].max_ratebucket_size << 10,
					  pAd->vow_bss_cfg[bss].min_ratebucket_size << 10));
			/* Airtime token */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Airtime token --> %u us(%u)/%u us(%u)\n",
					  (pAd->vow_bss_cfg[bss].max_airtime_token >> 3),
					  pAd->vow_bss_cfg[bss].max_airtime_token,
					  (pAd->vow_bss_cfg[bss].min_airtime_token >> 3),
					  pAd->vow_bss_cfg[bss].min_airtime_token));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Airtime bucket --> %u us/%u us\n",
					  pAd->vow_bss_cfg[bss].max_airtimebucket_size << 10,
					  pAd->vow_bss_cfg[bss].min_airtimebucket_size << 10));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_all_bss_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 i;

	for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
		CHAR str[4];

		sprintf(str, "%d", i);
		show_vow_bss_conf(pAd, str);
	}

	return TRUE;
}

INT vow_show_queue_status(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{

	return TRUE;
}

INT show_vow_help(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_rx_time (Non-wifi/OBSS)\n"
			  "vow_sta_conf = <wlanidx>\n"
			  "vow_sta_conf\n"
			  "vow_bss_conf = <group>\n"
			  "vow_all_bss_conf\n"
			  "vow_dump_sta (raw)\n"
			  "vow_dump_bss_bitmap (raw)\n"
			  "vow_dump_bss (raw)\n"
			  "vow_dump_vow (raw)\n"
			  "vow_show_sta_dtoken = <wlanidx> DWRR\n"
			  "vow_show_bss_dtoken = <group> DWRR\n"
			  "vow_show_bss_atoken = <group> airtime\n"
			  "vow_show_bss_ltoken = <group> length\n"));
	return TRUE;
}
/* airtime estimator */
INT set_vow_at_est_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_at_est.at_estimator_en = val;
			ret = vow_set_at_estimator(pAd, ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: value %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_at_mon_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_at_est.at_monitor_period = val;
			ret = vow_set_at_estimator(pAd, ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: period %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_group2band_map(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, group, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < 16)) { /*  */
			INT ret;

			pAd->vow_bss_cfg[group].band_idx = val;
			ret = vow_set_at_estimator_group(pAd, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, group);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: group %d, band %u.\n", __func__, group, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}
/* bad node detetor */
INT set_vow_bn_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_badnode.bn_en = val;
			ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_FEATURE_CTRL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: value %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_mon_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pAd->vow_badnode.bn_monitor_period = val;
			ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_MONITOR_PERIOD_CTRL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: period %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_fallback_th(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_badnode.bn_fallback_threshold = val;
			ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_FALLBACK_THRESHOLD);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: period %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_per_th(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pAd->vow_badnode.bn_per_threshold = val;
			ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_PER_THRESHOLD);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: per %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_counter_test_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 on, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &on);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 1, on);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: on = %d\n", __func__, on));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 period, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &period);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 2, period);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: period = %d\n", __func__, period));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_band(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 band, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &band);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 3, band);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: band = %d\n", __func__, band));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_avgcnt(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 avgcnt, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &avgcnt);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 4, avgcnt);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: avgcnt = %d\n", __func__, avgcnt));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_target(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 target, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &target);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 5, target);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: target = %d\n", __func__, target));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_watf_info(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	VOW_WATF	*pwatf;
	UINT8		macAddr[MAC_ADDR_LEN];
	INT			level, Num;

	pwatf = &pAd->vow_watf_mac[0];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== WATF Information ========\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_watf_en: %d\n", pAd->vow_watf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_watf_q_lv0: %d\n", pAd->vow_watf_q_lv0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_watf_q_lv1: %d\n", pAd->vow_watf_q_lv1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_watf_q_lv2: %d\n", pAd->vow_watf_q_lv2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("vow_watf_q_lv3: %d\n", pAd->vow_watf_q_lv3));

	for (level = 0; level < VOW_WATF_LEVEL_NUM; level++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("======== WATF LV%d's MAC Address List ========\n", level));

		for (Num = 0; Num < pwatf->Num; Num++) {
			NdisMoveMemory(&macAddr, pwatf->Entry[Num].Addr, MAC_ADDR_LEN);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Entry %d: %02x:%02x:%02x:%02x:%02x:%02x\n",
					  Num, macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
		}

		pwatf++;
	}

	return TRUE;
}


INT set_vow_watf_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, ret;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			pAd->vow_watf_en = val;
			ret = vow_set_feature_all(pAd);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: vow_watf_en is set to %u.\n", __func__, val));

			if (ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set command failed.\n", __func__));
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_watf_q(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT8		*pwatf_string;
	UINT32		val, rv, level;

	if (arg && vow_watf_is_enabled(pAd)) {
		rv = sscanf(arg, "%d-%d", &level, &val);

		if (rv > 1) {
			os_alloc_mem(NULL, (UCHAR **)&pwatf_string, sizeof(32));

			switch (level) {
			case 0: {
				pAd->vow_watf_q_lv0 =  val;
				sprintf(pwatf_string, "%d-%d", 0, pAd->vow_watf_q_lv0);
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vow_watf_q_lv0 is set to %d\n", pAd->vow_watf_q_lv0));
				break;
			}

			case 1: {
				pAd->vow_watf_q_lv1 =  val;
				sprintf(pwatf_string, "%d-%d", 1, pAd->vow_watf_q_lv1);
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vow_watf_q_lv1 is set to %d\n", pAd->vow_watf_q_lv1));
				break;
			}

			case 2: {
				pAd->vow_watf_q_lv2 =  val;
				sprintf(pwatf_string, "%d-%d", 2, pAd->vow_watf_q_lv2);
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vow_watf_q_lv2 is set to %d\n", pAd->vow_watf_q_lv2));
				break;
			}

			case 3: {
				pAd->vow_watf_q_lv3 =  val;
				sprintf(pwatf_string, "%d-%d", 3, pAd->vow_watf_q_lv3);
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vow_watf_q_lv3 is set to %d\n", pAd->vow_watf_q_lv3));
				break;
			}

			default:
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vow_watf_q_lv is setting fail.\n"));
			}

			if (pwatf_string != NULL)
				os_free_mem(pwatf_string);
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Wrong format, vow_watf_q=[Level]-[Quantum]\n"
					  "[Level] should be among 0 to 3 !\n"
					  "[Quantum] unit is 256us.\n"));
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_watf_add_entry(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32			rv, level;
	UINT32			tmp_macAddr[MAC_ADDR_LEN];
	UINT8			macAddr[MAC_ADDR_LEN];
	VOW_WATF		*pwatf, *ptmpwatf;
	INT			i, j;
	BOOLEAN		isDuplicate = FALSE;

	if (arg) {
		rv = sscanf(arg, "%d-%02x:%02x:%02x:%02x:%02x:%02x",
					&level, &tmp_macAddr[0], &tmp_macAddr[1], &tmp_macAddr[2], &tmp_macAddr[3], &tmp_macAddr[4], &tmp_macAddr[5]);
		pwatf = &pAd->vow_watf_mac[level];
		ptmpwatf = &pAd->vow_watf_mac[0];

		if ((rv == 7) && (level < VOW_WATF_LEVEL_NUM) && (level >= 0)) {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				macAddr[i] = tmp_macAddr[i];

			for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
				for (j = 0; j < ptmpwatf->Num; j++)
					if (memcmp(ptmpwatf->Entry[j].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
						isDuplicate = TRUE;
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 ("This MAC Address %02x:%02x:%02x:%02x:%02x:%02x is duplicate.\n",
								  macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
						break;
					}

				ptmpwatf++;
			}

			if (!isDuplicate) {
				NdisMoveMemory(pwatf->Entry[pwatf->Num++].Addr, &macAddr, MAC_ADDR_LEN);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("The entry Level %d - %02x:%02x:%02x:%02x:%02x:%02x is set complete!\n",
						  level, macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
			}
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Wrong format, vow_watf_add_entry=[Level]-[Addr]:[Addr]:[Addr]:[Addr]:[Addr]:[Addr]\n"
					  "[Level] should be among 0 to 3 !\n"));
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_watf_del_entry(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32			rv, level;
	UINT32			tmp_macAddr[MAC_ADDR_LEN];
	UINT8			macAddr[MAC_ADDR_LEN];
	UINT8			nullAddr[MAC_ADDR_LEN];
	VOW_WATF		*pwatf;
	INT			i;
	INT				j = 0;
	BOOLEAN		isFound = FALSE;

	if (arg) {
		rv = sscanf(arg, "%d-%02x:%02x:%02x:%02x:%02x:%02x",
					&level, &tmp_macAddr[0], &tmp_macAddr[1], &tmp_macAddr[2], &tmp_macAddr[3], &tmp_macAddr[4], &tmp_macAddr[5]);
		pwatf = &pAd->vow_watf_mac[level];
		NdisZeroMemory(nullAddr, MAC_ADDR_LEN);

		if ((rv == 7) && (level < VOW_WATF_LEVEL_NUM) && (level >= 0)) {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				macAddr[i] = tmp_macAddr[i];

			for (i = 0; i < pwatf->Num; i++) {
				if (memcmp(pwatf->Entry[i].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
					isFound = TRUE;
					NdisZeroMemory(pwatf->Entry[i].Addr, MAC_ADDR_LEN);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("The entry %02x:%02x:%02x:%02x:%02x:%02x founded will be deleted!\n",
							  macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
					break;
				}
			}

			if (!isFound) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("The entry %02x:%02x:%02x:%02x:%02x:%02x is not in the list!\n",
						  macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]));
			} else {
				for (i = 0; i < pwatf->Num; i++) {
					if (memcmp(pwatf->Entry[i].Addr, &nullAddr, MAC_ADDR_LEN) == 0)
						continue;
					else
						NdisMoveMemory(&(pAd->vow_watf_mac[level].Entry[j++].Addr), pwatf->Entry[i].Addr, MAC_ADDR_LEN);
				}

				pwatf->Num--;
			}
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Wrong format, vow_watf_add_entry=[Level]-[Addr]:[Addr]:[Addr]:[Addr]:[Addr]:[Addr]\n"
					  "[Level] should be among 0 to 3 !\n"));
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

VOID set_vow_watf_sta_dwrr(
	PRTMP_ADAPTER pAd,
	UINT8 *Addr,
	UINT8 Wcid)
{
	VOW_WATF		*pwatf;
	UINT8			*pwatf_string;
	UINT8			i, j;
	UINT8			level = 0;
	BOOLEAN		isFound = FALSE;

	if (vow_watf_is_enabled(pAd)) {
		pwatf = &pAd->vow_watf_mac[0];
		os_alloc_mem(NULL, (UCHAR **)&pwatf_string, sizeof(32));

		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			for (j = 0; j < pwatf->Num; j++) {
				if (memcmp(pwatf->Entry[j].Addr, Addr, MAC_ADDR_LEN) == 0) {
					isFound = TRUE;
					level = i;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("This MAC Address %02x:%02x:%02x:%02x:%02x:%02x is found in list.\n",
							  Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));
					break;
				}
			}

			pwatf++;
		}

		if (isFound) {
			for (i = 0; i < 4; i++)
				pAd->vow_sta_cfg[Wcid].dwrr_quantum[i] = level;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Update STA %d's DWRR quantum with LV%d\n", Wcid, level));
		} else {
			for (i = 0; i < 4; i++)
				pAd->vow_sta_cfg[Wcid].dwrr_quantum[i] = level;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Update STA %d's DWRR quantum with default LV%d\n", Wcid, level));
		}

		if (pwatf_string != NULL)
			os_free_mem(pwatf_string);
	}
}


VOID vow_reset_watf(
	IN PRTMP_ADAPTER pad)
{
	/* When interface down, it will reset WATF table. */
	VOW_WATF	*pwatf;
	INT	i, j;

	if (pad->vow_watf_en) {
		pwatf = &pad->vow_watf_mac[0];

		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			for (j = 0; j < pwatf->Num; j++)
				NdisZeroMemory(pwatf->Entry[j].Addr, MAC_ADDR_LEN);

			pwatf->Num = 0;
			pwatf++;
		}
	}
}


INT show_vow_info(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== VOW Control Information ========\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ATC Enbale: %d\n", pAd->vow_cfg.en_bw_ctrl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ATF Enbale: %d\n", pAd->vow_cfg.en_airtime_fairness));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("WATF Enable: %d\n", pAd->vow_watf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("en_bw_refill: %d\n", pAd->vow_cfg.en_bw_refill));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("en_txop_no_change_bss: %d\n", pAd->vow_cfg.en_txop_no_change_bss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dbdc0_search_rule: %d\n", pAd->vow_cfg.dbdc0_search_rule));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("dbdc1_search_rule: %d\n", pAd->vow_cfg.dbdc1_search_rule));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("refill_period: %d\n", pAd->vow_cfg.refill_period));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== VOW Max Deficit Information ========\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("VOW Max Deficit(unit 256us): %d\n", pAd->vow_cfg.sta_max_wait_time));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== VOW Quantum Information ========\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Quantum ID 0 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Quantum ID 1 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Quantum ID 2 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Quantum ID 3 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[3]));
	return TRUE;
}

VOID vow_reset_dvt(
	IN PRTMP_ADAPTER pad)
{
	/* When interface down, it will reset DVT parameters. */
	pad->vow_dvt_en = 0;
	NdisZeroMemory(vow_tx_time, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_rx_time, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_tx_ok, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_tx_fail, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_last_tx_time, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_last_rx_time, MAX_LEN_OF_MAC_TABLE);
	NdisZeroMemory(vow_tx_bss_byte, WMM_NUM_OF_AC);
	NdisZeroMemory(vow_rx_bss_byte, WMM_NUM_OF_AC);
	NdisZeroMemory(vow_tx_mbss_byte, VOW_MAX_GROUP_NUM);
	NdisZeroMemory(vow_rx_mbss_byte, VOW_MAX_GROUP_NUM);
	vow_sum_tx_rx_time = 0;
	vow_avg_sum_time = 0;
	vow_idx = 0;
	vow_ampdu_cnt = 0;
	vow_interval = 0;
	vow_last_free_cnt = 0;
}

VOID vow_display_info_periodic(
	IN  PRTMP_ADAPTER pAd)
{
	if (pAd->vow_dvt_en) {
		CHAR i, ac, bw;
		UINT32 wtbl_offset;
		UINT32 tx_sum, tx, rx_sum, rx, tx_ok[2], tx_fail[2], tx_cnt, tx_ok_sum, tx_fail_sum, tx_diff_time, rx_diff_time;
		UINT32 cnt, free_cnt;
		UINT32 ple_stat[17] = {0}, pg_flow_ctrl[6] = {0};
		UINT32 sta_pause[4] = {0}, dis_sta_map[4] = {0};
		INT32 k, l;
		UINT32 counter[2];
		UINT32 addr, phymode, rate, DW6;

		vow_idx++;

		/* airtime */
		for (i = 0; i <= pAd->vow_monitor_sta; i++) {
			wtbl_offset = (i << 8) | 0x3004C;
			tx_sum = rx_sum = 0;

			for (ac = 0; ac < 4; ac++) {
				RTMP_IO_READ32(pAd, wtbl_offset + (ac << 3), &tx);
				tx_sum += tx;
				RTMP_IO_READ32(pAd, wtbl_offset + (ac << 3) + 4, &rx);
				rx_sum += rx;
			}

			/* clear WTBL airtime statistic */
			tx_diff_time = tx_sum - vow_last_tx_time[i];
			rx_diff_time = rx_sum - vow_last_rx_time[i];
			vow_tx_time[i] += tx_diff_time;
			vow_rx_time[i] += rx_diff_time;
#ifdef CUSTOMER_DCC_FEATURE
			{
				PMAC_TABLE_ENTRY pEntry = NULL;
				if (VALID_UCAST_ENTRY_WCID(pAd, i))
				pEntry = &pAd->MacTab.Content[i];
				if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && pEntry->pMbss) {
					pEntry->ChannelUseTime += tx_diff_time + rx_diff_time;
					pEntry->pMbss->ChannelUseTime += tx_diff_time + rx_diff_time;
				}
			}
#endif

			vow_last_tx_time[i] = tx_sum;
			vow_last_rx_time[i] = rx_sum;
			vow_sum_tx_rx_time += tx_diff_time + rx_diff_time;
			vow_avg_sum_time += tx_diff_time + rx_diff_time;
			wtbl_offset = (i << 8) | 0x30000;
			RTMP_IO_READ32(pAd, wtbl_offset, &addr);
			addr = addr & 0xffff;
			wtbl_offset = (i << 8) | 0x30018;
			RTMP_IO_READ32(pAd, wtbl_offset, &DW6);
			phymode = (DW6 & 0x1c0) >> 6;
			rate = DW6 & 0x3f;

			if (i <= pAd->vow_show_sta) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("sta%d: tx -> %u, rx -> %u, vow_idx %d\n",
						 i, tx_diff_time, rx_diff_time, vow_idx));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("sta%d: addr %x:%x, Mode %d, MCS %d, vow_idx %d\n",
						 i, (addr & 0xff), (addr >> 8), phymode, rate, vow_idx));
			}

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AVG sta%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						 vow_tx_time[i]/pAd->vow_avg_num,
						 vow_tx_time[i],
						 vow_rx_time[i]/pAd->vow_avg_num,
						 vow_rx_time[i]));
				vow_avg_sum_time = 0;
				vow_tx_time[i] = 0;
				vow_rx_time[i] = 0;
			}
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Airtime: %u\n", vow_sum_tx_rx_time));
		vow_sum_tx_rx_time = 0;

		/* tx counter */
		for (i = 1; i <= pAd->vow_monitor_sta; i++) {
			wtbl_offset = (i << 8) | 0x30040;
			tx_ok_sum = tx_fail_sum = 0;

			for (bw = 0; bw < 2; bw++) {
				RTMP_IO_READ32(pAd, wtbl_offset + (bw << 2), &tx_cnt);
				tx_ok_sum += (tx_cnt & 0xffff);
				tx_ok[bw] = (tx_cnt & 0xffff);
				tx_fail_sum += ((tx_cnt >> 16) & 0xffff);
				tx_fail[bw] = ((tx_cnt >> 16) & 0xffff);
			}

			if (i <= pAd->vow_show_sta) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("sta%d: tx cnt -> %u/%u, tx fail -> %u/%u, vow_idx %d\n",
						 i, tx_ok[0], tx_ok[1], tx_fail[0], tx_fail[1], vow_idx));
			}

			vow_tx_ok[i] += tx_ok_sum;
			vow_tx_fail[i] += tx_fail_sum;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("AVG sta%d: tx cnt -> %u(%u), tx fail -> %u(%u)\n", i,
						  vow_tx_ok[i]/pAd->vow_avg_num,
						  vow_tx_ok[i],
						  vow_tx_fail[i]/pAd->vow_avg_num,
						  vow_tx_fail[i]));
				vow_tx_ok[i] = 0;
				vow_tx_fail[i] = 0;
			}
		}

		/* throughput */
		for (i = 0; i <= pAd->vow_monitor_bss; i++) {
			UINT32 txb, rxb;

			RTMP_IO_READ32(pAd, 0x23110 + (i << 2), &txb);
			RTMP_IO_READ32(pAd, 0x23130 + (i << 2), &rxb);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS%d: tx byte -> %u, rx byte -> %u\n", i, txb, rxb));
			vow_tx_bss_byte[i] += txb;
			vow_rx_bss_byte[i] += rxb;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("AVG bss%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						  vow_tx_bss_byte[i]/pAd->vow_avg_num,
						  vow_tx_bss_byte[i],
						  vow_rx_bss_byte[i]/pAd->vow_avg_num,
						  vow_rx_bss_byte[i]));
				vow_tx_bss_byte[i] = 0;
				vow_rx_bss_byte[i] = 0;
			}
		}

		for (i = 0; i <= pAd->vow_monitor_mbss; i++) {
			UINT32 txb, rxb;

			RTMP_IO_READ32(pAd, 0x23240 + (i << 2), &txb);
			RTMP_IO_READ32(pAd, 0x232C0 + (i << 2), &rxb);

			if (i < pAd->vow_show_mbss)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MBSS%d: tx byte -> %u, rx byte -> %u\n", i, txb, rxb));

			vow_tx_mbss_byte[i] += txb;
			vow_rx_mbss_byte[i] += rxb;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("AVG mbss%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						  vow_tx_mbss_byte[i]/pAd->vow_avg_num,
						  vow_tx_mbss_byte[i],
						  vow_rx_mbss_byte[i]/pAd->vow_avg_num,
						  vow_rx_mbss_byte[i]));
				vow_tx_mbss_byte[i] = 0;
				vow_rx_mbss_byte[i] = 0;
			}
		}

		/* read LPON free run counter */
		RTMP_IO_READ32(pAd, 0x2427c, &free_cnt);

		if (vow_last_free_cnt) {
			vow_interval += (free_cnt - vow_last_free_cnt);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("free count %d\n", free_cnt - vow_last_free_cnt));
			vow_last_free_cnt = free_cnt;
		}

		vow_last_free_cnt = free_cnt;
		/* read AMPDU count */
		RTMP_IO_READ32(pAd, 0x24838, &cnt);
		vow_ampdu_cnt += cnt;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AMPU count %d\n", cnt));

		if (vow_idx == pAd->vow_avg_num) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("total ampdu cnt -> %u, avg ampdu cnt --> %d\n",
					 vow_ampdu_cnt, vow_ampdu_cnt/pAd->vow_avg_num));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("total interval -> %u, avg interval --> %d\n",
					 vow_interval, vow_interval/pAd->vow_avg_num));
			vow_interval = 0;
			vow_idx = 0;
			vow_ampdu_cnt = 0;
		}

		/* Show airtime report, Mark for Debug */
		HW_IO_READ32(pAd, 0x215B4, &counter[0]); /* nonwifi airtime counter - band 0 */
		HW_IO_READ32(pAd, 0x215B8, &counter[1]); /* obss airtime counter - band 0 */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("nonwifi: %u, obss: %u.\n", counter[0], counter[1]));
		/* PLE Info  Mark for Debug */
		HW_IO_READ32(pAd, PLE_QUEUE_EMPTY, &ple_stat[0]);
		HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[1]);
		HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_1, &ple_stat[2]);
		HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_2, &ple_stat[3]);
		HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_3, &ple_stat[4]);
		HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[5]);
		HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_1, &ple_stat[6]);
		HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_2, &ple_stat[7]);
		HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_3, &ple_stat[8]);
		HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[9]);
		HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_1, &ple_stat[10]);
		HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_2, &ple_stat[11]);
		HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_3, &ple_stat[12]);
		HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[13]);
		HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_1, &ple_stat[14]);
		HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_2, &ple_stat[15]);
		HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_3, &ple_stat[16]);
		HW_IO_READ32(pAd, PLE_FREEPG_CNT, &pg_flow_ctrl[0]);
		HW_IO_READ32(pAd, PLE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
		HW_IO_READ32(pAd, PLE_PG_HIF_GROUP, &pg_flow_ctrl[2]);
		HW_IO_READ32(pAd, PLE_HIF_PG_INFO, &pg_flow_ctrl[3]);
		HW_IO_READ32(pAd, PLE_PG_CPU_GROUP, &pg_flow_ctrl[4]);
		HW_IO_READ32(pAd, PLE_CPU_PG_INFO, &pg_flow_ctrl[5]);
		HW_IO_READ32(pAd, DIS_STA_MAP0, &dis_sta_map[0]);
		HW_IO_READ32(pAd, DIS_STA_MAP1, &dis_sta_map[1]);
		HW_IO_READ32(pAd, DIS_STA_MAP2, &dis_sta_map[2]);
		HW_IO_READ32(pAd, DIS_STA_MAP3, &dis_sta_map[3]);
		HW_IO_READ32(pAd, STATION_PAUSE0, &sta_pause[0]);
		HW_IO_READ32(pAd, STATION_PAUSE1, &sta_pause[1]);
		HW_IO_READ32(pAd, STATION_PAUSE2, &sta_pause[2]);
		HW_IO_READ32(pAd, STATION_PAUSE3, &sta_pause[3]);

		for (l = 0; l < 16; l++) { /* show AC Q info */
			for (k = 0; k < 32; k++) {
				if (((ple_stat[l+1] & (0x1 << k)) >> k) == 0) {
					UINT32 hfid, tfid, pktcnt, ac_num = l/4; /* ctrl = 0; */
					UINT32 sta_num = k + (l % 4) * 32, fl_que_ctrl[3] = {0};
					struct wifi_dev *wdev = wdev_search_by_wcid(pAd, sta_num);
					UINT32 wmmidx = 0;

					if (wdev)
						wmmidx = HcGetWmmIdx(pAd, wdev);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("STA%d AC%d: ", sta_num, ac_num));
					fl_que_ctrl[0] |= (0x1 << 31);
					fl_que_ctrl[0] |= (0x2 << 14);
					fl_que_ctrl[0] |= (ac_num << 8);
					fl_que_ctrl[0] |= sta_num;
					HW_IO_WRITE32(pAd, PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
					HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
					hfid = fl_que_ctrl[1] & 0xfff;
					tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
					pktcnt = fl_que_ctrl[2] & 0xfff;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
							  tfid, hfid, pktcnt));
				}
			}
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("**************************************\n"));
	}
}

VOID vow_avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
	ad->vow_mcli_ctl.pkt_avg_len = 0;
	ad->vow_mcli_ctl.sta_nums = 0;
}

/*
 *
 */
VOID vow_avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0;
	struct multi_cli_ctl *mctrl = &ad->vow_mcli_ctl;

	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes / entry->avg_tx_pkts);

	if ((avg_pkt_len > VERIWAVE_INVALID_PKT_LEN_HIGH) ||
		(avg_pkt_len < VERIWAVE_INVALID_PKT_LEN_LOW))
		return;
	/*moving average for pkt avg length*/
	mctrl->pkt_avg_len =
		((mctrl->pkt_avg_len * mctrl->sta_nums) + avg_pkt_len) / (mctrl->sta_nums + 1);
	mctrl->sta_nums++;
}
#if defined (CONFIG_VOW_VIDEO_PARAM)
void set_vow_video_param(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 opmode)
{
	struct hdev_obj *obj;
	UINT8 pkt_num, retry_limit;
	UINT32 length;
	PEDCA_PARM pEdcaParm = NULL;
	UINT8 band_idx;
	struct video_ctl *videoctl = &pAd->vow_video_ctl;

	ASSERT(wdev);

	band_idx = HcGetBandByWdev(wdev);
	obj = wdev->pHObj;
	if (!hdev_obj_state_ready(obj))
		return;

	pEdcaParm = HcGetEdca(pAd, wdev);
	pkt_num = wlan_operate_get_rts_pkt_thld(wdev);
	retry_limit = wlan_operate_get_rts_retry_limit(wdev);
	length = wlan_operate_get_rts_len_thld(wdev);
	if (opmode == VOW_VIDEO_SET_PARAM) {
		os_move_mem(&videoctl->edca_backup[obj->WmmIdx], pEdcaParm, sizeof(EDCA_PARM));
		videoctl->rts_thld_pkt_num_backup[band_idx] = pkt_num;
		pkt_num = VIDEO_RTS_PKT_THLD;
		pEdcaParm->Cwmax[WMM_AC_VO] = VIDEO_CWMAX;
		pEdcaParm->Cwmax[WMM_AC_VI] = VIDEO_CWMAX;
		videoctl->rts_retry_limit_backup = retry_limit;
		retry_limit = VIDEO_RTS_RETRY_LIMIT;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\n\x1b[31mCwmaxVO:%d CwmaxVI:%d rts [%d %d %d] wmm_idx:%d!\x1b[m\n",
			pEdcaParm->Cwmax[WMM_AC_VO],
			pEdcaParm->Cwmax[WMM_AC_VI], pkt_num, length, retry_limit, obj->WmmIdx));
	} else {
		/* restore parameter */
		os_move_mem(pEdcaParm, &videoctl->edca_backup[obj->WmmIdx], sizeof(EDCA_PARM));
		pkt_num = videoctl->rts_thld_pkt_num_backup[band_idx];
		retry_limit = videoctl->rts_retry_limit_backup;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\n\x1b[32mCwmaxVO:%d CwmaxVI:%d rts [%d %d %d] wmm_idx:%d!\x1b[m\n",
			pEdcaParm->Cwmax[WMM_AC_VO],
			pEdcaParm->Cwmax[WMM_AC_VI], pkt_num, length, retry_limit, obj->WmmIdx));
	}

	HW_SET_RTS_THLD(pAd, wdev, pkt_num, length, retry_limit);
	HW_SET_EDCA(pAd, wdev, pEdcaParm);
}
#endif
#else
VOID vow_atf_off_init(PRTMP_ADAPTER pad)
{
	UINT32 reg32;
	/* disable RX airtime */
	HW_IO_READ32(pad, WTBL_OFF_ACR, &reg32);
	reg32 &= ~WTBL_ADM_RX_EN;
	HW_IO_WRITE32(pad, WTBL_OFF_ACR, reg32);
	HW_IO_READ32(pad, AIRTIME_DRR_SIZE, &reg32);
	reg32 &= ~STA_MAX_DEFICIT_MASK;
	reg32 |= 1; /* 256us */
	HW_IO_WRITE32(pad, AIRTIME_DRR_SIZE, reg32);
	/* disable RTS failed airtime charge for RTS deadlock */
	HW_IO_READ32(pad, AGG_SCR, &reg32);
	reg32 |= RTS_FAIL_CHARGE_DIS;
	HW_IO_WRITE32(pad, AGG_SCR, reg32);

	/* for Gen_2, set MAX_BFF_TIME to 10ms*/
	if (pad->vow_gen.VOW_GEN == VOW_GEN_2) {
		HW_IO_READ32(pad, AGG_ATCR0, &reg32);
		reg32 |= MAX_BFF_TIME_EN;
		reg32 |= MAX_BFF_TIME;
		HW_IO_WRITE32(pad, AGG_ATCR0, reg32);
	}
}
#endif /* VOW_SUPPORT */
