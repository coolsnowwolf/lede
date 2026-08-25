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

#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include "linux/libcompileoption.h"
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif
#include <ecnt_hook/ecnt_hook_qdma.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_multicast_general.h>
#include "common/xpon_global.h"
#include "common/xpon_api.h"

#include <ecnt_hook/ecnt_hook_vlan.h>
#include <ecnt_hook/ecnt_hook_bbf247.h>
extern int xgpon_fast_mode_flag;

/************************************************************************
*				D E F I N E S	&	C O N S T A N T S
*************************************************************************
*/
void __dump_skb(struct sk_buff *skb, uint pktLen) 
{
    unchar n, *p = (unchar *)skb->data ;
    int i ;

    for(i=0 ; i<pktLen ; i++) {
        n = i & 0x0f ;
        
        if(n == 0x00)       printk(" \n%.4x: ", i) ;
        else if(n == 0x08)  printk(" ") ;

        printk("%.2x ", *p++) ; 
    }
    printk("\n") ;
}


#if defined(TCSUPPORT_SWNAT)
extern int (*sw_upstream_nat_tx_hook) (struct sk_buff * skb, uint msg0, uint msg1, struct port_info* qdma_info);
extern int (*sw_downstream_nat_rx_hook) (struct sk_buff * skb); 
#endif

#if defined (TCSUPPORT_PON_SFU_HGU_HYBRID)
extern int (*xpon_hybrid_down_multicast_incoming_hook)(struct sk_buff *skb, int clone);
#endif

#ifdef TCSUPPORT_CPU_ARMV8
extern int get_ethaddr(unsigned char *ethaddr, int len);
#endif

#ifdef TCSUPPORT_PON_VLAN
extern int (*pon_vlan_is_ds_1_to_N_hook)(struct sk_buff **pskb, int *count);
extern int (*pon_vlan_ds_1_to_N_handler_hook)(struct sk_buff **pskb, int index);
#endif

#if defined(WAN2LAN) 
#define TX_STAG_LEN 6
extern void macSend(uint32 chanId, struct sk_buff *skb);
extern int masko_on_off;
#if defined(TCSUPPORT_WAN2LAN_EXT)
extern int masko_ext;
#endif
#endif

extern int (*multicast_hwnat_drop_entry_hook)(struct sk_buff* skb);
extern int (*wan_speed_test_hook)(struct sk_buff*);
extern int (*wan_tr471_hook)(struct sk_buff*);
#define SPEED_TEST_SUCCESS	0

#define dec_pwan_tx_data_cnt()
#define inc_pwan_tx_data_cnt()

uint16_t  qdma_int_enable = 0;

#if defined(WAN2LAN)

static inline int mirror_pre_tx_prepare(void)
{
#if defined(TCSUPPORT_WAN2LAN_EXT)
    if(masko_ext & 0x04){
        return 1;
    }
#endif
    return 0;
}

static inline int mirror_post_tx_prepare(void)
{
#if defined(TCSUPPORT_WAN2LAN_EXT)
	if(masko_on_off || (masko_ext & 0x01) )
#else
	if(masko_on_off)
#endif
    {
        return 1;
    }

    return 0;
}

static struct sk_buff * copy_pwan_skb(struct sk_buff *skb, unchar netIdx) 
{
	struct sk_buff *skb2 = NULL;
	int len = TX_STAG_LEN;
	static const u_char omciHdr[] = {0x00,0x00,0x00,0x00,0x00,0x02, 0x00,0x00,0x00,0x00,0x00,0x01, 0x88, 0xb5};
	
	if (PWAN_IF_OMCI == netIdx){
		len += 14;
	}

	//Check the skb headroom is enough or not. shnwind 20100121.
	if(skb_headroom(skb) < len )
		skb2 = skb_copy_expand(skb, len , skb_tailroom(skb) , GFP_ATOMIC);
	else
		skb2 = skb_copy(skb, GFP_ATOMIC);

#if defined(TCSUPPORT_RA_HWNAT)
	if (ra_sw_nat_hook_xfer)
		ra_sw_nat_hook_xfer(skb, skb2);
#endif	           

	if(unlikely(!skb2)){
		printk("\nFAIL(%s): wan2lan allocation\n", __FUNCTION__);
        return NULL;
	}
    
	if (PWAN_IF_OMCI == netIdx){
		skb_push(skb2, 14);
		memcpy(skb2->data, omciHdr, ARRAY_SIZE(omciHdr));
	}
	skb2->mark |= SKBUF_COPYTOLAN;
    return skb2;
}

#else

#define mirror_pre_tx_prepare()  (0)
#define mirror_post_tx_prepare() (0)
#define copy_pwan_skb(x,y) (NULL)

#endif

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
#ifdef CONFIG_8021P_REMARK
    /* 
     * Fill vlanTag's PCP (Priority Code Point) by skb->mark's bit8~11.
     * If packet's byte12-13 (vlanTag's TPID) is 0x8100, just change
     * the packet's vlanTag's PCP.
     * If packet's byte12-13 (vlanTag's TPID) isn't 0x8100, 
     * insert a VLAN tag with TPID=0x8100, PCP=mark, and VID=0.
     */
    static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
    {
        char * vlan_p = NULL, *ether_type_ptr = NULL;
        unsigned char ucprio = 0;
        unsigned char uc802prio = 0;
        uint16 vid=0;
    
        if ( skb->mark & QOS_8021p_MARK ) {
            /*vlan tagging*/
            ether_type_ptr = skb->data + 12;
            
            ucprio = (skb->mark & QOS_8021p_MARK) >> 8;
            if ( ucprio < QOS_8021P_0_MARK) { //0~7 remark
                uc802prio = ucprio;
            }
            else if ( QOS_8021P_0_MARK == ucprio ) {    //zero mark
                uc802prio = 0;
            }
            else{//pass through
                /*do nothing*/
                return skb;
            }
            if(ntohs(*(unsigned short *)ether_type_ptr) == 0x8100){
                vid=(ntohs(*(unsigned short *)(ether_type_ptr+2)) & 0xfff);
            }
            else{
               return skb; 
            }
        
            vlan_p = skb->data + 12;
            *(unsigned short *)vlan_p = htons(0x8100);
            
            vlan_p += 2;
            /*3 bits priority and vid vlaue*/
            *(unsigned short*)vlan_p = htons(((uc802prio & 0x7) << 13)|vid) ;
            skb->network_header -= htons(VLAN_HLEN);
            skb->mac_header -= htons(VLAN_HLEN);
    
        }
        return skb;
    }
