/*
 * SPI driver for the Vitesse VSC7385 ethernet switch
 *
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * Parts of this file are based on Atheros' 2.6.15 BSP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/bitops.h>
#include <linux/firmware.h>
#include <linux/spi/spi.h>
#include <linux/spi/vsc7385.h>

#define DRV_NAME	"spi-vsc7385"
#define DRV_DESC	"Vitesse VSC7385 Gbit ethernet switch driver"
#define DRV_VERSION	"0.1.0"

#define VSC73XX_BLOCK_MAC	0x1
#define VSC73XX_BLOCK_2		0x2
#define VSC73XX_BLOCK_MII	0x3
#define VSC73XX_BLOCK_4		0x4
#define VSC73XX_BLOCK_5		0x5
#define VSC73XX_BLOCK_SYSTEM	0x7

#define VSC73XX_SUBBLOCK_PORT_0		0
#define VSC73XX_SUBBLOCK_PORT_1		1
#define VSC73XX_SUBBLOCK_PORT_2		2
#define VSC73XX_SUBBLOCK_PORT_3		3
#define VSC73XX_SUBBLOCK_PORT_4		4
#define VSC73XX_SUBBLOCK_PORT_MAC	6

/* MAC Block registers */
#define VSC73XX_MAC_CFG		0x0
#define VSC73XX_ADVPORTM	0x19
#define VSC73XX_RXOCT		0x50
#define VSC73XX_TXOCT		0x51
#define VSC73XX_C_RX0		0x52
#define VSC73XX_C_RX1		0x53
#define VSC73XX_C_RX2		0x54
#define VSC73XX_C_TX0		0x55
#define VSC73XX_C_TX1		0x56
#define VSC73XX_C_TX2		0x57
#define VSC73XX_C_CFG		0x58

/* MAC_CFG register bits */
#define VSC73XX_MAC_CFG_WEXC_DIS	(1 << 31)
#define VSC73XX_MAC_CFG_PORT_RST	(1 << 29)
#define VSC73XX_MAC_CFG_TX_EN		(1 << 28)
#define VSC73XX_MAC_CFG_SEED_LOAD	(1 << 27)
#define VSC73XX_MAC_CFG_FDX		(1 << 18)
#define VSC73XX_MAC_CFG_GIGE		(1 << 17)
#define VSC73XX_MAC_CFG_RX_EN		(1 << 16)
#define VSC73XX_MAC_CFG_VLAN_DBLAWR	(1 << 15)
#define VSC73XX_MAC_CFG_VLAN_AWR	(1 << 14)
#define VSC73XX_MAC_CFG_100_BASE_T	(1 << 13)
#define VSC73XX_MAC_CFG_TX_IPG(x)	(((x) & 0x1f) << 6)
#define VSC73XX_MAC_CFG_MAC_RX_RST	(1 << 5)
#define VSC73XX_MAC_CFG_MAC_TX_RST	(1 << 4)
#define VSC73XX_MAC_CFG_BIT2		(1 << 2)
#define VSC73XX_MAC_CFG_CLK_SEL(x)	((x) & 0x3)

/* ADVPORTM register bits */
#define VSC73XX_ADVPORTM_IFG_PPM	(1 << 7)
#define VSC73XX_ADVPORTM_EXC_COL_CONT	(1 << 6)
#define VSC73XX_ADVPORTM_EXT_PORT	(1 << 5)
#define VSC73XX_ADVPORTM_INV_GTX	(1 << 4)
#define VSC73XX_ADVPORTM_ENA_GTX	(1 << 3)
#define VSC73XX_ADVPORTM_DDR_MODE	(1 << 2)
#define VSC73XX_ADVPORTM_IO_LOOPBACK	(1 << 1)
#define VSC73XX_ADVPORTM_HOST_LOOPBACK	(1 << 0)

/* MII Block registers */
#define VSC73XX_MII_STAT	0x0
#define VSC73XX_MII_CMD		0x1
#define VSC73XX_MII_DATA	0x2

/* System Block registers */
#define VSC73XX_ICPU_SIPAD		0x01
#define VSC73XX_ICPU_CLOCK_DELAY	0x05
#define VSC73XX_ICPU_CTRL		0x10
#define VSC73XX_ICPU_ADDR		0x11
#define VSC73XX_ICPU_SRAM		0x12
#define VSC73XX_ICPU_MBOX_VAL		0x15
#define VSC73XX_ICPU_MBOX_SET		0x16
#define VSC73XX_ICPU_MBOX_CLR		0x17
#define VSC73XX_ICPU_CHIPID		0x18
#define VSC73XX_ICPU_GPIO		0x34

