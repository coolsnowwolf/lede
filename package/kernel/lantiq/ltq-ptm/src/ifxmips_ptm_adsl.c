/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_adsl.c
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver common source file (core functions for Danube/
**                Amazon-SE/AR9)
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



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <asm/io.h>

/*
 *  Chip Specific Head File
 */
#include "ifxmips_ptm_adsl.h"


#include <lantiq_soc.h>

/*
 * ####################################
 *        Kernel Version Adaption
 * ####################################
 */
  #define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
  #define MODULE_PARM(a, b)         module_param(a, int, 0)



/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */

static int write_desc_delay     = 0x20;         /*  Write descriptor delay                          */

static int rx_max_packet_size   = ETH_MAX_FRAME_LENGTH;
                                                /*  Max packet size for RX                          */

static int dma_rx_descriptor_length = 24;       /*  Number of descriptors per DMA RX channel        */
static int dma_tx_descriptor_length = 24;       /*  Number of descriptors per DMA TX channel        */

static int eth_efmtc_crc_cfg = 0x03100710;      /*  default: tx_eth_crc_check: 1, tx_tc_crc_check: 1, tx_tc_crc_len = 16    */
                                                /*           rx_eth_crc_present: 1, rx_eth_crc_check: 1, rx_tc_crc_check: 1, rx_tc_crc_len = 16 */

MODULE_PARM(write_desc_delay, "i");
MODULE_PARM_DESC(write_desc_delay, "PPE core clock cycles between descriptor write and effectiveness in external RAM");

MODULE_PARM(rx_max_packet_size, "i");
MODULE_PARM_DESC(rx_max_packet_size, "Max packet size in byte for downstream ethernet frames");

MODULE_PARM(dma_rx_descriptor_length, "i");
MODULE_PARM_DESC(dma_rx_descriptor_length, "Number of descriptor assigned to DMA RX channel (>16)");
MODULE_PARM(dma_tx_descriptor_length, "i");
MODULE_PARM_DESC(dma_tx_descriptor_length, "Number of descriptor assigned to DMA TX channel (>16)");

MODULE_PARM(eth_efmtc_crc_cfg, "i");
MODULE_PARM_DESC(eth_efmtc_crc_cfg, "Configuration for PTM TX/RX ethernet/efm-tc CRC");



/*
 * ####################################
 *              Definition
 * ####################################
 */


#define DUMP_SKB_LEN                            ~0



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  Network Operations
 */
static void ptm_setup(struct net_device *, int);
static struct net_device_stats *ptm_get_stats(struct net_device *);
static int ptm_open(struct net_device *);
static int ptm_stop(struct net_device *);
  static unsigned int ptm_poll(int, unsigned int);
  static int ptm_napi_poll(struct napi_struct *, int);
static int ptm_hard_start_xmit(struct sk_buff *, struct net_device *);
static int ptm_ioctl(struct net_device *, struct ifreq *, int);
static void ptm_tx_timeout(struct net_device *);

/*
 *  DSL Data LED
 */
static INLINE void adsl_led_flash(void);

/*
 *  buffer manage functions
 */
static INLINE struct sk_buff* alloc_skb_rx(void);
//static INLINE struct sk_buff* alloc_skb_tx(unsigned int);
static INLINE struct sk_buff *get_skb_rx_pointer(unsigned int);
static INLINE int get_tx_desc(unsigned int, unsigned int *);

/*
 *  Mailbox handler and signal function
 */
static INLINE int mailbox_rx_irq_handler(unsigned int);
static irqreturn_t mailbox_irq_handler(int, void *);
static INLINE void mailbox_signal(unsigned int, int);
#ifdef CONFIG_IFX_PTM_RX_TASKLET
  static void do_ptm_tasklet(unsigned long);
#endif

/*
 *  Debug Functions
 */
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  static void dump_skb(struct sk_buff *, u32, char *, int, int, int);
#else
  #define dump_skb(skb, len, title, port, ch, is_tx)    do {} while (0)
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static void skb_swap(struct sk_buff *);
#else
  #define skb_swap(skb)                                 do {} while (0)
#endif

/*
 *  Proc File Functions
 */
static INLINE void proc_file_create(void);
static INLINE void proc_file_delete(void);
static int proc_read_version(char *, char **, off_t, int, int *, void *);
static int proc_read_wanmib(char *, char **, off_t, int, int *, void *);
static int proc_write_wanmib(struct file *, const char *, unsigned long, void *);
#if defined(ENABLE_FW_PROC) && ENABLE_FW_PROC
  static int proc_read_genconf(char *, char **, off_t, int, int *, void *);
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static int proc_read_dbg(char *, char **, off_t, int, int *, void *);
  static int proc_write_dbg(struct file *, const char *, unsigned long, void *);
#endif

/*
 *  Proc Help Functions
 */
static INLINE int stricmp(const char *, const char *);
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static INLINE int strincmp(const char *, const char *, int);
#endif
static INLINE int ifx_ptm_version(char *);

/*
 *  Init & clean-up functions
 */
static INLINE void check_parameters(void);
static INLINE int init_priv_data(void);
static INLINE void clear_priv_data(void);
static INLINE void init_tables(void);

/*
 *  Exteranl Function
 */
#if defined(CONFIG_IFXMIPS_DSL_CPE_MEI) || defined(CONFIG_IFXMIPS_DSL_CPE_MEI_MODULE)
  extern int ifx_mei_atm_showtime_check(int *is_showtime, struct port_cell_info *port_cell, void **xdata_addr);
#else
  static inline int ifx_mei_atm_showtime_check(int *is_showtime, struct port_cell_info *port_cell, void **xdata_addr)
  {
    if ( is_showtime != NULL )
        *is_showtime = 0;
    return 0;
  }
#endif

/*
 *  External variable
 */
#if defined(CONFIG_IFXMIPS_DSL_CPE_MEI) || defined(CONFIG_IFXMIPS_DSL_CPE_MEI_MODULE)
  extern int (*ifx_mei_atm_showtime_enter)(struct port_cell_info *, void *);
  extern int (*ifx_mei_atm_showtime_exit)(void);
#else
  int (*ifx_mei_atm_showtime_enter)(struct port_cell_info *, void *) = NULL;
  EXPORT_SYMBOL(ifx_mei_atm_showtime_enter);
  int (*ifx_mei_atm_showtime_exit)(void) = NULL;
  EXPORT_SYMBOL(ifx_mei_atm_showtime_exit);
#endif



/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static struct ptm_priv_data g_ptm_priv_data;

static struct net_device_ops g_ptm_netdev_ops = {
    .ndo_get_stats       = ptm_get_stats,
    .ndo_open            = ptm_open,
    .ndo_stop            = ptm_stop,
    .ndo_start_xmit      = ptm_hard_start_xmit,
    .ndo_validate_addr   = eth_validate_addr,
    .ndo_set_mac_address = eth_mac_addr,
    .ndo_do_ioctl        = ptm_ioctl,
    .ndo_tx_timeout      = ptm_tx_timeout,
};

