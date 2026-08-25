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

#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "common/drv_global.h"
#include "pwan/gpon_wan.h"
#include "pwan/xpon_netif.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_recovery.h"
#include "common/xpondrv.h"
#include "common/omci_oam_monitor.h"

#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include <ecnt_hook/ecnt_hook_smux.h>
#include <ecnt_hook/ecnt_hook_vlan.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif
#include "common/xpon_led.h"


#ifdef TCSUPPORT_BBF_247
#include <ecnt_hook/ecnt_hook_fe.h>
#endif
#include <ecnt_hook/ecnt_hook_gpon_flow.h>

/************************************************************************************************
							Struct Define
**************************************************************************************************/
typedef struct Omci_Hander{
	u16  transId;
	u8	 msgType;
	u8	 devId;
	u32    MeId;
	/* use in extended omci*/
	u16  msgContLen;
}__attribute__((packed)) Omci_Header_T, *pOmci_Header;



/************************************************************************************************
							Macro Define
**************************************************************************************************/
#define OMCI_MSG_TYPE_MIB_UPLOAD             13
#define OMCI_MSG_TYPE_DELETE                 6
#define OMCI_MSG_TYPE_CREATE                 4
#define OMCI_MSG_TYPE_SET                    8
#define OMCI_CLASS_ID_MAC_BRIDGE_PORT_CFG    47
#define OMCI_CLASS_ID_802_1P                 130
#define OMCI_CLASS_ID_EXT_VLAN_OP            171
#define OMCI_CLASS_ID_GEM_CTP                268

#define OMCI_CRC_LEN                (4)
#define OMCI_BASIC_MSG_FIX_LEN      (48)
#define OMCI_BASIC_MSG_DEV_ID       (0x0a)
#define OMCI_EXTENDED_MSG_DEV_ID    (0x0b)

#define XPON_WAN_TX_DIR		0
#define XPON_WAN_RX_DIR		1

/*************************************************************************************************
							Function Declare
***************************************************************************************************/
#ifdef TCSUPPORT_XPON_IGMP
extern int (*xpon_sfu_up_multicast_incoming_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_sfu_multicast_protocol_hook)(struct sk_buff *skb);
#endif
extern void macSend(uint32 chanId, struct sk_buff *skb);
extern u32 random32(void);

#ifdef TCSUPPORT_PON_VLAN
extern int (*pon_insert_tag_hook)(struct sk_buff **pskb);
#endif
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP) && defined(TCSUPPORT_XPON_IGMP)
extern int (*isVlanOperationInMulticastModule_hook)(struct sk_buff *skb);
extern int (*xpon_up_igmp_uni_vlan_filter_hook)(struct sk_buff *skb);
extern int (*xpon_sfu_up_multicast_vlan_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_up_igmp_ani_vlan_filter_hook)(struct sk_buff *skb);
#endif
#ifdef TCSUPPORT_PON_MAC_FILTER
extern int (*pon_check_mac_hook)(struct sk_buff *skb);
#endif

#ifdef TCSUPPORT_GPON_MAPPING
extern int (*gpon_mapping_hook)(struct sk_buff *pskb);
#endif

extern void xpon_send_traffic_up_event(void);
extern int isHighestPriorityPacket(struct sk_buff *skb);
extern void gpon_init_qdma_tx_buff(void);
extern int xpon_wan_restore_offload_info(struct sk_buff * skb,struct port_info *xpon_info, PWAN_FETxMsg_T *pTxMsg, int ponMode);
extern int gwan_tx_vlan_gem_map_handler_customer(struct sk_buff *skb, uint8_t netIdx);

/**************************************************************************************************
							Variable Define
***************************************************************************************************/
int g_service_change_flag = 0;
int g_rm_tcont_delay_us =  1000;
int channel_switch_enable = 0;
int deallocate_flag = 0;

extern Omci_Oam_Monitor_t * gpMonitor;
extern int omci_drop_flag;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int g_omci_creat_tcont;
#endif/*TCSUPPORT_COMPILE*/

/*************************************************************************************
				Function API Define
*****************************************************************************************/
#ifdef TCSUPPORT_WAN_GPON
static int remove_omci_crc_if_exist(struct sk_buff * skb)
{
    pOmci_Header pOmciHeader = NULL;
    u16 omci_msg_len = 0;
    u16 extra_len = 0;
    
    pOmciHeader = (pOmci_Header)skb->data;

    if(unlikely(skb->len < sizeof(Omci_Header_T) )) /* exception, all basic and extended omci msg should longer than this! */
    {
        return XPON_FAIL;
    }
    
    if(OMCI_BASIC_MSG_DEV_ID == pOmciHeader->devId ) /*baseline omci*/
    {
        if(OMCI_BASIC_MSG_FIX_LEN == skb->len)
        {
            /*remove CRC*/
            skb->len -= OMCI_CRC_LEN;
        }
        else if((OMCI_BASIC_MSG_FIX_LEN - OMCI_CRC_LEN) != skb->len)
        {
            return XPON_FAIL;
        }
    }
    else if(OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId) /*extended omci*/
    {
        omci_msg_len = ntohs(pOmciHeader->msgContLen) + sizeof(Omci_Header_T);
        if(skb->len > omci_msg_len ) {
            /*remove Message integrity check, or any redundant data */
            extra_len =  skb->len - omci_msg_len;
            if(unlikely(OMCI_CRC_LEN != extra_len) ){
                PON_MSG(MSG_ERR, "Message integrity check length fault\n");
            }
            skb->len -= extra_len;
        }
    }

    return XPON_SUCCESS;
}

static int gpon_igmp_uni_vlan_operation(struct sk_buff * skb)
{
	int is_igmp_handle = 0;
	if(2 == get_pon_link_type()){ //2:HGU)
		return is_igmp_handle;
	}
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
    if((xpon_sfu_multicast_protocol_hook) && (xpon_sfu_multicast_protocol_hook(skb))
        && (isVlanOperationInMulticastModule_hook) && (isVlanOperationInMulticastModule_hook(skb))) {
        is_igmp_handle = 1;
    }

    if (is_igmp_handle) {
        if ((xpon_sfu_up_multicast_vlan_hook) && (xpon_sfu_up_multicast_vlan_hook(skb,0) == -1)){
            XPON_DROP_PRINT;
            return -1;
        }

        if ((xpon_up_igmp_uni_vlan_filter_hook) && (xpon_up_igmp_uni_vlan_filter_hook(skb) == -1)){
            XPON_DROP_PRINT;
        	return -1;
        }
    }
#endif
	return is_igmp_handle;
}

