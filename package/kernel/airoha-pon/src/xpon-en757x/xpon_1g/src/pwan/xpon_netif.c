#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include <linux/foe_hook.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_multicast_general.h>
#include <ecnt_hook/ecnt_hook_xpon_mapping.h>
#include <ecnt_hook/ecnt_hook_vlan.h>
#include <ecnt_hook/ecnt_hook_ether.h>
#include <ecnt_hook/ecnt_hook_bbf247.h>

#include "common/drv_global.h"
#include "pwan/xpon_netif.h"
#include "xmcs/pon-defined.h"
extern int gpon_fast_mode_flag;

/***********************************************************************************
			Variable Define
*************************************************************************************/
char channel_retire_en = XPON_DISABLE;
extern int channel_switch_enable;
extern int deallocate_flag;
extern unsigned long flash_base;

extern int masko_on_off;
extern int masko_ext;
extern int aggressive_offload_short_cut_mode;

/*************************************************************************************
			Macro Define
**************************************************************************************/
#define SPEED_TEST_SUCCESS	0
#define KERNEL_3_18_21 		(LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,19))
#define TX_STAG_LEN 6

#ifdef CONFIG_TP_IMAGE
  /* fix bug 856251, wan mark is conflic with 8021.p remark, cause pbit incorrect and packet is dropped by olt */
  #ifdef CONFIG_8021P_REMARK
  #undef CONFIG_8021P_REMARK
  #endif
#else
#define CONFIG_8021P_REMARK 1
#endif /* CONFIG_TP_IMAGE */

#ifdef CONFIG_8021P_REMARK
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN			    4
#define VLAN_ETH_ALEN			6
#endif

#define SKB_IS_MULTICAST(skb) (1 == (skb)->data[0]&0x1)
#define GDMA2_TX_CHN_VLD   0xBFB51570

/************************************************************************************
			Function API declare
**************************************************************************************/
extern int (*wan_speed_test_hook)(struct sk_buff*);
#ifdef TCSUPPORT_CPU_ARMV8
extern int get_ethaddr(unsigned char *ethaddr, int len);
#endif
extern int (*multicast_hwnat_drop_entry_hook)(struct sk_buff* skb);

#if defined(TCSUPPORT_XPON_IGMP)
extern int (*xpon_hgu_down_multicast_incoming_hook)(struct sk_buff *skb, int clone);
#if defined (TCSUPPORT_PON_SFU_HGU_HYBRID)
extern int (*xpon_hybrid_down_multicast_incoming_hook)(struct sk_buff *skb, int clone);
#endif
#endif
#ifdef TCSUPPORT_PON_VLAN
extern int (*pon_vlan_is_ds_1_to_N_hook)(struct sk_buff **pskb, int *count);
extern int (*pon_vlan_ds_1_to_N_handler_hook)(struct sk_buff **pskb, int index);
#endif
extern void macSend(__u32 chanId, struct sk_buff *skb);
extern int (*sw_upstream_nat_tx_hook) (struct sk_buff * skb, uint msg0, uint msg1, struct port_info* qdma_info);
extern int (*sw_downstream_nat_rx_hook) (struct sk_buff * skb); 
extern int gwan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, struct port_info *xpon_info);
extern int ewan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, struct port_info *xpon_info);
extern void pwan_net_open_internet_led_control(struct net_device *dev);
extern void pwan_net_del_internet_led_control(struct net_device *dev);
extern void xpon_wan_tx_internet_led_control(uint8_t netIdx);
extern void xpon_wan_rx_internet_led_control(uint8_t netIdx);
extern int xpon_wan_tx_vlan_handle_customer(struct sk_buff *skb, uint8_t netIdx);
extern int xpon_wan_rx_vlan_handle_customer(struct sk_buff *skb, uint8_t netIdx);
extern int xpon_wan_rx_l2_ppp_mcast_shortcut(struct sk_buff *skb);
extern int  xpon_wan_rx_traffic_classify(struct sk_buff *skb, uint8_t netIdx);
extern int customer_proc_create(void);
extern void customer_proc_remove(void);
/*************************************************************************************
				Function API Define
*****************************************************************************************/
void __dump_skb(struct sk_buff *skb, uint pktLen) 
{
	unchar n, *p = (unchar *)skb->data ;
	int i ;

	for(i=0 ; i<pktLen ; i++) {
		n = i & 0x0f ;
		
		if(n == 0x00) 		printk(" \n%.4x: ", i) ;
		else if(n == 0x08) 	printk(" ") ;

		printk("%.2x ", *p++) ; 
	}
	printk("\n") ;
}

/* enable CPU path traffic */
void enable_cpu_us_traffic(void)
{
	gpPonSysData->isUpDataTraffic = XPON_ENABLE;
	gpPonSysData->isUpOmciOamTraffic = XPON_ENABLE;
}

/* disable CPU path traffic */
void disable_cpu_us_traffic(void)
{
	gpPonSysData->isUpDataTraffic = XPON_DISABLE;
	gpPonSysData->isUpOmciOamTraffic = XPON_DISABLE;
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
        if (ucprio < QOS_8021P_0_MARK) { //0~7 remark
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
        skb->network_header -= VLAN_HLEN;
        skb->mac_header -= VLAN_HLEN;

    }
    return skb;
}
#endif /*CONFIG_8021P_REMARK*/