#endif /*CONFIG_8021P_REMARK*/


int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
#define GOTO_DROP(n) do {XPON_DROP_PRINT; goto drop##n; }while(0)

	int ret = NETDEV_TX_OK ;
	PWAN_NetPriv_T *pNetPriv = NULL ;
	PWAN_FETxMsg_T txBmMsg;
	uint16 pktLens = 0;

#ifdef TCSUPPORT_WAN_GPON
	u16    gemport = 0;
	u16    gemIdx  = 0;
#endif
	int txq = 0;
	struct sk_buff * unshared_skb = NULL;
	struct port_info xpon_info= {0}; /* only used in gwan_prepare_tx_message() */
	struct sk_buff * skb_mirror = NULL;

	memset(&txBmMsg, 0, sizeof(PWAN_FETxMsg_T));
	
	PON_MSG(MSG_TRACE, "Start to transmit a packets.\n") ;
    
	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
	skb->dev = dev ; /* Use for tx DSCP recycle. */

	unshared_skb = skb_unshare(skb, GFP_ATOMIC);
	skb = unshared_skb;
	if(NULL == unshared_skb)
	{
		printk("ERR %s skb_unshare failed! \n",__FUNCTION__);
		GOTO_DROP(2);
	}

	if(mirror_pre_tx_prepare() && (skb_mirror = copy_pwan_skb(skb, pNetPriv->netIdx) ) ){
		macSend(WAN2LAN_CH_ID, skb_mirror);
		skb_mirror = NULL;
	}

	if(SOFT_LOOPBACK_MODE(pNetPriv)) {
		skb->protocol = eth_type_trans(skb, dev) ;
		skb->ip_summed = CHECKSUM_UNNECESSARY ;
		dev->last_rx = jiffies ;
		netif_rx(skb) ;
		goto post_qdma_tx;
		
	}
	
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_OFF) {
		GOTO_DROP(2);
	} 
	
	FH_VLAN_OPERATION(skb);
	FH_DSCP_OPERATION(skb);

#ifdef  CONFIG_8021P_REMARK
    skb=vlanPriRemark(skb);
        if(skb==NULL){
        printk("(%s)802.1p remark failure\r\n", __FUNCTION__);
        return NETDEV_TX_OK;
    }
#endif

#ifdef TCSUPPORT_WAN_GPON
	if((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && 
		((GPON_CURR_STATE == GPON_10G_STATE_O5) || (GPON_CURR_STATE == GPON_10G_STATE_O9 && ng2_o4_to_09))) {
		if(gwan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, txq, &xpon_info) != 0)
		{
			GOTO_DROP(2);
		}
		gemport=skb->gem_port;
	} else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
    if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
        if(ewan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, txq, &xpon_info) != 0)  
        {
            GOTO_DROP(2);
        }
    } else 
#endif /* TCSUPPORT_WAN_EPON */
	{
		PON_MSG(MSG_ERR, "The xPON link status is off.\n") ;
		GOTO_DROP(2);
	}

#ifdef XPON_MAC_CONFIG_DEBUG
	if(gpPonSysData->debugLevel & MSG_CONTENT) { 
			PON_MSG(MSG_CONTENT, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, txBmMsg.word[0], txBmMsg.word[1]) ;
			__dump_skb(skb, skb->len) ;
		}
#endif
	
	if(PWAN_IF_DATA != pNetPriv->netIdx) {
		if(((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) && (pNetPriv->netIdx == PWAN_IF_OAM || pNetPriv->netIdx == PWAN_IF_EAPOL))||
			((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && (pNetPriv->netIdx == PWAN_IF_OMCI))){ 

			/* make the OAM packet to NO_DROP_PACKET */
			
		}
		else{
#ifdef XPON_MAC_CONFIG_DEBUG
			if(gpPonSysData->debugLevel & MSG_CONTENT){
				PON_MSG(MSG_CONTENT, "%s(%d)sysLinkStatus = %d, netIdx=%d.\n",
					__func__, __LINE__, gpPonSysData->sysLinkStatus, pNetPriv->netIdx);
				__dump_skb(skb, skb->len);
			}
#endif
			GOTO_DROP(2);
		}
	} 
	else { /*data packet path*/

		/* padding for the packet that length less than 64 byte */
		if(unlikely(skb->len < ETH_ZLEN)) {
			if(skb_padto(skb, ETH_ZLEN)) {
				skb = NULL;
				GOTO_DROP(1);
			}
			skb_put(skb, ETH_ZLEN-skb->len) ;
		}
	} 

	if(mirror_post_tx_prepare() ){
		skb_mirror = copy_pwan_skb(skb, pNetPriv->netIdx);
	}

	pktLens= skb->len ;

	if (xgpon_fast_mode_flag) {
		xpon_info.fast = 1;
	} else {
		xpon_info.fast = 0;
	}


#if defined(TCSUPPORT_SWNAT)
    if(sw_upstream_nat_tx_hook){
        sw_upstream_nat_tx_hook(skb, txBmMsg.word[0], txBmMsg.word[1], &xpon_info);
    }
#endif

    if(pNetPriv->netIdx == PWAN_IF_EAPOL){
        //printk("[%s %d]Send EAPOL packet.\n", __FUNCTION__,__LINE__);
        if(skb->len<ETH_ZLEN){
            skb_put(skb, ETH_ZLEN-skb->len);
        }
    }

	/* call QDMA API to transmit the packet */
	ret = QDMA_API_TRANSMIT_PACKETS(ECNT_QDMA_WAN, skb, txBmMsg.word[0], txBmMsg.word[1], &xpon_info) ;
	/* 
	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		skb should not be used blow this line !!!
	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	*/

	if(ret != 0)  /* when drop keepalive packet,ret = 1 */
	{
		if(ret < 0) 
			PON_MSG(MSG_ERR, "Transmit packet to QDMA failed.\n") ;
		
		if (pNetPriv->netIdx==PWAN_IF_DATA){
			GOTO_DROP(1);
		}
		else{
			GOTO_DROP(2);
		}
	}
	else /* update packet conuter and length */
	{ 
		pNetPriv->stats.tx_packets++ ;
		pNetPriv->stats.tx_bytes += pktLens ;

#ifdef TCSUPPORT_WAN_GPON
		if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON)
		{
			if (pNetPriv->netIdx == PWAN_IF_OMCI)
				gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK) ;
			else
				gemIdx = (gemport) ? (gpWanPriv->gpon.gemIdToIndex[gemport] & GPON_GEM_IDX_MASK) : CONFIG_GPON_10G_MAX_GEMPORT ; 
			if(gemIdx < CONFIG_GPON_10G_MAX_GEMPORT) {
				gpWanPriv->gpon.gemPort[gemIdx].stats.tx_packets++ ;
				gpWanPriv->gpon.gemPort[gemIdx].stats.tx_bytes += pktLens ;
			}
		}