static int get_valid_data_gemIdx(struct sk_buff * skb, int dir)
{
	int gemIdx = 0 ;

	/* Just for FPGA Verification. the default gem port msut use default interface or dropped. */
	gemIdx = (skb->gem_port) ? (gpWanPriv->gpon.gemIdToIndex[skb->gem_port] & GPON_GEM_IDX_MASK) : GPON_GEMPORT_MAX_NUM ;
	if(dir == XPON_WAN_TX_DIR){
		if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid 
			|| (gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL 
			|| gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) 
			|| gpWanPriv->gpon.gemPort[gemIdx].info.ani >= GPON_MAX_ANI_INTERFACE 
			|| GPON_UNASSIGN_ALLOC_ID == gpWanPriv->gpon.allocId[gpWanPriv->gpon.gemPort[gemIdx].info.channel]) 
		{
			XPON_DROP_PRINT;
			return -1 ;
		}
	}else if(dir == XPON_WAN_RX_DIR){
		if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid 
			|| ((GPON_MULTICAST_CHANNEL != gpWanPriv->gpon.gemPort[gemIdx].info.channel) &&
             gpWanPriv->gpon.gemPort[gemIdx].info.ani>=GPON_MAX_ANI_INTERFACE ))
        {
			XPON_DROP_PRINT;
			return -1 ;
		}
	}else{
		XPON_DROP_PRINT;
		return -1 ;
	}

	skb->v_if = gpWanPriv->gpon.gemPort[gemIdx].info.ani ;
	return gemIdx;
}

static int gpon_ani_vlan_filter(struct sk_buff * skb, int igmp_handle)
{
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER) && defined(TCSUPPORT_XPON_IGMP)
	if (igmp_handle)
	{
		if ((xpon_up_igmp_ani_vlan_filter_hook) && (xpon_up_igmp_ani_vlan_filter_hook(skb) == -1)){
			XPON_DROP_PRINT;
			return -1;
		}
	}
#endif
	
#ifdef TCSUPPORT_PON_VLAN
	if (!igmp_handle)
	{

		skb->pon_vlan_flag |= PON_PKT_SEND_TO_WAN;
		if(pon_insert_tag_hook)
		{
			if(pon_insert_tag_hook(&skb) == -1){
				XPON_DROP_PRINT;
				return -1;
			}
		}
	}
#endif
	return 0;
}

static int gpon_mac_filter(struct sk_buff * skb)
{
#ifdef TCSUPPORT_PON_MAC_FILTER
		skb->pon_mac_filter_flag |= PKT_SEND_TO_WAN;
		if(pon_check_mac_hook)
		{
			if(pon_check_mac_hook(skb) == -1){
				XPON_DROP_PRINT;
				return -1;
			}
		}
#endif
	return 0;
}

static int gpon_igmp_pkt_handle(struct sk_buff * skb)
{
#if defined(TCSUPPORT_XPON_IGMP)
	    if(xpon_sfu_multicast_protocol_hook && xpon_sfu_multicast_protocol_hook(skb)) {
	        if(xpon_sfu_up_multicast_incoming_hook) {
	            if(xpon_sfu_up_multicast_incoming_hook(skb, 1) <= 0) {
	                XPON_DROP_PRINT;
	                return -1;
	            }
	        }
	    }
#endif   
	return 0;
}
/*****************************************************************************
******************************************************************************/
int gwan_config_tx_msg(struct sk_buff * skb,PWAN_FETxMsg_T *pTxMsg, uint8_t netIdx)
{
#ifdef TCSUPPORT_CPU_EN7580
	unsigned char acnt_g0 = HWNAT_ACNT_G0_DEFAULT;
	unsigned char acnt_g1 = HWNAT_ACNT_G1_DEFAULT;
	unsigned char tsid = HWNAT_TSID_DEFAULT;
#endif
	int gemIdx = 0 ;

	pTxMsg->raw.fport = TXMSG_FPORT_GMAC ;
	if(netIdx == PWAN_IF_OMCI) {
		pTxMsg->raw.oam = 1 ;
		pTxMsg->raw.channel = 0 ;
#ifdef TCSUPPORT_CPU_EN7580
		pTxMsg->raw.nboq = 0 ;
		pTxMsg->raw.ndp = 1 ;
		pTxMsg->raw.mtr_g = tsid;
		pTxMsg->raw.acnt_g0 = acnt_g0;
		pTxMsg->raw.acnt_g1 = acnt_g1;
#endif
		pTxMsg->raw.gem = GPON_OMCC_ID ;
	}
	else 
	{
		gemIdx = get_valid_data_gemIdx(skb, XPON_WAN_TX_DIR);
		if(gemIdx == -1){
			XPON_DROP_PRINT;
            return -1;
		}
		pTxMsg->raw.oam = 0 ;
		pTxMsg->raw.gem = gpWanPriv->gpon.gemPort[gemIdx].info.portId ;
		pTxMsg->raw.channel = gpWanPriv->gpon.gemPort[gemIdx].info.channel ;
#ifdef TCSUPPORT_CPU_EN7580
		pTxMsg->raw.nboq = gpWanPriv->gpon.gemPort[gemIdx].info.channel ;
#endif

		if((ra_sw_nat_hook_magic) && (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE))) {
			pTxMsg->raw.fport = TXMSG_FPORT_PPE ;//DPORT_PPE
		} 

#ifdef TCSUPPORT_CPU_EN7580			
        FE_API_GET_METER_IDX(skb, UP_STREAM, &tsid, 0);
        pTxMsg->raw.mtr_g = tsid;
        /*prapare acont_g0 & acont_g1,and pass them to QDMA tx_msg,which used in QDMA_API_TRANSMIT_PACKETS*/
        FE_API_GET_ACNT1_IDX(skb, UP_STREAM, &acnt_g1);
        FE_API_GET_ACNT0_IDX(skb, UP_STREAM, &acnt_g0);
        pTxMsg->raw.acnt_g0 = acnt_g0;
        pTxMsg->raw.acnt_g1 = acnt_g1;
#endif
	}

	return 0;
}


