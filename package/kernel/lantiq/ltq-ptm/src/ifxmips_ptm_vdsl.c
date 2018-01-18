/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_vdsl.c
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver common source file (core functions for VR9)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 07 JUL 2009  Xu Liang        Init Version
*******************************************************************************/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>

#include "ifxmips_ptm_vdsl.h"
#include <lantiq_soc.h>

#define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
#define MODULE_PARM(a, b)         module_param(a, int, 0)

static int wanqos_en = 0;
static int queue_gamma_map[4] = {0xFE, 0x01, 0x00, 0x00};

MODULE_PARM(wanqos_en, "i");
MODULE_PARM_DESC(wanqos_en, "WAN QoS support, 1 - enabled, 0 - disabled.");

MODULE_PARM_ARRAY(queue_gamma_map, "4-4i");
MODULE_PARM_DESC(queue_gamma_map, "TX QoS queues mapping to 4 TX Gamma interfaces.");

extern int (*ifx_mei_atm_showtime_enter)(struct port_cell_info *, void *);
extern int (*ifx_mei_atm_showtime_exit)(void);
extern int ifx_mei_atm_showtime_check(int *is_showtime, struct port_cell_info *port_cell, void **xdata_addr);

static int g_showtime = 0;
static void *g_xdata_addr = NULL;


#define ENABLE_TMP_DBG                          0

unsigned long cgu_get_pp32_clock(void)
{
	struct clk *c = clk_get_ppe();
	unsigned long rate = clk_get_rate(c);
	clk_put(c);
	return rate;
}

static void ptm_setup(struct net_device *, int);
static struct net_device_stats *ptm_get_stats(struct net_device *);
static int ptm_open(struct net_device *);
static int ptm_stop(struct net_device *);
  static unsigned int ptm_poll(int, unsigned int);
  static int ptm_napi_poll(struct napi_struct *, int);
static int ptm_hard_start_xmit(struct sk_buff *, struct net_device *);
static int ptm_change_mtu(struct net_device *, int);
static int ptm_ioctl(struct net_device *, struct ifreq *, int);
static void ptm_tx_timeout(struct net_device *);

static inline struct sk_buff* alloc_skb_rx(void);
static inline struct sk_buff* alloc_skb_tx(unsigned int);
static inline struct sk_buff *get_skb_pointer(unsigned int);
static inline int get_tx_desc(unsigned int, unsigned int *);

/*
 *  Mailbox handler and signal function
 */
static irqreturn_t mailbox_irq_handler(int, void *);

/*
 *  Tasklet to Handle Swap Descriptors
 */
static void do_swap_desc_tasklet(unsigned long);


/*
 *  Init & clean-up functions
 */
static inline int init_priv_data(void);
static inline void clear_priv_data(void);
static inline int init_tables(void);
static inline void clear_tables(void);

static int g_wanqos_en = 0;

static int g_queue_gamma_map[4];

static struct ptm_priv_data g_ptm_priv_data;

static struct net_device_ops g_ptm_netdev_ops = {
    .ndo_get_stats       = ptm_get_stats,
    .ndo_open            = ptm_open,
    .ndo_stop            = ptm_stop,
    .ndo_start_xmit      = ptm_hard_start_xmit,
    .ndo_validate_addr   = eth_validate_addr,
    .ndo_set_mac_address = eth_mac_addr,
    .ndo_change_mtu      = ptm_change_mtu,
    .ndo_do_ioctl        = ptm_ioctl,
    .ndo_tx_timeout      = ptm_tx_timeout,
};

static struct net_device *g_net_dev[1] = {0};
static char *g_net_dev_name[1] = {"dsl0"};

static int g_ptm_prio_queue_map[8];

static DECLARE_TASKLET(g_swap_desc_tasklet, do_swap_desc_tasklet, 0);


unsigned int ifx_ptm_dbg_enable = DBG_ENABLE_MASK_ERR;

/*
 * ####################################
 *            Local Function
 * ####################################
 */

static void ptm_setup(struct net_device *dev, int ndev)
{
    netif_carrier_off(dev);

    dev->netdev_ops      = &g_ptm_netdev_ops;
    netif_napi_add(dev, &g_ptm_priv_data.itf[ndev].napi, ptm_napi_poll, 16);
    dev->watchdog_timeo  = ETH_WATCHDOG_TIMEOUT;

    dev->dev_addr[0] = 0x00;
    dev->dev_addr[1] = 0x20;
	dev->dev_addr[2] = 0xda;
	dev->dev_addr[3] = 0x86;
	dev->dev_addr[4] = 0x23;
	dev->dev_addr[5] = 0x75 + ndev;
}

static struct net_device_stats *ptm_get_stats(struct net_device *dev)
{
   struct net_device_stats *s;
  
    if ( dev != g_net_dev[0] )
        return NULL;
s = &g_ptm_priv_data.itf[0].stats;

    return s;
}

static int ptm_open(struct net_device *dev)
{
    ASSERT(dev == g_net_dev[0], "incorrect device");

    napi_enable(&g_ptm_priv_data.itf[0].napi);

    IFX_REG_W32_MASK(0, 1, MBOX_IGU1_IER);

    netif_start_queue(dev);

    return 0;
}

static int ptm_stop(struct net_device *dev)
{
    ASSERT(dev == g_net_dev[0], "incorrect device");

    IFX_REG_W32_MASK(1 | (1 << 17), 0, MBOX_IGU1_IER);

    napi_disable(&g_ptm_priv_data.itf[0].napi);

    netif_stop_queue(dev);

    return 0;
}

