/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium I2C adapter driver.
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/i2c.h>
#include <linux/pm_qos.h>
#include <linux/i2c-smbus.h>

#define IC_DEFAULT_FUNCTIONALITY (I2C_FUNC_I2C |		\
				  I2C_FUNC_SMBUS_BYTE |		\
				  I2C_FUNC_SMBUS_BYTE_DATA |	\
				  I2C_FUNC_SMBUS_WORD_DATA |	\
				  I2C_FUNC_SMBUS_BLOCK_DATA |	\
				  I2C_FUNC_SMBUS_I2C_BLOCK)

#define IC_CON_MASTER			0x1
#define IC_CON_SPEED_STD		0x2
#define IC_CON_SPEED_FAST		0x4
#define IC_CON_SPEED_HIGH		0x6
#define IC_CON_SPEED_MASK		0x6
#define IC_CON_10BITADDR_SLAVE		0x8
#define IC_CON_10BITADDR_MASTER		0x10
#define IC_CON_RESTART_EN		0x20
#define IC_CON_SLAVE_DISABLE		0x40
#define IC_CON_STOP_DET_IFADDRESSED	0x80
#define IC_CON_TX_EMPTY_CTRL		0x100
#define IC_CON_RX_FIFO_FULL_HLD_CTRL	0x200

#define IC_CON				0x0
#define IC_TAR				0x4
#define IC_SAR				0x8
#define IC_DATA_CMD			0x10
#define IC_SS_SCL_HCNT			0x14
#define IC_SS_SCL_LCNT			0x18
#define IC_FS_SCL_HCNT			0x1c
#define IC_FS_SCL_LCNT			0x20
#define IC_HS_SCL_HCNT			0x24
#define IC_HS_SCL_LCNT			0x28
#define IC_INTR_STAT			0x2c
#define IC_INTR_MASK			0x30
#define IC_RAW_INTR_STAT		0x34
#define IC_RX_TL			0x38
#define IC_TX_TL			0x3c
#define IC_CLR_INTR			0x40
#define IC_CLR_RX_UNDER			0x44
#define IC_CLR_RX_OVER			0x48
#define IC_CLR_TX_OVER			0x4c
#define IC_CLR_RD_REQ			0x50
#define IC_CLR_TX_ABRT			0x54
#define IC_CLR_RX_DONE			0x58
#define IC_CLR_ACTIVITY			0x5c
#define IC_CLR_STOP_DET			0x60
#define IC_CLR_START_DET		0x64
#define IC_CLR_GEN_CALL			0x68
#define IC_ENABLE			0x6c
#define IC_STATUS			0x70
#define IC_TXFLR			0x74
#define IC_RXFLR			0x78
#define IC_SDA_HOLD			0x7c
#define IC_TX_ABRT_SOURCE		0x80
#define IC_ENABLE_STATUS		0x9c
#define IC_SMBCLK_LOW_MEXT		0xa8
#define IC_SMBCLK_LOW_TIMEOUT		0xac
#define IC_SMBDAT_STUCK_TIMEOUT		0xb4
#define IC_CLR_SMBCLK_EXT_LOW_TIMEOUT	0xbc
#define IC_CLR_SMBCLK_TMO_LOW_TIMEOUT	0xc0
#define IC_CLR_SMBDAT_LOW_TIMEOUT	0xc4
#define IC_CLR_SMBALERT_IN_N		0xd0

#define IC_INTR_RX_UNDER		0x001
#define IC_INTR_RX_OVER			0x002
#define IC_INTR_RX_FULL			0x004
#define IC_INTR_TX_OVER			0x008
#define IC_INTR_TX_EMPTY		0x010
#define IC_INTR_RD_REQ			0x020
#define IC_INTR_TX_ABRT			0x040
#define IC_INTR_RX_DONE			0x080
#define IC_INTR_ACTIVITY		0x100
#define IC_INTR_STOP_DET		0x200
#define IC_INTR_START_DET		0x400
#define IC_INTR_GEN_CALL		0x800
#define IC_INTR_SMBCLK_EXT_LOW_TIMEOUT	0x1000
#define IC_INTR_SMBCLK_TMO_LOW_TIMEOUT	0x2000
#define IC_INTR_SMBSDA_LOW_TIMEOUT	0x4000
#define IC_INTR_SMBALERT_IN_N		0x20000

