/*
 * sata_oxnas
 *      A driver to interface the 934 based sata core present in the ox820
 *      with libata and scsi
 * based on sata_oxnas driver by Ma Haijun <mahaijuns@gmail.com>
 * based on ox820 sata code by:
 *  Copyright (c) 2007 Oxford Semiconductor Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/ata.h>
#include <linux/libata.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/reset.h>

#include <linux/io.h>
#include <linux/sizes.h>

static inline void oxnas_register_clear_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val &= ~mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_set_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val |= mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_value_mask(void __iomem *p,
					     unsigned mask, unsigned new_value)
{
	/* TODO sanity check mask & new_value = new_value */
	u32 val = readl_relaxed(p);

	val &= ~mask;
	val |= new_value;
	writel_relaxed(val, p);
}

/* sgdma request structure */
struct sgdma_request {
	volatile u32 qualifier;
	volatile u32 control;
	dma_addr_t src_pa;
	dma_addr_t dst_pa;
} __packed __aligned(4);


/* Controller information */
enum {
	SATA_OXNAS_MAX_PRD = 63,
	SATA_OXNAS_DMA_SIZE = SATA_OXNAS_MAX_PRD *
				sizeof(struct ata_bmdma_prd) +
				sizeof(struct sgdma_request),
	SATA_OXNAS_MAX_PORTS	= 2,
	/** The different Oxsemi SATA core version numbers */
	SATA_OXNAS_CORE_VERSION = 0x1f3,
	SATA_OXNAS_IRQ_FLAG	= IRQF_SHARED,
	SATA_OXNAS_HOST_FLAGS	= (ATA_FLAG_SATA | ATA_FLAG_PIO_DMA |
			ATA_FLAG_NO_ATAPI /*| ATA_FLAG_NCQ*/),
	SATA_OXNAS_QUEUE_DEPTH	= 32,

	SATA_OXNAS_DMA_BOUNDARY = 0xFFFFFFFF,
};


/*
 * SATA Port Registers
 */
enum {
	/** sata host port register offsets */
	ORB1 = 0x00,
	ORB2 = 0x04,
	ORB3 = 0x08,
	ORB4 = 0x0C,
	ORB5 = 0x10,
	MASTER_STATUS = 0x10,
	FIS_CTRL = 0x18,
	FIS_DATA = 0x1C,
	INT_STATUS = 0x30,
	INT_CLEAR = 0x30,
	INT_ENABLE = 0x34,
	INT_DISABLE = 0x38,
	VERSION = 0x3C,
	SATA_CONTROL = 0x5C,
	SATA_COMMAND = 0x60,
	HID_FEATURES = 0x64,
	PORT_CONTROL = 0x68,
	DRIVE_CONTROL = 0x6C,
	/** These registers allow access to the link layer registers
	that reside in a different clock domain to the processor bus */
	LINK_DATA = 0x70,
	LINK_RD_ADDR = 0x74,
	LINK_WR_ADDR = 0x78,
	LINK_CONTROL = 0x7C,
	/* window control */
	WIN1LO = 0x80,
	WIN1HI = 0x84,
	WIN2LO = 0x88,
	WIN2HI = 0x8C,
	WIN0_CONTROL = 0x90,
};

/** sata port register bits */
enum{
	/**
	 * commands to issue in the master status to tell it to move shadow ,
	 * registers to the actual device ,
	 */
	SATA_OPCODE_MASK = 0x00000007,
	CMD_WRITE_TO_ORB_REGS_NO_COMMAND = 0x4,
	CMD_WRITE_TO_ORB_REGS = 0x2,
	CMD_SYNC_ESCAPE = 0x7,
	CMD_CORE_BUSY = (1 << 7),
	CMD_DRIVE_SELECT_SHIFT = 12,
	CMD_DRIVE_SELECT_MASK = (0xf << CMD_DRIVE_SELECT_SHIFT),

	/** interrupt bits */
	INT_END_OF_CMD = 1 << 0,
	INT_LINK_SERROR = 1 << 1,
	INT_ERROR = 1 << 2,
	INT_LINK_IRQ = 1 << 3,
	INT_REG_ACCESS_ERR = 1 << 7,
	INT_BIST_FIS = 1 << 11,
	INT_MASKABLE =	INT_END_OF_CMD |
			INT_LINK_SERROR |
			INT_ERROR |
			INT_LINK_IRQ |
			INT_REG_ACCESS_ERR |
			INT_BIST_FIS,
	INT_WANT =	INT_END_OF_CMD |
			INT_LINK_SERROR |
			INT_REG_ACCESS_ERR |
			INT_ERROR,
	INT_ERRORS =	INT_LINK_SERROR |
			INT_REG_ACCESS_ERR |
			INT_ERROR,

	/** raw interrupt bits, unmaskable, but do not generate interrupts */
	RAW_END_OF_CMD  = INT_END_OF_CMD << 16,
	RAW_LINK_SERROR = INT_LINK_SERROR  << 16,
	RAW_ERROR  = INT_ERROR << 16,
	RAW_LINK_IRQ  = INT_LINK_IRQ << 16,
	RAW_REG_ACCESS_ERR = INT_REG_ACCESS_ERR << 16,
	RAW_BIST_FIS  = INT_BIST_FIS << 16,
	RAW_WANT  = INT_WANT << 16,
	RAW_ERRORS  = INT_ERRORS << 16,

	/**
	 * variables to write to the device control register to set the current
	 * device, ie. master or slave.
	 */
	DR_CON_48 = 2,
	DR_CON_28 = 0,

	SATA_CTL_ERR_MASK = 0x00000016,

};

/* ATA SGDMA register offsets */
enum {
	SGDMA_CONTROL = 0x0,
	SGDMA_STATUS = 0x4,
	SGDMA_REQUESTPTR = 0x8,
	SGDMA_RESETS = 0xC,
	SGDMA_CORESIZE = 0x10,
};

/* DMA controller register offsets */
enum {
	DMA_CONTROL = 0x0,
	DMA_CORESIZE = 0x20,

	DMA_CONTROL_RESET = (1 << 12),
};

enum {
	/* see DMA core docs for the values. Out means from memory (bus A) out
	 * to disk (bus B) */
	SGDMA_REQCTL0OUT = 0x0497c03d,
	/* burst mode disabled when no micro code used */
	SGDMA_REQCTL0IN = 0x0493a3c1,
	SGDMA_REQCTL1OUT = 0x0497c07d,
	SGDMA_REQCTL1IN = 0x0497a3c5,
	SGDMA_CONTROL_NOGO = 0x3e,
	SGDMA_CONTROL_GO = SGDMA_CONTROL_NOGO | 1,
	SGDMA_ERRORMASK = 0x3f,
	SGDMA_BUSY = 0x80,

	SGDMA_RESETS_CTRL = 1 << 0,
	SGDMA_RESETS_ARBT = 1 << 1,
	SGDMA_RESETS_AHB = 1 << 2,
	SGDMA_RESETS_ALL =	SGDMA_RESETS_CTRL |
				SGDMA_RESETS_ARBT |
				SGDMA_RESETS_AHB,

	/* Final EOTs */
	SGDMA_REQQUAL = 0x00220001,

};

/** SATA core register offsets */
enum {
	DM_DBG1 = 0x000,
	RAID_SET = 0x004,
	DM_DBG2 = 0x008,
	DATACOUNT_PORT0 = 0x010,
	DATACOUNT_PORT1 = 0x014,
	CORE_INT_STATUS = 0x030,
	CORE_INT_CLEAR = 0x030,
	CORE_INT_ENABLE = 0x034,
	CORE_INT_DISABLE  = 0x038,
	CORE_REBUILD_ENABLE = 0x050,
	CORE_FAILED_PORT_R = 0x054,
	DEVICE_CONTROL = 0x068,
	EXCESS = 0x06C,
	RAID_SIZE_LOW = 0x070,
	RAID_SIZE_HIGH = 0x074,
	PORT_ERROR_MASK = 0x078,
	IDLE_STATUS = 0x07C,
	RAID_CONTROL = 0x090,
	DATA_PLANE_CTRL = 0x0AC,
	CORE_DATAPLANE_STAT = 0x0b8,
	PROC_PC = 0x100,
	CONFIG_IN = 0x3d8,
	PROC_START = 0x3f0,
	PROC_RESET = 0x3f4,
	UCODE_STORE = 0x1000,
	RAID_WP_BOT_LOW = 0x1FF0,
	RAID_WP_BOT_HIGH  = 0x1FF4,
	RAID_WP_TOP_LOW = 0x1FF8,
	RAID_WP_TOP_HIGH = 0x1FFC,
	DATA_MUX_RAM0 = 0x8000,
	DATA_MUX_RAM1 = 0xA000,
	PORT_SIZE = 0x10000,
};

enum {
	/* Sata core debug1 register bits */
	CORE_PORT0_DATA_DIR_BIT = 20,
	CORE_PORT1_DATA_DIR_BIT = 21,
	CORE_PORT0_DATA_DIR = 1 << CORE_PORT0_DATA_DIR_BIT,
	CORE_PORT1_DATA_DIR = 1 << CORE_PORT1_DATA_DIR_BIT,

	/** sata core control register bits */
	SCTL_CLR_ERR = 0x00003016,
	RAID_CLR_ERR = 0x0000011e,

	/* Interrupts direct from the ports */
	NORMAL_INTS_WANTED = 0x00000303,

	/* shift these left by port number */
	COREINT_HOST = 0x00000001,
	COREINT_END = 0x00000100,
	CORERAW_HOST = COREINT_HOST << 16,
	CORERAW_END = COREINT_END  << 16,

	/* Interrupts from the RAID controller only */
	RAID_INTS_WANTED = 0x00008300,

	/* The bits in the IDLE_STATUS that, when set indicate an idle core */
	IDLE_CORES = (1 << 18) | (1 << 19),

	/* Data plane control error-mask mask and bit, these bit in the data
	 * plane control mask out errors from the ports that prevent the SGDMA
	 * care from sending an interrupt */
	DPC_ERROR_MASK = 0x00000300,
	DPC_ERROR_MASK_BIT = 0x00000100,
	/* enable jbod micro-code */
	DPC_JBOD_UCODE = 1 << 0,
	DPC_FIS_SWCH = 1 << 1,

	/** Device Control register bits */
	DEVICE_CONTROL_DMABT = 1 << 4,
	DEVICE_CONTROL_ABORT = 1 << 2,
	DEVICE_CONTROL_PAD = 1 << 3,
	DEVICE_CONTROL_PADPAT = 1 << 16,
	DEVICE_CONTROL_PRTRST = 1 << 8,
	DEVICE_CONTROL_RAMRST = 1 << 12,
	DEVICE_CONTROL_ATA_ERR_OVERRIDE = 1 << 28,

	/** oxsemi HW raid modes */
	OXNASSATA_NOTRAID = 0,
	OXNASSATA_RAID0 = 1,
	OXNASSATA_RAID1 = 2,
	/** OX820 specific HW-RAID register values */
	RAID_TWODISKS = 3,
	UNKNOWN_MODE = ~0,

	CONFIG_IN_RESUME = 2,
};

/* SATA PHY Registers */
enum {
	PHY_STAT = 0x00,
	PHY_DATA = 0x04,
};