static unsigned int ptm_poll(int ndev, unsigned int work_to_do)
{
    unsigned int work_done = 0;
    volatile struct rx_descriptor *desc;
    struct rx_descriptor reg_desc;
    struct sk_buff *skb, *new_skb;

    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    while ( work_done < work_to_do ) {
	desc = &WAN_RX_DESC_BASE[g_ptm_priv_data.itf[0].rx_desc_pos];
        if ( desc->own /* || !desc->c */ )  //  if PP32 hold descriptor or descriptor not completed
            break;
        if ( ++g_ptm_priv_data.itf[0].rx_desc_pos == WAN_RX_DESC_NUM )
            g_ptm_priv_data.itf[0].rx_desc_pos = 0;

        reg_desc = *desc;
        skb = get_skb_pointer(reg_desc.dataptr);
        ASSERT(skb != NULL, "invalid pointer skb == NULL");

        new_skb = alloc_skb_rx();
        if ( new_skb != NULL ) {
            skb_reserve(skb, reg_desc.byteoff);
            skb_put(skb, reg_desc.datalen);

            //  parse protocol header
            skb->dev = g_net_dev[0];
            skb->protocol = eth_type_trans(skb, skb->dev);

            g_net_dev[0]->last_rx = jiffies;

            netif_receive_skb(skb);

            g_ptm_priv_data.itf[0].stats.rx_packets++;
            g_ptm_priv_data.itf[0].stats.rx_bytes += reg_desc.datalen;

            reg_desc.dataptr = (unsigned int)new_skb->data & 0x0FFFFFFF;
            reg_desc.byteoff = RX_HEAD_MAC_ADDR_ALIGNMENT;
        }

        reg_desc.datalen = RX_MAX_BUFFER_SIZE - RX_HEAD_MAC_ADDR_ALIGNMENT;
        reg_desc.own     = 1;
        reg_desc.c       = 0;

        /*  write discriptor to memory  */
        *((volatile unsigned int *)desc + 1) = *((unsigned int *)&reg_desc + 1);
        wmb();
        *(volatile unsigned int *)desc = *(unsigned int *)&reg_desc;

        work_done++;
    }

    return work_done;
}

static int ptm_napi_poll(struct napi_struct *napi, int budget)
{
    int ndev = 0;
    unsigned int work_done;

    work_done = ptm_poll(ndev, budget);

    //  interface down
    if ( !netif_running(napi->dev) ) {
        napi_complete(napi);
        return work_done;
    }

    //  clear interrupt
    IFX_REG_W32_MASK(0, 1, MBOX_IGU1_ISRC);
    //  no more traffic
    if (work_done < budget) {
	napi_complete(napi);
        IFX_REG_W32_MASK(0, 1, MBOX_IGU1_IER);
        return work_done;
    }

    //  next round
    return work_done;
}

static int ptm_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    unsigned int f_full;
    int desc_base;
    volatile struct tx_descriptor *desc;
    struct tx_descriptor reg_desc = {0};
    struct sk_buff *skb_to_free;
    unsigned int byteoff;

    ASSERT(dev == g_net_dev[0], "incorrect device");

    if ( !g_showtime ) {
        err("not in showtime");
        goto PTM_HARD_START_XMIT_FAIL;
    }

    /*  allocate descriptor */
    desc_base = get_tx_desc(0, &f_full);
    if ( f_full ) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
        netif_trans_update(dev);
#else
        dev->trans_start = jiffies;
#endif
        netif_stop_queue(dev);

        IFX_REG_W32_MASK(0, 1 << 17, MBOX_IGU1_ISRC);
        IFX_REG_W32_MASK(0, 1 << 17, MBOX_IGU1_IER);
    }
    if ( desc_base < 0 )
        goto PTM_HARD_START_XMIT_FAIL;
    desc = &CPU_TO_WAN_TX_DESC_BASE[desc_base];

    byteoff = (unsigned int)skb->data & (DATA_BUFFER_ALIGNMENT - 1);
    if ( skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff || skb_cloned(skb) ) {
        struct sk_buff *new_skb;

        ASSERT(skb_headroom(skb) >= sizeof(struct sk_buff *) + byteoff, "skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff");
        ASSERT(!skb_cloned(skb), "skb is cloned");

        new_skb = alloc_skb_tx(skb->len);
        if ( new_skb == NULL ) {
            dbg("no memory");
            goto ALLOC_SKB_TX_FAIL;
        }
        skb_put(new_skb, skb->len);
        memcpy(new_skb->data, skb->data, skb->len);
        dev_kfree_skb_any(skb);
        skb = new_skb;
        byteoff = (unsigned int)skb->data & (DATA_BUFFER_ALIGNMENT - 1);
        /*  write back to physical memory   */
        dma_cache_wback((unsigned long)skb->data, skb->len);
    }

    *(struct sk_buff **)((unsigned int)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;
    /*  write back to physical memory   */
    dma_cache_wback((unsigned long)skb->data - byteoff - sizeof(struct sk_buff *), skb->len + byteoff + sizeof(struct sk_buff *));

    /*  free previous skb   */
    skb_to_free = get_skb_pointer(desc->dataptr);
    if ( skb_to_free != NULL )
        dev_kfree_skb_any(skb_to_free);

    /*  update descriptor   */
    reg_desc.small   = 0;
    reg_desc.dataptr = (unsigned int)skb->data & (0x0FFFFFFF ^ (DATA_BUFFER_ALIGNMENT - 1));
    reg_desc.datalen = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
    reg_desc.qid     = g_ptm_prio_queue_map[skb->priority > 7 ? 7 : skb->priority];
    reg_desc.byteoff = byteoff;
    reg_desc.own     = 1;
    reg_desc.c       = 1;
    reg_desc.sop = reg_desc.eop = 1;

    /*  update MIB  */
    g_ptm_priv_data.itf[0].stats.tx_packets++;
    g_ptm_priv_data.itf[0].stats.tx_bytes += reg_desc.datalen;

    /*  write discriptor to memory  */
    *((volatile unsigned int *)desc + 1) = *((unsigned int *)&reg_desc + 1);
    wmb();
    *(volatile unsigned int *)desc = *(unsigned int *)&reg_desc;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
    netif_trans_update(dev);
#else
    dev->trans_start = jiffies;
#endif

    return 0;

ALLOC_SKB_TX_FAIL:
PTM_HARD_START_XMIT_FAIL:
    dev_kfree_skb_any(skb);
    g_ptm_priv_data.itf[0].stats.tx_dropped++;
    return 0;
}

