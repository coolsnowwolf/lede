/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium SD Card Interface driver
 *
 * Copyright (c) 2019-2023, Phytium Technology Co.,Ltd.
 */

/*---------------------------------------------------------------------------*/
/* Common Definition                                                         */
/*---------------------------------------------------------------------------*/
#define MAX_BD_NUM           0x1000
#define SD_BLOCK_SIZE        512

/*---------------------------------------------------------------------------*/
/* Register Offset                                                           */
/*---------------------------------------------------------------------------*/
#define SDCI_CONTROLLER      0x00  /* controller config reg                  */
#define SDCI_ARGUMENT        0x04  /* argument reg                           */
#define SDCI_COMMAND         0x08  /* command reg                            */
#define SDCI_CLOCK_D         0x0C  /* clock divide reg                       */
#define SDCI_SOFTWARE        0x10  /* controller reset reg                   */
#define SDCI_POWER           0X14  /* POWRE CONTROL REG                      */
#define SDCI_TIMEOUT_CMD     0x18  /* cmd timeout config reg                 */
#define SDCI_TIMEOUT_DATA    0x1C  /* data timeout reg                       */
#define SDCI_NORMAL_ISER     0x20  /* normal ISR config reg                  */
#define SDCI_ERROR_ISER      0x24  /* erroe ISR config reg                   */
#define SDCI_BD_ISER         0x28  /* BD ISR config reg                      */
#define SDCI_CAPA            0x2C  /* BD ISR config reg                      */
#define SDCI_SD_DRV          0x30  /* SD card driving phase position reg     */
#define SDCI_SD_SAMP         0x34  /* SD card sampling phase position reg    */
#define SDCI_SD_SEN          0x38  /* SD card detection reg                  */
#define SDCI_HDS_AXI         0x3C  /* AXI boundary config reg                */
#define SDCI_BD_RX           0x40  /* BD rx addr reg                         */
#define SDCI_BD_TX           0x60  /* BD tx addr reg                         */
#define SDCI_BLK_CNT         0x80  /* r/w block num reg                      */
#define SDCI_NORMAL_ISR      0xC0  /* normal ISR status reg                  */
#define SDCI_ERROR_ISR       0xC4  /* error ISR status reg                   */
#define SDCI_BD_ISR          0xC8  /* BD ISR status reg                      */
#define SDCI_BD_STATUS       0xCC  /* BD descriptor status reg               */
#define SDCI_STATUS          0xD0  /* status reg                             */
#define SDCI_BLOCK           0xD4  /* block len reg                          */
#define SDCI_RESP0           0xE0  /* response reg0                          */
#define SDCI_RESP1           0xE4  /* response reg1                          */
#define SDCI_RESP2           0xE8  /* response reg2                          */
#define SDCI_RESP3           0XEC  /* response reg3                          */

/*---------------------------------------------------------------------------*/
/* Register Mask                                                             */
/*---------------------------------------------------------------------------*/
/* SDCI_CONTROLLER mask */
#define SDCI_CONTROLLER_ECRCWR		(0x1 << 0)	/* RW */
#define SDCI_CONTROLLER_ECRCRD		(0x1 << 1)	/* RW */
#define SDCI_CONTROLLER_RESEDE		(0x1 << 2)	/* RW */
#define SDCI_CONTROLLER_PERMDR		(0x3 << 8)	/* RW */
#define SDCI_CONTROLLER_PERMDX		(0x3 << 10)	/* RW */

/* SDCI_SOFTWARE mask */
#define SDCI_SOFTWARE_SRST		(0x1 << 0)	/* RW */
#define SDCI_SOFTWARE_SCRST		(0x1 << 1)	/* RW */
#define SDCI_SOFTWARE_BDRST		(0x1 << 2)	/* RW */
#define SDCI_SOFTWARE_CFCLF		(0x1 << 3)	/* RW */
#define SDCI_SOFTWARE_SDRST		(0x1 << 4)	/* RW */

/* SDCI_NORMAL_ISER mask */
#define SDCI_SDCI_NORMAL_ISER_ECC_EN	(0x1 << 0)	/* RW */
#define SDCI_SDCI_NORMAL_ISER_ECR	(0x1 << 1)	/* RW */
#define SDCI_SDCI_NORMAL_ISER_ECI	(0x1 << 2)	/* RW */
#define SDCI_SDCI_NORMAL_ISER_EEI_EN	(0x1 << 15)	/* RW */

/* SDCI_NORMAL_ISR mask */
#define SDCI_NORMAL_ISR_CC		(0x1 << 0)	/* R  */
#define SDCI_NORMAL_ISR_CR		(0x1 << 1)	/* R  */
#define SDCI_NORMAL_ISR_CI		(0x1 << 2)	/* R  */
#define SDCI_NORMAL_ISR_TIMEOUT		(0x1 << 3)	/* R  */
#define SDCI_NORMAL_ISR_EI		(0x1 << 15)	/* R  */