#define VSC73XX_ICPU_CTRL_CLK_DIV	(1 << 8)
#define VSC73XX_ICPU_CTRL_SRST_HOLD	(1 << 7)
#define VSC73XX_ICPU_CTRL_BOOT_EN	(1 << 3)
#define VSC73XX_ICPU_CTRL_EXT_ACC_EN	(1 << 2)
#define VSC73XX_ICPU_CTRL_CLK_EN	(1 << 1)
#define VSC73XX_ICPU_CTRL_SRST		(1 << 0)

#define VSC73XX_ICPU_CHIPID_ID_SHIFT	12
#define VSC73XX_ICPU_CHIPID_ID_MASK	0xffff
#define VSC73XX_ICPU_CHIPID_REV_SHIFT	28
#define VSC73XX_ICPU_CHIPID_REV_MASK	0xf
#define VSC73XX_ICPU_CHIPID_ID_7385	0x7385
#define VSC73XX_ICPU_CHIPID_ID_7395	0x7395

#define VSC73XX_CMD_MODE_READ		0
#define VSC73XX_CMD_MODE_WRITE		1
#define VSC73XX_CMD_MODE_SHIFT		4
#define VSC73XX_CMD_BLOCK_SHIFT		5
#define VSC73XX_CMD_BLOCK_MASK		0x7
#define VSC73XX_CMD_SUBBLOCK_MASK	0xf

#define VSC7385_CLOCK_DELAY		((3 << 4) | 3)
#define VSC7385_CLOCK_DELAY_MASK	((3 << 4) | 3)

#define VSC73XX_ICPU_CTRL_STOP	(VSC73XX_ICPU_CTRL_SRST_HOLD | \
				 VSC73XX_ICPU_CTRL_BOOT_EN | \
				 VSC73XX_ICPU_CTRL_EXT_ACC_EN)

#define VSC73XX_ICPU_CTRL_START	(VSC73XX_ICPU_CTRL_CLK_DIV | \
				 VSC73XX_ICPU_CTRL_BOOT_EN | \
				 VSC73XX_ICPU_CTRL_CLK_EN | \
				 VSC73XX_ICPU_CTRL_SRST)

#define VSC7385_ADVPORTM_MASK	(VSC73XX_ADVPORTM_IFG_PPM | \
				 VSC73XX_ADVPORTM_EXC_COL_CONT | \
				 VSC73XX_ADVPORTM_EXT_PORT | \
				 VSC73XX_ADVPORTM_INV_GTX | \
				 VSC73XX_ADVPORTM_ENA_GTX | \
				 VSC73XX_ADVPORTM_DDR_MODE | \
				 VSC73XX_ADVPORTM_IO_LOOPBACK | \
				 VSC73XX_ADVPORTM_HOST_LOOPBACK)

#define VSC7385_ADVPORTM_INIT	(VSC73XX_ADVPORTM_EXT_PORT | \
				 VSC73XX_ADVPORTM_ENA_GTX | \
				 VSC73XX_ADVPORTM_DDR_MODE)

#define VSC7385_MAC_CFG_RESET	(VSC73XX_MAC_CFG_PORT_RST | \
				 VSC73XX_MAC_CFG_MAC_RX_RST | \
				 VSC73XX_MAC_CFG_MAC_TX_RST)

#define VSC73XX_MAC_CFG_INIT	(VSC73XX_MAC_CFG_TX_EN | \
				 VSC73XX_MAC_CFG_FDX | \
				 VSC73XX_MAC_CFG_GIGE | \
				 VSC73XX_MAC_CFG_RX_EN)

#define VSC73XX_RESET_DELAY	100

struct vsc7385 {
	struct spi_device		*spi;
	struct mutex			lock;
	struct vsc7385_platform_data	*pdata;
};

static int vsc7385_is_addr_valid(u8 block, u8 subblock)
{
	switch (block) {
	case VSC73XX_BLOCK_MAC:
		switch (subblock) {
		case 0 ... 4:
		case 6:
			return 1;
		}
		break;

	case VSC73XX_BLOCK_2:
	case VSC73XX_BLOCK_SYSTEM:
		switch (subblock) {
		case 0:
			return 1;
		}
		break;

	case VSC73XX_BLOCK_MII:
	case VSC73XX_BLOCK_4:
	case VSC73XX_BLOCK_5:
		switch (subblock) {
		case 0 ... 1:
			return 1;
		}
		break;
	}

	return 0;
}