#endif /* TCSUPPORT_WAN_GPON */
	}

post_qdma_tx:
	
	if( mirror_post_tx_prepare() && skb_mirror){
		macSend(WAN2LAN_CH_ID, skb_mirror);
		skb_mirror = NULL ;
	}

	dev->trans_start = jiffies ;
	return ret ;
	
drop1:
	dec_pwan_tx_data_cnt();
			
drop2:
	pNetPriv->stats.tx_dropped++ ;
	if( skb  )
		dev_kfree_skb_any(skb) ;


	if(skb_mirror) dev_kfree_skb_any(skb_mirror);
	return ret ;
#undef GOTO_DROP
}


int get_pon_link_type(void)
{
    return gpPonSysData->sysLinkStatus;
}

void get_interface_mac_addr(unchar * buffer)
{
	unchar flashMacAddr[ETH_ADDR_LEN] = {0};
#ifndef TCSUPPORT_CPU_ARMV8
	int i = 0;
#endif

#ifdef TCSUPPORT_CPU_ARMV8
	if(get_ethaddr(flashMacAddr, sizeof(flashMacAddr)) != 0){
		printk("func:%s get ethaddr error!!!\n",__func__);
	}
#else
	for (i = 0; i < ETH_ADDR_LEN; i++)
		flashMacAddr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
#endif	
	if (
		flashMacAddr[0] == 0 && flashMacAddr[1] == 0 && flashMacAddr[2] == 0 &&
		flashMacAddr[3] == 0 && flashMacAddr[4] == 0 && flashMacAddr[5] == 0
	)
		printk(KERN_INFO "\nThe MAC address in flash is null! Use default!\n"); 
	else	
		memcpy(buffer, flashMacAddr, ETH_ADDR_LEN);
}

int mtk_xpon_is_multicast_addr(struct sk_buff* skb)
{
	char mac[3]  =  { 0x01,0x00,0x5e};
	char mac2[2] = {0x33,0x33};
	unsigned char* addr= eth_hdr(skb)->h_dest;

	if(NULL == addr)
		return 0;

	if (!memcmp(addr,mac,3))
		return 1;
	if (!memcmp(addr,mac2,2))
		return 1;

	return 0;
}




struct sk_buff *replace_oam_pkt(struct sk_buff *skb, int len)
{
	struct sk_buff *skb2 = NULL;
	if(skb == NULL || len < 0){
		printk("\r\n[%s][%d] skb or len error.\r\n",__FUNCTION__,__LINE__); 
		return skb;
	}

	skb2 = dev_alloc_skb(len);		
	if (!skb2){
		printk("\r\n[%s][%d]alloc skb failed.\r\n",__FUNCTION__,__LINE__); 
		return skb;
	}

	skb_put(skb2, skb->len);
	memcpy(skb2->data, skb->data, len);

	dev_kfree_skb(skb);
	return skb2;
}
bool is_8021x_packet(unsigned char* data)
{
    /*EAPOL and MacSec ether type*/
    if (data[12]==0x88 && (data[13]==0x8e || data[13]==0xe5)){
        return TRUE;
    }
    return FALSE;
}
static int check_and_do_1toN_vlan_opreation(struct sk_buff *skb,char netIdx)
{
#ifdef TCSUPPORT_PON_VLAN
		char dst[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
		int vlanRuleCnt=0, i=0;
		struct sk_buff* skb2=NULL;
		int ret = 0;

		/*	 HGU downstream broadcast 1 to N vlan hander*/
		if(!memcmp(skb_mac_header(skb),dst,6) && PWAN_IF_DATA == netIdx  
			&& pon_vlan_is_ds_1_to_N_hook && pon_vlan_ds_1_to_N_handler_hook)
		{
			if(pon_vlan_is_ds_1_to_N_hook(&skb,&vlanRuleCnt))
			{
				for(i = 0; i < vlanRuleCnt; i++){
					if(i == vlanRuleCnt-1){   //last one, use skb itsself
						if(pon_vlan_ds_1_to_N_handler_hook(&skb,i+1) != 1){
							break;
						}
						ret = 1;
						netif_receive_skb(skb);
						break;
					}

					//copy skb, handle vlan and call netif_receive
					skb2 = skb_copy(skb,GFP_ATOMIC);
					if (skb2==NULL){
						continue;
					}

					if(pon_vlan_ds_1_to_N_handler_hook(&skb2,i+1) != 1){
						dev_kfree_skb(skb2);
						skb2 = NULL;
						continue;
					}
					netif_receive_skb(skb2);
				}
			}
		}
	return ret;
#else
	return 0;
#endif
}

int pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen)
{
	char netIdx=0xFF ;
	PWAN_NetPriv_T *pNetPriv = NULL ;
	int free_skb_flag = 0; /* 0: do nothing;    1: only drop skb;   2: drop skb and learn ppe drop entry */	
	PWAN_FERxMsg_T *pRxBmMsg_tmp = (PWAN_FERxMsg_T *)pMsg ; 	
	struct net_device *dev ;
	unchar LbFlag = 0 ;
	int ret = 0;
#ifdef TCSUPPORT_RA_HWNAT
	int ppeMagic = 0 ;
#endif /* TCSUPPORT_RA_HWNAT */	
	PWAN_FERxMsg_T pRxBmMsg_s;
	PWAN_FERxMsg_T *pRxBmMsg = &pRxBmMsg_s;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_TRAFFIC_CLASSIFY) || defined(TCSUPPORT_CMCCV2)
	unsigned char traffic_classify_flag = E_ECNT_FORWARD;
