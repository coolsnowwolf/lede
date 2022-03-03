// SPDX-License-Identifier: GPL-2.0-only

#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/spi-nor.h>

#include "rtl838x-spi.h"
#include <asm/mach-rtl838x/mach-rtl83xx.h>

extern struct rtl83xx_soc_info soc_info;

struct rtl838x_nor {
	struct spi_nor nor;
	struct device *dev;
	volatile void __iomem *base;
	bool fourByteMode;
	u32 chipSize;
	uint32_t flags;
	uint32_t io_status;
};

static uint32_t spi_prep(struct rtl838x_nor *rtl838x_nor)
{
	/* Needed because of MMU constraints */
	SPI_WAIT_READY;
	spi_w32w(SPI_CS_INIT, SFCSR);	//deactivate CS0, CS1
	spi_w32w(0, SFCSR);		//activate CS0,CS1
	spi_w32w(SPI_CS_INIT, SFCSR);	//deactivate CS0, CS1

	return (CS0 & rtl838x_nor->flags) ? (SPI_eCS0 & SPI_LEN_INIT)
			: ((SPI_eCS1 & SPI_LEN_INIT) | SFCSR_CHIP_SEL);
}

static uint32_t rtl838x_nor_get_SR(struct rtl838x_nor *rtl838x_nor)
{
	uint32_t sfcsr, sfdr;

	sfcsr = spi_prep(rtl838x_nor);
	sfdr  = (SPINOR_OP_RDSR)<<24;

	pr_debug("%s: rdid,sfcsr_val = %.8x,SFDR = %.8x\n", __func__, sfcsr, sfdr);
	pr_debug("rdid,sfcsr = %.8x\n", sfcsr | SPI_LEN4);
	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);
	spi_w32_mask(0, SPI_LEN4, SFCSR);
	SPI_WAIT_READY;

	return spi_r32(SFDR);
}

static void spi_write_disable(struct rtl838x_nor *rtl838x_nor)
{
	uint32_t sfcsr, sfdr;

	sfcsr = spi_prep(rtl838x_nor);
	sfdr = (SPINOR_OP_WRDI) << 24;
	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);
	pr_debug("%s: sfcsr_val = %.8x,SFDR = %.8x", __func__, sfcsr, sfdr);

	spi_prep(rtl838x_nor);
}

static void spi_write_enable(struct rtl838x_nor *rtl838x_nor)
{
	uint32_t sfcsr, sfdr;

	sfcsr = spi_prep(rtl838x_nor);
	sfdr = (SPINOR_OP_WREN) << 24;
	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);
	pr_debug("%s: sfcsr_val = %.8x,SFDR = %.8x", __func__, sfcsr, sfdr);

	spi_prep(rtl838x_nor);
}

static void spi_4b_set(struct rtl838x_nor *rtl838x_nor, bool enable)
{
	uint32_t sfcsr, sfdr;

	sfcsr = spi_prep(rtl838x_nor);
	if (enable)
		sfdr = (SPINOR_OP_EN4B) << 24;
	else
		sfdr = (SPINOR_OP_EX4B) << 24;

	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);
	pr_debug("%s: sfcsr_val = %.8x,SFDR = %.8x", __func__, sfcsr, sfdr);

	spi_prep(rtl838x_nor);
}

static int rtl838x_get_addr_mode(struct rtl838x_nor *rtl838x_nor)
{
	int res = 3;
	u32 reg;

	sw_w32(0x3, RTL838X_INT_RW_CTRL);
	if (!sw_r32(RTL838X_EXT_VERSION)) {
		if (sw_r32(RTL838X_STRAP_DBG) & (1 << 29))
			res = 4;
	} else {
		reg = sw_r32(RTL838X_PLL_CML_CTRL);
		if ((reg & (1 << 30)) && (reg & (1 << 31)))
			res = 4;
		if ((!(reg & (1 << 30)))
		     && sw_r32(RTL838X_STRAP_DBG) & (1 << 29))
			res = 4;
	}
	sw_w32(0x0, RTL838X_INT_RW_CTRL);
	return res;
}

static int rtl8390_get_addr_mode(struct rtl838x_nor *rtl838x_nor)
{
	if (spi_r32(RTL8390_SOC_SPI_MMIO_CONF) & (1 << 9))
		return 4;
	return 3;
}