int get_pon_link_type(void)
{
    return gpPonSysData->sysLinkStatus;
}


void get_interface_mac_addr(uint8_t * buffer)
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


/*****************************************************************************
******************************************************************************/

int isHighestPriorityPacket(struct sk_buff *skb){
    u8 *cp = NULL;
    u16 etherType = 0;
    u16 pppProtocol = 0;
    u16 isHighestPPacket = 0;
    struct iphdr *iph = NULL;
    
    cp = skb->data;
    cp += 12;
    etherType = ntohs(*(u16*)cp);
    cp += 2;

    //skip vlan tag
    while((0x8100 == etherType) || (0x88a8 == etherType) || (0x9100 == etherType))
    {
        cp += 2;
        etherType = ntohs(*(u16*)cp);
        cp += 2;
    }
    
    if ((ETH_P_ARP == etherType) || (ETH_P_PPP_DISC == etherType)) {
        isHighestPPacket = 1;
    }
    else if (ETH_P_PPP_SES == etherType) {
        /* skip pppoe head */
        cp += 6;                    /* 6: PPPoE header 2: PPP protocol */
        /* get ppp protocol */
        pppProtocol = ntohs(*(u16 *) cp);
        /* check if LCP protocol */
        if (pppProtocol == 0xc021 || 
            pppProtocol == 0xc023 || 
            pppProtocol == 0xc025 || 
            pppProtocol == 0x8021 || 
            pppProtocol == 0xc223) {
            isHighestPPacket = 1;
        }
    }
    else if(ETH_P_IP == etherType){
        iph = (struct iphdr *)cp;
        if (IPPROTO_IGMP == iph->protocol) {
            isHighestPPacket = 1;
        }           
    }   

    return isHighestPPacket;
}

static int mirror_post_tx_prepare(void)
{
#if defined(WAN2LAN)
#if defined(TCSUPPORT_WAN2LAN_EXT)
	if(masko_on_off || (masko_ext & 0x01) )
#else
	if(masko_on_off)
#endif
    {
        return 1;
    }
#endif
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


	if(unlikely(!skb2)){
		printk("\nFAIL(%s): wan2lan allocation\n", __FUNCTION__);
        return NULL;
	}
#if defined(TCSUPPORT_RA_HWNAT)
	if (ra_sw_nat_hook_xfer)
		ra_sw_nat_hook_xfer(skb, skb2);
#endif	           
    
	if (PWAN_IF_OMCI == netIdx){
		skb_push(skb2, 14);
		memcpy(skb2->data, omciHdr, ARRAY_SIZE(omciHdr));
	}
	skb2->mark |= SKBUF_COPYTOLAN;
    return skb2;
}

static int isBroadcastPkt(unchar *addr)
{
	if(addr == NULL)
		return 0;

	if((addr[0] == 0xff) &&  (addr[1] == 0xff) && (addr[2] == 0xff) && (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff))
		return 1;
	else
		return 0;
}

static int isUnknownUnicastPkt(unchar *addr)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	const unchar *myAddr = gpWanPriv->pPonNetDev[PWAN_IF_DATA]->dev_addr ;
#else
	unchar *myAddr = gpWanPriv->pPonNetDev[PWAN_IF_DATA]->dev_addr ;
#endif
	if(addr == NULL)
		return 0;

	if((addr[0] == myAddr[0]) &&  (addr[1] == myAddr[1]) && (addr[2] == myAddr[2]) && (addr[3] == myAddr[3]) && (addr[4] == myAddr[4]) && (addr[5] == myAddr[5]))
		return 0;
	else
		return 1;
}

static int isMulticastPkt(unchar *addr)
{
	if(addr == NULL)
		return 0;

	if((addr[0] == 0x01) )
		return 1;
	else
		return 0;
}


void check_gdma2_tx_chn(char * arr)
{
    int i;
    __u32 Raw = READ_REG_WORD(GDMA2_TX_CHN_VLD);
    for(i=0; i<CONFIG_QDMA_CHANNEL; i++){
        if((0 != (Raw & (1 << i))) && (GPON_UNASSIGN_ALLOC_ID == gpWanPriv->gpon.allocId[i])){
            arr[i] = 1;
        }
    }
}

int pwan_channel_retire(int chn)
{
    int ret = 0;
#if defined(TCSUPPORT_CPU_EN7580)
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
#endif

#if defined(TCSUPPORT_CPU_EN7580)
	chnlCloseStatusSet.chnlIdx = chn;
	chnlCloseStatusSet.chnlStatus = 0xFF;
	QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
#endif

	ret = FE_API_SET_CHANNEL_RETIRE_ONE(FE_GDM_SEL_GDMA2, chn);

#if defined(TCSUPPORT_CPU_EN7580)
	chnlCloseStatusSet.chnlIdx = chn;
	chnlCloseStatusSet.chnlStatus = 0x0;
	QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
#endif
        return ret;
}

int pwan_channel_check(void)
{
    int i;
    char chn_arr[CONFIG_QDMA_CHANNEL] = {0};
    int ret = 0;
    
    check_gdma2_tx_chn(chn_arr);

    for(i=0; i<CONFIG_QDMA_CHANNEL; i++){
        if(1 == chn_arr[i]){
            ret = pwan_channel_retire(i);
            if(0 != ret){
                return ret;
            }
            PON_MSG(MSG_ERR, "CHANNEL %d doing channel retire",i) ;
        }
    }
    return ret;
}

