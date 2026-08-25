/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/


#ifndef _UNION_IC_DEF_H_
#define _UNION_IC_DEF_H_

typedef enum{
	REGISTER_ACTION_EPON_CHECK_MAC_TX_MBI_STOP,
	REGISTER_ACTION_EPON_CHECK_MAC_TX_MPI_STOP,
	ONLINE_ACTION_EPON_GET_OLT_MAC_ADDRESS,
	REGISTER_ACTION_EPON_INT_ENABLE,
	REGISTER_ACTION_EPON_SET_Q_THRESHLD_CFG,
	REGISTER_ACTION_EPON_GET_Q_THRESHLD_CFG,
	REGISTER_ACTION_EPON_GET_WAN_CONF,
	PACKETS_ACTION_EPON_QOS_TX_RATE_METER_CFG,
	REGISTER_ACTION_EPON_DEFAULT_REPORT_METHOD,
	REGISTER_ACTION_EPON_SET_IPG,
	REGISTER_ACTION_EPON_MPCP_TIMEOUT_VALUE,
	REGISTER_ACTION_EPON_RTT_ADJUST,
	REGISTER_ACTION_EPON_GET_REPORT_BITMAP,
	REGISTER_ACTION_EPON_INIT_DSCVRY_STS,
	REGISTER_ACTION_EPON_MAC_BASE_REG_ADDRESS,
	REGISTER_ACTION_EPON_DOWN_KEY_CHANGE,
	REGISTER_ACTION_EPON_DOWN_KEY_MISS,
	REGISTER_ACTION_EPON_UP_KEY_CHANGE,
	REGISTER_ACTION_EPON_SET_LLID_KEY,
	REGISTER_ACTION_EPON_GET_LLID_KEY,
	REGISTER_ACTION_EPON_SET_LLID_TX_FEC,
	REGISTER_ACTION_EPON_GET_LLID_TX_FEC,
	REGISTER_ACTION_EPON_SET_10G_LLID_KEY,
	REGISTER_ACTION_EPON_GET_10G_LLID_KEY,
	REGISTER_ACTION_EPON_SET_DPOE_LLID_KEY,
	REGISTER_ACTION_EPON_SET_LLID_THRSHLD_NUM,
	REGISTER_ACTION_EPON_GET_LLID_THRSHLD_NUM,
	PACKETS_ACTION_EPON_TX_BUFF_USAGE_CFG,
	REGISTER_ACTION_PON_MAC_SCU_RESET,
	REGISTER_ACTION_EPON_MAC_LOGIC_RESET,
	ONLINE_ACTION_PON_GET_ONU_MODE_TYPE,
	REGISTER_ACTION_EPON_SET_DYGASP_HW_EN,
	REGISTER_ACTION_EPON_GET_DYGASP_HW_EN,
	REGISTER_ACTION_EPON_SET_DYGASP_NUM,
	REGISTER_ACTION_EPON_GET_DYGASP_NUM,
	/*Debug action define*/
	REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT,
	REGISTER_ACTION_EPON_DBG_GET_STATE,
	REGISTER_ACTION_EPON_DBG_SET_SNIFFER,
	REGISTER_ACTION_EPON_DBG_SET_STATIC_RPT,
	REGISTER_ACTION_EPON_DBG_SET_BAND_UTILIZATION,
	REGISTER_ACTION_EPON_DBG_SET_TX_RATE_ANALYZE,
	/*only defined, but not used*/
	REGISTER_ACTION_EPON_REPORT_QSIZEADJS,
	REGISTER_ACTION_EPON_REPORT_QSIZEADJS_FEC,
	REGISTER_ACTION_EPON_SET_REPORT_BITMAP,
	/*please add ACTION Macro above*/
	MAX_ACTION_NUM,
}UNION_IC_ACTION;

#define	RO			(1 << 0)
#define	WO			(1 << 1)
#define	RW			(1 << 2)
#define	NO_DEF		(1 << 3)
#define	W1C			(1 << 4)

typedef struct reg_check_s{
	char* name; /*Register Name*/
	uint8_t type;/*0:Read Only, 1: Write Only, 2: Write Read,3: No default value 4: W1C*/
	uint32_t	addr;/*Register location*/
	uint32_t	def_value;/*Default value*/
	uint32_t	mask;/*For read/write test*/
} reg_check_t;

typedef struct reg_ind_check_s{
	char *name; /*Register function name*/
	int (*func)(__u32 pattern);
} reg_ind_check_t;