enum {
	STAT_READ_VALID = (1 << 21),
	STAT_CR_ACK = (1 << 20),
	STAT_CR_READ = (1 << 19),
	STAT_CR_WRITE = (1 << 18),
	STAT_CAP_DATA = (1 << 17),
	STAT_CAP_ADDR = (1 << 16),

	STAT_ACK_ANY =	STAT_CR_ACK |
			STAT_CR_READ |
			STAT_CR_WRITE |
			STAT_CAP_DATA |
			STAT_CAP_ADDR,

	CR_READ_ENABLE = (1 << 16),
	CR_WRITE_ENABLE = (1 << 17),
	CR_CAP_DATA = (1 << 18),
};

enum {
	/* Link layer registers */
	SERROR_IRQ_MASK = 5,
};

enum {
	OXNAS_SATA_SOFTRESET = 1,
	OXNAS_SATA_REINIT = 2,
};

enum {
	OXNAS_SATA_UCODE_RAID0,
	OXNAS_SATA_UCODE_RAID1,
	OXNAS_SATA_UCODE_JBOD,
	OXNAS_SATA_UCODE_NONE,
};

enum {
	SATA_UNLOCKED,
	SATA_WRITER,
	SATA_READER,
	SATA_REBUILD,
	SATA_HWRAID,
	SATA_SCSI_STACK
};

typedef irqreturn_t (*oxnas_sata_isr_callback_t)(int, unsigned long, int);

struct sata_oxnas_host_priv {
	void __iomem *port_base;
	void __iomem *dmactl_base;
	void __iomem *sgdma_base;
	void __iomem *core_base;
	void __iomem *phy_base;
	dma_addr_t dma_base;
	void __iomem *dma_base_va;
	size_t dma_size;
	int irq;
	int n_ports;
	int current_ucode;
	u32 port_frozen;
	u32 port_in_eh;
	struct clk *clk;
	struct reset_control *rst_sata;
	struct reset_control *rst_link;
	struct reset_control *rst_phy;
	spinlock_t phy_lock;
	spinlock_t core_lock;
	int core_locked;
	int reentrant_port_no;
	int hw_lock_count;
	int direct_lock_count;
	void *locker_uid;
	int current_locker_type;
	int scsi_nonblocking_attempts;
	oxnas_sata_isr_callback_t isr_callback;
	void *isr_arg;
	wait_queue_head_t fast_wait_queue;
	wait_queue_head_t scsi_wait_queue;
};


struct sata_oxnas_port_priv {
	void __iomem *port_base;
	void __iomem *dmactl_base;
	void __iomem *sgdma_base;
	void __iomem *core_base;
	struct sgdma_request *sgdma_request;
	dma_addr_t sgdma_request_pa;
};

static u8 sata_oxnas_check_status(struct ata_port *ap);
static int sata_oxnas_cleanup(struct ata_host *ah);
static void sata_oxnas_tf_load(struct ata_port *ap,
				const struct ata_taskfile *tf);
static void sata_oxnas_irq_on(struct ata_port *ap);
static void sata_oxnas_post_reset_init(struct ata_port *ap);

static int sata_oxnas_acquire_hw(struct ata_port *ap, int may_sleep,
				 int timeout_jiffies);
static void sata_oxnas_release_hw(struct ata_port *ap);

static const void *HW_LOCKER_UID = (void *)0xdeadbeef;

/***************************************************************************
* ASIC access
***************************************************************************/
static void wait_cr_ack(void __iomem *phy_base)
{
	while ((ioread32(phy_base + PHY_STAT) >> 16) & 0x1f)
		; /* wait for an ack bit to be set */
}

static u16 read_cr(void __iomem *phy_base, u16 address)
{
	iowrite32((u32)address, phy_base + PHY_STAT);
	wait_cr_ack(phy_base);
	iowrite32(CR_READ_ENABLE, phy_base + PHY_DATA);
	wait_cr_ack(phy_base);
	return (u16)ioread32(phy_base + PHY_STAT);
}

static void write_cr(void __iomem *phy_base, u16 data, u16 address)
{
	iowrite32((u32)address, phy_base + PHY_STAT);
	wait_cr_ack(phy_base);
	iowrite32((data | CR_CAP_DATA), phy_base + PHY_DATA);
	wait_cr_ack(phy_base);
	iowrite32(CR_WRITE_ENABLE, phy_base + PHY_DATA);
	wait_cr_ack(phy_base);
}

#define PH_GAIN		 2
#define FR_GAIN		 3
#define PH_GAIN_OFFSET  6
#define FR_GAIN_OFFSET  8
#define PH_GAIN_MASK  (0x3 << PH_GAIN_OFFSET)
#define FR_GAIN_MASK  (0x3 << FR_GAIN_OFFSET)
#define USE_INT_SETTING  (1<<5)

void workaround5458(struct ata_host *ah)
{
	struct sata_oxnas_host_priv *hd = ah->private_data;
	void __iomem *phy_base = hd->phy_base;
	u16 rx_control;
	unsigned i;

	for (i = 0; i < 2; i++) {
		rx_control = read_cr(phy_base, 0x201d + (i << 8));
		rx_control &= ~(PH_GAIN_MASK | FR_GAIN_MASK);
		rx_control |= PH_GAIN << PH_GAIN_OFFSET;
		rx_control |= (FR_GAIN << FR_GAIN_OFFSET) | USE_INT_SETTING;
		write_cr(phy_base, rx_control, 0x201d+(i<<8));
	}
}

/**
 * allows access to the link layer registers
 * @param link_reg the link layer register to access (oxsemi indexing ie
 *		00 = static config, 04 = phy ctrl)
 */
void sata_oxnas_link_write(struct ata_port *ap, unsigned int link_reg, u32 val)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	struct sata_oxnas_host_priv *hd = ap->host->private_data;
	void __iomem *port_base = pd->port_base;
	u32 patience;
	unsigned long flags;

	DPRINTK("P%d [0x%02x]->0x%08x\n", ap->port_no, link_reg, val);

	spin_lock_irqsave(&hd->phy_lock, flags);
	iowrite32(val, port_base + LINK_DATA);

	/* accessed twice as a work around for a bug in the SATA abp bridge
	 * hardware (bug 6828) */
	iowrite32(link_reg , port_base + LINK_WR_ADDR);
	ioread32(port_base + LINK_WR_ADDR);

	for (patience = 0x100000; patience > 0; --patience) {
		if (ioread32(port_base + LINK_CONTROL) & 0x00000001)
			break;
	}
	spin_unlock_irqrestore(&hd->phy_lock, flags);
}

static int sata_oxnas_scr_write_port(struct ata_port *ap, unsigned int sc_reg,
					u32 val)
{
	sata_oxnas_link_write(ap, 0x20 + (sc_reg * 4), val);
	return 0;
}

static int sata_oxnas_scr_write(struct ata_link *link, unsigned int sc_reg,
				u32 val)
{
	return sata_oxnas_scr_write_port(link->ap, sc_reg, val);
}

u32 sata_oxnas_link_read(struct ata_port *ap, unsigned int link_reg)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	struct sata_oxnas_host_priv *hd = ap->host->private_data;
	void __iomem *port_base = pd->port_base;
	u32 result;
	u32 patience;
	unsigned long flags;

	spin_lock_irqsave(&hd->phy_lock, flags);
	/* accessed twice as a work around for a bug in the SATA abp bridge
	 * hardware (bug 6828) */
	iowrite32(link_reg, port_base + LINK_RD_ADDR);
	ioread32(port_base + LINK_RD_ADDR);

	for (patience = 0x100000; patience > 0; --patience) {
		if (ioread32(port_base + LINK_CONTROL) & 0x00000001)
			break;
	}
	if (patience == 0)
		DPRINTK("link read timed out for port %d\n", ap->port_no);

	result = ioread32(port_base + LINK_DATA);
	spin_unlock_irqrestore(&hd->phy_lock, flags);

	return result;
}

static int sata_oxnas_scr_read_port(struct ata_port *ap, unsigned int sc_reg,
					u32 *val)
{
	*val = sata_oxnas_link_read(ap, 0x20 + (sc_reg*4));
	return 0;
}

static int sata_oxnas_scr_read(struct ata_link *link,
			     unsigned int sc_reg, u32 *val)
{
	return sata_oxnas_scr_read_port(link->ap, sc_reg, val);
}

/**
 * sata_oxnas_irq_clear is called during probe just before the interrupt handler is
 * registered, to be sure hardware is quiet. It clears and masks interrupt bits
 * in the SATA core.
 *
 * @param ap hardware with the registers in
 */
static void sata_oxnas_irq_clear(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *port_priv = ap->private_data;

	/* clear pending interrupts */
	iowrite32(~0, port_priv->port_base + INT_CLEAR);
	iowrite32(COREINT_END, port_priv->core_base + CORE_INT_CLEAR);
}

/**
 * qc_issue is used to make a command active, once the hardware and S/G tables
 * have been prepared. IDE BMDMA drivers use the helper function
 * ata_qc_issue_prot() for taskfile protocol-based dispatch. More advanced
 * drivers roll their own ->qc_issue implementation, using this as the
 * "issue new ATA command to hardware" hook.
 * @param qc the queued command to issue
 */
static unsigned int sata_oxnas_qc_issue(struct ata_queued_cmd *qc)
{
	struct sata_oxnas_port_priv *pd = qc->ap->private_data;
	struct sata_oxnas_host_priv *hd = qc->ap->host->private_data;

	void __iomem *port_base = pd->port_base;
	void __iomem *core_base = pd->core_base;
	int port_no = qc->ap->port_no;
	int no_microcode = (hd->current_ucode == UNKNOWN_MODE);
	u32 reg;

	/* check the core is idle */
	if (ioread32(port_base + SATA_COMMAND) & CMD_CORE_BUSY) {
		int count = 0;

		DPRINTK("core busy for a command on port %d\n",
			qc->ap->port_no);
		do {
			mdelay(1);
			if (++count > 100) {
				DPRINTK("core busy for a command on port %d\n",
					qc->ap->port_no);
				/* CrazyDumpDebug(); */
				sata_oxnas_cleanup(qc->ap->host);
			}
		} while (ioread32(port_base + SATA_COMMAND) & CMD_CORE_BUSY);
	}

	/* enable passing of error signals to DMA sub-core by clearing the
	 * appropriate bit */
	reg = ioread32(core_base + DATA_PLANE_CTRL);
	if (no_microcode)
		reg |= (DPC_ERROR_MASK_BIT | (DPC_ERROR_MASK_BIT << 1));
	reg &= ~(DPC_ERROR_MASK_BIT << port_no);
	iowrite32(reg, core_base + DATA_PLANE_CTRL);

	/* Disable all interrupts for ports and RAID controller */
	iowrite32(~0, port_base + INT_DISABLE);

	/* Disable all interrupts for core */
	iowrite32(~0, core_base + CORE_INT_DISABLE);
	wmb();

	/* Load the command settings into the orb registers */
	sata_oxnas_tf_load(qc->ap, &qc->tf);

	/* both pio and dma commands use dma */
	if (ata_is_dma(qc->tf.protocol) || ata_is_pio(qc->tf.protocol)) {
		/* Start the DMA */
		iowrite32(SGDMA_CONTROL_GO,	pd->sgdma_base + SGDMA_CONTROL);
		wmb();
	}

	/* enable End of command interrupt */
	iowrite32(INT_WANT, port_base + INT_ENABLE);
	iowrite32(COREINT_END, core_base + CORE_INT_ENABLE);
	wmb();

	/* Start the command */
	reg = ioread32(port_base + SATA_COMMAND);
	reg &= ~SATA_OPCODE_MASK;
	reg |= CMD_WRITE_TO_ORB_REGS;
	iowrite32(reg , port_base + SATA_COMMAND);
	wmb();

	return 0;
}

