/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2016, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hybird_wan.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wei.sun		2016/10/17		Create
*/
    
#if defined(TCSUPPORT_XPON_HYBIRD)

#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>

#include "common/drv_global.h"
#include "pwan/hybird_wan.h"
#include "pwan/xpon_netif.h"
#include "common/xpondrv.h"

#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include <ecnt_hook/ecnt_hook_smux.h>
#include <ecnt_hook/ecnt_hook_net.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif

#ifdef TCSUPPORT_WAN_GPON
PWAN_HybirdPriv_T  pwan_hybridData;

#define ROUTE_MAC_USAGE_PRINT_WITH_RET(__HYBIRD_IN__) do {\
    int i = 0;\
    printk("\nUSAGE :\n");\
    for (i= 0; i< HYBIRD_ROUTE_MAC_MAX;i++)\
        printk("route mac%d XXXX:XXXX:XXXX\n",i);\
    printk("\nOr my mac [0-invalid,1-valid] XXXX:XXXX:XXXX\n");\
    printk("\nOr disbale route mac[0~%d]\n\n",(HYBIRD_ROUTE_MAC_MAX-1));\
    return __HYBIRD_IN__;\
} while(0)

#define HYBRID_PROC_MAX_STRING 36
#define HYBRID_MY_MAC_MAX      (7)

/*interface form NSB*/
static char * get_my_mac( void )
{
    static unsigned char macda[] = {0x0,0x1,0x2,0x3,0x4,0x5};
    return macda;
}

/*****************************************************************************
******************************************************************************/
static int c2i(char ch)
{
    if(ch >= '0' && ch <= '9')
        return (ch-48);
    if(ch >= 'A' && ch <= 'F') 
        return (ch -55);
    if(ch >= 'a' && ch <= 'f')
        return (ch -87);
    
    return -1;
}

static int hex2dec(char *hex)
{
    int len = 0;
    int num = 0;
    int temp = 0;
    int bits = 0;
    int i = 0;

    len = strlen(hex);

    for (i = 0,temp = 0; i < len; i++,temp = 0)
    {
        temp = c2i(*(hex + i));
        if (-1 == temp)
            return -1;
        bits = (len -i -1)*4;
        temp = temp << bits;
        num += temp;
    }
    return num;
}