static inline u8 vsc7385_make_addr(u8 mode, u8 block, u8 subblock)
{
	u8 ret;

	ret = (block & VSC73XX_CMD_BLOCK_MASK) << VSC73XX_CMD_BLOCK_SHIFT;
	ret |= (mode & 1) << VSC73XX_CMD_MODE_SHIFT;
	ret |= subblock & VSC73XX_CMD_SUBBLOCK_MASK;

	return ret;
}

static int vsc7385_read(struct vsc7385 *vsc, u8 block, u8 subblock, u8 reg,
			u32 *value)
{
	u8 cmd[4];
	u8 buf[4];
	struct spi_transfer t[2];
	struct spi_message m;
	int err;

	if (!vsc7385_is_addr_valid(block, subblock))
		return -EINVAL;

	spi_message_init(&m);

	memset(&t, 0, sizeof(t));

	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = sizeof(buf);
	spi_message_add_tail(&t[1], &m);

	cmd[0] = vsc7385_make_addr(VSC73XX_CMD_MODE_READ, block, subblock);
	cmd[1] = reg;
	cmd[2] = 0;
	cmd[3] = 0;

	mutex_lock(&vsc->lock);
	err = spi_sync(vsc->spi, &m);
	mutex_unlock(&vsc->lock);

	if (err)
		return err;

	*value = (((u32) buf[0]) << 24) | (((u32) buf[1]) << 16) |
		 (((u32) buf[2]) << 8) | ((u32) buf[3]);

	return 0;
}


static int vsc7385_write(struct vsc7385 *vsc, u8 block, u8 subblock, u8 reg,
			 u32 value)
{
	u8 cmd[2];
	u8 buf[4];
	struct spi_transfer t[2];
	struct spi_message m;
	int err;

	if (!vsc7385_is_addr_valid(block, subblock))
		return -EINVAL;

	spi_message_init(&m);

	memset(&t, 0, sizeof(t));

	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	t[1].len = sizeof(buf);
	spi_message_add_tail(&t[1], &m);

	cmd[0] = vsc7385_make_addr(VSC73XX_CMD_MODE_WRITE, block, subblock);
	cmd[1] = reg;

	buf[0] = (value >> 24) & 0xff;
	buf[1] = (value >> 16) & 0xff;
	buf[2] = (value >> 8) & 0xff;
	buf[3] = value & 0xff;

	mutex_lock(&vsc->lock);
	err = spi_sync(vsc->spi, &m);
	mutex_unlock(&vsc->lock);

	return err;
}

static inline int vsc7385_write_verify(struct vsc7385 *vsc, u8 block,
				       u8 subblock, u8 reg, u32 value,
				       u32 read_mask, u32 read_val)
{
	struct spi_device *spi = vsc->spi;
	u32 t;
	int err;

	err = vsc7385_write(vsc, block, subblock, reg, value);
	if (err)
		return err;

	err = vsc7385_read(vsc, block, subblock, reg, &t);
	if (err)
		return err;

	if ((t & read_mask) != read_val) {
		dev_err(&spi->dev, "register write error\n");
		return -EIO;
	}

	return 0;
}

static inline int vsc7385_set_clock_delay(struct vsc7385 *vsc, u32 val)
{
	return vsc7385_write(vsc, VSC73XX_BLOCK_SYSTEM, 0,
			     VSC73XX_ICPU_CLOCK_DELAY, val);
}

static inline int vsc7385_get_clock_delay(struct vsc7385 *vsc, u32 *val)
{
	return vsc7385_read(vsc, VSC73XX_BLOCK_SYSTEM, 0,
			    VSC73XX_ICPU_CLOCK_DELAY, val);
}

static inline int vsc7385_icpu_stop(struct vsc7385 *vsc)
{
	return vsc7385_write(vsc, VSC73XX_BLOCK_SYSTEM, 0, VSC73XX_ICPU_CTRL,
			     VSC73XX_ICPU_CTRL_STOP);
}

static inline int vsc7385_icpu_start(struct vsc7385 *vsc)
{
	return vsc7385_write(vsc, VSC73XX_BLOCK_SYSTEM, 0, VSC73XX_ICPU_CTRL,
			     VSC73XX_ICPU_CTRL_START);
}

