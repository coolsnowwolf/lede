// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ATH9K Fixup Driver
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2014 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 * Copyright (C) 2008 Florian Fainelli <f.fainelli@gmail.com>
 */

#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/ath9k_platform.h>

#define ATH9K_MAX_FIXUPS	2

#define ATH9K_DEF_LED_PIN	-1
#define ATH9K_DEF_PCI_DEV	255

struct ath9k_fixup {
	struct device *dev;
	struct resource *mem_res;
	u32 pci_dev;
	u8 mac[ETH_ALEN];
	struct ath9k_platform_data pdata;
};

static int ath9k_num_fixups;
static struct ath9k_fixup *ath9k_fixups[ATH9K_MAX_FIXUPS];

static void ath9k_pci_fixup(struct pci_dev *dev)
{
	void __iomem *mem;
	struct ath9k_fixup *priv = NULL;
	struct ath9k_platform_data *pdata = NULL;
	struct pci_dev *bridge = pci_upstream_bridge(dev);
	u16 *cal_data = NULL;
	u16 cmd;
	u32 bar0;
	u32 val;
	unsigned i;

	for (i = 0; i < ath9k_num_fixups; i++) {
		if (ath9k_fixups[i]->pci_dev != PCI_SLOT(dev->devfn))
			continue;

		priv = ath9k_fixups[i];
		cal_data = priv->pdata.eeprom_data;
		pdata = &priv->pdata;
		break;
	}

	if (cal_data == NULL)
		return;

	if (*cal_data != 0xa55a) {
		pr_err("pci %s: invalid calibration data\n", pci_name(dev));
		return;
	}

	pr_info("pci %s: fixup device configuration\n", pci_name(dev));

	val = priv->mem_res->start;
	mem = ioremap(priv->mem_res->start, resource_size(priv->mem_res));
	if (!mem) {
		pr_err("pci %s: ioremap error\n", pci_name(dev));
		return;
	}

	if (bridge)
		pci_enable_device(bridge);

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &bar0);
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &bar0);
	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, val);

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	/* set offset to first reg address */
	cal_data += 3;
	while(*cal_data != 0xffff) {
		u32 reg;

		reg = *cal_data++;
		val = *cal_data++;
		val |= (*cal_data++) << 16;

		writel(val, mem + reg);
		udelay(100);
	}

	pci_read_config_dword(dev, PCI_VENDOR_ID, &val);
	dev->vendor = val & 0xffff;
	dev->device = (val >> 16) & 0xffff;

	pci_read_config_dword(dev, PCI_CLASS_REVISION, &val);
	dev->revision = val & 0xff;
	dev->class = val >> 8; /* upper 3 bytes */

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd &= ~(PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, bar0);

	if (bridge)
		pci_disable_device(bridge);

	iounmap(mem);

	dev->dev.platform_data = pdata;
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_ATHEROS, PCI_ANY_ID, ath9k_pci_fixup);

static int ath9k_mtd_eeprom(struct ath9k_fixup *priv)
{
	struct device *dev = priv->dev;
	struct device_node *node = dev->of_node;
	struct device_node *mtd_np = NULL;
	struct mtd_info *the_mtd;
	phandle phandle;
	size_t eeprom_readlen;
	const char *part;
	const __be32 *list;
	int ret, i;

	list = of_get_property(node, "ath,eeprom", &i);
	if (!list || (i != (2 * sizeof(*list))))
		return -ENODEV;

	phandle = be32_to_cpup(list++);
	if (phandle)
		mtd_np = of_find_node_by_phandle(phandle);
	if (!mtd_np)
		return -ENODEV;

	part = of_get_property(mtd_np, "label", NULL);
	if (!part)
		part = mtd_np->name;

	the_mtd = get_mtd_device_nm(part);
	if (IS_ERR(the_mtd))
		return -ENODEV;

	ret = mtd_read(the_mtd, be32_to_cpup(list),
		       ATH9K_PLAT_EEP_MAX_WORDS * sizeof(u16),
		       &eeprom_readlen, (void *) priv->pdata.eeprom_data);
	put_mtd_device(the_mtd);
	if (ret)
		return ret;

	return 0;
}

static int ath9k_fixup_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct ath9k_fixup *priv;
	struct resource *res;
	const void *mac;
	int ret;

	if (ath9k_num_fixups >= ATH9K_MAX_FIXUPS)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;
	priv->mem_res = res;

	ret = of_property_read_u32(node, "pci-dev", &priv->pci_dev);
	if (ret)
		priv->pci_dev = ATH9K_DEF_PCI_DEV;

	ret = ath9k_mtd_eeprom(priv);
	if (ret)
		return ret;

	priv->pdata.endian_check = of_property_read_bool(node,
		"ath,endian-check");
	ret = of_property_read_s32(node, "ath,led-pin", &priv->pdata.led_pin);
	if (ret)
		priv->pdata.led_pin = ATH9K_DEF_LED_PIN;
	priv->pdata.led_active_high = of_property_read_bool(node,
		"ath,led-active-high");

	mac = of_get_mac_address(node);
	if (!IS_ERR_OR_NULL(mac)) {
		memcpy(priv->mac, mac, ETH_ALEN);
		dev_info(dev, "mtd mac %pM\n", priv->mac);
	} else {
		random_ether_addr(priv->mac);
		dev_info(dev, "random mac %pM\n", priv->mac);
	}

	priv->pdata.macaddr = priv->mac;

	ath9k_fixups[ath9k_num_fixups] = priv;
	ath9k_num_fixups++;

	return 0;
}

static const struct of_device_id ath9k_fixup_of_match[] = {
	{ .compatible = "brcm,ath9k-fixup" },
	{ /* sentinel */ }
};

static struct platform_driver ath9k_fixup_driver = {
	.probe = ath9k_fixup_probe,
	.driver	= {
		.name = "ath9k-fixup",
		.of_match_table = ath9k_fixup_of_match,
	},
};

int __init ath9k_fixup_init(void)
{
	int ret = platform_driver_register(&ath9k_fixup_driver);
	if (ret)
		pr_err("ath9k_fixup: Error registering platform driver!\n");
	return ret;
}
late_initcall(ath9k_fixup_init);
