/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Driver for Phytium Multimedia Card Interface
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_MCI_H
#define __PHYTIUM_MCI_H

#include <linux/scatterlist.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/interrupt.h>
#include <linux/mmc/host.h>

/*------------------------------------------------------*/
/* Common Definition					*/
/*------------------------------------------------------*/
#define MAX_BD_NUM		128
#define SD_BLOCK_SIZE		512

#define MCI_BUS_1BITS          0x0
#define MCI_BUS_4BITS          0x1
#define MCI_BUS_8BITS          (0x1 << 16)

#define MCI_SD_DRV_VALUE       0
#define MCI_SD_SAMP_VALUE_MAX  0
#define MCI_SD_SAMP_VALUE_MIN  0

#define MCI_TIMEOUT_CMD_VALUE  0xFFFFFFFF
#define MCI_POWER_ON           1
#define MCI_POWER_OFF          0

#define MCI_PREPARE_FLAG		(0x1 << 0)
#define MCI_ASYNC_FLAG			(0x1 << 1)
#define MCI_MMAP_FLAG			(0x1 << 2)

#define MCI_CMD_TIMEOUT	(HZ/10 * 50)	/* 100ms x5 */
#define MCI_DATA_TIMEOUT	(HZ    * 10)	/* 1000ms x5 */

#define MCI_CMD_TYPE_ADTC      0x2

#define MCI_F_MIN		400000
#define MCI_F_MAX		50000000

#define MCI_CLK		1200000000
#define MCI_REALEASE_MEM	0x1
#define MCI_MAX_FIFO_CNT	0x800

/* FIFOTH register defines */
#define MCI_SET_FIFOTH(m, r, t)	(((m) & 0x7) << 28 | \
					 ((r) & 0xFFF) << 16 | ((t) & 0xFFF))
/* Card read threshold */
#define MCI_SET_THLD(v, x)		(((v) & 0xFFF) << 16 | (x))
#define MCI_CARD_WR_THR_EN		BIT(2)
#define MCI_CARD_RD_THR_EN		BIT(0)

/*----------------------------------------------------------------------*/
/* Register Offset							*/
/*----------------------------------------------------------------------*/
#define MCI_CNTRL		0x00 /* the controller config reg */
#define MCI_PWREN		0x04 /* the power enable reg */
#define MCI_CLKDIV		0x08 /* the clock divider reg */
#define MCI_CLKENA		0x10 /* the clock enable reg */
#define MCI_TMOUT		0x14 /* the timeout reg */
#define MCI_CTYPE		0x18 /* the card type reg */
#define MCI_BLKSIZ		0x1C /* the block size reg */
#define MCI_BYTCNT		0x20 /* the byte count reg */
#define MCI_INT_MASK		0x24 /* the interrupt mask reg */
#define MCI_CMDARG		0x28 /* the command argument reg */
#define MCI_CMD		0x2C /* the command reg */
#define MCI_RESP0		0x30 /* the response reg0 */
#define MCI_RESP1		0x34 /* the response reg1 */
#define MCI_RESP2		0x38 /* the response reg2 */
#define MCI_RESP3		0X3C /* the response reg3 */
#define MCI_MASKED_INTS	0x40 /* the masked interrupt status reg */
#define MCI_RAW_INTS		0x44 /* the raw interrupt status reg */
#define MCI_STATUS		0x48 /* the status reg	*/
#define MCI_FIFOTH		0x4C /* the FIFO threshold watermark reg */
#define MCI_CARD_DETECT	0x50 /* the card detect reg */
#define MCI_CARD_WRTPRT	0x54 /* the card write protect reg */
#define MCI_CCLK_RDY		0x58 /* first div is ready? 1:ready,0:not ready*/
#define MCI_TRAN_CARD_CNT	0x5C /* the transferred CIU card byte count reg */
#define MCI_TRAN_FIFO_CNT	0x60 /* the transferred host to FIFO byte count reg  */
#define MCI_DEBNCE		0x64 /* the debounce count reg */
#define MCI_UID		0x68 /* the user ID reg */
#define MCI_VID		0x6C /* the controller version ID reg */
#define MCI_HWCONF		0x70 /* the hardware configuration reg */
#define MCI_UHS_REG		0x74 /* the UHS-I reg */
#define MCI_CARD_RESET		0x78 /* the card reset reg */
#define MCI_BUS_MODE		0x80 /* the bus mode reg */
#define MCI_DESC_LIST_ADDRL	0x88 /* the descriptor list low base address reg */
#define MCI_DESC_LIST_ADDRH	0x8C /* the descriptor list high base address reg */
#define MCI_DMAC_STATUS	0x90 /* the internal DMAC status reg */
#define MCI_DMAC_INT_ENA	0x94 /* the internal DMAC interrupt enable reg */
#define MCI_CUR_DESC_ADDRL	0x98 /* the current host descriptor low address reg */
#define MCI_CUR_DESC_ADDRH	0x9C /* the current host descriptor high address reg */
#define MCI_CUR_BUF_ADDRL	0xA0 /* the current buffer low address reg */
#define MCI_CUR_BUF_ADDRH	0xA4 /* the current buffer high address reg */
#define MCI_CARD_THRCTL	0x100 /* the card threshold control reg */
#define MCI_UHS_REG_EXT	0x108 /* the UHS register extension */
#define MCI_EMMC_DDR_REG	0x10C /* the EMMC DDR reg */
#define MCI_ENABLE_SHIFT	0x110 /* the enable phase shift reg */
#define MCI_DATA		0x200 /* the data FIFO access */