static int is_route_mode(char * mac)
{
    unsigned char p_macda[ETH_ADDR_LEN] = {0};
    int i = 0;
    int j = 0;
    char match_flag = 1;
    int ret = 0;

    memcpy(p_macda,mac,ETH_ADDR_LEN);

    spin_lock(&pwan_hybridData.macLock);
    PON_MSG(MSG_TRACE, "[%s][%d] :mac da = %02x%02x:%02x%02x:%02x%02x \n",__FUNCTION__,__LINE__,
            p_macda[0],p_macda[1],
            p_macda[2],p_macda[3],
            p_macda[4],p_macda[5]);
    for (i = 0; i < HYBIRD_ROUTE_MAC_MAX; i++ )
    {
        PON_MSG(MSG_TRACE, "[%s][%d] :route mac %d valid = %d  %02x%02x:%02x%02x:%02x%02x memcmp = %d ETH_ADDR_LEN = %d\n",__FUNCTION__,__LINE__,i,
            pwan_hybridData.routeMac[i].valid,
            pwan_hybridData.routeMac[i].macAddr[0],pwan_hybridData.routeMac[i].macAddr[1],
            pwan_hybridData.routeMac[i].macAddr[2],pwan_hybridData.routeMac[i].macAddr[3],
            pwan_hybridData.routeMac[i].macAddr[4],pwan_hybridData.routeMac[i].macAddr[5],
            memcmp(p_macda,pwan_hybridData.myMac.macAddr,ETH_ADDR_LEN),ETH_ADDR_LEN);
        if ((1 == pwan_hybridData.routeMac[i].valid) &&
            (0 == memcmp(p_macda,pwan_hybridData.routeMac[i].macAddr,ETH_ADDR_LEN))){
            PON_MSG(MSG_TRACE, "[%s][%d]  \n",__FUNCTION__,__LINE__);
            ret = 1;
            goto IS_ROUTE_MAC_RETURN;
        }   
    }
    
    PON_MSG(MSG_TRACE,"[%s][%d] :mymac = %02x%02x:%02x%02x:%02x%02x \n  \n",__FUNCTION__,__LINE__,
        pwan_hybridData.myMac.macAddr[0],pwan_hybridData.myMac.macAddr[1],pwan_hybridData.myMac.macAddr[2],
        pwan_hybridData.myMac.macAddr[3],pwan_hybridData.myMac.macAddr[4],pwan_hybridData.myMac.macAddr[5]);

    if(0 == pwan_hybridData.myMac.valid || (0 != memcmp(p_macda,pwan_hybridData.myMac.macAddr,ETH_ADDR_LEN-1))) {
        ret = 0;
        PON_MSG(MSG_TRACE, "[%s][%d]  \n",__FUNCTION__,__LINE__);
        goto IS_ROUTE_MAC_RETURN;
    }
            
    PON_MSG(MSG_TRACE, "[%s][%d] :DA=0x%02x MY_MAC=0x%02x \n",__FUNCTION__,__LINE__,
            p_macda[ETH_ADDR_LEN - 1],pwan_hybridData.myMac.macAddr[ETH_ADDR_LEN - 1]);
    if ((p_macda[ETH_ADDR_LEN - 1] >=  pwan_hybridData.myMac.macAddr[ETH_ADDR_LEN - 1]) && 
        (p_macda[ETH_ADDR_LEN - 1] <=  (pwan_hybridData.myMac.macAddr[ETH_ADDR_LEN - 1] + HYBRID_MY_MAC_MAX)))
        {
                PON_MSG(MSG_TRACE, "[%s][%d] :mac da route mode \n",__FUNCTION__,__LINE__);
                ret = 1;
                goto IS_ROUTE_MAC_RETURN;
        }

    PON_MSG(MSG_TRACE, "[%s][%d] :mac da bridge mode \n",__FUNCTION__,__LINE__);
    ret = 0;

IS_ROUTE_MAC_RETURN:
    spin_unlock(&pwan_hybridData.macLock);
    return ret;
}