/**
 * Will schedule the libATA error handler on the premise that there has
 * been a hotplug event on the port specified
 */
void sata_oxnas_checkforhotplug(struct ata_port *ap)
{
	DPRINTK("ENTER\n");

	ata_ehi_hotplugged(&ap->link.eh_info);
	ata_port_freeze(ap);
}


/**************************************************************************/
/* Locking                                                                */
/**************************************************************************/
/**
 * The underlying function that controls access to the sata core
 *
 * @return non-zero indicates that you have acquired exclusive access to the
 *         sata core.
 */
static int __acquire_sata_core(
	struct ata_host *ah,
	int port_no,
	oxnas_sata_isr_callback_t callback,
	void                    *arg,
	int                      may_sleep,
	int                      timeout_jiffies,
	int                      hw_access,
	void                    *uid,
	int                      locker_type)
{
	unsigned long end = jiffies + timeout_jiffies;
	int           acquired = 0;
	unsigned long flags;
	int           timed_out = 0;
	struct sata_oxnas_host_priv *hd;

	DEFINE_WAIT(wait);

	if (!ah)
		return acquired;

	hd = ah->private_data;

	spin_lock_irqsave(&hd->core_lock, flags);

	DPRINTK("Entered uid %p, port %d, h/w count %d, d count %d, "
		    "callback %p, hw_access %d, core_locked %d, "
		    "reentrant_port_no %d, isr_callback %p\n",
		uid, port_no, hd->hw_lock_count, hd->direct_lock_count,
		callback, hw_access, hd->core_locked, hd->reentrant_port_no,
		hd->isr_callback);

	while (!timed_out) {
		if (hd->core_locked ||
		    (!hw_access && hd->scsi_nonblocking_attempts)) {
			/* Can only allow access if from SCSI/SATA stack and if
			 * reentrant access is allowed and this access is to the
			 * same port for which the lock is current held
			 */
			if (hw_access && (port_no == hd->reentrant_port_no)) {
				BUG_ON(!hd->hw_lock_count);
				++(hd->hw_lock_count);

				DPRINTK("Allow SCSI/SATA re-entrant access to "
					"uid %p port %d\n", uid, port_no);
				acquired = 1;
				break;
			} else if (!hw_access) {
				if ((locker_type == SATA_READER) &&
				    (hd->current_locker_type == SATA_READER)) {
					WARN(1,
						"Already locked by reader, "
						"uid %p, locker_uid %p, "
						"port %d, h/w count %d, "
						"d count %d, hw_access %d\n",
						uid, hd->locker_uid, port_no,
						hd->hw_lock_count,
						hd->direct_lock_count,
						hw_access);
					goto check_uid;
				}

				if ((locker_type != SATA_READER) &&
				    (locker_type != SATA_WRITER)) {
					goto wait_for_lock;
				}

check_uid:
				WARN(uid == hd->locker_uid, "Attempt to lock "
					"by locker type %d uid %p, already "
					"locked by locker type %d with "
					"locker_uid %p, port %d, "
					"h/w count %d, d count %d, "
					"hw_access %d\n", locker_type, uid,
					hd->current_locker_type,
					hd->locker_uid, port_no,
					hd->hw_lock_count,
					hd->direct_lock_count, hw_access);
			}
		} else {
			WARN(hd->hw_lock_count || hd->direct_lock_count,
				"Core unlocked but counts non-zero: uid %p, "
				"locker_uid %p, port %d, h/w count %d, "
				"d count %d, hw_access %d\n", uid,
				hd->locker_uid, port_no, hd->hw_lock_count,
				hd->direct_lock_count, hw_access);

			BUG_ON(hd->current_locker_type != SATA_UNLOCKED);

			WARN(hd->locker_uid, "Attempt to lock uid %p when "
				"locker_uid %p is non-zero,  port %d, "
				"h/w count %d, d count %d, hw_access %d\n",
				uid, hd->locker_uid, port_no, hd->hw_lock_count,
				hd->direct_lock_count, hw_access);

			if (!hw_access) {
				/* Direct access attempting to acquire
				 * non-contented lock
				 */
				/* Must have callback for direct access */
				BUG_ON(!callback);
				/* Sanity check lock state */
				BUG_ON(hd->reentrant_port_no != -1);

				hd->isr_callback = callback;
				hd->isr_arg = arg;
				++(hd->direct_lock_count);

				hd->current_locker_type = locker_type;
			} else {
				/* SCSI/SATA attempting to acquire
				 * non-contented lock
				 */
				/* No callbacks for SCSI/SATA access */
				BUG_ON(callback);
				/* No callback args for SCSI/SATA access */
				BUG_ON(arg);

				/* Sanity check lock state */
				BUG_ON(hd->isr_callback);
				BUG_ON(hd->isr_arg);

				++(hd->hw_lock_count);
				hd->reentrant_port_no = port_no;

				hd->current_locker_type = SATA_SCSI_STACK;
			}

			hd->core_locked = 1;
			hd->locker_uid = uid;
			acquired = 1;
			break;
		}

wait_for_lock:
		if (!may_sleep) {
			DPRINTK("Denying for uid %p locker_type %d, "
			"hw_access %d, port %d, current_locker_type %d as "
			"cannot sleep\n", uid, locker_type, hw_access, port_no,
			hd->current_locker_type);

			if (hw_access)
				++(hd->scsi_nonblocking_attempts);

			break;
		}

		/* Core is locked and we're allowed to sleep, so wait to be
		 * awoken when the core is unlocked
		 */
		for (;;) {
			prepare_to_wait(hw_access ? &hd->scsi_wait_queue :
						    &hd->fast_wait_queue,
					&wait, TASK_UNINTERRUPTIBLE);
			if (!hd->core_locked &&
			    !(!hw_access && hd->scsi_nonblocking_attempts)) {
				/* We're going to use variables that will have
				 * been changed by the waker prior to clearing
				 * core_locked so we need to ensure we see
				 * changes to all those variables
				 */
				smp_rmb();
				break;
			}
			if (time_after(jiffies, end)) {
				printk(KERN_WARNING "__acquire_sata_core() "
					"uid %p failing for port %d timed out, "
					"locker_uid %p, h/w count %d, "
					"d count %d, callback %p, hw_access %d, "
					"core_locked %d, reentrant_port_no %d, "
					"isr_callback %p, isr_arg %p\n", uid,
					port_no, hd->locker_uid,
					hd->hw_lock_count,
					hd->direct_lock_count, callback,
					hw_access, hd->core_locked,
					hd->reentrant_port_no, hd->isr_callback,
					hd->isr_arg);
				timed_out = 1;
				break;
			}
			spin_unlock_irqrestore(&hd->core_lock, flags);
			if (!schedule_timeout(4*HZ)) {
				printk(KERN_INFO "__acquire_sata_core() uid %p, "
					"locker_uid %p, timed-out of "
					"schedule(), checking overall timeout\n",
					uid, hd->locker_uid);
			}
			spin_lock_irqsave(&hd->core_lock, flags);
		}
		finish_wait(hw_access ? &hd->scsi_wait_queue :
					&hd->fast_wait_queue, &wait);
	}

	if (hw_access && acquired) {
		if (hd->scsi_nonblocking_attempts)
			hd->scsi_nonblocking_attempts = 0;

		/* Wake any other SCSI/SATA waiters so they can get reentrant
		 * access to the same port if appropriate. This is because if
		 * the SATA core is locked by fast access, or SCSI/SATA access
		 * to other port, then can have >1 SCSI/SATA waiters on the wait
		 * list so want to give reentrant accessors a chance to get
		 * access ASAP
		 */
		if (!list_empty(&hd->scsi_wait_queue.head))
			wake_up(&hd->scsi_wait_queue);
	}

	DPRINTK("Leaving uid %p with acquired = %d, port %d, callback %p\n",
		uid, acquired, port_no, callback);

	spin_unlock_irqrestore(&hd->core_lock, flags);

	return acquired;
}

int sata_core_has_fast_waiters(struct ata_host *ah)
{
	int has_waiters;
	unsigned long flags;
	struct sata_oxnas_host_priv *hd = ah->private_data;

	spin_lock_irqsave(&hd->core_lock, flags);
	has_waiters = !list_empty(&hd->fast_wait_queue.head);
	spin_unlock_irqrestore(&hd->core_lock, flags);

	return has_waiters;
}
EXPORT_SYMBOL(sata_core_has_fast_waiters);

int sata_core_has_scsi_waiters(struct ata_host *ah)
{
	int has_waiters;
	unsigned long flags;
	struct sata_oxnas_host_priv *hd = ah->private_data;

	spin_lock_irqsave(&hd->core_lock, flags);
	has_waiters = hd->scsi_nonblocking_attempts ||
		      !list_empty(&hd->scsi_wait_queue.head);
	spin_unlock_irqrestore(&hd->core_lock, flags);

	return has_waiters;
}
EXPORT_SYMBOL(sata_core_has_scsi_waiters);

/*
 * ata_port operation to gain ownership of the SATA hardware prior to issuing
 * a command against a SATA host. Allows any number of users of the port against
 * which the lock was first acquired, thus enforcing that only one SATA core
 * port may be operated on at once.
 */
static int sata_oxnas_acquire_hw(
	struct ata_port *ap,
	int may_sleep,
	int timeout_jiffies)
{
	return __acquire_sata_core(ap->host, ap->port_no, NULL, 0, may_sleep,
				   timeout_jiffies, 1, (void *)HW_LOCKER_UID,
				   SATA_SCSI_STACK);
}

/*
 * operation to release ownership of the SATA hardware
 */
static void sata_oxnas_release_hw(struct ata_port *ap)
{
	unsigned long flags;
	int released = 0;
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	spin_lock_irqsave(&hd->core_lock, flags);

	DPRINTK("Entered port_no = %d, h/w count %d, d count %d, "
		"core locked = %d, reentrant_port_no = %d, isr_callback %p\n",
		ap->port_no, hd->hw_lock_count, hd->direct_lock_count,
		hd->core_locked, hd->reentrant_port_no, hd->isr_callback);

	if (!hd->core_locked) {
		/* Nobody holds the SATA lock */
		printk(KERN_WARNING "Nobody holds SATA lock, port_no %d\n",
		       ap->port_no);
		released = 1;
	} else if (!hd->hw_lock_count) {
		/* SCSI/SATA has released without holding the lock */
		printk(KERN_WARNING "SCSI/SATA does not hold SATA lock, "
		       "port_no %d\n", ap->port_no);
	} else {
		/* Trap incorrect usage */
		BUG_ON(hd->reentrant_port_no == -1);
		BUG_ON(ap->port_no != hd->reentrant_port_no);
		BUG_ON(hd->direct_lock_count);
		BUG_ON(hd->current_locker_type != SATA_SCSI_STACK);

		WARN(!hd->locker_uid || (hd->locker_uid != HW_LOCKER_UID),
			"Invalid locker uid %p, h/w count %d, d count %d, "
			"reentrant_port_no %d, core_locked %d, "
			"isr_callback %p\n", hd->locker_uid, hd->hw_lock_count,
			hd->direct_lock_count, hd->reentrant_port_no,
			hd->core_locked, hd->isr_callback);

		if (--(hd->hw_lock_count)) {
			DPRINTK("Still nested port_no %d\n", ap->port_no);
		} else {
			DPRINTK("Release port_no %d\n", ap->port_no);
			hd->reentrant_port_no = -1;
			hd->isr_callback = NULL;
			hd->current_locker_type = SATA_UNLOCKED;
			hd->locker_uid = 0;
			hd->core_locked = 0;
			released = 1;
			wake_up(!list_empty(&hd->scsi_wait_queue.head) ?
						&hd->scsi_wait_queue :
						&hd->fast_wait_queue);
		}
	}

	DPRINTK("Leaving, port_no %d, count %d\n", ap->port_no,
		hd->hw_lock_count);

	spin_unlock_irqrestore(&hd->core_lock, flags);

	/* CONFIG_SATA_OX820_DIRECT_HWRAID */
	/*    if (released)
	     ox820hwraid_restart_queue();
	} */
}

