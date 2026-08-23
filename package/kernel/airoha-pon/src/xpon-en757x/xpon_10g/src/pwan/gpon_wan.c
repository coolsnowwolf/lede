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

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include "common/xpon_global.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include "common/xpon_api.h"
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif
#include "common/xpon_led.h"


#include "ecnt_hook/ecnt_hook_vlan.h"
#include <ecnt_hook/ecnt_hook_gpon_flow.h>
#include "pwan/xpon_netif.h"
#include "gpon/gpon_recovery.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/	
#define OMCI_CRC_LEN                (4)
#define OMCI_BASIC_MSG_FIX_LEN      (48)
#define OMCI_BASIC_MSG_DEV_ID       (0x0a)
#define OMCI_EXTENDED_MSG_DEV_ID    (0x0b)
#define OMCI_MIC_RESULT_LEN         (5)

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
typedef struct Omci_Hander{
	u16  transId;
	u8	 msgType;
	u8	 devId;
	u32    MeId;
	/* use in extended omci*/
	u16  msgContLen;
}__attribute__((packed)) Omci_Header_T, *pOmci_Header;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern int omciIkIdxExchange;
extern int dropCpuTxPktsFlag;
extern int omciMicErrSwCnt;

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
extern u32 random32(void);


/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
int g_rm_tcont_delay_us =  1000;

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
#ifdef TCSUPPORT_PON_MAC_FILTER
	extern int (*pon_check_mac_hook)(struct sk_buff *skb);
#endif

#ifdef TCSUPPORT_XPON_IGMP
extern int (*xpon_sfu_up_multicast_incoming_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_sfu_multicast_protocol_hook)(struct sk_buff *skb);
#endif


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

#ifdef TCSUPPORT_CPU_ARMV8_64
extern struct device* get_xpon_dev(void);
#endif

#if defined(TCSUPPORT_CPU_AN7583)
uint8 set_retired_channel[CONFIG_GPON_10G_MAX_TCONT] = {0};
#endif
uint8 clear_channel[CONFIG_GPON_10G_MAX_TCONT] = {0};
struct tasklet_struct clear_channel_task = {0};

/*****************************************************************************
******************************************************************************/
#if defined(TCSUPPORT_CPU_AN7583)
void  set_FE_CHN_RETIRE_DONE(TIMER_FUN_PAAM arg) {
	int i = 0;
		
	for(i=0; i<CONFIG_GPON_10G_MAX_TCONT; i++){
		if(XPON_ENABLE == set_retired_channel[i]){
			FE_API_SET_CHN_RETIRE_DONE(FE_GDM_SEL_GDMA2, i);
			set_retired_channel[i] = XPON_DISABLE;
		}
	}
}
#endif

void gwan_clear_channel_func(ulong data)
{	
	int i;	
	
	if (NULL ==ra_sw_nat_hook_clean_entry_by_channel){		
		return;	
	}	

	for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++){		

		if(XPON_ENABLE == clear_channel[i]){
			if(gpGponPriv->gponCfg.Wan2WanTrafficTest){
				;
			} else {
				ra_sw_nat_hook_clean_entry_by_channel(i);
			}			
			clear_channel[i] = XPON_DISABLE;		
		}	
	}


}