/* Command register defines */
#define MCI_CMD_START			BIT(31)
#define MCI_CMD_USE_HOLD_REG		BIT(29)
#define MCI_CMD_VOLT_SWITCH		BIT(28)
#define MCI_CMD_CCS_EXP		BIT(23)
#define MCI_CMD_CEATA_RD		BIT(22)
#define MCI_CMD_UPD_CLK		BIT(21)
#define MCI_CMD_INIT			BIT(15)
#define MCI_CMD_STOP			BIT(14)
#define MCI_CMD_PRV_DAT_WAIT		BIT(13)
#define MCI_CMD_SEND_STOP		BIT(12)
#define MCI_CMD_STRM_MODE		BIT(11)
#define MCI_CMD_DAT_WR			BIT(10)
#define MCI_CMD_DAT_EXP		BIT(9)
#define MCI_CMD_RESP_CRC		BIT(8)
#define MCI_CMD_RESP_LONG		BIT(7)
#define MCI_CMD_RESP_EXP		BIT(6)
#define MCI_CMD_INDX(n)		((n) & 0x1F)

/*------------------------------------------------------*/
/* Register Mask					*/
/*------------------------------------------------------*/
/* MCI_CNTRL mask */
#define MCI_CNTRL_CONTROLLER_RESET	(0x1 << 0) /* RW */
#define MCI_CNTRL_FIFO_RESET		(0x1 << 1) /* RW */
#define MCI_CNTRL_DMA_RESET			(0x1 << 2) /* RW */
#define MCI_CNTRL_RES				(0x1 << 3) /*  */
#define MCI_CNTRL_INT_ENABLE		(0x1 << 4) /* RW */
#define MCI_CNTRL_DMA_ENABLE		(0x1 << 5) /* RW */
#define MCI_CNTRL_READ_WAIT			(0x1 << 6) /* RW */
#define MCI_CNTRL_SEND_IRQ_RESPONSE	(0x1 << 7) /* RW */
#define MCI_CNTRL_ABORT_READ_DATA	(0x1 << 8) /* RW */
#define MCI_CNTRL_ENDIAN			(0x1 << 11) /* RW */
//#define MCI_CNTRL_CARD_VOLTAGE_A	(0xF << 16) /* RW */
//#define MCI_CNTRL_CARD_VOLTAGE_B	(0xF << 20) /* RW */
#define MCI_CNTRL_ENABLE_OD_PULLUP	(0x1 << 24) /* RW */
#define MCI_CNTRL_USE_INTERNAL_DMAC	(0x1 << 25) /* RW */

/* MCI_PWREN mask */
#define MCI_PWREN_ENABLE		(0x1 << 0)  /* RW */