static int route_mac_read_proc(char *page, char **start, off_t off,
    int count, int *eof, void *data)
{
    int len = 0;
    int i = 0;

    len = sprintf(page, "\n");
	len += sprintf(page+len, "%s\n", "**************************************");
	len += sprintf(page+len, "%s\n", "**        HYBIRD ROUTE MAC          **");
	len += sprintf(page+len, "%s\n", "**************************************");
	len += sprintf(page+len, "\n");

    for (i = 0; i < HYBIRD_ROUTE_MAC_MAX; i++)
    {
        len += sprintf(page+len, "route mac%d valid=%d %02x%02x:%02x%02x:%02x%02x \n",
            i,pwan_hybridData.routeMac[i].valid,
            pwan_hybridData.routeMac[i].macAddr[0],pwan_hybridData.routeMac[i].macAddr[1],
            pwan_hybridData.routeMac[i].macAddr[2],pwan_hybridData.routeMac[i].macAddr[3],
            pwan_hybridData.routeMac[i].macAddr[4],pwan_hybridData.routeMac[i].macAddr[5]);
    }

    len += sprintf(page+len, "my mac %s %02x%02x:%02x%02x:%02x%02x \n",
            (1 == pwan_hybridData.myMac.valid) ? "valid":"invalid",
            pwan_hybridData.myMac.macAddr[0],pwan_hybridData.myMac.macAddr[1],
            pwan_hybridData.myMac.macAddr[2],pwan_hybridData.myMac.macAddr[3],
            pwan_hybridData.myMac.macAddr[4],pwan_hybridData.myMac.macAddr[5]);
    
    len += sprintf(page+len, "\n");

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

static int route_mac_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[HYBRID_PROC_MAX_STRING]={0};
    uint val=0;
    int tmp = 0;
    uint i=0;
    char idx[HYBRID_PROC_MAX_STRING] = {0};
    unsigned char mac[ETH_ADDR_LEN][HYBRID_PROC_MAX_STRING] = {0};
    HYBIRD_Route_MAC_t  route_mac_t;
	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
    
	val_string[count] = '\0';
    if (13 == sscanf(val_string, "route mac%s %c%c%c%c:%c%c%c%c:%c%c%c%c",&idx, \
					&mac[0][0], &mac[0][1],
					&mac[1][0], &mac[1][1],
					&mac[2][0], &mac[2][1],
					&mac[3][0], &mac[3][1],
					&mac[4][0], &mac[4][1],
					&mac[5][0], &mac[5][1]))
    {
        val = atoi(idx);
        if( val <0 || val >= HYBIRD_ROUTE_MAC_MAX )
            ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
        
        for (i = 0; i < ETH_ADDR_LEN; i++){
            //sscanf(mac[i],"%2x",&(pwan_hybridData.route_mac[val].macAddr[i]));
            tmp = hex2dec(mac[i]);
            if (-1 == tmp)
                ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
            
            route_mac_t.macAddr[i] = tmp & 0xFF;
        }
        route_mac_t.valid = 1;
        memcpy(&(pwan_hybridData.routeMac[val]),&route_mac_t,sizeof(HYBIRD_Route_MAC_t));
        printk("Set route mac%d to %02x%02x:%02x%02x:%02x%02x\n",val,
            pwan_hybridData.routeMac[val].macAddr[0],pwan_hybridData.routeMac[val].macAddr[1],
            pwan_hybridData.routeMac[val].macAddr[2],pwan_hybridData.routeMac[val].macAddr[3],
            pwan_hybridData.routeMac[val].macAddr[4],pwan_hybridData.routeMac[val].macAddr[5]);
	    return count;	
    }else if (12 == sscanf(val_string, "my mac %s %c%c%c%c:%c%c%c%c:%c%c%c%c", &idx,\
					&mac[0][0], &mac[0][1],
					&mac[1][0], &mac[1][1],
					&mac[2][0], &mac[2][1],
					&mac[3][0], &mac[3][1],
					&mac[4][0], &mac[4][1],
					&mac[5][0], &mac[5][1]))
    {
        unsigned char macAddr[ETH_ADDR_LEN];
        val = atoi(idx);
        if( val <0 || val >= 2 )
            ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
        
        for (i = 0; i < ETH_ADDR_LEN; i++){
            //sscanf(mac[i],"%2x",&(pwan_hybridData.route_mac[val].macAddr[i]));
            tmp = hex2dec(mac[i]);
            if (-1 == tmp)
                ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
            
            macAddr[i] = tmp & 0xFF;
        }

        pwan_hybridData.myMac.valid = 1;
        memcpy(pwan_hybridData.myMac.macAddr,macAddr,sizeof(macAddr));
        printk("Set my mac valid is %d to %02x%02x:%02x%02x:%02x%02x\n",pwan_hybridData.myMac.valid,
            pwan_hybridData.myMac.macAddr[0],pwan_hybridData.myMac.macAddr[1],
            pwan_hybridData.myMac.macAddr[2],pwan_hybridData.myMac.macAddr[3],
            pwan_hybridData.myMac.macAddr[4],pwan_hybridData.myMac.macAddr[5]);
	    return count;	
    }else if (1 == sscanf(val_string, "disable route mac%s",&idx)) {
        val = atoi(idx);
        if( val <0 || val >= HYBIRD_ROUTE_MAC_MAX )
            ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
        
        memset(&(pwan_hybridData.routeMac[val]),0,sizeof(HYBIRD_Route_MAC_t));
        printk("Disable route mac%d\n",val);
        return count;
    }else if (12 == sscanf(val_string, "test route mac %c%c%c%c:%c%c%c%c:%c%c%c%c",\
					&mac[0][0], &mac[0][1],
					&mac[1][0], &mac[1][1],
					&mac[2][0], &mac[2][1],
					&mac[3][0], &mac[3][1],
					&mac[4][0], &mac[4][1],
					&mac[5][0], &mac[5][1]))
    {
        unsigned char macAddr_t[ETH_ADDR_LEN] = {0};
        
        for (i = 0; i < ETH_ADDR_LEN; i++){
            //sscanf(mac[i],"%2x",&(pwan_hybridData.route_mac[val].macAddr[i]));
            tmp = hex2dec(mac[i]);
            if (-1 == tmp)
                ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
            
            macAddr_t[i] = tmp & 0xFF;
        }
        printk("%02x%02x:%02x%02x:%02x%02x ",
            macAddr_t[0],macAddr_t[1],
            macAddr_t[2],macAddr_t[3],
            macAddr_t[4],macAddr_t[5]);
        if(is_route_mode(macAddr_t))
           printk("is route mode\n") ;
        else
           printk("is bridge mode\n") ;
       
	    return count;	
    }
    
    ROUTE_MAC_USAGE_PRINT_WITH_RET(count); 
}

