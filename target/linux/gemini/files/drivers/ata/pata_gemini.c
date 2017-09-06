/*
 *  Support for Gemini PATA
 *
 *  Copyright (C) 2009 Janos Laube <janos.dev@gmail.com>
 *  Copyright (C) 2010 Frederic Pecourt <opengemini@free.fr>
 *  Copyright (C) 2011 Tobias Waldvogel <tobias.waldvogel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Values of IOMUX
 * 26:24 bits is "IDE IO Select"
 * 111:100 - Reserved
 * 011 - ata0 <-> sata0, sata1; bring out ata1
 * 010 - ata1 <-> sata1, sata0; bring out ata0
 * 001 - ata0 <-> sata0, ata1 <-> sata1; bring out ata1
 * 000 - ata0 <-> sata0, ata1 <-> sata1; bring out ata0
 *
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/libata.h>
#include <linux/leds.h>

#include <mach/hardware.h>
#include <mach/global_reg.h>

#define DRV_NAME		"pata-gemini"

#define PATA_GEMINI_PORTS	1

#define PIO_TIMING_REG		0x10
#define MDMA_TIMING_REG		0x11
#define UDMA_TIMING0_REG	0x12
#define UDMA_TIMING1_REG	0x13
#define CLK_MOD_REG		0x14

#define CLK_MOD_66M_DEV0_BIT	0
#define CLK_MOD_66M_DEV1_BIT	1
#define CLK_MOD_UDMA_DEV0_BIT	4
#define CLK_MOD_UDMA_DEV1_BIT	5

#define CLK_MOD_66M_DEV0	(1 << CLK_MOD_66M_DEV0_BIT)
#define CLK_MOD_66M_DEV1	(1 << CLK_MOD_66M_DEV1_BIT)
#define CLK_MOD_UDMA_DEV0	(1 << CLK_MOD_UDMA_DEV0_BIT)
#define CLK_MOD_UDMA_DEV1	(1 << CLK_MOD_UDMA_DEV1_BIT)

#define SATA_ENABLE_PDEV_MASK		0x01
#define SATA_ENABLE_PDEV_PM		0x02
#define SATA_ENABLE_PDEV_ADDED		0x04
#define SATA_ENABLE_PDEV_REMOVED	0x08
#define SATA_ENABLE_SDEV_MASK		0x10
#define SATA_ENABLE_SDEV_PM		0x20
#define SATA_ENABLE_SDEV_ADDED		0x40
#define SATA_ENABLE_SDEV_REMOVED	0x80

MODULE_AUTHOR("Janos Laube <janos.dev@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

static unsigned char PIO_TIMING[5] = {
	0xaa, 0xa3, 0xa1, 0x33, 0x31
};

static unsigned char TIMING_MW_DMA[4][2] = {
	{ 0x44, 1 }, // 480        4.2
	{ 0x42, 1 }, // 150       13.3
	{ 0x31, 1 }, // 120       16.7
	{ 0x21, 1 }, // 100       20
};

static unsigned char TIMING_UDMA[7][2] = {
	{ 0x33, 0 }, //240        16.7
	{ 0x31, 0 }, //160        25
	{ 0x21, 0 }, //120        33.3
	{ 0x21, 1 }, //90         44.4
	{ 0x11, 1 }, //60         66.7
	{ 0x11 | 0x80, 0 }, //40  100
	{ 0x11 | 0x80, 1 }, //30  133
};

static struct scsi_host_template pata_gemini_sht = {
	ATA_NCQ_SHT(DRV_NAME),
	.can_queue	= 1,
	.sg_tablesize	= 128,
	.dma_boundary	= 0xffffU,
};

static void gemini_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	void __iomem *clk_reg	= ap->ioaddr.bmdma_addr + CLK_MOD_REG;
	void __iomem *tim_reg	= ap->ioaddr.bmdma_addr + UDMA_TIMING0_REG;
	unsigned short udma	= adev->dma_mode;
	unsigned short speed	= udma;
	unsigned short devno	= adev->devno & 1;
	unsigned short i;
	u8 mod_udma_mask	= 1 << (CLK_MOD_UDMA_DEV0_BIT + devno);
	u8 mod_66m_mask		= 1 << (CLK_MOD_66M_DEV0_BIT + devno);
	u8 clk_mod;
	u8 timing;

	clk_mod = ioread8(clk_reg);
	clk_mod &= ~mod_udma_mask;

	if (speed & XFER_UDMA_0) {
		i = speed & ~XFER_UDMA_0;
		timing = TIMING_UDMA[i][0];
		clk_mod |= mod_udma_mask;
		if (TIMING_UDMA[i][1])
			clk_mod |= mod_66m_mask;
	} else {
		i = speed & ~XFER_MW_DMA_0;
		timing = TIMING_MW_DMA[i][0];
		clk_mod |= mod_udma_mask;
		if (TIMING_MW_DMA[i][1])
			clk_mod |= mod_66m_mask;
	}

	iowrite8(clk_mod, clk_reg);
	iowrite8(timing, tim_reg + devno);
	return;
}

static void gemini_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	void __iomem *pio_reg	= ap->ioaddr.bmdma_addr + PIO_TIMING_REG;
	unsigned int pio	= adev->pio_mode - XFER_PIO_0;

	iowrite8(PIO_TIMING[pio], pio_reg);
}

unsigned int gemini_qc_issue(struct ata_queued_cmd *qc)
{
	ledtrig_ide_activity();
	return ata_bmdma_qc_issue(qc);
}

static struct ata_port_operations pata_gemini_port_ops = {
	.inherits	= &ata_bmdma_port_ops,
	.set_dmamode	= gemini_set_dmamode,
	.set_piomode	= gemini_set_piomode,
	.qc_issue	= gemini_qc_issue,
};

static struct ata_port_info pata_gemini_portinfo = {
	.flags		= 0,
	.udma_mask	= ATA_UDMA6,
	.pio_mask	= ATA_PIO4,
	.port_ops	= &pata_gemini_port_ops,
};

static const struct ata_port_info *pata_gemini_ports = &pata_gemini_portinfo;

static int pata_gemini_probe(struct platform_device *pdev)
{
	struct ata_host *host;
	struct resource *res;
	unsigned int irq, i;
	void __iomem *mmio_base;

	/* standard bdma init */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	pr_info(DRV_NAME ": irq %d, io base 0x%08x\n", irq, res->start);

	mmio_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));

	host = ata_host_alloc_pinfo(&pdev->dev, &pata_gemini_ports, 1);
	if (!host)
		return -ENOMEM;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
		struct ata_ioports *ioaddr = &ap->ioaddr;

		ioaddr->bmdma_addr		= mmio_base;
		ioaddr->cmd_addr		= mmio_base + 0x20;
		ioaddr->ctl_addr		= mmio_base + 0x36;
		ioaddr->altstatus_addr		= ioaddr->ctl_addr;
		ata_sff_std_ports(ioaddr);
		host->ports[i]->cbl = ATA_CBL_SATA;
	}

	return ata_host_activate(host, irq, ata_bmdma_interrupt,
		IRQF_SHARED, &pata_gemini_sht);
}

static int pata_gemini_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	ata_host_detach(host);
	return 0;
}

static struct platform_driver pata_gemini_driver = {
	.probe		= pata_gemini_probe,
	.remove		= pata_gemini_remove,
	.driver.owner	= THIS_MODULE,
	.driver.name	= DRV_NAME,
};

static int __init pata_gemini_module_init(void)
{
	return platform_driver_probe(&pata_gemini_driver, pata_gemini_probe);
}

static void __exit pata_gemini_module_exit(void)
{
	platform_driver_unregister(&pata_gemini_driver);
}

module_init(pata_gemini_module_init);
module_exit(pata_gemini_module_exit);