static inline int sata_oxnas_is_host_frozen(struct ata_host *ah)
{
	struct sata_oxnas_host_priv *hd = ah->private_data;

	smp_rmb();
	return hd->port_in_eh || hd->port_frozen;
}


static inline u32 sata_oxnas_hostportbusy(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	return (ioread32(hd->port_base + SATA_COMMAND) & CMD_CORE_BUSY) ||
	       (hd->n_ports > 1 &&
		(ioread32(hd->port_base + PORT_SIZE + SATA_COMMAND) &
		 CMD_CORE_BUSY));
}

static inline u32 sata_oxnas_hostdmabusy(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;

	return ioread32(pd->sgdma_base + SGDMA_STATUS) & SGDMA_BUSY;
}


/**
 * Turns on the cores clock and resets it
 */
static void sata_oxnas_reset_core(struct ata_host *ah)
{
	struct sata_oxnas_host_priv *host_priv = ah->private_data;
	int n;

	DPRINTK("ENTER\n");
	clk_prepare_enable(host_priv->clk);

	reset_control_assert(host_priv->rst_sata);
	reset_control_assert(host_priv->rst_link);
	reset_control_assert(host_priv->rst_phy);

	udelay(50);

	/* un-reset the PHY, then Link and Controller */
	reset_control_deassert(host_priv->rst_phy);
	udelay(50);

	reset_control_deassert(host_priv->rst_sata);
	reset_control_deassert(host_priv->rst_link);
	udelay(50);

	workaround5458(ah);
	/* tune for sata compatibility */
	sata_oxnas_link_write(ah->ports[0], 0x60, 0x2988);

	for (n = 0; n < host_priv->n_ports; n++) {
		/* each port in turn */
		sata_oxnas_link_write(ah->ports[n], 0x70, 0x55629);
	}
	udelay(50);
}


/**
 * Called after an identify device command has worked out what kind of device
 * is on the port
 *
 * @param port The port to configure
 * @param pdev The hardware associated with controlling the port
 */
static void sata_oxnas_dev_config(struct ata_device *pdev)
{
	struct sata_oxnas_port_priv *pd = pdev->link->ap->private_data;
	void __iomem *port_base = pd->port_base;
	u32 reg;

	DPRINTK("ENTER\n");
	/* Set the bits to put the port into 28 or 48-bit node */
	reg = ioread32(port_base + DRIVE_CONTROL);
	reg &= ~3;
	reg |= (pdev->flags & ATA_DFLAG_LBA48) ? DR_CON_48 : DR_CON_28;
	iowrite32(reg, port_base + DRIVE_CONTROL);

	/* if this is an ATA-6 disk, put port into ATA-5 auto translate mode */
	if (pdev->flags & ATA_DFLAG_LBA48) {
		reg = ioread32(port_base + PORT_CONTROL);
		reg |= 2;
		iowrite32(reg, port_base + PORT_CONTROL);
	}
}
/**
 * called to write a taskfile into the ORB registers
 * @param ap hardware with the registers in
 * @param tf taskfile to write to the registers
 */
static void sata_oxnas_tf_load(struct ata_port *ap,
				const struct ata_taskfile *tf)
{
	u32 count = 0;
	u32 Orb1 = 0;
	u32 Orb2 = 0;
	u32 Orb3 = 0;
	u32 Orb4 = 0;
	u32 Command_Reg;

	struct sata_oxnas_port_priv *port_priv = ap->private_data;
	void __iomem *port_base = port_priv->port_base;
	unsigned int is_addr = tf->flags & ATA_TFLAG_ISADDR;

	/* wait a maximum of 10ms for the core to be idle */
	do {
		Command_Reg = ioread32(port_base + SATA_COMMAND);
		if (!(Command_Reg & CMD_CORE_BUSY))
			break;
		count++;
		udelay(50);
	} while (count < 200);

	/* check if the ctl register has interrupts disabled or enabled and
	 * modify the interrupt enable registers on the ata core as required */
	if (tf->ctl & ATA_NIEN) {
		/* interrupts disabled */
		u32 mask = (COREINT_END << ap->port_no);

		iowrite32(mask, port_priv->core_base + CORE_INT_DISABLE);
		sata_oxnas_irq_clear(ap);
	} else {
		sata_oxnas_irq_on(ap);
	}

	Orb2 |= (tf->command) << 24;

	/* write 48 or 28 bit tf parameters */
	if (is_addr) {
		/* set LBA bit as it's an address */
		Orb1 |= (tf->device & ATA_LBA) << 24;

		if (tf->flags & ATA_TFLAG_LBA48) {
			Orb1 |= ATA_LBA << 24;
			Orb2 |= (tf->hob_nsect) << 8;
			Orb3 |= (tf->hob_lbal) << 24;
			Orb4 |= (tf->hob_lbam) << 0;
			Orb4 |= (tf->hob_lbah) << 8;
			Orb4 |= (tf->hob_feature) << 16;
		} else {
			Orb3 |= (tf->device & 0xf) << 24;
		}

		/* write 28-bit lba */
		Orb2 |= (tf->nsect) << 0;
		Orb2 |= (tf->feature) << 16;
		Orb3 |= (tf->lbal) << 0;
		Orb3 |= (tf->lbam) << 8;
		Orb3 |= (tf->lbah) << 16;
		Orb4 |= (tf->ctl) << 24;
	}

	if (tf->flags & ATA_TFLAG_DEVICE)
		Orb1 |= (tf->device) << 24;

	ap->last_ctl = tf->ctl;

	/* write values to registers */
	iowrite32(Orb1, port_base + ORB1);
	iowrite32(Orb2, port_base + ORB2);
	iowrite32(Orb3, port_base + ORB3);
	iowrite32(Orb4, port_base + ORB4);
}


