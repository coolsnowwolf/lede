/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef __FXGMAC_OS_H__
#define __FXGMAC_OS_H__

#include <linux/dma-mapping.h>
//#include <linux/timecounter.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/pm_wakeup.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bitrev.h>
#include <net/addrconf.h>
#include <linux/bitops.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/fcntl.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/crc32.h>
#include <linux/dcbnl.h>
#include <linux/timer.h>
#include <linux/inet.h>
#include <linux/init.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/clk.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/ip.h>

#ifdef CONFIG_PCI_MSI
#include <linux/pci.h>
#endif

#define LINUX

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifndef RHEL_MAJOR

//must used like : if ( RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(a,b) )
#ifndef RHEL_RELEASE_VERSION
#define RHEL_RELEASE_VERSION(a,b)  (((a) << 8) + (b))
#define RHEL_RELEASE_CODE (0)
#endif

#include <linux/timecounter.h>
#else
#if (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,8))
#include <linux/timecounter.h>
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#include <linux/crc32poly.h>
#endif

#include "fuxi-dbg.h"

struct fxgmac_ring;
struct fxgmac_pdata;
struct fxgmac_channel;

#define FXGMAC_DRV_VERSION                  "1.0.30"

#ifdef CONFIG_PCI_MSI
/* undefined for legacy interrupt mode */
/* #undef CONFIG_PCI_MSI */
#endif

#define FXGMAC_INT_MODERATION_ENABLED   1

#define PCIE_LP_ASPM_L0S                    1
#define PCIE_LP_ASPM_L1                     2
#define PCIE_LP_ASPM_L1SS                   4
#define PCIE_LP_ASPM_LTR                    8

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
/*
 * There are multiple 16-bit CRC polynomials in common use, but this is
 * *the* standard CRC-32 polynomial, first popularized by Ethernet.
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
 */
#define CRC32_POLY_LE 0xedb88320
#define CRC32_POLY_BE 0x04c11db7

/*
 * This is the CRC32c polynomial, as outlined by Castagnoli.
 * x^32+x^28+x^27+x^26+x^25+x^23+x^22+x^20+x^19+x^18+x^14+x^13+x^11+x^10+x^9+
 * x^8+x^6+x^0
 */
#define CRC32C_POLY_LE 0x82F63B78
#endif

#define FXGMAC_FAIL    -1
#define FXGMAC_SUCCESS     0
#define FXGMAC_DEV_CMD      (SIOCDEVPRIVATE + 1)
#define FXGMAC_IOCTL_DFS_COMMAND    _IOWR('M', 0x80, struct ext_ioctl_data)

#define FXGMAC_MAX_DBG_TEST_PKT     150
#define FXGMAC_MAX_DBG_BUF_LEN      64000
#define FXGMAC_MAX_DBG_RX_DATA      1600
#define FXGMAC_NETDEV_OPS_BUF_LEN   256

#define FXGMAC_TEST_MAC_HEAD_LEN        14

#define FXGMAC_PM_WPI_READ_FEATURE_ENABLED         1

#define RSS_Q_COUNT                         4

/* #define FXGMAC_TX_INTERRUPT_ENABLED		1 */
#define FXGMAC_TX_HANG_TIMER_ENABLED             0
/* 1 to trigger(write reg 0x1000) for sniffer stop */
#define FXGMAC_TRIGGER_TX_HANG		0

/* driver feature configuration */
#if FXGMAC_TX_HANG_TIMER_ENABLED
/* 0: check hw current desc; 1: check software dirty */
#define FXGMAC_TX_HANG_CHECH_DIRTY	0
#endif

#define FXGMAC_DMA_BIT_MASK64       64
#define FXGMAC_DMA_BIT_MASK32       32

#ifdef CONFIG_PCI_MSI
/* should be same as FXGMAC_MAX_DMA_CHANNELS + 1 tx_irq */
#define FXGMAC_MAX_MSIX_Q_VECTORS	(FXGMAC_MSIX_Q_VECTORS + 1)
#define FXGMAC_MSIX_CH0RXDIS_ENABLED		0 //set to 1 for ch0 unbalance fix;
#define FXGMAC_MSIX_INTCTRL_EN      1