int xpon_wan_restore_offload_info(struct sk_buff * skb,struct port_info *xpon_info, PWAN_FETxMsg_T *pTxMsg, int ponMode)
{
	uint8_t need_store = 0;

#ifndef TCSUPPORT_CPU_EN7580
	if(skb->macEnque&(1<<7)){ 
		skb->macEnque&= ~(1<<7);
		need_store = 1;
	}
#endif
#if defined(TCSUPPORT_CT_HWNAT_OFFLOAD)
	if((skb->qosEnque & 0x02) == 0x02){
		skb->qosEnque &= ~(0x02);
		need_store = 1;
	}
#endif

#if defined(TCSUPPORT_CT_HWNAT_OFFLOAD) || !defined(TCSUPPORT_CPU_EN7580)
	if(!need_store 
		|| (ponMode != FOE_MAGIC_GPON && ponMode != FOE_MAGIC_EPON))
	{
		return need_store;
	}
	
	if(restore_offload_info_hook)
		restore_offload_info_hook(skb, xpon_info, ponMode);
	if(ponMode == FOE_MAGIC_GPON){
		pTxMsg->raw.gem = xpon_info->stag;
	}else{
#ifndef TCSUPPORT_CPU_EN7580
		pTxMsg->raw.tse = (xpon_info->tsid) & 0x1;
		pTxMsg->raw.tsid = (xpon_info->tsid>>1) & 0x1f;
#endif
	}
	pTxMsg->raw.channel = xpon_info->channel;
#ifdef TCSUPPORT_CPU_EN7580
	pTxMsg->raw.nboq = xpon_info->channel;
#endif

#endif	
	return need_store;
}

static int is_tx_packet_drop(int netIdx)
{
	uint8_t is_drop = 0;
	switch(netIdx){
		case PWAN_IF_DATA:
			is_drop = gpPonSysData->isUpDataTraffic == XPON_DISABLE ? 1:0;
			break;
		case PWAN_IF_OAM:
			if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON){
				is_drop = gpPonSysData->isUpOmciOamTraffic == XPON_DISABLE ? 1:0;
			}
			break;
		case PWAN_IF_OMCI:
			if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON){
				is_drop = gpPonSysData->isUpOmciOamTraffic == XPON_DISABLE ? 1:0;
			}
			break;
		default:
			is_drop = 1;
			break;
	};
	return is_drop;
}

static void xpon_wan_tx_statistic_update(PWAN_NetPriv_T *pNetPriv, uint16_t gemport, uint16_t pktLens)
{
    uint16_t gemIdx  = 0;
	
	pNetPriv->stats.tx_packets++ ;
	pNetPriv->stats.tx_bytes += pktLens ;

	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) 
	{
		if (pNetPriv->netIdx == PWAN_IF_OMCI){
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK) ; 
		}else{
			if(GPON_CURR_STATE == GPON_STATE_O5){
				gemIdx = (gemport) ? (gpWanPriv->gpon.gemIdToIndex[gemport] & GPON_GEM_IDX_MASK) : GPON_GEMPORT_MAX_NUM ;
			}else{
				gemIdx  = GPON_GEMPORT_MAX_NUM;
			}
		}
		if(gemIdx < GPON_GEMPORT_MAX_NUM) {
			gpWanPriv->gpon.gemPort[gemIdx].stats.tx_packets++ ;
			gpWanPriv->gpon.gemPort[gemIdx].stats.tx_bytes += pktLens ;
		}
	}

}