/* MCI_CLKENA mask */
#define MCI_CLKENA_CCLK_ENABLE		(0x1 << 0) /* RW */
#define MCI_CLKENA_CCLK_LOW_POWER	(0x1 << 16) /* RW */
#define MCI_EXT_CLK_ENABLE		(0x1 << 1)

/* MCI_INT_MASK mask */
#define MCI_INT_MASK_CD		(0x1 << 0) /* RW */
#define MCI_INT_MASK_RE		(0x1 << 1) /* RW */
#define MCI_INT_MASK_CMD		(0x1 << 2) /* RW */
#define MCI_INT_MASK_DTO		(0x1 << 3) /* RW */
#define MCI_INT_MASK_TXDR		(0x1 << 4) /* RW */
#define MCI_INT_MASK_RXDR		(0x1 << 5) /* RW */
#define MCI_INT_MASK_RCRC		(0x1 << 6) /* RW */
#define MCI_INT_MASK_DCRC		(0x1 << 7) /* RW */
#define MCI_INT_MASK_RTO		(0x1 << 8) /* RW */
#define MCI_INT_MASK_DRTO		(0x1 << 9) /* RW */
#define MCI_INT_MASK_HTO		(0x1 << 10) /* RW */
#define MCI_INT_MASK_FRUN		(0x1 << 11) /* RW */
#define MCI_INT_MASK_HLE		(0x1 << 12) /* RW */
#define MCI_INT_MASK_SBE_BCI		(0x1 << 13) /* RW */
#define MCI_INT_MASK_ACD		(0x1 << 14) /* RW */
#define MCI_INT_MASK_EBE		(0x1 << 15) /* RW */
#define MCI_INT_MASK_SDIO		(0x1 << 16) /* RW */

/* MCI_MASKED_INTS mask */
#define MCI_MASKED_INTS_CD		(0x1 << 0) /* RO */
#define MCI_MASKED_INTS_RE		(0x1 << 1) /* RO */
#define MCI_MASKED_INTS_CMD		(0x1 << 2) /* RO */
#define MCI_MASKED_INTS_DTO		(0x1 << 3) /* RO */
#define MCI_MASKED_INTS_TXDR		(0x1 << 4) /* RO */
#define MCI_MASKED_INTS_RXDR		(0x1 << 5) /* RO */
#define MCI_MASKED_INTS_RCRC		(0x1 << 6) /* RO */
#define MCI_MASKED_INTS_DCRC		(0x1 << 7) /* RO */
#define MCI_MASKED_INTS_RTO		(0x1 << 8) /* RO */
#define MCI_MASKED_INTS_DRTO		(0x1 << 9) /* RO */
#define MCI_MASKED_INTS_HTO		(0x1 << 10) /* RO */
#define MCI_MASKED_INTS_FRUN		(0x1 << 11) /* RO */
#define MCI_MASKED_INTS_HLE		(0x1 << 12) /* RO */
#define MCI_MASKED_INTS_SBE_BCI	(0x1 << 13) /* RO */
#define MCI_MASKED_INTS_ACD		(0x1 << 14) /* RO */
#define MCI_MASKED_INTS_EBE		(0x1 << 15) /* RO */
#define MCI_MASKED_INTS_SDIO		(0x1 << 16) /* RO */

/* MCI_RAW_INTS mask */
#define MCI_RAW_INTS_CD		(0x1 << 0) /* W1C */
#define MCI_RAW_INTS_RE		(0x1 << 1) /* W1C */
#define MCI_RAW_INTS_CMD		(0x1 << 2) /* W1C */
#define MCI_RAW_INTS_DTO		(0x1 << 3) /* W1C */
#define MCI_RAW_INTS_TXDR		(0x1 << 4) /* W1C */
#define MCI_RAW_INTS_RXDR		(0x1 << 5) /* W1C */
#define MCI_RAW_INTS_RCRC		(0x1 << 6) /* W1C */
#define MCI_RAW_INTS_DCRC		(0x1 << 7) /* W1C */
#define MCI_RAW_INTS_RTO		(0x1 << 8) /* W1C */
#define MCI_RAW_INTS_DRTO		(0x1 << 9) /* W1C */
#define MCI_RAW_INTS_HTO		(0x1 << 10) /* W1C */
#define MCI_RAW_INTS_FRUN		(0x1 << 11) /* W1C */
#define MCI_RAW_INTS_HLE		(0x1 << 12) /* W1C */
#define MCI_RAW_INTS_SBE_BCI		(0x1 << 13) /* W1C */
#define MCI_RAW_INTS_ACD		(0x1 << 14) /* W1C */
#define MCI_RAW_INTS_EBE		(0x1 << 15) /* W1C */
#define MCI_RAW_INTS_SDIO		(0x1 << 16)  /* W1C */