static struct net_device *g_net_dev[2] = {0};
static char *g_net_dev_name[2] = {"dsl0", "dslfast0"};

#ifdef CONFIG_IFX_PTM_RX_TASKLET
  static struct tasklet_struct g_ptm_tasklet[] = {
    {NULL, 0, ATOMIC_INIT(0), do_ptm_tasklet, 0},
    {NULL, 0, ATOMIC_INIT(0), do_ptm_tasklet, 1},
  };
#endif

unsigned int ifx_ptm_dbg_enable = DBG_ENABLE_MASK_ERR;

static struct proc_dir_entry* g_ptm_dir = NULL;

static int g_showtime = 0;



/*
 * ####################################
 *            Local Function
 * ####################################
 */

static void ptm_setup(struct net_device *dev, int ndev)
{
#if defined(CONFIG_IFXMIPS_DSL_CPE_MEI) || defined(CONFIG_IFXMIPS_DSL_CPE_MEI_MODULE)
    netif_carrier_off(dev);
#endif

    /*  hook network operations */
    dev->netdev_ops      = &g_ptm_netdev_ops;
    /* Allow up to 1508 bytes, for RFC4638 */
    dev->max_mtu         = ETH_DATA_LEN + 8;
    netif_napi_add(dev, &g_ptm_priv_data.itf[ndev].napi, ptm_napi_poll, 25);
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
    int ndev;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    g_ptm_priv_data.itf[ndev].stats.rx_errors   = WAN_MIB_TABLE[ndev].wrx_tccrc_err_pdu + WAN_MIB_TABLE[ndev].wrx_ethcrc_err_pdu;
    g_ptm_priv_data.itf[ndev].stats.rx_dropped  = WAN_MIB_TABLE[ndev].wrx_nodesc_drop_pdu + WAN_MIB_TABLE[ndev].wrx_len_violation_drop_pdu + (WAN_MIB_TABLE[ndev].wrx_correct_pdu - g_ptm_priv_data.itf[ndev].stats.rx_packets);

    return &g_ptm_priv_data.itf[ndev].stats;
}

static int ptm_open(struct net_device *dev)
{
    int ndev;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    napi_enable(&g_ptm_priv_data.itf[ndev].napi);

    IFX_REG_W32_MASK(0, 1 << ndev, MBOX_IGU1_IER);

    netif_start_queue(dev);

    return 0;
}

static int ptm_stop(struct net_device *dev)
{
    int ndev;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    IFX_REG_W32_MASK((1 << ndev) | (1 << (ndev + 16)), 0, MBOX_IGU1_IER);

    napi_disable(&g_ptm_priv_data.itf[ndev].napi);

    netif_stop_queue(dev);

    return 0;
}

static unsigned int ptm_poll(int ndev, unsigned int work_to_do)
{
    unsigned int work_done = 0;

    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    while ( work_done < work_to_do && WRX_DMA_CHANNEL_CONFIG(ndev)->vlddes > 0 ) {
        if ( mailbox_rx_irq_handler(ndev) < 0 )
            break;

        work_done++;
    }

    return work_done;
}
static int ptm_napi_poll(struct napi_struct *napi, int budget)
{
    int ndev;
    unsigned int work_done;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != napi->dev; ndev++ );

    work_done = ptm_poll(ndev, budget);

    //  interface down
    if ( !netif_running(napi->dev) ) {
        napi_complete(napi);
        return work_done;
    }

    //  no more traffic
    if ( WRX_DMA_CHANNEL_CONFIG(ndev)->vlddes == 0 ) {
        //  clear interrupt
        IFX_REG_W32_MASK(0, 1 << ndev, MBOX_IGU1_ISRC);
        //  double check
        if ( WRX_DMA_CHANNEL_CONFIG(ndev)->vlddes == 0 ) {
            napi_complete(napi);
            IFX_REG_W32_MASK(0, 1 << ndev, MBOX_IGU1_IER);
            return work_done;
        }
    }

    //  next round
    return work_done;
}

static int ptm_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int ndev;
    unsigned int f_full;
    int desc_base;
    register struct tx_descriptor reg_desc = {0};

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    if ( !g_showtime ) {
        err("not in showtime");
        goto PTM_HARD_START_XMIT_FAIL;
    }

    /*  allocate descriptor */
    desc_base = get_tx_desc(ndev, &f_full);
    if ( f_full ) {
        netif_trans_update(dev);
        netif_stop_queue(dev);

        IFX_REG_W32_MASK(0, 1 << (ndev + 16), MBOX_IGU1_ISRC);
        IFX_REG_W32_MASK(0, 1 << (ndev + 16), MBOX_IGU1_IER);
    }
    if ( desc_base < 0 )
        goto PTM_HARD_START_XMIT_FAIL;

    if ( g_ptm_priv_data.itf[ndev].tx_skb[desc_base] != NULL )
        dev_kfree_skb_any(g_ptm_priv_data.itf[ndev].tx_skb[desc_base]);
    g_ptm_priv_data.itf[ndev].tx_skb[desc_base] = skb;

    reg_desc.dataptr = (unsigned int)skb->data >> 2;
    reg_desc.datalen = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
    reg_desc.byteoff = (unsigned int)skb->data & (DATA_BUFFER_ALIGNMENT - 1);
    reg_desc.own     = 1;
    reg_desc.c       = 1;
    reg_desc.sop = reg_desc.eop = 1;

    /*  write discriptor to memory and write back cache */
    g_ptm_priv_data.itf[ndev].tx_desc[desc_base] = reg_desc;
    dma_cache_wback((unsigned long)skb->data, skb->len);
    wmb();

    dump_skb(skb, DUMP_SKB_LEN, (char *)__func__, ndev, ndev, 1);

    if ( (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP) ) {
        skb_swap(skb);
    }

    g_ptm_priv_data.itf[ndev].stats.tx_packets++;
    g_ptm_priv_data.itf[ndev].stats.tx_bytes += reg_desc.datalen;

    netif_trans_update(dev);
    mailbox_signal(ndev, 1);

    adsl_led_flash();

    return NETDEV_TX_OK;

PTM_HARD_START_XMIT_FAIL:
    dev_kfree_skb_any(skb);
    g_ptm_priv_data.itf[ndev].stats.tx_dropped++;
    return NETDEV_TX_OK;
}