#define IC_INTR_DEFAULT_MASK		(IC_INTR_RX_FULL | \
					 IC_INTR_TX_ABRT | \
					 IC_INTR_STOP_DET)
#define IC_INTR_MASTER_MASK		(IC_INTR_DEFAULT_MASK | \
					 IC_INTR_TX_EMPTY)
#define IC_INTR_SLAVE_MASK		(IC_INTR_DEFAULT_MASK | \
					 IC_INTR_RX_DONE | \
					 IC_INTR_RX_UNDER | \
					 IC_INTR_RD_REQ)
#define IC_INTR_SMBUS_MASK		(IC_INTR_MASTER_MASK | \
					 IC_INTR_SMBCLK_EXT_LOW_TIMEOUT | \
					 IC_INTR_SMBCLK_TMO_LOW_TIMEOUT | \
					 IC_INTR_SMBSDA_LOW_TIMEOUT)

#define IC_STATUS_ACTIVITY		0x1
#define IC_STATUS_TFE			BIT(2)
#define IC_STATUS_MASTER_ACTIVITY	BIT(5)
#define IC_STATUS_SLAVE_ACTIVITY	BIT(6)

#define IC_SDA_HOLD_RX_SHIFT		16
#define IC_SDA_HOLD_RX_MASK		GENMASK(23, IC_SDA_HOLD_RX_SHIFT)

#define IC_ERR_TX_ABRT			0x1

#define IC_TAR_10BITADDR_MASTER		BIT(12)

#define IC_COMP_PARAM_1_SPEED_MODE_HIGH	(BIT(2) | BIT(3))
#define IC_COMP_PARAM_1_SPEED_MODE_MASK	GENMASK(3, 2)

#define STATUS_IDLE			0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS		0x2

/*
 * operation modes
 */
#define PHYTIUM_IC_MASTER		0
#define PHYTIUM_IC_SLAVE		1

#define ABRT_7B_ADDR_NOACK		0
#define ABRT_10ADDR1_NOACK		1
#define ABRT_10ADDR2_NOACK		2
#define ABRT_TXDATA_NOACK		3
#define ABRT_GCALL_NOACK		4
#define ABRT_GCALL_READ			5
#define ABRT_SBYTE_ACKDET		7
#define ABRT_SBYTE_NORSTRT		9
#define ABRT_10B_RD_NORSTRT		10
#define ABRT_MASTER_DIS			11
#define ARB_LOST			12
#define ABRT_SLAVE_FLUSH_TXFIFO		13
#define ABRT_SLAVE_ARBLOST		14
#define ABRT_SLAVE_RD_INTX		15

#define IC_TX_ABRT_7B_ADDR_NOACK	(1UL << ABRT_7B_ADDR_NOACK)
#define IC_TX_ABRT_10ADDR1_NOACK	(1UL << ABRT_10ADDR1_NOACK)
#define IC_TX_ABRT_10ADDR2_NOACK	(1UL << ABRT_10ADDR2_NOACK)
#define IC_TX_ABRT_TXDATA_NOACK		(1UL << ABRT_TXDATA_NOACK)
#define IC_TX_ABRT_GCALL_NOACK		(1UL << ABRT_GCALL_NOACK)
#define IC_TX_ABRT_GCALL_READ		(1UL << ABRT_GCALL_READ)
#define IC_TX_ABRT_SBYTE_ACKDET		(1UL << ABRT_SBYTE_ACKDET)
#define IC_TX_ABRT_SBYTE_NORSTRT	(1UL << ABRT_SBYTE_NORSTRT)
#define IC_TX_ABRT_10B_RD_NORSTRT	(1UL << ABRT_10B_RD_NORSTRT)
#define IC_TX_ABRT_MASTER_DIS		(1UL << ABRT_MASTER_DIS)
#define IC_TX_ARB_LOST			(1UL << ARB_LOST)
#define IC_RX_ABRT_SLAVE_RD_INTX	(1UL << ABRT_SLAVE_RD_INTX)
#define IC_RX_ABRT_SLAVE_ARBLOST	(1UL << ABRT_SLAVE_ARBLOST)
#define IC_RX_ABRT_SLAVE_FLUSH_TXFIFO	(1UL << ABRT_SLAVE_FLUSH_TXFIFO)