int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int ret = NETDEV_TX_OK ;
    PWAN_NetPriv_T *pNetPriv ;
    PWAN_FETxMsg_T txBmMsg;
    struct sk_buff * unshared_skb = NULL;
    struct port_info xpon_info= {0}; /* only used in gwan_prepare_tx_message() */
    struct sk_buff * skb_mirror = NULL;
	uint16_t gemport = 0;
	uint16_t pktLens=0;

    if(skb == NULL || dev == NULL)
    {
        printk("ERR %s skb or dev is NULL! \n",__FUNCTION__);
        return -1;
    }
    
    PON_MSG(MSG_TRACE, "Start to transmit a packets.\n") ;

    memset(&txBmMsg, 0, sizeof(PWAN_FETxMsg_T));
    
    if(isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552) {
        /* in EN7580, tsid=0x7F means no sharping, tsid=0~126 measn sharping */
        xpon_info.tsid = HWNAT_TSID_DEFAULT;
    }
    pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
    skb->dev = dev ; /* Use for tx DSCP recycle. */

    unshared_skb = skb_unshare(skb, GFP_ATOMIC);
    skb = unshared_skb;
    if(NULL == skb)
    {
        printk("ERR %s skb_unshare failed! \n",__FUNCTION__);
        goto tx_drop;
    }
	
    if(SOFT_LOOPBACK_MODE(pNetPriv)) {
        skb->protocol = eth_type_trans(skb, dev) ;
        skb->ip_summed = CHECKSUM_UNNECESSARY ;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
        dev->last_rx = jiffies ; 
#endif
        netif_rx(skb) ;
        goto post_qdma_tx;
        
    }
    
    if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_OFF) {
		XPON_DROP_PRINT;
        goto tx_drop;
    } 

	ret = xpon_wan_tx_vlan_handle_customer(skb, pNetPriv->netIdx);
	if(ret == ECNT_RETURN){
		ret = NETDEV_TX_OK;
		goto post_qdma_tx;
	}else if(ret == ECNT_HOOK_ERROR){
#ifdef  CONFIG_8021P_REMARK
		/*AIROHA Pbit Remark*/
	    skb=vlanPriRemark(skb);
#endif
	}else{
		PON_MSG(MSG_WARN, "The VLAN Operation is Customer.\n") ;
	}
    if(skb==NULL){
        printk("(%s)802.1p remark failure\r\n", __FUNCTION__);
        return NETDEV_TX_OK;
    }

    if((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && (GPON_CURR_STATE == GPON_STATE_O5)) {
        ret = gwan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, &xpon_info);
		gemport=skb->gem_port;
    } else if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
        ret = ewan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, &xpon_info);
    } else {
        PON_MSG(MSG_ERR, "The xPON link status is off.\n") ;
        ret = -1;
    }
	if(ret == -1){
		XPON_DROP_PRINT;
		goto tx_drop;
	}

	if(is_tx_packet_drop(pNetPriv->netIdx)){
		XPON_DROP_PRINT;
		goto tx_drop;
    }

    if(gpPonSysData->debugLevel & MSG_CONTENT) {
        PON_MSG(MSG_CONTENT, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, txBmMsg.word[0], txBmMsg.word[1]) ;
        __dump_skb(skb, skb->len) ;
    }
    
    
	if(pNetPriv->netIdx == PWAN_IF_DATA){
	/* padding for the packet that length less than 64 byte */
		if(unlikely(skb->len < ETH_ZLEN)) {
			if(skb_padto(skb, ETH_ZLEN)) {
				skb = NULL;	
				XPON_DROP_PRINT;
				goto tx_drop;
			}
			skb_put(skb, ETH_ZLEN-skb->len) ;
		}
	}
	
	pktLens= skb->len ;
    if( mirror_post_tx_prepare() ){
        skb_mirror = copy_pwan_skb(skb, pNetPriv->netIdx);
    }
	
	if (gpon_fast_mode_flag) {
		xpon_info.fast = 1;
	} else {
		xpon_info.fast = 0;
	}


#if defined(TCSUPPORT_SWNAT)
    if(sw_upstream_nat_tx_hook){
        sw_upstream_nat_tx_hook(skb, txBmMsg.word[0], txBmMsg.word[1], &xpon_info);
    }
#endif
    
    /* call QDMA API to transmit the packet */
    ret = QDMA_API_TRANSMIT_PACKETS(ECNT_QDMA_WAN, skb, txBmMsg.word[0], txBmMsg.word[1], &xpon_info) ;

/******************************************************************************************************************* 
		   			!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					skb should not be used blow this line !!!
		   			!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*******************************************************************************************************************/

    if(ret != 0)  /* when drop keepalive packet,ret = 1 */
    {
        if(ret < 0) {
			if((1 == channel_switch_enable) && (1 == deallocate_flag)){
				gwan_switch_channel_id();
			}
            if((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && (XPON_ENABLE == channel_retire_en)) {
                pwan_channel_check();
            }
			PON_MSG(MSG_ERR, "Transmit packet to QDMA failed.\n") ;
        }
        XPON_DROP_PRINT;
		goto tx_drop;  
    }
    else /* update packet conuter and length */
    { 
        xpon_wan_tx_statistic_update(pNetPriv, gemport, pktLens);
    }

post_qdma_tx:
    
    if(mirror_post_tx_prepare() && skb_mirror){
        macSend(WAN2LAN_CH_ID, skb_mirror);
		skb_mirror = NULL ;
    }
	xpon_wan_tx_internet_led_control(pNetPriv->netIdx);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
	if(dev != NULL){
    	dev->trans_start = jiffies ;
	}
#endif
    return ret ;
    
            
tx_drop:
    pNetPriv->stats.tx_dropped++ ;
    if( skb )
        dev_kfree_skb_any(skb) ;
    if(skb_mirror) 
		dev_kfree_skb_any(skb_mirror);
    return ret ;
}


/*****************************************************************************
******************************************************************************/
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

int gpon_ds_transmit_packet(struct sk_buff* skb)
{
#ifdef TCSUPPORT_BBF_247
/* 2476.2.19  6.2.21ҪmappingUNI */
    int uni = -1;
    char name[16];
    struct net_device *dev_p = NULL;
    __u8 *mac = NULL;

	if(NULL == skb){
		return -1;
	}
	
    skb_push(skb,ETH_HLEN);
    mac = skb->data;
    if((gpPonSysData->sysLinkStatus != PON_LINK_STATUS_GPON) || (SKB_IS_MULTICAST(skb)) || !isBroadcastPkt(mac))
    {
    	goto Fail;
    }
        
    uni = XPON_MAPPING_GET_DS_UNI_ENCT_HOOK(skb);
    if(uni != -1)
    {
    	uni =  uni + 1;
        sprintf(name, "eth0.%d", uni);
	    dev_p=dev_get_by_name(&init_net,name);
        if(dev_p != NULL)
        {
            skb->original_dev = skb->dev;
			skb->dev = dev_p;
    		skb->pon_vlan_flag |= PON_PKT_FROM_WAN;
		    dev_queue_xmit(skb);    
            PON_MSG(MSG_ERR,"%s mapping success packet send to %s\n",__FUNCTION__,name);
            return 0;
        }  
    }   
    
Fail:
    skb_pull(skb,ETH_HLEN);   
#endif
	return -1;
}