#endif/*TCSUPPORT_COMPILE*/

	memcpy(pRxBmMsg, pRxBmMsg_tmp, sizeof(PWAN_FERxMsg_T));

    if(is_8021x_packet(skb->data)){
        pRxBmMsg->raw.oam = 1;
    }

	if( pRxBmMsg->raw.oam )
		skb = replace_oam_pkt(skb, pktLen);
#ifdef XPON_MAC_CONFIG_DEBUG	
	if(gpPonSysData->debugLevel & MSG_CONTENT) {
			PON_MSG(MSG_CONTENT, "RX PKT LEN: %d, Rx Msg: %.8x, %.8x", pktLen, pRxBmMsg->word[0], pRxBmMsg->word[1]) ;
			__dump_skb(skb, pktLen) ;
	}
#endif
#ifdef TCSUPPORT_LAN_VLAN
	skb->lan_vlan_tci = 0;
	skb->lan_vlan_tci_valid = 0;
#endif
	/* 1. process the message information						   *
	 * 2. calculate the netIdx value from GEM port or LLID value   */
	/* parer the packet to corresponding interface */
#ifdef TCSUPPORT_WAN_GPON
	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		if((netIdx = gwan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag))<0 || (netIdx!=PWAN_IF_OMCI && (netIdx&0x7)!=PWAN_IF_DATA)) {
			if ((netIdx +1+PWAN_IF_DATA) < 0)
			{
				/*broadcast, unknown unicast, multicast*/
				ret = (netIdx +1+PWAN_IF_DATA) ;
			}
			goto drop ;
		}
#ifdef TCSUPPORT_RA_HWNAT
		ppeMagic = FOE_MAGIC_GPON ;
#endif /* TCSUPPORT_RA_HWNAT */
	} else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		if((netIdx = ewan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag))<0 || (netIdx!=PWAN_IF_OAM && netIdx!=PWAN_IF_DATA && netIdx!=PWAN_IF_EAPOL)) {
            PON_MSG(MSG_ERR, "%s %d Drop packet.\n",__FUNCTION__,__LINE__) ;
			goto drop ;
		}
#ifdef TCSUPPORT_RA_HWNAT
		ppeMagic = FOE_MAGIC_EPON ;
#endif /* TCSUPPORT_RA_HWNAT */
	} else 
