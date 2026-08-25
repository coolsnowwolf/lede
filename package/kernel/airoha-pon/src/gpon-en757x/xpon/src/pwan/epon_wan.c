#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "common/drv_global.h"
#include "pwan/epon_wan.h"
#include "pwan/xpon_netif.h"

#ifdef TCSUPPORT_EPON_DUMMY
extern __u8 gDummyEnable;
#endif

extern unsigned short int gMpcpSynctime;

#ifdef TCSUPPORT_EPON_MAPPING
extern int (*epon_mapping_hook)(struct sk_buff *skb);
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
extern u8 isSfu;
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
		pTxMsg->raw.channel = p[0] ;
		pTxMsg->raw.oam = 1 ;
#ifndef TCSUPPORT_CPU_EN7521
		/* EN7512 tx queue is configured by QDMA */
		pTxMsg->raw.queue = 7 ;
#endif
		skb_pull(skb, 2) ;

        gpEponPriv->oamChannel = pTxMsg->raw.channel;

        if(gpEponPriv->gBitmapSet){
            bitmap_format_convert();
        }
	} else {		
/*Warning: operations on skb must behind ct hwnat offload*/


/*Warning: operations on skb must behind ct hwnat offload*/	
#ifdef TCSUPPORT_EPON_MAPPING
		if (  epon_mapping_hook){
			epon_mapping_hook(skb);
		}
#endif
		idx = skb->v_if ;
		if(idx>=CONFIG_EPON_MAX_LLID || !gpWanPriv->epon.llid[idx].info.valid) {
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
#ifndef TCSUPPORT_CPU_EN7521
/* EN7512 tx queue is configured by QDMA */
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#ifdef TCSUPPORT_QOS
		if(isSfu){
#ifdef TCSUPPORT_EPON_DUMMY
			if(gDummyEnable){
				if(isDummyPkt(skb)){
					pTxMsg->raw.queue = 0 ;
				}else{
					if(((skb->mark & QOS_FILTER_MARK) >> 4) < 7){
						pTxMsg->raw.queue = ((skb->mark & QOS_FILTER_MARK) >> 4) + 1 ;
					}
					else{
						pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4;
					}
				}
			}
			else{
				pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
			}
#else			
			pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
#endif
		}
		else{
#ifdef TCSUPPORT_EPON_DUMMY
			if(gDummyEnable){
				if(isDummyPkt(skb)){
					pTxMsg->raw.queue = 0 ;
				}else{
					if(txq  < 7){
						pTxMsg->raw.queue = txq + 1 ;
					}
					else{
						pTxMsg->raw.queue = txq;
					}
				}
			}
			else{
				pTxMsg->raw.queue = txq ;
			}
#else
			pTxMsg->raw.queue = txq ;
#endif
		}
#else
#ifdef TCSUPPORT_EPON_DUMMY
		if(gDummyEnable){
			if(isDummyPkt(skb)){
					pTxMsg->raw.queue = 0 ;
			}else{
				if(((skb->mark & QOS_FILTER_MARK) >> 4) < 7){
					pTxMsg->raw.queue = ((skb->mark & QOS_FILTER_MARK) >> 4) + 1 ;
				}
				else{
					pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
				}
			}
		}
		else{
			pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
		}
	#else
		pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
	#endif
#endif
#else
	#ifdef TCSUPPORT_EPON_DUMMY
		if(gDummyEnable){
			if(isDummyPkt(skb)){
					pTxMsg->raw.queue = 0 ;
			}else{
				if(((skb->mark & QOS_FILTER_MARK) >> 4) < 7){
					pTxMsg->raw.queue = ((skb->mark & QOS_FILTER_MARK) >> 4) + 1 ;
				}
				else{
					pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
				}
			}
		}
		else{
			pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
		}
	#else
		pTxMsg->raw.queue = (skb->mark & QOS_FILTER_MARK) >> 4 ;
	#endif
#endif
#endif /* TCSUPPORT_CPU_EN7521 */
		pTxMsg->raw.tse = (skb->pon_mark & QOS_TSE_MARK) ? 1 : 0 ;
		pTxMsg->raw.tsid = (pTxMsg->raw.tse) ? (skb->pon_mark & QOS_TSID_MARK) : 0 ;	
		pTxMsg->raw.channel = skb->v_if ;
#ifndef TCSUPPORT_CPU_EN7521
		if(isHighestPriorityPacket(skb)){		/* Set pppoe packet to highest queue, queue 6 */
			pTxMsg->raw.queue = queue6;
		}
#endif
		
#ifdef TCSUPPORT_RA_HWNAT
		if((ra_sw_nat_hook_magic) && (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE))) {
			pTxMsg->raw.fport = TXMSG_FPORT_PPE ;//DPORT_PPE
		} 
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)		
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		if ((isNeedHWQoS == 1 ) && (ra_sw_nat_hook_tx != NULL)) 
#else
		if(ra_sw_nat_hook_tx != NULL)
