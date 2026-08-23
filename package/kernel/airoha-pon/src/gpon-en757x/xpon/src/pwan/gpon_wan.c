#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "common/drv_global.h"
#include "pwan/gpon_wan.h"
#include "pwan/xpon_netif.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_recovery.h"
#include "common/xpondrv.h"

#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include <ecnt_hook/ecnt_hook_smux.h>
#include <ecnt_hook/ecnt_hook_net.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif


#ifdef TCSUPPORT_XPON_IGMP
extern int (*xpon_sfu_up_multicast_incoming_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_sfu_multicast_protocol_hook)(struct sk_buff *skb);
#endif

int g_service_change_flag = 0;


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

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int (*sw_isSWQosActive) (void);
extern int (*sw_isHwnatOffloadEnable) (void);
#endif

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int isNeedHWQoS;
#endif
extern int trtcmEnable;
extern int trtcmTsid;
#endif

extern int isHighestPriorityPacket(struct sk_buff *skb);
#ifdef TCSUPPORT_WAN_GPON
//ONU Type : 1=SFU 2=HGU
#define SFU 1
#define HGU 2


typedef struct Omci_Hander{
    u16  transId;
    u8   msgType;
    u8   devId;
    u32    MeId;
    /* use in extended omci*/
    u16  msgContLen;
}__attribute__((packed)) Omci_Header_T, *pOmci_Header;

#define OMCI_CRC_LEN                (4)
#define OMCI_BASIC_MSG_FIX_LEN      (48)
#define OMCI_BASIC_MSG_DEV_ID       (0x0a)
#define OMCI_EXTENDED_MSG_DEV_ID    (0x0b)