/*_____________________________________________________________________________
**      function name: gwan_update_gem_mib_table
**      descriptions:
**           It's used to update gem mib table, 32 for each unit, the number fo statistical gemport id is valid.
**      parameters:
**             None
**      global:
**             None
**      return:
**             None
**      call:
**   	         gponDevUpdateGemMibIdxTable
**      revision:
**            None
**____________________________________________________________________________
*/
void gwan_update_gem_mib_table(TIMER_FUN_PAAM arg)
{
	int idx=0, i=0, j =0 ;
	uint gemIdx =0;

	gponDevUpdateGemMibIdxTable(0, 0) ;
	for(j=1, i=0 ; i<GPON_10G_MAX_GEM_ID ; i++) {
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK);
		if(gemIdx<CONFIG_GPON_10G_MAX_GEMPORT && gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
			idx++ ;
		}
        if(((i&0x1F)==0x1F) && (j< XGPON_GPID_TAB_MAX_ADDR)) {
			gponDevUpdateGemMibIdxTable(j, idx) ;
            
			j++ ;
		}
	}
}
/*_____________________________________________________________________________
**      function name: gwan_get_gem_mib_table
**      descriptions:
**           It's used to get gem mib table .
**      parameters:
**            None
**      global:
**             None
**      return:
**             None
**      call:
**   	         gponDevGetGemMibIdxTable
**      revision:
**            None
**____________________________________________________________________________
*/
void gwan_get_gem_mib_table(void)
{
	int idx =0;
	int i =0;
	int j =0;
    uint sum =0;
	uint gemIdx =0 ;
	
	for(j=1, i=0 ; i<GPON_10G_MAX_GEM_ID ; i++) {
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK);
		if(gemIdx<CONFIG_GPON_10G_MAX_GEMPORT && gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
			idx++ ;
		}
		if(((i%0x20)==0x0) && (i!=0)&& (j!=2048)) {
			gponDevGetGemMibIdxTable(j, &sum) ;
            printk("j:%d sum:%d idx:%d i:%d (i/0x20):%d\n",j,sum,idx,i,(i%0x20));
			j++ ;
		}
	}
}
/*****************************************************************************
******************************************************************************/
int gwan_create_new_tcont(ushort allocId)
{
	int ret = 0;
	unchar channel = 0;
	uint i=0;
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
		
	ret = gponDevEnableTCont(allocId) ;
	if((ret>=0) && (ret < CONFIG_GPON_10G_MAX_TCONT)) {
		channel = ret ;
		gpWanPriv->gpon.allocId[channel] = allocId ;
		for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
			if(gpWanPriv->gpon.gemPort[i].info.valid == 1 && gpWanPriv->gpon.gemPort[i].info.allocId == allocId){
				gpon_recovery_set_channel(gpWanPriv->gpon.gemPort[i].info.portId, channel);
				gpWanPriv->gpon.gemPort[i].info.channel = channel;
			}
		}
#if defined(TCSUPPORT_CPU_AN7583)
		FE_API_SET_PSE_OQ_RSV_ENABLE(FE_DP_GDM2, channel, FE_ENABLE);
		if(!isFPGA) {
			FE_API_SET_CHN_RETIRE_DONE(FE_GDM_SEL_GDMA2, channel);
			set_retired_channel[channel] = XPON_DISABLE;
		} 
#endif
		/* Enable GDM2 channel Tx */
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, channel, FE_ENABLE);
		/* Enable CDM2 Rx DMA */
		//FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, channel, FE_ENABLE);
		chnlCloseStatusSet.chnlIdx = channel;
		chnlCloseStatusSet.chnlStatus = 0x0;
		QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
		/*no gpon recover function*/
		return 0 ;
	} 
	
	return ret ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_tcont(ushort allocId)
{
	int i=0 ;
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
	uint readData = 0;

	for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++) {
		if(gpWanPriv->gpon.allocId[i] == allocId) {
			gpWanPriv->gpon.allocId[i] = GPON_10G_UNASSIGN_ALLOC_ID ;
			/*
			if (ra_sw_nat_hook_clean_table){
				ra_sw_nat_hook_clean_table();
			}
			*/
			/* Disable Channel for GDM2 Tx */
			//FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
			/* Disable CDM2 Rx DMA */
			//FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
			chnlCloseStatusSet.chnlIdx = i;
			chnlCloseStatusSet.chnlStatus = 0xFF;
			QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
         
			udelay(g_rm_tcont_delay_us);
			gponDevDisableTCont(allocId);

#if defined(TCSUPPORT_CPU_AN7583)
			FE_API_SET_PSE_OQ_RSV_ENABLE(FE_DP_GDM2,i, FE_DISABLE);
			readData = (0x1 << 8) + i;
			IO_SREG(TX_FCS_TBL_CLEAR,readData);
			FE_API_SET_CHN_RETIRE_ACTION(FE_GDM_SEL_GDMA2, i);
			set_retired_channel[i] = XPON_ENABLE;
#else 
			readData = IO_GREG(G_TX_FCS_TBL_INIT);             
			readData = readData | 0x1;            
			IO_SREG(G_TX_FCS_TBL_INIT,readData);

			FE_API_SET_CHANNEL_RETIRE_ONE(FE_GDM_SEL_GDMA2, i);
#endif
			clear_channel[i] = XPON_ENABLE;
			tasklet_hi_schedule(&clear_channel_task);
			
			return 0 ;
		}
	}

	return -ENOENT ;
}
/*****************************************************************************
******************************************************************************/
int gwan_remove_all_tcont(void)
{
	int i = 0;

	for(i=0 ; i< CONFIG_GPON_10G_MAX_TCONT ; i++) {
	    gponDevDisableTCont(gpWanPriv->gpon.allocId[i]);
		gpWanPriv->gpon.allocId[i]  = GPON_10G_UNASSIGN_ALLOC_ID ;
	}
	
	return 0 ;
}

