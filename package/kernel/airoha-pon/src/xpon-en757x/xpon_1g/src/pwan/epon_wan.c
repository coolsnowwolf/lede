#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif

#include "common/drv_global.h"
#include "pwan/epon_wan.h"
#include "pwan/xpon_netif.h"

/************************************************************************************************
							Struct Define
**************************************************************************************************/


/************************************************************************************************
							Macro Define
**************************************************************************************************/


/*************************************************************************************************
							Function Declare
***************************************************************************************************/

#ifdef TCSUPPORT_XPON_IGMP
extern int (*xpon_sfu_up_multicast_vlan_hook)(struct sk_buff *skb, int clone);
extern int (*xpon_sfu_multicast_protocol_hook)(struct sk_buff *skb);
extern int (*xpon_sfu_up_multicast_incoming_hook)(struct sk_buff *skb, int clone);
#endif
extern int isHighestPriorityPacket(struct sk_buff *skb);
extern int bitmap_format_convert(void);
extern int xpon_wan_restore_offload_info(struct sk_buff * skb,struct port_info *xpon_info, PWAN_FETxMsg_T *pTxMsg, int ponMode);
extern int ewan_tx_vlan_handler_customer(struct sk_buff *skb, uint8_t netIdx);

extern epon_t eponDrv;
extern __u32 auto_switch_oam_fav;

uint keepalive_count=0;
uint rcv_srv_oam_flag = 0;
uint oam_fav_set_disabled = 0;
/**************************************************************************************************
							Variable Define
***************************************************************************************************/
extern unsigned short int gMpcpSynctime;
int isWan2WanTrafficTest = 0;
	
#ifdef TCSUPPORT_EPON_MAPPING
extern int (*epon_mapping_hook)(struct sk_buff *skb);
#endif


#ifdef TCSUPPORT_WAN_EPON
static int epon_igmp_pkt_handle(struct sk_buff * skb)
{
 #ifdef TCSUPPORT_XPON_IGMP
   /*epon sfu multicast vlan operation and fwdtbl learn*/
    if(xpon_sfu_multicast_protocol_hook && xpon_sfu_multicast_protocol_hook(skb))
    {
        if(xpon_sfu_up_multicast_vlan_hook)
        {
            if(xpon_sfu_up_multicast_vlan_hook(skb, 0) <=0)
            {
				XPON_DROP_PRINT;
                return -1;
            }
        }
        
        if(xpon_sfu_up_multicast_incoming_hook) 
        {
            if(xpon_sfu_up_multicast_incoming_hook(skb, 1) <= 0)
            {
				XPON_DROP_PRINT;
                return -1;
            }
        }
        
    }
#endif

	return 0;
}

int ewan_config_tx_msg(struct sk_buff * skb,PWAN_FETxMsg_T *pTxMsg, uint8_t netIdx)
{
#ifdef TCSUPPORT_CPU_EN7580
	unsigned char acnt_g0 = HWNAT_ACNT_G0_DEFAULT;
	unsigned char acnt_g1 = HWNAT_ACNT_G1_DEFAULT;
	unsigned char tsid = HWNAT_TSID_DEFAULT;
#endif	
	uint16_t *p = NULL;
	uint8_t channel = 0;

	pTxMsg->raw.fport = TXMSG_FPORT_GMAC ;
	
	if(netIdx == PWAN_IF_OAM) {
		pTxMsg->raw.oam = 1 ;
		p = (uint16_t *)skb->data ;
		channel = ntohs(p[0]) ;
		pTxMsg->raw.channel = channel ;
#ifdef TCSUPPORT_CPU_EN7580
		pTxMsg->raw.nboq = channel;
		pTxMsg->raw.ndp = 1;
		pTxMsg->raw.mtr_g = tsid;
		pTxMsg->raw.acnt_g0 = acnt_g0;
		pTxMsg->raw.acnt_g1 = acnt_g1;
#endif

	}
	else 
	{
		channel = skb->v_if;
		pTxMsg->raw.oam = 0 ;
		pTxMsg->raw.channel = channel ;
#ifdef TCSUPPORT_CPU_EN7580
		pTxMsg->raw.nboq = channel ;
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
#else
		pTxMsg->raw.tse = (skb->pon_mark & QOS_TSE_MARK) ? 1 : 0 ;
		pTxMsg->raw.tsid = (pTxMsg->raw.tse) ? (skb->pon_mark & QOS_TSID_MARK) : 0 ;	
#endif
	}

	return 0;
}


