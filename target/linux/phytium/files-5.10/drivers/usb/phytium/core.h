/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PHYTIUM_CORE_H__
#define __PHYTIUM_CORE_H__

#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include "host_api.h"
#include "gadget.h"

#define MAX_EPS_CHANNELS	16

struct phytium_ep {
	struct phytium_cusb	*config;
	u16			max_packet;
	u8			ep_num;
	struct GADGET_EP	*gadget_ep;
	struct list_head	req_list;
	struct usb_ep		end_point;
	char			name[12];
	u8 is_tx;
	const struct usb_endpoint_descriptor	*desc;
	u8			busy;
};

struct phytium_request {
	struct usb_request request;
	struct GADGET_REQ	*gadget_request;
	struct list_head	list;
	struct phytium_ep	*ep;
	struct phytium_cusb	*config;
	u8			is_tx;
	u8			epnum;
};

struct phytium_cusb {
	struct device		*dev;
	void __iomem		*regs;
	void __iomem		*phy_regs;
	int			irq;
	spinlock_t		lock;
	enum usb_dr_mode	dr_mode;

	struct GADGET_OBJ	*gadget_obj;
	struct GADGET_CFG	gadget_cfg;
	struct GADGET_CALLBACKS gadget_callbacks;
	struct GADGET_SYSREQ	gadget_sysreq;
	struct GADGET_DEV	*gadget_dev;
	void			*gadget_priv;

	struct usb_gadget	gadget;
	struct usb_gadget_driver *gadget_driver;
	struct phytium_ep	endpoints_tx[MAX_EPS_CHANNELS];
	struct phytium_ep	endpoints_rx[MAX_EPS_CHANNELS];
	u8			ep0_data_stage_is_tx;

	struct HOST_OBJ		*host_obj;
	struct HOST_CFG		host_cfg;
	struct HOST_CALLBACKS	host_callbacks;
	struct HOST_SYSREQ	host_sysreq;
	void			*host_priv;
	struct usb_hcd          *hcd;

	struct DMA_OBJ		*dma_obj;
	struct DMA_CFG		dma_cfg;
	struct DMA_CALLBACKS	dma_callbacks;
	struct DMA_SYSREQ	dma_sysreq;
	bool	isVhubHost;
};

int phytium_core_reset(struct phytium_cusb *config, bool skip_wait);

int phytium_host_init(struct phytium_cusb *config);
int phytium_host_uninit(struct phytium_cusb *config);

#ifdef CONFIG_PM
int phytium_host_resume(void *priv);
int phytium_host_suspend(void *priv);
int phytium_gadget_resume(void *priv);
int phytium_gadget_suspend(void *priv);
#endif

int phytium_gadget_init(struct phytium_cusb *config);
int phytium_gadget_uninit(struct phytium_cusb *config);

uint32_t phytium_read32(uint32_t *address);

void phytium_write32(uint32_t *address, uint32_t value);

uint16_t phytium_read16(uint16_t *address);

void phytium_write16(uint16_t *address, uint16_t value);

uint8_t phytium_read8(uint8_t *address);

void phytium_write8(uint8_t *address, uint8_t value);

#endif