ssize_t rtl838x_do_read(struct rtl838x_nor *rtl838x_nor, loff_t from,
			       size_t length, u_char *buffer, uint8_t command)
{
	uint32_t sfcsr, sfdr;
	uint32_t len = length;

	sfcsr = spi_prep(rtl838x_nor);
	sfdr = command << 24;

	/* Perform SPINOR_OP_READ: 1 byte command & 3 byte addr*/
	sfcsr |= SPI_LEN4;
	sfdr |= from;

	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);

	/* Read Data, 4 bytes at a time */
	while (length >= 4) {
		SPI_WAIT_READY;
		*((uint32_t *) buffer) = spi_r32(SFDR);
		buffer += 4;
		length -= 4;
	}

	/* The rest needs to be read 1 byte a time */
	sfcsr &= SPI_LEN_INIT|SPI_LEN1;
	SPI_WAIT_READY;
	spi_w32w(sfcsr, SFCSR);
	while (length > 0) {
		SPI_WAIT_READY;
		*(buffer) = spi_r32(SFDR) >> 24;
		buffer++;
		length--;
	}
	return len;
}

/*
 * Do fast read in 3 or 4 Byte addressing mode
 */
static ssize_t rtl838x_do_4bf_read(struct rtl838x_nor *rtl838x_nor, loff_t from,
			       size_t length, u_char *buffer, uint8_t command)
{
	int sfcsr_addr_len = rtl838x_nor->fourByteMode ? 0x3 : 0x2;
	int sfdr_addr_shift = rtl838x_nor->fourByteMode ? 0 : 8;
	uint32_t sfcsr;
	uint32_t len = length;

	pr_debug("Fast read from %llx, len %x, shift %d\n",
		 from, sfcsr_addr_len, sfdr_addr_shift);
	sfcsr = spi_prep(rtl838x_nor);

	/* Send read command */
	spi_w32w(sfcsr | SPI_LEN1, SFCSR);
	spi_w32w(command << 24, SFDR);

	/* Send address */
	spi_w32w(sfcsr | (sfcsr_addr_len << 28), SFCSR);
	spi_w32w(from << sfdr_addr_shift, SFDR);

	/* Dummy cycles */
	spi_w32w(sfcsr | SPI_LEN1, SFCSR);
	spi_w32w(0, SFDR);

	/* Start reading */
	spi_w32w(sfcsr | SPI_LEN4, SFCSR);

	/* Read Data, 4 bytes at a time */
	while (length >= 4) {
		SPI_WAIT_READY;
		*((uint32_t *) buffer) = spi_r32(SFDR);
		buffer += 4;
		length -= 4;
	}

	/* The rest needs to be read 1 byte a time */
	sfcsr &= SPI_LEN_INIT|SPI_LEN1;
	SPI_WAIT_READY;
	spi_w32w(sfcsr, SFCSR);
	while (length > 0) {
		SPI_WAIT_READY;
		*(buffer) = spi_r32(SFDR) >> 24;
		buffer++;
		length--;
	}
	return len;

}

/*
 * Do write (Page Programming) in 3 or 4 Byte addressing mode
 */