#endif /* TCSUPPORT_WAN_EPON */
	{
		PON_MSG(MSG_ERR, "%s %d Drop packet.\n",__FUNCTION__,__LINE__) ;
		goto drop ;
	}

	PON_MSG(MSG_TRACE, "%s %d receive OAM packet.\n",__FUNCTION__,__LINE__) ;

	/*broadcast, unknown unicast, multicast*/
	if (netIdx > PWAN_IF_DATA)
	{
		ret = netIdx - PWAN_IF_DATA ;
		netIdx = PWAN_IF_DATA ;
	}
	PON_MSG(MSG_TRACE, "%s %d receive OAM packet.\n",__FUNCTION__,__LINE__) ;
	dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)netIdx] ;
	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;

	if(LbFlag) {   	
		skb_put(skb, pktLen) ;
		pwan_net_start_xmit(skb, dev) ;
	} 
	else {
		/* process the packet data to high layer */
		skb_put(skb, pktLen) ;		
#if defined(WAN2LAN)
		/* do wan2lan after skb_put, 
		 * because skb->len starts to have value from it */ 	
#if defined(TCSUPPORT_WAN2LAN_EXT)
		if(masko_on_off || (masko_ext & 0x02))
#else
		if(masko_on_off)
#endif
		{
			struct sk_buff *skb2 = NULL;
			int len = TX_STAG_LEN;
			const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
			
			if (netIdx == PWAN_IF_OMCI){
				len += 14;	// add dst mac, src mac, ether type
			}
			//Check the skb headroom is enough or not. shnwind 20100121.
			if(skb_headroom(skb) < len)
				skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
			else
				skb2 = skb_copy(skb, GFP_ATOMIC);
				
			if(skb2 == NULL)
				printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
			else
			{
				/* For oam: remove header 2 bytes: llid value  */
				if (netIdx == PWAN_IF_OAM){
					skb_pull(skb2, 2) ;
				}
				else if (netIdx == PWAN_IF_OMCI){
					skb_push(skb2, 14);
					memcpy(skb2->data, omciHdr, 14);
				}
				skb2->mark |= SKBUF_COPYTOLAN;
				macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx
			}
		}

#endif		

		skb->dev = dev ;
		dev->last_rx = jiffies ;
		skb->protocol = eth_type_trans(skb, dev) ;
		skb->ip_summed = CHECKSUM_UNNECESSARY ;
		if (PWAN_IF_DATA != netIdx){
			skb->protocol = 0;
		}

		if(wan_speed_test_hook)
		{
			if(wan_speed_test_hook(skb)==SPEED_TEST_SUCCESS){
				/* calculate the statistic counters */
				pNetPriv->stats.rx_packets++ ;
				pNetPriv->stats.rx_bytes += pktLen ;
				return SPEED_TEST_SUCCESS;
			}
		}

#ifdef TCSUPPORT_RA_HWNAT
		if(ra_sw_nat_hook_rxinfo && ppeMagic) {
			ra_sw_nat_hook_rxinfo(skb, ppeMagic, (char *)&(pRxBmMsg->word[1]), 4);
		}

		if(wan_tr471_hook)
		{
			if(wan_tr471_hook(skb)==SPEED_TEST_SUCCESS){
				/* calculate the statistic counters */
				pNetPriv->stats.rx_packets++ ;
				pNetPriv->stats.rx_bytes += pktLen ;
				return SPEED_TEST_SUCCESS;
			}
		}

		if(ra_sw_nat_hook_rx != NULL) {
			if(ra_sw_nat_hook_rx(skb)) {
				#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_TRAFFIC_CLASSIFY) || defined(TCSUPPORT_CMCCV2)
				if(PWAN_IF_DATA == netIdx)
				{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CMCCV2)
					skb->mark2 &= (~(QUEUE_MARK_MASK));
					skb->mark2 |= QUEUE_FROM_WAN;
#else/*TCSUPPORT_COMPILE*/
					skb->mark &= (~(PORT_MASK << PORT_MASK_OFFSET));
					skb->mark |= ((E_WAN_PORT & PORT_MASK) << PORT_MASK_OFFSET);
#endif/*TCSUPPORT_COMPILE*/
					ECNT_TRAFFIC_CLASSIFY_HOOK(skb, &traffic_classify_flag);

					if(E_ECNT_DROP == traffic_classify_flag)
					{
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
						if(ra_sw_nat_hook_drop_packet)
						{
							ra_sw_nat_hook_drop_packet(skb);
						}
#endif
						goto drop;
					}
			
				}
#endif/*TCSUPPORT_COMPILE*/
					//hybridmode xpon_igmp downstream multicast operation 
#if defined(TCSUPPORT_PON_SFU_HGU_HYBRID)
				if(xpon_hybrid_down_multicast_incoming_hook  && PWAN_IF_DATA == netIdx)
				{
					if(xpon_hybrid_down_multicast_incoming_hook(skb, 1) <= 0){
						PON_MSG(MSG_ERR, "xpon_hybrid_down_multicast_incoming_hook fail, drop pkt\n");
						free_skb_flag = 1;
						XPON_DROP_PRINT;
					        goto free_skb;
					}
				}
#endif
	
		if((PWAN_IF_DATA == netIdx)&&(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb)))
		{
			free_skb_flag = 2;
			goto free_skb;
		}

#if 1 //def CONFIG_USE_RX_NAPI
#if defined(WAN2LAN)
	#if defined(TCSUPPORT_WAN2LAN_EXT)
				if(masko_ext & 0x08)
				{
					struct sk_buff *skb2 = NULL;
					int len = TX_STAG_LEN;
					const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
					
					if (netIdx == PWAN_IF_OMCI){
						len += 14;	// add dst mac, src mac, ether type
					}
					//Check the skb headroom is enough or not. shnwind 20100121.
					if(skb_headroom(skb) < len)
						skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
					else
						skb2 = skb_copy(skb, GFP_ATOMIC);
						
					if(skb2 == NULL)
						printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
					else
					{
						skb_push(skb2,14);
						/* For oam: remove header 2 bytes: llid value  */
						if (netIdx == PWAN_IF_OAM){
							skb_pull(skb2, 2) ;
						}else if (netIdx == PWAN_IF_OMCI){
							skb_push(skb2, 14);
							memcpy(skb2->data, omciHdr, 14);
						}
						skb2->mark |= SKBUF_COPYTOLAN;
						macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx

					}
				}
	#endif		
#endif
				#if defined(TCSUPPORT_SWNAT)
				if(sw_downstream_nat_rx_hook){
					if(sw_downstream_nat_rx_hook(skb)){
						return 0;
					}
				}
				#endif

				if(PWAN_IF_DATA == netIdx){
					ret = FH_VLAN_FILTER(skb);
					if(-1 == ret){
						//printk("CALL_USER_HOOK_VLAN_FILTER_HANDLE fail, return\n");
						goto drop;
					}
					ret = FH_VLAN_RX_PROC(skb);
					if(-1 == ret){
						//printk("fb_pwan_rx_vlan_proc  fail free skb, return\n");
						goto drop;
					}
					
					FH_VLAN_PARSER(skb);
					CALL_USER_HOOK_MULTICAST_RX_DATA(skb);
				}

				if(PWAN_IF_DATA == netIdx && ECNT_CONTINUE == ECNT_API_BBF247_PKT_DS_HANDLE(skb)){
					//pkt has been forward by bbf247
				}
				else if(check_and_do_1toN_vlan_opreation(skb,netIdx)  == 1)
				{
					//pkt has done vlan operation and been forward 
				}
				else{
					netif_receive_skb(skb) ;
				}

#else
				netif_rx(skb) ;
#endif /* CONFIG_USE_RX_NAPI */

			}/*end ra_sw_nat_hook_rx*/
		}/*end ra_sw_nat_hook_rx != NULL*/
		else
#endif /* TCSUPPORT_RA_HWNAT */
		{
			if((PWAN_IF_DATA == netIdx)&&(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb)))
			{
				goto drop;
			}
			
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_TRAFFIC_CLASSIFY) || defined(TCSUPPORT_CMCCV2)
			if(PWAN_IF_DATA == netIdx)
			{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CMCCV2)
				skb->mark2 &= (~(QUEUE_MARK_MASK));
				skb->mark2 |= QUEUE_FROM_WAN;
