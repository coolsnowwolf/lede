// SPDX-License-Identifier: GPL-2.0-only
/*
################################################################################
#
# r8125 is the Linux device driver released for Realtek 2.5Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2022 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

/*
 * This driver is modified from r8169.c in Linux kernel 2.6.18
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/ip.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#endif
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/completion.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
#include <linux/pci-aspm.h>
#endif
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
#include <linux/prefetch.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define dev_printk(A,B,fmt,args...) printk(A fmt,##args)
#else
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
#include <linux/mdio.h>
#endif

#include <asm/io.h>
#include <asm/irq.h>

#include "r8125.h"
#include "rtl_eeprom.h"
#include "rtltool.h"
#include "r8125_firmware.h"

#ifdef ENABLE_R8125_PROCFS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#define FIRMWARE_8125A_3	"rtl_nic/rtl8125a-3.fw"
#define FIRMWARE_8125B_2	"rtl_nic/rtl8125b-2.fw"
#define FIRMWARE_8126A_1	"rtl_nic/rtl8126a-1.fw"

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC. */
static const int multicast_filter_limit = 32;

static const struct {
        const char *name;
        const char *fw_name;
} rtl_chip_fw_infos[] = {
        /* PCI-E devices. */
        [CFG_METHOD_2] = {"RTL8125A"				},
        [CFG_METHOD_3] = {"RTL8125A",		FIRMWARE_8125A_3},
        [CFG_METHOD_4] = {"RTL8125B",                       },
        [CFG_METHOD_5] = {"RTL8125B",		FIRMWARE_8125B_2},
        [CFG_METHOD_6] = {"RTL8168KB",		FIRMWARE_8125A_3},
        [CFG_METHOD_7] = {"RTL8168KB",		FIRMWARE_8125B_2},
        [CFG_METHOD_8] = {"RTL8126A",		FIRMWARE_8126A_1},
        [CFG_METHOD_DEFAULT] = {"Unknown",                  },
};

#define _R(NAME,MAC,RCR,MASK,JumFrameSz) \
    { .name = NAME, .mcfg = MAC, .RCR_Cfg = RCR, .RxConfigMask = MASK, .jumbo_frame_sz = JumFrameSz }

static const struct {
        const char *name;
        u8 mcfg;
        u32 RCR_Cfg;
        u32 RxConfigMask;   /* Clears the bits supported by this chip */
        u32 jumbo_frame_sz;
} rtl_chip_info[] = {
        _R("RTL8125A",
        CFG_METHOD_2,
        BIT_30 | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8125A",
        CFG_METHOD_3,
        BIT_30 | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8125B",
        CFG_METHOD_4,
        BIT_30 | RxCfg_pause_slot_en | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8125B",
        CFG_METHOD_5,
        BIT_30 | RxCfg_pause_slot_en | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8168KB",
        CFG_METHOD_6,
        BIT_30 | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8168KB",
        CFG_METHOD_7,
        BIT_30 | RxCfg_pause_slot_en | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("RTL8126A",
        CFG_METHOD_8,
        BIT_30 | RxCfg_pause_slot_en | EnableInnerVlan | EnableOuterVlan | (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_9k),

        _R("Unknown",
        CFG_METHOD_DEFAULT,
        (RX_DMA_BURST << RxCfgDMAShift),
        0xff7e5880,
        Jumbo_Frame_1k)
};
#undef _R


#ifndef PCI_VENDOR_ID_DLINK
#define PCI_VENDOR_ID_DLINK 0x1186
#endif

static struct pci_device_id rtl8125_pci_tbl[] = {
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8125), },
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8162), },
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x3000), },
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8126), },
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x5000), },
        {0,},
};

MODULE_DEVICE_TABLE(pci, rtl8125_pci_tbl);

static int use_dac = 1;
static int timer_count = 0x2600;
static int timer_count_v2 = (0x2600 / 0x100);

static struct {
        u32 msg_enable;
} debug = { -1 };

static unsigned int speed_mode = SPEED_5000;
static unsigned int duplex_mode = DUPLEX_FULL;
static unsigned int autoneg_mode = AUTONEG_ENABLE;
static unsigned int advertising_mode =  ADVERTISED_10baseT_Half |
                                        ADVERTISED_10baseT_Full |
                                        ADVERTISED_100baseT_Half |
                                        ADVERTISED_100baseT_Full |
                                        ADVERTISED_1000baseT_Half |
                                        ADVERTISED_1000baseT_Full |
                                        ADVERTISED_2500baseX_Full;
#ifdef CONFIG_ASPM
static int aspm = 1;
#else
static int aspm = 0;
#endif
#ifdef ENABLE_S5WOL
static int s5wol = 1;
#else
static int s5wol = 0;
#endif
#ifdef ENABLE_S5_KEEP_CURR_MAC
static int s5_keep_curr_mac = 1;
#else
static int s5_keep_curr_mac = 0;
#endif
#ifdef ENABLE_EEE
static int eee_enable = 1;
#else
static int eee_enable = 0;
#endif
#ifdef CONFIG_SOC_LAN
static ulong hwoptimize = HW_PATCH_SOC_LAN;
#else
static ulong hwoptimize = 0;
#endif
#ifdef ENABLE_S0_MAGIC_PACKET
static int s0_magic_packet = 1;
#else
static int s0_magic_packet = 0;
#endif
#ifdef ENABLE_TX_NO_CLOSE
static int tx_no_close_enable = 1;
#else
static int tx_no_close_enable = 0;
#endif
#ifdef ENABLE_PTP_MASTER_MODE
static int enable_ptp_master_mode = 1;
#else
static int enable_ptp_master_mode = 0;
#endif
#ifdef DISABLE_PM_SUPPORT
static int disable_pm_support = 1;
#else
static int disable_pm_support = 0;
#endif

#ifdef ENABLE_DOUBLE_VLAN
static int enable_double_vlan = 1;
#else
static int enable_double_vlan = 0;
#endif

MODULE_AUTHOR("Realtek and the Linux r8125 crew <netdev@vger.kernel.org>");
MODULE_DESCRIPTION("Realtek RTL8125 2.5Gigabit Ethernet driver");

module_param(speed_mode, uint, 0);
MODULE_PARM_DESC(speed_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(duplex_mode, uint, 0);
MODULE_PARM_DESC(duplex_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(autoneg_mode, uint, 0);
MODULE_PARM_DESC(autoneg_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(advertising_mode, uint, 0);
MODULE_PARM_DESC(advertising_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(aspm, int, 0);
MODULE_PARM_DESC(aspm, "Enable ASPM.");

module_param(s5wol, int, 0);
MODULE_PARM_DESC(s5wol, "Enable Shutdown Wake On Lan.");

module_param(s5_keep_curr_mac, int, 0);
MODULE_PARM_DESC(s5_keep_curr_mac, "Enable Shutdown Keep Current MAC Address.");

module_param(use_dac, int, 0);
MODULE_PARM_DESC(use_dac, "Enable PCI DAC. Unsafe on 32 bit PCI slot.");

module_param(timer_count, int, 0);
MODULE_PARM_DESC(timer_count, "Timer Interrupt Interval.");

module_param(eee_enable, int, 0);
MODULE_PARM_DESC(eee_enable, "Enable Energy Efficient Ethernet.");

module_param(hwoptimize, ulong, 0);
MODULE_PARM_DESC(hwoptimize, "Enable HW optimization function.");

module_param(s0_magic_packet, int, 0);
MODULE_PARM_DESC(s0_magic_packet, "Enable S0 Magic Packet.");

module_param(tx_no_close_enable, int, 0);
MODULE_PARM_DESC(tx_no_close_enable, "Enable TX No Close.");

module_param(enable_ptp_master_mode, int, 0);
MODULE_PARM_DESC(enable_ptp_master_mode, "Enable PTP Master Mode.");

module_param(disable_pm_support, int, 0);
MODULE_PARM_DESC(disable_pm_support, "Disable PM support.");

module_param(enable_double_vlan, int, 0);
MODULE_PARM_DESC(enable_double_vlan, "Enable Double VLAN.");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
module_param_named(debug, debug.msg_enable, int, 0);
MODULE_PARM_DESC(debug, "Debug verbosity level (0=none, ..., 16=all)");
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

MODULE_LICENSE("GPL");
#ifdef ENABLE_USE_FIRMWARE_FILE
MODULE_FIRMWARE(FIRMWARE_8125A_3);
MODULE_FIRMWARE(FIRMWARE_8125B_2);
#endif

MODULE_VERSION(RTL8125_VERSION);

/*
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void rtl8125_esd_timer(unsigned long __opaque);
#else
static void rtl8125_esd_timer(struct timer_list *t);
#endif
*/
/*
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void rtl8125_link_timer(unsigned long __opaque);
#else
static void rtl8125_link_timer(struct timer_list *t);
#endif
*/

static netdev_tx_t rtl8125_start_xmit(struct sk_buff *skb, struct net_device *dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8125_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t rtl8125_interrupt(int irq, void *dev_instance);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8125_interrupt_msix(int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t rtl8125_interrupt_msix(int irq, void *dev_instance);
#endif
static void rtl8125_set_rx_mode(struct net_device *dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static void rtl8125_tx_timeout(struct net_device *dev, unsigned int txqueue);
#else
static void rtl8125_tx_timeout(struct net_device *dev);
#endif
static int rtl8125_rx_interrupt(struct net_device *, struct rtl8125_private *, struct rtl8125_rx_ring *, napi_budget);
static int rtl8125_tx_interrupt(struct rtl8125_tx_ring *ring, int budget);
static int rtl8125_tx_interrupt_with_vector(struct rtl8125_private *tp, const int message_id, int budget);
static void rtl8125_wait_for_quiescence(struct net_device *dev);
static int rtl8125_change_mtu(struct net_device *dev, int new_mtu);
static void rtl8125_down(struct net_device *dev);

static int rtl8125_set_mac_address(struct net_device *dev, void *p);
static void rtl8125_rar_set(struct rtl8125_private *tp, const u8 *addr);
static void rtl8125_desc_addr_fill(struct rtl8125_private *);
static void rtl8125_tx_desc_init(struct rtl8125_private *tp);
static void rtl8125_rx_desc_init(struct rtl8125_private *tp);

static u32 mdio_direct_read_phy_ocp(struct rtl8125_private *tp, u16 RegAddr);
static u16 rtl8125_get_hw_phy_mcu_code_ver(struct rtl8125_private *tp);
static void rtl8125_phy_power_up(struct net_device *dev);
static void rtl8125_phy_power_down(struct net_device *dev);
static int rtl8125_set_speed(struct net_device *dev, u8 autoneg, u32 speed, u8 duplex, u64 adv);
static bool rtl8125_set_phy_mcu_patch_request(struct rtl8125_private *tp);
static bool rtl8125_clear_phy_mcu_patch_request(struct rtl8125_private *tp);

#ifdef CONFIG_R8125_NAPI
static int rtl8125_poll(napi_ptr napi, napi_budget budget);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8125_reset_task(void *_data);
static void rtl8125_esd_task(void *_data);
static void rtl8125_linkchg_task(void *_data);
#else
static void rtl8125_reset_task(struct work_struct *work);
static void rtl8125_esd_task(struct work_struct *work);
static void rtl8125_linkchg_task(struct work_struct *work);
#endif
static void rtl8125_schedule_reset_work(struct rtl8125_private *tp);
static void rtl8125_schedule_esd_work(struct rtl8125_private *tp);
static void rtl8125_schedule_linkchg_work(struct rtl8125_private *tp);
static void rtl8125_init_all_schedule_work(struct rtl8125_private *tp);
static void rtl8125_cancel_all_schedule_work(struct rtl8125_private *tp);

static inline struct device *tp_to_dev(struct rtl8125_private *tp)
{
        return &tp->pci_dev->dev;
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0) && \
     LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,00)))
void ethtool_convert_legacy_u32_to_link_mode(unsigned long *dst,
                u32 legacy_u32)
{
        bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS);
        dst[0] = legacy_u32;
}

bool ethtool_convert_link_mode_to_legacy_u32(u32 *legacy_u32,
                const unsigned long *src)
{
        bool retval = true;

        /* TODO: following test will soon always be true */
        if (__ETHTOOL_LINK_MODE_MASK_NBITS > 32) {
                __ETHTOOL_DECLARE_LINK_MODE_MASK(ext);

                bitmap_zero(ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
                bitmap_fill(ext, 32);
                bitmap_complement(ext, ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
                if (bitmap_intersects(ext, src,
                                      __ETHTOOL_LINK_MODE_MASK_NBITS)) {
                        /* src mask goes beyond bit 31 */
                        retval = false;
                }
        }
        *legacy_u32 = src[0];
        return retval;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)

#ifndef LPA_1000FULL
#define LPA_1000FULL            0x0800
#endif

#ifndef LPA_1000HALF
#define LPA_1000HALF            0x0400
#endif

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
static inline void eth_hw_addr_random(struct net_device *dev)
{
        random_ether_addr(dev->dev_addr);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#undef ethtool_ops
#define ethtool_ops _kc_ethtool_ops

struct _kc_ethtool_ops {
        int  (*get_settings)(struct net_device *, struct ethtool_cmd *);
        int  (*set_settings)(struct net_device *, struct ethtool_cmd *);
        void (*get_drvinfo)(struct net_device *, struct ethtool_drvinfo *);
        int  (*get_regs_len)(struct net_device *);
        void (*get_regs)(struct net_device *, struct ethtool_regs *, void *);
        void (*get_wol)(struct net_device *, struct ethtool_wolinfo *);
        int  (*set_wol)(struct net_device *, struct ethtool_wolinfo *);
        u32  (*get_msglevel)(struct net_device *);
        void (*set_msglevel)(struct net_device *, u32);
        int  (*nway_reset)(struct net_device *);
        u32  (*get_link)(struct net_device *);
        int  (*get_eeprom_len)(struct net_device *);
        int  (*get_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
        int  (*set_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
        int  (*get_coalesce)(struct net_device *, struct ethtool_coalesce *);
        int  (*set_coalesce)(struct net_device *, struct ethtool_coalesce *);
        void (*get_ringparam)(struct net_device *, struct ethtool_ringparam *);
        int  (*set_ringparam)(struct net_device *, struct ethtool_ringparam *);
        void (*get_pauseparam)(struct net_device *,
                               struct ethtool_pauseparam*);
        int  (*set_pauseparam)(struct net_device *,
                               struct ethtool_pauseparam*);
        u32  (*get_rx_csum)(struct net_device *);
        int  (*set_rx_csum)(struct net_device *, u32);
        u32  (*get_tx_csum)(struct net_device *);
        int  (*set_tx_csum)(struct net_device *, u32);
        u32  (*get_sg)(struct net_device *);
        int  (*set_sg)(struct net_device *, u32);
        u32  (*get_tso)(struct net_device *);
        int  (*set_tso)(struct net_device *, u32);
        int  (*self_test_count)(struct net_device *);
        void (*self_test)(struct net_device *, struct ethtool_test *, u64 *);
        void (*get_strings)(struct net_device *, u32 stringset, u8 *);
        int  (*phys_id)(struct net_device *, u32);
        int  (*get_stats_count)(struct net_device *);
        void (*get_ethtool_stats)(struct net_device *, struct ethtool_stats *,
                                  u64 *);
} *ethtool_ops = NULL;

#undef SET_ETHTOOL_OPS
#define SET_ETHTOOL_OPS(netdev, ops) (ethtool_ops = (ops))

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)
#ifndef SET_ETHTOOL_OPS
#define SET_ETHTOOL_OPS(netdev,ops) \
         ( (netdev)->ethtool_ops = (ops) )
#endif //SET_ETHTOOL_OPS
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)

//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#ifndef netif_msg_init
#define netif_msg_init _kc_netif_msg_init
/* copied from linux kernel 2.6.20 include/linux/netdevice.h */
static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
        /* use default */
        if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
                return default_msg_enable_bits;
        if (debug_value == 0)   /* no output */
                return 0;
        /* set low N bits */
        return (1 << debug_value) - 1;
}

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
static inline void eth_copy_and_sum (struct sk_buff *dest,
                                     const unsigned char *src,
                                     int len, int base)
{
        skb_copy_to_linear_data(dest, src, len);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 /include/linux/time.h */
/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC    1000L

/* copied from linux kernel 2.6.20 /include/linux/jiffies.h */
/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return MAX_LONG, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
static inline unsigned int _kc_jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
        return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
        return (j + (HZ / MSEC_PER_SEC) - 1)/(HZ / MSEC_PER_SEC);
#else
        return (j * MSEC_PER_SEC) / HZ;
#endif
}

static inline unsigned long _kc_msecs_to_jiffies(const unsigned int m)
{
        if (m > _kc_jiffies_to_msecs(MAX_JIFFY_OFFSET))
                return MAX_JIFFY_OFFSET;
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
        return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
        return m * (HZ / MSEC_PER_SEC);
#else
        return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
#endif
}
#endif  //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

/* copied from linux kernel 2.6.12.6 /include/linux/pm.h */
typedef int __bitwise pci_power_t;

/* copied from linux kernel 2.6.12.6 /include/linux/pci.h */
typedef u32 __bitwise pm_message_t;

#define PCI_D0  ((pci_power_t __force) 0)
#define PCI_D1  ((pci_power_t __force) 1)
#define PCI_D2  ((pci_power_t __force) 2)
#define PCI_D3hot   ((pci_power_t __force) 3)
#define PCI_D3cold  ((pci_power_t __force) 4)
#define PCI_POWER_ERROR ((pci_power_t __force) -1)

/* copied from linux kernel 2.6.12.6 /drivers/pci/pci.c */
/**
 * pci_choose_state - Choose the power state of a PCI device
 * @dev: PCI device to be suspended
 * @state: target sleep state for the whole system. This is the value
 *  that is passed to suspend() function.
 *
 * Returns PCI power state suitable for given device and given system
 * message.
 */

pci_power_t pci_choose_state(struct pci_dev *dev, pm_message_t state)
{
        if (!pci_find_capability(dev, PCI_CAP_ID_PM))
                return PCI_D0;

        switch (state) {
        case 0:
                return PCI_D0;
        case 3:
                return PCI_D3hot;
        default:
                printk("They asked me for state %d\n", state);
//      BUG();
        }
        return PCI_D0;
}
#endif  //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
/**
 * msleep_interruptible - sleep waiting for waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
#define msleep_interruptible _kc_msleep_interruptible
unsigned long _kc_msleep_interruptible(unsigned int msecs)
{
        unsigned long timeout = _kc_msecs_to_jiffies(msecs);

        while (timeout && !signal_pending(current)) {
                set_current_state(TASK_INTERRUPTIBLE);
                timeout = schedule_timeout(timeout);
        }
        return _kc_jiffies_to_msecs(timeout);
}
#endif  //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 include/linux/sched.h */
#ifndef __sched
#define __sched     __attribute__((__section__(".sched.text")))
#endif

/* copied from linux kernel 2.6.20 kernel/timer.c */
signed long __sched schedule_timeout_uninterruptible(signed long timeout)
{
        __set_current_state(TASK_UNINTERRUPTIBLE);
        return schedule_timeout(timeout);
}

/* copied from linux kernel 2.6.20 include/linux/mii.h */
#undef if_mii
#define if_mii _kc_if_mii
static inline struct mii_ioctl_data *if_mii(struct ifreq *rq)
{
        return (struct mii_ioctl_data *) &rq->ifr_ifru;
}
#endif  //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)

static u32 rtl8125_read_thermal_sensor(struct rtl8125_private *tp)
{
        u16 ts_digout;

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                ts_digout = mdio_direct_read_phy_ocp(tp, 0xBD84);
                ts_digout &= 0x3ff;
                break;
        default:
                ts_digout = 0xffff;
                break;
        }

        return ts_digout;
}

int rtl8125_dump_tally_counter(struct rtl8125_private *tp, dma_addr_t paddr)
{
        u32 cmd;
        u32 WaitCnt;
        int retval = -1;

        RTL_W32(tp, CounterAddrHigh, (u64)paddr >> 32);
        cmd = (u64)paddr & DMA_BIT_MASK(32);
        RTL_W32(tp, CounterAddrLow, cmd);
        RTL_W32(tp, CounterAddrLow, cmd | CounterDump);

        WaitCnt = 0;
        while (RTL_R32(tp, CounterAddrLow) & CounterDump) {
                udelay(10);

                WaitCnt++;
                if (WaitCnt > 20)
                        break;
        }

        if (WaitCnt <= 20)
                retval = 0;

        return retval;
}

static u32 rtl8125_convert_link_speed(u16 status)
{
        u32 speed = SPEED_UNKNOWN;

        if (status & LinkStatus) {
                if (status & _5000bpsF)
                        speed = SPEED_5000;
                else if (status & _2500bpsF)
                        speed = SPEED_2500;
                else if (status & _1000bpsF)
                        speed = SPEED_1000;
                else if (status & _100bps)
                        speed = SPEED_100;
                else if (status & _10bps)
                        speed = SPEED_10;
        }

        return speed;
}

static void rtl8125_get_cp_len(struct rtl8125_private *tp,
                               u16 cp_len[RTL8125_CP_NUM])
{
        int i;
        int count;
        u16 status;
        u16 tmp_cp_len = 0;

        status = RTL_R16(tp, PHYstatus);
        if (status & LinkStatus) {
                if (status & _10bps) {
                        tmp_cp_len = 0;
                        goto no_cpdr;
                } else if (status & _1000bpsF) {
                        rtl8125_mdio_write(tp, 0x1f, 0x0a88);
                        tmp_cp_len = rtl8125_mdio_read(tp, 0x10);
                        goto no_cpdr;
                } else if (status & _2500bpsF) {
                        switch (tp->mcfg) {
                        case CFG_METHOD_2:
                        case CFG_METHOD_3:
                        case CFG_METHOD_6:
                                rtl8125_mdio_write(tp, 0x1f, 0x0ac5);
                                tmp_cp_len = rtl8125_mdio_read(tp, 0x14) >> 4;
                                break;
                        default:
                                rtl8125_mdio_write(tp, 0x1f, 0x0acb);
                                tmp_cp_len = rtl8125_mdio_read(tp, 0x15) >> 2;
                                break;
                        }
                        goto no_cpdr;
                }
        }

        rtl8125_mdio_write(tp, 0x1f, 0x0a42);
        rtl8125_mdio_write(tp, 0x11, 0x0000);
        rtl8125_set_eth_phy_bit(tp, 0x11, 0xf0);
        rtl8125_set_eth_phy_bit(tp, 0x11, 0x01);

        count = 0;
        while (!(rtl8125_mdio_read(tp, 0x11) & BIT_15) &&
               (count++ < 200))
                msleep(100);

        rtl8125_mdio_write(tp, 0x1f, 0x0a43);
        for (i=0; i<RTL8125_CP_NUM; i++) {
                u16 val;
                rtl8125_mdio_write(tp, 0x13, 0x8029 + 4 * i);
                val = rtl8125_mdio_read(tp, 0x14);
                tmp_cp_len = ((val & 0x3fff) / 80) & 0xff;
                if (tmp_cp_len > 200)
                        tmp_cp_len = 0;
                cp_len[i] = tmp_cp_len;
        }

        goto exit;

no_cpdr:
        tmp_cp_len &= 0xff;
        for (i=0; i<RTL8125_CP_NUM; i++)
                cp_len[i] = tmp_cp_len;

exit:
        rtl8125_mdio_write(tp, 0x1f, 0x0000);

        for (i=0; i<RTL8125_CP_NUM; i++)
                if (cp_len[i] > RTL8125_MAX_SUPPORT_cp_len)
                        cp_len[i] = RTL8125_MAX_SUPPORT_cp_len;

        return;
}

static int _rtl8125_get_cp_status(u16 val)
{
        switch (val) {
        case 0x0060:
                return rtl8125_cp_normal;
        case 0x0042:
        case 0x0048:
                return rtl8125_cp_open;
        case 0x0044:
        case 0x0050:
                return rtl8125_cp_short;
        default:
                return rtl8125_cp_normal;
        }
}

static int rtl8125_get_cp_status(struct rtl8125_private *tp, u8 pair_num)
{
        u16 val;
        int cp_status = rtl8125_cp_unknown;

        if (pair_num > 3)
                goto exit;

        rtl8125_mdio_write(tp, 0x1f, 0x0a43);
        rtl8125_mdio_write(tp, 0x13, 0x8027 + 4 * pair_num);
        val = rtl8125_mdio_read(tp, 0x14);
        rtl8125_mdio_write(tp, 0x1f, 0x0000);

        cp_status = _rtl8125_get_cp_status(val);

exit:
        return cp_status;
}

static const char * rtl8125_get_cp_status_string(int cp_status)
{
        switch(cp_status) {
        case rtl8125_cp_normal:
                return "normal";
        case rtl8125_cp_short:
                return "short";
        case rtl8125_cp_open:
                return "open";
        default:
                return "unknown";
        }
}

static void rtl8125_get_cp(struct rtl8125_private *tp,
                           u16 cp_len[RTL8125_CP_NUM],
                           int cp_status[RTL8125_CP_NUM])
{
        int i;

        rtl8125_get_cp_len(tp, cp_len);
        for (i =0; i<RTL8125_CP_NUM; i++)
                cp_status[i] = rtl8125_get_cp_status(tp, i);
}

#ifdef ENABLE_R8125_PROCFS
/****************************************************************************
*   -----------------------------PROCFS STUFF-------------------------
*****************************************************************************
*/

static struct proc_dir_entry *rtl8125_proc;
static int proc_init_num = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int proc_get_driver_variable(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        struct rtl8125_private *tp = netdev_priv(dev);

        seq_puts(m, "\nDump Driver Variable\n");

        rtnl_lock();

        seq_puts(m, "Variable\tValue\n----------\t-----\n");
        seq_printf(m, "MODULENAME\t%s\n", MODULENAME);
        seq_printf(m, "driver version\t%s\n", RTL8125_VERSION);
        seq_printf(m, "mcfg\t%d\n", tp->mcfg);
        seq_printf(m, "chipset\t%d\n", tp->chipset);
        seq_printf(m, "chipset_name\t%s\n", rtl_chip_info[tp->chipset].name);
        seq_printf(m, "mtu\t%d\n", dev->mtu);
        seq_printf(m, "NUM_RX_DESC\t0x%x\n", tp->rx_ring[0].num_rx_desc);
        seq_printf(m, "cur_rx0\t0x%x\n", tp->rx_ring[0].cur_rx);
        seq_printf(m, "dirty_rx0\t0x%x\n", tp->rx_ring[0].dirty_rx);
        seq_printf(m, "cur_rx1\t0x%x\n", tp->rx_ring[1].cur_rx);
        seq_printf(m, "dirty_rx1\t0x%x\n", tp->rx_ring[1].dirty_rx);
        seq_printf(m, "cur_rx2\t0x%x\n", tp->rx_ring[2].cur_rx);
        seq_printf(m, "dirty_rx2\t0x%x\n", tp->rx_ring[2].dirty_rx);
        seq_printf(m, "cur_rx3\t0x%x\n", tp->rx_ring[3].cur_rx);
        seq_printf(m, "dirty_rx3\t0x%x\n", tp->rx_ring[3].dirty_rx);
        seq_printf(m, "NUM_TX_DESC\t0x%x\n", tp->tx_ring[0].num_tx_desc);
        seq_printf(m, "cur_tx0\t0x%x\n", tp->tx_ring[0].cur_tx);
        seq_printf(m, "dirty_tx0\t0x%x\n", tp->tx_ring[0].dirty_tx);
        seq_printf(m, "cur_tx1\t0x%x\n", tp->tx_ring[1].cur_tx);
        seq_printf(m, "dirty_tx1\t0x%x\n", tp->tx_ring[1].dirty_tx);
        seq_printf(m, "rx_buf_sz\t0x%x\n", tp->rx_buf_sz);
#ifdef ENABLE_PAGE_REUSE
        seq_printf(m, "rx_buf_page_order\t0x%x\n", tp->rx_buf_page_order);
        seq_printf(m, "rx_buf_page_size\t0x%x\n", tp->rx_buf_page_size);
        seq_printf(m, "page_reuse_fail_cnt\t0x%x\n", tp->page_reuse_fail_cnt);
#endif //ENABLE_PAGE_REUSE
        seq_printf(m, "esd_flag\t0x%x\n", tp->esd_flag);
        seq_printf(m, "pci_cfg_is_read\t0x%x\n", tp->pci_cfg_is_read);
        seq_printf(m, "rtl8125_rx_config\t0x%x\n", tp->rtl8125_rx_config);
        seq_printf(m, "cp_cmd\t0x%x\n", tp->cp_cmd);
        seq_printf(m, "intr_mask\t0x%x\n", tp->intr_mask);
        seq_printf(m, "timer_intr_mask\t0x%x\n", tp->timer_intr_mask);
        seq_printf(m, "wol_enabled\t0x%x\n", tp->wol_enabled);
        seq_printf(m, "wol_opts\t0x%x\n", tp->wol_opts);
        seq_printf(m, "efuse_ver\t0x%x\n", tp->efuse_ver);
        seq_printf(m, "eeprom_type\t0x%x\n", tp->eeprom_type);
        seq_printf(m, "autoneg\t0x%x\n", tp->autoneg);
        seq_printf(m, "duplex\t0x%x\n", tp->duplex);
        seq_printf(m, "speed\t%d\n", tp->speed);
        seq_printf(m, "advertising\t0x%llx\n", tp->advertising);
        seq_printf(m, "eeprom_len\t0x%x\n", tp->eeprom_len);
        seq_printf(m, "cur_page\t0x%x\n", tp->cur_page);
        seq_printf(m, "bios_setting\t0x%x\n", tp->bios_setting);
        seq_printf(m, "features\t0x%x\n", tp->features);
        seq_printf(m, "org_pci_offset_99\t0x%x\n", tp->org_pci_offset_99);
        seq_printf(m, "org_pci_offset_180\t0x%x\n", tp->org_pci_offset_180);
        seq_printf(m, "issue_offset_99_event\t0x%x\n", tp->issue_offset_99_event);
        seq_printf(m, "org_pci_offset_80\t0x%x\n", tp->org_pci_offset_80);
        seq_printf(m, "org_pci_offset_81\t0x%x\n", tp->org_pci_offset_81);
        seq_printf(m, "use_timer_interrrupt\t0x%x\n", tp->use_timer_interrrupt);
        seq_printf(m, "HwIcVerUnknown\t0x%x\n", tp->HwIcVerUnknown);
        seq_printf(m, "NotWrRamCodeToMicroP\t0x%x\n", tp->NotWrRamCodeToMicroP);
        seq_printf(m, "NotWrMcuPatchCode\t0x%x\n", tp->NotWrMcuPatchCode);
        seq_printf(m, "HwHasWrRamCodeToMicroP\t0x%x\n", tp->HwHasWrRamCodeToMicroP);
        seq_printf(m, "sw_ram_code_ver\t0x%x\n", tp->sw_ram_code_ver);
        seq_printf(m, "hw_ram_code_ver\t0x%x\n", tp->hw_ram_code_ver);
        seq_printf(m, "rtk_enable_diag\t0x%x\n", tp->rtk_enable_diag);
        seq_printf(m, "ShortPacketSwChecksum\t0x%x\n", tp->ShortPacketSwChecksum);
        seq_printf(m, "UseSwPaddingShortPkt\t0x%x\n", tp->UseSwPaddingShortPkt);
        seq_printf(m, "RequireRduNonStopPatch\t0x%x\n", tp->RequireRduNonStopPatch);
        seq_printf(m, "RequireAdcBiasPatch\t0x%x\n", tp->RequireAdcBiasPatch);
        seq_printf(m, "AdcBiasPatchIoffset\t0x%x\n", tp->AdcBiasPatchIoffset);
        seq_printf(m, "RequireAdjustUpsTxLinkPulseTiming\t0x%x\n", tp->RequireAdjustUpsTxLinkPulseTiming);
        seq_printf(m, "SwrCnt1msIni\t0x%x\n", tp->SwrCnt1msIni);
        seq_printf(m, "HwSuppNowIsOobVer\t0x%x\n", tp->HwSuppNowIsOobVer);
        seq_printf(m, "HwFiberModeVer\t0x%x\n", tp->HwFiberModeVer);
        seq_printf(m, "HwFiberStat\t0x%x\n", tp->HwFiberStat);
        seq_printf(m, "HwSwitchMdiToFiber\t0x%x\n", tp->HwSwitchMdiToFiber);
        seq_printf(m, "NicCustLedValue\t0x%x\n", tp->NicCustLedValue);
        seq_printf(m, "RequiredSecLanDonglePatch\t0x%x\n", tp->RequiredSecLanDonglePatch);
        seq_printf(m, "HwSuppDashVer\t0x%x\n", tp->HwSuppDashVer);
        seq_printf(m, "DASH\t0x%x\n", tp->DASH);
        seq_printf(m, "dash_printer_enabled\t0x%x\n", tp->dash_printer_enabled);
        seq_printf(m, "HwSuppKCPOffloadVer\t0x%x\n", tp->HwSuppKCPOffloadVer);
        seq_printf(m, "speed_mode\t0x%x\n", speed_mode);
        seq_printf(m, "duplex_mode\t0x%x\n", duplex_mode);
        seq_printf(m, "autoneg_mode\t0x%x\n", autoneg_mode);
        seq_printf(m, "advertising_mode\t0x%x\n", advertising_mode);
        seq_printf(m, "aspm\t0x%x\n", aspm);
        seq_printf(m, "s5wol\t0x%x\n", s5wol);
        seq_printf(m, "s5_keep_curr_mac\t0x%x\n", s5_keep_curr_mac);
        seq_printf(m, "eee_enable\t0x%x\n", tp->eee.eee_enabled);
        seq_printf(m, "hwoptimize\t0x%lx\n", hwoptimize);
        seq_printf(m, "proc_init_num\t0x%x\n", proc_init_num);
        seq_printf(m, "s0_magic_packet\t0x%x\n", s0_magic_packet);
        seq_printf(m, "disable_pm_support\t0x%x\n", disable_pm_support);
        seq_printf(m, "enable_double_vlan\t0x%x\n", enable_double_vlan);
        seq_printf(m, "HwSuppMagicPktVer\t0x%x\n", tp->HwSuppMagicPktVer);
        seq_printf(m, "HwSuppLinkChgWakeUpVer\t0x%x\n", tp->HwSuppLinkChgWakeUpVer);
        seq_printf(m, "HwSuppD0SpeedUpVer\t0x%x\n", tp->HwSuppD0SpeedUpVer);
        seq_printf(m, "D0SpeedUpSpeed\t0x%x\n", tp->D0SpeedUpSpeed);
        seq_printf(m, "HwSuppCheckPhyDisableModeVer\t0x%x\n", tp->HwSuppCheckPhyDisableModeVer);
        seq_printf(m, "HwPkgDet\t0x%x\n", tp->HwPkgDet);
        seq_printf(m, "HwSuppTxNoCloseVer\t0x%x\n", tp->HwSuppTxNoCloseVer);
        seq_printf(m, "EnableTxNoClose\t0x%x\n", tp->EnableTxNoClose);
        seq_printf(m, "NextHwDesCloPtr0\t0x%x\n", tp->tx_ring[0].NextHwDesCloPtr);
        seq_printf(m, "BeginHwDesCloPtr0\t0x%x\n", tp->tx_ring[0].BeginHwDesCloPtr);
        seq_printf(m, "NextHwDesCloPtr1\t0x%x\n", tp->tx_ring[1].NextHwDesCloPtr);
        seq_printf(m, "BeginHwDesCloPtr1\t0x%x\n", tp->tx_ring[1].BeginHwDesCloPtr);
        seq_printf(m, "InitRxDescType\t0x%x\n", tp->InitRxDescType);
        seq_printf(m, "RxDescLength\t0x%x\n", tp->RxDescLength);
        seq_printf(m, "num_rx_rings\t0x%x\n", tp->num_rx_rings);
        seq_printf(m, "num_tx_rings\t0x%x\n", tp->num_tx_rings);
        seq_printf(m, "tot_rx_rings\t0x%x\n", rtl8125_tot_rx_rings(tp));
        seq_printf(m, "tot_tx_rings\t0x%x\n", rtl8125_tot_tx_rings(tp));
        seq_printf(m, "EnableRss\t0x%x\n", tp->EnableRss);
        seq_printf(m, "EnablePtp\t0x%x\n", tp->EnablePtp);
        seq_printf(m, "ptp_master_mode\t0x%x\n", tp->ptp_master_mode);
        seq_printf(m, "min_irq_nvecs\t0x%x\n", tp->min_irq_nvecs);
        seq_printf(m, "irq_nvecs\t0x%x\n", tp->irq_nvecs);
        seq_printf(m, "ring_lib_enabled\t0x%x\n", tp->ring_lib_enabled);
        seq_printf(m, "HwSuppIsrVer\t0x%x\n", tp->HwSuppIsrVer);
        seq_printf(m, "HwCurrIsrVer\t0x%x\n", tp->HwCurrIsrVer);
#ifdef ENABLE_PTP_SUPPORT
        seq_printf(m, "tx_hwtstamp_timeouts\t0x%x\n", tp->tx_hwtstamp_timeouts);
        seq_printf(m, "tx_hwtstamp_skipped\t0x%x\n", tp->tx_hwtstamp_skipped);
#endif
        seq_printf(m, "random_mac\t0x%x\n", tp->random_mac);
        seq_printf(m, "org_mac_addr\t%pM\n", tp->org_mac_addr);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        seq_printf(m, "perm_addr\t%pM\n", dev->perm_addr);
#endif
        seq_printf(m, "dev_addr\t%pM\n", dev->dev_addr);

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_tally_counter(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        struct rtl8125_private *tp = netdev_priv(dev);
        struct rtl8125_counters *counters;
        dma_addr_t paddr;

        seq_puts(m, "\nDump Tally Counter\n");

        rtnl_lock();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters) {
                seq_puts(m, "\nDump Tally Counter Fail\n");
                goto out_unlock;
        }

        rtl8125_dump_tally_counter(tp, paddr);

        seq_puts(m, "Statistics\tValue\n----------\t-----\n");
        seq_printf(m, "tx_packets\t%lld\n", le64_to_cpu(counters->tx_packets));
        seq_printf(m, "rx_packets\t%lld\n", le64_to_cpu(counters->rx_packets));
        seq_printf(m, "tx_errors\t%lld\n", le64_to_cpu(counters->tx_errors));
        seq_printf(m, "rx_errors\t%d\n", le32_to_cpu(counters->rx_errors));
        seq_printf(m, "rx_missed\t%d\n", le16_to_cpu(counters->rx_missed));
        seq_printf(m, "align_errors\t%d\n", le16_to_cpu(counters->align_errors));
        seq_printf(m, "tx_one_collision\t%d\n", le32_to_cpu(counters->tx_one_collision));
        seq_printf(m, "tx_multi_collision\t%d\n", le32_to_cpu(counters->tx_multi_collision));
        seq_printf(m, "rx_unicast\t%lld\n", le64_to_cpu(counters->rx_unicast));
        seq_printf(m, "rx_broadcast\t%lld\n", le64_to_cpu(counters->rx_broadcast));
        seq_printf(m, "rx_multicast\t%d\n", le32_to_cpu(counters->rx_multicast));
        seq_printf(m, "tx_aborted\t%d\n", le16_to_cpu(counters->tx_aborted));
        seq_printf(m, "tx_underrun\t%d\n", le16_to_cpu(counters->tx_underrun));

        seq_printf(m, "tx_octets\t%lld\n", le64_to_cpu(counters->tx_octets));
        seq_printf(m, "rx_octets\t%lld\n", le64_to_cpu(counters->rx_octets));
        seq_printf(m, "rx_multicast64\t%lld\n", le64_to_cpu(counters->rx_multicast64));
        seq_printf(m, "tx_unicast64\t%lld\n", le64_to_cpu(counters->tx_unicast64));
        seq_printf(m, "tx_broadcast64\t%lld\n", le64_to_cpu(counters->tx_broadcast64));
        seq_printf(m, "tx_multicast64\t%lld\n", le64_to_cpu(counters->tx_multicast64));
        seq_printf(m, "tx_pause_on\t%d\n", le32_to_cpu(counters->tx_pause_on));
        seq_printf(m, "tx_pause_off\t%d\n", le32_to_cpu(counters->tx_pause_off));
        seq_printf(m, "tx_pause_all\t%d\n", le32_to_cpu(counters->tx_pause_all));
        seq_printf(m, "tx_deferred\t%d\n", le32_to_cpu(counters->tx_deferred));
        seq_printf(m, "tx_late_collision\t%d\n", le32_to_cpu(counters->tx_late_collision));
        seq_printf(m, "tx_all_collision\t%d\n", le32_to_cpu(counters->tx_all_collision));
        seq_printf(m, "tx_aborted32\t%d\n", le32_to_cpu(counters->tx_aborted32));
        seq_printf(m, "align_errors32\t%d\n", le32_to_cpu(counters->align_errors32));
        seq_printf(m, "rx_frame_too_long\t%d\n", le32_to_cpu(counters->rx_frame_too_long));
        seq_printf(m, "rx_runt\t%d\n", le32_to_cpu(counters->rx_runt));
        seq_printf(m, "rx_pause_on\t%d\n", le32_to_cpu(counters->rx_pause_on));
        seq_printf(m, "rx_pause_off\t%d\n", le32_to_cpu(counters->rx_pause_off));
        seq_printf(m, "rx_pause_all\t%d\n", le32_to_cpu(counters->rx_pause_all));
        seq_printf(m, "rx_unknown_opcode\t%d\n", le32_to_cpu(counters->rx_unknown_opcode));
        seq_printf(m, "rx_mac_error\t%d\n", le32_to_cpu(counters->rx_mac_error));
        seq_printf(m, "tx_underrun32\t%d\n", le32_to_cpu(counters->tx_underrun32));
        seq_printf(m, "rx_mac_missed\t%d\n", le32_to_cpu(counters->rx_mac_missed));
        seq_printf(m, "rx_tcam_dropped\t%d\n", le32_to_cpu(counters->rx_tcam_dropped));
        seq_printf(m, "tdu\t%d\n", le32_to_cpu(counters->tdu));
        seq_printf(m, "rdu\t%d\n", le32_to_cpu(counters->rdu));

        seq_putc(m, '\n');

out_unlock:
        rtnl_unlock();

        return 0;
}

static int proc_get_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8125_MAC_REGS_SIZE;
        u8 byte_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;

        seq_puts(m, "\nDump MAC Registers\n");
        seq_puts(m, "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 16 && n < max; i++, n++) {
                        byte_rd = readb(ioaddr + n);
                        seq_printf(m, "%02x ", byte_rd);
                }
        }

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_pcie_phy(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8125_EPHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8125_private *tp = netdev_priv(dev);

        seq_puts(m, "\nDump PCIE PHY\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        rtnl_lock();

        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8125_ephy_read(tp, n);
                        seq_printf(m, "%04x ", word_rd);
                }
        }

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_eth_phy(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8125_PHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8125_private *tp = netdev_priv(dev);

        seq_puts(m, "\nDump Ethernet PHY\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        rtnl_lock();

        seq_puts(m, "\n####################page 0##################\n ");
        rtl8125_mdio_write(tp, 0x1f, 0x0000);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8125_mdio_read(tp, n);
                        seq_printf(m, "%04x ", word_rd);
                }
        }

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_extended_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8125_ERI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8125_private *tp = netdev_priv(dev);

        seq_puts(m, "\nDump Extended Registers\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        rtnl_lock();

        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        dword_rd = rtl8125_eri_read(tp, n, 4, ERIAR_ExGMAC);
                        seq_printf(m, "%08x ", dword_rd);
                }
        }

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_pci_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8125_PCI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8125_private *tp = netdev_priv(dev);

        seq_puts(m, "\nDump PCI Registers\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        rtnl_lock();

        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%03x:\t", n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
                        seq_printf(m, "%08x ", dword_rd);
                }
        }

        n = 0x110;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        seq_printf(m, "\n0x%03x:\t%08x ", n, dword_rd);
        n = 0x70c;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        seq_printf(m, "\n0x%03x:\t%08x ", n, dword_rd);

        rtnl_unlock();

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_temperature(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 ts_digout, tj, fah;

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                seq_puts(m, "\nChip Temperature\n");
                break;
        default:
                return -EOPNOTSUPP;
        }

        rtnl_lock();
        ts_digout = rtl8125_read_thermal_sensor(tp);
        rtnl_unlock();

        tj = ts_digout / 2;
        if (ts_digout <= 512) {
                tj = ts_digout / 2;
                seq_printf(m, "Cel:%d\n", tj);
                fah = tj * (9/5) + 32;
                seq_printf(m, "Fah:%d\n", fah);
        } else {
                tj = (512 - ((ts_digout / 2) - 512)) / 2;
                seq_printf(m, "Cel:-%d\n", tj);
                fah = tj * (9/5) + 32;
                seq_printf(m, "Fah:-%d\n", fah);
        }

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_cable_info(struct seq_file *m, void *v)
{
        int i;
        u16 status;
        int cp_status[RTL8125_CP_NUM];
        u16 cp_len[RTL8125_CP_NUM] = {0};
        struct net_device *dev = m->private;
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2 ... CFG_METHOD_7:
                /* support */
                break;
        default:
                return -EOPNOTSUPP;
        }

        rtnl_lock();

        status = RTL_R16(tp, PHYstatus);
        if (status & LinkStatus)
                seq_printf(m, "\nlink speed:%d",
                           rtl8125_convert_link_speed(status));
        else
                seq_puts(m, "\nlink status:off");

        rtl8125_get_cp(tp, cp_len, cp_status);

        rtnl_unlock();

        seq_puts(m, "\npair\tlength\tstaus\n");

        for (i =0; i<RTL8125_CP_NUM; i++)
                seq_printf(m, "%d-%d\t%d\t%s\n", i+1, i+2, cp_len[i],
                           rtl8125_get_cp_status_string(cp_status[i]));

        seq_putc(m, '\n');
        return 0;
}
#else

static int proc_get_driver_variable(char *page, char **start,
                                    off_t offset, int count,
                                    int *eof, void *data)
{
        struct net_device *dev = data;
        struct rtl8125_private *tp = netdev_priv(dev);
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Driver Driver\n");

        rtnl_lock();

        len += snprintf(page + len, count - len,
                        "Variable\tValue\n----------\t-----\n");

        len += snprintf(page + len, count - len,
                        "MODULENAME\t%s\n"
                        "driver version\t%s\n"
                        "mcfg\t%d\n"
                        "chipset\t%d\n"
                        "chipset_name\t%s\n"
                        "mtu\t%d\n"
                        "NUM_RX_DESC\t0x%x\n"
                        "cur_rx0\t0x%x\n"
                        "dirty_rx0\t0x%x\n"
                        "cur_rx1\t0x%x\n"
                        "dirty_rx1\t0x%x\n"
                        "cur_rx2\t0x%x\n"
                        "dirty_rx2\t0x%x\n"
                        "cur_rx3\t0x%x\n"
                        "dirty_rx3\t0x%x\n"
                        "NUM_TX_DESC\t0x%x\n"
                        "cur_tx0\t0x%x\n"
                        "dirty_tx0\t0x%x\n"
                        "cur_tx1\t0x%x\n"
                        "dirty_tx1\t0x%x\n"
                        "rx_buf_sz\t0x%x\n"
#ifdef ENABLE_PAGE_REUSE
                        "rx_buf_page_order\t0x%x\n"
                        "rx_buf_page_size\t0x%x\n"
                        "page_reuse_fail_cnt\t0x%x\n"
#endif //ENABLE_PAGE_REUSE
                        "esd_flag\t0x%x\n"
                        "pci_cfg_is_read\t0x%x\n"
                        "rtl8125_rx_config\t0x%x\n"
                        "cp_cmd\t0x%x\n"
                        "intr_mask\t0x%x\n"
                        "timer_intr_mask\t0x%x\n"
                        "wol_enabled\t0x%x\n"
                        "wol_opts\t0x%x\n"
                        "efuse_ver\t0x%x\n"
                        "eeprom_type\t0x%x\n"
                        "autoneg\t0x%x\n"
                        "duplex\t0x%x\n"
                        "speed\t%d\n"
                        "advertising\t0x%llx\n"
                        "eeprom_len\t0x%x\n"
                        "cur_page\t0x%x\n"
                        "bios_setting\t0x%x\n"
                        "features\t0x%x\n"
                        "org_pci_offset_99\t0x%x\n"
                        "org_pci_offset_180\t0x%x\n"
                        "issue_offset_99_event\t0x%x\n"
                        "org_pci_offset_80\t0x%x\n"
                        "org_pci_offset_81\t0x%x\n"
                        "use_timer_interrrupt\t0x%x\n"
                        "HwIcVerUnknown\t0x%x\n"
                        "NotWrRamCodeToMicroP\t0x%x\n"
                        "NotWrMcuPatchCode\t0x%x\n"
                        "HwHasWrRamCodeToMicroP\t0x%x\n"
                        "sw_ram_code_ver\t0x%x\n"
                        "hw_ram_code_ver\t0x%x\n"
                        "rtk_enable_diag\t0x%x\n"
                        "ShortPacketSwChecksum\t0x%x\n"
                        "UseSwPaddingShortPkt\t0x%x\n"
                        "RequireRduNonStopPatch\t0x%x\n"
                        "RequireAdcBiasPatch\t0x%x\n"
                        "AdcBiasPatchIoffset\t0x%x\n"
                        "RequireAdjustUpsTxLinkPulseTiming\t0x%x\n"
                        "SwrCnt1msIni\t0x%x\n"
                        "HwSuppNowIsOobVer\t0x%x\n"
                        "HwFiberModeVer\t0x%x\n"
                        "HwFiberStat\t0x%x\n"
                        "HwSwitchMdiToFiber\t0x%x\n"
                        "NicCustLedValue\t0x%x\n"
                        "RequiredSecLanDonglePatch\t0x%x\n"
                        "HwSuppDashVer\t0x%x\n"
                        "DASH\t0x%x\n"
                        "dash_printer_enabled\t0x%x\n"
                        "HwSuppKCPOffloadVer\t0x%x\n"
                        "speed_mode\t0x%x\n"
                        "duplex_mode\t0x%x\n"
                        "autoneg_mode\t0x%x\n"
                        "advertising_mode\t0x%x\n"
                        "aspm\t0x%x\n"
                        "s5wol\t0x%x\n"
                        "s5_keep_curr_mac\t0x%x\n"
                        "eee_enable\t0x%x\n"
                        "hwoptimize\t0x%lx\n"
                        "proc_init_num\t0x%x\n"
                        "s0_magic_packet\t0x%x\n"
                        "disable_pm_support\t0x%x\n"
                        "enable_double_vlan\t0x%x\n"
                        "HwSuppMagicPktVer\t0x%x\n"
                        "HwSuppLinkChgWakeUpVer\t0x%x\n"
                        "HwSuppD0SpeedUpVer\t0x%x\n"
                        "D0SpeedUpSpeed\t0x%x\n"
                        "HwSuppCheckPhyDisableModeVer\t0x%x\n"
                        "HwPkgDet\t0x%x\n"
                        "HwSuppTxNoCloseVer\t0x%x\n"
                        "EnableTxNoClose\t0x%x\n"
                        "NextHwDesCloPtr0\t0x%x\n"
                        "BeginHwDesCloPtr0\t0x%x\n"
                        "NextHwDesCloPtr1\t0x%x\n"
                        "BeginHwDesCloPtr1\t0x%x\n"
                        "InitRxDescType\t0x%x\n"
                        "RxDescLength\t0x%x\n"
                        "num_rx_rings\t0x%x\n"
                        "num_tx_rings\t0x%x\n"
                        "tot_rx_rings\t0x%x\n"
                        "tot_tx_rings\t0x%x\n"
                        "EnableRss\t0x%x\n"
                        "EnablePtp\t0x%x\n"
                        "ptp_master_mode\t0x%x\n"
                        "min_irq_nvecs\t0x%x\n"
                        "irq_nvecs\t0x%x\n"
                        "ring_lib_enabled\t0x%x\n"
                        "HwSuppIsrVer\t0x%x\n"
                        "HwCurrIsrVer\t0x%x\n"
#ifdef ENABLE_PTP_SUPPORT
                        "tx_hwtstamp_timeouts\t0x%x\n"
                        "tx_hwtstamp_skipped\t0x%x\n"
#endif
                        "random_mac\t0x%x\n"
                        "org_mac_addr\t%pM\n"
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                        "perm_addr\t%pM\n"
#endif
                        "dev_addr\t%pM\n",
                        MODULENAME,
                        RTL8125_VERSION,
                        tp->mcfg,
                        tp->chipset,
                        rtl_chip_info[tp->chipset].name,
                        dev->mtu,
                        tp->rx_ring[0].num_rx_desc,
                        tp->rx_ring[0].cur_rx,
                        tp->rx_ring[0].dirty_rx,
                        tp->rx_ring[1].cur_rx,
                        tp->rx_ring[1].dirty_rx,
                        tp->rx_ring[2].cur_rx,
                        tp->rx_ring[2].dirty_rx,
                        tp->rx_ring[3].cur_rx,
                        tp->rx_ring[3].dirty_rx,
                        tp->tx_ring[0].num_tx_desc,
                        tp->tx_ring[0].cur_tx,
                        tp->tx_ring[0].dirty_tx,
                        tp->tx_ring[1].cur_tx,
                        tp->tx_ring[1].dirty_tx,
                        tp->rx_buf_sz,
#ifdef ENABLE_PAGE_REUSE
                        tp->rx_buf_page_order,
                        tp->rx_buf_page_size,
                        tp->page_reuse_fail_cnt,
#endif //ENABLE_PAGE_REUSE
                        tp->esd_flag,
                        tp->pci_cfg_is_read,
                        tp->rtl8125_rx_config,
                        tp->cp_cmd,
                        tp->intr_mask,
                        tp->timer_intr_mask,
                        tp->wol_enabled,
                        tp->wol_opts,
                        tp->efuse_ver,
                        tp->eeprom_type,
                        tp->autoneg,
                        tp->duplex,
                        tp->speed,
                        tp->advertising,
                        tp->eeprom_len,
                        tp->cur_page,
                        tp->bios_setting,
                        tp->features,
                        tp->org_pci_offset_99,
                        tp->org_pci_offset_180,
                        tp->issue_offset_99_event,
                        tp->org_pci_offset_80,
                        tp->org_pci_offset_81,
                        tp->use_timer_interrrupt,
                        tp->HwIcVerUnknown,
                        tp->NotWrRamCodeToMicroP,
                        tp->NotWrMcuPatchCode,
                        tp->HwHasWrRamCodeToMicroP,
                        tp->sw_ram_code_ver,
                        tp->hw_ram_code_ver,
                        tp->rtk_enable_diag,
                        tp->ShortPacketSwChecksum,
                        tp->UseSwPaddingShortPkt,
                        tp->RequireRduNonStopPatch,
                        tp->RequireAdcBiasPatch,
                        tp->AdcBiasPatchIoffset,
                        tp->RequireAdjustUpsTxLinkPulseTiming,
                        tp->SwrCnt1msIni,
                        tp->HwSuppNowIsOobVer,
                        tp->HwFiberModeVer,
                        tp->HwFiberStat,
                        tp->HwSwitchMdiToFiber,
                        tp->NicCustLedValue,
                        tp->RequiredSecLanDonglePatch,
                        tp->HwSuppDashVer,
                        tp->DASH,
                        tp->dash_printer_enabled,
                        tp->HwSuppKCPOffloadVer,
                        speed_mode,
                        duplex_mode,
                        autoneg_mode,
                        advertising_mode,
                        aspm,
                        s5wol,
                        s5_keep_curr_mac,
                        tp->eee.eee_enabled,
                        hwoptimize,
                        proc_init_num,
                        s0_magic_packet,
                        disable_pm_support,
                        enable_double_vlan,
                        tp->HwSuppMagicPktVer,
                        tp->HwSuppLinkChgWakeUpVer,
                        tp->HwSuppD0SpeedUpVer,
                        tp->D0SpeedUpSpeed,
                        tp->HwSuppCheckPhyDisableModeVer,
                        tp->HwPkgDet,
                        tp->HwSuppTxNoCloseVer,
                        tp->EnableTxNoClose,
                        tp->tx_ring[0].NextHwDesCloPtr,
                        tp->tx_ring[0].BeginHwDesCloPtr,
                        tp->tx_ring[1].NextHwDesCloPtr,
                        tp->tx_ring[1].BeginHwDesCloPtr,
                        tp->InitRxDescType,
                        tp->RxDescLength,
                        tp->num_rx_rings,
                        tp->num_tx_rings,
                        tp->tot_rx_rings,
                        tp->tot_tx_rings,
                        tp->EnableRss,
                        tp->EnablePtp,
                        tp->ptp_master_mode,
                        tp->min_irq_nvecs,
                        tp->irq_nvecs,
                        tp->ring_lib_enabled,
                        tp->HwSuppIsrVer,
                        tp->HwCurrIsrVer,
#ifdef ENABLE_PTP_SUPPORT
                        tp->tx_hwtstamp_timeouts,
                        tp->tx_hwtstamp_skipped,
#endif
                        tp->random_mac,
                        tp->org_mac_addr,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                        dev->perm_addr,
#endif
                        dev->dev_addr
                       );

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_tally_counter(char *page, char **start,
                                  off_t offset, int count,
                                  int *eof, void *data)
{
        struct net_device *dev = data;
        struct rtl8125_private *tp = netdev_priv(dev);
        struct rtl8125_counters *counters;
        dma_addr_t paddr;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Tally Counter\n");

        rtnl_lock();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters) {
                len += snprintf(page + len, count - len,
                                "\nDump Tally Counter Fail\n");
                goto out_unlock;
        }

        rtl8125_dump_tally_counter(tp, paddr);

        len += snprintf(page + len, count - len,
                        "Statistics\tValue\n----------\t-----\n");

        len += snprintf(page + len, count - len,
                        "tx_packets\t%lld\n"
                        "rx_packets\t%lld\n"
                        "tx_errors\t%lld\n"
                        "rx_errors\t%d\n"
                        "rx_missed\t%d\n"
                        "align_errors\t%d\n"
                        "tx_one_collision\t%d\n"
                        "tx_multi_collision\t%d\n"
                        "rx_unicast\t%lld\n"
                        "rx_broadcast\t%lld\n"
                        "rx_multicast\t%d\n"
                        "tx_aborted\t%d\n"
                        "tx_underrun\t%d\n",

                        "tx_octets\t%lld\n",
                        "rx_octets\t%lld\n",
                        "rx_multicast64\t%lld\n",
                        "tx_unicast64\t%lld\n",
                        "tx_broadcast64\t%lld\n",
                        "tx_multicast64\t%lld\n",
                        "tx_pause_on\t%d\n",
                        "tx_pause_off\t%d\n",
                        "tx_pause_all\t%d\n",
                        "tx_deferred\t%d\n",
                        "tx_late_collision\t%d\n",
                        "tx_all_collision\t%d\n",
                        "tx_aborted32\t%d\n",
                        "align_errors32\t%d\n",
                        "rx_frame_too_long\t%d\n",
                        "rx_runt\t%d\n",
                        "rx_pause_on\t%d\n",
                        "rx_pause_off\t%d\n",
                        "rx_pause_all\t%d\n",
                        "rx_unknown_opcode\t%d\n",
                        "rx_mac_error\t%d\n",
                        "tx_underrun32\t%d\n",
                        "rx_mac_missed\t%d\n",
                        "rx_tcam_dropped\t%d\n",
                        "tdu\t%d\n",
                        "rdu\t%d\n",
                        le64_to_cpu(counters->tx_packets),
                        le64_to_cpu(counters->rx_packets),
                        le64_to_cpu(counters->tx_errors),
                        le32_to_cpu(counters->rx_errors),
                        le16_to_cpu(counters->rx_missed),
                        le16_to_cpu(counters->align_errors),
                        le32_to_cpu(counters->tx_one_collision),
                        le32_to_cpu(counters->tx_multi_collision),
                        le64_to_cpu(counters->rx_unicast),
                        le64_to_cpu(counters->rx_broadcast),
                        le32_to_cpu(counters->rx_multicast),
                        le16_to_cpu(counters->tx_aborted),
                        le16_to_cpu(counters->tx_underrun),

                        le64_to_cpu(counters->tx_octets),
                        le64_to_cpu(counters->rx_octets),
                        le64_to_cpu(counters->rx_multicast64),
                        le64_to_cpu(counters->tx_unicast64),
                        le64_to_cpu(counters->tx_broadcast64),
                        le64_to_cpu(counters->tx_multicast64),
                        le32_to_cpu(counters->tx_pause_on),
                        le32_to_cpu(counters->tx_pause_off),
                        le32_to_cpu(counters->tx_pause_all),
                        le32_to_cpu(counters->tx_deferred),
                        le32_to_cpu(counters->tx_late_collision),
                        le32_to_cpu(counters->tx_all_collision),
                        le32_to_cpu(counters->tx_aborted32),
                        le32_to_cpu(counters->align_errors32),
                        le32_to_cpu(counters->rx_frame_too_long),
                        le32_to_cpu(counters->rx_runt),
                        le32_to_cpu(counters->rx_pause_on),
                        le32_to_cpu(counters->rx_pause_off),
                        le32_to_cpu(counters->rx_pause_all),
                        le32_to_cpu(counters->rx_unknown_opcode),
                        le32_to_cpu(counters->rx_mac_error),
                        le32_to_cpu(counters->tx_underrun32),
                        le32_to_cpu(counters->rx_mac_missed),
                        le32_to_cpu(counters->rx_tcam_dropped),
                        le32_to_cpu(counters->tdu),
                        le32_to_cpu(counters->rdu),
                       );

        len += snprintf(page + len, count - len, "\n");
out_unlock:
        rtnl_unlock();

        *eof = 1;
        return len;
}

static int proc_get_registers(char *page, char **start,
                              off_t offset, int count,
                              int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8125_MAC_REGS_SIZE;
        u8 byte_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump MAC Registers\n"
                        "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 16 && n < max; i++, n++) {
                        byte_rd = readb(ioaddr + n);
                        len += snprintf(page + len, count - len,
                                        "%02x ",
                                        byte_rd);
                }
        }

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_pcie_phy(char *page, char **start,
                             off_t offset, int count,
                             int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8125_EPHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump PCIE PHY\n"
                        "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8125_ephy_read(tp, n);
                        len += snprintf(page + len, count - len,
                                        "%04x ",
                                        word_rd);
                }
        }

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_eth_phy(char *page, char **start,
                            off_t offset, int count,
                            int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8125_PHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Ethernet PHY\n"
                        "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        len += snprintf(page + len, count - len,
                        "\n####################page 0##################\n");
        rtl8125_mdio_write(tp, 0x1f, 0x0000);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8125_mdio_read(tp, n);
                        len += snprintf(page + len, count - len,
                                        "%04x ",
                                        word_rd);
                }
        }

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_extended_registers(char *page, char **start,
                                       off_t offset, int count,
                                       int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8125_ERI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Extended Registers\n"
                        "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        dword_rd = rtl8125_eri_read(tp, n, 4, ERIAR_ExGMAC);
                        len += snprintf(page + len, count - len,
                                        "%08x ",
                                        dword_rd);
                }
        }

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_pci_registers(char *page, char **start,
                                  off_t offset, int count,
                                  int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8125_PCI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8125_private *tp = netdev_priv(dev);
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump PCI Registers\n"
                        "Offset\tValue\n------\t-----\n");

        rtnl_lock();

        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%03x:\t",
                                n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
                        len += snprintf(page + len, count - len,
                                        "%08x ",
                                        dword_rd);
                }
        }

        n = 0x110;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        len += snprintf(page + len, count - len,
                        "\n0x%03x:\t%08x ",
                        n,
                        dword_rd);
        n = 0x70c;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        len += snprintf(page + len, count - len,
                        "\n0x%03x:\t%08x ",
                        n,
                        dword_rd);

        rtnl_unlock();

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_temperature(char *page, char **start,
                                off_t offset, int count,
                                int *eof, void *data)
{
        struct net_device *dev = data;
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 ts_digout, tj, fah;
        int len = 0;

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                len += snprintf(page + len, count - len,
                                "\nChip Temperature\n");
                break;
        default:
                return -EOPNOTSUPP;
        }

        rtnl_lock();
        ts_digout = rtl8125_read_thermal_sensor(tp);
        rtnl_unlock();

        tj = ts_digout / 2;
        if (ts_digout <= 512) {
                tj = ts_digout / 2;
                len += snprintf(page + len, count - len,
                                "Cel:%d\n",
                                tj);
                fah = tj * (9/5) + 32;
                len += snprintf(page + len, count - len,
                                "Fah:%d\n",
                                fah);

        } else {
                tj = (512 - ((ts_digout / 2) - 512)) / 2;
                len += snprintf(page + len, count - len,
                                "Cel:-%d\n",
                                tj);
                fah = tj * (9/5) + 32;
                len += snprintf(page + len, count - len,
                                "Fah:-%d\n",
                                fah);
        }

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_cable_info(char *page, char **start,
                               off_t offset, int count,
                               int *eof, void *data)
{
        int i;
        u16 status;
        int len = 0;
        struct net_device *dev = data;
        int cp_status[RTL8125_CP_NUM] = {0};
        u16 cp_len[RTL8125_CP_NUM] = {0};
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2 ... CFG_METHOD_7:
                /* support */
                break;
        default:
                return -EOPNOTSUPP;
        }

        rtnl_lock();

        status = RTL_R16(tp, PHYstatus);
        if (status & LinkStatus)
                len += snprintf(page + len, count - len,
                                "\nlink speed:%d",
                                rtl8125_convert_link_speed(status));
        else
                len += snprintf(page + len, count - len,
                                "\nlink status:off");

        rtl8125_get_cp(tp, cp_len, cp_status);

        rtnl_unlock();

        len += snprintf(page + len, count - len,
                        "\npair\tlength\tstaus\n");

        for (i =0; i<RTL8125_CP_NUM; i++)
                len += snprintf(page + len, count - len,
                                "%d-%d\t%d\t%s\n",
                                i+1, i+2, cp_len[i],
                                rtl8125_get_cp_status_string(cp_status[i]));

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}
#endif
static void rtl8125_proc_module_init(void)
{
        //create /proc/net/r8125
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
        rtl8125_proc = proc_mkdir(MODULENAME, init_net.proc_net);
#else
        rtl8125_proc = proc_mkdir(MODULENAME, proc_net);
#endif
        if (!rtl8125_proc)
                dprintk("cannot create %s proc entry \n", MODULENAME);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
/*
 * seq_file wrappers for procfile show routines.
 */
static int rtl8125_proc_open(struct inode *inode, struct file *file)
{
        struct net_device *dev = proc_get_parent_data(inode);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
        int (*show)(struct seq_file *, void *) = pde_data(inode);
#else
        int (*show)(struct seq_file *, void *) = PDE_DATA(inode);
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)

        return single_open(file, show, dev);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops rtl8125_proc_fops = {
        .proc_open           = rtl8125_proc_open,
        .proc_read           = seq_read,
        .proc_lseek          = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations rtl8125_proc_fops = {
        .open           = rtl8125_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#endif

/*
 * Table of proc files we need to create.
 */
struct rtl8125_proc_file {
        char name[12];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        int (*show)(struct seq_file *, void *);
#else
        int (*show)(char *, char **, off_t, int, int *, void *);
#endif
};

static const struct rtl8125_proc_file rtl8125_proc_files[] = {
        { "driver_var", &proc_get_driver_variable },
        { "tally", &proc_get_tally_counter },
        { "registers", &proc_get_registers },
        { "pcie_phy", &proc_get_pcie_phy },
        { "eth_phy", &proc_get_eth_phy },
        { "ext_regs", &proc_get_extended_registers },
        { "pci_regs", &proc_get_pci_registers },
        { "temp", &proc_get_temperature },
        { "cdt", &proc_get_cable_info },
        { "", NULL }
};

static void rtl8125_proc_init(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        const struct rtl8125_proc_file *f;
        struct proc_dir_entry *dir;

        if (rtl8125_proc && !tp->proc_dir) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                dir = proc_mkdir_data(dev->name, 0, rtl8125_proc, dev);
                if (!dir) {
                        printk("Unable to initialize /proc/net/%s/%s\n",
                               MODULENAME, dev->name);
                        return;
                }

                tp->proc_dir = dir;
                proc_init_num++;

                for (f = rtl8125_proc_files; f->name[0]; f++) {
                        if (!proc_create_data(f->name, S_IFREG | S_IRUGO, dir,
                                              &rtl8125_proc_fops, f->show)) {
                                printk("Unable to initialize "
                                       "/proc/net/%s/%s/%s\n",
                                       MODULENAME, dev->name, f->name);
                                return;
                        }
                }
#else
                dir = proc_mkdir(dev->name, rtl8125_proc);
                if (!dir) {
                        printk("Unable to initialize /proc/net/%s/%s\n",
                               MODULENAME, dev->name);
                        return;
                }

                tp->proc_dir = dir;
                proc_init_num++;

                for (f = rtl8125_proc_files; f->name[0]; f++) {
                        if (!create_proc_read_entry(f->name, S_IFREG | S_IRUGO,
                                                    dir, f->show, dev)) {
                                printk("Unable to initialize "
                                       "/proc/net/%s/%s/%s\n",
                                       MODULENAME, dev->name, f->name);
                                return;
                        }
                }
#endif
        }
}

static void rtl8125_proc_remove(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->proc_dir) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                remove_proc_subtree(dev->name, rtl8125_proc);
                proc_init_num--;

#else
                const struct rtl8125_proc_file *f;
                struct rtl8125_private *tp = netdev_priv(dev);

                for (f = rtl8125_proc_files; f->name[0]; f++)
                        remove_proc_entry(f->name, tp->proc_dir);

                remove_proc_entry(dev->name, rtl8125_proc);
                proc_init_num--;
#endif
                tp->proc_dir = NULL;
        }
}

#endif //ENABLE_R8125_PROCFS

static inline u16 map_phy_ocp_addr(u16 PageNum, u8 RegNum)
{
        u16 OcpPageNum = 0;
        u8 OcpRegNum = 0;
        u16 OcpPhyAddress = 0;

        if ( PageNum == 0 ) {
                OcpPageNum = OCP_STD_PHY_BASE_PAGE + ( RegNum / 8 );
                OcpRegNum = 0x10 + ( RegNum % 8 );
        } else {
                OcpPageNum = PageNum;
                OcpRegNum = RegNum;
        }

        OcpPageNum <<= 4;

        if ( OcpRegNum < 16 ) {
                OcpPhyAddress = 0;
        } else {
                OcpRegNum -= 16;
                OcpRegNum <<= 1;

                OcpPhyAddress = OcpPageNum + OcpRegNum;
        }


        return OcpPhyAddress;
}

static void mdio_real_direct_write_phy_ocp(struct rtl8125_private *tp,
                u16 RegAddr,
                u16 value)
{
        u32 data32;
        int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(RegAddr % 2);
#endif
        data32 = RegAddr/2;
        data32 <<= OCPR_Addr_Reg_shift;
        data32 |= OCPR_Write | value;

        RTL_W32(tp, PHYOCP, data32);
        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                if (!(RTL_R32(tp, PHYOCP) & OCPR_Flag))
                        break;
        }
}

static void mdio_direct_write_phy_ocp(struct rtl8125_private *tp,
                                      u16 RegAddr,
                                      u16 value)
{
        if (tp->rtk_enable_diag) return;

        mdio_real_direct_write_phy_ocp(tp, RegAddr, value);
}

/*
static void rtl8125_mdio_write_phy_ocp(struct rtl8125_private *tp,
                                       u16 PageNum,
                                       u32 RegAddr,
                                       u32 value)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        mdio_direct_write_phy_ocp(tp, ocp_addr, value);
}
*/

static void rtl8125_mdio_real_write_phy_ocp(struct rtl8125_private *tp,
                u16 PageNum,
                u32 RegAddr,
                u32 value)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        mdio_real_direct_write_phy_ocp(tp, ocp_addr, value);
}

static void mdio_real_write(struct rtl8125_private *tp,
                            u16 RegAddr,
                            u16 value)
{
        if (RegAddr == 0x1F) {
                tp->cur_page = value;
                return;
        }
        rtl8125_mdio_real_write_phy_ocp(tp, tp->cur_page, RegAddr, value);
}

void rtl8125_mdio_write(struct rtl8125_private *tp,
                        u16 RegAddr,
                        u16 value)
{
        if (tp->rtk_enable_diag) return;

        mdio_real_write(tp, RegAddr, value);
}

void rtl8125_mdio_prot_write(struct rtl8125_private *tp,
                             u32 RegAddr,
                             u32 value)
{
        mdio_real_write(tp, RegAddr, value);
}

void rtl8125_mdio_prot_direct_write_phy_ocp(struct rtl8125_private *tp,
                u32 RegAddr,
                u32 value)
{
        mdio_real_direct_write_phy_ocp(tp, RegAddr, value);
}

static u32 mdio_real_direct_read_phy_ocp(struct rtl8125_private *tp,
                u16 RegAddr)
{
        u32 data32;
        int i, value = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(RegAddr % 2);
#endif
        data32 = RegAddr/2;
        data32 <<= OCPR_Addr_Reg_shift;

        RTL_W32(tp, PHYOCP, data32);
        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                if (RTL_R32(tp, PHYOCP) & OCPR_Flag)
                        break;
        }
        value = RTL_R32(tp, PHYOCP) & OCPDR_Data_Mask;

        return value;
}

static u32 mdio_direct_read_phy_ocp(struct rtl8125_private *tp,
                                    u16 RegAddr)
{
        if (tp->rtk_enable_diag) return 0xffffffff;

        return mdio_real_direct_read_phy_ocp(tp, RegAddr);
}

/*
static u32 rtl8125_mdio_read_phy_ocp(struct rtl8125_private *tp,
                                     u16 PageNum,
                                     u32 RegAddr)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        return mdio_direct_read_phy_ocp(tp, ocp_addr);
}
*/

static u32 rtl8125_mdio_real_read_phy_ocp(struct rtl8125_private *tp,
                u16 PageNum,
                u32 RegAddr)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        return mdio_real_direct_read_phy_ocp(tp, ocp_addr);
}

static u32 mdio_real_read(struct rtl8125_private *tp,
                          u16 RegAddr)
{
        return rtl8125_mdio_real_read_phy_ocp(tp, tp->cur_page, RegAddr);
}

u32 rtl8125_mdio_read(struct rtl8125_private *tp,
                      u16 RegAddr)
{
        if (tp->rtk_enable_diag) return 0xffffffff;

        return mdio_real_read(tp, RegAddr);
}

u32 rtl8125_mdio_prot_read(struct rtl8125_private *tp,
                           u32 RegAddr)
{
        return mdio_real_read(tp, RegAddr);
}

u32 rtl8125_mdio_prot_direct_read_phy_ocp(struct rtl8125_private *tp,
                u32 RegAddr)
{
        return mdio_real_direct_read_phy_ocp(tp, RegAddr);
}

static void ClearAndSetEthPhyBit(struct rtl8125_private *tp, u8  addr, u16 clearmask, u16 setmask)
{
        u16 PhyRegValue;

        PhyRegValue = rtl8125_mdio_read(tp, addr);
        PhyRegValue &= ~clearmask;
        PhyRegValue |= setmask;
        rtl8125_mdio_write(tp, addr, PhyRegValue);
}

void rtl8125_clear_eth_phy_bit(struct rtl8125_private *tp, u8 addr, u16 mask)
{
        ClearAndSetEthPhyBit(tp,
                             addr,
                             mask,
                             0
                            );
}

void rtl8125_set_eth_phy_bit(struct rtl8125_private *tp,  u8  addr, u16  mask)
{
        ClearAndSetEthPhyBit(tp,
                             addr,
                             0,
                             mask
                            );
}

static void ClearAndSetEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 clearmask, u16 setmask)
{
        u16 PhyRegValue;

        PhyRegValue = mdio_direct_read_phy_ocp(tp, addr);
        PhyRegValue &= ~clearmask;
        PhyRegValue |= setmask;
        mdio_direct_write_phy_ocp(tp, addr, PhyRegValue);
}

void ClearEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 mask)
{
        ClearAndSetEthPhyOcpBit(tp,
                                addr,
                                mask,
                                0
                               );
}

void SetEthPhyOcpBit(struct rtl8125_private *tp,  u16 addr, u16 mask)
{
        ClearAndSetEthPhyOcpBit(tp,
                                addr,
                                0,
                                mask
                               );
}

void rtl8125_mac_ocp_write(struct rtl8125_private *tp, u16 reg_addr, u16 value)
{
        u32 data32;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(reg_addr % 2);
#endif

        data32 = reg_addr/2;
        data32 <<= OCPR_Addr_Reg_shift;
        data32 += value;
        data32 |= OCPR_Write;

        RTL_W32(tp, MACOCP, data32);
}

u32 rtl8125_mac_ocp_read(struct rtl8125_private *tp, u16 reg_addr)
{
        u32 data32;
        u16 data16 = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(reg_addr % 2);
#endif

        data32 = reg_addr/2;
        data32 <<= OCPR_Addr_Reg_shift;

        RTL_W32(tp, MACOCP, data32);
        data16 = (u16)RTL_R32(tp, MACOCP);

        return data16;
}

#ifdef ENABLE_USE_FIRMWARE_FILE
static void mac_mcu_write(struct rtl8125_private *tp, u16 reg, u16 value)
{
        if (reg == 0x1f) {
                tp->ocp_base = value << 4;
                return;
        }

        rtl8125_mac_ocp_write(tp, tp->ocp_base + reg, value);
}

static u32 mac_mcu_read(struct rtl8125_private *tp, u16 reg)
{
        return rtl8125_mac_ocp_read(tp, tp->ocp_base + reg);
}
#endif

static void
ClearAndSetMcuAccessRegBit(
        struct rtl8125_private *tp,
        u16   addr,
        u16   clearmask,
        u16   setmask
)
{
        u16 PhyRegValue;

        PhyRegValue = rtl8125_mac_ocp_read(tp, addr);
        PhyRegValue &= ~clearmask;
        PhyRegValue |= setmask;
        rtl8125_mac_ocp_write(tp, addr, PhyRegValue);
}

static void
ClearMcuAccessRegBit(
        struct rtl8125_private *tp,
        u16   addr,
        u16   mask
)
{
        ClearAndSetMcuAccessRegBit(tp,
                                   addr,
                                   mask,
                                   0
                                  );
}

static void
SetMcuAccessRegBit(
        struct rtl8125_private *tp,
        u16   addr,
        u16   mask
)
{
        ClearAndSetMcuAccessRegBit(tp,
                                   addr,
                                   0,
                                   mask
                                  );
}

u32 rtl8125_ocp_read_with_oob_base_address(struct rtl8125_private *tp, u16 addr, u8 len, const u32 base_address)
{
        return rtl8125_eri_read_with_oob_base_address(tp, addr, len, ERIAR_OOB, base_address);
}

u32 rtl8125_ocp_read(struct rtl8125_private *tp, u16 addr, u8 len)
{
        u32 value = 0;

        if (HW_DASH_SUPPORT_TYPE_2(tp))
                value = rtl8125_ocp_read_with_oob_base_address(tp, addr, len, NO_BASE_ADDRESS);
        else if (HW_DASH_SUPPORT_TYPE_3(tp))
                value = rtl8125_ocp_read_with_oob_base_address(tp, addr, len, RTL8168FP_OOBMAC_BASE);

        return value;
}

u32 rtl8125_ocp_write_with_oob_base_address(struct rtl8125_private *tp, u16 addr, u8 len, u32 value, const u32 base_address)
{
        return rtl8125_eri_write_with_oob_base_address(tp, addr, len, value, ERIAR_OOB, base_address);
}

void rtl8125_ocp_write(struct rtl8125_private *tp, u16 addr, u8 len, u32 value)
{
        if (HW_DASH_SUPPORT_TYPE_2(tp))
                rtl8125_ocp_write_with_oob_base_address(tp, addr, len, value, NO_BASE_ADDRESS);
        else if (HW_DASH_SUPPORT_TYPE_3(tp))
                rtl8125_ocp_write_with_oob_base_address(tp, addr, len, value, RTL8168FP_OOBMAC_BASE);
}

void rtl8125_oob_mutex_lock(struct rtl8125_private *tp)
{
        u8 reg_16, reg_a0;
        u32 wait_cnt_0, wait_Cnt_1;
        u16 ocp_reg_mutex_ib;
        u16 ocp_reg_mutex_oob;
        u16 ocp_reg_mutex_prio;

        if (!tp->DASH) return;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                ocp_reg_mutex_oob = 0x110;
                ocp_reg_mutex_ib = 0x114;
                ocp_reg_mutex_prio = 0x11C;
                break;
        default:
                return;
        }

        rtl8125_ocp_write(tp, ocp_reg_mutex_ib, 1, BIT_0);
        reg_16 = rtl8125_ocp_read(tp, ocp_reg_mutex_oob, 1);
        wait_cnt_0 = 0;
        while(reg_16) {
                reg_a0 = rtl8125_ocp_read(tp, ocp_reg_mutex_prio, 1);
                if (reg_a0) {
                        rtl8125_ocp_write(tp, ocp_reg_mutex_ib, 1, 0x00);
                        reg_a0 = rtl8125_ocp_read(tp, ocp_reg_mutex_prio, 1);
                        wait_Cnt_1 = 0;
                        while(reg_a0) {
                                reg_a0 = rtl8125_ocp_read(tp, ocp_reg_mutex_prio, 1);

                                wait_Cnt_1++;

                                if (wait_Cnt_1 > 2000)
                                        break;
                        };
                        rtl8125_ocp_write(tp, ocp_reg_mutex_ib, 1, BIT_0);

                }
                reg_16 = rtl8125_ocp_read(tp, ocp_reg_mutex_oob, 1);

                wait_cnt_0++;

                if (wait_cnt_0 > 2000)
                        break;
        };
}

void rtl8125_oob_mutex_unlock(struct rtl8125_private *tp)
{
        u16 ocp_reg_mutex_ib;
        u16 ocp_reg_mutex_oob;
        u16 ocp_reg_mutex_prio;

        if (!tp->DASH) return;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                ocp_reg_mutex_oob = 0x110;
                ocp_reg_mutex_ib = 0x114;
                ocp_reg_mutex_prio = 0x11C;
                break;
        default:
                return;
        }

        rtl8125_ocp_write(tp, ocp_reg_mutex_prio, 1, BIT_0);
        rtl8125_ocp_write(tp, ocp_reg_mutex_ib, 1, 0x00);
}

void rtl8125_oob_notify(struct rtl8125_private *tp, u8 cmd)
{
        rtl8125_eri_write(tp, 0xE8, 1, cmd, ERIAR_ExGMAC);

        rtl8125_ocp_write(tp, 0x30, 1, 0x01);
}

static int rtl8125_check_dash(struct rtl8125_private *tp)
{
        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                if (rtl8125_ocp_read(tp, 0x128, 1) & BIT_0)
                        return 1;
        }

        return 0;
}

void rtl8125_dash2_disable_tx(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                u16 WaitCnt;
                u8 TmpUchar;

                //Disable oob Tx
                RTL_CMAC_W8(tp, CMAC_IBCR2, RTL_CMAC_R8(tp, CMAC_IBCR2) & ~( BIT_0 ));
                WaitCnt = 0;

                //wait oob tx disable
                do {
                        TmpUchar = RTL_CMAC_R8(tp, CMAC_IBISR0);

                        if ( TmpUchar & ISRIMR_DASH_TYPE2_TX_DISABLE_IDLE ) {
                                break;
                        }

                        udelay( 50 );
                        WaitCnt++;
                } while(WaitCnt < 2000);

                //Clear ISRIMR_DASH_TYPE2_TX_DISABLE_IDLE
                RTL_CMAC_W8(tp, CMAC_IBISR0, RTL_CMAC_R8(tp, CMAC_IBISR0) | ISRIMR_DASH_TYPE2_TX_DISABLE_IDLE);
        }
}

void rtl8125_dash2_enable_tx(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                RTL_CMAC_W8(tp, CMAC_IBCR2, RTL_CMAC_R8(tp, CMAC_IBCR2) | BIT_0);
        }
}

void rtl8125_dash2_disable_rx(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                RTL_CMAC_W8(tp, CMAC_IBCR0, RTL_CMAC_R8(tp, CMAC_IBCR0) & ~( BIT_0 ));
        }
}

void rtl8125_dash2_enable_rx(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                RTL_CMAC_W8(tp, CMAC_IBCR0, RTL_CMAC_R8(tp, CMAC_IBCR0) | BIT_0);
        }
}

static void rtl8125_dash2_disable_txrx(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                rtl8125_dash2_disable_tx( tp );
                rtl8125_dash2_disable_rx( tp );
        }
}

static int rtl8125_wait_dash_fw_ready(struct rtl8125_private *tp)
{
        int rc = -1;
        int timeout;

        if (HW_DASH_SUPPORT_TYPE_2(tp) == FALSE &&
            HW_DASH_SUPPORT_TYPE_3(tp) == FALSE)
                goto out;

        if (!tp->DASH)
                goto out;

        for (timeout = 0; timeout < 10; timeout++) {
                mdelay(10);
                if (rtl8125_ocp_read(tp, 0x124, 1) & BIT_0) {
                        rc = 1;
                        goto out;
                }
        }

        rc = 0;

out:
        return rc;
}

static void rtl8125_driver_start(struct rtl8125_private *tp)
{
        u32 tmp_value;

        if (HW_DASH_SUPPORT_TYPE_2(tp) == FALSE &&
            HW_DASH_SUPPORT_TYPE_3(tp) == FALSE)
                return;

        rtl8125_ocp_write(tp, 0x180, 1, OOB_CMD_DRIVER_START);
        tmp_value = rtl8125_ocp_read(tp, 0x30, 1);
        tmp_value |= BIT_0;
        rtl8125_ocp_write(tp, 0x30, 1, tmp_value);

        rtl8125_wait_dash_fw_ready(tp);
}

static void rtl8125_driver_stop(struct rtl8125_private *tp)
{
        u32 tmp_value;
        struct net_device *dev = tp->dev;

        if (HW_DASH_SUPPORT_TYPE_2(tp) == FALSE &&
            HW_DASH_SUPPORT_TYPE_3(tp) == FALSE)
                return;

        rtl8125_dash2_disable_txrx(dev);

        rtl8125_ocp_write(tp, 0x180, 1, OOB_CMD_DRIVER_STOP);
        tmp_value = rtl8125_ocp_read(tp, 0x30, 1);
        tmp_value |= BIT_0;
        rtl8125_ocp_write(tp, 0x30, 1, tmp_value);

        rtl8125_wait_dash_fw_ready(tp);
}

void rtl8125_ephy_write(struct rtl8125_private *tp, int RegAddr, int value)
{
        int i;

        RTL_W32(tp, EPHYAR,
                EPHYAR_Write |
                (RegAddr & EPHYAR_Reg_Mask_v2) << EPHYAR_Reg_shift |
                (value & EPHYAR_Data_Mask));

        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                /* Check if the RTL8125 has completed EPHY write */
                if (!(RTL_R32(tp, EPHYAR) & EPHYAR_Flag))
                        break;
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);
}

u16 rtl8125_ephy_read(struct rtl8125_private *tp, int RegAddr)
{
        int i;
        u16 value = 0xffff;

        RTL_W32(tp, EPHYAR,
                EPHYAR_Read | (RegAddr & EPHYAR_Reg_Mask_v2) << EPHYAR_Reg_shift);

        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                /* Check if the RTL8125 has completed EPHY read */
                if (RTL_R32(tp, EPHYAR) & EPHYAR_Flag) {
                        value = (u16) (RTL_R32(tp, EPHYAR) & EPHYAR_Data_Mask);
                        break;
                }
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);

        return value;
}

static void ClearAndSetPCIePhyBit(struct rtl8125_private *tp, u8 addr, u16 clearmask, u16 setmask)
{
        u16 EphyValue;

        EphyValue = rtl8125_ephy_read(tp, addr);
        EphyValue &= ~clearmask;
        EphyValue |= setmask;
        rtl8125_ephy_write(tp, addr, EphyValue);
}

static void ClearPCIePhyBit(struct rtl8125_private *tp, u8 addr, u16 mask)
{
        ClearAndSetPCIePhyBit( tp,
                               addr,
                               mask,
                               0
                             );
}

static void SetPCIePhyBit( struct rtl8125_private *tp, u8 addr, u16 mask)
{
        ClearAndSetPCIePhyBit( tp,
                               addr,
                               0,
                               mask
                             );
}

static u32
rtl8125_csi_other_fun_read(struct rtl8125_private *tp,
                           u8 multi_fun_sel_bit,
                           u32 addr)
{
        u32 cmd;
        int i;
        u32 value = 0xffffffff;

        cmd = CSIAR_Read | CSIAR_ByteEn << CSIAR_ByteEn_shift | (addr & CSIAR_Addr_Mask);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                multi_fun_sel_bit = 0;

        if (multi_fun_sel_bit > 7)
                goto exit;

        cmd |= multi_fun_sel_bit << 16;

        RTL_W32(tp, CSIAR, cmd);

        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                /* Check if the RTL8125 has completed CSI read */
                if (RTL_R32(tp, CSIAR) & CSIAR_Flag) {
                        value = (u32)RTL_R32(tp, CSIDR);
                        break;
                }
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);

exit:
        return value;
}

static void
rtl8125_csi_other_fun_write(struct rtl8125_private *tp,
                            u8 multi_fun_sel_bit,
                            u32 addr,
                            u32 value)
{
        u32 cmd;
        int i;

        RTL_W32(tp, CSIDR, value);
        cmd = CSIAR_Write | CSIAR_ByteEn << CSIAR_ByteEn_shift | (addr & CSIAR_Addr_Mask);
        if (tp->mcfg == CFG_METHOD_DEFAULT)
                multi_fun_sel_bit = 0;

        if ( multi_fun_sel_bit > 7 )
                return;

        cmd |= multi_fun_sel_bit << 16;

        RTL_W32(tp, CSIAR, cmd);

        for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8125_CHANNEL_WAIT_TIME);

                /* Check if the RTL8125 has completed CSI write */
                if (!(RTL_R32(tp, CSIAR) & CSIAR_Flag))
                        break;
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);
}

static u32
rtl8125_csi_read(struct rtl8125_private *tp,
                 u32 addr)
{
        u8 multi_fun_sel_bit;

        multi_fun_sel_bit = 0;

        return rtl8125_csi_other_fun_read(tp, multi_fun_sel_bit, addr);
}

static void
rtl8125_csi_write(struct rtl8125_private *tp,
                  u32 addr,
                  u32 value)
{
        u8 multi_fun_sel_bit;

        multi_fun_sel_bit = 0;

        rtl8125_csi_other_fun_write(tp, multi_fun_sel_bit, addr, value);
}

static u8
rtl8125_csi_fun0_read_byte(struct rtl8125_private *tp,
                           u32 addr)
{
        u8 RetVal = 0;

        if (tp->mcfg == CFG_METHOD_DEFAULT) {
                struct pci_dev *pdev = tp->pci_dev;

                pci_read_config_byte(pdev, addr, &RetVal);
        } else {
                u32 TmpUlong;
                u16 RegAlignAddr;
                u8 ShiftByte;

                RegAlignAddr = addr & ~(0x3);
                ShiftByte = addr & (0x3);
                TmpUlong = rtl8125_csi_other_fun_read(tp, 0, RegAlignAddr);
                TmpUlong >>= (8*ShiftByte);
                RetVal = (u8)TmpUlong;
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);

        return RetVal;
}

static void
rtl8125_csi_fun0_write_byte(struct rtl8125_private *tp,
                            u32 addr,
                            u8 value)
{
        if (tp->mcfg == CFG_METHOD_DEFAULT) {
                struct pci_dev *pdev = tp->pci_dev;

                pci_write_config_byte(pdev, addr, value);
        } else {
                u32 TmpUlong;
                u16 RegAlignAddr;
                u8 ShiftByte;

                RegAlignAddr = addr & ~(0x3);
                ShiftByte = addr & (0x3);
                TmpUlong = rtl8125_csi_other_fun_read(tp, 0, RegAlignAddr);
                TmpUlong &= ~(0xFF << (8*ShiftByte));
                TmpUlong |= (value << (8*ShiftByte));
                rtl8125_csi_other_fun_write( tp, 0, RegAlignAddr, TmpUlong );
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);
}

u32 rtl8125_eri_read_with_oob_base_address(struct rtl8125_private *tp, int addr, int len, int type, const u32 base_address)
{
        int i, val_shift, shift = 0;
        u32 value1 = 0, value2 = 0, mask;
        u32 eri_cmd;
        const u32 transformed_base_address = ((base_address & 0x00FFF000) << 6) | (base_address & 0x000FFF);

        if (len > 4 || len <= 0)
                return -1;

        while (len > 0) {
                val_shift = addr % ERIAR_Addr_Align;
                addr = addr & ~0x3;

                eri_cmd = ERIAR_Read |
                          transformed_base_address |
                          type << ERIAR_Type_shift |
                          ERIAR_ByteEn << ERIAR_ByteEn_shift |
                          (addr & 0x0FFF);
                if (addr & 0xF000) {
                        u32 tmp;

                        tmp = addr & 0xF000;
                        tmp >>= 12;
                        eri_cmd |= (tmp << 20) & 0x00F00000;
                }

                RTL_W32(tp, ERIAR, eri_cmd);

                for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8125_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8125 has completed ERI read */
                        if (RTL_R32(tp, ERIAR) & ERIAR_Flag)
                                break;
                }

                if (len == 1)       mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 2)  mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 3)  mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else            mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

                value1 = RTL_R32(tp, ERIDR) & mask;
                value2 |= (value1 >> val_shift * 8) << shift * 8;

                if (len <= 4 - val_shift) {
                        len = 0;
                } else {
                        len -= (4 - val_shift);
                        shift = 4 - val_shift;
                        addr += 4;
                }
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);

        return value2;
}

u32 rtl8125_eri_read(struct rtl8125_private *tp, int addr, int len, int type)
{
        return rtl8125_eri_read_with_oob_base_address(tp, addr, len, type, 0);
}

int rtl8125_eri_write_with_oob_base_address(struct rtl8125_private *tp, int addr, int len, u32 value, int type, const u32 base_address)
{
        int i, val_shift, shift = 0;
        u32 value1 = 0, mask;
        u32 eri_cmd;
        const u32 transformed_base_address = ((base_address & 0x00FFF000) << 6) | (base_address & 0x000FFF);

        if (len > 4 || len <= 0)
                return -1;

        while (len > 0) {
                val_shift = addr % ERIAR_Addr_Align;
                addr = addr & ~0x3;

                if (len == 1)       mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 2)  mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 3)  mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else            mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

                value1 = rtl8125_eri_read_with_oob_base_address(tp, addr, 4, type, base_address) & ~mask;
                value1 |= ((value << val_shift * 8) >> shift * 8);

                RTL_W32(tp, ERIDR, value1);

                eri_cmd = ERIAR_Write |
                          transformed_base_address |
                          type << ERIAR_Type_shift |
                          ERIAR_ByteEn << ERIAR_ByteEn_shift |
                          (addr & 0x0FFF);
                if (addr & 0xF000) {
                        u32 tmp;

                        tmp = addr & 0xF000;
                        tmp >>= 12;
                        eri_cmd |= (tmp << 20) & 0x00F00000;
                }

                RTL_W32(tp, ERIAR, eri_cmd);

                for (i = 0; i < R8125_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8125_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8125 has completed ERI write */
                        if (!(RTL_R32(tp, ERIAR) & ERIAR_Flag))
                                break;
                }

                if (len <= 4 - val_shift) {
                        len = 0;
                } else {
                        len -= (4 - val_shift);
                        shift = 4 - val_shift;
                        addr += 4;
                }
        }

        udelay(R8125_CHANNEL_EXIT_DELAY_TIME);

        return 0;
}

int rtl8125_eri_write(struct rtl8125_private *tp, int addr, int len, u32 value, int type)
{
        return rtl8125_eri_write_with_oob_base_address(tp, addr, len, value, type, NO_BASE_ADDRESS);
}

static void
rtl8125_enable_rxdvgate(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_3);
                mdelay(2);
                break;
        }
}

static void
rtl8125_disable_rxdvgate(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_3);
                mdelay(2);
                break;
        }
}

static u8
rtl8125_is_gpio_low(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u8 gpio_low = FALSE;

        switch (tp->HwSuppCheckPhyDisableModeVer) {
        case 3:
                if (!(rtl8125_mac_ocp_read(tp, 0xDC04) & BIT_13))
                        gpio_low = TRUE;
                break;
        }

        if (gpio_low)
                dprintk("gpio is low.\n");

        return gpio_low;
}

static u8
rtl8125_is_phy_disable_mode_enabled(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u8 phy_disable_mode_enabled = FALSE;

        switch (tp->HwSuppCheckPhyDisableModeVer) {
        case 3:
                if (RTL_R8(tp, 0xF2) & BIT_5)
                        phy_disable_mode_enabled = TRUE;
                break;
        }

        if (phy_disable_mode_enabled)
                dprintk("phy disable mode enabled.\n");

        return phy_disable_mode_enabled;
}

static u8
rtl8125_is_in_phy_disable_mode(struct net_device *dev)
{
        u8 in_phy_disable_mode = FALSE;

        if (rtl8125_is_phy_disable_mode_enabled(dev) && rtl8125_is_gpio_low(dev))
                in_phy_disable_mode = TRUE;

        if (in_phy_disable_mode)
                dprintk("Hardware is in phy disable mode.\n");

        return in_phy_disable_mode;
}

static bool
rtl8125_stop_all_request(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        RTL_W8(tp, ChipCmd, RTL_R8(tp, ChipCmd) | StopReq);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                for (i = 0; i < 20; i++) {
                        udelay(10);
                        if (!(RTL_R8(tp, ChipCmd) & StopReq)) break;
                }

                if (i == 20)
                        return 0;
                break;
        }

        return 1;
}

void
rtl8125_wait_txrx_fifo_empty(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_stop_all_request(dev);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                for (i = 0; i < 3000; i++) {
                        udelay(50);
                        if ((RTL_R8(tp, MCUCmd_reg) & (Txfifo_empty | Rxfifo_empty)) == (Txfifo_empty | Rxfifo_empty))
                                break;
                }
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                for (i = 0; i < 3000; i++) {
                        udelay(50);
                        if ((RTL_R16(tp, IntrMitigate) & (BIT_0 | BIT_1 | BIT_8)) == (BIT_0 | BIT_1 | BIT_8))
                                break;
                }
                break;
        }
}

#ifdef ENABLE_DASH_SUPPORT

static inline void
rtl8125_enable_dash2_interrupt(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                RTL_CMAC_W8(tp, CMAC_IBIMR0, ( ISRIMR_DASH_TYPE2_ROK | ISRIMR_DASH_TYPE2_TOK | ISRIMR_DASH_TYPE2_TDU | ISRIMR_DASH_TYPE2_RDU | ISRIMR_DASH_TYPE2_RX_DISABLE_IDLE ));
        }
}

static inline void
rtl8125_disable_dash2_interrupt(struct rtl8125_private *tp)
{
        if (!tp->DASH) return;

        if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                RTL_CMAC_W8(tp, CMAC_IBIMR0, 0);
        }
}
#endif

void
rtl8125_enable_hw_linkchg_interrupt(struct rtl8125_private *tp)
{
        switch (tp->HwCurrIsrVer) {
        case 2:
                RTL_W32(tp, IMR_V2_SET_REG_8125, ISRIMR_V2_LINKCHG);
                break;
        case 1:
                RTL_W32(tp, tp->imr_reg[0], LinkChg | RTL_R32(tp, tp->imr_reg[0]));
                break;
        }

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH)
                rtl8125_enable_dash2_interrupt(tp);
#endif
}

static inline void
rtl8125_enable_hw_interrupt(struct rtl8125_private *tp)
{
        switch (tp->HwCurrIsrVer) {
        case 2:
                RTL_W32(tp, IMR_V2_SET_REG_8125, tp->intr_mask);
                break;
        case 1:
                RTL_W32(tp, tp->imr_reg[0], tp->intr_mask);

                if (R8125_MULTI_RX_Q(tp)) {
                        int i;
                        for (i=1; i<tp->num_rx_rings; i++)
                                RTL_W16(tp, tp->imr_reg[i], other_q_intr_mask);
                }
                break;
        }

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH)
                rtl8125_enable_dash2_interrupt(tp);
#endif
}

static inline void rtl8125_clear_hw_isr_v2(struct rtl8125_private *tp,
                u32 message_id)
{
        RTL_W32(tp, ISR_V2_8125, BIT(message_id));
}

static inline void
rtl8125_disable_hw_interrupt(struct rtl8125_private *tp)
{
        if (tp->HwCurrIsrVer == 2) {
                RTL_W32(tp, IMR_V2_CLEAR_REG_8125, 0xFFFFFFFF);
        } else {
                RTL_W32(tp, tp->imr_reg[0], 0x0000);

                if (R8125_MULTI_RX_Q(tp)) {
                        int i;
                        for (i=1; i<tp->num_rx_rings; i++)
                                RTL_W16(tp, tp->imr_reg[i], 0);
                }

#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH)
                        rtl8125_disable_dash2_interrupt(tp);
#endif
        }
}

static inline void
rtl8125_switch_to_hw_interrupt(struct rtl8125_private *tp)
{
        RTL_W32(tp, TIMER_INT0_8125, 0x0000);

        rtl8125_enable_hw_interrupt(tp);
}

static inline void
rtl8125_switch_to_timer_interrupt(struct rtl8125_private *tp)
{
        if (tp->use_timer_interrrupt) {
                RTL_W32(tp, TIMER_INT0_8125, timer_count);
                RTL_W32(tp, TCTR0_8125, timer_count);
                RTL_W32(tp, tp->imr_reg[0], tp->timer_intr_mask);

#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH)
                        rtl8125_enable_dash2_interrupt(tp);
#endif
        } else {
                rtl8125_switch_to_hw_interrupt(tp);
        }
}

static void
rtl8125_irq_mask_and_ack(struct rtl8125_private *tp)
{
        rtl8125_disable_hw_interrupt(tp);

        if (tp->HwCurrIsrVer == 2) {
                RTL_W32(tp, ISR_V2_8125, 0xFFFFFFFF);
        } else {
#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH) {
                        if (tp->dash_printer_enabled) {
                                RTL_W32(tp, tp->isr_reg[0], RTL_R32(tp, tp->isr_reg[0]) &
                                        ~(ISRIMR_DASH_INTR_EN | ISRIMR_DASH_INTR_CMAC_RESET));
                        } else {
                                if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                                        RTL_CMAC_W8(tp, CMAC_IBISR0, RTL_CMAC_R8(tp, CMAC_IBISR0));
                                }
                        }
                } else {
                        RTL_W32(tp, tp->isr_reg[0], RTL_R32(tp, tp->isr_reg[0]));
                }
#else
                RTL_W32(tp, tp->isr_reg[0], RTL_R32(tp, tp->isr_reg[0]));
#endif
                if (R8125_MULTI_RX_Q(tp)) {
                        int i;
                        for (i=1; i<tp->num_rx_rings; i++)
                                RTL_W16(tp, tp->isr_reg[i], RTL_R16(tp, tp->isr_reg[i]));
                }
        }
}

static void
rtl8125_disable_rx_packet_filter(struct rtl8125_private *tp)
{

        RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) &
                ~(AcceptErr | AcceptRunt |AcceptBroadcast | AcceptMulticast |
                  AcceptMyPhys |  AcceptAllPhys));
}

static void
rtl8125_nic_reset(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        rtl8125_disable_rx_packet_filter(tp);

        rtl8125_enable_rxdvgate(dev);

        rtl8125_stop_all_request(dev);

        rtl8125_wait_txrx_fifo_empty(dev);

        mdelay(2);

        /* Soft reset the chip. */
        RTL_W8(tp, ChipCmd, CmdReset);

        /* Check that the chip has finished the reset. */
        for (i = 100; i > 0; i--) {
                udelay(100);
                if ((RTL_R8(tp, ChipCmd) & CmdReset) == 0)
                        break;
        }
}

static void
rtl8125_hw_set_interrupt_type(struct rtl8125_private *tp, u8 isr_ver)
{
        u8 tmp;

        switch (tp->HwSuppIsrVer) {
        case 2:
                tmp = RTL_R8(tp, INT_CFG0_8125);
                tmp &= ~(INT_CFG0_ENABLE_8125);
                if (isr_ver == 2)
                        tmp |= INT_CFG0_ENABLE_8125;
                RTL_W8(tp, INT_CFG0_8125, tmp);
                break;
        }
}

static void
rtl8125_hw_clear_timer_int(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W32(tp, TIMER_INT0_8125, 0x0000);
                RTL_W32(tp, TIMER_INT1_8125, 0x0000);
                RTL_W32(tp, TIMER_INT2_8125, 0x0000);
                RTL_W32(tp, TIMER_INT3_8125, 0x0000);
                break;
        }
}

static void
rtl8125_hw_clear_int_miti(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        switch (tp->HwSuppIntMitiVer) {
        case 3:
                //IntMITI_0-IntMITI_31
                for (i=0xA00; i<0xB00; i+=4)
                        RTL_W32(tp, i, 0x0000);
                break;
        case 4:
                //IntMITI_0-IntMITI_15
                for (i = 0xA00; i < 0xA80; i += 4)
                        RTL_W32(tp, i, 0x0000);

                RTL_W8(tp, INT_CFG0_8125, RTL_R8(tp, INT_CFG0_8125) &
                       ~(INT_CFG0_TIMEOUT0_BYPASS_8125 | INT_CFG0_MITIGATION_BYPASS_8125));

                RTL_W16(tp, INT_CFG1_8125, 0x0000);
                break;
        }
}

void
rtl8125_hw_set_timer_int_8125(struct rtl8125_private *tp,
                              u32 message_id,
                              u8 timer_intmiti_val)
{
        switch (tp->HwSuppIntMitiVer) {
        case 4:
#ifdef ENABLE_LIB_SUPPORT
                if (message_id < R8125_MAX_RX_QUEUES_VEC_V3)
                        timer_intmiti_val = 0;
#else
                if (tp->EnableRss && (message_id < R8125_MAX_RX_QUEUES_VEC_V3))
                        timer_intmiti_val = 0;
#endif //ENABLE_LIB_SUPPORT
                if (message_id < R8125_MAX_RX_QUEUES_VEC_V3) //ROK
                        RTL_W8(tp,INT_MITI_V2_0_RX + 8 * message_id, timer_intmiti_val);
                else if (message_id == 16) //TOK
                        RTL_W8(tp,INT_MITI_V2_0_TX, timer_intmiti_val);
                else if (message_id == 18) //TOK
                        RTL_W8(tp,INT_MITI_V2_1_TX, timer_intmiti_val);
                break;
        }
}

void
rtl8125_hw_reset(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_lib_reset_prepare(tp);

        /* Disable interrupts */
        rtl8125_irq_mask_and_ack(tp);

        rtl8125_hw_clear_timer_int(dev);

        rtl8125_nic_reset(dev);
}

static unsigned int
rtl8125_xmii_reset_pending(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        unsigned int retval;

        rtl8125_mdio_write(tp, 0x1f, 0x0000);
        retval = rtl8125_mdio_read(tp, MII_BMCR) & BMCR_RESET;

        return retval;
}

static unsigned int
rtl8125_xmii_link_ok(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        unsigned int retval;

        retval = (RTL_R16(tp, PHYstatus) & LinkStatus) ? 1 : 0;

        return retval;
}

static int
rtl8125_wait_phy_reset_complete(struct rtl8125_private *tp)
{
        int i, val;

        for (i = 0; i < 2500; i++) {
                val = rtl8125_mdio_read(tp, MII_BMCR) & BMCR_RESET;
                if (!val)
                        return 0;

                mdelay(1);
        }

        return -1;
}

static void
rtl8125_xmii_reset_enable(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (rtl8125_is_in_phy_disable_mode(dev)) {
                return;
        }

        rtl8125_mdio_write(tp, 0x1f, 0x0000);
        rtl8125_mdio_write(tp, MII_ADVERTISE, rtl8125_mdio_read(tp, MII_ADVERTISE) &
                           ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                             ADVERTISE_100HALF | ADVERTISE_100FULL));
        rtl8125_mdio_write(tp, MII_CTRL1000, rtl8125_mdio_read(tp, MII_CTRL1000) &
                           ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL));
        mdio_direct_write_phy_ocp(tp, 0xA5D4, mdio_direct_read_phy_ocp(tp, 0xA5D4) & ~(RTK_ADVERTISE_2500FULL));
        rtl8125_mdio_write(tp, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);

        if (rtl8125_wait_phy_reset_complete(tp) == 0) return;

        if (netif_msg_link(tp))
                printk(KERN_ERR "%s: PHY reset failed.\n", dev->name);
}

void
rtl8125_init_ring_indexes(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->HwSuppNumTxQueues; i++) {
                struct rtl8125_tx_ring *ring = &tp->tx_ring[i];
                ring->dirty_tx = ring->cur_tx = 0;
                ring->NextHwDesCloPtr = 0;
                ring->BeginHwDesCloPtr = 0;
                ring->index = i;
                ring->priv = tp;
        }

        for (i = 0; i < tp->HwSuppNumRxQueues; i++) {
                struct rtl8125_rx_ring *ring = &tp->rx_ring[i];
                ring->dirty_rx = ring->cur_rx = 0;
                ring->index = i;
                ring->priv = tp;
        }

#ifdef ENABLE_LIB_SUPPORT
        for (i = 0; i < tp->HwSuppNumTxQueues; i++) {
                struct rtl8125_ring *ring = &tp->lib_tx_ring[i];
                ring->direction = RTL8125_CH_DIR_TX;
                ring->queue_num = i;
                ring->private = tp;
        }

        for (i = 0; i < tp->HwSuppNumRxQueues; i++) {
                struct rtl8125_ring *ring = &tp->lib_rx_ring[i];
                ring->direction = RTL8125_CH_DIR_RX;
                ring->queue_num = i;
                ring->private = tp;
        }
#endif
}

static void
rtl8125_issue_offset_99_event(struct rtl8125_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xE09A,  rtl8125_mac_ocp_read(tp, 0xE09A) | BIT_0);
                break;
        }
}

#ifdef ENABLE_DASH_SUPPORT
static void
NICChkTypeEnableDashInterrupt(struct rtl8125_private *tp)
{
        if (tp->DASH) {
                //
                // even disconnected, enable 3 dash interrupt mask bits for in-band/out-band communication
                //
                if (HW_DASH_SUPPORT_TYPE_2(tp) || HW_DASH_SUPPORT_TYPE_3(tp)) {
                        rtl8125_enable_dash2_interrupt(tp);
                        RTL_W16(tp, IntrMask, (ISRIMR_DASH_INTR_EN | ISRIMR_DASH_INTR_CMAC_RESET));
                }
        }
}
#endif

static int rtl8125_enable_eee_plus(struct rtl8125_private *tp)
{
        int ret;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xE080, rtl8125_mac_ocp_read(tp, 0xE080)|BIT_1);
                break;

        default:
//      dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support EEEPlus\n");
                ret = -EOPNOTSUPP;
                break;
        }

        return ret;
}

static int rtl8125_disable_eee_plus(struct rtl8125_private *tp)
{
        int ret;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xE080, rtl8125_mac_ocp_read(tp, 0xE080)&~BIT_1);
                break;
        default:
//      dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support EEEPlus\n");
                ret = -EOPNOTSUPP;
                break;
        }

        return ret;
}

static void rtl8125_enable_double_vlan(struct rtl8125_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W16(tp, DOUBLE_VLAN_CONFIG, 0xf002);
                break;
        default:
                break;
        }
}

static void rtl8125_disable_double_vlan(struct rtl8125_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W16(tp, DOUBLE_VLAN_CONFIG, 0);
                break;
        default:
                break;
        }
}

static void
rtl8125_link_on_patch(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_hw_config(dev);

        if ((tp->mcfg == CFG_METHOD_2) &&
            netif_running(dev)) {
                if (RTL_R16(tp, PHYstatus)&FullDup)
                        RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | (BIT_24 | BIT_25)) & ~BIT_19);
                else
                        RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | BIT_25) & ~(BIT_19 | BIT_24));
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                if (RTL_R8(tp, PHYstatus) & _10bps)
                        rtl8125_enable_eee_plus(tp);
                break;
        default:
                break;
        }

        rtl8125_hw_start(dev);

        netif_carrier_on(dev);

        netif_tx_wake_all_queues(dev);

        tp->phy_reg_aner = rtl8125_mdio_read(tp, MII_EXPANSION);
        tp->phy_reg_anlpar = rtl8125_mdio_read(tp, MII_LPA);
        tp->phy_reg_gbsr = rtl8125_mdio_read(tp, MII_STAT1000);
        tp->phy_reg_status_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D6);
}

static void
rtl8125_link_down_patch(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        tp->phy_reg_aner = 0;
        tp->phy_reg_anlpar = 0;
        tp->phy_reg_gbsr = 0;
        tp->phy_reg_status_2500 = 0;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_disable_eee_plus(tp);
                break;
        default:
                break;
        }

        netif_carrier_off(dev);

        netif_tx_disable(dev);

        rtl8125_hw_reset(dev);

        rtl8125_tx_clear(tp);

        rtl8125_rx_clear(tp);

        rtl8125_init_ring(dev);

        rtl8125_enable_hw_linkchg_interrupt(tp);

        //rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH) {
                NICChkTypeEnableDashInterrupt(tp);
        }
#endif
}

static void
_rtl8125_check_link_status(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->link_ok(dev)) {
                rtl8125_link_on_patch(dev);

                if (netif_msg_ifup(tp))
                        printk(KERN_INFO PFX "%s: link up\n", dev->name);
        } else {
                if (netif_msg_ifdown(tp))
                        printk(KERN_INFO PFX "%s: link down\n", dev->name);

                rtl8125_link_down_patch(dev);
        }
}

static void
rtl8125_check_link_status(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        _rtl8125_check_link_status(dev);

        tp->resume_not_chg_speed = 0;
}

static void
rtl8125_link_option_giga(u8 *aut,
                         u32 *spd,
                         u8 *dup,
                         u32 *adv)
{
        if ((*spd != SPEED_1000) &&
            (*spd != SPEED_100) &&
            (*spd != SPEED_10))
                *spd = SPEED_1000;

        if ((*dup != DUPLEX_FULL) && (*dup != DUPLEX_HALF))
                *dup = DUPLEX_FULL;

        if ((*aut != AUTONEG_ENABLE) && (*aut != AUTONEG_DISABLE))
                *aut = AUTONEG_ENABLE;

        *adv &= (ADVERTISED_10baseT_Half |
                 ADVERTISED_10baseT_Full |
                 ADVERTISED_100baseT_Half |
                 ADVERTISED_100baseT_Full |
                 ADVERTISED_1000baseT_Half |
                 ADVERTISED_1000baseT_Full);
        if (*adv == 0)
                *adv = (ADVERTISED_10baseT_Half |
                        ADVERTISED_10baseT_Full |
                        ADVERTISED_100baseT_Half |
                        ADVERTISED_100baseT_Full |
                        ADVERTISED_1000baseT_Half |
                        ADVERTISED_1000baseT_Full);
}

static void
rtl8125_link_option(u8 *aut,
                    u32 *spd,
                    u8 *dup,
                    u32 *adv)
{
        if ((*spd != SPEED_2500) && (*spd != SPEED_1000) &&
            (*spd != SPEED_100) && (*spd != SPEED_10))
                *spd = SPEED_2500;

        if ((*dup != DUPLEX_FULL) && (*dup != DUPLEX_HALF))
                *dup = DUPLEX_FULL;

        if ((*aut != AUTONEG_ENABLE) && (*aut != AUTONEG_DISABLE))
                *aut = AUTONEG_ENABLE;

        *adv &= (ADVERTISED_10baseT_Half |
                 ADVERTISED_10baseT_Full |
                 ADVERTISED_100baseT_Half |
                 ADVERTISED_100baseT_Full |
                 ADVERTISED_1000baseT_Half |
                 ADVERTISED_1000baseT_Full |
                 ADVERTISED_2500baseX_Full);
        if (*adv == 0)
                *adv = (ADVERTISED_10baseT_Half |
                        ADVERTISED_10baseT_Full |
                        ADVERTISED_100baseT_Half |
                        ADVERTISED_100baseT_Full |
                        ADVERTISED_1000baseT_Half |
                        ADVERTISED_1000baseT_Full |
                        ADVERTISED_2500baseX_Full);
}

/*
static void
rtl8125_enable_ocp_phy_power_saving(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 val;

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                val = mdio_direct_read_phy_ocp(tp, 0xC416);
                if (val != 0x0050) {
                        rtl8125_set_phy_mcu_patch_request(tp);
                        mdio_direct_write_phy_ocp(tp, 0xC416, 0x0000);
                        mdio_direct_write_phy_ocp(tp, 0xC416, 0x0050);
                        rtl8125_clear_phy_mcu_patch_request(tp);
                }
        }
}
*/

static void
rtl8125_disable_ocp_phy_power_saving(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 val;

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                val = mdio_direct_read_phy_ocp(tp, 0xC416);
                if (val != 0x0500) {
                        rtl8125_set_phy_mcu_patch_request(tp);
                        mdio_direct_write_phy_ocp(tp, 0xC416, 0x0000);
                        mdio_direct_write_phy_ocp(tp, 0xC416, 0x0500);
                        rtl8125_clear_phy_mcu_patch_request(tp);
                }
        }
}

static void
rtl8125_wait_ll_share_fifo_ready(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        for (i = 0; i < 10; i++) {
                udelay(100);
                if (RTL_R16(tp, 0xD2) & BIT_9)
                        break;
        }
}

static void
rtl8125_disable_pci_offset_99(struct rtl8125_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xE032,  rtl8125_mac_ocp_read(tp, 0xE032) & ~(BIT_0 | BIT_1));
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_csi_fun0_write_byte(tp, 0x99, 0x00);
                break;
        }
}

static void
rtl8125_enable_pci_offset_99(struct rtl8125_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_csi_fun0_write_byte(tp, 0x99, tp->org_pci_offset_99);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE032);
                csi_tmp &= ~(BIT_0 | BIT_1);
                if (tp->org_pci_offset_99 & (BIT_5 | BIT_6))
                        csi_tmp |= BIT_1;
                if (tp->org_pci_offset_99 & BIT_2)
                        csi_tmp |= BIT_0;
                rtl8125_mac_ocp_write(tp, 0xE032, csi_tmp);
                break;
        }
}

static void
rtl8125_init_pci_offset_99(struct rtl8125_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xCDD0, 0x9003);
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE034);
                csi_tmp |= (BIT_15 | BIT_14);
                rtl8125_mac_ocp_write(tp, 0xE034, csi_tmp);
                rtl8125_mac_ocp_write(tp, 0xCDD2, 0x889C);
                rtl8125_mac_ocp_write(tp, 0xCDD8, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDD4, 0x8C30);
                rtl8125_mac_ocp_write(tp, 0xCDDA, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDD6, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDDC, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDE8, 0x883E);
                rtl8125_mac_ocp_write(tp, 0xCDEA, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDEC, 0x889C);
                rtl8125_mac_ocp_write(tp, 0xCDEE, 0x9003);
                rtl8125_mac_ocp_write(tp, 0xCDF0, 0x8C09);
                rtl8125_mac_ocp_write(tp, 0xCDF2, 0x9003);
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE032);
                csi_tmp |= (BIT_14);
                rtl8125_mac_ocp_write(tp, 0xE032, csi_tmp);
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE0A2);
                csi_tmp |= (BIT_0);
                rtl8125_mac_ocp_write(tp, 0xE0A2, csi_tmp);
                break;
        }

        rtl8125_enable_pci_offset_99(tp);
}

static void
rtl8125_disable_pci_offset_180(struct rtl8125_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE092);
                csi_tmp &= 0xFF00;
                rtl8125_mac_ocp_write(tp, 0xE092, csi_tmp);
                break;
        }
}

static void
rtl8125_enable_pci_offset_180(struct rtl8125_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE094);
                csi_tmp &= 0x00FF;
                rtl8125_mac_ocp_write(tp, 0xE094, csi_tmp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xE092);
                csi_tmp &= 0xFF00;
                csi_tmp |= BIT_2;
                rtl8125_mac_ocp_write(tp, 0xE092, csi_tmp);
                break;
        }
}

static void
rtl8125_init_pci_offset_180(struct rtl8125_private *tp)
{
        rtl8125_enable_pci_offset_180(tp);
}

static void
rtl8125_set_pci_99_180_exit_driver_para(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                if (tp->org_pci_offset_99 & BIT_2)
                        rtl8125_issue_offset_99_event(tp);
                rtl8125_disable_pci_offset_99(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_disable_pci_offset_180(tp);
                break;
        }
}

static void
rtl8125_enable_cfg9346_write(struct rtl8125_private *tp)
{
        RTL_W8(tp, Cfg9346, RTL_R8(tp, Cfg9346) | Cfg9346_Unlock);
}

static void
rtl8125_disable_cfg9346_write(struct rtl8125_private *tp)
{
        RTL_W8(tp, Cfg9346, RTL_R8(tp, Cfg9346) & ~Cfg9346_Unlock);
}

static void
rtl8125_enable_exit_l1_mask(struct rtl8125_private *tp)
{
        //(1)ERI(0xD4)(OCP 0xC0AC).bit[7:12]=6'b111111, L1 Mask
        SetMcuAccessRegBit(tp, 0xC0AC, (BIT_7 | BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12));
}

static void
rtl8125_disable_exit_l1_mask(struct rtl8125_private *tp)
{
        //(1)ERI(0xD4)(OCP 0xC0AC).bit[7:12]=6'b000000, L1 Mask
        ClearMcuAccessRegBit(tp, 0xC0AC, (BIT_7 | BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12));
}

static void
rtl8125_enable_extend_tally_couter(struct rtl8125_private *tp)
{
        switch (tp->HwSuppExtendTallyCounterVer) {
        case 1:
                SetMcuAccessRegBit(tp, 0xEA84, (BIT_1 | BIT_0));
                break;
        }
}

static void
rtl8125_disable_extend_tally_couter(struct rtl8125_private *tp)
{
        switch (tp->HwSuppExtendTallyCounterVer) {
        case 1:
                ClearMcuAccessRegBit(tp, 0xEA84, (BIT_1 | BIT_0));
                break;
        }
}

static void
rtl8125_hw_d3_para(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        RTL_W16(tp, RxMaxSize, RX_BUF_SIZE);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                rtl8125_enable_cfg9346_write(tp);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                rtl8125_disable_cfg9346_write(tp);
                break;
        }

        rtl8125_disable_exit_l1_mask(tp);

#ifdef ENABLE_REALWOW_SUPPORT
        rtl8125_set_realwow_d3_para(dev);
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xEA18, 0x0064);
                break;
        }

        rtl8125_set_pci_99_180_exit_driver_para(dev);

        /*disable ocp phy power saving*/
        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8)
                rtl8125_disable_ocp_phy_power_saving(dev);

        rtl8125_disable_rxdvgate(dev);

        rtl8125_disable_extend_tally_couter(tp);
}

static void
rtl8125_enable_magic_packet(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V3:
                rtl8125_mac_ocp_write(tp, 0xC0B6, rtl8125_mac_ocp_read(tp, 0xC0B6) | BIT_0);
                break;
        }
}
static void
rtl8125_disable_magic_packet(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V3:
                rtl8125_mac_ocp_write(tp, 0xC0B6, rtl8125_mac_ocp_read(tp, 0xC0B6) & ~BIT_0);
                break;
        }
}

static void
rtl8125_enable_linkchg_wakeup(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppLinkChgWakeUpVer) {
        case 3:
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) | LinkUp);
                ClearAndSetMcuAccessRegBit(tp, 0xE0C6,  (BIT_5 | BIT_3 | BIT_2),  (BIT_4 | BIT_1 | BIT_0));
                break;
        }
}

static void
rtl8125_disable_linkchg_wakeup(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppLinkChgWakeUpVer) {
        case 3:
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~LinkUp);
                ClearMcuAccessRegBit(tp, 0xE0C6,  (BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0));
                break;
        }
}

#define WAKE_ANY (WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST)

static u32
rtl8125_get_hw_wol(struct rtl8125_private *tp)
{
        u8 options;
        u32 csi_tmp;
        u32 wol_opts = 0;

        if (disable_pm_support)
                goto out;

        options = RTL_R8(tp, Config1);
        if (!(options & PMEnable))
                goto out;

        options = RTL_R8(tp, Config3);
        if (options & LinkUp)
                wol_opts |= WAKE_PHY;

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V3:
                csi_tmp = rtl8125_mac_ocp_read(tp, 0xC0B6);
                if (csi_tmp & BIT_0)
                        wol_opts |= WAKE_MAGIC;
                break;
        }

        options = RTL_R8(tp, Config5);
        if (options & UWF)
                wol_opts |= WAKE_UCAST;
        if (options & BWF)
                wol_opts |= WAKE_BCAST;
        if (options & MWF)
                wol_opts |= WAKE_MCAST;

out:
        return wol_opts;
}

static void
rtl8125_enable_d0_speedup(struct rtl8125_private *tp)
{
        if (FALSE == HW_SUPPORT_D0_SPEED_UP(tp)) return;
        if (tp->D0SpeedUpSpeed == D0_SPEED_UP_SPEED_DISABLE) return;

        if (tp->HwSuppD0SpeedUpVer == 1) {
                u16 mac_ocp_data;

                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_3);

                //speed up speed
                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE10A);
                mac_ocp_data &= ~(BIT_10 | BIT_9 | BIT_8 | BIT_7);
                if (tp->D0SpeedUpSpeed == D0_SPEED_UP_SPEED_2500) {
                        mac_ocp_data |= BIT_7;
                }
                rtl8125_mac_ocp_write(tp, 0xE10A, mac_ocp_data);

                //speed up flowcontrol
                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE860);
                mac_ocp_data |= (BIT_15 | BIT_14);
                rtl8125_mac_ocp_write(tp, 0xE860, mac_ocp_data);
        }
}

static void
rtl8125_disable_d0_speedup(struct rtl8125_private *tp)
{
        if (FALSE == HW_SUPPORT_D0_SPEED_UP(tp)) return;

        if (tp->HwSuppD0SpeedUpVer == 1)
                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) & ~BIT_7);
}

static void
rtl8125_set_hw_wol(struct net_device *dev, u32 wolopts)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i,tmp;
        static struct {
                u32 opt;
                u16 reg;
                u8  mask;
        } cfg[] = {
                { WAKE_PHY,   Config3, LinkUp },
                { WAKE_UCAST, Config5, UWF },
                { WAKE_BCAST, Config5, BWF },
                { WAKE_MCAST, Config5, MWF },
                { WAKE_ANY,   Config5, LanWake },
                { WAKE_MAGIC, Config3, MagicPacket },
        };

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V3:
        default:
                tmp = ARRAY_SIZE(cfg) - 1;

                if (wolopts & WAKE_MAGIC)
                        rtl8125_enable_magic_packet(dev);
                else
                        rtl8125_disable_magic_packet(dev);
                break;
        }

        rtl8125_enable_cfg9346_write(tp);

        for (i = 0; i < tmp; i++) {
                u8 options = RTL_R8(tp, cfg[i].reg) & ~cfg[i].mask;
                if (wolopts & cfg[i].opt)
                        options |= cfg[i].mask;
                RTL_W8(tp, cfg[i].reg, options);
        }

        switch (tp->HwSuppLinkChgWakeUpVer) {
        case 3:
                if (wolopts & WAKE_PHY)
                        rtl8125_enable_linkchg_wakeup(dev);
                else
                        rtl8125_disable_linkchg_wakeup(dev);
                break;
        }

        rtl8125_disable_cfg9346_write(tp);
}

static void
rtl8125_phy_restart_nway(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (rtl8125_is_in_phy_disable_mode(dev)) return;

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        rtl8125_mdio_write(tp, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);
}

static void
rtl8125_phy_setup_force_mode(struct net_device *dev, u32 speed, u8 duplex)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 bmcr_true_force = 0;

        if (rtl8125_is_in_phy_disable_mode(dev)) return;

        if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
                bmcr_true_force = BMCR_SPEED10;
        } else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
                bmcr_true_force = BMCR_SPEED10 | BMCR_FULLDPLX;
        } else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
                bmcr_true_force = BMCR_SPEED100;
        } else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
                bmcr_true_force = BMCR_SPEED100 | BMCR_FULLDPLX;
        } else {
                netif_err(tp, drv, dev, "Failed to set phy force mode!\n");
                return;
        }

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        rtl8125_mdio_write(tp, MII_BMCR, bmcr_true_force);
}

static void
rtl8125_set_pci_pme(struct rtl8125_private *tp, int set)
{
        struct pci_dev *pdev = tp->pci_dev;
        u16 pmc;

        if (!pdev->pm_cap)
                return;

        pci_read_config_word(pdev, pdev->pm_cap + PCI_PM_CTRL, &pmc);
        pmc |= PCI_PM_CTRL_PME_STATUS;
        if (set)
                pmc |= PCI_PM_CTRL_PME_ENABLE;
        else
                pmc &= ~PCI_PM_CTRL_PME_ENABLE;
        pci_write_config_word(pdev, pdev->pm_cap + PCI_PM_CTRL, pmc);
}

static void
rtl8125_set_wol_link_speed(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int auto_nego;
        int giga_ctrl;
        int ctrl_2500;
        u32 adv;
        u16 anlpar;
        u16 gbsr;
        u16 status_2500;
        u16 aner;

        if (tp->autoneg != AUTONEG_ENABLE)
                goto exit;

        rtl8125_mdio_write(tp, 0x1F, 0x0000);

        auto_nego = rtl8125_mdio_read(tp, MII_ADVERTISE);
        auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL
                       | ADVERTISE_100HALF | ADVERTISE_100FULL);

        giga_ctrl = rtl8125_mdio_read(tp, MII_CTRL1000);
        giga_ctrl &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);

        ctrl_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D4);
        ctrl_2500 &= ~(RTK_ADVERTISE_2500FULL);

        aner = tp->phy_reg_aner;
        anlpar = tp->phy_reg_anlpar;
        gbsr = tp->phy_reg_gbsr;
        status_2500 = tp->phy_reg_status_2500;
        if (tp->link_ok(dev)) {
                aner = rtl8125_mdio_read(tp, MII_EXPANSION);
                anlpar = rtl8125_mdio_read(tp, MII_LPA);
                gbsr = rtl8125_mdio_read(tp, MII_STAT1000);
                status_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D6);
        }

        adv = tp->advertising;
        if ((aner | anlpar | gbsr | status_2500) == 0) {
                int auto_nego_tmp = 0;
                if (adv & ADVERTISED_10baseT_Half)
                        auto_nego_tmp |= ADVERTISE_10HALF;
                if (adv & ADVERTISED_10baseT_Full)
                        auto_nego_tmp |= ADVERTISE_10FULL;
                if (adv & ADVERTISED_100baseT_Half)
                        auto_nego_tmp |= ADVERTISE_100HALF;
                if (adv & ADVERTISED_100baseT_Full)
                        auto_nego_tmp |= ADVERTISE_100FULL;

                if (auto_nego_tmp == 0)
                        goto exit;

                auto_nego |= auto_nego_tmp;
                goto skip_check_lpa;
        }
        if (!(aner & EXPANSION_NWAY)) goto exit;

        if ((adv & ADVERTISED_10baseT_Half) && (anlpar & LPA_10HALF))
                auto_nego |= ADVERTISE_10HALF;
        else if ((adv & ADVERTISED_10baseT_Full) && (anlpar & LPA_10FULL))
                auto_nego |= ADVERTISE_10FULL;
        else if ((adv & ADVERTISED_100baseT_Half) && (anlpar & LPA_100HALF))
                auto_nego |= ADVERTISE_100HALF;
        else if ((adv & ADVERTISED_100baseT_Full) && (anlpar & LPA_100FULL))
                auto_nego |= ADVERTISE_100FULL;
        else if (adv & ADVERTISED_1000baseT_Half && (gbsr & LPA_1000HALF))
                giga_ctrl |= ADVERTISE_1000HALF;
        else if (adv & ADVERTISED_1000baseT_Full && (gbsr & LPA_1000FULL))
                giga_ctrl |= ADVERTISE_1000FULL;
        else if (adv & ADVERTISED_2500baseX_Full && (status_2500 & RTK_LPA_ADVERTISE_2500FULL))
                ctrl_2500 |= RTK_ADVERTISE_2500FULL;
        else
                goto exit;

skip_check_lpa:
        if (tp->DASH)
                auto_nego |= (ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10HALF | ADVERTISE_10FULL);

#ifdef CONFIG_DOWN_SPEED_100
        auto_nego |= (ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10HALF | ADVERTISE_10FULL);
#endif

        rtl8125_mdio_write(tp, MII_ADVERTISE, auto_nego);
        rtl8125_mdio_write(tp, MII_CTRL1000, giga_ctrl);
        mdio_direct_write_phy_ocp(tp, 0xA5D4, ctrl_2500);

        rtl8125_phy_restart_nway(dev);

exit:
        return;
}

static bool
rtl8125_keep_wol_link_speed(struct net_device *dev, u8 from_suspend)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if ((from_suspend && !tp->link_ok(dev)) ||
            (!from_suspend && tp->resume_not_chg_speed))
                return 1;

        return 0;
}
static void
rtl8125_powerdown_pll(struct net_device *dev, u8 from_suspend)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        tp->check_keep_link_speed = 0;
        if (tp->wol_enabled == WOL_ENABLED || tp->DASH || tp->EnableKCPOffload) {
                rtl8125_set_hw_wol(dev, tp->wol_opts);

                if (tp->mcfg == CFG_METHOD_2 ||
                    tp->mcfg == CFG_METHOD_3 ||
                    tp->mcfg == CFG_METHOD_4 ||
                    tp->mcfg == CFG_METHOD_5 ||
                    tp->mcfg == CFG_METHOD_6 ||
                    tp->mcfg == CFG_METHOD_7 ||
                    tp->mcfg == CFG_METHOD_8) {
                        rtl8125_enable_cfg9346_write(tp);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | PMSTS_En);
                        rtl8125_disable_cfg9346_write(tp);
                }

                /* Enable the PME and clear the status */
                rtl8125_set_pci_pme(tp, 1);

                if (rtl8125_keep_wol_link_speed(dev, from_suspend)) {
                        if (tp->wol_opts & WAKE_PHY)
                                tp->check_keep_link_speed = 1;
                } else {
                        if (HW_SUPPORT_D0_SPEED_UP(tp)) {
                                rtl8125_enable_d0_speedup(tp);
                                tp->check_keep_link_speed = 1;
                        }

                        rtl8125_set_wol_link_speed(dev);
                }

                RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) | AcceptBroadcast | AcceptMulticast | AcceptMyPhys);

                return;
        }

        if (tp->DASH)
                return;

        rtl8125_phy_power_down(dev);

        if (!tp->HwIcVerUnknown) {
                switch (tp->mcfg) {
                case CFG_METHOD_2:
                case CFG_METHOD_3:
                case CFG_METHOD_4:
                case CFG_METHOD_5:
                case CFG_METHOD_6:
                case CFG_METHOD_7:
                case CFG_METHOD_8:
                        RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) & ~BIT_7);
                        break;
                }
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_6);
                break;
        }
}

static void rtl8125_powerup_pll(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) | BIT_7 | BIT_6);
                break;
        }

        if (tp->resume_not_chg_speed) return;

        rtl8125_phy_power_up(dev);
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static void
rtl8125_get_wol(struct net_device *dev,
                struct ethtool_wolinfo *wol)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u8 options;

        wol->wolopts = 0;

        if (tp->mcfg == CFG_METHOD_DEFAULT || disable_pm_support) {
                wol->supported = 0;
                return;
        } else {
                wol->supported = WAKE_ANY;
        }

        options = RTL_R8(tp, Config1);
        if (!(options & PMEnable))
                return;

        wol->wolopts = tp->wol_opts;
}

static int
rtl8125_set_wol(struct net_device *dev,
                struct ethtool_wolinfo *wol)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT || disable_pm_support)
                return -EOPNOTSUPP;

        tp->wol_opts = wol->wolopts;

        tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

        device_set_wakeup_enable(tp_to_dev(tp), wol->wolopts);

        return 0;
}

static void
rtl8125_get_drvinfo(struct net_device *dev,
                    struct ethtool_drvinfo *info)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct rtl8125_fw *rtl_fw = tp->rtl_fw;

        strcpy(info->driver, MODULENAME);
        strcpy(info->version, RTL8125_VERSION);
        strcpy(info->bus_info, pci_name(tp->pci_dev));
        info->regdump_len = R8125_REGS_DUMP_SIZE;
        info->eedump_len = tp->eeprom_len;
        BUILD_BUG_ON(sizeof(info->fw_version) < sizeof(rtl_fw->version));
        if (rtl_fw)
                strlcpy(info->fw_version, rtl_fw->version,
                        sizeof(info->fw_version));
}

static int
rtl8125_get_regs_len(struct net_device *dev)
{
        return R8125_REGS_DUMP_SIZE;
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

static void
rtl8125_set_d0_speedup_speed(struct rtl8125_private *tp)
{
        if (FALSE == HW_SUPPORT_D0_SPEED_UP(tp)) return;

        tp->D0SpeedUpSpeed = D0_SPEED_UP_SPEED_DISABLE;
        if (tp->autoneg == AUTONEG_ENABLE) {
                if (tp->speed == SPEED_2500)
                        tp->D0SpeedUpSpeed = D0_SPEED_UP_SPEED_2500;
                else if(tp->speed == SPEED_1000)
                        tp->D0SpeedUpSpeed = D0_SPEED_UP_SPEED_1000;
        }
}

static int
rtl8125_set_speed_xmii(struct net_device *dev,
                       u8 autoneg,
                       u32 speed,
                       u8 duplex,
                       u64 adv)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int auto_nego = 0;
        int giga_ctrl = 0;
        int ctrl_2500 = 0;
        int rc = -EINVAL;

        //Disable Giga Lite
        ClearEthPhyOcpBit(tp, 0xA428, BIT_9);
        ClearEthPhyOcpBit(tp, 0xA5EA, BIT_0);

        if (speed != SPEED_5000 &&
            speed != SPEED_2500 &&
            (speed != SPEED_1000) &&
            (speed != SPEED_100) &&
            (speed != SPEED_10)) {
                speed = SPEED_2500;
                duplex = DUPLEX_FULL;
        }

        giga_ctrl = rtl8125_mdio_read(tp, MII_CTRL1000);
        giga_ctrl &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);
        ctrl_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D4);
        ctrl_2500 &= ~(RTK_ADVERTISE_2500FULL | RTK_ADVERTISE_5000FULL);

        if (autoneg == AUTONEG_ENABLE) {
                /*n-way force*/
                auto_nego = rtl8125_mdio_read(tp, MII_ADVERTISE);
                auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                               ADVERTISE_100HALF | ADVERTISE_100FULL |
                               ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

                if (adv & ADVERTISED_10baseT_Half)
                        auto_nego |= ADVERTISE_10HALF;
                if (adv & ADVERTISED_10baseT_Full)
                        auto_nego |= ADVERTISE_10FULL;
                if (adv & ADVERTISED_100baseT_Half)
                        auto_nego |= ADVERTISE_100HALF;
                if (adv & ADVERTISED_100baseT_Full)
                        auto_nego |= ADVERTISE_100FULL;
                if (adv & ADVERTISED_1000baseT_Half)
                        giga_ctrl |= ADVERTISE_1000HALF;
                if (adv & ADVERTISED_1000baseT_Full)
                        giga_ctrl |= ADVERTISE_1000FULL;
                if (adv & ADVERTISED_2500baseX_Full)
                        ctrl_2500 |= RTK_ADVERTISE_2500FULL;
                if (HW_SUPP_PHY_LINK_SPEED_5000M(tp)) {
                        if (adv & RTK_ADVERTISED_5000baseX_Full)
                                ctrl_2500 |= RTK_ADVERTISE_5000FULL;
                }

                //flow control
                if (dev->mtu <= ETH_DATA_LEN && tp->fcpause == rtl8125_fc_full)
                        auto_nego |= ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM;

                tp->phy_auto_nego_reg = auto_nego;
                tp->phy_1000_ctrl_reg = giga_ctrl;

                tp->phy_2500_ctrl_reg = ctrl_2500;

                rtl8125_mdio_write(tp, 0x1f, 0x0000);
                rtl8125_mdio_write(tp, MII_ADVERTISE, auto_nego);
                rtl8125_mdio_write(tp, MII_CTRL1000, giga_ctrl);
                mdio_direct_write_phy_ocp(tp, 0xA5D4, ctrl_2500);
                rtl8125_phy_restart_nway(dev);
                mdelay(20);
        } else {
                /*true force*/
                if (speed == SPEED_10 || speed == SPEED_100)
                        rtl8125_phy_setup_force_mode(dev, speed, duplex);
                else
                        goto out;
        }

        tp->autoneg = autoneg;
        tp->speed = speed;
        tp->duplex = duplex;
        tp->advertising = adv;

        rtl8125_set_d0_speedup_speed(tp);

        rc = 0;
out:
        return rc;
}

static int
rtl8125_set_speed(struct net_device *dev,
                  u8 autoneg,
                  u32 speed,
                  u8 duplex,
                  u64 adv)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret;

        if (tp->resume_not_chg_speed) return 0;

        ret = tp->set_speed(dev, autoneg, speed, duplex, adv);

        return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int
rtl8125_set_settings(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                     struct ethtool_cmd *cmd
#else
                     const struct ethtool_link_ksettings *cmd
#endif
                    )
{
        int ret;
        u8 autoneg;
        u32 speed;
        u8 duplex;
        u64 supported = 0, advertising = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        autoneg = cmd->autoneg;
        speed = cmd->speed;
        duplex = cmd->duplex;
        supported = cmd->supported;
        advertising = cmd->advertising;
#else
        struct rtl8125_private *tp = netdev_priv(dev);
        const struct ethtool_link_settings *base = &cmd->base;
        autoneg = base->autoneg;
        speed = base->speed;
        duplex = base->duplex;
        ethtool_convert_link_mode_to_legacy_u32((u32*)&supported,
                                                cmd->link_modes.supported);
        ethtool_convert_link_mode_to_legacy_u32((u32*)&advertising,
                                                cmd->link_modes.advertising);
        if (test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                     cmd->link_modes.supported))
                supported |= ADVERTISED_2500baseX_Full;
        if (test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                     cmd->link_modes.advertising))
                advertising |= ADVERTISED_2500baseX_Full;
        if (HW_SUPP_PHY_LINK_SPEED_5000M(tp)) {
                if (test_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                             cmd->link_modes.supported))
                        supported |= RTK_ADVERTISED_5000baseX_Full;
                if (test_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                             cmd->link_modes.advertising))
                        advertising |= RTK_ADVERTISED_5000baseX_Full;
        }
#endif
        if (advertising & ~supported)
                return -EINVAL;

        ret = rtl8125_set_speed(dev, autoneg, speed, duplex, advertising);

        return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
static u32
rtl8125_get_tx_csum(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 ret;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        ret = ((dev->features & NETIF_F_IP_CSUM) != 0);
#else
        ret = ((dev->features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) != 0);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

        return ret;
}

static u32
rtl8125_get_rx_csum(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 ret;

        ret = tp->cp_cmd & RxChkSum;

        return ret;
}

static int
rtl8125_set_tx_csum(struct net_device *dev,
                    u32 data)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        if (data)
                dev->features |= NETIF_F_IP_CSUM;
        else
                dev->features &= ~NETIF_F_IP_CSUM;
#else
        if (data)
                dev->features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
        else
                dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

        return 0;
}

static int
rtl8125_set_rx_csum(struct net_device *dev,
                    u32 data)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

        if (data)
                tp->cp_cmd |= RxChkSum;
        else
                tp->cp_cmd &= ~RxChkSum;

        RTL_W16(tp, CPlusCmd, tp->cp_cmd);

        return 0;
}
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

static u32
rtl8125_rx_desc_opts1(struct rtl8125_private *tp,
                      struct RxDesc *desc)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                return ((struct RxDescV3 *)desc)->RxDescNormalDDWord4.opts1;
        else
                return desc->opts1;
}

static u32
rtl8125_rx_desc_opts2(struct rtl8125_private *tp,
                      struct RxDesc *desc)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                return ((struct RxDescV3 *)desc)->RxDescNormalDDWord4.opts2;
        else
                return desc->opts2;
}

#ifdef CONFIG_R8125_VLAN

static void
rtl8125_clear_rx_desc_opts2(struct rtl8125_private *tp,
                            struct RxDesc *desc)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                ((struct RxDescV3 *)desc)->RxDescNormalDDWord4.opts2 = 0;
        else
                desc->opts2 = 0;
}

static inline u32
rtl8125_tx_vlan_tag(struct rtl8125_private *tp,
                    struct sk_buff *skb)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        return (tp->vlgrp && vlan_tx_tag_present(skb)) ?
               TxVlanTag | swab16(vlan_tx_tag_get(skb)) : 0x00;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
        return (vlan_tx_tag_present(skb)) ?
               TxVlanTag | swab16(vlan_tx_tag_get(skb)) : 0x00;
#else
        return (skb_vlan_tag_present(skb)) ?
               TxVlanTag | swab16(skb_vlan_tag_get(skb)) : 0x00;
#endif

        return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static void
rtl8125_vlan_rx_register(struct net_device *dev,
                         struct vlan_group *grp)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        tp->vlgrp = grp;
        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                if (tp->vlgrp) {
                        tp->rtl8125_rx_config |= (EnableInnerVlan | EnableOuterVlan);
                        RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) | (EnableInnerVlan | EnableOuterVlan))
                } else {
                        tp->rtl8125_rx_config &= ~(EnableInnerVlan | EnableOuterVlan);
                        RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) & ~(EnableInnerVlan | EnableOuterVlan))
                }
        }
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
static void
rtl8125_vlan_rx_kill_vid(struct net_device *dev,
                         unsigned short vid)
{
        struct rtl8125_private *tp = netdev_priv(dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
        if (tp->vlgrp)
                tp->vlgrp->vlan_devices[vid] = NULL;
#else
        vlan_group_set_device(tp->vlgrp, vid, NULL);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
}
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)

static int
rtl8125_rx_vlan_skb(struct rtl8125_private *tp,
                    struct RxDesc *desc,
                    struct sk_buff *skb)
{
        u32 opts2 = le32_to_cpu(rtl8125_rx_desc_opts2(tp, desc));
        int ret = -1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        if (tp->vlgrp && (opts2 & RxVlanTag)) {
                rtl8125_rx_hwaccel_skb(skb, tp->vlgrp,
                                       swab16(opts2 & 0xffff));
                ret = 0;
        }
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
        if (opts2 & RxVlanTag)
                __vlan_hwaccel_put_tag(skb, swab16(opts2 & 0xffff));
#else
        if (opts2 & RxVlanTag)
                __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), swab16(opts2 & 0xffff));
#endif

        rtl8125_clear_rx_desc_opts2(tp, desc);
        return ret;
}

#else /* !CONFIG_R8125_VLAN */

static inline u32
rtl8125_tx_vlan_tag(struct rtl8125_private *tp,
                    struct sk_buff *skb)
{
        return 0;
}

static int
rtl8125_rx_vlan_skb(struct rtl8125_private *tp,
                    struct RxDesc *desc,
                    struct sk_buff *skb)
{
        return -1;
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)

static netdev_features_t rtl8125_fix_features(struct net_device *dev,
                netdev_features_t features)
{
        if (dev->mtu > MSS_MAX)
                features &= ~NETIF_F_ALL_TSO;
        if (dev->mtu > ETH_DATA_LEN) {
                features &= ~NETIF_F_ALL_TSO;
                features &= ~NETIF_F_ALL_CSUM;
        }
#ifndef CONFIG_R8125_VLAN
        features &= ~NETIF_F_ALL_CSUM;
#endif

        return features;
}

static int rtl8125_hw_set_features(struct net_device *dev,
                                   netdev_features_t features)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 rx_config;

        rx_config = RTL_R32(tp, RxConfig);
        if (features & NETIF_F_RXALL) {
                tp->rtl8125_rx_config |= (AcceptErr | AcceptRunt);
                rx_config |= (AcceptErr | AcceptRunt);
        } else {
                tp->rtl8125_rx_config &= ~(AcceptErr | AcceptRunt);
                rx_config &= ~(AcceptErr | AcceptRunt);
        }

        if (features & NETIF_F_HW_VLAN_RX) {
                tp->rtl8125_rx_config |= (EnableInnerVlan | EnableOuterVlan);
                rx_config |= (EnableInnerVlan | EnableOuterVlan);
        } else {
                tp->rtl8125_rx_config &= ~(EnableInnerVlan | EnableOuterVlan);
                rx_config &= ~(EnableInnerVlan | EnableOuterVlan);
        }

        RTL_W32(tp, RxConfig, rx_config);

        if (features & NETIF_F_RXCSUM)
                tp->cp_cmd |= RxChkSum;
        else
                tp->cp_cmd &= ~RxChkSum;

        RTL_W16(tp, CPlusCmd, tp->cp_cmd);
        RTL_R16(tp, CPlusCmd);

        return 0;
}

static int rtl8125_set_features(struct net_device *dev,
                                netdev_features_t features)
{
        features &= NETIF_F_RXALL | NETIF_F_RXCSUM | NETIF_F_HW_VLAN_RX;

        rtl8125_hw_set_features(dev, features);

        return 0;
}

#endif

static void rtl8125_gset_xmii(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                              struct ethtool_cmd *cmd
#else
                              struct ethtool_link_ksettings *cmd
#endif
                             )
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 aner = tp->phy_reg_aner;
        u16 anlpar = tp->phy_reg_anlpar;
        u16 gbsr = tp->phy_reg_gbsr;
        u16 status_2500 = tp->phy_reg_status_2500;
        u32 lpa_adv = 0;
        u16 status;
        u8 autoneg, duplex;
        u32 speed = 0;
        u16 bmcr;
        u64 supported, advertising;
        u8 report_lpa = 0;

        supported = SUPPORTED_10baseT_Half |
                    SUPPORTED_10baseT_Full |
                    SUPPORTED_100baseT_Half |
                    SUPPORTED_100baseT_Full |
                    SUPPORTED_1000baseT_Full |
                    SUPPORTED_2500baseX_Full |
                    SUPPORTED_Autoneg |
                    SUPPORTED_TP |
                    SUPPORTED_Pause	|
                    SUPPORTED_Asym_Pause;

        if (!HW_SUPP_PHY_LINK_SPEED_2500M(tp))
                supported &= ~SUPPORTED_2500baseX_Full;

        advertising = ADVERTISED_TP;

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        bmcr = rtl8125_mdio_read(tp, MII_BMCR);

        if (bmcr & BMCR_ANENABLE) {
                advertising |= ADVERTISED_Autoneg;
                autoneg = AUTONEG_ENABLE;

                if (tp->phy_auto_nego_reg & ADVERTISE_10HALF)
                        advertising |= ADVERTISED_10baseT_Half;
                if (tp->phy_auto_nego_reg & ADVERTISE_10FULL)
                        advertising |= ADVERTISED_10baseT_Full;
                if (tp->phy_auto_nego_reg & ADVERTISE_100HALF)
                        advertising |= ADVERTISED_100baseT_Half;
                if (tp->phy_auto_nego_reg & ADVERTISE_100FULL)
                        advertising |= ADVERTISED_100baseT_Full;
                if (tp->phy_1000_ctrl_reg & ADVERTISE_1000FULL)
                        advertising |= ADVERTISED_1000baseT_Full;
        } else {
                autoneg = AUTONEG_DISABLE;
        }

        status = RTL_R16(tp, PHYstatus);
        if (netif_running(dev) && (status & LinkStatus))
                report_lpa = 1;

        if (report_lpa) {
                /*link on*/
                speed = rtl8125_convert_link_speed(status);

                if (status & TxFlowCtrl)
                        advertising |= ADVERTISED_Asym_Pause;

                if (status & RxFlowCtrl)
                        advertising |= ADVERTISED_Pause;

                duplex = ((status & (_1000bpsF | _2500bpsF | _5000bpsF)) ||
                          (status & FullDup)) ?
                         DUPLEX_FULL : DUPLEX_HALF;

                /*link partner*/
                if (aner & EXPANSION_NWAY)
                        lpa_adv |= ADVERTISED_Autoneg;
                if (anlpar & LPA_10HALF)
                        lpa_adv |= ADVERTISED_10baseT_Half;
                if (anlpar & LPA_10FULL)
                        lpa_adv |= ADVERTISED_10baseT_Full;
                if (anlpar & LPA_100HALF)
                        lpa_adv |= ADVERTISED_100baseT_Half;
                if (anlpar & LPA_100FULL)
                        lpa_adv |= ADVERTISED_100baseT_Full;
                if (anlpar & LPA_PAUSE_CAP)
                        lpa_adv |= ADVERTISED_Pause;
                if (anlpar & LPA_PAUSE_ASYM)
                        lpa_adv |= ADVERTISED_Asym_Pause;
                if (gbsr & LPA_1000HALF)
                        lpa_adv |= ADVERTISED_1000baseT_Half;
                if (gbsr & LPA_1000FULL)
                        lpa_adv |= ADVERTISED_1000baseT_Full;
                if (status_2500 & RTK_LPA_ADVERTISE_2500FULL)
                        lpa_adv |= ADVERTISED_2500baseX_Full;
        } else {
                /*link down*/
                speed = SPEED_UNKNOWN;
                duplex = DUPLEX_UNKNOWN;
                lpa_adv = 0;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        cmd->supported = (u32)supported;
        cmd->advertising = (u32)advertising;
        cmd->autoneg = autoneg;
        cmd->speed = speed;
        cmd->duplex = duplex;
        cmd->port = PORT_TP;
        cmd->lp_advertising = lpa_adv;
#else
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported,
                                                supported);
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising,
                                                advertising);
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.lp_advertising,
                                                lpa_adv);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
        if (supported & SUPPORTED_2500baseX_Full) {
                linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
                                 cmd->link_modes.supported, 0);
                linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                                 cmd->link_modes.supported, 1);
        }
        if (advertising & ADVERTISED_2500baseX_Full) {
                linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
                                 cmd->link_modes.advertising, 0);
                linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                                 cmd->link_modes.advertising, 1);
        }
        if (HW_SUPP_PHY_LINK_SPEED_5000M(tp)) {
                linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                                 cmd->link_modes.supported, 1);
                linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                                 cmd->link_modes.advertising, tp->phy_2500_ctrl_reg & RTK_ADVERTISE_5000FULL);
        }
        if (report_lpa) {
                if (lpa_adv & ADVERTISED_2500baseX_Full) {
                        linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
                                         cmd->link_modes.lp_advertising, 0);
                        linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                                         cmd->link_modes.lp_advertising, 1);
                }
                if (status_2500 & RTK_LPA_ADVERTISE_5000FULL)
                        linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                                         cmd->link_modes.lp_advertising, 1);
                if (status_2500 & RTK_LPA_ADVERTISE_10000FULL)
                        linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
                                         cmd->link_modes.lp_advertising, 1);
        }
#endif
        cmd->base.autoneg = autoneg;
        cmd->base.speed = speed;
        cmd->base.duplex = duplex;
        cmd->base.port = PORT_TP;
#endif
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int
rtl8125_get_settings(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                     struct ethtool_cmd *cmd
#else
                     struct ethtool_link_ksettings *cmd
#endif
                    )
{
        struct rtl8125_private *tp = netdev_priv(dev);

        tp->get_settings(dev, cmd);

        return 0;
}

static void rtl8125_get_regs(struct net_device *dev, struct ethtool_regs *regs,
                             void *p)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;
        unsigned int i;
        u8 *data = p;

        if (regs->len < R8125_REGS_DUMP_SIZE)
                return /* -EINVAL */;

        memset(p, 0, regs->len);

        for (i = 0; i < R8125_MAC_REGS_SIZE; i++)
                *data++ = readb(ioaddr + i);
        data = (u8*)p + 256;

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        for (i = 0; i < R8125_PHY_REGS_SIZE/2; i++) {
                *(u16*)data = rtl8125_mdio_read(tp, i);
                data += 2;
        }
        data = (u8*)p + 256 * 2;

        for (i = 0; i < R8125_EPHY_REGS_SIZE/2; i++) {
                *(u16*)data = rtl8125_ephy_read(tp, i);
                data += 2;
        }
        data = (u8*)p + 256 * 3;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        default:
                for (i = 0; i < R8125_ERI_REGS_SIZE; i+=4) {
                        *(u32*)data = rtl8125_eri_read(tp, i , 4, ERIAR_ExGMAC);
                        data += 4;
                }
                break;
        }
}

static void rtl8125_get_pauseparam(struct net_device *dev,
                                   struct ethtool_pauseparam *pause)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        pause->autoneg = (tp->autoneg ? AUTONEG_ENABLE : AUTONEG_DISABLE);
        if (tp->fcpause == rtl8125_fc_rx_pause)
                pause->rx_pause = 1;
        else if (tp->fcpause == rtl8125_fc_tx_pause)
                pause->tx_pause = 1;
        else if (tp->fcpause == rtl8125_fc_full) {
                pause->rx_pause = 1;
                pause->tx_pause = 1;
        }
}

static int rtl8125_set_pauseparam(struct net_device *dev,
                                  struct ethtool_pauseparam *pause)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        enum rtl8125_fc_mode newfc;

        if (pause->tx_pause || pause->rx_pause)
                newfc = rtl8125_fc_full;
        else
                newfc = rtl8125_fc_none;

        if (tp->fcpause != newfc) {
                tp->fcpause = newfc;

                rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
        }

        return 0;

}

static u32
rtl8125_get_msglevel(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        return tp->msg_enable;
}

static void
rtl8125_set_msglevel(struct net_device *dev,
                     u32 value)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        tp->msg_enable = value;
}

static const char rtl8125_gstrings[][ETH_GSTRING_LEN] = {
        /* legacy */
        "tx_packets",
        "rx_packets",
        "tx_errors",
        "rx_errors",
        "rx_missed",
        "align_errors",
        "tx_single_collisions",
        "tx_multi_collisions",
        "unicast",
        "broadcast",
        "multicast",
        "tx_aborted",
        "tx_underrun",

        /* extended */
        "tx_octets",
        "rx_octets",
        "rx_multicast64",
        "tx_unicast64",
        "tx_broadcast64",
        "tx_multicast64",
        "tx_pause_on",
        "tx_pause_off",
        "tx_pause_all",
        "tx_deferred",
        "tx_late_collision",
        "tx_all_collision",
        "tx_aborted32",
        "align_errors32",
        "rx_frame_too_long",
        "rx_runt",
        "rx_pause_on",
        "rx_pause_off",
        "rx_pause_all",
        "rx_unknown_opcode",
        "rx_mac_error",
        "tx_underrun32",
        "rx_mac_missed",
        "rx_tcam_dropped",
        "tdu",
        "rdu",
};
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int rtl8125_get_stats_count(struct net_device *dev)
{
        return ARRAY_SIZE(rtl8125_gstrings);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
#else
static int rtl8125_get_sset_count(struct net_device *dev, int sset)
{
        switch (sset) {
        case ETH_SS_STATS:
                return ARRAY_SIZE(rtl8125_gstrings);
        default:
                return -EOPNOTSUPP;
        }
}
#endif

static void
rtl8125_set_ring_size(struct rtl8125_private *tp, u32 rx, u32 tx)
{
        int i;

        for (i = 0; i < tp->num_rx_rings; i++)
                tp->rx_ring[i].num_rx_desc = rx;

        for (i = 0; i < tp->num_tx_rings; i++)
                tp->tx_ring[i].num_tx_desc = tx;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
static void rtl8125_get_ringparam(struct net_device *dev,
                                  struct ethtool_ringparam *ring,
                                  struct kernel_ethtool_ringparam *kernel_ring,
                                  struct netlink_ext_ack *extack)
#else
static void rtl8125_get_ringparam(struct net_device *dev,
                                  struct ethtool_ringparam *ring)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        ring->rx_max_pending = MAX_NUM_TX_DESC;
        ring->tx_max_pending = MAX_NUM_RX_DESC;
        ring->rx_pending = tp->rx_ring[0].num_rx_desc;
        ring->tx_pending = tp->tx_ring[0].num_tx_desc;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
static int rtl8125_set_ringparam(struct net_device *dev,
                                 struct ethtool_ringparam *ring,
                                 struct kernel_ethtool_ringparam *kernel_ring,
                                 struct netlink_ext_ack *extack)
#else
static int rtl8125_set_ringparam(struct net_device *dev,
                                 struct ethtool_ringparam *ring)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 new_rx_count, new_tx_count;
        int rc = 0;

        if ((ring->rx_mini_pending) || (ring->rx_jumbo_pending))
                return -EINVAL;

        new_tx_count = clamp_t(u32, ring->tx_pending,
                               MIN_NUM_TX_DESC, MAX_NUM_TX_DESC);

        new_rx_count = clamp_t(u32, ring->rx_pending,
                               MIN_NUM_RX_DESC, MAX_NUM_RX_DESC);

        if ((new_rx_count == tp->rx_ring[0].num_rx_desc) &&
            (new_tx_count == tp->tx_ring[0].num_tx_desc)) {
                /* nothing to do */
                return 0;
        }

        if (netif_running(dev)) {
                rtl8125_wait_for_quiescence(dev);
                rtl8125_close(dev);
        }

        rtl8125_set_ring_size(tp, new_rx_count, new_tx_count);

        if (netif_running(dev))
                rc = rtl8125_open(dev);

        return rc;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static void
rtl8125_get_ethtool_stats(struct net_device *dev,
                          struct ethtool_stats *stats,
                          u64 *data)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct rtl8125_counters *counters;
        dma_addr_t paddr;

        ASSERT_RTNL();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters)
                return;

        rtl8125_dump_tally_counter(tp, paddr);

        data[0] = le64_to_cpu(counters->tx_packets);
        data[1] = le64_to_cpu(counters->rx_packets);
        data[2] = le64_to_cpu(counters->tx_errors);
        data[3] = le32_to_cpu(counters->rx_errors);
        data[4] = le16_to_cpu(counters->rx_missed);
        data[5] = le16_to_cpu(counters->align_errors);
        data[6] = le32_to_cpu(counters->tx_one_collision);
        data[7] = le32_to_cpu(counters->tx_multi_collision);
        data[8] = le64_to_cpu(counters->rx_unicast);
        data[9] = le64_to_cpu(counters->rx_broadcast);
        data[10] = le32_to_cpu(counters->rx_multicast);
        data[11] = le16_to_cpu(counters->tx_aborted);
        data[12] = le16_to_cpu(counters->tx_underrun);

        data[13] = le64_to_cpu(counters->tx_octets);
        data[14] = le64_to_cpu(counters->rx_octets);
        data[15] = le64_to_cpu(counters->rx_multicast64);
        data[16] = le64_to_cpu(counters->tx_unicast64);
        data[17] = le64_to_cpu(counters->tx_broadcast64);
        data[18] = le64_to_cpu(counters->tx_multicast64);
        data[19] = le32_to_cpu(counters->tx_pause_on);
        data[20] = le32_to_cpu(counters->tx_pause_off);
        data[21] = le32_to_cpu(counters->tx_pause_all);
        data[22] = le32_to_cpu(counters->tx_deferred);
        data[23] = le32_to_cpu(counters->tx_late_collision);
        data[24] = le32_to_cpu(counters->tx_all_collision);
        data[25] = le32_to_cpu(counters->tx_aborted32);
        data[26] = le32_to_cpu(counters->align_errors32);
        data[27] = le32_to_cpu(counters->rx_frame_too_long);
        data[28] = le32_to_cpu(counters->rx_runt);
        data[29] = le32_to_cpu(counters->rx_pause_on);
        data[30] = le32_to_cpu(counters->rx_pause_off);
        data[31] = le32_to_cpu(counters->rx_pause_all);
        data[32] = le32_to_cpu(counters->rx_unknown_opcode);
        data[33] = le32_to_cpu(counters->rx_mac_error);
        data[34] = le32_to_cpu(counters->tx_underrun32);
        data[35] = le32_to_cpu(counters->rx_mac_missed);
        data[36] = le32_to_cpu(counters->rx_tcam_dropped);
        data[37] = le32_to_cpu(counters->tdu);
        data[38] = le32_to_cpu(counters->rdu);
}

static void
rtl8125_get_strings(struct net_device *dev,
                    u32 stringset,
                    u8 *data)
{
        switch (stringset) {
        case ETH_SS_STATS:
                memcpy(data, rtl8125_gstrings, sizeof(rtl8125_gstrings));
                break;
        }
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

static int rtl_get_eeprom_len(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        return tp->eeprom_len;
}

static int rtl_get_eeprom(struct net_device *dev, struct ethtool_eeprom *eeprom, u8 *buf)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i,j,ret;
        int start_w, end_w;
        int VPD_addr, VPD_data;
        u32 *eeprom_buff;
        u16 tmp;

        if (tp->eeprom_type == EEPROM_TYPE_NONE) {
                dev_printk(KERN_DEBUG, tp_to_dev(tp), "Detect none EEPROM\n");
                return -EOPNOTSUPP;
        } else if (eeprom->len == 0 || (eeprom->offset+eeprom->len) > tp->eeprom_len) {
                dev_printk(KERN_DEBUG, tp_to_dev(tp), "Invalid parameter\n");
                return -EINVAL;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        default:
                VPD_addr = 0xD2;
                VPD_data = 0xD4;
                break;
        }

        start_w = eeprom->offset >> 2;
        end_w = (eeprom->offset + eeprom->len - 1) >> 2;

        eeprom_buff = kmalloc(sizeof(u32)*(end_w - start_w + 1), GFP_KERNEL);
        if (!eeprom_buff)
                return -ENOMEM;

        rtl8125_enable_cfg9346_write(tp);
        ret = -EFAULT;
        for (i=start_w; i<=end_w; i++) {
                pci_write_config_word(tp->pci_dev, VPD_addr, (u16)i*4);
                ret = -EFAULT;
                for (j = 0; j < 10; j++) {
                        udelay(400);
                        pci_read_config_word(tp->pci_dev, VPD_addr, &tmp);
                        if (tmp&0x8000) {
                                ret = 0;
                                break;
                        }
                }

                if (ret)
                        break;

                pci_read_config_dword(tp->pci_dev, VPD_data, &eeprom_buff[i-start_w]);
        }
        rtl8125_disable_cfg9346_write(tp);

        if (!ret)
                memcpy(buf, (u8 *)eeprom_buff + (eeprom->offset & 3), eeprom->len);

        kfree(eeprom_buff);

        return ret;
}

#undef ethtool_op_get_link
#define ethtool_op_get_link _kc_ethtool_op_get_link
static u32 _kc_ethtool_op_get_link(struct net_device *dev)
{
        return netif_carrier_ok(dev) ? 1 : 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#undef ethtool_op_get_sg
#define ethtool_op_get_sg _kc_ethtool_op_get_sg
static u32 _kc_ethtool_op_get_sg(struct net_device *dev)
{
#ifdef NETIF_F_SG
        return (dev->features & NETIF_F_SG) != 0;
#else
        return 0;
#endif
}

#undef ethtool_op_set_sg
#define ethtool_op_set_sg _kc_ethtool_op_set_sg
static int _kc_ethtool_op_set_sg(struct net_device *dev, u32 data)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

#ifdef NETIF_F_SG
        if (data)
                dev->features |= NETIF_F_SG;
        else
                dev->features &= ~NETIF_F_SG;
#endif

        return 0;
}
#endif

static int rtl8125_enable_eee(struct rtl8125_private *tp)
{
        struct ethtool_eee *eee = &tp->eee;
        u16 eee_adv_t = ethtool_adv_to_mmd_eee_adv_t(eee->advertised);
        int ret;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                RTL_W16(tp, EEE_TXIDLE_TIMER_8125, eee->tx_lpi_timer);

                SetMcuAccessRegBit(tp, 0xE040, (BIT_1|BIT_0));
                SetMcuAccessRegBit(tp, 0xEB62, (BIT_2|BIT_1));

                SetEthPhyOcpBit(tp, 0xA432, BIT_4);
                ClearAndSetEthPhyOcpBit(tp,
                                        0xA5D0,
                                        MDIO_EEE_100TX | MDIO_EEE_1000T,
                                        eee_adv_t);
                ClearEthPhyOcpBit(tp, 0xA6D4, BIT_0);

                ClearEthPhyOcpBit(tp, 0xA6D8, BIT_4);
                ClearEthPhyOcpBit(tp, 0xA428, BIT_7);
                ClearEthPhyOcpBit(tp, 0xA4A2, BIT_9);
                break;
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W16(tp, EEE_TXIDLE_TIMER_8125, eee->tx_lpi_timer);

                SetMcuAccessRegBit(tp, 0xE040, (BIT_1|BIT_0));

                ClearAndSetEthPhyOcpBit(tp,
                                        0xA5D0,
                                        MDIO_EEE_100TX | MDIO_EEE_1000T,
                                        eee_adv_t);
                if (eee->advertised & SUPPORTED_2500baseX_Full)
                        SetEthPhyOcpBit(tp, 0xA6D4, BIT_0);
                else
                        ClearEthPhyOcpBit(tp, 0xA6D4, BIT_0);
                if (HW_SUPP_PHY_LINK_SPEED_5000M(tp))
                        ClearEthPhyOcpBit(tp, 0xA6D4, BIT_1);

                ClearEthPhyOcpBit(tp, 0xA6D8, BIT_4);
                ClearEthPhyOcpBit(tp, 0xA428, BIT_7);
                ClearEthPhyOcpBit(tp, 0xA4A2, BIT_9);
                break;
        default:
//      dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support EEE\n");
                ret = -EOPNOTSUPP;
                break;
        }

        /*Advanced EEE*/
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_set_phy_mcu_patch_request(tp);
                ClearMcuAccessRegBit(tp, 0xE052, BIT_0);
                ClearEthPhyOcpBit(tp, 0xA442, BIT_12 | BIT_13);
                ClearEthPhyOcpBit(tp, 0xA430, BIT_15);
                rtl8125_clear_phy_mcu_patch_request(tp);
                break;
        }

        return ret;
}

static int rtl8125_disable_eee(struct rtl8125_private *tp)
{
        int ret;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                ClearMcuAccessRegBit(tp, 0xE040, (BIT_1|BIT_0));
                ClearMcuAccessRegBit(tp, 0xEB62, (BIT_2|BIT_1));

                ClearEthPhyOcpBit(tp, 0xA432, BIT_4);
                ClearEthPhyOcpBit(tp, 0xA5D0, (BIT_2 | BIT_1));
                ClearEthPhyOcpBit(tp, 0xA6D4, BIT_0);

                ClearEthPhyOcpBit(tp, 0xA6D8, BIT_4);
                ClearEthPhyOcpBit(tp, 0xA428, BIT_7);
                ClearEthPhyOcpBit(tp, 0xA4A2, BIT_9);
                break;
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                ClearMcuAccessRegBit(tp, 0xE040, (BIT_1|BIT_0));

                ClearEthPhyOcpBit(tp, 0xA5D0, (BIT_2 | BIT_1));
                ClearEthPhyOcpBit(tp, 0xA6D4, BIT_0);
                if (HW_SUPP_PHY_LINK_SPEED_5000M(tp))
                        ClearEthPhyOcpBit(tp, 0xA6D4, BIT_1);

                ClearEthPhyOcpBit(tp, 0xA6D8, BIT_4);
                ClearEthPhyOcpBit(tp, 0xA428, BIT_7);
                ClearEthPhyOcpBit(tp, 0xA4A2, BIT_9);
                break;
        default:
//      dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support EEE\n");
                ret = -EOPNOTSUPP;
                break;
        }

        /*Advanced EEE*/
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_set_phy_mcu_patch_request(tp);
                ClearMcuAccessRegBit(tp, 0xE052, BIT_0);
                ClearEthPhyOcpBit(tp, 0xA442, BIT_12 | BIT_13);
                ClearEthPhyOcpBit(tp, 0xA430, BIT_15);
                rtl8125_clear_phy_mcu_patch_request(tp);
                break;
        }

        return ret;
}

static int rtl_nway_reset(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret, bmcr;

        if (unlikely(tp->rtk_enable_diag))
                return -EBUSY;

        /* if autoneg is off, it's an error */
        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        bmcr = rtl8125_mdio_read(tp, MII_BMCR);

        if (bmcr & BMCR_ANENABLE) {
                bmcr |= BMCR_ANRESTART;
                rtl8125_mdio_write(tp, MII_BMCR, bmcr);
                ret = 0;
        } else {
                ret = -EINVAL;
        }

        return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
static u32
rtl8125_tx_lpi_timer_to_us(struct rtl8125_private *tp , u32 tx_lpi_timer)
{
        u32 to_us;
        u16 status;

        to_us = tx_lpi_timer * 80;
        status = RTL_R16(tp, PHYstatus);
        if (status & LinkStatus) {
                /*link on*/
                if (HW_SUPP_PHY_LINK_SPEED_5000M(tp)) {
                        //5G : tx_lpi_timer * 12.8ns
                        //2.5G : tx_lpi_timer * 25.6ns
                        //Giga: tx_lpi_timer * 8ns
                        //100M : tx_lpi_timer * 80ns
                        if (status & (_5000bpsF))
                                to_us = (tx_lpi_timer * 128) / 10;
                        else if (status & _2500bpsF)
                                to_us = (tx_lpi_timer * 256) / 10;
                        else if (status & _1000bpsF)
                                to_us = tx_lpi_timer * 8;
                } else {
                        //2.5G : tx_lpi_timer * 3.2ns
                        //Giga: tx_lpi_timer * 8ns
                        //100M : tx_lpi_timer * 80ns
                        if (status & _2500bpsF)
                                to_us = (tx_lpi_timer * 32) / 10;
                        else if (status & _1000bpsF)
                                to_us = tx_lpi_timer * 8;
                }
        }

        //ns to us
        to_us /= 1000;

        return to_us;
}

static int
rtl_ethtool_get_eee(struct net_device *net, struct ethtool_eee *edata)
{
        struct rtl8125_private *tp = netdev_priv(net);
        struct ethtool_eee *eee = &tp->eee;
        u32 lp, adv, tx_lpi_timer, supported = 0;
        u16 val;

        if (unlikely(tp->rtk_enable_diag))
                return -EBUSY;

        /* Get Supported EEE */
        //val = mdio_direct_read_phy_ocp(tp, 0xA5C4);
        //supported = mmd_eee_cap_to_ethtool_sup_t(val);
        supported = eee->supported;

        /* Get advertisement EEE */
        val = mdio_direct_read_phy_ocp(tp, 0xA5D0);
        adv = mmd_eee_adv_to_ethtool_adv_t(val);
        val = mdio_direct_read_phy_ocp(tp, 0xA6D4);
        if (val & RTK_EEE_ADVERTISE_2500FULL)
                adv |= ADVERTISED_2500baseX_Full;

        /* Get LP advertisement EEE */
        val = mdio_direct_read_phy_ocp(tp, 0xA5D2);
        lp = mmd_eee_adv_to_ethtool_adv_t(val);
        val = mdio_direct_read_phy_ocp(tp, 0xA6D0);
        if (val & RTK_LPA_EEE_ADVERTISE_2500FULL)
                lp |= ADVERTISED_2500baseX_Full;

        /* Get EEE Tx LPI timer*/
        tx_lpi_timer = RTL_R16(tp, EEE_TXIDLE_TIMER_8125);

        val = rtl8125_mac_ocp_read(tp, 0xE040);
        val &= BIT_1 | BIT_0;

        edata->eee_enabled = !!val;
        edata->eee_active = !!(supported & adv & lp);
        edata->supported = supported;
        edata->advertised = adv;
        edata->lp_advertised = lp;
        edata->tx_lpi_enabled = edata->eee_enabled;
        edata->tx_lpi_timer = rtl8125_tx_lpi_timer_to_us(tp, tx_lpi_timer);

        return 0;
}

static int
rtl_ethtool_set_eee(struct net_device *net, struct ethtool_eee *edata)
{
        struct rtl8125_private *tp = netdev_priv(net);
        struct ethtool_eee *eee = &tp->eee;
        u32 advertising;
        int rc = 0;

        if (!HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp) ||
            tp->DASH)
                return -EOPNOTSUPP;

        if (unlikely(tp->rtk_enable_diag)) {
                dev_printk(KERN_WARNING, tp_to_dev(tp), "Diag Enabled\n");
                rc = -EBUSY;
                goto out;
        }

        if (tp->autoneg != AUTONEG_ENABLE) {
                dev_printk(KERN_WARNING, tp_to_dev(tp), "EEE requires autoneg\n");
                rc = -EINVAL;
                goto out;
        }

        if (edata->tx_lpi_enabled) {
                if (edata->tx_lpi_timer > tp->max_jumbo_frame_size ||
                    edata->tx_lpi_timer < ETH_MIN_MTU) {
                        dev_printk(KERN_WARNING, tp_to_dev(tp), "Valid LPI timer range is %d to %d. \n",
                                   ETH_MIN_MTU, tp->max_jumbo_frame_size);
                        rc = -EINVAL;
                        goto out;
                }
        }

        advertising = tp->advertising;
        if (!edata->advertised) {
                edata->advertised = advertising & eee->supported;
        } else if (edata->advertised & ~advertising) {
                dev_printk(KERN_WARNING, tp_to_dev(tp), "EEE advertised %x must be a subset of autoneg advertised speeds %x\n",
                           edata->advertised, advertising);
                rc = -EINVAL;
                goto out;
        }

        if (edata->advertised & ~eee->supported) {
                dev_printk(KERN_WARNING, tp_to_dev(tp), "EEE advertised %x must be a subset of support %x\n",
                           edata->advertised, eee->supported);
                rc = -EINVAL;
                goto out;
        }

        //tp->eee.eee_enabled = edata->eee_enabled;
        //tp->eee_adv_t = ethtool_adv_to_mmd_eee_adv_t(edata->advertised);

        dev_printk(KERN_WARNING, tp_to_dev(tp), "EEE tx_lpi_timer %x must be a subset of support %x\n",
                   edata->tx_lpi_timer, eee->tx_lpi_timer);

        eee->advertised = edata->advertised;
        eee->tx_lpi_enabled = edata->tx_lpi_enabled;
        eee->tx_lpi_timer = edata->tx_lpi_timer;
        eee->eee_enabled = edata->eee_enabled;

        if (eee->eee_enabled)
                rtl8125_enable_eee(tp);
        else
                rtl8125_disable_eee(tp);

        rtl_nway_reset(net);

        return rc;

out:

        return rc;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static const struct ethtool_ops rtl8125_ethtool_ops = {
        .get_drvinfo        = rtl8125_get_drvinfo,
        .get_regs_len       = rtl8125_get_regs_len,
        .get_link       = ethtool_op_get_link,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_ringparam      = rtl8125_get_ringparam,
        .set_ringparam      = rtl8125_set_ringparam,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        .get_settings       = rtl8125_get_settings,
        .set_settings       = rtl8125_set_settings,
#else
        .get_link_ksettings       = rtl8125_get_settings,
        .set_link_ksettings       = rtl8125_set_settings,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_pauseparam     = rtl8125_get_pauseparam,
        .set_pauseparam     = rtl8125_set_pauseparam,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_msglevel       = rtl8125_get_msglevel,
        .set_msglevel       = rtl8125_set_msglevel,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        .get_rx_csum        = rtl8125_get_rx_csum,
        .set_rx_csum        = rtl8125_set_rx_csum,
        .get_tx_csum        = rtl8125_get_tx_csum,
        .set_tx_csum        = rtl8125_set_tx_csum,
        .get_sg         = ethtool_op_get_sg,
        .set_sg         = ethtool_op_set_sg,
#ifdef NETIF_F_TSO
        .get_tso        = ethtool_op_get_tso,
        .set_tso        = ethtool_op_set_tso,
#endif //NETIF_F_TSO
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        .get_regs       = rtl8125_get_regs,
        .get_wol        = rtl8125_get_wol,
        .set_wol        = rtl8125_set_wol,
        .get_strings        = rtl8125_get_strings,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        .get_stats_count    = rtl8125_get_stats_count,
#else
        .get_sset_count     = rtl8125_get_sset_count,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        .get_ethtool_stats  = rtl8125_get_ethtool_stats,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#ifdef ETHTOOL_GPERMADDR
        .get_perm_addr      = ethtool_op_get_perm_addr,
#endif //ETHTOOL_GPERMADDR
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
        .get_eeprom     = rtl_get_eeprom,
        .get_eeprom_len     = rtl_get_eeprom_len,
#ifdef ENABLE_RSS_SUPPORT
        .get_rxnfc		= rtl8125_get_rxnfc,
        .set_rxnfc		= rtl8125_set_rxnfc,
        .get_rxfh_indir_size	= rtl8125_rss_indir_size,
        .get_rxfh_key_size	= rtl8125_get_rxfh_key_size,
        .get_rxfh		= rtl8125_get_rxfh,
        .set_rxfh		= rtl8125_set_rxfh,
#endif //ENABLE_RSS_SUPPORT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
#ifdef ENABLE_PTP_SUPPORT
        .get_ts_info        = rtl8125_get_ts_info,
#else
        .get_ts_info        = ethtool_op_get_ts_info,
#endif //ENABLE_PTP_SUPPORT
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
        .get_eee = rtl_ethtool_get_eee,
        .set_eee = rtl_ethtool_set_eee,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */
        .nway_reset = rtl_nway_reset,

};
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

#if 0

static int rtl8125_enable_green_feature(struct rtl8125_private *tp)
{
        u16 gphy_val;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8011);
                SetEthPhyOcpBit(tp, 0xA438, BIT_15);
                rtl8125_mdio_write(tp, 0x00, 0x9200);
                break;
        default:
                dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support Green Feature\n");
                break;
        }

        return 0;
}

static int rtl8125_disable_green_feature(struct rtl8125_private *tp)
{
        u16 gphy_val;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8011);
                ClearEthPhyOcpBit(tp, 0xA438, BIT_15);
                rtl8125_mdio_write(tp, 0x00, 0x9200);
                break;
        default:
                dev_printk(KERN_DEBUG, tp_to_dev(tp), "Not Support Green Feature\n");
                break;
        }

        return 0;
}

#endif

static void rtl8125_get_mac_version(struct rtl8125_private *tp)
{
        u32 reg,val32;
        u32 ICVerID;
        struct pci_dev *pdev = tp->pci_dev;

        val32 = RTL_R32(tp, TxConfig);
        reg = val32 & 0x7c800000;
        ICVerID = val32 & 0x00700000;

        switch (reg) {
        case 0x60800000:
                if (ICVerID == 0x00000000) {
                        tp->mcfg = CFG_METHOD_2;
                } else if (ICVerID == 0x100000) {
                        tp->mcfg = CFG_METHOD_3;
                } else {
                        tp->mcfg = CFG_METHOD_3;
                        tp->HwIcVerUnknown = TRUE;
                }

                tp->efuse_ver = EFUSE_SUPPORT_V4;
                break;
        case 0x64000000:
                if (ICVerID == 0x00000000) {
                        tp->mcfg = CFG_METHOD_4;
                } else if (ICVerID == 0x100000) {
                        tp->mcfg = CFG_METHOD_5;
                } else {
                        tp->mcfg = CFG_METHOD_5;
                        tp->HwIcVerUnknown = TRUE;
                }

                tp->efuse_ver = EFUSE_SUPPORT_V4;
                break;
        case 0x64800000:
                tp->mcfg = CFG_METHOD_8;

                tp->efuse_ver = EFUSE_SUPPORT_V4;
                break;
        default:
                printk("unknown chip version (%x)\n",reg);
                tp->mcfg = CFG_METHOD_DEFAULT;
                tp->HwIcVerUnknown = TRUE;
                tp->efuse_ver = EFUSE_NOT_SUPPORT;
                break;
        }

        if (pdev->subsystem_vendor == 0x8162) {
                if (tp->mcfg == CFG_METHOD_3)
                        tp->mcfg = CFG_METHOD_6;
                else if (tp->mcfg == CFG_METHOD_5)
                        tp->mcfg = CFG_METHOD_7;
        }
}

static void
rtl8125_print_mac_version(struct rtl8125_private *tp)
{
        int i;
        for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
                if (tp->mcfg == rtl_chip_info[i].mcfg) {
                        dprintk("Realtek PCIe 2.5GbE Family Controller mcfg = %04d\n",
                                rtl_chip_info[i].mcfg);
                        return;
                }
        }

        dprintk("mac_version == Unknown\n");
}

static void
rtl8125_tally_counter_addr_fill(struct rtl8125_private *tp)
{
        if (!tp->tally_paddr)
                return;

        RTL_W32(tp, CounterAddrHigh, (u64)tp->tally_paddr >> 32);
        RTL_W32(tp, CounterAddrLow, (u64)tp->tally_paddr & (DMA_BIT_MASK(32)));
}

static void
rtl8125_tally_counter_clear(struct rtl8125_private *tp)
{
        if (!tp->tally_paddr)
                return;

        RTL_W32(tp, CounterAddrHigh, (u64)tp->tally_paddr >> 32);
        RTL_W32(tp, CounterAddrLow, ((u64)tp->tally_paddr & (DMA_BIT_MASK(32))) | CounterReset);
}

static void
rtl8125_clear_phy_ups_reg(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                ClearEthPhyOcpBit(tp, 0xA466, BIT_0);
                break;
        };
        ClearEthPhyOcpBit(tp, 0xA468, BIT_3 | BIT_1);
}

static int
rtl8125_is_ups_resume(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8)
                return (rtl8125_mac_ocp_read(tp, 0xD42C) & BIT_8);

        return 0;
}

static void
rtl8125_clear_ups_resume_bit(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7)
                rtl8125_mac_ocp_write(tp, 0xD408, rtl8125_mac_ocp_read(tp, 0xD408) & ~(BIT_8));
}

static void
rtl8125_wait_phy_ups_resume(struct net_device *dev, u16 PhyState)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 TmpPhyState;
        int i=0;

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                do {
                        TmpPhyState = mdio_direct_read_phy_ocp(tp, 0xA420);
                        TmpPhyState &= 0x7;
                        mdelay(1);
                        i++;
                } while ((i < 100) && (TmpPhyState != PhyState));
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(i == 100);
#endif
}

void
rtl8125_enable_now_is_oob(struct rtl8125_private *tp)
{
        if ( tp->HwSuppNowIsOobVer == 1 ) {
                RTL_W8(tp, MCUCmd_reg, RTL_R8(tp, MCUCmd_reg) | Now_is_oob);
        }
}

void
rtl8125_disable_now_is_oob(struct rtl8125_private *tp)
{
        if ( tp->HwSuppNowIsOobVer == 1 ) {
                RTL_W8(tp, MCUCmd_reg, RTL_R8(tp, MCUCmd_reg) & ~Now_is_oob);
        }
}

static void
rtl8125_exit_oob(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 data16;

        rtl8125_disable_rx_packet_filter(tp);

        if (HW_DASH_SUPPORT_DASH(tp)) {
                rtl8125_driver_start(tp);
                rtl8125_dash2_disable_txrx(dev);
#ifdef ENABLE_DASH_SUPPORT
                DashHwInit(dev);
#endif
        }

#ifdef ENABLE_REALWOW_SUPPORT
        rtl8125_realwow_hw_init(dev);
#else
        //Disable realwow  function
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xC0BC, 0x00FF);
                break;
        }
#endif //ENABLE_REALWOW_SUPPORT

        rtl8125_nic_reset(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_disable_now_is_oob(tp);

                data16 = rtl8125_mac_ocp_read(tp, 0xE8DE) & ~BIT_14;
                rtl8125_mac_ocp_write(tp, 0xE8DE, data16);
                rtl8125_wait_ll_share_fifo_ready(dev);

                rtl8125_mac_ocp_write(tp, 0xC0AA, 0x07D0);
                rtl8125_mac_ocp_write(tp, 0xC0A6, 0x01B5);
                rtl8125_mac_ocp_write(tp, 0xC01E, 0x5555);

                rtl8125_wait_ll_share_fifo_ready(dev);
                break;
        }

        //wait ups resume (phy state 2)
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                if (rtl8125_is_ups_resume(dev)) {
                        rtl8125_wait_phy_ups_resume(dev, 2);
                        rtl8125_clear_ups_resume_bit(dev);
                        rtl8125_clear_phy_ups_reg(dev);
                }
                break;
        };
}

void
rtl8125_hw_disable_mac_mcu_bps(struct net_device *dev)
{
        u16 regAddr;

        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_enable_cfg9346_write(tp);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                rtl8125_disable_cfg9346_write(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xFC48, 0x0000);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                for (regAddr = 0xFC28; regAddr < 0xFC48; regAddr += 2) {
                        rtl8125_mac_ocp_write(tp, regAddr, 0x0000);
                }

                mdelay(3);

                rtl8125_mac_ocp_write(tp, 0xFC26, 0x0000);
                break;
        }
}

#ifndef ENABLE_USE_FIRMWARE_FILE
static void
rtl8125_switch_mac_mcu_ram_code_page(struct rtl8125_private *tp, u16 page)
{
        u16 tmpUshort;

        page &= (BIT_1 | BIT_0);
        tmpUshort = rtl8125_mac_ocp_read(tp, 0xE446);
        tmpUshort &= ~(BIT_1 | BIT_0);
        tmpUshort |= page;
        rtl8125_mac_ocp_write(tp, 0xE446, tmpUshort);
}

static void
_rtl8125_write_mac_mcu_ram_code(struct rtl8125_private *tp, const u16 *entry, u16 entry_cnt)
{
        u16 i;

        for (i = 0; i < entry_cnt; i++) {
                rtl8125_mac_ocp_write(tp, 0xF800 + i * 2, entry[i]);
        }
}

static void
_rtl8125_write_mac_mcu_ram_code_with_page(struct rtl8125_private *tp, const u16 *entry, u16 entry_cnt, u16 page_size)
{
        u16 i;
        u16 offset;

        if (page_size == 0) return;

        for (i = 0; i < entry_cnt; i++) {
                offset = i % page_size;
                if (offset == 0) {
                        u16 page = (i / page_size);
                        rtl8125_switch_mac_mcu_ram_code_page(tp, page);
                }
                rtl8125_mac_ocp_write(tp, 0xF800 + offset * 2, entry[i]);
        }
}

static void
rtl8125_write_mac_mcu_ram_code(struct rtl8125_private *tp, const u16 *entry, u16 entry_cnt)
{
        if (FALSE == HW_SUPPORT_MAC_MCU(tp)) return;
        if (entry == NULL || entry_cnt == 0) return;

        if (tp->MacMcuPageSize > 0)
                _rtl8125_write_mac_mcu_ram_code_with_page(tp, entry, entry_cnt, tp->MacMcuPageSize);
        else
                _rtl8125_write_mac_mcu_ram_code(tp, entry, entry_cnt);
}

static void
rtl8125_set_mac_mcu_8125a_1(struct net_device *dev)
{
        rtl8125_hw_disable_mac_mcu_bps(dev);
}

static void
rtl8125_set_mac_mcu_8125a_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        static const u16 mcu_patch_code_8125a_2[] =  {
                0xE010, 0xE012, 0xE022, 0xE024, 0xE029, 0xE02B, 0xE094, 0xE09D, 0xE09F,
                0xE0AA, 0xE0B5, 0xE0C6, 0xE0CC, 0xE0D1, 0xE0D6, 0xE0D8, 0xC602, 0xBE00,
                0x0000, 0xC60F, 0x73C4, 0x49B3, 0xF106, 0x73C2, 0xC608, 0xB406, 0xC609,
                0xFF80, 0xC605, 0xB406, 0xC605, 0xFF80, 0x0544, 0x0568, 0xE906, 0xCDE8,
                0xC602, 0xBE00, 0x0000, 0x48C1, 0x48C2, 0x9C46, 0xC402, 0xBC00, 0x0A12,
                0xC602, 0xBE00, 0x0EBA, 0x1501, 0xF02A, 0x1500, 0xF15D, 0xC661, 0x75C8,
                0x49D5, 0xF00A, 0x49D6, 0xF008, 0x49D7, 0xF006, 0x49D8, 0xF004, 0x75D2,
                0x49D9, 0xF150, 0xC553, 0x77A0, 0x75C8, 0x4855, 0x4856, 0x4857, 0x4858,
                0x48DA, 0x48DB, 0x49FE, 0xF002, 0x485A, 0x49FF, 0xF002, 0x485B, 0x9DC8,
                0x75D2, 0x4859, 0x9DD2, 0xC643, 0x75C0, 0x49D4, 0xF033, 0x49D0, 0xF137,
                0xE030, 0xC63A, 0x75C8, 0x49D5, 0xF00E, 0x49D6, 0xF00C, 0x49D7, 0xF00A,
                0x49D8, 0xF008, 0x75D2, 0x49D9, 0xF005, 0xC62E, 0x75C0, 0x49D7, 0xF125,
                0xC528, 0x77A0, 0xC627, 0x75C8, 0x4855, 0x4856, 0x4857, 0x4858, 0x48DA,
                0x48DB, 0x49FE, 0xF002, 0x485A, 0x49FF, 0xF002, 0x485B, 0x9DC8, 0x75D2,
                0x4859, 0x9DD2, 0xC616, 0x75C0, 0x4857, 0x9DC0, 0xC613, 0x75C0, 0x49DA,
                0xF003, 0x49D0, 0xF107, 0xC60B, 0xC50E, 0x48D9, 0x9DC0, 0x4859, 0x9DC0,
                0xC608, 0xC702, 0xBF00, 0x3AE0, 0xE860, 0xB400, 0xB5D4, 0xE908, 0xE86C,
                0x1200, 0xC409, 0x6780, 0x48F1, 0x8F80, 0xC404, 0xC602, 0xBE00, 0x10AA,
                0xC010, 0xEA7C, 0xC602, 0xBE00, 0x0000, 0x740A, 0x4846, 0x4847, 0x9C0A,
                0xC607, 0x74C0, 0x48C6, 0x9CC0, 0xC602, 0xBE00, 0x13FE, 0xE054, 0x72CA,
                0x4826, 0x4827, 0x9ACA, 0xC607, 0x72C0, 0x48A6, 0x9AC0, 0xC602, 0xBE00,
                0x07DC, 0xE054, 0xC60F, 0x74C4, 0x49CC, 0xF109, 0xC60C, 0x74CA, 0x48C7,
                0x9CCA, 0xC609, 0x74C0, 0x4846, 0x9CC0, 0xC602, 0xBE00, 0x2480, 0xE092,
                0xE0C0, 0xE054, 0x7420, 0x48C0, 0x9C20, 0x7444, 0xC602, 0xBE00, 0x12F8,
                0x1BFF, 0x46EB, 0x1BFF, 0xC102, 0xB900, 0x0D5A, 0x1BFF, 0x46EB, 0x1BFF,
                0xC102, 0xB900, 0x0E2A, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6486,
                0x0B15, 0x090E, 0x1139
        };

        rtl8125_hw_disable_mac_mcu_bps(dev);

        rtl8125_write_mac_mcu_ram_code(tp, mcu_patch_code_8125a_2, ARRAY_SIZE(mcu_patch_code_8125a_2));

        rtl8125_mac_ocp_write(tp, 0xFC26, 0x8000);

        rtl8125_mac_ocp_write(tp, 0xFC2A, 0x0540);
        rtl8125_mac_ocp_write(tp, 0xFC2E, 0x0A06);
        rtl8125_mac_ocp_write(tp, 0xFC30, 0x0EB8);
        rtl8125_mac_ocp_write(tp, 0xFC32, 0x3A5C);
        rtl8125_mac_ocp_write(tp, 0xFC34, 0x10A8);
        rtl8125_mac_ocp_write(tp, 0xFC40, 0x0D54);
        rtl8125_mac_ocp_write(tp, 0xFC42, 0x0E24);

        rtl8125_mac_ocp_write(tp, 0xFC48, 0x307A);
}

static void
rtl8125_set_mac_mcu_8125b_1(struct net_device *dev)
{
        rtl8125_hw_disable_mac_mcu_bps(dev);
}

static void
rtl8125_set_mac_mcu_8125b_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        static const u16 mcu_patch_code_8125b_2[] =  {
                0xE010, 0xE01B, 0xE026, 0xE037, 0xE03D, 0xE057, 0xE05B, 0xE060, 0xE062,
                0xE064, 0xE066, 0xE068, 0xE06A, 0xE06C, 0xE06E, 0xE070, 0x740A, 0x4846,
                0x4847, 0x9C0A, 0xC607, 0x74C0, 0x48C6, 0x9CC0, 0xC602, 0xBE00, 0x13F0,
                0xE054, 0x72CA, 0x4826, 0x4827, 0x9ACA, 0xC607, 0x72C0, 0x48A6, 0x9AC0,
                0xC602, 0xBE00, 0x081C, 0xE054, 0xC60F, 0x74C4, 0x49CC, 0xF109, 0xC60C,
                0x74CA, 0x48C7, 0x9CCA, 0xC609, 0x74C0, 0x4846, 0x9CC0, 0xC602, 0xBE00,
                0x2494, 0xE092, 0xE0C0, 0xE054, 0x7420, 0x48C0, 0x9C20, 0x7444, 0xC602,
                0xBE00, 0x12DC, 0x733A, 0x21B5, 0x25BC, 0x1304, 0xF111, 0x1B12, 0x1D2A,
                0x3168, 0x3ADA, 0x31AB, 0x1A00, 0x9AC0, 0x1300, 0xF1FB, 0x7620, 0x236E,
                0x276F, 0x1A3C, 0x22A1, 0x41B5, 0x9EE2, 0x76E4, 0x486F, 0x9EE4, 0xC602,
                0xBE00, 0x4A26, 0x733A, 0x49BB, 0xC602, 0xBE00, 0x47A2, 0x48C1, 0x48C2,
                0x9C46, 0xC402, 0xBC00, 0x0A52, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00,
                0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00,
                0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00,
                0x0000, 0xC602, 0xBE00, 0x0000
        };

        rtl8125_hw_disable_mac_mcu_bps(dev);

        rtl8125_write_mac_mcu_ram_code(tp, mcu_patch_code_8125b_2, ARRAY_SIZE(mcu_patch_code_8125b_2));

        rtl8125_mac_ocp_write(tp, 0xFC26, 0x8000);

        rtl8125_mac_ocp_write(tp, 0xFC28, 0x13E6);
        rtl8125_mac_ocp_write(tp, 0xFC2A, 0x0812);
        rtl8125_mac_ocp_write(tp, 0xFC2C, 0x248C);
        rtl8125_mac_ocp_write(tp, 0xFC2E, 0x12DA);
        rtl8125_mac_ocp_write(tp, 0xFC30, 0x4A20);
        rtl8125_mac_ocp_write(tp, 0xFC32, 0x47A0);
        //rtl8125_mac_ocp_write(tp, 0xFC34, 0x0A46);

        rtl8125_mac_ocp_write(tp, 0xFC48, 0x003F);
}

static void
rtl8125_set_mac_mcu_8126a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        static const u16 mcu_patch_code_8126a_1[] =  {
                0xE010, 0xE019, 0xE01B, 0xE01D, 0xE01F, 0xE021, 0xE023, 0xE025, 0xE027,
                0xE029, 0xE02B, 0xE02D, 0xE02F, 0xE031, 0xE033, 0xE035, 0x48C0, 0x9C66,
                0x7446, 0x4840, 0x48C1, 0x48C2, 0x9C46, 0xC402, 0xBC00, 0x0AD6, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000
        };

        rtl8125_hw_disable_mac_mcu_bps(dev);

        rtl8125_write_mac_mcu_ram_code(tp, mcu_patch_code_8126a_1, ARRAY_SIZE(mcu_patch_code_8126a_1));

        rtl8125_mac_ocp_write(tp, 0xFC26, 0x8000);

        rtl8125_mac_ocp_write(tp, 0xFC28, 0x0AAA);

        rtl8125_mac_ocp_write(tp, 0xFC48, 0x0001);
}

static void
rtl8125_hw_mac_mcu_config(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->NotWrMcuPatchCode == TRUE) return;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                rtl8125_set_mac_mcu_8125a_1(dev);
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                rtl8125_set_mac_mcu_8125a_2(dev);
                break;
        case CFG_METHOD_4:
                rtl8125_set_mac_mcu_8125b_1(dev);
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                rtl8125_set_mac_mcu_8125b_2(dev);
                break;
        case CFG_METHOD_8:
                rtl8125_set_mac_mcu_8126a_1(dev);
                break;
        }
}
#endif

#ifdef ENABLE_USE_FIRMWARE_FILE
static void rtl8125_release_firmware(struct rtl8125_private *tp)
{
        if (tp->rtl_fw) {
                rtl8125_fw_release_firmware(tp->rtl_fw);
                kfree(tp->rtl_fw);
                tp->rtl_fw = NULL;
        }
}

void rtl8125_apply_firmware(struct rtl8125_private *tp)
{
        /* TODO: release firmware if rtl_fw_write_firmware signals failure. */
        if (tp->rtl_fw) {
                rtl8125_fw_write_firmware(tp, tp->rtl_fw);
                /* At least one firmware doesn't reset tp->ocp_base. */
                tp->ocp_base = OCP_STD_PHY_BASE;

                /* PHY soft reset may still be in progress */
                //phy_read_poll_timeout(tp->phydev, MII_BMCR, val,
                //		      !(val & BMCR_RESET),
                //		      50000, 600000, true);
                rtl8125_wait_phy_reset_complete(tp);

                tp->hw_ram_code_ver = rtl8125_get_hw_phy_mcu_code_ver(tp);
                tp->sw_ram_code_ver = tp->hw_ram_code_ver;
                tp->HwHasWrRamCodeToMicroP = TRUE;
        }
}
#endif

static void
rtl8125_hw_init(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_enable_cfg9346_write(tp);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                rtl8125_disable_cfg9346_write(tp);
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                break;
        }

        //Disable UPS
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xD40A, rtl8125_mac_ocp_read( tp, 0xD40A) & ~(BIT_4));
                break;
        }

#ifndef ENABLE_USE_FIRMWARE_FILE
        if (!tp->rtl_fw)
                rtl8125_hw_mac_mcu_config(dev);
#endif

        /*disable ocp phy power saving*/
        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8)
                rtl8125_disable_ocp_phy_power_saving(dev);

        //Set PCIE uncorrectable error status mask pcie 0x108
        csi_tmp = rtl8125_csi_read(tp, 0x108);
        csi_tmp |= BIT_20;
        rtl8125_csi_write(tp, 0x108, csi_tmp);

        rtl8125_enable_cfg9346_write(tp);
        rtl8125_disable_linkchg_wakeup(dev);
        rtl8125_disable_cfg9346_write(tp);
        rtl8125_disable_magic_packet(dev);
        rtl8125_disable_d0_speedup(tp);
        rtl8125_set_pci_pme(tp, 0);
        if (s0_magic_packet == 1)
                rtl8125_enable_magic_packet(dev);

#ifdef ENABLE_USE_FIRMWARE_FILE
        if (tp->rtl_fw &&
            !tp->resume_not_chg_speed &&
            !(HW_DASH_SUPPORT_TYPE_3(tp) &&
              tp->HwPkgDet == 0x06))
                rtl8125_apply_firmware(tp);
#endif
}

static void
rtl8125_hw_ephy_config(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                rtl8125_ephy_write(tp, 0x01, 0xA812);
                rtl8125_ephy_write(tp, 0x09, 0x520C);
                rtl8125_ephy_write(tp, 0x04, 0xD000);
                rtl8125_ephy_write(tp, 0x0D, 0xF702);
                rtl8125_ephy_write(tp, 0x0A, 0x8653);
                rtl8125_ephy_write(tp, 0x06, 0x001E);
                rtl8125_ephy_write(tp, 0x08, 0x3595);
                rtl8125_ephy_write(tp, 0x20, 0x9455);
                rtl8125_ephy_write(tp, 0x21, 0x99FF);
                rtl8125_ephy_write(tp, 0x02, 0x6046);
                rtl8125_ephy_write(tp, 0x29, 0xFE00);
                rtl8125_ephy_write(tp, 0x23, 0xAB62);

                rtl8125_ephy_write(tp, 0x41, 0xA80C);
                rtl8125_ephy_write(tp, 0x49, 0x520C);
                rtl8125_ephy_write(tp, 0x44, 0xD000);
                rtl8125_ephy_write(tp, 0x4D, 0xF702);
                rtl8125_ephy_write(tp, 0x4A, 0x8653);
                rtl8125_ephy_write(tp, 0x46, 0x001E);
                rtl8125_ephy_write(tp, 0x48, 0x3595);
                rtl8125_ephy_write(tp, 0x60, 0x9455);
                rtl8125_ephy_write(tp, 0x61, 0x99FF);
                rtl8125_ephy_write(tp, 0x42, 0x6046);
                rtl8125_ephy_write(tp, 0x69, 0xFE00);
                rtl8125_ephy_write(tp, 0x63, 0xAB62);
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                rtl8125_ephy_write(tp, 0x04, 0xD000);
                rtl8125_ephy_write(tp, 0x0A, 0x8653);
                rtl8125_ephy_write(tp, 0x23, 0xAB66);
                rtl8125_ephy_write(tp, 0x20, 0x9455);
                rtl8125_ephy_write(tp, 0x21, 0x99FF);
                rtl8125_ephy_write(tp, 0x29, 0xFE04);

                rtl8125_ephy_write(tp, 0x44, 0xD000);
                rtl8125_ephy_write(tp, 0x4A, 0x8653);
                rtl8125_ephy_write(tp, 0x63, 0xAB66);
                rtl8125_ephy_write(tp, 0x60, 0x9455);
                rtl8125_ephy_write(tp, 0x61, 0x99FF);
                rtl8125_ephy_write(tp, 0x69, 0xFE04);

                ClearAndSetPCIePhyBit(tp,
                                      0x2A,
                                      (BIT_14 | BIT_13 | BIT_12),
                                      (BIT_13 | BIT_12)
                                     );
                ClearPCIePhyBit(tp, 0x19, BIT_6);
                SetPCIePhyBit(tp, 0x1B, (BIT_11 | BIT_10 | BIT_9));
                ClearPCIePhyBit(tp, 0x1B, (BIT_14 | BIT_13 | BIT_12));
                rtl8125_ephy_write(tp, 0x02, 0x6042);
                rtl8125_ephy_write(tp, 0x06, 0x0014);

                ClearAndSetPCIePhyBit(tp,
                                      0x6A,
                                      (BIT_14 | BIT_13 | BIT_12),
                                      (BIT_13 | BIT_12)
                                     );
                ClearPCIePhyBit(tp, 0x59, BIT_6);
                SetPCIePhyBit(tp, 0x5B, (BIT_11 | BIT_10 | BIT_9));
                ClearPCIePhyBit(tp, 0x5B, (BIT_14 | BIT_13 | BIT_12));
                rtl8125_ephy_write(tp, 0x42, 0x6042);
                rtl8125_ephy_write(tp, 0x46, 0x0014);
                break;
        case CFG_METHOD_4:
                rtl8125_ephy_write(tp, 0x06, 0x001F);
                rtl8125_ephy_write(tp, 0x0A, 0xB66B);
                rtl8125_ephy_write(tp, 0x01, 0xA852);
                rtl8125_ephy_write(tp, 0x24, 0x0008);
                rtl8125_ephy_write(tp, 0x2F, 0x6052);
                rtl8125_ephy_write(tp, 0x0D, 0xF716);
                rtl8125_ephy_write(tp, 0x20, 0xD477);
                rtl8125_ephy_write(tp, 0x21, 0x4477);
                rtl8125_ephy_write(tp, 0x22, 0x0013);
                rtl8125_ephy_write(tp, 0x23, 0xBB66);
                rtl8125_ephy_write(tp, 0x0B, 0xA909);
                rtl8125_ephy_write(tp, 0x29, 0xFF04);
                rtl8125_ephy_write(tp, 0x1B, 0x1EA0);

                rtl8125_ephy_write(tp, 0x46, 0x001F);
                rtl8125_ephy_write(tp, 0x4A, 0xB66B);
                rtl8125_ephy_write(tp, 0x41, 0xA84A);
                rtl8125_ephy_write(tp, 0x64, 0x000C);
                rtl8125_ephy_write(tp, 0x6F, 0x604A);
                rtl8125_ephy_write(tp, 0x4D, 0xF716);
                rtl8125_ephy_write(tp, 0x60, 0xD477);
                rtl8125_ephy_write(tp, 0x61, 0x4477);
                rtl8125_ephy_write(tp, 0x62, 0x0013);
                rtl8125_ephy_write(tp, 0x63, 0xBB66);
                rtl8125_ephy_write(tp, 0x4B, 0xA909);
                rtl8125_ephy_write(tp, 0x69, 0xFF04);
                rtl8125_ephy_write(tp, 0x5B, 0x1EA0);
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                rtl8125_ephy_write(tp, 0x0B, 0xA908);
                rtl8125_ephy_write(tp, 0x1E, 0x20EB);
                rtl8125_ephy_write(tp, 0x22, 0x0023);
                rtl8125_ephy_write(tp, 0x02, 0x60C2);
                rtl8125_ephy_write(tp, 0x29, 0xFF00);

                rtl8125_ephy_write(tp, 0x4B, 0xA908);
                rtl8125_ephy_write(tp, 0x5E, 0x28EB);
                rtl8125_ephy_write(tp, 0x62, 0x0023);
                rtl8125_ephy_write(tp, 0x42, 0x60C2);
                rtl8125_ephy_write(tp, 0x69, 0xFF00);
                break;
        case CFG_METHOD_8:
                break;
        }
}

static u16
rtl8125_get_hw_phy_mcu_code_ver(struct rtl8125_private *tp)
{
        u16 hw_ram_code_ver = ~0;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x801E);
                hw_ram_code_ver = mdio_direct_read_phy_ocp(tp, 0xA438);
                break;
        }

        return hw_ram_code_ver;
}

static int
rtl8125_check_hw_phy_mcu_code_ver(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ram_code_ver_match = 0;

        tp->hw_ram_code_ver = rtl8125_get_hw_phy_mcu_code_ver(tp);

        if (tp->hw_ram_code_ver == tp->sw_ram_code_ver) {
                ram_code_ver_match = 1;
                tp->HwHasWrRamCodeToMicroP = TRUE;
        }

        return ram_code_ver_match;
}

bool
rtl8125_set_phy_mcu_patch_request(struct rtl8125_private *tp)
{
        u16 gphy_val;
        u16 WaitCount;
        bool bSuccess = TRUE;

        SetEthPhyOcpBit(tp, 0xB820, BIT_4);

        WaitCount = 0;
        do {
                gphy_val = mdio_direct_read_phy_ocp(tp, 0xB800);
                udelay(100);
                WaitCount++;
        } while (!(gphy_val & BIT_6) && (WaitCount < 1000));

        if (!(gphy_val & BIT_6) && (WaitCount == 1000)) bSuccess = FALSE;

        if (!bSuccess)
                dprintk("rtl8125_set_phy_mcu_patch_request fail.\n");

        return bSuccess;
}

bool
rtl8125_clear_phy_mcu_patch_request(struct rtl8125_private *tp)
{
        u16 gphy_val;
        u16 WaitCount;
        bool bSuccess = TRUE;

        ClearEthPhyOcpBit(tp, 0xB820, BIT_4);

        WaitCount = 0;
        do {
                gphy_val = mdio_direct_read_phy_ocp(tp, 0xB800);
                udelay(100);
                WaitCount++;
        } while ((gphy_val & BIT_6) && (WaitCount < 1000));

        if ((gphy_val & BIT_6) && (WaitCount == 1000)) bSuccess = FALSE;

        if (!bSuccess)
                dprintk("rtl8125_clear_phy_mcu_patch_request fail.\n");

        return bSuccess;
}

#ifndef ENABLE_USE_FIRMWARE_FILE
static void
rtl8125_write_hw_phy_mcu_code_ver(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x801E);
                mdio_direct_write_phy_ocp(tp, 0xA438, tp->sw_ram_code_ver);
                tp->hw_ram_code_ver = tp->sw_ram_code_ver;
                break;
        }
}

static void
rtl8125_acquire_phy_mcu_patch_key_lock(struct rtl8125_private *tp)
{
        u16 PatchKey;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                PatchKey = 0x8600;
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                PatchKey = 0x8601;
                break;
        case CFG_METHOD_4:
                PatchKey = 0x3700;
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                PatchKey = 0x3701;
                break;
        default:
                return;
        }
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8024);
        mdio_direct_write_phy_ocp(tp, 0xA438, PatchKey);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xB82E);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0001);
}

static void
rtl8125_release_phy_mcu_patch_key_lock(struct rtl8125_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x0000);
                mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
                ClearEthPhyOcpBit(tp, 0xB82E, BIT_0);
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8024);
                mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
                break;
        default:
                break;
        }
}

static void
rtl8125_set_phy_mcu_ram_code(struct net_device *dev, const u16 *ramcode, u16 codesize)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 i;
        u16 addr;
        u16 val;

        if (ramcode == NULL || codesize % 2) {
                goto out;
        }

        for (i = 0; i < codesize; i += 2) {
                addr = ramcode[i];
                val = ramcode[i + 1];
                if (addr == 0xFFFF && val == 0xFFFF) {
                        break;
                }
                mdio_direct_write_phy_ocp(tp, addr, val);
        }

out:
        return;
}

static void
rtl8125_enable_phy_disable_mode(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppCheckPhyDisableModeVer) {
        case 3:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_5);
                break;
        }

        dprintk("enable phy disable mode.\n");
}

static void
rtl8125_disable_phy_disable_mode(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->HwSuppCheckPhyDisableModeVer) {
        case 3:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_5);
                break;
        }

        mdelay(1);

        dprintk("disable phy disable mode.\n");
}

static void
rtl8125_set_hw_phy_before_init_phy_mcu(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u16 PhyRegValue;

        switch (tp->mcfg) {
        case CFG_METHOD_4:
                mdio_direct_write_phy_ocp(tp, 0xBF86, 0x9000);

                SetEthPhyOcpBit(tp, 0xC402, BIT_10);
                ClearEthPhyOcpBit(tp, 0xC402, BIT_10);

                PhyRegValue = mdio_direct_read_phy_ocp(tp, 0xBF86);
                PhyRegValue &= (BIT_1 | BIT_0);
                if (PhyRegValue != 0)
                        dprintk("PHY watch dog not clear, value = 0x%x \n", PhyRegValue);

                mdio_direct_write_phy_ocp(tp, 0xBD86, 0x1010);
                mdio_direct_write_phy_ocp(tp, 0xBD88, 0x1010);

                ClearAndSetEthPhyOcpBit(tp,
                                        0xBD4E,
                                        BIT_11 | BIT_10,
                                        BIT_11);
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBF46,
                                        BIT_11 | BIT_10 | BIT_9 | BIT_8,
                                        BIT_10 | BIT_9 | BIT_8);
                break;
        }
}

static void
rtl8125_real_set_phy_mcu_8125a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_acquire_phy_mcu_patch_key_lock(tp);


        SetEthPhyOcpBit(tp, 0xB820, BIT_7);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8013);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8021);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x802f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x803d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8042);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8051);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8051);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa088);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a50);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8008);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1a3);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x401a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd707);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40c2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60a6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f8b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a6c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8080);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd019);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1a2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x401a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd707);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40c4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60a6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f8b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a84);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8970);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c07);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0901);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcf09);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd705);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xceff);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf0a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1213);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8401);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8580);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1253);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd064);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd181);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4018);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc50f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd706);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2c59);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x804d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc60f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc605);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x10fd);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA026);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA024);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA022);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x10f4);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA020);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1252);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA006);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1206);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA004);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a78);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a60);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a4f);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA008);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3f00);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0010);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8066);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x807c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8089);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x808e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80b2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80c2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x62db);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x655c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd73e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x614a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0505);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0509);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x653c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd73e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x614a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0502);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0506);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x050a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd73e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x614a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0505);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0506);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x050c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd73e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x614a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0509);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x050a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x050c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0508);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0304);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd73e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x614a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0321);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0502);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0321);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0321);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0508);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0321);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0346);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8208);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x609d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa50f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x001a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x001a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x607d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00ab);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60fd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa50f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaa0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x017b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a05);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x017b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60fd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa50f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaa0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x01e0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a05);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x01e0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60fd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa50f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaa0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0231);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0503);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a05);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0231);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08E);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0221);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08A);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x01ce);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA088);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0169);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA086);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00a6);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA084);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x000d);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA082);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0308);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA080);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x029f);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA090);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x007f);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0020);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8017);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8029);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8054);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x805a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8064);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80a7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9430);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9480);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb408);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd120);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd057);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x064b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb80);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9906);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0567);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb94);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x82a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x800a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8406);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8dff);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0773);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb91);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4063);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd139);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd140);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07dc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa110);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa2a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4045);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa180);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x405d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa720);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0742);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07ec);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f74);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0742);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7fb6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x82a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07dc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x064b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07c0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5fa7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0481);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x94bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x870c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa00a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa280);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8220);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x078e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb92);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4063);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd140);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd150);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd703);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6121);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x61a2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6223);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf02f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d10);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf00f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d20);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf00a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d30);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf005);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d40);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa008);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4046);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x405d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa720);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0742);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07f7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f74);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0742);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7fb5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x800a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3ad4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0537);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x064b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8301);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x800a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x82a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa70c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9402);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x890c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x064b);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10E);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0642);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0686);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10A);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0788);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA108);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x047b);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA106);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x065c);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA104);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0769);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA102);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0565);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA100);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x06f9);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA110);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00ff);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb87c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8530);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb87e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf85);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3caf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8593);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf85);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9caf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x85a5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5afb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe083);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfb0c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x020d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x021b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x10bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86d7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbe0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x83fc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1b10);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xda02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xdd02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5afb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe083);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfd0c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x020d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x021b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x10bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86dd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86e0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbe0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x83fe);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1b10);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf2f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbd02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2cac);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0286);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x65af);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x212b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x022c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86b6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf21);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cd1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x03bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8710);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x870d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8719);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8716);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x871f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x871c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8728);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8725);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8707);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbad);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x281c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd100);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1302);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2202);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2b02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae1a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd101);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1302);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2202);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2b02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd101);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3402);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3102);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3d02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3a02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4302);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4c02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4902);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd100);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2e02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4602);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf87);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4f02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ab7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf35);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7ff8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfaef);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x69bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86e3);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86fb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86e6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86fe);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86e9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86ec);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfbbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x025a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7bf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86ef);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0262);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7cbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86f2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0262);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7cbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86f5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0262);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7cbf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x86f8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0262);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7cef);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x96fe);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfc04);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf8fa);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xef69);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xef02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6273);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf202);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6273);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf502);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6273);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbf86);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf802);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6273);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xef96);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfefc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0420);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb540);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x53b5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4086);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb540);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb9b5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40c8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb03a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc8b0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbac8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb13a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc8b1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xba77);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbd26);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffbd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2677);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbd28);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffbd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbd26);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc8bd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2640);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbd28);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc8bd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x28bb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa430);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x98b0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1eba);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb01e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xdcb0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e98);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb09e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbab0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9edc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb09e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x98b1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1eba);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb11e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xdcb1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e98);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb19e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbab1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9edc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb19e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x11b0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e22);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb01e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x33b0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e11);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb09e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x22b0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9e33);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb09e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x11b1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e22);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb11e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x33b1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1e11);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb19e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x22b1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9e33);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb19e);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb85e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2f71);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb860);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x20d9);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb862);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2109);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb864);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x34e7);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb878);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x000f);


        ClearEthPhyOcpBit(tp, 0xB820, BIT_7);


        rtl8125_release_phy_mcu_patch_key_lock(tp);
}

static void
rtl8125_set_phy_mcu_8125a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8125a_1(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);
}

static void
rtl8125_real_set_phy_mcu_8125a_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_acquire_phy_mcu_patch_key_lock(tp);


        SetEthPhyOcpBit(tp, 0xB820, BIT_7);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x808b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x808f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8093);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8097);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x809d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80a1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80aa);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x607b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf00e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x42da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf01e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x615b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1456);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14a4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f2e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf01c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1456);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14a4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f2e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf024);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1456);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14a4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f2e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf02c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1456);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14a4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x14bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f2e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf034);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd719);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4118);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac11);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa410);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4779);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1444);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf034);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd719);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4118);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac22);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa420);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4559);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1444);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf023);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd719);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4118);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac44);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa440);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4339);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1444);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd719);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4118);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac88);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa480);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xce00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4119);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xac0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1444);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf001);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1456);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd718);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5fac);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc48f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x141b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd504);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x121a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd0b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1bb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0898);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd0b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1bb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a0e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd064);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd18a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0b7e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x401c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd501);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa804);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8804);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x053b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd500);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa301);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0648);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc520);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa201);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x252d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1646);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd708);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4006);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1646);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0308);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA026);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0307);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA024);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1645);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA022);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0647);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA020);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x053a);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA006);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0b7c);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA004);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0a0c);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0896);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x11a1);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA008);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xff00);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0010);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8015);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x801a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xad02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x02d7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00ed);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0509);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xc100);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x008f);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08E);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA08A);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA088);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA086);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA084);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA082);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x008d);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA080);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00eb);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA090);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0103);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA016);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0020);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA012);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8014);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8018);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8024);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8051);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8055);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8072);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x80dc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfffd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfffd);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8301);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x800a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x82a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa70c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x9402);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x890c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8840);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa380);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x066e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb91);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4063);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd139);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd140);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa110);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa2a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4085);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa180);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8280);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x405d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa720);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0743);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07f0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5f74);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0743);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7fb6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x82a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0c0f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x066e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd158);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd04d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x03d4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x94bc);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x870c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8380);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd10d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07c4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5fb4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa190);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa00a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa280);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa404);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa220);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd130);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07c4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5fb4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xbb80);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1c4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd074);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa301);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x604b);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa90c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0556);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xcb92);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4063);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd116);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd119);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd040);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd703);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x60a0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6241);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x63e2);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6583);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf054);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x611e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d10);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf02f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d50);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf02a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x611e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d20);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf021);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d60);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf01c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x611e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d30);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf013);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d70);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf00e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x611e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x40da);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d40);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf005);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d80);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x405d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa720);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd700);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x5ff4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa008);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd704);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x4046);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0743);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07fb);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd703);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7f6f);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7f4e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7f2d);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7f0c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x800a);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0cf0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0d00);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07e8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8010);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa740);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0743);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7fb5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd701);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3ad4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0556);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8610);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x066e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd1f5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xd049);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x1800);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x01ec);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10E);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x01ea);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x06a9);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA10A);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x078a);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA108);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x03d2);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA106);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x067f);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA104);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0665);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA102);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA100);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xA110);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00fc);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb87c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8530);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb87e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf85);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x3caf);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8545);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf85);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x45af);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8545);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xee82);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf900);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0103);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xaf03);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb7f8);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe0a6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00e1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa601);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xef01);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x58f0);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa080);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x37a1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8402);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae16);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa185);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x02ae);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x11a1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8702);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae0c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xa188);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x02ae);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x07a1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8902);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae02);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xae1c);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe0b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x62e1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb463);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6901);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe4b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x62e5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb463);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe0b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x62e1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb463);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6901);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xe4b4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x62e5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xb463);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xfc04);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb85e);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x03b3);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb860);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb862);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb864);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xffff);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0xb878);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0001);


        ClearEthPhyOcpBit(tp, 0xB820, BIT_7);


        rtl8125_release_phy_mcu_patch_key_lock(tp);
}

static void
rtl8125_set_phy_mcu_8125a_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8125a_2(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);
}

static const u16 phy_mcu_ram_code_8125b_1[] = {
        0xa436, 0x8024, 0xa438, 0x3700, 0xa436, 0xB82E, 0xa438, 0x0001,
        0xb820, 0x0090, 0xa436, 0xA016, 0xa438, 0x0000, 0xa436, 0xA012,
        0xa438, 0x0000, 0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010,
        0xa438, 0x1800, 0xa438, 0x8025, 0xa438, 0x1800, 0xa438, 0x803a,
        0xa438, 0x1800, 0xa438, 0x8044, 0xa438, 0x1800, 0xa438, 0x8083,
        0xa438, 0x1800, 0xa438, 0x808d, 0xa438, 0x1800, 0xa438, 0x808d,
        0xa438, 0x1800, 0xa438, 0x808d, 0xa438, 0xd712, 0xa438, 0x4077,
        0xa438, 0xd71e, 0xa438, 0x4159, 0xa438, 0xd71e, 0xa438, 0x6099,
        0xa438, 0x7f44, 0xa438, 0x1800, 0xa438, 0x1a14, 0xa438, 0x9040,
        0xa438, 0x9201, 0xa438, 0x1800, 0xa438, 0x1b1a, 0xa438, 0xd71e,
        0xa438, 0x2425, 0xa438, 0x1a14, 0xa438, 0xd71f, 0xa438, 0x3ce5,
        0xa438, 0x1afb, 0xa438, 0x1800, 0xa438, 0x1b00, 0xa438, 0xd712,
        0xa438, 0x4077, 0xa438, 0xd71e, 0xa438, 0x4159, 0xa438, 0xd71e,
        0xa438, 0x60b9, 0xa438, 0x2421, 0xa438, 0x1c17, 0xa438, 0x1800,
        0xa438, 0x1a14, 0xa438, 0x9040, 0xa438, 0x1800, 0xa438, 0x1c2c,
        0xa438, 0xd71e, 0xa438, 0x2425, 0xa438, 0x1a14, 0xa438, 0xd71f,
        0xa438, 0x3ce5, 0xa438, 0x1c0f, 0xa438, 0x1800, 0xa438, 0x1c13,
        0xa438, 0xd702, 0xa438, 0xd501, 0xa438, 0x6072, 0xa438, 0x8401,
        0xa438, 0xf002, 0xa438, 0xa401, 0xa438, 0x1000, 0xa438, 0x146e,
        0xa438, 0x1800, 0xa438, 0x0b77, 0xa438, 0xd703, 0xa438, 0x665d,
        0xa438, 0x653e, 0xa438, 0x641f, 0xa438, 0xd700, 0xa438, 0x62c4,
        0xa438, 0x6185, 0xa438, 0x6066, 0xa438, 0x1800, 0xa438, 0x165a,
        0xa438, 0xc101, 0xa438, 0xcb00, 0xa438, 0x1000, 0xa438, 0x1945,
        0xa438, 0xd700, 0xa438, 0x7fa6, 0xa438, 0x1800, 0xa438, 0x807d,
        0xa438, 0xc102, 0xa438, 0xcb00, 0xa438, 0x1000, 0xa438, 0x1945,
        0xa438, 0xd700, 0xa438, 0x2569, 0xa438, 0x8058, 0xa438, 0x1800,
        0xa438, 0x807d, 0xa438, 0xc104, 0xa438, 0xcb00, 0xa438, 0x1000,
        0xa438, 0x1945, 0xa438, 0xd700, 0xa438, 0x7fa4, 0xa438, 0x1800,
        0xa438, 0x807d, 0xa438, 0xc120, 0xa438, 0xcb00, 0xa438, 0x1000,
        0xa438, 0x1945, 0xa438, 0xd703, 0xa438, 0x7fbf, 0xa438, 0x1800,
        0xa438, 0x807d, 0xa438, 0xc140, 0xa438, 0xcb00, 0xa438, 0x1000,
        0xa438, 0x1945, 0xa438, 0xd703, 0xa438, 0x7fbe, 0xa438, 0x1800,
        0xa438, 0x807d, 0xa438, 0xc180, 0xa438, 0xcb00, 0xa438, 0x1000,
        0xa438, 0x1945, 0xa438, 0xd703, 0xa438, 0x7fbd, 0xa438, 0xc100,
        0xa438, 0xcb00, 0xa438, 0xd708, 0xa438, 0x6018, 0xa438, 0x1800,
        0xa438, 0x165a, 0xa438, 0x1000, 0xa438, 0x14f6, 0xa438, 0xd014,
        0xa438, 0xd1e3, 0xa438, 0x1000, 0xa438, 0x1356, 0xa438, 0xd705,
        0xa438, 0x5fbe, 0xa438, 0x1800, 0xa438, 0x1559, 0xa436, 0xA026,
        0xa438, 0xffff, 0xa436, 0xA024, 0xa438, 0xffff, 0xa436, 0xA022,
        0xa438, 0xffff, 0xa436, 0xA020, 0xa438, 0x1557, 0xa436, 0xA006,
        0xa438, 0x1677, 0xa436, 0xA004, 0xa438, 0x0b75, 0xa436, 0xA002,
        0xa438, 0x1c17, 0xa436, 0xA000, 0xa438, 0x1b04, 0xa436, 0xA008,
        0xa438, 0x1f00, 0xa436, 0xA016, 0xa438, 0x0020, 0xa436, 0xA012,
        0xa438, 0x0000, 0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010,
        0xa438, 0x1800, 0xa438, 0x817f, 0xa438, 0x1800, 0xa438, 0x82ab,
        0xa438, 0x1800, 0xa438, 0x83f8, 0xa438, 0x1800, 0xa438, 0x8444,
        0xa438, 0x1800, 0xa438, 0x8454, 0xa438, 0x1800, 0xa438, 0x8459,
        0xa438, 0x1800, 0xa438, 0x8465, 0xa438, 0xcb11, 0xa438, 0xa50c,
        0xa438, 0x8310, 0xa438, 0xd701, 0xa438, 0x4076, 0xa438, 0x0c03,
        0xa438, 0x0903, 0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f,
        0xa438, 0x0d00, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d00,
        0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0x1000, 0xa438, 0x0a4d,
        0xa438, 0xcb12, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5f84, 0xa438, 0xd102, 0xa438, 0xd040, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xd701,
        0xa438, 0x60f3, 0xa438, 0xd413, 0xa438, 0x1000, 0xa438, 0x0a37,
        0xa438, 0xd410, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xcb13,
        0xa438, 0xa108, 0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8108,
        0xa438, 0xa00a, 0xa438, 0xa910, 0xa438, 0xa780, 0xa438, 0xd14a,
        0xa438, 0xd048, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd701,
        0xa438, 0x6255, 0xa438, 0xd700, 0xa438, 0x5f74, 0xa438, 0x6326,
        0xa438, 0xd702, 0xa438, 0x5f07, 0xa438, 0x800a, 0xa438, 0xa004,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8004, 0xa438, 0xa001,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001, 0xa438, 0x0c03,
        0xa438, 0x0902, 0xa438, 0xffe2, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x5fab, 0xa438, 0xba08, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f8b, 0xa438, 0x9a08,
        0xa438, 0x800a, 0xa438, 0xd702, 0xa438, 0x6535, 0xa438, 0xd40d,
        0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xcb14, 0xa438, 0xa004,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8004, 0xa438, 0xa001,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001, 0xa438, 0xa00a,
        0xa438, 0xa780, 0xa438, 0xd14a, 0xa438, 0xd048, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0x6206,
        0xa438, 0xd702, 0xa438, 0x5f47, 0xa438, 0x800a, 0xa438, 0xa004,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8004, 0xa438, 0xa001,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001, 0xa438, 0x0c03,
        0xa438, 0x0902, 0xa438, 0x1800, 0xa438, 0x8064, 0xa438, 0x800a,
        0xa438, 0xd40e, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xb920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac,
        0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x7f8c, 0xa438, 0xd701, 0xa438, 0x6073, 0xa438, 0xd701,
        0xa438, 0x4216, 0xa438, 0xa004, 0xa438, 0x1000, 0xa438, 0x0a42,
        0xa438, 0x8004, 0xa438, 0xa001, 0xa438, 0x1000, 0xa438, 0x0a42,
        0xa438, 0x8001, 0xa438, 0xd120, 0xa438, 0xd040, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0x8504,
        0xa438, 0xcb21, 0xa438, 0xa301, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5f9f, 0xa438, 0x8301, 0xa438, 0xd704,
        0xa438, 0x40e0, 0xa438, 0xd196, 0xa438, 0xd04d, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xcb22,
        0xa438, 0x1000, 0xa438, 0x0a6d, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0xa640, 0xa438, 0x9503, 0xa438, 0x8910, 0xa438, 0x8720,
        0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d01,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d01, 0xa438, 0x1000,
        0xa438, 0x0a7d, 0xa438, 0x0c1f, 0xa438, 0x0f14, 0xa438, 0xcb23,
        0xa438, 0x8fc0, 0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0xaf40,
        0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0x0cc0, 0xa438, 0x0f80,
        0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0xafc0, 0xa438, 0x1000,
        0xa438, 0x0a25, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd701,
        0xa438, 0x5dee, 0xa438, 0xcb24, 0xa438, 0x8f1f, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd701, 0xa438, 0x7f6e, 0xa438, 0xa111,
        0xa438, 0xa215, 0xa438, 0xa401, 0xa438, 0x8404, 0xa438, 0xa720,
        0xa438, 0xcb25, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x8640,
        0xa438, 0x9503, 0xa438, 0x1000, 0xa438, 0x0b43, 0xa438, 0x1000,
        0xa438, 0x0b86, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xb920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac,
        0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x7f8c, 0xa438, 0xcb26, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x5f82, 0xa438, 0x8111, 0xa438, 0x8205,
        0xa438, 0x8404, 0xa438, 0xcb27, 0xa438, 0xd404, 0xa438, 0x1000,
        0xa438, 0x0a37, 0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f,
        0xa438, 0x0d02, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d02,
        0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0xa710, 0xa438, 0xa104,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8104, 0xa438, 0xa001,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001, 0xa438, 0xa120,
        0xa438, 0xaa0f, 0xa438, 0x8110, 0xa438, 0xa284, 0xa438, 0xa404,
        0xa438, 0xa00a, 0xa438, 0xd193, 0xa438, 0xd046, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xcb28,
        0xa438, 0xa110, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fa8, 0xa438, 0x8110, 0xa438, 0x8284, 0xa438, 0xa404,
        0xa438, 0x800a, 0xa438, 0x8710, 0xa438, 0xb804, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f82, 0xa438, 0x9804,
        0xa438, 0xcb29, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5f85, 0xa438, 0xa710, 0xa438, 0xb820, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f65, 0xa438, 0x9820,
        0xa438, 0xcb2a, 0xa438, 0xa190, 0xa438, 0xa284, 0xa438, 0xa404,
        0xa438, 0xa00a, 0xa438, 0xd13d, 0xa438, 0xd04a, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x3444, 0xa438, 0x8149,
        0xa438, 0xa220, 0xa438, 0xd1a0, 0xa438, 0xd040, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x3444, 0xa438, 0x8151,
        0xa438, 0xd702, 0xa438, 0x5f51, 0xa438, 0xcb2f, 0xa438, 0xa302,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd708, 0xa438, 0x5f63,
        0xa438, 0xd411, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0x8302,
        0xa438, 0xd409, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xb920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac,
        0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x7f8c, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5fa3, 0xa438, 0x8190, 0xa438, 0x82a4, 0xa438, 0x8404,
        0xa438, 0x800a, 0xa438, 0xb808, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x7fa3, 0xa438, 0x9808, 0xa438, 0x1800,
        0xa438, 0x0433, 0xa438, 0xcb15, 0xa438, 0xa508, 0xa438, 0xd700,
        0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d01, 0xa438, 0xf003,
        0xa438, 0x0c1f, 0xa438, 0x0d01, 0xa438, 0x1000, 0xa438, 0x0a7d,
        0xa438, 0x1000, 0xa438, 0x0a4d, 0xa438, 0xa301, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5f9f, 0xa438, 0x8301,
        0xa438, 0xd704, 0xa438, 0x40e0, 0xa438, 0xd115, 0xa438, 0xd04f,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0xd413, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xcb16,
        0xa438, 0x1000, 0xa438, 0x0a6d, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0xa640, 0xa438, 0x9503, 0xa438, 0x8720, 0xa438, 0xd17a,
        0xa438, 0xd04c, 0xa438, 0x0c1f, 0xa438, 0x0f14, 0xa438, 0xcb17,
        0xa438, 0x8fc0, 0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0xaf40,
        0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0x0cc0, 0xa438, 0x0f80,
        0xa438, 0x1000, 0xa438, 0x0a25, 0xa438, 0xafc0, 0xa438, 0x1000,
        0xa438, 0x0a25, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd701,
        0xa438, 0x61ce, 0xa438, 0xd700, 0xa438, 0x5db4, 0xa438, 0xcb18,
        0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x8640, 0xa438, 0x9503,
        0xa438, 0xa720, 0xa438, 0x1000, 0xa438, 0x0b43, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xffd6, 0xa438, 0x8f1f, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd701, 0xa438, 0x7f8e, 0xa438, 0xa131,
        0xa438, 0xaa0f, 0xa438, 0xa2d5, 0xa438, 0xa407, 0xa438, 0xa720,
        0xa438, 0x8310, 0xa438, 0xa308, 0xa438, 0x8308, 0xa438, 0xcb19,
        0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x8640, 0xa438, 0x9503,
        0xa438, 0x1000, 0xa438, 0x0b43, 0xa438, 0x1000, 0xa438, 0x0b86,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xb920, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac, 0xa438, 0x9920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f8c,
        0xa438, 0xcb1a, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5f82, 0xa438, 0x8111, 0xa438, 0x82c5, 0xa438, 0xa404,
        0xa438, 0x8402, 0xa438, 0xb804, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x7f82, 0xa438, 0x9804, 0xa438, 0xcb1b,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5f85,
        0xa438, 0xa710, 0xa438, 0xb820, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x7f65, 0xa438, 0x9820, 0xa438, 0xcb1c,
        0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d02,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d02, 0xa438, 0x1000,
        0xa438, 0x0a7d, 0xa438, 0xa110, 0xa438, 0xa284, 0xa438, 0xa404,
        0xa438, 0x8402, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fa8, 0xa438, 0xcb1d, 0xa438, 0xa180, 0xa438, 0xa402,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fa8,
        0xa438, 0xa220, 0xa438, 0xd1f5, 0xa438, 0xd049, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x3444, 0xa438, 0x8221,
        0xa438, 0xd702, 0xa438, 0x5f51, 0xa438, 0xb920, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac, 0xa438, 0x9920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f8c,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fa3,
        0xa438, 0xa504, 0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f,
        0xa438, 0x0d00, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d00,
        0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0xa00a, 0xa438, 0x8190,
        0xa438, 0x82a4, 0xa438, 0x8402, 0xa438, 0xa404, 0xa438, 0xb808,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7fa3,
        0xa438, 0x9808, 0xa438, 0xcb2b, 0xa438, 0xcb2c, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5f84, 0xa438, 0xd14a,
        0xa438, 0xd048, 0xa438, 0xa780, 0xa438, 0xcb2d, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5f94, 0xa438, 0x6208,
        0xa438, 0xd702, 0xa438, 0x5f27, 0xa438, 0x800a, 0xa438, 0xa004,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8004, 0xa438, 0xa001,
        0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001, 0xa438, 0x0c03,
        0xa438, 0x0902, 0xa438, 0xa00a, 0xa438, 0xffe9, 0xa438, 0xcb2e,
        0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d02,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d02, 0xa438, 0x1000,
        0xa438, 0x0a7d, 0xa438, 0xa190, 0xa438, 0xa284, 0xa438, 0xa406,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fa8,
        0xa438, 0xa220, 0xa438, 0xd1a0, 0xa438, 0xd040, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x3444, 0xa438, 0x827d,
        0xa438, 0xd702, 0xa438, 0x5f51, 0xa438, 0xcb2f, 0xa438, 0xa302,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd708, 0xa438, 0x5f63,
        0xa438, 0xd411, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0x8302,
        0xa438, 0xd409, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0xb920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac,
        0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x7f8c, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5fa3, 0xa438, 0x8190, 0xa438, 0x82a4, 0xa438, 0x8406,
        0xa438, 0x800a, 0xa438, 0xb808, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x7fa3, 0xa438, 0x9808, 0xa438, 0x1800,
        0xa438, 0x0433, 0xa438, 0xcb30, 0xa438, 0x8380, 0xa438, 0xcb31,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5f86,
        0xa438, 0x9308, 0xa438, 0xb204, 0xa438, 0xb301, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd701, 0xa438, 0x5fa2, 0xa438, 0xb302,
        0xa438, 0x9204, 0xa438, 0xcb32, 0xa438, 0xd408, 0xa438, 0x1000,
        0xa438, 0x0a37, 0xa438, 0xd141, 0xa438, 0xd043, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xd704,
        0xa438, 0x4ccc, 0xa438, 0xd700, 0xa438, 0x4c81, 0xa438, 0xd702,
        0xa438, 0x609e, 0xa438, 0xd1e5, 0xa438, 0xd04d, 0xa438, 0xf003,
        0xa438, 0xd1e5, 0xa438, 0xd04d, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xd700, 0xa438, 0x6083,
        0xa438, 0x0c1f, 0xa438, 0x0d01, 0xa438, 0xf003, 0xa438, 0x0c1f,
        0xa438, 0x0d01, 0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0x8710,
        0xa438, 0xa108, 0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8108,
        0xa438, 0xa203, 0xa438, 0x8120, 0xa438, 0x8a0f, 0xa438, 0xa111,
        0xa438, 0x8204, 0xa438, 0xa140, 0xa438, 0x1000, 0xa438, 0x0a42,
        0xa438, 0x8140, 0xa438, 0xd17a, 0xa438, 0xd04b, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xa204,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fa7,
        0xa438, 0xb920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x5fac, 0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x7f8c, 0xa438, 0xd404, 0xa438, 0x1000,
        0xa438, 0x0a37, 0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f,
        0xa438, 0x0d02, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d02,
        0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0xa710, 0xa438, 0x8101,
        0xa438, 0x8201, 0xa438, 0xa104, 0xa438, 0x1000, 0xa438, 0x0a42,
        0xa438, 0x8104, 0xa438, 0xa120, 0xa438, 0xaa0f, 0xa438, 0x8110,
        0xa438, 0xa284, 0xa438, 0xa404, 0xa438, 0xa00a, 0xa438, 0xd193,
        0xa438, 0xd047, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0xa110, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5fa8, 0xa438, 0xa180, 0xa438, 0xd13d,
        0xa438, 0xd04a, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0xf024, 0xa438, 0xa710, 0xa438, 0xa00a,
        0xa438, 0x8190, 0xa438, 0x8204, 0xa438, 0xa280, 0xa438, 0xa404,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fa7,
        0xa438, 0x8710, 0xa438, 0xb920, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x5fac, 0xa438, 0x9920, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f8c, 0xa438, 0x800a,
        0xa438, 0x8190, 0xa438, 0x8284, 0xa438, 0x8406, 0xa438, 0xd700,
        0xa438, 0x4121, 0xa438, 0xd701, 0xa438, 0x60f3, 0xa438, 0xd1e5,
        0xa438, 0xd04d, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0x8710, 0xa438, 0xa00a, 0xa438, 0x8190,
        0xa438, 0x8204, 0xa438, 0xa280, 0xa438, 0xa404, 0xa438, 0xb920,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x5fac,
        0xa438, 0x9920, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f,
        0xa438, 0x7f8c, 0xa438, 0xcb33, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd71f, 0xa438, 0x5f85, 0xa438, 0xa710, 0xa438, 0xb820,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd71f, 0xa438, 0x7f65,
        0xa438, 0x9820, 0xa438, 0xcb34, 0xa438, 0xa00a, 0xa438, 0xa190,
        0xa438, 0xa284, 0xa438, 0xa404, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5fa9, 0xa438, 0xd701, 0xa438, 0x6853,
        0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d00,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d00, 0xa438, 0x1000,
        0xa438, 0x0a7d, 0xa438, 0x8190, 0xa438, 0x8284, 0xa438, 0xcb35,
        0xa438, 0xd407, 0xa438, 0x1000, 0xa438, 0x0a37, 0xa438, 0x8110,
        0xa438, 0x8204, 0xa438, 0xa280, 0xa438, 0xa00a, 0xa438, 0xd704,
        0xa438, 0x4215, 0xa438, 0xa304, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5fb8, 0xa438, 0xd1c3, 0xa438, 0xd043,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0x8304, 0xa438, 0xd700, 0xa438, 0x4109, 0xa438, 0xf01e,
        0xa438, 0xcb36, 0xa438, 0xd412, 0xa438, 0x1000, 0xa438, 0x0a37,
        0xa438, 0xd700, 0xa438, 0x6309, 0xa438, 0xd702, 0xa438, 0x42c7,
        0xa438, 0x800a, 0xa438, 0x8180, 0xa438, 0x8280, 0xa438, 0x8404,
        0xa438, 0xa004, 0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8004,
        0xa438, 0xa001, 0xa438, 0x1000, 0xa438, 0x0a42, 0xa438, 0x8001,
        0xa438, 0x0c03, 0xa438, 0x0902, 0xa438, 0xa00a, 0xa438, 0xd14a,
        0xa438, 0xd048, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0xd700, 0xa438, 0x6083, 0xa438, 0x0c1f,
        0xa438, 0x0d02, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d02,
        0xa438, 0x1000, 0xa438, 0x0a7d, 0xa438, 0xcc55, 0xa438, 0xcb37,
        0xa438, 0xa00a, 0xa438, 0xa190, 0xa438, 0xa2a4, 0xa438, 0xa404,
        0xa438, 0xd700, 0xa438, 0x6041, 0xa438, 0xa402, 0xa438, 0xd13d,
        0xa438, 0xd04a, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700,
        0xa438, 0x5fa9, 0xa438, 0xd702, 0xa438, 0x5f71, 0xa438, 0xcb38,
        0xa438, 0x8224, 0xa438, 0xa288, 0xa438, 0x8180, 0xa438, 0xa110,
        0xa438, 0xa404, 0xa438, 0x800a, 0xa438, 0xd700, 0xa438, 0x6041,
        0xa438, 0x8402, 0xa438, 0xd415, 0xa438, 0x1000, 0xa438, 0x0a37,
        0xa438, 0xd13d, 0xa438, 0xd04a, 0xa438, 0x1000, 0xa438, 0x0a5e,
        0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xcb39, 0xa438, 0xa00a,
        0xa438, 0xa190, 0xa438, 0xa2a0, 0xa438, 0xa404, 0xa438, 0xd700,
        0xa438, 0x6041, 0xa438, 0xa402, 0xa438, 0xd17a, 0xa438, 0xd047,
        0xa438, 0x1000, 0xa438, 0x0a5e, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0x1800, 0xa438, 0x0560, 0xa438, 0xa111, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0xd3f5,
        0xa438, 0xd219, 0xa438, 0x1000, 0xa438, 0x0c31, 0xa438, 0xd708,
        0xa438, 0x5fa5, 0xa438, 0xa215, 0xa438, 0xd30e, 0xa438, 0xd21a,
        0xa438, 0x1000, 0xa438, 0x0c31, 0xa438, 0xd708, 0xa438, 0x63e9,
        0xa438, 0xd708, 0xa438, 0x5f65, 0xa438, 0xd708, 0xa438, 0x7f36,
        0xa438, 0xa004, 0xa438, 0x1000, 0xa438, 0x0c35, 0xa438, 0x8004,
        0xa438, 0xa001, 0xa438, 0x1000, 0xa438, 0x0c35, 0xa438, 0x8001,
        0xa438, 0xd708, 0xa438, 0x4098, 0xa438, 0xd102, 0xa438, 0x9401,
        0xa438, 0xf003, 0xa438, 0xd103, 0xa438, 0xb401, 0xa438, 0x1000,
        0xa438, 0x0c27, 0xa438, 0xa108, 0xa438, 0x1000, 0xa438, 0x0c35,
        0xa438, 0x8108, 0xa438, 0x8110, 0xa438, 0x8294, 0xa438, 0xa202,
        0xa438, 0x1800, 0xa438, 0x0bdb, 0xa438, 0xd39c, 0xa438, 0xd210,
        0xa438, 0x1000, 0xa438, 0x0c31, 0xa438, 0xd708, 0xa438, 0x5fa5,
        0xa438, 0xd39c, 0xa438, 0xd210, 0xa438, 0x1000, 0xa438, 0x0c31,
        0xa438, 0xd708, 0xa438, 0x5fa5, 0xa438, 0x1000, 0xa438, 0x0c31,
        0xa438, 0xd708, 0xa438, 0x29b5, 0xa438, 0x840e, 0xa438, 0xd708,
        0xa438, 0x5f4a, 0xa438, 0x0c1f, 0xa438, 0x1014, 0xa438, 0x1000,
        0xa438, 0x0c31, 0xa438, 0xd709, 0xa438, 0x7fa4, 0xa438, 0x901f,
        0xa438, 0x1800, 0xa438, 0x0c23, 0xa438, 0xcb43, 0xa438, 0xa508,
        0xa438, 0xd701, 0xa438, 0x3699, 0xa438, 0x844a, 0xa438, 0xa504,
        0xa438, 0xa190, 0xa438, 0xa2a0, 0xa438, 0xa404, 0xa438, 0xa00a,
        0xa438, 0xd700, 0xa438, 0x2109, 0xa438, 0x05ea, 0xa438, 0xa402,
        0xa438, 0x1800, 0xa438, 0x05ea, 0xa438, 0xcb90, 0xa438, 0x0cf0,
        0xa438, 0x0ca0, 0xa438, 0x1800, 0xa438, 0x06db, 0xa438, 0xd1ff,
        0xa438, 0xd052, 0xa438, 0xa508, 0xa438, 0x8718, 0xa438, 0xa00a,
        0xa438, 0xa190, 0xa438, 0xa2a0, 0xa438, 0xa404, 0xa438, 0x0cf0,
        0xa438, 0x0c50, 0xa438, 0x1800, 0xa438, 0x09ef, 0xa438, 0x1000,
        0xa438, 0x0a5e, 0xa438, 0xd704, 0xa438, 0x2e70, 0xa438, 0x06da,
        0xa438, 0xd700, 0xa438, 0x5f55, 0xa438, 0xa90c, 0xa438, 0x1800,
        0xa438, 0x0645, 0xa436, 0xA10E, 0xa438, 0x0644, 0xa436, 0xA10C,
        0xa438, 0x09e9, 0xa436, 0xA10A, 0xa438, 0x06da, 0xa436, 0xA108,
        0xa438, 0x05e1, 0xa436, 0xA106, 0xa438, 0x0be4, 0xa436, 0xA104,
        0xa438, 0x0435, 0xa436, 0xA102, 0xa438, 0x0141, 0xa436, 0xA100,
        0xa438, 0x026d, 0xa436, 0xA110, 0xa438, 0x00ff, 0xa436, 0xb87c,
        0xa438, 0x85fe, 0xa436, 0xb87e, 0xa438, 0xaf86, 0xa438, 0x16af,
        0xa438, 0x8699, 0xa438, 0xaf86, 0xa438, 0xe5af, 0xa438, 0x86f9,
        0xa438, 0xaf87, 0xa438, 0x7aaf, 0xa438, 0x883a, 0xa438, 0xaf88,
        0xa438, 0x58af, 0xa438, 0x8b6c, 0xa438, 0xd48b, 0xa438, 0x7c02,
        0xa438, 0x8644, 0xa438, 0x2c00, 0xa438, 0x503c, 0xa438, 0xffd6,
        0xa438, 0xac27, 0xa438, 0x18e1, 0xa438, 0x82fe, 0xa438, 0xad28,
        0xa438, 0x0cd4, 0xa438, 0x8b84, 0xa438, 0x0286, 0xa438, 0x442c,
        0xa438, 0x003c, 0xa438, 0xac27, 0xa438, 0x06ee, 0xa438, 0x8299,
        0xa438, 0x01ae, 0xa438, 0x04ee, 0xa438, 0x8299, 0xa438, 0x00af,
        0xa438, 0x23dc, 0xa438, 0xf9fa, 0xa438, 0xcefa, 0xa438, 0xfbef,
        0xa438, 0x79fb, 0xa438, 0xc4bf, 0xa438, 0x8b76, 0xa438, 0x026c,
        0xa438, 0x6dac, 0xa438, 0x2804, 0xa438, 0xd203, 0xa438, 0xae02,
        0xa438, 0xd201, 0xa438, 0xbdd8, 0xa438, 0x19d9, 0xa438, 0xef94,
        0xa438, 0x026c, 0xa438, 0x6d78, 0xa438, 0x03ef, 0xa438, 0x648a,
        0xa438, 0x0002, 0xa438, 0xbdd8, 0xa438, 0x19d9, 0xa438, 0xef94,
        0xa438, 0x026c, 0xa438, 0x6d78, 0xa438, 0x03ef, 0xa438, 0x7402,
        0xa438, 0x72cd, 0xa438, 0xac50, 0xa438, 0x02ef, 0xa438, 0x643a,
        0xa438, 0x019f, 0xa438, 0xe4ef, 0xa438, 0x4678, 0xa438, 0x03ac,
        0xa438, 0x2002, 0xa438, 0xae02, 0xa438, 0xd0ff, 0xa438, 0xffef,
        0xa438, 0x97ff, 0xa438, 0xfec6, 0xa438, 0xfefd, 0xa438, 0x041f,
        0xa438, 0x771f, 0xa438, 0x221c, 0xa438, 0x450d, 0xa438, 0x481f,
        0xa438, 0x00ac, 0xa438, 0x7f04, 0xa438, 0x1a94, 0xa438, 0xae08,
        0xa438, 0x1a94, 0xa438, 0xac7f, 0xa438, 0x03d7, 0xa438, 0x0100,
        0xa438, 0xef46, 0xa438, 0x0d48, 0xa438, 0x1f00, 0xa438, 0x1c45,
        0xa438, 0xef69, 0xa438, 0xef57, 0xa438, 0xef74, 0xa438, 0x0272,
        0xa438, 0xe8a7, 0xa438, 0xffff, 0xa438, 0x0d1a, 0xa438, 0x941b,
        0xa438, 0x979e, 0xa438, 0x072d, 0xa438, 0x0100, 0xa438, 0x1a64,
        0xa438, 0xef76, 0xa438, 0xef97, 0xa438, 0x0d98, 0xa438, 0xd400,
        0xa438, 0xff1d, 0xa438, 0x941a, 0xa438, 0x89cf, 0xa438, 0x1a75,
        0xa438, 0xaf74, 0xa438, 0xf9bf, 0xa438, 0x8b79, 0xa438, 0x026c,
        0xa438, 0x6da1, 0xa438, 0x0005, 0xa438, 0xe180, 0xa438, 0xa0ae,
        0xa438, 0x03e1, 0xa438, 0x80a1, 0xa438, 0xaf26, 0xa438, 0x9aac,
        0xa438, 0x284d, 0xa438, 0xe08f, 0xa438, 0xffef, 0xa438, 0x10c0,
        0xa438, 0xe08f, 0xa438, 0xfe10, 0xa438, 0x1b08, 0xa438, 0xa000,
        0xa438, 0x04c8, 0xa438, 0xaf40, 0xa438, 0x67c8, 0xa438, 0xbf8b,
        0xa438, 0x8c02, 0xa438, 0x6c4e, 0xa438, 0xc4bf, 0xa438, 0x8b8f,
        0xa438, 0x026c, 0xa438, 0x6def, 0xa438, 0x74e0, 0xa438, 0x830c,
        0xa438, 0xad20, 0xa438, 0x0302, 0xa438, 0x74ac, 0xa438, 0xccef,
        0xa438, 0x971b, 0xa438, 0x76ad, 0xa438, 0x5f02, 0xa438, 0xae13,
        0xa438, 0xef69, 0xa438, 0xef30, 0xa438, 0x1b32, 0xa438, 0xc4ef,
        0xa438, 0x46e4, 0xa438, 0x8ffb, 0xa438, 0xe58f, 0xa438, 0xfce7,
        0xa438, 0x8ffd, 0xa438, 0xcc10, 0xa438, 0x11ae, 0xa438, 0xb8d1,
        0xa438, 0x00a1, 0xa438, 0x1f03, 0xa438, 0xaf40, 0xa438, 0x4fbf,
        0xa438, 0x8b8c, 0xa438, 0x026c, 0xa438, 0x4ec4, 0xa438, 0xbf8b,
        0xa438, 0x8f02, 0xa438, 0x6c6d, 0xa438, 0xef74, 0xa438, 0xe083,
        0xa438, 0x0cad, 0xa438, 0x2003, 0xa438, 0x0274, 0xa438, 0xaccc,
        0xa438, 0xef97, 0xa438, 0x1b76, 0xa438, 0xad5f, 0xa438, 0x02ae,
        0xa438, 0x04ef, 0xa438, 0x69ef, 0xa438, 0x3111, 0xa438, 0xaed1,
        0xa438, 0x0287, 0xa438, 0x80af, 0xa438, 0x2293, 0xa438, 0xf8f9,
        0xa438, 0xfafb, 0xa438, 0xef59, 0xa438, 0xe080, 0xa438, 0x13ad,
        0xa438, 0x252f, 0xa438, 0xbf88, 0xa438, 0x2802, 0xa438, 0x6c6d,
        0xa438, 0xef64, 0xa438, 0x1f44, 0xa438, 0xe18f, 0xa438, 0xb91b,
        0xa438, 0x64ad, 0xa438, 0x4f1d, 0xa438, 0xd688, 0xa438, 0x2bd7,
        0xa438, 0x882e, 0xa438, 0x0274, 0xa438, 0x73ad, 0xa438, 0x5008,
        0xa438, 0xbf88, 0xa438, 0x3102, 0xa438, 0x737c, 0xa438, 0xae03,
        0xa438, 0x0287, 0xa438, 0xd0bf, 0xa438, 0x882b, 0xa438, 0x0273,
        0xa438, 0x73e0, 0xa438, 0x824c, 0xa438, 0xf621, 0xa438, 0xe482,
        0xa438, 0x4cbf, 0xa438, 0x8834, 0xa438, 0x0273, 0xa438, 0x7cef,
        0xa438, 0x95ff, 0xa438, 0xfefd, 0xa438, 0xfc04, 0xa438, 0xf8f9,
        0xa438, 0xfafb, 0xa438, 0xef79, 0xa438, 0xbf88, 0xa438, 0x1f02,
        0xa438, 0x737c, 0xa438, 0x1f22, 0xa438, 0xac32, 0xa438, 0x31ef,
        0xa438, 0x12bf, 0xa438, 0x8822, 0xa438, 0x026c, 0xa438, 0x4ed6,
        0xa438, 0x8fba, 0xa438, 0x1f33, 0xa438, 0xac3c, 0xa438, 0x1eef,
        0xa438, 0x13bf, 0xa438, 0x8837, 0xa438, 0x026c, 0xa438, 0x4eef,
        0xa438, 0x96d8, 0xa438, 0x19d9, 0xa438, 0xbf88, 0xa438, 0x2502,
        0xa438, 0x6c4e, 0xa438, 0xbf88, 0xa438, 0x2502, 0xa438, 0x6c4e,
        0xa438, 0x1616, 0xa438, 0x13ae, 0xa438, 0xdf12, 0xa438, 0xaecc,
        0xa438, 0xbf88, 0xa438, 0x1f02, 0xa438, 0x7373, 0xa438, 0xef97,
        0xa438, 0xfffe, 0xa438, 0xfdfc, 0xa438, 0x0466, 0xa438, 0xac88,
        0xa438, 0x54ac, 0xa438, 0x88f0, 0xa438, 0xac8a, 0xa438, 0x92ac,
        0xa438, 0xbadd, 0xa438, 0xac6c, 0xa438, 0xeeac, 0xa438, 0x6cff,
        0xa438, 0xad02, 0xa438, 0x99ac, 0xa438, 0x0030, 0xa438, 0xac88,
        0xa438, 0xd4c3, 0xa438, 0x5000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x00b4, 0xa438, 0xecee,
        0xa438, 0x8298, 0xa438, 0x00af, 0xa438, 0x1412, 0xa438, 0xf8bf,
        0xa438, 0x8b5d, 0xa438, 0x026c, 0xa438, 0x6d58, 0xa438, 0x03e1,
        0xa438, 0x8fb8, 0xa438, 0x2901, 0xa438, 0xe58f, 0xa438, 0xb8a0,
        0xa438, 0x0049, 0xa438, 0xef47, 0xa438, 0xe483, 0xa438, 0x02e5,
        0xa438, 0x8303, 0xa438, 0xbfc2, 0xa438, 0x5f1a, 0xa438, 0x95f7,
        0xa438, 0x05ee, 0xa438, 0xffd2, 0xa438, 0x00d8, 0xa438, 0xf605,
        0xa438, 0x1f11, 0xa438, 0xef60, 0xa438, 0xbf8b, 0xa438, 0x3002,
        0xa438, 0x6c4e, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c6d,
        0xa438, 0xf728, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c4e,
        0xa438, 0xf628, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c4e,
        0xa438, 0x0c64, 0xa438, 0xef46, 0xa438, 0xbf8b, 0xa438, 0x6002,
        0xa438, 0x6c4e, 0xa438, 0x0289, 0xa438, 0x9902, 0xa438, 0x3920,
        0xa438, 0xaf89, 0xa438, 0x96a0, 0xa438, 0x0149, 0xa438, 0xef47,
        0xa438, 0xe483, 0xa438, 0x04e5, 0xa438, 0x8305, 0xa438, 0xbfc2,
        0xa438, 0x5f1a, 0xa438, 0x95f7, 0xa438, 0x05ee, 0xa438, 0xffd2,
        0xa438, 0x00d8, 0xa438, 0xf605, 0xa438, 0x1f11, 0xa438, 0xef60,
        0xa438, 0xbf8b, 0xa438, 0x3002, 0xa438, 0x6c4e, 0xa438, 0xbf8b,
        0xa438, 0x3302, 0xa438, 0x6c6d, 0xa438, 0xf729, 0xa438, 0xbf8b,
        0xa438, 0x3302, 0xa438, 0x6c4e, 0xa438, 0xf629, 0xa438, 0xbf8b,
        0xa438, 0x3302, 0xa438, 0x6c4e, 0xa438, 0x0c64, 0xa438, 0xef46,
        0xa438, 0xbf8b, 0xa438, 0x6302, 0xa438, 0x6c4e, 0xa438, 0x0289,
        0xa438, 0x9902, 0xa438, 0x3920, 0xa438, 0xaf89, 0xa438, 0x96a0,
        0xa438, 0x0249, 0xa438, 0xef47, 0xa438, 0xe483, 0xa438, 0x06e5,
        0xa438, 0x8307, 0xa438, 0xbfc2, 0xa438, 0x5f1a, 0xa438, 0x95f7,
        0xa438, 0x05ee, 0xa438, 0xffd2, 0xa438, 0x00d8, 0xa438, 0xf605,
        0xa438, 0x1f11, 0xa438, 0xef60, 0xa438, 0xbf8b, 0xa438, 0x3002,
        0xa438, 0x6c4e, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c6d,
        0xa438, 0xf72a, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c4e,
        0xa438, 0xf62a, 0xa438, 0xbf8b, 0xa438, 0x3302, 0xa438, 0x6c4e,
        0xa438, 0x0c64, 0xa438, 0xef46, 0xa438, 0xbf8b, 0xa438, 0x6602,
        0xa438, 0x6c4e, 0xa438, 0x0289, 0xa438, 0x9902, 0xa438, 0x3920,
        0xa438, 0xaf89, 0xa438, 0x96ef, 0xa438, 0x47e4, 0xa438, 0x8308,
        0xa438, 0xe583, 0xa438, 0x09bf, 0xa438, 0xc25f, 0xa438, 0x1a95,
        0xa438, 0xf705, 0xa438, 0xeeff, 0xa438, 0xd200, 0xa438, 0xd8f6,
        0xa438, 0x051f, 0xa438, 0x11ef, 0xa438, 0x60bf, 0xa438, 0x8b30,
        0xa438, 0x026c, 0xa438, 0x4ebf, 0xa438, 0x8b33, 0xa438, 0x026c,
        0xa438, 0x6df7, 0xa438, 0x2bbf, 0xa438, 0x8b33, 0xa438, 0x026c,
        0xa438, 0x4ef6, 0xa438, 0x2bbf, 0xa438, 0x8b33, 0xa438, 0x026c,
        0xa438, 0x4e0c, 0xa438, 0x64ef, 0xa438, 0x46bf, 0xa438, 0x8b69,
        0xa438, 0x026c, 0xa438, 0x4e02, 0xa438, 0x8999, 0xa438, 0x0239,
        0xa438, 0x20af, 0xa438, 0x8996, 0xa438, 0xaf39, 0xa438, 0x1ef8,
        0xa438, 0xf9fa, 0xa438, 0xe08f, 0xa438, 0xb838, 0xa438, 0x02ad,
        0xa438, 0x2702, 0xa438, 0xae03, 0xa438, 0xaf8b, 0xa438, 0x201f,
        0xa438, 0x66ef, 0xa438, 0x65bf, 0xa438, 0xc21f, 0xa438, 0x1a96,
        0xa438, 0xf705, 0xa438, 0xeeff, 0xa438, 0xd200, 0xa438, 0xdaf6,
        0xa438, 0x05bf, 0xa438, 0xc22f, 0xa438, 0x1a96, 0xa438, 0xf705,
        0xa438, 0xeeff, 0xa438, 0xd200, 0xa438, 0xdbf6, 0xa438, 0x05ef,
        0xa438, 0x021f, 0xa438, 0x110d, 0xa438, 0x42bf, 0xa438, 0x8b3c,
        0xa438, 0x026c, 0xa438, 0x4eef, 0xa438, 0x021b, 0xa438, 0x031f,
        0xa438, 0x110d, 0xa438, 0x42bf, 0xa438, 0x8b36, 0xa438, 0x026c,
        0xa438, 0x4eef, 0xa438, 0x021a, 0xa438, 0x031f, 0xa438, 0x110d,
        0xa438, 0x42bf, 0xa438, 0x8b39, 0xa438, 0x026c, 0xa438, 0x4ebf,
        0xa438, 0xc23f, 0xa438, 0x1a96, 0xa438, 0xf705, 0xa438, 0xeeff,
        0xa438, 0xd200, 0xa438, 0xdaf6, 0xa438, 0x05bf, 0xa438, 0xc24f,
        0xa438, 0x1a96, 0xa438, 0xf705, 0xa438, 0xeeff, 0xa438, 0xd200,
        0xa438, 0xdbf6, 0xa438, 0x05ef, 0xa438, 0x021f, 0xa438, 0x110d,
        0xa438, 0x42bf, 0xa438, 0x8b45, 0xa438, 0x026c, 0xa438, 0x4eef,
        0xa438, 0x021b, 0xa438, 0x031f, 0xa438, 0x110d, 0xa438, 0x42bf,
        0xa438, 0x8b3f, 0xa438, 0x026c, 0xa438, 0x4eef, 0xa438, 0x021a,
        0xa438, 0x031f, 0xa438, 0x110d, 0xa438, 0x42bf, 0xa438, 0x8b42,
        0xa438, 0x026c, 0xa438, 0x4eef, 0xa438, 0x56d0, 0xa438, 0x201f,
        0xa438, 0x11bf, 0xa438, 0x8b4e, 0xa438, 0x026c, 0xa438, 0x4ebf,
        0xa438, 0x8b48, 0xa438, 0x026c, 0xa438, 0x4ebf, 0xa438, 0x8b4b,
        0xa438, 0x026c, 0xa438, 0x4ee1, 0xa438, 0x8578, 0xa438, 0xef03,
        0xa438, 0x480a, 0xa438, 0x2805, 0xa438, 0xef20, 0xa438, 0x1b01,
        0xa438, 0xad27, 0xa438, 0x3f1f, 0xa438, 0x44e0, 0xa438, 0x8560,
        0xa438, 0xe185, 0xa438, 0x61bf, 0xa438, 0x8b51, 0xa438, 0x026c,
        0xa438, 0x4ee0, 0xa438, 0x8566, 0xa438, 0xe185, 0xa438, 0x67bf,
        0xa438, 0x8b54, 0xa438, 0x026c, 0xa438, 0x4ee0, 0xa438, 0x856c,
        0xa438, 0xe185, 0xa438, 0x6dbf, 0xa438, 0x8b57, 0xa438, 0x026c,
        0xa438, 0x4ee0, 0xa438, 0x8572, 0xa438, 0xe185, 0xa438, 0x73bf,
        0xa438, 0x8b5a, 0xa438, 0x026c, 0xa438, 0x4ee1, 0xa438, 0x8fb8,
        0xa438, 0x5900, 0xa438, 0xf728, 0xa438, 0xe58f, 0xa438, 0xb8af,
        0xa438, 0x8b2c, 0xa438, 0xe185, 0xa438, 0x791b, 0xa438, 0x21ad,
        0xa438, 0x373e, 0xa438, 0x1f44, 0xa438, 0xe085, 0xa438, 0x62e1,
        0xa438, 0x8563, 0xa438, 0xbf8b, 0xa438, 0x5102, 0xa438, 0x6c4e,
        0xa438, 0xe085, 0xa438, 0x68e1, 0xa438, 0x8569, 0xa438, 0xbf8b,
        0xa438, 0x5402, 0xa438, 0x6c4e, 0xa438, 0xe085, 0xa438, 0x6ee1,
        0xa438, 0x856f, 0xa438, 0xbf8b, 0xa438, 0x5702, 0xa438, 0x6c4e,
        0xa438, 0xe085, 0xa438, 0x74e1, 0xa438, 0x8575, 0xa438, 0xbf8b,
        0xa438, 0x5a02, 0xa438, 0x6c4e, 0xa438, 0xe18f, 0xa438, 0xb859,
        0xa438, 0x00f7, 0xa438, 0x28e5, 0xa438, 0x8fb8, 0xa438, 0xae4a,
        0xa438, 0x1f44, 0xa438, 0xe085, 0xa438, 0x64e1, 0xa438, 0x8565,
        0xa438, 0xbf8b, 0xa438, 0x5102, 0xa438, 0x6c4e, 0xa438, 0xe085,
        0xa438, 0x6ae1, 0xa438, 0x856b, 0xa438, 0xbf8b, 0xa438, 0x5402,
        0xa438, 0x6c4e, 0xa438, 0xe085, 0xa438, 0x70e1, 0xa438, 0x8571,
        0xa438, 0xbf8b, 0xa438, 0x5702, 0xa438, 0x6c4e, 0xa438, 0xe085,
        0xa438, 0x76e1, 0xa438, 0x8577, 0xa438, 0xbf8b, 0xa438, 0x5a02,
        0xa438, 0x6c4e, 0xa438, 0xe18f, 0xa438, 0xb859, 0xa438, 0x00f7,
        0xa438, 0x28e5, 0xa438, 0x8fb8, 0xa438, 0xae0c, 0xa438, 0xe18f,
        0xa438, 0xb839, 0xa438, 0x04ac, 0xa438, 0x2f04, 0xa438, 0xee8f,
        0xa438, 0xb800, 0xa438, 0xfefd, 0xa438, 0xfc04, 0xa438, 0xf0ac,
        0xa438, 0x8efc, 0xa438, 0xac8c, 0xa438, 0xf0ac, 0xa438, 0xfaf0,
        0xa438, 0xacf8, 0xa438, 0xf0ac, 0xa438, 0xf6f0, 0xa438, 0xad00,
        0xa438, 0xf0ac, 0xa438, 0xfef0, 0xa438, 0xacfc, 0xa438, 0xf0ac,
        0xa438, 0xf4f0, 0xa438, 0xacf2, 0xa438, 0xf0ac, 0xa438, 0xf0f0,
        0xa438, 0xacb0, 0xa438, 0xf0ac, 0xa438, 0xaef0, 0xa438, 0xacac,
        0xa438, 0xf0ac, 0xa438, 0xaaf0, 0xa438, 0xacee, 0xa438, 0xf0b0,
        0xa438, 0x24f0, 0xa438, 0xb0a4, 0xa438, 0xf0b1, 0xa438, 0x24f0,
        0xa438, 0xb1a4, 0xa438, 0xee8f, 0xa438, 0xb800, 0xa438, 0xd400,
        0xa438, 0x00af, 0xa438, 0x3976, 0xa438, 0x66ac, 0xa438, 0xeabb,
        0xa438, 0xa430, 0xa438, 0x6e50, 0xa438, 0x6e53, 0xa438, 0x6e56,
        0xa438, 0x6e59, 0xa438, 0x6e5c, 0xa438, 0x6e5f, 0xa438, 0x6e62,
        0xa438, 0x6e65, 0xa438, 0xd9ac, 0xa438, 0x70f0, 0xa438, 0xac6a,
        0xa436, 0xb85e, 0xa438, 0x23b7, 0xa436, 0xb860, 0xa438, 0x74db,
        0xa436, 0xb862, 0xa438, 0x268c, 0xa436, 0xb864, 0xa438, 0x3FE5,
        0xa436, 0xb886, 0xa438, 0x2250, 0xa436, 0xb888, 0xa438, 0x140e,
        0xa436, 0xb88a, 0xa438, 0x3696, 0xa436, 0xb88c, 0xa438, 0x3973,
        0xa436, 0xb838, 0xa438, 0x00ff, 0xb820, 0x0010, 0xa436, 0x8464,
        0xa438, 0xaf84, 0xa438, 0x7caf, 0xa438, 0x8485, 0xa438, 0xaf85,
        0xa438, 0x13af, 0xa438, 0x851e, 0xa438, 0xaf85, 0xa438, 0xb9af,
        0xa438, 0x8684, 0xa438, 0xaf87, 0xa438, 0x01af, 0xa438, 0x8701,
        0xa438, 0xac38, 0xa438, 0x03af, 0xa438, 0x38bb, 0xa438, 0xaf38,
        0xa438, 0xc302, 0xa438, 0x4618, 0xa438, 0xbf85, 0xa438, 0x0a02,
        0xa438, 0x54b7, 0xa438, 0xbf85, 0xa438, 0x1002, 0xa438, 0x54c0,
        0xa438, 0xd400, 0xa438, 0x0fbf, 0xa438, 0x8507, 0xa438, 0x024f,
        0xa438, 0x48bf, 0xa438, 0x8504, 0xa438, 0x024f, 0xa438, 0x6759,
        0xa438, 0xf0a1, 0xa438, 0x3008, 0xa438, 0xbf85, 0xa438, 0x0d02,
        0xa438, 0x54c0, 0xa438, 0xae06, 0xa438, 0xbf85, 0xa438, 0x0d02,
        0xa438, 0x54b7, 0xa438, 0xbf85, 0xa438, 0x0402, 0xa438, 0x4f67,
        0xa438, 0xa183, 0xa438, 0x02ae, 0xa438, 0x15a1, 0xa438, 0x8502,
        0xa438, 0xae10, 0xa438, 0x59f0, 0xa438, 0xa180, 0xa438, 0x16bf,
        0xa438, 0x8501, 0xa438, 0x024f, 0xa438, 0x67a1, 0xa438, 0x381b,
        0xa438, 0xae0b, 0xa438, 0xe18f, 0xa438, 0xffbf, 0xa438, 0x84fe,
        0xa438, 0x024f, 0xa438, 0x48ae, 0xa438, 0x17bf, 0xa438, 0x84fe,
        0xa438, 0x0254, 0xa438, 0xb7bf, 0xa438, 0x84fb, 0xa438, 0x0254,
        0xa438, 0xb7ae, 0xa438, 0x09a1, 0xa438, 0x5006, 0xa438, 0xbf84,
        0xa438, 0xfb02, 0xa438, 0x54c0, 0xa438, 0xaf04, 0xa438, 0x4700,
        0xa438, 0xad34, 0xa438, 0xfdad, 0xa438, 0x0670, 0xa438, 0xae14,
        0xa438, 0xf0a6, 0xa438, 0x00b8, 0xa438, 0xbd32, 0xa438, 0x30bd,
        0xa438, 0x30aa, 0xa438, 0xbd2c, 0xa438, 0xccbd, 0xa438, 0x2ca1,
        0xa438, 0x0705, 0xa438, 0xec80, 0xa438, 0xaf40, 0xa438, 0xf7af,
        0xa438, 0x40f5, 0xa438, 0xd101, 0xa438, 0xbf85, 0xa438, 0xa402,
        0xa438, 0x4f48, 0xa438, 0xbf85, 0xa438, 0xa702, 0xa438, 0x54c0,
        0xa438, 0xd10f, 0xa438, 0xbf85, 0xa438, 0xaa02, 0xa438, 0x4f48,
        0xa438, 0x024d, 0xa438, 0x6abf, 0xa438, 0x85ad, 0xa438, 0x024f,
        0xa438, 0x67bf, 0xa438, 0x8ff7, 0xa438, 0xddbf, 0xa438, 0x85b0,
        0xa438, 0x024f, 0xa438, 0x67bf, 0xa438, 0x8ff8, 0xa438, 0xddbf,
        0xa438, 0x85b3, 0xa438, 0x024f, 0xa438, 0x67bf, 0xa438, 0x8ff9,
        0xa438, 0xddbf, 0xa438, 0x85b6, 0xa438, 0x024f, 0xa438, 0x67bf,
        0xa438, 0x8ffa, 0xa438, 0xddd1, 0xa438, 0x00bf, 0xa438, 0x85aa,
        0xa438, 0x024f, 0xa438, 0x4802, 0xa438, 0x4d6a, 0xa438, 0xbf85,
        0xa438, 0xad02, 0xa438, 0x4f67, 0xa438, 0xbf8f, 0xa438, 0xfbdd,
        0xa438, 0xbf85, 0xa438, 0xb002, 0xa438, 0x4f67, 0xa438, 0xbf8f,
        0xa438, 0xfcdd, 0xa438, 0xbf85, 0xa438, 0xb302, 0xa438, 0x4f67,
        0xa438, 0xbf8f, 0xa438, 0xfddd, 0xa438, 0xbf85, 0xa438, 0xb602,
        0xa438, 0x4f67, 0xa438, 0xbf8f, 0xa438, 0xfedd, 0xa438, 0xbf85,
        0xa438, 0xa702, 0xa438, 0x54b7, 0xa438, 0xbf85, 0xa438, 0xa102,
        0xa438, 0x54b7, 0xa438, 0xaf3c, 0xa438, 0x2066, 0xa438, 0xb800,
        0xa438, 0xb8bd, 0xa438, 0x30ee, 0xa438, 0xbd2c, 0xa438, 0xb8bd,
        0xa438, 0x7040, 0xa438, 0xbd86, 0xa438, 0xc8bd, 0xa438, 0x8640,
        0xa438, 0xbd88, 0xa438, 0xc8bd, 0xa438, 0x8802, 0xa438, 0x1929,
        0xa438, 0xa202, 0xa438, 0x02ae, 0xa438, 0x03a2, 0xa438, 0x032e,
        0xa438, 0xd10f, 0xa438, 0xbf85, 0xa438, 0xaa02, 0xa438, 0x4f48,
        0xa438, 0xe18f, 0xa438, 0xf7bf, 0xa438, 0x85ad, 0xa438, 0x024f,
        0xa438, 0x48e1, 0xa438, 0x8ff8, 0xa438, 0xbf85, 0xa438, 0xb002,
        0xa438, 0x4f48, 0xa438, 0xe18f, 0xa438, 0xf9bf, 0xa438, 0x85b3,
        0xa438, 0x024f, 0xa438, 0x48e1, 0xa438, 0x8ffa, 0xa438, 0xbf85,
        0xa438, 0xb602, 0xa438, 0x4f48, 0xa438, 0xae2c, 0xa438, 0xd100,
        0xa438, 0xbf85, 0xa438, 0xaa02, 0xa438, 0x4f48, 0xa438, 0xe18f,
        0xa438, 0xfbbf, 0xa438, 0x85ad, 0xa438, 0x024f, 0xa438, 0x48e1,
        0xa438, 0x8ffc, 0xa438, 0xbf85, 0xa438, 0xb002, 0xa438, 0x4f48,
        0xa438, 0xe18f, 0xa438, 0xfdbf, 0xa438, 0x85b3, 0xa438, 0x024f,
        0xa438, 0x48e1, 0xa438, 0x8ffe, 0xa438, 0xbf85, 0xa438, 0xb602,
        0xa438, 0x4f48, 0xa438, 0xbf86, 0xa438, 0x7e02, 0xa438, 0x4f67,
        0xa438, 0xa100, 0xa438, 0x02ae, 0xa438, 0x25a1, 0xa438, 0x041d,
        0xa438, 0xe18f, 0xa438, 0xf1bf, 0xa438, 0x8675, 0xa438, 0x024f,
        0xa438, 0x48e1, 0xa438, 0x8ff2, 0xa438, 0xbf86, 0xa438, 0x7802,
        0xa438, 0x4f48, 0xa438, 0xe18f, 0xa438, 0xf3bf, 0xa438, 0x867b,
        0xa438, 0x024f, 0xa438, 0x48ae, 0xa438, 0x29a1, 0xa438, 0x070b,
        0xa438, 0xae24, 0xa438, 0xbf86, 0xa438, 0x8102, 0xa438, 0x4f67,
        0xa438, 0xad28, 0xa438, 0x1be1, 0xa438, 0x8ff4, 0xa438, 0xbf86,
        0xa438, 0x7502, 0xa438, 0x4f48, 0xa438, 0xe18f, 0xa438, 0xf5bf,
        0xa438, 0x8678, 0xa438, 0x024f, 0xa438, 0x48e1, 0xa438, 0x8ff6,
        0xa438, 0xbf86, 0xa438, 0x7b02, 0xa438, 0x4f48, 0xa438, 0xaf09,
        0xa438, 0x8420, 0xa438, 0xbc32, 0xa438, 0x20bc, 0xa438, 0x3e76,
        0xa438, 0xbc08, 0xa438, 0xfda6, 0xa438, 0x1a00, 0xa438, 0xb64e,
        0xa438, 0xd101, 0xa438, 0xbf85, 0xa438, 0xa402, 0xa438, 0x4f48,
        0xa438, 0xbf85, 0xa438, 0xa702, 0xa438, 0x54c0, 0xa438, 0xd10f,
        0xa438, 0xbf85, 0xa438, 0xaa02, 0xa438, 0x4f48, 0xa438, 0x024d,
        0xa438, 0x6abf, 0xa438, 0x85ad, 0xa438, 0x024f, 0xa438, 0x67bf,
        0xa438, 0x8ff7, 0xa438, 0xddbf, 0xa438, 0x85b0, 0xa438, 0x024f,
        0xa438, 0x67bf, 0xa438, 0x8ff8, 0xa438, 0xddbf, 0xa438, 0x85b3,
        0xa438, 0x024f, 0xa438, 0x67bf, 0xa438, 0x8ff9, 0xa438, 0xddbf,
        0xa438, 0x85b6, 0xa438, 0x024f, 0xa438, 0x67bf, 0xa438, 0x8ffa,
        0xa438, 0xddd1, 0xa438, 0x00bf, 0xa438, 0x85aa, 0xa438, 0x024f,
        0xa438, 0x4802, 0xa438, 0x4d6a, 0xa438, 0xbf85, 0xa438, 0xad02,
        0xa438, 0x4f67, 0xa438, 0xbf8f, 0xa438, 0xfbdd, 0xa438, 0xbf85,
        0xa438, 0xb002, 0xa438, 0x4f67, 0xa438, 0xbf8f, 0xa438, 0xfcdd,
        0xa438, 0xbf85, 0xa438, 0xb302, 0xa438, 0x4f67, 0xa438, 0xbf8f,
        0xa438, 0xfddd, 0xa438, 0xbf85, 0xa438, 0xb602, 0xa438, 0x4f67,
        0xa438, 0xbf8f, 0xa438, 0xfedd, 0xa438, 0xbf85, 0xa438, 0xa702,
        0xa438, 0x54b7, 0xa438, 0xaf00, 0xa438, 0x8800, 0xa436, 0xb818,
        0xa438, 0x38b8, 0xa436, 0xb81a, 0xa438, 0x0444, 0xa436, 0xb81c,
        0xa438, 0x40ee, 0xa436, 0xb81e, 0xa438, 0x3C1A, 0xa436, 0xb850,
        0xa438, 0x0981, 0xa436, 0xb852, 0xa438, 0x0085, 0xa436, 0xb878,
        0xa438, 0xffff, 0xa436, 0xb884, 0xa438, 0xffff, 0xa436, 0xb832,
        0xa438, 0x003f, 0xa436, 0x0000, 0xa438, 0x0000, 0xa436, 0xB82E,
        0xa438, 0x0000, 0xa436, 0x8024, 0xa438, 0x0000, 0xb820, 0x0000,
        0xa436, 0x801E, 0xa438, 0x0021, 0xFFFF, 0xFFFF
};

static const u16 phy_mcu_ram_code_8125b_2[] = {
        0xa436, 0x8024, 0xa438, 0x3701, 0xa436, 0xB82E, 0xa438, 0x0001,
        0xb820, 0x0090, 0xa436, 0xA016, 0xa438, 0x0000, 0xa436, 0xA012,
        0xa438, 0x0000, 0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010,
        0xa438, 0x1800, 0xa438, 0x801a, 0xa438, 0x1800, 0xa438, 0x803f,
        0xa438, 0x1800, 0xa438, 0x8045, 0xa438, 0x1800, 0xa438, 0x8067,
        0xa438, 0x1800, 0xa438, 0x806d, 0xa438, 0x1800, 0xa438, 0x8071,
        0xa438, 0x1800, 0xa438, 0x80b1, 0xa438, 0xd093, 0xa438, 0xd1c4,
        0xa438, 0x1000, 0xa438, 0x135c, 0xa438, 0xd704, 0xa438, 0x5fbc,
        0xa438, 0xd504, 0xa438, 0xc9f1, 0xa438, 0x1800, 0xa438, 0x0fc9,
        0xa438, 0xbb50, 0xa438, 0xd505, 0xa438, 0xa202, 0xa438, 0xd504,
        0xa438, 0x8c0f, 0xa438, 0xd500, 0xa438, 0x1000, 0xa438, 0x1519,
        0xa438, 0x1000, 0xa438, 0x135c, 0xa438, 0xd75e, 0xa438, 0x5fae,
        0xa438, 0x9b50, 0xa438, 0x1000, 0xa438, 0x135c, 0xa438, 0xd75e,
        0xa438, 0x7fae, 0xa438, 0x1000, 0xa438, 0x135c, 0xa438, 0xd707,
        0xa438, 0x40a7, 0xa438, 0xd719, 0xa438, 0x4071, 0xa438, 0x1800,
        0xa438, 0x1557, 0xa438, 0xd719, 0xa438, 0x2f70, 0xa438, 0x803b,
        0xa438, 0x2f73, 0xa438, 0x156a, 0xa438, 0x5e70, 0xa438, 0x1800,
        0xa438, 0x155d, 0xa438, 0xd505, 0xa438, 0xa202, 0xa438, 0xd500,
        0xa438, 0xffed, 0xa438, 0xd709, 0xa438, 0x4054, 0xa438, 0xa788,
        0xa438, 0xd70b, 0xa438, 0x1800, 0xa438, 0x172a, 0xa438, 0xc0c1,
        0xa438, 0xc0c0, 0xa438, 0xd05a, 0xa438, 0xd1ba, 0xa438, 0xd701,
        0xa438, 0x2529, 0xa438, 0x022a, 0xa438, 0xd0a7, 0xa438, 0xd1b9,
        0xa438, 0xa208, 0xa438, 0x1000, 0xa438, 0x080e, 0xa438, 0xd701,
        0xa438, 0x408b, 0xa438, 0x1000, 0xa438, 0x0a65, 0xa438, 0xf003,
        0xa438, 0x1000, 0xa438, 0x0a6b, 0xa438, 0xd701, 0xa438, 0x1000,
        0xa438, 0x0920, 0xa438, 0x1000, 0xa438, 0x0915, 0xa438, 0x1000,
        0xa438, 0x0909, 0xa438, 0x228f, 0xa438, 0x804e, 0xa438, 0x9801,
        0xa438, 0xd71e, 0xa438, 0x5d61, 0xa438, 0xd701, 0xa438, 0x1800,
        0xa438, 0x022a, 0xa438, 0x2005, 0xa438, 0x091a, 0xa438, 0x3bd9,
        0xa438, 0x0919, 0xa438, 0x1800, 0xa438, 0x0916, 0xa438, 0xd090,
        0xa438, 0xd1c9, 0xa438, 0x1800, 0xa438, 0x1064, 0xa438, 0xd096,
        0xa438, 0xd1a9, 0xa438, 0xd503, 0xa438, 0xa104, 0xa438, 0x0c07,
        0xa438, 0x0902, 0xa438, 0xd500, 0xa438, 0xbc10, 0xa438, 0xd501,
        0xa438, 0xce01, 0xa438, 0xa201, 0xa438, 0x8201, 0xa438, 0xce00,
        0xa438, 0xd500, 0xa438, 0xc484, 0xa438, 0xd503, 0xa438, 0xcc02,
        0xa438, 0xcd0d, 0xa438, 0xaf01, 0xa438, 0xd500, 0xa438, 0xd703,
        0xa438, 0x4371, 0xa438, 0xbd08, 0xa438, 0x1000, 0xa438, 0x135c,
        0xa438, 0xd75e, 0xa438, 0x5fb3, 0xa438, 0xd503, 0xa438, 0xd0f5,
        0xa438, 0xd1c6, 0xa438, 0x0cf0, 0xa438, 0x0e50, 0xa438, 0xd704,
        0xa438, 0x401c, 0xa438, 0xd0f5, 0xa438, 0xd1c6, 0xa438, 0x0cf0,
        0xa438, 0x0ea0, 0xa438, 0x401c, 0xa438, 0xd07b, 0xa438, 0xd1c5,
        0xa438, 0x8ef0, 0xa438, 0x401c, 0xa438, 0x9d08, 0xa438, 0x1000,
        0xa438, 0x135c, 0xa438, 0xd75e, 0xa438, 0x7fb3, 0xa438, 0x1000,
        0xa438, 0x135c, 0xa438, 0xd75e, 0xa438, 0x5fad, 0xa438, 0x1000,
        0xa438, 0x14c5, 0xa438, 0xd703, 0xa438, 0x3181, 0xa438, 0x80af,
        0xa438, 0x60ad, 0xa438, 0x1000, 0xa438, 0x135c, 0xa438, 0xd703,
        0xa438, 0x5fba, 0xa438, 0x1800, 0xa438, 0x0cc7, 0xa438, 0xa802,
        0xa438, 0xa301, 0xa438, 0xa801, 0xa438, 0xc004, 0xa438, 0xd710,
        0xa438, 0x4000, 0xa438, 0x1800, 0xa438, 0x1e79, 0xa436, 0xA026,
        0xa438, 0x1e78, 0xa436, 0xA024, 0xa438, 0x0c93, 0xa436, 0xA022,
        0xa438, 0x1062, 0xa436, 0xA020, 0xa438, 0x0915, 0xa436, 0xA006,
        0xa438, 0x020a, 0xa436, 0xA004, 0xa438, 0x1726, 0xa436, 0xA002,
        0xa438, 0x1542, 0xa436, 0xA000, 0xa438, 0x0fc7, 0xa436, 0xA008,
        0xa438, 0xff00, 0xa436, 0xA016, 0xa438, 0x0010, 0xa436, 0xA012,
        0xa438, 0x0000, 0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010,
        0xa438, 0x1800, 0xa438, 0x801d, 0xa438, 0x1800, 0xa438, 0x802c,
        0xa438, 0x1800, 0xa438, 0x802c, 0xa438, 0x1800, 0xa438, 0x802c,
        0xa438, 0x1800, 0xa438, 0x802c, 0xa438, 0x1800, 0xa438, 0x802c,
        0xa438, 0x1800, 0xa438, 0x802c, 0xa438, 0xd700, 0xa438, 0x6090,
        0xa438, 0x60d1, 0xa438, 0xc95c, 0xa438, 0xf007, 0xa438, 0x60b1,
        0xa438, 0xc95a, 0xa438, 0xf004, 0xa438, 0xc956, 0xa438, 0xf002,
        0xa438, 0xc94e, 0xa438, 0x1800, 0xa438, 0x00cd, 0xa438, 0xd700,
        0xa438, 0x6090, 0xa438, 0x60d1, 0xa438, 0xc95c, 0xa438, 0xf007,
        0xa438, 0x60b1, 0xa438, 0xc95a, 0xa438, 0xf004, 0xa438, 0xc956,
        0xa438, 0xf002, 0xa438, 0xc94e, 0xa438, 0x1000, 0xa438, 0x022a,
        0xa438, 0x1800, 0xa438, 0x0132, 0xa436, 0xA08E, 0xa438, 0xffff,
        0xa436, 0xA08C, 0xa438, 0xffff, 0xa436, 0xA08A, 0xa438, 0xffff,
        0xa436, 0xA088, 0xa438, 0xffff, 0xa436, 0xA086, 0xa438, 0xffff,
        0xa436, 0xA084, 0xa438, 0xffff, 0xa436, 0xA082, 0xa438, 0x012f,
        0xa436, 0xA080, 0xa438, 0x00cc, 0xa436, 0xA090, 0xa438, 0x0103,
        0xa436, 0xA016, 0xa438, 0x0020, 0xa436, 0xA012, 0xa438, 0x0000,
        0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010, 0xa438, 0x1800,
        0xa438, 0x8020, 0xa438, 0x1800, 0xa438, 0x802a, 0xa438, 0x1800,
        0xa438, 0x8035, 0xa438, 0x1800, 0xa438, 0x803c, 0xa438, 0x1800,
        0xa438, 0x803c, 0xa438, 0x1800, 0xa438, 0x803c, 0xa438, 0x1800,
        0xa438, 0x803c, 0xa438, 0xd107, 0xa438, 0xd042, 0xa438, 0xa404,
        0xa438, 0x1000, 0xa438, 0x09df, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0x8280, 0xa438, 0xd700, 0xa438, 0x6065, 0xa438, 0xd125,
        0xa438, 0xf002, 0xa438, 0xd12b, 0xa438, 0xd040, 0xa438, 0x1800,
        0xa438, 0x077f, 0xa438, 0x0cf0, 0xa438, 0x0c50, 0xa438, 0xd104,
        0xa438, 0xd040, 0xa438, 0x1000, 0xa438, 0x0aa8, 0xa438, 0xd700,
        0xa438, 0x5fb4, 0xa438, 0x1800, 0xa438, 0x0a2e, 0xa438, 0xcb9b,
        0xa438, 0xd110, 0xa438, 0xd040, 0xa438, 0x1000, 0xa438, 0x0b7b,
        0xa438, 0x1000, 0xa438, 0x09df, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0x1800, 0xa438, 0x081b, 0xa438, 0x1000, 0xa438, 0x09df,
        0xa438, 0xd704, 0xa438, 0x7fb8, 0xa438, 0xa718, 0xa438, 0x1800,
        0xa438, 0x074e, 0xa436, 0xA10E, 0xa438, 0xffff, 0xa436, 0xA10C,
        0xa438, 0xffff, 0xa436, 0xA10A, 0xa438, 0xffff, 0xa436, 0xA108,
        0xa438, 0xffff, 0xa436, 0xA106, 0xa438, 0x074d, 0xa436, 0xA104,
        0xa438, 0x0818, 0xa436, 0xA102, 0xa438, 0x0a2c, 0xa436, 0xA100,
        0xa438, 0x077e, 0xa436, 0xA110, 0xa438, 0x000f, 0xa436, 0xb87c,
        0xa438, 0x8625, 0xa436, 0xb87e, 0xa438, 0xaf86, 0xa438, 0x3daf,
        0xa438, 0x8689, 0xa438, 0xaf88, 0xa438, 0x69af, 0xa438, 0x8887,
        0xa438, 0xaf88, 0xa438, 0x9caf, 0xa438, 0x889c, 0xa438, 0xaf88,
        0xa438, 0x9caf, 0xa438, 0x889c, 0xa438, 0xbf86, 0xa438, 0x49d7,
        0xa438, 0x0040, 0xa438, 0x0277, 0xa438, 0x7daf, 0xa438, 0x2727,
        0xa438, 0x0000, 0xa438, 0x7205, 0xa438, 0x0000, 0xa438, 0x7208,
        0xa438, 0x0000, 0xa438, 0x71f3, 0xa438, 0x0000, 0xa438, 0x71f6,
        0xa438, 0x0000, 0xa438, 0x7229, 0xa438, 0x0000, 0xa438, 0x722c,
        0xa438, 0x0000, 0xa438, 0x7217, 0xa438, 0x0000, 0xa438, 0x721a,
        0xa438, 0x0000, 0xa438, 0x721d, 0xa438, 0x0000, 0xa438, 0x7211,
        0xa438, 0x0000, 0xa438, 0x7220, 0xa438, 0x0000, 0xa438, 0x7214,
        0xa438, 0x0000, 0xa438, 0x722f, 0xa438, 0x0000, 0xa438, 0x7223,
        0xa438, 0x0000, 0xa438, 0x7232, 0xa438, 0x0000, 0xa438, 0x7226,
        0xa438, 0xf8f9, 0xa438, 0xfae0, 0xa438, 0x85b3, 0xa438, 0x3802,
        0xa438, 0xad27, 0xa438, 0x02ae, 0xa438, 0x03af, 0xa438, 0x8830,
        0xa438, 0x1f66, 0xa438, 0xef65, 0xa438, 0xbfc2, 0xa438, 0x1f1a,
        0xa438, 0x96f7, 0xa438, 0x05ee, 0xa438, 0xffd2, 0xa438, 0x00da,
        0xa438, 0xf605, 0xa438, 0xbfc2, 0xa438, 0x2f1a, 0xa438, 0x96f7,
        0xa438, 0x05ee, 0xa438, 0xffd2, 0xa438, 0x00db, 0xa438, 0xf605,
        0xa438, 0xef02, 0xa438, 0x1f11, 0xa438, 0x0d42, 0xa438, 0xbf88,
        0xa438, 0x4202, 0xa438, 0x6e7d, 0xa438, 0xef02, 0xa438, 0x1b03,
        0xa438, 0x1f11, 0xa438, 0x0d42, 0xa438, 0xbf88, 0xa438, 0x4502,
        0xa438, 0x6e7d, 0xa438, 0xef02, 0xa438, 0x1a03, 0xa438, 0x1f11,
        0xa438, 0x0d42, 0xa438, 0xbf88, 0xa438, 0x4802, 0xa438, 0x6e7d,
        0xa438, 0xbfc2, 0xa438, 0x3f1a, 0xa438, 0x96f7, 0xa438, 0x05ee,
        0xa438, 0xffd2, 0xa438, 0x00da, 0xa438, 0xf605, 0xa438, 0xbfc2,
        0xa438, 0x4f1a, 0xa438, 0x96f7, 0xa438, 0x05ee, 0xa438, 0xffd2,
        0xa438, 0x00db, 0xa438, 0xf605, 0xa438, 0xef02, 0xa438, 0x1f11,
        0xa438, 0x0d42, 0xa438, 0xbf88, 0xa438, 0x4b02, 0xa438, 0x6e7d,
        0xa438, 0xef02, 0xa438, 0x1b03, 0xa438, 0x1f11, 0xa438, 0x0d42,
        0xa438, 0xbf88, 0xa438, 0x4e02, 0xa438, 0x6e7d, 0xa438, 0xef02,
        0xa438, 0x1a03, 0xa438, 0x1f11, 0xa438, 0x0d42, 0xa438, 0xbf88,
        0xa438, 0x5102, 0xa438, 0x6e7d, 0xa438, 0xef56, 0xa438, 0xd020,
        0xa438, 0x1f11, 0xa438, 0xbf88, 0xa438, 0x5402, 0xa438, 0x6e7d,
        0xa438, 0xbf88, 0xa438, 0x5702, 0xa438, 0x6e7d, 0xa438, 0xbf88,
        0xa438, 0x5a02, 0xa438, 0x6e7d, 0xa438, 0xe185, 0xa438, 0xa0ef,
        0xa438, 0x0348, 0xa438, 0x0a28, 0xa438, 0x05ef, 0xa438, 0x201b,
        0xa438, 0x01ad, 0xa438, 0x2735, 0xa438, 0x1f44, 0xa438, 0xe085,
        0xa438, 0x88e1, 0xa438, 0x8589, 0xa438, 0xbf88, 0xa438, 0x5d02,
        0xa438, 0x6e7d, 0xa438, 0xe085, 0xa438, 0x8ee1, 0xa438, 0x858f,
        0xa438, 0xbf88, 0xa438, 0x6002, 0xa438, 0x6e7d, 0xa438, 0xe085,
        0xa438, 0x94e1, 0xa438, 0x8595, 0xa438, 0xbf88, 0xa438, 0x6302,
        0xa438, 0x6e7d, 0xa438, 0xe085, 0xa438, 0x9ae1, 0xa438, 0x859b,
        0xa438, 0xbf88, 0xa438, 0x6602, 0xa438, 0x6e7d, 0xa438, 0xaf88,
        0xa438, 0x3cbf, 0xa438, 0x883f, 0xa438, 0x026e, 0xa438, 0x9cad,
        0xa438, 0x2835, 0xa438, 0x1f44, 0xa438, 0xe08f, 0xa438, 0xf8e1,
        0xa438, 0x8ff9, 0xa438, 0xbf88, 0xa438, 0x5d02, 0xa438, 0x6e7d,
        0xa438, 0xe08f, 0xa438, 0xfae1, 0xa438, 0x8ffb, 0xa438, 0xbf88,
        0xa438, 0x6002, 0xa438, 0x6e7d, 0xa438, 0xe08f, 0xa438, 0xfce1,
        0xa438, 0x8ffd, 0xa438, 0xbf88, 0xa438, 0x6302, 0xa438, 0x6e7d,
        0xa438, 0xe08f, 0xa438, 0xfee1, 0xa438, 0x8fff, 0xa438, 0xbf88,
        0xa438, 0x6602, 0xa438, 0x6e7d, 0xa438, 0xaf88, 0xa438, 0x3ce1,
        0xa438, 0x85a1, 0xa438, 0x1b21, 0xa438, 0xad37, 0xa438, 0x341f,
        0xa438, 0x44e0, 0xa438, 0x858a, 0xa438, 0xe185, 0xa438, 0x8bbf,
        0xa438, 0x885d, 0xa438, 0x026e, 0xa438, 0x7de0, 0xa438, 0x8590,
        0xa438, 0xe185, 0xa438, 0x91bf, 0xa438, 0x8860, 0xa438, 0x026e,
        0xa438, 0x7de0, 0xa438, 0x8596, 0xa438, 0xe185, 0xa438, 0x97bf,
        0xa438, 0x8863, 0xa438, 0x026e, 0xa438, 0x7de0, 0xa438, 0x859c,
        0xa438, 0xe185, 0xa438, 0x9dbf, 0xa438, 0x8866, 0xa438, 0x026e,
        0xa438, 0x7dae, 0xa438, 0x401f, 0xa438, 0x44e0, 0xa438, 0x858c,
        0xa438, 0xe185, 0xa438, 0x8dbf, 0xa438, 0x885d, 0xa438, 0x026e,
        0xa438, 0x7de0, 0xa438, 0x8592, 0xa438, 0xe185, 0xa438, 0x93bf,
        0xa438, 0x8860, 0xa438, 0x026e, 0xa438, 0x7de0, 0xa438, 0x8598,
        0xa438, 0xe185, 0xa438, 0x99bf, 0xa438, 0x8863, 0xa438, 0x026e,
        0xa438, 0x7de0, 0xa438, 0x859e, 0xa438, 0xe185, 0xa438, 0x9fbf,
        0xa438, 0x8866, 0xa438, 0x026e, 0xa438, 0x7dae, 0xa438, 0x0ce1,
        0xa438, 0x85b3, 0xa438, 0x3904, 0xa438, 0xac2f, 0xa438, 0x04ee,
        0xa438, 0x85b3, 0xa438, 0x00af, 0xa438, 0x39d9, 0xa438, 0x22ac,
        0xa438, 0xeaf0, 0xa438, 0xacf6, 0xa438, 0xf0ac, 0xa438, 0xfaf0,
        0xa438, 0xacf8, 0xa438, 0xf0ac, 0xa438, 0xfcf0, 0xa438, 0xad00,
        0xa438, 0xf0ac, 0xa438, 0xfef0, 0xa438, 0xacf0, 0xa438, 0xf0ac,
        0xa438, 0xf4f0, 0xa438, 0xacf2, 0xa438, 0xf0ac, 0xa438, 0xb0f0,
        0xa438, 0xacae, 0xa438, 0xf0ac, 0xa438, 0xacf0, 0xa438, 0xacaa,
        0xa438, 0xa100, 0xa438, 0x0ce1, 0xa438, 0x8ff7, 0xa438, 0xbf88,
        0xa438, 0x8402, 0xa438, 0x6e7d, 0xa438, 0xaf26, 0xa438, 0xe9e1,
        0xa438, 0x8ff6, 0xa438, 0xbf88, 0xa438, 0x8402, 0xa438, 0x6e7d,
        0xa438, 0xaf26, 0xa438, 0xf520, 0xa438, 0xac86, 0xa438, 0xbf88,
        0xa438, 0x3f02, 0xa438, 0x6e9c, 0xa438, 0xad28, 0xa438, 0x03af,
        0xa438, 0x3324, 0xa438, 0xad38, 0xa438, 0x03af, 0xa438, 0x32e6,
        0xa438, 0xaf32, 0xa438, 0xfb00, 0xa436, 0xb87c, 0xa438, 0x8ff6,
        0xa436, 0xb87e, 0xa438, 0x0705, 0xa436, 0xb87c, 0xa438, 0x8ff8,
        0xa436, 0xb87e, 0xa438, 0x19cc, 0xa436, 0xb87c, 0xa438, 0x8ffa,
        0xa436, 0xb87e, 0xa438, 0x28e3, 0xa436, 0xb87c, 0xa438, 0x8ffc,
        0xa436, 0xb87e, 0xa438, 0x1047, 0xa436, 0xb87c, 0xa438, 0x8ffe,
        0xa436, 0xb87e, 0xa438, 0x0a45, 0xa436, 0xb85e, 0xa438, 0x271E,
        0xa436, 0xb860, 0xa438, 0x3846, 0xa436, 0xb862, 0xa438, 0x26E6,
        0xa436, 0xb864, 0xa438, 0x32E3, 0xa436, 0xb886, 0xa438, 0xffff,
        0xa436, 0xb888, 0xa438, 0xffff, 0xa436, 0xb88a, 0xa438, 0xffff,
        0xa436, 0xb88c, 0xa438, 0xffff, 0xa436, 0xb838, 0xa438, 0x000f,
        0xb820, 0x0010, 0xa436, 0x846e, 0xa438, 0xaf84, 0xa438, 0x86af,
        0xa438, 0x8690, 0xa438, 0xaf86, 0xa438, 0xa4af, 0xa438, 0x86a4,
        0xa438, 0xaf86, 0xa438, 0xa4af, 0xa438, 0x86a4, 0xa438, 0xaf86,
        0xa438, 0xa4af, 0xa438, 0x86a4, 0xa438, 0xee82, 0xa438, 0x5f00,
        0xa438, 0x0284, 0xa438, 0x90af, 0xa438, 0x0441, 0xa438, 0xf8e0,
        0xa438, 0x8ff3, 0xa438, 0xa000, 0xa438, 0x0502, 0xa438, 0x84a4,
        0xa438, 0xae06, 0xa438, 0xa001, 0xa438, 0x0302, 0xa438, 0x84c8,
        0xa438, 0xfc04, 0xa438, 0xf8f9, 0xa438, 0xef59, 0xa438, 0xe080,
        0xa438, 0x15ad, 0xa438, 0x2702, 0xa438, 0xae03, 0xa438, 0xaf84,
        0xa438, 0xc3bf, 0xa438, 0x53ca, 0xa438, 0x0252, 0xa438, 0xc8ad,
        0xa438, 0x2807, 0xa438, 0x0285, 0xa438, 0x2cee, 0xa438, 0x8ff3,
        0xa438, 0x01ef, 0xa438, 0x95fd, 0xa438, 0xfc04, 0xa438, 0xf8f9,
        0xa438, 0xfaef, 0xa438, 0x69bf, 0xa438, 0x53ca, 0xa438, 0x0252,
        0xa438, 0xc8ac, 0xa438, 0x2822, 0xa438, 0xd480, 0xa438, 0x00bf,
        0xa438, 0x8684, 0xa438, 0x0252, 0xa438, 0xa9bf, 0xa438, 0x8687,
        0xa438, 0x0252, 0xa438, 0xa9bf, 0xa438, 0x868a, 0xa438, 0x0252,
        0xa438, 0xa9bf, 0xa438, 0x868d, 0xa438, 0x0252, 0xa438, 0xa9ee,
        0xa438, 0x8ff3, 0xa438, 0x00af, 0xa438, 0x8526, 0xa438, 0xe08f,
        0xa438, 0xf4e1, 0xa438, 0x8ff5, 0xa438, 0xe28f, 0xa438, 0xf6e3,
        0xa438, 0x8ff7, 0xa438, 0x1b45, 0xa438, 0xac27, 0xa438, 0x0eee,
        0xa438, 0x8ff4, 0xa438, 0x00ee, 0xa438, 0x8ff5, 0xa438, 0x0002,
        0xa438, 0x852c, 0xa438, 0xaf85, 0xa438, 0x26e0, 0xa438, 0x8ff4,
        0xa438, 0xe18f, 0xa438, 0xf52c, 0xa438, 0x0001, 0xa438, 0xe48f,
        0xa438, 0xf4e5, 0xa438, 0x8ff5, 0xa438, 0xef96, 0xa438, 0xfefd,
        0xa438, 0xfc04, 0xa438, 0xf8f9, 0xa438, 0xef59, 0xa438, 0xbf53,
        0xa438, 0x2202, 0xa438, 0x52c8, 0xa438, 0xa18b, 0xa438, 0x02ae,
        0xa438, 0x03af, 0xa438, 0x85da, 0xa438, 0xbf57, 0xa438, 0x7202,
        0xa438, 0x52c8, 0xa438, 0xe48f, 0xa438, 0xf8e5, 0xa438, 0x8ff9,
        0xa438, 0xbf57, 0xa438, 0x7502, 0xa438, 0x52c8, 0xa438, 0xe48f,
        0xa438, 0xfae5, 0xa438, 0x8ffb, 0xa438, 0xbf57, 0xa438, 0x7802,
        0xa438, 0x52c8, 0xa438, 0xe48f, 0xa438, 0xfce5, 0xa438, 0x8ffd,
        0xa438, 0xbf57, 0xa438, 0x7b02, 0xa438, 0x52c8, 0xa438, 0xe48f,
        0xa438, 0xfee5, 0xa438, 0x8fff, 0xa438, 0xbf57, 0xa438, 0x6c02,
        0xa438, 0x52c8, 0xa438, 0xa102, 0xa438, 0x13ee, 0xa438, 0x8ffc,
        0xa438, 0x80ee, 0xa438, 0x8ffd, 0xa438, 0x00ee, 0xa438, 0x8ffe,
        0xa438, 0x80ee, 0xa438, 0x8fff, 0xa438, 0x00af, 0xa438, 0x8599,
        0xa438, 0xa101, 0xa438, 0x0cbf, 0xa438, 0x534c, 0xa438, 0x0252,
        0xa438, 0xc8a1, 0xa438, 0x0303, 0xa438, 0xaf85, 0xa438, 0x77bf,
        0xa438, 0x5322, 0xa438, 0x0252, 0xa438, 0xc8a1, 0xa438, 0x8b02,
        0xa438, 0xae03, 0xa438, 0xaf86, 0xa438, 0x64e0, 0xa438, 0x8ff8,
        0xa438, 0xe18f, 0xa438, 0xf9bf, 0xa438, 0x8684, 0xa438, 0x0252,
        0xa438, 0xa9e0, 0xa438, 0x8ffa, 0xa438, 0xe18f, 0xa438, 0xfbbf,
        0xa438, 0x8687, 0xa438, 0x0252, 0xa438, 0xa9e0, 0xa438, 0x8ffc,
        0xa438, 0xe18f, 0xa438, 0xfdbf, 0xa438, 0x868a, 0xa438, 0x0252,
        0xa438, 0xa9e0, 0xa438, 0x8ffe, 0xa438, 0xe18f, 0xa438, 0xffbf,
        0xa438, 0x868d, 0xa438, 0x0252, 0xa438, 0xa9af, 0xa438, 0x867f,
        0xa438, 0xbf53, 0xa438, 0x2202, 0xa438, 0x52c8, 0xa438, 0xa144,
        0xa438, 0x3cbf, 0xa438, 0x547b, 0xa438, 0x0252, 0xa438, 0xc8e4,
        0xa438, 0x8ff8, 0xa438, 0xe58f, 0xa438, 0xf9bf, 0xa438, 0x547e,
        0xa438, 0x0252, 0xa438, 0xc8e4, 0xa438, 0x8ffa, 0xa438, 0xe58f,
        0xa438, 0xfbbf, 0xa438, 0x5481, 0xa438, 0x0252, 0xa438, 0xc8e4,
        0xa438, 0x8ffc, 0xa438, 0xe58f, 0xa438, 0xfdbf, 0xa438, 0x5484,
        0xa438, 0x0252, 0xa438, 0xc8e4, 0xa438, 0x8ffe, 0xa438, 0xe58f,
        0xa438, 0xffbf, 0xa438, 0x5322, 0xa438, 0x0252, 0xa438, 0xc8a1,
        0xa438, 0x4448, 0xa438, 0xaf85, 0xa438, 0xa7bf, 0xa438, 0x5322,
        0xa438, 0x0252, 0xa438, 0xc8a1, 0xa438, 0x313c, 0xa438, 0xbf54,
        0xa438, 0x7b02, 0xa438, 0x52c8, 0xa438, 0xe48f, 0xa438, 0xf8e5,
        0xa438, 0x8ff9, 0xa438, 0xbf54, 0xa438, 0x7e02, 0xa438, 0x52c8,
        0xa438, 0xe48f, 0xa438, 0xfae5, 0xa438, 0x8ffb, 0xa438, 0xbf54,
        0xa438, 0x8102, 0xa438, 0x52c8, 0xa438, 0xe48f, 0xa438, 0xfce5,
        0xa438, 0x8ffd, 0xa438, 0xbf54, 0xa438, 0x8402, 0xa438, 0x52c8,
        0xa438, 0xe48f, 0xa438, 0xfee5, 0xa438, 0x8fff, 0xa438, 0xbf53,
        0xa438, 0x2202, 0xa438, 0x52c8, 0xa438, 0xa131, 0xa438, 0x03af,
        0xa438, 0x85a7, 0xa438, 0xd480, 0xa438, 0x00bf, 0xa438, 0x8684,
        0xa438, 0x0252, 0xa438, 0xa9bf, 0xa438, 0x8687, 0xa438, 0x0252,
        0xa438, 0xa9bf, 0xa438, 0x868a, 0xa438, 0x0252, 0xa438, 0xa9bf,
        0xa438, 0x868d, 0xa438, 0x0252, 0xa438, 0xa9ef, 0xa438, 0x95fd,
        0xa438, 0xfc04, 0xa438, 0xf0d1, 0xa438, 0x2af0, 0xa438, 0xd12c,
        0xa438, 0xf0d1, 0xa438, 0x44f0, 0xa438, 0xd146, 0xa438, 0xbf86,
        0xa438, 0xa102, 0xa438, 0x52c8, 0xa438, 0xbf86, 0xa438, 0xa102,
        0xa438, 0x52c8, 0xa438, 0xd101, 0xa438, 0xaf06, 0xa438, 0xa570,
        0xa438, 0xce42, 0xa436, 0xb818, 0xa438, 0x043d, 0xa436, 0xb81a,
        0xa438, 0x06a3, 0xa436, 0xb81c, 0xa438, 0xffff, 0xa436, 0xb81e,
        0xa438, 0xffff, 0xa436, 0xb850, 0xa438, 0xffff, 0xa436, 0xb852,
        0xa438, 0xffff, 0xa436, 0xb878, 0xa438, 0xffff, 0xa436, 0xb884,
        0xa438, 0xffff, 0xa436, 0xb832, 0xa438, 0x0003, 0xa436, 0x0000,
        0xa438, 0x0000, 0xa436, 0xB82E, 0xa438, 0x0000, 0xa436, 0x8024,
        0xa438, 0x0000, 0xa436, 0x801E, 0xa438, 0x0021, 0xb820, 0x0000,
        0xFFFF, 0xFFFF
};

static const u16 phy_mcu_ram_code_8126a_1_1[] = {
        0xa436, 0x8023, 0xa438, 0x4900, 0xa436, 0xB82E, 0xa438, 0x0001,
        0xBFBA, 0xE000, 0xBF1A, 0xC1B9, 0xBFA8, 0x10F0, 0xBFB0, 0x0210,
        0xBFB4, 0xE7E4, 0xb820, 0x0090, 0xa436, 0xA016, 0xa438, 0x0000,
        0xa436, 0xA012, 0xa438, 0x0000, 0xa436, 0xA014, 0xa438, 0x1800,
        0xa438, 0x8010, 0xa438, 0x1800, 0xa438, 0x8062, 0xa438, 0x1800,
        0xa438, 0x8069, 0xa438, 0x1800, 0xa438, 0x80e2, 0xa438, 0x1800,
        0xa438, 0x80eb, 0xa438, 0x1800, 0xa438, 0x80f5, 0xa438, 0x1800,
        0xa438, 0x811b, 0xa438, 0x1800, 0xa438, 0x8120, 0xa438, 0xd500,
        0xa438, 0xd049, 0xa438, 0xd1b9, 0xa438, 0xa208, 0xa438, 0x8208,
        0xa438, 0xd503, 0xa438, 0xa104, 0xa438, 0x0c07, 0xa438, 0x0902,
        0xa438, 0xd500, 0xa438, 0xbc10, 0xa438, 0xc484, 0xa438, 0xd503,
        0xa438, 0xcc02, 0xa438, 0xcd0d, 0xa438, 0xaf01, 0xa438, 0xd500,
        0xa438, 0xd703, 0xa438, 0x4531, 0xa438, 0xbd08, 0xa438, 0x1000,
        0xa438, 0x16bb, 0xa438, 0xd75e, 0xa438, 0x5fb3, 0xa438, 0xd503,
        0xa438, 0xd04d, 0xa438, 0xd1c7, 0xa438, 0x0cf0, 0xa438, 0x0e10,
        0xa438, 0xd704, 0xa438, 0x5ffc, 0xa438, 0xd04d, 0xa438, 0xd1c7,
        0xa438, 0x0cf0, 0xa438, 0x0e20, 0xa438, 0xd704, 0xa438, 0x5ffc,
        0xa438, 0xd04d, 0xa438, 0xd1c7, 0xa438, 0x0cf0, 0xa438, 0x0e40,
        0xa438, 0xd704, 0xa438, 0x5ffc, 0xa438, 0xd04d, 0xa438, 0xd1c7,
        0xa438, 0x0cf0, 0xa438, 0x0e80, 0xa438, 0xd704, 0xa438, 0x5ffc,
        0xa438, 0xd07b, 0xa438, 0xd1c5, 0xa438, 0x8ef0, 0xa438, 0xd704,
        0xa438, 0x5ffc, 0xa438, 0x9d08, 0xa438, 0x1000, 0xa438, 0x16bb,
        0xa438, 0xd75e, 0xa438, 0x7fb3, 0xa438, 0x1000, 0xa438, 0x16bb,
        0xa438, 0xd75e, 0xa438, 0x5fad, 0xa438, 0x1000, 0xa438, 0x181f,
        0xa438, 0xd703, 0xa438, 0x3181, 0xa438, 0x8059, 0xa438, 0x60ad,
        0xa438, 0x1000, 0xa438, 0x16bb, 0xa438, 0xd703, 0xa438, 0x5fbb,
        0xa438, 0x1000, 0xa438, 0x16bb, 0xa438, 0xd719, 0xa438, 0x7fa8,
        0xa438, 0xd500, 0xa438, 0xd049, 0xa438, 0xd1b9, 0xa438, 0x1800,
        0xa438, 0x0f0b, 0xa438, 0xd500, 0xa438, 0xd07b, 0xa438, 0xd1b5,
        0xa438, 0xd0f6, 0xa438, 0xd1c5, 0xa438, 0x1800, 0xa438, 0x1049,
        0xa438, 0xd707, 0xa438, 0x4121, 0xa438, 0xd706, 0xa438, 0x40fa,
        0xa438, 0xd099, 0xa438, 0xd1c6, 0xa438, 0x1000, 0xa438, 0x16bb,
        0xa438, 0xd704, 0xa438, 0x5fbc, 0xa438, 0xbc80, 0xa438, 0xc489,
        0xa438, 0xd503, 0xa438, 0xcc08, 0xa438, 0xcd46, 0xa438, 0xaf01,
        0xa438, 0xd500, 0xa438, 0x1000, 0xa438, 0x0903, 0xa438, 0x1000,
        0xa438, 0x16bb, 0xa438, 0xd75e, 0xa438, 0x5f6d, 0xa438, 0x1000,
        0xa438, 0x181f, 0xa438, 0xd504, 0xa438, 0xa210, 0xa438, 0xd500,
        0xa438, 0x1000, 0xa438, 0x16bb, 0xa438, 0xd719, 0xa438, 0x5fbc,
        0xa438, 0xd504, 0xa438, 0x8210, 0xa438, 0xd503, 0xa438, 0xc6d0,
        0xa438, 0xa521, 0xa438, 0xcd49, 0xa438, 0xaf01, 0xa438, 0xd504,
        0xa438, 0xa220, 0xa438, 0xd500, 0xa438, 0x1000, 0xa438, 0x16bb,
        0xa438, 0xd75e, 0xa438, 0x5fad, 0xa438, 0x1000, 0xa438, 0x181f,
        0xa438, 0xd503, 0xa438, 0xa704, 0xa438, 0x0c07, 0xa438, 0x0904,
        0xa438, 0xd504, 0xa438, 0xa102, 0xa438, 0xd500, 0xa438, 0x1000,
        0xa438, 0x16bb, 0xa438, 0xd718, 0xa438, 0x5fab, 0xa438, 0xd503,
        0xa438, 0xc6f0, 0xa438, 0xa521, 0xa438, 0xd505, 0xa438, 0xa404,
        0xa438, 0xd500, 0xa438, 0xd701, 0xa438, 0x6085, 0xa438, 0xd504,
        0xa438, 0xc9f1, 0xa438, 0xf003, 0xa438, 0xd504, 0xa438, 0xc9f0,
        0xa438, 0xd503, 0xa438, 0xcd4a, 0xa438, 0xaf01, 0xa438, 0xd500,
        0xa438, 0xd504, 0xa438, 0xa802, 0xa438, 0xd500, 0xa438, 0x1000,
        0xa438, 0x16bb, 0xa438, 0xd707, 0xa438, 0x5fb1, 0xa438, 0xd707,
        0xa438, 0x5f10, 0xa438, 0xd505, 0xa438, 0xa402, 0xa438, 0xd503,
        0xa438, 0xd707, 0xa438, 0x41a1, 0xa438, 0xd706, 0xa438, 0x60ba,
        0xa438, 0x60fc, 0xa438, 0x0c07, 0xa438, 0x0204, 0xa438, 0xf009,
        0xa438, 0x0c07, 0xa438, 0x0202, 0xa438, 0xf006, 0xa438, 0x0c07,
        0xa438, 0x0206, 0xa438, 0xf003, 0xa438, 0x0c07, 0xa438, 0x0202,
        0xa438, 0xd500, 0xa438, 0xd703, 0xa438, 0x3181, 0xa438, 0x80e0,
        0xa438, 0x616d, 0xa438, 0xd701, 0xa438, 0x6065, 0xa438, 0x1800,
        0xa438, 0x1229, 0xa438, 0x1000, 0xa438, 0x16bb, 0xa438, 0xd707,
        0xa438, 0x6061, 0xa438, 0xd704, 0xa438, 0x5f7c, 0xa438, 0x1800,
        0xa438, 0x124a, 0xa438, 0xd504, 0xa438, 0x8c0f, 0xa438, 0xd505,
        0xa438, 0xa20e, 0xa438, 0xd500, 0xa438, 0x1000, 0xa438, 0x1871,
        0xa438, 0x1800, 0xa438, 0x1899, 0xa438, 0xd70b, 0xa438, 0x60b0,
        0xa438, 0xd05a, 0xa438, 0xd19a, 0xa438, 0x1800, 0xa438, 0x1aef,
        0xa438, 0xd0ef, 0xa438, 0xd19a, 0xa438, 0x1800, 0xa438, 0x1aef,
        0xa438, 0x1000, 0xa438, 0x1d09, 0xa438, 0xd708, 0xa438, 0x3399,
        0xa438, 0x1b63, 0xa438, 0xd709, 0xa438, 0x5f5d, 0xa438, 0xd70b,
        0xa438, 0x6130, 0xa438, 0xd70d, 0xa438, 0x6163, 0xa438, 0xd709,
        0xa438, 0x430b, 0xa438, 0xd71e, 0xa438, 0x62c2, 0xa438, 0xb401,
        0xa438, 0xf014, 0xa438, 0xc901, 0xa438, 0x1000, 0xa438, 0x810e,
        0xa438, 0xf010, 0xa438, 0xc902, 0xa438, 0x1000, 0xa438, 0x810e,
        0xa438, 0xf00c, 0xa438, 0xce04, 0xa438, 0xcf01, 0xa438, 0xd70a,
        0xa438, 0x5fe2, 0xa438, 0xce04, 0xa438, 0xcf02, 0xa438, 0xc900,
        0xa438, 0xd70a, 0xa438, 0x4057, 0xa438, 0xb401, 0xa438, 0x0800,
        0xa438, 0x1800, 0xa438, 0x1b5d, 0xa438, 0xa480, 0xa438, 0xa2b0,
        0xa438, 0xa806, 0xa438, 0x1800, 0xa438, 0x225c, 0xa438, 0xa7e8,
        0xa438, 0xac08, 0xa438, 0x1800, 0xa438, 0x1a4e, 0xa436, 0xA026,
        0xa438, 0x1a4d, 0xa436, 0xA024, 0xa438, 0x225a, 0xa436, 0xA022,
        0xa438, 0x1b53, 0xa436, 0xA020, 0xa438, 0x1aed, 0xa436, 0xA006,
        0xa438, 0x1892, 0xa436, 0xA004, 0xa438, 0x11a4, 0xa436, 0xA002,
        0xa438, 0x103c, 0xa436, 0xA000, 0xa438, 0x0ea6, 0xa436, 0xA008,
        0xa438, 0xff00, 0xa436, 0xA016, 0xa438, 0x0000, 0xa436, 0xA012,
        0xa438, 0x0ff8, 0xa436, 0xA014, 0xa438, 0x0000, 0xa438, 0xD098,
        0xa438, 0xc483, 0xa438, 0xc483, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa436, 0xA152, 0xa438, 0x3fff,
        0xa436, 0xA154, 0xa438, 0x0413, 0xa436, 0xA156, 0xa438, 0x1A32,
        0xa436, 0xA158, 0xa438, 0x1CC0, 0xa436, 0xA15A, 0xa438, 0x3fff,
        0xa436, 0xA15C, 0xa438, 0x3fff, 0xa436, 0xA15E, 0xa438, 0x3fff,
        0xa436, 0xA160, 0xa438, 0x3fff, 0xa436, 0xA150, 0xa438, 0x000E,
        0xa436, 0xA016, 0xa438, 0x0020, 0xa436, 0xA012, 0xa438, 0x0000,
        0xa436, 0xA014, 0xa438, 0x1800, 0xa438, 0x8010, 0xa438, 0x1800,
        0xa438, 0x8021, 0xa438, 0x1800, 0xa438, 0x8037, 0xa438, 0x1800,
        0xa438, 0x803f, 0xa438, 0x1800, 0xa438, 0x8084, 0xa438, 0x1800,
        0xa438, 0x80c5, 0xa438, 0x1800, 0xa438, 0x80cc, 0xa438, 0x1800,
        0xa438, 0x80d5, 0xa438, 0xa00a, 0xa438, 0xa280, 0xa438, 0xa404,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x1800, 0xa438, 0x099b, 0xa438, 0x1000, 0xa438, 0x1021,
        0xa438, 0xd700, 0xa438, 0x5fab, 0xa438, 0xa208, 0xa438, 0x8204,
        0xa438, 0xcb38, 0xa438, 0xaa40, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x1800, 0xa438, 0x0b2a,
        0xa438, 0x82a0, 0xa438, 0x8404, 0xa438, 0xa110, 0xa438, 0xd706,
        0xa438, 0x4041, 0xa438, 0xa180, 0xa438, 0x1800, 0xa438, 0x0e7f,
        0xa438, 0x8190, 0xa438, 0xcb93, 0xa438, 0x1000, 0xa438, 0x0ef4,
        0xa438, 0xd704, 0xa438, 0x7fb8, 0xa438, 0xa008, 0xa438, 0xd706,
        0xa438, 0x4040, 0xa438, 0xa002, 0xa438, 0xd705, 0xa438, 0x4079,
        0xa438, 0x1000, 0xa438, 0x10ad, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0x85f0, 0xa438, 0x9503, 0xa438, 0xd705, 0xa438, 0x40d9,
        0xa438, 0xd70c, 0xa438, 0x6083, 0xa438, 0x0c1f, 0xa438, 0x0d09,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d0a, 0xa438, 0x0cc0,
        0xa438, 0x0d80, 0xa438, 0x1000, 0xa438, 0x104f, 0xa438, 0x1000,
        0xa438, 0x0ef4, 0xa438, 0x8020, 0xa438, 0xd705, 0xa438, 0x40d9,
        0xa438, 0xd704, 0xa438, 0x609f, 0xa438, 0xd70c, 0xa438, 0x6043,
        0xa438, 0x8504, 0xa438, 0xcb94, 0xa438, 0x1000, 0xa438, 0x0ef4,
        0xa438, 0xd706, 0xa438, 0x7fa2, 0xa438, 0x800a, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0x0cf0, 0xa438, 0x05a0, 0xa438, 0x9503,
        0xa438, 0xd705, 0xa438, 0x40b9, 0xa438, 0x0c1f, 0xa438, 0x0d00,
        0xa438, 0x8dc0, 0xa438, 0xf005, 0xa438, 0xa190, 0xa438, 0x0c1f,
        0xa438, 0x0d17, 0xa438, 0x8dc0, 0xa438, 0x1000, 0xa438, 0x104f,
        0xa438, 0xd705, 0xa438, 0x39cc, 0xa438, 0x0c7d, 0xa438, 0x1800,
        0xa438, 0x0e67, 0xa438, 0xcb96, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0xab05, 0xa438, 0xac04, 0xa438, 0xac08, 0xa438, 0x9503,
        0xa438, 0x0c1f, 0xa438, 0x0d00, 0xa438, 0x8dc0, 0xa438, 0x1000,
        0xa438, 0x104f, 0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd706,
        0xa438, 0x2215, 0xa438, 0x8099, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0xae02, 0xa438, 0x9503, 0xa438, 0xd706, 0xa438, 0x6451,
        0xa438, 0xd71f, 0xa438, 0x2e70, 0xa438, 0x0f00, 0xa438, 0xd706,
        0xa438, 0x3290, 0xa438, 0x80be, 0xa438, 0xd704, 0xa438, 0x2e70,
        0xa438, 0x8090, 0xa438, 0xd706, 0xa438, 0x339c, 0xa438, 0x8090,
        0xa438, 0x8718, 0xa438, 0x8910, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0xc500, 0xa438, 0x9503, 0xa438, 0x0c1f, 0xa438, 0x0d17,
        0xa438, 0x8dc0, 0xa438, 0x1000, 0xa438, 0x104f, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0x8c04, 0xa438, 0x9503, 0xa438, 0xa00a,
        0xa438, 0xa190, 0xa438, 0xa280, 0xa438, 0xa404, 0xa438, 0x1800,
        0xa438, 0x0f35, 0xa438, 0x1800, 0xa438, 0x0f07, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0x8c08, 0xa438, 0x8c04, 0xa438, 0x9503,
        0xa438, 0x1800, 0xa438, 0x0f02, 0xa438, 0x1000, 0xa438, 0x1021,
        0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0xaa10, 0xa438, 0x1800,
        0xa438, 0x0c6b, 0xa438, 0x82a0, 0xa438, 0x8406, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0xac04, 0xa438, 0x8602, 0xa438, 0x9503,
        0xa438, 0x1800, 0xa438, 0x0e09, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x8308, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0xc555, 0xa438, 0x9503, 0xa438, 0xa728,
        0xa438, 0x8440, 0xa438, 0x0c03, 0xa438, 0x0901, 0xa438, 0x8801,
        0xa438, 0xd700, 0xa438, 0x4040, 0xa438, 0xa801, 0xa438, 0xd701,
        0xa438, 0x4052, 0xa438, 0xa810, 0xa438, 0xd701, 0xa438, 0x4054,
        0xa438, 0xa820, 0xa438, 0xd701, 0xa438, 0x4057, 0xa438, 0xa640,
        0xa438, 0xd704, 0xa438, 0x4046, 0xa438, 0xa840, 0xa438, 0xd706,
        0xa438, 0x40b5, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0xae20,
        0xa438, 0x9503, 0xa438, 0xd401, 0xa438, 0x1000, 0xa438, 0x0fcf,
        0xa438, 0x1000, 0xa438, 0x0fda, 0xa438, 0x1000, 0xa438, 0x1008,
        0xa438, 0x1000, 0xa438, 0x0fe3, 0xa438, 0xcc00, 0xa438, 0x80c0,
        0xa438, 0x8103, 0xa438, 0x83e0, 0xa438, 0xd71e, 0xa438, 0x2318,
        0xa438, 0x01ae, 0xa438, 0xd704, 0xa438, 0x40bc, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0x8302, 0xa438, 0x9503, 0xa438, 0xb801,
        0xa438, 0xd706, 0xa438, 0x2b59, 0xa438, 0x07f8, 0xa438, 0xd700,
        0xa438, 0x2109, 0xa438, 0x04ab, 0xa438, 0xa508, 0xa438, 0xcb15,
        0xa438, 0xd70c, 0xa438, 0x430c, 0xa438, 0x1000, 0xa438, 0x10ca,
        0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0xa108, 0xa438, 0x9503,
        0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x0c1f, 0xa438, 0x0f13,
        0xa438, 0x9503, 0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd70c,
        0xa438, 0x5fb3, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x8f1f,
        0xa438, 0x9503, 0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd70c,
        0xa438, 0x7f33, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x0c0f,
        0xa438, 0x0d00, 0xa438, 0x0c70, 0xa438, 0x0b00, 0xa438, 0xab08,
        0xa438, 0x9503, 0xa438, 0xd704, 0xa438, 0x3cf1, 0xa438, 0x01f9,
        0xa438, 0x0c1f, 0xa438, 0x0d11, 0xa438, 0xf003, 0xa438, 0x0c1f,
        0xa438, 0x0d0d, 0xa438, 0x0cc0, 0xa438, 0x0d40, 0xa438, 0x1000,
        0xa438, 0x104f, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0xab80,
        0xa438, 0x9503, 0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xa940,
        0xa438, 0xd700, 0xa438, 0x5f99, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0x8b80, 0xa438, 0x9503, 0xa438, 0x8940, 0xa438, 0xd700,
        0xa438, 0x5bbf, 0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x8b08,
        0xa438, 0x9503, 0xa438, 0xba20, 0xa438, 0xd704, 0xa438, 0x4100,
        0xa438, 0xd115, 0xa438, 0xd04f, 0xa438, 0xf001, 0xa438, 0x1000,
        0xa438, 0x1021, 0xa438, 0xd700, 0xa438, 0x5fb4, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0x0c0f, 0xa438, 0x0d00, 0xa438, 0x0c70,
        0xa438, 0x0b10, 0xa438, 0xab08, 0xa438, 0x9503, 0xa438, 0xd704,
        0xa438, 0x3cf1, 0xa438, 0x8178, 0xa438, 0x0c1f, 0xa438, 0x0d11,
        0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d0d, 0xa438, 0x0cc0,
        0xa438, 0x0d40, 0xa438, 0x1000, 0xa438, 0x104f, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0xab80, 0xa438, 0x9503, 0xa438, 0x1000,
        0xa438, 0x1021, 0xa438, 0xd706, 0xa438, 0x5fad, 0xa438, 0xd407,
        0xa438, 0x1000, 0xa438, 0x0fcf, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0x8b88, 0xa438, 0x9503, 0xa438, 0x1000, 0xa438, 0x1021,
        0xa438, 0xd702, 0xa438, 0x7fa4, 0xa438, 0xd706, 0xa438, 0x61bf,
        0xa438, 0x0c03, 0xa438, 0x1502, 0xa438, 0x0c30, 0xa438, 0x0110,
        0xa438, 0xa304, 0xa438, 0x9503, 0xa438, 0xd199, 0xa438, 0xd04b,
        0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd700, 0xa438, 0x5fb4,
        0xa438, 0xd704, 0xa438, 0x3cf1, 0xa438, 0x81a5, 0xa438, 0x0c1f,
        0xa438, 0x0d02, 0xa438, 0xf003, 0xa438, 0x0c1f, 0xa438, 0x0d01,
        0xa438, 0x0cc0, 0xa438, 0x0d40, 0xa438, 0xa420, 0xa438, 0x8720,
        0xa438, 0x1000, 0xa438, 0x104f, 0xa438, 0x1000, 0xa438, 0x0fda,
        0xa438, 0xd70c, 0xa438, 0x41ac, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0x8108, 0xa438, 0x9503, 0xa438, 0x0cc0, 0xa438, 0x0040,
        0xa438, 0x0c03, 0xa438, 0x0102, 0xa438, 0x0ce0, 0xa438, 0x03e0,
        0xa438, 0xccce, 0xa438, 0xf008, 0xa438, 0x0cc0, 0xa438, 0x0040,
        0xa438, 0x0c03, 0xa438, 0x0100, 0xa438, 0x0ce0, 0xa438, 0x0380,
        0xa438, 0xcc9c, 0xa438, 0x1000, 0xa438, 0x103f, 0xa438, 0x0c03,
        0xa438, 0x1502, 0xa438, 0xa640, 0xa438, 0x9503, 0xa438, 0xcb16,
        0xa438, 0xd706, 0xa438, 0x6129, 0xa438, 0xd70c, 0xa438, 0x608c,
        0xa438, 0xd17a, 0xa438, 0xd04a, 0xa438, 0xf006, 0xa438, 0xd17a,
        0xa438, 0xd04b, 0xa438, 0xf003, 0xa438, 0xd13d, 0xa438, 0xd04b,
        0xa438, 0x0c1f, 0xa438, 0x0f14, 0xa438, 0xcb17, 0xa438, 0x8fc0,
        0xa438, 0x1000, 0xa438, 0x0fbd, 0xa438, 0xaf40, 0xa438, 0x1000,
        0xa438, 0x0fbd, 0xa438, 0x0cc0, 0xa438, 0x0f80, 0xa438, 0x1000,
        0xa438, 0x0fbd, 0xa438, 0xafc0, 0xa438, 0x1000, 0xa438, 0x0fbd,
        0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd701, 0xa438, 0x652e,
        0xa438, 0xd700, 0xa438, 0x5db4, 0xa438, 0x0c03, 0xa438, 0x1502,
        0xa438, 0x8640, 0xa438, 0xa702, 0xa438, 0x9503, 0xa438, 0xa720,
        0xa438, 0x1000, 0xa438, 0x0fda, 0xa438, 0xa108, 0xa438, 0x1000,
        0xa438, 0x0fec, 0xa438, 0x8108, 0xa438, 0x1000, 0xa438, 0x0fe3,
        0xa438, 0xa202, 0xa438, 0xa308, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x8308, 0xa438, 0xcb18,
        0xa438, 0x1000, 0xa438, 0x10c2, 0xa438, 0x1000, 0xa438, 0x1021,
        0xa438, 0xd70c, 0xa438, 0x2c60, 0xa438, 0x02bd, 0xa438, 0xff58,
        0xa438, 0x8f1f, 0xa438, 0x1000, 0xa438, 0x1021, 0xa438, 0xd701,
        0xa438, 0x7f8e, 0xa438, 0x1000, 0xa438, 0x0fe3, 0xa438, 0xa130,
        0xa438, 0xaa2f, 0xa438, 0xa2d5, 0xa438, 0xa407, 0xa438, 0xa720,
        0xa438, 0x8310, 0xa438, 0xa308, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x0000,
        0xa438, 0x0000, 0xa438, 0x0000, 0xa438, 0x8308, 0xa438, 0x1800,
        0xa438, 0x02d2, 0xa436, 0xA10E, 0xa438, 0x017f, 0xa436, 0xA10C,
        0xa438, 0x0e04, 0xa436, 0xA10A, 0xa438, 0x0c67, 0xa436, 0xA108,
        0xa438, 0x0f13, 0xa436, 0xA106, 0xa438, 0x0eb1, 0xa436, 0xA104,
        0xa438, 0x0e79, 0xa436, 0xA102, 0xa438, 0x0b23, 0xa436, 0xA100,
        0xa438, 0x0908, 0xa436, 0xA110, 0xa438, 0x00ff, 0xa436, 0xb87c,
        0xa438, 0x8ad8, 0xa436, 0xb87e, 0xa438, 0xaf8a, 0xa438, 0xf0af,
        0xa438, 0x8af9, 0xa438, 0xaf8d, 0xa438, 0xdaaf, 0xa438, 0x8e1c,
        0xa438, 0xaf8f, 0xa438, 0x03af, 0xa438, 0x8f06, 0xa438, 0xaf8f,
        0xa438, 0x06af, 0xa438, 0x8f06, 0xa438, 0x0265, 0xa438, 0xa002,
        0xa438, 0x8d78, 0xa438, 0xaf23, 0xa438, 0x47a1, 0xa438, 0x0d06,
        0xa438, 0x028b, 0xa438, 0x05af, 0xa438, 0x225a, 0xa438, 0xaf22,
        0xa438, 0x66f8, 0xa438, 0xe08a, 0xa438, 0x33a0, 0xa438, 0x0005,
        0xa438, 0x028b, 0xa438, 0x21ae, 0xa438, 0x0ea0, 0xa438, 0x0105,
        0xa438, 0x028b, 0xa438, 0xb3ae, 0xa438, 0x06a0, 0xa438, 0x0203,
        0xa438, 0x028c, 0xa438, 0x9dfc, 0xa438, 0x04f8, 0xa438, 0xfbfa,
        0xa438, 0xef69, 0xa438, 0xe080, 0xa438, 0x13ad, 0xa438, 0x267e,
        0xa438, 0xd067, 0xa438, 0xe48a, 0xa438, 0x34e4, 0xa438, 0x8a36,
        0xa438, 0xe48a, 0xa438, 0x38e4, 0xa438, 0x8a3a, 0xa438, 0xd0ae,
        0xa438, 0xe48a, 0xa438, 0x35e4, 0xa438, 0x8a37, 0xa438, 0xe48a,
        0xa438, 0x39e4, 0xa438, 0x8a3b, 0xa438, 0xd000, 0xa438, 0xe48a,
        0xa438, 0x3ce4, 0xa438, 0x8a3d, 0xa438, 0xe48a, 0xa438, 0x3ee4,
        0xa438, 0x8a3f, 0xa438, 0xe48a, 0xa438, 0x40e4, 0xa438, 0x8a41,
        0xa438, 0xe48a, 0xa438, 0x42e4, 0xa438, 0x8a43, 0xa438, 0xe48a,
        0xa438, 0x44d0, 0xa438, 0x02e4, 0xa438, 0x8a45, 0xa438, 0xd00a,
        0xa438, 0xe48a, 0xa438, 0x46d0, 0xa438, 0x16e4, 0xa438, 0x8a47,
        0xa438, 0xd01e, 0xa438, 0xe48a, 0xa438, 0x48d1, 0xa438, 0x02bf,
        0xa438, 0x8dce, 0xa438, 0x026b, 0xa438, 0xd0d1, 0xa438, 0x0abf,
        0xa438, 0x8dd1, 0xa438, 0x026b, 0xa438, 0xd0d1, 0xa438, 0x16bf,
        0xa438, 0x8dd4, 0xa438, 0x026b, 0xa438, 0xd0d1, 0xa438, 0x1ebf,
        0xa438, 0x8dd7, 0xa438, 0x026b, 0xa438, 0xd002, 0xa438, 0x73ab,
        0xa438, 0xef47, 0xa438, 0xe585, 0xa438, 0x5de4, 0xa438, 0x855c,
        0xa438, 0xee8a, 0xa438, 0x3301, 0xa438, 0xae03, 0xa438, 0x0224,
        0xa438, 0x95ef, 0xa438, 0x96fe, 0xa438, 0xfffc, 0xa438, 0x04f8,
        0xa438, 0xf9fa, 0xa438, 0xcefa, 0xa438, 0xef69, 0xa438, 0xfb02,
        0xa438, 0x8dab, 0xa438, 0xad50, 0xa438, 0x2ee1, 0xa438, 0x8a44,
        0xa438, 0xa104, 0xa438, 0x2bee, 0xa438, 0x8a33, 0xa438, 0x02e1,
        0xa438, 0x8a45, 0xa438, 0xbf8d, 0xa438, 0xce02, 0xa438, 0x6bd0,
        0xa438, 0xe18a, 0xa438, 0x46bf, 0xa438, 0x8dd1, 0xa438, 0x026b,
        0xa438, 0xd0e1, 0xa438, 0x8a47, 0xa438, 0xbf8d, 0xa438, 0xd402,
        0xa438, 0x6bd0, 0xa438, 0xe18a, 0xa438, 0x48bf, 0xa438, 0x8dd7,
        0xa438, 0x026b, 0xa438, 0xd0af, 0xa438, 0x8c94, 0xa438, 0xd200,
        0xa438, 0xbe00, 0xa438, 0x0002, 0xa438, 0x8ca5, 0xa438, 0x12a2,
        0xa438, 0x04f6, 0xa438, 0xe18a, 0xa438, 0x44a1, 0xa438, 0x0020,
        0xa438, 0xd129, 0xa438, 0xbf8d, 0xa438, 0xce02, 0xa438, 0x6bd0,
        0xa438, 0xd121, 0xa438, 0xbf8d, 0xa438, 0xd102, 0xa438, 0x6bd0,
        0xa438, 0xd125, 0xa438, 0xbf8d, 0xa438, 0xd402, 0xa438, 0x6bd0,
        0xa438, 0xbf8d, 0xa438, 0xd702, 0xa438, 0x6bd0, 0xa438, 0xae44,
        0xa438, 0xa101, 0xa438, 0x1ed1, 0xa438, 0x31bf, 0xa438, 0x8dce,
        0xa438, 0x026b, 0xa438, 0xd0bf, 0xa438, 0x8dd1, 0xa438, 0x026b,
        0xa438, 0xd0d1, 0xa438, 0x2dbf, 0xa438, 0x8dd4, 0xa438, 0x026b,
        0xa438, 0xd0bf, 0xa438, 0x8dd7, 0xa438, 0x026b, 0xa438, 0xd0ae,
        0xa438, 0x23a1, 0xa438, 0x0220, 0xa438, 0xd139, 0xa438, 0xbf8d,
        0xa438, 0xce02, 0xa438, 0x6bd0, 0xa438, 0xbf8d, 0xa438, 0xd102,
        0xa438, 0x6bd0, 0xa438, 0xd13d, 0xa438, 0xbf8d, 0xa438, 0xd402,
        0xa438, 0x6bd0, 0xa438, 0xd135, 0xa438, 0xbf8d, 0xa438, 0xd702,
        0xa438, 0x6bd0, 0xa438, 0xae00, 0xa438, 0xe18a, 0xa438, 0x4411,
        0xa438, 0xe58a, 0xa438, 0x44d0, 0xa438, 0x00e4, 0xa438, 0x8a3c,
        0xa438, 0xe48a, 0xa438, 0x3de4, 0xa438, 0x8a3e, 0xa438, 0xe48a,
        0xa438, 0x3fe4, 0xa438, 0x8a40, 0xa438, 0xe48a, 0xa438, 0x41e4,
        0xa438, 0x8a42, 0xa438, 0xe48a, 0xa438, 0x4302, 0xa438, 0x73ab,
        0xa438, 0xef47, 0xa438, 0xe585, 0xa438, 0x5de4, 0xa438, 0x855c,
        0xa438, 0xffef, 0xa438, 0x96fe, 0xa438, 0xc6fe, 0xa438, 0xfdfc,
        0xa438, 0x0402, 0xa438, 0x2495, 0xa438, 0xee8a, 0xa438, 0x3300,
        0xa438, 0x04f8, 0xa438, 0xf9fa, 0xa438, 0xfbef, 0xa438, 0x79fb,
        0xa438, 0xcffb, 0xa438, 0xd300, 0xa438, 0xa200, 0xa438, 0x09bf,
        0xa438, 0x8dc2, 0xa438, 0x026b, 0xa438, 0xefaf, 0xa438, 0x8cda,
        0xa438, 0xa201, 0xa438, 0x09bf, 0xa438, 0x8dc5, 0xa438, 0x026b,
        0xa438, 0xefaf, 0xa438, 0x8cda, 0xa438, 0xa202, 0xa438, 0x09bf,
        0xa438, 0x8dc8, 0xa438, 0x026b, 0xa438, 0xefaf, 0xa438, 0x8cda,
        0xa438, 0xbf8d, 0xa438, 0xcb02, 0xa438, 0x6bef, 0xa438, 0xef64,
        0xa438, 0xbf8a, 0xa438, 0x3c1a, 0xa438, 0x921a, 0xa438, 0x92d8,
        0xa438, 0x19d9, 0xa438, 0xef74, 0xa438, 0x0273, 0xa438, 0x93ef,
        0xa438, 0x47bf, 0xa438, 0x8a3c, 0xa438, 0x1a92, 0xa438, 0x1a92,
        0xa438, 0xdc19, 0xa438, 0xddd1, 0xa438, 0x0011, 0xa438, 0xa1ff,
        0xa438, 0xfc13, 0xa438, 0xa310, 0xa438, 0xaf02, 0xa438, 0x8d0e,
        0xa438, 0xffc7, 0xa438, 0xffef, 0xa438, 0x97ff, 0xa438, 0xfefd,
        0xa438, 0xfc04, 0xa438, 0xf8fa, 0xa438, 0xfbef, 0xa438, 0x79fb,
        0xa438, 0xcffb, 0xa438, 0xbf8a, 0xa438, 0x3c1a, 0xa438, 0x921a,
        0xa438, 0x92d8, 0xa438, 0x19d9, 0xa438, 0xef64, 0xa438, 0xbf8a,
        0xa438, 0x341a, 0xa438, 0x921a, 0xa438, 0x92d8, 0xa438, 0x19d9,
        0xa438, 0xef74, 0xa438, 0x0273, 0xa438, 0x78a2, 0xa438, 0x0005,
        0xa438, 0xbe8d, 0xa438, 0xceae, 0xa438, 0x13a2, 0xa438, 0x0105,
        0xa438, 0xbe8d, 0xa438, 0xd1ae, 0xa438, 0x0ba2, 0xa438, 0x0205,
        0xa438, 0xbe8d, 0xa438, 0xd4ae, 0xa438, 0x03be, 0xa438, 0x8dd7,
        0xa438, 0xad50, 0xa438, 0x17bf, 0xa438, 0x8a45, 0xa438, 0x1a92,
        0xa438, 0x0702, 0xa438, 0x6bef, 0xa438, 0x07dd, 0xa438, 0xef46,
        0xa438, 0xbf8a, 0xa438, 0x341a, 0xa438, 0x921a, 0xa438, 0x92dc,
        0xa438, 0x19dd, 0xa438, 0xffc7, 0xa438, 0xffef, 0xa438, 0x97ff,
        0xa438, 0xfefc, 0xa438, 0x04ee, 0xa438, 0x8a33, 0xa438, 0x00ee,
        0xa438, 0x8a32, 0xa438, 0x0404, 0xa438, 0xf8fa, 0xa438, 0xef69,
        0xa438, 0xe080, 0xa438, 0x13ad, 0xa438, 0x2624, 0xa438, 0xd102,
        0xa438, 0xbf8d, 0xa438, 0xce02, 0xa438, 0x6bd0, 0xa438, 0xd10a,
        0xa438, 0xbf8d, 0xa438, 0xd102, 0xa438, 0x6bd0, 0xa438, 0xd116,
        0xa438, 0xbf8d, 0xa438, 0xd402, 0xa438, 0x6bd0, 0xa438, 0xd11e,
        0xa438, 0xbf8d, 0xa438, 0xd702, 0xa438, 0x6bd0, 0xa438, 0xee8a,
        0xa438, 0x3300, 0xa438, 0xef96, 0xa438, 0xfefc, 0xa438, 0x04f8,
        0xa438, 0xfae0, 0xa438, 0x855c, 0xa438, 0xe185, 0xa438, 0x5def,
        0xa438, 0x64d0, 0xa438, 0x00e1, 0xa438, 0x8a32, 0xa438, 0xef74,
        0xa438, 0x0273, 0xa438, 0xc6fe, 0xa438, 0xfc04, 0xa438, 0xf0b2,
        0xa438, 0x02f0, 0xa438, 0xb282, 0xa438, 0xf0b3, 0xa438, 0x02f0,
        0xa438, 0xb382, 0xa438, 0x50ac, 0xa438, 0xd450, 0xa438, 0xacd6,
        0xa438, 0xb6ac, 0xa438, 0xd4b6, 0xa438, 0xacd6, 0xa438, 0xbf8e,
        0xa438, 0x0d02, 0xa438, 0x6bd0, 0xa438, 0xd0ff, 0xa438, 0xd1fe,
        0xa438, 0xbf8e, 0xa438, 0x1002, 0xa438, 0x6bd0, 0xa438, 0xd004,
        0xa438, 0xd14d, 0xa438, 0xbf8e, 0xa438, 0x1302, 0xa438, 0x6bd0,
        0xa438, 0xd0fc, 0xa438, 0xd1c6, 0xa438, 0xbf8e, 0xa438, 0x1602,
        0xa438, 0x6bd0, 0xa438, 0xd009, 0xa438, 0xd146, 0xa438, 0xbf8e,
        0xa438, 0x1902, 0xa438, 0x6bd0, 0xa438, 0xef13, 0xa438, 0xaf2d,
        0xa438, 0xbdf0, 0xa438, 0xac1c, 0xa438, 0xf0ac, 0xa438, 0x2af0,
        0xa438, 0xac2c, 0xa438, 0xf0ac, 0xa438, 0x2ef0, 0xa438, 0xac30,
        0xa438, 0xbf8e, 0xa438, 0xf102, 0xa438, 0x6bef, 0xa438, 0xac28,
        0xa438, 0x70bf, 0xa438, 0x8eeb, 0xa438, 0x026b, 0xa438, 0xefac,
        0xa438, 0x2867, 0xa438, 0xbf8e, 0xa438, 0xee02, 0xa438, 0x6bef,
        0xa438, 0xad28, 0xa438, 0x5bbf, 0xa438, 0x8ff2, 0xa438, 0xd8bf,
        0xa438, 0x8ff3, 0xa438, 0xd9bf, 0xa438, 0x8ef4, 0xa438, 0x026b,
        0xa438, 0xd0bf, 0xa438, 0x8ff0, 0xa438, 0xd8bf, 0xa438, 0x8ff1,
        0xa438, 0xd9bf, 0xa438, 0x8ef7, 0xa438, 0x026b, 0xa438, 0xd0bf,
        0xa438, 0x8fee, 0xa438, 0xd8bf, 0xa438, 0x8fef, 0xa438, 0xd9bf,
        0xa438, 0x8efa, 0xa438, 0x026b, 0xa438, 0xd0bf, 0xa438, 0x8fec,
        0xa438, 0xd8bf, 0xa438, 0x8fed, 0xa438, 0xd9bf, 0xa438, 0x8efd,
        0xa438, 0x026b, 0xa438, 0xd0bf, 0xa438, 0x8fea, 0xa438, 0xd8bf,
        0xa438, 0x8feb, 0xa438, 0xd9bf, 0xa438, 0x8f00, 0xa438, 0x026b,
        0xa438, 0xd0bf, 0xa438, 0x8fe8, 0xa438, 0xd8bf, 0xa438, 0x8fe9,
        0xa438, 0xd9bf, 0xa438, 0x8e0d, 0xa438, 0x026b, 0xa438, 0xd01f,
        0xa438, 0x00e1, 0xa438, 0x86ee, 0xa438, 0x1b64, 0xa438, 0xaf3d,
        0xa438, 0x7abf, 0xa438, 0x8ffe, 0xa438, 0xd8bf, 0xa438, 0x8fff,
        0xa438, 0xd9bf, 0xa438, 0x8ef4, 0xa438, 0x026b, 0xa438, 0xd0bf,
        0xa438, 0x8ffc, 0xa438, 0xd8bf, 0xa438, 0x8ffd, 0xa438, 0xd9bf,
        0xa438, 0x8ef7, 0xa438, 0x026b, 0xa438, 0xd0bf, 0xa438, 0x8ffa,
        0xa438, 0xd8bf, 0xa438, 0x8ffb, 0xa438, 0xd9bf, 0xa438, 0x8efa,
        0xa438, 0x026b, 0xa438, 0xd0bf, 0xa438, 0x8ff8, 0xa438, 0xd8bf,
        0xa438, 0x8ff9, 0xa438, 0xd9bf, 0xa438, 0x8efd, 0xa438, 0x026b,
        0xa438, 0xd0bf, 0xa438, 0x8ff6, 0xa438, 0xd8bf, 0xa438, 0x8ff7,
        0xa438, 0xd9bf, 0xa438, 0x8f00, 0xa438, 0x026b, 0xa438, 0xd0bf,
        0xa438, 0x8ff4, 0xa438, 0xd8bf, 0xa438, 0x8ff5, 0xa438, 0xd9bf,
        0xa438, 0x8e0d, 0xa438, 0x026b, 0xa438, 0xd0ae, 0xa438, 0xa766,
        0xa438, 0xac5c, 0xa438, 0xbbac, 0xa438, 0x5c99, 0xa438, 0xac5c,
        0xa438, 0xf0ac, 0xa438, 0x26f0, 0xa438, 0xac24, 0xa438, 0xf0ac,
        0xa438, 0x22f0, 0xa438, 0xac20, 0xa438, 0xf0ac, 0xa438, 0x1eaf,
        0xa438, 0x44f8, 0xa436, 0xb85e, 0xa438, 0x2344, 0xa436, 0xb860,
        0xa438, 0x2254, 0xa436, 0xb862, 0xa438, 0x2DB5, 0xa436, 0xb864,
        0xa438, 0x3D6C, 0xa436, 0xb886, 0xa438, 0x44ED, 0xa436, 0xb888,
        0xa438, 0xffff, 0xa436, 0xb88a, 0xa438, 0xffff, 0xa436, 0xb88c,
        0xa438, 0xffff, 0xa436, 0xb838, 0xa438, 0x001f, 0xb820, 0x0010,
        0xa436, 0x87ad, 0xa438, 0xaf87, 0xa438, 0xc5af, 0xa438, 0x87e4,
        0xa438, 0xaf8a, 0xa438, 0x3daf, 0xa438, 0x8a62, 0xa438, 0xaf8a,
        0xa438, 0x62af, 0xa438, 0x8a62, 0xa438, 0xaf8a, 0xa438, 0x62af,
        0xa438, 0x8a62, 0xa438, 0x2810, 0xa438, 0x0d01, 0xa438, 0xe484,
        0xa438, 0xbf29, 0xa438, 0x100d, 0xa438, 0x11e5, 0xa438, 0x84c0,
        0xa438, 0x2a10, 0xa438, 0x0d21, 0xa438, 0xe684, 0xa438, 0xc12b,
        0xa438, 0x100d, 0xa438, 0x31e7, 0xa438, 0x84c2, 0xa438, 0xaf3f,
        0xa438, 0x7cf8, 0xa438, 0xe080, 0xa438, 0x4cac, 0xa438, 0x222c,
        0xa438, 0xe080, 0xa438, 0x40ad, 0xa438, 0x2232, 0xa438, 0xbf8a,
        0xa438, 0x2502, 0xa438, 0x6752, 0xa438, 0xad29, 0xa438, 0x0502,
        0xa438, 0x8827, 0xa438, 0xae0d, 0xa438, 0xad28, 0xa438, 0x0502,
        0xa438, 0x8961, 0xa438, 0xae05, 0xa438, 0x0214, 0xa438, 0x04ae,
        0xa438, 0x00e0, 0xa438, 0x8040, 0xa438, 0xac22, 0xa438, 0x1102,
        0xa438, 0x13e1, 0xa438, 0xae0c, 0xa438, 0x0288, 0xa438, 0x7c02,
        0xa438, 0x8a10, 0xa438, 0x0214, 0xa438, 0x2502, 0xa438, 0x1404,
        0xa438, 0xfcaf, 0xa438, 0x13c6, 0xa438, 0xf8f8, 0xa438, 0xccf9,
        0xa438, 0xfaef, 0xa438, 0x69fb, 0xa438, 0xe080, 0xa438, 0x18ad,
        0xa438, 0x223b, 0xa438, 0xbf8a, 0xa438, 0x2b02, 0xa438, 0x6752,
        0xa438, 0xad28, 0xa438, 0x32bf, 0xa438, 0x8a28, 0xa438, 0x026f,
        0xa438, 0x17ee, 0xa438, 0x8ff3, 0xa438, 0x00bf, 0xa438, 0x6854,
        0xa438, 0x0267, 0xa438, 0x52ad, 0xa438, 0x281f, 0xa438, 0xbf68,
        0xa438, 0x5d02, 0xa438, 0x6752, 0xa438, 0xad28, 0xa438, 0x16e0,
        0xa438, 0x8ff4, 0xa438, 0xe18f, 0xa438, 0xf502, 0xa438, 0x8891,
        0xa438, 0xad50, 0xa438, 0x0abf, 0xa438, 0x8a28, 0xa438, 0x026f,
        0xa438, 0x20ee, 0xa438, 0x8ff3, 0xa438, 0x0102, 0xa438, 0x1404,
        0xa438, 0xffef, 0xa438, 0x96fe, 0xa438, 0xfdc4, 0xa438, 0xfcfc,
        0xa438, 0x04f8, 0xa438, 0xf9ef, 0xa438, 0x59e0, 0xa438, 0x8018,
        0xa438, 0xad22, 0xa438, 0x06bf, 0xa438, 0x8a28, 0xa438, 0x026f,
        0xa438, 0x17ef, 0xa438, 0x95fd, 0xa438, 0xfc04, 0xa438, 0xf8f9,
        0xa438, 0xf9ef, 0xa438, 0x59fa, 0xa438, 0xface, 0xa438, 0xe48f,
        0xa438, 0xfee5, 0xa438, 0x8fff, 0xa438, 0xbf6e, 0xa438, 0x1b02,
        0xa438, 0x6f20, 0xa438, 0xbf6e, 0xa438, 0x1802, 0xa438, 0x6f17,
        0xa438, 0xd102, 0xa438, 0xbf6e, 0xa438, 0x1202, 0xa438, 0x6733,
        0xa438, 0xbf6e, 0xa438, 0x1502, 0xa438, 0x6f17, 0xa438, 0xbe00,
        0xa438, 0x00cc, 0xa438, 0xbf69, 0xa438, 0xcb02, 0xa438, 0x6733,
        0xa438, 0xbf69, 0xa438, 0xce02, 0xa438, 0x6f17, 0xa438, 0xbf69,
        0xa438, 0xce02, 0xa438, 0x6f20, 0xa438, 0xbf69, 0xa438, 0xd102,
        0xa438, 0x6752, 0xa438, 0xad28, 0xa438, 0xf70c, 0xa438, 0x81bf,
        0xa438, 0x8ff6, 0xa438, 0x1a98, 0xa438, 0xef59, 0xa438, 0xbf69,
        0xa438, 0xd402, 0xa438, 0x6752, 0xa438, 0xef95, 0xa438, 0xdc19,
        0xa438, 0xdd0d, 0xa438, 0x8118, 0xa438, 0xa800, 0xa438, 0x04c9,
        0xa438, 0xbf69, 0xa438, 0xce02, 0xa438, 0x6f17, 0xa438, 0xe08f,
        0xa438, 0xfce1, 0xa438, 0x8ffd, 0xa438, 0xef74, 0xa438, 0xe08f,
        0xa438, 0xfae1, 0xa438, 0x8ffb, 0xa438, 0xef64, 0xa438, 0x026e,
        0xa438, 0x57ad, 0xa438, 0x5008, 0xa438, 0xe08f, 0xa438, 0xfce1,
        0xa438, 0x8ffd, 0xa438, 0xae06, 0xa438, 0xe08f, 0xa438, 0xfae1,
        0xa438, 0x8ffb, 0xa438, 0xe28f, 0xa438, 0xf8e3, 0xa438, 0x8ff9,
        0xa438, 0xef75, 0xa438, 0xe28f, 0xa438, 0xf6e3, 0xa438, 0x8ff7,
        0xa438, 0xef65, 0xa438, 0x026e, 0xa438, 0x57ad, 0xa438, 0x5008,
        0xa438, 0xe28f, 0xa438, 0xf8e3, 0xa438, 0x8ff9, 0xa438, 0xae06,
        0xa438, 0xe28f, 0xa438, 0xf6e3, 0xa438, 0x8ff7, 0xa438, 0x1b45,
        0xa438, 0xad27, 0xa438, 0x05d7, 0xa438, 0x0000, 0xa438, 0xae0d,
        0xa438, 0xef74, 0xa438, 0xe08f, 0xa438, 0xfee1, 0xa438, 0x8fff,
        0xa438, 0xef64, 0xa438, 0x026e, 0xa438, 0x57c6, 0xa438, 0xfefe,
        0xa438, 0xef95, 0xa438, 0xfdfd, 0xa438, 0xfc04, 0xa438, 0xf8f9,
        0xa438, 0xfaef, 0xa438, 0x69fb, 0xa438, 0xe080, 0xa438, 0x18ac,
        0xa438, 0x2103, 0xa438, 0xaf8a, 0xa438, 0x06bf, 0xa438, 0x8a2b,
        0xa438, 0xac21, 0xa438, 0x03af, 0xa438, 0x8a06, 0xa438, 0xbf8a,
        0xa438, 0x2802, 0xa438, 0x6f17, 0xa438, 0xee8f, 0xa438, 0xee00,
        0xa438, 0xee8f, 0xa438, 0xed00, 0xa438, 0xbf8a, 0xa438, 0x2e02,
        0xa438, 0x6752, 0xa438, 0xad28, 0xa438, 0x03af, 0xa438, 0x8a06,
        0xa438, 0xe28f, 0xa438, 0xefe3, 0xa438, 0x8ff0, 0xa438, 0xbf68,
        0xa438, 0x5102, 0xa438, 0x6752, 0xa438, 0xac28, 0xa438, 0x11e2,
        0xa438, 0x8ff1, 0xa438, 0xe38f, 0xa438, 0xf2bf, 0xa438, 0x6848,
        0xa438, 0x0267, 0xa438, 0x52ac, 0xa438, 0x2802, 0xa438, 0xae53,
        0xa438, 0xbf68, 0xa438, 0x5a02, 0xa438, 0x6752, 0xa438, 0xad28,
        0xa438, 0x0aef, 0xa438, 0x4502, 0xa438, 0x8891, 0xa438, 0xac50,
        0xa438, 0x38ae, 0xa438, 0x40bf, 0xa438, 0x8a31, 0xa438, 0x0267,
        0xa438, 0x52ef, 0xa438, 0x31bf, 0xa438, 0x8a34, 0xa438, 0x0267,
        0xa438, 0x520c, 0xa438, 0x311e, 0xa438, 0x31bf, 0xa438, 0x8a37,
        0xa438, 0x0267, 0xa438, 0x520c, 0xa438, 0x311e, 0xa438, 0x31bf,
        0xa438, 0x8a3a, 0xa438, 0x0267, 0xa438, 0x520c, 0xa438, 0x311e,
        0xa438, 0x31e7, 0xa438, 0x8fee, 0xa438, 0xa30c, 0xa438, 0x02ae,
        0xa438, 0x08a3, 0xa438, 0x0e02, 0xa438, 0xae03, 0xa438, 0xa30d,
        0xa438, 0x0aee, 0xa438, 0x8fed, 0xa438, 0x01bf, 0xa438, 0x8a28,
        0xa438, 0x026f, 0xa438, 0x2002, 0xa438, 0x1404, 0xa438, 0xffef,
        0xa438, 0x96fe, 0xa438, 0xfdfc, 0xa438, 0x04f8, 0xa438, 0xfaef,
        0xa438, 0x69e0, 0xa438, 0x8018, 0xa438, 0xad21, 0xa438, 0x06bf,
        0xa438, 0x8a28, 0xa438, 0x026f, 0xa438, 0x17ef, 0xa438, 0x96fe,
        0xa438, 0xfc04, 0xa438, 0xf8a4, 0xa438, 0xb677, 0xa438, 0xa4b6,
        0xa438, 0x22a4, 0xa438, 0x4222, 0xa438, 0xa668, 0xa438, 0x00b2,
        0xa438, 0x3e00, 0xa438, 0xb2be, 0xa438, 0x00b3, 0xa438, 0x3e00,
        0xa438, 0xb3be, 0xa438, 0xd10f, 0xa438, 0xbf8a, 0xa438, 0x5c02,
        0xa438, 0x6733, 0xa438, 0xbf8a, 0xa438, 0x5f02, 0xa438, 0x6733,
        0xa438, 0xbf8a, 0xa438, 0x5c02, 0xa438, 0x6f17, 0xa438, 0xbf8a,
        0xa438, 0x5f02, 0xa438, 0x6f17, 0xa438, 0x1f00, 0xa438, 0xaf3d,
        0xa438, 0x0c30, 0xa438, 0xa85a, 0xa438, 0xfcad, 0xa438, 0x0e00,
        0xa436, 0xb818, 0xa438, 0x3f31, 0xa436, 0xb81a, 0xa438, 0x13a4,
        0xa436, 0xb81c, 0xa438, 0x3d0a, 0xa436, 0xb81e, 0xa438, 0xffff,
        0xa436, 0xb850, 0xa438, 0xffff, 0xa436, 0xb852, 0xa438, 0xffff,
        0xa436, 0xb878, 0xa438, 0xffff, 0xa436, 0xb884, 0xa438, 0xffff,
        0xa436, 0xb832, 0xa438, 0x0007, 0xa436, 0x84cf, 0xa438, 0x0101,
        0xa466, 0x0002, 0xa436, 0x86a7, 0xa438, 0x0000, 0xa436, 0x0000,
        0xa438, 0x0000, 0xa436, 0xB82E, 0xa438, 0x0000, 0xa436, 0x8023,
        0xa438, 0x0000, 0xa436, 0x801E, 0xa438, 0x0023, 0xb820, 0x0000,
        0xFFFF, 0xFFFF
};

static const u16 phy_mcu_ram_code_8126a_1_2[] = {
        0xB87C, 0x8a32, 0xB87E, 0x0400, 0xB87C, 0x8376, 0xB87E, 0x0300,
        0xce00, 0x6CAF, 0xB87C, 0x8301, 0xB87E, 0x1133, 0xB87C, 0x8105,
        0xB87E, 0xa000, 0xB87C, 0x8148, 0xB87E, 0xa000, 0xa436, 0x81d8,
        0xa438, 0x5865, 0xacf8, 0xCCC0, 0xac90, 0x52B0, 0xad2C, 0x8000,
        0xB87C, 0x83e6, 0xB87E, 0x4A0E, 0xB87C, 0x83d2, 0xB87E, 0x0A0E,
        0xB87C, 0x80a0, 0xB87E, 0xB8B6, 0xB87C, 0x805e, 0xB87E, 0xB8B6,
        0xB87C, 0x8057, 0xB87E, 0x305A, 0xB87C, 0x8099, 0xB87E, 0x305A,
        0xB87C, 0x8052, 0xB87E, 0x3333, 0xB87C, 0x8094, 0xB87E, 0x3333,
        0xB87C, 0x807F, 0xB87E, 0x7975, 0xB87C, 0x803D, 0xB87E, 0x7975,
        0xB87C, 0x8036, 0xB87E, 0x305A, 0xB87C, 0x8078, 0xB87E, 0x305A,
        0xB87C, 0x8031, 0xB87E, 0x3335, 0xB87C, 0x8073, 0xB87E, 0x3335,
        0xa436, 0x81D8, 0xa438, 0x5865, 0xB87C, 0x867c, 0xB87E, 0x0617,
        0xad94, 0x0092, 0xB87C, 0x89B1, 0xB87E, 0x5050, 0xB87C, 0x86E0,
        0xB87E, 0x809A, 0xB87C, 0x86E2, 0xB87E, 0xB34D, 0xB87C, 0x8FD2,
        0xB87E, 0x004B, 0xB87C, 0x8691, 0xB87E, 0x007D, 0xB87E, 0x00AF,
        0xB87E, 0x00E1, 0xB87E, 0x00FF, 0xB87C, 0x867F, 0xB87E, 0x0201,
        0xB87E, 0x0201, 0xB87E, 0x0201, 0xB87E, 0x0201, 0xB87E, 0x0201,
        0xB87E, 0x0201, 0xB87C, 0x86DA, 0xB87E, 0xCDCD, 0xB87E, 0xE6CD,
        0xB87E, 0xCDCD, 0xB87C, 0x8FE8, 0xB87E, 0x0368, 0xB87E, 0x033F,
        0xB87E, 0x1046, 0xB87E, 0x147D, 0xB87E, 0x147D, 0xB87E, 0x147D,
        0xB87E, 0x0368, 0xB87E, 0x033F, 0xB87E, 0x1046, 0xB87E, 0x147D,
        0xB87E, 0x147D, 0xB87E, 0x147D, 0xa436, 0x80dd, 0xa438, 0xf0AB,
        0xa436, 0x80df, 0xa438, 0xC009, 0xa436, 0x80e7, 0xa438, 0x401E,
        0xa436, 0x80e1, 0xa438, 0x120A, 0xa436, 0x86f2, 0xa438, 0x5094,
        0xa436, 0x8701, 0xa438, 0x5094, 0xa436, 0x80f1, 0xa438, 0x30CC,
        0xa436, 0x80f3, 0xa438, 0x0001, 0xa436, 0x80f5, 0xa438, 0x330B,
        0xa436, 0x80f8, 0xa438, 0xCB76, 0xa436, 0x8105, 0xa438, 0xf0D3,
        0xa436, 0x8107, 0xa438, 0x0002, 0xa436, 0x8109, 0xa438, 0xff0B,
        0xa436, 0x810c, 0xa438, 0xC86D, 0xB87C, 0x8a32, 0xB87E, 0x0400,
        0xa6f8, 0x0000, 0xa6f8, 0x0000, 0xa436, 0x81bc, 0xa438, 0x1300,
        0xa846, 0x2410, 0xa86A, 0x0801, 0xa85C, 0x9680, 0xa436, 0x841D,
        0xa438, 0x4A28, 0xa436, 0x8016, 0xa438, 0xBE05, 0xBF9C, 0x004A,
        0xBF96, 0x41FA, 0xBF9A, 0xDC81, 0xa436, 0x8018, 0xa438, 0x0700,
        0xa436, 0x8ff4, 0xa438, 0x01AE, 0xa436, 0x8fef, 0xa438, 0x0172,
        0xa438, 0x00dc, 0xc842, 0x0002, 0xFFFF, 0xFFFF
};

static const u16 phy_mcu_ram_code_8126a_1_3[] = {
        0xb892, 0x0000, 0xB88E, 0xC236, 0xB890, 0x1A1C, 0xB88E, 0xC238,
        0xB890, 0x1C1C, 0xB890, 0x1C1C, 0xB890, 0x2D2D, 0xB890, 0x2D2D,
        0xB890, 0x2D2A, 0xB890, 0x2A2A, 0xB890, 0x2A2A, 0xB890, 0x2A19,
        0xB88E, 0xC272, 0xB890, 0x8484, 0xB890, 0x8484, 0xB890, 0x84B4,
        0xB890, 0xB4B4, 0xB890, 0xB4B4, 0xB890, 0xF8F8, 0xB890, 0xF8F8,
        0xB890, 0xF8F8, 0xB88E, 0xC000, 0xB890, 0x0303, 0xB890, 0x0405,
        0xB890, 0x0608, 0xB890, 0x0A0B, 0xB890, 0x0E11, 0xB890, 0x1519,
        0xB890, 0x2028, 0xB890, 0x3503, 0xB890, 0x0304, 0xB890, 0x0405,
        0xB890, 0x0606, 0xB890, 0x0708, 0xB890, 0x090A, 0xB890, 0x0B0D,
        0xB890, 0x0F11, 0xB890, 0x1315, 0xB890, 0x181A, 0xB890, 0x2029,
        0xB890, 0x2F36, 0xB890, 0x3D43, 0xB890, 0x0101, 0xB890, 0x0102,
        0xB890, 0x0202, 0xB890, 0x0303, 0xB890, 0x0405, 0xB890, 0x0607,
        0xB890, 0x090A, 0xB890, 0x0C0E, 0xB88E, 0xC038, 0xB890, 0x6AE1,
        0xB890, 0x8E6B, 0xB890, 0xA767, 0xB890, 0x01EF, 0xB890, 0x5A63,
        0xB890, 0x2B99, 0xB890, 0x7F5D, 0xB890, 0x361F, 0xB890, 0xA127,
        0xB890, 0xB558, 0xB890, 0x11C3, 0xB890, 0x7D85, 0xB890, 0xBAC5,
        0xB890, 0xE691, 0xB890, 0x8F79, 0xB890, 0x3164, 0xB890, 0x3293,
        0xB890, 0xB80D, 0xB890, 0xE2B7, 0xB890, 0x0D62, 0xB890, 0x4F85,
        0xB890, 0xC919, 0xB890, 0x78F3, 0xB890, 0x77FF, 0xB890, 0xBD9E,
        0xB890, 0x69D6, 0xB890, 0x6DA4, 0xB890, 0x0CC5, 0xB88E, 0xC1D2,
        0xB890, 0x2425, 0xB890, 0x2627, 0xB890, 0x2829, 0xB890, 0x2A2B,
        0xB890, 0x2C2D, 0xB890, 0x2E2F, 0xB890, 0x3031, 0xB890, 0x3233,
        0xB890, 0x2323, 0xB890, 0x2424, 0xB890, 0x2525, 0xB890, 0x2626,
        0xB890, 0x2727, 0xB890, 0x2828, 0xB890, 0x2929, 0xB890, 0x2A2A,
        0xB890, 0x2B2C, 0xB890, 0x2C2D, 0xB890, 0x2D2E, 0xB890, 0x2E2F,
        0xB890, 0x2F30, 0xB890, 0x1A1B, 0xB890, 0x1D1E, 0xB890, 0x1F20,
        0xB890, 0x2123, 0xB890, 0x2425, 0xB890, 0x2628, 0xB890, 0x292A,
        0xB890, 0x2B2C, 0xB890, 0x2E12, 0xB88E, 0xC09A, 0xB890, 0xD3D3,
        0xB890, 0xD3D3, 0xB890, 0xD3D3, 0xB890, 0xD3D3, 0xB890, 0xD3D3,
        0xB890, 0xD3D3, 0xB890, 0xD3D3, 0xB890, 0xD3D3, 0xFFFF, 0xFFFF
};

static void
rtl8125_real_set_phy_mcu_8125b_1(struct net_device *dev)
{
        rtl8125_set_phy_mcu_ram_code(dev,
                                     phy_mcu_ram_code_8125b_1,
                                     ARRAY_SIZE(phy_mcu_ram_code_8125b_1)
                                    );
}

static void
rtl8125_set_phy_mcu_8125b_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8125b_1(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);
}

static void
rtl8125_real_set_phy_mcu_8125b_2(struct net_device *dev)
{
        rtl8125_set_phy_mcu_ram_code(dev,
                                     phy_mcu_ram_code_8125b_2,
                                     ARRAY_SIZE(phy_mcu_ram_code_8125b_2)
                                    );
}

static void
rtl8125_set_phy_mcu_8125b_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8125b_2(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);
}

static void
rtl8125_real_set_phy_mcu_8126a_1_1(struct net_device *dev)
{
        rtl8125_set_phy_mcu_ram_code(dev,
                                     phy_mcu_ram_code_8126a_1_1,
                                     ARRAY_SIZE(phy_mcu_ram_code_8126a_1_1)
                                    );
}

static void
rtl8125_real_set_phy_mcu_8126a_1_2(struct net_device *dev)
{
        rtl8125_set_phy_mcu_ram_code(dev,
                                     phy_mcu_ram_code_8126a_1_2,
                                     ARRAY_SIZE(phy_mcu_ram_code_8126a_1_2)
                                    );
}

static void
rtl8125_real_set_phy_mcu_8126a_1_3(struct net_device *dev)
{
        rtl8125_set_phy_mcu_ram_code(dev,
                                     phy_mcu_ram_code_8126a_1_3,
                                     ARRAY_SIZE(phy_mcu_ram_code_8126a_1_3)
                                    );
}

static void
rtl8125_set_phy_mcu_8126a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8126a_1_1(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8126a_1_2(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);

        rtl8125_set_phy_mcu_patch_request(tp);

        rtl8125_real_set_phy_mcu_8126a_1_3(dev);

        rtl8125_clear_phy_mcu_patch_request(tp);
}

static void
rtl8125_init_hw_phy_mcu(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u8 require_disable_phy_disable_mode = FALSE;

        if (tp->NotWrRamCodeToMicroP == TRUE) return;
        if (rtl8125_check_hw_phy_mcu_code_ver(dev)) return;

        if (HW_SUPPORT_CHECK_PHY_DISABLE_MODE(tp) && rtl8125_is_in_phy_disable_mode(dev))
                require_disable_phy_disable_mode = TRUE;

        if (require_disable_phy_disable_mode)
                rtl8125_disable_phy_disable_mode(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                rtl8125_set_phy_mcu_8125a_1(dev);
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                rtl8125_set_phy_mcu_8125a_2(dev);
                break;
        case CFG_METHOD_4:
                rtl8125_set_phy_mcu_8125b_1(dev);
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                rtl8125_set_phy_mcu_8125b_2(dev);
                break;
        case CFG_METHOD_8:
                rtl8125_set_phy_mcu_8126a_1(dev);
                break;
        }

        if (require_disable_phy_disable_mode)
                rtl8125_enable_phy_disable_mode(dev);

        rtl8125_write_hw_phy_mcu_code_ver(dev);

        rtl8125_mdio_write(tp,0x1F, 0x0000);

        tp->HwHasWrRamCodeToMicroP = TRUE;
}
#endif

static void
rtl8125_enable_phy_aldps(struct rtl8125_private *tp)
{
        //enable aldps
        //GPHY OCP 0xA430 bit[2] = 0x1 (en_aldps)
        SetEthPhyOcpBit(tp, 0xA430, BIT_2);
}

static void
rtl8125_hw_phy_config_8125a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        ClearAndSetEthPhyOcpBit(tp,
                                0xAD40,
                                0x03FF,
                                0x84
                               );

        SetEthPhyOcpBit(tp, 0xAD4E, BIT_4);
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD16,
                                0x03FF,
                                0x0006
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD32,
                                0x003F,
                                0x0006
                               );
        ClearEthPhyOcpBit(tp, 0xAC08, BIT_12);
        ClearEthPhyOcpBit(tp, 0xAC08, BIT_8);
        ClearAndSetEthPhyOcpBit(tp,
                                0xAC8A,
                                BIT_15|BIT_14|BIT_13|BIT_12,
                                BIT_14|BIT_13|BIT_12
                               );
        SetEthPhyOcpBit(tp, 0xAD18, BIT_10);
        SetEthPhyOcpBit(tp, 0xAD1A, 0x3FF);
        SetEthPhyOcpBit(tp, 0xAD1C, 0x3FF);

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EA);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0xC400
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EB);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0x0700,
                                0x0300
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80F8);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x1C00
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80F1);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x3000
                               );

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80FE);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0xA500
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8102);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x5000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8105);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x3300
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8100);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x7000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8104);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0xF000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8106);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x6500
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DC);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0xED00
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DF);
        SetEthPhyOcpBit(tp, 0xA438, BIT_8);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80E1);
        ClearEthPhyOcpBit(tp, 0xA438, BIT_8);

        ClearAndSetEthPhyOcpBit(tp,
                                0xBF06,
                                0x003F,
                                0x38
                               );

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x819F);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xD0B6);

        mdio_direct_write_phy_ocp(tp, 0xBC34, 0x5555);
        ClearAndSetEthPhyOcpBit(tp,
                                0xBF0A,
                                BIT_11|BIT_10|BIT_9,
                                BIT_11|BIT_9
                               );

        ClearEthPhyOcpBit(tp, 0xA5C0, BIT_10);

        SetEthPhyOcpBit(tp, 0xA442, BIT_11);

        //enable aldps
        //GPHY OCP 0xA430 bit[2] = 0x1 (en_aldps)
        if (aspm) {
                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8125_enable_phy_aldps(tp);
                }
        }
}

static void
rtl8125_hw_phy_config_8125a_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        SetEthPhyOcpBit(tp, 0xAD4E, BIT_4);
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD16,
                                0x03FF,
                                0x03FF
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD32,
                                0x003F,
                                0x0006
                               );
        ClearEthPhyOcpBit(tp, 0xAC08, BIT_12);
        ClearEthPhyOcpBit(tp, 0xAC08, BIT_8);
        ClearAndSetEthPhyOcpBit(tp,
                                0xACC0,
                                BIT_1|BIT_0,
                                BIT_1
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD40,
                                BIT_7|BIT_6|BIT_5,
                                BIT_6
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD40,
                                BIT_2|BIT_1|BIT_0,
                                BIT_2
                               );
        ClearEthPhyOcpBit(tp, 0xAC14, BIT_7);
        ClearEthPhyOcpBit(tp, 0xAC80, BIT_9|BIT_8);
        ClearAndSetEthPhyOcpBit(tp,
                                0xAC5E,
                                BIT_2|BIT_1|BIT_0,
                                BIT_1
                               );
        mdio_direct_write_phy_ocp(tp, 0xAD4C, 0x00A8);
        mdio_direct_write_phy_ocp(tp, 0xAC5C, 0x01FF);
        ClearAndSetEthPhyOcpBit(tp,
                                0xAC8A,
                                BIT_7|BIT_6|BIT_5|BIT_4,
                                BIT_5|BIT_4
                               );
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8157);
        ClearAndSetEthPhyOcpBit(tp,
                                0xB87E,
                                0xFF00,
                                0x0500
                               );
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8159);
        ClearAndSetEthPhyOcpBit(tp,
                                0xB87E,
                                0xFF00,
                                0x0700
                               );


        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80A2);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0153);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x809C);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0153);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x81B3);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0043);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00A7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00D6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00EC);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00F6);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FB);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FD);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FF);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x00BB);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0058);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0029);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0013);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0009);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0004);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0002);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8257);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x020F);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EA);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x7843);


        rtl8125_set_phy_mcu_patch_request(tp);

        ClearEthPhyOcpBit(tp, 0xB896, BIT_0);
        ClearEthPhyOcpBit(tp, 0xB892, 0xFF00);

        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC091);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E12);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC092);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1214);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC094);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1516);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC096);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x171B);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC098);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1B1C);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09A);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1F1F);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09C);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x2021);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09E);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x2224);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A0);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A2);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A4);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC018);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0AF2);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01A);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0D4A);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01C);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0F26);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01E);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x118D);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC020);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x14F3);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC022);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x175A);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC024);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x19C0);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC026);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1C26);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC089);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x6050);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08A);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x5F6E);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08C);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E6E);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08E);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E6E);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC090);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E12);

        SetEthPhyOcpBit(tp, 0xB896, BIT_0);

        rtl8125_clear_phy_mcu_patch_request(tp);


        SetEthPhyOcpBit(tp, 0xD068, BIT_13);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x81A2);
        SetEthPhyOcpBit(tp, 0xA438, BIT_8);
        ClearAndSetEthPhyOcpBit(tp,
                                0xB54C,
                                0xFF00,
                                0xDB00);


        ClearEthPhyOcpBit(tp, 0xA454, BIT_0);


        SetEthPhyOcpBit(tp, 0xA5D4, BIT_5);
        ClearEthPhyOcpBit(tp, 0xAD4E, BIT_4);
        ClearEthPhyOcpBit(tp, 0xA86A, BIT_0);


        SetEthPhyOcpBit(tp, 0xA442, BIT_11);


        if (tp->RequirePhyMdiSwapPatch) {
                u16 adccal_offset_p0;
                u16 adccal_offset_p1;
                u16 adccal_offset_p2;
                u16 adccal_offset_p3;
                u16 rg_lpf_cap_xg_p0;
                u16 rg_lpf_cap_xg_p1;
                u16 rg_lpf_cap_xg_p2;
                u16 rg_lpf_cap_xg_p3;
                u16 rg_lpf_cap_p0;
                u16 rg_lpf_cap_p1;
                u16 rg_lpf_cap_p2;
                u16 rg_lpf_cap_p3;

                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0007,
                                        0x0001
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0000
                                       );
                adccal_offset_p0 = mdio_direct_read_phy_ocp(tp, 0xD06A);
                adccal_offset_p0 &= 0x07FF;
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0008
                                       );
                adccal_offset_p1 = mdio_direct_read_phy_ocp(tp, 0xD06A);
                adccal_offset_p1 &= 0x07FF;
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0010
                                       );
                adccal_offset_p2 = mdio_direct_read_phy_ocp(tp, 0xD06A);
                adccal_offset_p2 &= 0x07FF;
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0018
                                       );
                adccal_offset_p3 = mdio_direct_read_phy_ocp(tp, 0xD06A);
                adccal_offset_p3 &= 0x07FF;


                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0000
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD06A,
                                        0x07FF,
                                        adccal_offset_p3
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0008
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD06A,
                                        0x07FF,
                                        adccal_offset_p2
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0010
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD06A,
                                        0x07FF,
                                        adccal_offset_p1
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD068,
                                        0x0018,
                                        0x0018
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xD06A,
                                        0x07FF,
                                        adccal_offset_p0
                                       );


                rg_lpf_cap_xg_p0 = mdio_direct_read_phy_ocp(tp, 0xBD5A);
                rg_lpf_cap_xg_p0 &= 0x001F;
                rg_lpf_cap_xg_p1 = mdio_direct_read_phy_ocp(tp, 0xBD5A);
                rg_lpf_cap_xg_p1 &= 0x1F00;
                rg_lpf_cap_xg_p2 = mdio_direct_read_phy_ocp(tp, 0xBD5C);
                rg_lpf_cap_xg_p2 &= 0x001F;
                rg_lpf_cap_xg_p3 = mdio_direct_read_phy_ocp(tp, 0xBD5C);
                rg_lpf_cap_xg_p3 &= 0x1F00;
                rg_lpf_cap_p0 = mdio_direct_read_phy_ocp(tp, 0xBC18);
                rg_lpf_cap_p0 &= 0x001F;
                rg_lpf_cap_p1 = mdio_direct_read_phy_ocp(tp, 0xBC18);
                rg_lpf_cap_p1 &= 0x1F00;
                rg_lpf_cap_p2 = mdio_direct_read_phy_ocp(tp, 0xBC1A);
                rg_lpf_cap_p2 &= 0x001F;
                rg_lpf_cap_p3 = mdio_direct_read_phy_ocp(tp, 0xBC1A);
                rg_lpf_cap_p3 &= 0x1F00;


                ClearAndSetEthPhyOcpBit(tp,
                                        0xBD5A,
                                        0x001F,
                                        rg_lpf_cap_xg_p3 >> 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBD5A,
                                        0x1F00,
                                        rg_lpf_cap_xg_p2 << 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBD5C,
                                        0x001F,
                                        rg_lpf_cap_xg_p1 >> 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBD5C,
                                        0x1F00,
                                        rg_lpf_cap_xg_p0 << 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBC18,
                                        0x001F,
                                        rg_lpf_cap_p3 >> 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBC18,
                                        0x1F00,
                                        rg_lpf_cap_p2 << 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBC1A,
                                        0x001F,
                                        rg_lpf_cap_p1 >> 8
                                       );
                ClearAndSetEthPhyOcpBit(tp,
                                        0xBC1A,
                                        0x1F00,
                                        rg_lpf_cap_p0 << 8
                                       );
        }


        SetEthPhyOcpBit(tp, 0xA424, BIT_3);


        if (aspm) {
                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8125_enable_phy_aldps(tp);
                }
        }
}

static void
rtl8125_hw_phy_config_8125b_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        SetEthPhyOcpBit(tp, 0xA442, BIT_11);


        SetEthPhyOcpBit(tp, 0xBC08, (BIT_3 | BIT_2));


        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FFF);
                ClearAndSetEthPhyOcpBit(tp,
                                        0xA438,
                                        0xFF00,
                                        0x0400
                                       );
        }
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8560);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8562);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8564);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8566);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8568);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x856A);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FFE);
                mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0907);
        }
        ClearAndSetEthPhyOcpBit(tp,
                                0xACDA,
                                0xFF00,
                                0xFF00
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xACDE,
                                0xF000,
                                0xF000
                               );
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80D6);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F2);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F4);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x6077);
        mdio_direct_write_phy_ocp(tp, 0xB506, 0x01E7);
        mdio_direct_write_phy_ocp(tp, 0xAC8C, 0x0FFC);
        mdio_direct_write_phy_ocp(tp, 0xAC46, 0xB7B4);
        mdio_direct_write_phy_ocp(tp, 0xAC50, 0x0FBC);
        mdio_direct_write_phy_ocp(tp, 0xAC3C, 0x9240);
        mdio_direct_write_phy_ocp(tp, 0xAC4E, 0x0DB4);
        mdio_direct_write_phy_ocp(tp, 0xACC6, 0x0707);
        mdio_direct_write_phy_ocp(tp, 0xACC8, 0xA0D3);
        mdio_direct_write_phy_ocp(tp, 0xAD08, 0x0007);

        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8013);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0700);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FB9);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBA);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0100);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBC);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x1900);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBE);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xE100);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC0);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0800);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC2);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xE500);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC4);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0F00);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC6);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF100);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC8);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0400);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCa);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF300);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCc);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFD00);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCe);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFF00);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD0);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFB00);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD2);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0100);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD4);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF400);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD6);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFF00);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD8);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF600);


        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x813D);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x390E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x814F);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x790E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80B0);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0F31);
        SetEthPhyOcpBit(tp, 0xBF4C, BIT_1);
        SetEthPhyOcpBit(tp, 0xBCCA, (BIT_9 | BIT_8));
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8141);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x320E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8153);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x720E);
        ClearEthPhyOcpBit(tp, 0xA432, BIT_6);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8529);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x050E);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x816C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xC4A0);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8170);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xC4A0);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8174);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x04A0);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8178);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x04A0);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x817C);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0719);
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FF4);
                mdio_direct_write_phy_ocp(tp, 0xA438, 0x0400);
                mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FF1);
                mdio_direct_write_phy_ocp(tp, 0xA438, 0x0404);
        }
        mdio_direct_write_phy_ocp(tp, 0xBF4A, 0x001B);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8033);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8037);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x803B);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFC32);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x803F);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8043);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8047);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);


        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8145);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x370E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8157);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x770E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8169);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0D0A);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x817B);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x1D0A);


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8217);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x5000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x821A);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x5000
                               );

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DA);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0403);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DC);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x1000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B3);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0384);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2007);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80BA);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x6C00
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xF009);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80BD);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x9F00
                               );

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80C7);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xf083);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DD);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x03f0);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DF);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x1000
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80CB);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x2007);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80CE);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x6C00
                               );
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80C9);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x8009);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80D1);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x8000
                               );

        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A3);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x200A);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A5);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0xF0AD);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x809F);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x6073);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x000B);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A9);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0xC000
                               );

        rtl8125_set_phy_mcu_patch_request(tp);

        ClearEthPhyOcpBit(tp, 0xB896, BIT_0);
        ClearEthPhyOcpBit(tp, 0xB892, 0xFF00);

        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC23E);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0000);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC240);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0103);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC242);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0507);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC244);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x090B);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC246);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x0C0E);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC248);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1012);
        mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC24A);
        mdio_direct_write_phy_ocp(tp, 0xB890, 0x1416);

        SetEthPhyOcpBit(tp, 0xB896, BIT_0);

        rtl8125_clear_phy_mcu_patch_request(tp);


        SetEthPhyOcpBit(tp, 0xA86A, BIT_0);
        SetEthPhyOcpBit(tp, 0xA6F0, BIT_0);


        mdio_direct_write_phy_ocp(tp, 0xBFA0, 0xD70D);
        mdio_direct_write_phy_ocp(tp, 0xBFA2, 0x4100);
        mdio_direct_write_phy_ocp(tp, 0xBFA4, 0xE868);
        mdio_direct_write_phy_ocp(tp, 0xBFA6, 0xDC59);
        mdio_direct_write_phy_ocp(tp, 0xB54C, 0x3C18);
        ClearEthPhyOcpBit(tp, 0xBFA4, BIT_5);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x817D);
        SetEthPhyOcpBit(tp, 0xA438, BIT_12);


        if (aspm) {
                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8125_enable_phy_aldps(tp);
                }
        }
}

static void
rtl8125_hw_phy_config_8125b_2(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        SetEthPhyOcpBit(tp, 0xA442, BIT_11);


        ClearAndSetEthPhyOcpBit(tp,
                                0xAC46,
                                0x00F0,
                                0x0090
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xAD30,
                                0x0003,
                                0x0001
                               );


        RTL_W16(tp, EEE_TXIDLE_TIMER_8125, tp->eee.tx_lpi_timer);

        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F5);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x760E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8107);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x360E);
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8551);
        ClearAndSetEthPhyOcpBit(tp,
                                0xB87E,
                                BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8,
                                BIT_11
                               );

        ClearAndSetEthPhyOcpBit(tp,
                                0xbf00,
                                0xE000,
                                0xA000
                               );
        ClearAndSetEthPhyOcpBit(tp,
                                0xbf46,
                                0x0F00,
                                0x0300
                               );
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8044);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x804A);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8050);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8056);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x805C);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8062);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8068);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x806E);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x8074);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
        mdio_direct_write_phy_ocp(tp, 0xa436, 0x807A);
        mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);


        SetEthPhyOcpBit(tp, 0xA4CA, BIT_6);


        ClearAndSetEthPhyOcpBit(tp,
                                0xBF84,
                                BIT_15 | BIT_14 | BIT_13,
                                BIT_15 | BIT_13
                               );


        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8170);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                BIT_13 | BIT_10 | BIT_9 | BIT_8,
                                BIT_15 | BIT_14 | BIT_12 | BIT_11
                               );


        SetEthPhyOcpBit(tp, 0xA424, BIT_3);

        /*
        mdio_direct_write_phy_ocp(tp, 0xBFA0, 0xD70D);
        mdio_direct_write_phy_ocp(tp, 0xBFA2, 0x4100);
        mdio_direct_write_phy_ocp(tp, 0xBFA4, 0xE868);
        mdio_direct_write_phy_ocp(tp, 0xBFA6, 0xDC59);
        mdio_direct_write_phy_ocp(tp, 0xB54C, 0x3C18);
        ClearEthPhyOcpBit(tp, 0xBFA4, BIT_5);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x817D);
        SetEthPhyOcpBit(tp, 0xA438, BIT_12);
        */


        if (aspm) {
                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8125_enable_phy_aldps(tp);
                }
        }
}

static void
rtl8125_hw_phy_config_8126a_1(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        SetEthPhyOcpBit(tp, 0xA442, BIT_11);

        RTL_W16(tp, EEE_TXIDLE_TIMER_8125, tp->eee.tx_lpi_timer);

        if (aspm) {
                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8125_enable_phy_aldps(tp);
                }
        }
}

static void
rtl8125_hw_phy_config(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (tp->resume_not_chg_speed) return;

        tp->phy_reset_enable(dev);

        if (HW_DASH_SUPPORT_TYPE_3(tp) && tp->HwPkgDet == 0x06) return;

#ifndef ENABLE_USE_FIRMWARE_FILE
        if (!tp->rtl_fw) {
                rtl8125_set_hw_phy_before_init_phy_mcu(dev);

                rtl8125_init_hw_phy_mcu(dev);
        }
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                rtl8125_hw_phy_config_8125a_1(dev);
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                rtl8125_hw_phy_config_8125a_2(dev);
                break;
        case CFG_METHOD_4:
                rtl8125_hw_phy_config_8125b_1(dev);
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                rtl8125_hw_phy_config_8125b_2(dev);
                break;
        case CFG_METHOD_8:
                rtl8125_hw_phy_config_8126a_1(dev);
                break;
        }

        //legacy force mode(Chap 22)
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        default:
                rtl8125_mdio_write(tp, 0x1F, 0x0A5B);
                rtl8125_clear_eth_phy_bit(tp, 0x12, BIT_15);
                rtl8125_mdio_write(tp, 0x1F, 0x0000);
                break;
        }

        /*ocp phy power saving*/
        /*
        if (aspm) {
        if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_6)
                rtl8125_enable_ocp_phy_power_saving(dev);
        }
        */

        rtl8125_mdio_write(tp, 0x1F, 0x0000);

        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                if (tp->eee.eee_enabled)
                        rtl8125_enable_eee(tp);
                else
                        rtl8125_disable_eee(tp);
        }
}

static void
rtl8125_up(struct net_device *dev)
{
        rtl8125_hw_init(dev);
        rtl8125_hw_reset(dev);
        rtl8125_powerup_pll(dev);
        rtl8125_hw_ephy_config(dev);
        rtl8125_hw_phy_config(dev);
        rtl8125_hw_config(dev);
}

/*
static inline void rtl8125_delete_esd_timer(struct net_device *dev, struct timer_list *timer)
{
        del_timer_sync(timer);
}

static inline void rtl8125_request_esd_timer(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->esd_timer;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        setup_timer(timer, rtl8125_esd_timer, (unsigned long)dev);
#else
        timer_setup(timer, rtl8125_esd_timer, 0);
#endif
        mod_timer(timer, jiffies + RTL8125_ESD_TIMEOUT);
}
*/

/*
static inline void rtl8125_delete_link_timer(struct net_device *dev, struct timer_list *timer)
{
        del_timer_sync(timer);
}

static inline void rtl8125_request_link_timer(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->link_timer;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        setup_timer(timer, rtl8125_link_timer, (unsigned long)dev);
#else
        timer_setup(timer, rtl8125_link_timer, 0);
#endif
        mod_timer(timer, jiffies + RTL8125_LINK_TIMEOUT);
}
*/

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void
rtl8125_netpoll(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;
        for (i = 0; i < tp->irq_nvecs; i++) {
                struct r8125_irq *irq = &tp->irq_tbl[i];
                struct r8125_napi *r8125napi = &tp->r8125napi[i];

                disable_irq(irq->vector);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
                irq->handler(irq->vector, r8125napi);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
                irq->handler(irq->vector, r8125napi, NULL);
#else
                irq->handler(irq->vector, r8125napi);
#endif

                enable_irq(irq->vector);
        }
}
#endif //CONFIG_NET_POLL_CONTROLLER

static void
rtl8125_get_bios_setting(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->bios_setting = RTL_R32(tp, TimeInt2);
                break;
        }
}

static void
rtl8125_set_bios_setting(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W32(tp, TimeInt2, tp->bios_setting);
                break;
        }
}

static void
rtl8125_setup_mqs_reg(struct rtl8125_private *tp)
{
        int i;

        //tx
        tp->tx_ring[0].tdsar_reg = TxDescStartAddrLow;
        for (i = 1; i < R8125_MAX_TX_QUEUES; i++) {
                tp->tx_ring[i].tdsar_reg = (u16)(TNPDS_Q1_LOW_8125 + (i - 1) * 8);
        }

        for (i = 0; i < R8125_MAX_TX_QUEUES; i++) {
                if (tp->HwSuppTxNoCloseVer == 4) {
                        tp->tx_ring[i].hw_clo_ptr_reg = (u16)(HW_CLO_PTR0_8126 + i * 4);
                        tp->tx_ring[i].sw_tail_ptr_reg = (u16)(SW_TAIL_PTR0_8126 + i * 4);
                } else {
                        tp->tx_ring[i].hw_clo_ptr_reg = (u16)(HW_CLO_PTR0_8125 + i * 4);
                        tp->tx_ring[i].sw_tail_ptr_reg = (u16)(SW_TAIL_PTR0_8125 + i * 4);
                }
        }

        //rx
        tp->rx_ring[0].rdsar_reg = RxDescAddrLow;
        for (i = 1; i < R8125_MAX_RX_QUEUES; i++) {
                tp->rx_ring[i].rdsar_reg = (u16)(RDSAR_Q1_LOW_8125 + (i - 1) * 8);
        }

        tp->isr_reg[0] = ISR0_8125;
        for (i = 1; i < R8125_MAX_QUEUES; i++) {
                tp->isr_reg[i] = (u16)(ISR1_8125 + (i - 1) * 4);
        }

        tp->imr_reg[0] = IMR0_8125;
        for (i = 1; i < R8125_MAX_QUEUES; i++) {
                tp->imr_reg[i] = (u16)(IMR1_8125 + (i - 1) * 4);
        }
}

static void
rtl8125_init_software_variable(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;

        rtl8125_get_bios_setting(dev);

#ifdef ENABLE_LIB_SUPPORT
        tp->ring_lib_enabled = 1;
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                //tp->HwSuppDashVer = 3;
                break;
        default:
                tp->HwSuppDashVer = 0;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwPkgDet = rtl8125_mac_ocp_read(tp, 0xDC00);
                tp->HwPkgDet = (tp->HwPkgDet >> 3) & 0x07;
                break;
        }

        if (HW_DASH_SUPPORT_TYPE_3(tp) && tp->HwPkgDet == 0x06)
                eee_enable = 0;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppNowIsOobVer = 1;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwPcieSNOffset = 0x16C;
                break;
        }

#ifdef ENABLE_REALWOW_SUPPORT
        rtl8125_get_realwow_hw_version(dev);
#endif //ENABLE_REALWOW_SUPPORT

        if (HW_DASH_SUPPORT_DASH(tp) && rtl8125_check_dash(tp))
                tp->DASH = 1;
        else
                tp->DASH = 0;

        if (tp->DASH) {
                if (HW_DASH_SUPPORT_TYPE_3(tp)) {
                        u64 CmacMemPhysAddress;
                        void __iomem *cmac_ioaddr = NULL;

                        //map CMAC IO space
                        CmacMemPhysAddress = rtl8125_csi_other_fun_read(tp, 0, 0x18);
                        if (!(CmacMemPhysAddress & BIT_0)) {
                                if (CmacMemPhysAddress & BIT_2)
                                        CmacMemPhysAddress |=  (u64)rtl8125_csi_other_fun_read(tp, 0, 0x1C) << 32;

                                CmacMemPhysAddress &=  0xFFFFFFF0;
                                /* ioremap MMIO region */
                                cmac_ioaddr = ioremap(CmacMemPhysAddress, R8125_REGS_SIZE);
                        }

                        if (cmac_ioaddr == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                                if (netif_msg_probe(tp))
                                        dev_err(&pdev->dev, "cannot remap CMAC MMIO, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        }

                        if (cmac_ioaddr == NULL) {
                                tp->DASH = 0;
                        } else {
                                tp->mapped_cmac_ioaddr = cmac_ioaddr;
                        }
                }

                eee_enable = 0;
        }

        if (HW_DASH_SUPPORT_TYPE_3(tp))
                tp->cmac_ioaddr = tp->mapped_cmac_ioaddr;

        if (aspm) {
                switch (tp->mcfg) {
                case CFG_METHOD_2:
                case CFG_METHOD_3:
                case CFG_METHOD_4:
                case CFG_METHOD_5:
                case CFG_METHOD_6:
                case CFG_METHOD_7:
                case CFG_METHOD_8:
                        tp->org_pci_offset_99 = rtl8125_csi_fun0_read_byte(tp, 0x99);
                        tp->org_pci_offset_99 &= ~(BIT_5|BIT_6);
                        break;
                }

                switch (tp->mcfg) {
                case CFG_METHOD_2:
                case CFG_METHOD_3:
                case CFG_METHOD_6:
                        tp->org_pci_offset_180 = rtl8125_csi_fun0_read_byte(tp, 0x264);
                        break;
                case CFG_METHOD_4:
                case CFG_METHOD_5:
                case CFG_METHOD_7:
                        tp->org_pci_offset_180 = rtl8125_csi_fun0_read_byte(tp, 0x214);
                        break;
                case CFG_METHOD_8:
                        tp->org_pci_offset_180 = rtl8125_csi_fun0_read_byte(tp, 0x22c);
                        break;
                }
        }

        pci_read_config_byte(pdev, 0x80, &tp->org_pci_offset_80);
        pci_read_config_byte(pdev, 0x81, &tp->org_pci_offset_81);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        default:
                tp->use_timer_interrrupt = TRUE;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
                tp->HwSuppMaxPhyLinkSpeed = 2500;
                break;
        case CFG_METHOD_8:
                tp->HwSuppMaxPhyLinkSpeed = 5000;
                break;
        default:
                tp->HwSuppMaxPhyLinkSpeed = 1000;
                break;
        }

        if (timer_count == 0 || tp->mcfg == CFG_METHOD_DEFAULT)
                tp->use_timer_interrrupt = FALSE;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_V3;
                break;
        default:
                tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_NOT_SUPPORT;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppLinkChgWakeUpVer = 3;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppD0SpeedUpVer = 1;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppCheckPhyDisableModeVer = 3;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
                tp->HwSuppTxNoCloseVer = 3;
                break;
        case CFG_METHOD_8:
                tp->HwSuppTxNoCloseVer = 4;
                break;
        }

        if (tp->HwSuppTxNoCloseVer == 4)
                tp->MaxTxDescPtrMask = MAX_TX_NO_CLOSE_DESC_PTR_MASK_V3;
        else if (tp->HwSuppTxNoCloseVer == 3)
                tp->MaxTxDescPtrMask = MAX_TX_NO_CLOSE_DESC_PTR_MASK_V2;
        else
                tx_no_close_enable = 0;

        if (tp->HwSuppTxNoCloseVer > 0 && tx_no_close_enable == 1)
                tp->EnableTxNoClose = TRUE;

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                tp->RequireLSOPatch = TRUE;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_2;
                break;
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_3;
                break;
        case CFG_METHOD_4:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_4;
                break;
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_5;
                break;
        case CFG_METHOD_8:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_8;
                break;
        }

        if (tp->HwIcVerUnknown) {
                tp->NotWrRamCodeToMicroP = TRUE;
                tp->NotWrMcuPatchCode = TRUE;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                if ((rtl8125_mac_ocp_read(tp, 0xD442) & BIT_5) &&
                    (mdio_direct_read_phy_ocp(tp, 0xD068) & BIT_1))
                        tp->RequirePhyMdiSwapPatch = TRUE;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppMacMcuVer = 2;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->MacMcuPageSize = RTL8125_MAC_MCU_PAGE_SIZE;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppNumTxQueues = 2;
                tp->HwSuppNumRxQueues = 4;
                break;
        default:
                tp->HwSuppNumTxQueues = 1;
                tp->HwSuppNumRxQueues = 1;
                break;
        }

        tp->num_tx_rings = 1;
#ifdef ENABLE_MULTIPLE_TX_QUEUE
#ifndef ENABLE_LIB_SUPPORT
        tp->num_tx_rings = tp->HwSuppNumTxQueues;
#endif
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppRssVer = 5;
                tp->HwSuppIndirTblEntries = 128;
                break;
        }

        tp->num_rx_rings = 1;
#ifdef ENABLE_RSS_SUPPORT
#ifdef ENABLE_LIB_SUPPORT
        if (tp->HwSuppRssVer > 0)
                tp->EnableRss = 1;
#else
        if (tp->HwSuppRssVer > 0) {
                u8 rss_queue_num = netif_get_num_default_rss_queues();
                tp->num_rx_rings = (tp->HwSuppNumRxQueues > rss_queue_num)?
                                   rss_queue_num : tp->HwSuppNumRxQueues;

                if (!(tp->num_rx_rings >= 2 && tp->irq_nvecs >= tp->num_rx_rings))
                        tp->num_rx_rings = 1;

                if (tp->num_rx_rings >= 2)
                        tp->EnableRss = 1;
        }
#endif
#endif

        rtl8125_setup_mqs_reg(tp);

        rtl8125_set_ring_size(tp, NUM_RX_DESC, NUM_TX_DESC);

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                tp->HwSuppPtpVer = 1;
                break;
        }
#ifdef ENABLE_PTP_SUPPORT
        if (tp->HwSuppPtpVer > 0)
                tp->EnablePtp = 1;
#endif

        //init interrupt
        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppIsrVer = 2;
                break;
        default:
                tp->HwSuppIsrVer = 1;
                break;
        }

        tp->HwCurrIsrVer = tp->HwSuppIsrVer;
        if (tp->HwSuppIsrVer == 2) {
                if (!(tp->features & RTL_FEATURE_MSIX) ||
                    tp->irq_nvecs < R8125_MIN_MSIX_VEC_8125B)
                        tp->HwCurrIsrVer = 1;
        }

        if (tp->HwCurrIsrVer < 2 || tp->irq_nvecs < 19)
                tp->num_tx_rings = 1;

        if (tp->HwCurrIsrVer == 2) {
                int i;

                tp->intr_mask = ISRIMR_V2_LINKCHG | ISRIMR_TOK_Q0;
                if (tp->num_tx_rings > 1)
                        tp->intr_mask |= ISRIMR_TOK_Q1;

                for (i = 0; i < tp->num_rx_rings; i++)
                        tp->intr_mask |= ISRIMR_V2_ROK_Q0 << i;
        } else {
                tp->intr_mask = LinkChg | RxDescUnavail | TxOK | RxOK | SWInt;
                tp->timer_intr_mask = LinkChg | PCSTimeout;

#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH) {
                        if (HW_DASH_SUPPORT_TYPE_3(tp)) {
                                tp->timer_intr_mask |= ( ISRIMR_DASH_INTR_EN | ISRIMR_DASH_INTR_CMAC_RESET);
                                tp->intr_mask |= ( ISRIMR_DASH_INTR_EN | ISRIMR_DASH_INTR_CMAC_RESET);
                        }
                }
#endif
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_6:
                tp->HwSuppIntMitiVer = 3;
                break;
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppIntMitiVer = 4;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->HwSuppExtendTallyCounterVer = 1;
                break;
        }

        timer_count_v2 = (timer_count / 0x100);

#ifndef ENABLE_LIB_SUPPORT
        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
                if (tp->HwSuppIsrVer == 2) {
                        tp->RequireRduNonStopPatch = 1;
                        tp->EnableRss = 0;
                }
                break;
        }
#endif

        tp->InitRxDescType = RX_DESC_RING_TYPE_1;
        if (tp->EnableRss || tp->EnablePtp)
                tp->InitRxDescType = RX_DESC_RING_TYPE_3;

        tp->RxDescLength = RX_DESC_LEN_TYPE_1;
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                tp->RxDescLength = RX_DESC_LEN_TYPE_3;

        tp->rtl8125_rx_config = rtl_chip_info[tp->chipset].RCR_Cfg;
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                tp->rtl8125_rx_config |= EnableRxDescV3;

        tp->NicCustLedValue = RTL_R16(tp, CustomLED);

        tp->wol_opts = rtl8125_get_hw_wol(tp);
        tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

        if (HW_SUPP_PHY_LINK_SPEED_2500M(tp))
                rtl8125_link_option((u8*)&autoneg_mode, (u32*)&speed_mode, (u8*)&duplex_mode, (u32*)&advertising_mode);
        else
                rtl8125_link_option_giga((u8*)&autoneg_mode, (u32*)&speed_mode, (u8*)&duplex_mode, (u32*)&advertising_mode);

        tp->autoneg = autoneg_mode;
        tp->speed = speed_mode;
        tp->duplex = duplex_mode;
        tp->advertising = advertising_mode;
        if (HW_SUPP_PHY_LINK_SPEED_5000M(tp))
                tp->advertising |= RTK_ADVERTISED_5000baseX_Full;
        tp->fcpause = rtl8125_fc_full;

        tp->max_jumbo_frame_size = rtl_chip_info[tp->chipset].jumbo_frame_sz;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
        /* MTU range: 60 - hw-specific max */
        dev->min_mtu = ETH_MIN_MTU;
        dev->max_mtu = tp->max_jumbo_frame_size;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)

        if (tp->mcfg != CFG_METHOD_DEFAULT) {
                struct ethtool_eee *eee = &tp->eee;

                eee->eee_enabled = eee_enable;
                eee->supported  = SUPPORTED_100baseT_Full |
                                  SUPPORTED_1000baseT_Full;
                switch (tp->mcfg) {
                case CFG_METHOD_4:
                case CFG_METHOD_5:
                case CFG_METHOD_8:
                        eee->supported |= SUPPORTED_2500baseX_Full;
                        break;
                }
                eee->advertised = mmd_eee_adv_to_ethtool_adv_t(MDIO_EEE_1000T | MDIO_EEE_100TX);
                eee->tx_lpi_timer = dev->mtu + ETH_HLEN + 0x20;
        }

        tp->ptp_master_mode = enable_ptp_master_mode;

#ifdef ENABLE_RSS_SUPPORT
        if (tp->EnableRss)
                rtl8125_init_rss(tp);
#endif
}

static void
rtl8125_release_board(struct pci_dev *pdev,
                      struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;

        rtl8125_set_bios_setting(dev);
        rtl8125_rar_set(tp, tp->org_mac_addr);
        tp->wol_enabled = WOL_DISABLED;

        if (!tp->DASH)
                rtl8125_phy_power_down(dev);

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH)
                FreeAllocatedDashShareMemory(dev);
#endif

        if (tp->mapped_cmac_ioaddr != NULL)
                iounmap(tp->mapped_cmac_ioaddr);

        iounmap(ioaddr);
        pci_release_regions(pdev);
        pci_clear_mwi(pdev);
        pci_disable_device(pdev);
        free_netdev(dev);
}

static void
rtl8125_hw_address_set(struct net_device *dev, u8 mac_addr[MAC_ADDR_LEN])
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
        eth_hw_addr_set(dev, mac_addr);
#else
        memcpy(dev->dev_addr, mac_addr, MAC_ADDR_LEN);
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
}

static int
rtl8125_get_mac_address(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;
        u8 mac_addr[MAC_ADDR_LEN];

        for (i = 0; i < MAC_ADDR_LEN; i++)
                mac_addr[i] = RTL_R8(tp, MAC0 + i);

        if(tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                *(u32*)&mac_addr[0] = RTL_R32(tp, BACKUP_ADDR0_8125);
                *(u16*)&mac_addr[4] = RTL_R16(tp, BACKUP_ADDR1_8125);
        }

        if (!is_valid_ether_addr(mac_addr)) {
                netif_err(tp, probe, dev, "Invalid ether addr %pM\n",
                          mac_addr);
                eth_random_addr(mac_addr);
                dev->addr_assign_type = NET_ADDR_RANDOM;
                netif_info(tp, probe, dev, "Random ether addr %pM\n",
                           mac_addr);
                tp->random_mac = 1;
        }

        rtl8125_hw_address_set(dev, mac_addr);
        rtl8125_rar_set(tp, mac_addr);

        /* keep the original MAC address */
        memcpy(tp->org_mac_addr, dev->dev_addr, MAC_ADDR_LEN);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
#endif
        return 0;
}

/**
 * rtl8125_set_mac_address - Change the Ethernet Address of the NIC
 * @dev: network interface device structure
 * @p:   pointer to an address structure
 *
 * Return 0 on success, negative on failure
 **/
static int
rtl8125_set_mac_address(struct net_device *dev,
                        void *p)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct sockaddr *addr = p;

        if (!is_valid_ether_addr(addr->sa_data))
                return -EADDRNOTAVAIL;

        rtl8125_hw_address_set(dev, addr->sa_data);

        rtl8125_rar_set(tp, dev->dev_addr);

        return 0;
}

/******************************************************************************
 * rtl8125_rar_set - Puts an ethernet address into a receive address register.
 *
 * tp - The private data structure for driver
 * addr - Address to put into receive address register
 *****************************************************************************/
void
rtl8125_rar_set(struct rtl8125_private *tp,
                const u8 *addr)
{
        uint32_t rar_low = 0;
        uint32_t rar_high = 0;

        rar_low = ((uint32_t) addr[0] |
                   ((uint32_t) addr[1] << 8) |
                   ((uint32_t) addr[2] << 16) |
                   ((uint32_t) addr[3] << 24));

        rar_high = ((uint32_t) addr[4] |
                    ((uint32_t) addr[5] << 8));

        rtl8125_enable_cfg9346_write(tp);
        RTL_W32(tp, MAC0, rar_low);
        RTL_W32(tp, MAC4, rar_high);

        rtl8125_disable_cfg9346_write(tp);
}

#ifdef ETHTOOL_OPS_COMPAT
static int ethtool_get_settings(struct net_device *dev, void *useraddr)
{
        struct ethtool_cmd cmd = { ETHTOOL_GSET };
        int err;

        if (!ethtool_ops->get_settings)
                return -EOPNOTSUPP;

        err = ethtool_ops->get_settings(dev, &cmd);
        if (err < 0)
                return err;

        if (copy_to_user(useraddr, &cmd, sizeof(cmd)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_settings(struct net_device *dev, void *useraddr)
{
        struct ethtool_cmd cmd;

        if (!ethtool_ops->set_settings)
                return -EOPNOTSUPP;

        if (copy_from_user(&cmd, useraddr, sizeof(cmd)))
                return -EFAULT;

        return ethtool_ops->set_settings(dev, &cmd);
}

static int ethtool_get_drvinfo(struct net_device *dev, void *useraddr)
{
        struct ethtool_drvinfo info;
        struct ethtool_ops *ops = ethtool_ops;

        if (!ops->get_drvinfo)
                return -EOPNOTSUPP;

        memset(&info, 0, sizeof(info));
        info.cmd = ETHTOOL_GDRVINFO;
        ops->get_drvinfo(dev, &info);

        if (ops->self_test_count)
                info.testinfo_len = ops->self_test_count(dev);
        if (ops->get_stats_count)
                info.n_stats = ops->get_stats_count(dev);
        if (ops->get_regs_len)
                info.regdump_len = ops->get_regs_len(dev);
        if (ops->get_eeprom_len)
                info.eedump_len = ops->get_eeprom_len(dev);

        if (copy_to_user(useraddr, &info, sizeof(info)))
                return -EFAULT;
        return 0;
}

static int ethtool_get_regs(struct net_device *dev, char *useraddr)
{
        struct ethtool_regs regs;
        struct ethtool_ops *ops = ethtool_ops;
        void *regbuf;
        int reglen, ret;

        if (!ops->get_regs || !ops->get_regs_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&regs, useraddr, sizeof(regs)))
                return -EFAULT;

        reglen = ops->get_regs_len(dev);
        if (regs.len > reglen)
                regs.len = reglen;

        regbuf = kmalloc(reglen, GFP_USER);
        if (!regbuf)
                return -ENOMEM;

        ops->get_regs(dev, &regs, regbuf);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &regs, sizeof(regs)))
                goto out;
        useraddr += offsetof(struct ethtool_regs, data);
        if (copy_to_user(useraddr, regbuf, reglen))
                goto out;
        ret = 0;

out:
        kfree(regbuf);
        return ret;
}

static int ethtool_get_wol(struct net_device *dev, char *useraddr)
{
        struct ethtool_wolinfo wol = { ETHTOOL_GWOL };

        if (!ethtool_ops->get_wol)
                return -EOPNOTSUPP;

        ethtool_ops->get_wol(dev, &wol);

        if (copy_to_user(useraddr, &wol, sizeof(wol)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_wol(struct net_device *dev, char *useraddr)
{
        struct ethtool_wolinfo wol;

        if (!ethtool_ops->set_wol)
                return -EOPNOTSUPP;

        if (copy_from_user(&wol, useraddr, sizeof(wol)))
                return -EFAULT;

        return ethtool_ops->set_wol(dev, &wol);
}

static int ethtool_get_msglevel(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GMSGLVL };

        if (!ethtool_ops->get_msglevel)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_msglevel(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_msglevel(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_msglevel)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        ethtool_ops->set_msglevel(dev, edata.data);
        return 0;
}

static int ethtool_nway_reset(struct net_device *dev)
{
        if (!ethtool_ops->nway_reset)
                return -EOPNOTSUPP;

        return ethtool_ops->nway_reset(dev);
}

static int ethtool_get_link(struct net_device *dev, void *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GLINK };

        if (!ethtool_ops->get_link)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_link(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_get_eeprom(struct net_device *dev, void *useraddr)
{
        struct ethtool_eeprom eeprom;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->get_eeprom || !ops->get_eeprom_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
                return -EFAULT;

        /* Check for wrap and zero */
        if (eeprom.offset + eeprom.len <= eeprom.offset)
                return -EINVAL;

        /* Check for exceeding total eeprom len */
        if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
                return -EINVAL;

        data = kmalloc(eeprom.len, GFP_USER);
        if (!data)
                return -ENOMEM;

        ret = -EFAULT;
        if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
                goto out;

        ret = ops->get_eeprom(dev, &eeprom, data);
        if (ret)
                goto out;

        ret = -EFAULT;
        if (copy_to_user(useraddr, &eeprom, sizeof(eeprom)))
                goto out;
        if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_set_eeprom(struct net_device *dev, void *useraddr)
{
        struct ethtool_eeprom eeprom;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->set_eeprom || !ops->get_eeprom_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
                return -EFAULT;

        /* Check for wrap and zero */
        if (eeprom.offset + eeprom.len <= eeprom.offset)
                return -EINVAL;

        /* Check for exceeding total eeprom len */
        if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
                return -EINVAL;

        data = kmalloc(eeprom.len, GFP_USER);
        if (!data)
                return -ENOMEM;

        ret = -EFAULT;
        if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
                goto out;

        ret = ops->set_eeprom(dev, &eeprom, data);
        if (ret)
                goto out;

        if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
                ret = -EFAULT;

out:
        kfree(data);
        return ret;
}

static int ethtool_get_coalesce(struct net_device *dev, void *useraddr)
{
        struct ethtool_coalesce coalesce = { ETHTOOL_GCOALESCE };

        if (!ethtool_ops->get_coalesce)
                return -EOPNOTSUPP;

        ethtool_ops->get_coalesce(dev, &coalesce);

        if (copy_to_user(useraddr, &coalesce, sizeof(coalesce)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_coalesce(struct net_device *dev, void *useraddr)
{
        struct ethtool_coalesce coalesce;

        if (!ethtool_ops->get_coalesce)
                return -EOPNOTSUPP;

        if (copy_from_user(&coalesce, useraddr, sizeof(coalesce)))
                return -EFAULT;

        return ethtool_ops->set_coalesce(dev, &coalesce);
}

static int ethtool_get_ringparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_ringparam ringparam = { ETHTOOL_GRINGPARAM };

        if (!ethtool_ops->get_ringparam)
                return -EOPNOTSUPP;

        ethtool_ops->get_ringparam(dev, &ringparam);

        if (copy_to_user(useraddr, &ringparam, sizeof(ringparam)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_ringparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_ringparam ringparam;

        if (!ethtool_ops->get_ringparam)
                return -EOPNOTSUPP;

        if (copy_from_user(&ringparam, useraddr, sizeof(ringparam)))
                return -EFAULT;

        return ethtool_ops->set_ringparam(dev, &ringparam);
}

static int ethtool_get_pauseparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_pauseparam pauseparam = { ETHTOOL_GPAUSEPARAM };

        if (!ethtool_ops->get_pauseparam)
                return -EOPNOTSUPP;

        ethtool_ops->get_pauseparam(dev, &pauseparam);

        if (copy_to_user(useraddr, &pauseparam, sizeof(pauseparam)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_pauseparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_pauseparam pauseparam;

        if (!ethtool_ops->get_pauseparam)
                return -EOPNOTSUPP;

        if (copy_from_user(&pauseparam, useraddr, sizeof(pauseparam)))
                return -EFAULT;

        return ethtool_ops->set_pauseparam(dev, &pauseparam);
}

static int ethtool_get_rx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GRXCSUM };

        if (!ethtool_ops->get_rx_csum)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_rx_csum(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_rx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_rx_csum)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        ethtool_ops->set_rx_csum(dev, edata.data);
        return 0;
}

static int ethtool_get_tx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GTXCSUM };

        if (!ethtool_ops->get_tx_csum)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_tx_csum(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_tx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_tx_csum)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_tx_csum(dev, edata.data);
}

static int ethtool_get_sg(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GSG };

        if (!ethtool_ops->get_sg)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_sg(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_sg(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_sg)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_sg(dev, edata.data);
}

static int ethtool_get_tso(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GTSO };

        if (!ethtool_ops->get_tso)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_tso(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_tso(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_tso)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_tso(dev, edata.data);
}

static int ethtool_self_test(struct net_device *dev, char *useraddr)
{
        struct ethtool_test test;
        struct ethtool_ops *ops = ethtool_ops;
        u64 *data;
        int ret;

        if (!ops->self_test || !ops->self_test_count)
                return -EOPNOTSUPP;

        if (copy_from_user(&test, useraddr, sizeof(test)))
                return -EFAULT;

        test.len = ops->self_test_count(dev);
        data = kmalloc(test.len * sizeof(u64), GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->self_test(dev, &test, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &test, sizeof(test)))
                goto out;
        useraddr += sizeof(test);
        if (copy_to_user(useraddr, data, test.len * sizeof(u64)))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_get_strings(struct net_device *dev, void *useraddr)
{
        struct ethtool_gstrings gstrings;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->get_strings)
                return -EOPNOTSUPP;

        if (copy_from_user(&gstrings, useraddr, sizeof(gstrings)))
                return -EFAULT;

        switch (gstrings.string_set) {
        case ETH_SS_TEST:
                if (!ops->self_test_count)
                        return -EOPNOTSUPP;
                gstrings.len = ops->self_test_count(dev);
                break;
        case ETH_SS_STATS:
                if (!ops->get_stats_count)
                        return -EOPNOTSUPP;
                gstrings.len = ops->get_stats_count(dev);
                break;
        default:
                return -EINVAL;
        }

        data = kmalloc(gstrings.len * ETH_GSTRING_LEN, GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->get_strings(dev, gstrings.string_set, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &gstrings, sizeof(gstrings)))
                goto out;
        useraddr += sizeof(gstrings);
        if (copy_to_user(useraddr, data, gstrings.len * ETH_GSTRING_LEN))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_phys_id(struct net_device *dev, void *useraddr)
{
        struct ethtool_value id;

        if (!ethtool_ops->phys_id)
                return -EOPNOTSUPP;

        if (copy_from_user(&id, useraddr, sizeof(id)))
                return -EFAULT;

        return ethtool_ops->phys_id(dev, id.data);
}

static int ethtool_get_stats(struct net_device *dev, void *useraddr)
{
        struct ethtool_stats stats;
        struct ethtool_ops *ops = ethtool_ops;
        u64 *data;
        int ret;

        if (!ops->get_ethtool_stats || !ops->get_stats_count)
                return -EOPNOTSUPP;

        if (copy_from_user(&stats, useraddr, sizeof(stats)))
                return -EFAULT;

        stats.n_stats = ops->get_stats_count(dev);
        data = kmalloc(stats.n_stats * sizeof(u64), GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->get_ethtool_stats(dev, &stats, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &stats, sizeof(stats)))
                goto out;
        useraddr += sizeof(stats);
        if (copy_to_user(useraddr, data, stats.n_stats * sizeof(u64)))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_ioctl(struct ifreq *ifr)
{
        struct net_device *dev = __dev_get_by_name(ifr->ifr_name);
        void *useraddr = (void *) ifr->ifr_data;
        u32 ethcmd;

        /*
         * XXX: This can be pushed down into the ethtool_* handlers that
         * need it.  Keep existing behaviour for the moment.
         */
        if (!capable(CAP_NET_ADMIN))
                return -EPERM;

        if (!dev || !netif_device_present(dev))
                return -ENODEV;

        if (copy_from_user(&ethcmd, useraddr, sizeof (ethcmd)))
                return -EFAULT;

        switch (ethcmd) {
        case ETHTOOL_GSET:
                return ethtool_get_settings(dev, useraddr);
        case ETHTOOL_SSET:
                return ethtool_set_settings(dev, useraddr);
        case ETHTOOL_GDRVINFO:
                return ethtool_get_drvinfo(dev, useraddr);
        case ETHTOOL_GREGS:
                return ethtool_get_regs(dev, useraddr);
        case ETHTOOL_GWOL:
                return ethtool_get_wol(dev, useraddr);
        case ETHTOOL_SWOL:
                return ethtool_set_wol(dev, useraddr);
        case ETHTOOL_GMSGLVL:
                return ethtool_get_msglevel(dev, useraddr);
        case ETHTOOL_SMSGLVL:
                return ethtool_set_msglevel(dev, useraddr);
        case ETHTOOL_NWAY_RST:
                return ethtool_nway_reset(dev);
        case ETHTOOL_GLINK:
                return ethtool_get_link(dev, useraddr);
        case ETHTOOL_GEEPROM:
                return ethtool_get_eeprom(dev, useraddr);
        case ETHTOOL_SEEPROM:
                return ethtool_set_eeprom(dev, useraddr);
        case ETHTOOL_GCOALESCE:
                return ethtool_get_coalesce(dev, useraddr);
        case ETHTOOL_SCOALESCE:
                return ethtool_set_coalesce(dev, useraddr);
        case ETHTOOL_GRINGPARAM:
                return ethtool_get_ringparam(dev, useraddr);
        case ETHTOOL_SRINGPARAM:
                return ethtool_set_ringparam(dev, useraddr);
        case ETHTOOL_GPAUSEPARAM:
                return ethtool_get_pauseparam(dev, useraddr);
        case ETHTOOL_SPAUSEPARAM:
                return ethtool_set_pauseparam(dev, useraddr);
        case ETHTOOL_GRXCSUM:
                return ethtool_get_rx_csum(dev, useraddr);
        case ETHTOOL_SRXCSUM:
                return ethtool_set_rx_csum(dev, useraddr);
        case ETHTOOL_GTXCSUM:
                return ethtool_get_tx_csum(dev, useraddr);
        case ETHTOOL_STXCSUM:
                return ethtool_set_tx_csum(dev, useraddr);
        case ETHTOOL_GSG:
                return ethtool_get_sg(dev, useraddr);
        case ETHTOOL_SSG:
                return ethtool_set_sg(dev, useraddr);
        case ETHTOOL_GTSO:
                return ethtool_get_tso(dev, useraddr);
        case ETHTOOL_STSO:
                return ethtool_set_tso(dev, useraddr);
        case ETHTOOL_TEST:
                return ethtool_self_test(dev, useraddr);
        case ETHTOOL_GSTRINGS:
                return ethtool_get_strings(dev, useraddr);
        case ETHTOOL_PHYS_ID:
                return ethtool_phys_id(dev, useraddr);
        case ETHTOOL_GSTATS:
                return ethtool_get_stats(dev, useraddr);
        default:
                return -EOPNOTSUPP;
        }

        return -EOPNOTSUPP;
}
#endif //ETHTOOL_OPS_COMPAT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
static int rtl8125_siocdevprivate(struct net_device *dev, struct ifreq *ifr,
                                  void __user *data, int cmd)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret = 0;

        switch (cmd) {
#ifdef ENABLE_DASH_SUPPORT
        case SIOCDEVPRIVATE_RTLDASH:
                if (!netif_running(dev)) {
                        ret = -ENODEV;
                        break;
                }
                if (!capable(CAP_NET_ADMIN)) {
                        ret = -EPERM;
                        break;
                }

                ret = rtl8125_dash_ioctl(dev, ifr);
                break;
#endif

#ifdef ENABLE_REALWOW_SUPPORT
        case SIOCDEVPRIVATE_RTLREALWOW:
                if (!netif_running(dev)) {
                        ret = -ENODEV;
                        break;
                }

                ret = rtl8125_realwow_ioctl(dev, ifr);
                break;
#endif

        case SIOCRTLTOOL:
                if (!capable(CAP_NET_ADMIN)) {
                        ret = -EPERM;
                        break;
                }

                ret = rtl8125_tool_ioctl(tp, ifr);
                break;

        default:
                ret = -EOPNOTSUPP;
        }

        return ret;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)

static int
rtl8125_do_ioctl(struct net_device *dev,
                 struct ifreq *ifr,
                 int cmd)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct mii_ioctl_data *data = if_mii(ifr);
        int ret = 0;

        switch (cmd) {
        case SIOCGMIIPHY:
                data->phy_id = 32; /* Internal PHY */
                break;

        case SIOCGMIIREG:
                rtl8125_mdio_write(tp, 0x1F, 0x0000);
                data->val_out = rtl8125_mdio_read(tp, data->reg_num);
                break;

        case SIOCSMIIREG:
                if (!capable(CAP_NET_ADMIN))
                        return -EPERM;
                rtl8125_mdio_write(tp, 0x1F, 0x0000);
                rtl8125_mdio_write(tp, data->reg_num, data->val_in);
                break;

#ifdef ETHTOOL_OPS_COMPAT
        case SIOCETHTOOL:
                ret = ethtool_ioctl(ifr);
                break;
#endif

#ifdef ENABLE_PTP_SUPPORT
        case SIOCSHWTSTAMP:
        case SIOCGHWTSTAMP:
                if (tp->EnablePtp)
                        ret = rtl8125_ptp_ioctl(dev, ifr, cmd);
                else
                        ret = -EOPNOTSUPP;
                break;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
#ifdef ENABLE_DASH_SUPPORT
        case SIOCDEVPRIVATE_RTLDASH:
                if (!netif_running(dev)) {
                        ret = -ENODEV;
                        break;
                }
                if (!capable(CAP_NET_ADMIN)) {
                        ret = -EPERM;
                        break;
                }

                ret = rtl8125_dash_ioctl(dev, ifr);
                break;
#endif

#ifdef ENABLE_REALWOW_SUPPORT
        case SIOCDEVPRIVATE_RTLREALWOW:
                if (!netif_running(dev)) {
                        ret = -ENODEV;
                        break;
                }

                if (!capable(CAP_NET_ADMIN)) {
                        ret = -EPERM;
                        break;
                }

                ret = rtl8125_realwow_ioctl(dev, ifr);
                break;
#endif

        case SIOCRTLTOOL:
                if (!capable(CAP_NET_ADMIN)) {
                        ret = -EPERM;
                        break;
                }

                ret = rtl8125_tool_ioctl(tp, ifr);
                break;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)

        default:
                ret = -EOPNOTSUPP;
                break;
        }

        return ret;
}

static void
rtl8125_phy_power_up(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (rtl8125_is_in_phy_disable_mode(dev)) {
                return;
        }

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        rtl8125_mdio_write(tp, MII_BMCR, BMCR_ANENABLE);

        //wait ups resume (phy state 3)
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_wait_phy_ups_resume(dev, 3);
                break;
        };
}

static void
rtl8125_phy_power_down(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        rtl8125_mdio_write(tp, MII_BMCR, BMCR_ANENABLE | BMCR_PDOWN);
}

static int __devinit
rtl8125_init_board(struct pci_dev *pdev,
                   struct net_device **dev_out,
                   void __iomem **ioaddr_out)
{
        void __iomem *ioaddr;
        struct net_device *dev;
        struct rtl8125_private *tp;
        int rc = -ENOMEM, i, pm_cap;

        assert(ioaddr_out != NULL);

        /* dev zeroed in alloc_etherdev */
        dev = alloc_etherdev_mq(sizeof (*tp), R8125_MAX_QUEUES);
        if (dev == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_drv(&debug))
                        dev_err(&pdev->dev, "unable to alloc new ethernet\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out;
        }

        SET_MODULE_OWNER(dev);
        SET_NETDEV_DEV(dev, &pdev->dev);
        tp = netdev_priv(dev);
        tp->dev = dev;
        tp->pci_dev = pdev;
        tp->msg_enable = netif_msg_init(debug.msg_enable, R8125_MSG_DEFAULT);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
        if (!aspm)
                pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S | PCIE_LINK_STATE_L1 |
                                       PCIE_LINK_STATE_CLKPM);
#endif

        /* enable device (incl. PCI PM wakeup and hotplug setup) */
        rc = pci_enable_device(pdev);
        if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "enable failure\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out_free_dev;
        }

        if (pci_set_mwi(pdev) < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_drv(&debug))
                        dev_info(&pdev->dev, "Mem-Wr-Inval unavailable.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        }

        /* save power state before pci_enable_device overwrites it */
        pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
        if (pm_cap) {
                u16 pwr_command;

                pci_read_config_word(pdev, pm_cap + PCI_PM_CTRL, &pwr_command);
        } else {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp)) {
                        dev_err(&pdev->dev, "PowerManagement capability not found.\n");
                }
#else
                printk("PowerManagement capability not found.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

        }

        /* make sure PCI base addr 1 is MMIO */
        if (!(pci_resource_flags(pdev, 2) & IORESOURCE_MEM)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "region #1 not an MMIO resource, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -ENODEV;
                goto err_out_mwi;
        }
        /* check for weird/broken PCI region reporting */
        if (pci_resource_len(pdev, 2) < R8125_REGS_SIZE) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "Invalid PCI region size(s), aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -ENODEV;
                goto err_out_mwi;
        }

        rc = pci_request_regions(pdev, MODULENAME);
        if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "could not request regions.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out_mwi;
        }

        if ((sizeof(dma_addr_t) > 4) &&
            use_dac &&
            !dma_set_mask(&pdev->dev, DMA_BIT_MASK(64)) &&
            !dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(64))) {
                dev->features |= NETIF_F_HIGHDMA;
        } else {
                rc = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
                if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        if (netif_msg_probe(tp))
                                dev_err(&pdev->dev, "DMA configuration failed.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        goto err_out_free_res;
                }
        }

        /* ioremap MMIO region */
        ioaddr = ioremap(pci_resource_start(pdev, 2), pci_resource_len(pdev, 2));
        if (ioaddr == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "cannot remap MMIO, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -EIO;
                goto err_out_free_res;
        }

        tp->mmio_addr = ioaddr;

        /* Identify chip attached to board */
        rtl8125_get_mac_version(tp);

        rtl8125_print_mac_version(tp);

        for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
                if (tp->mcfg == rtl_chip_info[i].mcfg)
                        break;
        }

        if (i < 0) {
                /* Unknown chip: assume array element #0, original RTL-8125 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_printk(KERN_DEBUG, &pdev->dev, "unknown chip version, assuming %s\n", rtl_chip_info[0].name);
#else
                printk("Realtek unknown chip version, assuming %s\n", rtl_chip_info[0].name);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
                i++;
        }

        tp->chipset = i;

        *ioaddr_out = ioaddr;
        *dev_out = dev;
out:
        return rc;

err_out_free_res:
        pci_release_regions(pdev);
err_out_mwi:
        pci_clear_mwi(pdev);
        pci_disable_device(pdev);
err_out_free_dev:
        free_netdev(dev);
err_out:
        *ioaddr_out = NULL;
        *dev_out = NULL;
        goto out;
}

static void
rtl8125_esd_checker(struct rtl8125_private *tp)
{
        struct net_device *dev = tp->dev;
        struct pci_dev *pdev = tp->pci_dev;
        u8 cmd;
        u16 io_base_l;
        u16 mem_base_l;
        u16 mem_base_h;
        u8 ilr;
        u16 resv_0x1c_h;
        u16 resv_0x1c_l;
        u16 resv_0x20_l;
        u16 resv_0x20_h;
        u16 resv_0x24_l;
        u16 resv_0x24_h;
        u16 resv_0x2c_h;
        u16 resv_0x2c_l;
        u32 pci_sn_l;
        u32 pci_sn_h;

        if (unlikely(tp->rtk_enable_diag))
                goto exit;

        tp->esd_flag = 0;

        pci_read_config_byte(pdev, PCI_COMMAND, &cmd);
        if (cmd != tp->pci_cfg_space.cmd) {
                printk(KERN_ERR "%s: cmd = 0x%02x, should be 0x%02x \n.", dev->name, cmd, tp->pci_cfg_space.cmd);
                pci_write_config_byte(pdev, PCI_COMMAND, tp->pci_cfg_space.cmd);
                tp->esd_flag |= BIT_0;

                pci_read_config_byte(pdev, PCI_COMMAND, &cmd);
                if (cmd == 0xff) {
                        printk(KERN_ERR "%s: pci link is down \n.", dev->name);
                        goto exit;
                }
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &io_base_l);
        if (io_base_l != tp->pci_cfg_space.io_base_l) {
                printk(KERN_ERR "%s: io_base_l = 0x%04x, should be 0x%04x \n.", dev->name, io_base_l, tp->pci_cfg_space.io_base_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_0, tp->pci_cfg_space.io_base_l);
                tp->esd_flag |= BIT_1;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &mem_base_l);
        if (mem_base_l != tp->pci_cfg_space.mem_base_l) {
                printk(KERN_ERR "%s: mem_base_l = 0x%04x, should be 0x%04x \n.", dev->name, mem_base_l, tp->pci_cfg_space.mem_base_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_2, tp->pci_cfg_space.mem_base_l);
                tp->esd_flag |= BIT_2;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &mem_base_h);
        if (mem_base_h!= tp->pci_cfg_space.mem_base_h) {
                printk(KERN_ERR "%s: mem_base_h = 0x%04x, should be 0x%04x \n.", dev->name, mem_base_h, tp->pci_cfg_space.mem_base_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, tp->pci_cfg_space.mem_base_h);
                tp->esd_flag |= BIT_3;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_3, &resv_0x1c_l);
        if (resv_0x1c_l != tp->pci_cfg_space.resv_0x1c_l) {
                printk(KERN_ERR "%s: resv_0x1c_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x1c_l, tp->pci_cfg_space.resv_0x1c_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_3, tp->pci_cfg_space.resv_0x1c_l);
                tp->esd_flag |= BIT_4;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, &resv_0x1c_h);
        if (resv_0x1c_h != tp->pci_cfg_space.resv_0x1c_h) {
                printk(KERN_ERR "%s: resv_0x1c_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x1c_h, tp->pci_cfg_space.resv_0x1c_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, tp->pci_cfg_space.resv_0x1c_h);
                tp->esd_flag |= BIT_5;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &resv_0x20_l);
        if (resv_0x20_l != tp->pci_cfg_space.resv_0x20_l) {
                printk(KERN_ERR "%s: resv_0x20_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x20_l, tp->pci_cfg_space.resv_0x20_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_4, tp->pci_cfg_space.resv_0x20_l);
                tp->esd_flag |= BIT_6;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &resv_0x20_h);
        if (resv_0x20_h != tp->pci_cfg_space.resv_0x20_h) {
                printk(KERN_ERR "%s: resv_0x20_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x20_h, tp->pci_cfg_space.resv_0x20_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, tp->pci_cfg_space.resv_0x20_h);
                tp->esd_flag |= BIT_7;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &resv_0x24_l);
        if (resv_0x24_l != tp->pci_cfg_space.resv_0x24_l) {
                printk(KERN_ERR "%s: resv_0x24_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x24_l, tp->pci_cfg_space.resv_0x24_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_5, tp->pci_cfg_space.resv_0x24_l);
                tp->esd_flag |= BIT_8;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &resv_0x24_h);
        if (resv_0x24_h != tp->pci_cfg_space.resv_0x24_h) {
                printk(KERN_ERR "%s: resv_0x24_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x24_h, tp->pci_cfg_space.resv_0x24_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, tp->pci_cfg_space.resv_0x24_h);
                tp->esd_flag |= BIT_9;
        }

        pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &ilr);
        if (ilr != tp->pci_cfg_space.ilr) {
                printk(KERN_ERR "%s: ilr = 0x%02x, should be 0x%02x \n.", dev->name, ilr, tp->pci_cfg_space.ilr);
                pci_write_config_byte(pdev, PCI_INTERRUPT_LINE, tp->pci_cfg_space.ilr);
                tp->esd_flag |= BIT_10;
        }

        pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &resv_0x2c_l);
        if (resv_0x2c_l != tp->pci_cfg_space.resv_0x2c_l) {
                printk(KERN_ERR "%s: resv_0x2c_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x2c_l, tp->pci_cfg_space.resv_0x2c_l);
                pci_write_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, tp->pci_cfg_space.resv_0x2c_l);
                tp->esd_flag |= BIT_11;
        }

        pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, &resv_0x2c_h);
        if (resv_0x2c_h != tp->pci_cfg_space.resv_0x2c_h) {
                printk(KERN_ERR "%s: resv_0x2c_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x2c_h, tp->pci_cfg_space.resv_0x2c_h);
                pci_write_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, tp->pci_cfg_space.resv_0x2c_h);
                tp->esd_flag |= BIT_12;
        }

        if (tp->HwPcieSNOffset > 0) {
                pci_sn_l = rtl8125_csi_read(tp, tp->HwPcieSNOffset);
                if (pci_sn_l != tp->pci_cfg_space.pci_sn_l) {
                        printk(KERN_ERR "%s: pci_sn_l = 0x%08x, should be 0x%08x \n.", dev->name, pci_sn_l, tp->pci_cfg_space.pci_sn_l);
                        rtl8125_csi_write(tp, tp->HwPcieSNOffset, tp->pci_cfg_space.pci_sn_l);
                        tp->esd_flag |= BIT_13;
                }

                pci_sn_h = rtl8125_csi_read(tp, tp->HwPcieSNOffset + 4);
                if (pci_sn_h != tp->pci_cfg_space.pci_sn_h) {
                        printk(KERN_ERR "%s: pci_sn_h = 0x%08x, should be 0x%08x \n.", dev->name, pci_sn_h, tp->pci_cfg_space.pci_sn_h);
                        rtl8125_csi_write(tp, tp->HwPcieSNOffset + 4, tp->pci_cfg_space.pci_sn_h);
                        tp->esd_flag |= BIT_14;
                }
        }

        if (tp->esd_flag != 0) {
                printk(KERN_ERR "%s: esd_flag = 0x%04x\n.\n", dev->name, tp->esd_flag);
                netif_carrier_off(dev);
                netif_tx_disable(dev);
                rtl8125_hw_reset(dev);
                rtl8125_tx_clear(tp);
                rtl8125_rx_clear(tp);
                rtl8125_init_ring(dev);
                rtl8125_up(dev);
                rtl8125_enable_hw_linkchg_interrupt(tp);
                rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
                tp->esd_flag = 0;
        }
exit:
        return;
}
/*
static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
rtl8125_esd_timer(unsigned long __opaque)
#else
rtl8125_esd_timer(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        struct net_device *dev = (struct net_device *)__opaque;
        struct rtl8125_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->esd_timer;
#else
        struct rtl8125_private *tp = from_timer(tp, t, esd_timer);
        //struct net_device *dev = tp->dev;
        struct timer_list *timer = t;
#endif
        rtl8125_esd_checker(tp);

        mod_timer(timer, jiffies + timeout);
}
*/

/*
static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
rtl8125_link_timer(unsigned long __opaque)
#else
rtl8125_link_timer(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        struct net_device *dev = (struct net_device *)__opaque;
        struct rtl8125_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->link_timer;
#else
        struct rtl8125_private *tp = from_timer(tp, t, link_timer);
        struct net_device *dev = tp->dev;
        struct timer_list *timer = t;
#endif
        rtl8125_check_link_status(dev);

        mod_timer(timer, jiffies + RTL8125_LINK_TIMEOUT);
}
*/

int
rtl8125_enable_msix(struct rtl8125_private *tp)
{
        int i, nvecs = 0;
        struct msix_entry msix_ent[R8125_MAX_MSIX_VEC];
        //struct net_device *dev = tp->dev;
        //const int len = sizeof(tp->irq_tbl[0].name);

        for (i = 0; i < R8125_MAX_MSIX_VEC; i++) {
                msix_ent[i].entry = i;
                msix_ent[i].vector = 0;
        }

        nvecs = pci_enable_msix_range(tp->pci_dev, msix_ent,
                                      tp->min_irq_nvecs, tp->max_irq_nvecs);
        if (nvecs < 0)
                goto out;

        for (i = 0; i < nvecs; i++) {
                struct r8125_irq *irq = &tp->irq_tbl[i];
                irq->vector = msix_ent[i].vector;
                //snprintf(irq->name, len, "%s-%d", dev->name, i);
                //irq->handler = rtl8125_interrupt_msix;
        }

out:
        return nvecs;
}

void rtl8125_dump_msix_tbl(struct rtl8125_private *tp)
{
        void __iomem *ioaddr;

        /* ioremap MMIO region */
        ioaddr = ioremap(pci_resource_start(tp->pci_dev, 4), pci_resource_len(tp->pci_dev, 4));
        if (ioaddr) {
                int i = 0;
                for (i=0; i<tp->irq_nvecs; i++) {
                        printk("entry 0x%d %08X %08X %08X %08X \n",
                               i,
                               readl(ioaddr + 16 * i),
                               readl(ioaddr + 16 * i + 4),
                               readl(ioaddr + 16 * i + 8),
                               readl(ioaddr + 16 * i + 12));
                }
                iounmap(ioaddr);
        }
}

/* Cfg9346_Unlock assumed. */
static int rtl8125_try_msi(struct rtl8125_private *tp)
{
        struct pci_dev *pdev = tp->pci_dev;
        unsigned msi = 0;
        int nvecs = 1;

        tp->max_irq_nvecs = 1;
        tp->min_irq_nvecs = 1;
#ifndef DISABLE_MULTI_MSIX_VECTOR
        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                tp->max_irq_nvecs = R8125_MAX_MSIX_VEC_8125B;
                tp->min_irq_nvecs = R8125_MIN_MSIX_VEC_8125B;
                break;
        }
#endif

#if defined(RTL_USE_NEW_INTR_API)
        if ((nvecs = pci_alloc_irq_vectors(pdev, tp->min_irq_nvecs, tp->max_irq_nvecs, PCI_IRQ_MSIX)) > 0)
                msi |= RTL_FEATURE_MSIX;
        else if ((nvecs = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_ALL_TYPES)) > 0 &&
                 pci_dev_msi_enabled(pdev))
                msi |= RTL_FEATURE_MSI;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        if ((nvecs = rtl8125_enable_msix(tp)) > 0)
                msi |= RTL_FEATURE_MSIX;
        else if (!pci_enable_msi(pdev))
                msi |= RTL_FEATURE_MSI;
#endif
        if (!(msi & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX)))
                dev_info(&pdev->dev, "no MSI/MSI-X. Back to INTx.\n");

        if (!(msi & RTL_FEATURE_MSIX) || nvecs < 1)
                nvecs = 1;

        tp->irq_nvecs = nvecs;

        tp->features |= msi;

        return nvecs;
}

static void rtl8125_disable_msi(struct pci_dev *pdev, struct rtl8125_private *tp)
{
#if defined(RTL_USE_NEW_INTR_API)
        if (tp->features & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX))
                pci_free_irq_vectors(pdev);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        if (tp->features & (RTL_FEATURE_MSIX))
                pci_disable_msix(pdev);
        else if (tp->features & (RTL_FEATURE_MSI))
                pci_disable_msi(pdev);
#endif
        tp->features &= ~(RTL_FEATURE_MSI | RTL_FEATURE_MSIX);
}

static int rtl8125_get_irq(struct pci_dev *pdev)
{
#if defined(RTL_USE_NEW_INTR_API)
        return pci_irq_vector(pdev, 0);
#else
        return pdev->irq;
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static void
rtl8125_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct rtl8125_counters *counters = tp->tally_vaddr;
        dma_addr_t paddr = tp->tally_paddr;

        if (!counters)
                return;

        netdev_stats_to_stats64(stats, &dev->stats);
        dev_fetch_sw_netstats(stats, dev->tstats);

        /*
         * Fetch additional counter values missing in stats collected by driver
         * from tally counters.
         */
        rtl8125_dump_tally_counter(tp, paddr);

        stats->tx_errors = le64_to_cpu(counters->tx_errors);
        stats->collisions = le32_to_cpu(counters->tx_multi_collision);
        stats->tx_aborted_errors = le16_to_cpu(counters->tx_aborted) ;
        stats->rx_missed_errors = le16_to_cpu(counters->rx_missed);
}
#else
/**
 *  rtl8125_get_stats - Get rtl8125 read/write statistics
 *  @dev: The Ethernet Device to get statistics for
 *
 *  Get TX/RX statistics for rtl8125
 */
static struct
net_device_stats *rtl8125_get_stats(struct net_device *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
        struct rtl8125_private *tp = netdev_priv(dev);
#endif
        return &RTLDEV->stats;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static const struct net_device_ops rtl8125_netdev_ops = {
        .ndo_open       = rtl8125_open,
        .ndo_stop       = rtl8125_close,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
        .ndo_get_stats64    = rtl8125_get_stats64,
#else
        .ndo_get_stats      = rtl8125_get_stats,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
        .ndo_start_xmit     = rtl8125_start_xmit,
        .ndo_tx_timeout     = rtl8125_tx_timeout,
        .ndo_change_mtu     = rtl8125_change_mtu,
        .ndo_set_mac_address    = rtl8125_set_mac_address,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
        .ndo_do_ioctl       = rtl8125_do_ioctl,
#else
        .ndo_siocdevprivate = rtl8125_siocdevprivate,
        .ndo_eth_ioctl      = rtl8125_do_ioctl,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,1,0)
        .ndo_set_multicast_list = rtl8125_set_rx_mode,
#else
        .ndo_set_rx_mode    = rtl8125_set_rx_mode,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#ifdef CONFIG_R8125_VLAN
        .ndo_vlan_rx_register   = rtl8125_vlan_rx_register,
#endif
#else
        .ndo_fix_features   = rtl8125_fix_features,
        .ndo_set_features   = rtl8125_set_features,
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller    = rtl8125_netpoll,
#endif
};
#endif


#ifdef  CONFIG_R8125_NAPI

static int rtl8125_poll(napi_ptr napi, napi_budget budget)
{
        struct r8125_napi *r8125napi = RTL_GET_PRIV(napi, struct r8125_napi);
        struct rtl8125_private *tp = r8125napi->priv;
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        unsigned int work_done = 0;
        int i;

        for (i = 0; i < tp->num_tx_rings; i++)
                rtl8125_tx_interrupt(&tp->tx_ring[i], budget);

        for (i = 0; i < tp->num_rx_rings; i++)
                work_done += rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[i], budget);

        RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

        if (work_done < work_to_do) {
#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH)
                        HandleDashInterrupt(tp->dev);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
                if (RTL_NETIF_RX_COMPLETE(dev, napi, work_done) == FALSE) return RTL_NAPI_RETURN_VALUE;
#else
                RTL_NETIF_RX_COMPLETE(dev, napi, work_done);
#endif
                /*
                 * 20040426: the barrier is not strictly required but the
                 * behavior of the irq handler could be less predictable
                 * without it. Btw, the lack of flush for the posted pci
                 * write is safe - FR
                 */
                smp_wmb();

                rtl8125_switch_to_timer_interrupt(tp);
        }

        return RTL_NAPI_RETURN_VALUE;
}

#if 0
static int rtl8125_poll_msix_ring(napi_ptr napi, napi_budget budget)
{
        struct r8125_napi *r8125napi = RTL_GET_PRIV(napi, struct r8125_napi);
        struct rtl8125_private *tp = r8125napi->priv;
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        unsigned int work_done = 0;
        const int message_id = r8125napi->index;

        rtl8125_tx_interrupt_with_vector(tp, message_id, budget);

        work_done += rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[message_id], budget);

        RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

        if (work_done < work_to_do) {
#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH && message_id == 0)
                        HandleDashInterrupt(tp->dev);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
                if (RTL_NETIF_RX_COMPLETE(dev, napi, work_done) == FALSE) return RTL_NAPI_RETURN_VALUE;
#else
                RTL_NETIF_RX_COMPLETE(dev, napi, work_done);
#endif
                /*
                 * 20040426: the barrier is not strictly required but the
                 * behavior of the irq handler could be less predictable
                 * without it. Btw, the lack of flush for the posted pci
                 * write is safe - FR
                 */
                smp_wmb();

                rtl8125_enable_hw_interrupt_v2(tp, message_id);
        }

        return RTL_NAPI_RETURN_VALUE;
}
#endif

static int rtl8125_poll_msix_tx(napi_ptr napi, napi_budget budget)
{
        struct r8125_napi *r8125napi = RTL_GET_PRIV(napi, struct r8125_napi);
        struct rtl8125_private *tp = r8125napi->priv;
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        unsigned int work_done = 0;
        const int message_id = r8125napi->index;

        //suppress unused variable
        (void)(dev);

        rtl8125_tx_interrupt_with_vector(tp, message_id, budget);

        RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

        if (work_done < work_to_do) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
                if (RTL_NETIF_RX_COMPLETE(dev, napi, work_done) == FALSE) return RTL_NAPI_RETURN_VALUE;
#else
                RTL_NETIF_RX_COMPLETE(dev, napi, work_done);
#endif
                /*
                 * 20040426: the barrier is not strictly required but the
                 * behavior of the irq handler could be less predictable
                 * without it. Btw, the lack of flush for the posted pci
                 * write is safe - FR
                 */
                smp_wmb();

                rtl8125_enable_hw_interrupt_v2(tp, message_id);
        }

        return RTL_NAPI_RETURN_VALUE;
}

static int rtl8125_poll_msix_other(napi_ptr napi, napi_budget budget)
{
        struct r8125_napi *r8125napi = RTL_GET_PRIV(napi, struct r8125_napi);
        struct rtl8125_private *tp = r8125napi->priv;
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        const int message_id = r8125napi->index;

        //suppress unused variable
        (void)(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
        RTL_NETIF_RX_COMPLETE(dev, napi, work_to_do);
#else
        RTL_NETIF_RX_COMPLETE(dev, napi, work_to_do);
#endif

        rtl8125_enable_hw_interrupt_v2(tp, message_id);

        return 1;
}

static int rtl8125_poll_msix_rx(napi_ptr napi, napi_budget budget)
{
        struct r8125_napi *r8125napi = RTL_GET_PRIV(napi, struct r8125_napi);
        struct rtl8125_private *tp = r8125napi->priv;
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        unsigned int work_done = 0;
        const int message_id = r8125napi->index;

        work_done += rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[message_id], budget);

        RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

        if (work_done < work_to_do) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
                if (RTL_NETIF_RX_COMPLETE(dev, napi, work_done) == FALSE) return RTL_NAPI_RETURN_VALUE;
#else
                RTL_NETIF_RX_COMPLETE(dev, napi, work_done);
#endif
                /*
                 * 20040426: the barrier is not strictly required but the
                 * behavior of the irq handler could be less predictable
                 * without it. Btw, the lack of flush for the posted pci
                 * write is safe - FR
                 */
                smp_wmb();

                rtl8125_enable_hw_interrupt_v2(tp, message_id);
        }

        return RTL_NAPI_RETURN_VALUE;
}

void rtl8125_enable_napi(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        int i;

        for (i = 0; i < tp->irq_nvecs; i++)
                RTL_NAPI_ENABLE(tp->dev, &tp->r8125napi[i].napi);
#endif
}

static void rtl8125_disable_napi(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        int i;

        for (i = 0; i < tp->irq_nvecs; i++)
                RTL_NAPI_DISABLE(tp->dev, &tp->r8125napi[i].napi);
#endif
}

static void rtl8125_del_napi(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        int i;

        for (i = 0; i < tp->irq_nvecs; i++)
                RTL_NAPI_DEL((&tp->r8125napi[i]));
#endif
}
#endif //CONFIG_R8125_NAPI

static void rtl8125_init_napi(struct rtl8125_private *tp)
{
        int i;

        for (i=0; i<tp->irq_nvecs; i++) {
                struct r8125_napi *r8125napi = &tp->r8125napi[i];
#ifdef CONFIG_R8125_NAPI
                int (*poll)(struct napi_struct *, int);

                if (tp->features & RTL_FEATURE_MSIX &&
                    tp->HwCurrIsrVer == 2) {
                        if (i < R8125_MAX_RX_QUEUES_VEC_V3)
                                poll = rtl8125_poll_msix_rx;
                        else if (i == 16 || i == 18)
                                poll = rtl8125_poll_msix_tx;
                        else
                                poll = rtl8125_poll_msix_other;
                } else {
                        poll = rtl8125_poll;
                }

                RTL_NAPI_CONFIG(tp->dev, r8125napi, poll, R8125_NAPI_WEIGHT);
#endif

                r8125napi->priv = tp;
                r8125napi->index = i;
        }
}

static int
rtl8125_set_real_num_queue(struct rtl8125_private *tp)
{
        int retval = 0;

        retval = netif_set_real_num_tx_queues(tp->dev, tp->num_tx_rings);
        if (retval < 0)
                goto exit;

        retval = netif_set_real_num_rx_queues(tp->dev, tp->num_rx_rings);
        if (retval < 0)
                goto exit;

exit:
        return retval;
}

static int __devinit
rtl8125_init_one(struct pci_dev *pdev,
                 const struct pci_device_id *ent)
{
        struct net_device *dev = NULL;
        struct rtl8125_private *tp;
        void __iomem *ioaddr = NULL;
        static int board_idx = -1;

        int rc;

        assert(pdev != NULL);
        assert(ent != NULL);

        board_idx++;

        if (netif_msg_drv(&debug))
                printk(KERN_INFO "%s 2.5Gigabit Ethernet driver %s loaded\n",
                       MODULENAME, RTL8125_VERSION);

        rc = rtl8125_init_board(pdev, &dev, &ioaddr);
        if (rc)
                goto out;

        tp = netdev_priv(dev);
        assert(ioaddr != NULL);

        tp->set_speed = rtl8125_set_speed_xmii;
        tp->get_settings = rtl8125_gset_xmii;
        tp->phy_reset_enable = rtl8125_xmii_reset_enable;
        tp->phy_reset_pending = rtl8125_xmii_reset_pending;
        tp->link_ok = rtl8125_xmii_link_ok;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
        dev->tstats = devm_netdev_alloc_pcpu_stats(&pdev->dev,
                        struct pcpu_sw_netstats);
        if (!dev->tstats)
                goto err_out_1;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)

        rc = rtl8125_try_msi(tp);
        if (rc < 0) {
                dev_err(&pdev->dev, "Can't allocate interrupt\n");
                goto err_out_1;
        }
#ifdef ENABLE_PTP_SUPPORT
        spin_lock_init(&tp->lock);
#endif
        rtl8125_init_software_variable(dev);

        RTL_NET_DEVICE_OPS(rtl8125_netdev_ops);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
        SET_ETHTOOL_OPS(dev, &rtl8125_ethtool_ops);
#endif

        dev->watchdog_timeo = RTL8125_TX_TIMEOUT;
        dev->irq = rtl8125_get_irq(pdev);
        dev->base_addr = (unsigned long) ioaddr;

        rtl8125_init_napi(tp);

#ifdef CONFIG_R8125_VLAN
        if (tp->mcfg != CFG_METHOD_DEFAULT) {
                dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
                dev->vlan_rx_kill_vid = rtl8125_vlan_rx_kill_vid;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
        }
#endif

        /* There has been a number of reports that using SG/TSO results in
         * tx timeouts. However for a lot of people SG/TSO works fine.
         * Therefore disable both features by default, but allow users to
         * enable them. Use at own risk!
         */
        tp->cp_cmd |= RTL_R16(tp, CPlusCmd);
        if (tp->mcfg != CFG_METHOD_DEFAULT) {
                dev->features |= NETIF_F_IP_CSUM;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
                tp->cp_cmd |= RxChkSum;
#else
                dev->features |= NETIF_F_RXCSUM;
                dev->hw_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_TSO |
                                   NETIF_F_RXCSUM | NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
                dev->vlan_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_TSO |
                                     NETIF_F_HIGHDMA;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                dev->hw_features |= NETIF_F_RXALL;
                dev->hw_features |= NETIF_F_RXFCS;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
                dev->hw_features |= NETIF_F_IPV6_CSUM | NETIF_F_TSO6;
                dev->features |=  NETIF_F_IPV6_CSUM;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                netif_set_tso_max_size(dev, LSO_64K);
                netif_set_tso_max_segs(dev, NIC_MAX_PHYS_BUF_COUNT_LSO2);
#else //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                netif_set_gso_max_size(dev, LSO_64K);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
                dev->gso_max_segs = NIC_MAX_PHYS_BUF_COUNT_LSO2;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
                dev->gso_min_segs = NIC_MIN_PHYS_BUF_COUNT;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)

#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

#ifdef ENABLE_RSS_SUPPORT
                if (tp->EnableRss) {
                        dev->hw_features |= NETIF_F_RXHASH;
                        dev->features |=  NETIF_F_RXHASH;
                }
#endif
        }

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH)
                AllocateDashShareMemory(dev);
#endif

#ifdef ENABLE_LIB_SUPPORT
        ATOMIC_INIT_NOTIFIER_HEAD(&tp->lib_nh);
#endif
        rtl8125_init_all_schedule_work(tp);

        rc = rtl8125_set_real_num_queue(tp);
        if (rc < 0)
                goto err_out;

        rtl8125_exit_oob(dev);

        rtl8125_powerup_pll(dev);

        rtl8125_hw_init(dev);

        rtl8125_hw_reset(dev);

        /* Get production from EEPROM */
        rtl8125_eeprom_type(tp);

        if (tp->eeprom_type == EEPROM_TYPE_93C46 || tp->eeprom_type == EEPROM_TYPE_93C56)
                rtl8125_set_eeprom_sel_low(tp);

        rtl8125_get_mac_address(dev);

        tp->fw_name = rtl_chip_fw_infos[tp->mcfg].fw_name;

        tp->tally_vaddr = dma_alloc_coherent(&pdev->dev, sizeof(*tp->tally_vaddr),
                                             &tp->tally_paddr, GFP_KERNEL);
        if (!tp->tally_vaddr) {
                rc = -ENOMEM;
                goto err_out;
        }

        rtl8125_tally_counter_clear(tp);

        pci_set_drvdata(pdev, dev);

        rc = register_netdev(dev);
        if (rc)
                goto err_out;

        printk(KERN_INFO "%s: This product is covered by one or more of the following patents: US6,570,884, US6,115,776, and US6,327,625.\n", MODULENAME);

        rtl8125_disable_rxdvgate(dev);

        device_set_wakeup_enable(&pdev->dev, tp->wol_enabled);

        netif_carrier_off(dev);

        printk("%s", GPL_CLAIM);

out:
        return rc;

err_out:
        if (tp->tally_vaddr != NULL) {
                dma_free_coherent(&pdev->dev, sizeof(*tp->tally_vaddr), tp->tally_vaddr,
                                  tp->tally_paddr);

                tp->tally_vaddr = NULL;
        }
#ifdef  CONFIG_R8125_NAPI
        rtl8125_del_napi(tp);
#endif
        rtl8125_disable_msi(pdev, tp);

err_out_1:
        rtl8125_release_board(pdev, dev);

        goto out;
}

static void __devexit
rtl8125_remove_one(struct pci_dev *pdev)
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8125_private *tp = netdev_priv(dev);

        assert(dev != NULL);
        assert(tp != NULL);

        set_bit(R8125_FLAG_DOWN, tp->task_flags);

        rtl8125_cancel_all_schedule_work(tp);

#ifdef  CONFIG_R8125_NAPI
        rtl8125_del_napi(tp);
#endif
        if (HW_DASH_SUPPORT_DASH(tp))
                rtl8125_driver_stop(tp);

        unregister_netdev(dev);
        rtl8125_disable_msi(pdev, tp);
#ifdef ENABLE_R8125_PROCFS
        rtl8125_proc_remove(dev);
#endif
        if (tp->tally_vaddr != NULL) {
                dma_free_coherent(&pdev->dev, sizeof(*tp->tally_vaddr), tp->tally_vaddr, tp->tally_paddr);
                tp->tally_vaddr = NULL;
        }

        rtl8125_release_board(pdev, dev);

#ifdef ENABLE_USE_FIRMWARE_FILE
        rtl8125_release_firmware(tp);
#endif

        pci_set_drvdata(pdev, NULL);
}

#ifdef ENABLE_PAGE_REUSE
static inline unsigned int rtl8125_rx_page_order(unsigned rx_buf_sz, unsigned page_size)
{
        unsigned truesize = SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) +
                            SKB_DATA_ALIGN(rx_buf_sz + R8125_RX_ALIGN);

        return get_order(truesize * 2);
}
#endif //ENABLE_PAGE_REUSE

static void
rtl8125_set_rxbufsize(struct rtl8125_private *tp,
                      struct net_device *dev)
{
        unsigned int mtu = dev->mtu;

        tp->rms = (mtu > ETH_DATA_LEN) ? mtu + ETH_HLEN + 8 + 1 : RX_BUF_SIZE;
        tp->rx_buf_sz = tp->rms;
#ifdef ENABLE_RX_PACKET_FRAGMENT
        tp->rx_buf_sz =  SKB_DATA_ALIGN(RX_BUF_SIZE);
#endif //ENABLE_RX_PACKET_FRAGMENT
#ifdef ENABLE_PAGE_REUSE
        tp->rx_buf_page_order = rtl8125_rx_page_order(tp->rx_buf_sz, PAGE_SIZE);
        tp->rx_buf_page_size = rtl8125_rx_page_size(tp->rx_buf_page_order);
#endif //ENABLE_PAGE_REUSE
}

static void rtl8125_free_irq(struct rtl8125_private *tp)
{
        int i;

        for (i=0; i<tp->irq_nvecs; i++) {
                struct r8125_irq *irq = &tp->irq_tbl[i];
                struct r8125_napi *r8125napi = &tp->r8125napi[i];

                if (irq->requested) {
                        irq->requested = 0;
#if defined(RTL_USE_NEW_INTR_API)
                        pci_free_irq(tp->pci_dev, i, r8125napi);
#else
                        free_irq(irq->vector, r8125napi);
#endif
                }
        }
}

static int rtl8125_alloc_irq(struct rtl8125_private *tp)
{
        struct net_device *dev = tp->dev;
        int rc = 0;
        struct r8125_irq *irq;
        struct r8125_napi *r8125napi;
        int i = 0;
        const int len = sizeof(tp->irq_tbl[0].name);

#if defined(RTL_USE_NEW_INTR_API)
        for (i=0; i<tp->irq_nvecs; i++) {
                irq = &tp->irq_tbl[i];
                if (tp->features & RTL_FEATURE_MSIX &&
                    tp->HwCurrIsrVer == 2)
                        irq->handler = rtl8125_interrupt_msix;
                else
                        irq->handler = rtl8125_interrupt;

                r8125napi = &tp->r8125napi[i];
                snprintf(irq->name, len, "%s-%d", dev->name, i);
                rc = pci_request_irq(tp->pci_dev, i, irq->handler, NULL, r8125napi,
                                     irq->name);
                if (rc)
                        break;

                irq->vector = pci_irq_vector(tp->pci_dev, i);
                irq->requested = 1;
        }
#else
        unsigned long irq_flags = 0;
#ifdef ENABLE_LIB_SUPPORT
        irq_flags |= IRQF_NO_SUSPEND;
#endif
        if (tp->features & RTL_FEATURE_MSIX &&
            tp->HwCurrIsrVer == 2) {
                for (i=0; i<tp->irq_nvecs; i++) {
                        irq = &tp->irq_tbl[i];
                        irq->handler = rtl8125_interrupt_msix;
                        r8125napi = &tp->r8125napi[i];
                        snprintf(irq->name, len, "%s-%d", dev->name, i);
                        rc = request_irq(irq->vector, irq->handler, irq_flags, irq->name, r8125napi);

                        if (rc)
                                break;

                        irq->requested = 1;
                }
        } else {
                irq = &tp->irq_tbl[0];
                irq->handler = rtl8125_interrupt;
                r8125napi = &tp->r8125napi[0];
                snprintf(irq->name, len, "%s-0", dev->name);
                if (!(tp->features & RTL_FEATURE_MSIX))
                        irq->vector = dev->irq;
                irq_flags |= (tp->features & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX)) ? 0 : SA_SHIRQ;
                rc = request_irq(irq->vector, irq->handler, irq_flags, irq->name, r8125napi);

                if (rc == 0)
                        irq->requested = 1;
        }
#endif
        if (rc)
                rtl8125_free_irq(tp);

        return rc;
}

static int rtl8125_alloc_tx_desc(struct rtl8125_private *tp)
{
        struct rtl8125_tx_ring *ring;
        struct pci_dev *pdev = tp->pci_dev;
        int i;

        for (i = 0; i < tp->num_tx_rings; i++) {
                ring = &tp->tx_ring[i];
                ring->TxDescAllocSize = (ring->num_tx_desc + 1) * sizeof(struct TxDesc);
                ring->TxDescArray = dma_alloc_coherent(&pdev->dev,
                                                       ring->TxDescAllocSize,
                                                       &ring->TxPhyAddr,
                                                       GFP_KERNEL);

                if (!ring->TxDescArray)
                        return -1;
        }

        return 0;
}

static int rtl8125_alloc_rx_desc(struct rtl8125_private *tp)
{
        struct rtl8125_rx_ring *ring;
        struct pci_dev *pdev = tp->pci_dev;
        int i;

        for (i = 0; i < tp->num_rx_rings; i++) {
                ring = &tp->rx_ring[i];
                ring->RxDescAllocSize = (ring->num_rx_desc + 1) * tp->RxDescLength;
                ring->RxDescArray = dma_alloc_coherent(&pdev->dev,
                                                       ring->RxDescAllocSize,
                                                       &ring->RxPhyAddr,
                                                       GFP_KERNEL);

                if (!ring->RxDescArray)
                        return -1;
        }

        return 0;
}

static int rtl8125_alloc_patch_mem(struct rtl8125_private *tp)
{
        struct pci_dev *pdev = tp->pci_dev;

        if (tp->RequireRduNonStopPatch) {
                tp->ShortPacketEmptyBuffer = dma_alloc_coherent(&pdev->dev,
                                             SHORT_PACKET_PADDING_BUF_SIZE,
                                             &tp->ShortPacketEmptyBufferPhy,
                                             GFP_KERNEL);
                if (!tp->ShortPacketEmptyBuffer)
                        return -1;

                memset(tp->ShortPacketEmptyBuffer, 0x0, SHORT_PACKET_PADDING_BUF_SIZE);
        }

        return 0;
}

static void rtl8125_free_tx_desc(struct rtl8125_private *tp)
{
        struct rtl8125_tx_ring *ring;
        struct pci_dev *pdev = tp->pci_dev;
        int i;

        for (i = 0; i < tp->num_tx_rings; i++) {
                ring = &tp->tx_ring[i];
                if (ring->TxDescArray) {
                        dma_free_coherent(&pdev->dev,
                                          ring->TxDescAllocSize,
                                          ring->TxDescArray,
                                          ring->TxPhyAddr);
                        ring->TxDescArray = NULL;
                }
        }
}

static void rtl8125_free_rx_desc(struct rtl8125_private *tp)
{
        struct rtl8125_rx_ring *ring;
        struct pci_dev *pdev = tp->pci_dev;
        int i;

        for (i = 0; i < tp->num_rx_rings; i++) {
                ring = &tp->rx_ring[i];
                if (ring->RxDescArray) {
                        dma_free_coherent(&pdev->dev,
                                          ring->RxDescAllocSize,
                                          ring->RxDescArray,
                                          ring->RxPhyAddr);
                        ring->RxDescArray = NULL;
                }
        }
}

static void rtl8125_free_patch_mem(struct rtl8125_private *tp)
{
        struct pci_dev *pdev = tp->pci_dev;

        if (tp->ShortPacketEmptyBuffer) {
                dma_free_coherent(&pdev->dev,
                                  SHORT_PACKET_PADDING_BUF_SIZE,
                                  tp->ShortPacketEmptyBuffer,
                                  tp->ShortPacketEmptyBufferPhy);
                tp->ShortPacketEmptyBuffer = NULL;
        }
}

static void rtl8125_free_alloc_resources(struct rtl8125_private *tp)
{
        rtl8125_free_rx_desc(tp);

        rtl8125_free_tx_desc(tp);

        rtl8125_free_patch_mem(tp);
}

#ifdef ENABLE_USE_FIRMWARE_FILE
static void rtl8125_request_firmware(struct rtl8125_private *tp)
{
        struct rtl8125_fw *rtl_fw;

        /* firmware loaded already or no firmware available */
        if (tp->rtl_fw || !tp->fw_name)
                return;

        rtl_fw = kzalloc(sizeof(*rtl_fw), GFP_KERNEL);
        if (!rtl_fw)
                return;

        rtl_fw->phy_write = rtl8125_mdio_write;
        rtl_fw->phy_read = rtl8125_mdio_read;
        rtl_fw->mac_mcu_write = mac_mcu_write;
        rtl_fw->mac_mcu_read = mac_mcu_read;
        rtl_fw->fw_name = tp->fw_name;
        rtl_fw->dev = tp_to_dev(tp);

        if (rtl8125_fw_request_firmware(rtl_fw))
                kfree(rtl_fw);
        else
                tp->rtl_fw = rtl_fw;
}
#endif

int rtl8125_open(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int retval;

        retval = -ENOMEM;

#ifdef ENABLE_R8125_PROCFS
        rtl8125_proc_init(dev);
#endif
        rtl8125_set_rxbufsize(tp, dev);
        /*
         * Rx and Tx descriptors needs 256 bytes alignment.
         * pci_alloc_consistent provides more.
         */
        if (rtl8125_alloc_tx_desc(tp) < 0 || rtl8125_alloc_rx_desc(tp) < 0)
                goto err_free_all_allocated_mem;

        retval = rtl8125_init_ring(dev);
        if (retval < 0)
                goto err_free_all_allocated_mem;

        retval = rtl8125_alloc_patch_mem(tp);
        if (retval < 0)
                goto err_free_all_allocated_mem;

        retval = rtl8125_alloc_irq(tp);
        if (retval < 0)
                goto err_free_all_allocated_mem;

        if (netif_msg_probe(tp)) {
                printk(KERN_INFO "%s: 0x%lx, "
                       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, "
                       "IRQ %d\n",
                       dev->name,
                       dev->base_addr,
                       dev->dev_addr[0], dev->dev_addr[1],
                       dev->dev_addr[2], dev->dev_addr[3],
                       dev->dev_addr[4], dev->dev_addr[5], dev->irq);
        }

#ifdef ENABLE_USE_FIRMWARE_FILE
        rtl8125_request_firmware(tp);
#endif
        pci_set_master(tp->pci_dev);

#ifdef  CONFIG_R8125_NAPI
        rtl8125_enable_napi(tp);
#endif

        rtl8125_exit_oob(dev);

        rtl8125_up(dev);

#ifdef ENABLE_PTP_SUPPORT
        if (tp->EnablePtp)
                rtl8125_ptp_init(tp);
#endif
        clear_bit(R8125_FLAG_DOWN, tp->task_flags);

        if (tp->resume_not_chg_speed)
                rtl8125_check_link_status(dev);
        else
                rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);

        if (tp->esd_flag == 0) {
                //rtl8125_request_esd_timer(dev);

                rtl8125_schedule_esd_work(tp);
        }

        //rtl8125_request_link_timer(dev);

        rtl8125_enable_hw_linkchg_interrupt(tp);

out:

        return retval;

err_free_all_allocated_mem:
        rtl8125_free_alloc_resources(tp);

        goto out;
}

static void
set_offset70F(struct rtl8125_private *tp, u8 setting)
{
        u32 csi_tmp;
        u32 temp = (u32)setting;
        temp = temp << 24;
        /*set PCI configuration space offset 0x70F to setting*/
        /*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/

        csi_tmp = rtl8125_csi_read(tp, 0x70c) & 0x00ffffff;
        rtl8125_csi_write(tp, 0x70c, csi_tmp | temp);
}

static void
set_offset79(struct rtl8125_private *tp, u8 setting)
{
        //Set PCI configuration space offset 0x79 to setting

        struct pci_dev *pdev = tp->pci_dev;
        u8 device_control;

        if (hwoptimize & HW_PATCH_SOC_LAN) return;

        pci_read_config_byte(pdev, 0x79, &device_control);
        device_control &= ~0x70;
        device_control |= setting;
        pci_write_config_byte(pdev, 0x79, device_control);
}

void
rtl8125_hw_set_rx_packet_filter(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 mc_filter[2];   /* Multicast hash filter */
        int rx_mode;
        u32 tmp = 0;

        if (dev->flags & IFF_PROMISC) {
                /* Unconditionally log net taps. */
                if (netif_msg_link(tp))
                        printk(KERN_NOTICE "%s: Promiscuous mode enabled.\n",
                               dev->name);

                rx_mode =
                        AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
                        AcceptAllPhys;
                mc_filter[1] = mc_filter[0] = 0xffffffff;
        } else if ((netdev_mc_count(dev) > multicast_filter_limit)
                   || (dev->flags & IFF_ALLMULTI)) {
                /* Too many to filter perfectly -- accept all multicasts. */
                rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0xffffffff;
        } else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
                struct dev_mc_list *mclist;
                unsigned int i;

                rx_mode = AcceptBroadcast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0;
                for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
                     i++, mclist = mclist->next) {
                        int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
                        mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
                        rx_mode |= AcceptMulticast;
                }
#else
                struct netdev_hw_addr *ha;

                rx_mode = AcceptBroadcast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0;
                netdev_for_each_mc_addr(ha, dev) {
                        int bit_nr = ether_crc(ETH_ALEN, ha->addr) >> 26;
                        mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
                        rx_mode |= AcceptMulticast;
                }
#endif
        }

        if (dev->features & NETIF_F_RXALL)
                rx_mode |= (AcceptErr | AcceptRunt);

        tmp = mc_filter[0];
        mc_filter[0] = swab32(mc_filter[1]);
        mc_filter[1] = swab32(tmp);

        tmp = tp->rtl8125_rx_config | rx_mode | (RTL_R32(tp, RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);

        RTL_W32(tp, RxConfig, tmp);
        RTL_W32(tp, MAR0 + 0, mc_filter[0]);
        RTL_W32(tp, MAR0 + 4, mc_filter[1]);
}

static void
rtl8125_set_rx_mode(struct net_device *dev)
{
        rtl8125_hw_set_rx_packet_filter(dev);
}

void
rtl8125_set_rx_q_num(struct rtl8125_private *tp,
                     unsigned int num_rx_queues)
{
        u16 q_ctrl;
        u16 rx_q_num;

        rx_q_num = (u16)ilog2(num_rx_queues);
        rx_q_num &= (BIT_0 | BIT_1 | BIT_2);
        rx_q_num <<= 2;
        q_ctrl = RTL_R16(tp, Q_NUM_CTRL_8125);
        q_ctrl &= ~(BIT_2 | BIT_3 | BIT_4);
        q_ctrl |= rx_q_num;
        RTL_W16(tp, Q_NUM_CTRL_8125, q_ctrl);
}

void
rtl8125_set_tx_q_num(struct rtl8125_private *tp,
                     unsigned int num_tx_queues)
{
        u16 mac_ocp_data;

        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE63E);
        mac_ocp_data &= ~(BIT_11 | BIT_10);
        mac_ocp_data |= ((ilog2(num_tx_queues) & 0x03) << 10);
        rtl8125_mac_ocp_write(tp, 0xE63E, mac_ocp_data);
}

void
rtl8125_hw_config(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;
        u16 mac_ocp_data;

        RTL_W32(tp, RxConfig, (RX_DMA_BURST << RxCfgDMAShift));

        rtl8125_hw_reset(dev);

        rtl8125_enable_cfg9346_write(tp);
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                break;
        }

        //clear io_rdy_l23
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~BIT_1);
                break;
        }

        //keep magic packet only
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B6);
                mac_ocp_data &= BIT_0;
                rtl8125_mac_ocp_write(tp, 0xC0B6, mac_ocp_data);
                break;
        }

        rtl8125_tally_counter_addr_fill(tp);

        rtl8125_enable_extend_tally_couter(tp);

        rtl8125_desc_addr_fill(tp);

        /* Set DMA burst size and Interframe Gap Time */
        RTL_W32(tp, TxConfig, (TX_DMA_BURST_unlimited << TxDMAShift) |
                (InterFrameGap << TxInterFrameGapShift));

        if (tp->EnableTxNoClose)
                RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | BIT_6));

        if (enable_double_vlan)
                rtl8125_enable_double_vlan(tp);
        else
                rtl8125_disable_double_vlan(tp);

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5 ||
            tp->mcfg == CFG_METHOD_6 ||
            tp->mcfg == CFG_METHOD_7 ||
            tp->mcfg == CFG_METHOD_8) {
                set_offset70F(tp, 0x27);
                set_offset79(tp, 0x50);

                RTL_W16(tp, 0x382, 0x221B);

#ifdef ENABLE_RSS_SUPPORT
                rtl8125_config_rss(tp);
#else
                RTL_W32(tp, RSS_CTRL_8125, 0x00);
#endif
                rtl8125_set_rx_q_num(tp, rtl8125_tot_rx_rings(tp));

                RTL_W8(tp, Config1, RTL_R8(tp, Config1) & ~0x10);

                rtl8125_mac_ocp_write(tp, 0xC140, 0xFFFF);
                rtl8125_mac_ocp_write(tp, 0xC142, 0xFFFF);

                //new tx desc format
                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB58);
                mac_ocp_data |= (BIT_0);
                rtl8125_mac_ocp_write(tp, 0xEB58, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE614);
                mac_ocp_data &= ~( BIT_10 | BIT_9 | BIT_8);
                if (tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5 ||
                    tp->mcfg == CFG_METHOD_7)
                        mac_ocp_data |= ((2 & 0x07) << 8);
                else if (tp->mcfg == CFG_METHOD_8)
                        mac_ocp_data |= ((4 & 0x07) << 8);
                else
                        mac_ocp_data |= ((3 & 0x07) << 8);
                rtl8125_mac_ocp_write(tp, 0xE614, mac_ocp_data);

                rtl8125_set_tx_q_num(tp, rtl8125_tot_tx_rings(tp));

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE63E);
                mac_ocp_data &= ~(BIT_5 | BIT_4);
                if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
                    tp->mcfg == CFG_METHOD_6 || tp->mcfg == CFG_METHOD_8)
                        mac_ocp_data |= ((0x02 & 0x03) << 4);
                rtl8125_mac_ocp_write(tp, 0xE63E, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B4);
                mac_ocp_data &= ~BIT_0;
                rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);
                mac_ocp_data |= BIT_0;
                rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B4);
                mac_ocp_data |= (BIT_3|BIT_2);
                rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB6A);
                mac_ocp_data &= ~(BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
                mac_ocp_data |= (BIT_5 | BIT_4 | BIT_1 | BIT_0);
                rtl8125_mac_ocp_write(tp, 0xEB6A, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB50);
                mac_ocp_data &= ~(BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5);
                mac_ocp_data |= (BIT_6);
                rtl8125_mac_ocp_write(tp, 0xEB50, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE056);
                mac_ocp_data &= ~(BIT_7 | BIT_6 | BIT_5 | BIT_4);
                //mac_ocp_data |= (BIT_4 | BIT_5);
                rtl8125_mac_ocp_write(tp, 0xE056, mac_ocp_data);

                RTL_W8(tp, TDFNR, 0x10);

                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_7);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE040);
                mac_ocp_data &= ~(BIT_12);
                rtl8125_mac_ocp_write(tp, 0xE040, mac_ocp_data);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEA1C);
                mac_ocp_data &= ~(BIT_1 | BIT_0);
                mac_ocp_data |= (BIT_0);
                rtl8125_mac_ocp_write(tp, 0xEA1C, mac_ocp_data);

                rtl8125_mac_ocp_write(tp, 0xE0C0, 0x4000);

                SetMcuAccessRegBit(tp, 0xE052, (BIT_6 | BIT_5));
                ClearMcuAccessRegBit(tp, 0xE052, BIT_3 | BIT_7);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xD430);
                mac_ocp_data &= ~(BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
                mac_ocp_data |= 0x45F;
                rtl8125_mac_ocp_write(tp, 0xD430, mac_ocp_data);

                //rtl8125_mac_ocp_write(tp, 0xE0C0, 0x4F87);
                if (!tp->DASH)
                        RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_6 | BIT_7);
                else
                        RTL_W8(tp, 0xD0, (RTL_R8(tp, 0xD0) & ~BIT_6) | BIT_7);

                if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
                    tp->mcfg == CFG_METHOD_6)
                        RTL_W8(tp, MCUCmd_reg, RTL_R8(tp, MCUCmd_reg) | BIT_0);

                rtl8125_disable_eee_plus(tp);

                mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEA1C);
                mac_ocp_data &= ~(BIT_2);
                rtl8125_mac_ocp_write(tp, 0xEA1C, mac_ocp_data);

                SetMcuAccessRegBit(tp, 0xEB54, BIT_0);
                udelay(1);
                ClearMcuAccessRegBit(tp, 0xEB54, BIT_0);
                RTL_W16(tp, 0x1880, RTL_R16(tp, 0x1880) & ~(BIT_4 | BIT_5));
        }

        /* csum offload command for RTL8125 */
        tp->tx_tcp_csum_cmd = TxTCPCS_C;
        tp->tx_udp_csum_cmd = TxUDPCS_C;
        tp->tx_ip_csum_cmd = TxIPCS_C;
        tp->tx_ipv6_csum_cmd = TxIPV6F_C;

        /* config interrupt type for RTL8125B */
        if (tp->HwSuppIsrVer == 2)
                rtl8125_hw_set_interrupt_type(tp, tp->HwCurrIsrVer);

        //other hw parameters
        rtl8125_hw_clear_timer_int(dev);

        rtl8125_hw_clear_int_miti(dev);

        if (tp->RequireRduNonStopPatch &&
            tp->ShortPacketEmptyBuffer) {
                RTL_W32(tp, RSS_INDIRECTION_TBL_8125_V2, ((u64)tp->ShortPacketEmptyBufferPhy & DMA_BIT_MASK(32)));
                RTL_W32(tp, RSS_INDIRECTION_TBL_8125_V2 + 4, ((u64)tp->ShortPacketEmptyBufferPhy >> 32));
        }

        if (tp->use_timer_interrrupt &&
            (tp->HwCurrIsrVer == 2) &&
            (tp->HwSuppIntMitiVer == 4) &&
            (tp->features & RTL_FEATURE_MSIX)) {
                int i;
                for (i = 0; i < tp->irq_nvecs; i++)
                        rtl8125_hw_set_timer_int_8125(tp, i, timer_count_v2);
        }

        rtl8125_enable_exit_l1_mask(tp);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_mac_ocp_write(tp, 0xE098, 0xC302);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_disable_pci_offset_99(tp);
                if (aspm) {
                        if (tp->org_pci_offset_99 & (BIT_2 | BIT_5 | BIT_6))
                                rtl8125_init_pci_offset_99(tp);
                }
                break;
        }
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                rtl8125_disable_pci_offset_180(tp);
                if (aspm) {
                        if (tp->org_pci_offset_180 & (BIT_0|BIT_1))
                                rtl8125_init_pci_offset_180(tp);
                }
                break;
        }

        tp->cp_cmd &= ~(EnableBist | Macdbgo_oe | Force_halfdup |
                        Force_rxflow_en | Force_txflow_en | Cxpl_dbg_sel |
                        ASF | Macdbgo_sel);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        RTL_W16(tp, CPlusCmd, tp->cp_cmd);
#else
        rtl8125_hw_set_features(dev, dev->features);
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7: {
                int timeout;
                for (timeout = 0; timeout < 10; timeout++) {
                        if ((rtl8125_mac_ocp_read(tp, 0xE00E) & BIT_13)==0)
                                break;
                        mdelay(1);
                }
        }
        break;
        }

        RTL_W16(tp, RxMaxSize, tp->rms);

        rtl8125_disable_rxdvgate(dev);

        if (!tp->pci_cfg_is_read) {
                pci_read_config_byte(pdev, PCI_COMMAND, &tp->pci_cfg_space.cmd);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &tp->pci_cfg_space.io_base_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, &tp->pci_cfg_space.io_base_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &tp->pci_cfg_space.mem_base_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &tp->pci_cfg_space.mem_base_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_3, &tp->pci_cfg_space.resv_0x1c_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, &tp->pci_cfg_space.resv_0x1c_h);
                pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &tp->pci_cfg_space.ilr);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &tp->pci_cfg_space.resv_0x20_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &tp->pci_cfg_space.resv_0x20_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &tp->pci_cfg_space.resv_0x24_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &tp->pci_cfg_space.resv_0x24_h);
                pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &tp->pci_cfg_space.resv_0x2c_l);
                pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, &tp->pci_cfg_space.resv_0x2c_h);
                if (tp->HwPcieSNOffset > 0) {
                        tp->pci_cfg_space.pci_sn_l = rtl8125_csi_read(tp, tp->HwPcieSNOffset);
                        tp->pci_cfg_space.pci_sn_h = rtl8125_csi_read(tp, tp->HwPcieSNOffset + 4);
                }

                tp->pci_cfg_is_read = 1;
        }

        /* Set Rx packet filter */
        rtl8125_hw_set_rx_packet_filter(dev);

#ifdef ENABLE_DASH_SUPPORT
        if (tp->DASH && !tp->dash_printer_enabled)
                NICChkTypeEnableDashInterrupt(tp);
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
                if (aspm) {
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) | BIT_0);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                } else {
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                }
                break;
        }

        rtl8125_disable_cfg9346_write(tp);

        udelay(10);
}

void
rtl8125_hw_start(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

#ifdef ENABLE_LIB_SUPPORT
        rtl8125_init_lib_ring(tp);
#endif

        RTL_W8(tp, ChipCmd, CmdTxEnb | CmdRxEnb);

        rtl8125_enable_hw_interrupt(tp);

        rtl8125_lib_reset_complete(tp);
}

static int
rtl8125_change_mtu(struct net_device *dev,
                   int new_mtu)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
        if (new_mtu < ETH_MIN_MTU)
                return -EINVAL;
        else if (new_mtu > tp->max_jumbo_frame_size)
                new_mtu = tp->max_jumbo_frame_size;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)

        dev->mtu = new_mtu;

        if (!netif_running(dev))
                goto out;

        rtl8125_down(dev);

        rtl8125_set_rxbufsize(tp, dev);

        ret = rtl8125_init_ring(dev);

        if (ret < 0)
                goto err_out;

#ifdef CONFIG_R8125_NAPI
        rtl8125_enable_napi(tp);
#endif//CONFIG_R8125_NAPI

        //netif_carrier_off(dev);
        //netif_tx_disable(dev);
        rtl8125_hw_config(dev);
        rtl8125_enable_hw_linkchg_interrupt(tp);

        rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);

        //mod_timer(&tp->esd_timer, jiffies + RTL8125_ESD_TIMEOUT);
        //mod_timer(&tp->link_timer, jiffies + RTL8125_LINK_TIMEOUT);
out:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
        netdev_update_features(dev);
#endif

err_out:
        return ret;
}

static inline void
rtl8125_mark_to_asic_v3(struct RxDescV3 *descv3,
                        u32 rx_buf_sz)
{
        u32 eor = le32_to_cpu(descv3->RxDescNormalDDWord4.opts1) & RingEnd;

        WRITE_ONCE(descv3->RxDescNormalDDWord4.opts1, cpu_to_le32(DescOwn | eor | rx_buf_sz));
}

void
rtl8125_mark_to_asic(struct rtl8125_private *tp,
                     struct RxDesc *desc,
                     u32 rx_buf_sz)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                rtl8125_mark_to_asic_v3((struct RxDescV3 *)desc, rx_buf_sz);
        else {
                u32 eor = le32_to_cpu(desc->opts1) & RingEnd;

                WRITE_ONCE(desc->opts1, cpu_to_le32(DescOwn | eor | rx_buf_sz));
        }
}

static inline void
rtl8125_map_to_asic(struct rtl8125_private *tp,
                    struct rtl8125_rx_ring *ring,
                    struct RxDesc *desc,
                    dma_addr_t mapping,
                    u32 rx_buf_sz,
                    const u32 cur_rx)
{
        ring->RxDescPhyAddr[cur_rx] = mapping;
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                ((struct RxDescV3 *)desc)->addr = cpu_to_le64(mapping);
        else
                desc->addr = cpu_to_le64(mapping);
        wmb();
        rtl8125_mark_to_asic(tp, desc, rx_buf_sz);
}

#ifdef ENABLE_PAGE_REUSE

static int
rtl8125_alloc_rx_page(struct rtl8125_private *tp, struct rtl8125_rx_ring *ring,
                      struct rtl8125_rx_buffer *rxb)
{
        struct page *page;
        dma_addr_t dma;
        unsigned int order = tp->rx_buf_page_order;

        //get free page
        page = dev_alloc_pages(order);

        if (unlikely(!page))
                return -ENOMEM;

        dma = dma_map_page_attrs(&tp->pci_dev->dev, page, 0,
                                 tp->rx_buf_page_size,
                                 DMA_FROM_DEVICE,
                                 (DMA_ATTR_SKIP_CPU_SYNC | DMA_ATTR_WEAK_ORDERING));

        if (unlikely(dma_mapping_error(&tp->pci_dev->dev, dma))) {
                __free_pages(page, order);
                return -ENOMEM;
        }

        rxb->page = page;
        rxb->data = page_address(page);
        rxb->page_offset = ring->rx_offset;
        rxb->dma = dma;

        //after page alloc, page refcount already = 1

        return 0;
}

static void
rtl8125_free_rx_page(struct rtl8125_private *tp, struct rtl8125_rx_buffer *rxb)
{
        if (!rxb->page)
                return;

        dma_unmap_page_attrs(&tp->pci_dev->dev, rxb->dma,
                             tp->rx_buf_page_size,
                             DMA_FROM_DEVICE,
                             (DMA_ATTR_SKIP_CPU_SYNC | DMA_ATTR_WEAK_ORDERING));
        __free_pages(rxb->page, tp->rx_buf_page_order);
        rxb->page = NULL;
}

static void
_rtl8125_rx_clear(struct rtl8125_private *tp, struct rtl8125_rx_ring *ring)
{
        int i;
        struct rtl8125_rx_buffer *rxb;

        for (i = 0; i < ring->num_rx_desc; i++) {
                rxb = &ring->rx_buffer[i];
                if (rxb->skb) {
                        dev_kfree_skb(rxb->skb);
                        rxb->skb = NULL;
                }
                rtl8125_free_rx_page(tp, rxb);
        }
}

static u32
rtl8125_rx_fill(struct rtl8125_private *tp,
                struct rtl8125_rx_ring *ring,
                struct net_device *dev,
                u32 start,
                u32 end,
                u8 in_intr)
{
        u32 cur;
        struct rtl8125_rx_buffer *rxb;

        for (cur = start; end - cur > 0; cur++) {
                int ret, i = cur % ring->num_rx_desc;

                rxb = &ring->rx_buffer[i];
                if (rxb->page)
                        continue;

                ret = rtl8125_alloc_rx_page(tp, ring, rxb);
                if (ret)
                        break;

                dma_sync_single_range_for_device(tp_to_dev(tp),
                                                 rxb->dma,
                                                 rxb->page_offset,
                                                 tp->rx_buf_sz,
                                                 DMA_FROM_DEVICE);

                rtl8125_map_to_asic(tp, ring, ring->RxDescArray + i,
                                    rxb->dma + rxb->page_offset,
                                    tp->rx_buf_sz, i);
        }
        return cur - start;
}

#else //ENABLE_PAGE_REUSE

static void
rtl8125_free_rx_skb(struct rtl8125_private *tp,
                    struct rtl8125_rx_ring *ring,
                    struct sk_buff **sk_buff,
                    struct RxDesc *desc,
                    const u32 cur_rx)
{
        struct pci_dev *pdev = tp->pci_dev;

        dma_unmap_single(&pdev->dev, ring->RxDescPhyAddr[cur_rx], tp->rx_buf_sz,
                         DMA_FROM_DEVICE);
        dev_kfree_skb(*sk_buff);
        *sk_buff = NULL;
        rtl8125_make_unusable_by_asic(tp, desc);
}

static int
rtl8125_alloc_rx_skb(struct rtl8125_private *tp,
                     struct rtl8125_rx_ring *ring,
                     struct sk_buff **sk_buff,
                     struct RxDesc *desc,
                     int rx_buf_sz,
                     const u32 cur_rx,
                     u8 in_intr)
{
        struct sk_buff *skb;
        dma_addr_t mapping;
        int ret = 0;

        if (in_intr)
                skb = RTL_ALLOC_SKB_INTR(&tp->r8125napi[ring->index].napi, rx_buf_sz + R8125_RX_ALIGN);
        else
                skb = dev_alloc_skb(rx_buf_sz + R8125_RX_ALIGN);

        if (unlikely(!skb))
                goto err_out;

        skb_reserve(skb, R8125_RX_ALIGN);

        mapping = dma_map_single(tp_to_dev(tp), skb->data, rx_buf_sz,
                                 DMA_FROM_DEVICE);
        if (unlikely(dma_mapping_error(tp_to_dev(tp), mapping))) {
                if (unlikely(net_ratelimit()))
                        netif_err(tp, drv, tp->dev, "Failed to map RX DMA!\n");
                goto err_out;
        }

        *sk_buff = skb;
        rtl8125_map_to_asic(tp, ring, desc, mapping, rx_buf_sz, cur_rx);
out:
        return ret;

err_out:
        if (skb)
                dev_kfree_skb(skb);
        ret = -ENOMEM;
        rtl8125_make_unusable_by_asic(tp, desc);
        goto out;
}

static void
_rtl8125_rx_clear(struct rtl8125_private *tp, struct rtl8125_rx_ring *ring)
{
        int i;

        for (i = 0; i < ring->num_rx_desc; i++) {
                if (ring->Rx_skbuff[i]) {
                        rtl8125_free_rx_skb(tp,
                                            ring,
                                            ring->Rx_skbuff + i,
                                            rtl8125_get_rxdesc(tp, ring->RxDescArray, i),
                                            i);
                        ring->Rx_skbuff[i] = NULL;
                }
        }
}

static u32
rtl8125_rx_fill(struct rtl8125_private *tp,
                struct rtl8125_rx_ring *ring,
                struct net_device *dev,
                u32 start,
                u32 end,
                u8 in_intr)
{
        u32 cur;

        for (cur = start; end - cur > 0; cur++) {
                int ret, i = cur % ring->num_rx_desc;

                if (ring->Rx_skbuff[i])
                        continue;

                ret = rtl8125_alloc_rx_skb(tp,
                                           ring,
                                           ring->Rx_skbuff + i,
                                           rtl8125_get_rxdesc(tp, ring->RxDescArray, i),
                                           tp->rx_buf_sz,
                                           i,
                                           in_intr
                                          );
                if (ret < 0)
                        break;
        }
        return cur - start;
}

#endif //ENABLE_PAGE_REUSE

void
rtl8125_rx_clear(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->num_rx_rings; i++) {
                struct rtl8125_rx_ring *ring = &tp->rx_ring[i];

                _rtl8125_rx_clear(tp, ring);
        }
}

static inline void
rtl8125_mark_as_last_descriptor_8125(struct RxDescV3 *descv3)
{
        descv3->RxDescNormalDDWord4.opts1 |= cpu_to_le32(RingEnd);
}

static inline void
rtl8125_mark_as_last_descriptor(struct rtl8125_private *tp,
                                struct RxDesc *desc)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                rtl8125_mark_as_last_descriptor_8125((struct RxDescV3 *)desc);
        else
                desc->opts1 |= cpu_to_le32(RingEnd);
}

static void
rtl8125_desc_addr_fill(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->num_tx_rings; i++) {
                struct rtl8125_tx_ring *ring = &tp->tx_ring[i];
                RTL_W32(tp, ring->tdsar_reg, ((u64)ring->TxPhyAddr & DMA_BIT_MASK(32)));
                RTL_W32(tp, ring->tdsar_reg + 4, ((u64)ring->TxPhyAddr >> 32));
        }

        for (i = 0; i < tp->num_rx_rings; i++) {
                struct rtl8125_rx_ring *ring = &tp->rx_ring[i];
                RTL_W32(tp, ring->rdsar_reg, ((u64)ring->RxPhyAddr & DMA_BIT_MASK(32)));
                RTL_W32(tp, ring->rdsar_reg + 4, ((u64)ring->RxPhyAddr >> 32));
        }
}

static void
rtl8125_tx_desc_init(struct rtl8125_private *tp)
{
        int i = 0;

        for (i = 0; i < tp->num_tx_rings; i++) {
                struct rtl8125_tx_ring *ring = &tp->tx_ring[i];
                memset(ring->TxDescArray, 0x0, ring->TxDescAllocSize);

                ring->TxDescArray[ring->num_tx_desc - 1].opts1 = cpu_to_le32(RingEnd);
        }
}

static void
rtl8125_rx_desc_init(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->num_rx_rings; i++) {
                struct rtl8125_rx_ring *ring = &tp->rx_ring[i];
                memset(ring->RxDescArray, 0x0, ring->RxDescAllocSize);
        }
}

int
rtl8125_init_ring(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;

        rtl8125_init_ring_indexes(tp);

        rtl8125_tx_desc_init(tp);
        rtl8125_rx_desc_init(tp);

        for (i = 0; i < tp->num_tx_rings; i++) {
                struct rtl8125_tx_ring *ring = &tp->tx_ring[i];
                memset(ring->tx_skb, 0x0, sizeof(ring->tx_skb));
        }

        for (i = 0; i < tp->num_rx_rings; i++) {
                struct rtl8125_rx_ring *ring = &tp->rx_ring[i];
#ifdef ENABLE_PAGE_REUSE
                ring->rx_offset = R8125_RX_ALIGN;
#else
                memset(ring->Rx_skbuff, 0x0, sizeof(ring->Rx_skbuff));
#endif //ENABLE_PAGE_REUSE
                if (rtl8125_rx_fill(tp, ring, dev, 0, ring->num_rx_desc, 0) != ring->num_rx_desc)
                        goto err_out;

                rtl8125_mark_as_last_descriptor(tp, rtl8125_get_rxdesc(tp, ring->RxDescArray, ring->num_rx_desc - 1));
        }

        return 0;

err_out:
        rtl8125_rx_clear(tp);
        return -ENOMEM;
}

static void
rtl8125_unmap_tx_skb(struct pci_dev *pdev,
                     struct ring_info *tx_skb,
                     struct TxDesc *desc)
{
        unsigned int len = tx_skb->len;

        dma_unmap_single(&pdev->dev, le64_to_cpu(desc->addr), len, DMA_TO_DEVICE);

        desc->opts1 = cpu_to_le32(RTK_MAGIC_DEBUG_VALUE);
        desc->opts2 = 0x00;
        desc->addr = RTL8125_MAGIC_NUMBER;
        tx_skb->len = 0;
}

static void
rtl8125_tx_clear_range(struct rtl8125_private *tp,
                       struct rtl8125_tx_ring *ring,
                       u32 start,
                       unsigned int n)
{
        unsigned int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
        struct net_device *dev = tp->dev;
#endif

        for (i = 0; i < n; i++) {
                unsigned int entry = (start + i) % ring->num_tx_desc;
                struct ring_info *tx_skb = ring->tx_skb + entry;
                unsigned int len = tx_skb->len;

                if (len) {
                        struct sk_buff *skb = tx_skb->skb;

                        rtl8125_unmap_tx_skb(tp->pci_dev, tx_skb,
                                             ring->TxDescArray + entry);
                        if (skb) {
                                RTLDEV->stats.tx_dropped++;
                                dev_kfree_skb_any(skb);
                                tx_skb->skb = NULL;
                        }
                }
        }
}

void
rtl8125_tx_clear(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->num_tx_rings; i++) {
                struct rtl8125_tx_ring *ring = &tp->tx_ring[i];
                rtl8125_tx_clear_range(tp, ring, ring->dirty_tx, ring->num_tx_desc);
                ring->cur_tx = ring->dirty_tx = 0;
        }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8125_schedule_reset_work(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        set_bit(R8125_FLAG_TASK_RESET_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->reset_task, 4);
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
}

static void rtl8125_schedule_esd_work(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        set_bit(R8125_FLAG_TASK_ESD_CHECK_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->esd_task, RTL8125_ESD_TIMEOUT);
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
}

static void rtl8125_schedule_linkchg_work(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        set_bit(R8125_FLAG_TASK_LINKCHG_CHECK_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->linkchg_task, 4);
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
}

#define rtl8125_cancel_schedule_reset_work(a)
#define rtl8125_cancel_schedule_esd_work(a)
#define rtl8125_cancel_schedule_linkchg_work(a)

#else
static void rtl8125_schedule_reset_work(struct rtl8125_private *tp)
{
        set_bit(R8125_FLAG_TASK_RESET_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->reset_task, 4);
}

static void rtl8125_cancel_schedule_reset_work(struct rtl8125_private *tp)
{
        struct work_struct *work = &tp->reset_task.work;

        if (!work->func) return;

        cancel_delayed_work_sync(&tp->reset_task);
}

static void rtl8125_schedule_esd_work(struct rtl8125_private *tp)
{
        set_bit(R8125_FLAG_TASK_ESD_CHECK_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->esd_task, RTL8125_ESD_TIMEOUT);
}

static void rtl8125_cancel_schedule_esd_work(struct rtl8125_private *tp)
{
        struct work_struct *work = &tp->esd_task.work;

        if (!work->func) return;

        cancel_delayed_work_sync(&tp->esd_task);
}

static void rtl8125_schedule_linkchg_work(struct rtl8125_private *tp)
{
        set_bit(R8125_FLAG_TASK_LINKCHG_CHECK_PENDING, tp->task_flags);
        schedule_delayed_work(&tp->linkchg_task, 4);
}

static void rtl8125_cancel_schedule_linkchg_work(struct rtl8125_private *tp)
{
        struct work_struct *work = &tp->linkchg_task.work;

        if (!work->func) return;

        cancel_delayed_work_sync(&tp->linkchg_task);
}
#endif

static void rtl8125_init_all_schedule_work(struct rtl8125_private *tp)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
        INIT_WORK(&tp->reset_task, rtl8125_reset_task, dev);
        INIT_WORK(&tp->esd_task, rtl8125_esd_task, dev);
        INIT_WORK(&tp->linkchg_task, rtl8125_linkchg_task, dev);
#else
        INIT_DELAYED_WORK(&tp->reset_task, rtl8125_reset_task);
        INIT_DELAYED_WORK(&tp->esd_task, rtl8125_esd_task);
        INIT_DELAYED_WORK(&tp->linkchg_task, rtl8125_linkchg_task);
#endif
}

static void rtl8125_cancel_all_schedule_work(struct rtl8125_private *tp)
{
        rtl8125_cancel_schedule_reset_work(tp);
        rtl8125_cancel_schedule_esd_work(tp);
        rtl8125_cancel_schedule_linkchg_work(tp);
}

static void
rtl8125_wait_for_irq_complete(struct rtl8125_private *tp)
{
        if (tp->features & RTL_FEATURE_MSIX) {
                int i;
                for (i = 0; i < tp->irq_nvecs; i++)
                        synchronize_irq(tp->irq_tbl[i].vector);
        } else {
                synchronize_irq(tp->dev->irq);
        }
}

void
_rtl8125_wait_for_quiescence(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        /* Wait for any pending NAPI task to complete */
#ifdef CONFIG_R8125_NAPI
        rtl8125_disable_napi(tp);
#endif//CONFIG_R8125_NAPI

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,67)
        /* Give a racing hard_start_xmit a few cycles to complete. */
        synchronize_net();
#endif

        rtl8125_irq_mask_and_ack(tp);

        rtl8125_wait_for_irq_complete(tp);
}

static void
rtl8125_wait_for_quiescence(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        //suppress unused variable
        (void)(tp);

        _rtl8125_wait_for_quiescence(dev);

#ifdef CONFIG_R8125_NAPI
        rtl8125_enable_napi(tp);
#endif//CONFIG_R8125_NAPI
}

static int rtl8125_rx_nostuck(struct rtl8125_private *tp)
{
        int i, ret = 1;
        for (i = 0; i < tp->num_rx_rings; i++)
                ret &= (tp->rx_ring[i].dirty_rx == tp->rx_ring[i].cur_rx);
        return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8125_reset_task(void *_data)
{
        struct net_device *dev = _data;
        struct rtl8125_private *tp = netdev_priv(dev);
#else
static void rtl8125_reset_task(struct work_struct *work)
{
        struct rtl8125_private *tp =
                container_of(work, struct rtl8125_private, reset_task.work);
        struct net_device *dev = tp->dev;
#endif
        u32 budget = ~(u32)0;
        int i;

        rtnl_lock();

        if (!netif_running(dev) ||
            test_bit(R8125_FLAG_DOWN, tp->task_flags) ||
            !test_and_clear_bit(R8125_FLAG_TASK_RESET_PENDING, tp->task_flags))
                goto out_unlock;

        rtl8125_wait_for_quiescence(dev);

        for (i = 0; i < tp->num_rx_rings; i++) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
                rtl8125_rx_interrupt(dev, tp,  &tp->rx_ring[i], &budget);
#else
                rtl8125_rx_interrupt(dev, tp,  &tp->rx_ring[i], budget);
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
        }

        netif_carrier_off(dev);
        netif_tx_disable(dev);
        rtl8125_hw_reset(dev);

        rtl8125_tx_clear(tp);

        if (rtl8125_rx_nostuck(tp)) {
                rtl8125_rx_clear(tp);
                rtl8125_init_ring(dev);
#ifdef ENABLE_PTP_SUPPORT
                rtl8125_ptp_reset(tp);
#endif
                if (tp->resume_not_chg_speed) {
                        _rtl8125_check_link_status(dev);

                        tp->resume_not_chg_speed = 0;
                } else {
                        rtl8125_enable_hw_linkchg_interrupt(tp);

                        rtl8125_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
                }
        } else {
                if (unlikely(net_ratelimit())) {
                        struct rtl8125_private *tp = netdev_priv(dev);

                        if (netif_msg_intr(tp)) {
                                printk(PFX KERN_EMERG
                                       "%s: Rx buffers shortage\n", dev->name);
                        }
                }
                rtl8125_schedule_reset_work(tp);
        }

out_unlock:
        rtnl_unlock();
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8125_esd_task(void *_data)
{
        struct net_device *dev = _data;
        struct rtl8125_private *tp = netdev_priv(dev);
#else
static void rtl8125_esd_task(struct work_struct *work)
{
        struct rtl8125_private *tp =
                container_of(work, struct rtl8125_private, esd_task.work);
        struct net_device *dev = tp->dev;
#endif
        rtnl_lock();

        if (!netif_running(dev) ||
            test_bit(R8125_FLAG_DOWN, tp->task_flags) ||
            !test_and_clear_bit(R8125_FLAG_TASK_ESD_CHECK_PENDING, tp->task_flags))
                goto out_unlock;

        rtl8125_esd_checker(tp);

        rtl8125_schedule_esd_work(tp);

out_unlock:
        rtnl_unlock();
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8125_linkchg_task(void *_data)
{
        struct net_device *dev = _data;
        //struct rtl8125_private *tp = netdev_priv(dev);
#else
static void rtl8125_linkchg_task(struct work_struct *work)
{
        struct rtl8125_private *tp =
                container_of(work, struct rtl8125_private, linkchg_task.work);
        struct net_device *dev = tp->dev;
#endif
        rtnl_lock();

        if (!netif_running(dev) ||
            test_bit(R8125_FLAG_DOWN, tp->task_flags) ||
            !test_and_clear_bit(R8125_FLAG_TASK_LINKCHG_CHECK_PENDING, tp->task_flags))
                goto out_unlock;

        rtl8125_check_link_status(dev);

out_unlock:
        rtnl_unlock();
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static void
rtl8125_tx_timeout(struct net_device *dev, unsigned int txqueue)
#else
static void
rtl8125_tx_timeout(struct net_device *dev)
#endif
{
        struct rtl8125_private *tp = netdev_priv(dev);

        /* Let's wait a bit while any (async) irq lands on */
        rtl8125_schedule_reset_work(tp);
}

static u32
rtl8125_get_txd_opts1(struct rtl8125_tx_ring *ring,
                      u32 opts1,
                      u32 len,
                      unsigned int entry)
{
        u32 status = opts1 | len;

        if (entry == ring->num_tx_desc - 1)
                status |= RingEnd;

        return status;
}

static int
rtl8125_xmit_frags(struct rtl8125_private *tp,
                   struct rtl8125_tx_ring *ring,
                   struct sk_buff *skb,
                   const u32 *opts)
{
        struct skb_shared_info *info = skb_shinfo(skb);
        unsigned int cur_frag, entry;
        struct TxDesc *txd = NULL;
        const unsigned char nr_frags = info->nr_frags;
        unsigned long PktLenCnt = 0;
        bool LsoPatchEnabled = FALSE;

        entry = ring->cur_tx;
        for (cur_frag = 0; cur_frag < nr_frags; cur_frag++) {
                skb_frag_t *frag = info->frags + cur_frag;
                dma_addr_t mapping;
                u32 status, len;
                void *addr;

                entry = (entry + 1) % ring->num_tx_desc;

                txd = ring->TxDescArray + entry;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
                len = frag->size;
                addr = ((void *) page_address(frag->page)) + frag->page_offset;
#else
                len = skb_frag_size(frag);
                addr = skb_frag_address(frag);
#endif
                if (tp->RequireLSOPatch  &&
                    (cur_frag == nr_frags - 1) &&
                    (opts[0] & (GiantSendv4|GiantSendv6)) &&
                    PktLenCnt < ETH_FRAME_LEN &&
                    len > 1) {
                        len -= 1;
                        mapping = dma_map_single(tp_to_dev(tp), addr, len, DMA_TO_DEVICE);

                        if (unlikely(dma_mapping_error(tp_to_dev(tp), mapping))) {
                                if (unlikely(net_ratelimit()))
                                        netif_err(tp, drv, tp->dev,
                                                  "Failed to map TX fragments DMA!\n");
                                goto err_out;
                        }

                        /* anti gcc 2.95.3 bugware (sic) */
                        status = rtl8125_get_txd_opts1(ring, opts[0], len, entry);

                        txd->addr = cpu_to_le64(mapping);

                        ring->tx_skb[entry].len = len;

                        txd->opts2 = cpu_to_le32(opts[1]);
                        wmb();
                        txd->opts1 = cpu_to_le32(status);

                        //second txd
                        addr += len;
                        len = 1;
                        entry = (entry + 1) % ring->num_tx_desc;
                        txd = ring->TxDescArray + entry;
                        cur_frag += 1;

                        LsoPatchEnabled = TRUE;
                }

                mapping = dma_map_single(tp_to_dev(tp), addr, len, DMA_TO_DEVICE);

                if (unlikely(dma_mapping_error(tp_to_dev(tp), mapping))) {
                        if (unlikely(net_ratelimit()))
                                netif_err(tp, drv, tp->dev,
                                          "Failed to map TX fragments DMA!\n");
                        goto err_out;
                }

                /* anti gcc 2.95.3 bugware (sic) */
                status = rtl8125_get_txd_opts1(ring, opts[0], len, entry);
                if (cur_frag == (nr_frags - 1) || LsoPatchEnabled == TRUE) {
                        //ring->tx_skb[entry].skb = skb;
                        status |= LastFrag;
                }

                txd->addr = cpu_to_le64(mapping);

                ring->tx_skb[entry].len = len;

                txd->opts2 = cpu_to_le32(opts[1]);
                wmb();
                txd->opts1 = cpu_to_le32(status);

                PktLenCnt += len;
        }

        return cur_frag;

err_out:
        rtl8125_tx_clear_range(tp, ring, ring->cur_tx + 1, cur_frag);
        return -EIO;
}

static inline
__be16 get_protocol(struct sk_buff *skb)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
        return vlan_get_protocol(skb);
#else
        __be16 protocol;

        if (skb->protocol == htons(ETH_P_8021Q))
                protocol = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
        else
                protocol = skb->protocol;

        return protocol;
#endif
}

static inline
u8 rtl8125_get_l4_protocol(struct sk_buff *skb)
{
        int no = skb_network_offset(skb);
        struct ipv6hdr *i6h, _i6h;
        struct iphdr *ih, _ih;
        u8 ip_protocol = IPPROTO_RAW;

        switch (get_protocol(skb)) {
        case  __constant_htons(ETH_P_IP):
                ih = skb_header_pointer(skb, no, sizeof(_ih), &_ih);
                if (ih)
                        ip_protocol = ih->protocol;
                break;
        case  __constant_htons(ETH_P_IPV6):
                i6h = skb_header_pointer(skb, no, sizeof(_i6h), &_i6h);
                if (i6h)
                        ip_protocol = i6h->nexthdr;
                break;
        }

        return ip_protocol;
}

static bool rtl8125_skb_pad_with_len(struct sk_buff *skb, unsigned int len)
{
        if (skb_padto(skb, len))
                return false;
        skb_put(skb, len - skb->len);
        return true;
}

static bool rtl8125_skb_pad(struct sk_buff *skb)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
        return rtl8125_skb_pad_with_len(skb, ETH_ZLEN);
#else
        return !eth_skb_pad(skb);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
/* msdn_giant_send_check()
 * According to the document of microsoft, the TCP Pseudo Header excludes the
 * packet length for IPv6 TCP large packets.
 */
static int msdn_giant_send_check(struct sk_buff *skb)
{
        const struct ipv6hdr *ipv6h;
        struct tcphdr *th;
        int ret;

        ret = skb_cow_head(skb, 0);
        if (ret)
                return ret;

        ipv6h = ipv6_hdr(skb);
        th = tcp_hdr(skb);

        th->check = 0;
        th->check = ~tcp_v6_check(0, &ipv6h->saddr, &ipv6h->daddr, 0);

        return ret;
}
#endif

#define MIN_PATCH_LEN (47)
static u32
rtl8125_get_patch_pad_len(struct sk_buff *skb)
{
        u32 pad_len = 0;
        int trans_data_len;
        u32 hdr_len;
        u32 pkt_len = skb->len;
        u8 ip_protocol;
        bool has_trans = skb_transport_header_was_set(skb);

        if (!(has_trans && (pkt_len < 175))) //128 + MIN_PATCH_LEN
                goto no_padding;

        ip_protocol = rtl8125_get_l4_protocol(skb);
        if (!(ip_protocol == IPPROTO_TCP || ip_protocol == IPPROTO_UDP))
                goto no_padding;

        trans_data_len = pkt_len -
                         (skb->transport_header -
                          skb_headroom(skb));
        if (ip_protocol == IPPROTO_UDP) {
                if (trans_data_len > 3 && trans_data_len < MIN_PATCH_LEN) {
                        u16 dest_port = 0;

                        skb_copy_bits(skb, skb->transport_header - skb_headroom(skb) + 2, &dest_port, 2);
                        dest_port = ntohs(dest_port);

                        if (dest_port == 0x13f ||
                            dest_port == 0x140) {
                                pad_len = MIN_PATCH_LEN - trans_data_len;
                                goto out;
                        }
                }
        }

        hdr_len = 0;
        if (ip_protocol == IPPROTO_TCP)
                hdr_len = 20;
        else if (ip_protocol == IPPROTO_UDP)
                hdr_len = 8;
        if (trans_data_len < hdr_len)
                pad_len = hdr_len - trans_data_len;

out:
        if ((pkt_len + pad_len) < ETH_ZLEN)
                pad_len = ETH_ZLEN - pkt_len;

        return pad_len;

no_padding:

        return 0;
}

static bool
rtl8125_tso_csum(struct sk_buff *skb,
                 struct net_device *dev,
                 u32 *opts)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        unsigned long large_send = 0;
        u32 csum_cmd = 0;
        u8 sw_calc_csum = false;
        u8 check_patch_required = true;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        if (dev->features & (NETIF_F_TSO | NETIF_F_TSO6)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
                u32 mss = skb_shinfo(skb)->tso_size;
#else
                u32 mss = skb_shinfo(skb)->gso_size;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)

                /* TCP Segmentation Offload (or TCP Large Send) */
                if (mss) {
                        assert((skb_transport_offset(skb)%2) == 0);
                        switch (get_protocol(skb)) {
                        case __constant_htons(ETH_P_IP):
                                if (skb_transport_offset(skb) <= GTTCPHO_MAX) {
                                        opts[0] |= GiantSendv4;
                                        opts[0] |= skb_transport_offset(skb) << GTTCPHO_SHIFT;
                                        opts[1] |= min(mss, MSS_MAX) << 18;
                                        large_send = 1;
                                }
                                break;
                        case __constant_htons(ETH_P_IPV6):
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
                                if (msdn_giant_send_check(skb))
                                        return false;
#endif
                                if (skb_transport_offset(skb) <= GTTCPHO_MAX) {
                                        opts[0] |= GiantSendv6;
                                        opts[0] |= skb_transport_offset(skb) << GTTCPHO_SHIFT;
                                        opts[1] |= min(mss, MSS_MAX) << 18;
                                        large_send = 1;
                                }
                                break;
                        default:
                                if (unlikely(net_ratelimit()))
                                        dprintk("tso proto=%x!\n", skb->protocol);
                                break;
                        }

                        if (large_send == 0)
                                return false;

                        return true;
                }
        }
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

        if (skb->ip_summed == CHECKSUM_PARTIAL) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
                const struct iphdr *ip = skb->nh.iph;

                if (dev->features & NETIF_F_IP_CSUM) {
                        if (ip->protocol == IPPROTO_TCP)
                                csum_cmd = tp->tx_ip_csum_cmd | tp->tx_tcp_csum_cmd;
                        else if (ip->protocol == IPPROTO_UDP)
                                csum_cmd = tp->tx_ip_csum_cmd | tp->tx_udp_csum_cmd;
                        else if (ip->protocol == IPPROTO_IP)
                                csum_cmd = tp->tx_ip_csum_cmd;
                }
#else
                u8 ip_protocol = IPPROTO_RAW;

                switch (get_protocol(skb)) {
                case  __constant_htons(ETH_P_IP):
                        if (dev->features & NETIF_F_IP_CSUM) {
                                ip_protocol = ip_hdr(skb)->protocol;
                                csum_cmd = tp->tx_ip_csum_cmd;
                        }
                        break;
                case  __constant_htons(ETH_P_IPV6):
                        if (dev->features & NETIF_F_IPV6_CSUM) {
                                if (skb_transport_offset(skb) > 0 && skb_transport_offset(skb) <= TCPHO_MAX) {
                                        ip_protocol = ipv6_hdr(skb)->nexthdr;
                                        csum_cmd = tp->tx_ipv6_csum_cmd;
                                        csum_cmd |= skb_transport_offset(skb) << TCPHO_SHIFT;
                                }
                        }
                        break;
                default:
                        if (unlikely(net_ratelimit()))
                                dprintk("checksum_partial proto=%x!\n", skb->protocol);
                        break;
                }

                if (ip_protocol == IPPROTO_TCP)
                        csum_cmd |= tp->tx_tcp_csum_cmd;
                else if (ip_protocol == IPPROTO_UDP)
                        csum_cmd |= tp->tx_udp_csum_cmd;
#endif
                if (csum_cmd == 0) {
                        sw_calc_csum = true;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        WARN_ON(1); /* we need a WARN() */
#endif
                }

                if (ip_protocol == IPPROTO_TCP)
                        check_patch_required = false;
        }

        if (check_patch_required) {
                u32 pad_len = rtl8125_get_patch_pad_len(skb);

                if (pad_len > 0) {
                        if (!rtl8125_skb_pad_with_len(skb, skb->len + pad_len))
                                return false;

                        if (csum_cmd != 0)
                                sw_calc_csum = true;
                }
        }

        if (skb->len < ETH_ZLEN) {
                if (tp->UseSwPaddingShortPkt ||
                    (tp->ShortPacketSwChecksum && csum_cmd != 0)) {
                        if (!rtl8125_skb_pad(skb))
                                return false;

                        if (csum_cmd != 0)
                                sw_calc_csum = true;
                }
        }

        if (sw_calc_csum) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,7)
                skb_checksum_help(&skb, 0);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
                skb_checksum_help(skb, 0);
#else
                skb_checksum_help(skb);
#endif
        } else
                opts[1] |= csum_cmd;

        return true;
}

static bool rtl8125_tx_slots_avail(struct rtl8125_private *tp,
                                   struct rtl8125_tx_ring *ring)
{
        unsigned int slots_avail = READ_ONCE(ring->dirty_tx) + ring->num_tx_desc
                                   - READ_ONCE(ring->cur_tx);

        /* A skbuff with nr_frags needs nr_frags+1 entries in the tx queue */
        return slots_avail > MAX_SKB_FRAGS;
}

static inline u32
rtl8125_fast_mod_mask(const u32 input, const u32 mask)
{
        return input > mask ? input & mask : input;
}

static netdev_tx_t
rtl8125_start_xmit(struct sk_buff *skb,
                   struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        unsigned int entry;
        struct TxDesc *txd;
        dma_addr_t mapping;
        u32 len;
        u32 opts[2];
        netdev_tx_t ret = NETDEV_TX_OK;
        int frags;
        u8 EnableTxNoClose = tp->EnableTxNoClose;
        const u16 queue_mapping = skb_get_queue_mapping(skb);
        struct rtl8125_tx_ring *ring;
        bool stop_queue;

        assert(queue_mapping < tp->num_tx_rings);

        ring = &tp->tx_ring[queue_mapping];

        if (unlikely(!rtl8125_tx_slots_avail(tp, ring))) {
                if (netif_msg_drv(tp)) {
                        printk(KERN_ERR
                               "%s: BUG! Tx Ring[%d] full when queue awake!\n",
                               dev->name,
                               queue_mapping);
                }
                goto err_stop;
        }

        entry = ring->cur_tx % ring->num_tx_desc;
        txd = ring->TxDescArray + entry;

        if (!EnableTxNoClose) {
                if (unlikely(le32_to_cpu(txd->opts1) & DescOwn)) {
                        if (netif_msg_drv(tp)) {
                                printk(KERN_ERR
                                       "%s: BUG! Tx Desc is own by hardware!\n",
                                       dev->name);
                        }
                        goto err_stop;
                }
        }

        opts[0] = DescOwn;
        opts[1] = rtl8125_tx_vlan_tag(tp, skb);

        if (unlikely(!rtl8125_tso_csum(skb, dev, opts)))
                goto err_dma_0;

        frags = rtl8125_xmit_frags(tp, ring, skb, opts);
        if (unlikely(frags < 0))
                goto err_dma_0;
        if (frags) {
                len = skb_headlen(skb);
                opts[0] |= FirstFrag;
        } else {
                len = skb->len;

                //ring->tx_skb[entry].skb = skb;

                opts[0] |= FirstFrag | LastFrag;
        }

        opts[0] = rtl8125_get_txd_opts1(ring, opts[0], len, entry);
        mapping = dma_map_single(tp_to_dev(tp), skb->data, len, DMA_TO_DEVICE);
        if (unlikely(dma_mapping_error(tp_to_dev(tp), mapping))) {
                if (unlikely(net_ratelimit()))
                        netif_err(tp, drv, dev, "Failed to map TX DMA!\n");
                goto err_dma_1;
        }
        ring->tx_skb[entry].len = len;
#ifdef ENABLE_PTP_SUPPORT
        if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)) {
                if (tp->hwtstamp_config.tx_type == HWTSTAMP_TX_ON &&
                    !tp->ptp_tx_skb) {
                        skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;

                        tp->ptp_tx_skb = skb_get(skb);
                        tp->ptp_tx_start = jiffies;
                        schedule_work(&tp->ptp_tx_work);
                } else {
                        tp->tx_hwtstamp_skipped++;
                }
        }
#endif
        ring->tx_skb[entry].skb = skb;
        txd->addr = cpu_to_le64(mapping);
        txd->opts2 = cpu_to_le32(opts[1]);
        wmb();
        txd->opts1 = cpu_to_le32(opts[0]);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)
        dev->trans_start = jiffies;
#else
        skb_tx_timestamp(skb);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)

        /* rtl_tx needs to see descriptor changes before updated tp->cur_tx */
        smp_wmb();

        WRITE_ONCE(ring->cur_tx, ring->cur_tx + frags + 1);

        stop_queue = !rtl8125_tx_slots_avail(tp, ring);
        if (unlikely(stop_queue)) {
                /* Avoid wrongly optimistic queue wake-up: rtl_tx thread must
                 * not miss a ring update when it notices a stopped queue.
                 */
                smp_wmb();
                netif_stop_subqueue(dev, queue_mapping);
        }

        if (EnableTxNoClose) {
                if (tp->HwSuppTxNoCloseVer == 4)
                        RTL_W32(tp, ring->sw_tail_ptr_reg, ring->cur_tx);
                else
                        RTL_W16(tp, ring->sw_tail_ptr_reg, ring->cur_tx);
        } else
                RTL_W16(tp, TPPOLL_8125, BIT(ring->index));    /* set polling bit */

        if (unlikely(stop_queue)) {
                /* Sync with rtl_tx:
                 * - publish queue status and cur_tx ring index (write barrier)
                 * - refresh dirty_tx ring index (read barrier).
                 * May the current thread have a pessimistic view of the ring
                 * status and forget to wake up queue, a racing rtl_tx thread
                 * can't.
                 */
                smp_mb();
                if (rtl8125_tx_slots_avail(tp, ring))
                        netif_start_subqueue(dev, queue_mapping);
        }
out:
        return ret;
err_dma_1:
        ring->tx_skb[entry].skb = NULL;
        rtl8125_tx_clear_range(tp, ring, ring->cur_tx + 1, frags);
err_dma_0:
        RTLDEV->stats.tx_dropped++;
        dev_kfree_skb_any(skb);
        ret = NETDEV_TX_OK;
        goto out;
err_stop:
        netif_stop_subqueue(dev, queue_mapping);
        ret = NETDEV_TX_BUSY;
        RTLDEV->stats.tx_dropped++;
        goto out;
}

static u32
rtl8125_get_hw_clo_ptr(struct rtl8125_tx_ring *ring)
{
        struct rtl8125_private *tp = ring->priv;

        switch (tp->HwSuppTxNoCloseVer) {
        case 3:
                return RTL_R16(tp, ring->hw_clo_ptr_reg);
        case 4:
                return RTL_R32(tp, ring->hw_clo_ptr_reg);
        default:
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                WARN_ON(1);
#endif
                return 0;
        }
}

static int
rtl8125_tx_interrupt(struct rtl8125_tx_ring *ring, int budget)
{
        struct rtl8125_private *tp = ring->priv;
        struct net_device *dev = tp->dev;
        unsigned int dirty_tx, tx_left;
        unsigned int count = 0;
        u8 EnableTxNoClose = tp->EnableTxNoClose;

        dirty_tx = ring->dirty_tx;
        smp_rmb();
        tx_left = READ_ONCE(ring->cur_tx) - dirty_tx;
        if (EnableTxNoClose) {
                unsigned int tx_desc_closed;
                u32 NextHwDesCloPtr = rtl8125_get_hw_clo_ptr(ring);
                ring->NextHwDesCloPtr = NextHwDesCloPtr;
                smp_rmb();
                tx_desc_closed = rtl8125_fast_mod_mask(NextHwDesCloPtr - ring->BeginHwDesCloPtr, tp->MaxTxDescPtrMask);
                if(tx_left > tx_desc_closed) tx_left = tx_desc_closed;
                ring->BeginHwDesCloPtr = NextHwDesCloPtr;
        }

        while (tx_left > 0) {
                unsigned int entry = dirty_tx % ring->num_tx_desc;
                struct ring_info *tx_skb = ring->tx_skb + entry;

                if (!EnableTxNoClose &&
                    (le32_to_cpu(ring->TxDescArray[entry].opts1) & DescOwn))
                        break;

                RTLDEV->stats.tx_bytes += tx_skb->len;
                RTLDEV->stats.tx_packets++;

                rtl8125_unmap_tx_skb(tp->pci_dev,
                                     tx_skb,
                                     ring->TxDescArray + entry);

                if (tx_skb->skb != NULL) {
                        RTL_NAPI_CONSUME_SKB_ANY(tx_skb->skb, budget);
                        tx_skb->skb = NULL;
                }
                dirty_tx++;
                tx_left--;
        }

        if (ring->dirty_tx != dirty_tx) {
                count = dirty_tx - ring->dirty_tx;
                WRITE_ONCE(ring->dirty_tx, dirty_tx);
                smp_wmb();
                if (__netif_subqueue_stopped(dev, ring->index) &&
                    (rtl8125_tx_slots_avail(tp, ring))) {
                        netif_start_subqueue(dev, ring->index);
                }
                smp_rmb();
                if (!EnableTxNoClose && (ring->cur_tx != dirty_tx)) {
                        RTL_W16(tp, TPPOLL_8125, BIT(ring->index));
                }
        }

        return count;
}

static int
rtl8125_tx_interrupt_with_vector(struct rtl8125_private *tp,
                                 const int message_id,
                                 int budget)
{
        int count = 0;

        if (message_id == 16)
                count += rtl8125_tx_interrupt(&tp->tx_ring[0], budget);
#ifdef ENABLE_MULTIPLE_TX_QUEUE
        else if (message_id == 18)
                count += rtl8125_tx_interrupt(&tp->tx_ring[1], budget);
#endif

        return count;
}

static inline int
rtl8125_fragmented_frame(struct rtl8125_private *tp, u32 status)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                return (status & (FirstFrag_V3 | LastFrag_V3)) != (FirstFrag_V3 | LastFrag_V3);
        else
                return (status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag);
}

static inline int
rtl8125_is_non_eop(struct rtl8125_private *tp, u32 status)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                return !(status & LastFrag_V3);
        else
                return !(status & LastFrag);
}

static inline int
rtl8125_rx_desc_type(u32 status)
{
        return ((status >> 26) & 0x0F);
}

static inline void
rtl8125_rx_v3_csum(struct rtl8125_private *tp,
                   struct sk_buff *skb,
                   struct RxDescV3 *descv3)
{
        u32 opts2 = le32_to_cpu(descv3->RxDescNormalDDWord4.opts2);

        /* rx csum offload for RTL8125 */
        if (((opts2 & RxTCPT_v3) && !(opts2 & RxTCPF_v3)) ||
            ((opts2 & RxUDPT_v3) && !(opts2 & RxUDPF_v3)))
                skb->ip_summed = CHECKSUM_UNNECESSARY;
        else
                skb_checksum_none_assert(skb);
}

static inline void
rtl8125_rx_csum(struct rtl8125_private *tp,
                struct sk_buff *skb,
                struct RxDesc *desc,
                u32 opts1)
{
        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3)
                rtl8125_rx_v3_csum(tp, skb, (struct RxDescV3 *)desc);
        else {
                /* rx csum offload for RTL8125 */
                if (((opts1 & RxTCPT) && !(opts1 & RxTCPF)) ||
                    ((opts1 & RxUDPT) && !(opts1 & RxUDPF)))
                        skb->ip_summed = CHECKSUM_UNNECESSARY;
                else
                        skb_checksum_none_assert(skb);
        }
}

/*
static inline int
rtl8125_try_rx_copy(struct rtl8125_private *tp,
                    struct rtl8125_rx_ring *ring,
                    struct sk_buff **sk_buff,
                    int pkt_size,
                    struct RxDesc *desc,
                    int rx_buf_sz)
{
        int ret = -1;

        struct sk_buff *skb;

        skb = RTL_ALLOC_SKB_INTR(&tp->r8125napi[ring->index].napi, pkt_size + R8125_RX_ALIGN);
        if (skb) {
                u8 *data;

                data = sk_buff[0]->data;
                skb_reserve(skb, R8125_RX_ALIGN);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
                prefetch(data - R8125_RX_ALIGN);
#endif
                eth_copy_and_sum(skb, data, pkt_size, 0);
                *sk_buff = skb;
                rtl8125_mark_to_asic(tp, desc, rx_buf_sz);
                ret = 0;
        }

        return ret;
}
*/

static inline void
rtl8125_rx_skb(struct rtl8125_private *tp,
               struct sk_buff *skb,
               u32 ring_index)
{
#ifdef CONFIG_R8125_NAPI
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
        netif_receive_skb(skb);
#else
        napi_gro_receive(&tp->r8125napi[ring_index].napi, skb);
#endif
#else
        netif_rx(skb);
#endif
}

static int
rtl8125_check_rx_desc_error(struct net_device *dev,
                            struct rtl8125_private *tp,
                            u32 status)
{
        int ret = 0;

        if (tp->InitRxDescType == RX_DESC_RING_TYPE_3) {
                if (unlikely(status & RxRES_V3)) {
                        if (status & (RxRWT_V3 | RxRUNT_V3))
                                RTLDEV->stats.rx_length_errors++;
                        if (status & RxCRC_V3)
                                RTLDEV->stats.rx_crc_errors++;

                        ret = -1;
                }
        } else {
                if (unlikely(status & RxRES)) {
                        if (status & (RxRWT | RxRUNT))
                                RTLDEV->stats.rx_length_errors++;
                        if (status & RxCRC)
                                RTLDEV->stats.rx_crc_errors++;

                        ret = -1;
                }
        }

        return ret;
}

#ifdef ENABLE_PAGE_REUSE

static inline bool
rtl8125_reuse_rx_ok(struct page *page)
{
        /* avoid re-using remote pages */
        if (!dev_page_is_reusable(page)) {
                //printk(KERN_INFO "r8125 page pfmemalloc, can't reuse!\n");
                return false;
        }
        /* if we are only owner of page we can reuse it */
        if (unlikely(page_ref_count(page) != 1)) {
                //printk(KERN_INFO "r8125 page refcnt %d, can't reuse!\n", page_ref_count(page));
                return false;
        }

        return true;
}

static void
rtl8125_reuse_rx_buffer(struct rtl8125_private *tp, struct rtl8125_rx_ring *ring, u32 cur_rx, struct rtl8125_rx_buffer *rxb)
{
        struct page *page = rxb->page;

        u32 dirty_rx = ring->dirty_rx;
        u32 entry = dirty_rx % ring->num_rx_desc;
        struct rtl8125_rx_buffer *nrxb = &ring->rx_buffer[entry];

        u32 noffset;

        //the page gonna be shared by us and kernel, keep page ref = 2
        page_ref_inc(page);

        //flip the buffer in page to use next
        noffset = rxb->page_offset ^ (tp->rx_buf_page_size / 2); //one page, two buffer, ping-pong

        nrxb->dma = rxb->dma;
        nrxb->page_offset = noffset;
        nrxb->data = rxb->data;

        if (cur_rx != dirty_rx) {
                //move the buffer to other slot
                nrxb->page = page;
                rxb->page = NULL;
        }
}

static void rtl8125_put_rx_buffer(struct rtl8125_private *tp,
                                  struct rtl8125_rx_ring *ring,
                                  u32 cur_rx,
                                  struct rtl8125_rx_buffer *rxb)
{
        struct rtl8125_rx_buffer *nrxb;
        struct page *page = rxb->page;
        u32 entry;

        entry = ring->dirty_rx % ring->num_rx_desc;
        nrxb = &ring->rx_buffer[entry];
        if (likely(rtl8125_reuse_rx_ok(page))) {
                /* hand second half of page back to the ring */
                rtl8125_reuse_rx_buffer(tp, ring, cur_rx, rxb);
        } else {
                tp->page_reuse_fail_cnt++;

                dma_unmap_page_attrs(&tp->pci_dev->dev, rxb->dma,
                                     tp->rx_buf_page_size,
                                     DMA_FROM_DEVICE,
                                     (DMA_ATTR_SKIP_CPU_SYNC | DMA_ATTR_WEAK_ORDERING));
                //the page ref is kept 1, uniquely owned by kernel now
                rxb->page = NULL;

                return;
        }

        dma_sync_single_range_for_device(tp_to_dev(tp),
                                         nrxb->dma,
                                         nrxb->page_offset,
                                         tp->rx_buf_sz,
                                         DMA_FROM_DEVICE);

        rtl8125_map_to_asic(tp, ring, ring->RxDescArray + entry,
                            nrxb->dma + nrxb->page_offset,
                            tp->rx_buf_sz, entry);

        ring->dirty_rx++;
}

#endif //ENABLE_PAGE_REUSE

static int
rtl8125_rx_interrupt(struct net_device *dev,
                     struct rtl8125_private *tp,
                     struct rtl8125_rx_ring *ring,
                     napi_budget budget)
{
        unsigned int cur_rx, rx_left;
        unsigned int delta, count = 0;
        unsigned int entry;
        struct RxDesc *desc;
        struct sk_buff *skb;
        u32 status;
        u32 rx_quota;
        u32 ring_index = ring->index;
#ifdef ENABLE_PAGE_REUSE
        struct rtl8125_rx_buffer *rxb;
#else //ENABLE_PAGE_REUSE
        u64 rx_buf_phy_addr;
#endif //ENABLE_PAGE_REUSE
        unsigned int total_rx_multicast_packets = 0;
        unsigned int total_rx_bytes = 0, total_rx_packets = 0;

        assert(dev != NULL);
        assert(tp != NULL);

        if (ring->RxDescArray == NULL)
                goto rx_out;

        rx_quota = RTL_RX_QUOTA(budget);
        cur_rx = ring->cur_rx;
        rx_left = ring->num_rx_desc + ring->dirty_rx - cur_rx;
        rx_left = rtl8125_rx_quota(rx_left, (u32)rx_quota);

        for (; rx_left > 0; rx_left--, cur_rx++) {
                int pkt_size;
#ifndef ENABLE_PAGE_REUSE
                const void *rx_buf;
#endif //!ENABLE_PAGE_REUSE

                entry = cur_rx % ring->num_rx_desc;
                desc = rtl8125_get_rxdesc(tp, ring->RxDescArray, entry);
                status = le32_to_cpu(rtl8125_rx_desc_opts1(tp, desc));

                if (status & DescOwn)
                        break;

                rmb();

                if (unlikely(rtl8125_check_rx_desc_error(dev, tp, status) < 0)) {
                        if (netif_msg_rx_err(tp)) {
                                printk(KERN_INFO
                                       "%s: Rx ERROR. status = %08x\n",
                                       dev->name, status);
                        }

                        RTLDEV->stats.rx_errors++;

                        if (!(dev->features & NETIF_F_RXALL))
                                goto release_descriptor;
                }
                pkt_size = status & 0x00003fff;
                if (likely(!(dev->features & NETIF_F_RXFCS)) &&
                    !rtl8125_is_non_eop(tp, status))
                        pkt_size -= ETH_FCS_LEN;

                if (unlikely(pkt_size > tp->rx_buf_sz))
                        goto drop_packet;

#if !defined(ENABLE_RX_PACKET_FRAGMENT) || !defined(ENABLE_PAGE_REUSE)
                /*
                 * The driver does not support incoming fragmented
                 * frames. They are seen as a symptom of over-mtu
                 * sized frames.
                 */
                if (unlikely(rtl8125_fragmented_frame(tp, status)))
                        goto drop_packet;
#endif //!ENABLE_RX_PACKET_FRAGMENT || !ENABLE_PAGE_REUSE

#ifdef ENABLE_PTP_SUPPORT
                if (tp->EnablePtp) {
                        u8 desc_type;

                        desc_type = rtl8125_rx_desc_type(status);
                        if (desc_type == RXDESC_TYPE_NEXT && rx_left > 0) {
                                u32 status_next;
                                struct RxDescV3 *desc_next;
                                unsigned int entry_next;
                                struct sk_buff *skb_next;

                                entry_next = (cur_rx + 1) % ring->num_rx_desc;
                                desc_next = (struct RxDescV3 *)rtl8125_get_rxdesc(tp, ring->RxDescArray, entry_next);
                                rmb();
                                status_next = le32_to_cpu(desc_next->RxDescNormalDDWord4.opts1);
                                if (unlikely(status_next & DescOwn)) {
                                        udelay(1);
                                        rmb();
                                        status_next = le32_to_cpu(desc_next->RxDescNormalDDWord4.opts1);
                                        if (unlikely(status_next & DescOwn)) {
                                                if (netif_msg_rx_err(tp)) {
                                                        printk(KERN_ERR
                                                               "%s: Rx Next Desc ERROR. status = %08x\n",
                                                               dev->name, status_next);
                                                }
                                                break;
                                        }
                                }

                                cur_rx++;
                                rx_left--;
                                desc_type = rtl8125_rx_desc_type(status_next);
                                if (desc_type == RXDESC_TYPE_PTP)
                                        rtl8125_rx_ptp_pktstamp(tp, skb, desc_next);
                                else
                                        WARN_ON(1);

                                rx_buf_phy_addr = ring->RxDescPhyAddr[entry_next];
                                dma_unmap_single(tp_to_dev(tp), rx_buf_phy_addr,
                                                 tp->rx_buf_sz, DMA_FROM_DEVICE);
                                skb_next = ring->Rx_skbuff[entry_next];
                                dev_kfree_skb_any(skb_next);
                                ring->Rx_skbuff[entry_next] = NULL;
                        } else
                                WARN_ON(desc_type != RXDESC_TYPE_NORMAL);
                }
#endif
#ifdef ENABLE_PAGE_REUSE
                rxb = &ring->rx_buffer[entry];
                skb = rxb->skb;
                rxb->skb = NULL;
                if (!skb) {
                        skb = RTL_BUILD_SKB_INTR(rxb->data + rxb->page_offset - ring->rx_offset, tp->rx_buf_page_size / 2);
                        if (!skb) {
                                //netdev_err(tp->dev, "Failed to allocate RX skb!\n");
                                goto drop_packet;
                        }

                        skb->dev = dev;
                        skb_reserve(skb, R8125_RX_ALIGN);
                        skb_put(skb, pkt_size);
                } else
                        skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, rxb->page,
                                        rxb->page_offset, pkt_size, tp->rx_buf_page_size / 2);

                //recycle desc
                rtl8125_put_rx_buffer(tp, ring, cur_rx, rxb);

                dma_sync_single_range_for_cpu(tp_to_dev(tp),
                                              rxb->dma,
                                              rxb->page_offset,
                                              tp->rx_buf_sz,
                                              DMA_FROM_DEVICE);
#else //ENABLE_PAGE_REUSE
                skb = RTL_ALLOC_SKB_INTR(&tp->r8125napi[ring->index].napi, pkt_size + R8125_RX_ALIGN);
                if (!skb) {
                        //netdev_err(tp->dev, "Failed to allocate RX skb!\n");
                        goto drop_packet;
                }

                skb->dev = dev;
                skb_reserve(skb, R8125_RX_ALIGN);
                skb_put(skb, pkt_size);

                rx_buf_phy_addr = ring->RxDescPhyAddr[entry];
                dma_sync_single_for_cpu(tp_to_dev(tp),
                                        rx_buf_phy_addr, tp->rx_buf_sz,
                                        DMA_FROM_DEVICE);
                rx_buf = ring->Rx_skbuff[entry]->data;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
                prefetch(rx_buf - R8125_RX_ALIGN);
#endif
                eth_copy_and_sum(skb, rx_buf, pkt_size, 0);

                dma_sync_single_for_device(tp_to_dev(tp), rx_buf_phy_addr,
                                           tp->rx_buf_sz, DMA_FROM_DEVICE);
#endif //ENABLE_PAGE_REUSE

#ifdef ENABLE_RX_PACKET_FRAGMENT
                if (rtl8125_is_non_eop(tp, status)) {
                        unsigned int entry_next;
                        entry_next = (entry + 1) % ring->num_rx_desc;
                        rxb = &ring->rx_buffer[entry_next];
                        rxb->skb = skb;
                        continue;
                }
#endif //ENABLE_RX_PACKET_FRAGMENT

#ifdef ENABLE_RSS_SUPPORT
                rtl8125_rx_hash(tp, (struct RxDescV3 *)desc, skb);
#endif
                rtl8125_rx_csum(tp, skb, desc, status);

                skb->protocol = eth_type_trans(skb, dev);

                total_rx_bytes += skb->len;

                if (skb->pkt_type == PACKET_MULTICAST)
                        total_rx_multicast_packets++;

                if (rtl8125_rx_vlan_skb(tp, desc, skb) < 0)
                        rtl8125_rx_skb(tp, skb, ring_index);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
                dev->last_rx = jiffies;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
                total_rx_packets++;

#ifdef ENABLE_PAGE_REUSE
                rxb->skb = NULL;
                continue;
#endif

release_descriptor:
                rtl8125_mark_to_asic(tp, desc, tp->rx_buf_sz);
                continue;
drop_packet:
                RTLDEV->stats.rx_dropped++;
                RTLDEV->stats.rx_length_errors++;
                goto release_descriptor;
        }

        count = cur_rx - ring->cur_rx;
        ring->cur_rx = cur_rx;

        delta = rtl8125_rx_fill(tp, ring, dev, ring->dirty_rx, ring->cur_rx, 1);
        if (!delta && count && netif_msg_intr(tp))
                printk(KERN_INFO "%s: no Rx buffer allocated\n", dev->name);
        ring->dirty_rx += delta;

        RTLDEV->stats.rx_bytes += total_rx_bytes;
        RTLDEV->stats.rx_packets += total_rx_packets;
        RTLDEV->stats.multicast += total_rx_multicast_packets;

        /*
         * FIXME: until there is periodic timer to try and refill the ring,
         * a temporary shortage may definitely kill the Rx process.
         * - disable the asic to try and avoid an overflow and kick it again
         *   after refill ?
         * - how do others driver handle this condition (Uh oh...).
         */
        if ((ring->dirty_rx + ring->num_rx_desc == ring->cur_rx) && netif_msg_intr(tp))
                printk(KERN_EMERG "%s: Rx buffers exhausted\n", dev->name);

rx_out:
        return total_rx_packets;
}

static bool
rtl8125_linkchg_interrupt(struct rtl8125_private *tp, u32 status)
{
        if (tp->HwCurrIsrVer == 2)
                return status & ISRIMR_V2_LINKCHG;

        return status & LinkChg;
}

/*
 *The interrupt handler does all of the Rx thread work and cleans up after
 *the Tx thread.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8125_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t rtl8125_interrupt(int irq, void *dev_instance)
#endif
{
        struct r8125_napi *r8125napi = dev_instance;
        struct rtl8125_private *tp = r8125napi->priv;
        struct net_device *dev = tp->dev;
        u32 status;
        int handled = 0;

        do {
                status = RTL_R32(tp, tp->isr_reg[0]);

                if (!(tp->features & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX))) {
                        /* hotplug/major error/no more work/shared irq */
                        if (!status)
                                break;

                        if (status == 0xFFFFFFFF)
                                break;

                        if (!(status & (tp->intr_mask | tp->timer_intr_mask)))
                                break;
                }

                handled = 1;

#if defined(RTL_USE_NEW_INTR_API)
                if (!tp->irq_tbl[0].requested)
                        break;
#endif
                rtl8125_disable_hw_interrupt(tp);

                RTL_W32(tp, tp->isr_reg[0], status&~RxFIFOOver);

                if (rtl8125_linkchg_interrupt(tp, status))
                        rtl8125_schedule_linkchg_work(tp);

#ifdef ENABLE_DASH_SUPPORT
                if (tp->DASH) {
                        if (HW_DASH_SUPPORT_TYPE_3(tp)) {
                                u8 DashIntType2Status;

                                if (status & ISRIMR_DASH_INTR_CMAC_RESET)
                                        tp->CmacResetIntr = TRUE;

                                DashIntType2Status = RTL_CMAC_R8(tp, CMAC_IBISR0);
                                if (DashIntType2Status & ISRIMR_DASH_TYPE2_ROK) {
                                        tp->RcvFwDashOkEvt = TRUE;
                                }
                                if (DashIntType2Status & ISRIMR_DASH_TYPE2_TOK) {
                                        tp->SendFwHostOkEvt = TRUE;
                                }
                                if (DashIntType2Status & ISRIMR_DASH_TYPE2_RX_DISABLE_IDLE) {
                                        tp->DashFwDisableRx = TRUE;
                                }

                                RTL_CMAC_W8(tp, CMAC_IBISR0, DashIntType2Status);
                        }
                }
#endif

#ifdef CONFIG_R8125_NAPI
                if (status & tp->intr_mask || tp->keep_intr_cnt-- > 0) {
                        if (status & tp->intr_mask)
                                tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;

                        if (likely(RTL_NETIF_RX_SCHEDULE_PREP(dev, &tp->r8125napi[0].napi)))
                                __RTL_NETIF_RX_SCHEDULE(dev, &tp->r8125napi[0].napi);
                        else if (netif_msg_intr(tp))
                                printk(KERN_INFO "%s: interrupt %04x in poll\n",
                                       dev->name, status);
                } else {
                        tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;
                        rtl8125_switch_to_hw_interrupt(tp);
                }
#else
                if (status & tp->intr_mask || tp->keep_intr_cnt-- > 0) {
                        u32 budget = ~(u32)0;
                        int i;

                        if (status & tp->intr_mask)
                                tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;

                        for (i = 0; i < tp->num_tx_rings; i++)
                                rtl8125_tx_interrupt(&tp->tx_ring[i], ~(u32)0);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
                        rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[0], &budget);
#else
                        rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[0], budget);
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)

#ifdef ENABLE_DASH_SUPPORT
                        if (tp->DASH) {
                                struct net_device *dev = tp->dev;

                                HandleDashInterrupt(dev);
                        }
#endif

                        rtl8125_switch_to_timer_interrupt(tp);
                } else {
                        tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;
                        rtl8125_switch_to_hw_interrupt(tp);
                }
#endif
        } while (false);

        return IRQ_RETVAL(handled);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8125_interrupt_msix(int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t rtl8125_interrupt_msix(int irq, void *dev_instance)
#endif
{
        struct r8125_napi *r8125napi = dev_instance;
        struct rtl8125_private *tp = r8125napi->priv;
        struct net_device *dev = tp->dev;
        int message_id = r8125napi->index;
#ifndef CONFIG_R8125_NAPI
        u32 budget = ~(u32)0;
#endif

        do {
#if defined(RTL_USE_NEW_INTR_API)
                if (!tp->irq_tbl[message_id].requested)
                        break;
#endif
                rtl8125_disable_hw_interrupt_v2(tp, message_id);

                rtl8125_clear_hw_isr_v2(tp, message_id);

                //link change
                if (message_id == 21) {
                        rtl8125_schedule_linkchg_work(tp);
                        break;
                }

#ifdef CONFIG_R8125_NAPI
                if (likely(RTL_NETIF_RX_SCHEDULE_PREP(dev, &r8125napi->napi)))
                        __RTL_NETIF_RX_SCHEDULE(dev, &r8125napi->napi);
                else if (netif_msg_intr(tp))
                        printk(KERN_INFO "%s: interrupt message id %d in poll_msix\n",
                               dev->name, message_id);
#else
                rtl8125_tx_interrupt_with_vector(tp, message_id, ~(u32)0);

                if (message_id < tp->num_rx_rings) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
                        rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[message_id], &budget);
#else
                        rtl8125_rx_interrupt(dev, tp, &tp->rx_ring[message_id], budget);
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
                }

                rtl8125_enable_hw_interrupt_v2(tp, message_id);
#endif

        } while (false);

        return IRQ_HANDLED;
}

static void rtl8125_down(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        //rtl8125_delete_esd_timer(dev, &tp->esd_timer);

        //rtl8125_delete_link_timer(dev, &tp->link_timer);

        netif_carrier_off(dev);

        netif_tx_disable(dev);

        _rtl8125_wait_for_quiescence(dev);

        rtl8125_hw_reset(dev);

        rtl8125_tx_clear(tp);

        rtl8125_rx_clear(tp);
}

static int rtl8125_resource_freed(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < tp->num_tx_rings; i++)
                if (tp->tx_ring[i].TxDescArray) return 0;

        for (i = 0; i < tp->num_rx_rings; i++)
                if (tp->rx_ring[i].RxDescArray) return 0;

        return 1;
}

int rtl8125_close(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if (!rtl8125_resource_freed(tp)) {
                set_bit(R8125_FLAG_DOWN, tp->task_flags);

                rtl8125_down(dev);

                pci_clear_master(tp->pci_dev);

#ifdef ENABLE_PTP_SUPPORT
                rtl8125_ptp_stop(tp);
#endif
                rtl8125_hw_d3_para(dev);

                rtl8125_powerdown_pll(dev, 0);

                rtl8125_free_irq(tp);

                rtl8125_free_alloc_resources(tp);
        } else {
                rtl8125_hw_d3_para(dev);

                rtl8125_powerdown_pll(dev, 0);
        }

        return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
static void rtl8125_shutdown(struct pci_dev *pdev)
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8125_private *tp = netdev_priv(dev);

        rtnl_lock();

        if (HW_DASH_SUPPORT_DASH(tp))
                rtl8125_driver_stop(tp);

        rtl8125_set_bios_setting(dev);
        if (s5_keep_curr_mac == 0 && tp->random_mac == 0)
                rtl8125_rar_set(tp, tp->org_mac_addr);

        if (s5wol == 0)
                tp->wol_enabled = WOL_DISABLED;

        rtl8125_close(dev);
        rtl8125_disable_msi(pdev, tp);

        rtnl_unlock();

        if (system_state == SYSTEM_POWER_OFF) {
                pci_clear_master(tp->pci_dev);
                pci_wake_from_d3(pdev, tp->wol_enabled);
                pci_set_power_state(pdev, PCI_D3hot);
        }
}
#endif

#ifdef CONFIG_PM

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static int
rtl8125_suspend(struct pci_dev *pdev, u32 state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int
rtl8125_suspend(struct device *device)
#else
static int
rtl8125_suspend(struct pci_dev *pdev, pm_message_t state)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
        struct pci_dev *pdev = to_pci_dev(device);
        struct net_device *dev = pci_get_drvdata(pdev);
#else
        struct net_device *dev = pci_get_drvdata(pdev);
#endif
        struct rtl8125_private *tp = netdev_priv(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        u32 pci_pm_state = pci_choose_state(pdev, state);
#endif
        if (!netif_running(dev))
                goto out;

        //rtl8125_cancel_all_schedule_work(tp);

        //rtl8125_delete_esd_timer(dev, &tp->esd_timer);

        //rtl8125_delete_link_timer(dev, &tp->link_timer);

        rtnl_lock();

        set_bit(R8125_FLAG_DOWN, tp->task_flags);

        netif_carrier_off(dev);

        netif_tx_disable(dev);

        netif_device_detach(dev);

#ifdef ENABLE_PTP_SUPPORT
        rtl8125_ptp_suspend(tp);
#endif
        rtl8125_hw_reset(dev);

        pci_clear_master(pdev);

        rtl8125_hw_d3_para(dev);

        rtl8125_powerdown_pll(dev, 1);

        if (HW_DASH_SUPPORT_DASH(tp))
                rtl8125_driver_stop(tp);

        rtnl_unlock();
out:

        pci_disable_device(pdev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        pci_save_state(pdev, &pci_pm_state);
#else
        pci_save_state(pdev);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
        pci_enable_wake(pdev, pci_choose_state(pdev, state), tp->wol_enabled);
#endif

        pci_prepare_to_sleep(pdev);

        return 0;
}

static int
rtl8125_hw_d3_not_power_off(struct net_device *dev)
{
        return rtl8125_check_hw_phy_mcu_code_ver(dev);
}

static int rtl8125_wait_phy_nway_complete_sleep(struct rtl8125_private *tp)
{
        int i, val;

        for (i = 0; i < 30; i++) {
                val = rtl8125_mdio_read(tp, MII_BMSR) & BMSR_ANEGCOMPLETE;
                if (val)
                        return 0;

                msleep(100);
        }

        return -1;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
static int
rtl8125_resume(struct pci_dev *pdev)
#else
static int
rtl8125_resume(struct device *device)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
        struct pci_dev *pdev = to_pci_dev(device);
        struct net_device *dev = pci_get_drvdata(pdev);
#else
        struct net_device *dev = pci_get_drvdata(pdev);
#endif
        struct rtl8125_private *tp = netdev_priv(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        u32 pci_pm_state = PCI_D0;
#endif
        u32 err;

        rtnl_lock();

        err = pci_enable_device(pdev);
        if (err) {
                dev_err(&pdev->dev, "Cannot enable PCI device from suspend\n");
                goto out_unlock;
        }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        pci_restore_state(pdev, &pci_pm_state);
#else
        pci_restore_state(pdev);
#endif
        pci_enable_wake(pdev, PCI_D0, 0);

        /* restore last modified mac address */
        rtl8125_rar_set(tp, dev->dev_addr);

        tp->resume_not_chg_speed = 0;
        if (tp->check_keep_link_speed &&
            //tp->link_ok(dev) &&
            rtl8125_hw_d3_not_power_off(dev) &&
            rtl8125_wait_phy_nway_complete_sleep(tp) == 0)
                tp->resume_not_chg_speed = 1;

        if (!netif_running(dev))
                goto out_unlock;

        pci_set_master(pdev);

        rtl8125_exit_oob(dev);

        rtl8125_up(dev);

        clear_bit(R8125_FLAG_DOWN, tp->task_flags);

        rtl8125_schedule_reset_work(tp);

        rtl8125_schedule_esd_work(tp);

        //mod_timer(&tp->esd_timer, jiffies + RTL8125_ESD_TIMEOUT);
        //mod_timer(&tp->link_timer, jiffies + RTL8125_LINK_TIMEOUT);
out_unlock:
        netif_device_attach(dev);

        rtnl_unlock();

        return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)

static struct dev_pm_ops rtl8125_pm_ops = {
        .suspend = rtl8125_suspend,
        .resume = rtl8125_resume,
        .freeze = rtl8125_suspend,
        .thaw = rtl8125_resume,
        .poweroff = rtl8125_suspend,
        .restore = rtl8125_resume,
};

#define RTL8125_PM_OPS	(&rtl8125_pm_ops)

#endif

#else /* !CONFIG_PM */

#define RTL8125_PM_OPS	NULL

#endif /* CONFIG_PM */

static struct pci_driver rtl8125_pci_driver = {
        .name       = MODULENAME,
        .id_table   = rtl8125_pci_tbl,
        .probe      = rtl8125_init_one,
        .remove     = __devexit_p(rtl8125_remove_one),
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
        .shutdown   = rtl8125_shutdown,
#endif
#ifdef CONFIG_PM
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
        .suspend    = rtl8125_suspend,
        .resume     = rtl8125_resume,
#else
        .driver.pm	= RTL8125_PM_OPS,
#endif
#endif
};

static int __init
rtl8125_init_module(void)
{
        int ret = 0;
#ifdef ENABLE_R8125_PROCFS
        rtl8125_proc_module_init();
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

        ret = pci_register_driver(&rtl8125_pci_driver);
#else
        ret = pci_module_init(&rtl8125_pci_driver);
#endif

        return ret;
}

static void __exit
rtl8125_cleanup_module(void)
{
        pci_unregister_driver(&rtl8125_pci_driver);

#ifdef ENABLE_R8125_PROCFS
        if (rtl8125_proc) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                remove_proc_subtree(MODULENAME, init_net.proc_net);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
                remove_proc_entry(MODULENAME, init_net.proc_net);
#else
                remove_proc_entry(MODULENAME, proc_net);
#endif  //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#endif  //LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                rtl8125_proc = NULL;
        }
#endif
}

module_init(rtl8125_init_module);
module_exit(rtl8125_cleanup_module);
