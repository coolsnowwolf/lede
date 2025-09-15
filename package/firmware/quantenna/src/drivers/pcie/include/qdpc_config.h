/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef __QDPC_CONFIG_H__
#define __QDPC_CONFIG_H__

/* Global debug log messsage function mask definition of the moudle pcie */
#define QDRV_LF_TRACE				0x00000001
#define QDRV_LF_WARN				0x00000002
#define QDRV_LF_ERROR				0x00000004
#define QDRV_LF_PKT_RX				0x00000008
#define QDRV_LF_PKT_TX				0x00000010
#define QDRV_LF_ALL				0x0000001F
#define DBG_LM					DBG_LM_QPCIE
#define DBGFMT	"%s-%d: "
#define DBGARG	__func__, __LINE__

#define DBGPRINTF(ll, lf, fmt, ...)					\
	do {								\
		if(printk_ratelimit()) {				\
			printk(DBGFMT fmt, DBGARG, ##__VA_ARGS__);	\
		}							\
	} while(0)


/* Configuration parameters for desc ring */

#define	QDPC_PCIE_AGGRESSIVE_LEVEL		0

#define QDPC_DESC_RING_SIZE			256   /* Max. number of descriptors in a ring */
#define	QDPC_STOP_QUEUE_TIMER_DELAY		(20)

#if (QDPC_PCIE_AGGRESSIVE_LEVEL==1)
#define	QDPC_USE_SKB_RECYCLE			1

#define QDPC_DESC_USRING_SIZE			(QDPC_DESC_RING_SIZE >> 1)
#define QDPC_DESC_DSRING_SIZE			(QDPC_DESC_RING_SIZE >> 1)

#define	QDPC_VETH_TX_LOW_WATERMARK		((QDPC_DESC_RING_SIZE >> 3) + 8)
#define	QDPC_VETH_RX_LOW_WATERMARK		((QDPC_DESC_RING_SIZE >> 1) - (QDPC_VETH_TX_LOW_WATERMARK) + 16)

#else

#define QDPC_DESC_USRING_SIZE			(QDPC_DESC_RING_SIZE >> 1)
#define QDPC_DESC_DSRING_SIZE			(QDPC_DESC_RING_SIZE >> 1)

#define	QDPC_VETH_TX_LOW_WATERMARK		((QDPC_DESC_RING_SIZE >> 2) + 8)
#define	QDPC_VETH_RX_LOW_WATERMARK		(QDPC_DESC_RING_SIZE >> 1)
#endif

#define QDPC_SHARED_MEM_SIZE			(sizeof(qdpc_epbuf_t))

#define QDPC_DMA_ALIGN				8
#define QDPC_MIN_MTU				60
#define QDPC_MAX_MTU				1518
#define QDPC_MIN_FRAMESZ			64
#define QDPC_DMA_MAXBUF				1600
#define QDPC_DMA_MINBUF				64
#define QDPC_MAX_FRAMESZ			QDPC_DMA_MAXBUF
#define QDPC_MAX_JABBER				((QDPC_MAX_FRAMESZ) + 128)

#define QDPC_MAC_ADDR_SIZE			6

#define QDPC_PKT_DATA_OFFSET			40    /*	NET_IP_ALIGN+NET_SKB_PAD	*/
#define QDPC_PKT_DATA_INTF_GAP			64    /*	Inter-Frame Gap			*/
#define QDPC_PKT_HEADROOM			(NET_IP_ALIGN + NET_SKB_PAD)

#define	QDPC_H2EP_INTERRUPT_BIT			BIT(31)
#define	QDPC_H2EP_INTERRUPT_MASK		BIT(31)

#define QDPC_EP_TXDONE				BIT(1)
#define QDPC_EP_RXDONE				BIT(2)
#define QDPC_EP_STOPQ				BIT(3)

#define QDPC_HOST_TXREADY			BIT(1)
#define QDPC_HOST_ENABLEDMA			BIT(2)
#define QDPC_HOST_TXDONE			BIT(3)
#define QDPC_HOST_START_RX			BIT(4)

#define QDPC_DMA_OWN				BIT(31)
#define QDPC_DMA_LAST_DESC			BIT(26)
#define QDPC_DMA_TX_LASTSEG			BIT(30)
#define QDPC_DMA_TX_FIRSTSEG			BIT(29)
#define QDPC_DMA_TX_NOCRC			BIT(28)

#define QDPC_DMA_RX_LASTSEG			BIT(29)
#define QDPC_DMA_RX_FIRSTSEG			BIT(30)
#define QDPC_DMA_MASK_BITS(nbits)		((nbits) ? (BIT(nbits) - 1) : 0)
#define QDPC_DMA_LEN_MASK			(QDPC_DMA_MASK_BITS(12))

#define QDPC_DMA_RXLEN(x)			((x) & QDPC_DMA_LEN_MASK)

#define QDPC_DMA_OWNED(x)			((x) & QDPC_DMA_OWN)

#define QDPC_DMA_SINGLE_TXBUFFER		(QDPC_DMA_TX_LASTSEG | QDPC_DMA_TX_FIRSTSEG)
#define QDPC_DMA_SINGLE_RXBUFFER		(QDPC_DMA_RX_LASTSEG | QDPC_DMA_RX_FIRSTSEG)
#define QDPC_DMA_SINGLE_BUFFER(x)		(((x) & QDPC_DMA_SINGLE_RXBUFFER) == QDPC_DMA_SINGLE_RXBUFFER)

#define QDPC_DMA_TX_DONE			BIT(31)

/*
 * Using Type/Length field for checking if data packet or
 * netlink packet(call_qcsapi remote interface).
 * Using 0x0601 as netlink packet type and MAC magic number(Quantenna OUI)
 * to distinguish netlink packet
 */
#define QDPC_APP_NETLINK_TYPE 0x0601
#define QDPC_NETLINK_DST_MAGIC "\x00\x26\x86\x00\x00\x00"
#define QDPC_NETLINK_SRC_MAGIC "\x00\x26\x86\x00\x00\x00"
typedef struct qdpc_cmd_hdr {
	uint8_t dst_magic[ETH_ALEN];
	uint8_t src_magic[ETH_ALEN];
	__be16 type;
	__be16 len;
}qdpc_cmd_hdr_t;

/* State of the driver. This is used to syncronize with target during init. */
enum qdpc_drv_state
{
	QDPC_NO_INIT,   /* Uninitialized */
	QDPC_INIT,      /* Init */
	QDPC_CONN_EST,  /* In sync state */
	QDPC_EXIT
};

struct qdpc_init_info {
	uint32_t	handshake;
	uint32_t	remote_mem;
	uint32_t	remote_memsz;
	uint8_t		mac_addr[QDPC_MAC_ADDR_SIZE]; /* MAC of Ruby board */
};

typedef struct qdpc_init_info qdpc_init_info_t;

/*
  * Use this as the start to specify the structure ofthe shared memory area with the EP.
  *
  * The area is mapped as both inbound and outbound
  * On the EP is starts at 0xC200 0000 as before
  *
  *
  * The EMAC0 DMA will read from the descriptor block below across the PCIe bus
  * The 4Mb shared memory buffer on the EP is no longer necessary 
  *
  * Ignore the work OC has done
  *
  * For RX: The RX DMA descriptors map to the top of the buffer as shown
  * and continues to use the OC buffer scheme for now. 
  * Once the TX buffer descriptor work is completed, this will be removed and
  * a similar DMA buffer scheme like the TX one will be used as its replacement
  * For TX: On X86 memcpy the skb data to the TX data area and update the descriptor 
  */


/* Hardware DMA descriptor
  * This is the hardware format used by the Arasan EMAC
  */
typedef struct qdpc_dmadesc {
	uint32_t dma_status;
	uint32_t dma_control;
	uint32_t dma_data;
	uint32_t dma_ptr;
}    __attribute__ ((packed)) qdpc_dmadesc_t;

/* DMA data area */
typedef struct qdpc_dmadata {
	uint8_t	dma_data[QDPC_DMA_MAXBUF];
} __attribute__ ((packed))  qdpc_dmadata_t;


#include "queue.h"
/*
 * TX software descriptor
 * Must be aligned on a 32bit boundary
 */
struct qdpc_desc {
	/* hardware DMA descriptor */
	qdpc_dmadesc_t	*dd_hwdesc;

	/* Metadata pointer used for things like the skb associated with the buffer */
	void			*dd_metadata;
	void			*dd_vaddr;
	uint32_t		dd_paddr;
	size_t			dd_size;

	uint32_t		dd_flags;
	uint64_t		dd_qtime;

	/* Linked list entry */
	STAILQ_ENTRY(qdpc_desc)		dd_entry;
} __attribute__ ((aligned (8)));

typedef struct qdpc_desc qdpc_desc_t;

/* Packet DMA ring */
typedef struct qdpc_pdring {
	/* Size of ring */
	uint32_t		pd_ringsize;

	/*
	  * Number of free descriptors
	  */
	uint32_t		pd_nfree;
	uint32_t		pd_npending;
	uint32_t		pd_tx_basereg;
	uint32_t		pd_rx_basereg;
	uint32_t		pd_highwatermark;
	uint32_t		pd_lowatermark;

	/* Location of the next free descriptor
	 * Initialize to the first descriptor in the ring.
	 * It must increase in one direction only and wrap around when the end of the ring is reached.
	 * Increase by 1 each time a pkt is queued
	 */

	qdpc_desc_t		*pd_nextdesc;
	qdpc_desc_t		*pd_lastdesc;
	qdpc_desc_t		*pd_firstdesc;


	/* Start of HW descriptor block */
	qdpc_dmadesc_t	*pd_hwdesc;

	/* Bus Address of HW descriptor block */
	uint32_t		pd_src_busaddr;
	uint32_t		pd_dst_busaddr;
	void *			pd_handle;

	/* txq spinlock */
	spinlock_t		pd_lock;		  /* Packet enqueue/dequeue lock */

	/* Status flags */
	uint32_t		pd_f_dma_active:1;	/* DMA fetch in progress */

	/* Pending list of  buffers owned by DMA hardware */
	STAILQ_HEAD(qdpc_pdpend, qdpc_desc) pd_pending;

	/* Descriptor pool */
	qdpc_desc_t		pd_desc[QDPC_DESC_RING_SIZE];
	uint8_t			*src_macaddr;

}   qdpc_pdring_t;


typedef struct qdpc_pktring {
	qdpc_pdring_t	pkt_usq;
	qdpc_pdring_t	pkt_dsq;
} qdpc_pktring_t;

#define QDPC_PCIE_BDA_VERSION	0x1000

#define QDPC_BDA_PCIE_INIT              0x01
#define QDPC_BDA_PCIE_RDY               0x02
#define QDPC_BDA_FW_LOAD_RDY    0x03
#define QDPC_BDA_FW_LOAD_DONE   0x04
#define QDPC_BDA_FW_START               0x05
#define QDPC_BDA_FW_RUN                 0x06
#define QDPC_BDA_FW_HOST_RDY    0x07
#define QDPC_BDA_FW_TARGET_RDY  0x11
#define QDPC_BDA_FW_TARGET_BOOT 0x12
#define QDPC_BDA_FW_FLASH_BOOT  0x13
#define QDPC_BDA_FW_HOST_LOAD   0x08
#define QDPC_BDA_FW_BLOCK_DONE  0x09
#define QDPC_BDA_FW_BLOCK_RDY   0x0A
#define QDPC_BDA_FW_EP_RDY              0x0B
#define QDPC_BDA_FW_BLOCK_END   0x0C
#define QDPC_BDA_FW_CONFIG              0x0D
#define QDPC_BDA_FW_RUNNING             0x0E


#define QDPC_BDA_PCIE_FAIL	0x82
#define QDPC_BDA_FW_LOAD_FAIL	0x85


#define PCIE_BDA_RCMODE                 BIT(1)
#define PCIE_BDA_MSI                    BIT(2)
#define PCIE_BDA_BAR64                  BIT(3)
#define PCIE_BDA_FLASH_PRESENT          BIT(4)  /* Tell the Host if EP have flash contain firmware */
#define PCIE_BDA_FLASH_BOOT             BIT(5)  /* Tell TARGET to boot from flash */
#define PCIE_BDA_TARGET_FBOOT_ERR       BIT(8)  /* TARGET flash boot failed */
#define PCIE_BDA_TARGET_FWLOAD_ERR      BIT(9)  /* TARGET firmware load failed */
#define PCIE_BDA_HOST_NOFW_ERR          BIT(12) /* Host not find any firmware */
#define PCIE_BDA_HOST_MEMALLOC_ERR      BIT(13) /* Host malloc firmware download memory block failed */
#define PCIE_BDA_HOST_MEMMAP_ERR        BIT(14) /* Host pci map download memory block failed */
#define PCIE_BDA_VER(x)                 (((x) >> 4) & 0xFF)
#define PCIE_BDA_ERROR_MASK             0xFF00  /* take the second 8 bits as error flag */


#define PCIE_BDA_NAMELEN		32

#define QDPC_PCI_ENDIAN_DETECT_DATA	0x12345678
#define QDPC_PCI_ENDIAN_REVERSE_DATA	0x78563412

#define QDPC_PCI_ENDIAN_VALID_STATUS	0x3c3c3c3c
#define QDPC_PCI_ENDIAN_INVALID_STATUS	0

#define QDPC_PCI_LITTLE_ENDIAN		0
#define	QDPC_PCI_BIG_ENDIAN		0xffffffff

#define QDPC_SCHED_TIMEOUT		(HZ / 20)


typedef struct qdpc_pcie_board_cfg {
	int bc_board_id;
	char *bc_name;		/* optional name of cfg */
	int bc_ddr_type;	/* ID */
	int bc_ddr_speed;	/* speed in MHz */
	int bc_ddr_size;	/* in bytes */
	int bc_emac0;		/* in use? */
	int bc_emac1;		/* in use? */
	int bc_phy0_addr;	/* address */
	int bc_phy1_addr;	/* address */
	int bc_spi1;		/* in use? */
	int bc_wifi_hw;		/* WiFi hardware type */
	int bc_uart1;		/* in use? */
	int bc_pcie;		/* in use? */
	int bc_rgmii_timing;	/* special timing value for RGMII */
}  __attribute__ ((packed)) qdpc_pcie_board_cfg_t;

/* There is a copy named ruby_pcie_bda_t in ruby_pcie_bda.h they must be the same */
typedef struct qdpc_pcie_bda {
	uint16_t	bda_len;			/* Size of BDA block */
	uint16_t	bda_version;			/* BDA version */
	uint32_t	bda_bootstate;			/* Boot state of device */
	uint32_t	bda_dma_mask;			/* Number of addressable DMA bits */
	uint32_t	bda_dma_offset;			/* HW specific offset for DMA engine */
	uint32_t	bda_flags;
	uint32_t	bda_img;			/* Current load image block */
	uint32_t	bda_img_size;			/* Current load image block size */
	uint32_t	bda_ep2h_irqstatus;		/* Added here to allow boot loader to use irqs if desired */
	uint32_t	bda_h2ep_irqstatus;		/* Added here to allow boot loader to use irqs if desired */
	uint32_t	bda_msi_addr;
	qdpc_pcie_board_cfg_t	bda_boardcfg;
	uint32_t	bda_flashsz;
	char		bda_boardname[PCIE_BDA_NAMELEN];
	/* Warning: these two fields are re-used at run time to store the host MAC address. */
	uint32_t	bda_pci_pre_status;		/* PCI endian check previous status */
	uint32_t	bda_pci_endian;			/* Check pci memory endian format */
	uint32_t	bda_pci_post_status;		/* PCI endian check post status */
	int32_t		bda_h2ep_txd_budget;		/* txdone replenish budget for ep */
	int32_t		bda_ep2h_txd_budget;		/* txdone replenish budget for host */
}  __attribute__ ((packed)) qdpc_pcie_bda_t;

typedef struct qdpc_epshmem_hdr {
	uint32_t		eps_mapsize;		/* Iomap size for PCIe hardware */
	uint32_t		eps_ver;			/* Header version */
	uint32_t		eps_size;			/* Size of shared memory area */
	uint32_t		eps_dma_offset;		/* HW dependent DMA offset */
	uint32_t		eps_dsdma_desc;		/* Downstream Descriptor offset */
	uint32_t		eps_usdma_desc;		/* Upstream Descriptor offset */
	uint32_t		eps_dsdma_ndesc;	/* Number of downstream descriptros */
	uint32_t		eps_usdma_ndesc;	/* Number of upstream descriptros */
	uint32_t		eps_align;
	uint32_t		eps_maxbuf;
	uint32_t		eps_minbuf;
} __attribute__ ((packed)) qdpc_epshmem_hdr_t;

#endif /* __QDPC_CONFIG_H__ */