#else/*TCSUPPORT_COMPILE*/
				skb->mark &= (~(PORT_MASK << PORT_MASK_OFFSET));
				skb->mark |= ((E_WAN_PORT & PORT_MASK) << PORT_MASK_OFFSET);
#endif/*TCSUPPORT_COMPILE*/
				ECNT_TRAFFIC_CLASSIFY_HOOK(skb, &traffic_classify_flag);

				if(E_ECNT_DROP == traffic_classify_flag)
				{
					goto drop;
				}
			}
#endif/*TCSUPPORT_COMPILE*/
			//hybridmode xpon_igmp downstream multicast operation 
#if defined(TCSUPPORT_PON_SFU_HGU_HYBRID)
			if(xpon_hybrid_down_multicast_incoming_hook  && PWAN_IF_DATA == netIdx)
			{
				if(xpon_hybrid_down_multicast_incoming_hook(skb, 1) <= 0){
					PON_MSG(MSG_ERR, "xpon_hybrid_down_multicast_incoming_hook fail, drop pkt\n");
					free_skb_flag = 1;
					XPON_DROP_PRINT;
					goto free_skb;
				}
			}
#endif

				if((PWAN_IF_DATA == netIdx)&&(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb)))
				{
					free_skb_flag = 2;
					goto free_skb;
				}

#if 1 //def CONFIG_USE_RX_NAPI
#if defined(WAN2LAN)
#if defined(TCSUPPORT_WAN2LAN_EXT)
				if(masko_ext & 0x08)
				{
					struct sk_buff *skb2 = NULL;
					int len = TX_STAG_LEN;
					const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
					
					if (netIdx == PWAN_IF_OMCI){
						len += 14;	// add dst mac, src mac, ether type
					}
					//Check the skb headroom is enough or not. shnwind 20100121.
					if(skb_headroom(skb) < len)
						skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
					else
						skb2 = skb_copy(skb, GFP_ATOMIC);
						
					if(skb2 == NULL)
						printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
					else
					{
						skb_push(skb2,14);
						/* For oam: remove header 2 bytes: llid value  */
						if (netIdx == PWAN_IF_OAM){
							skb_pull(skb2, 2) ;
						}else if (netIdx == PWAN_IF_OMCI){
							skb_push(skb2, 14);
							memcpy(skb2->data, omciHdr, 14);
						}
						skb2->mark |= SKBUF_COPYTOLAN;
						macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx
					}
				}
#endif		
#endif

				#if defined(TCSUPPORT_SWNAT)
				if(sw_downstream_nat_rx_hook){
					if(sw_downstream_nat_rx_hook(skb)){
						return 0;
					}
				}
				#endif

				if(PWAN_IF_DATA == netIdx){
					ret = FH_VLAN_FILTER(skb);
					if(-1 == ret){
						printk("CALL_USER_HOOK_VLAN_FILTER_HANDLE fail, return\n");
						goto drop;
					}
					ret = FH_VLAN_RX_PROC(skb);
					if(-1 == ret){
						printk("fb_pwan_rx_vlan_proc  fail free skb, return\n");
						goto drop;
					}
					
					FH_VLAN_PARSER(skb);
					CALL_USER_HOOK_MULTICAST_RX_DATA(skb);
				}

				if(PWAN_IF_DATA == netIdx && ECNT_CONTINUE == ECNT_API_BBF247_PKT_DS_HANDLE(skb)){
					//pkt has been forward by bbf247
				}
				else if(check_and_do_1toN_vlan_opreation(skb,netIdx)  == 1)
				{
					//pkt has done vlan operation and been forward 
				}
				else{
					netif_receive_skb(skb) ;
				}


#else
				netif_rx(skb) ;
#endif /* CONFIG_USE_RX_NAPI */
		}
	}/*end LbFlag*/
	/* calculate the statistic counters */
	pNetPriv->stats.rx_packets++ ;
	pNetPriv->stats.rx_bytes += pktLen ;
	return ret ;
free_skb:
	if(free_skb_flag)
		{
			if(1 == free_skb_flag)
			{
#if defined(TCSUPPORT_PON_SFU_HGU_HYBRID)
				/*WARNING: mutlicast hwnat drop rule must be add by mtk_xpon_multicast
				otherwise cant't be deleted by mtk_multicast while joining*/
				if(multicast_hwnat_drop_entry_hook && mtk_xpon_is_multicast_addr(skb))
				{
					multicast_hwnat_drop_entry_hook(skb);
				}
#else
			#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
				if(ra_sw_nat_hook_drop_packet && mtk_xpon_is_multicast_addr(skb))
				{
					ra_sw_nat_hook_drop_packet(skb);
				}
			#endif
#endif
			}
			dev_kfree_skb(skb);
			skb = NULL;
	}

drop:
	/* Dropped the packet */
	dev_kfree_skb(skb);
	/* calculate the statistic counters */
	gpWanPriv->dropUnknownPackets++ ;
	PON_MSG(MSG_ERR, "%s %d Drop packet.\n",__FUNCTION__,__LINE__) ;
	return -EFAULT+ret ;
}

int pwan_cb_event_handler(QDMA_EventType_t event)
{

	switch(event) {
		case QDMA_EVENT_RECV_PKTS:
#ifdef CONFIG_USE_RX_NAPI
			{
				struct net_device *dev = gpWanPriv->pPonNetDev[PWAN_IF_DATA] ;
				
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
				PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
				if(pNetPriv == NULL) {
					return IRQ_HANDLED ;
				}
				qdma_int_enable = 0;
				if(napi_schedule_prep(&pNetPriv->napi)) {
					QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
					__napi_schedule(&pNetPriv->napi) ;
				}
#else
				if(netif_rx_schedule_prep(dev)) {
					QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
					__netif_rx_schedule(dev) ;
				}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26) */
			}
#endif /* CONFIG_USE_RX_NAPI */
			break ;
		
		case QDMA_EVENT_NO_RX_BUFFER:
			break ;
			
		case QDMA_EVENT_TX_CROWDED:
			break ;
		
		default:
			break ;
	}
	