int gwan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, struct port_info *xpon_info)
{
	int is_igmp_handle = 0;

	if(netIdx == PWAN_IF_OMCI) {
        if(XPON_SUCCESS != remove_omci_crc_if_exist(skb)){
            XPON_DROP_PRINT;
            return -1;
        }
        
		gwan_config_tx_msg(skb, pTxMsg, netIdx);
		if(gpPonSysData->debugLevel & MSG_OMCI) {
			PON_MSG(MSG_OMCI, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
			__dump_skb(skb, skb->len) ;
		}
	}
	else {
		
#if defined(TCSUPPORT_PON_VLAN)		
		skb->pon_vlan_flag |= PON_VLAN_TX_CALL_HOOK;		
		if(pon_insert_tag_hook)		
		{			
			if(pon_insert_tag_hook(&skb) == -1){				
				XPON_DROP_PRINT;				
				return -1;			
			}		
		}
#endif
		if(xpon_wan_restore_offload_info(skb,xpon_info,pTxMsg, FOE_MAGIC_GPON)){ 
			PON_MSG(MSG_TRACE, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
			return 0;
		}

		is_igmp_handle = gpon_igmp_uni_vlan_operation(skb);
		if(is_igmp_handle == -1){
			XPON_DROP_PRINT;
            return -1;
		}

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FH_PON)		
			/*customer define*/
#else/*TCSUPPORT_COMPILE*/
		if(ECNT_API_GPON_FLOW_UPSTREAM_ANI_HOOK(skb)){
			XPON_DROP_PRINT;
			return -1;
		}
#endif/*TCSUPPORT_COMPILE*/

		if( -1 == gwan_tx_vlan_gem_map_handler_customer(skb, netIdx) ){
			XPON_DROP_PRINT;
			return -1;
		}
		/*
			config hwnat gemport, channel, force port, tsid(7580), counter statistic(7580),
			set skb->v_if(ANI port)
		*/
		if(-1 == gwan_config_tx_msg(skb, pTxMsg, netIdx)){
			XPON_DROP_PRINT;
            return -1;
		}
/****************************************************************************************************
/////////////////// ANI Port Info Must After gwan_config_tx_msg ////////////////////////////////////
*****************************************************************************************************/
		/*need ani port for vlan filter*/
		if(-1 == gpon_ani_vlan_filter(skb, is_igmp_handle)){
			XPON_DROP_PRINT;
            return -1;
		}
		/*need ani port for mac filter*/
		if(-1 == gpon_mac_filter(skb)){
			XPON_DROP_PRINT;
            return -1;
		}
		/*for igmp packet(join/leave) handle after all vlan operation*/
		if(-1 == gpon_igmp_pkt_handle(skb)){
			XPON_DROP_PRINT;
            return -1;
		}

		xpon_info->magic = FOE_MAGIC_GPON;
		xpon_info->stag = pTxMsg->raw.gem ;
		xpon_info->channel = pTxMsg->raw.channel;
#ifdef TCSUPPORT_CPU_EN7580			
		xpon_info->nbq = xpon_info->channel;
        xpon_info->tsid = pTxMsg->raw.mtr_g;
#endif
        
        if(gpWanPriv->gpon.hgu_mode_txq) {
            xpon_info->txq_is_valid = TRUE;
            xpon_info->txq = (skb->mark & QOS_FILTER_MARK) >> 4 ;
			
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---omci config txq xpon_info->txq = 0x%x\n",xpon_info->txq) ;
        }
        else
        {
            xpon_info->txq_is_valid = FALSE;
        }
		PON_MSG(MSG_TRACE, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
		PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---gemid = 0x%x\n",xpon_info->stag) ; 
		PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->channel = 0x%x\n",xpon_info->channel) ;
		PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->txq = 0x%x\n",xpon_info->txq) ;
		PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->tsid = 0x%x\n",xpon_info->tsid) ;
	}
	
	return 0 ;
}

void gwan_omci_check(struct sk_buff *skb){
    unchar * dataPtr = NULL;
    unchar msgType = 0;
    unsigned short * data16Ptr = NULL;
    unsigned short classId = 0;

    if(skb == NULL){
        printk("ERROR: func:%s skb is NULL !!!\n",__func__);
        return;
    }

    dataPtr = (unchar *)skb->data;
    msgType = dataPtr[2];
    data16Ptr = (unsigned short*)(&dataPtr[4]);
    classId = ntohs(*data16Ptr);
		
    if((msgType & 0x1f) == OMCI_MSG_TYPE_MIB_UPLOAD){
        gpon_init_qdma_tx_buff();
        if(gpMonitor->run_state != MONITOR_RUNNING){
            start_omci_oam_monitor();
        }
    }else if(channel_switch_enable && (((msgType & 0x1f) == OMCI_MSG_TYPE_CREATE) || 
		((msgType & 0x1f) == OMCI_MSG_TYPE_SET) || ((msgType & 0x1f) == OMCI_MSG_TYPE_DELETE))
		&& (classId  == OMCI_CLASS_ID_MAC_BRIDGE_PORT_CFG || classId  == OMCI_CLASS_ID_802_1P ||
		classId  == OMCI_CLASS_ID_EXT_VLAN_OP || classId  == OMCI_CLASS_ID_GEM_CTP)){
        #if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_NG)
        gpon_enable_cpu_protection(300);
        #endif/*TCSUPPORT_COMPILE*/
    }
    
    return;
}

/*****************************************************************************
******************************************************************************/
static int gwan_rx_loopback_per_gem(int gemIdx)
{
	int loopback_enable = 0;
	if(gpWanPriv->gpon.gemPort[gemIdx].info.rxLb) {
		if(gpWanPriv->gpon.gemPort[gemIdx].info.channel<GPON_TCONT_MAX_NUM) {
			loopback_enable = 1 ;
			PON_MSG(MSG_TRACE, "RX Gemport=%d loopback\n", gpWanPriv->gpon.gemPort[gemIdx].info.portId) ;
		}
	}

	return loopback_enable;
}

static void gwan_rx_set_igmp_pkt_vlan_flag(struct sk_buff *skb)
{
#ifdef TCSUPPORT_XPON_IGMP_CHT
	unsigned char * mac = NULL;
	mac = skb->data;
	if(!isMulticastPkt(mac))
	{
	#ifdef TCSUPPORT_VLAN_TAG
		skb->vlan_tag_flag |= VLAN_TAG_INSERT_FLAG;
		skb->vlan_tag_flag |= VLAN_TAG_FROM_INDEV;
	#endif

	#ifdef TCSUPPORT_PON_VLAN
		skb->pon_vlan_flag |= PON_PKT_INSERT_FLAG;
	#endif
	}
#endif
}
void gwan_check_ds_omci_type(struct sk_buff * skb){

    pOmci_Header pOmciHeader = NULL;

    pOmciHeader = (pOmci_Header)skb->data;

    if(OMCI_BASIC_MSG_DEV_ID == pOmciHeader->devId || OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*Total omci*/
        gpWanPriv->gpon.rx_omci_cnt++;
    }

    if(OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*extended omci*/
        gpWanPriv->gpon.rx_omci_extend_cnt++;
    }
}

static void gwan_rx_gem_statistic_update(int gemIdx, uint32_t pktLens)
{
	gpWanPriv->gpon.gemPort[gemIdx].stats.rx_packets++ ;
	gpWanPriv->gpon.gemPort[gemIdx].stats.rx_bytes += pktLens ;
}
int gwan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx = -1;
	uint gemIdx ;

	if(pRxMsg->raw.crcer) {
		if(gpPonSysData->debugLevel & MSG_ERR) {
			PON_MSG(MSG_ERR, "Packet Ethernet CRC Error:%.8x, %.8x, %.8x, %.8x\n", pRxMsg->word[0], pRxMsg->word[1], pRxMsg->word[2], pRxMsg->word[3]) ;
			__dump_skb(skb, pktLens) ;
		}
	}
	
	if(pRxMsg->raw.oam) {
		netIdx = PWAN_IF_OMCI ;
		if(GPON_OMCC_ID != GPON_UNASSIGN_GEM_ID){
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK);
		}else{
			XPON_DROP_PRINT;
			return -1;
		}
		gwan_rx_gem_statistic_update(gemIdx, pktLens);
        gwan_check_ds_omci_type(skb);
        gwan_omci_check(skb); 
        
		if(gpPonSysData->debugLevel & MSG_OMCI) {
			PON_MSG(MSG_OMCI, "RX PKT LEN: %d, Rx Msg: %.8x, %.8x", pktLens, pRxMsg->word[0], pRxMsg->word[1]) ;
			__dump_skb(skb, pktLens) ;
		}

		if(omci_drop_flag){
			return -1;
		}
	} else {
		netIdx = PWAN_IF_DATA ;		
		skb->gem_port = pRxMsg->raw.gem ;
		gemIdx = get_valid_data_gemIdx(skb, XPON_WAN_RX_DIR);
		if(gemIdx == -1){
			XPON_DROP_PRINT;
            return -1;
		}

		gwan_rx_gem_statistic_update(gemIdx, pktLens);
        if(GPON_MULTICAST_CHANNEL == gpWanPriv->gpon.gemPort[gemIdx].info.channel){
            skb->gem_type = GPON_MULTICAST_GEM ;
        }else{
            skb->gem_type = GPON_UNICAST_GEM ;           
        }

		skb->pon_mark |= DS_PKT_FORM_WAN;

		if(gwan_rx_loopback_per_gem(gemIdx)){
			*pFlag = 1;
			return netIdx;
		}

		/* do PCP encode/decode check, if PCP self test enabled */
		if(gpGponPriv->gponCfg.dvtPcpCheck) {
			gpon_dvt_pcp_check(gpGponPriv->gponCfg.dvtPcpCheck, skb) ;
		}
		
#ifdef TCSUPPORT_HWNAT_L2VID
		if(ra_sw_nat_hook_rx_set_l2lu)
			ra_sw_nat_hook_rx_set_l2lu(skb, 0, pRxMsg->raw.ppe);
#endif
	
		

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FH_PON)		
        /*customer define*/
#else/*TCSUPPORT_COMPILE*/
		if(ECNT_API_GPON_FLOW_DOWNSTREAM_ANI_HOOK(skb)){
                XPON_DROP_PRINT;
                return -1;
        }
#endif/*TCSUPPORT_COMPILE*/

		gwan_rx_set_igmp_pkt_vlan_flag(skb);

	}

		
	return netIdx ;	
}