static int check_and_do_1toN_vlan_opreation(struct sk_buff *skb,char netIdx)
{
	char dst[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	int vlanRuleCnt=0, i=0;
	struct sk_buff* skb2=NULL;
	int ret = 0;

	if(NULL == skb){
		return ret;
	}
	/*	 HGU downstream broadcast 1 to N vlan hander*/
	if(pon_vlan_is_ds_1_to_N_hook && pon_vlan_ds_1_to_N_handler_hook  
		&& !memcmp(skb_mac_header(skb),dst,6) && PWAN_IF_DATA == netIdx)
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
}

static struct net_device * xpon_wan_rx_get_dev_info(struct sk_buff *skb, int netIdx)
{
	struct net_device *dev = NULL ;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
	uint8_t wan_if = 0;	

	if(NULL == skb){
		return dev;
	}
	wan_if = skb->ecnt_sk_buff.wan_if;

	if ( HYBIRD_PWAN_IF_GPON0 == wan_if )
	{
		dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_GPON0] ;
		PON_MSG(MSG_TRACE, "[%s][%d] :device name = %s	.\n",__FUNCTION__,__LINE__,dev->name);
	}
	else if ( HYBIRD_PWAN_IF_WAN0 == wan_if )
	{
		dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_WAN0] ;
		PON_MSG(MSG_TRACE, "[%s][%d] :device name = %s	.\n",__FUNCTION__,__LINE__,dev->name);
	}
	else
#endif/*TCSUPPORT_COMPILE*/
	{
		dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)netIdx] ;
		PON_MSG(MSG_TRACE, "[%s][%d] :device name = %s	.\n",__FUNCTION__,__LINE__,dev->name);
	}

	return dev;
}

static void xpon_wan_rx_2_lan(struct sk_buff *skb, int netIdx)
{
#if defined(WAN2LAN)
	if(NULL == skb){
		return;
	}
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
			}else if (netIdx == PWAN_IF_OMCI){
				skb_push(skb2, 14);
				memcpy(skb2->data, omciHdr, 14);
			}
			skb2->mark |= SKBUF_COPYTOLAN;
			macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx
		}
	}

#endif		
}

static void xpon_wan_rx_statistic_update(PWAN_NetPriv_T *pNetPriv, int pktLen)
{
	if(pNetPriv == NULL){
		return;
	}
	pNetPriv->stats.rx_packets++ ;
	pNetPriv->stats.rx_bytes += pktLen ;
}

static int xpon_wan_rx_igmp_handle(struct sk_buff *skb, int netIdx)
{
	int ret = 0;
	char * macDa = NULL;
	//hybridmode xpon_igmp downstream multicast operation 
	if(NULL == skb || PWAN_IF_DATA != netIdx){
		return ret;
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CF_UNIFIED_PLATFORM)  || defined(TCSUPPORT_FH_DP)
	if((PWAN_IF_DATA == netIdx) && (ECNT_RETURN_DROP == ECNT_ETH_SEND_HOOK(&skb)))
	{
		return -1;
	}
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_PON_SFU_HGU_HYBRID)
	if(xpon_hybrid_down_multicast_incoming_hook  && PWAN_IF_DATA == netIdx)
	{
		if(xpon_hybrid_down_multicast_incoming_hook(skb, 1) <= 0){
			 PON_MSG(MSG_ERR, "xpon_hybrid_down_multicast_incoming_hook fail, drop pkt\n");
			 goto hwnat_drop;
		}
	}
#endif		
#if defined(TCSUPPORT_XPON_HAL_API_MCST)
	if(mtk_multicast_data_handle_hook && PWAN_IF_DATA == netIdx)
	{
		if(0 == mtk_multicast_data_handle_hook(skb))
		{
			ret = 1;
			goto free_skb;
		}
	}
#endif

	if(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb))
	{
		ret = 1;
		goto free_skb;
	}


	macDa =  skb_mac_header(skb);
#if defined(TCSUPPORT_XPON_IGMP)
	if((0x01 == (0x01 & macDa[0]) ) && xpon_hgu_down_multicast_incoming_hook)
	{
		 if (xpon_hgu_down_multicast_incoming_hook(skb, 1) > 0 )
		 {
			 ret = -1;  
		 }
	}
#endif

	return ret;

#if defined(TCSUPPORT_PON_SFU_HGU_HYBRID)
hwnat_drop:
/*WARNING: mutlicast hwnat drop rule must be add by mtk_xpon_multicast
	otherwise cant't be deleted by mtk_multicast while joining*/
	if(multicast_hwnat_drop_entry_hook && mtk_xpon_is_multicast_addr(skb))
	{
		multicast_hwnat_drop_entry_hook(skb);
	}
	return -1;
#endif

free_skb:
	dev_kfree_skb(skb);
	skb = NULL;

	return ret;

}