int gwanCheckAllocIdExist(ushort allocId)
{
	int i = 0;

	for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++) 
	{
		if(gpWanPriv->gpon.allocId[i] == allocId )
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

/*****************************************************************************
******************************************************************************/
int gwan_create_new_gemport(ushort gemPortId, unchar channel, unchar type, ushort allocId)
{
	int i = 0;
	ushort encry_info = 0;
	struct Gem_Recovery_Info bakGemPort = {0};

	if(gemPortId>GPON_10G_MAX_VALID_GEM_ID || (channel>=CONFIG_GPON_10G_MAX_TCONT && channel!=GPON_MULTICAST_CHANNEL && channel!=GPON_UNKNOWN_CHANNEL)) {
		return -EINVAL ;
	}

	if(gpWanPriv->gpon.gemNumbers >= CONFIG_GPON_10G_MAX_GEMPORT) {
		return -EINVAL ;
	}

	for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) 
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
		gpWanPriv->gpon.gemPort[i].info.valid = 1 ;
		gpWanPriv->gpon.gemPort[i].info.allocId = allocId ;
		gpWanPriv->gpon.gemNumbers++ ;

		if(gemPortId != GPON_OMCC_ID)
		{
			xpon_send_traffic_up_event();
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_CMCC)
			change_pon_led_status(LED_ON);
#endif/*TCSUPPORT_COMPILE*/	

			bakGemPort.allocId = allocId;
			bakGemPort.gemPortId = gemPortId;
			bakGemPort.channel = channel;
			bakGemPort.ani = gpWanPriv->gpon.gemPort[i].info.ani;
			gpon_recover_backup_gemport(&bakGemPort);

		}

		gponDevSetGemInfo(gemPortId, XPON_ENABLE, type, (gpWanPriv->gpon.gemPort[i].info.txEncrypt)?XPON_ENABLE:XPON_DISABLE) ;
		mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ; /* 2000 ms */
		return 0 ;
	}
	
	return -ENOENT ;
}
/*****************************************************************************
******************************************************************************/
int gwan_config_gemport_encrypt(ushort gemPortId, uint gemIdx, uint value)
{
	uint gemType = 0;
	struct Gem_Recovery_Info * backGem = NULL;

	backGem = find_backup_gemport(gemPortId);
	if(NULL != backGem)
	{
		backGem->encryption = value;				
	}
	else
	{		
		DBG_INFO(gponRecovery.dbgPrint,"%s %d line back gem encryption err \n",__FUNCTION__, __LINE__);
		return -ENOENT ;
	}

	if(gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) {		
		return -EINVAL ;
	}
	DBG_INFO(gponRecovery.dbgPrint, "%s %d line enter\n",__FUNCTION__,__LINE__);

	switch(value)
    {
        case GPON_UNICAST_ENCRYPTION:
            gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = XPON_ENABLE;
            gpWanPriv->gpon.gemPort[gemIdx].info.txEncrypt = (UPAES_MODE_14BIT== gpGponPriv->gemUpAESMode || UPAES_MODE_128BIT== gpGponPriv->gemUpAESMode) ?
                                                             XPON_ENABLE : XPON_DISABLE;           
            break;
            
        case GPON_BROADCAST_ENCRYPTION:
            gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = XPON_ENABLE;					
            gpWanPriv->gpon.gemPort[gemIdx].info.txEncrypt = XPON_DISABLE;             
            break;

        case GPON_UNICAST_ENCRYPTION_ONLY_DOWN:
            gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = XPON_ENABLE;					
            gpWanPriv->gpon.gemPort[gemIdx].info.txEncrypt = XPON_DISABLE;             
            break;

        case NO_ENCRYPTION:
            gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = XPON_DISABLE;
            gpWanPriv->gpon.gemPort[gemIdx].info.txEncrypt = XPON_DISABLE;
            break;

        default:
            printk("[%s %d] case error.\n", __FUNCTION__, __LINE__);
			break;
    }

	if(GPON_MULTICAST_CHANNEL == backGem->channel){
	    gemType = GPON_MULTICAST_GEM;
    }
    else{
		gemType = GPON_UNICAST_GEM;
    }			
    gponDevSetGemInfo(gemPortId, XPON_ENABLE, gemType, gpWanPriv->gpon.gemPort[gemIdx].info.txEncrypt);

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_config_gemport(ushort gemPortId, ENUM_GWanGemCfgType_t cfgType, uint value)
{
	uint gemIdx = 0;
	struct Gem_Recovery_Info * backGem = NULL;

	if(gemPortId > GPON_10G_MAX_VALID_GEM_ID) {
		return -EINVAL ;
	}
	
	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
	if(gemIdx>=CONFIG_GPON_10G_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || gpWanPriv->gpon.gemPort[gemIdx].info.portId!=gemPortId) {
		return -ENOENT ;
	}

	backGem = find_backup_gemport(gemPortId);

	switch(cfgType) {
		case ENUM_CFG_NETIDX:
			if(gpWanPriv->gpon.gemPort[gemIdx].info.ani == GPON_MAX_ANI_INTERFACE){
			gpWanPriv->gpon.gemPort[gemIdx].info.ani = (value>=GPON_MAX_ANI_INTERFACE) ? GPON_MAX_ANI_INTERFACE : value ;
			    gpon_recovery_set_ani(gemPortId, gpWanPriv->gpon.gemPort[gemIdx].info.ani);
			}else{
				DBG_INFO(gponRecovery.dbgPrint,"gemport %d ani %d has be configed yet \n",gemPortId,gpWanPriv->gpon.gemPort[gemIdx].info.ani);
            }
			break ;
			
		case ENUM_CFG_CHANNEL:
			if((value>=CONFIG_GPON_10G_MAX_TCONT) && (value!=GPON_MULTICAST_CHANNEL)) {
				return -EINVAL ;
			}			
			gpWanPriv->gpon.gemPort[gemIdx].info.channel = value ;
			if(NULL != backGem)
			{
				if(GPON_MULTICAST_CHANNEL == value){
					backGem->allocId = GPON_10G_UNASSIGN_ALLOC_ID;
				}else{
					backGem->allocId = gpWanPriv->gpon.allocId[value];
				}
				backGem->channel = value;			
			}
			break ;
			
		case ENUM_CFG_ENCRYPTION:
			gwan_config_gemport_encrypt(gemPortId,gemIdx,value);			
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
	uint gemIdx = 0;

	if(gemPortId > GPON_10G_MAX_VALID_GEM_ID || gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
	if((gemIdx >= CONFIG_GPON_10G_MAX_GEMPORT) || (!gpWanPriv->gpon.gemPort[gemIdx].info.valid)) {
		return -ENOENT ;
	}

	memset(&gpWanPriv->gpon.gemPort[gemIdx].info, 0, sizeof(GWAN_GemInfo_T)) ;
	memset(&gpWanPriv->gpon.gemPort[gemIdx].stats, 0, sizeof(struct net_device_stats)) ;
	gpWanPriv->gpon.gemIdToIndex[gemPortId] |= GPON_GEM_IDX_MASK ;
	
	gpWanPriv->gpon.gemNumbers-- ;
	gponDevSetGemInfo(gemPortId, XPON_DISABLE,XPON_DISABLE, XPON_DISABLE) ;
	mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ; /* 2000 ms */
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport_for_disable(void)
{
	ushort gemPortId = 0;
	int i = 0 ;
	
	for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid && gpWanPriv->gpon.gemPort[i].info.portId!=GPON_OMCC_ID) {
			gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			gpWanPriv->gpon.gemIdToIndex[gemPortId] = GPON_GEM_IDX_MASK ;
			
			memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
		}
	}
	gpWanPriv->gpon.gemNumbers = 0 ;
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport(void)
{
	ushort gemPortId = 0 ;
	int i = 0 ;
	
	for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid && gpWanPriv->gpon.gemPort[i].info.portId!=GPON_OMCC_ID) {
			gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			gpWanPriv->gpon.gemIdToIndex[gemPortId] = GPON_GEM_IDX_MASK ;
			
			memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;

			gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE, XPON_DISABLE) ;
		}
	}

	gpWanPriv->gpon.gemNumbers = 0 ;
	gponDevResetGemInfo() ;

	return 0 ;
}