/*****************************************************************************
******************************************************************************/
void gwan_update_gem_mib_table(TIMER_FUN_PAAM arg)
{
	int idx=0, i, j ;
	uint gemIdx ;
	
#ifndef TCSUPPORT_CPU_EN7521
	if(atomic_read(&fe_reset_flag) ) {
		return ;
	}
#endif				
	gponDevUpdateGemMibIdxTable(0, 0) ;
	for(j=1, i=0 ; i<GPON_MAX_GEM_ID ; i++) {
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK);
		if(gemIdx<GPON_GEMPORT_MAX_NUM && gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
			idx++ ;
		}
		if((i&0xF)==0xF && j!=256) {
			gponDevUpdateGemMibIdxTable(j, idx) ;
			j++ ;
		}
	}
}

/*****************************************************************************
******************************************************************************/
int gwan_create_new_tcont(ushort allocId)
{
	char retValue ;
	unchar channel ;
#ifndef TCSUPPORT_CPU_EN7521
	uint regVal = 0;
#endif
	uint i=0;
#if defined(TCSUPPORT_CPU_EN7580)
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
	QDMA_TxQCngstChannelCfg_T txChannelCfg = {0};
#endif
		
	retValue = gponDevEnableTCont(allocId) ;
	if((retValue >= 0) && (retValue < GPON_TCONT_MAX_NUM)) {
		channel = retValue ;

		gpWanPriv->gpon.allocId[channel] = allocId ;
		for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
			if(gpWanPriv->gpon.gemPort[i].info.valid == 1 && gpWanPriv->gpon.gemPort[i].tcontID == allocId){
				gpon_recovery_set_channel(gpWanPriv->gpon.gemPort[i].info.portId, channel);
				gpWanPriv->gpon.gemPort[i].info.channel = channel;
			}
		}
#if defined(TCSUPPORT_CPU_AN7583)
		FE_API_SET_PSE_OQ_RSV_ENABLE(FE_DP_GDM2, i, FE_DISABLE);
#endif
		#ifdef TCSUPPORT_CPU_EN7521
		/* Enable GDM2 channel Tx */
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, channel, FE_ENABLE);
		/* Enable CDM2 Rx DMA */
