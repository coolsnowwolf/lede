/*
 * (C) Copyright 2005
 * Oxford Semiconductor Ltd
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,`
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/clock.h>

/**
 * SATA related definitions
 */
#define ATA_PORT_CTL        0
#define ATA_PORT_FEATURE    1
#define ATA_PORT_NSECT      2
#define ATA_PORT_LBAL       3
#define ATA_PORT_LBAM       4
#define ATA_PORT_LBAH       5
#define ATA_PORT_DEVICE     6
#define ATA_PORT_COMMAND    7

/* The offsets to the SATA registers */
#define SATA_ORB1_OFF           0
#define SATA_ORB2_OFF           1
#define SATA_ORB3_OFF           2
#define SATA_ORB4_OFF           3
#define SATA_ORB5_OFF           4

#define SATA_FIS_ACCESS         11
#define SATA_INT_STATUS_OFF     12  /* Read only */
#define SATA_INT_CLR_OFF        12  /* Write only */
#define SATA_INT_ENABLE_OFF     13  /* Read only */
#define SATA_INT_ENABLE_SET_OFF 13  /* Write only */
#define SATA_INT_ENABLE_CLR_OFF 14  /* Write only */
#define SATA_VERSION_OFF        15
#define SATA_CONTROL_OFF        23
#define SATA_COMMAND_OFF        24
#define SATA_PORT_CONTROL_OFF   25
#define SATA_DRIVE_CONTROL_OFF  26

/* The offsets to the link registers that are access in an asynchronous manner */
#define SATA_LINK_DATA     28
#define SATA_LINK_RD_ADDR  29
#define SATA_LINK_WR_ADDR  30
#define SATA_LINK_CONTROL  31

/* SATA interrupt status register fields */
#define SATA_INT_STATUS_EOC_RAW_BIT     ( 0 + 16)
#define SATA_INT_STATUS_ERROR_BIT       ( 2 + 16)
#define SATA_INT_STATUS_EOADT_RAW_BIT   ( 1 + 16)

/* SATA core command register commands */
#define SATA_CMD_WRITE_TO_ORB_REGS              2
#define SATA_CMD_WRITE_TO_ORB_REGS_NO_COMMAND   4

#define SATA_CMD_BUSY_BIT 7

#define SATA_SCTL_CLR_ERR 0x00000316UL

#define SATA_LBAL_BIT    0
#define SATA_LBAM_BIT    8
#define SATA_LBAH_BIT    16
#define SATA_HOB_LBAH_BIT 24
#define SATA_DEVICE_BIT  24
#define SATA_NSECT_BIT   0
#define SATA_HOB_NSECT_BIT   8
#define SATA_LBA32_BIT       0
#define SATA_LBA40_BIT       8
#define SATA_FEATURE_BIT 16
#define SATA_COMMAND_BIT 24
#define SATA_CTL_BIT     24

/* ATA status (7) register field definitions */
#define ATA_STATUS_BSY_BIT     7
#define ATA_STATUS_DRDY_BIT    6
#define ATA_STATUS_DF_BIT      5
#define ATA_STATUS_DRQ_BIT     3
#define ATA_STATUS_ERR_BIT     0

/* ATA device (6) register field definitions */
#define ATA_DEVICE_FIXED_MASK 0xA0
#define ATA_DEVICE_DRV_BIT 4
#define ATA_DEVICE_DRV_NUM_BITS 1
#define ATA_DEVICE_LBA_BIT 6

/* ATA Command register initiated commands */
#define ATA_CMD_INIT    0x91
#define ATA_CMD_IDENT   0xEC

#define SATA_STD_ASYNC_REGS_OFF 0x20
#define SATA_SCR_STATUS      0
#define SATA_SCR_ERROR       1
#define SATA_SCR_CONTROL     2
#define SATA_SCR_ACTIVE      3
#define SATA_SCR_NOTIFICAION 4

#define SATA_BURST_BUF_FORCE_EOT_BIT        0
#define SATA_BURST_BUF_DATA_INJ_ENABLE_BIT  1
#define SATA_BURST_BUF_DIR_BIT              2
#define SATA_BURST_BUF_DATA_INJ_END_BIT     3
#define SATA_BURST_BUF_FIFO_DIS_BIT         4
#define SATA_BURST_BUF_DIS_DREQ_BIT         5
#define SATA_BURST_BUF_DREQ_BIT             6

#define SATA_OPCODE_MASK 0x3

#define SATA_DMA_CHANNEL 0

#define DMA_CTRL_STATUS      (0x0)
#define DMA_BASE_SRC_ADR     (0x4)
#define DMA_BASE_DST_ADR     (0x8)
#define DMA_BYTE_CNT         (0xC)
#define DMA_CURRENT_SRC_ADR  (0x10)
#define DMA_CURRENT_DST_ADR  (0x14)
#define DMA_CURRENT_BYTE_CNT (0x18)
#define DMA_INTR_ID          (0x1C)
#define DMA_INTR_CLEAR_REG   (DMA_CURRENT_SRC_ADR)

#define DMA_CALC_REG_ADR(channel, register) ((volatile u32*)(DMA_BASE + ((channel) << 5) + (register)))