return 0 ;
}

/*****************************************************************************
******************************************************************************/
static int pwan_net_open(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;
	printk("*********************%s entered*********************\n", __FUNCTION__);
	
#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_enable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	
	netif_start_queue(dev) ;

#if defined(TCSUPPORT_XPON_LED)
	/* Schedule timer */
	if(strcmp(dev->name, "pon") == 0)
	{
	/*	init_timer(&gpWanPriv->pwan_timer);
		gpWanPriv->pwan_timer.expires = jiffies + msecs_to_jiffies(250);
		gpWanPriv->pwan_timer.function = pwan_net_timer;
		gpWanPriv->pwan_timer.data = 0;
		add_timer(&gpWanPriv->pwan_timer);
		memset(&pwanStats, 0, sizeof(pwanStats));*/ 
	}
#endif
	netif_carrier_on(dev);

	return 0 /* -ENODEV */ ;
}
/*****************************************************************************
******************************************************************************/
static int pwan_net_stop(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;
	printk("*********************%s entered*********************\n", __FUNCTION__);
#if defined(TCSUPPORT_XPON_LED)
	if((strcmp(dev->name, "pon") == 0) && (dev->flags & IFF_UP))
	{
	//	del_timer_sync(&gpWanPriv->pwan_timer);
	}
#endif

	netif_stop_queue(dev) ;

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{	
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_disable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	netif_carrier_off(dev);

	return 0 ;
}

/*****************************************************************
*****************************************************************/
static struct net_device_stats *pwan_net_get_stats(struct net_device *dev)
{
	PWAN_NetPriv_T *pNetPriv = NULL ;

	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
	return &pNetPriv->stats ;
}
/*****************************************************************
*****************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
static void pwan_net_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}
#endif

/*****************************************************************
*****************************************************************/
static int pwan_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret = 0 ;
	
	switch (cmd) {
		default :
			break ;
	}
		
	return ret ;
}
/*****************************************************************
*****************************************************************/
static int pwan_net_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    	return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	return 0;
}

#ifdef CONFIG_USE_RX_NAPI
/*****************************************************************
*****************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
static int pwan_net_poll(struct napi_struct *napi, int budget)
#else
static int pwan_net_poll(struct net_device *dev, int *budget)
#endif /* KERNEL_VERSION(2,6,26) */
{
	int npackets = 0 ;
	int done = 0 ;
	ulong flags = 0 ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	int quota = budget ;
#else
	int quota = min(dev->quota, *budget) ;
#endif /* KERNEL_VERSION(2,6,26) */
	
	npackets = QDMA_API_RECEIVE_PACKETS(ECNT_QDMA_WAN, quota) ;

		
	if(npackets < quota) {
		done = 1 ;
		
		spin_lock_irqsave(&gpWanPriv->rxLock, flags) ;
		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
		__napi_complete(napi);
#else
		__netif_rx_complete(dev) ;
#endif /* KERNEL_VERSION(2,6,26) */
		qdma_int_enable = 1;
		QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
		spin_unlock_irqrestore(&gpWanPriv->rxLock, flags) ;
	} 
	else {
		done = 0 ;
	}	
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	return npackets ;
#else 
	*budget -= npackets ;
	dev->quota -= npackets ;
	return done ? 0 : 1 ;
#endif /* KERNEL_VERSION(2,6,26) */
}
#endif /* CONFIG_USE_RX_NAPI */

/**********************************************************************************
 net_device_ops2.6.29net_deviceoperationX
 net_device_opsc
**********************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
const struct net_device_ops xpon_netdev_ops = {
	.ndo_open				= pwan_net_open,
	.ndo_stop				= pwan_net_stop,
	.ndo_start_xmit			= pwan_net_start_xmit,
	.ndo_get_stats 			= pwan_net_get_stats,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,19)
	.ndo_set_multicast_list 	= pwan_net_set_multicast_list,
#endif
	.ndo_do_ioctl          	 	= pwan_net_ioctl,
	.ndo_set_mac_address		= pwan_net_set_macaddr,
};
#endif /* KERNEL_VERSION(2,6,29) */

extern unsigned long flash_base;


/*****************************************************************
*****************************************************************/
int pwan_create_net_interface(uint idx)
{
	unchar mac_addr[ETH_ADDR_LEN] = PWAN_BASE_MAC_ADDR ;
	PWAN_NetPriv_T *pNetPriv = NULL ;
	unchar if_name[16]={0} ;

	if(idx >= PWAN_IF_NUMS) {
		return -EINVAL ;
	}

	if(gpWanPriv->pPonNetDev[idx]) {
		return -EEXIST ;
	}
	
	get_interface_mac_addr( mac_addr);
	memset(if_name, 0, 16) ;
	if(idx == PWAN_IF_OAM) {
		sprintf(if_name, "oam") ;
	} 
	else if(idx == PWAN_IF_OMCI) {
		sprintf(if_name, "omci") ;
	}
	else if(idx == PWAN_IF_EAPOL){
	    sprintf(if_name, "x_eapol");
	}
	else {
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		sprintf(if_name, CONST_XMCI_DEV_NAME) ;	
#else/*TCSUPPORT_COMPILE*/
		sprintf(if_name, "pon") ;
#endif/*TCSUPPORT_COMPILE*/		
	} 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,19)
	gpWanPriv->pPonNetDev[idx] = alloc_netdev(sizeof(PWAN_NetPriv_T), if_name, NET_NAME_USER,ether_setup);
#else	
	gpWanPriv->pPonNetDev[idx] = alloc_netdev(sizeof(PWAN_NetPriv_T), if_name, ether_setup) ;