#if defined(TCSUPPORT_CPU_EN7580)
		chnlCloseStatusSet.chnlIdx = channel;
		chnlCloseStatusSet.chnlStatus = 0x0;
		QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
		if(GPON_ONU_ID == allocId)  /*set omci channel flowctrl nonblocking*/
		{
			txChannelCfg.channel = channel;
			txChannelCfg.channelMode = QDMA_CHANNEL_NONBLOCKING;
			QDMA_API_SET_TXQ_CNGST_CHANNEL_NONBLOCKING(ECNT_QDMA_WAN, &txChannelCfg);
		}
#else
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, channel, FE_ENABLE);
#endif
		
		#else
		/* Enable LMGR channel Tx */
		regVal = READ_FE_REG(0xBFB51844);
		regVal = regVal & (~(1<<channel));
		WRITE_FE_REG(0xBFB51844, regVal);

		/* Enable GDM2 channel Tx */
		regVal = READ_FE_REG(0xBFB5152C);
		regVal = (regVal | (1<<channel));
		WRITE_FE_REG(0xBFB5152C, regVal);

		/* MT7520 has no CDM2 rx DMA register */
		#endif

		
		return 0 ;
	} 
	
	return retValue ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_tcont(ushort allocId)
{
	int i ;
	unchar change_service_flag = false;
#if defined(TCSUPPORT_CPU_EN7580)
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
#endif
#ifndef TCSUPPORT_CPU_EN7521
    uint regVal = 0;
#endif

	for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) {
		if(gpWanPriv->gpon.allocId[i] == allocId) {
		
            if((2 == gpWanPriv->activeChannelNum) && (allocId != gpGponPriv->gponCfg.onu_id))
            {
                change_service_flag = true;
            }
            
			gpWanPriv->gpon.allocId[i] = GPON_UNASSIGN_ALLOC_ID ;

			#ifdef TCSUPPORT_CPU_EN7521
			/* Disable Channel for GDM2 Tx */
			//FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
			
			/* Disable CDM2 Rx DMA */
			
#if defined(TCSUPPORT_CPU_EN7580)
			chnlCloseStatusSet.chnlIdx = i;
			chnlCloseStatusSet.chnlStatus = 0xFF;
			QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
#else
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
#endif
			/* retire single channel */
#if defined(TCSUPPORT_CPU_EN7580)
			FE_API_SET_CHANNEL_RETIRE_ONE((FE_Gdma_Sel_t)FE_CDM_SEL_CDMA2,i);
#endif

#if defined(TCSUPPORT_CPU_AN7583)
			FE_API_SET_PSE_OQ_RSV_ENABLE(FE_DP_GDM2, i, FE_DISABLE);
#endif

			#else
			/* Disable LMGR channel Tx */
			regVal = READ_FE_REG(0xBFB51844);
			regVal |= (1<<i);
			WRITE_FE_REG(0xBFB51844, regVal);
			
			/* Disable GDM2 channel Tx */
			regVal = READ_FE_REG(0xBFB5152C);
			regVal &= ~(1<<i);
			WRITE_FE_REG(0xBFB5152C, regVal);

			/* MT7520 has no CDM2 rx DMA register */
			#endif

			delay1us(g_rm_tcont_delay_us);
			gponDevDisableTCont(allocId);
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)
			if((change_service_flag) && (0 == g_omci_creat_tcont))
#else/*TCSUPPORT_COMPILE*/
			if(change_service_flag)
#endif/*TCSUPPORT_COMPILE*/
			{
			    g_service_change_flag = 1;
			    change_service_flag   = false;
			}

			return 0 ;
		}
	}
	
	return -ENOENT ;
}