int hybird_pwan_set_route_mac(GPON_HYBIRD_ROUTE_MAC_T * pRouteMac, GPON_HYBIRD_MAC_ACTION_TYPE_t type)
{
    int i = 0;
    int first_unused = HYBIRD_ROUTE_MAC_MAX;
    int match_idx = HYBIRD_ROUTE_MAC_MAX;
    int ret = 0;
    
    if ((HYBIRD_MY_MAC != pRouteMac->type ) && (HYBIRD_ROUTE_MAC != pRouteMac->type ))
        return -1;

    if(type>= HYBIRD_ROUTE_MAC_MAX_ACTION)
        return -1;
    
    spin_lock(&pwan_hybridData.macLock);

    /* add or delete my mac*/
    if (HYBIRD_MY_MAC == pRouteMac->type )
    {
        /* delete action*/
        if(HYBIRD_ROUTE_MAC_DEL_ACTION == type)
        {
            if (pwan_hybridData.myMac.valid && (0 == memcmp(pwan_hybridData.myMac.macAddr,pRouteMac->macAddr,ETH_ADDR_LEN)))
            {
                PON_MSG(MSG_TRACE, "Delete my mac %02x%02x:%02x%02x:%02x%02x\n",
                pRouteMac->macAddr[0],pRouteMac->macAddr[1],
                pRouteMac->macAddr[2],pRouteMac->macAddr[3],
                pRouteMac->macAddr[4],pRouteMac->macAddr[5]) ;
                
                memset(&(pwan_hybridData.myMac),0,sizeof(HYBIRD_Route_MAC_t));
                ret = 0;
                goto SET_HYBIRD_MAC_RETURN;
            }
            else
            {
                ret = -1;
                goto SET_HYBIRD_MAC_RETURN;
            }
        }
        
        /* add action */
        if(HYBIRD_ROUTE_MAC_ADD_ACTION == type)
        {
            if (pwan_hybridData.myMac.valid)
            {
                if (0 == memcmp(pwan_hybridData.myMac.macAddr,pRouteMac->macAddr,ETH_ADDR_LEN))
                    ret = 0;
                else
                    ret = -1;
                goto SET_HYBIRD_MAC_RETURN;
            }
                
            PON_MSG(MSG_TRACE, "set my mac %02x%02x:%02x%02x:%02x%02x\n",
            pRouteMac->macAddr[0],pRouteMac->macAddr[1],
            pRouteMac->macAddr[2],pRouteMac->macAddr[3],
            pRouteMac->macAddr[4],pRouteMac->macAddr[5]) ;

            memcpy(pwan_hybridData.myMac.macAddr,pRouteMac->macAddr,ETH_ADDR_LEN);
            pwan_hybridData.myMac.valid = 1;

            ret =  0;
            goto SET_HYBIRD_MAC_RETURN;
        }
    }

    /*add or delete route mac */
    /* loop to serach the same mac idx and first unused idx */
    for (i = 0; i < HYBIRD_ROUTE_MAC_MAX ; i++ )
    {
        if (0 != pwan_hybridData.routeMac[i].valid)
        { 
            if (0 == memcmp(pwan_hybridData.routeMac[i].macAddr,pRouteMac->macAddr,ETH_ADDR_LEN))
                match_idx = i;
            
        }
        else
        {
            if (HYBIRD_ROUTE_MAC_MAX == first_unused)
                first_unused = i;
        }
    }

    printk( " match_idx = %d first_unused = %d \r\n",match_idx,first_unused);
    /* delete action*/
    if (HYBIRD_ROUTE_MAC_DEL_ACTION == type) 
    {
        if(HYBIRD_ROUTE_MAC_MAX != match_idx)
        {
            memset(&(pwan_hybridData.routeMac[match_idx]),0,sizeof(HYBIRD_Route_MAC_t));
            ret = 0;
            goto SET_HYBIRD_MAC_RETURN;
        }
        else
        {
            ret = -1;
            goto SET_HYBIRD_MAC_RETURN;
        }
    }

    /* add action*/
    if ((HYBIRD_ROUTE_MAC_ADD_ACTION == type))
    {
        if(HYBIRD_ROUTE_MAC_MAX != match_idx)
        {
            PON_MSG(MSG_WARN, "This mac is already in the route mac table!\n");
            ret = 0;
            goto SET_HYBIRD_MAC_RETURN;
        }
        if (HYBIRD_ROUTE_MAC_MAX == first_unused)
        {
            PON_MSG(MSG_ERR, "Route mac table is full!\n");
            ret = -1;
            goto SET_HYBIRD_MAC_RETURN;
        }
        
        memcpy(pwan_hybridData.routeMac[first_unused].macAddr,pRouteMac->macAddr,ETH_ADDR_LEN);
        pwan_hybridData.routeMac[first_unused].valid = 1;
    }
    
SET_HYBIRD_MAC_RETURN:  
    spin_unlock(&pwan_hybridData.macLock);
    return ret;
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

static int isMulticastPkt(unchar *addr)
{
	if(addr == NULL)
		return 0;

	if((addr[0] == 0x01) )
		return 1;
	else
		return 0;
}

static void hybrid_pwan_stats_set(struct net_device_stats * pstats, struct net_device_stats * pstats_o,struct net_device_stats * pstats_n)
{
    pstats->collisions += pstats_n->collisions - pstats_o->collisions ;
    pstats->rx_packets  += pstats_n->rx_packets  - pstats_o->rx_packets  ;
    pstats->tx_packets  += pstats_n->tx_packets  - pstats_o->tx_packets  ;
    pstats->rx_bytes  += pstats_n->rx_bytes  - pstats_o->rx_bytes  ;
    pstats->tx_bytes  += pstats_n->tx_bytes  - pstats_o->tx_bytes  ;
    pstats->rx_errors  += pstats_n->rx_errors  - pstats_o->rx_errors  ;
    pstats->tx_errors  += pstats_n->tx_errors  - pstats_o->tx_errors  ;
    pstats->rx_dropped  += pstats_n->rx_dropped  - pstats_o->rx_dropped  ;
    pstats->tx_dropped  += pstats_n->tx_dropped  - pstats_o->tx_dropped  ;
    pstats->multicast  += pstats_n->multicast  - pstats_o->multicast  ;
    pstats->collisions  += pstats_n->collisions  - pstats_o->collisions  ;
    pstats->rx_length_errors  += pstats_n->rx_length_errors  - pstats_o->rx_length_errors  ;
    pstats->rx_over_errors  += pstats_n->rx_over_errors  - pstats_o->rx_over_errors  ;
    pstats->rx_crc_errors  += pstats_n->rx_crc_errors  - pstats_o->rx_crc_errors  ;
    pstats->rx_frame_errors  += pstats_n->rx_frame_errors  - pstats_o->rx_frame_errors  ;
    pstats->rx_fifo_errors  += pstats_n->rx_fifo_errors  - pstats_o->rx_fifo_errors  ;
    pstats->rx_missed_errors  += pstats_n->rx_missed_errors  - pstats_o->rx_missed_errors  ;
    pstats->tx_aborted_errors  += pstats_n->tx_aborted_errors  - pstats_o->tx_aborted_errors  ;
    pstats->tx_carrier_errors  += pstats_n->tx_carrier_errors  - pstats_o->tx_carrier_errors  ;
    pstats->tx_fifo_errors  += pstats_n->tx_fifo_errors  - pstats_o->tx_fifo_errors  ;
    pstats->tx_heartbeat_errors  += pstats_n->tx_heartbeat_errors  - pstats_o->tx_heartbeat_errors  ;
    pstats->tx_window_errors  += pstats_n->tx_window_errors  - pstats_o->tx_window_errors  ;
    pstats->rx_compressed  += pstats_n->rx_compressed  - pstats_o->rx_compressed  ;
    pstats->tx_compressed  += pstats_n->tx_compressed  - pstats_o->tx_compressed  ;
    return;
}

int hybird_pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    PWAN_NetPriv_T *pNetPriv;
    PWAN_NetPriv_T *pNetPriv_pon;
    struct net_device_stats stats_old = {0};
    struct net_device_stats stats_new = {0};
    int ret = NETDEV_TX_OK ;
    
    pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev);
    

    PON_MSG(MSG_TRACE,"[%s][%d] :devce = %s\n", __FUNCTION__,__LINE__,dev->name) ;

    if( 0 == memcmp(dev->name,"gpon0",sizeof("gpon0")) || 
        0 == memcmp(dev->name,"wan0",sizeof("wan0")))
    {
        dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_DATA];
        pNetPriv_pon = (PWAN_NetPriv_T *)netdev_priv(dev);
        
        PON_MSG(MSG_TRACE,"[%s][%d] :new device name = %d.\n",__FUNCTION__,__LINE__,dev->name);
    }
    memcpy(&stats_old,&(pNetPriv_pon->stats),sizeof(struct net_device_stats));
    ret = pwan_net_start_xmit(skb, dev);
    if (ret != NETDEV_TX_OK) {
        PON_MSG(MSG_TRACE,"[%s][%d] :Packet droped.\n",__FUNCTION__,__LINE__);
        return ret;
    }
    memcpy(&stats_new,&(pNetPriv_pon->stats),sizeof(struct net_device_stats));
    hybrid_pwan_stats_set(&(pNetPriv->stats),&stats_old,&stats_old);
    
    return ret;
}