#define EPON_LOGIC_RESET_HOLD_ON		(1)
#define EPON_LOGIC_RESET_HOLD_OFF		(0)

typedef enum{
	EPON_SNIFFER_ENABLE_SELF_ALL,
	EPON_SNIFFER_DISABLE_ALL,
	EPON_SNIFFER_ENABLE_OTHER_MPCP,
	EPON_SNIFFER_ENABLE_OTHER_ETHERNET,
}EPON_SNIFFER_MODE_T;

extern int UNION_IC_FUNCTION_HOOK(UNION_IC_ACTION action_idx, const void *in, void *out);
extern int init_union_ic_function(void);

extern uint32_t g_epon_llid_dscv_stat[32];
extern reg_check_t *epon_reg;
extern reg_ind_check_t *indirect_reg;

extern void add_fix_reg_list(uint32_t addr, uint32_t value);
extern void del_fix_reg_list(uint32_t addr);
/*********************************Union IC Input parameter define*******************************************/

typedef struct epon_llid_report_bitmap_s{
	uint8_t llidIndex;
	uint8_t bitmap;
}epon_llid_report_bitmap_t,*epon_llid_report_bitmap_ptr;

typedef struct static_report_info_s{
	uint8_t enable;
	uint32_t report_val;
}static_report_info_t,*static_report_info_ptr;


typedef struct xepon_debug_statistic_s
{
    uint8_t     mpcpErrCnt;
    uint8_t     mpcpRgstCnt;
    uint16_t    mpcpDscvGateCnt;
    uint16_t    mpcpRgstReqCnt;
    uint16_t    mpcpRgstAckCnt;
    uint32_t    rxMpiEth;
    uint32_t    rxMbiEth;
    uint32_t    txMbiEth;
    uint32_t    txMpiEth;
    uint32_t    rxOamCnt;
    uint32_t    txOamCnt;
    uint16_t    rxMpiCrc32ErrCnt;
    uint16_t    rxMpiCrc8ErrCnt;
    uint16_t    rxMpiEofDropCnt;
	uint8_t     rxMpiFifoOvRunCnt;
    uint16_t    rxMbiSofDropCnt;
    uint16_t    rxMbiSnfDropCnt;
    uint16_t    rxMbiCrcErrCnt;
    uint16_t    rxMbiEndDropCnt;
    uint32_t    rxMbiSnfCnt;
    uint32_t    rxMpiUcEthCnt;
    uint32_t    rxMpiBcEthCnt;
    uint32_t    rxMpiMcEthCnt;
    uint32_t    rxMpiOamCnt;
    uint32_t    rxMpiMpcpCnt;
    uint32_t    rxMpiGateCnt;
    uint32_t    rxMpiNrlGateCnt;
    uint32_t    txMbiUcEthCnt;
    uint16_t    txMbiBcEthCnt;
    uint16_t    txMbiMcEthCnt;
    uint16_t    txMbiErrCnt;
    uint16_t    txMbiRptCnt;
    uint16_t    txMpiRptCnt;
    uint16_t    txMpiOamCnt;
    uint8_t     txMpiRgstReqCnt;
    uint8_t     txMpiRgstAckCnt;
    uint32_t    txMpiUcEthCnt;
    uint16_t    txMpiBcEthCnt;
    uint16_t    txMpiMcEthCnt;
    uint16_t    rxMpiChurnOkCnt;
    uint16_t    rxMpiChurnErrCnt;
}epon_debug_statistic_t,*xepon_debug_statistic_ptr;

typedef struct epon_dbg_state_s
{
    uint32_t    glbCfg;
    uint32_t    glbCfg2;
    uint32_t    intSts;
    uint32_t    intEn;
    uint32_t    intSts2;
    uint32_t    intEn2;
    uint32_t    intSts3;
    uint32_t    intEn3;
    uint32_t    llidCfg0_3;
    uint32_t    rptCfg;
    uint32_t    laserTm;
    uint32_t    syncTm;
    uint32_t    txCnst;
    uint32_t    pwrSvCfg;
    uint32_t    dygspCfg;
    uint32_t    rdmDlySts;    
    uint32_t    glbSts1;
    uint32_t    glbSts2;
    uint32_t    dsKeyChg;
    uint32_t    usKeyChg;
    uint32_t    rptBmp;
    uint32_t    oltOvTm;
    uint32_t    crptCfg;
    uint32_t    desecCfg;
    uint32_t    ensecCfg;
    uint32_t    rdmDlyCfg;
}epon_dbg_state_t,*epon_dbg_state_t_ptr;

#endif