static int ptm_change_mtu(struct net_device *dev, int mtu)
{
	/* Allow up to 1508 bytes, for RFC4638 */
        if (mtu < 68 || mtu > ETH_DATA_LEN + 8)
                return -EINVAL;
        dev->mtu = mtu;
        return 0;
}

static int ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    ASSERT(dev == g_net_dev[0], "incorrect device");

    switch ( cmd )
    {
    case IFX_PTM_MIB_CW_GET:
	((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxNoIdleCodewords   = IFX_REG_R32(DREG_AR_CELL0) + IFX_REG_R32(DREG_AR_CELL1);
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxIdleCodewords     = IFX_REG_R32(DREG_AR_IDLE_CNT0) + IFX_REG_R32(DREG_AR_IDLE_CNT1);
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxCodingViolation   = IFX_REG_R32(DREG_AR_CVN_CNT0) + IFX_REG_R32(DREG_AR_CVN_CNT1) + IFX_REG_R32(DREG_AR_CVNP_CNT0) + IFX_REG_R32(DREG_AR_CVNP_CNT1);
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxNoIdleCodewords   = IFX_REG_R32(DREG_AT_CELL0) + IFX_REG_R32(DREG_AT_CELL1);
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxIdleCodewords     = IFX_REG_R32(DREG_AT_IDLE_CNT0) + IFX_REG_R32(DREG_AT_IDLE_CNT1);
        break;
    case IFX_PTM_MIB_FRAME_GET:
	{
            PTM_FRAME_MIB_T data = {0};
            int i;

            data.RxCorrect = IFX_REG_R32(DREG_AR_HEC_CNT0) + IFX_REG_R32(DREG_AR_HEC_CNT1) + IFX_REG_R32(DREG_AR_AIIDLE_CNT0) + IFX_REG_R32(DREG_AR_AIIDLE_CNT1);
            for ( i = 0; i < 4; i++ )
                data.RxDropped += WAN_RX_MIB_TABLE(i)->wrx_dropdes_pdu;
            for ( i = 0; i < 8; i++ )
                data.TxSend    += WAN_TX_MIB_TABLE(i)->wtx_total_pdu;

            *((PTM_FRAME_MIB_T *)ifr->ifr_data) = data;
        }
        break;
    case IFX_PTM_CFG_GET:
	//  use bear channel 0 preemption gamma interface settings
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent = 1;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck   = RX_GAMMA_ITF_CFG(0)->rx_eth_fcs_ver_dis == 0 ? 1 : 0;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck    = RX_GAMMA_ITF_CFG(0)->rx_tc_crc_ver_dis == 0 ? 1 : 0;;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen      = RX_GAMMA_ITF_CFG(0)->rx_tc_crc_size == 0 ? 0 : (RX_GAMMA_ITF_CFG(0)->rx_tc_crc_size * 16);
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen     = TX_GAMMA_ITF_CFG(0)->tx_eth_fcs_gen_dis == 0 ? 1 : 0;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen      = TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size == 0 ? 0 : 1;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen      = TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size == 0 ? 0 : (TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size * 16);
        break;
    case IFX_PTM_CFG_SET:
	{
            int i;

            for ( i = 0; i < 4; i++ ) {
                RX_GAMMA_ITF_CFG(i)->rx_eth_fcs_ver_dis = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck ? 0 : 1;

                RX_GAMMA_ITF_CFG(0)->rx_tc_crc_ver_dis = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck ? 0 : 1;

                switch ( ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen ) {
                    case 16: RX_GAMMA_ITF_CFG(0)->rx_tc_crc_size = 1; break;
                    case 32: RX_GAMMA_ITF_CFG(0)->rx_tc_crc_size = 2; break;
                    default: RX_GAMMA_ITF_CFG(0)->rx_tc_crc_size = 0;
                }

                TX_GAMMA_ITF_CFG(0)->tx_eth_fcs_gen_dis = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen ? 0 : 1;

                if ( ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen ) {
                    switch ( ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen ) {
                        case 16: TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size = 1; break;
                        case 32: TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size = 2; break;
                        default: TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size = 0;
                    }
                }
                else
                    TX_GAMMA_ITF_CFG(0)->tx_tc_crc_size = 0;
            }
        }
        break;
    case IFX_PTM_MAP_PKT_PRIO_TO_Q:
        {
            struct ppe_prio_q_map cmd;

            if ( copy_from_user(&cmd, ifr->ifr_data, sizeof(cmd)) )
                return -EFAULT;

            if ( cmd.pkt_prio < 0 || cmd.pkt_prio >= ARRAY_SIZE(g_ptm_prio_queue_map) )
                return -EINVAL;

            if ( cmd.qid < 0 || cmd.qid >= g_wanqos_en )
                return -EINVAL;

            g_ptm_prio_queue_map[cmd.pkt_prio] = cmd.qid;
        }
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void ptm_tx_timeout(struct net_device *dev)
{
    ASSERT(dev == g_net_dev[0], "incorrect device");

    /*  disable TX irq, release skb when sending new packet */
    IFX_REG_W32_MASK(1 << 17, 0, MBOX_IGU1_IER);

    /*  wake up TX queue    */
    netif_wake_queue(dev);

    return;
}

static inline struct sk_buff* alloc_skb_rx(void)
{
    struct sk_buff *skb;

    /*  allocate memroy including trailer and padding   */
    skb = dev_alloc_skb(RX_MAX_BUFFER_SIZE + DATA_BUFFER_ALIGNMENT);
    if ( skb != NULL ) {
        /*  must be burst length alignment and reserve two more bytes for MAC address alignment  */
        if ( ((unsigned int)skb->data & (DATA_BUFFER_ALIGNMENT - 1)) != 0 )
            skb_reserve(skb, ~((unsigned int)skb->data + (DATA_BUFFER_ALIGNMENT - 1)) & (DATA_BUFFER_ALIGNMENT - 1));
        /*  pub skb in reserved area "skb->data - 4"    */
        *((struct sk_buff **)skb->data - 1) = skb;
        wmb();
        /*  write back and invalidate cache    */
        dma_cache_wback_inv((unsigned long)skb->data - sizeof(skb), sizeof(skb));
        /*  invalidate cache    */
        dma_cache_inv((unsigned long)skb->data, (unsigned int)skb->end - (unsigned int)skb->data);
    }

    return skb;
}

static inline struct sk_buff* alloc_skb_tx(unsigned int size)
{
    struct sk_buff *skb;

    /*  allocate memory including padding   */
    size = RX_MAX_BUFFER_SIZE;
    size = (size + DATA_BUFFER_ALIGNMENT - 1) & ~(DATA_BUFFER_ALIGNMENT - 1);
    skb = dev_alloc_skb(size + DATA_BUFFER_ALIGNMENT);
    /*  must be burst length alignment  */
    if ( skb != NULL )
        skb_reserve(skb, ~((unsigned int)skb->data + (DATA_BUFFER_ALIGNMENT - 1)) & (DATA_BUFFER_ALIGNMENT - 1));
    return skb;
}

static inline struct sk_buff *get_skb_pointer(unsigned int dataptr)
{
    unsigned int skb_dataptr;
    struct sk_buff *skb;

    //  usually, CPE memory is less than 256M bytes
    //  so NULL means invalid pointer
    if ( dataptr == 0 ) {
        dbg("dataptr is 0, it's supposed to be invalid pointer");
        return NULL;
    }

    skb_dataptr = (dataptr - 4) | KSEG1;
    skb = *(struct sk_buff **)skb_dataptr;

    ASSERT((unsigned int)skb >= KSEG0, "invalid skb - skb = %#08x, dataptr = %#08x", (unsigned int)skb, dataptr);
    ASSERT((((unsigned int)skb->data & (0x0FFFFFFF ^ (DATA_BUFFER_ALIGNMENT - 1))) | KSEG1) == (dataptr | KSEG1), "invalid skb - skb = %#08x, skb->data = %#08x, dataptr = %#08x", (unsigned int)skb, (unsigned int)skb->data, dataptr);

    return skb;
}

static inline int get_tx_desc(unsigned int itf, unsigned int *f_full)
{
    int desc_base = -1;
    struct ptm_itf *p_itf = &g_ptm_priv_data.itf[0];

    //  assume TX is serial operation
    //  no protection provided

    *f_full = 1;

    if ( CPU_TO_WAN_TX_DESC_BASE[p_itf->tx_desc_pos].own == 0 ) {
        desc_base = p_itf->tx_desc_pos;
        if ( ++(p_itf->tx_desc_pos) == CPU_TO_WAN_TX_DESC_NUM )
            p_itf->tx_desc_pos = 0;
        if ( CPU_TO_WAN_TX_DESC_BASE[p_itf->tx_desc_pos].own == 0 )
            *f_full = 0;
    }

    return desc_base;
}

static irqreturn_t mailbox_irq_handler(int irq, void *dev_id)
{
    unsigned int isr;
    int i;

    isr = IFX_REG_R32(MBOX_IGU1_ISR);
    IFX_REG_W32(isr, MBOX_IGU1_ISRC);
    isr &= IFX_REG_R32(MBOX_IGU1_IER);

            if (isr & BIT(0)) {
                IFX_REG_W32_MASK(1, 0, MBOX_IGU1_IER);
                napi_schedule(&g_ptm_priv_data.itf[0].napi);
#if defined(ENABLE_TMP_DBG) && ENABLE_TMP_DBG
                {
                    volatile struct rx_descriptor *desc = &WAN_RX_DESC_BASE[g_ptm_priv_data.itf[0].rx_desc_pos];

                    if ( desc->own ) {  //  PP32 hold
                        err("invalid interrupt");
                    }
                }
#endif
            }
	   if (isr & BIT(16)) {
                IFX_REG_W32_MASK(1 << 16, 0, MBOX_IGU1_IER);
                tasklet_hi_schedule(&g_swap_desc_tasklet);
            }
	    if (isr & BIT(17)) {
                IFX_REG_W32_MASK(1 << 17, 0, MBOX_IGU1_IER);
                netif_wake_queue(g_net_dev[0]);
        	}

    return IRQ_HANDLED;
}

static void do_swap_desc_tasklet(unsigned long arg)
{
    int budget = 32;
    volatile struct tx_descriptor *desc;
    struct sk_buff *skb;
    unsigned int byteoff;

    while ( budget-- > 0 ) {
	if ( WAN_SWAP_DESC_BASE[g_ptm_priv_data.itf[0].tx_swap_desc_pos].own )  //  if PP32 hold descriptor
            break;

        desc = &WAN_SWAP_DESC_BASE[g_ptm_priv_data.itf[0].tx_swap_desc_pos];
        if ( ++g_ptm_priv_data.itf[0].tx_swap_desc_pos == WAN_SWAP_DESC_NUM )
            g_ptm_priv_data.itf[0].tx_swap_desc_pos = 0;

        skb = get_skb_pointer(desc->dataptr);
        if ( skb != NULL )
            dev_kfree_skb_any(skb);

        skb = alloc_skb_tx(RX_MAX_BUFFER_SIZE);
        if ( skb == NULL )
            panic("can't allocate swap buffer for PPE firmware use\n");
        byteoff = (unsigned int)skb->data & (DATA_BUFFER_ALIGNMENT - 1);
        *(struct sk_buff **)((unsigned int)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;

        desc->dataptr = (unsigned int)skb->data & 0x0FFFFFFF;
        desc->own = 1;
    }

    //  clear interrupt
    IFX_REG_W32_MASK(0, 16, MBOX_IGU1_ISRC);
    //  no more skb to be replaced
    if ( WAN_SWAP_DESC_BASE[g_ptm_priv_data.itf[0].tx_swap_desc_pos].own ) {    //  if PP32 hold descriptor
        IFX_REG_W32_MASK(0, 1 << 16, MBOX_IGU1_IER);
        return;
    }

    tasklet_hi_schedule(&g_swap_desc_tasklet);
    return;
}


static inline int ifx_ptm_version(char *buf)
{
    int len = 0;
    unsigned int major, minor;

    ifx_ptm_get_fw_ver(&major, &minor);

    len += sprintf(buf + len, "PTM %d.%d.%d", IFX_PTM_VER_MAJOR, IFX_PTM_VER_MID, IFX_PTM_VER_MINOR);
    len += sprintf(buf + len, "    PTM (E1) firmware version %d.%d\n", major, minor);

    return len;
}

static inline int init_priv_data(void)
{
    int i, j;

    g_wanqos_en = wanqos_en ? wanqos_en : 8;
    if ( g_wanqos_en > 8 )
        g_wanqos_en = 8;

    for ( i = 0; i < ARRAY_SIZE(g_queue_gamma_map); i++ )
    {
        g_queue_gamma_map[i] = queue_gamma_map[i] & ((1 << g_wanqos_en) - 1);
        for ( j = 0; j < i; j++ )
            g_queue_gamma_map[i] &= ~g_queue_gamma_map[j];
    }

    memset(&g_ptm_priv_data, 0, sizeof(g_ptm_priv_data));

    {
        int max_packet_priority = ARRAY_SIZE(g_ptm_prio_queue_map);
        int tx_num_q;
        int q_step, q_accum, p_step;

        tx_num_q = __ETH_WAN_TX_QUEUE_NUM;
        q_step = tx_num_q - 1;
        p_step = max_packet_priority - 1;
        for ( j = 0, q_accum = 0; j < max_packet_priority; j++, q_accum += q_step )
            g_ptm_prio_queue_map[j] = q_step - (q_accum + (p_step >> 1)) / p_step;
    }

    return 0;
}

static inline void clear_priv_data(void)
{
}

static inline int init_tables(void)
{
    struct sk_buff *skb_pool[WAN_RX_DESC_NUM] = {0};
    struct cfg_std_data_len cfg_std_data_len = {0};
    struct tx_qos_cfg tx_qos_cfg = {0};
    struct psave_cfg psave_cfg = {0};
    struct eg_bwctrl_cfg eg_bwctrl_cfg = {0};
    struct test_mode test_mode = {0};
    struct rx_bc_cfg rx_bc_cfg = {0};
    struct tx_bc_cfg tx_bc_cfg = {0};
    struct gpio_mode gpio_mode = {0};
    struct gpio_wm_cfg gpio_wm_cfg = {0};
    struct rx_gamma_itf_cfg rx_gamma_itf_cfg = {0};
    struct tx_gamma_itf_cfg tx_gamma_itf_cfg = {0};
    struct wtx_qos_q_desc_cfg wtx_qos_q_desc_cfg = {0};
    struct rx_descriptor rx_desc = {0};
    struct tx_descriptor tx_desc = {0};
    int i;

    for ( i = 0; i < WAN_RX_DESC_NUM; i++ ) {
        skb_pool[i] = alloc_skb_rx();
        if ( skb_pool[i] == NULL )
            goto ALLOC_SKB_RX_FAIL;
    }

    cfg_std_data_len.byte_off = RX_HEAD_MAC_ADDR_ALIGNMENT; //  this field replaces byte_off in rx descriptor of VDSL ingress
    cfg_std_data_len.data_len = 1600;
    *CFG_STD_DATA_LEN = cfg_std_data_len;

    tx_qos_cfg.time_tick    = cgu_get_pp32_clock() / 62500; //  16 * (cgu_get_pp32_clock() / 1000000)
    tx_qos_cfg.overhd_bytes = 0;
    tx_qos_cfg.eth1_eg_qnum = __ETH_WAN_TX_QUEUE_NUM;
    tx_qos_cfg.eth1_burst_chk = 1;
    tx_qos_cfg.eth1_qss     = 0;
    tx_qos_cfg.shape_en     = 0;    //  disable
    tx_qos_cfg.wfq_en       = 0;    //  strict priority
    *TX_QOS_CFG = tx_qos_cfg;

    psave_cfg.start_state   = 0;
    psave_cfg.sleep_en      = 1;    //  enable sleep mode
    *PSAVE_CFG = psave_cfg;

    eg_bwctrl_cfg.fdesc_wm  = 16;
    eg_bwctrl_cfg.class_len = 128;
    *EG_BWCTRL_CFG = eg_bwctrl_cfg;

    //*GPIO_ADDR = (unsigned int)IFX_GPIO_P0_OUT;
    *GPIO_ADDR = (unsigned int)0x00000000;  //  disabled by default

    gpio_mode.gpio_bit_bc1 = 2;
    gpio_mode.gpio_bit_bc0 = 1;
    gpio_mode.gpio_bc1_en  = 0;
    gpio_mode.gpio_bc0_en  = 0;
    *GPIO_MODE = gpio_mode;

    gpio_wm_cfg.stop_wm_bc1  = 2;
    gpio_wm_cfg.start_wm_bc1 = 4;
    gpio_wm_cfg.stop_wm_bc0  = 2;
    gpio_wm_cfg.start_wm_bc0 = 4;
    *GPIO_WM_CFG = gpio_wm_cfg;

    test_mode.mib_clear_mode    = 0;
    test_mode.test_mode         = 0;
    *TEST_MODE = test_mode;

    rx_bc_cfg.local_state   = 0;
    rx_bc_cfg.remote_state  = 0;
    rx_bc_cfg.to_false_th   = 7;
    rx_bc_cfg.to_looking_th = 3;
    *RX_BC_CFG(0) = rx_bc_cfg;
    *RX_BC_CFG(1) = rx_bc_cfg;

    tx_bc_cfg.fill_wm   = 2;
    tx_bc_cfg.uflw_wm   = 2;
    *TX_BC_CFG(0) = tx_bc_cfg;
    *TX_BC_CFG(1) = tx_bc_cfg;

    rx_gamma_itf_cfg.receive_state      = 0;
    rx_gamma_itf_cfg.rx_min_len         = 60;
    rx_gamma_itf_cfg.rx_pad_en          = 1;
    rx_gamma_itf_cfg.rx_eth_fcs_ver_dis = 0;
    rx_gamma_itf_cfg.rx_rm_eth_fcs      = 1;
    rx_gamma_itf_cfg.rx_tc_crc_ver_dis  = 0;
    rx_gamma_itf_cfg.rx_tc_crc_size     = 1;
    rx_gamma_itf_cfg.rx_eth_fcs_result  = 0xC704DD7B;
    rx_gamma_itf_cfg.rx_tc_crc_result   = 0x1D0F1D0F;
    rx_gamma_itf_cfg.rx_crc_cfg         = 0x2500;
    rx_gamma_itf_cfg.rx_eth_fcs_init_value  = 0xFFFFFFFF;
    rx_gamma_itf_cfg.rx_tc_crc_init_value   = 0x0000FFFF;
    rx_gamma_itf_cfg.rx_max_len_sel     = 0;
    rx_gamma_itf_cfg.rx_edit_num2       = 0;
    rx_gamma_itf_cfg.rx_edit_pos2       = 0;
    rx_gamma_itf_cfg.rx_edit_type2      = 0;
    rx_gamma_itf_cfg.rx_edit_en2        = 0;
    rx_gamma_itf_cfg.rx_edit_num1       = 0;
    rx_gamma_itf_cfg.rx_edit_pos1       = 0;
    rx_gamma_itf_cfg.rx_edit_type1      = 0;
    rx_gamma_itf_cfg.rx_edit_en1        = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_1l   = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_1h   = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_2l   = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_2h   = 0;
    rx_gamma_itf_cfg.rx_len_adj         = -6;
    for ( i = 0; i < 4; i++ )
        *RX_GAMMA_ITF_CFG(i) = rx_gamma_itf_cfg;

    tx_gamma_itf_cfg.tx_len_adj         = 6;
    tx_gamma_itf_cfg.tx_crc_off_adj     = 6;
    tx_gamma_itf_cfg.tx_min_len         = 0;
    tx_gamma_itf_cfg.tx_eth_fcs_gen_dis = 0;
    tx_gamma_itf_cfg.tx_tc_crc_size     = 1;
    tx_gamma_itf_cfg.tx_crc_cfg         = 0x2F00;
    tx_gamma_itf_cfg.tx_eth_fcs_init_value  = 0xFFFFFFFF;
    tx_gamma_itf_cfg.tx_tc_crc_init_value   = 0x0000FFFF;
    for ( i = 0; i < ARRAY_SIZE(g_queue_gamma_map); i++ ) {
        tx_gamma_itf_cfg.queue_mapping = g_queue_gamma_map[i];
        *TX_GAMMA_ITF_CFG(i) = tx_gamma_itf_cfg;
    }

    for ( i = 0; i < __ETH_WAN_TX_QUEUE_NUM; i++ ) {
        wtx_qos_q_desc_cfg.length = WAN_TX_DESC_NUM;
        wtx_qos_q_desc_cfg.addr   = __ETH_WAN_TX_DESC_BASE(i);
        *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg;
    }

    //  default TX queue QoS config is all ZERO

    //  TX Ctrl K Table
    IFX_REG_W32(0x90111293, TX_CTRL_K_TABLE(0));
    IFX_REG_W32(0x14959617, TX_CTRL_K_TABLE(1));
    IFX_REG_W32(0x18999A1B, TX_CTRL_K_TABLE(2));
    IFX_REG_W32(0x9C1D1E9F, TX_CTRL_K_TABLE(3));
    IFX_REG_W32(0xA02122A3, TX_CTRL_K_TABLE(4));
    IFX_REG_W32(0x24A5A627, TX_CTRL_K_TABLE(5));
    IFX_REG_W32(0x28A9AA2B, TX_CTRL_K_TABLE(6));
    IFX_REG_W32(0xAC2D2EAF, TX_CTRL_K_TABLE(7));
    IFX_REG_W32(0x30B1B233, TX_CTRL_K_TABLE(8));
    IFX_REG_W32(0xB43536B7, TX_CTRL_K_TABLE(9));
    IFX_REG_W32(0xB8393ABB, TX_CTRL_K_TABLE(10));
    IFX_REG_W32(0x3CBDBE3F, TX_CTRL_K_TABLE(11));
    IFX_REG_W32(0xC04142C3, TX_CTRL_K_TABLE(12));
    IFX_REG_W32(0x44C5C647, TX_CTRL_K_TABLE(13));
    IFX_REG_W32(0x48C9CA4B, TX_CTRL_K_TABLE(14));
    IFX_REG_W32(0xCC4D4ECF, TX_CTRL_K_TABLE(15));

    //  init RX descriptor
    rx_desc.own     = 1;
    rx_desc.c       = 0;
    rx_desc.sop     = 1;
    rx_desc.eop     = 1;
    rx_desc.byteoff = RX_HEAD_MAC_ADDR_ALIGNMENT;
    rx_desc.datalen = RX_MAX_BUFFER_SIZE - RX_HEAD_MAC_ADDR_ALIGNMENT;
    for ( i = 0; i < WAN_RX_DESC_NUM; i++ ) {
        rx_desc.dataptr = (unsigned int)skb_pool[i]->data & 0x0FFFFFFF;
        WAN_RX_DESC_BASE[i] = rx_desc;
    }

    //  init TX descriptor
    tx_desc.own     = 0;
    tx_desc.c       = 0;
    tx_desc.sop     = 1;
    tx_desc.eop     = 1;
    tx_desc.byteoff = 0;
    tx_desc.qid     = 0;
    tx_desc.datalen = 0;
    tx_desc.small   = 0;
    tx_desc.dataptr = 0;
    for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ )
        CPU_TO_WAN_TX_DESC_BASE[i] = tx_desc;
    for ( i = 0; i < WAN_TX_DESC_NUM_TOTAL; i++ )
        WAN_TX_DESC_BASE(0)[i] = tx_desc;

    //  init Swap descriptor
    for ( i = 0; i < WAN_SWAP_DESC_NUM; i++ )
        WAN_SWAP_DESC_BASE[i] = tx_desc;

    //  init fastpath TX descriptor
    tx_desc.own     = 1;
    for ( i = 0; i < FASTPATH_TO_WAN_TX_DESC_NUM; i++ )
        FASTPATH_TO_WAN_TX_DESC_BASE[i] = tx_desc;

    return 0;

ALLOC_SKB_RX_FAIL:
    while ( i-- > 0 )
        dev_kfree_skb_any(skb_pool[i]);
    return -1;
}

static inline void clear_tables(void)
{
    struct sk_buff *skb;
    int i, j;

    for ( i = 0; i < WAN_RX_DESC_NUM; i++ ) {
        skb = get_skb_pointer(WAN_RX_DESC_BASE[i].dataptr);
        if ( skb != NULL )
            dev_kfree_skb_any(skb);
    }

    for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ ) {
        skb = get_skb_pointer(CPU_TO_WAN_TX_DESC_BASE[i].dataptr);
        if ( skb != NULL )
            dev_kfree_skb_any(skb);
    }

    for ( j = 0; j < 8; j++ )
        for ( i = 0; i < WAN_TX_DESC_NUM; i++ ) {
            skb = get_skb_pointer(WAN_TX_DESC_BASE(j)[i].dataptr);
            if ( skb != NULL )
                dev_kfree_skb_any(skb);
        }

    for ( i = 0; i < WAN_SWAP_DESC_NUM; i++ ) {
        skb = get_skb_pointer(WAN_SWAP_DESC_BASE[i].dataptr);
        if ( skb != NULL )
            dev_kfree_skb_any(skb);
    }

    for ( i = 0; i < FASTPATH_TO_WAN_TX_DESC_NUM; i++ ) {
        skb = get_skb_pointer(FASTPATH_TO_WAN_TX_DESC_BASE[i].dataptr);
        if ( skb != NULL )
            dev_kfree_skb_any(skb);
    }
}

static int ptm_showtime_enter(struct port_cell_info *port_cell, void *xdata_addr)
{
	int i;

	ASSERT(port_cell != NULL, "port_cell is NULL");
	ASSERT(xdata_addr != NULL, "xdata_addr is NULL");

	//  TODO: ReTX set xdata_addr
	g_xdata_addr = xdata_addr;

	g_showtime = 1;

	for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
		netif_carrier_on(g_net_dev[i]);

	IFX_REG_W32(0x0F, UTP_CFG);

	//#ifdef CONFIG_VR9
	//    IFX_REG_W32_MASK(1 << 17, 0, FFSM_CFG0);
	//#endif

	printk("enter showtime\n");

	return 0;
}

static int ptm_showtime_exit(void)
{
	int i;

	if ( !g_showtime )
		return -1;

	//#ifdef CONFIG_VR9
	//    IFX_REG_W32_MASK(0, 1 << 17, FFSM_CFG0);
	//#endif

	IFX_REG_W32(0x00, UTP_CFG);

	for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
		netif_carrier_off(g_net_dev[i]);

	g_showtime = 0;

	//  TODO: ReTX clean state
	g_xdata_addr = NULL;

	printk("leave showtime\n");

	return 0;
}



static int ifx_ptm_init(void)
{
    int ret;
    int i;
    char ver_str[128];
    struct port_cell_info port_cell = {0};

    ret = init_priv_data();
    if ( ret != 0 ) {
        err("INIT_PRIV_DATA_FAIL");
        goto INIT_PRIV_DATA_FAIL;
    }

    ifx_ptm_init_chip();
    ret = init_tables();
    if ( ret != 0 ) {
        err("INIT_TABLES_FAIL");
        goto INIT_TABLES_FAIL;
    }

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ ) {
        g_net_dev[i] = alloc_netdev(0, g_net_dev_name[i], NET_NAME_UNKNOWN, ether_setup);
        if ( g_net_dev[i] == NULL )
            goto ALLOC_NETDEV_FAIL;
        ptm_setup(g_net_dev[i], i);
    }

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ ) {
        ret = register_netdev(g_net_dev[i]);
        if ( ret != 0 )
            goto REGISTER_NETDEV_FAIL;
    }

    /*  register interrupt handler  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
    ret = request_irq(PPE_MAILBOX_IGU1_INT, mailbox_irq_handler, 0, "ptm_mailbox_isr", &g_ptm_priv_data);
#else
    ret = request_irq(PPE_MAILBOX_IGU1_INT, mailbox_irq_handler, IRQF_DISABLED, "ptm_mailbox_isr", &g_ptm_priv_data);
#endif
    if ( ret ) {
        if ( ret == -EBUSY ) {
            err("IRQ may be occupied by other driver, please reconfig to disable it.");
        }
        else {
            err("request_irq fail");
        }
        goto REQUEST_IRQ_PPE_MAILBOX_IGU1_INT_FAIL;
    }
    disable_irq(PPE_MAILBOX_IGU1_INT);

    ret = ifx_pp32_start(0);
    if ( ret ) {
        err("ifx_pp32_start fail!");
        goto PP32_START_FAIL;
    }
    IFX_REG_W32(1 << 16, MBOX_IGU1_IER);    //  enable SWAP interrupt
    IFX_REG_W32(~0, MBOX_IGU1_ISRC);

    enable_irq(PPE_MAILBOX_IGU1_INT);

    ifx_mei_atm_showtime_check(&g_showtime, &port_cell, &g_xdata_addr);
    if ( g_showtime ) {
	ptm_showtime_enter(&port_cell, &g_xdata_addr);
    }

    ifx_mei_atm_showtime_enter = ptm_showtime_enter;
    ifx_mei_atm_showtime_exit  = ptm_showtime_exit;

    ifx_ptm_version(ver_str);
    printk(KERN_INFO "%s", ver_str);

    printk("ifxmips_ptm: PTM init succeed\n");

    return 0;

PP32_START_FAIL:
    free_irq(PPE_MAILBOX_IGU1_INT, &g_ptm_priv_data);
REQUEST_IRQ_PPE_MAILBOX_IGU1_INT_FAIL:
    i = ARRAY_SIZE(g_net_dev);
REGISTER_NETDEV_FAIL:
    while ( i-- )
        unregister_netdev(g_net_dev[i]);
    i = ARRAY_SIZE(g_net_dev);
ALLOC_NETDEV_FAIL:
    while ( i-- ) {
        free_netdev(g_net_dev[i]);
        g_net_dev[i] = NULL;
    }
INIT_TABLES_FAIL:
INIT_PRIV_DATA_FAIL:
    clear_priv_data();
    printk("ifxmips_ptm: PTM init failed\n");
    return ret;
}

static void __exit ifx_ptm_exit(void)
{
    int i;
	ifx_mei_atm_showtime_enter = NULL;
	ifx_mei_atm_showtime_exit  = NULL;


    ifx_pp32_stop(0);

    free_irq(PPE_MAILBOX_IGU1_INT, &g_ptm_priv_data);

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
        unregister_netdev(g_net_dev[i]);

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ ) {
        free_netdev(g_net_dev[i]);
        g_net_dev[i] = NULL;
    }

    clear_tables();

    ifx_ptm_uninit_chip();

    clear_priv_data();
}

#ifndef MODULE
static int __init wanqos_en_setup(char *line)
{
    wanqos_en = simple_strtoul(line, NULL, 0);

    if ( wanqos_en < 1 || wanqos_en > 8 )
        wanqos_en = 0;

    return 0;
}

static int __init queue_gamma_map_setup(char *line)
{
    char *p;
    int i;

    for ( i = 0, p = line; i < ARRAY_SIZE(queue_gamma_map) && isxdigit(*p); i++ )
    {
        queue_gamma_map[i] = simple_strtoul(p, &p, 0);
        if ( *p == ',' || *p == ';' || *p == ':' )
            p++;
    }

    return 0;
}
#endif
module_init(ifx_ptm_init);
module_exit(ifx_ptm_exit);
#ifndef MODULE
  __setup("wanqos_en=", wanqos_en_setup);
  __setup("queue_gamma_map=", queue_gamma_map_setup);
#endif

MODULE_LICENSE("GPL");