/* MCI_STATUS mask */
#define MCI_STATUS_FIFO_RX		(0x1 << 0) /* RO */
#define MCI_STATUS_FIFO_TX		(0x1 << 1) /* RO */
#define MCI_STATUS_FIFO_EMPTY		(0x1 << 2) /* RO */
#define MCI_STATUS_FIFO_FULL		(0x1 << 3) /* RO */
#define MCI_STATUS_CARD_STATUS		(0x1 << 8) /* RO */
#define MCI_STATUS_CARD_BUSY		(0x1 << 9) /* RO */
#define MCI_STATUS_DATA_BUSY		(0x1 << 10) /* RO */
#define MCI_STATUS_RESPOSE_INDEX_OFFSET		(11)
#define MCI_STATUS_RESPOSE_INDEX_MASK		(0x3f << MCI_STATUS_RESPOSE_INDEX_OFFSET) /* RO */
#define MCI_STATUS_RESPOSE_INDEX(reg)		(((reg) & MCI_STATUS_RESPOSE_INDEX_MASK) >> MCI_STATUS_RESPOSE_INDEX_OFFSET)
#define MCI_STATUS_DMA_ACK		(0x1 << 31) /* RO */
#define MCI_STATUS_DMA_REQ		(0x1 << 32) /* RO */

/* MCI_UHS_REG mask */
#define MCI_UHS_REG_VOLT		(0x1 << 0) /* RW */
#define MCI_UHS_REG_DDR		(0x1 << 16) /* RW */

/* MCI_CARD_RESET mask */
#define MCI_CARD_RESET_ENABLE		(0x1 << 0) /* RW */

/* MCI_BUS_MODE mask */
#define MCI_BUS_MODE_SWR		(0x1 << 0) /* RW */
#define MCI_BUS_MODE_FB		(0x1 << 1) /* RW */
#define MCI_BUS_MODE_DE		(0x1 << 7) /* RW */

/* MCI_DMAC_STATUS mask */
#define MCI_DMAC_STATUS_TI		(0x1 << 0) /* RW */
#define MCI_DMAC_STATUS_RI		(0x1 << 1) /* RW */
#define MCI_DMAC_STATUS_FBE		(0x1 << 2) /* RW */
#define MCI_DMAC_STATUS_DU		(0x1 << 4) /* RW */
#define MCI_DMAC_STATUS_NIS		(0x1 << 8) /* RW */
#define MCI_DMAC_STATUS_AIS		(0x1 << 9) /* RW */

/* MCI_DMAC_INT_ENA mask */
#define MCI_DMAC_INT_ENA_TI		(0x1 << 0) /* RW */
#define MCI_DMAC_INT_ENA_RI		(0x1 << 1) /* RW */
#define MCI_DMAC_INT_ENA_FBE		(0x1 << 2) /* RW */
#define MCI_DMAC_INT_ENA_DU		(0x1 << 4) /* RW */
#define MCI_DMAC_INT_ENA_CES		(0x1 << 5) /* RW */
#define MCI_DMAC_INT_ENA_NIS		(0x1 << 8) /* RW */
#define MCI_DMAC_INT_ENA_AIS		(0x1 << 9) /* RW */

/* MCI_CARD_THRCTL mask */
#define MCI_CARD_THRCTL_CARDRD		(0x1 << 0) /* RW */
#define MCI_CARD_THRCTL_BUSY_CLR	(0x1 << 1) /* RW */
#define MCI_CARD_THRCTL_CARDWR		(0x1 << 2) /* RW */