static inline int vsc7385_icpu_reset(struct vsc7385 *vsc)
{
	int rc;

	rc = vsc7385_write(vsc, VSC73XX_BLOCK_SYSTEM, 0, VSC73XX_ICPU_ADDR,
			   0x0000);
	if (rc)
		dev_err(&vsc->spi->dev,
			"could not reset microcode, err=%d\n", rc);

	return rc;
}

static int vsc7385_upload_ucode(struct vsc7385 *vsc)
{
	struct spi_device *spi = vsc->spi;
	const struct firmware *firmware;
	char *ucode_name;
	unsigned char *dp;
	unsigned int curVal;
	int i;
	int diffs;
	int rc;

	ucode_name = (vsc->pdata->ucode_name) ? vsc->pdata->ucode_name
					      : "vsc7385_ucode.bin";
	rc = request_firmware(&firmware, ucode_name, &spi->dev);
	if (rc) {
		dev_err(&spi->dev, "request_firmware failed, err=%d\n",
			rc);
		return rc;
	}

	rc = vsc7385_icpu_stop(vsc);
	if (rc)
		goto out;

	rc = vsc7385_icpu_reset(vsc);
	if (rc)
		goto out;

	dev_info(&spi->dev, "uploading microcode...\n");

	dp = (unsigned char *) firmware->data;
	for (i = 0; i < firmware->size; i++) {
		rc = vsc7385_write(vsc, VSC73XX_BLOCK_SYSTEM, 0,
				   VSC73XX_ICPU_SRAM, *dp++);
		if (rc) {
			dev_err(&spi->dev, "could not load microcode, err=%d\n",
				rc);
			goto out;
		}
	}

	rc = vsc7385_icpu_reset(vsc);
	if (rc)
		goto out;

	dev_info(&spi->dev, "verifying microcode...\n");

	dp = (unsigned char *) firmware->data;
	diffs = 0;
	for (i = 0; i < firmware->size; i++) {
		rc = vsc7385_read(vsc, VSC73XX_BLOCK_SYSTEM, 0,
				  VSC73XX_ICPU_SRAM, &curVal);
		if (rc) {
			dev_err(&spi->dev, "could not read microcode %d\n",
				rc);
			goto out;
		}

		if (curVal > 0xff) {
			dev_err(&spi->dev, "bad val read: %04x : %02x  %02x\n",
				i, *dp, curVal);
			rc = -EIO;
			goto out;
		}

		if ((curVal & 0xff) != *dp) {
			diffs++;
			dev_err(&spi->dev, "verify error: %04x : %02x  %02x\n",
				i, *dp, curVal);

			if (diffs > 4)
				break;
			}
		dp++;
	}

	if (diffs) {
		dev_err(&spi->dev, "microcode verification failed\n");
		rc = -EIO;
		goto out;
	}

	dev_info(&spi->dev, "microcode uploaded\n");

	rc = vsc7385_icpu_start(vsc);

out:
	release_firmware(firmware);
	return rc;
}

static int vsc7385_setup(struct vsc7385 *vsc)
{
	struct vsc7385_platform_data *pdata = vsc->pdata;
	u32 t;
	int err;

	err = vsc7385_write_verify(vsc, VSC73XX_BLOCK_SYSTEM, 0,
				   VSC73XX_ICPU_CLOCK_DELAY,
				   VSC7385_CLOCK_DELAY,
				   VSC7385_CLOCK_DELAY_MASK,
				   VSC7385_CLOCK_DELAY);
	if (err)
		goto err;

	err = vsc7385_write_verify(vsc, VSC73XX_BLOCK_MAC,
				   VSC73XX_SUBBLOCK_PORT_MAC, VSC73XX_ADVPORTM,
				   VSC7385_ADVPORTM_INIT,
				   VSC7385_ADVPORTM_MASK,
				   VSC7385_ADVPORTM_INIT);
	if (err)
		goto err;

	err = vsc7385_write(vsc, VSC73XX_BLOCK_MAC, VSC73XX_SUBBLOCK_PORT_MAC,
			    VSC73XX_MAC_CFG, VSC7385_MAC_CFG_RESET);
	if (err)
		goto err;

	t = VSC73XX_MAC_CFG_INIT;
	t |= VSC73XX_MAC_CFG_TX_IPG(pdata->mac_cfg.tx_ipg);
	t |= VSC73XX_MAC_CFG_CLK_SEL(pdata->mac_cfg.clk_sel);
	if (pdata->mac_cfg.bit2)
		t |= VSC73XX_MAC_CFG_BIT2;

	err = vsc7385_write(vsc, VSC73XX_BLOCK_MAC, VSC73XX_SUBBLOCK_PORT_MAC,
			    VSC73XX_MAC_CFG, t);
	if (err)
		goto err;

	return 0;

err:
	return err;
}