static ssize_t rtl838x_do_4b_write(struct rtl838x_nor *rtl838x_nor, loff_t to,
				    size_t length, const u_char *buffer,
				    uint8_t command)
{
	int sfcsr_addr_len = rtl838x_nor->fourByteMode ? 0x3 : 0x2;
	int sfdr_addr_shift = rtl838x_nor->fourByteMode ? 0 : 8;
	uint32_t sfcsr;
	uint32_t len = length;

	pr_debug("Write to %llx, len %x, shift %d\n",
		 to, sfcsr_addr_len, sfdr_addr_shift);
	sfcsr = spi_prep(rtl838x_nor);

	/* Send write command, command IO-width is 1 (bit 25/26) */
	spi_w32w(sfcsr | SPI_LEN1 | (0 << 25), SFCSR);
	spi_w32w(command << 24, SFDR);

	/* Send address */
	spi_w32w(sfcsr | (sfcsr_addr_len << 28) | (0 << 25), SFCSR);
	spi_w32w(to << sfdr_addr_shift, SFDR);

	/* Write Data, 1 byte at a time, if we are not 4-byte aligned */
	if (((long)buffer) % 4) {
		spi_w32w(sfcsr | SPI_LEN1, SFCSR);
		while (length > 0 && (((long)buffer) % 4)) {
			SPI_WAIT_READY;
			spi_w32(*(buffer) << 24, SFDR);
			buffer += 1;
			length -= 1;
		}
	}

	/* Now we can write 4 bytes at a time */
	SPI_WAIT_READY;
	spi_w32w(sfcsr | SPI_LEN4, SFCSR);
	while (length >= 4) {
		SPI_WAIT_READY;
		spi_w32(*((uint32_t *)buffer), SFDR);
		buffer += 4;
		length -= 4;
	}

	/* Final bytes might need to be written 1 byte at a time, again */
	SPI_WAIT_READY;
	spi_w32w(sfcsr | SPI_LEN1, SFCSR);
	while (length > 0) {
		SPI_WAIT_READY;
		spi_w32(*(buffer) << 24, SFDR);
		buffer++;
		length--;
	}
	return len;
}

static ssize_t rtl838x_nor_write(struct spi_nor *nor, loff_t to, size_t len,
				 const u_char *buffer)
{
	int ret = 0;
	uint32_t offset = 0;
	struct rtl838x_nor *rtl838x_nor = nor->priv;
	size_t l = len;
	uint8_t cmd = SPINOR_OP_PP;

	/* Do write in 4-byte mode on large Macronix chips */
	if (rtl838x_nor->fourByteMode) {
		cmd = SPINOR_OP_PP_4B;
		spi_4b_set(rtl838x_nor, true);
	}

	pr_debug("In %s %8x to: %llx\n", __func__,
		 (unsigned int) rtl838x_nor, to);

	while (l >= SPI_MAX_TRANSFER_SIZE) {
		while
			(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WIP);
		do {
			spi_write_enable(rtl838x_nor);
		} while (!(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WEL));
		ret = rtl838x_do_4b_write(rtl838x_nor, to + offset,
				SPI_MAX_TRANSFER_SIZE, buffer+offset, cmd);
		l -= SPI_MAX_TRANSFER_SIZE;
		offset += SPI_MAX_TRANSFER_SIZE;
	}

	if (l > 0) {
		while
			(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WIP);
		do {
			spi_write_enable(rtl838x_nor);
		} while (!(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WEL));
		ret = rtl838x_do_4b_write(rtl838x_nor, to+offset,
					  len, buffer+offset, cmd);
	}

	return len;
}

static ssize_t rtl838x_nor_read(struct spi_nor *nor, loff_t from,
				size_t length, u_char *buffer)
{
	uint32_t offset = 0;
	uint8_t cmd = SPINOR_OP_READ_FAST;
	size_t l = length;
	struct rtl838x_nor *rtl838x_nor = nor->priv;

	/* Do fast read in 3, or 4-byte mode on large Macronix chips */
	if (rtl838x_nor->fourByteMode) {
		cmd = SPINOR_OP_READ_FAST_4B;
		spi_4b_set(rtl838x_nor, true);
	}

	/* TODO: do timeout and return error */
	pr_debug("Waiting for pending writes\n");
	while
		(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WIP);
	do {
		spi_write_enable(rtl838x_nor);
	} while (!(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WEL));

	pr_debug("cmd is %d\n", cmd);
	pr_debug("%s: addr %.8llx to addr %.8x, cmd %.8x, size %d\n", __func__,
		 from, (u32)buffer, (u32)cmd, length);

	while (l >= SPI_MAX_TRANSFER_SIZE) {
		rtl838x_do_4bf_read(rtl838x_nor, from + offset,
				    SPI_MAX_TRANSFER_SIZE, buffer+offset, cmd);
		l -= SPI_MAX_TRANSFER_SIZE;
		offset += SPI_MAX_TRANSFER_SIZE;
	}

	if (l > 0)
		rtl838x_do_4bf_read(rtl838x_nor, from + offset, l, buffer+offset, cmd);

	return length;
}