static int xpon_wan_rx_left_to_right_handle(struct sk_buff *skb, PWAN_FERxMsg_T *pRxBmMsg)
{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FORWARD_LEFT_TO_RIGHT)
	if(NULL == skb){
		return -1;
	}
	if(aggressive_offload_short_cut_mode && isEN751221 && 
		(pRxBmMsg->raw.crsn == 0x16 || pRxBmMsg->raw.sport == 5))
	{
		skb_put(skb, pktLen) ;
		skb->protocol = eth_type_trans(skb, gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_DATA]) ;
		if(ra_sw_nat_hook_rxinfo) {
			ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_GPON, (char *)&(pRxBmMsg->word[1]), 4);
		}
		
		if(ra_sw_nat_hook_rx) {
			if(0 == ra_sw_nat_hook_rx(skb)) 
				return 1;
		}
		return -1;
	}
#endif/*TCSUPPORT_COMPILE*/   
	return 0;
}
int pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen)
{
	int netIdx=-1 ;
	PWAN_NetPriv_T *pNetPriv = NULL ;
	PWAN_FERxMsg_T *pRxBmMsg_tmp = (PWAN_FERxMsg_T *)pMsg ; 
	struct net_device *dev = NULL ;
	unchar LbFlag = 0 ;
	int ret = 0;
	int igmpFlag = 0;
	PWAN_FERxMsg_T pRxBmMsg_s;
	PWAN_FERxMsg_T *pRxBmMsg = &pRxBmMsg_s;
    int ppeMagic = 0 ;
    
    unsigned int     value = 0;
    unsigned int    ifc_id = 0;
    unsigned char  ifc_hit = 0;
    
	memcpy(pRxBmMsg, pRxBmMsg_tmp, sizeof(PWAN_FERxMsg_T));

	if(NULL == skb){
		return -1;
	}
	
	ret = xpon_wan_rx_left_to_right_handle(skb, pRxBmMsg);
	if(ret == 1){
		goto success_ret;
	}else if(ret == -1){
		XPON_DROP_PRINT;
		goto rx_drop;
	}else{
	}

	if( pRxBmMsg->raw.oam ){
		skb = replace_oam_pkt(skb, pktLen);
	}
	if(gpPonSysData->debugLevel & MSG_CONTENT) {
		PON_MSG(MSG_CONTENT, "RX PKT LEN: %d, Rx Msg: %.8x, %.8x", pktLen, pRxBmMsg->word[0], pRxBmMsg->word[1]) ;
		__dump_skb(skb, pktLen) ;
	}

#ifdef TCSUPPORT_LAN_VLAN
    skb->lan_vlan_tci = 0;
    skb->lan_vlan_tci_valid = 0;
#endif
	/* 1. process the message information                          *
	 * 2. calculate the netIdx value from GEM port or LLID value   */
	/* parer the packet to corresponding interface */
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		netIdx = gwan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag);
		ppeMagic = FOE_MAGIC_GPON ;
	} else if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		netIdx = ewan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag);
		ppeMagic = FOE_MAGIC_EPON ;
	} else {
		XPON_DROP_PRINT;
		goto rx_drop ;
	}
	PON_MSG(MSG_TRACE, "pwan_cb_rx_packet: netIdx = %d line = %d\n", netIdx, __LINE__);
	/* handle broadcast /unknown unicast /multicast pkt */
	if (isBroadcastPkt(skb->data)) 
		ret = BROADCAST_OFFSET;
	else if (isMulticastPkt(skb->data))
		ret = MULTICAST_OFFSET;
	else if(isUnknownUnicastPkt(skb->data))
		ret = UNKNOWN_UNICAST_OFFSET;	
	else 
		ret = 0;
	if(netIdx < 0)
	{
		XPON_DROP_PRINT;
		ret = -ret;
		goto rx_drop;
	}

	if(LbFlag) {
		skb_put(skb, pktLen) ;
		pwan_net_start_xmit(skb, dev) ;
	} else {	
		/* process the packet data to high layer */
		skb_put(skb, pktLen) ;	

		xpon_wan_rx_2_lan(skb, netIdx);
		xpon_wan_rx_internet_led_control(netIdx);

		if(xpon_wan_rx_l2_ppp_mcast_shortcut(skb)){
			ret = 0;
			goto success_ret;
		}

		dev = xpon_wan_rx_get_dev_info(skb, netIdx);
        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		
		skb->dev = dev ;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
		dev->last_rx = jiffies ;
#endif
		skb->protocol = eth_type_trans(skb, dev) ;
		if ((pRxBmMsg->raw.ipv4 || pRxBmMsg->raw.ipv6) && (pRxBmMsg->raw.l4vld) && 
			(!pRxBmMsg->raw.ipv4f && !pRxBmMsg->raw.l4f)){
		skb->ip_summed = CHECKSUM_UNNECESSARY ;
		}else{
			skb->ip_summed = CHECKSUM_NONE;
		}
		if (PWAN_IF_OMCI == netIdx ){
			skb->protocol = 0;
		}

		
		if(wan_speed_test_hook)
		{
			if(wan_speed_test_hook(skb)==SPEED_TEST_SUCCESS){
				ret = SPEED_TEST_SUCCESS;
				goto success_ret;
			}
		}

		if(ra_sw_nat_hook_rxinfo && ppeMagic) {
			ra_sw_nat_hook_rxinfo(skb, ppeMagic, (char *)&(pRxBmMsg->word[1]), 4);
		}
        
	    if(ra_sw_nat_hook_ifc_hit_info)
        {
            QDMA_API_GET_RXMSG(ECNT_QDMA_WAN, pMsg, RXMSG_FIELD_IFC_HIT, &value);
			ifc_hit = value ? 1 : 0;
            if(ifc_hit)
            {
                QDMA_API_GET_RXMSG(ECNT_QDMA_WAN, pMsg, RXMSG_FIELD_IFC_ID,  &ifc_id);
                ra_sw_nat_hook_ifc_hit_info(skb, ifc_hit, ifc_id);
            }
        }
        
		if(ra_sw_nat_hook_rx != NULL) {
			if(ra_sw_nat_hook_rx(skb)==0){
				ret = 0;
				goto success_ret;
			}
		}

		if( -1 == xpon_wan_rx_traffic_classify(skb, netIdx) ){
			XPON_DROP_PRINT;
			goto rx_drop;
		}

		if( -1 == xpon_wan_rx_vlan_handle_customer(skb, netIdx) ){
			XPON_DROP_PRINT;
			goto rx_drop;
		}

		
#if defined(TCSUPPORT_SWNAT)
		if(sw_downstream_nat_rx_hook){
			if(sw_downstream_nat_rx_hook(skb)){
				return 0;
			}
		}
#endif

		igmpFlag = xpon_wan_rx_igmp_handle(skb, netIdx);
		if(igmpFlag == -1){
			XPON_DROP_PRINT;
			goto rx_drop;
		}else if(igmpFlag == 1){
			goto success_ret;			
		}
		/*
		if(PWAN_IF_DATA == netIdx
			&& -1 != gpon_ds_transmit_packet(skb)){
			goto success_ret;
		}*/
		if(PWAN_IF_DATA == netIdx && ECNT_CONTINUE == ECNT_API_BBF247_PKT_DS_HANDLE(skb)){
			goto success_ret;
		}
		
#ifdef TCSUPPORT_TLS
		if(-1 != FE_API_TLS_FORWARD(skb,FE_TLS_FORWARD_DOWNSTREAM)){
			goto success_ret;
		}
#endif
		if(check_and_do_1toN_vlan_opreation(skb,netIdx)  != 0)
		{
			goto success_ret;
		}

		netif_receive_skb(skb) ;
    }
	