/*****************************************************************************
******************************************************************************/
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

/******************************************************************************
******************************************************************************/
int gwan_add_us_omci_mic(struct sk_buff * skb){

    int ret = 0;
    pOmci_Header pOmciHeader = NULL;
    unchar * omci_mic =NULL; 
    unchar *omci_msg = NULL;
    unchar *omci_reslt = NULL;    
    u16 omci_msg_len = 0;    
    unchar cmacKeyIndex =0;
    ulong flags = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_omciMsg_phy;
	dma_addr_t dram_omciReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return XPON_FAIL;
	}
#endif

        
    pOmciHeader = (pOmci_Header)skb->data;

    if(OMCI_BASIC_MSG_DEV_ID == pOmciHeader->devId ){ /*baseline omci*/
        omci_msg_len = (OMCI_BASIC_MSG_FIX_LEN - OMCI_CRC_LEN);
    }else if(OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*extended omci*/
        omci_msg_len = ntohs(pOmciHeader->msgContLen) + sizeof(Omci_Header_T);
    }else{
        PON_MSG(MSG_ERR, "wrong omci message type\n");
        return XPON_FAIL;
    }
		
#ifdef TCSUPPORT_CPU_ARMV8_64
	PON_MSG(MSG_DBG, "US ARM architecture start cmac0 calculate omci mic \n") ;
	for (count = 0; (omci_msg==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		omci_msg = (unchar *)dma_alloc_coherent(xpon_dev, omci_msg_len, &dram_omciMsg_phy, GFP_ATOMIC);
	}

	if(omci_msg == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: gwan US ARM dma_alloc_coherent for omci_msg fail\n");
		return XPON_FAIL;
	}
	
	for (count = 0; (omci_reslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		omci_reslt = (unchar *)dma_alloc_coherent(xpon_dev, OMCI_MIC_RESULT_LEN, &dram_omciReslt_phy, GFP_ATOMIC);
	}

	if(omci_reslt == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: gwan US ARM dma_alloc_coherent for omci_reslt fail\n");
		dma_free_coherent(xpon_dev, omci_msg_len, omci_msg, dram_omciMsg_phy);
		return XPON_FAIL;
	}
#else
    omci_msg = CACHE_TO_NONCACHE((unchar *)kmalloc(omci_msg_len, GFP_KERNEL));
	if(omci_msg == NULL){
		 return XPON_FAIL;
	}
	
    omci_reslt = CACHE_TO_NONCACHE((unchar *)kmalloc(OMCI_MIC_RESULT_LEN, GFP_KERNEL)) ;
	if(omci_reslt == NULL){
		 kfree(NONCACHE_TO_CACHE(omci_msg));
		 return XPON_FAIL;
	}
#endif

	spin_lock_irqsave(&gpGponPriv->cmac_test, flags) ;
	
    memcpy(omci_msg,(unchar *)skb->data, omci_msg_len);
    memset(omci_reslt, 0x0,OMCI_MIC_RESULT_LEN);
    
    cmacKeyIndex =(gpGponPriv->gponSecurity.omciIkIdx == GPON_OMCI_IK_IDX0)? GPON_CMAC_OMCI_IDX0 : GPON_CMAC_OMCI_IDX1;

#ifdef TCSUPPORT_CPU_ARMV8_64
    ret =gponDevSetCmac0Start(cmacKeyIndex,GPON_CMAC_UPSTREAM,NULL,(unchar *)dram_omciMsg_phy,omci_msg_len,(unchar *)dram_omciReslt_phy,omci_reslt,5);
#else
	ret =gponDevSetCmac0Start(cmacKeyIndex,GPON_CMAC_UPSTREAM,NULL,omci_msg,omci_msg_len,omci_reslt,NULL,5);
#endif

    if(ret != 0){
       PON_MSG(MSG_DBG, "cmac0 calculate omci mic fail with key index:%d\n",gpGponPriv->gponSecurity.omciIkIdx) ;
    }
	else{
        skb->len += OMCI_CRC_LEN;
        omci_mic =skb->data + omci_msg_len;
        memcpy(omci_mic,omci_reslt, OMCI_CRC_LEN);
    }
  
    XPON_ARR_PRINT(MSG_DBG,"cmac golden Downstream omci mic reslt noncache data: ","%x ",omci_reslt,5);    
    
    spin_unlock_irqrestore(&gpGponPriv->cmac_test, flags) ;
    
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, omci_msg_len, omci_msg, dram_omciMsg_phy);
	dma_free_coherent(xpon_dev, OMCI_MIC_RESULT_LEN, omci_reslt, dram_omciReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(omci_msg));
    kfree(NONCACHE_TO_CACHE(omci_reslt)); 
#endif
    
    return XPON_SUCCESS;
}    
/******************************************************************************
******************************************************************************/
int gwan_check_ds_omci_mic(struct sk_buff * skb){

    int ret = XPON_SUCCESS;
    pOmci_Header pOmciHeader = NULL;
    unchar * omci_mic =NULL; 
    unchar *omci_msg = NULL;
    unchar *omci_reslt = NULL;    
    u16 omci_msg_len = 0;
    
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_omciMsg_phy;
	dma_addr_t dram_omciReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return XPON_FAIL;
	}
#endif
    
    pOmciHeader = (pOmci_Header)skb->data;

    if(OMCI_BASIC_MSG_DEV_ID == pOmciHeader->devId ){ /*baseline omci*/
        omci_msg_len = (OMCI_BASIC_MSG_FIX_LEN - OMCI_CRC_LEN);
    }else if(OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*extended omci*/
        omci_msg_len = ntohs(pOmciHeader->msgContLen) + sizeof(Omci_Header_T);
    }else{
        PON_MSG(MSG_ERR, "wrong omci message type\n");
        return XPON_FAIL;
    }