static int rtl838x_erase(struct spi_nor *nor, loff_t offs)
{
	struct rtl838x_nor *rtl838x_nor = nor->priv;
	int sfcsr_addr_len = rtl838x_nor->fourByteMode ? 0x3 : 0x2;
	int sfdr_addr_shift = rtl838x_nor->fourByteMode ? 0 : 8;
	uint32_t sfcsr;
	uint8_t cmd = SPINOR_OP_SE;

	pr_debug("Erasing sector at %llx\n", offs);

	/* Do erase in 4-byte mode on large Macronix chips */
	if (rtl838x_nor->fourByteMode) {
		cmd = SPINOR_OP_SE_4B;
		spi_4b_set(rtl838x_nor, true);
	}
	/* TODO: do timeout and return error */
	while
		(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WIP);
	do {
		spi_write_enable(rtl838x_nor);
	} while (!(rtl838x_nor_get_SR(rtl838x_nor) & SPI_WEL));

	sfcsr = spi_prep(rtl838x_nor);

	/* Send erase command, command IO-width is 1 (bit 25/26) */
	spi_w32w(sfcsr | SPI_LEN1 | (0 << 25), SFCSR);
	spi_w32w(cmd << 24, SFDR);

	/* Send address */
	spi_w32w(sfcsr | (sfcsr_addr_len << 28) | (0 << 25), SFCSR);
	spi_w32w(offs << sfdr_addr_shift, SFDR);

	return 0;
}

static int rtl838x_nor_read_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	int length = len;
	u8 *buffer = buf;
	uint32_t sfcsr, sfdr;
	struct rtl838x_nor *rtl838x_nor = nor->priv;

	pr_debug("In %s: opcode %x, len %x\n", __func__, opcode, len);

	sfcsr = spi_prep(rtl838x_nor);
	sfdr = opcode << 24;

	sfcsr |= SPI_LEN1;

	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);

	while (length > 0) {
		SPI_WAIT_READY;
		*(buffer) = spi_r32(SFDR) >> 24;
		buffer++;
		length--;
	}

	return len;
}

static int rtl838x_nor_write_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	uint32_t sfcsr, sfdr;
	struct rtl838x_nor *rtl838x_nor = nor->priv;

	pr_debug("In %s, opcode %x, len %x\n", __func__, opcode, len);
	sfcsr = spi_prep(rtl838x_nor);
	sfdr = opcode << 24;

	if (len == 1) { /* SPINOR_OP_WRSR */
		sfdr |= buf[0];
		sfcsr |= SPI_LEN2;
	}
	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);
	return 0;
}

static int spi_enter_sio(struct spi_nor *nor)
{
	uint32_t sfcsr, sfcr2, sfdr;
	uint32_t ret = 0, reg = 0, size_bits;
	struct rtl838x_nor *rtl838x_nor = nor->priv;

	pr_debug("In %s\n", __func__);
	rtl838x_nor->io_status = 0;
	sfdr = SPI_C_RSTQIO << 24;
	sfcsr = spi_prep(rtl838x_nor);

	reg = spi_r32(SFCR2);
	pr_debug("SFCR2: %x, size %x, rdopt: %x\n", reg, SFCR2_GETSIZE(reg),
						  (reg & SFCR2_RDOPT));
	size_bits = rtl838x_nor->fourByteMode ? SFCR2_SIZE(0x6) : SFCR2_SIZE(0x7);

	sfcr2 = SFCR2_HOLD_TILL_SFDR2 | size_bits
		| (reg & SFCR2_RDOPT) | SFCR2_CMDIO(0)
		| SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4)
		| SFCR2_DATAIO(0) | SFCR2_SFCMD(SPINOR_OP_READ_FAST);
	pr_debug("SFCR2: %x, size %x\n", reg, SFCR2_GETSIZE(reg));

	SPI_WAIT_READY;
	spi_w32w(sfcr2, SFCR2);
	spi_w32w(sfcsr, SFCSR);
	spi_w32w(sfdr, SFDR);

	spi_w32_mask(SFCR2_HOLD_TILL_SFDR2, 0, SFCR2);
	rtl838x_nor->io_status &= ~IOSTATUS_CIO_MASK;
	rtl838x_nor->io_status |= CIO1;

	spi_prep(rtl838x_nor);

	return ret;
}