static int ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    int ndev;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    switch ( cmd )
    {
    case IFX_PTM_MIB_CW_GET:
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxNoIdleCodewords   = WAN_MIB_TABLE[ndev].wrx_nonidle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxIdleCodewords     = WAN_MIB_TABLE[ndev].wrx_idle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxCodingViolation   = WAN_MIB_TABLE[ndev].wrx_err_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxNoIdleCodewords   = 0;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxIdleCodewords     = 0;
        break;
    case IFX_PTM_MIB_FRAME_GET:
        ((PTM_FRAME_MIB_T *)ifr->ifr_data)->RxCorrect   = WAN_MIB_TABLE[ndev].wrx_correct_pdu;
        ((PTM_FRAME_MIB_T *)ifr->ifr_data)->TC_CrcError = WAN_MIB_TABLE[ndev].wrx_tccrc_err_pdu;
        ((PTM_FRAME_MIB_T *)ifr->ifr_data)->RxDropped   = WAN_MIB_TABLE[ndev].wrx_nodesc_drop_pdu + WAN_MIB_TABLE[ndev].wrx_len_violation_drop_pdu;
        ((PTM_FRAME_MIB_T *)ifr->ifr_data)->TxSend      = WAN_MIB_TABLE[ndev].wtx_total_pdu;
        break;
    case IFX_PTM_CFG_GET:
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent = CFG_ETH_EFMTC_CRC->rx_eth_crc_present;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck   = CFG_ETH_EFMTC_CRC->rx_eth_crc_check;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck    = CFG_ETH_EFMTC_CRC->rx_tc_crc_check;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen      = CFG_ETH_EFMTC_CRC->rx_tc_crc_len;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen     = CFG_ETH_EFMTC_CRC->tx_eth_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen      = CFG_ETH_EFMTC_CRC->tx_tc_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen      = CFG_ETH_EFMTC_CRC->tx_tc_crc_len;
        break;
    case IFX_PTM_CFG_SET:
        CFG_ETH_EFMTC_CRC->rx_eth_crc_present   = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent ? 1 : 0;
        CFG_ETH_EFMTC_CRC->rx_eth_crc_check     = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck ? 1 : 0;
        if ( ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck && (((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen == 16 || ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen == 32) )
        {
            CFG_ETH_EFMTC_CRC->rx_tc_crc_check  = 1;
            CFG_ETH_EFMTC_CRC->rx_tc_crc_len    = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen;
        }
        else
        {
            CFG_ETH_EFMTC_CRC->rx_tc_crc_check  = 0;
            CFG_ETH_EFMTC_CRC->rx_tc_crc_len    = 0;
        }
        CFG_ETH_EFMTC_CRC->tx_eth_crc_gen       = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen ? 1 : 0;
        if ( ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen && (((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen == 16 || ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen == 32) )
        {
            CFG_ETH_EFMTC_CRC->tx_tc_crc_gen    = 1;
            CFG_ETH_EFMTC_CRC->tx_tc_crc_len    = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen;
        }
        else
        {
            CFG_ETH_EFMTC_CRC->tx_tc_crc_gen    = 0;
            CFG_ETH_EFMTC_CRC->tx_tc_crc_len    = 0;
        }
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void ptm_tx_timeout(struct net_device *dev)
{
    int ndev;

    for ( ndev = 0; ndev < ARRAY_SIZE(g_net_dev) && g_net_dev[ndev] != dev; ndev++ );
    ASSERT(ndev >= 0 && ndev < ARRAY_SIZE(g_net_dev), "ndev = %d (wrong value)", ndev);

    /*  disable TX irq, release skb when sending new packet */
    IFX_REG_W32_MASK(1 << (ndev + 16), 0, MBOX_IGU1_IER);

    /*  wake up TX queue    */
    netif_wake_queue(dev);

    return;
}

static INLINE void adsl_led_flash(void)
{
}

static INLINE struct sk_buff* alloc_skb_rx(void)
{
    struct sk_buff *skb;

    /*  allocate memroy including trailer and padding   */
    skb = dev_alloc_skb(rx_max_packet_size + RX_HEAD_MAC_ADDR_ALIGNMENT + DATA_BUFFER_ALIGNMENT);
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

#if 0
static INLINE struct sk_buff* alloc_skb_tx(unsigned int size)
{
    struct sk_buff *skb;

    /*  allocate memory including padding   */
    size = (size + DATA_BUFFER_ALIGNMENT - 1) & ~(DATA_BUFFER_ALIGNMENT - 1);
    skb = dev_alloc_skb(size + DATA_BUFFER_ALIGNMENT);
    /*  must be burst length alignment  */
    if ( skb != NULL )
        skb_reserve(skb, ~((unsigned int)skb->data + (DATA_BUFFER_ALIGNMENT - 1)) & (DATA_BUFFER_ALIGNMENT - 1));
    return skb;
}
#endif

static INLINE struct sk_buff *get_skb_rx_pointer(unsigned int dataptr)
{
    unsigned int skb_dataptr;
    struct sk_buff *skb;

    skb_dataptr = ((dataptr - 1) << 2) | KSEG1;
    skb = *(struct sk_buff **)skb_dataptr;

    ASSERT((unsigned int)skb >= KSEG0, "invalid skb - skb = %#08x, dataptr = %#08x", (unsigned int)skb, dataptr);
    ASSERT(((unsigned int)skb->data | KSEG1) == ((dataptr << 2) | KSEG1), "invalid skb - skb = %#08x, skb->data = %#08x, dataptr = %#08x", (unsigned int)skb, (unsigned int)skb->data, dataptr);

    return skb;
}

static INLINE int get_tx_desc(unsigned int itf, unsigned int *f_full)
{
    int desc_base = -1;
    struct ptm_itf *p_itf = &g_ptm_priv_data.itf[itf];

    //  assume TX is serial operation
    //  no protection provided

    *f_full = 1;

    if ( p_itf->tx_desc[p_itf->tx_desc_pos].own == 0 ) {
        desc_base = p_itf->tx_desc_pos;
        if ( ++(p_itf->tx_desc_pos) == dma_tx_descriptor_length )
            p_itf->tx_desc_pos = 0;
        if ( p_itf->tx_desc[p_itf->tx_desc_pos].own == 0 )
            *f_full = 0;
    }

    return desc_base;
}

static INLINE int mailbox_rx_irq_handler(unsigned int ch)   //  return: < 0 - descriptor not available, 0 - received one packet
{
    unsigned int ndev = ch;
    struct sk_buff *skb;
    struct sk_buff *new_skb;
    volatile struct rx_descriptor *desc;
    struct rx_descriptor reg_desc;
    int netif_rx_ret;

    desc = &g_ptm_priv_data.itf[ndev].rx_desc[g_ptm_priv_data.itf[ndev].rx_desc_pos];
    if ( desc->own || !desc->c )    //  if PP32 hold descriptor or descriptor not completed
        return -EAGAIN;
    if ( ++g_ptm_priv_data.itf[ndev].rx_desc_pos == dma_rx_descriptor_length )
        g_ptm_priv_data.itf[ndev].rx_desc_pos = 0;

    reg_desc = *desc;
    skb = get_skb_rx_pointer(reg_desc.dataptr);

    if ( !reg_desc.err ) {
        new_skb = alloc_skb_rx();
        if ( new_skb != NULL ) {
            skb_reserve(skb, reg_desc.byteoff);
            skb_put(skb, reg_desc.datalen);

            dump_skb(skb, DUMP_SKB_LEN, (char *)__func__, ndev, ndev, 0);

            //  parse protocol header
            skb->dev = g_net_dev[ndev];
            skb->protocol = eth_type_trans(skb, skb->dev);

            netif_rx_ret = netif_receive_skb(skb);

            if ( netif_rx_ret != NET_RX_DROP ) {
                g_ptm_priv_data.itf[ndev].stats.rx_packets++;
                g_ptm_priv_data.itf[ndev].stats.rx_bytes += reg_desc.datalen;
            }

            reg_desc.dataptr = ((unsigned int)new_skb->data >> 2) & 0x0FFFFFFF;
            reg_desc.byteoff = RX_HEAD_MAC_ADDR_ALIGNMENT;
        }
    }
    else
        reg_desc.err = 0;

    reg_desc.datalen = rx_max_packet_size;
    reg_desc.own     = 1;
    reg_desc.c       = 0;

    //  update descriptor
    *desc = reg_desc;
    wmb();

    mailbox_signal(ndev, 0);

    adsl_led_flash();

    return 0;
}

static irqreturn_t mailbox_irq_handler(int irq, void *dev_id)
{
    unsigned int isr;
    int i;

    isr = IFX_REG_R32(MBOX_IGU1_ISR);
    IFX_REG_W32(isr, MBOX_IGU1_ISRC);
    isr &= IFX_REG_R32(MBOX_IGU1_IER);

    while ( (i = __fls(isr)) >= 0 ) {
        isr ^= 1 << i;

        if ( i >= 16 ) {
            //  TX
            IFX_REG_W32_MASK(1 << i, 0, MBOX_IGU1_IER);
            i -= 16;
            if ( i < MAX_ITF_NUMBER )
                netif_wake_queue(g_net_dev[i]);
        }
        else {
            //  RX
#ifdef CONFIG_IFX_PTM_RX_INTERRUPT
            while ( WRX_DMA_CHANNEL_CONFIG(i)->vlddes > 0 )
                mailbox_rx_irq_handler(i);
#else
            IFX_REG_W32_MASK(1 << i, 0, MBOX_IGU1_IER);
            napi_schedule(&g_ptm_priv_data.itf[i].napi);
#endif
        }
    }

    return IRQ_HANDLED;
}

static INLINE void mailbox_signal(unsigned int itf, int is_tx)
{
    int count = 1000;

    if ( is_tx ) {
        while ( MBOX_IGU3_ISR_ISR(itf + 16) && count > 0 )
            count--;
        IFX_REG_W32(MBOX_IGU3_ISRS_SET(itf + 16), MBOX_IGU3_ISRS);
    }
    else {
        while ( MBOX_IGU3_ISR_ISR(itf) && count > 0 )
            count--;
        IFX_REG_W32(MBOX_IGU3_ISRS_SET(itf), MBOX_IGU3_ISRS);
    }

    ASSERT(count != 0, "MBOX_IGU3_ISR = 0x%08x", IFX_REG_R32(MBOX_IGU3_ISR));
}

#ifdef CONFIG_IFX_PTM_RX_TASKLET
static void do_ptm_tasklet(unsigned long arg)
{
    unsigned int work_to_do = 25;
    unsigned int work_done = 0;

    ASSERT(arg >= 0 && arg < ARRAY_SIZE(g_net_dev), "arg = %lu (wrong value)", arg);

    while ( work_done < work_to_do && WRX_DMA_CHANNEL_CONFIG(arg)->vlddes > 0 ) {
        if ( mailbox_rx_irq_handler(arg) < 0 )
            break;

        work_done++;
    }

    //  interface down
    if ( !netif_running(g_net_dev[arg]) )
        return;

    //  no more traffic
    if ( WRX_DMA_CHANNEL_CONFIG(arg)->vlddes == 0 ) {
        //  clear interrupt
        IFX_REG_W32_MASK(0, 1 << arg, MBOX_IGU1_ISRC);
        //  double check
        if ( WRX_DMA_CHANNEL_CONFIG(arg)->vlddes == 0 ) {
            IFX_REG_W32_MASK(0, 1 << arg, MBOX_IGU1_IER);
            return;
        }
    }

    //  next round
    tasklet_schedule(&g_ptm_tasklet[arg]);
}
#endif

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
static void dump_skb(struct sk_buff *skb, u32 len, char *title, int port, int ch, int is_tx)
{
    int i;

    if ( !(ifx_ptm_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX)) )
        return;

    if ( skb->len < len )
        len = skb->len;

    if ( len > rx_max_packet_size ) {
        printk("too big data length: skb = %08x, skb->data = %08x, skb->len = %d\n", (u32)skb, (u32)skb->data, skb->len);
        return;
    }

    if ( ch >= 0 )
        printk("%s (port %d, ch %d)\n", title, port, ch);
    else
        printk("%s\n", title);
    printk("  skb->data = %08X, skb->tail = %08X, skb->len = %d\n", (u32)skb->data, (u32)skb->tail, (int)skb->len);
    for ( i = 1; i <= len; i++ ) {
        if ( i % 16 == 1 )
            printk("  %4d:", i - 1);
        printk(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            printk("\n");
    }
    if ( (i - 1) % 16 != 0 )
        printk("\n");
}
#endif

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static void skb_swap(struct sk_buff *skb)
{
    unsigned char tmp[8];
    unsigned char *p = skb->data;

    if ( !(p[0] & 0x01) ) { //  bypass broadcast/multicast
        //  swap MAC
        memcpy(tmp, p, 6);
        memcpy(p, p + 6, 6);
        memcpy(p + 6, tmp, 6);
        p += 12;

        //  bypass VLAN
        while ( p[0] == 0x81 && p[1] == 0x00 )
            p += 4;

        //  IP
        if ( p[0] == 0x08 && p[1] == 0x00 ) {
            p += 14;
            memcpy(tmp, p, 4);
            memcpy(p, p + 4, 4);
            memcpy(p + 4, tmp, 4);
            p += 8;
        }

        dma_cache_wback((unsigned long)skb->data, (unsigned long)p - (unsigned long)skb->data);
    }
}
#endif

static INLINE void proc_file_create(void)
{
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
    struct proc_dir_entry *res;

    g_ptm_dir = proc_mkdir("driver/ifx_ptm", NULL);

    create_proc_read_entry("version",
                            0,
                            g_ptm_dir,
                            proc_read_version,
                            NULL);

    res = create_proc_entry("wanmib",
                            0,
                            g_ptm_dir);
    if ( res != NULL ) {
        res->read_proc  = proc_read_wanmib;
        res->write_proc = proc_write_wanmib;
    }

#if defined(ENABLE_FW_PROC) && ENABLE_FW_PROC
    create_proc_read_entry("genconf",
                            0,
                            g_ptm_dir,
                            proc_read_genconf,
                            NULL);

  #ifdef CONFIG_AR9
    create_proc_read_entry("regs",
                            0,
                            g_ptm_dir,
                            ifx_ptm_proc_read_regs,
                            NULL);
  #endif
#endif

    res = create_proc_entry("dbg",
                            0,
                            g_ptm_dir);
    if ( res != NULL ) {
        res->read_proc  = proc_read_dbg;
        res->write_proc = proc_write_dbg;
    }
#endif
}

static INLINE void proc_file_delete(void)
{
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
    remove_proc_entry("dbg", g_ptm_dir);
#endif

#if defined(ENABLE_FW_PROC) && ENABLE_FW_PROC
  #ifdef CONFIG_AR9
    remove_proc_entry("regs", g_ptm_dir);
  #endif

    remove_proc_entry("genconf", g_ptm_dir);
#endif

    remove_proc_entry("wanmib", g_ptm_dir);

    remove_proc_entry("version", g_ptm_dir);

    remove_proc_entry("driver/ifx_ptm", NULL);
}

static int proc_read_version(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    len += ifx_ptm_version(buf + len);

    if ( offset >= len ) {
        *start = buf;
        *eof = 1;
        return 0;
    }
    *start = buf + offset;
    if ( (len -= offset) > count )
        return count;
    *eof = 1;
    return len;
}

static int proc_read_wanmib(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    int i;
    char *title[] = {
        "dsl0\n",
        "dslfast0\n"
    };

    for ( i = 0; i < ARRAY_SIZE(title); i++ ) {
        len += sprintf(page + off + len, title[i]);
        len += sprintf(page + off + len, "  wrx_correct_pdu            = %d\n", WAN_MIB_TABLE[i].wrx_correct_pdu);
        len += sprintf(page + off + len, "  wrx_correct_pdu_bytes      = %d\n", WAN_MIB_TABLE[i].wrx_correct_pdu_bytes);
        len += sprintf(page + off + len, "  wrx_tccrc_err_pdu          = %d\n", WAN_MIB_TABLE[i].wrx_tccrc_err_pdu);
        len += sprintf(page + off + len, "  wrx_tccrc_err_pdu_bytes    = %d\n", WAN_MIB_TABLE[i].wrx_tccrc_err_pdu_bytes);
        len += sprintf(page + off + len, "  wrx_ethcrc_err_pdu         = %d\n", WAN_MIB_TABLE[i].wrx_ethcrc_err_pdu);
        len += sprintf(page + off + len, "  wrx_ethcrc_err_pdu_bytes   = %d\n", WAN_MIB_TABLE[i].wrx_ethcrc_err_pdu_bytes);
        len += sprintf(page + off + len, "  wrx_nodesc_drop_pdu        = %d\n", WAN_MIB_TABLE[i].wrx_nodesc_drop_pdu);
        len += sprintf(page + off + len, "  wrx_len_violation_drop_pdu = %d\n", WAN_MIB_TABLE[i].wrx_len_violation_drop_pdu);
        len += sprintf(page + off + len, "  wrx_idle_bytes             = %d\n", WAN_MIB_TABLE[i].wrx_idle_bytes);
        len += sprintf(page + off + len, "  wrx_nonidle_cw             = %d\n", WAN_MIB_TABLE[i].wrx_nonidle_cw);
        len += sprintf(page + off + len, "  wrx_idle_cw                = %d\n", WAN_MIB_TABLE[i].wrx_idle_cw);
        len += sprintf(page + off + len, "  wrx_err_cw                 = %d\n", WAN_MIB_TABLE[i].wrx_err_cw);
        len += sprintf(page + off + len, "  wtx_total_pdu              = %d\n", WAN_MIB_TABLE[i].wtx_total_pdu);
        len += sprintf(page + off + len, "  wtx_total_bytes            = %d\n", WAN_MIB_TABLE[i].wtx_total_bytes);
    }

    *eof = 1;

    return len;
}

static int proc_write_wanmib(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( stricmp(p, "clear") == 0 || stricmp(p, "clean") == 0 ) {
        for ( i = 0; i < 2; i++ )
            memset((void*)&WAN_MIB_TABLE[i], 0, sizeof(WAN_MIB_TABLE[i]));
    }

    return count;
}

#if defined(ENABLE_FW_PROC) && ENABLE_FW_PROC

static int proc_read_genconf(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    int len_max = off + count;
    char *pstr;
    char str[2048];
    int llen = 0;
    int i;
    unsigned long bit;

    pstr = *start = page;

    __sync();

    llen += sprintf(str + llen, "CFG_WAN_WRDES_DELAY (0x%08X): %d\n", (unsigned int)CFG_WAN_WRDES_DELAY, IFX_REG_R32(CFG_WAN_WRDES_DELAY));
    llen += sprintf(str + llen, "CFG_WRX_DMACH_ON    (0x%08X):", (unsigned int)CFG_WRX_DMACH_ON);
    for ( i = 0, bit = 1; i < MAX_RX_DMA_CHANNEL_NUMBER; i++, bit <<= 1 )
        llen += sprintf(str + llen, " %d - %s", i, (IFX_REG_R32(CFG_WRX_DMACH_ON) & bit) ? "on " : "off");
    llen += sprintf(str + llen, "\n");
    llen += sprintf(str + llen, "CFG_WTX_DMACH_ON    (0x%08X):", (unsigned int)CFG_WTX_DMACH_ON);
    for ( i = 0, bit = 1; i < MAX_TX_DMA_CHANNEL_NUMBER; i++, bit <<= 1 )
        llen += sprintf(str + llen, " %d - %s", i, (IFX_REG_R32(CFG_WTX_DMACH_ON) & bit) ? "on " : "off");
    llen += sprintf(str + llen, "\n");
    llen += sprintf(str + llen, "CFG_WRX_LOOK_BITTH  (0x%08X): %d\n", (unsigned int)CFG_WRX_LOOK_BITTH, IFX_REG_R32(CFG_WRX_LOOK_BITTH));
    llen += sprintf(str + llen, "CFG_ETH_EFMTC_CRC   (0x%08X): rx_tc_crc_len    - %2d,  rx_tc_crc_check    - %s\n", (unsigned int)CFG_ETH_EFMTC_CRC, CFG_ETH_EFMTC_CRC->rx_tc_crc_len, CFG_ETH_EFMTC_CRC->rx_tc_crc_check ? " on" : "off");
    llen += sprintf(str + llen, "                                  rx_eth_crc_check - %s, rx_eth_crc_present - %s\n",   CFG_ETH_EFMTC_CRC->rx_eth_crc_check ? " on" : "off", CFG_ETH_EFMTC_CRC->rx_eth_crc_present ? " on" : "off");
    llen += sprintf(str + llen, "                                  tx_tc_crc_len    - %2d,  tx_tc_crc_gen      - %s\n", CFG_ETH_EFMTC_CRC->tx_tc_crc_len, CFG_ETH_EFMTC_CRC->tx_tc_crc_gen ? " on" : "off");
    llen += sprintf(str + llen, "                                  tx_eth_crc_gen   - %s\n", CFG_ETH_EFMTC_CRC->tx_eth_crc_gen ? " on" : "off");

    llen += sprintf(str + llen, "RX Port:\n");
    for ( i = 0; i < MAX_RX_DMA_CHANNEL_NUMBER; i++ )
        llen += sprintf(str + llen, "  %d (0x%08X). mfs - %5d, dmach - %d, local_state - %d, partner_state - %d\n", i, (unsigned int)WRX_PORT_CONFIG(i), WRX_PORT_CONFIG(i)->mfs, WRX_PORT_CONFIG(i)->dmach, WRX_PORT_CONFIG(i)->local_state, WRX_PORT_CONFIG(i)->partner_state);
    llen += sprintf(str + llen, "RX DMA Channel:\n");
    for ( i = 0; i < MAX_RX_DMA_CHANNEL_NUMBER; i++ )
        llen += sprintf(str + llen, "  %d (0x%08X). desba - 0x%08X (0x%08X), deslen - %d, vlddes - %d\n", i, (unsigned int)WRX_DMA_CHANNEL_CONFIG(i), WRX_DMA_CHANNEL_CONFIG(i)->desba, ((unsigned int)WRX_DMA_CHANNEL_CONFIG(i)->desba << 2) | KSEG1, WRX_DMA_CHANNEL_CONFIG(i)->deslen, WRX_DMA_CHANNEL_CONFIG(i)->vlddes);

    llen += sprintf(str + llen, "TX Port:\n");
    for ( i = 0; i < MAX_TX_DMA_CHANNEL_NUMBER; i++ )
        llen += sprintf(str + llen, "  %d (0x%08X). tx_cwth2 - %d, tx_cwth1 - %d\n", i, (unsigned int)WTX_PORT_CONFIG(i), WTX_PORT_CONFIG(i)->tx_cwth2, WTX_PORT_CONFIG(i)->tx_cwth1);
    llen += sprintf(str + llen, "TX DMA Channel:\n");
    for ( i = 0; i < MAX_TX_DMA_CHANNEL_NUMBER; i++ )
        llen += sprintf(str + llen, "  %d (0x%08X). desba - 0x%08X (0x%08X), deslen - %d, vlddes - %d\n", i, (unsigned int)WTX_DMA_CHANNEL_CONFIG(i), WTX_DMA_CHANNEL_CONFIG(i)->desba, ((unsigned int)WTX_DMA_CHANNEL_CONFIG(i)->desba << 2) | KSEG1, WTX_DMA_CHANNEL_CONFIG(i)->deslen, WTX_DMA_CHANNEL_CONFIG(i)->vlddes);

    if ( len <= off && len + llen > off )
    {
        memcpy(pstr, str + off - len, len + llen - off);
        pstr += len + llen - off;
    }
    else if ( len > off )
    {
        memcpy(pstr, str, llen);
        pstr += llen;
    }
    len += llen;
    if ( len >= len_max )
        goto PROC_READ_GENCONF_OVERRUN_END;

    *eof = 1;

    return len - off;

PROC_READ_GENCONF_OVERRUN_END:
    return len - llen - off;
}

#endif  //  defined(ENABLE_FW_PROC) && ENABLE_FW_PROC

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC

static int proc_read_dbg(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + off + len, "error print      - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_ERR)              ? "enabled" : "disabled");
    len += sprintf(page + off + len, "debug print      - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    len += sprintf(page + off + len, "assert           - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    len += sprintf(page + off + len, "dump rx skb      - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    len += sprintf(page + off + len, "dump tx skb      - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");
    len += sprintf(page + off + len, "mac swap         - %s\n", (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP)         ? "enabled" : "disabled");

    *eof = 1;

    return len;
}

static int proc_write_dbg(struct file *file, const char *buf, unsigned long count, void *data)
{
    static const char *dbg_enable_mask_str[] = {
        " error print",
        " err",
        " debug print",
        " dbg",
        " assert",
        " assert",
        " dump rx skb",
        " rx",
        " dump tx skb",
        " tx",
        " dump init",
        " init",
        " dump qos",
        " qos",
        " mac swap",
        " swap",
        " all"
    };
    static const int dbg_enable_mask_str_len[] = {
        12, 4,
        12, 4,
        7,  7,
        12, 3,
        12, 3,
        10, 5,
        9,  4,
        9,  5,
        4
    };
    unsigned int dbg_enable_mask[] = {
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_SKB_RX,
        DBG_ENABLE_MASK_DUMP_SKB_TX,
        DBG_ENABLE_MASK_DUMP_INIT,
        DBG_ENABLE_MASK_DUMP_QOS,
        DBG_ENABLE_MASK_MAC_SWAP,
        DBG_ENABLE_MASK_ALL
    };

    char str[2048];
    char *p;

    int len, rlen;

    int f_enable = 0;
    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    //  debugging feature for enter/leave showtime
    if ( strincmp(p, "enter", 5) == 0 && ifx_mei_atm_showtime_enter != NULL )
        ifx_mei_atm_showtime_enter(NULL, NULL);
    else if ( strincmp(p, "leave", 5) == 0 && ifx_mei_atm_showtime_exit != NULL )
        ifx_mei_atm_showtime_exit();

    if ( strincmp(p, "enable", 6) == 0 ) {
        p += 6;
        f_enable = 1;
    }
    else if ( strincmp(p, "disable", 7) == 0 ) {
        p += 7;
        f_enable = -1;
    }
    else if ( strincmp(p, "help", 4) == 0 || *p == '?' ) {
        printk("echo <enable/disable> [err/dbg/assert/rx/tx/init/qos/swap/all] > /proc/driver/ifx_ptm/dbg\n");
    }

    if ( f_enable ) {
        if ( *p == 0 ) {
            if ( f_enable > 0 )
                ifx_ptm_dbg_enable |= DBG_ENABLE_MASK_ALL & ~DBG_ENABLE_MASK_MAC_SWAP;
            else
                ifx_ptm_dbg_enable &= ~DBG_ENABLE_MASK_ALL | DBG_ENABLE_MASK_MAC_SWAP;
        }
        else {
            do {
                for ( i = 0; i < ARRAY_SIZE(dbg_enable_mask_str); i++ )
                    if ( strincmp(p, dbg_enable_mask_str[i], dbg_enable_mask_str_len[i]) == 0 ) {
                        if ( f_enable > 0 )
                            ifx_ptm_dbg_enable |= dbg_enable_mask[i >> 1];
                        else
                            ifx_ptm_dbg_enable &= ~dbg_enable_mask[i >> 1];
                        p += dbg_enable_mask_str_len[i];
                        break;
                    }
            } while ( i < ARRAY_SIZE(dbg_enable_mask_str) );
        }
    }

    return count;
}

#endif  //  defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC

static INLINE int stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static INLINE int strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}
#endif

static INLINE int ifx_ptm_version(char *buf)
{
    int len = 0;
    unsigned int major, minor;

    ifx_ptm_get_fw_ver(&major, &minor);

    len += sprintf(buf + len, "PTM %d.%d.%d", IFX_PTM_VER_MAJOR, IFX_PTM_VER_MID, IFX_PTM_VER_MINOR);
    len += sprintf(buf + len, "    PTM (E1) firmware version %d.%d\n", major, minor);

    return len;
}

static INLINE void check_parameters(void)
{
    /*  There is a delay between PPE write descriptor and descriptor is       */
    /*  really stored in memory. Host also has this delay when writing        */
    /*  descriptor. So PPE will use this value to determine if the write      */
    /*  operation makes effect.                                               */
    if ( write_desc_delay < 0 )
        write_desc_delay = 0;

    /*  Because of the limitation of length field in descriptors, the packet  */
    /*  size could not be larger than 64K minus overhead size.                */
    if ( rx_max_packet_size < ETH_MIN_FRAME_LENGTH )
        rx_max_packet_size = ETH_MIN_FRAME_LENGTH;
    else if ( rx_max_packet_size > 65536 - 1 )
        rx_max_packet_size = 65536 - 1;

    if ( dma_rx_descriptor_length < 2 )
        dma_rx_descriptor_length = 2;
    if ( dma_tx_descriptor_length < 2 )
        dma_tx_descriptor_length = 2;
}

static INLINE int init_priv_data(void)
{
    void *p;
    int i;
    struct rx_descriptor rx_desc = {0};
    struct sk_buff *skb;
    volatile struct rx_descriptor *p_rx_desc;
    volatile struct tx_descriptor *p_tx_desc;
    struct sk_buff **ppskb;

    //  clear ptm private data structure
    memset(&g_ptm_priv_data, 0, sizeof(g_ptm_priv_data));

    //  allocate memory for RX descriptors
    p = kzalloc(MAX_ITF_NUMBER * dma_rx_descriptor_length * sizeof(struct rx_descriptor) + DESC_ALIGNMENT, GFP_KERNEL);
    if ( p == NULL )
        return -1;
    dma_cache_inv((unsigned long)p, MAX_ITF_NUMBER * dma_rx_descriptor_length * sizeof(struct rx_descriptor) + DESC_ALIGNMENT);
    g_ptm_priv_data.rx_desc_base = p;
    //p = (void *)((((unsigned int)p + DESC_ALIGNMENT - 1) & ~(DESC_ALIGNMENT - 1)) | KSEG1);

    //  allocate memory for TX descriptors
    p = kzalloc(MAX_ITF_NUMBER * dma_tx_descriptor_length * sizeof(struct tx_descriptor) + DESC_ALIGNMENT, GFP_KERNEL);
    if ( p == NULL )
        return -1;
    dma_cache_inv((unsigned long)p, MAX_ITF_NUMBER * dma_tx_descriptor_length * sizeof(struct tx_descriptor) + DESC_ALIGNMENT);
    g_ptm_priv_data.tx_desc_base = p;

    //  allocate memroy for TX skb pointers
    p = kzalloc(MAX_ITF_NUMBER * dma_tx_descriptor_length * sizeof(struct sk_buff *) + 4, GFP_KERNEL);
    if ( p == NULL )
        return -1;
    dma_cache_wback_inv((unsigned long)p, MAX_ITF_NUMBER * dma_tx_descriptor_length * sizeof(struct sk_buff *) + 4);
    g_ptm_priv_data.tx_skb_base = p;

    p_rx_desc = (volatile struct rx_descriptor *)((((unsigned int)g_ptm_priv_data.rx_desc_base + DESC_ALIGNMENT - 1) & ~(DESC_ALIGNMENT - 1)) | KSEG1);
    p_tx_desc = (volatile struct tx_descriptor *)((((unsigned int)g_ptm_priv_data.tx_desc_base + DESC_ALIGNMENT - 1) & ~(DESC_ALIGNMENT - 1)) | KSEG1);
    ppskb = (struct sk_buff **)(((unsigned int)g_ptm_priv_data.tx_skb_base + 3) & ~3);
    for ( i = 0; i < MAX_ITF_NUMBER; i++ ) {
        g_ptm_priv_data.itf[i].rx_desc = &p_rx_desc[i * dma_rx_descriptor_length];
        g_ptm_priv_data.itf[i].tx_desc = &p_tx_desc[i * dma_tx_descriptor_length];
        g_ptm_priv_data.itf[i].tx_skb = &ppskb[i * dma_tx_descriptor_length];
    }

    rx_desc.own     = 1;
    rx_desc.c       = 0;
    rx_desc.sop     = 1;
    rx_desc.eop     = 1;
    rx_desc.byteoff = RX_HEAD_MAC_ADDR_ALIGNMENT;
    rx_desc.id      = 0;
    rx_desc.err     = 0;
    rx_desc.datalen = rx_max_packet_size;
    for ( i = 0; i < MAX_ITF_NUMBER * dma_rx_descriptor_length; i++ ) {
        skb = alloc_skb_rx();
        if ( skb == NULL )
            return -1;
        rx_desc.dataptr = ((unsigned int)skb->data >> 2) & 0x0FFFFFFF;
        p_rx_desc[i] = rx_desc;
    }

    return 0;
}

static INLINE void clear_priv_data(void)
{
    int i, j;
    struct sk_buff *skb;

    for ( i = 0; i < MAX_ITF_NUMBER; i++ ) {
        if ( g_ptm_priv_data.itf[i].tx_skb != NULL ) {
            for ( j = 0; j < dma_tx_descriptor_length; j++ )
                if ( g_ptm_priv_data.itf[i].tx_skb[j] != NULL )
                    dev_kfree_skb_any(g_ptm_priv_data.itf[i].tx_skb[j]);
        }
        if ( g_ptm_priv_data.itf[i].rx_desc != NULL ) {
            for ( j = 0; j < dma_rx_descriptor_length; j++ ) {
                if ( g_ptm_priv_data.itf[i].rx_desc[j].sop || g_ptm_priv_data.itf[i].rx_desc[j].eop ) {    //  descriptor initialized
                    skb = get_skb_rx_pointer(g_ptm_priv_data.itf[i].rx_desc[j].dataptr);
                    dev_kfree_skb_any(skb);
                }
            }
        }
    }

    if ( g_ptm_priv_data.rx_desc_base != NULL )
        kfree(g_ptm_priv_data.rx_desc_base);

    if ( g_ptm_priv_data.tx_desc_base != NULL )
        kfree(g_ptm_priv_data.tx_desc_base);

    if ( g_ptm_priv_data.tx_skb_base != NULL )
        kfree(g_ptm_priv_data.tx_skb_base);
}

static INLINE void init_tables(void)
{
    int i;
    volatile unsigned int *p;
    struct wrx_dma_channel_config rx_config = {0};
    struct wtx_dma_channel_config tx_config = {0};
    struct wrx_port_cfg_status    rx_port_cfg = { 0 };
    struct wtx_port_cfg           tx_port_cfg = { 0 };

    /*
     *  CDM Block 1
     */
    IFX_REG_W32(CDM_CFG_RAM1_SET(0x00) | CDM_CFG_RAM0_SET(0x00), CDM_CFG);  //  CDM block 1 must be data memory and mapped to 0x5000 (dword addr)
    p = CDM_DATA_MEMORY(0, 0);                                              //  Clear CDM block 1
    for ( i = 0; i < CDM_DATA_MEMORY_DWLEN; i++, p++ )
        IFX_REG_W32(0, p);

    /*
     *  General Registers
     */
    IFX_REG_W32(write_desc_delay, CFG_WAN_WRDES_DELAY);
    IFX_REG_W32((1 << MAX_RX_DMA_CHANNEL_NUMBER) - 1, CFG_WRX_DMACH_ON);
    IFX_REG_W32((1 << MAX_TX_DMA_CHANNEL_NUMBER) - 1, CFG_WTX_DMACH_ON);

    IFX_REG_W32(8, CFG_WRX_LOOK_BITTH); // WAN RX EFM-TC Looking Threshold

    IFX_REG_W32(eth_efmtc_crc_cfg, CFG_ETH_EFMTC_CRC);

    /*
     *  WRX DMA Channel Configuration Table
     */
    rx_config.deslen = dma_rx_descriptor_length;
    rx_port_cfg.mfs = ETH_MAX_FRAME_LENGTH;
    rx_port_cfg.local_state = 0;     // looking for sync
    rx_port_cfg.partner_state = 0;   // parter receiver is out of sync

    for ( i = 0; i < MAX_RX_DMA_CHANNEL_NUMBER; i++ ) {
        rx_config.desba = ((unsigned int)g_ptm_priv_data.itf[i].rx_desc >> 2) & 0x0FFFFFFF;
        *WRX_DMA_CHANNEL_CONFIG(i) = rx_config;

        rx_port_cfg.dmach = i;
        *WRX_PORT_CONFIG(i) = rx_port_cfg;
    }

    /*
     *  WTX DMA Channel Configuration Table
     */
    tx_config.deslen = dma_tx_descriptor_length;
    tx_port_cfg.tx_cwth1 = 5;
    tx_port_cfg.tx_cwth2 = 4;

    for ( i = 0; i < MAX_TX_DMA_CHANNEL_NUMBER; i++ ) {
        tx_config.desba = ((unsigned int)g_ptm_priv_data.itf[i].tx_desc >> 2) & 0x0FFFFFFF;
        *WTX_DMA_CHANNEL_CONFIG(i) = tx_config;

        *WTX_PORT_CONFIG(i) = tx_port_cfg;
    }
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */

static int ptm_showtime_enter(struct port_cell_info *port_cell, void *xdata_addr)
{
    int i;

    g_showtime = 1;

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
        netif_carrier_on(g_net_dev[i]);

    printk("enter showtime\n");

    return 0;
}

static int ptm_showtime_exit(void)
{
    int i;

    if ( !g_showtime )
        return -1;

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
        netif_carrier_off(g_net_dev[i]);

    g_showtime = 0;

    printk("leave showtime\n");

    return 0;
}


static const struct of_device_id ltq_ptm_match[] = {
#ifdef CONFIG_DANUBE
       { .compatible = "lantiq,ppe-danube", .data = NULL },
#elif defined CONFIG_AMAZON_SE
       { .compatible = "lantiq,ppe-ase", .data = NULL },
#elif defined CONFIG_AR9
       { .compatible = "lantiq,ppe-arx100", .data = NULL },
#elif defined CONFIG_VR9
       { .compatible = "lantiq,ppe-xrx200", .data = NULL },
#endif
       {},
};
MODULE_DEVICE_TABLE(of, ltq_ptm_match);

/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

/*
 *  Description:
 *    Initialize global variables, PP32, comunication structures, register IRQ
 *    and register device.
 *  Input:
 *    none
 *  Output:
 *    0    --- successful
 *    else --- failure, usually it is negative value of error code
 */
static int ltq_ptm_probe(struct platform_device *pdev)
{
    int ret;
    struct port_cell_info port_cell = {0};
    void *xdata_addr = NULL;
    int i;
    char ver_str[256];

    check_parameters();

    ret = init_priv_data();
    if ( ret != 0 ) {
        err("INIT_PRIV_DATA_FAIL");
        goto INIT_PRIV_DATA_FAIL;
    }

    ifx_ptm_init_chip(pdev);
    init_tables();

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
    ret = request_irq(PPE_MAILBOX_IGU1_INT, mailbox_irq_handler, 0, "ptm_mailbox_isr", &g_ptm_priv_data);
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
    IFX_REG_W32(0, MBOX_IGU1_IER);
    IFX_REG_W32(~0, MBOX_IGU1_ISRC);

    enable_irq(PPE_MAILBOX_IGU1_INT);


    proc_file_create();

    port_cell.port_num = 1;
    ifx_mei_atm_showtime_check(&g_showtime, &port_cell, &xdata_addr);
    if ( g_showtime ) {
	ptm_showtime_enter(&port_cell, &xdata_addr);
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
INIT_PRIV_DATA_FAIL:
    clear_priv_data();
    printk("ifxmips_ptm: PTM init failed\n");
    return ret;
}

/*
 *  Description:
 *    Release memory, free IRQ, and deregister device.
 *  Input:
 *    none
 *  Output:
 *   none
 */
static int ltq_ptm_remove(struct platform_device *pdev)
{
    int i;

    ifx_mei_atm_showtime_enter = NULL;
    ifx_mei_atm_showtime_exit  = NULL;

    proc_file_delete();


    ifx_pp32_stop(0);

    free_irq(PPE_MAILBOX_IGU1_INT, &g_ptm_priv_data);

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ )
        unregister_netdev(g_net_dev[i]);

    for ( i = 0; i < ARRAY_SIZE(g_net_dev); i++ ) {
        free_netdev(g_net_dev[i]);
        g_net_dev[i] = NULL;
    }

    ifx_ptm_uninit_chip();

    clear_priv_data();

    return 0;
}

static struct platform_driver ltq_ptm_driver = {
       .probe = ltq_ptm_probe,
       .remove = ltq_ptm_remove,
       .driver = {
               .name = "ptm",
               .owner = THIS_MODULE,
               .of_match_table = ltq_ptm_match,
       },
};

module_platform_driver(ltq_ptm_driver);

MODULE_LICENSE("GPL");