#ifdef TCSUPPORT_CPU_ARMV8_64
	PON_MSG(MSG_DBG, "DS ARM architecture start cmac0 calculate omci mic \n") ;
	for (count = 0; (omci_msg==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		omci_msg = (unchar *)dma_alloc_coherent(xpon_dev, omci_msg_len, &dram_omciMsg_phy, GFP_ATOMIC);
	}
	
	if(omci_msg == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: gwan DS ARM dma_alloc_coherent for omci_msg fail\n");
		return XPON_FAIL;
	}
	
	for (count = 0; (omci_reslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		omci_reslt = (unchar *)dma_alloc_coherent(xpon_dev, OMCI_MIC_RESULT_LEN, &dram_omciReslt_phy, GFP_ATOMIC);
	}

	if(omci_reslt == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: gwan DS ARM dma_alloc_coherent for omci_reslt fail\n");
		dma_free_coherent(xpon_dev, omci_msg_len, omci_msg, dram_omciMsg_phy);
		return XPON_FAIL;
	}
#else
    omci_msg = CACHE_TO_NONCACHE((unchar *)kmalloc(omci_msg_len, GFP_KERNEL));
	if(omci_msg == NULL)
		return XPON_FAIL;
	
	
    omci_reslt = CACHE_TO_NONCACHE((unchar *)kmalloc(OMCI_MIC_RESULT_LEN, GFP_KERNEL)) ;
	if(omci_reslt == NULL){
		kfree(NONCACHE_TO_CACHE(omci_msg));
		return XPON_FAIL;
	}
#endif

	memcpy(omci_msg,(unchar *)skb->data, omci_msg_len);
    memset(omci_reslt, 0x0,OMCI_MIC_RESULT_LEN);

#ifdef TCSUPPORT_CPU_ARMV8_64	
    ret =gponDevSetCmac0Start(GPON_CMAC_OMCI_IDX0,GPON_CMAC_DOWNSTREAM,NULL,(unchar *)dram_omciMsg_phy,omci_msg_len,(unchar *)dram_omciReslt_phy,omci_reslt,5);
#else
    ret =gponDevSetCmac0Start(GPON_CMAC_OMCI_IDX0,GPON_CMAC_DOWNSTREAM,NULL,omci_msg,omci_msg_len,omci_reslt,NULL,5);
#endif

    if(ret != 0){
       PON_MSG(MSG_ERR, "cmac0 calculate omci mic fail with key index:%d\n",gpGponPriv->gponSecurity.omciIkIdx) ;
       omciMicErrSwCnt++;
       ret = XPON_FAIL;
    }else{    
        omci_mic = skb->data + omci_msg_len;
        if(memcmp(omci_mic,omci_reslt, OMCI_CRC_LEN)!=0){
            PON_MSG(MSG_ERR, "cmac0 check ds omci mic err with key index:%d\n",gpGponPriv->gponSecurity.omciIkIdx) ;
           omciMicErrSwCnt++;
           ret = XPON_FAIL;
        }else{
            XPON_ARR_PRINT(MSG_OMCI, "cmac0 check ds omci mic success, calculated mic value:","%x ",omci_reslt,OMCI_CRC_LEN) ;
            ret = XPON_SUCCESS;
        }
    }
  
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, omci_msg_len, omci_msg, dram_omciMsg_phy);
	dma_free_coherent(xpon_dev, OMCI_MIC_RESULT_LEN, omci_reslt, dram_omciReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(omci_msg));
    kfree(NONCACHE_TO_CACHE(omci_reslt));   
#endif

    return ret;
}