void sata_oxnas_set_mode(struct ata_host *ah, u32 mode, u32 force)
{
	struct sata_oxnas_host_priv *host_priv = ah->private_data;
	void __iomem *core_base = host_priv->core_base;

	unsigned int *src;
	void __iomem *dst;
	unsigned int progmicrocode = 0;
	unsigned int changeparameters = 0;

	u32 previous_mode;

	/* these micro-code programs _should_ include the version word */

	/* JBOD */
	static const unsigned int jbod[] = {
		0x07B400AC, 0x0228A280, 0x00200001, 0x00204002, 0x00224001,
		0x00EE0009, 0x00724901, 0x01A24903, 0x00E40009, 0x00224001,
		0x00621120, 0x0183C908, 0x00E20005, 0x00718908, 0x0198A206,
		0x00621124, 0x0183C908, 0x00E20046, 0x00621104, 0x0183C908,
		0x00E20015, 0x00EE009D, 0x01A3E301, 0x00E2001B, 0x0183C900,
		0x00E2001B, 0x00210001, 0x00EE0020, 0x01A3E302, 0x00E2009D,
		0x0183C901, 0x00E2009D, 0x00210002, 0x0235D700, 0x0208A204,
		0x0071C908, 0x000F8207, 0x000FC207, 0x0071C920, 0x000F8507,
		0x000FC507, 0x0228A240, 0x02269A40, 0x00094004, 0x00621104,
		0x0180C908, 0x00E40031, 0x00621112, 0x01A3C801, 0x00E2002B,
		0x00294000, 0x0228A220, 0x01A69ABF, 0x002F8000, 0x002FC000,
		0x0198A204, 0x0001C022, 0x01B1A220, 0x0001C106, 0x00088007,
		0x0183C903, 0x00E2009D, 0x0228A220, 0x0071890C, 0x0208A206,
		0x0198A206, 0x0001C022, 0x01B1A220, 0x0001C106, 0x00088007,
		0x00EE009D, 0x00621104, 0x0183C908, 0x00E2004A, 0x00EE009D,
		0x01A3C901, 0x00E20050, 0x0021E7FF, 0x0183E007, 0x00E2009D,
		0x00EE0054, 0x0061600B, 0x0021E7FF, 0x0183C507, 0x00E2009D,
		0x01A3E301, 0x00E2005A, 0x0183C900, 0x00E2005A, 0x00210001,
		0x00EE005F, 0x01A3E302, 0x00E20005, 0x0183C901, 0x00E20005,
		0x00210002, 0x0235D700, 0x0208A204, 0x000F8109, 0x000FC109,
		0x0071C918, 0x000F8407, 0x000FC407, 0x0001C022, 0x01A1A2BF,
		0x0001C106, 0x00088007, 0x02269A40, 0x00094004, 0x00621112,
		0x01A3C801, 0x00E4007F, 0x00621104, 0x0180C908, 0x00E4008D,
		0x00621128, 0x0183C908, 0x00E2006C, 0x01A3C901, 0x00E2007B,
		0x0021E7FF, 0x0183E007, 0x00E2007F, 0x00EE006C, 0x0061600B,
		0x0021E7FF, 0x0183C507, 0x00E4006C, 0x00621111, 0x01A3C801,
		0x00E2007F, 0x00621110, 0x01A3C801, 0x00E20082, 0x0228A220,
		0x00621119, 0x01A3C801, 0x00E20086, 0x0001C022, 0x01B1A220,
		0x0001C106, 0x00088007, 0x0198A204, 0x00294000, 0x01A69ABF,
		0x002F8000, 0x002FC000, 0x0183C903, 0x00E20005, 0x0228A220,
		0x0071890C, 0x0208A206, 0x0198A206, 0x0001C022, 0x01B1A220,
		0x0001C106, 0x00088007, 0x00EE009D, 0x00621128, 0x0183C908,
		0x00E20005, 0x00621104, 0x0183C908, 0x00E200A6, 0x0062111C,
		0x0183C908, 0x00E20005, 0x0071890C, 0x0208A206, 0x0198A206,
		0x00718908, 0x0208A206, 0x00EE0005, ~0
	};

	/* Bi-Modal RAID-0/1 */
	static const unsigned int raid[] = {
		0x00F20145, 0x00EE20FA, 0x00EE20A7, 0x0001C009, 0x00EE0004,
		0x00220000, 0x0001000B, 0x037003FF, 0x00700018, 0x037003FE,
		0x037043FD, 0x00704118, 0x037043FC, 0x01A3D240, 0x00E20017,
		0x00B3C235, 0x00E40018, 0x0093C104, 0x00E80014, 0x0093C004,
		0x00E80017, 0x01020000, 0x00274020, 0x00EE0083, 0x0080C904,
		0x0093C104, 0x00EA0020, 0x0093C103, 0x00EC001F, 0x00220002,
		0x00924104, 0x0005C009, 0x00EE0058, 0x0093CF04, 0x00E80026,
		0x00900F01, 0x00600001, 0x00910400, 0x00EE0058, 0x00601604,
		0x01A00003, 0x00E2002C, 0x01018000, 0x00274040, 0x00EE0083,
		0x0093CF03, 0x00EC0031, 0x00220003, 0x00924F04, 0x0005C009,
		0x00810104, 0x00B3C235, 0x00E20037, 0x0022C000, 0x00218210,
		0x00EE0039, 0x0022C001, 0x00218200, 0x00600401, 0x00A04901,
		0x00604101, 0x01A0C401, 0x00E20040, 0x00216202, 0x00EE0041,
		0x00216101, 0x02018506, 0x00EE2141, 0x00904901, 0x00E20049,
		0x00A00401, 0x00600001, 0x02E0C301, 0x00EE2141, 0x00216303,
		0x037003EE, 0x01A3C001, 0x00E40105, 0x00250080, 0x00204000,
		0x002042F1, 0x0004C001, 0x00230001, 0x00100006, 0x02C18605,
		0x00100006, 0x01A3D502, 0x00E20055, 0x00EE0053, 0x00004009,
		0x00000004, 0x00B3C235, 0x00E40062, 0x0022C001, 0x0020C000,
		0x00EE2141, 0x0020C001, 0x00EE2141, 0x00EE006B, 0x0022C000,
		0x0060D207, 0x00EE2141, 0x00B3C242, 0x00E20069, 0x01A3D601,
		0x00E2006E, 0x02E0C301, 0x00EE2141, 0x00230001, 0x00301303,
		0x00EE007B, 0x00218210, 0x01A3C301, 0x00E20073, 0x00216202,
		0x00EE0074, 0x00216101, 0x02018506, 0x00214000, 0x037003EE,
		0x01A3C001, 0x00E40108, 0x00230001, 0x00100006, 0x00250080,
		0x00204000, 0x002042F1, 0x0004C001, 0x00EE007F, 0x0024C000,
		0x01A3D1F0, 0x00E20088, 0x00230001, 0x00300000, 0x01A3D202,
		0x00E20085, 0x00EE00A5, 0x00B3C800, 0x00E20096, 0x00218000,
		0x00924709, 0x0005C009, 0x00B20802, 0x00E40093, 0x037103FD,
		0x00710418, 0x037103FC, 0x00EE0006, 0x00220000, 0x0001000F,
		0x00EE0006, 0x00800B0C, 0x00B00001, 0x00204000, 0x00208550,
		0x00208440, 0x002083E0, 0x00208200, 0x00208100, 0x01008000,
		0x037083EE, 0x02008212, 0x02008216, 0x01A3C201, 0x00E400A5,
		0x0100C000, 0x00EE20FA, 0x02800000, 0x00208000, 0x00B24C00,
		0x00E400AD, 0x00224001, 0x00724910, 0x0005C009, 0x00B3CDC4,
		0x00E200D5, 0x00B3CD29, 0x00E200D5, 0x00B3CD20, 0x00E200D5,
		0x00B3CD24, 0x00E200D5, 0x00B3CDC5, 0x00E200D2, 0x00B3CD39,
		0x00E200D2, 0x00B3CD30, 0x00E200D2, 0x00B3CD34, 0x00E200D2,
		0x00B3CDCA, 0x00E200CF, 0x00B3CD35, 0x00E200CF, 0x00B3CDC8,
		0x00E200CC, 0x00B3CD25, 0x00E200CC, 0x00B3CD40, 0x00E200CB,
		0x00B3CD42, 0x00E200CB, 0x01018000, 0x00EE0083, 0x0025C000,
		0x036083EE, 0x0000800D, 0x00EE00D8, 0x036083EE, 0x00208035,
		0x00EE00DA, 0x036083EE, 0x00208035, 0x00EE00DA, 0x00208007,
		0x036083EE, 0x00208025, 0x036083EF, 0x02400000, 0x01A3D208,
		0x00E200D8, 0x0067120A, 0x0021C000, 0x0021C224, 0x00220000,
		0x00404B1C, 0x00600105, 0x00800007, 0x0020C00E, 0x00214000,
		0x01004000, 0x01A0411F, 0x00404E01, 0x01A3C101, 0x00E200F1,
		0x00B20800, 0x00E400D8, 0x00220001, 0x0080490B, 0x00B04101,
		0x0040411C, 0x00EE00E1, 0x02269A01, 0x01020000, 0x02275D80,
		0x01A3D202, 0x00E200F4, 0x01B75D80, 0x01030000, 0x01B69A01,
		0x00EE00D8, 0x01A3D204, 0x00E40104, 0x00224000, 0x0020C00E,
		0x0020001E, 0x00214000, 0x01004000, 0x0212490E, 0x00214001,
		0x01004000, 0x02400000, 0x00B3D702, 0x00E80112, 0x00EE010E,
		0x00B3D702, 0x00E80112, 0x00B3D702, 0x00E4010E, 0x00230001,
		0x00EE0140, 0x00200005, 0x036003EE, 0x00204001, 0x00EE0116,
		0x00230001, 0x00100006, 0x02C18605, 0x00100006, 0x01A3D1F0,
		0x00E40083, 0x037003EE, 0x01A3C002, 0x00E20121, 0x0020A300,
		0x0183D102, 0x00E20124, 0x037003EE, 0x01A00005, 0x036003EE,
		0x01A0910F, 0x00B3C20F, 0x00E2012F, 0x01A3D502, 0x00E20116,
		0x01A3C002, 0x00E20116, 0x00B3D702, 0x00E4012C, 0x00300000,
		0x00EE011F, 0x02C18605, 0x00100006, 0x00EE0116, 0x01A3D1F0,
		0x00E40083, 0x037003EE, 0x01A3C004, 0x00E20088, 0x00200003,
		0x036003EE, 0x01A3D502, 0x00E20136, 0x00230001, 0x00B3C101,
		0x00E4012C, 0x00100006, 0x02C18605, 0x00100006, 0x00204000,
		0x00EE0116, 0x00100006, 0x01A3D1F0, 0x00E40083, 0x01000000,
		0x02400000, ~0
	};

	DPRINTK("ENTER: mode:%d, force:%d\n", mode, force);

	if (force)
		previous_mode = UNKNOWN_MODE;
	else
		previous_mode = host_priv->current_ucode;

	if (mode == previous_mode)
		return;

	host_priv->current_ucode = mode;

	/* decide what needs to be done using the STD in my logbook */
	switch (previous_mode) {
	case OXNASSATA_RAID1:
		switch (mode) {
		case OXNASSATA_RAID0:
			changeparameters = 1;
			break;
		case OXNASSATA_NOTRAID:
			changeparameters = 1;
			progmicrocode = 1;
			break;
		}
		break;
	case OXNASSATA_RAID0:
		switch (mode) {
		case OXNASSATA_RAID1:
			changeparameters = 1;
			break;
		case OXNASSATA_NOTRAID:
			changeparameters = 1;
			progmicrocode = 1;
			break;
		}
		break;
	case OXNASSATA_NOTRAID:
		switch (mode) {
		case OXNASSATA_RAID0:
		case OXNASSATA_RAID1:
			changeparameters = 1;
			progmicrocode = 1;
			break;
		}
		break;
	case UNKNOWN_MODE:
		changeparameters = 1;
		progmicrocode = 1;
		break;
	}

	/* no need to reprogram everything if already in the right mode */
	if (progmicrocode) {
		/* reset micro-code processor */
		iowrite32(1, core_base + PROC_RESET);
		wmb();

		/* select micro-code */
		switch (mode) {
		case OXNASSATA_RAID1:
		case OXNASSATA_RAID0:
			VPRINTK("Loading RAID micro-code\n");
			src = (unsigned int *)&raid[1];
			break;
		case OXNASSATA_NOTRAID:
			VPRINTK("Loading JBOD micro-code\n");
			src = (unsigned int *)&jbod[1];
			break;
		default:
			BUG();
			break;
		}

		/* load micro code */
		dst = core_base + UCODE_STORE;
		while (*src != ~0) {
			iowrite32(*src, dst);
			src++;
			dst += sizeof(*src);
		}
		wmb();
	}

	if (changeparameters) {
		u32 reg;
		/* set other mode dependent flags */
		switch (mode) {
		case OXNASSATA_RAID1:
			/* clear JBOD mode */
			reg = ioread32(core_base + DATA_PLANE_CTRL);
			reg |= DPC_JBOD_UCODE;
			reg &= ~DPC_FIS_SWCH;
			iowrite32(reg, core_base + DATA_PLANE_CTRL);
			wmb();

			/* set the hardware up for RAID-1 */
			iowrite32(0, core_base + RAID_WP_BOT_LOW);
			iowrite32(0, core_base + RAID_WP_BOT_HIGH);
			iowrite32(0xffffffff, core_base + RAID_WP_TOP_LOW);
			iowrite32(0x7fffffff, core_base + RAID_WP_TOP_HIGH);
			iowrite32(0, core_base + RAID_SIZE_LOW);
			iowrite32(0, core_base + RAID_SIZE_HIGH);
			wmb();
			break;
		case OXNASSATA_RAID0:
			/* clear JBOD mode */
			reg = ioread32(core_base + DATA_PLANE_CTRL);
			reg |= DPC_JBOD_UCODE;
			reg &= ~DPC_FIS_SWCH;
			iowrite32(reg, core_base + DATA_PLANE_CTRL);
			wmb();

			/* set the hardware up for RAID-1 */
			iowrite32(0, core_base + RAID_WP_BOT_LOW);
			iowrite32(0, core_base + RAID_WP_BOT_HIGH);
			iowrite32(0xffffffff, core_base + RAID_WP_TOP_LOW);
			iowrite32(0x7fffffff, core_base + RAID_WP_TOP_HIGH);
			iowrite32(0xffffffff, core_base + RAID_SIZE_LOW);
			iowrite32(0x7fffffff, core_base + RAID_SIZE_HIGH);
			wmb();
			break;
		case OXNASSATA_NOTRAID:
			/* enable jbod mode */
			reg = ioread32(core_base + DATA_PLANE_CTRL);
			reg &= ~DPC_JBOD_UCODE;
			reg &= ~DPC_FIS_SWCH;
			iowrite32(reg, core_base + DATA_PLANE_CTRL);
			wmb();

			/* start micro-code processor*/
			iowrite32(1, core_base + PROC_START);
			break;
		default:
			reg = ioread32(core_base + DATA_PLANE_CTRL);
			reg |= DPC_JBOD_UCODE;
			reg &= ~DPC_FIS_SWCH;
			iowrite32(reg, core_base + DATA_PLANE_CTRL);
			wmb();
			break;
		}
	}
}

/**
 * sends a sync-escape if there is a link present
 */
static inline void sata_oxnas_send_sync_escape(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 reg;

	/* read the SSTATUS register and only send a sync escape if there is a
	* link active */
	if ((sata_oxnas_link_read(ap, 0x20) & 3) == 3) {
		reg = ioread32(pd->port_base + SATA_COMMAND);
		reg &= ~SATA_OPCODE_MASK;
		reg |= CMD_SYNC_ESCAPE;
		iowrite32(reg, pd->port_base + SATA_COMMAND);
	}
}