success_ret:
	xpon_wan_rx_statistic_update(pNetPriv, pktLen);
	
	return ret ;

rx_drop:
	/* Dropped the packet */
	dev_kfree_skb(skb);
	if(pNetPriv != NULL){
		pNetPriv->stats.rx_dropped++ ;
	}else{
		/* calculate the statistic counters */
		gpWanPriv->dropUnknownPackets++ ;
	}
	return -EFAULT+ret ;
}

/*****************************************************************************
******************************************************************************
 network device operation function 
******************************************************************************
******************************************************************************/

static inline struct sk_buff * pwan_dev_alloc_skb(void)
{
    if (PWAN_MAX_MTU <= SKBMGR_RX_BUF_LEN){
        return skbmgr_dev_alloc_skb2k() ;
    }else if (PWAN_MAX_MTU <= SKBMGR_4K_RX_BUF_LEN){
        return skbmgr_dev_alloc_skb4k() ;
    }else{
        return dev_alloc_skb(PWAN_MAX_MTU) ;
    }
}

/*****************************************************************************
******************************************************************************/
static int pwan_net_open(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_enable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	
	netif_start_queue(dev) ;

	pwan_net_open_internet_led_control(dev);

	return 0 /* -ENODEV */ ;
}

/*****************************************************************************
******************************************************************************/
static int pwan_net_stop(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;

	pwan_net_del_internet_led_control(dev);
	
	netif_stop_queue(dev) ;

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{	
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_disable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */

	return 0 ;
}


/*****************************************************************
*****************************************************************/
static struct net_device_stats *pwan_net_get_stats(struct net_device *dev)
{
	PWAN_NetPriv_T *pNetPriv ;

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
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len); 
#endif
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
	ulong flags ;
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
		napi_complete(napi);
#else
		__napi_complete(napi);
#endif
#else
		__netif_rx_complete(dev) ;
#endif /* KERNEL_VERSION(2,6,26) */

		QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;

		spin_unlock_irqrestore(&gpWanPriv->rxLock, flags) ;
	} else {
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
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
	.ndo_start_xmit			= hybird_pwan_net_start_xmit,
#else/*TCSUPPORT_COMPILE*/
	.ndo_start_xmit			= pwan_net_start_xmit,
#endif/*TCSUPPORT_COMPILE*/
	.ndo_get_stats 			= pwan_net_get_stats,
#if !KERNEL_3_18_21
	.ndo_set_multicast_list 	= pwan_net_set_multicast_list,
#endif
	.ndo_do_ioctl          	 	= pwan_net_ioctl,
	.ndo_set_mac_address		= pwan_net_set_macaddr,
};
#endif /* KERNEL_VERSION(2,6,29) */