/*****************************************************************************
******************************************************************************/
int gwan_modify_tcont_channel_id(unsigned char channel)
{
	int i = 0;
	char retValue = 0;
	ushort allocId = 0;
#ifndef TCSUPPORT_CPU_EN7521
	uint regVal = 0;
#endif
	unsigned char oldChannel = 0;
	unsigned char newChannel = channel;

	/*Get a using alloc id, and release it's channel.*/
	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		if((gpWanPriv->gpon.allocId[i] != GPON_UNASSIGN_ALLOC_ID) /*Get a valid alloc id.*/
			&& (gpWanPriv->gpon.allocId[i] != GPON_ONU_ID))
		{
			if (ra_sw_nat_hook_clean_table)
			{
				ra_sw_nat_hook_clean_table();
			}
			oldChannel = i;
			allocId = gpWanPriv->gpon.allocId[oldChannel];
			gpWanPriv->gpon.allocId[oldChannel] = GPON_UNASSIGN_ALLOC_ID;
			gpWanPriv->gpon.allocId[newChannel] = allocId;

			/*Update the gemports channel id of current tcont.*/
			for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
				if(gpWanPriv->gpon.gemPort[i].info.valid && (gpWanPriv->gpon.gemPort[i].info.channel == oldChannel)) {
					gpWanPriv->gpon.gemPort[i].info.channel = newChannel ;
				}
			}

			/*Use current new channel, and band it with the alloc id.*/
			retValue = gponDevSetTCont(GPON_TCONT_VALID, newChannel, allocId);
			if(retValue != GPON_TCONT_CMD_SUCCESS) {
				return -EFAULT ;
			}

		#ifdef TCSUPPORT_CPU_EN7521
			/* Enable GDM2 channel Tx */
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, newChannel, FE_ENABLE);
			/* Enable CDM2 Rx DMA */
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, newChannel, FE_ENABLE);
		#else
			/* Enable LMGR channel Tx */
			regVal = IO_GREG(0xBFB51844);
			regVal = regVal & (~(1<<newChannel));
			IO_SREG(0xBFB51844, regVal);

			/* Enable GDM2 channel Tx */
			regVal = IO_GREG(0xBFB5152C);
			regVal = (regVal | (1<<newChannel));
			IO_SREG(0xBFB5152C, regVal);

			/* MT7520 has no CDM2 rx DMA register */
		#endif

		/*Disable the old channel.*/
		#ifdef TCSUPPORT_CPU_EN7521
			/* Disable Channel for GDM2 Tx */
			//FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, oldChannel, FE_DISABLE);

			/* Disable CDM2 Rx DMA */
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, oldChannel, FE_DISABLE);
		#else
			/* Disable LMGR channel Tx */
			regVal = IO_GREG(0xBFB51844);
			regVal |= (1<<oldChannel);
			IO_SREG(0xBFB51844, regVal);
			
			/* Disable GDM2 channel Tx */
			regVal = IO_GREG(0xBFB5152C);
			regVal &= ~(1<<oldChannel);
			IO_SREG(0xBFB5152C, regVal);

			/* MT7520 has no CDM2 rx DMA register */
		#endif

			delay1us(g_rm_tcont_delay_us);

			retValue = gponDevSetTCont(GPON_TCONT_INVALID, oldChannel, 0xFF);
			if(retValue != GPON_TCONT_CMD_SUCCESS) {
				return -EFAULT ;
			}

			printk("Switch Channel:Alloc ID %d, From channel ID %d to %d\n",allocId,oldChannel,newChannel);
			return 0 ;
		}
	}
	return -ENOENT ;
}

int gwan_all_channels_unused(int number, unsigned char *channelPtr)
{
	int i = 0;

	if((number > CONFIG_GPON_MAX_TCONT) || (channelPtr == NULL)) {
		return -EINVAL ;
	}
	for(i=0 ; i < number ; i++) {
		if(	gpWanPriv->gpon.allocId[*channelPtr] != GPON_UNASSIGN_ALLOC_ID){
			return -1;
		}
		channelPtr ++;
	}
	return 0;
}
int gwan_switch_channel_id(void)
{
#if !defined(TCSUPPORT_CPU_EN7527) && !defined(TCSUPPORT_CPU_EN7528) && !defined(TCSUPPORT_CPU_EN7580)
	int i = 0 ;
	QDMA_DscpChnlInfo_T DscpChnlInfo = {0};

	if(gpMonitor->run_state != MONITOR_DONE){
		return 0;
	}

	if(QDMA_API_GET_DSCP_CHNL_INFO(ECNT_QDMA_WAN, &DscpChnlInfo) == 0){
		if(0 == gwan_all_channels_unused(DscpChnlInfo.chnlNum, DscpChnlInfo.channel)){
			for(i =0; i < DscpChnlInfo.chnlNum; i++){
				gwan_modify_tcont_channel_id(DscpChnlInfo.channel[i]);
			}
			deallocate_flag = 0;
		}
	}
#endif
	return 0;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_all_tcont(void)
{
	int i ;

	for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) {
		gpWanPriv->gpon.allocId[i] = GPON_UNASSIGN_ALLOC_ID ;
	}

	return 0 ;
}

int gwanCheckAllocIdExist(ushort allocId)
{
	int i = 0;

	for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) 
	{
		if(gpWanPriv->gpon.allocId[i] == allocId )
			/*&& gponDevCheckTContReg(i, allocId))*/
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*****************************************************************************
******************************************************************************/
int gwan_create_new_gemport(ushort gemPortId, unchar channel, unchar encryption, ushort allocId)
{
	int i = 0;
	ushort encry_info;
	struct Gem_Recovery_Info bakGemPort = {0};

	if(gemPortId>=GPON_MAX_GEM_ID || (channel>=GPON_TCONT_MAX_NUM && channel!=GPON_MULTICAST_CHANNEL && channel!=GPON_UNKNOWN_CHANNEL)) {
		return -EINVAL ;
	}

	if(gpWanPriv->gpon.gemNumbers >= GPON_GEMPORT_MAX_NUM) {
		return -ENOSPC ;
	}

	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) 
	{
		if(gpWanPriv->gpon.gemPort[i].info.valid){
			continue;
		}
		
		memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
		memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;

		encry_info = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_ENCRY_MASK);
		gpWanPriv->gpon.gemIdToIndex[gemPortId] = i + encry_info;
		gpWanPriv->gpon.gemPort[i].info.portId = gemPortId ;
		gpWanPriv->gpon.gemPort[i].info.ani = (gemPortId==GPON_OMCC_ID) ? GPON_OMCI_INTERFACE : GPON_MAX_ANI_INTERFACE ;
		gpWanPriv->gpon.gemPort[i].info.channel = channel ;
		gpWanPriv->gpon.gemPort[i].info.rxEncrypt = (encryption) ? 1 : (encry_info ? 1 : 0) ;
		gpWanPriv->gpon.gemPort[i].info.valid = 1 ;
		gpWanPriv->gpon.gemPort[i].tcontID = allocId ;
		gpWanPriv->gpon.gemNumbers++ ;
#ifdef TCSUPPORT_BBF_247
		/* 2476.2.12٣4gemportʱ */
        if(gpWanPriv->gpon.gemNumbers >= 5 && gpWanPriv->activeChannelNum >= 5)
        {
        	printk("gem num has reached 4 and tcont has reached 4, Open L2 bridge \n");
        	FE_API_SET_ETHER_TYEP(3, FE_ENABLE, FE_NOT_PPPOE, 0);
			FE_API_SET_ETHER_TYEP(4, FE_ENABLE, FE_NOT_PPPOE, 0);
        }
#endif
		if(gemPortId != GPON_OMCC_ID)
		{

			xpon_send_traffic_up_event();
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_CMCC)
			change_pon_led_status(ALARM_LED_ON);
#endif/*TCSUPPORT_COMPILE*/	
			
			bakGemPort.allocId = allocId;
			bakGemPort.gemPortId = gemPortId;
			bakGemPort.channel = channel;
			bakGemPort.encryption = encryption;
			bakGemPort.ani = gpWanPriv->gpon.gemPort[i].info.ani;
			gpon_recover_backup_gemport(&bakGemPort);
		}
		else
		{
			GPON_OMCC_GEM_ENCRYPTION = encryption;
		}
		
		gponDevSetGemInfo(gemPortId, XPON_ENABLE, (gpWanPriv->gpon.gemPort[i].info.rxEncrypt)?XPON_ENABLE:XPON_DISABLE) ;

		mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ; /* 2000 ms */
		
		return 0 ;
	}
	
	return -ENOENT ;
}

