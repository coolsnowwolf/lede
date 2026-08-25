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
#ifndef _ECNT_HOOK_FE_H_
#define _ECNT_HOOK_FE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include "ecnt_hook.h"
#include "ecnt_hook_fe_type.h"

//for NPU_WIFI_OFFLOAD
#include "modules/npu/wifi_mail.h"
extern void npu_wifi_offload_set_force_to_cpu_flag(char isForceToCpu);
extern char npu_stat;
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
static inline int FE_API_SET_PACKET_LENGTH(FE_Gdma_Sel_t _gdm_sel, uint _length_long, uint _length_short) {
	struct ecnt_fe_data in_data; 
	int ret = 0;
	
	in_data.function_id = FE_SET_PACKET_LENGTH; 
	in_data.gdm_sel = _gdm_sel; 
	in_data.api_data.pkt_len.length_long = _length_long; 
	in_data.api_data.pkt_len.length_short = _length_short;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CHANNEL_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel, unchar _channel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_CHANNEL_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	in_data.channel = _channel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MAC_ADDR(FE_Gdma_Sel_t _gdm_sel, unchar *_mac, ushort _mask) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_MAC_ADDR;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.mac_addr.mac = _mac;
	in_data.api_data.mac_addr.mask = _mask;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MAC_ADDR_7516(FE_MacSet_Sel_t _macSet_sel, unsigned int _mac_h, unsigned int  _mac_lmin,unsigned int  _mac_lmax) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_MAC_ADDR_7516;
	in_data.macSet_sel = _macSet_sel;
	in_data.api_data.mac_addr_7516.mac_h = _mac_h;
	in_data.api_data.mac_addr_7516.mac_lmin = _mac_lmin;
	in_data.api_data.mac_addr_7516.mac_lmax = _mac_lmax;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_WAN_PORT_7516(FE_Enable_t wan1_en, FE_WanPort_Sel_t wan1_port, FE_WanPort_Sel_t wan0_port) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_WAN_PORT_7516;
	in_data.api_data.wan_port_7516.wan1_en = wan1_en;
	in_data.api_data.wan_port_7516.wan1_port = wan1_port;
	in_data.api_data.wan_port_7516.wan0_port = wan0_port;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_HWFWD_CHANNEL(FE_Cdma_Sel_t _cdm_sel, unchar _channel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_HWFWD_CHANNEL;
	in_data.cdm_sel = _cdm_sel;
	in_data.channel = _channel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CHANNEL_RETIRE(FE_Gdma_Sel_t _gdm_sel, unchar _channel, FE_Linkup_t _mode) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CHANNEL_RETIRE;
	in_data.gdm_sel = _gdm_sel;
	in_data.channel = _channel;
	in_data.api_data.link_mode = _mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CRC_STRIP(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_CRC_STRIP;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_PADDING(FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_PADDING;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_EXT_TPID(uint _tpid) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_EXT_TPID;
	in_data.reg_val = _tpid;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_EXT_TPID(uint *_tpid) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_EXT_TPID;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_tpid = in_data.reg_val;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_FW_CFG(FE_Gdma_Sel_t _gdm_sel, FE_Frame_type_t _dp_sel, FE_Frame_dp_t _dp_type) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_FW_CFG;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.fw_cfg.dp_sel = _dp_sel;
	in_data.api_data.fw_cfg.dp_val = _dp_type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_FW_CFG(FE_Gdma_Sel_t _gdm_sel, uint* pval) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_FW_CFG;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*pval = in_data.reg_val;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DROP_UDP_CHKSUM_ERR_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_UDP_CHKSUM_ERR_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DROP_TCP_CHKSUM_ERR_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_TCP_CHKSUM_ERR_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int FE_API_SET_DROP_IP_CHKSUM_ERR_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_IP_CHKSUM_ERR_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DROP_CRC_ERR_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_CRC_ERR_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DROP_RUNT_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_RUNT_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DROP_LONG_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_DROP_LONG_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_VLAN_CHECK(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_VLAN_CHECK;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_OK_CNT(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_OK_CNT;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_RX_CRC_ERR_CNT(uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_RX_CRC_ERR_CNT;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_RX_DROP_FIFO_CNT(FE_Gdma_Sel_t _gdm_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_RX_DROP_FIFO_CNT;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int FE_API_GET_RX_DROP_ERR_CNT(FE_Gdma_Sel_t _gdm_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_RX_DROP_ERR_CNT;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int FE_API_GET_OK_BYTE_CNT(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_OK_BYTE_CNT;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_TX_GET_CNT(FE_Gdma_Sel_t _gdm_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_TX_GET_CNT;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_TX_DROP_CNT(FE_Gdma_Sel_t _gdm_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_TX_DROP_CNT;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_TIEM_STAMP(uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_TIEM_STAMP;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_TIME_STAMP(uint ts) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_TIEM_STAMP;
    in_data.timeStamp = ts & 0xffff;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_INS_VLAN_TPID(FE_Gdma_Sel_t _gdm_sel, uint _tpid) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_INS_VLAN_TPID;
	in_data.gdm_sel = _gdm_sel;
	in_data.reg_val = _tpid;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int FE_API_SET_VLAN_ENABLE(FE_Gdma_Sel_t _gdm_sel, FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_VLAN_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_BLACK_LIST(FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_BLACK_LIST;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_ETHER_TYEP(uint _index, FE_Enable_t _enable, FE_PPPOE_t _is_pppoe, uint _value) {
	struct ecnt_fe_data in_data;
	int ret = 0;

	in_data.function_id = FE_SET_ETHER_TYEP;
	in_data.index = _index;
	in_data.api_data.eth_cfg.enable = _enable;
	in_data.api_data.eth_cfg.is_pppoe = _is_pppoe;
	in_data.api_data.eth_cfg.value = _value;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_L2U_KEY(uint _index, FE_L2U_KEY_t _key_sel, uint _key0, uint _key1) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_L2U_KEY;
	in_data.index = _index;
	in_data.api_data.l2u_key.key_sel = _key_sel;
	in_data.api_data.l2u_key.key0 = _key0;
	in_data.api_data.l2u_key.key1 = _key1;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_AC_GROUP_PKT_CNT(uint _index, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_AC_GROUP_PKT_CNT;
	in_data.index = _index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_AC_GROUP_BYTE_CNT(uint _index, uint *_cnt, uint *_cnt_hi) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_AC_GROUP_BYTE_CNT;
	in_data.index = _index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	*_cnt_hi = in_data.cnt_hi;
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int FE_API_CLR_AC_GROUP_PKT_CNT(uint _index) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_CLR_AC_GROUP_PKT_CNT;
	in_data.index = _index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_CLR_AC_GROUP_BYTE_CNT(uint _index) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_CLR_AC_GROUP_BYTE_CNT;
	in_data.index = _index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_METER_GROUP(uint _index, uint _value) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_METER_GROUP;
	in_data.index = _index;
	in_data.reg_val = _value;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_GDM_PCP_CODING(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel, FE_PcpMode_t mode) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_GDM_PCP_CODING;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	in_data.api_data.coding_mode = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CDM_PCP_CODING(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel, FE_PcpMode_t mode) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CDM_PCP_CODING;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	in_data.api_data.coding_mode = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}	

static inline int FE_API_SET_VIP_ENABLE(uint _index, FE_Enable_t _enable, FE_Patn_type _patten_type, uint _patten) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_VIP_ENABLE;
	in_data.index = _index;
	in_data.api_data.vip_cfg.enable = _enable;
	in_data.api_data.vip_cfg.patten_type = _patten_type;
	in_data.api_data.vip_cfg.patten = _patten;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ETH_RX_CNT(FE_RxCnt_t *_rxCnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_ETH_RX_CNT;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_rxCnt = in_data.api_data.FE_RxCnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ETH_TX_CNT(FE_TxCnt_t *_txCnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_ETH_TX_CNT;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_txCnt = in_data.api_data.FE_TxCnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ETH_FRAME_CNT(FE_TXRX_Sel_t _txrx_sel, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_ETH_FRAME_CNT;
	in_data.txrx_sel = _txrx_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ETH_ERR_CNT(FE_Err_type_t _type, uint *_cnt) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_ETH_ERR_CNT;
	in_data.err_type = _type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_cnt = in_data.cnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CLEAR_MIB(FE_Gdma_Sel_t _gdm_sel, FE_TXRX_Sel_t _txrx_sel) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_CLEAR_MIB;
	in_data.gdm_sel = _gdm_sel;
	in_data.txrx_sel = _txrx_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CDM_RX_RED_DROP(FE_Cdma_Sel_t _cdm_sel, FE_RedDropQ_Sel_t _dropQ_sel, FE_RedDropMode_Sel_t _dropMode_sel) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CDM_RX_RED_DROP;
	in_data.cdm_sel = _cdm_sel;
	in_data.dropQ_sel = _dropQ_sel;
	in_data.dropMode_sel = _dropMode_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_CDM_RX_RED_DROP(FE_Cdma_Sel_t _cdm_sel, FE_RedDropQ_Sel_t _dropQ_sel, FE_RedDropMode_Sel_t *_dropMode_sel_p) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_CDM_RX_RED_DROP;
	in_data.cdm_sel = _cdm_sel;
	in_data.dropQ_sel = _dropQ_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *_dropMode_sel_p = in_data.dropMode_sel;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CHANNEL_RETIRE_ALL(FE_Gdma_Sel_t gdm_sel, unchar channel) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CHANNEL_RETIRE_ALL;
	in_data.gdm_sel = gdm_sel;
	in_data.channel = channel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CHANNEL_RETIRE_ONE(FE_Gdma_Sel_t gdm_sel, unchar channel) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CHANNEL_RETIRE_ONE;
	in_data.gdm_sel = gdm_sel;
	in_data.channel = channel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_TX_RATE(uint rate,uint mode,uint maxBkSzie,uint tick)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_TX_RATE;
	in_data.api_data.rate_cfg.rate = rate;
	in_data.api_data.rate_cfg.mode = mode;	
	in_data.api_data.rate_cfg.maxBkSize = maxBkSzie;
	in_data.api_data.rate_cfg.tick = tick;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_RXUC_RATE(uint rate,uint mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_RXUC_RATE;
	in_data.api_data.rate_cfg.rate = rate;
	in_data.api_data.rate_cfg.mode = mode;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_RXBC_RATE(uint rate,uint mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_RXBC_RATE;
	in_data.api_data.rate_cfg.rate = rate;
	in_data.api_data.rate_cfg.mode = mode;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_RXMC_RATE(uint rate,uint mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_RXMC_RATE;
	in_data.api_data.rate_cfg.rate = rate;
	in_data.api_data.rate_cfg.mode = mode;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_RXOC_RATE(uint rate,uint mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_RXOC_RATE;
	in_data.api_data.rate_cfg.rate = rate;
	in_data.api_data.rate_cfg.mode = mode;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_VIP_ETHER(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_VIP_ETHER;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_VIP_PPP(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_VIP_PPP;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_VIP_IP(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_VIP_IP;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_VIP_TCP(ushort src,ushort dst,unchar mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_VIP_TCP;
	in_data.api_data.vip.src = src;
	in_data.api_data.vip.dst = dst;	
	in_data.api_data.vip.mode = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_VIP_UDP(ushort src,ushort dst,unchar mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_VIP_UDP;
	in_data.api_data.vip.src = src;
	in_data.api_data.vip.dst = dst;	
	in_data.api_data.vip.mode = mode;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int FE_API_DEL_VIP_ETHER(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_VIP_ETHER;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_VIP_PPP(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_VIP_PPP;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_VIP_IP(ushort type)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_VIP_IP;
	in_data.api_data.vip.type = type;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_VIP_TCP(ushort src,ushort dst,unchar mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_VIP_TCP;
	in_data.api_data.vip.src = src;
	in_data.api_data.vip.dst = dst;	
	in_data.api_data.vip.mode = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_VIP_UDP(ushort src,ushort dst,unchar mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_VIP_UDP;
	in_data.api_data.vip.src = src;
	in_data.api_data.vip.dst = dst;	
	in_data.api_data.vip.mode = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}



static inline int FE_API_ADD_L2LU_VLAN_DSCP(ushort svlan,ushort cvlan,unchar dscp,unchar mask)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_L2LU_VLAN_DSCP;
	in_data.api_data.l2lu.svlan = svlan;
	in_data.api_data.l2lu.cvlan = cvlan;
	in_data.api_data.l2lu.dscp = dscp;
	in_data.api_data.l2lu.mask = mask;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_L2LU_VLAN_TRFC(ushort svlan,ushort cvlan,unchar trfc,unchar mask)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_L2LU_VLAN_TRFC;
	in_data.api_data.l2lu.svlan = svlan;
	in_data.api_data.l2lu.cvlan = cvlan;
	in_data.api_data.l2lu.dscp = trfc;
	in_data.api_data.l2lu.mask = mask;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_L2LU_VLAN_DSCP(ushort svlan,ushort cvlan,unchar dscp,unchar mask)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_L2LU_VLAN_DSCP;
	in_data.api_data.l2lu.svlan = svlan;
	in_data.api_data.l2lu.cvlan = cvlan;
	in_data.api_data.l2lu.dscp = dscp;
	in_data.api_data.l2lu.mask = mask;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEL_L2LU_VLAN_TRFC(ushort svlan,ushort cvlan,unchar trfc,unchar mask)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_DEL_L2LU_VLAN_TRFC;
	in_data.api_data.l2lu.svlan = svlan;
	in_data.api_data.l2lu.cvlan = cvlan;
	in_data.api_data.l2lu.dscp = trfc;
	in_data.api_data.l2lu.mask = mask;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_TX_FAVOR_OAM_ENABLE(FE_Enable_t _enable) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_TX_FAVOR_OAM_ENABLE;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_TLS_FORWARD(void* skb,FE_Tls_forward_direction_t dir) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_TLS_FORWARD;
	in_data.api_data.tls_forward.skb = skb;
	in_data.api_data.tls_forward.dir = dir;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DO_FE_RESET(FE_Reset_mode_t reset_mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_DO_RESET;
	in_data.api_data.reset_mode = reset_mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_LOOPBACK_ENABLE(FE_Gdma_Sel_t _gdm_sel, uint channel, FE_Enable_t enable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_LOOPBACK_ENABLE;
	in_data.gdm_sel = _gdm_sel;
	in_data.channel = channel;
	in_data.api_data.enable = enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_LOOPBACK_MODE(FE_Gdma_Sel_t _gdm_sel, FE_Random_mode_t channel_mode, FE_Random_mode_t length_mode, FE_Random_mode_t gap_mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_LOOPBACK_MODE;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.lpbp_mode.channel_mode = channel_mode;
	in_data.api_data.lpbp_mode.length_mode = length_mode;
	in_data.api_data.lpbp_mode.gap_mode = gap_mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_METER_RATELIMIT(uint ratelimit, unchar idx, FE_Dir_t dir, FE_Meter_Mode_t mode, unchar enable)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_METER_RATELIMIT;
    in_data.api_data.meter_cfg.rate = ratelimit;
    in_data.api_data.meter_cfg.enable = enable;
    in_data.api_data.meter_cfg.pkt_mode = METER_BYTE_MODE;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.mode = mode;
    in_data.api_data.meter_cfg.tick_sel = FAST_TICK;
    in_data.api_data.meter_cfg.bucket_size = 0;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_METER_RATELIMIT(uint *ratelimit, unchar idx, FE_Dir_t dir, FE_Meter_Mode_t mode)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_METER_RATELIMIT;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.mode = mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*ratelimit = in_data.api_data.meter_cfg.rate;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_METER_RATELIMIT_MODE(FE_Dir_t dir, FE_Meter_Mode_t mode)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_RX_RATELIMIT_MODE;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.mode = mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_FlOW_CNT(unchar idx, FE_Dir_t dir, FE_ACNT_Mode_t mode, uint *cnt_lo, uint *cnt_hi)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_FLOW_CNT;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;/*up or down stream*/
    in_data.api_data.meter_cfg.acnt_mode = mode;/**/
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*cnt_lo = in_data.api_data.meter_cfg.byteCnt_L;
	*cnt_hi = in_data.api_data.meter_cfg.byteCnt_H;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_FlOW_PKT_CNT(unchar idx, FE_Dir_t dir, FE_ACNT_Mode_t mode, uint *pkt_cnt, uint *pkt_cnt_hi)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_FLOW_CNT;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.acnt_mode = mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*pkt_cnt = in_data.api_data.meter_cfg.pktCnt;
	*pkt_cnt_hi = in_data.api_data.meter_cfg.pktCnt_H;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_CLEAR_FlOW_CNT(unchar idx, FE_Dir_t dir, FE_ACNT_Mode_t mode)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_CLEAR_FLOW_CNT;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.acnt_mode = mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_WAN_ITF_INDEX(struct sk_buff *skb, unchar *wan_idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_WAN_ITF_INDEX;
    in_data.api_data.meter_cfg.skb= skb;
	memcpy(in_data.api_data.meter_cfg.itf_name, skb->dev->name, 16);
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *wan_idx = in_data.api_data.meter_cfg.idx;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_METER_IDX(struct sk_buff *skb, FE_Dir_t dir, unchar *meter_idx, unchar idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_METER_IDX;
    in_data.api_data.meter_cfg.skb= skb;
    in_data.api_data.meter_cfg.dir = dir;
    in_data.api_data.meter_cfg.idx = idx;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *meter_idx = in_data.api_data.meter_cfg.meter_id;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ACNT2_IDX(struct sk_buff *skb, FE_Dir_t dir,unchar *acnt2_idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_ACNT2_IDX;
    in_data.api_data.meter_cfg.skb= skb;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *acnt2_idx = in_data.api_data.meter_cfg.acnt2_id;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ACNT1_IDX(struct sk_buff *skb, FE_Dir_t dir,unchar *acnt1_idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_ACNT1_IDX;
    in_data.api_data.meter_cfg.skb= skb;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *acnt1_idx = in_data.api_data.meter_cfg.acnt1_id;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ACNT0_IDX(struct sk_buff *skb, FE_Dir_t dir,unchar *acnt0_idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_ACNT0_IDX;
    in_data.api_data.meter_cfg.skb= skb;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *acnt0_idx = in_data.api_data.meter_cfg.acnt0_id;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ACNT1_MODE(FE_Dir_t dir, unchar *acnt1_mode)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_ACNT1_MODE;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *acnt1_mode = in_data.api_data.meter_cfg.acnt_mode;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_ACNT0_MODE(FE_Dir_t dir,unchar *acnt0_mode)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_ACNT0_MODE;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    *acnt0_mode = in_data.api_data.meter_cfg.acnt_mode;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_ACNT1_MODE(FE_Dir_t dir, unchar acnt1_mode)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_ACNT1_MODE;
    in_data.api_data.meter_cfg.dir = dir;
	in_data.api_data.meter_cfg.acnt_mode = acnt1_mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_ACNT0_MODE(FE_Dir_t dir,unchar acnt0_mode)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_ACNT0_MODE;
    in_data.api_data.meter_cfg.dir = dir;
	in_data.api_data.meter_cfg.acnt_mode = acnt0_mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_METER_ENABLE(unchar *enable, unchar idx, FE_Dir_t dir)
{
    struct ecnt_fe_data in_data;
	int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_METER_ENABLE;
    in_data.api_data.meter_cfg.idx = idx;
    in_data.api_data.meter_cfg.dir = dir;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*enable = in_data.api_data.meter_cfg.enable;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_METER_CTL_BY_OLT(unchar enable)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_METER_CTL_BY_OLT;
    in_data.api_data.meter_cfg.idx= enable;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_DEV_MAC_INDEX(unchar *mac, unchar *mac_idx)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_GET_DEV_MAC_INDEX;
    memcpy(in_data.api_data.meter_cfg.mac, mac, 6);
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*mac_idx = in_data.api_data.meter_cfg.idx;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_INIT_RESOURCE_MANAGE(void)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_INIT_RESOURCE_MANAGE;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_DEINIT_RESOURCE_MANAGE(void)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_DEINIT_RESOURCE_MANAGE;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_PSE_OQ_THRESHOLD(FE_PonMode_Sel_t pon_mode)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_PSE_OQ_THR;
    in_data.api_data.pon_mode = pon_mode;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int FE_API_GET_UNKNOWN_MUL_PKT(uint unknown_mul_pkt)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_UNKNOWN_MUL_PKT;
	in_data.api_data.unknown_mul_pkt= unknown_mul_pkt;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int FE_API_SET_GLO_RATE_BYTE(FE_Rate_minus_t rate_minus, unchar rate_byte)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_GLO_RATE_BYTE;
	in_data.api_data.glo_rate_byte.rate_minus = rate_minus;
	in_data.api_data.glo_rate_byte.rate_byte = rate_byte;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_ADD_DEV_TO_TOTAL_ACCOUNT(unsigned char *mac) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_ADD_DEV_TO_TOTAL_ACCOUNT;
	memcpy(in_data.api_data.dev_cfg.mac, mac, 6);
    
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MC_VLAN_GLOBAL(FE_Enable_t enable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_SET_MC_VLAN_GLOBAL;
	in_data.api_data.enable = enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_MC_VLAN_GLOBAL(FE_Enable_t *enable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_GET_MC_VLAN_GLOBAL;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*enable = in_data.api_data.enable;
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MC_VLAN_TABLE_CFG(FE_McVlanTableCfg_t *mcVlanTableCfg)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_SET_MC_VLAN_TABLE_CFG;
	in_data.api_data.fe_mcvlan_table_cfg.mc_table_id = mcVlanTableCfg->mc_table_id;
	in_data.api_data.fe_mcvlan_table_cfg.enable = mcVlanTableCfg->enable;
	in_data.api_data.fe_mcvlan_table_cfg.ppe_entry_id = mcVlanTableCfg->ppe_entry_id;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_MC_VLAN_TABLE_CFG(FE_McVlanTableCfg_t *mcVlanTableCfg)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_GET_MC_VLAN_TABLE_CFG;
	in_data.api_data.fe_mcvlan_table_cfg.mc_table_id = mcVlanTableCfg->mc_table_id;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	mcVlanTableCfg->enable = in_data.api_data.fe_mcvlan_table_cfg.enable;
	mcVlanTableCfg->ppe_entry_id = in_data.api_data.fe_mcvlan_table_cfg.ppe_entry_id;

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MC_VLAN_ACTION_CFG(FE_McVlanActionCfg_t *mcVlanActionCfg)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_SET_MC_VLAN_ACTION_CFG;
	in_data.api_data.fe_mcvlan_action_cfg.mc_table_id = mcVlanActionCfg->mc_table_id;
	in_data.api_data.fe_mcvlan_action_cfg.mc_chnl_id = mcVlanActionCfg->mc_chnl_id;
	in_data.api_data.fe_mcvlan_action_cfg.mc_vlan_op = mcVlanActionCfg->mc_vlan_op;
	in_data.api_data.fe_mcvlan_action_cfg.mc_vlan_vpm = mcVlanActionCfg->mc_vlan_vpm;
	in_data.api_data.fe_mcvlan_action_cfg.mc_vlan = mcVlanActionCfg->mc_vlan;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_MC_VLAN_ACTION_CFG(FE_McVlanActionCfg_t *mcVlanActionCfg)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_GET_MC_VLAN_ACTION_CFG;
	in_data.api_data.fe_mcvlan_action_cfg.mc_table_id = mcVlanActionCfg->mc_table_id;
	in_data.api_data.fe_mcvlan_action_cfg.mc_chnl_id = mcVlanActionCfg->mc_chnl_id;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	mcVlanActionCfg->mc_vlan_op = in_data.api_data.fe_mcvlan_action_cfg.mc_vlan_op;
	mcVlanActionCfg->mc_vlan_vpm = in_data.api_data.fe_mcvlan_action_cfg.mc_vlan_vpm;
	mcVlanActionCfg->mc_vlan = in_data.api_data.fe_mcvlan_action_cfg.mc_vlan;

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MC_VLAN_CLEAR_ALL(unsigned char mc_table_id)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_SET_MC_VLAN_CLEAR_ALL;
	in_data.api_data.fe_mcvlan_table_cfg.mc_table_id = mc_table_id;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_XFI_PHY_LINK_CHANGE(XFI_LINK_STATUS_t status)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

	in_data.function_id = FE_XFI_PHY_LINK_CHANGE;
	in_data.api_data.status = status;
    
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API,(struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_GDMA_MISC_CONFIG(FE_Gdma_Sel_t _gdm_sel, FE_MISC_CONFIG_t  mode)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_GDMA_MISC_CONFIG;
	in_data.gdm_sel = _gdm_sel;
	in_data.api_data.fe_misc_cfg  = mode;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}



static inline int FE_API_GET_HSGMII_RX_CNT(FE_RxCnt_t *_rxCnt, int hsgmii_index) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_HSGMII_LAN_RX_CNT;
	in_data.index = hsgmii_index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_rxCnt = in_data.api_data.FE_RxCnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_HSGMII_TX_CNT(FE_TxCnt_t *_txCnt, int hsgmii_index) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_HSGMII_LAN_TX_CNT;
	in_data.index = hsgmii_index;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*_txCnt = in_data.api_data.FE_TxCnt;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_AEWAN_FWDFQ(FE_Gdma_Sel_t _gdm_sel, uint aewan_fwdfq)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_AEWAN_FWDFQ;
	in_data.gdm_sel = _gdm_sel;
	in_data.reg_val = aewan_fwdfq;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_AEWAN_IFCDISABLE(uint aewan_ifcdisable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	 
	in_data.function_id = FE_SET_AEWAN_IFCDISABLE;
	in_data.reg_val = aewan_ifcdisable;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_GMD2_SPTAG_FOR_LOOPBACK(FE_Enable_t enable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_GMD2_SPTAG_FOR_LOOPBACK;
	in_data.api_data.enable = enable;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline void FE_API_SET_NPU_FORCE_TO_CPU_FLAG(char forceToCpu)
{

	if(npu_stat == 1)
		WIFI_MAIL_API_SET_WAIT_IS_FORCE_TO_CPU(0, forceToCpu);
	else
		npu_wifi_offload_set_force_to_cpu_flag(forceToCpu);

	return;
}


static inline int FE_API_SET_TUNNEL_CFG(unsigned int tableIndex, unsigned int offset, unsigned int value0,unsigned int value1,unsigned int value2,unsigned int value3)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_TUNNEL_CFG;
	in_data.api_data.fe_tunnel_cfg.tableIndex = tableIndex;
	in_data.api_data.fe_tunnel_cfg.offset = offset;
	in_data.api_data.fe_tunnel_cfg.value0 = value0;
	in_data.api_data.fe_tunnel_cfg.value1 = value1;
	in_data.api_data.fe_tunnel_cfg.value2 = value2;
	in_data.api_data.fe_tunnel_cfg.value3 = value3;
	
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int FE_API_SET_GDM_SPTAG_FOR_EXTSWITCH(FE_Enable_t enable, FE_Gdma_Sel_t gdm_sel, uint channel)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_GDM_SPTAG_FOR_EXTSWITCH;
	in_data.api_data.enable = enable;
	in_data.gdm_sel= gdm_sel;
	in_data.channel = channel;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_PSE_OQ_RSV_ENABLE(FE_Frame_dp_t port, uint channel, FE_Enable_t enable)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_PSE_OQ_RSV_EN;
	in_data.api_data.fe_oq_rsv_en.port = port;
    in_data.api_data.fe_oq_rsv_en.channel = channel;
    in_data.api_data.fe_oq_rsv_en.enable = enable;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_MBI_ARB_RST(FE_Gdma_Sel_t gdm_sel)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_MBI_ARB_RST;
	in_data.gdm_sel= gdm_sel;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_RMBI_FRAG(FE_Gdma_Sel_t gdm_sel, uint channel, uint reg_val)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_RMBI_FRAG;
	in_data.gdm_sel= gdm_sel;
	in_data.channel= channel;
	in_data.reg_val= reg_val;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_GET_CHN_RLS(FE_Gdma_Sel_t _gdm_sel, uint* pval) {
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_GET_CHN_RLS;
	in_data.gdm_sel = _gdm_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	*pval = in_data.reg_val;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_TMBI_FRAG(FE_Gdma_Sel_t gdm_sel, uint channel, uint reg_val)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	 
	in_data.function_id = FE_SET_TMBI_FRAG;
	in_data.gdm_sel= gdm_sel;
	in_data.channel= channel;
	in_data.reg_val= reg_val;
	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_GDMA_ENABLE(FE_Gdma_Sel_t gdm_sel)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
	
	in_data.function_id = FE_SET_GDMA_ENABLE;
	in_data.gdm_sel= gdm_sel;

	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_GDMA_DISABLE(FE_Gdma_Sel_t gdm_sel)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

	in_data.function_id = FE_SET_GDMA_DISABLE;
	in_data.gdm_sel= gdm_sel;

	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int FE_API_SET_CHN_RETIRE_ACTION(FE_Gdma_Sel_t gdm_sel, unchar channel)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    
    in_data.function_id = FE_SET_CHN_RETIRE_ACTION;
    in_data.gdm_sel = gdm_sel;
    in_data.channel = channel;
    ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_CHN_RETIRE_DONE(FE_Gdma_Sel_t gdm_sel, unchar channel) 
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_CHN_RETIRE_DONE;
	in_data.gdm_sel = gdm_sel;
	in_data.channel = channel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_QBI_FTTR_CHN_DISABLE(void) 
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_QBI_FTTR_CHN_DISABLE;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_FORCE_SLOW_ENABLE(FE_Enable_t _enable) 
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_FORCE_SLOW_ENABLE;
	in_data.api_data.enable = _enable;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_FORCE_SLOW_DUTY(FE_Enable_t fix_duty, uint fast_duty, uint slow_duty) 
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_FORCE_SLOW_DUTY;
	in_data.api_data.force_slow_cfg.fix_duty_enable  = fix_duty;
	in_data.api_data.force_slow_cfg.fast_duty		 = fast_duty;
	in_data.api_data.force_slow_cfg.slow_duty 		 = slow_duty;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_VIP_RXQ_SELECTION(unsigned char rxq_sel)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;
    
	in_data.function_id = FE_SET_VIP_RXQ_SELECTION;
	in_data.channel = rxq_sel;
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_VIP_FOR_TCP_SPEEDTEST(ushort operation,ushort src_port,ushort dst_port)
{
	struct ecnt_fe_data in_data;
	int ret = 0;
	
	in_data.function_id = FE_SET_VIP_FOR_TCP_SPEEDTEST;
	in_data.api_data.vip.type = operation;
	in_data.api_data.vip.src = src_port;
	in_data.api_data.vip.dst = dst_port;	
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int FE_API_SET_DEV_STAT_RATELIMIT_MODE(unsigned int wan_conn_type, char *wan_itf_name)
{
    struct ecnt_fe_data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ecnt_fe_data)) ;

    in_data.function_id = FE_SET_DEV_STAT_RATELIMIT_MODE;
	in_data.api_data.dev_statistics_cfg.wan_conn_type = wan_conn_type;
	memcpy(in_data.api_data.dev_statistics_cfg.wan_itf_name, wan_itf_name, 16);
	ret = __ECNT_HOOK(ECNT_FE, ECNT_FE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_FE_H_ */