static int remove_omci_crc_if_exist(struct sk_buff * skb)
{
    pOmci_Header pOmciHeader = NULL;
    u16 omci_msg_len = 0;
    u16 extra_len = 0;
    
    pOmciHeader = skb->data;

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

/*****************************************************************************
******************************************************************************/
int gwan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, int txq, struct port_info *xpon_info)
{
	int onu_type;
	uint gemIdx ;
	int isDefineTcsupportQoS;
#if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
	int is_up_igmp_pkt = 0;
#endif
	uint chnMask, qMask;

	if (isEN751221 || isEN751627) {
		/* PPE foe's channel field has 8 bits for EN751221 and 4 bits for MT751020  */
		chnMask = 0xff;
		qMask = 0x7;
	} else {
		/* PPE foe's queue field has 3 bits for EN751221 and 4 bits for MT751020 */
		chnMask = 0xf;
		qMask = 0xf;
	}

#ifdef TCSUPPORT_QOS
	isDefineTcsupportQoS = XPON_ENABLE;
#else
	isDefineTcsupportQoS = XPON_DISABLE;
#endif

	pTxMsg->raw.fport = TXMSG_FPORT_GMAC ;
	if(netIdx == PWAN_IF_OMCI) {
		pTxMsg->raw.oam = 1 ;

        if(XPON_SUCCESS != remove_omci_crc_if_exist(skb)){
            XPON_DROP_PRINT;
            return -1;
        }
        
#ifndef TCSUPPORT_CPU_EN7521
		/* EN7512 tx queue is configured by QDMA */
		pTxMsg->raw.queue = 7 ;
#endif
		pTxMsg->raw.channel = 0 ;
		pTxMsg->raw.gem = GPON_OMCC_ID ;
#ifdef XPON_MAC_CONFIG_DEBUG
		if(gpPonSysData->debugLevel & MSG_OMCI) {
			PON_MSG(MSG_OMCI, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
			__dump_skb(skb, skb->len) ;
		}
#endif
	} else {
		pTxMsg->raw.oam = 0 ;
/*Warning: operations on skb must behind ct hwnat offload*/
/*Warning: operations on skb must behind ct hwnat offload*/

    #if defined(TCSUPPORT_PON_VLAN) && defined(TCSUPPORT_PON_VLAN_FILTER)&& defined(TCSUPPORT_XPON_IGMP)
        if((xpon_sfu_multicast_protocol_hook) && (xpon_sfu_multicast_protocol_hook(skb)) && (1 == get_pon_link_type()) //1:SFU
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
	

#ifdef TCSUPPORT_GPON_MAPPING
		if(gpon_mapping_hook && (-1 == gpon_mapping_hook(skb))){
            XPON_DROP_PRINT;
			return -1;
		}
#endif

		

		/* Just for FPGA Verification. the default gem port msut use default interface or dropped. */
		gemIdx = (skb->gem_port) ? (gpWanPriv->gpon.gemIdToIndex[skb->gem_port] & GPON_GEM_IDX_MASK) : CONFIG_GPON_MAX_GEMPORT ;
		if(gemIdx>=CONFIG_GPON_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || 
											  (gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL || gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) ||
											   gpWanPriv->gpon.gemPort[gemIdx].info.ani >= GPON_MAX_ANI_INTERFACE||
											   GPON_UNASSIGN_ALLOC_ID == gpWanPriv->gpon.allocId[gpWanPriv->gpon.gemPort[gemIdx].info.channel]) {
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
        
		//if(GPON_MAX_FILTER_PSEUDO(skb) != 0) {
		//	return -1 ;
		//}

#ifndef TCSUPPORT_CPU_EN7521
		/* EN7512 tx queue is configured by QDMA */
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		xmcs_get_onu_type(&onu_type);
		if(isDefineTcsupportQoS == XPON_DISABLE ||
		   (isDefineTcsupportQoS == XPON_ENABLE && SFU == onu_type)) {
			pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
		} else {
			pTxMsg->raw.queue = txq ;
		}
#else
		pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
#endif

		pTxMsg->raw.tse = (skb->pon_mark&QOS_TSE_MARK) ? 1 : 0 ;
		pTxMsg->raw.tsid = (pTxMsg->raw.tse) ? (skb->pon_mark&QOS_TSID_MARK) : 0 ;	

		/* Set pppoe packet to highest queue, queue 6 */
		if(isHighestPriorityPacket(skb)) {
			pTxMsg->raw.queue = queue6;
		}
#endif /* TCSUPPORT_CPU_EN7521 */
		pTxMsg->raw.gem = gpWanPriv->gpon.gemPort[gemIdx].info.portId ;
		pTxMsg->raw.channel = gpWanPriv->gpon.gemPort[gemIdx].info.channel ;


#ifdef TCSUPPORT_RA_HWNAT
		if((ra_sw_nat_hook_magic) && (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE))) {
			pTxMsg->raw.fport = TXMSG_FPORT_PPE ;//DPORT_PPE
		} 

		if((ra_sw_nat_hook_tx != NULL)
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)			
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
			&& (isNeedHWQoS == 1 )
#endif
#endif
		) {
    	 	 xpon_info->magic = FOE_MAGIC_GPON;
#ifdef TCSUPPORT_CPU_EN7521
			/* In EN7512/21, GPON's tcon is filled in PPE foe's channel field, GPON's gem port is filled in PPE foe's stag field. */
			/* EN7512 tx queue is configured by QDMA */
			xpon_info->stag = pTxMsg->raw.gem ;
			xpon_info->channel = pTxMsg->raw.channel & chnMask;
            
            xmcs_get_onu_type(&onu_type);
            if(isDefineTcsupportQoS == XPON_DISABLE ||
                (isDefineTcsupportQoS == XPON_ENABLE && SFU == onu_type)) {
                xpon_info->txq_is_valid = TRUE;
                xpon_info->txq = ((skb->mark & QOS_FILTER_MARK) >> 4) & qMask ;
            }
            else
            {
                xpon_info->txq_is_valid = FALSE;
            }

			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---gemid = 0x%x\n",xpon_info->stag) ; 
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->channel = 0x%x\n",xpon_info->channel) ;
#else /* TCSUPPORT_CPU_EN7521 */
			/* In MT7510/20, GPON's tcon/gem port are all filled in PPE foe's stag field. */
			xpon_info->txq = pTxMsg->raw.queue & qMask;
			xpon_info->stag = (pTxMsg->raw.channel & chnMask) | (pTxMsg->raw.gem << 4);
			
		/* In MT7510/20, bit5~1 and bit0 of PPE foe's tsid field stand for tsid and tse respectively */
			xpon_info->tsid = ((pTxMsg->raw.tsid&0x1f)<<1) |  (pTxMsg->raw.tse&0x1);	
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)						
		//for TRTCM test --Trey
			if (trtcmEnable) {
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
				if(xpon_info->txq == 0)	{			
					xpon_info->tsid = 0;
			}else{
					xpon_info->tsid = (((CAR_QUEUE_NUM - xpon_info->txq +1)&0x1f)<<1) | 0x1;
			}
#else		
				xpon_info->tsid = ((trtcmTsid&0x1f)<<1) | 0x1;
#endif
		}	
#endif

			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---gemid = 0x%x\n",xpon_info->stag>>4) ; 
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->channel = 0x%x\n",xpon_info->stag&0xf) ;
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->txq = 0x%x\n",xpon_info->txq) ;
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->tse = 0x%x\n",xpon_info->tsid&0x1) ;
			PON_MSG(MSG_TRACE, "pwan_net_start_xmit:---xpon_info->tsid = 0x%x\n",(xpon_info->tsid>>1)&0x1f) ;

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
		if((sw_isSWQosActive && !sw_isSWQosActive()) ||
		   (sw_isSWQosActive && sw_isSWQosActive() && sw_isHwnatOffloadEnable && sw_isHwnatOffloadEnable()))
#endif
		{
			if(ra_sw_nat_hook_tx(skb, xpon_info, xpon_info->magic) == 0) {
                XPON_DROP_PRINT;
				return 1 ; //don't printf err msg when drop keepalive packet.
			}
		}
#endif /* TCSUPPORT_CPU_EN7521 */
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

void gwan_omci_check_start(struct sk_buff *skb)
{
	unchar *dp = ((unchar*)skb->data) + 2;
	static int cnt = 0;

	if (gpWanPriv->devCfg.flags.isTxDropOmcc && (GPON_CURR_STATE == GPON_STATE_O5)){
        /* if receive ONU-Data ME or 'set', 'create' msg  */
		if (/*(((*dp)&0xF)==0x9) && */(((*(dp+1))==0x0a) &&((*(dp+2))==0x00) &&((*(dp+3))==0x02)) 
			|| (((*dp)&0xF)==0x8)|| (((*dp)&0xF)==0x4)){
			if ((cnt++ % 20) == 0)
				PON_MSG(MSG_DBG, "%u Reset Drop Timer to %us\n",cnt, gpWanPriv->txDropTimer.data/1000);

			GPON_START_TIMER(gpWanPriv->txDropTimer);
		}
	}
}


/*****************************************************************************
******************************************************************************/
int gwan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx ;
	uint gemPortId, gemIdx ;
	int i, foo ;
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
		netIdx = PWAN_IF_OMCI ;
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK);
		gpWanPriv->gpon.gemPort[gemIdx].stats.rx_packets++ ;
		gpWanPriv->gpon.gemPort[gemIdx].stats.rx_bytes += pktLens ;
		
		gwan_omci_check_start(skb);
		
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
		 if(gemIdx>=CONFIG_GPON_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || (
#ifdef TCSUPPORT_CDS
            (GPON_MULTICAST_CHANNEL != gpWanPriv->gpon.gemPort[gemIdx].info.channel) &&
#endif
             gpWanPriv->gpon.gemPort[gemIdx].info.ani>=GPON_MAX_ANI_INTERFACE )){
			if (gpGponPriv->gponCfg.flags.sniffer) {
				struct sk_buff *skb2 = NULL;
				int len = 6;
				const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
				int isOmciPkt = (pktLens == 44) ? XPON_ENABLE : XPON_DISABLE ;

				PON_MSG(MSG_WARN, "Packet Sniffer Gemport Id: %x\n", gemPortId) ;
				
				if(isOmciPkt)
					len += 14;

				/* process the packet data to high layer */		
				skb_put(skb, pktLens) ; 
				
				/* Check the skb headroom is enough or not.*/
				if(skb_headroom(skb) < len)
					skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
				else
					skb2 = skb_copy(skb, GFP_ATOMIC);

				if(skb2 == NULL){
					PON_MSG(MSG_ERR, "packet sniffer for skb alloc fail\n") ;
				} else {		
					if (isOmciPkt)
					{
						skb_push(skb2, 14);
						memcpy(skb2->data, omciHdr, 14);
					}
				
					skb2->mark |= SKBUF_COPYTOLAN;
					macSend(WAN2LAN_CH_ID,skb2); /* tc3262_mac_tx */
				}	

				/* restore the skb */
				skb_trim(skb, skb->len -pktLens);
			}
			goto fail ;
		}
		
		/* do PCP encode/decode check, if PCP self test enabled */
		if(gpGponPriv->gponCfg.dvtPcpCheck) {
			gpon_dvt_pcp_check(gpGponPriv->gponCfg.dvtPcpCheck, skb) ;
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
		
#ifdef TCSUPPORT_HWNAT_L2VID
		if(ra_sw_nat_hook_rx_set_l2lu)
			ra_sw_nat_hook_rx_set_l2lu(skb);
#endif
		
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

		if(gpWanPriv->gpon.gemPort[gemIdx].info.rxLb) {
#ifdef CONFIG_USE_FOR_TEST
			if(gpWanPriv->devCfg.flags.isRandomLb) {
				foo = random32() % gpWanPriv->gpon.gemNumbers ;
				for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
					if(gpWanPriv->gpon.gemPort[i].info.valid) {
						if(!foo) {
							skb->gem_port = gpWanPriv->gpon.gemPort[i].info.portId ;
							skb->mark = (skb->mark & ~QOS_FILTER_MARK) | ((random32()&0x7)<<4) ;
							break ;
						}
						foo-- ;
					}
				}
				if(i == CONFIG_GPON_MAX_GEMPORT) {
					skb->mark = (skb->mark & ~QOS_FILTER_MARK) | ((random32()&0x7)<<4) ;
					skb->gem_port = gemPortId ;
				}
				*pFlag = 1 ;
			} else 
#endif /* CONFIG_USE_FOR_TEST */
			{
				if(gpWanPriv->gpon.gemPort[gemIdx].info.channel>=0 && gpWanPriv->gpon.gemPort[gemIdx].info.channel<CONFIG_GPON_MAX_TCONT) {
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
	}

	#ifdef TCSUPPORT_XPON_IGMP_CHT
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
void gwan_update_gem_mib_table(unsigned long arg)
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
		if(gemIdx<CONFIG_GPON_MAX_GEMPORT && gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
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
	uint regVal = 0;
		
	retValue = gponDevEnableTCont(allocId) ;
	if(retValue>=0 || retValue<CONFIG_GPON_MAX_TCONT) {
		channel = retValue ;

		gpWanPriv->gpon.allocId[channel] = allocId ;

		#ifdef TCSUPPORT_CPU_EN7521
		/* Enable GDM2 channel Tx */
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, channel, FE_ENABLE);
		/* Enable CDM2 Rx DMA */
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, channel, FE_ENABLE);
		#else
		/* Enable LMGR channel Tx */
		regVal = IO_GREG(0xBFB51844);
		regVal = regVal & (~(1<<channel));
		IO_SREG(0xBFB51844, regVal);

		/* Enable GDM2 channel Tx */
		regVal = IO_GREG(0xBFB5152C);
		regVal = (regVal | (1<<channel));
		IO_SREG(0xBFB5152C, regVal);

		/* MT7520 has no CDM2 rx DMA register */
		#endif

		if(GPON_ONU_ID != allocId)
		{
			gpon_recover_backup_allocId(allocId);
		}
		
		return 0 ;
	} 
	
	return retValue ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_tcont(ushort allocId)
{
	int i ;
	uint regVal = 0;
	unchar change_service_flag = false;

	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		if(gpWanPriv->gpon.allocId[i] == allocId) {
		
            if((2 == gpWanPriv->activeChannelNum) && (allocId != gpGponPriv->gponCfg.onu_id))
            {
                xpon_set_qdma_qos(QDMA_ENABLE);
                change_service_flag = true;
            }
            
			gpWanPriv->gpon.allocId[i] = GPON_UNASSIGN_ALLOC_ID ;

			#ifdef TCSUPPORT_CPU_EN7521
			/* Disable Channel for GDM2 Tx */
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
			/* Disable CDM2 Rx DMA */
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
			#else
			/* Disable LMGR channel Tx */
			regVal = IO_GREG(0xBFB51844);
			regVal |= (1<<i);
			IO_SREG(0xBFB51844, regVal);
			
			/* Disable GDM2 channel Tx */
			regVal = IO_GREG(0xBFB5152C);
			regVal &= ~(1<<i);
			IO_SREG(0xBFB5152C, regVal);

			/* MT7520 has no CDM2 rx DMA register */
			#endif

			gponDevDisableTCont(allocId);
#if !defined(TCSUPPORT_XPON_HAL_API) && !defined(TCSUPPORT_XPON_HAL_API_EXT) 
			if(change_service_flag)
#endif
			{
			    g_service_change_flag = 1;
			    change_service_flag   = false;
			}

			if(allocId != GPON_ONU_ID){
				gpon_recover_delete_allocId(allocId);
			}
			return 0 ;
		}
	}
	
	return -ENOENT ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_all_tcont(void)
{
	int i ;

	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		gpWanPriv->gpon.allocId[i] = GPON_UNASSIGN_ALLOC_ID ;
	}

	return 0 ;
}

int gwanCheckAllocIdExist(ushort allocId)
{
	int i = 0;

	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) 
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
int gwan_create_new_gemport(ushort gemPortId, unchar channel, unchar encryption)
{
	int i = 0;
	ushort encry_info;

	if(gemPortId>=GPON_MAX_GEM_ID || (channel>=CONFIG_GPON_MAX_TCONT && channel!=GPON_MULTICAST_CHANNEL && channel!=GPON_UNKNOWN_CHANNEL)) {
		return -EINVAL ;
	}

	if(gpWanPriv->gpon.gemNumbers >= CONFIG_GPON_MAX_GEMPORT) {
		return -ENOSPC ;
	}

	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) 
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
		gpWanPriv->gpon.gemNumbers++ ;

		if(gemPortId != GPON_OMCC_ID)
		{
			ushort allocid = GPON_UNASSIGN_ALLOC_ID;
			
			if(GPON_MULTICAST_CHANNEL !=  channel)
			{
				allocid = gpWanPriv->gpon.allocId[channel];
			}
			
			struct Gem_Recovery_Info bakGemPort = {0};
			
			bakGemPort.allocId = allocid;
			bakGemPort.gemPortId = gemPortId;
			bakGemPort.channel = channel;
			bakGemPort.encryption = encryption;
			gpon_recover_backup_gemport(&bakGemPort);
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
	if(gemIdx>=CONFIG_GPON_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid || gpWanPriv->gpon.gemPort[gemIdx].info.portId!=gemPortId) {
		return -ENOENT ;
	}

	tmp = find_backup_gemport(gemPortId);

	switch(cfgType) {
		case ENUM_CFG_NETIDX:
			gpWanPriv->gpon.gemPort[gemIdx].info.ani = (value>=GPON_MAX_ANI_INTERFACE) ? GPON_MAX_ANI_INTERFACE : value ;
			break ;
			
		case ENUM_CFG_CHANNEL:
			if(value>=CONFIG_GPON_MAX_TCONT && value!=GPON_MULTICAST_CHANNEL) {
				return -EINVAL ;
			}			
			gpWanPriv->gpon.gemPort[gemIdx].info.channel = value ;

			if(NULL != tmp)
			{
				if(GPON_MULTICAST_CHANNEL == value){
					tmp->allocId = GPON_UNASSIGN_ALLOC_ID;
				}else{
					tmp->allocId = gpWanPriv->gpon.allocId[value];
				}
				tmp->channel = value;
			
			}
			else if(gponRecovery.dbgPrint)
			{
			    printk("%s %d line change channel err\n", __FUNCTION__,__LINE__);
			}
			
			break ;
			
		case ENUM_CFG_ENCRYPTION:
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_MULTICAST_CHANNEL || gpWanPriv->gpon.gemPort[gemIdx].info.channel==GPON_UNKNOWN_CHANNEL) {
				return -EINVAL ;
			}			
			gpWanPriv->gpon.gemPort[gemIdx].info.rxEncrypt = (value)?1:0 ;
			gponDevSetGemInfo(gemPortId, XPON_ENABLE, (value)?XPON_ENABLE:XPON_DISABLE) ;
			
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
	if(gemIdx>=CONFIG_GPON_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
		return -ENOENT ;
	}

	memset(&gpWanPriv->gpon.gemPort[gemIdx].info, 0, sizeof(GWAN_GemInfo_T)) ;
	memset(&gpWanPriv->gpon.gemPort[gemIdx].stats, 0, sizeof(struct net_device_stats)) ;			
	gpWanPriv->gpon.gemIdToIndex[gemPortId] |= GPON_GEM_IDX_MASK ;
	
	gpWanPriv->gpon.gemNumbers-- ;
	gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE) ;
	mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ; /* 2000 ms */

	if(GPON_OMCC_ID != gemPortId){
		gpon_recover_delete_gemport(gemPortId);
	}
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport_for_disable(void)
{
	ushort gemPortId ;
	int i ;
	
	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
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
	ushort gemPortId ;
	int i ;
	
	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
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
	return ((gemIdx<CONFIG_GPON_MAX_GEMPORT) ? gpWanPriv->gpon.gemPort[gemIdx].info.valid : -1) ;
}

/*****************************************************************************
******************************************************************************/
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

int service_change_proc_init(void)
{
    g_service_change_flag = 0;

    //with traffic, change service, reset
    struct proc_dir_entry *service_change_proc = NULL;
    service_change_proc = create_proc_entry("tc3162/service_change", 0, NULL);
    if(NULL == service_change_proc)
    {
        printk("creat service change proc faile.\n");
        return -1;
    }
    
    service_change_proc->read_proc  = service_change_read_proc;
    service_change_proc->write_proc = service_change_write_proc;
    
    return 0;
    
}

GPON_TRAFFIC_STATUS_t gwan_get_traffic_status()
{
	if( (gpGponPriv->typeBOnGoing) || \
	    (GPON_STATE_O6 == GPON_CURR_STATE) ){
		return GPON_TRAFFIC_UP;
	}

	if( MONITOR_DONE != gpPonSysData->Omci_Oam_Monitor.run_state || \
		GPON_CURR_STATE != GPON_STATE_O5){
        return GPON_TRAFFIC_DOWN;
    }

    return GPON_TRAFFIC_UP;
}

/*****************************************************************************
******************************************************************************/
int gwan_init(GWAN_Priv_T *pGWanPriv)
{
	int i ;
	
	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		pGWanPriv->allocId[i] = GPON_UNASSIGN_ALLOC_ID;
	}

	for(i=0 ; i<GPON_MAX_GEM_ID ; i++) {
		pGWanPriv->gemIdToIndex[i] = GPON_GEM_IDX_MASK;
	}
	
	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
		memset(&pGWanPriv->gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
		memset(&pGWanPriv->gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
	}
	pGWanPriv->gemNumbers = 0 ;

	init_timer(&pGWanPriv->gemMibTimer) ;
	pGWanPriv->gemMibTimer.function = gwan_update_gem_mib_table ;
    service_change_proc_init();
	pwan_create_net_interface(PWAN_IF_OMCI) ;

#ifdef TCSUPPORT_CPU_EN7521
#ifdef TCSUPPORT_WAN_GPON
	/* disable all tx channel */
	for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++) {
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

	for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++) {
		if(-1 == QDMA_API_SET_CNTR_CHANNEL(ECNT_QDMA_WAN, i)) {
			printk("gwan_init QDMA_API_SET_CNTR_CHANNEL %d failed\n",i);
		}
	}
	
#endif
	return 0 ;	
}

#endif /* TCSUPPORT_WAN_GPON */