/*****************************************************************************
******************************************************************************/
int ewan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, struct port_info *xpon_info)
{
	uint idx = 0;

	if(netIdx == PWAN_IF_OAM) {

		ewan_config_tx_msg(skb, pTxMsg, netIdx);
		//remove oam llid index info(2bytes)
		skb_pull(skb, 2) ;

        gpEponPriv->oamChannel = pTxMsg->raw.channel;

        if(gpEponPriv->gBitmapSet){
            bitmap_format_convert();
        }
	}
	else 
	{
		if(isWan2WanTrafficTest)
	        return -1;
		if(xpon_wan_restore_offload_info(skb,xpon_info,pTxMsg, FOE_MAGIC_EPON)){ 
			PON_MSG(MSG_TRACE, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pTxMsg->word[0], pTxMsg->word[1]) ;
			return 0;
		}

		if( -1 == ewan_tx_vlan_handler_customer(skb, netIdx) ){
			XPON_DROP_PRINT;
            return -1;
		}
		
#ifdef TCSUPPORT_EPON_MAPPING
		if (  epon_mapping_hook){
			epon_mapping_hook(skb);
		}
#endif
		idx = skb->v_if ;
		if(idx>=EPON_1G_MAX_LLID_NUM ||
			!gpWanPriv->epon.llid[idx].info.valid)
		{
			XPON_DROP_PRINT;
			PON_MSG(MSG_ERR, "Transmit frame to invalid channel %d.\n", idx) ;
			//__dump_skb(skb, skb->len) ;
			return -1 ;
		}

		if(-1 == epon_igmp_pkt_handle(skb)){
			XPON_DROP_PRINT;
            return -1;
		}
        /********************************************/

		if(gpWanPriv->epon.llid[idx].info.txDrop) {
			gpWanPriv->epon.llid[idx].stats.tx_dropped++ ;
			XPON_DROP_PRINT;
			return -1 ;
		} 

		ewan_config_tx_msg(skb, pTxMsg, netIdx);
		
#ifdef TCSUPPORT_CPU_EN7580			
		xpon_info->nbq = xpon_info->channel;
		xpon_info->tsid = pTxMsg->raw.mtr_g;
#endif
		xpon_info->magic = FOE_MAGIC_EPON;    	
		xpon_info->channel = pTxMsg->raw.channel;
		PON_MSG(MSG_TRACE, "ewan_hwnat_hook_tx:---llid = 0x%x\n",xpon_info->channel);
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
				gpEponPriv->eponRdmMfy.mpcpGrantLen = *((unsigned short int *)(p + 25));
			    printk("[%s %d]Grant Length 0x%x\n", __FUNCTION__, __LINE__, gpEponPriv->eponRdmMfy.mpcpGrantLen);
			}
		}
		return 1;	
	}
	
	return 0;
}

#ifdef EPON_MAC_HW_TEST
int print_packet = 0;
int loopback_flag = 0;
#endif

/*****************************************************************************
//function :
//		is_service_packet
//description : 
//		this function is used to see if received oam service packet.
//input :	
//	struct sk_buff *skb
//      
//output :
//	TRUE:is service packet
//	FALSE: not service packet
******************************************************************************/	
bool is_service_packet(unsigned char * data)
{
    if ((data[18]==0x11 && data[19]==0x11 && data[20]==0x11)  //CUC & CTC OUI
	  ||(data[18]==0x58 && data[19]==0xD0 && data[20]==0x8F)  //SI A OUI
	  ||(data[18]==0x00 && data[19]==0x10 && data[20]==0x00)  //DPOE OUI
	  ||(data[18]==0x90 && data[19]==0x82 && data[20]==0x60)) //SI B OUI
	{
        return TRUE;
    }
	return FALSE;
}

void mpcp_fav_if_traffic_up(struct sk_buff *skb){
	
	if(is_service_packet(skb->data)){
		rcv_srv_oam_flag = 1;
		PON_MSG(MSG_WARN, "%s %d Received service OAM.\n",__FUNCTION__,__LINE__) ;
	}
	else if(1 == rcv_srv_oam_flag){
		keepalive_count++;
		PON_MSG(MSG_WARN, "%s %d Received kpalive OAM.\n",__FUNCTION__,__LINE__) ;

		/*Switch to MPCP fav if received 5 keepalive packets.*/
		if((keepalive_count >= EPON_KEEPALIVE_OAM_THRESHOLD) && (0 == oam_fav_set_disabled)){
				eponMacTxOamFav(0);
				oam_fav_set_disabled = 1;
				PON_MSG(MSG_WARN, "%s %d Switch to MPCP favor.\n",__FUNCTION__,__LINE__) ;
		}
	}
	else
		PON_MSG(MSG_WARN, "%s %d Received keepalive OAM but no service OAM yet.\n",__FUNCTION__,__LINE__) ;

	return;
}

static int ewan_enhance_attack(struct sk_buff *skb)
{
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
	if((attackEnhance == 1) && (noRxDscpInt == 1))
	{				
		if(isPPPorDHCP(skb) != 1)
		{
			PON_MSG(MSG_DBG, "Drop TCP sync flood frame for epon test.\n") ;
			return -1;
		}
	}
#endif
	return 0;
}
int ewan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag)
{
	uint netIdx, idx ;
	ushort *p ;
	
	idx = pRxMsg->raw.channel ;
	
	if(pRxMsg->raw.oam) {
#ifdef EPON_MAC_HW_TEST
		if(loopback_flag){
			if(print_packet){
				__dump_skb(skb,pktLens);
			}
			return -1;
		}
#endif	
#ifndef TCSUPPORT_CPU_AN7552
		if((auto_switch_oam_fav == 1) 
		   && (idx == 0) 
		   && (eponDrv.llidMask & 0x1))
		{			
			mpcp_fav_if_traffic_up(skb);		
		}
#endif	
		skb_push(skb, 2) ;
		p = (ushort *)skb->data ;
		p[0] =  htons(idx) ;
		netIdx = PWAN_IF_OAM ;
	} else {
		if(isDiscoverGate(skb) == 1){
			PON_MSG(MSG_DBG, "Drop MPCP discover gatepacket.\n") ;
			return -1;
		}
		if(ewan_enhance_attack(skb) == -1){
			PON_MSG(MSG_DBG, "Drop MPCP discover gatepacket.\n") ;
			return -1;
		}
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
    
	for(i=0 ; i<EPON_1G_MAX_LLID_NUM ; i++) {
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