/*****************************************************************************
******************************************************************************/
int gwan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, int txq, struct port_info *xpon_info)
{
	uint gemIdx = 0;
    unsigned char acnt_g0 = 0x1f, acnt_g1 = 0x1f;
    unsigned char tsid = 0x7f;
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
	int is_up_igmp_pkt = 0;
#endif
	uint chnMask = 0, qMask = 0;
	uint mtr_logic_idx = 0;

	if (1){// (isEN751221) {  
		/* PPE foe's channel field has 8 bits for EN751221 and 4 bits for MT751020	*/
		chnMask = 0xff;
		qMask = 0x7;
	} else {
		/* PPE foe's queue field has 3 bits for EN751221 and 4 bits for MT751020 */
		chnMask = 0xf;
		qMask = 0xf;
	}

	pTxMsg->raw.fport = TXMSG_FPORT_GMAC ;
	if(netIdx == PWAN_IF_OMCI) {
		pTxMsg->raw.oam = 1 ;

		if(XPON_SUCCESS != remove_omci_crc_if_exist(skb)){
			XPON_DROP_PRINT;
			return -1;
		}

        if((gpGponPriv->gponCfg.usOmciMicCtrl == XGPON_SW)){
            if(XPON_SUCCESS != gwan_add_us_omci_mic(skb)){
                XPON_DROP_PRINT;
                return -1;
            }
        }
        
		pTxMsg->raw.channel = 0 ;
		pTxMsg->raw.nboq= 0 ;
		pTxMsg->raw.gem = GPON_OMCC_ID ;
        pTxMsg->raw.ndp = 1 ;
		if(omciIkIdxExchange){
			pTxMsg->raw.mic_idx = gpGponPriv->gponSecurity.omciIkIdx ? GPON_OMCI_IK_IDX0 : GPON_OMCI_IK_IDX1;
		}else{
        	pTxMsg->raw.mic_idx = gpGponPriv->gponSecurity.omciIkIdx;
		}
        pTxMsg->raw.mtr_g = tsid;
		pTxMsg->raw.acnt_g0 = acnt_g0;
		pTxMsg->raw.acnt_g1 = acnt_g1;
#ifdef XPON_MAC_CONFIG_DEBUG
		if(gpPonSysData->debugLevel & MSG_OMCI) {
			PON_MSG(MSG_OMCI, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
			__dump_skb(skb, skb->len) ;
		}
#endif
	} else {

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

		if(dropCpuTxPktsFlag){
			return -1;
		}
		pTxMsg->raw.oam = 0 ;
        pTxMsg->raw.ndp = 0 ;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_HWNAT_OFFLOAD)
    if((skb->qosEnque & 0x02) == 0x02){ /*from hw nat offload*/
        if(restore_offload_info_hook)
            restore_offload_info_hook(skb, xpon_info, FOE_MAGIC_GPON);
            pTxMsg->raw.gem = xpon_info->stag;
            pTxMsg->raw.channel = xpon_info->channel & chnMask;
            pTxMsg->raw.nboq = xpon_info->channel & chnMask;
            skb->qosEnque &= ~(0x02);
        return 0;
    }
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
		if((xpon_sfu_multicast_protocol_hook) && (xpon_sfu_multicast_protocol_hook(skb)) && (1 == get_pon_link_type())
			&& (isVlanOperationInMulticastModule_hook) && (isVlanOperationInMulticastModule_hook(skb))) {
			is_up_igmp_pkt = 1;
		}

		if (is_up_igmp_pkt) {
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
	
		FH_VLAN_TX_TRANS(skb);
		
		if(ECNT_API_GPON_FLOW_UPSTREAM_ANI_HOOK(skb)){
			XPON_DROP_PRINT;
			return -1;
		}

        /*prepare meter and anct info */
        if(skb->pon_mark & QOS_TSE_MARK)
        {
            mtr_logic_idx = skb->pon_mark & QOS_TSID_MARK;
            PON_MSG(MSG_TRACE, "gemport ratelimit is eanble, mtr_logic_idx= %d\n",mtr_logic_idx) ; 
        }
        
        FE_API_GET_METER_IDX(skb, UP_STREAM, &tsid, mtr_logic_idx);
        xpon_info->tsid = tsid;
        pTxMsg->raw.mtr_g = tsid;
        /*prapare acont_g0 & acont_g1,and pass them to QDMA tx_msg,which used in QDMA_API_TRANSMIT_PACKETS*/
        FE_API_GET_ACNT1_IDX(skb, UP_STREAM, &acnt_g1);
        FE_API_GET_ACNT0_IDX(skb, UP_STREAM, &acnt_g0);
        pTxMsg->raw.acnt_g0 = acnt_g0;
        pTxMsg->raw.acnt_g1 = acnt_g1;
		FH_VLAN_TX_PROC(skb);

		/* Just for FPGA Verification. the default gem port msut use default interface or dropped. */
		gemIdx = (skb->gem_port) ? (gpWanPriv->gpon.gemIdToIndex[skb->gem_port] & GPON_GEM_IDX_MASK) : CONFIG_GPON_10G_MAX_GEMPORT ;
		if(gemIdx>=CONFIG_GPON_10G_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || 
											  (gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL || gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) ||
											  gpWanPriv->gpon.allocId[gpWanPriv->gpon.gemPort[gemIdx].info.channel] == GPON_10G_UNASSIGN_ALLOC_ID ||
											   gpWanPriv->gpon.gemPort[gemIdx].info.ani >= GPON_MAX_ANI_INTERFACE) {
			XPON_DROP_PRINT;
			return -1 ;
		}
		
		//Do GPON MAC filter
		skb->v_if = gpWanPriv->gpon.gemPort[gemIdx].info.ani ;
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER) && defined(TCSUPPORT_XPON_IGMP)
		if (is_up_igmp_pkt)
		{
			if ((xpon_up_igmp_ani_vlan_filter_hook) && (xpon_up_igmp_ani_vlan_filter_hook(skb) == -1)){
				XPON_DROP_PRINT;
				return -1;
			}
		}
#endif

#ifdef TCSUPPORT_PON_VLAN
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
		if (is_up_igmp_pkt == 0)
#endif
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

#if defined(TCSUPPORT_XPON_IGMP)
		if(xpon_sfu_multicast_protocol_hook && xpon_sfu_multicast_protocol_hook(skb)) {
			if(xpon_sfu_up_multicast_incoming_hook) {
				if(0 >= xpon_sfu_up_multicast_incoming_hook(skb, 1)) {
					XPON_DROP_PRINT;
					return -1;
				}
			}
		}
#endif

		pTxMsg->raw.gem = gpWanPriv->gpon.gemPort[gemIdx].info.portId ;
		pTxMsg->raw.channel = gpWanPriv->gpon.gemPort[gemIdx].info.channel ;
        pTxMsg->raw.nboq = gpWanPriv->gpon.gemPort[gemIdx].info.channel ;

#ifdef TCSUPPORT_RA_HWNAT
		if((ra_sw_nat_hook_magic) && (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE))) {
			pTxMsg->raw.fport = TXMSG_FPORT_PPE ;//DPORT_PPE
		}
		if(gpWanPriv->gpon.hgu_mode_txq ) {
			xpon_info->txq_is_valid = TRUE;
			xpon_info->txq = (skb->mark & QOS_FILTER_MARK) >> 4 ;
				
			PON_MSG(MSG_WARN, "pwan_net_start_xmit:---omci config txq xpon_info->txq = 0x%x\n",xpon_info->txq) ;
		}else{
			xpon_info->txq_is_valid = FALSE;
		}
		 
		if(ra_sw_nat_hook_tx != NULL)
		{
    	 	 xpon_info->magic = FOE_MAGIC_GPON;
			/* In EN7512/21, GPON's tcon is filled in PPE foe's channel field, GPON's gem port is filled in PPE foe's stag field. */
			/* EN7512 tx queue is configured by QDMA */
			xpon_info->stag = pTxMsg->raw.gem ;
			xpon_info->channel = pTxMsg->raw.channel & chnMask;
			xpon_info->nbq = xpon_info->channel;
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---gemid = 0x%x\n",xpon_info->stag) ; 
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->channel = 0x%x\n",xpon_info->channel) ;
		}        
#endif /* TCSUPPORT_RA_HWNAT */

	} 
	
	return 0 ;
}