/* MCI_UHS_REG_EXT mask */
#define MCI_UHS_REG_EXT_MMC_VOLT	(0x1 << 0) /* RW */
#define MCI_UHS_REG_EXT_CLK_ENA	(0x1 << 1) /* RW */

/* MCI_EMMC_DDR_REG mask */
#define MCI_EMMC_DDR_CYCLE		(0x1 << 0) /* RW */

/*--------------------------------------*/
/*		Structure Type		*/
/*--------------------------------------*/
/* Maximum segments assuming a 512KiB maximum requisition */
/* size and a minimum4KiB page size. */
#define MCI_MAX_SEGS		128
/* ADMA2 64-bit DMA descriptor size */
#define ADMA2_64_DESC_SZ	32

/* mmc request timeout 5000ms */
#define MMC_REQ_TIMEOUT_MS 5000

/* Each descriptor can transfer up to 4KB of data in chained mode */
/*ADMA2 64-bit descriptor.*/
struct phytium_adma2_64_desc {
	u32 attribute;
#define IDMAC_DES0_DIC	BIT(1)
#define IDMAC_DES0_LD	BIT(2)
#define IDMAC_DES0_FD	BIT(3)
#define IDMAC_DES0_CH	BIT(4)
#define IDMAC_DES0_ER	BIT(5)
#define IDMAC_DES0_CES	BIT(30)
#define IDMAC_DES0_OWN	BIT(31)
	u32	NON1;
	u32	len;
	u32	NON2;
	u32	addr_lo; /* Lower 32-bits of Buffer Address Pointer 1*/
	u32	addr_hi; /* Upper 32-bits of Buffer Address Pointer 1*/
	u32	desc_lo; /* Lower 32-bits of Next Descriptor Address */
	u32	desc_hi; /* Upper 32-bits of Next Descriptor Address */
} __packed __aligned(4);

struct phytium_mci_dma {
	struct scatterlist *sg;	/* I/O scatter list */
	/* ADMA descriptor table, pointer to adma_table array */
	struct phytium_adma2_64_desc *adma_table;
	/* Mapped ADMA descr. table, the physical address of adma_table array */
	dma_addr_t adma_addr;
	unsigned int desc_sz;	/* ADMA descriptor size */
};

enum adtc_t {
	COMMOM_ADTC	= 0,
	BLOCK_RW_ADTC	= 1
};

struct phytium_mci_host {
	struct device *dev;
	struct mmc_host *mmc;
	u32 caps;
	u32 caps2;
	spinlock_t lock;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	int error;
	void __iomem *base;		    /* host base address */
	void *adma_table1;
	dma_addr_t adma_addr1;
	struct phytium_mci_dma dma_rx;	/* dma channel */
	struct phytium_mci_dma dma_tx;	/* dma channel */
	struct phytium_mci_dma dma;	/* dma channel */
	u64 dma_mask;
	bool vqmmc_enabled;
	u32 *sg_virt_addr;
	enum adtc_t adtc_type;      /* 0:common adtc cmd; 1:block r/w adtc cmd;*/
	struct timer_list hotplug_timer;
	struct timer_list timeout_timer;
	struct delayed_work req_timeout;
	int irq;		    /* host interrupt */
	u32 current_rca;    /*the current rca value*/
	u32 current_ios_clk;
	u32 is_use_dma;
	u32 is_device_x100;
	struct clk *src_clk;	    /* phytium_mci source clock */
	unsigned long clk_rate;
	unsigned long clk_div;
	unsigned long irq_flags;
	unsigned long flags;
#define MCI_CARD_NEED_INIT	1

};

int phytium_mci_common_probe(struct phytium_mci_host *host);
void phytium_mci_deinit_hw(struct phytium_mci_host *host);
int phytium_mci_runtime_suspend(struct device *dev);
int phytium_mci_runtime_resume(struct device *dev);
int phytium_mci_resume(struct device *dev);
int phytium_mci_suspend(struct device *dev);

#endif /* __PHYTIUM_MCI_HW_H */