int rtl838x_spi_nor_scan(struct spi_nor *nor, const char *name)
{
	static const struct spi_nor_hwcaps hwcaps = {
		.mask = SNOR_HWCAPS_READ | SNOR_HWCAPS_PP
			| SNOR_HWCAPS_READ_FAST
	};

	struct rtl838x_nor *rtl838x_nor = nor->priv;

	pr_debug("In %s\n", __func__);

	spi_w32_mask(0, SFCR_EnableWBO, SFCR);
	spi_w32_mask(0, SFCR_EnableRBO, SFCR);

	rtl838x_nor->flags = CS0 | R_MODE;

	spi_nor_scan(nor, NULL, &hwcaps);
	pr_debug("------------- Got size: %llx\n", nor->mtd.size);

	return 0;
}

int rtl838x_nor_init(struct rtl838x_nor *rtl838x_nor,
			struct device_node *flash_node)
{
	int ret;
	struct spi_nor *nor;

	pr_info("%s called\n", __func__);
	nor = &rtl838x_nor->nor;
	nor->dev = rtl838x_nor->dev;
	nor->priv = rtl838x_nor;
	spi_nor_set_flash_node(nor, flash_node);

	nor->read_reg = rtl838x_nor_read_reg;
	nor->write_reg = rtl838x_nor_write_reg;
	nor->read = rtl838x_nor_read;
	nor->write = rtl838x_nor_write;
	nor->erase = rtl838x_erase;
	nor->mtd.name = "rtl838x_nor";
	nor->erase_opcode = rtl838x_nor->fourByteMode ? SPINOR_OP_SE_4B
					: SPINOR_OP_SE;
	/* initialized with NULL */
	ret = rtl838x_spi_nor_scan(nor, NULL);
	if (ret)
		return ret;

	spi_enter_sio(nor);
	spi_write_disable(rtl838x_nor);

	ret = mtd_device_parse_register(&nor->mtd, NULL, NULL, NULL, 0);
	return ret;
}

static int rtl838x_nor_drv_probe(struct platform_device *pdev)
{
	struct device_node *flash_np;
	struct resource *res;
	int ret;
	struct rtl838x_nor *rtl838x_nor;
	int addrMode;

	pr_info("Initializing rtl838x_nor_driver\n");
	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	rtl838x_nor = devm_kzalloc(&pdev->dev, sizeof(*rtl838x_nor), GFP_KERNEL);
	if (!rtl838x_nor)
		return -ENOMEM;
	platform_set_drvdata(pdev, rtl838x_nor);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rtl838x_nor->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR((void *)rtl838x_nor->base))
		return PTR_ERR((void *)rtl838x_nor->base);

	pr_info("SPI resource base is %08x\n", (u32)rtl838x_nor->base);
	rtl838x_nor->dev = &pdev->dev;

	/* only support one attached flash */
	flash_np = of_get_next_available_child(pdev->dev.of_node, NULL);
	if (!flash_np) {
		dev_err(&pdev->dev, "no SPI flash device to configure\n");
		ret = -ENODEV;
		goto nor_free;
	}

	/* Get the 3/4 byte address mode as configure by bootloader */
	if (soc_info.family == RTL8390_FAMILY_ID)
		addrMode = rtl8390_get_addr_mode(rtl838x_nor);
	else
		addrMode = rtl838x_get_addr_mode(rtl838x_nor);
	pr_info("Address mode is %d bytes\n", addrMode);
	if (addrMode == 4)
		rtl838x_nor->fourByteMode = true;

	ret = rtl838x_nor_init(rtl838x_nor, flash_np);

nor_free:
	return ret;
}

static int rtl838x_nor_drv_remove(struct platform_device *pdev)
{
/*	struct rtl8xx_nor *rtl838x_nor = platform_get_drvdata(pdev); */
	return 0;
}

static const struct of_device_id rtl838x_nor_of_ids[] = {
	{ .compatible = "realtek,rtl838x-nor"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl838x_nor_of_ids);

static struct platform_driver rtl838x_nor_driver = {
	.probe = rtl838x_nor_drv_probe,
	.remove = rtl838x_nor_drv_remove,
	.driver = {
		.name = "rtl838x-nor",
		.pm = NULL,
		.of_match_table = rtl838x_nor_of_ids,
	},
};

module_platform_driver(rtl838x_nor_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("RTL838x SPI NOR Flash Driver");
