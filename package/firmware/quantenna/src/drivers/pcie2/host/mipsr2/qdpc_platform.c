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
#include <qdpc_regs.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

/*
 * Enable MSI interrupt of PCIe.
 */
void enable_vmac_ints(struct vmac_priv *vmp)
{
	volatile uint32_t *dma_wrd_imwr = QDPC_BAR_VADDR(vmp->dmareg_bar, TOPAZ_IMWR_DONE_ADDRLO_OFFSET);

	writel(vmp->dma_msi_imwr, dma_wrd_imwr);
}

/*
 * Disable MSI interrupt of PCIe.
 */
void disable_vmac_ints(struct vmac_priv *vmp)
{
	volatile uint32_t *dma_wrd_imwr = QDPC_BAR_VADDR(vmp->dmareg_bar, TOPAZ_IMWR_DONE_ADDRLO_OFFSET);
	writel(vmp->dma_msi_dummy, dma_wrd_imwr);
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