/* SDCI_ERROR_ISER mask */
#define SDCI_ERROR_ISER_ECTE_EN		(0x1 << 0)	/* RW */
#define SDCI_ERROR_ISR_CCRCE_EN		(0x1 << 1)	/* RW */
#define SDCI_ERROR_ISR_CIR_EN		(0x1 << 3)	/* RW */
#define SDCI_ERROR_ISR_CNR_EN		(0x1 << 4)	/* RW */
/* SDCI_ERROR_ISR mask */
#define SDCI_ERROR_ISR_CTE		(0x1 << 0)	/* R  */
#define SDCI_ERROR_ISR_CCRCE		(0x1 << 1)	/* R  */
#define SDCI_ERROR_ISR_CIR		(0x1 << 3)	/* R  */
#define SDCI_ERROR_ISR_CNR		(0x1 << 4)	/* R  */

/* SDCI_BD_ISER mask */
#define SDCI_BD_ISER_ETRS_EN		(0x1 << 8)	/* RW */
#define SDCI_BD_ISER_DATFRAX_EN		(0x1 << 7)	/* RW */

/* SDCI_BD_ISR mask */
#define SDCI_BD_ISR_TRS_W		(0x1 << 0)	/* R  */
#define SDCI_BD_ISR_TRS_R		(0x1 << 8)	/* R  */
#define SDCI_BD_ISR_EDTE		(0x1 << 3)	/* R  */
#define SDCI_BD_ISR_DAIS		(0x1 << 15)	/* R  */
#define SDCI_BD_ISR_DATFRAX		(0x1 << 7)	/* R  */

/* SDCI_HDS_AXI mask */
#define SDCI_HDS_AXI_AWDOMAIN		(0x1 << 0)	/* RW */
#define SDCI_HDS_AXI_ARDOMAIN		(0x1 << 12)	/* RW */
#define SDCI_HDS_AXI_AWCACHE		(0x6 << 24)	/* RW */
#define SDCI_HDS_AXI_ARCACHE		(0xB << 28)	/* RW */

/* SDCI_STATUS mask */
#define SDCI_STATUS_CMD_BUSY		(0x0 << 0)	/* R  */
#define SDCI_STATUS_CMD_READY		(0x1 << 0)	/* R  */
#define SDCI_STATUS_IDIE		(0x1 << 12)	/* R  */
#define SDCI_CARD_BUSY_IN_PRG		(0x1 << 20)	/* R D0 BUSY:0,IDLE:1 */

/* SDCI_STATUS */
#define SDCI_STATUS_CDSL		(0x1 << 19)	/* R  */

/*---------------------------------------------------------------------------*/
/* Register Value                                                            */
/*---------------------------------------------------------------------------*/
#define SDCI_SD_DRV_VALUE		0
#define SDCI_SD_SAMP_VALUE_MAX		50
#define SDCI_SD_SAMP_VALUE_MIN		0

#define SDCI_TIMEOUT_CMD_VALUE		0xFFFFFFFF
#define SDCI_TIMEOUT_DATA_VALUE		0xFFFFFFFF
#define SDCI_POWER_ON			1
#define SDCI_POWER_OFF			0

#define SDCI_CMD_TIMEOUT		10
#define SDCI_DAT_TIMEOUT		5000

#define SDCI_CMD_TYPE_ADTC		0x2

#define SDCI_F_MIN			400000
#define SDCI_F_MAX			25000000

#define SDCI_SEN_CREFR_VAL		(0x1 << 1)
#define SDCI_SEN_DEBNCE_VAL		(0xB << 8)

#define CARD_CURRENT_STATE		(0xF << 9)
#define CARD_PRG_STATE			(0x7 << 9)
#define CARD_TRAN_STATE		(0x4 << 9)

#define SDCI_CMD13_OK			1
#define SDCI_CMD13_FAILED		0

#define ERR_TIMEOUT			(0x1 << 0)
#define ERR_CARD_ABSENT		(0x1 << 1)
#define ERR_CMD_RESPONED		(0x1 << 2)

/*---------------------------------------------------------------------------*/
/*  Structure Type                                                           */
/*---------------------------------------------------------------------------*/
struct phytium_sdci_dma {
	struct scatterlist *sg;
	u32 *buf;
	dma_addr_t bd_addr;
	size_t bytes;
};

enum adtc_type {
	COMMOM_ADTC = 0,
	BLOCK_RW_ADTC,
};

struct phytium_sdci_host {
	struct device *dev;
	struct mmc_host *mmc;
	u32 caps;
	spinlock_t lock;

	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	int error;

	void __iomem *base;

	struct phytium_sdci_dma dma_rx;
	struct phytium_sdci_dma dma_tx;

	u32 *sg_virt_addr;
	enum adtc_type adtc_type;

	struct timer_list hotplug_timer;

	struct delayed_work req_timeout;
	u32 cmd_timeout;
	u32 data_timeout;

	int irq;
	int irq_err;
	int irq_bd;

	struct clk *src_clk;
	unsigned long clk_rate;
	unsigned long clk_div;
	unsigned long real_rate;

	u32 current_rca;
	bool is_multi_rw_only_one_blkcnt;
};