#endif
#else	
		if(ra_sw_nat_hook_tx != NULL) 
#endif
{

		xpon_info->magic = FOE_MAGIC_EPON;
    	
#ifdef TCSUPPORT_CPU_EN7521
		/* In EN7512/21, EPON's llid is filled in PPE foe's channel field. */
		xpon_info->channel = pTxMsg->raw.channel & chnMask;
		PON_MSG(MSG_TRACE, "ewan_hwnat_hook_tx:---llid = 0x%x\n",xpon_info->channel);
#else /* TCSUPPORT_CPU_EN7521 */
		/* In MT7510/20, EPON's llid is filled in PPE foe's stag field. */
		xpon_info->stag = pTxMsg->raw.channel & chnMask;
		/* EN7512 tx queue is configured by QDMA */
		xpon_info->txq = pTxMsg->raw.queue & qMask ;
				
		/* In MT7510/20, bit5~1 and bit0 of PPE foe's tsid field stand for tsid and tse respectively */
			xpon_info->tsid = ((pTxMsg->raw.tsid & 0x1f)<<1) | (pTxMsg->raw.tse & 0x1) ;
			
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)					
			if (trtcmEnable)
			{	
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
				if(xpon_info->txq == 0) {			
					xpon_info->tsid = 0;
				}else{
					xpon_info->tsid = ((CAR_QUEUE_NUM - xpon_info->txq +1)&0x1f)<<1 | 1;
				}
#else		
				xpon_info->tsid = ((trtcmTsid&0x1f)<<1) | 1;
#endif
			}	
#endif		
		PON_MSG(MSG_TRACE, "ewan_hwnat_hook_tx:---xpon_info->txq = 0x%x\n",xpon_info->txq) ;
		PON_MSG(MSG_TRACE, "ewan_hwnat_hook_tx:---llid = 0x%x\n",xpon_info->stag);

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
		if((sw_isSWQosActive && !sw_isSWQosActive())
			||(sw_isSWQosActive && sw_isSWQosActive() && sw_isHwnatOffloadEnable && sw_isHwnatOffloadEnable())){
#endif
				if (ra_sw_nat_hook_tx(skb, xpon_info, xpon_info->magic) == 0) {
				return 1;//don't printf err msg when drop keepalive packet.
			}
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
		}
		#endif
#endif /* TCSUPPORT_CPU_EN7521 */
		}
#endif
	} 
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
uint8 noRxDscpInt=0;
static int isDHCPPacket(unsigned char *ip_header){
	unsigned short int skb_src_port, skb_dst_port;

	if(ip_header == NULL){
		return 0;	
	}

	skb_dst_port = *((unsigned short int *)(ip_header + 22));
	skb_src_port = *((unsigned short int *)(ip_header + 20));
	
	if(((skb_dst_port == 68) || (skb_src_port == 68)) //CPE is client
		||((skb_dst_port == 67) || (skb_src_port == 67))) //CPE is server
	{
		PON_MSG(MSG_DBG, "isDHCPPacket:received DHCP packet!\n") ;	
		return 1;	
	}
	return 0;
}
static int isPPPPacket(unsigned char *ip_header){
	unsigned short int ether_protocol, p2p_protocol;

	if(ip_header == NULL){
		return 0;	
	}
	
	ether_protocol = *((unsigned short int *)(ip_header));
	if(ether_protocol == 0x8863)//pppoe discovery
	{
		PON_MSG(MSG_DBG, "isPPPPacket:received pppoe discovery packet!\n") ;	
		return 1;
	}
	else if(ether_protocol == 0x8864)//pppoe session
	{	
		p2p_protocol = *((unsigned short int *)(ip_header + 8));
		
		if((p2p_protocol == 0xc021) //LCP
			|| (p2p_protocol == 0xc223) //CHAP
			|| (p2p_protocol == 0xc023) //PAP
			|| (p2p_protocol == 0x8021)) //IPCP 0x8021?
		{
			PON_MSG(MSG_DBG, "isPPPPacket:received pppoe session packet!\n") ;
			return 1;	
		}
	}
	return 0;
}

