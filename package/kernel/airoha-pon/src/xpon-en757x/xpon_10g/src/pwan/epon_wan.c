#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "common/xpon_global.h"
#include "pwan/epon_wan.h"
#include "pwan/xpon_netif.h"
#include "common/xpon_api.h"
#include <ecnt_hook/ecnt_hook_fe.h>

#include "ecnt_hook/ecnt_hook_vlan.h"

extern unsigned short int gMpcpSynctime;

#if defined(TCSUPPORT_ACCOUNT_METER_V2)	
extern int (*fe_resource_mark_llid_hook)( struct sk_buff *skb, u8 llid, u8 dir);
#endif

#ifdef TCSUPPORT_EPON_MAPPING
extern int (*epon_mapping_hook)(struct sk_buff *skb);
#endif



#ifdef TCSUPPORT_XPON_IGMP
extern int (*xpon_sfu_up_multicast_vlan_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_sfu_multicast_protocol_hook)(struct sk_buff *skb);
extern int (*xpon_sfu_up_multicast_incoming_hook)(struct sk_buff *skb, int clone);
#endif

extern int isHighestPriorityPacket(struct sk_buff *skb);

#ifdef TCSUPPORT_WAN_EPON
/*****************************************************************************
******************************************************************************/
int ewan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, int txq, struct port_info *xpon_info)
{
	uint idx ;
	ushort *p ;

	uint chnMask, qMask;
    unsigned char acnt_g0 = 0x1f, acnt_g1 = 0x1f;
    unsigned char tsid = 0x7f;

	if (isEN751221) {
		/* PPE foe's channel field has 8 bits for EN751221 and 4 bits for MT751020  */
		chnMask = 0xff;
		qMask = 0x7;
	} else {
		/* PPE foe's queue field has 3 bits for EN751221 and 4 bits for MT751020 */
		chnMask = 0xf;
		qMask = 0xf;
	}

	pTxMsg->raw.fport = TXMSG_FPORT_GMAC ;
	if(netIdx == PWAN_IF_OAM) {
		p = (ushort *)skb->data ;
		pTxMsg->raw.channel = htons(p[0]) ;
		pTxMsg->raw.nboq = htons(p[0]);
		pTxMsg->raw.ndp = 1;
		pTxMsg->raw.oam = 1 ;
		/* EN7512 tx queue is configured by QDMA */
		pTxMsg->raw.queue = 7 ;
        pTxMsg->raw.mtr_g = tsid;
		pTxMsg->raw.acnt_g0 = acnt_g0;
		pTxMsg->raw.acnt_g1 = acnt_g1;
		skb_pull(skb, 2) ;
		PON_MSG(MSG_ERR,"epon oam tx channel: %d\n",pTxMsg->raw.channel);

       // gpEponPriv->oamChannel = pTxMsg->raw.channel;
        //bitmap_format_convert();
    }else if(netIdx == PWAN_IF_EAPOL){
        pTxMsg->raw.channel = 0;
        pTxMsg->raw.nboq = 0;
        pTxMsg->raw.ndp = 1;
        pTxMsg->raw.oam = 1;
        pTxMsg->raw.queue = 7;
        pTxMsg->raw.mtr_g = tsid;
        pTxMsg->raw.acnt_g0 = acnt_g0;
        pTxMsg->raw.acnt_g1 = acnt_g1;
    }else{		
/*Warning: operations on skb must behind ct hwnat offload*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_HWNAT_OFFLOAD)
		if((skb->qosEnque & 0x02) == 0x02){ /*from hw nat offload*/
			pTxMsg->raw.oam = 0;
			if(restore_offload_info_hook)
				restore_offload_info_hook(skb, xpon_info, FOE_MAGIC_EPON);

			if (isEN751221) {
				pTxMsg->raw.channel = xpon_info->channel & chnMask;
				pTxMsg->raw.nboq = xpon_info->channel & chnMask;
			}
			else {
				pTxMsg->raw.channel = xpon_info->stag & chnMask;
				pTxMsg->raw.nboq = xpon_info->stag & chnMask;
				/* EN7512 tx queue is configured by QDMA */
				pTxMsg->raw.queue = xpon_info->txq & qMask;
			}
			skb->qosEnque &= ~(0x02);
			return 0;
		}
#endif/*TCSUPPORT_COMPILE*/

		FH_VLAN_TX_TRANS(skb);
		FH_VLAN_TX_PROC(skb);

/*Warning: operations on skb must behind ct hwnat offload*/	
#ifdef TCSUPPORT_EPON_MAPPING
		if (  epon_mapping_hook){
			epon_mapping_hook(skb);
		}
#endif
		idx = skb->v_if ;
		if(idx >= EPON_LLID_MAX_NUM) {
			return -1 ;
		}else if(!gpWanPriv->epon.llid[idx].info.valid) {
			PON_MSG(MSG_ERR, "Transmit frame to invalid LLID %d.\n", gpWanPriv->epon.llid[idx].info.llid) ;
			//__dump_skb(skb, skb->len) ;
			return -1 ;
		}
        
 #ifdef TCSUPPORT_XPON_IGMP
       /*epon sfu multicast vlan operation and fwdtbl learn*/
        if(xpon_sfu_multicast_protocol_hook && xpon_sfu_multicast_protocol_hook(skb))
        {
            if(xpon_sfu_up_multicast_vlan_hook)
            {
                if(0 >= xpon_sfu_up_multicast_vlan_hook(skb, 0))
                {
                    return -1;
                }
            }
            
            if(xpon_sfu_up_multicast_incoming_hook) 
            {
                if(0 >= xpon_sfu_up_multicast_incoming_hook(skb, 1))
                {
                    return -1;
                }
            }
            
        }
#endif
        /********************************************/

		if(gpWanPriv->epon.llid[idx].info.txDrop) {
			gpWanPriv->epon.llid[idx].stats.tx_dropped++ ;
			return -1 ;
		} 

		pTxMsg->raw.oam = 0 ;
		pTxMsg->raw.channel = skb->v_if ;
		pTxMsg->raw.nboq = skb->v_if ;
		
        FE_API_GET_METER_IDX(skb, UP_STREAM, &tsid, 0);

        xpon_info->tsid = tsid;
        pTxMsg->raw.mtr_g = tsid;
        /*prapare acont_g0 & acont_g1,and pass them to QDMA tx_msg,which used in QDMA_API_TRANSMIT_PACKETS*/
        FE_API_GET_ACNT1_IDX(skb, UP_STREAM, &acnt_g1);
        FE_API_GET_ACNT0_IDX(skb, UP_STREAM, &acnt_g0);
        pTxMsg->raw.acnt_g0 = acnt_g0;
        pTxMsg->raw.acnt_g1 = acnt_g1;

#ifdef TCSUPPORT_RA_HWNAT
		if((ra_sw_nat_hook_magic) && (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE))) {
			pTxMsg->raw.fport = TXMSG_FPORT_PPE ;//DPORT_PPE
		} 
{

		xpon_info->magic = FOE_MAGIC_EPON;
    	
		/* In EN7512/21, EPON's llid is filled in PPE foe's channel field. */
		xpon_info->channel = pTxMsg->raw.channel & chnMask;
		xpon_info->nbq = xpon_info->channel ;
		PON_MSG(MSG_TRACE, "ewan_hwnat_hook_tx:---llid = 0x%x\n",xpon_info->channel);
		}
