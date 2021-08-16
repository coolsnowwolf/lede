/**
 * Copyright (c) 2012-2012 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

/*
 * Platform dependant implement. Customer needs to modify this file.
 */
#include <linux/interrupt.h>
#include <qdpc_platform.h>
#include <topaz_vnet.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

/*
 * Enable MSI interrupt of PCIe.
 */
void enable_vmac_ints(struct vmac_priv *vmp)
{
	enable_irq(vmp->pdev->irq);
}

/*
 * Disable MSI interrupt of PCIe.
 */
void disable_vmac_ints(struct vmac_priv *vmp)
{
	disable_irq_nosync(vmp->pdev->irq);
}


/*
 * Enable interrupt for detecting EP reset.
 */
void enable_ep_rst_detection(struct net_device *ndev)
{
}

/*
 * Disable interrupt for detecting EP reset.
 */
void disable_ep_rst_detection(struct net_device *ndev)
{
}

/*
 * Interrupt context for detecting EP reset.
 * This function should do:
 *   1. check interrupt status to see if EP reset.
 *   2. if EP reset, handle it.
 */
void handle_ep_rst_int(struct net_device *ndev)
{
}

/*
 * PCIe driver update resource in PCI configure space after EP reset.
 * This function should be called in such case:
 *   1. The PCI configure space can be accessed after EP reset;
 *   2. Kernel does not support PCIe hot-plug.
 */
void qdpc_update_hw_bar(struct pci_dev *pdev, uint8_t index)
{
	struct pci_bus_region region;
	uint32_t addr, new;
	int offset = PCI_BASE_ADDRESS_0 + 4 * index;
	struct resource *res = pdev->resource + index;

	if (!res->flags)
		return;

	pcibios_resource_to_bus(pdev, &region, res);
	new = region.start | (res->flags & PCI_REGION_FLAG_MASK);
	pci_read_config_dword(pdev, offset, &addr);

	if (addr != new) {
		printk("PCI region %d: reset to PCI address %#llx", index, (unsigned long long)region.start);
		pci_write_config_dword(pdev, offset, new);
		if ((new & (PCI_BASE_ADDRESS_SPACE|PCI_BASE_ADDRESS_MEM_TYPE_MASK)) ==
		    (PCI_BASE_ADDRESS_SPACE_MEMORY|PCI_BASE_ADDRESS_MEM_TYPE_64)) {
			printk(" (64bit address)");
			new = region.start >> 16 >> 16;
			pci_write_config_dword(pdev, offset + 4, new);
		}
		printk("\n");
	}
}