/* clears errors */
static inline void sata_oxnas_clear_CS_error(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 *base = pd->port_base;
	u32 reg;

	reg = ioread32(base + SATA_CONTROL);
	reg &= SATA_CTL_ERR_MASK;
	iowrite32(reg, base + SATA_CONTROL);
}

static inline void sata_oxnas_reset_sgdma(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;

	iowrite32(SGDMA_RESETS_CTRL, pd->sgdma_base + SGDMA_RESETS);
}

static inline void sata_oxnas_reset_dma(struct ata_port *ap, int assert)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 reg;

	reg = ioread32(pd->dmactl_base + DMA_CONTROL);
	if (assert)
		reg |= DMA_CONTROL_RESET;
	else
		reg &= ~DMA_CONTROL_RESET;

	iowrite32(reg, pd->dmactl_base + DMA_CONTROL);
};

/**
 * Clears the error caused by the core's registers being accessed when the
 * core is busy.
 */
static inline void sata_oxnas_clear_reg_access_error(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 *base = pd->port_base;
	u32 reg;

	reg = ioread32(base + INT_STATUS);

	DPRINTK("ENTER\n");
	if (reg & INT_REG_ACCESS_ERR) {
		DPRINTK("clearing register access error on port %d\n",
			ap->port_no);
		iowrite32(INT_REG_ACCESS_ERR, base + INT_STATUS);
	}
	reg = ioread32(base + INT_STATUS);
	if (reg & INT_REG_ACCESS_ERR)
		DPRINTK("register access error didn't clear\n");
}

static inline void sata_oxnas_clear_sctl_error(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 *base = pd->port_base;
	u32 reg;

	reg = ioread32(base + SATA_CONTROL);
	reg |= SCTL_CLR_ERR;
	iowrite32(reg, base + SATA_CONTROL);
}

static inline void sata_oxnas_clear_raid_error(struct ata_host *ah)
{
	return;
};

/**
 * Clean up all the state machines in the sata core.
 * @return post cleanup action required
 */
static int sata_oxnas_cleanup(struct ata_host *ah)
{
	struct sata_oxnas_host_priv *hd = ah->private_data;
	int actions_required = 0;
	int n;

	printk(KERN_INFO "sata_oxnas: resetting SATA core\n");
	/* core not recovering, reset it */
	mdelay(5);
	sata_oxnas_reset_core(ah);
	mdelay(5);
	actions_required |= OXNAS_SATA_REINIT;
	/* Perform any SATA core re-initialisation after reset post reset init
	 * needs to be called for both ports as there's one reset for both
	 * ports */
	for (n = 0; n < hd->n_ports; n++)
		sata_oxnas_post_reset_init(ah->ports[n]);


	return actions_required;
}

/**
 *  ata_qc_new - Request an available ATA command, for queueing
 *  @ap: Port associated with device @dev
 *  @return non zero will refuse a new command, zero will may grant on subject
 *          to conditions elsewhere.
 *
 */
static int sata_oxnas_qc_new(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	DPRINTK("port %d\n", ap->port_no);
	smp_rmb();
	if (hd->port_frozen || hd->port_in_eh)
		return 1;
	else
		return !sata_oxnas_acquire_hw(ap, 0, 0);
}

/**
 * releases the lock on the port the command used
 */
static void sata_oxnas_qc_free(struct ata_queued_cmd *qc)
{
	DPRINTK("\n");
	sata_oxnas_release_hw(qc->ap);
}

static void sata_oxnas_freeze(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	DPRINTK("\n");
	hd->port_frozen |= BIT(ap->port_no);
	smp_wmb();
}

static void sata_oxnas_thaw(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	DPRINTK("\n");
	hd->port_frozen &= ~BIT(ap->port_no);
	smp_wmb();
}

void sata_oxnas_freeze_host(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	DPRINTK("ENTER\n");
	hd->port_in_eh |= BIT(ap->port_no);
	smp_wmb();
}

void sata_oxnas_thaw_host(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	DPRINTK("ENTER\n");
	hd->port_in_eh &= ~BIT(ap->port_no);
	smp_wmb();
}

static void sata_oxnas_post_internal_cmd(struct ata_queued_cmd *qc)
{
	DPRINTK("ENTER\n");
	/* If the core is busy here, make it idle */
	if (qc->flags & ATA_QCFLAG_FAILED)
		sata_oxnas_cleanup(qc->ap->host);
}


/**
 * turn on the interrupts
 *
 * @param ap Hardware with the registers in
 */
static void sata_oxnas_irq_on(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	u32 mask = (COREINT_END << ap->port_no);

	/* Clear pending interrupts */
	iowrite32(~0, pd->port_base + INT_CLEAR);
	iowrite32(mask, pd->core_base + CORE_INT_STATUS);
	wmb();

	/* enable End of command interrupt */
	iowrite32(INT_WANT, pd->port_base + INT_ENABLE);
	iowrite32(mask, pd->core_base + CORE_INT_ENABLE);
}


/** @return true if the port has a cable connected */
int sata_oxnas_check_link(struct ata_port *ap)
{
	int reg;

	sata_oxnas_scr_read_port(ap, SCR_STATUS, &reg);
	/* Check for the cable present indicated by SCR status bit-0 set */
	return reg & 0x1;
}

/**
 *	ata_std_postreset - standard postreset callback
 *	@link: the target ata_link
 *	@classes: classes of attached devices
 *
 *	This function is invoked after a successful reset. Note that
 *	the device might have been reset more than once using
 *	different reset methods before postreset is invoked.
 *
 *	LOCKING:
 *	Kernel thread context (may sleep)
 */
static void sata_oxnas_postreset(struct ata_link *link, unsigned int *classes)
{
	struct ata_port *ap = link->ap;
	struct sata_oxnas_host_priv *hd = ap->host->private_data;

	unsigned int dev;

	DPRINTK("ENTER\n");
	ata_std_postreset(link, classes);

	/* turn on phy error detection by removing the masks */
	sata_oxnas_link_write(ap->host->ports[0], 0x0c, 0x30003);
	if (hd->n_ports > 1)
		sata_oxnas_link_write(ap->host->ports[1], 0x0c, 0x30003);

	/* bail out if no device is present */
	if (classes[0] == ATA_DEV_NONE && classes[1] == ATA_DEV_NONE) {
		DPRINTK("EXIT, no device\n");
		return;
	}

	/* go through all the devices and configure them */
	for (dev = 0; dev < ATA_MAX_DEVICES; ++dev) {
		if (ap->link.device[dev].class == ATA_DEV_ATA)
			sata_oxnas_dev_config(&(ap->link.device[dev]));
	}

	DPRINTK("EXIT\n");
}

/**
 * Called to read the hardware registers / DMA buffers, to
 * obtain the current set of taskfile register values.
 * @param ap hardware with the registers in
 * @param tf taskfile to read the registers into
 */
static void sata_oxnas_tf_read(struct ata_port *ap, struct ata_taskfile *tf)
{
	struct sata_oxnas_port_priv *port_priv = ap->private_data;
	void __iomem *port_base = port_priv->port_base;
	/* read the orb registers */
	u32 Orb1 = ioread32(port_base + ORB1);
	u32 Orb2 = ioread32(port_base + ORB2);
	u32 Orb3 = ioread32(port_base + ORB3);
	u32 Orb4 = ioread32(port_base + ORB4);

	/* read common 28/48 bit tf parameters */
	tf->device = (Orb1 >> 24);
	tf->nsect = (Orb2 >> 0);
	tf->feature = (Orb2 >> 16);
	tf->command = sata_oxnas_check_status(ap);

	/* read 48 or 28 bit tf parameters */
	if (tf->flags & ATA_TFLAG_LBA48) {
		tf->hob_nsect = (Orb2 >> 8);
		tf->lbal = (Orb3 >> 0);
		tf->lbam = (Orb3 >> 8);
		tf->lbah = (Orb3 >> 16);
		tf->hob_lbal = (Orb3 >> 24);
		tf->hob_lbam = (Orb4 >> 0);
		tf->hob_lbah = (Orb4 >> 8);
		/* feature ext and control are write only */
	} else {
		/* read 28-bit lba */
		tf->lbal = (Orb3 >> 0);
		tf->lbam = (Orb3 >> 8);
		tf->lbah = (Orb3 >> 16);
	}
}

/**
 * Read a result task-file from the sata core registers.
 */
static bool sata_oxnas_qc_fill_rtf(struct ata_queued_cmd *qc)
{
	/* Read the most recently received FIS from the SATA core ORB registers
	 and convert to an ATA taskfile */
	sata_oxnas_tf_read(qc->ap, &qc->result_tf);
	return true;
}

/**
 * Reads the Status ATA shadow register from hardware.
 *
 * @return The status register
 */
static u8 sata_oxnas_check_status(struct ata_port *ap)
{
	u32 Reg;
	u8 status;
	struct sata_oxnas_port_priv *port_priv = ap->private_data;
	void __iomem *port_base = port_priv->port_base;

	/* read byte 3 of Orb2 register */
	status = ioread32(port_base + ORB2) >> 24;

	/* check for the drive going missing indicated by SCR status bits
	 * 0-3 = 0 */
	sata_oxnas_scr_read_port(ap, SCR_STATUS, &Reg);

	if (!(Reg & 0x1)) {
		status |= ATA_DF;
		status |= ATA_ERR;
	}

	return status;
}

static inline void sata_oxnas_reset_ucode(struct ata_host *ah, int force,
					  int no_microcode)
{
	struct sata_oxnas_host_priv *hd = ah->private_data;

	DPRINTK("ENTER\n");
	if (no_microcode) {
		u32 reg;

		sata_oxnas_set_mode(ah, UNKNOWN_MODE, force);
		reg = ioread32(hd->core_base + DEVICE_CONTROL);
		reg |= DEVICE_CONTROL_ATA_ERR_OVERRIDE;
		iowrite32(reg, hd->core_base + DEVICE_CONTROL);
	} else {
		/* JBOD uCode */
		sata_oxnas_set_mode(ah, OXNASSATA_NOTRAID, force);
		/* Turn the work around off as it may have been left on by any
		 * HW-RAID code that we've been working with */
		iowrite32(0x0, hd->core_base + PORT_ERROR_MASK);
	}
}

/**
 * Prepare as much as possible for a command without involving anything that is
 * shared between ports.
 */
static enum ata_completion_errors sata_oxnas_qc_prep(struct ata_queued_cmd *qc)
{
	struct sata_oxnas_port_priv *pd;
	int port_no = qc->ap->port_no;

	/* if the port's not connected, complete now with an error */
	if (!sata_oxnas_check_link(qc->ap)) {
		ata_port_err(qc->ap,
			"port %d not connected completing with error\n",
			port_no);
		qc->err_mask |= AC_ERR_ATA_BUS;
		ata_qc_complete(qc);
	}

	sata_oxnas_reset_ucode(qc->ap->host, 0, 0);