#define IC_TX_ABRT_NOACK		(IC_TX_ABRT_7B_ADDR_NOACK | \
					 IC_TX_ABRT_10ADDR1_NOACK | \
					 IC_TX_ABRT_10ADDR2_NOACK | \
					 IC_TX_ABRT_TXDATA_NOACK | \
					 IC_TX_ABRT_GCALL_NOACK)
#define CONTROLLER_TYPE_IIC		0
#define CONTROLLER_TYPE_SMBUS		1

struct phytium_i2c_dev {
	struct device		*dev;
	void __iomem		*base;
	int			irq;
	u32			flags;
	struct completion	cmd_complete;
	struct clk		*clk;
	struct reset_control	*rst;
	int			mode;
	struct i2c_client	*slave;
	u32			(*get_clk_rate_khz)(struct phytium_i2c_dev *dev);

	struct i2c_adapter	adapter;
	struct i2c_client	*ara;
	struct i2c_smbus_alert_setup alert_data;

	struct phytium_pci_i2c *controller;

	unsigned int		status;
	int			cmd_err;
	u32			abort_source;

	struct i2c_msg		*msgs;
	int			msgs_num;
	int			msg_write_idx;
	int			msg_read_idx;
	int			msg_err;
	u32			tx_buf_len;
	u8			*tx_buf;
	u32			rx_buf_len;
	u8			*rx_buf;

	u32			master_cfg;
	u32			slave_cfg;
	u32			functionality;
	unsigned int		tx_fifo_depth;
	unsigned int		rx_fifo_depth;
	int			rx_outstanding;

	struct i2c_timings	timings;
	u32			sda_hold_time;
	u16			ss_hcnt;
	u16			ss_lcnt;
	u16			fs_hcnt;
	u16			fs_lcnt;
	u16			fp_hcnt;
	u16			fp_lcnt;
	u16			hs_hcnt;
	u16			hs_lcnt;

	bool			pm_disabled;
	void			(*disable)(struct phytium_i2c_dev *dev);
	void			(*disable_int)(struct phytium_i2c_dev *dev);
	int			(*init)(struct phytium_i2c_dev *dev);
};

#define ACCESS_INTR_MASK	0x00000004

#define DEFAULT_CLOCK_FREQUENCY	48000000

u32 phytium_readl(struct phytium_i2c_dev *dev, int offset);
void phytium_writel(struct phytium_i2c_dev *dev, u32 b, int offset);
unsigned long i2c_phytium_clk_rate(struct phytium_i2c_dev *dev);
int i2c_phytium_prepare_clk(struct phytium_i2c_dev *dev, bool prepare);
int i2c_phytium_wait_bus_not_busy(struct phytium_i2c_dev *dev);
int i2c_phytium_handle_tx_abort(struct phytium_i2c_dev *dev);
u32 i2c_phytium_func(struct i2c_adapter *adap);
void i2c_phytium_disable(struct phytium_i2c_dev *dev);
void i2c_phytium_disable_int(struct phytium_i2c_dev *dev);
int i2c_phytium_set_sda_hold(struct phytium_i2c_dev *dev);
u32 i2c_phytium_scl_hcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset);
u32 i2c_phytium_scl_lcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset);

static inline void __i2c_phytium_enable(struct phytium_i2c_dev *dev)
{
	phytium_writel(dev, 1, IC_ENABLE);
}

static inline void __i2c_phytium_disable_nowait(struct phytium_i2c_dev *dev)
{
	phytium_writel(dev, 0, IC_ENABLE);
}

void __i2c_phytium_disable(struct phytium_i2c_dev *dev);

extern int i2c_phytium_probe(struct phytium_i2c_dev *dev);

extern int i2c_phytium_probe_slave(struct phytium_i2c_dev *dev);