int isBroadcastPkt(unchar *addr)
{
	if(addr == NULL)
		return -1;

	if((addr[0] == 0xff) &&  (addr[1] == 0xff) && (addr[2] == 0xff) && (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff))
		return 0;
	else
		return -1;
}


int isUnknownUnicastPkt(unchar *addr)
{
	unchar *myAddr = gpWanPriv->pPonNetDev[PWAN_IF_DATA]->dev_addr ;
	if(addr == NULL)
		return -1;

	if((addr[0] == myAddr[0]) &&  (addr[1] == myAddr[1]) && (addr[2] == myAddr[2]) && (addr[3] == myAddr[3]) && (addr[4] == myAddr[4]) && (addr[5] == myAddr[5]))
		return -1;
	else
		return 0;
}

int isMulticastPkt(unchar *addr)
{
	if(addr == NULL)
		return -1;

	if((addr[0] == 0x01) )
		return 0;
	else
		return -1;
}
void gwan_check_ds_omci_type(struct sk_buff * skb){

    pOmci_Header pOmciHeader = NULL;

    pOmciHeader = (pOmci_Header)skb->data;

    if(OMCI_BASIC_MSG_DEV_ID == pOmciHeader->devId || OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*Total omci*/
        gpWanPriv->gpon.rx_omci_cnt++;
    }

    if(OMCI_EXTENDED_MSG_DEV_ID == pOmciHeader->devId){    /*extended omci*/
        gpWanPriv->gpon.rx_omci_extend_cnt ++;
    }
}

int gwan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx = 0 ;
	uint gemPortId = 0 , gemIdx = 0 ;
	int i = 0, foo = 0 ;
	unsigned char * mac = NULL;
	unsigned short tpid = 0;
	unsigned char tagNum = 0;


	if(pRxMsg->raw.crcer) {
#ifdef XPON_MAC_CONFIG_DEBUG
		if(gpPonSysData->debugLevel & MSG_ERR) {
			PON_MSG(MSG_ERR, "Packet Ethernet CRC Error:%.8x, %.8x, %.8x, %.8x\n", pRxMsg->word[0], pRxMsg->word[1], pRxMsg->word[2], pRxMsg->word[3]) ;
			__dump_skb(skb, pktLens) ;
		}
#endif
	}
	
	if(pRxMsg->raw.oam) {
        if(pRxMsg->raw.no_mic) {
            if(XPON_SUCCESS !=gwan_check_ds_omci_mic(skb)){
                XPON_DROP_PRINT;
                return -1;
            }
        }
        
		netIdx = PWAN_IF_OMCI ;
		if(GPON_OMCC_ID != GPON_UNASSIGN_ONU_ID){
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK);
			gpWanPriv->gpon.gemPort[gemIdx].stats.rx_packets++ ;
			gpWanPriv->gpon.gemPort[gemIdx].stats.rx_bytes += pktLens ;
            gwan_check_ds_omci_type(skb);
		}else{
			XPON_DROP_PRINT;
			return -1;
		}	
		
#ifdef XPON_MAC_CONFIG_DEBUG
		if(gpPonSysData->debugLevel & MSG_OMCI) {  
			PON_MSG(MSG_OMCI, "RX PKT LEN: %d, Rx Msg: %.8x, %.8x", pktLens, pRxMsg->word[0], pRxMsg->word[1]) ;
			__dump_skb(skb, pktLens) ;
		}