	/* both pio and dma commands use dma */
	if (ata_is_dma(qc->tf.protocol) || ata_is_pio(qc->tf.protocol)) {

		/* program the scatterlist into the prd table */
		ata_bmdma_qc_prep(qc);

		/* point the sgdma controller at the dma request structure */
		pd = qc->ap->private_data;

		iowrite32(pd->sgdma_request_pa,
				pd->sgdma_base + SGDMA_REQUESTPTR);

		/* setup the request table */
		if (port_no == 0) {
			pd->sgdma_request->control =
				(qc->dma_dir == DMA_FROM_DEVICE) ?
					SGDMA_REQCTL0IN : SGDMA_REQCTL0OUT;
		} else {
			pd->sgdma_request->control =
				(qc->dma_dir == DMA_FROM_DEVICE) ?
					SGDMA_REQCTL1IN : SGDMA_REQCTL1OUT;
		}
		pd->sgdma_request->qualifier = SGDMA_REQQUAL;
		pd->sgdma_request->src_pa = qc->ap->bmdma_prd_dma;
		pd->sgdma_request->dst_pa = qc->ap->bmdma_prd_dma;
		smp_wmb();

		/* tell it to wait */
		iowrite32(SGDMA_CONTROL_NOGO, pd->sgdma_base + SGDMA_CONTROL);
	}

	return AC_ERR_OK;
}