#ifdef FXGMAC_MISC_NOT_ENABLED
#define FXGMAC_MISC_INT_NUM          0
#else
#define FXGMAC_MISC_INT_NUM          1
#endif
#define FXGMAC_MSIX_INT_NUMS (FXGMAC_MAX_MSIX_Q_VECTORS + FXGMAC_MISC_INT_NUM)
#else
#define FXGMAC_MSIX_CH0RXDIS_ENABLED		0   /* NO modification needed! for non-MSI, set to 0 always */
#define FXGMAC_MSIX_INTCTRL_EN      0
#endif

/* RSS features */
#ifdef FXGMAC_ONE_CHANNEL
#define FXGMAC_RSS_FEATURE_ENABLED	0 /* 1:enable rss ; 0: rss not included. */
#else
#define FXGMAC_RSS_FEATURE_ENABLED	1 /* 1:enable rss ; 0: rss not included. */
#endif
#define FXGMAC_RSS_HASH_KEY_LINUX	1 /* 0:hard to default rss key ;1: normal hash key process from Linux. */

/* WOL features */
#define FXGMAC_WOL_FEATURE_ENABLED	1 /* 1:enable wol ; 0: wol not included. */
/* since wol upon link will cause issue, disabled it always. */
#define FXGMAC_WOL_UPON_EPHY_LINK	1 /* 1:enable ephy link change wol ; 0: ephy link change wol is not supported. */

/* Pause features */
#define FXGMAC_PAUSE_FEATURE_ENABLED	1 /* 1:enable flow control/pause framce ; 0: flow control/pause frame not included. */

/* ARP offload engine (AOE) */
#define FXGMAC_AOE_FEATURE_ENABLED	1 /* 1:enable arp offload engine ; 0: aoe is not included. */

/* NS offload engine */
#define FXGMAC_NS_OFFLOAD_ENABLED	1 /* 1:enable NS offload for IPv6 ; 0: NS is not included. */

/* for fpga ver after, which needs release phy before set of MAC tx/rx */
#define FXGMAC_TXRX_EN_AFTER_PHY_RELEASE	1 /* 1:release ephy before mac tx/rx bits are set. */

/*  power management features */
#define FXGMAC_PM_FEATURE_ENABLED	1 /* 1:enable PM ; 0: PM not included. */

/* sanity check */
#define FXGMAC_SANITY_CHECK_ENABLED	0 /* 1:enable health checking ; */

/* vlan id filter */
#define FXGMAC_FILTER_SINGLE_VLAN_ENABLED	0

/* Linux driver implement VLAN HASH Table feature to support mutliple VLAN feautre */
#define FXGMAC_FILTER_MULTIPLE_VLAN_ENABLED 1

/* Linux driver implement MAC HASH Table feature */
#define FXGMAC_MAC_HASH_TABLE 1

/* Linux driver implement write multiple mac addr */
#define FXGMAC_FILTER_MULTIPLE_MAC_ADDR_ENABLED 1

/* Linux driver disable MISC Interrupt */
#define FXGMAC_MISC_INT_HANDLE_FEATURE_ENABLED             1

#define FXGMAC_ESD_RESTORE_PCIE_CFG

#define FXGMAC_WOL_INTEGRATED_WOL_PARAMETER

#define FXGMAC_LINK_SPEED_CHECK_PHY_LINK

#define FXGMAC_FLUSH_TX_CHECK_ENABLED

#define FXGMAC_POWER_MANAGEMENT

#define FXGMAC_INTERRUPT_TX_INTERVAL

#define FXGMAC_INTERRUPT_RX_INTERVAL

#define FXGMAC_WAIT_TX_STOP

#define FXGMAC_WAIT_RX_STOP_BY_PRXQ_RXQSTS

#define FXGMAC_USE_DEFAULT_RSS_KEY_TBALE

#define FXGMAC_MISC_NOT_ENABLED

#define FXGMAC_RX_VLAN_FILTERING_ENABLED  (pdata->netdev->features & NETIF_F_HW_VLAN_CTAG_FILTER)