#endif
	} else {
		netIdx = PWAN_IF_DATA ;
/* handle broadcast /unknown unicast /multicast pkt */
		mac = skb->data ;
		if (!isBroadcastPkt(mac))
			netIdx += BROADCAST_OFFSET;
		else if (!isMulticastPkt(mac))
			netIdx += MULTICAST_OFFSET;
		else if(!isUnknownUnicastPkt(mac))
			netIdx += UNKNOWN_UNICAST_OFFSET;			
		
		gemPortId = pRxMsg->raw.gem ;
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
		if(gemIdx>=CONFIG_GPON_10G_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || (
			(GPON_MULTICAST_CHANNEL != gpWanPriv->gpon.gemPort[gemIdx].info.channel) &&
			gpWanPriv->gpon.gemPort[gemIdx].info.ani>=GPON_MAX_ANI_INTERFACE )){
			 XPON_DROP_PRINT;
			 return -1;
		}
		gpWanPriv->gpon.gemPort[gemIdx].stats.rx_packets++ ;
		gpWanPriv->gpon.gemPort[gemIdx].stats.rx_bytes += pktLens ;
		if(GPON_MULTICAST_CHANNEL == gpWanPriv->gpon.gemPort[gemIdx].info.channel){
			skb->gem_type = GPON_MULTICAST_GEM ;
		}else{
			skb->gem_type = GPON_UNICAST_GEM ;			 
		}

		skb->gem_port = gemPortId ;
		skb->v_if = gpWanPriv->gpon.gemPort[gemIdx].info.ani ;
		skb->pon_mark |= DS_PKT_FORM_WAN;
		
/*excute ani port mac filter function here*/		
#ifdef TCSUPPORT_PON_MAC_FILTER
		skb->pon_mac_filter_flag |= PKT_SEND_TO_LAN;
		mac = skb->data + 12;
		 
		while((tpid =((*mac << 8) + *(mac + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
		{
			tagNum ++;
			mac += 4;
		}

		skb->pon_tag_num = tagNum;
		if(pon_check_mac_hook)
		{
			if(pon_check_mac_hook(skb) == -1)
				goto fail;
		}
		skb->pon_mac_filter_flag &= ~(PKT_SEND_TO_LAN) ;	
		skb->pon_tag_num = 0;
#endif
        
#ifdef TCSUPPORT_HWNAT_L2VID
		if(ra_sw_nat_hook_rx_set_l2lu)
			ra_sw_nat_hook_rx_set_l2lu(skb, 0, pRxMsg->raw.ppe);
#endif

		if(gpWanPriv->gpon.gemPort[gemIdx].info.rxLb) {
#ifdef CONFIG_USE_FOR_TEST
			if(gpWanPriv->devCfg.flags.isRandomLb) {
				foo = random32() % gpWanPriv->gpon.gemNumbers ;
				for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
					if(gpWanPriv->gpon.gemPort[i].info.valid) {
						if(!foo) {
							skb->gem_port = gpWanPriv->gpon.gemPort[i].info.portId ;
							skb->mark = (skb->mark & ~QOS_FILTER_MARK) | ((random32()&0x7)<<4) ;
							break ;
						}
						foo-- ;
					}
				}
				if(i == CONFIG_GPON_10G_MAX_GEMPORT) {
					skb->mark = (skb->mark & ~QOS_FILTER_MARK) | ((random32()&0x7)<<4) ;
					skb->gem_port = gemPortId ;
				}
				*pFlag = 1 ;
			} else 
#endif /* CONFIG_USE_FOR_TEST */
			{
				if(gpWanPriv->gpon.gemPort[gemIdx].info.channel<CONFIG_GPON_10G_MAX_TCONT) {
					skb->gem_port = gemPortId ;
					*pFlag = 1 ;
				} else {
#ifdef XPON_MAC_CONFIG_DEBUG  
					if(gpPonSysData->debugLevel & MSG_WARN) {
						PON_MSG(MSG_ERR, "Receive frame from multicast GEM port %d.\n", gemPortId) ;
						__dump_skb(skb, pktLens) ;
					}
#endif
				}
			}
		}

		if(ECNT_API_GPON_FLOW_DOWNSTREAM_ANI_HOOK(skb)){
    		XPON_DROP_PRINT;
    		return -1;
	    }
	}	
	
	return netIdx ;
	
fail:
#ifdef XPON_MAC_CONFIG_DEBUG
	if(gpPonSysData->debugLevel & MSG_WARN) {
		PON_MSG(MSG_WARN, "Receive frame from invalid GEM port %d.\n", gemPortId) ;
		__dump_skb(skb, pktLens) ;
	}
#endif
	return -1-netIdx ;
}
/*****************************************************************************
******************************************************************************/
void gwan_channel_init(void){
    
    feDevGdm2Cdm2Stop(XPON_RESET_HOLD_ON);
    feDevGdm2Cdm2Stop(XPON_RESET_RELEASE);
    /* set GPON packet length */
    FE_API_SET_PACKET_LENGTH(FE_GDM_SEL_GDMA2, GPON_PACKET_LEN_UPPER_LIMIT, GPON_PACKET_LEN_LOWER_LIMIT);
}
/*****************************************************************************
******************************************************************************/
GPON_TRAFFIC_STATUS_t gwan_get_traffic_status(void){

	int i = 0;
	GPON_TRAFFIC_STATUS_t ret = GPON_TRAFFIC_DOWN;
	if( (gpGponPriv->typeBOnGoing) || (GPON_10G_STATE_O6 == GPON_CURR_STATE) ){
		ret = GPON_TRAFFIC_UP;
	}else if((GPON_10G_STATE_O5 == GPON_CURR_STATE) ||
	( ng2_o4_to_09 == 1 && GPON_10G_STATE_O9 == GPON_CURR_STATE)){
		for(i = 0; i<CONFIG_GPON_10G_MAX_GEMPORT; i++){
			if( (gpWanPriv->gpon.gemPort[i].info.valid) && \
				 (gpWanPriv->gpon.gemPort[i].info.portId != GPON_OMCC_ID) ){
				 break;
			}
		}
		if(i < CONFIG_GPON_10G_MAX_GEMPORT ){
			ret = GPON_TRAFFIC_UP;
		}
	}else{
		ret = GPON_TRAFFIC_DOWN; 
	}
	return ret;
}

/*****************************************************************************
******************************************************************************/
int gwan_init(GWAN_Priv_T *pGWanPriv)
{
	int i = 0 ;
	XMCSIF_OnuType_t onu_type;
	
	for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++){
		pGWanPriv->allocId[i] = GPON_10G_UNASSIGN_ALLOC_ID;
	}

	for(i=0 ; i<GPON_10G_MAX_GEM_ID ; i++){
		pGWanPriv->gemIdToIndex[i] = GPON_GEM_IDX_MASK;
	}
	
	for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++){
		memset(&pGWanPriv->gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
		memset(&pGWanPriv->gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
	}
	pGWanPriv->gemNumbers = 0 ;	
	xmcs_get_onu_type(&onu_type);	
	if(XMCS_IF_ONU_TYPE_SFU == onu_type ){		
		pGWanPriv->hgu_mode_txq = 1;	
	}else{		
		pGWanPriv->hgu_mode_txq = 0;	
	}

	pGWanPriv->rx_omci_cnt = 0;
    pGWanPriv->rx_omci_extend_cnt = 0;
	GPON_CREATE_TIMER(&pGWanPriv->gemMibTimer,gwan_update_gem_mib_table,0);

	tasklet_init(&clear_channel_task, gwan_clear_channel_func,0x0);
	pwan_create_net_interface(PWAN_IF_OMCI) ;
    gwan_channel_init();
	return 0 ;	
}
/*****************************************************************************
******************************************************************************/