#define DMA_CTRL_STATUS_FAIR_SHARE_ARB            (1 << 0)
#define DMA_CTRL_STATUS_IN_PROGRESS               (1 << 1)
#define DMA_CTRL_STATUS_SRC_DREQ_MASK             (0x0000003C)
#define DMA_CTRL_STATUS_SRC_DREQ_SHIFT            (2)
#define DMA_CTRL_STATUS_DEST_DREQ_MASK            (0x000003C0)
#define DMA_CTRL_STATUS_DEST_DREQ_SHIFT           (6)
#define DMA_CTRL_STATUS_INTR                      (1 << 10)
#define DMA_CTRL_STATUS_NXT_FREE                  (1 << 11)
#define DMA_CTRL_STATUS_RESET                     (1 << 12)
#define DMA_CTRL_STATUS_DIR_MASK                  (0x00006000)
#define DMA_CTRL_STATUS_DIR_SHIFT                 (13)
#define DMA_CTRL_STATUS_SRC_ADR_MODE              (1 << 15)
#define DMA_CTRL_STATUS_DEST_ADR_MODE             (1 << 16)
#define DMA_CTRL_STATUS_TRANSFER_MODE_A           (1 << 17)
#define DMA_CTRL_STATUS_TRANSFER_MODE_B           (1 << 18)
#define DMA_CTRL_STATUS_SRC_WIDTH_MASK            (0x00380000)
#define DMA_CTRL_STATUS_SRC_WIDTH_SHIFT           (19)
#define DMA_CTRL_STATUS_DEST_WIDTH_MASK           (0x01C00000)
#define DMA_CTRL_STATUS_DEST_WIDTH_SHIFT          (22)
#define DMA_CTRL_STATUS_PAUSE                     (1 << 25)
#define DMA_CTRL_STATUS_INTERRUPT_ENABLE          (1 << 26)
#define DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED      (1 << 27)
#define DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED (1 << 28)
#define DMA_CTRL_STATUS_STARVE_LOW_PRIORITY       (1 << 29)
#define DMA_CTRL_STATUS_INTR_CLEAR_ENABLE         (1 << 30)

#define DMA_BYTE_CNT_MASK        ((1 << 21) - 1)
#define DMA_BYTE_CNT_WR_EOT_MASK (1 << 30)
#define DMA_BYTE_CNT_RD_EOT_MASK (1 << 31)
#define DMA_BYTE_CNT_BURST_MASK  (1 << 28)

#define MAKE_FIELD(value, num_bits, bit_num) (((value) & ((1 << (num_bits)) - 1)) << (bit_num))

typedef enum oxnas_dma_mode {
	OXNAS_DMA_MODE_FIXED, OXNAS_DMA_MODE_INC
} oxnas_dma_mode_t;

typedef enum oxnas_dma_direction {
	OXNAS_DMA_TO_DEVICE, OXNAS_DMA_FROM_DEVICE
} oxnas_dma_direction_t;

/* The available buses to which the DMA controller is attached */
typedef enum oxnas_dma_transfer_bus {
	OXNAS_DMA_SIDE_A, OXNAS_DMA_SIDE_B
} oxnas_dma_transfer_bus_t;

/* Direction of data flow between the DMA controller's pair of interfaces */
typedef enum oxnas_dma_transfer_direction {
	OXNAS_DMA_A_TO_A, OXNAS_DMA_B_TO_A, OXNAS_DMA_A_TO_B, OXNAS_DMA_B_TO_B
} oxnas_dma_transfer_direction_t;

/* The available data widths */
typedef enum oxnas_dma_transfer_width {
	OXNAS_DMA_TRANSFER_WIDTH_8BITS,
	OXNAS_DMA_TRANSFER_WIDTH_16BITS,
	OXNAS_DMA_TRANSFER_WIDTH_32BITS
} oxnas_dma_transfer_width_t;

/* The mode of the DMA transfer */
typedef enum oxnas_dma_transfer_mode {
	OXNAS_DMA_TRANSFER_MODE_SINGLE, OXNAS_DMA_TRANSFER_MODE_BURST
} oxnas_dma_transfer_mode_t;

/* The available transfer targets */
typedef enum oxnas_dma_dreq {
	OXNAS_DMA_DREQ_SATA = 0, OXNAS_DMA_DREQ_MEMORY = 15
} oxnas_dma_dreq_t;

typedef struct oxnas_dma_device_settings {
	unsigned long address_;
	unsigned fifo_size_; // Chained transfers must take account of FIFO offset at end of previous transfer
	unsigned char dreq_;
	unsigned read_eot_ :1;
	unsigned read_final_eot_ :1;
	unsigned write_eot_ :1;
	unsigned write_final_eot_ :1;
	unsigned bus_ :1;
	unsigned width_ :2;
	unsigned transfer_mode_ :1;
	unsigned address_mode_ :1;
	unsigned address_really_fixed_ :1;
} oxnas_dma_device_settings_t;

static const int MAX_NO_ERROR_LOOPS = 100000; /* 1 second in units of 10uS */
static const int MAX_DMA_XFER_LOOPS = 300000; /* 30 seconds in units of 100uS */
static const int MAX_DMA_ABORT_LOOPS = 10000; /* 0.1 second in units of 10uS */
static const int MAX_SRC_READ_LOOPS = 10000; /* 0.1 second in units of 10uS */
static const int MAX_SRC_WRITE_LOOPS = 10000; /* 0.1 second in units of 10uS */
static const int MAX_NOT_BUSY_LOOPS = 10000; /* 1 second in units of 100uS */

/* The internal SATA drive on which we should attempt to find partitions */
static volatile u32* sata_regs_base[2] = { (volatile u32*) SATA_0_REGS_BASE,
		(volatile u32*) SATA_1_REGS_BASE,

};
static u32 wr_sata_orb1[2] = { 0, 0 };
static u32 wr_sata_orb2[2] = { 0, 0 };
static u32 wr_sata_orb3[2] = { 0, 0 };
static u32 wr_sata_orb4[2] = { 0, 0 };

#ifdef CONFIG_LBA48
/* need keeping a record of NSECT LBAL LBAM LBAH ide_outb values for lba48 support */
#define OUT_HISTORY_BASE	ATA_PORT_NSECT
#define OUT_HISTORY_MAX		ATA_PORT_LBAH
static unsigned char out_history[2][OUT_HISTORY_MAX - OUT_HISTORY_BASE + 1] = {};
#endif