static int sata_oxnas_port_start(struct ata_port *ap)
{
	struct sata_oxnas_host_priv *host_priv = ap->host->private_data;
	struct device *dev = ap->host->dev;
	struct sata_oxnas_port_priv *pp;
	void *mem;
	dma_addr_t mem_dma;

	DPRINTK("ENTER\n");

	pp = kzalloc(sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	pp->port_base = host_priv->port_base +
			(ap->port_no ? PORT_SIZE : 0);
	pp->dmactl_base = host_priv->dmactl_base +
			 (ap->port_no ? DMA_CORESIZE : 0);
	pp->sgdma_base = host_priv->sgdma_base +
			 (ap->port_no ? SGDMA_CORESIZE : 0);
	pp->core_base = host_priv->core_base;

	/* preallocated */
	if (host_priv->dma_size >= SATA_OXNAS_DMA_SIZE * host_priv->n_ports) {
		DPRINTK("using preallocated DMA\n");
		mem_dma = host_priv->dma_base +
				(ap->port_no ? SATA_OXNAS_DMA_SIZE : 0);
		mem = ioremap(mem_dma, SATA_OXNAS_DMA_SIZE);
	} else {
		mem = dma_alloc_coherent(dev, SATA_OXNAS_DMA_SIZE, &mem_dma,
					 GFP_KERNEL);
	}
	if (!mem)
		goto err_ret;

	pp->sgdma_request_pa = mem_dma;
	pp->sgdma_request = mem;

	ap->bmdma_prd_dma = mem_dma + sizeof(struct sgdma_request);
	ap->bmdma_prd = mem + sizeof(struct sgdma_request);

	ap->private_data = pp;

	sata_oxnas_post_reset_init(ap);

	return 0;

err_ret:
	kfree(pp);
	return -ENOMEM;

}

static void sata_oxnas_port_stop(struct ata_port *ap)
{
	struct device *dev = ap->host->dev;
	struct sata_oxnas_port_priv *pp = ap->private_data;
	struct sata_oxnas_host_priv *host_priv = ap->host->private_data;

	DPRINTK("ENTER\n");
	ap->private_data = NULL;
	if (host_priv->dma_size) {
		iounmap(pp->sgdma_request);
	} else {
		dma_free_coherent(dev, SATA_OXNAS_DMA_SIZE,
				  pp->sgdma_request, pp->sgdma_request_pa);
	}

	kfree(pp);
}


static void sata_oxnas_post_reset_init(struct ata_port *ap)
{
	uint dev;

	/* force to load u-code only once after reset */
	sata_oxnas_reset_ucode(ap->host, !ap->port_no, 0);

	/* turn on phy error detection by removing the masks */
	sata_oxnas_link_write(ap, 0x0C, 0x30003);

	/* enable hotplug event detection */
	sata_oxnas_scr_write_port(ap, SCR_ERROR, ~0);
	sata_oxnas_scr_write_port(ap, SERROR_IRQ_MASK, 0x03feffff);
	sata_oxnas_scr_write_port(ap, SCR_ACTIVE, ~0 & ~(1 << 26) & ~(1 << 16));

	/* enable interrupts for ports */
	sata_oxnas_irq_on(ap);

	/* go through all the devices and configure them */
	for (dev = 0; dev < ATA_MAX_DEVICES; ++dev) {
		if (ap->link.device[dev].class == ATA_DEV_ATA) {
			sata_std_hardreset(&ap->link, NULL, jiffies + HZ);
			sata_oxnas_dev_config(&(ap->link.device[dev]));
		}
	}

	/* clean up any remaining errors */
	sata_oxnas_scr_write_port(ap, SCR_ERROR, ~0);
	VPRINTK("done\n");
}

/**
 * host_stop() is called when the rmmod or hot unplug process begins. The
 * hook must stop all hardware interrupts, DMA engines, etc.
 *
 * @param ap hardware with the registers in
 */
static void sata_oxnas_host_stop(struct ata_host *host_set)
{
	DPRINTK("\n");
}


#define ERROR_HW_ACQUIRE_TIMEOUT_JIFFIES (10 * HZ)
static void sata_oxnas_error_handler(struct ata_port *ap)
{
	DPRINTK("Enter port_no %d\n", ap->port_no);
	sata_oxnas_freeze_host(ap);

	/* If the core is busy here, make it idle */
	sata_oxnas_cleanup(ap->host);

	ata_std_error_handler(ap);

	sata_oxnas_thaw_host(ap);
}

static int sata_oxnas_softreset(struct ata_link *link, unsigned int *class,
				 unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct sata_oxnas_port_priv *pd = ap->private_data;
	void __iomem *port_base = pd->port_base;
	int rc;

	struct ata_taskfile tf;
	u32 Command_Reg;

	DPRINTK("ENTER\n");

	port_base = pd->port_base;

	if (ata_link_offline(link)) {
		DPRINTK("PHY reports no device\n");
		*class = ATA_DEV_NONE;
		goto out;
	}

	/* write value to register */
	iowrite32(0, port_base + ORB1);
	iowrite32(0, port_base + ORB2);
	iowrite32(0, port_base + ORB3);
	iowrite32((ap->ctl) << 24, port_base + ORB4);

	/* command the core to send a control FIS */
	Command_Reg = ioread32(port_base + SATA_COMMAND);
	Command_Reg &= ~SATA_OPCODE_MASK;
	Command_Reg |= CMD_WRITE_TO_ORB_REGS_NO_COMMAND;
	iowrite32(Command_Reg, port_base + SATA_COMMAND);
	udelay(20);	/* FIXME: flush */

	/* write value to register */
	iowrite32((ap->ctl | ATA_SRST) << 24, port_base + ORB4);

	/* command the core to send a control FIS */
	Command_Reg &= ~SATA_OPCODE_MASK;
	Command_Reg |= CMD_WRITE_TO_ORB_REGS_NO_COMMAND;
	iowrite32(Command_Reg, port_base + SATA_COMMAND);
	udelay(20);	/* FIXME: flush */

	/* write value to register */
	iowrite32((ap->ctl) << 24, port_base + ORB4);

	/* command the core to send a control FIS */
	Command_Reg &= ~SATA_OPCODE_MASK;
	Command_Reg |= CMD_WRITE_TO_ORB_REGS_NO_COMMAND;
	iowrite32(Command_Reg, port_base + SATA_COMMAND);

	msleep(150);

	rc = ata_sff_wait_ready(link, deadline);

	/* if link is occupied, -ENODEV too is an error */
	if (rc && (rc != -ENODEV || sata_scr_valid(link))) {
		ata_link_err(link, "SRST failed (errno=%d)\n", rc);
		return rc;
	}

	/* determine by signature whether we have ATA or ATAPI devices */
	sata_oxnas_tf_read(ap, &tf);
	*class = ata_dev_classify(&tf);

	if (*class == ATA_DEV_UNKNOWN)
		*class = ATA_DEV_NONE;

out:
	DPRINTK("EXIT, class=%u\n", *class);
	return 0;
}


int	sata_oxnas_init_controller(struct ata_host *host)
{
	return 0;
}

/**
 * Ref bug-6320
 *
 * This code is a work around for a DMA hardware bug that will repeat the
 * penultimate 8-bytes on some reads. This code will check that the amount
 * of data transferred is a multiple of 512 bytes, if not the in it will
 * fetch the correct data from a buffer in the SATA core and copy it into
 * memory.
 *
 * @param port SATA port to check and if necessary, correct.
 */
static int sata_oxnas_bug_6320_detect(struct ata_port *ap)
{
	struct sata_oxnas_port_priv *pd = ap->private_data;
	void __iomem *core_base = pd->core_base;
	int is_read;
	int quads_transferred;
	int remainder;
	int sector_quads_remaining;
	int bug_present = 0;

	/* Only want to apply fix to reads */
	is_read = !(ioread32(core_base + DM_DBG1) & (ap->port_no ?
			BIT(CORE_PORT1_DATA_DIR_BIT) :
				BIT(CORE_PORT0_DATA_DIR_BIT)));

	/* Check for an incomplete transfer, i.e. not a multiple of 512 bytes
	   transferred (datacount_port register counts quads transferred) */
	quads_transferred =
		ioread32(core_base + (ap->port_no ?
					DATACOUNT_PORT1 : DATACOUNT_PORT0));

	remainder = quads_transferred & 0x7f;
	sector_quads_remaining = remainder ? (0x80 - remainder) : 0;

	if (is_read && (sector_quads_remaining == 2)) {
		bug_present = 1;
	} else if (sector_quads_remaining) {
		if (is_read) {
			ata_port_warn(ap, "SATA read fixup cannot deal with "
				"%d quads remaining\n",
				sector_quads_remaining);
		} else {
			ata_port_warn(ap, "SATA write fixup of %d quads "
				"remaining not supported\n",
				sector_quads_remaining);
		}
	}

	return bug_present;
}

/* This port done an interrupt */
static void sata_oxnas_port_irq(struct ata_port *ap, int force_error)
{
	struct ata_queued_cmd *qc;
	struct sata_oxnas_port_priv *pd = ap->private_data;
	void __iomem *port_base = pd->port_base;

	u32 int_status;
	unsigned long flags = 0;

	DPRINTK("ENTER port %d irqstatus %x\n", ap->port_no,
		ioread32(port_base + INT_STATUS));

	if (ap->qc_active & (1ULL << ATA_TAG_INTERNAL)) {
			qc = ata_qc_from_tag(ap, ATA_TAG_INTERNAL);
			DPRINTK("completing non-ncq cmd\n");

			if (qc)
				ata_qc_complete(qc);

			return;
	}

	qc = ata_qc_from_tag(ap, ap->link.active_tag);


	/* record the port's interrupt */
	int_status = ioread32(port_base + INT_STATUS);

	/* If there's no command associated with this IRQ, ignore it. We may get
	 * spurious interrupts when cleaning-up after a failed command, ignore
	 * these too. */
	if (likely(qc)) {
		/* get the status before any error cleanup */
		qc->err_mask = ac_err_mask(sata_oxnas_check_status(ap));
		if (force_error) {
			/* Pretend there has been a link error */
			qc->err_mask |= AC_ERR_ATA_BUS;
			DPRINTK(" ####force error####\n");
		}
		/* tell libata we're done */
		local_irq_save(flags);
		sata_oxnas_irq_clear(ap);
		local_irq_restore(flags);
		ata_qc_complete(qc);
	} else {
		VPRINTK("Ignoring interrupt, can't find the command tag="
			"%d %08x\n", ap->link.active_tag, ap->qc_active);
	}

	/* maybe a hotplug event */
	if (unlikely(int_status & INT_LINK_SERROR)) {
		u32 serror;

		sata_oxnas_scr_read_port(ap, SCR_ERROR, &serror);
		if (serror & (SERR_DEV_XCHG | SERR_PHYRDY_CHG)) {
			ata_ehi_hotplugged(&ap->link.eh_info);
			ata_port_freeze(ap);
		}
	}
}

/**
 * irq_handler is the interrupt handling routine registered with the system,
 * by libata.
 */
static irqreturn_t sata_oxnas_interrupt(int irq, void *dev_instance)
{
	struct ata_host *ah = dev_instance;
	struct sata_oxnas_host_priv *hd = ah->private_data;
	void __iomem *core_base = hd->core_base;

	u32 int_status;
	irqreturn_t ret = IRQ_NONE;
	u32 port_no;
	u32 mask;
	int bug_present;

	/* loop until there are no more interrupts */
	while ((int_status = (ioread32(core_base + CORE_INT_STATUS)) &
		(COREINT_END | (COREINT_END << 1)))) {

		/* clear any interrupt */
		iowrite32(int_status, core_base + CORE_INT_CLEAR);

		/* Only need workaround_bug_6320 for single disk systems as dual
		 * disk will use uCode which prevents this read underrun problem
		 * from occurring.
		 * All single disk systems will use port 0 */
		for (port_no = 0; port_no < hd->n_ports; ++port_no) {
			/* check the raw end of command interrupt to see if the
			 * port is done */
			mask = (COREINT_END << port_no);
			if (!(int_status & mask))
				continue;

			/* this port had an interrupt, clear it */
			iowrite32(mask, core_base + CORE_INT_CLEAR);
			/* check for bug 6320 only if no microcode was loaded */
			bug_present = (hd->current_ucode == UNKNOWN_MODE) &&
				sata_oxnas_bug_6320_detect(ah->ports[port_no]);

			sata_oxnas_port_irq(ah->ports[port_no],
						bug_present);
			ret = IRQ_HANDLED;
		}
	}

	return ret;
}

/*
 * scsi mid-layer and libata interface structures
 */
static struct scsi_host_template sata_oxnas_sht = {
	ATA_NCQ_SHT("sata_oxnas"),
	.can_queue = SATA_OXNAS_QUEUE_DEPTH,
	.sg_tablesize = SATA_OXNAS_MAX_PRD,
	.dma_boundary = ATA_DMA_BOUNDARY,
	.unchecked_isa_dma  = 0,
};


static struct ata_port_operations sata_oxnas_ops = {
	.inherits = &sata_port_ops,
	.qc_prep = sata_oxnas_qc_prep,
	.qc_issue = sata_oxnas_qc_issue,
	.qc_fill_rtf = sata_oxnas_qc_fill_rtf,
	.qc_new = sata_oxnas_qc_new,
	.qc_free = sata_oxnas_qc_free,

	.scr_read = sata_oxnas_scr_read,
	.scr_write = sata_oxnas_scr_write,

	.freeze = sata_oxnas_freeze,
	.thaw = sata_oxnas_thaw,
	.softreset = sata_oxnas_softreset,
	/* .hardreset = sata_oxnas_hardreset, */
	.postreset = sata_oxnas_postreset,
	.error_handler = sata_oxnas_error_handler,
	.post_internal_cmd = sata_oxnas_post_internal_cmd,

	.port_start = sata_oxnas_port_start,
	.port_stop = sata_oxnas_port_stop,

	.host_stop = sata_oxnas_host_stop,
	/* .pmp_attach = sata_oxnas_pmp_attach, */
	/* .pmp_detach = sata_oxnas_pmp_detach, */
	.sff_check_status = sata_oxnas_check_status,
	.acquire_hw = sata_oxnas_acquire_hw,
};

static const struct ata_port_info sata_oxnas_port_info = {
	.flags = SATA_OXNAS_HOST_FLAGS,
	.pio_mask = ATA_PIO4,
	.udma_mask = ATA_UDMA6,
	.port_ops = &sata_oxnas_ops,
};

static int sata_oxnas_probe(struct platform_device *ofdev)
{
	int retval = -ENXIO;
	int n_ports = 0;
	void __iomem *port_base = NULL;
	void __iomem *dmactl_base = NULL;
	void __iomem *sgdma_base = NULL;
	void __iomem *core_base = NULL;
	void __iomem *phy_base = NULL;
	struct reset_control *rstc;

	struct resource res = {};
	struct sata_oxnas_host_priv *host_priv = NULL;
	int irq = 0;
	struct ata_host *host = NULL;
	struct clk *clk = NULL;

	const struct ata_port_info *ppi[] = { &sata_oxnas_port_info, NULL };

	of_property_read_u32(ofdev->dev.of_node, "nr-ports", &n_ports);
	if (n_ports < 1 || n_ports > SATA_OXNAS_MAX_PORTS)
		goto error_exit_with_cleanup;

	port_base = of_iomap(ofdev->dev.of_node, 0);
	if (!port_base)
		goto error_exit_with_cleanup;

	dmactl_base = of_iomap(ofdev->dev.of_node, 1);
	if (!dmactl_base)
		goto error_exit_with_cleanup;

	sgdma_base = of_iomap(ofdev->dev.of_node, 2);
	if (!sgdma_base)
		goto error_exit_with_cleanup;

	core_base = of_iomap(ofdev->dev.of_node, 3);
	if (!core_base)
		goto error_exit_with_cleanup;

	phy_base = of_iomap(ofdev->dev.of_node, 4);
	if (!phy_base)
		goto error_exit_with_cleanup;

	host_priv = devm_kzalloc(&ofdev->dev,
					sizeof(struct sata_oxnas_host_priv),
					GFP_KERNEL);
	if (!host_priv)
		goto error_exit_with_cleanup;

	host_priv->port_base = port_base;
	host_priv->dmactl_base = dmactl_base;
	host_priv->sgdma_base = sgdma_base;
	host_priv->core_base = core_base;
	host_priv->phy_base = phy_base;
	host_priv->n_ports = n_ports;
	host_priv->current_ucode = UNKNOWN_MODE;

	if (!of_address_to_resource(ofdev->dev.of_node, 5, &res)) {
		host_priv->dma_base = res.start;
		host_priv->dma_size = resource_size(&res);
	}

	irq = irq_of_parse_and_map(ofdev->dev.of_node, 0);
	if (!irq) {
		dev_err(&ofdev->dev, "invalid irq from platform\n");
		goto error_exit_with_cleanup;
	}
	host_priv->irq = irq;

	clk = of_clk_get(ofdev->dev.of_node, 0);
	if (IS_ERR(clk)) {
		retval = PTR_ERR(clk);
		clk = NULL;
		goto error_exit_with_cleanup;
	}
	host_priv->clk = clk;

	rstc = devm_reset_control_get(&ofdev->dev, "sata");
	if (IS_ERR(rstc)) {
		retval = PTR_ERR(rstc);
		goto error_exit_with_cleanup;
	}
	host_priv->rst_sata = rstc;

	rstc = devm_reset_control_get(&ofdev->dev, "link");
	if (IS_ERR(rstc)) {
		retval = PTR_ERR(rstc);
		goto error_exit_with_cleanup;
	}
	host_priv->rst_link = rstc;

	rstc = devm_reset_control_get(&ofdev->dev, "phy");
	if (IS_ERR(rstc)) {
		retval = PTR_ERR(rstc);
		goto error_exit_with_cleanup;
	}
	host_priv->rst_phy = rstc;

	/* allocate host structure */
	host = ata_host_alloc_pinfo(&ofdev->dev, ppi, n_ports);

	if (!host) {
		retval = -ENOMEM;
		goto error_exit_with_cleanup;
	}
	host->private_data = host_priv;
	host->iomap = port_base;

	/* initialize core locking and queues */
	init_waitqueue_head(&host_priv->fast_wait_queue);
	init_waitqueue_head(&host_priv->scsi_wait_queue);
	spin_lock_init(&host_priv->phy_lock);
	spin_lock_init(&host_priv->core_lock);
	host_priv->core_locked = 0;
	host_priv->reentrant_port_no = -1;
	host_priv->hw_lock_count = 0;
	host_priv->direct_lock_count = 0;
	host_priv->locker_uid = 0;
	host_priv->current_locker_type = SATA_UNLOCKED;
	host_priv->isr_arg = NULL;
	host_priv->isr_callback = NULL;

	/* initialize host controller */
	retval = sata_oxnas_init_controller(host);
	if (retval)
		goto error_exit_with_cleanup;

	/*
	 * Now, register with libATA core, this will also initiate the
	 * device discovery process, invoking our port_start() handler &
	 * error_handler() to execute a dummy softreset EH session
	 */
	ata_host_activate(host, irq, sata_oxnas_interrupt, SATA_OXNAS_IRQ_FLAG,
			  &sata_oxnas_sht);

	return 0;

error_exit_with_cleanup:
	if (irq)
		irq_dispose_mapping(host_priv->irq);
	if (clk)
		clk_put(clk);
	if (host)
		ata_host_detach(host);
	if (port_base)
		iounmap(port_base);
	if (sgdma_base)
		iounmap(sgdma_base);
	if (core_base)
		iounmap(core_base);
	if (phy_base)
		iounmap(phy_base);
	return retval;
}


static int sata_oxnas_remove(struct platform_device *ofdev)
{
	struct ata_host *host = dev_get_drvdata(&ofdev->dev);
	struct sata_oxnas_host_priv *host_priv = host->private_data;

	ata_host_detach(host);

	irq_dispose_mapping(host_priv->irq);
	iounmap(host_priv->port_base);
	iounmap(host_priv->sgdma_base);
	iounmap(host_priv->core_base);

	/* reset Controller, Link and PHY */
	reset_control_assert(host_priv->rst_sata);
	reset_control_assert(host_priv->rst_link);
	reset_control_assert(host_priv->rst_phy);

	/* Disable the clock to the SATA block */
	clk_disable_unprepare(host_priv->clk);
	clk_put(host_priv->clk);

	return 0;
}

#ifdef CONFIG_PM
static int sata_oxnas_suspend(struct platform_device *op, pm_message_t state)
{
	struct ata_host *host = dev_get_drvdata(&op->dev);

	return ata_host_suspend(host, state);
}

static int sata_oxnas_resume(struct platform_device *op)
{
	struct ata_host *host = dev_get_drvdata(&op->dev);
	int ret;

	ret = sata_oxnas_init_controller(host);
	if (ret) {
		dev_err(&op->dev, "Error initializing hardware\n");
		return ret;
	}
	ata_host_resume(host);
	return 0;
}
#endif



static struct of_device_id oxnas_sata_match[] = {
	{
		.compatible = "plxtech,nas782x-sata",
	},
	{},
};

MODULE_DEVICE_TABLE(of, oxnas_sata_match);

static struct platform_driver oxnas_sata_driver = {
	.driver = {
		.name = "oxnas-sata",
		.owner = THIS_MODULE,
		.of_match_table = oxnas_sata_match,
	},
	.probe		= sata_oxnas_probe,
	.remove		= sata_oxnas_remove,
#ifdef CONFIG_PM
	.suspend	= sata_oxnas_suspend,
	.resume		= sata_oxnas_resume,
#endif
};

module_platform_driver(oxnas_sata_driver);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Oxford Semiconductor Ltd.");
MODULE_DESCRIPTION("low-level driver for Oxford 934 SATA core");