#define FXGMAC_NETDEV_PR_MODE_ENABLED ((pdata->netdev->flags & IFF_PROMISC) != 0)
#define FXGMAC_NETDEV_AM_MODE_ENABLED ((pdata->netdev->flags & IFF_ALLMULTI) != 0)
#define FXGMAC_NETDEV_MU_MODE_ENABLED ((pdata->netdev->flags & IFF_MULTICAST) != 0)
#define FXGMAC_NETDEV_BD_MODE_ENABLED ((pdata->netdev->flags & IFF_BROADCAST) != 0)

#define FXGMAC_RX_CHECKSUM_ENABLED (pdata->netdev->features & NETIF_F_RXCSUM)

#define TEST_MAC_HEAD               14
#define TEST_TCP_HEAD_LEN_OFFSET    12
#define TEST_TCP_OFFLOAD_LEN_OFFSET 48
#define TEST_TCP_FIX_HEAD_LEN       24
#define TEST_TCP_MSS_OFFSET         56

#define DF_MAX_NIC_NUM      16

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &(((TYPE*)0)->MEMBER))
#endif

#define ETH_IS_ZEROADDRESS(Address)               \
    ((((u8*)(Address))[0] == ((u8)0x00))            \
    && (((u8*)(Address))[1] == ((u8)0x00))          \
    && (((u8*)(Address))[2] == ((u8)0x00))          \
    && (((u8*)(Address))[3] == ((u8)0x00))          \
    && (((u8*)(Address))[4] == ((u8)0x00))          \
    && (((u8*)(Address))[5] == ((u8)0x00)))

/* centos 7 define start */
#ifndef dma_rmb
#define dma_rmb()	barrier()
#endif

#ifndef dma_wmb
#define dma_wmb()	barrier()
#endif

#ifndef smp_mb__before_atomic
#define smp_mb__before_atomic()	barrier()
#endif

#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic()	barrier()
#endif

#ifndef skb_vlan_tag_present
#define skb_vlan_tag_present(__skb)	((__skb)->vlan_tci & VLAN_TAG_PRESENT)
#endif

#ifndef skb_vlan_tag_get
#define skb_vlan_tag_get(__skb)		((__skb)->vlan_tci & ~VLAN_TAG_PRESENT)
#endif

#ifndef GENMASK
/* Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif

#ifndef ETH_RSS_HASH_TOP
#define __ETH_RSS_HASH_BIT(bit)	((u32)1 << (bit))
#define __ETH_RSS_HASH(name)	__ETH_RSS_HASH_BIT(ETH_RSS_HASH_##name##_BIT)

#define ETH_RSS_HASH_TOP	__ETH_RSS_HASH(TOP)
#endif

/* centos 7 needs set these definitions to 0
 * 1. FXGMAC_RSS_FEATURE_ENABLED
 * 2. FXGMAC_RSS_HASH_KEY_LINUX
 * 3. FXGMAC_PAUSE_FEATURE_ENABLED
 *  #undef CONFIG_PCI_MSI
 */
#if (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,0)) && (RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,8))

#define strscpy		strlcpy

#undef FXGMAC_RSS_FEATURE_ENABLED
#define FXGMAC_RSS_FEATURE_ENABLED	0

#undef FXGMAC_RSS_HASH_KEY_LINUX
#define FXGMAC_RSS_HASH_KEY_LINUX	0

#undef FXGMAC_PAUSE_FEATURE_ENABLED
#define FXGMAC_PAUSE_FEATURE_ENABLED	0

#undef CONFIG_PCI_MSI
#endif

/* centos 7 define end */

 /* read from 8bit register via pci config space */
#define cfg_r8(_pdata, reg, pdat) pci_read_config_byte((_pdata)->pdev, (reg), (u8 *)(pdat))

 /* read from 16bit register via pci config space */
#define cfg_r16(_pdata, reg, pdat) pci_read_config_word((_pdata)->pdev, (reg), (u16 *)(pdat))

 /* read from 32bit register via pci config space */
