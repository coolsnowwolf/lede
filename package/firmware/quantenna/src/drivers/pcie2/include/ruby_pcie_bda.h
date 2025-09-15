/*
 * (C) Copyright 2011 Quantenna Communications Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Header file which describes Ruby PCI Express Boot Data Area
 * Has to be used by both kernel and bootloader.
 */

#ifndef RUBY_PCIE_BDA_H
#define RUBY_PCIE_BDA_H

/* Area mapped by via the BAR visible to the host */
#define RUBY_PCIE_BDA_ADDR		CONFIG_ARC_PCIE_BASE
#define RUBY_PCIE_BDA_SIZE		CONFIG_ARC_PCIE_SIZE

#define RUBY_BDA_VADDR			(RUBY_PCIE_BDA_ADDR + 0x80000000)


#define QDPC_PCIE_BDA_VERSION	0x1000

#define QDPC_BDA_PCIE_INIT		0x01
#define QDPC_BDA_PCIE_RDY		0x02
#define QDPC_BDA_FW_LOAD_RDY		0x03
#define QDPC_BDA_FW_LOAD_DONE		0x04
#define QDPC_BDA_FW_START		0x05
#define QDPC_BDA_FW_RUN			0x06
#define QDPC_BDA_FW_HOST_RDY		0x07
#define QDPC_BDA_FW_TARGET_RDY		0x11
#define QDPC_BDA_FW_TARGET_BOOT		0x12
#define QDPC_BDA_FW_FLASH_BOOT		0x13
#define QDPC_BDA_FW_HOST_LOAD		0x08
#define QDPC_BDA_FW_BLOCK_DONE		0x09
#define QDPC_BDA_FW_BLOCK_RDY		0x0A
#define QDPC_BDA_FW_EP_RDY		0x0B
#define QDPC_BDA_FW_BLOCK_END		0x0C
#define QDPC_BDA_FW_CONFIG		0x0D
#define QDPC_BDA_FW_RUNNING		0x0E

#define QDPC_BDA_PCIE_FAIL		0x82
#define QDPC_BDA_FW_LOAD_FAIL		0x85


#define PCIE_BDA_RCMODE                 BIT(1)
#define PCIE_BDA_MSI                    BIT(2)
#define PCIE_BDA_BAR64                  BIT(3)
#define PCIE_BDA_FLASH_PRESENT          BIT(4)  /* Tell the Host if EP have flash contain firmware */
#define PCIE_BDA_FLASH_BOOT             BIT(5)  /* Tell TARGET to boot from flash */
#define PCIE_BDA_XMIT_UBOOT             BIT(6) /* EP ask for u-boot.bin */
#define PCIE_BDA_TARGET_FBOOT_ERR       BIT(8)  /* TARGET flash boot failed */
#define PCIE_BDA_TARGET_FWLOAD_ERR      BIT(9)  /* TARGET firmware load failed */
#define PCIE_BDA_HOST_NOFW_ERR          BIT(12) /* Host not find any firmware */
#define PCIE_BDA_HOST_MEMALLOC_ERR      BIT(13) /* Host malloc firmware download memory block failed */
#define PCIE_BDA_HOST_MEMMAP_ERR        BIT(14) /* Host pci map download memory block failed */
#define PCIE_BDA_VER(x)                 (((x) >> 4) & 0xFF)
#define PCIE_BDA_ERROR_MASK             0xFF00  /* take the second 8 bits as error flag */

#define PCIE_DMA_OFFSET_ERROR		0xFFFF
#define PCIE_DMA_OFFSET_ERROR_MASK	0xFFFF

#define PCIE_BDA_NAMELEN		32

#define QDPC_PCI_ENDIAN_DETECT_DATA	0x12345678
#define QDPC_PCI_ENDIAN_REVERSE_DATA	0x78563412

#define QDPC_PCI_ENDIAN_VALID_STATUS	0x3c3c3c3c
#define QDPC_PCI_ENDIAN_INVALID_STATUS	0

#define QDPC_PCI_LITTLE_ENDIAN		0
#define	QDPC_PCI_BIG_ENDIAN		0xffffffff

#define QDPC_SCHED_TIMEOUT		(HZ / 20)

#define PCIE_DMA_ISSUE_LOG_NUM		128

#define PCIE_RC_TX_QUEUE_LEN		256
#define PCIE_TX_VALID_PKT		0x80000000
#define PCIE_PKT_LEN_MASK		0xffff

struct vmac_pkt_info {
	uint32_t addr;
	uint32_t info;
};

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
	uint8_t		reserved1[56];			/* Reserve 56 bytes to make it compatible with older version */
	uint32_t	bda_flashsz;
	char		bda_boardname[PCIE_BDA_NAMELEN];
	uint32_t	bda_pci_pre_status;		/* PCI endian check previous status */
	uint32_t	bda_pci_endian;			/* Check pci memory endian format */
	uint32_t	bda_pci_post_status;		/* PCI endian check post status */
	int32_t		bda_h2ep_txd_budget;		/* txdone replenish budget for ep */
	int32_t		bda_ep2h_txd_budget;		/* txdone replenish budget for host */
	uint32_t	bda_rc_rx_bd_base;		/* EP rx buffer descriptors base address */
	uint32_t	bda_rc_rx_bd_num;
	uint32_t	bda_rc_tx_bd_base;		/* RC rx buffer descriptors base address */
	uint32_t	bda_rc_tx_bd_num;
	uint8_t		bda_ep_link_state;
	uint8_t		bda_rc_link_state;
	uint8_t		bda_rc_msi_enabled;
	uint8_t		reserved2;
        uint32_t        bda_ep_next_pkt;		/* A pointer to RC's memory specifying next packet to be handled by EP */
	struct vmac_pkt_info request[PCIE_RC_TX_QUEUE_LEN];
} qdpc_pcie_bda_t;

#endif