int hybird_pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen)
{
    PWAN_FERxMsg_T *pRxBmMsg_tmp = (PWAN_FERxMsg_T *)pMsg ; 
    int ret = NETDEV_TX_OK ;
    PWAN_FERxMsg_T pRxBmMsg_s;
	PWAN_FERxMsg_T *pRxBmMsg = &pRxBmMsg_s;
	memcpy(pRxBmMsg, pRxBmMsg_tmp, sizeof(PWAN_FERxMsg_T));
    struct sk_buff *skb2 = NULL;

    PON_MSG(MSG_TRACE,"[%s][%d] :Receive packet.\n",__FUNCTION__,__LINE__);
    if ( pRxBmMsg->raw.oam || (isMulticastPkt(skb->data)) || (gpPonSysData->sysLinkStatus != PON_LINK_STATUS_GPON))
    {
        PON_MSG(MSG_TRACE,"[%s][%d] :Receive oam packet/multicast packet/ not GPON packet send to the orignal data path.\n",
            __FUNCTION__,__LINE__);
        skb->ecnt_sk_buff.wan_if = HYBIRD_PWAN_IF_NONE;
        ret = pwan_cb_rx_packet(pMsg, msgLen, skb, pktLen);
    }
    else if (isBroadcastPkt(skb->data))
    {
        PON_MSG(MSG_TRACE,"[%s][%d] :Receive broadcast packet should send to both GPON0 and WAN0.\n",
            __FUNCTION__,__LINE__);
        
        
        skb2 = dev_alloc_skb(pktLen);      
        if (!skb2){
            printk("\r\n[%s][%d]alloc skb failed.\r\n",__FUNCTION__,__LINE__); 
            return ret;
        }
        skb_put(skb2, skb->len);
        memcpy(skb2->data, skb->data, pktLen);
        
        skb->ecnt_sk_buff.wan_if = HYBIRD_PWAN_IF_GPON0;
        ret = pwan_cb_rx_packet(pMsg, msgLen, skb, pktLen);

        skb2->ecnt_sk_buff.wan_if = HYBIRD_PWAN_IF_WAN0;
        if ( NETDEV_TX_OK != ret )
            ret = pwan_cb_rx_packet(pMsg, msgLen, skb2, pktLen);
        else
            pwan_cb_rx_packet(pMsg, msgLen, skb2, pktLen);
    }
    else if  (is_route_mode(skb->data))
    {
        PON_MSG(MSG_TRACE,"[%s][%d] :Receive router packet send to GPON0.\n",
            __FUNCTION__,__LINE__);
        skb->ecnt_sk_buff.wan_if = HYBIRD_PWAN_IF_GPON0;
        ret = pwan_cb_rx_packet(pMsg, msgLen, skb, pktLen);
    }
    else
    {
        PON_MSG(MSG_TRACE,"[%s][%d] :Receive bridge packet send to WAN0.\n",
            __FUNCTION__,__LINE__);
        skb->ecnt_sk_buff.wan_if = HYBIRD_PWAN_IF_WAN0;
        ret = pwan_cb_rx_packet(pMsg, msgLen, skb, pktLen);
    }
    
    return ret;
}