#define cfg_r32(_pdata, reg, pdat) pci_read_config_dword((_pdata)->pdev, (reg), (u32 *)(pdat))

/* write to 8bit register via pci config space */
#define cfg_w8(_pdata, reg, val) pci_write_config_byte((_pdata)->pdev, (reg), (u8)(val))

/* write to 16bit register via pci config space */
#define cfg_w16(_pdata, reg, val) pci_write_config_word((_pdata)->pdev, (reg), (u16)(val))

/* write to 32bit register via pci config space */
#define cfg_w32(_pdata, reg, val) pci_write_config_dword((_pdata)->pdev, (reg), (u32)(val))

#define readreg(pAdapter, addr) (readl(addr))
#define writereg(pAdapter, val, addr) (writel(val, addr))
#define usleep_range_ex(pAdapter, a, b) (usleep_range(a, b))
#define _CR(Record, TYPE, Field)  ((TYPE *) ((char *) (Record) - (char *) &(((TYPE *) 0)->Field)))

#define FXGMAC_GET_REG_BITS(var, pos, len) ({				\
    typeof(pos) _pos = (pos);					            \
    typeof(len) _len = (len);					            \
    ((var) & GENMASK(_pos + _len - 1, _pos)) >> (_pos);		\
})

#define FXGMAC_GET_REG_BITS_LE(var, pos, len) ({			\
    typeof(pos) _pos = (pos);					            \
    typeof(len) _len = (len);					            \
    typeof(var) _var = le32_to_cpu((var));				    \
    ((_var) & GENMASK(_pos + _len - 1, _pos)) >> (_pos);	\
})

#define FXGMAC_SET_REG_BITS(var, pos, len, val) ({			\
    typeof(var) _var = (var);					            \
    typeof(pos) _pos = (pos);					            \
    typeof(len) _len = (len);					            \
    typeof(val) _val = (val);					            \
    _val = (_val << _pos) & GENMASK(_pos + _len - 1, _pos);	\
    _var = (_var & ~GENMASK(_pos + _len - 1, _pos)) | _val;	\
})

#define FXGMAC_SET_REG_BITS_LE(var, pos, len, val) ({		\
    typeof(var) _var = (var);					            \
    typeof(pos) _pos = (pos);					            \
    typeof(len) _len = (len);					            \
    typeof(val) _val = (val);					            \
    _val = (_val << _pos) & GENMASK(_pos + _len - 1, _pos);	\
    _var = (_var & ~GENMASK(_pos + _len - 1, _pos)) | _val;	\
    cpu_to_le32(_var);						                \
})

#define STR_FORMAT      "%s"

#define DbgPrintF(level,  fmt, ...)
#define DBGPRINT(Level, Fmt)
#define DBGPRINT_RAW(Level, Fmt)
#define DBGPRINT_S(Status, Fmt)
#define DBGPRINT_UNICODE(Level, UString)
#define Dump(p,cb,fAddress,ulGroup)

#undef ASSERT
#define ASSERT(x)

#define DbgPrintOidName(_Oid)
#define DbgPrintAddress(_pAddress)

// #define fxgmac_dump_buffer(_skb, _len, _tx_rx)
#define DumpLine(_p, _cbLine, _fAddress, _ulGroup )

#ifndef __far
#define __far
#endif

#ifndef FXGMAC_DEBUG
/* #define FXGMAC_DEBUG */
#endif