static int isPPPorDHCP(struct sk_buff *skb)
{
		unsigned char *mac_header = NULL;
		unsigned char *ip_header = NULL;
		
		mac_header = (unsigned char *)(skb->data);
		if(mac_header != NULL){ //check vlan_tag
			ip_header = mac_header+12;
			
			if(*((unsigned short int *)(ip_header)) == 0x8100){
				ip_header += 4;
				if(*((unsigned short int *)(ip_header)) == 0x8100){
					ip_header += 4;
				}
			}
		}
		
		if(isDHCPPacket(ip_header) || isPPPPacket(ip_header))
		{	
			return 1;
		}	
		
		return 0;
}

extern uint8 attackEnhance;
#endif
#ifndef TCSUPPORT_CPU_EN7521
/*****************************************************************************
//function :
//		isDiscoverGate
//description : 
//		this function is set MPCP synctime.
//input :	
//	struct sk_buff *skb
//      
//output :
//	0:is not mpcp packet
//	1: is mpcp packet, need drop
******************************************************************************/	
static int isDiscoverGate(struct sk_buff *skb)
{
	unsigned short int ether_protocol, p2p_protocol;
	unsigned char  *p = NULL;
	unsigned short int type,opcode;	
	unsigned char flag;
	
	p = (unsigned char *)(skb->data);
	type = *((unsigned short int *)(p + 12));
	if(type == 0x8808){		/*mpcp packet*/
		opcode = *((unsigned short int *)(p + 14));
		if(opcode == 0x2){	/*gate*/
			flag = *(p+20);
			if(flag & (1<<3)){	/*discovery gate*/
				gMpcpSynctime = *((unsigned short int *)(p + 27));
				//printk("isDiscoverGate gMpcpSynctime=0x%x\n",gMpcpSynctime);
			}
		}
		return 1;	
	}
	
	return 0;
}
#endif
int ewan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx, idx ;
	ushort *p ;
	
	idx = pRxMsg->raw.channel ;
	
	if(pRxMsg->raw.oam) {
		skb_push(skb, 2) ;
		p = (ushort *)skb->data ;
		p[0] = idx ;
		netIdx = PWAN_IF_OAM ;
	} else {
#ifndef TCSUPPORT_CPU_EN7521
		if(isDiscoverGate(skb) == 1){
			PON_MSG(MSG_DBG, "Drop MPCP discover gatepacket.\n") ;
			return -1;
		}
#endif
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
		if((attackEnhance == 1) && (noRxDscpInt == 1))
		{
//			if(gpPonSysData->debugLevel & MSG_DBG)
//				__dump_skb(skb, skb->len) ;	
			
			if(isPPPorDHCP(skb) != 1)
			{
				PON_MSG(MSG_DBG, "Drop TCP sync flood frame for epon test.\n") ;
				return -1;
			}
		}
#endif
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
			return -1 ;
		}

		netIdx = PWAN_IF_DATA ;
	}
		
	return netIdx ;
}

/*****************************************************************************
******************************************************************************/
int ewan_init(EWAN_Priv_T *pEWanPriv)
{
	int i ;
    
	for(i=0 ; i<CONFIG_EPON_MAX_LLID ; i++) {
		memset(&pEWanPriv->llid[i].info, 0, sizeof(EWAN_LlidInfo_T)) ;
		memset(&pEWanPriv->llid[i].stats, 0, sizeof(struct net_device_stats)) ;
		pEWanPriv->llid[i].info.rxDrop = 1 ;
		pEWanPriv->llid[i].info.txDrop = 1 ;
#ifdef TCSUPPORT_EPON_FPGA_WORKAROUND
		pEWanPriv->llid[i].info.rxDrop = 0 ;//		 1
		pEWanPriv->llid[i].info.txDrop = 0 ;//	   1
		
		//pEWanPriv->llid[i].info.rxLb = 0 ;//lpbk test
#endif
		
	}

	pwan_create_net_interface(PWAN_IF_OAM) ;

	return 0 ;
}


#endif /* TCSUPPORT_WAN_EPON */