static int route_mac_proc_init(void)
{
    pwan_hybridData.route_mac_proc = create_proc_entry("tc3162/hybird_route_mac", 0, NULL);
    if(NULL == pwan_hybridData.route_mac_proc)
    {
        printk("Creat Hybird route MAC faile.\n");
        return -1;
    }
    
    pwan_hybridData.route_mac_proc->read_proc  = route_mac_read_proc;
    pwan_hybridData.route_mac_proc->write_proc = route_mac_write_proc;
    
    return 0;
    
}

int pwan_hybird_init(void)
{
	int i ;
    int ret = 0;
    char * mac = NULL;
    GPON_HYBIRD_ROUTE_MAC_T  my_mac;
    GPON_HYBIRD_MAC_ACTION_TYPE_t act_type = HYBIRD_ROUTE_MAC_ADD_ACTION;

	memset(&pwan_hybridData, 0, sizeof(PWAN_HybirdPriv_T));
    memset(&my_mac, 0, sizeof(GPON_HYBIRD_ROUTE_MAC_T));

    /* create proc to set route mac */
    ret = route_mac_proc_init();
    if (0 != ret)
        return ret;

    /* create GPON0 device*/
	ret = pwan_create_net_interface(PWAN_IF_GPON0);
    if (0 != ret)
        return ret;
    
    /* create WAN0 device*/
	ret = pwan_create_net_interface(PWAN_IF_WAN0);
    if (0 != ret)
        return ret;

    spin_lock_init(&pwan_hybridData.macLock) ;

    /*init my mac*/
    memcpy(my_mac.macAddr, get_my_mac(),ETH_ADDR_LEN);
    my_mac.type = HYBIRD_MY_MAC;
    ret = hybird_pwan_set_route_mac(&my_mac ,act_type);
    if (0 != ret)
        return ret;
    
	return 0 ;	
}

#endif /* TCSUPPORT_WAN_GPON */

#endif