static oxnas_dma_device_settings_t oxnas_sata_dma_settings = { .address_ =
	SATA_DATA_BASE, .fifo_size_ = 16, .dreq_ = OXNAS_DMA_DREQ_SATA,
		.read_eot_ = 0, .read_final_eot_ = 1, .write_eot_ = 0,
		.write_final_eot_ = 1, .bus_ = OXNAS_DMA_SIDE_B, .width_ =
			OXNAS_DMA_TRANSFER_WIDTH_32BITS, .transfer_mode_ =
			OXNAS_DMA_TRANSFER_MODE_BURST, .address_mode_ =
			OXNAS_DMA_MODE_FIXED, .address_really_fixed_ = 0 };

oxnas_dma_device_settings_t oxnas_ram_dma_settings = { .address_ = 0,
		.fifo_size_ = 0, .dreq_ = OXNAS_DMA_DREQ_MEMORY, .read_eot_ = 1,
		.read_final_eot_ = 1, .write_eot_ = 1, .write_final_eot_ = 1,
		.bus_ = OXNAS_DMA_SIDE_A, .width_ =
			OXNAS_DMA_TRANSFER_WIDTH_32BITS, .transfer_mode_ =
			OXNAS_DMA_TRANSFER_MODE_BURST, .address_mode_ =
			OXNAS_DMA_MODE_FIXED, .address_really_fixed_ = 1 };

static void xfer_wr_shadow_to_orbs(int device)
{
	*(sata_regs_base[device] + SATA_ORB1_OFF) = wr_sata_orb1[device];
	*(sata_regs_base[device] + SATA_ORB2_OFF) = wr_sata_orb2[device];
	*(sata_regs_base[device] + SATA_ORB3_OFF) = wr_sata_orb3[device];
	*(sata_regs_base[device] + SATA_ORB4_OFF) = wr_sata_orb4[device];
}

static inline void device_select(int device)
{
	/* master/slave has no meaning to SATA core */
}

static int disk_present[CONFIG_SYS_IDE_MAXDEVICE];

#include <ata.h>

unsigned char ide_inb(int device, int port)
{
	unsigned char val = 0;

	/* Only permit accesses to disks found to be present during ide_preinit() */
	if (!disk_present[device]) {
		return ATA_STAT_FAULT;
	}

	device_select(device);

	switch (port) {
	case ATA_PORT_CTL:
		val = (*(sata_regs_base[device] + SATA_ORB4_OFF)
			& (0xFFUL << SATA_CTL_BIT)) >> SATA_CTL_BIT;
		break;
	case ATA_PORT_FEATURE:
		val = (*(sata_regs_base[device] + SATA_ORB2_OFF)
			& (0xFFUL << SATA_FEATURE_BIT)) >> SATA_FEATURE_BIT;
		break;
	case ATA_PORT_NSECT:
		val = (*(sata_regs_base[device] + SATA_ORB2_OFF)
			& (0xFFUL << SATA_NSECT_BIT)) >> SATA_NSECT_BIT;
		break;
	case ATA_PORT_LBAL:
		val = (*(sata_regs_base[device] + SATA_ORB3_OFF)
			& (0xFFUL << SATA_LBAL_BIT)) >> SATA_LBAL_BIT;
		break;
	case ATA_PORT_LBAM:
		val = (*(sata_regs_base[device] + SATA_ORB3_OFF)
			& (0xFFUL << SATA_LBAM_BIT)) >> SATA_LBAM_BIT;
		break;
	case ATA_PORT_LBAH:
		val = (*(sata_regs_base[device] + SATA_ORB3_OFF)
			& (0xFFUL << SATA_LBAH_BIT)) >> SATA_LBAH_BIT;
		break;
	case ATA_PORT_DEVICE:
		val = (*(sata_regs_base[device] + SATA_ORB3_OFF)
			& (0xFFUL << SATA_HOB_LBAH_BIT)) >> SATA_HOB_LBAH_BIT;
		val |= (*(sata_regs_base[device] + SATA_ORB1_OFF)
			& (0xFFUL << SATA_DEVICE_BIT)) >> SATA_DEVICE_BIT;
		break;
	case ATA_PORT_COMMAND:
		val = (*(sata_regs_base[device] + SATA_ORB2_OFF)
			& (0xFFUL << SATA_COMMAND_BIT)) >> SATA_COMMAND_BIT;
		val |= ATA_STAT_DRQ;
		break;
	default:
		printf("ide_inb() Unknown port = %d\n", port);
		break;
	}

	//    printf("inb: %d:%01x => %02x\n", device, port, val);

	return val;
}

/**
 * Possible that ATA status will not become no-error, so must have timeout
 * @returns An int which is zero on error
 */
static inline int wait_no_error(int device)
{
	int status = 0;

	/* Check for ATA core error */
	if (*(sata_regs_base[device] + SATA_INT_STATUS_OFF)
		& (1 << SATA_INT_STATUS_ERROR_BIT)) {
		printf("wait_no_error() SATA core flagged error\n");
	} else {
		int loops = MAX_NO_ERROR_LOOPS;
		do {
			/* Check for ATA device error */
			if (!(ide_inb(device, ATA_PORT_COMMAND)
				& (1 << ATA_STATUS_ERR_BIT))) {
				status = 1;
				break;
			}
			udelay(10);
		} while (--loops);

		if (!loops) {
			printf("wait_no_error() Timed out of wait for SATA no-error condition\n");
		}
	}

	return status;
}

/**
 * Expect SATA command to always finish, perhaps with error
 * @returns An int which is zero on error
 */