#endif
	if(gpWanPriv->pPonNetDev[idx] == NULL) {
		PON_MSG(MSG_ERR, "Alloc net interface device %d (alloc_netdev()) failed\n", idx) ;
		return -ENOMEM ;
	}

	pNetPriv = netdev_priv(gpWanPriv->pPonNetDev[idx]) ;
	memset(pNetPriv, 0, sizeof(PWAN_NetPriv_T)) ;
	pNetPriv->netIdx = idx ;
	pNetPriv->drvLb = 0 ;
	memset(&pNetPriv->stats, 0, sizeof(struct net_device_stats)) ;
	gpWanPriv->pPonNetDev[idx]->addr_len = ETH_ADDR_LEN ;
	memcpy(gpWanPriv->pPonNetDev[idx]->dev_addr, mac_addr, ETH_ADDR_LEN) ;
	gpWanPriv->pPonNetDev[idx]->mtu = PWAN_MAX_MTU ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	gpWanPriv->pPonNetDev[idx]->netdev_ops = &xpon_netdev_ops;
#else
	gpWanPriv->pPonNetDev[idx]->open = pwan_net_open ;
	gpWanPriv->pPonNetDev[idx]->stop = pwan_net_stop ;
	gpWanPriv->pPonNetDev[idx]->hard_start_xmit = pwan_net_start_xmit ;
	gpWanPriv->pPonNetDev[idx]->get_stats = pwan_net_get_stats ;
	gpWanPriv->pPonNetDev[idx]->set_multicast_list = pwan_net_set_multicast_list ;
	gpWanPriv->pPonNetDev[idx]->do_ioctl = pwan_net_ioctl ;
	gpWanPriv->pPonNetDev[idx]->set_mac_address = pwan_net_set_macaddr;
#endif /* KERNEL_VERSION(2,6,29) */

#ifdef CONFIG_USE_RX_NAPI  
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pNetPriv->napi.weight = 128 ;
	netif_napi_add(gpWanPriv->pPonNetDev[idx], &pNetPriv->napi, pwan_net_poll, 128) ;
#else
	gpWanPriv->pPonNetDev[idx]->poll = pwan_net_poll ;
	gpWanPriv->pPonNetDev[idx]->weight = 128 ;
#endif /* KERNEL_VERSION(2,6,26) */
#endif /* CONFIG_USE_RX_NAPI */

	if(idx==PWAN_IF_OAM || idx==PWAN_IF_OMCI || idx==PWAN_IF_EAPOL) {
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_NOARP ;
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_BROADCAST ;
		gpWanPriv->pPonNetDev[idx]->flags &= ~IFF_MULTICAST ;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,19)
		gpWanPriv->pPonNetDev[idx]->features |= NETIF_F_NO_CSUM ;
#endif
	}

	if(register_netdev(gpWanPriv->pPonNetDev[idx]) != 0) {
		PON_MSG(MSG_ERR, "Register net device %s failed\n", if_name) ;
		free_netdev(gpWanPriv->pPonNetDev[idx]) ;
		gpWanPriv->pPonNetDev[idx] = NULL ;
		return -EFAULT ;
	}
	
	test_and_set_bit(__LINK_STATE_NOCARRIER, &gpWanPriv->pPonNetDev[idx]->state);
	
	return 0 ;
}

static int pwan_net_del(struct net_device *dev)
{

#if defined(TCSUPPORT_XPON_LED)
	if((strcmp(dev->name, "pon") == 0) && (dev->flags & IFF_UP))
	{
		//del_timer_sync(&gpWanPriv->pwan_timer);
	}
#endif

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{	
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		netif_napi_del(&pNetPriv->napi) ;
	
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */

	return 0 ;
}
/*****************************************************************
*****************************************************************/
int pwan_delete_net_interface(uint idx)
{
	if(idx >= PWAN_IF_NUMS) {
		return -EFAULT ;
	}
	
	if(!gpWanPriv->pPonNetDev[idx]) {
		return -EEXIST ;
	}
	
	pwan_net_del(gpWanPriv->pPonNetDev[idx]);

	unregister_netdev(gpWanPriv->pPonNetDev[idx]) ;
	free_netdev(gpWanPriv->pPonNetDev[idx]) ;
	gpWanPriv->pPonNetDev[idx] = NULL ;
 	
 	return 0 ;
}

/*****************************************************************************
******************************************************************************/
void pwan_destroy(void)
{
	int i = 0;
	
	for(i=0 ; i<PWAN_IF_NUMS ; i++) {
		pwan_delete_net_interface(i) ;
    }
}
/*****************************************************************************
******************************************************************************/
int pwan_init(void)
{
	int i = 0, ret = 0 ;
	
	for(i=0 ; i<PWAN_IF_NUMS ; i++) {
		gpWanPriv->pPonNetDev[i] = NULL ;
	}
	gpWanPriv->dropUnknownPackets = 0 ;
	gpWanPriv->dropForHookBuf = 0;
	gpWanPriv->activeChannelNum = 1; // default is 1 channel(LLID or T-CONT)
	gpWanPriv->greenMaxthreshold = 15 ;

#ifdef TCSUPPORT_WAN_GPON
	if((ret = gwan_init(&gpWanPriv->gpon)) != 0) {
		PON_MSG(MSG_ERR, "GPON WAN initialization failed\n") ;
		return ret ;
	}
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	if((ret = ewan_init(&gpWanPriv->epon)) != 0) {
		PON_MSG(MSG_ERR, "EPON WAN initialization failed\n") ;
		return ret ;
	}
#endif /* TCSUPPORT_WAN_EPON */

	pwan_create_net_interface(PWAN_IF_DATA) ;
	memset(&gpWanPriv->devCfg, 0, sizeof(PWAN_Config_T)) ;
	gpWanPriv->devCfg.flags.isQosUp = 0 ;
	
	gpWanPriv->devCfg.flags.isTxDropOmcc = 0;
	spin_lock_init(&gpWanPriv->rxLock) ;
	PON_MSG(MSG_TRACE, "PON WAN interface initialization done\n") ;
	return 0 ;
}