#endif
	} 
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int ewan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx; 
	ushort idx ;
	ushort *p ;
	ushort llid_value=0 ;
	
	idx = pRxMsg->raw.channel ;
	
#if defined(TCSUPPORT_ACCOUNT_METER_V2)		
	if(fe_resource_mark_llid_hook){
		fe_resource_mark_llid_hook(skb, pRxMsg->raw.channel, 0); //downstream 
		PON_MSG(MSG_ERR, "%s %d downstream :: llid=%d\n",__FUNCTION__,__LINE__,pRxMsg->raw.channel) ;
	}
#endif
	
	if(pRxMsg->raw.oam) {
       if(is_8021x_packet(skb->data)){
            netIdx = PWAN_IF_EAPOL;
            //printk("[%s %d]Receive EAPOL packet.\n", __FUNCTION__,__LINE__);
        }else{
			skb_push(skb, 2) ;
			p = (ushort *)skb->data ;
			p[0] = htons(idx) ;
			netIdx = PWAN_IF_OAM ;
        	PON_MSG(MSG_ERR, "%s %d Receive Oam packet.epon rx channel: %d, llid val=%#x\n",__FUNCTION__,__LINE__,idx, pRxMsg->raw.gem) ;
        }		
		llid_value = pRxMsg->raw.gem;
		if(pRxMsg->raw.no_mic ==1){	
			PON_MSG(MSG_CONTENT, "llid_value:%d llid:%d nomic:%d\n",llid_value,gpWanPriv->epon.llid[idx].info.llid,pRxMsg->raw.no_mic); 
			if(gpPonSysData->debugLevel & MSG_CONTENT) {
				__dump_skb(skb, pktLens) ;
			}
		}

	} else {
		//printk("=================================================== there shouldn't receive data packet!!\n");
		//__dump_skb(skb, pktLens) ;
	PON_MSG(MSG_ERR, "%s %d receive data packet.\n",__FUNCTION__,__LINE__) ;
		if(gpWanPriv->epon.llid[idx].info.valid) {
			skb->v_if = pRxMsg->raw.channel ;
			
			if(gpWanPriv->epon.llid[idx].info.rxDrop) {
				gpWanPriv->epon.llid[idx].stats.rx_dropped++ ;
				return -1 ;
			} else {
				if(gpWanPriv->epon.llid[idx].info.rxLb) {
					skb->mark = (skb->mark & ~QOS_FILTER_MARK) ;
					*pFlag = 1 ;
				}
				gpWanPriv->epon.llid[idx].stats.rx_packets++ ;
				gpWanPriv->epon.llid[idx].stats.rx_bytes += pktLens ;
			} 
		} else {
			PON_MSG(MSG_ERR, "Receive frame from invalid LLID %d.\n", gpWanPriv->epon.llid[idx].info.llid) ;
			//__dump_skb(skb, pktLens) ;	
			PON_MSG(MSG_ERR, "%s %d OAM packet netIdx return -1 \n",__FUNCTION__,__LINE__) ;
			return -1 ;
		}

		netIdx = PWAN_IF_DATA ;
	}
	PON_MSG(MSG_ERR, "%s %d OAM packet netIdx %x.\n",__FUNCTION__,__LINE__,netIdx) ;	
	return netIdx ;
}

/*****************************************************************************
******************************************************************************/
int ewan_init(EWAN_Priv_T *pEWanPriv)
{
	int i ;
    
	for(i=0 ; i<EPON_LLID_MAX_NUM ; i++) {
		memset(&pEWanPriv->llid[i].info, 0, sizeof(EWAN_LlidInfo_T)) ;
		memset(&pEWanPriv->llid[i].stats, 0, sizeof(struct net_device_stats)) ;
		pEWanPriv->llid[i].info.rxDrop = 1 ;
		pEWanPriv->llid[i].info.txDrop = 1 ;
		
	}

	pwan_create_net_interface(PWAN_IF_OAM) ;
	pwan_create_net_interface(PWAN_IF_EAPOL);

	return 0 ;
}


#endif /* TCSUPPORT_WAN_EPON */