/* For debug prints */
#ifdef FXGMAC_DEBUG
#define FXGMAC_PR(fmt, args...) \
    pr_alert("[%s,%d]:" fmt, __func__, __LINE__, ## args)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
/*
 * If you want to continue a line, you NEED to use KERN_CONT.
 * That has always been true. It hasn't always been enforced, though.
 * If you do two printk's and the second one doesn't say "I'm a continuation",
 * the printk logic assumes you're just confused and wanted two lines.
 */
#define DPRINTK(fmt, args...) \
    printk(KERN_CONT fmt, ## args)
#else
#define DPRINTK printk
#endif

#else
#define FXGMAC_PR(x...)		do { } while (0)
#define DPRINTK(x...)
#endif

#define IOC_MAGIC 'M'
#define IOC_MAXNR (0x80 + 5)

#define FXGMAC_DFS_IOCTL_DEVICE_INACTIVE   0x10001
#define FXGMAC_DFS_IOCTL_DEVICE_RESET      0x10002
#define FXGMAC_DFS_IOCTL_DIAG_BEGIN        0x10003
#define FXGMAC_DFS_IOCTL_DIAG_END          0x10004
#define FXGMAC_DFS_IOCTL_DIAG_TX_PKT       0x10005
#define FXGMAC_DFS_IOCTL_DIAG_RX_PKT       0x10006

#define FXGMAC_EFUSE_UPDATE_LED_CFG                  0x10007
#define FXGMAC_EFUSE_WRITE_LED                       0x10008
#define FXGMAC_EFUSE_WRITE_PATCH_REG                 0x10009
#define FXGMAC_EFUSE_WRITE_PATCH_PER_INDEX           0x1000A
#define FXGMAC_EFUSE_WRITE_OOB                       0x1000B
#define FXGMAC_EFUSE_LOAD                            0x1000C
#define FXGMAC_EFUSE_READ_REGIONABC                  0x1000D
#define FXGMAC_EFUSE_READ_PATCH_REG                  0x1000E
#define FXGMAC_EFUSE_READ_PATCH_PER_INDEX            0x1000F
#define FXGMAC_EFUSE_LED_TEST                        0x10010

#define FXGMAC_GET_MAC_DATA                         0x10011
#define FXGMAC_SET_MAC_DATA                         0x10012
#define FXGMAC_GET_SUBSYS_ID                        0x10013
#define FXGMAC_SET_SUBSYS_ID                        0x10014
#define FXGMAC_GET_REG                              0x10015
#define FXGMAC_SET_REG                              0x10016
#define FXGMAC_GET_PHY_REG                          0x10017
#define FXGMAC_SET_PHY_REG                          0x10018
#define FXGMAC_EPHY_STATISTICS                       0x10019
#define FXGMAC_GET_STATISTICS                       0x1001A
#define FXGMAC_GET_PCIE_LOCATION                    0x1001B

#define FXGMAC_GET_GSO_SIZE                         0x1001C
#define FXGMAC_SET_GSO_SIZE                         0x1001D
#define FXGMAC_SET_RX_MODERATION                    0x1001E
#define FXGMAC_SET_TX_MODERATION                    0x1001F
#define FXGMAC_GET_TXRX_MODERATION                  0x10020

#define MAX_PKT_BUF                 1
#define FXGAMC_MAX_DATA_SIZE (1024 * 4 + 16)

#ifndef PCI_CAP_ID_MSI
#define PCI_CAP_ID_MSI      0x05    /* Message Signalled Interrupts */
#endif

#ifndef PCI_CAP_ID_MSIX
#define PCI_CAP_ID_MSIX     0x11    /* MSI-X */
#endif

#define PCI_CAP_ID_MSI_ENABLE_POS   0x10
#define PCI_CAP_ID_MSI_ENABLE_LEN   0x1
#define PCI_CAP_ID_MSIX_ENABLE_POS  0x1F
#define PCI_CAP_ID_MSIX_ENABLE_LEN  0x1

#define FXGMAC_IRQ_ENABLE       0x1
#define FXGMAC_IRQ_DISABLE      0x0
#define FXGMAC_NAPI_ENABLE      0x1
#define FXGMAC_NAPI_DISABLE     0x0

#ifndef fallthrough

#ifdef __has_attribute
#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

#endif

#define PHY_POWER_DOWN  1
#define PHY_POWER_UP    0

#define FXGMAC_MMC_IER_ALL_DEFAULT      0

/* #define FXGMAC_ESD_CHECK_ENABLED */
#ifdef FXGMAC_ESD_CHECK_ENABLED
#define FXGMAC_ESD_INTERVAL     (5 * HZ)
#define FXGMAC_ESD_ERROR_THRESHOLD  (u64)4000000000
#define FXGMAC_PCIE_LINK_DOWN   0xFFFFFFFF
#define FXGMAC_PCIE_RECOVER_TIMES 5000
#define FXGMAC_PCIE_IO_MEM_MASTER_ENABLE 0x7
#endif

//#define FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
#define FXGMAC_LOOPBACK_CHECK_INTERVAL     (5 * HZ)
#define FXGMAC_PHY_LOOPBACK_DETECT_THRESOLD 2
#endif

//#define FXGMAC_ASPM_ENABLED
#ifdef FXGMAC_ASPM_ENABLED
#define FXGMAC_CHECK_DEV_STATE
#define FXGMAC_ASPM_INTERVAL                (20 * HZ)
#endif

#ifndef BIT
#define BIT(n) (0x1<<(n))
#endif

#define UDP_RSS_FLAGS (BIT(MAC_RSSCR_UDP4TE_POS) | \
		       BIT(MAC_RSSCR_UDP6TE_POS))

#define MF90_SUB_VENTOR_ID  0x17aa
#define MF90_SUB_DEVICE_ID  0x3509

#pragma pack(1)
/* it's better to make this struct's size to 128byte. */
struct pattern_packet{
    u8                  ether_daddr[ETH_ALEN];
    u8                  ether_saddr[ETH_ALEN];
    u16                 ether_type;

    __be16              ar_hrd;                 /* format of hardware address */
    __be16              ar_pro;                 /* format of protocol  */
    unsigned char       ar_hln;                 /* length of hardware address */
    unsigned char       ar_pln;                 /* length of protocol address */
    __be16              ar_op;                  /* ARP opcode (command)     */
    unsigned char       ar_sha[ETH_ALEN];       /* sender hardware address  */
    unsigned char       ar_sip[4];              /* sender IP address        */
    unsigned char       ar_tha[ETH_ALEN];       /* target hardware address  */
    unsigned char       ar_tip[4];              /* target IP address        */

    u8                  reverse[86];
};
#pragma pack()

typedef dma_addr_t                  DMA_ADDR_T;
typedef enum pkt_hash_types         RSS_HASH_TYPE;
typedef void __iomem*               IOMEM;
typedef struct pci_dev              PCI_DEV;

struct ext_command_buf {
    void* buf;
    u32 size_in;
    u32 size_out;
};

struct ext_command_mac {
    u32  num;
    union {
        u32  val32;
        u16  val16;
        u8   val8;
    };
};

struct ext_command_mii {
    u16  dev;
    u16  num;
    u16  val;
};

struct ext_ioctl_data {
    u32	cmd_type;
    struct ext_command_buf cmd_buf;
};

typedef struct _fxgmac_test_buf {
    u8* addr;
    u32 offset;
    u32 length;
} fxgmac_test_buf, *pfxgmac_test_buf;

typedef struct _fxgmac_test_packet {
    struct _fxgmac_test_packet  * next;
    u32                      length;             /* total length of the packet(buffers) */
    u32                      type;               /* packet type, vlan, ip checksum, TSO, etc. */

    fxgmac_test_buf          buf[MAX_PKT_BUF];
    fxgmac_test_buf          sGList[MAX_PKT_BUF];
    u16                      vlanID;
    u16                      mss;
    u32                      hash;
    u16                      cpuNum;
    u16                      xsum;               /* rx, ip-payload checksum */
    u16                      csumStart;          /* custom checksum offset to the mac-header */
    u16                      csumPos;            /* custom checksom position (to the mac_header) */
    void*                    upLevelReserved[4];
    void*                    lowLevelReserved[4];
} fxgmac_test_packet, *pfxgmac_test_packet;

typedef struct fxgmac_channel_of_platform
{
    char dma_irq_name[IFNAMSIZ + 32];

    u32 dma_irq_tx; //for MSIx to match the type of struct msix_entry.vector
    char dma_irq_name_tx[IFNAMSIZ + 32];

    /* Netdev related settings */
    struct napi_struct napi_tx;

    /* Netdev related settings */
    struct napi_struct napi_rx;
    struct timer_list tx_timer;

#if FXGMAC_TX_HANG_TIMER_ENABLED
    unsigned int tx_hang_timer_active;
    struct timer_list tx_hang_timer;
    unsigned int tx_hang_hw_cur;
#endif
}FXGMAC_CHANNEL_OF_PLATFORM;

typedef struct per_regisiter_info
{
    unsigned int           size;
    unsigned int           address;
    unsigned int           value;
    unsigned char          data[FXGAMC_MAX_DATA_SIZE];
}PER_REG_INFO;

/*
 * for FXGMAC_EFUSE_WRITE_PATCH_PER_INDEX,val0 is index, val1 is offset,
 * val2 is value
 */
typedef struct ext_command_data {
    u32 val0;
    u32 val1;
    u32 val2;
}CMD_DATA;

enum fxgmac_task_flag {
    FXGMAC_FLAG_TASK_DOWN = 0,
    FXGMAC_FLAG_TASK_RESET_PENDING,
    FXGMAC_FLAG_TASK_ESD_CHECK_PENDING,
    FXGMAC_FLAG_TASK_LINKCHG_CHECK_PENDING,
    FXGMAC_FLAG_TASK_MAX
};

typedef struct fxgmac_esd_stats {
    u32 tx_abort_excess_collisions;
    u32 tx_dma_underrun;
    u32 tx_lost_crs;
    u32 tx_late_collisions;
    u32 rx_crc_errors;
    u32 rx_align_errors;
    u32 rx_runt_errors;
    u32 single_collisions;
    u32 multi_collisions;
    u32 tx_deferred_frames;
}FXGMAC_ESD_STATS;

typedef enum fxgmac_dev_state {
    FXGMAC_DEV_OPEN    = 0x0,
    FXGMAC_DEV_CLOSE   = 0x1,
    FXGMAC_DEV_STOP    = 0x2,
    FXGMAC_DEV_START   = 0x3,
    FXGMAC_DEV_SUSPEND = 0x4,
    FXGMAC_DEV_RESUME  = 0x5,
    FXGMAC_DEV_PROBE   = 0xFF,
}DEV_STATE;

typedef struct fxgmac_pdata_of_platform
{
    u32                             cfg_pci_cmd;
    u32                             cfg_cache_line_size;
    u32                             cfg_mem_base;
    u32                             cfg_mem_base_hi;
    u32                             cfg_io_base;
    u32                             cfg_int_line;
    u32                             cfg_device_ctrl1;
    u32                             cfg_pci_link_ctrl;
    u32                             cfg_device_ctrl2;
    u32                             cfg_msix_capability;

    int                             pre_phy_speed;
    int                             pre_phy_duplex;
    int                             pre_phy_autoneg;

    struct work_struct              restart_work;
#ifdef FXGMAC_ESD_CHECK_ENABLED
    struct delayed_work             esd_work;
    FXGMAC_ESD_STATS                esd_stats;
    DECLARE_BITMAP(task_flags, FXGMAC_FLAG_TASK_MAX);
#endif

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
    struct delayed_work             loopback_work;
    u32                             lb_test_flag;  // for tool
    u32                             lb_cable_flag;  // for driver
    u32                             lb_cable_detect_count;  // for driver
#endif

#ifdef FXGMAC_ASPM_ENABLED
    struct delayed_work             aspm_config_work;
    bool                            aspm_en;
    bool                            aspm_work_active;
#endif
    bool                            recover_from_aspm;

    u32                             int_flags; /* legacy, msi or msix */
    int                             misc_irq;
#ifdef CONFIG_PCI_MSI
    struct msix_entry               *msix_entries;
#endif

    /* power management and wol*/
    u32                             wol;
    unsigned long                   powerstate;
    /*for ns-offload table. 2 entries supported. */
    unsigned int                    ns_offload_tab_idx;
    netdev_features_t               netdev_features;
    struct napi_struct              napi;
#ifndef FXGMAC_MISC_NOT_ENABLED
    struct napi_struct              napi_misc;
#endif
    u8                              recover_phy_state;
    char                            misc_irq_name[IFNAMSIZ + 32];
    bool                            phy_link;
    bool                            fxgmac_test_tso_flag;
    u32                             fxgmac_test_tso_seg_num;
    u32                             fxgmac_test_last_tso_len;
    u32                             fxgmac_test_packet_len;
    volatile u32                    fxgmac_test_skb_arr_in_index;
    volatile u32                    fxgmac_test_skb_arr_out_index;
    /* CMD_DATA                        ex_cmd_data; */
    /* PER_REG_INFO                    per_reg_data; */
    struct sk_buff                  *fxgmac_test_skb_array[FXGMAC_MAX_DBG_TEST_PKT];
    DEV_STATE                       dev_state;
    struct mutex                    mutex;
    struct timer_list               phy_poll_tm;
}FXGMAC_PDATA_OF_PLATFORM;

void fxgmac_restart_dev(struct fxgmac_pdata *pdata);
long fxgmac_netdev_ops_ioctl(struct file *file, unsigned int cmd,
                                unsigned long arg);

int  fxgmac_init(struct fxgmac_pdata *pdata, bool save_private_reg);
/* for phy interface */
#if 0
int  fxgmac_ephy_autoneg_ability_get(struct fxgmac_pdata *pdata,
                                        unsigned int *cap_mask);
#endif

int  fxgmac_ephy_status_get(struct fxgmac_pdata *pdata, int* speed,
                                int* duplex, int* ret_link, int *media);
int  fxgmac_ephy_soft_reset(struct fxgmac_pdata *pdata);
int fxgmac_phy_force_mode(struct fxgmac_pdata *pdata);
int fxgmac_phy_force_speed(struct fxgmac_pdata *pdata, int speed);
int fxgmac_phy_force_duplex(struct fxgmac_pdata *pdata, int duplex);
int fxgmac_phy_force_autoneg(struct fxgmac_pdata *pdata, int autoneg);
//void fxgmac_act_phy_link(struct fxgmac_pdata *pdata);
int  fxgmac_phy_timer_init(struct fxgmac_pdata *pdata);
void fxgmac_phy_timer_destroy(struct fxgmac_pdata *pdata);
void fxgmac_phy_update_link(struct net_device *netdev);

unsigned int    fxgmac_get_netdev_ip4addr(struct fxgmac_pdata *pdata);
unsigned char * fxgmac_get_netdev_ip6addr(struct fxgmac_pdata *pdata,
                                            unsigned char *ipval,
                                            unsigned char *ip6addr_solicited,
                                            unsigned int ifa_flag);

#if FXGMAC_PM_FEATURE_ENABLED
void fxgmac_net_powerdown(struct fxgmac_pdata *pdata, unsigned int wol);
void fxgmac_net_powerup(struct fxgmac_pdata *pdata);
#endif

inline unsigned int fxgmac_tx_avail_desc(struct fxgmac_ring *ring);
inline unsigned int fxgmac_rx_dirty_desc(struct fxgmac_ring *ring);
int fxgmac_start(struct fxgmac_pdata *pdata);
void fxgmac_stop(struct fxgmac_pdata *pdata);
void fxgmac_free_rx_data(struct fxgmac_pdata *pdata);
void fxgmac_free_tx_data(struct fxgmac_pdata *pdata);

void fxgmac_tx_start_xmit(struct fxgmac_channel *channel, struct fxgmac_ring *ring);
void fxgmac_dev_xmit(struct fxgmac_channel *channel);
void fxgmac_config_wol(struct fxgmac_pdata *pdata, int en);
void fxgmac_print_pkt(struct net_device *netdev, struct sk_buff *skb, bool tx_rx);

void fxgmac_lock(struct fxgmac_pdata * pdata);
void fxgmac_unlock(struct fxgmac_pdata * pdata);

void fxgmac_set_phy_link_ksettings(struct fxgmac_pdata *pdata);

#ifdef FXGMAC_ASPM_ENABLED
void fxgmac_schedule_aspm_config_work(struct fxgmac_pdata *pdata);
void fxgmac_cancel_aspm_config_work(struct fxgmac_pdata *pdata);
bool fxgmac_aspm_action_linkup(struct fxgmac_pdata *pdata);
#endif

#endif /* __FXGMAC_OS_H__ */