/*****************************************************************************
******************************************************************************/
int gwan_config_gemport(ushort gemPortId, ENUM_GWanGemCfgType_t cfgType, uint value)
{
	uint gemIdx ;
	struct Gem_Recovery_Info * tmp = NULL;

	if(gemPortId >= GPON_MAX_GEM_ID) {
		return -EINVAL ;
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
	if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || gpWanPriv->gpon.gemPort[gemIdx].info.portId!=gemPortId) {
		return -ENOENT ;
	}

	tmp = find_backup_gemport(gemPortId);

	switch(cfgType) {
		case ENUM_CFG_NETIDX:
			DBG_INFO(gponRecovery.dbgPrint,"begin to configure gemport %d ani %d\n",gemPortId,value);
            if(gpWanPriv->gpon.gemPort[gemIdx].info.ani == GPON_MAX_ANI_INTERFACE){
			    gpWanPriv->gpon.gemPort[gemIdx].info.ani = (value>=GPON_MAX_ANI_INTERFACE) ? GPON_MAX_ANI_INTERFACE : value ;
			    gpon_recovery_set_ani(gemPortId, gpWanPriv->gpon.gemPort[gemIdx].info.ani);
			}else{
				DBG_INFO(gponRecovery.dbgPrint,"gemport %d ani %d has be configed yet \n",gemPortId,gpWanPriv->gpon.gemPort[gemIdx].info.ani);
            }
            
			break ;
			
		case ENUM_CFG_CHANNEL:
			if(value>GPON_UNKNOWN_CHANNEL) {
				return -EINVAL ;
			}			
			gpWanPriv->gpon.gemPort[gemIdx].info.channel = value ;

			if(NULL != tmp)
			{
				if(GPON_MULTICAST_CHANNEL == value){
					tmp->allocId = GPON_UNASSIGN_ALLOC_ID;
				}else if(value<GPON_TCONT_MAX_NUM){
					tmp->allocId = gpWanPriv->gpon.allocId[value];
				}else{
					DBG_INFO(gponRecovery.dbgPrint,"%s %d; backup gemportId:%d with unknow channel\n", __FUNCTION__,__LINE__,gemPortId);
				}
				tmp->channel = value;
			
			}
			else if(gponRecovery.dbgPrint)
			{
			    printk("%s %d line change channel err\n", __FUNCTION__,__LINE__);
			}
			
			break ;
			
		case ENUM_CFG_ENCRYPTION:
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL) {
				return -EINVAL ;
			}			
			gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = (value)?1:0 ;
			gponDevSetGemInfo(gemPortId, XPON_ENABLE, (value)?XPON_ENABLE:XPON_DISABLE) ;
			if(gemPortId == GPON_OMCC_ID)
				GPON_OMCC_GEM_ENCRYPTION = (value)?1:0;
			if(NULL != tmp)
			{
				tmp->encryption = (value)?1:0;
			}
			else if(gponRecovery.dbgPrint)
			{
				printk("%s %d line change encryption err\n", __FUNCTION__,__LINE__);
			}
			
			break ;
			
		case ENUM_CFG_LOOPBACK:
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL || gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) {
				return -EINVAL ;
			}

			gpWanPriv->gpon.gemPort[gemIdx].info.rxLb = (value)?1:0 ;
			break ;
			
		default:
			return -EINVAL ;
	}
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_gemport(ushort gemPortId)
{
	uint gemIdx ;

	if(gemPortId>=GPON_MAX_GEM_ID || gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
	if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
		return -ENOENT ;
	}

	memset(&gpWanPriv->gpon.gemPort[gemIdx].info, 0, sizeof(GWAN_GemInfo_T)) ;
	memset(&gpWanPriv->gpon.gemPort[gemIdx].stats, 0, sizeof(struct net_device_stats)) ;
	gpWanPriv->gpon.gemIdToIndex[gemPortId] |= GPON_GEM_IDX_MASK ;
	
	gpWanPriv->gpon.gemNumbers-- ;
	gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE) ;
	mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ; /* 2000 ms */

	if((TRAFFIC_UP == gpGponPriv->gpon_traffic_status)&&(TRAFFIC_DOWN == is_really_up())){		
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;		
		gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;	
	}
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport_for_disable(void)
{
	ushort gemPortId ;
	int i ;
	
	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid && gpWanPriv->gpon.gemPort[i].info.portId!=GPON_OMCC_ID) {
			gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			gpWanPriv->gpon.gemIdToIndex[gemPortId] = GPON_GEM_IDX_MASK ;
			
			memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
		}
	}
	if((TRAFFIC_UP == gpGponPriv->gpon_traffic_status)&&(TRAFFIC_DOWN == is_really_up())){		
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;		
		gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;	
	}
	gpWanPriv->gpon.gemNumbers = 0 ;

	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport(void)
{
	ushort gemPortId ;
	int i ;
	
	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid && gpWanPriv->gpon.gemPort[i].info.portId!=GPON_OMCC_ID) {
			gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			gpWanPriv->gpon.gemIdToIndex[gemPortId] = GPON_GEM_IDX_MASK ;
			
			memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;

			gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE) ;
		}
	}

	gpWanPriv->gpon.gemNumbers = 0 ;
	gponDevResetGemInfo() ;
	gwan_update_gem_mib_table(0) ;
	if((TRAFFIC_UP == gpGponPriv->gpon_traffic_status)&&(TRAFFIC_DOWN == is_really_up())){		
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;		
		gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;	
	}
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_is_gemport_valid(ushort gemPortId)
{
	uint gemIdx ;

	if(gemPortId >= GPON_MAX_GEM_ID) {
		return -EINVAL ;
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
	return ((gemIdx<GPON_GEMPORT_MAX_NUM) ? gpWanPriv->gpon.gemPort[gemIdx].info.valid : -1) ;
}

/*****************************************************************************
******************************************************************************/
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t service_change_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len;
	loff_t off = *ppos;
	len = sprintf(buf, "%d\n", g_service_change_flag);

	len -= off;
	if (len > count)
		len = count;
	
	if (len < 0)
		len = 0;

	return len;
}