static inline int wait_sata_command_not_busy(int device)
{
	/* Wait for data to be available */
	int status = 0;
	int loops = MAX_NOT_BUSY_LOOPS;
	do {
		if (!(*(sata_regs_base[device] + SATA_COMMAND_OFF)
			& (1 << SATA_CMD_BUSY_BIT))) {
			status = 1;
			break;
		}
		udelay(100);
	} while (--loops);

	if (!loops) {
		printf("wait_sata_command_not_busy() Timed out of wait for SATA command to finish\n");
	}

	return status;
}

void ide_outb(int device, int port, unsigned char val)
{
	typedef enum send_method {
		SEND_NONE, SEND_SIMPLE, SEND_CMD, SEND_CTL,
	} send_method_t;

	/* Only permit accesses to disks found to be present during ide_preinit() */
	if (!disk_present[device]) {
		return;
	}

	//    printf("outb: %d:%01x <= %02x\n", device, port, val);

	device_select(device);

#ifdef CONFIG_LBA48
	if (port >= OUT_HISTORY_BASE && port <= OUT_HISTORY_MAX) {
		out_history[0][port - OUT_HISTORY_BASE] =
			out_history[1][port - OUT_HISTORY_BASE];
		out_history[1][port - OUT_HISTORY_BASE] = val;
	}
#endif
	send_method_t send_regs = SEND_NONE;
	switch (port) {
	case ATA_PORT_CTL:
		wr_sata_orb4[device] &= ~(0xFFUL << SATA_CTL_BIT);
		wr_sata_orb4[device] |= (val << SATA_CTL_BIT);
		send_regs = SEND_CTL;
		break;
	case ATA_PORT_FEATURE:
		wr_sata_orb2[device] &= ~(0xFFUL << SATA_FEATURE_BIT);
		wr_sata_orb2[device] |= (val << SATA_FEATURE_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_NSECT:
		wr_sata_orb2[device] &= ~(0xFFUL << SATA_NSECT_BIT);
		wr_sata_orb2[device] |= (val << SATA_NSECT_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_LBAL:
		wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAL_BIT);
		wr_sata_orb3[device] |= (val << SATA_LBAL_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_LBAM:
		wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAM_BIT);
		wr_sata_orb3[device] |= (val << SATA_LBAM_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_LBAH:
		wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAH_BIT);
		wr_sata_orb3[device] |= (val << SATA_LBAH_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_DEVICE:
		wr_sata_orb1[device] &= ~(0xFFUL << SATA_DEVICE_BIT);
		wr_sata_orb1[device] |= (val << SATA_DEVICE_BIT);
		send_regs = SEND_SIMPLE;
		break;
	case ATA_PORT_COMMAND:
		wr_sata_orb2[device] &= ~(0xFFUL << SATA_COMMAND_BIT);
		wr_sata_orb2[device] |= (val << SATA_COMMAND_BIT);
		send_regs = SEND_CMD;
#ifdef CONFIG_LBA48
		if (val == ATA_CMD_READ_EXT || val == ATA_CMD_WRITE_EXT)
		{
			/* fill high bytes of LBA48 && NSECT */
			wr_sata_orb2[device] &= ~(0xFFUL << SATA_HOB_NSECT_BIT);
			wr_sata_orb2[device] |=
				(out_history[0][ATA_PORT_NSECT - OUT_HISTORY_BASE] << SATA_HOB_NSECT_BIT);

			wr_sata_orb3[device] &= ~(0xFFUL << SATA_HOB_LBAH_BIT);
			wr_sata_orb3[device] |=
				(out_history[0][ATA_PORT_LBAL - OUT_HISTORY_BASE] << SATA_HOB_LBAH_BIT);

			wr_sata_orb4[device] &= ~(0xFFUL << SATA_LBA32_BIT);
			wr_sata_orb4[device] |=
				(out_history[0][ATA_PORT_LBAM - OUT_HISTORY_BASE] << SATA_LBA32_BIT);

			wr_sata_orb4[device] &= ~(0xFFUL << SATA_LBA40_BIT);
			wr_sata_orb4[device] |=
				(out_history[0][ATA_PORT_LBAH - OUT_HISTORY_BASE] << SATA_LBA40_BIT);
		}
#endif
		break;
	default:
		printf("ide_outb() Unknown port = %d\n", port);
	}

	u32 command;
	switch (send_regs) {
	case SEND_CMD:
		wait_sata_command_not_busy(device);
		command = *(sata_regs_base[device] + SATA_COMMAND_OFF);
		command &= ~SATA_OPCODE_MASK;
		command |= SATA_CMD_WRITE_TO_ORB_REGS;
		xfer_wr_shadow_to_orbs(device);
		wait_sata_command_not_busy(device);
		*(sata_regs_base[device] + SATA_COMMAND_OFF) = command;
		if (!wait_no_error(device)) {
			printf("ide_outb() Wait for ATA no-error timed-out\n");
		}
		break;
	case SEND_CTL:
		wait_sata_command_not_busy(device);
		command = *(sata_regs_base[device] + SATA_COMMAND_OFF);
		command &= ~SATA_OPCODE_MASK;
		command |= SATA_CMD_WRITE_TO_ORB_REGS_NO_COMMAND;
		xfer_wr_shadow_to_orbs(device);
		wait_sata_command_not_busy(device);
		*(sata_regs_base[device] + SATA_COMMAND_OFF) = command;
		if (!wait_no_error(device)) {
			printf("ide_outb() Wait for ATA no-error timed-out\n");
		}
		break;
	default:
		break;
	}
}

static u32 encode_start(u32 ctrl_status)
{
	return ctrl_status & ~DMA_CTRL_STATUS_PAUSE;
}

/* start a paused DMA transfer in channel 0 of the SATA DMA core */
static void dma_start(void)
{
	unsigned int reg;
	reg = readl(SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
	reg = encode_start(reg);
	writel(reg, SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
}

static unsigned long encode_control_status(
	oxnas_dma_device_settings_t* src_settings,
	oxnas_dma_device_settings_t* dst_settings)
{
	unsigned long ctrl_status;
	oxnas_dma_transfer_direction_t direction;

	ctrl_status = DMA_CTRL_STATUS_PAUSE;                           // Paused
	ctrl_status |= DMA_CTRL_STATUS_FAIR_SHARE_ARB;          // High priority
	ctrl_status |= (src_settings->dreq_ << DMA_CTRL_STATUS_SRC_DREQ_SHIFT); // Dreq
	ctrl_status |= (dst_settings->dreq_ << DMA_CTRL_STATUS_DEST_DREQ_SHIFT); // Dreq
	ctrl_status &= ~DMA_CTRL_STATUS_RESET;                         // !RESET

	// Use new interrupt clearing register
	ctrl_status |= DMA_CTRL_STATUS_INTR_CLEAR_ENABLE;

	// Setup the transfer direction and burst/single mode for the two DMA busses
	if (src_settings->bus_ == OXNAS_DMA_SIDE_A) {
		// Set the burst/single mode for bus A based on src device's settings
		if (src_settings->transfer_mode_
			== OXNAS_DMA_TRANSFER_MODE_BURST) {
			ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_A;
		} else {
			ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_A;
		}

		if (dst_settings->bus_ == OXNAS_DMA_SIDE_A) {
			direction = OXNAS_DMA_A_TO_A;
		} else {
			direction = OXNAS_DMA_A_TO_B;

			// Set the burst/single mode for bus B based on dst device's settings
			if (dst_settings->transfer_mode_
				== OXNAS_DMA_TRANSFER_MODE_BURST) {
				ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_B;
			} else {
				ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_B;
			}
		}
	} else {
		// Set the burst/single mode for bus B based on src device's settings
		if (src_settings->transfer_mode_
			== OXNAS_DMA_TRANSFER_MODE_BURST) {
			ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_B;
		} else {
			ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_B;
		}

		if (dst_settings->bus_ == OXNAS_DMA_SIDE_A) {
			direction = OXNAS_DMA_B_TO_A;

			// Set the burst/single mode for bus A based on dst device's settings
			if (dst_settings->transfer_mode_
				== OXNAS_DMA_TRANSFER_MODE_BURST) {
				ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_A;
			} else {
				ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_A;
			}
		} else {
			direction = OXNAS_DMA_B_TO_B;
		}
	}
	ctrl_status |= (direction << DMA_CTRL_STATUS_DIR_SHIFT);

	// Setup source address mode fixed or increment
	if (src_settings->address_mode_ == OXNAS_DMA_MODE_FIXED) {
		// Fixed address
		ctrl_status &= ~(DMA_CTRL_STATUS_SRC_ADR_MODE);

		// Set up whether fixed address is _really_ fixed
		if (src_settings->address_really_fixed_) {
			ctrl_status |= DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
		} else {
			ctrl_status &= ~DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
		}
	} else {
		// Incrementing address
		ctrl_status |= DMA_CTRL_STATUS_SRC_ADR_MODE;
		ctrl_status &= ~DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
	}

	// Setup destination address mode fixed or increment
	if (dst_settings->address_mode_ == OXNAS_DMA_MODE_FIXED) {
		// Fixed address
		ctrl_status &= ~(DMA_CTRL_STATUS_DEST_ADR_MODE);

		// Set up whether fixed address is _really_ fixed
		if (dst_settings->address_really_fixed_) {
			ctrl_status |=
				DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
		} else {
			ctrl_status &=
				~DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
		}
	} else {
		// Incrementing address
		ctrl_status |= DMA_CTRL_STATUS_DEST_ADR_MODE;
		ctrl_status &= ~DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
	}

	// Set up the width of the transfers on the DMA buses
	ctrl_status |=
		(src_settings->width_ << DMA_CTRL_STATUS_SRC_WIDTH_SHIFT);
	ctrl_status |=
		(dst_settings->width_ << DMA_CTRL_STATUS_DEST_WIDTH_SHIFT);

	// Setup the priority arbitration scheme
	ctrl_status &= ~DMA_CTRL_STATUS_STARVE_LOW_PRIORITY; // !Starve low priority

	return ctrl_status;
}

static u32 encode_final_eot(oxnas_dma_device_settings_t* src_settings,
				oxnas_dma_device_settings_t* dst_settings,
				unsigned long length)
{
	// Write the length, with EOT configuration for a final transfer
	unsigned long encoded = length;
	if (dst_settings->write_final_eot_) {
		encoded |= DMA_BYTE_CNT_WR_EOT_MASK;
	} else {
		encoded &= ~DMA_BYTE_CNT_WR_EOT_MASK;
	}
	if (src_settings->read_final_eot_) {
		encoded |= DMA_BYTE_CNT_RD_EOT_MASK;
	} else {
		encoded &= ~DMA_BYTE_CNT_RD_EOT_MASK;
	}
	/*    if((src_settings->transfer_mode_) ||
	 (src_settings->transfer_mode_)) {
	 encoded |= DMA_BYTE_CNT_BURST_MASK;
	 } else {
	 encoded &= ~DMA_BYTE_CNT_BURST_MASK;
	 }*/
	return encoded;
}

static void dma_start_write(const ulong* buffer, int num_bytes)
{
	// Assemble complete memory settings
	oxnas_dma_device_settings_t mem_settings = oxnas_ram_dma_settings;
	mem_settings.address_ = (unsigned long) buffer;
	mem_settings.address_mode_ = OXNAS_DMA_MODE_INC;

	writel(encode_control_status(&mem_settings, &oxnas_sata_dma_settings),
		SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
	writel(mem_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_SRC_ADR);
	writel(oxnas_sata_dma_settings.address_,
		SATA_DMA_REGS_BASE + DMA_BASE_DST_ADR);
	writel(encode_final_eot(&mem_settings, &oxnas_sata_dma_settings,
				num_bytes),
		SATA_DMA_REGS_BASE + DMA_BYTE_CNT);

	dma_start();
}

static void dma_start_read(ulong* buffer, int num_bytes)
{
	// Assemble complete memory settings
	oxnas_dma_device_settings_t mem_settings = oxnas_ram_dma_settings;
	mem_settings.address_ = (unsigned long) buffer;
	mem_settings.address_mode_ = OXNAS_DMA_MODE_INC;

	writel(encode_control_status(&oxnas_sata_dma_settings, &mem_settings),
		SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
	writel(oxnas_sata_dma_settings.address_,
		SATA_DMA_REGS_BASE + DMA_BASE_SRC_ADR);
	writel(mem_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_DST_ADR);
	writel(encode_final_eot(&oxnas_sata_dma_settings, &mem_settings,
				num_bytes),
		SATA_DMA_REGS_BASE + DMA_BYTE_CNT);

	dma_start();
}

static inline int dma_busy(void)
{
	return readl(SATA_DMA_REGS_BASE + DMA_CTRL_STATUS)
		& DMA_CTRL_STATUS_IN_PROGRESS;
}

static int wait_dma_not_busy(int device)
{
	unsigned int cleanup_required = 0;

	/* Poll for DMA completion */
	int loops = MAX_DMA_XFER_LOOPS;
	do {
		if (!dma_busy()) {
			break;
		}
		udelay(100);
	} while (--loops);

	if (!loops) {
		printf("wait_dma_not_busy() Timed out of wait for DMA not busy\n");
		cleanup_required = 1;
	}

	if (cleanup_required) {
		/* Abort DMA to make sure it has finished. */
		unsigned int ctrl_status = readl(
			SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
		ctrl_status |= DMA_CTRL_STATUS_RESET;
		writel(ctrl_status, SATA_DMA_CHANNEL + DMA_CTRL_STATUS);

		// Wait for the channel to become idle - should be quick as should
		// finish after the next AHB single or burst transfer
		loops = MAX_DMA_ABORT_LOOPS;
		do {
			if (!dma_busy()) {
				break;
			}
			udelay(10);
		} while (--loops);

		if (!loops) {
			printf("wait_dma_not_busy() Timed out of wait for DMA channel abort\n");
		} else {
			/* Successfully cleanup the DMA channel */
			cleanup_required = 0;
		}

		// Deassert reset for the channel
		ctrl_status = readl(SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
		ctrl_status &= ~DMA_CTRL_STATUS_RESET;
		writel(ctrl_status, SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
	}

	return !cleanup_required;
}

/**
 * Possible that ATA status will not become not-busy, so must have timeout
 */
static unsigned int wait_not_busy(int device, unsigned long timeout_secs)
{
	int busy = 1;
	unsigned long loops = (timeout_secs * 1000) / 50;
	do {
		// Test the ATA status register BUSY flag
		if (!((*(sata_regs_base[device] + SATA_ORB2_OFF)
			>> SATA_COMMAND_BIT) & (1UL << ATA_STATUS_BSY_BIT))) {
			/* Not busy, so stop polling */
			busy = 0;
			break;
		}

		// Wait for 50mS before sampling ATA status register again
		udelay(50000);
	} while (--loops);

	return busy;
}

void ide_output_data(int device, const ulong *sect_buf, int words)
{
	/* Only permit accesses to disks found to be present during ide_preinit() */
	if (!disk_present[device]) {
		return;
	}

	/* Select the required internal SATA drive */
	device_select(device);

	/* Start the DMA channel sending data from the passed buffer to the SATA core */
	dma_start_write(sect_buf, words << 2);

	/* Don't know why we need this delay, but without it the wait for DMA not
	 busy times soemtimes out, e.g. when saving environment to second disk */
	udelay(1000);

	/* Wait for DMA to finish */
	if (!wait_dma_not_busy(device)) {
		printf("Timed out of wait for DMA channel for SATA device %d to have in-progress clear\n",
			device);
	}

	/* Sata core should finish after DMA */
	if (wait_not_busy(device, 30)) {
		printf("Timed out of wait for SATA device %d to have BUSY clear\n",
			device);
	}
	if (!wait_no_error(device)) {
		printf("oxnas_sata_output_data() Wait for ATA no-error timed-out\n");
	}
}


#define SATA_DM_DBG1			(SATA_HOST_REGS_BASE + 0)
#define SATA_DATACOUNT_PORT0		(SATA_HOST_REGS_BASE + 0x10)
#define SATA_DATACOUNT_PORT1		(SATA_HOST_REGS_BASE + 0x14)
#define SATA_DATA_MUX_RAM0		(SATA_HOST_REGS_BASE + 0x8000)
#define SATA_DATA_MUX_RAM1		(SATA_HOST_REGS_BASE + 0xA000)
/* Sata core debug1 register bits */
#define SATA_CORE_PORT0_DATA_DIR_BIT	20
#define SATA_CORE_PORT1_DATA_DIR_BIT	21
#define SATA_CORE_PORT0_DATA_DIR	(1 << SATA_CORE_PORT0_DATA_DIR_BIT)
#define SATA_CORE_PORT1_DATA_DIR	(1 << SATA_CORE_PORT1_DATA_DIR_BIT)

/**
 * Ref bug-6320
 *
 * This code is a work around for a DMA hardware bug that will repeat the
 * penultimate 8-bytes on some reads. This code will check that the amount
 * of data transferred is a multiple of 512 bytes, if not the in it will
 * fetch the correct data from a buffer in the SATA core and copy it into
 * memory.
 *
 */
static void sata_bug_6320_workaround(int port, ulong *candidate)
{
	int is_read;
	int quads_transferred;
	int remainder;
	int sector_quads_remaining;

	/* Only want to apply fix to reads */
	is_read = !(*((unsigned long*) SATA_DM_DBG1)
		& (port ? SATA_CORE_PORT1_DATA_DIR : SATA_CORE_PORT0_DATA_DIR));

	/* Check for an incomplete transfer, i.e. not a multiple of 512 bytes
	 transferred (datacount_port register counts quads transferred) */
	quads_transferred = *((unsigned long*) (
		port ? SATA_DATACOUNT_PORT1 : SATA_DATACOUNT_PORT0));

	remainder = quads_transferred & 0x7f;
	sector_quads_remaining = remainder ? (0x80 - remainder) : 0;

	if (is_read && (sector_quads_remaining == 2)) {
		debug("SATA read fixup, only transfered %d quads, "
			"sector_quads_remaining %d, port %d\n",
			quads_transferred, sector_quads_remaining, port);

		int total_len = ATA_SECT_SIZE;
		ulong *sata_data_ptr = (void*) (
			port ? SATA_DATA_MUX_RAM1 : SATA_DATA_MUX_RAM0)
			+ ((total_len - 8) % 2048);

		*candidate = *sata_data_ptr;
		*(candidate + 1) = *(sata_data_ptr + 1);
	}
}


void ide_input_data(int device, ulong *sect_buf, int words)
{
	/* Only permit accesses to disks found to be present during ide_preinit() */
	if (!disk_present[device]) {
		return;
	}

	/* Select the required internal SATA drive */
	device_select(device);

	/* Start the DMA channel receiving data from the SATA core into the passed buffer */
	dma_start_read(sect_buf, words << 2);

	/* Sata core should finish before DMA */
	if (wait_not_busy(device, 30)) {
		printf("Timed out of wait for SATA device %d to have BUSY clear\n",
			device);
	}
	if (!wait_no_error(device)) {
		printf("oxnas_sata_output_data() Wait for ATA no-error timed-out\n");
	}

	/* Wait for DMA to finish */
	if (!wait_dma_not_busy(device)) {
		printf("Timed out of wait for DMA channel for SATA device %d to have in-progress clear\n",
			device);
	}

	if (words == ATA_SECTORWORDS)
		sata_bug_6320_workaround(device, sect_buf + words - 2);
}

static u32 scr_read(int device, unsigned int sc_reg)
{
	/* Setup adr of required register. std regs start eight into async region */
	*(sata_regs_base[device] + SATA_LINK_RD_ADDR) = sc_reg
		* 4+ SATA_STD_ASYNC_REGS_OFF;

	/* Wait for data to be available */
	int loops = MAX_SRC_READ_LOOPS;
	do {
		if (*(sata_regs_base[device] + SATA_LINK_CONTROL) & 1UL) {
			break;
		}
		udelay(10);
	} while (--loops);

	if (!loops) {
		printf("scr_read() Timed out of wait for read completion\n");
	}

	/* Read the data from the async register */
	return *(sata_regs_base[device] + SATA_LINK_DATA);
}

static void scr_write(int device, unsigned int sc_reg, u32 val)
{
	/* Setup the data for the write */
	*(sata_regs_base[device] + SATA_LINK_DATA) = val;

	/* Setup adr of required register. std regs start eight into async region */
	*(sata_regs_base[device] + SATA_LINK_WR_ADDR) = sc_reg
		* 4+ SATA_STD_ASYNC_REGS_OFF;

	/* Wait for data to be written */
	int loops = MAX_SRC_WRITE_LOOPS;
	do {
		if (*(sata_regs_base[device] + SATA_LINK_CONTROL) & 1UL) {
			break;
		}
		udelay(10);
	} while (--loops);

	if (!loops) {
		printf("scr_write() Timed out of wait for write completion\n");
	}
}
extern void workaround5458(void);

#define PHY_LOOP_COUNT  25  /* Wait for upto 5 seconds for PHY to be found */
#define LOS_AND_TX_LVL   0x2988
#define TX_ATTEN         0x55629

static int phy_reset(int device)
{
	int phy_status = 0;
	int loops = 0;

	scr_write(device, (0x60 - SATA_STD_ASYNC_REGS_OFF) / 4, LOS_AND_TX_LVL);
	scr_write(device, (0x70 - SATA_STD_ASYNC_REGS_OFF) / 4, TX_ATTEN);

	/* limit it to Gen-1 SATA (1.5G) */
	scr_write(device, SATA_SCR_CONTROL, 0x311); /* Issue phy wake & core reset */
	scr_read(device, SATA_SCR_STATUS); /* Dummy read; flush */
	udelay(1000);
	scr_write(device, SATA_SCR_CONTROL, 0x310); /* Issue phy wake & clear core reset */

	/* Wait for upto 5 seconds for PHY to become ready */
	do {
		udelay(200000);
		if ((scr_read(device, SATA_SCR_STATUS) & 0xf) == 3) {
			scr_write(device, SATA_SCR_ERROR, ~0);
			phy_status = 1;
			break;
		}
		//printf("No SATA PHY found status:0x%x\n", scr_read(device, SATA_SCR_STATUS));
	} while (++loops < PHY_LOOP_COUNT);

	if (phy_status) {
		udelay(500000); /* wait half a second */
	}

	return phy_status;
}

#define FIS_LOOP_COUNT  25  /* Wait for upto 5 seconds for FIS to be received */
static int wait_FIS(int device)
{
	int status = 0;
	int loops = 0;

	do {
		udelay(200000);
		if (ide_inb(device, ATA_PORT_NSECT) > 0) {
			status = 1;
			break;
		}
	} while (++loops < FIS_LOOP_COUNT);

	return status;
}


#define SATA_PHY_ASIC_STAT  (0x44900000)
#define SATA_PHY_ASIC_DATA  (0x44900004)

/**
 * initialise functions and macros for ASIC implementation
 */
#define PH_GAIN         2
#define FR_GAIN         3
#define PH_GAIN_OFFSET  6
#define FR_GAIN_OFFSET  8
#define PH_GAIN_MASK  (0x3 << PH_GAIN_OFFSET)
#define FR_GAIN_MASK  (0x3 << FR_GAIN_OFFSET)
#define USE_INT_SETTING  (1<<5)

#define CR_READ_ENABLE  (1<<16)
#define CR_WRITE_ENABLE (1<<17)
#define CR_CAP_DATA     (1<<18)

static void wait_cr_ack(void)
{
	while ((readl(SATA_PHY_ASIC_STAT) >> 16) & 0x1f)
		/* wait for an ack bit to be set */;
}

static unsigned short read_cr(unsigned short address)
{
	writel(address, SATA_PHY_ASIC_STAT);
	wait_cr_ack();
	writel(CR_READ_ENABLE, SATA_PHY_ASIC_DATA);
	wait_cr_ack();
	return readl(SATA_PHY_ASIC_STAT);
}

static void write_cr(unsigned short data, unsigned short address)
{
	writel(address, SATA_PHY_ASIC_STAT);
	wait_cr_ack();
	writel((data | CR_CAP_DATA), SATA_PHY_ASIC_DATA);
	wait_cr_ack();
	writel(CR_WRITE_ENABLE, SATA_PHY_ASIC_DATA);
	wait_cr_ack();
	return;
}

void workaround5458(void)
{
	unsigned i;

	for (i = 0; i < 2; i++) {
		unsigned short rx_control = read_cr(0x201d + (i << 8));
		rx_control &= ~(PH_GAIN_MASK | FR_GAIN_MASK);
		rx_control |= PH_GAIN << PH_GAIN_OFFSET;
		rx_control |= FR_GAIN << FR_GAIN_OFFSET;
		rx_control |= USE_INT_SETTING;
		write_cr(rx_control, 0x201d + (i << 8));
	}
}

int ide_preinit(void)
{
	int num_disks_found = 0;

	/* Initialise records of which disks are present to all present */
	int i;
	for (i = 0; i < CONFIG_SYS_IDE_MAXDEVICE; i++) {
		disk_present[i] = 1;
	}

	/* Block reset SATA and DMA cores */
	reset_block(SYS_CTRL_RST_SATA, 1);
	reset_block(SYS_CTRL_RST_SATA_LINK, 1);
	reset_block(SYS_CTRL_RST_SATA_PHY, 1);
	reset_block(SYS_CTRL_RST_SGDMA, 1);

	/* Enable clocks to SATA and DMA cores */
	enable_clock(SYS_CTRL_CLK_SATA);
	enable_clock(SYS_CTRL_CLK_DMA);

	udelay(5000);
	reset_block(SYS_CTRL_RST_SATA_PHY, 0);
	udelay(50);
	reset_block(SYS_CTRL_RST_SATA, 0);
	reset_block(SYS_CTRL_RST_SATA_LINK, 0);
	udelay(50);
	reset_block(SYS_CTRL_RST_SGDMA, 0);
	udelay(100);
	/* Apply the Synopsis SATA PHY workarounds */
	workaround5458();
	udelay(10000);

	/* disable and clear core interrupts */
	*((unsigned long*) SATA_HOST_REGS_BASE + SATA_INT_ENABLE_CLR_OFF) =
		~0UL;
	*((unsigned long*) SATA_HOST_REGS_BASE + SATA_INT_CLR_OFF) = ~0UL;

	int device;
	for (device = 0; device < CONFIG_SYS_IDE_MAXDEVICE; device++) {
		int found = 0;
		int retries = 1;

		/* Disable SATA interrupts */
		*(sata_regs_base[device] + SATA_INT_ENABLE_CLR_OFF) = ~0UL;

		/* Clear any pending SATA interrupts */
		*(sata_regs_base[device] + SATA_INT_CLR_OFF) = ~0UL;

		do {
			/* clear sector count register for FIS detection */
			ide_outb(device, ATA_PORT_NSECT, 0);

			/* Get the PHY working */
			if (!phy_reset(device)) {
				printf("SATA PHY not ready for device %d\n",
					device);
				break;
			}

			if (!wait_FIS(device)) {
				printf("No FIS received from device %d\n",
					device);
			} else {
				if ((scr_read(device, SATA_SCR_STATUS) & 0xf)
					== 0x3) {
					if (wait_not_busy(device, 30)) {
						printf("Timed out of wait for SATA device %d to have BUSY clear\n",
							device);
					} else {
						++num_disks_found;
						found = 1;
					}
				} else {
					printf("No SATA device %d found, PHY status = 0x%08x\n",
						device,
						scr_read(
							device,
							SATA_SCR_STATUS));
				}
				break;
			}
		} while (retries--);

		/* Record whether disk is present, so won't attempt to access it later */
		disk_present[device] = found;
	}

	/* post disk detection clean-up */
	for (device = 0; device < CONFIG_SYS_IDE_MAXDEVICE; device++) {
		if (disk_present[device]) {
			/* set as ata-5 (28-bit) */
			*(sata_regs_base[device] + SATA_DRIVE_CONTROL_OFF) =
				0UL;

			/* clear phy/link errors */
			scr_write(device, SATA_SCR_ERROR, ~0);

			/* clear host errors */
			*(sata_regs_base[device] + SATA_CONTROL_OFF) |=
				SATA_SCTL_CLR_ERR;

			/* clear interrupt register as this clears the error bit in the IDE
			 status register */
			*(sata_regs_base[device] + SATA_INT_CLR_OFF) = ~0UL;
		}
	}

	return !num_disks_found;
}