/*****************************************************************
*****************************************************************/
int pwan_create_net_interface(uint idx)
{
	unchar mac_addr[ETH_ADDR_LEN] = PWAN_BASE_MAC_ADDR ;
    PWAN_NetPriv_T *pNetPriv ;
	unchar if_name[16] ;
#ifndef TCSUPPORT_CPU_ARMV8
	int i;
#endif

	if(idx >= PWAN_IF_NUMS) {
		return -EINVAL ;
	}

	if(gpWanPriv->pPonNetDev[idx]) {
		return -EEXIST ;
	}

	get_interface_mac_addr(mac_addr);

	memset(if_name, 0, 16) ;
	if(idx == PWAN_IF_OAM) {
		sprintf(if_name, "oam") ;
	} else if(idx == PWAN_IF_OMCI) {
		sprintf(if_name, "omci") ;
	} else
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
    if (idx == PWAN_IF_GPON0)
    {
        sprintf(if_name, "gpon0") ;
    }else if (idx == PWAN_IF_WAN0)
    {
        sprintf(if_name, "wan0") ;
    }else
#endif/*TCSUPPORT_COMPILE*/	
    {
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		sprintf(if_name, CONST_XMCI_DEV_NAME) ;	
#else/*TCSUPPORT_COMPILE*/
		sprintf(if_name, "pon") ;
#endif/*TCSUPPORT_COMPILE*/		
	} 
#if KERNEL_3_18_21
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
	memcpy(gpWanPriv->pPonNetDev[idx]->dev_addr, mac_addr, ETH_ADDR_LEN) ;
#endif
	gpWanPriv->pPonNetDev[idx]->mtu = PWAN_MAX_MTU ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	gpWanPriv->pPonNetDev[idx]->netdev_ops = &xpon_netdev_ops;
#else
	gpWanPriv->pPonNetDev[idx]->open = pwan_net_open ;
	gpWanPriv->pPonNetDev[idx]->stop = pwan_net_stop ;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
	gpWanPriv->pPonNetDev[idx]->hard_start_xmit = hybird_pwan_net_start_xmit ;
#else/*TCSUPPORT_COMPILE*/
	gpWanPriv->pPonNetDev[idx]->hard_start_xmit = pwan_net_start_xmit ;
#endif/*TCSUPPORT_COMPILE*/
	gpWanPriv->pPonNetDev[idx]->get_stats = pwan_net_get_stats ;
	gpWanPriv->pPonNetDev[idx]->set_multicast_list = pwan_net_set_multicast_list ;
	gpWanPriv->pPonNetDev[idx]->do_ioctl = pwan_net_ioctl ;
	gpWanPriv->pPonNetDev[idx]->set_mac_address = pwan_net_set_macaddr;
#endif /* KERNEL_VERSION(2,6,29) */

#ifdef CONFIG_USE_RX_NAPI
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pNetPriv->napi.weight = 128 ;
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	netif_napi_add_weight(gpWanPriv->pPonNetDev[idx], &pNetPriv->napi, pwan_net_poll, 128) ;
#else
	netif_napi_add(gpWanPriv->pPonNetDev[idx], &pNetPriv->napi, pwan_net_poll, 128) ;
#endif
#else
	gpWanPriv->pPonNetDev[idx]->poll = pwan_net_poll ;
	gpWanPriv->pPonNetDev[idx]->weight = 128 ;
#endif /* KERNEL_VERSION(2,6,26) */
#endif /* CONFIG_USE_RX_NAPI */

	if(idx==PWAN_IF_OAM || idx==PWAN_IF_OMCI) {
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_NOARP ;
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_BROADCAST ;
		gpWanPriv->pPonNetDev[idx]->flags &= ~IFF_MULTICAST ;
#if !KERNEL_3_18_21
		gpWanPriv->pPonNetDev[idx]->features |= NETIF_F_NO_CSUM ;
#endif
	}

	if(register_netdev(gpWanPriv->pPonNetDev[idx]) != 0) {
		PON_MSG(MSG_ERR, "Register net device %s failed\n", if_name) ;
		free_netdev(gpWanPriv->pPonNetDev[idx]) ;
		gpWanPriv->pPonNetDev[idx] = NULL ;
		return -EFAULT ;
	}
	
	return 0 ;
}

static int pwan_net_del(struct net_device *dev)
{

	pwan_net_del_internet_led_control(dev);
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
void pwan_destroy(void)
{
	int i ;
	
	for(i=0 ; i<PWAN_IF_NUMS ; i++) {
		pwan_delete_net_interface(i) ;
    }
    customer_proc_remove();
#ifdef TCSUPPORT_WAN_GPON
    remove_proc_entry("tc3162/service_change", 0);
#endif
}

/*****************************************************************************
******************************************************************************/
int pwan_init(void)
{
	int i, ret ;
	
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

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
    if((ret = pwan_hybird_init( )) != 0) {
            PON_MSG(MSG_ERR, "PWAN hybird initialization failed\n") ;
            return ret ;
    }
#endif/*TCSUPPORT_COMPILE*/

	pwan_create_net_interface(PWAN_IF_DATA) ;

	memset(&gpWanPriv->devCfg, 0, sizeof(PWAN_Config_T)) ;
	
	spin_lock_init(&gpWanPriv->rxLock) ;

	customer_proc_create();

    PON_MSG(MSG_TRACE, "PON WAN interface initialization done\n") ;
	return 0 ;
}