static int vsc7385_detect(struct vsc7385 *vsc)
{
	struct spi_device *spi = vsc->spi;
	u32 t;
	u32 id;
	u32 rev;
	int err;

	err = vsc7385_read(vsc, VSC73XX_BLOCK_SYSTEM, 0,
				VSC73XX_ICPU_MBOX_VAL, &t);
	if (err) {
		dev_err(&spi->dev, "unable to read mailbox, err=%d\n", err);
		return err;
	}

	if (t == 0xffffffff) {
		dev_dbg(&spi->dev, "assert chip reset\n");
		if (vsc->pdata->reset)
			vsc->pdata->reset();

	}

	err = vsc7385_read(vsc, VSC73XX_BLOCK_SYSTEM, 0,
				VSC73XX_ICPU_CHIPID, &t);
	if (err) {
		dev_err(&spi->dev, "unable to read chip id, err=%d\n", err);
		return err;
	}

	id = (t >> VSC73XX_ICPU_CHIPID_ID_SHIFT) & VSC73XX_ICPU_CHIPID_ID_MASK;
	switch (id) {
	case VSC73XX_ICPU_CHIPID_ID_7385:
	case VSC73XX_ICPU_CHIPID_ID_7395:
		break;
	default:
		dev_err(&spi->dev, "unsupported chip, id=%04x\n", id);
		return -ENODEV;
	}

	rev = (t >> VSC73XX_ICPU_CHIPID_REV_SHIFT) &
	      VSC73XX_ICPU_CHIPID_REV_MASK;
	dev_info(&spi->dev, "VSC%04X (rev. %d) switch found\n", id, rev);

	return 0;
}

static int vsc7385_probe(struct spi_device *spi)
{
	struct vsc7385 *vsc;
	struct vsc7385_platform_data *pdata;
	int	err;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION"\n");

	pdata = spi->dev.platform_data;
	if (!pdata) {
		dev_err(&spi->dev, "no platform data specified\n");
		return -ENODEV;
	}

	vsc = kzalloc(sizeof(*vsc), GFP_KERNEL);
	if (!vsc) {
		dev_err(&spi->dev, "no memory for private data\n");
		return -ENOMEM;
	}

	mutex_init(&vsc->lock);
	vsc->pdata = pdata;
	vsc->spi = spi_dev_get(spi);
	dev_set_drvdata(&spi->dev, vsc);

	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	err = spi_setup(spi);
	if (err) {
		dev_err(&spi->dev, "spi_setup failed, err=%d\n", err);
		goto err_drvdata;
	}

	err = vsc7385_detect(vsc);
	if (err) {
		dev_err(&spi->dev, "no chip found, err=%d\n", err);
		goto err_drvdata;
	}

	err = vsc7385_upload_ucode(vsc);
	if (err)
		goto err_drvdata;

	err = vsc7385_setup(vsc);
	if (err)
		goto err_drvdata;

	return 0;

err_drvdata:
	dev_set_drvdata(&spi->dev, NULL);
	kfree(vsc);
	return err;
}

static int vsc7385_remove(struct spi_device *spi)
{
	struct vsc7385_data	*vsc;

	vsc = dev_get_drvdata(&spi->dev);
	dev_set_drvdata(&spi->dev, NULL);
	kfree(vsc);

	return 0;
}

static struct spi_driver vsc7385_driver = {
	.driver = {
		.name		= DRV_NAME,
		.bus		= &spi_bus_type,
		.owner		= THIS_MODULE,
	},
	.probe		= vsc7385_probe,
	.remove		= vsc7385_remove,
};

static int __init vsc7385_init(void)
{
	return spi_register_driver(&vsc7385_driver);
}
module_init(vsc7385_init);

static void __exit vsc7385_exit(void)
{
	spi_unregister_driver(&vsc7385_driver);
}
module_exit(vsc7385_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");