static ssize_t service_change_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    
	char val_string[8]={0};
    uint val=0;;
	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buf, count))
		return -EFAULT;
    
	val_string[count] = '\0';

	sscanf(val_string, "%d", &val);

    g_service_change_flag = val;
	
	return count;

}
#else
static int service_change_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", g_service_change_flag);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int service_change_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
    
	char val_string[8]={0};
    uint val=0;;
	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
    
	val_string[count] = '\0';

	sscanf(val_string, "%d", &val);

    g_service_change_flag = val;
	
	return count;

}
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static const struct proc_ops proc_service_change_ops = {
	.proc_read	= service_change_read_proc,
	.proc_write	= service_change_write_proc,
};
#endif
int service_change_proc_init(void)
{
    struct proc_dir_entry *service_change_proc = NULL;
    
    g_service_change_flag = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
    //with traffic, change service, reset    
    service_change_proc = proc_create("tc3162/service_change", 0, NULL, &proc_service_change_ops);
    if(NULL == service_change_proc)
    {
        printk("creat service change proc faile.\n");
        return -1;
    }
#else
    //with traffic, change service, reset    
    service_change_proc = create_proc_entry("tc3162/service_change", 0, NULL);
    if(NULL == service_change_proc)
    {
        printk("creat service change proc faile.\n");
        return -1;
    }
    
    service_change_proc->read_proc  = service_change_read_proc;
    service_change_proc->write_proc = service_change_write_proc;
#endif

    return 0;
    
}

GPON_TRAFFIC_STATUS_t gwan_get_traffic_status()
{
	int i = 0;
	
	if( (gpGponPriv->typeBOnGoing) || \
	    (GPON_STATE_O6 == GPON_CURR_STATE) ){
		return GPON_TRAFFIC_UP;
	}

	if( MONITOR_DONE != gpPonSysData->Omci_Oam_Monitor.run_state || \
		GPON_CURR_STATE != GPON_STATE_O5){
        return GPON_TRAFFIC_DOWN;
    }

	for(i = 0; i<GPON_GEMPORT_MAX_NUM; i++){
		if( (gpWanPriv->gpon.gemPort[i].info.valid) && \
			 (gpWanPriv->gpon.gemPort[i].info.portId != GPON_OMCC_ID) ){
			 break;
			}
		}
	if(GPON_GEMPORT_MAX_NUM == i){
		return GPON_TRAFFIC_DOWN;
		}

    return GPON_TRAFFIC_UP;
}

void gpon_set_qdma_tx_buf_timer_expires(TIMER_FUN_PAAM arg)
{
	xpon_reset_qdma_tx_buf();
}
/*****************************************************************************
******************************************************************************/
int gwan_init(GWAN_Priv_T *pGWanPriv)
{
	int i ;
	
	for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) {
		pGWanPriv->allocId[i] = GPON_UNASSIGN_ALLOC_ID;
	}

	for(i=0 ; i<GPON_MAX_GEM_ID ; i++) {
		pGWanPriv->gemIdToIndex[i] = GPON_GEM_IDX_MASK;
	}
	
	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
		memset(&pGWanPriv->gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
		memset(&pGWanPriv->gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
	}
	pGWanPriv->gemNumbers = 0 ;
    pGWanPriv->rx_omci_cnt = 0;
	pGWanPriv->rx_omci_extend_cnt = 0;
	GPON_CREATE_TIMER(&pGWanPriv->setQdmaTxBuffTimer,gpon_set_qdma_tx_buf_timer_expires,GPON_SET_QDMA_TX_BUFF_TIMER);/* 1,000 ms */
	GPON_CREATE_TIMER(&pGWanPriv->gemMibTimer,gwan_update_gem_mib_table,0);

    service_change_proc_init();
	pwan_create_net_interface(PWAN_IF_OMCI) ;

	pGWanPriv->hgu_mode_txq = 0;

#ifdef TCSUPPORT_CPU_EN7521
#ifdef TCSUPPORT_WAN_GPON
	/* disable all tx channel */
	for(i = 0; i < GPON_TCONT_MAX_NUM; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
	}
	/* disable all rx channel */
	for(i = 0; i < 16; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_DISABLE);
	}
	/* enable 2 rx channel */
	for(i = 0; i < 2; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
	}
	/* set GPON packet length */
	FE_API_SET_PACKET_LENGTH(FE_GDM_SEL_GDMA2, GPON_PACKET_LEN_UPPER_LIMIT, GPON_PACKET_LEN_LOWER_LIMIT);
#endif

	for(i = 0; i < GPON_TCONT_MAX_NUM; i++) {
		if(-1 == QDMA_API_SET_CNTR_CHANNEL(ECNT_QDMA_WAN, i)) {
			printk("gwan_init QDMA_API_SET_CNTR_CHANNEL %d failed\n",i);
		}
	}
	
#endif
	return 0 ;	
}

#endif /* TCSUPPORT_WAN_GPON */
