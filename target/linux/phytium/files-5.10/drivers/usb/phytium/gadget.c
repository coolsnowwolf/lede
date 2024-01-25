// SPDX-License-Identifier: GPL-2.0

#include <linux/dma-mapping.h>
#include "gadget.h"
#include "dma.h"
#include "core.h"
#include "hw-regs.h"

#define DRV_NAME "phytium_gadget"

#define GADGET_PRIV_BUFFER_SIZE 64
#define GADGET_USB_EP_NUMBER_MASK 0xf
#define DMA_ADDR_INVALID	(~(dma_addr_t)0)

#define GADGET_ESTALL		1
#define GADGET_EUNHANDLED	2
#define GADGET_EAUTOACK		3
#define GADGET_ESHUTDOWN	4
#define GADGET_ECONNRESET	5
#define GADGET_EAGAIN		6

static inline struct GadgetEp *toGadgetEp(struct GADGET_EP *gadget_Ep)
{
	return (struct GadgetEp *)((uintptr_t)gadget_Ep -
					((uintptr_t)&((struct GadgetEp *)0)->gadgetEp));
}

static inline struct GadgetRequest *requestToGadgetRequest(struct GADGET_REQ *req)
{
	return (struct GadgetRequest *)((uintptr_t)req -
					((uintptr_t)&((struct GadgetRequest *)0)->request));
}

static inline struct GADGET_REQ *listToGadgetRequest(struct list_head *list)
{
	return (struct GADGET_REQ *)((uintptr_t)list -
					((uintptr_t)&((struct GADGET_REQ *)0)->list));
}

#define listBrowsingRequest(iterator, head, memeber)	\
	for (iterator = listToGadgetRequest((head)->next); \
		&iterator->list != (head);	\
		iterator = listToGadgetRequest(iterator->list.next))

static inline struct GADGET_REQ *gadgetGetNextReq(struct GadgetEp *gadgetEp)
{
	struct list_head *list = &gadgetEp->request;

	if (list_empty(list)) {
		pr_debug("no request available for %s\n", gadgetEp->gadgetEp.name);
		return NULL;
	}

	return listToGadgetRequest(list->next);
}

static inline struct GADGET_REQ *gadgetGetNextEp0Req(struct GADGET_CTRL *priv)
{
	struct list_head *queue;

	if (!priv)
		return NULL;

	queue = &priv->in[0].request;

	if (list_empty(queue))
		return NULL;

	return listToGadgetRequest(queue->next);
}

void gadget_giveback(struct phytium_ep *phy_ep, struct usb_request *usb_req, int status)
{
	struct phytium_request	*phy_req;
	struct phytium_cusb	*config;
	int busy;

	if (!phy_ep || !usb_req)
		return;

	busy = phy_ep->busy;
	phy_req = usb_req ? container_of(usb_req, struct phytium_request, request) : NULL;
	config = phy_req->config;

	list_del(&phy_req->list);

	if (usb_req->status == -EINPROGRESS) {
		if (status == GADGET_ESHUTDOWN)
			usb_req->status = -ESHUTDOWN;
		else if (status == GADGET_ECONNRESET)
			usb_req->status = -ECONNRESET;
		else
			usb_req->status = -phy_req->gadget_request->status;
	}

	if (usb_req->status == 0)
		pr_debug("%s done request %p, %d/%d\n", phy_ep->end_point.name,
				usb_req, usb_req->actual, usb_req->length);
	else
		pr_debug("%s request %p, %d/%d fault %d\n", phy_ep->end_point.name,
				usb_req, usb_req->actual, usb_req->length, usb_req->status);

	usb_gadget_unmap_request(&config->gadget, &phy_req->request, phy_req->is_tx);

	busy = phy_ep->busy;
	phy_ep->busy = 1;

	spin_unlock(&config->lock);

	if (phy_req->request.complete)
		phy_req->request.complete(&phy_req->ep->end_point, usb_req);

	spin_lock(&config->lock);

	phy_ep->busy = busy;
}

static void gadget_callback_complete(struct GADGET_EP *gadget_ep, struct GADGET_REQ *gadget_req)
{
	struct phytium_ep	*phy_ep;
	struct phytium_request	*phy_req;
	struct usb_request	*usb_req;

	if (!gadget_ep || !gadget_req)
		return;

	phy_req = gadget_req->context;
	usb_req = &phy_req->request;
	phy_ep = phy_req->ep;
	usb_req->actual = gadget_req->actual;
	usb_req->length = gadget_req->length;

	gadget_giveback(phy_ep, usb_req, gadget_req->status);
}

static void gadgetDisconnect(struct GADGET_CTRL *priv)
{
	pr_info("Disconnect USB Device Driver\n");

	if (!priv)
		return;

	priv->gadgetDev.speed = USB_SPEED_UNKNOWN;
	priv->gadgetDev.state = USB_STATE_NOTATTACHED;

	if (priv->eventCallback.disconnect)
		priv->eventCallback.disconnect(priv);
}

static int gadget_get_frame(struct usb_gadget *gadget)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return 0;
}

static int gadget_wakeup(struct usb_gadget *gadget)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return 0;
}

static int gadget_vbus_session(struct usb_gadget *gadget, int is_active)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return 0;
}

static int gadget_vbus_draw(struct usb_gadget *gadget, unsigned int mA)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return 0;
}

static int gadget_pullup(struct usb_gadget *gadget, int is_on)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return 0;
}

static int gadget_udc_start(struct usb_gadget *gadget, struct usb_gadget_driver *driver)
{
	unsigned long flags;
	struct phytium_cusb *config;
	struct GADGET_CTRL *priv;
	uint32_t gen_cfg;

	if (!gadget || !driver)
		return -EINVAL;

	if (driver->max_speed < USB_SPEED_HIGH)
		return -EINVAL;

	config = container_of(gadget, struct phytium_cusb, gadget);

	pr_info("registering driver %s\n", driver->function);

	spin_lock_irqsave(&config->lock, flags);
	config->gadget_driver = driver;
	spin_unlock_irqrestore(&config->lock, flags);

	config->gadget_obj->gadget_start(config->gadget_priv);

	priv = (struct GADGET_CTRL *)config->gadget_priv;
	if (priv->phy_regs) {
		gen_cfg = phytium_read32(&priv->phy_regs->gen_cfg);
		gen_cfg = gen_cfg & (~BIT(7));
		phytium_write32(&priv->phy_regs->gen_cfg, gen_cfg);
	}

	return 0;
}

static int gadget_udc_stop(struct usb_gadget *gadget)
{
	struct phytium_cusb *config;
	unsigned long flags;
	struct GADGET_CTRL *priv;
	uint32_t gen_cfg;

	if (!gadget)
		return -EINVAL;

	config = container_of(gadget, struct phytium_cusb, gadget);

	priv = (struct GADGET_CTRL *)config->gadget_priv;
	if (priv->phy_regs) {
		gen_cfg = phytium_read32(&priv->phy_regs->gen_cfg);
		gen_cfg = gen_cfg | BIT(7);
		phytium_write32(&priv->phy_regs->gen_cfg, gen_cfg);
	}
	spin_lock_irqsave(&config->lock, flags);
	config->gadget_driver = NULL;
	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static struct usb_gadget_ops phytium_gadget_ops = {
	.get_frame	=	gadget_get_frame,
	.wakeup		=	gadget_wakeup,
	.vbus_session	=	gadget_vbus_session,
	.vbus_draw	=	gadget_vbus_draw,
	.pullup		=	gadget_pullup,
	.udc_start	=	gadget_udc_start,
	.udc_stop	=	gadget_udc_stop,
};

static int gadget_ep_enable(struct usb_ep *ls_ep, const struct usb_endpoint_descriptor *desc)
{
	struct phytium_ep *phy_ep = NULL;
	struct phytium_cusb *config;
	unsigned long flags;

	if (!ls_ep || !desc)
		return -EINVAL;

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;

	if (phy_ep->desc)
		return -EBUSY;

	spin_lock_irqsave(&config->lock, flags);

	phy_ep->desc = desc;
	phy_ep->busy = 0;
	config->gadget_obj->gadget_epEnable(config->gadget_priv, phy_ep->gadget_ep, desc);

	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static int gadget_ep_disable(struct usb_ep *ls_ep)
{
	struct phytium_ep *phy_ep = NULL;
	struct phytium_cusb *config;
	unsigned long flags;

	if (!ls_ep)
		return -EBUSY;

	pr_info("%s %d\n", __func__, __LINE__);

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;

	spin_lock_irqsave(&config->lock, flags);

	phy_ep->desc = NULL;
	phy_ep->busy = 0;
	phy_ep->end_point.desc = NULL;
	config->gadget_obj->gadget_epDisable(config->gadget_priv, phy_ep->gadget_ep);

	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static struct usb_request *gadget_ep_alloc_request(struct usb_ep *ls_ep, gfp_t gfp_flags)
{
	struct phytium_ep *phy_ep;
	struct phytium_cusb *config;
	struct GADGET_EP *gadget_ep;
	struct phytium_request *phy_request;

	if (!ls_ep)
		return NULL;

	pr_info("%s %d\n", __func__, __LINE__);
	phy_request = kzalloc(sizeof(*phy_request), gfp_flags);
	if (!phy_request) {
		pr_err("not enough momory\n");
		return NULL;
	}

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;
	gadget_ep = phy_ep->gadget_ep;

	INIT_LIST_HEAD(&phy_request->list);
	phy_request->request.dma = DMA_ADDR_INVALID;
	phy_request->epnum = phy_ep->ep_num;
	phy_request->ep = phy_ep;
	phy_request->config = phy_ep->config;

	config->gadget_obj->gadget_reqAlloc(config->gadget_priv, gadget_ep,
						&phy_request->gadget_request);

	return &phy_request->request;
}

static void gadget_ep_free_request(struct usb_ep *ls_ep, struct usb_request *ls_req)
{
	struct phytium_ep *phy_ep;
	struct phytium_cusb *config;
	struct phytium_request *phy_request;

	if (!ls_ep || !ls_req)
		return;

	pr_info("%s %d\n", __func__, __LINE__);
	phy_request = ls_req ? container_of(ls_req, struct phytium_request, request) : NULL;
	config = phy_request->config;
	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;

	config->gadget_obj->gadget_reqFree(config->gadget_priv, phy_ep->gadget_ep,
						phy_request->gadget_request);
	kfree(phy_request);
}

static int gadget_ep_enqueue(struct usb_ep *ls_ep, struct usb_request *ls_req, gfp_t gfp_flags)
{
	struct phytium_ep *phy_ep;
	struct phytium_cusb *config;
	struct phytium_request *phy_request;
	unsigned long flags;
	int status = 0;

	if (!ls_ep || !ls_req)
		return -EINVAL;

	if (!ls_req->buf)
		return -ENODATA;

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;
	phy_request = ls_req ? container_of(ls_req, struct phytium_request, request) : NULL;
	phy_request->config = config;

	if (phy_request->ep != phy_ep)
		return -EINVAL;

	phy_request->request.actual = 0;
	phy_request->request.status = -EINPROGRESS;
	phy_request->epnum = phy_ep->ep_num;
	phy_request->is_tx = phy_ep->is_tx;

	phy_request->gadget_request->length = ls_req->length;
	phy_request->gadget_request->status = 0;
	phy_request->gadget_request->complete = gadget_callback_complete;
	phy_request->gadget_request->buf = ls_req->buf;
	phy_request->gadget_request->context = ls_req;

	status = usb_gadget_map_request(&config->gadget, &phy_request->request, phy_request->is_tx);

	if (!phy_ep->desc) {
		pr_debug("req %p queued to %s while ep %s\n", phy_request, ls_ep->name, "disabled");
		status = -ESHUTDOWN;
		usb_gadget_unmap_request(&config->gadget, &phy_request->request,
					phy_request->is_tx);
		return status;
	}

	spin_lock_irqsave(&config->lock, flags);

	phy_request->gadget_request->dma = phy_request->request.dma;
	list_add_tail(&phy_request->list, &phy_ep->req_list);

	pr_debug("queue to %s (%s), length = %d\n", phy_ep->name,
			phy_ep->is_tx ? "IN/TX" : "OUT/RX", phy_request->request.length);

	status = config->gadget_obj->gadget_reqQueue(config->gadget_priv, phy_ep->gadget_ep,
							phy_request->gadget_request);

	spin_unlock_irqrestore(&config->lock, flags);

	return status;
}

static int gadget_ep_dequeue(struct usb_ep *ls_ep, struct usb_request *ls_req)
{
	struct phytium_ep *phy_ep;
	struct phytium_cusb *config;
	unsigned long flags;
	int status = 0;
	struct phytium_request *phy_request;
	struct phytium_request *phy_next_request;

	if (!ls_ep || !ls_req)
		return -EINVAL;

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;
	phy_request = ls_req ? container_of(ls_req, struct phytium_request, request) : NULL;

	if (phy_request->ep != phy_ep)
		return -EINVAL;

	spin_lock_irqsave(&config->lock, flags);

	list_for_each_entry(phy_next_request, &phy_ep->req_list, list) {
		if (phy_next_request == phy_request)
			break;
	}

	if (phy_next_request != phy_request) {
		pr_info("request %p not queued to %s\n", phy_request, ls_ep->name);
		status = -EINVAL;
		goto done;
	}

	status = config->gadget_obj->gadget_reqDequeue(config->gadget_priv, phy_ep->gadget_ep,
							phy_request->gadget_request);
done:
	spin_unlock_irqrestore(&config->lock, flags);
	return status;
}

static int gadget_ep_set_halt(struct usb_ep *ls_ep, int value)
{
	struct phytium_ep *phy_ep;
	struct phytium_cusb *config;
	struct GADGET_EP *gadget_ep = NULL;
	unsigned long flags;
	int status = 0;

	if (!ls_ep)
		return -EINVAL;

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;
	gadget_ep = phy_ep->gadget_ep;

	spin_lock_irqsave(&config->lock, flags);

	status = config->gadget_obj->gadget_epSetHalt(config->gadget_priv,
							phy_ep->gadget_ep, value);
	if (status > 0) {
		spin_unlock_irqrestore(&config->lock, flags);
		return -status;
	}

	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static const struct usb_ep_ops  gadget_ep_ops = {
	.enable		=	gadget_ep_enable,
	.disable	=	gadget_ep_disable,
	.alloc_request	=	gadget_ep_alloc_request,
	.free_request	=	gadget_ep_free_request,
	.queue		=	gadget_ep_enqueue,
	.dequeue	=	gadget_ep_dequeue,
	.set_halt	=	gadget_ep_set_halt,
};

static int gadget_ep0_enable(struct usb_ep *ep, const struct usb_endpoint_descriptor *desc)
{
	return -EINVAL;
}

static int gadget_ep0_disable(struct usb_ep *ep)
{
	return -EINVAL;
}

static int gadget_ep0_enqueue(struct usb_ep *ls_ep, struct usb_request *ls_req, gfp_t gfp_flags)
{
	struct phytium_ep *phy_ep;
	struct phytium_request *phy_request;
	struct phytium_cusb *config;
	int status = 0;
	unsigned long flags;

	if (!ls_ep || !ls_req)
		return -EINVAL;

	phy_ep = ls_ep ? container_of(ls_ep, struct phytium_ep, end_point) : NULL;
	config = phy_ep->config;
	phy_request = ls_req ? container_of(ls_req, struct phytium_request, request) : NULL;

	spin_lock_irqsave(&config->lock, flags);

	if (!list_empty(&phy_ep->req_list)) {
		status = -EBUSY;
		goto cleanup;
	}

	phy_request->config = config;
	phy_request->request.actual = 0;
	phy_request->gadget_request->actual = 0;
	phy_request->is_tx = config->ep0_data_stage_is_tx;
	phy_request->gadget_request->length = phy_request->request.length;
	phy_request->gadget_request->status = 0;
	phy_request->gadget_request->complete = gadget_callback_complete;
	phy_request->gadget_request->buf = phy_request->request.buf;
	phy_request->gadget_request->context = phy_request;

	status = usb_gadget_map_request(&config->gadget, &phy_request->request, phy_request->is_tx);
	if (status) {
		pr_info("failed to map request\n");
		status = -EINVAL;
		goto cleanup;
	}

	phy_request->gadget_request->dma = phy_request->request.dma;
	list_add_tail(&phy_request->list, &phy_ep->req_list);

	pr_debug("queue to %s (%s), length = %d\n", phy_ep->name,
			phy_ep->is_tx ? "IN/TX" : "OUT/RX", phy_request->request.length);

	status = config->gadget_obj->gadget_reqQueue(config->gadget_priv, phy_ep->gadget_ep,
							phy_request->gadget_request);
	if (status > 0) {
		status = -status;
		usb_gadget_unmap_request(&config->gadget, &phy_request->request,
					phy_request->is_tx);
		list_del(&phy_request->list);
		goto cleanup;
	}

cleanup:
	spin_unlock_irqrestore(&config->lock, flags);
	return status;
}

static int gadget_ep0_dequeue(struct usb_ep *ep, struct usb_request *ls_request)
{
	return -EOPNOTSUPP;
}

static int gadget_ep0_set_halt(struct usb_ep *ep, int value)
{
	return -EINVAL;
}


static const struct usb_ep_ops  gadget_ep0_ops = {
	.enable		=	gadget_ep0_enable,
	.disable	=	gadget_ep0_disable,
	.alloc_request	=	gadget_ep_alloc_request,
	.free_request	=	gadget_ep_free_request,
	.queue		=	gadget_ep0_enqueue,
	.dequeue	=	gadget_ep0_dequeue,
	.set_halt	=	gadget_ep0_set_halt,
};

static int32_t gadgetWaitForBusyBit(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep)
{
	struct GadgetEp *gadgetEp;
	uint8_t epNum;
	uint8_t txcs = CS_BUSY;
	uint8_t buf = 0;
	uint8_t bufflag = 0;

	if (!priv || !gadget_Ep)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	epNum = gadgetEp->hwEpNum;

	if (gadgetEp->isInEp || gadgetEp->hwEpNum == 0)
		return 0;

	buf = phytium_read8(&priv->regs->ep[epNum - 1].txcon) & CON_BUF;

	while ((txcs & CS_BUSY) || (bufflag == 0)) {
		txcs = phytium_read8(&priv->regs->ep[epNum - 1].txcs);

		if (((txcs & CS_NPAK) >> CS_NPAK_OFFSET) == buf || buf == 0)
			bufflag = 1;
		else
			bufflag = 0;
	}

	return 0;
}

static inline void gadgetEpXDataReceive(struct GADGET_CTRL *priv,
					struct GadgetRequest *gadgetRequest)
{
	struct GadgetEp *gadgetEp;
	struct GADGET_REQ *gadgetReq;
	uint8_t epType;
	uint32_t requestSize, channelStatus, chMaxLen;

	if (!priv || !gadgetRequest)
		return;

	gadgetEp = gadgetRequest->ep;
	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, gadgetEp->channel);
	epType = gadgetEp->gadgetEp.desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	channelStatus = priv->dmaDrv->dma_getChannelStatus(priv->dmaController, gadgetEp->channel);

	gadgetReq = &gadgetRequest->request;
	if (gadgetReq->actual < gadgetReq->length || gadgetRequest->zlp) {
		gadgetRequest->zlp = 0;
		if ((gadgetReq->length - gadgetReq->actual) < chMaxLen)
			requestSize = gadgetReq->length - gadgetReq->actual;
		else
			requestSize = chMaxLen;

		priv->dmaDrv->dma_channelProgram(priv->dmaController, gadgetEp->channel,
				gadgetEp->gadgetEp.maxPacket,
				gadgetReq->dma + gadgetReq->actual, requestSize, NULL, 0);
	}
}

static inline void gadgetEpXDataSend(struct GADGET_CTRL *priv, struct GadgetRequest *gadgetRequest)
{
	struct GadgetEp *gadgetEp;
	struct GADGET_REQ *gadgetReq;
	uint8_t epType;
	uint32_t requestSize, channelStatus, chMaxLen;

	if (!priv || !gadgetRequest)
		return;

	gadgetEp = gadgetRequest->ep;
	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, gadgetEp->channel);
	epType = gadgetEp->gadgetEp.desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	channelStatus = priv->dmaDrv->dma_getChannelStatus(priv->dmaController, gadgetEp->channel);

	gadgetReq = &gadgetRequest->request;
	if ((gadgetReq->length - gadgetReq->actual) < chMaxLen)
		requestSize = gadgetReq->length - gadgetReq->actual;
	else
		requestSize = chMaxLen;
	pr_debug("Transmit/IN %s gadgetReq %p gadgetRequest:%p requestSize:0x%x packetSize:0x%x\n",
		       gadgetEp->gadgetEp.name, gadgetReq, gadgetRequest,
		       requestSize, gadgetEp->gadgetEp.maxPacket);

	gadgetRequest->zlp = 0;
	priv->dmaDrv->dma_channelProgram(priv->dmaController, gadgetEp->channel,
				gadgetEp->gadgetEp.maxPacket,
				gadgetReq->dma + gadgetReq->actual, requestSize, NULL, 0);
}

static int32_t gadgetEpXSetHalt(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
				uint8_t value)
{
	struct GadgetEp *gadgetEp;
	uint8_t epType;
	struct GADGET_REQ *req = NULL;
	struct GadgetRequest *gadgetRequest = NULL;
	uint8_t epNum, txcon, rxcon;
	uint32_t status = DMA_STATUS_ARMED;

	if (!priv || !gadget_Ep)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	epType = gadgetEp->gadgetEp.desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	if (epType == USB_ENDPOINT_XFER_ISOC)
		return -EINVAL;

	pr_debug("%s: %s stall\n", gadget_Ep->name, value ? "set" : "clear");
	req = gadgetGetNextReq(gadgetEp);

	if (!value)
		gadgetEp->wedged = 0;

	if (value && gadgetEp->isInEp && req && gadgetEp->state == GADGET_EP_BUSY) {
		while (status == DMA_STATUS_ARMED)
			status = priv->dmaDrv->dma_getChannelStatus(priv->dmaController,
									gadgetEp->channel);

		gadgetWaitForBusyBit(priv, gadget_Ep);
	}

	epNum = gadgetEp->hwEpNum;

	if (gadgetEp->isInEp) {
		txcon = phytium_read8(&priv->regs->ep[epNum - 1].txcon);
		if (value) {
			phytium_write8(&priv->regs->ep[epNum - 1].txcon, txcon | CON_STALL);
			phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum);
			phytium_write8(&priv->regs->endprst,
					ENDPRST_IO_TX | epNum | ENDPRST_FIFORST);

		} else {
			phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum);
			phytium_write8(&priv->regs->endprst,
					ENDPRST_IO_TX | epNum | ENDPRST_TOGRST);
			phytium_write8(&priv->regs->ep[epNum - 1].txcon, txcon & (~CON_STALL));
		}
	} else {
		rxcon = phytium_read8(&priv->regs->ep[epNum - 1].rxcon);
		if (value) {
			phytium_write8(&priv->regs->ep[epNum - 1].rxcon, rxcon | CON_STALL);
			phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum);
			phytium_write8(&priv->regs->endprst, epNum | ENDPRST_FIFORST);
		} else {
			phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum);
			phytium_write8(&priv->regs->endprst,
					epNum | ENDPRST_TOGRST | ENDPRST_FIFORST);
			phytium_write8(&priv->regs->ep[epNum - 1].rxcon, rxcon & (~CON_STALL));
		}
	}

	if (gadgetEp->state != GADGET_EP_BUSY && !value && req) {
		gadgetRequest = requestToGadgetRequest(req);
		if (gadgetEp->isInEp)
			gadgetEpXDataSend(priv, gadgetRequest);
		else
			gadgetEpXDataReceive(priv, gadgetRequest);
	}

	return 0;
}

static int32_t gadgetEp0SetHalt(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
				uint8_t value)
{
	struct GadgetEp *gadgetEp;

	if (!priv || !gadget_Ep)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	if (!list_empty(&gadgetEp->request))
		return -EBUSY;

	switch (priv->ep0State) {
	case GADGET_EP0_STAGE_IN:
	case GADGET_EP0_STAGE_OUT:
	case GADGET_EP0_STAGE_ACK:
	case GADGET_EP0_STAGE_STATUSIN:
	case GADGET_EP0_STAGE_STATUSOUT:
		priv->ep0State = GADGET_EP0_STAGE_SETUP;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


static void gadgetEp0Callback(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req, uint8_t status)
{
	if (!priv || !gadgetEp || !req)
		return;

	priv->ep0State = GADGET_EP0_STAGE_SETUP;
	list_del(&req->list);
	gadgetEp->requestsInList--;

	if (req->status == EINPROGRESS)
		req->status = status;

	if (req->complete)
		req->complete(&gadgetEp->gadgetEp, req);
}

static enum usb_device_speed gadgetGetActualSpeed(struct GADGET_CTRL *priv)
{
	uint8_t speedctrl;

	if (!priv)
		return USB_SPEED_UNKNOWN;

	speedctrl = phytium_read8(&priv->regs->speedctrl) & (~SPEEDCTRL_HSDISABLE);
	switch (speedctrl) {
	case SPEEDCTRL_HS:
		return USB_SPEED_HIGH;
	case SPEEDCTRL_FS:
		return USB_SPEED_FULL;
	case SPEEDCTRL_LS:
		return USB_SPEED_LOW;
	default:
		return USB_SPEED_UNKNOWN;
	}
}

static int32_t gadgetServiceSetFeatureReq(struct GADGET_CTRL *priv, struct usb_ctrlrequest *setup)
{
	uint8_t epNum, isIn;
	struct GadgetEp *gadgetEp;

	if (!priv || !setup)
		return 0;

	switch (setup->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		switch (setup->wValue) {
		case USB_DEVICE_REMOTE_WAKEUP:
			pr_info("set feature - remote wakup\n");
			priv->isRemoteWakeup = 1;
			break;
		case USB_DEVICE_B_HNP_ENABLE:
			pr_info("set feature - B HNP Enable\n");
			pr_info("otg not implement\n");
			return -EINVAL;
		case USB_DEVICE_A_HNP_SUPPORT:
			pr_info("set feature - A HNP support\n");
			pr_info("otg not implete\n");
			return -EINVAL;
		}
		break;
	case USB_RECIP_INTERFACE:
		break;
	case USB_RECIP_ENDPOINT:
		epNum = setup->wIndex & 0x0f;
		isIn = setup->wIndex & USB_DIR_IN;
		if (epNum == 0 || epNum > 15 || setup->wValue != 0)
			return -EINVAL;

		gadgetEp = isIn ? &priv->in[epNum] : &priv->out[epNum];

		gadgetEpXSetHalt(priv, &gadgetEp->gadgetEp, 1);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int32_t gadgetServiceClearFeatureReq(struct GADGET_CTRL *priv, struct usb_ctrlrequest *setup)
{
	uint8_t epNum, isIn;
	struct GadgetEp *gadgetEp;

	if (!priv || !setup)
		return -EINVAL;

	switch (setup->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		if (setup->wValue == USB_DEVICE_B_HNP_ENABLE) {
			pr_err("otg not implement\n");
			return -EINVAL;
		}

		if (setup->wValue != USB_DEVICE_REMOTE_WAKEUP)
			return GADGET_EUNHANDLED;

		priv->isRemoteWakeup = 0;
		return GADGET_EAUTOACK;
	case USB_RECIP_INTERFACE:
		break;
	case USB_RECIP_ENDPOINT:
		pr_info("clear feature wIndex:0x%x wValue:0x%x\n", setup->wIndex, setup->wValue);
		epNum = setup->wIndex & 0x7f;
		isIn = setup->wIndex & USB_DIR_IN;
		if (epNum == 0 || epNum > 15 || setup->wValue != 0)
			return GADGET_EUNHANDLED;

		gadgetEp = isIn ? &priv->in[epNum] : &priv->out[epNum];
		if (!gadgetEp->gadgetEp.desc)
			return -EINVAL;

		if (gadgetEp->wedged)
			break;
		gadgetEpXSetHalt(priv, &gadgetEp->gadgetEp, 0);
		break;
	default:
		return GADGET_EUNHANDLED;
	}

	return 0;
}

static int32_t gadgetServiceSetupReq(struct GADGET_CTRL *priv, struct usb_ctrlrequest *setup)
{
	struct GadgetEp *gadgetEp;
	uint8_t isIn, epNum;
	uint8_t rxcon, txcon;
	int len;

	if (!priv || !setup)
		return -EINVAL;

	if ((setup->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		if (setup->bRequest != USB_REQ_GET_STATUS)
			return GADGET_EUNHANDLED;
	} else
		return GADGET_EUNHANDLED;

	priv->privBuffAddr[1] = 0;
	priv->privBuffAddr[0] = 0;

	switch (setup->bRequest & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		pr_info("wIndex:0x%x isSelfPowered:%d isRomoteWakeup:%d\n",
				setup->wIndex, priv->isSelfPowered, priv->isRemoteWakeup);

		if (setup->wIndex == OTG_STS_SELECTOR)
			priv->privBuffAddr[0] = priv->hostRequestFlag;
		else {
			priv->privBuffAddr[0] = priv->isSelfPowered ? USB_DEVICE_SELF_POWERED : 0;
			priv->privBuffAddr[1] = priv->isRemoteWakeup ? USB_DEVICE_REMOTE_WAKEUP : 0;
		}
		break;
	case USB_RECIP_INTERFACE:
		break;
	case USB_RECIP_ENDPOINT:
		epNum = setup->wIndex & 0x0f;
		if (!epNum)
			break;

		isIn = setup->wIndex & USB_DIR_IN;

		gadgetEp = isIn ? &priv->in[epNum] : &priv->out[epNum];
		if (!gadgetEp->gadgetEp.desc)
			return -EINVAL;

		if (isIn) {
			txcon = phytium_read8(&priv->regs->ep[epNum - 1].txcon);
			priv->privBuffAddr[0] = (txcon & CON_STALL) ? 1 : 0;
		} else {
			rxcon = phytium_read8(&priv->regs->ep[epNum - 1].rxcon);
			priv->privBuffAddr[0] = (rxcon & CON_STALL) ? 1 : 0;
		}
		break;
	default:
		return GADGET_EUNHANDLED;
	}

	len = setup->wLength;
	if (len > 2)
		len = 2;

	priv->dmaDrv->dma_channelProgram(priv->dmaController, priv->in[0].channel,
			priv->in[0].gadgetEp.maxPacket, priv->privBuffDma, len, NULL, 0);

	return 0;
}

static int32_t gadgetGetSetup(struct GADGET_CTRL *priv, struct usb_ctrlrequest *setup)
{
	int i;
	uint8_t ep0cs;
	struct GADGET_REQ *request = NULL;
	struct GadgetRequest *gadgetRequest;

	if (!priv || !setup)
		return -EINVAL;

	phytium_write8(&priv->regs->ep0cs, EP0CS_CHGSET);

	for (i = 0; i < 8; i++)
		((char *)setup)[i] = phytium_read8(&priv->regs->setupdat[i]);

	ep0cs = phytium_read8(&priv->regs->ep0cs);
	if (ep0cs & EP0CS_CHGSET) {
		pr_info("setup flags change: not error\n");
		return GADGET_EAUTOACK;
	}

	phytium_write8(&priv->regs->usbirq, USBIR_SUDAV);
	pr_debug("setup packet: req%02x.%02x v:%04x i:%04x I%d\n", setup->bRequestType,
			setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);

	request = gadgetGetNextEp0Req(priv);
	if (request) {
		gadgetRequest = requestToGadgetRequest(request);
		pr_info("Previous request has not been finished but new was received\n");
		gadgetEp0Callback(priv, gadgetRequest->ep, request, 0);
	}

	if (setup->wLength) {
		if (setup->bRequestType & USB_DIR_IN)
			priv->ep0State = GADGET_EP0_STAGE_IN;
		else
			priv->ep0State = GADGET_EP0_STAGE_OUT;
	} else
		priv->ep0State = GADGET_EP0_STAGE_ACK;

	return 0;
}

static void gadgetEp0StageSetup(struct GADGET_CTRL *priv)
{
	struct usb_ctrlrequest setup;
	int32_t retval;
	uint8_t ep0cs;

	if (!priv)
		return;

	retval = gadgetGetSetup(priv, &setup);

	priv->gadgetDev.speed = gadgetGetActualSpeed(priv);

	switch (priv->ep0State) {
	case GADGET_EP0_STAGE_ACK:
		if ((setup.bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD)
			retval = GADGET_EUNHANDLED;
		else {
			switch (setup.bRequest) {
			case USB_REQ_SET_ADDRESS:
				priv->deviceAddress = setup.wValue & 0x7F;
				priv->gadgetDev.state = USB_STATE_ADDRESS;
				pr_info("set address: %d\n", priv->deviceAddress);
				retval = GADGET_EAUTOACK;
				break;
			case USB_REQ_SET_FEATURE:
				retval = gadgetServiceSetFeatureReq(priv, &setup);
				break;
			case USB_REQ_CLEAR_FEATURE:
				retval = gadgetServiceClearFeatureReq(priv, &setup);
				break;
			default:
				retval = GADGET_EUNHANDLED;
				break;
			}
		}

		if (retval == GADGET_EUNHANDLED)
			break;
		else if (retval == 0)
			phytium_write8(&priv->regs->ep0cs, EP0CS_HSNAK);

		priv->ep0State = GADGET_EP0_STAGE_SETUP;
		break;
	case GADGET_EP0_STAGE_IN:
		pr_debug("setup data stage in\n");
		retval = gadgetServiceSetupReq(priv, &setup);

		if (retval == 0)
			priv->ep0State = GADGET_EP0_STAGE_STATUSOUT;
		break;
	case GADGET_EP0_STAGE_OUT:
		pr_debug("setup data stage out\n");
		phytium_write8(&priv->regs->ep0Rxbc, 0);
		retval = GADGET_EUNHANDLED;
		break;
	default:
		if (retval == GADGET_EAUTOACK)
			return;

		pr_debug("forward request\n");
		retval = GADGET_EUNHANDLED;
		break;
	}

	if (retval == GADGET_EUNHANDLED) {
		if (priv->eventCallback.setup)
			retval = priv->eventCallback.setup(priv, &setup);

		if (retval == 0x7FFF) {
			pr_debug("Respond Delayed not finished yet\n");
			return;
		}

		if (retval)
			retval = GADGET_ESTALL;
	}

	if (retval == GADGET_EUNHANDLED || retval == GADGET_ESTALL) {
		pr_debug("request not handled - send stall\n");
		ep0cs = phytium_read8(&priv->regs->ep0cs);
		ep0cs |= EP0CS_STALL;
		phytium_write8(&priv->regs->ep0cs, ep0cs);
		priv->ep0State = GADGET_EP0_STAGE_SETUP;
	} else if (priv->ep0State == GADGET_EP0_STAGE_ACK) {
		priv->ep0State = GADGET_EP0_STAGE_SETUP;
		phytium_write8(&priv->regs->ep0cs, EP0CS_HSNAK);
		pr_debug("setup transfer completed\n");
	}
}
static void gadgetEp0DataSend(struct GADGET_CTRL *priv)
{
	struct GADGET_REQ *request;
	uint32_t chMaxLen, requestSize;

	if (!priv)
		return;

	request = gadgetGetNextEp0Req(priv);
	if (!request) {
		pr_debug("Ep0 queue is empty\n");
		return;
	}

	if (priv->dmaDrv->dma_getChannelStatus(priv->dmaController, priv->in[0].channel)
						>= DMA_STATUS_BUSY) {
		pr_err("transfer is pending now\n");
		return;
	}

	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, priv->in[0].channel);
	requestSize = (request->length < chMaxLen) ? request->length : chMaxLen;
	pr_debug("usbRequest;%p requestSize:%d packetSize:%d\n", request, requestSize,
			priv->in[0].gadgetEp.maxPacket);
	priv->ep0State = GADGET_EP0_STAGE_STATUSOUT;

	priv->dmaDrv->dma_channelProgram(priv->dmaController, priv->in[0].channel,
			priv->in[0].gadgetEp.maxPacket, request->dma, requestSize, NULL, 0);
}

static void gadgetEp0DataReceive(struct GADGET_CTRL *priv)
{
	uint32_t chMaxLen, requestSize;
	struct GADGET_REQ *request;

	if (!priv)
		return;

	request = gadgetGetNextEp0Req(priv);
	if (!request) {
		pr_debug("Ep0 queue is empty\n");
		return;
	}

	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, priv->out[0].channel);
	requestSize = (request->length < chMaxLen) ? request->length : chMaxLen;
	pr_debug("usbRequest;%p requestSize:%d packetSize:%d\n", request, requestSize,
			priv->out[0].gadgetEp.maxPacket);
	priv->ep0State = GADGET_EP0_STAGE_STATUSIN;

	priv->dmaDrv->dma_channelProgram(priv->dmaController, priv->out[0].channel,
			priv->out[0].gadgetEp.maxPacket, request->dma, requestSize, NULL, 0);
}

static uint32_t gadgetEp0Irq(struct GADGET_CTRL *priv)
{
	uint8_t usbcs;
	struct GADGET_REQ *request;

	if (!priv)
		return 0;

	switch (priv->ep0State) {
	case GADGET_EP0_STAGE_IN://send data
		pr_debug("DATA Stage IN\n");
		gadgetEp0DataSend(priv);
		break;
	case GADGET_EP0_STAGE_OUT://receive data
		pr_debug("DATA Stage OUT\n");
		gadgetEp0DataReceive(priv);
		break;
	case GADGET_EP0_STAGE_STATUSIN:
		pr_debug("DATA Stage STATUS IN\n");
		request = gadgetGetNextEp0Req(priv);
		if (request)
			request->actual = priv->dmaDrv->dma_getActualLength(priv->dmaController,
								priv->out[0].channel);

		priv->ep0State = GADGET_EP0_STAGE_SETUP;
		phytium_write8(&priv->regs->ep0cs, EP0CS_HSNAK);
		gadgetEp0Callback(priv, &priv->out[0], request, 0);
		break;
	case GADGET_EP0_STAGE_STATUSOUT:
		pr_debug("DATA Stage STATUS OUT\n");
		request = gadgetGetNextEp0Req(priv);
		if (request)
			request->actual = priv->dmaDrv->dma_getActualLength(priv->dmaController,
								priv->in[0].channel);

		phytium_write8(&priv->regs->ep0cs, EP0CS_HSNAK);
		if (request)
			gadgetEp0Callback(priv, &priv->in[0], request, 0);
		else
			priv->ep0State = GADGET_EP0_STAGE_SETUP;
		break;
	case GADGET_EP0_STAGE_SETUP:
		pr_debug("DATA Stage SETUP\n");
		gadgetEp0StageSetup(priv);
		break;
	case GADGET_EP0_STAGE_ACK:
		pr_debug("DATA Stage ACK\n");
		break;
	default:
		pr_debug("DATA Stage UNKNOWN\n");
		usbcs = phytium_read8(&priv->regs->usbcs);
		usbcs |= EP0CS_STALL;
		phytium_write8(&priv->regs->usbcs, usbcs);
		break;
	}

	return 0;
}

static void gadgetEpXCallback(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req, uint32_t status)
{
	if (!gadgetEp || !req)
		return;

	list_del(&req->list);
	gadgetEp->requestsInList--;
	req->status = status;

	if (req->complete)
		req->complete(&gadgetEp->gadgetEp, req);
}

static void gadgetEpXDataCallback(struct GADGET_CTRL *priv, uint8_t epNum, uint8_t epDir)
{
	struct GadgetEp *gadgetEp;
	struct GADGET_REQ *gadgetReq;
	uint32_t actual_length = 0;
	uint32_t chMaxLen = 0;
	uint8_t epType;

	if (!priv)
		return;

	pr_debug("%s %d epNum:%d epDir:%d\n", __func__, __LINE__, epNum, epDir);
	gadgetEp = epDir ? &priv->in[epNum] : &priv->out[epNum];

	gadgetReq = gadgetGetNextReq(gadgetEp);
	if (!gadgetReq) {
		pr_debug("%s queue is empty\n", gadgetEp->gadgetEp.name);
		return;
	}

	if (gadgetEp->channel) {
		actual_length = priv->dmaDrv->dma_getActualLength(priv->dmaController,
				gadgetEp->channel);
		chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, gadgetEp->channel);
		gadgetReq->actual += actual_length;
	}

	if (gadgetReq->actual == gadgetReq->length || actual_length < chMaxLen) {
		gadgetEpXCallback(priv, gadgetEp, gadgetReq, 0);

		if (gadgetEp->gadgetEp.desc) {
			epType = gadgetEp->gadgetEp.desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
			if (epType == USB_ENDPOINT_XFER_ISOC)
				return;

			gadgetReq = gadgetGetNextReq(gadgetEp);
			gadgetEp->state = GADGET_EP_ALLOCATED;
			if (!gadgetReq) {
				pr_debug("%s queue is empty\n", gadgetEp->gadgetEp.name);
				return;
			}
			gadgetEp->state = GADGET_EP_BUSY;
			if (epDir)
				gadgetEpXDataSend(priv, requestToGadgetRequest(gadgetReq));
			else
				gadgetEpXDataReceive(priv, requestToGadgetRequest(gadgetReq));
		}
	} else {
		if (epDir)
			gadgetEpXDataSend(priv, requestToGadgetRequest(gadgetReq));
		else
			gadgetEpXDataReceive(priv, requestToGadgetRequest(gadgetReq));
	}
}

void gadget_CallbackTransfer(void *priv, uint8_t epNum, uint8_t epDir, bool resubmit)
{
	if (!epNum)
		gadgetEp0Irq(priv);
	else
		gadgetEpXDataCallback(priv, epNum, epDir);
}

static void gadgetAbortEndpoint(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp)
{
	struct GADGET_REQ *gadgetReq;

	pr_debug("Abort Device endpoint: %s, dma channel: %p\n", gadgetEp->gadgetEp.name,
			gadgetEp->channel);

	if (gadgetEp->channel && gadgetEp->hwEpNum != 0) {
		priv->dmaDrv->dma_channelRelease(priv->dmaController, gadgetEp->channel);
		gadgetEp->channel = NULL;
	}

	if (gadgetEp->channel && gadgetEp->hwEpNum == 0) {
		if (priv->releaseEp0Flag == 1) {
			priv->dmaDrv->dma_channelAbort(priv->dmaController, gadgetEp->channel);
			priv->dmaDrv->dma_channelAlloc(priv->dmaController, gadgetEp->isInEp,
					gadgetEp->hwEpNum, 0);
		}
	}

	while (gadgetEp->request.next != &gadgetEp->request) {
		gadgetReq = listToGadgetRequest(gadgetEp->request.next);
		pr_debug("shutdown request %p form epName:%s\n", gadgetReq,
				gadgetEp->gadgetEp.name);
		if (!gadgetEp->gadgetEp.address)
			gadgetEp0Callback(priv, gadgetEp, gadgetReq, GADGET_ESHUTDOWN);
		else
			gadgetEpXCallback(priv, gadgetEp, gadgetReq, GADGET_ESHUTDOWN);
	}

	gadgetEp->state = GADGET_EP_FREE;
}

static void gadgetStopActivity(struct GADGET_CTRL *priv)
{
	int i = 0;
	struct GadgetEp *gadgetEp;

	pr_debug("USB Stop Activity\n");

	if (!priv)
		return;

	for (i = 0; i < 16; i++) {
		gadgetEp = &priv->in[i];
		if (gadgetEp->state != GADGET_EP_NOT_IMPLEMENTED)
			gadgetAbortEndpoint(priv, gadgetEp);

		gadgetEp = &priv->out[i];
		if (gadgetEp->state != GADGET_EP_NOT_IMPLEMENTED)
			gadgetAbortEndpoint(priv, gadgetEp);
	}
}


static int32_t gadgetEp0Enable(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp)
{
	uint8_t txien, rxien;

	if (!priv || !gadgetEp)
		return -EINVAL;

	if (gadgetEp->state != GADGET_EP_FREE)
		return -EBUSY;

	if (!gadgetEp->isInEp) {
		rxien = phytium_read16(&priv->regs->rxien);
		rxien &= ~(1 << gadgetEp->hwEpNum);
		phytium_write16(&priv->regs->rxien, rxien);
		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_IO_TX | FIFOCTRL_FIFOAUTO);
	} else {
		txien = phytium_read16(&priv->regs->txien);
		txien &= ~(1 << gadgetEp->hwEpNum);
		phytium_write16(&priv->regs->txien, txien);
		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO);
	}

	gadgetEp->gadgetEp.desc = NULL;
	gadgetEp->state = GADGET_EP_ALLOCATED;
	gadgetEp->channel = priv->dmaDrv->dma_channelAlloc(priv->dmaController,
			gadgetEp->isInEp, gadgetEp->hwEpNum, 0);
	phytium_write8(&priv->regs->ep0maxpack, 0x40);

	return 0;
}

static int32_t gadgetEpXEnable(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		const struct usb_endpoint_descriptor *desc)
{
	uint32_t status = -EINVAL;
	uint32_t payload;
	uint16_t type, iso = 0;
	uint8_t epNum = 0;

	if (!priv || !gadgetEp || !desc)
		return -EINVAL;

	pr_debug("enable endpoint %s\n", gadgetEp->gadgetEp.name);
	if (gadgetEp->state != GADGET_EP_FREE) {
		status = -EBUSY;
		goto fail;
	}

	payload = desc->wMaxPacketSize & 0x7ff;
	if (!payload) {
		status = -EINVAL;
		goto fail;
	}

	epNum = gadgetEp->hwEpNum;

	type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	switch (type) {
	case USB_ENDPOINT_XFER_ISOC:
		type = CON_TYPE_ISOC;
		switch (payload >> 11) {
		case 0:
			iso = CON_TYPE_ISOC_1_ISOD;
			break;
		case 1:
			payload *= 2;
			iso = CON_TYPE_ISOC_2_ISOD;
			break;
		case 2:
			payload *= 3;
			iso = CON_TYPE_ISOC_3_ISOD;
			break;
		}
		break;
	case USB_ENDPOINT_XFER_INT:
		type = CON_TYPE_INT;
		break;
	case USB_ENDPOINT_XFER_BULK:
		type = CON_TYPE_BULK;
		break;
	}

	if (desc->bEndpointAddress & USB_DIR_IN) {
		if (!gadgetEp->isInEp) {
			status = -ENODEV;
			goto fail;
		}

		if (payload > priv->gadgetCfg.epIN[epNum].maxPacketSize) {
			status = -EINVAL;
			goto fail;
		}

		phytium_write16(&priv->regs->txmaxpack[epNum - 1], payload);
		phytium_write8(&priv->regs->ep[epNum - 1].txcon, CON_VAL | type
				| iso | (priv->gadgetCfg.epIN[epNum - 1].bufferingValue - 1));

		phytium_write8(&priv->regs->fifoctrl, FIFOCTRL_FIFOAUTO | FIFOCTRL_IO_TX | epNum);
		phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum);
		phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX | epNum |
				ENDPRST_FIFORST | ENDPRST_TOGRST);
	} else {
		if (gadgetEp->isInEp) {
			status = -ENODEV;
			goto fail;
		}

		if (payload > priv->gadgetCfg.epOUT[epNum].maxPacketSize) {
			status = -EINVAL;
			goto fail;
		}

		phytium_write16(&priv->regs->rxmaxpack[epNum - 1], payload);
		phytium_write8(&priv->regs->ep[epNum - 1].rxcon, CON_VAL | type
				| iso | (priv->gadgetCfg.epIN[epNum - 1].bufferingValue - 1));

		phytium_write8(&priv->regs->fifoctrl, FIFOCTRL_FIFOAUTO | epNum);
		phytium_write8(&priv->regs->endprst, epNum);
		phytium_write8(&priv->regs->endprst, epNum | ENDPRST_FIFORST | ENDPRST_TOGRST);
	}

	if (priv->dmaController)
		gadgetEp->channel = priv->dmaDrv->dma_channelAlloc(priv->dmaController,
				gadgetEp->isInEp, epNum, (type == CON_TYPE_ISOC) ? 1 : 0);

	if (type == CON_TYPE_ISOC) {
		if (gadgetEp->isInEp) {
			phytium_write16(&priv->regs->isoautodump, 1 << epNum);
			phytium_write16(&priv->regs->isodctrl, 1 << epNum);
		}

		priv->dmaDrv->dma_setMaxLength(priv->dmaController, gadgetEp->channel, payload);
	}

	gadgetEp->state = GADGET_EP_ALLOCATED;
	gadgetEp->gadgetEp.desc = desc;
fail:
	return status;
}

static int32_t gadgetEpEnable(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct GadgetEp *gadgetEp = NULL;

	if (!priv || !gadget_Ep || !desc)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	gadgetEp->wedged = 0;

	if (gadgetEp->hwEpNum)
		return gadgetEpXEnable(priv, gadgetEp, desc);
	else
		return gadgetEp0Enable(priv, gadgetEp);
}

static int32_t gadgetEpXDisable(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp)
{
	uint8_t txcon, rxcon;

	if (!priv || !gadgetEp)
		return -EINVAL;

	pr_debug("disable endpoint %s\n", gadgetEp->gadgetEp.name);
	if (gadgetEp->isInEp) {
		txcon = phytium_read8(&priv->regs->ep[gadgetEp->hwEpNum - 1].txcon);
		txcon &= ~CON_VAL;
		phytium_write8(&priv->regs->ep[gadgetEp->hwEpNum - 1].txcon, txcon);
	} else {
		rxcon = phytium_read8(&priv->regs->ep[gadgetEp->hwEpNum - 1].rxcon);
		rxcon &= ~CON_VAL;
		phytium_write8(&priv->regs->ep[gadgetEp->hwEpNum - 1].rxcon, rxcon);
	}
	gadgetAbortEndpoint(priv, gadgetEp);
	gadgetEp->gadgetEp.desc = 0;
	gadgetEp->state = GADGET_EP_FREE;
	return 0;
}

static int32_t gadgetEp0Disable(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp)
{
	if (!priv || !gadgetEp)
		return -EINVAL;

	pr_debug("disable endpoint %s\n", gadgetEp->gadgetEp.name);
	gadgetAbortEndpoint(priv, gadgetEp);

	return 0;
}

static int32_t gadgetEpDisable(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep)
{
	struct GadgetEp *gadgetEp = NULL;

	if (!priv || !gadget_Ep)
		return -EINVAL;

	if (gadget_Ep->address == 0)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	gadgetEp->wedged = 0;
	if (gadgetEp->hwEpNum)
		return gadgetEpXDisable(priv, gadgetEp);
	else
		return gadgetEp0Disable(priv, gadgetEp);
}

static int32_t gadgetEpXQueue(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req)
{
	struct GadgetRequest *gadgetRequest;

	if (!priv || !gadgetEp || !req)
		return -EINVAL;

	req->actual = 0;
	req->status = EINPROGRESS;

	gadgetRequest = requestToGadgetRequest(req);
	gadgetRequest->ep = gadgetEp;

	if (req->length == 0)
		gadgetRequest->zlp = 1;

	if (gadgetEp->gadgetEp.desc == NULL) {
		pr_info("%s is disabled - can not queue request %p\n",
				gadgetEp->gadgetEp.name, req);
		return -EINVAL;
	}

	list_add_tail(&req->list, &gadgetEp->request);
	pr_debug("queue to %s (%s), length:%d\n", gadgetEp->gadgetEp.name,
			(gadgetEp->isInEp ? "IN/TX" : "OUT/RX"), req->length);

	if ((gadgetEp->state == GADGET_EP_ALLOCATED) && (&req->list == gadgetEp->request.next)) {
		if (gadgetEp->isInEp) {
			if (!(phytium_read8(&priv->regs->ep[gadgetEp->hwEpNum - 1].txcon)
						& CON_STALL)) {
				gadgetEp->state = GADGET_EP_BUSY;
				gadgetEpXDataSend(priv, gadgetRequest);
			}
		} else {
			if (!(phytium_read8(&priv->regs->ep[gadgetEp->hwEpNum - 1].rxcon)
						& CON_STALL)) {
				gadgetEp->state = GADGET_EP_BUSY;
				gadgetEpXDataReceive(priv, gadgetRequest);
			}
		}
	} else if (gadgetEp->state == GADGET_EP_BUSY) {
		if (usb_endpoint_xfer_isoc(gadgetEp->gadgetEp.desc)) {
			if (gadgetEp->isInEp)
				gadgetEpXDataSend(priv, gadgetRequest);
			else
				gadgetEpXDataReceive(priv, gadgetRequest);
		}
	}

	pr_debug("endpoint %s (%s) now is busy - transfer will be waiting in Queue\n",
			gadgetEp->gadgetEp.name, (gadgetEp->isInEp ? "IN/TX" : "OUT/RX"));

	return 0;
}

static int32_t gadgetEp0Queue(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req)
{
	struct GadgetRequest *gadgetRequest;

	if (!priv || !gadgetEp || !req)
		return -EINVAL;

	req->actual = 0;
	req->status = EINPROGRESS;

	if (!list_empty(&gadgetEp->request))
		return -EBUSY;

	gadgetRequest = requestToGadgetRequest(req);
	gadgetRequest->ep = gadgetEp;

	switch (priv->ep0State) {
	case GADGET_EP0_STAGE_OUT:
	case GADGET_EP0_STAGE_IN:
	case GADGET_EP0_STAGE_ACK:
		break;
	default:
		return -EINVAL;
	}

	list_add_tail(&req->list, &gadgetEp->request);
	gadgetEp->requestsInList++;

	pr_debug("queue to %s (%s), length:%d stage:%d\n", gadgetEp->gadgetEp.name,
			gadgetEp->isInEp ? "IN/TX" : "OUT/RX", req->length, priv->ep0State);

	switch (priv->ep0State) {
	case GADGET_EP0_STAGE_OUT:
		gadgetEp0DataReceive(priv);
		break;
	case GADGET_EP0_STAGE_IN:
		gadgetEp0DataSend(priv);
		break;
	case GADGET_EP0_STAGE_ACK:
		if (req->length)
			return -EINVAL;
		phytium_write8(&priv->regs->ep0cs, EP0CS_HSNAK);
		gadgetEp0Callback(priv, gadgetRequest->ep, req, 0);
		pr_info("control transfer completed\n");
		break;
	default:
		break;
	}

	return 0;
}

static int32_t gadgetEpQueue(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
		struct GADGET_REQ *req)
{
	struct GadgetEp *gadgetEp = NULL;

	if (!priv || !gadget_Ep || !req)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);

	if (gadget_Ep->address & GADGET_USB_EP_NUMBER_MASK)
		return gadgetEpXQueue(priv, gadgetEp, req);
	else
		return gadgetEp0Queue(priv, gadgetEp, req);
}

static int32_t gadgetEpXDequeue(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req)
{
	struct GadgetRequest *gadgetRequest;
	struct GADGET_REQ *iterator;

	if (!priv || !gadgetEp || !req)
		return -EINVAL;

	gadgetRequest = requestToGadgetRequest(req);
	if (gadgetRequest->ep != gadgetEp)
		return -EINVAL;

	pr_debug("Dequeue request %p form %s\n", req, gadgetEp->gadgetEp.name);

	listBrowsingRequest(iterator, &gadgetEp->request, list) {
		if (req == iterator)
			break;
	}

	if (req != iterator) {
		pr_info("request %p not queued to %s\n", req, gadgetEp->gadgetEp.name);
		return -EINVAL;
	}

	if (gadgetEp->state == GADGET_EP_BUSY) {
		priv->dmaDrv->dma_channelAbort(priv->dmaController, gadgetEp->channel);
		gadgetEp->state = GADGET_EP_ALLOCATED;
	}

	gadgetEpXCallback(priv, gadgetEp, req, GADGET_ECONNRESET);

	return 0;
}

static int32_t gadgetEp0Dequeue(struct GADGET_CTRL *priv, struct GadgetEp *gadgetEp,
		struct GADGET_REQ *req)
{
	return 0;
}

static int32_t gadgetEpDequeue(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
		struct GADGET_REQ *req)
{
	struct GadgetEp *gadgetEp = NULL;

	if (!priv || !gadget_Ep || !req)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);

	if (gadget_Ep->address & GADGET_USB_EP_NUMBER_MASK)
		return gadgetEpXDequeue(priv, gadgetEp, req);
	else
		return gadgetEp0Dequeue(priv, gadgetEp, req);
}

static int32_t gadgetEpSetHalt(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep, uint8_t value)
{
	if (!priv || !gadget_Ep)
		return -EINVAL;

	if (gadget_Ep->address & GADGET_USB_EP_NUMBER_MASK)
		return gadgetEpXSetHalt(priv, gadget_Ep, value);
	else
		return gadgetEp0SetHalt(priv, gadget_Ep, value);
}

static int32_t gadgetEpSetWedge(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep)
{
	struct GadgetEp *gadgetEp = NULL;

	if (!priv || !gadget_Ep)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	gadgetEp->wedged = 1;

	return gadgetEpSetHalt(priv, gadget_Ep, 1);
}

static int32_t gadgetEpFifoStatus(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep)
{
	if (!priv || !gadget_Ep)
		return -EINVAL;

	return 0;
}

static void gadgetEpFifoFlush(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep)
{
	if (!priv || !gadget_Ep)
		return;
}

static int32_t gadgetEpAllocRequest(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
		struct GADGET_REQ **req)
{
	struct GadgetEp *gadgetEp = NULL;
	struct GadgetRequest *gadgetRequest = NULL;

	if (!priv || !gadget_Ep || !req)
		return -EINVAL;

	gadgetEp = toGadgetEp(gadget_Ep);
	if (priv->eventCallback.usbRequestMemAlloc)
		gadgetRequest = priv->eventCallback.usbRequestMemAlloc(priv,
				sizeof(*gadgetRequest));
	if (!gadgetRequest)
		return -ENOMEM;

	memset(gadgetRequest, 0, sizeof(*gadgetRequest));
	*req = &gadgetRequest->request;
	INIT_LIST_HEAD(&gadgetRequest->request.list);
	gadgetRequest->ep = gadgetEp;

	return 0;
}

static void gadgetEpFreeRequest(struct GADGET_CTRL *priv, struct GADGET_EP *gadget_Ep,
		struct GADGET_REQ *req)
{
	struct GadgetRequest *gadgetRequest = NULL;

	if (!priv || !gadget_Ep || !req)
		return;

	gadgetRequest = requestToGadgetRequest(req);

	if (priv->eventCallback.usbRequestMemFree)
		priv->eventCallback.usbRequestMemFree(priv, gadgetRequest);
}

static struct GADGET_EP_OPS gadgetEpOps = {
	.epEnable	=	gadgetEpEnable,
	.epDisable	=	gadgetEpDisable,
	.reqQueue	=	gadgetEpQueue,
	.reqDequeue	=	gadgetEpDequeue,
	.epSetHalt	=	gadgetEpSetHalt,
	.epSetWedge	=	gadgetEpSetWedge,
	.epFifoStatus	=	gadgetEpFifoStatus,
	.reqAlloc	=	gadgetEpAllocRequest,
	.reqFree	=	gadgetEpFreeRequest
};

static void gadgetInitDeviceEp(struct GADGET_CTRL *priv, uint8_t isInEp)
{
	uint8_t num;
	struct GadgetEp *gadgetEp;
	struct GADGET_EP_CFG epCfg;

	if (!priv)
		return;

	for (num = 0; num < 16; num++) {
		gadgetEp = isInEp ? &priv->in[num] : &priv->out[num];
		if (num) {
			epCfg = isInEp ? priv->gadgetCfg.epIN[num] : priv->gadgetCfg.epOUT[num];
			if (!epCfg.bufferingValue) {
				gadgetEp->state = GADGET_EP_NOT_IMPLEMENTED;
				gadgetEp->hwEpNum = num;
				continue;
			}
		}
		INIT_LIST_HEAD(&gadgetEp->gadgetEp.epList);
		INIT_LIST_HEAD(&gadgetEp->request);
		gadgetEp->state = GADGET_EP_FREE;
		gadgetEp->hwEpNum = num;
		gadgetEp->isInEp = isInEp;
		gadgetEp->requestsInList = 0;
		snprintf(gadgetEp->gadgetEp.name, sizeof(gadgetEp->gadgetEp.name),
				"Ep%d%s", num, isInEp ? "in" : "out");
		if (!num) {
			gadgetEp->gadgetEp.maxPacket = priv->gadgetCfg.epIN[num].maxPacketSize;
			if (isInEp)
				priv->gadgetDev.ep0 = &gadgetEp->gadgetEp;
			gadgetEp->gadgetEp.ops = &gadgetEpOps;
			gadgetEp0Enable(priv, gadgetEp);
			continue;
		}

		if (isInEp) {
			if (epCfg.startBuf)
				phytium_write16(&priv->regs->txstaddr[num - 1].addr,
						epCfg.startBuf);
			phytium_write8(&priv->regs->ep[num - 1].txcon, 0);
			gadgetEp->gadgetEp.maxPacket = epCfg.maxPacketSize;
		} else {
			if (epCfg.startBuf)
				phytium_write16(&priv->regs->rxstaddr[num - 1].addr,
						epCfg.startBuf);
			phytium_write8(&priv->regs->ep[num - 1].rxcon, 0);
			gadgetEp->gadgetEp.maxPacket = epCfg.maxPacketSize;
		}

		gadgetEp->gadgetEp.ops = &gadgetEpOps;
		gadgetEp->gadgetEp.address = isInEp ? (0x80 | num) : num;
		gadgetEp->gadgetEp.maxburst = 0;
		gadgetEp->gadgetEp.mult = 0;
		gadgetEp->gadgetEp.maxStreams = 0;
		priv->endpointInList++;
		list_add_tail(&gadgetEp->gadgetEp.epList, &priv->gadgetDev.epList);
	}
}

static void gadgetInitEndpoint(struct GADGET_CTRL *priv)
{
	if (!priv)
		return;

	gadgetInitDeviceEp(priv, 1);
	gadgetInitDeviceEp(priv, 0);
}

static void gadgetSetup(struct GADGET_CTRL *priv)
{
	if (!priv)
		return;

	INIT_LIST_HEAD(&priv->gadgetDev.epList);
	priv->gadgetDev.state = USB_STATE_NOTATTACHED;
	priv->gadgetDev.maxSpeed = USB_SPEED_HIGH;
	priv->gadgetDev.speed = USB_SPEED_FULL;
	snprintf(priv->gadgetDev.name, sizeof(priv->gadgetDev.name), "Phytium USB SD Driver");

	gadgetInitEndpoint(priv);

	phytium_write8(&priv->regs->ep0maxpack, 0x40);
	phytium_write16(&priv->regs->rxien, 0);
	phytium_write16(&priv->regs->txien, 0);
	phytium_write16(&priv->regs->rxirq, 0xFFFF);
	phytium_write16(&priv->regs->txirq, 0xFFFF);
	phytium_write8(&priv->regs->usbirq, 0xEF);
	phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST | ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST);
	priv->isReady = 1;
}
int32_t gadgetInit(struct GADGET_CTRL *priv, struct GADGET_CFG *config,
		struct GADGET_CALLBACKS *callbacks, struct device *pdev)
{
	struct DMA_SYSREQ dmaSysReq;
	uint8_t usbcs;

	if (!priv || !config || !callbacks)
		return -EINVAL;

	priv->dev = pdev;
	priv->eventCallback = *callbacks;
	priv->gadgetCfg = *config;
	priv->regs = (struct HW_REGS *)config->regBase;
	priv->phy_regs = (struct VHUB_REGS *)config->phy_regBase;
	priv->gadgetDrv = GADGET_GetInstance();
	priv->dmaDrv = DMA_GetInstance();
	priv->dmaController = (void *)(priv + 1);
	priv->dmaCfg.dmaModeRx = 0xFFFF;
	priv->dmaCfg.dmaModeTx = 0xFFFF;
	priv->dmaCfg.regBase = config->regBase + 0x400;
	priv->dmaCfg.trbAddr = config->trbAddr;
	priv->dmaCfg.trbDmaAddr = config->trbDmaAddr;

	priv->dmaDrv->dma_probe(NULL, &dmaSysReq);
	priv->privBuffAddr = (uint8_t *)((uintptr_t)config->trbAddr + dmaSysReq.trbMemSize);
	priv->privBuffDma = (uintptr_t)((uintptr_t)config->trbDmaAddr + dmaSysReq.trbMemSize);
	priv->dmaCallback.complete = gadget_CallbackTransfer;
	priv->dmaDrv->dma_init(priv->dmaController, &priv->dmaCfg, &priv->dmaCallback);
	priv->dmaDrv->dma_setParentPriv(priv->dmaController, priv);

	usbcs = phytium_read8(&priv->regs->usbcs);
	usbcs |= USBCS_DISCON | USBCS_LPMNYET;
	phytium_write8(&priv->regs->usbcs, usbcs);

	gadgetSetup(priv);

	usbcs = phytium_read8(&priv->regs->usbcs);
	usbcs &= USBCS_DISCON;
	phytium_write8(&priv->regs->usbcs, usbcs);

	return 0;
}

static void gadgetDestroy(struct GADGET_CTRL *priv)
{
	pr_debug("Destroy Device Controller driver\n");

	if (priv)
		return;
	gadgetDisconnect(priv);

	gadgetStopActivity(priv);

	phytium_write8(&priv->regs->usbcs, USBCS_DISCON);

	priv->isReady = 0;
}

static void gadgetStart(struct GADGET_CTRL *priv)
{
	uint8_t usbien, usbcs;

	pr_debug("Usb Device Controller start\n");
	if (!priv)
		return;

	usbien = phytium_read8(&priv->regs->usbien);
	usbien |= USBIR_URES | USBIR_SUDAV | USBIR_LPMIR;
	phytium_write8(&priv->regs->usbien, usbien);

	usbcs = phytium_read8(&priv->regs->usbcs);
	usbcs &= ~USBCS_DISCON;
	phytium_write8(&priv->regs->usbcs, usbcs);

	priv->dmaDrv->dma_start(priv->dmaController);
}

static void gadgetReset(struct GADGET_CTRL *priv)
{
	int i = 0;

	pr_debug("Usb Disable Device Activity\n");

	if (!priv)
		return;

	if (priv->gadgetDev.speed != USB_SPEED_UNKNOWN)
		gadgetDisconnect(priv);

	priv->gadgetDev.aHnpSupport = 0;
	priv->gadgetDev.bHnpEnable = 0;
	priv->gadgetDev.state = USB_STATE_DEFAULT;
	priv->deviceAddress = 0;
	priv->ep0State = GADGET_EP0_STAGE_SETUP;

	gadgetStopActivity(priv);

	for (i = 0; i < 1000; i++) {
		priv->gadgetDev.speed = gadgetGetActualSpeed(priv);
		if (priv->gadgetDev.speed == USB_SPEED_HIGH)
			return;
	}
}

static void gadgetStop(struct GADGET_CTRL *priv)
{
	pr_debug("Usb Device Controller stop\n");

	if (!priv)
		return;

	if (!priv->isReady)
		return;

	gadgetReset(priv);

	phytium_write8(&priv->regs->usbien, 0);
	priv->dmaDrv->dma_stop(priv->dmaController);

	priv->isReady = 0;
}

static void gadgetIsr(struct GADGET_CTRL *priv)
{
	uint8_t usbirq, usbien, usbcs;

	if (!priv)
		return;

	usbirq = phytium_read8(&priv->regs->usbirq);
	usbien = phytium_read8(&priv->regs->usbien);

	pr_debug("usbirq:0x%x usbien:0x%x\n", usbirq, usbien);

	usbirq = usbirq & usbien;

	if (!usbirq)
		goto DMA_IRQ;

	if (usbirq & USBIR_LPMIR) {
		pr_debug("USBIRQ LPM\n");
		usbcs = phytium_read8(&priv->regs->usbcs);
		usbcs &= ~USBCS_LPMNYET;
		phytium_write8(&priv->regs->usbcs, usbcs);
		phytium_write8(&priv->regs->usbirq, USBIR_LPMIR);
	}

	if (usbirq & USBIR_URES) {
		pr_debug("USBIRQ RESET\n");
		phytium_write8(&priv->regs->usbirq, USBIR_URES);
		priv->releaseEp0Flag = 1;
		gadgetReset(priv);
		priv->releaseEp0Flag = 0;
		priv->gadgetDev.state = USB_STATE_DEFAULT;
		if (priv->eventCallback.connect)
			priv->eventCallback.connect(priv);
	}

	if (usbirq & USBIR_HSPEED) {
		pr_debug("USBIRQ HighSpeed\n");
		phytium_write8(&priv->regs->usbirq, USBIR_HSPEED);
		priv->gadgetDev.speed = USB_SPEED_HIGH;
	}

	if (usbirq & USBIR_SUDAV) {
		pr_debug("USBIRQ SUDAV\n");
		priv->ep0State = GADGET_EP0_STAGE_SETUP;
		gadgetEp0Irq(priv);
	}

	if (usbirq & USBIR_SOF) {
		pr_debug("USBIRQ SOF\n");
		phytium_write8(&priv->regs->usbirq, USBIR_SOF);
	}

	if (usbirq & USBIR_SUTOK) {
		pr_debug("USBIRQ SUTOK\n");
		phytium_write8(&priv->regs->usbirq, USBIR_SUTOK);
	}

	if (usbirq & USBIR_SUSP) {
		pr_debug("USBIRQ SUSPEND\n");
		phytium_write8(&priv->regs->usbirq, USBIR_SUSP);
	}

	return;
DMA_IRQ:
	priv->dmaDrv->dma_isr(priv->dmaController);
}

static void gadgetGetDevInstance(struct GADGET_CTRL *priv, struct GADGET_DEV **dev)
{
	if (!priv || !dev)
		return;

	*dev = &priv->gadgetDev;
}

static int32_t gadgetGetFrame(struct GADGET_CTRL *priv, uint32_t *numOfFrame)
{
	if (!priv || !numOfFrame)
		return -EINVAL;

	*numOfFrame = phytium_read16(&priv->regs->frmnr);

	return 0;
}

static int32_t gadgetWakeUp(struct GADGET_CTRL *priv)
{
	if (!priv)
		return -EINVAL;

	return -ENOTSUPP;
}

static int32_t gadgetSetSelfPowered(struct GADGET_CTRL *priv)
{
	if (!priv)
		return -EINVAL;

	priv->isSelfPowered = 1;

	return 0;
}

static int32_t gadgetClearSelfPowered(struct GADGET_CTRL *priv)
{
	if (!priv)
		return -EINVAL;

	priv->isSelfPowered = 0;

	return 0;
}

static int32_t gadgetVbusSession(struct GADGET_CTRL *priv, uint8_t isActive)
{
	if (!priv)
		return -EINVAL;

	return -ENOTSUPP;
}

static int32_t gadgetVbusDraw(struct GADGET_CTRL *priv, uint8_t mA)
{
	if (!priv)
		return -EINVAL;

	return -ENOTSUPP;
}

static int32_t gadgetPullUp(struct GADGET_CTRL *priv, uint8_t isOn)
{
	if (!priv)
		return -EINVAL;

	return -ENOTSUPP;
}

struct GADGET_OBJ GadgetObj = {
	.gadget_init	=	gadgetInit,
	.gadget_destroy =	gadgetDestroy,
	.gadget_start	=	gadgetStart,
	.gadget_stop	=	gadgetStop,
	.gadget_isr	=	gadgetIsr,
	//endpoint operation
	.gadget_epEnable	=	gadgetEpEnable,
	.gadget_epDisable	=	gadgetEpDisable,
	.gadget_epSetHalt	=	gadgetEpSetHalt,
	.gadget_epSetWedge	=	gadgetEpSetWedge,
	.gadget_epFifoStatus	=	gadgetEpFifoStatus,
	.gadget_epFifoFlush	=	gadgetEpFifoFlush,
	.gadget_reqQueue	=	gadgetEpQueue,
	.gadget_reqDequeue	=	gadgetEpDequeue,
	.gadget_reqAlloc	=	gadgetEpAllocRequest,
	.gadget_reqFree		=	gadgetEpFreeRequest,

	//Device operations
	.gadget_getDevInstance =	gadgetGetDevInstance,
	.gadget_dGetFrame	=	gadgetGetFrame,
	.gadget_dWakeUp		=	gadgetWakeUp,
	.gadget_dSetSelfpowered =	gadgetSetSelfPowered,
	.gadget_dClearSelfpowered =	gadgetClearSelfPowered,
	.gadget_dVbusSession	=	gadgetVbusSession,
	.gadget_dVbusDraw	=	gadgetVbusDraw,
	.gadget_dPullUp		=	gadgetPullUp,
};

struct GADGET_OBJ *GADGET_GetInstance(void)
{
	return &GadgetObj;
}

static int phytium_gadget_set_default_cfg(struct phytium_cusb *config)
{
	int index;

	config->gadget_cfg.regBase = (uintptr_t)config->regs;
	config->gadget_cfg.phy_regBase = (uintptr_t)config->phy_regs;
	config->gadget_cfg.dmaInterfaceWidth = GADGET_DMA_32_WIDTH;

	for (index = 0; index < 16; index++) {
		if (index == 0) {
			config->gadget_cfg.epIN[index].bufferingValue = 1;
			config->gadget_cfg.epIN[index].maxPacketSize = 64;
			config->gadget_cfg.epIN[index].startBuf = 0;

			config->gadget_cfg.epOUT[index].bufferingValue = 1;
			config->gadget_cfg.epOUT[index].maxPacketSize = 64;
			config->gadget_cfg.epOUT[index].startBuf = 0;
		} else {
			config->gadget_cfg.epIN[index].bufferingValue = 4;
			config->gadget_cfg.epIN[index].maxPacketSize = 1024;
			config->gadget_cfg.epIN[index].startBuf = 64 + 4096 * (index - 1);

			config->gadget_cfg.epOUT[index].bufferingValue = 4;
			config->gadget_cfg.epOUT[index].maxPacketSize = 1024;
			config->gadget_cfg.epOUT[index].startBuf = 64 + 4096 * (index - 1);
		}
	}

	return 0;
}

void gadget_callback_connect(struct GADGET_CTRL *priv)
{
	if (!priv)
		return;
}

void gadget_callback_disconnect(struct GADGET_CTRL *priv)
{
	if (!priv)
		return;
}

int32_t gadget_callback_setup(struct GADGET_CTRL *priv, struct usb_ctrlrequest *ctrl)
{
	struct phytium_cusb *config;
	int ret = 0;

	if (!priv || !ctrl)
		return -EINVAL;

	config = dev_get_drvdata(priv->dev);
	if (!config)
		return -1;

	if (!config->gadget_driver)
		return -EOPNOTSUPP;

	if (ctrl->bRequestType & USB_DIR_IN)
		config->ep0_data_stage_is_tx = 1;
	else
		config->ep0_data_stage_is_tx = 0;

	spin_unlock(&config->lock);
	ret = config->gadget_driver->setup(&config->gadget, ctrl);
	spin_lock(&config->lock);

	if (ret == 0x7FFF)
		return ret;

	if (ret < 0)
		return 1;

	return 0;
}

void *gadget_callback_usbRequestMemAlloc(struct GADGET_CTRL *priv, u32 size)
{
	struct GADGET_REQ *gadget_req = NULL;

	gadget_req = kzalloc(size, GFP_NOWAIT);
	if (!gadget_req)
		return NULL;

	return gadget_req;
}

void gadget_callback_usbRequestMemFree(struct GADGET_CTRL *priv, void *usbReq)
{
	if (!usbReq)
		return;

	kfree(usbReq);
}

static void init_peripheral_ep(struct phytium_cusb *config,
		struct phytium_ep *phy_ep, struct GADGET_EP *gadget_ep, int is_tx)
{
	if (!config || !phy_ep || !gadget_ep)
		return;

	memset(phy_ep, 0, sizeof(*phy_ep));
	phy_ep->config = config;
	phy_ep->is_tx = is_tx;
	phy_ep->gadget_ep = gadget_ep;
	phy_ep->ep_num = gadget_ep->address & 0xF;
	phy_ep->end_point.maxpacket = gadget_ep->maxPacket;
	phy_ep->end_point.maxpacket_limit = 1024;

	INIT_LIST_HEAD(&phy_ep->req_list);
	sprintf(phy_ep->name, "ep%d%s", phy_ep->ep_num, is_tx ? "in" : "out");

	switch (phy_ep->ep_num) {
	case 0:
		phy_ep->end_point.caps.type_control = 1;
		break;
	case 1:
		phy_ep->end_point.caps.type_bulk = 1;
		break;
	case 2:
		phy_ep->end_point.caps.type_int = 1;
		break;
	case 3:
		phy_ep->end_point.caps.type_iso = 1;
		break;
	default:
		phy_ep->end_point.caps.type_int = 1;
		phy_ep->end_point.caps.type_bulk = 1;
		break;
	}

	if (is_tx) {
		phy_ep->end_point.caps.dir_in = 1;
		phy_ep->end_point.caps.dir_out = 0;
	} else {
		phy_ep->end_point.caps.dir_in = 0;
		phy_ep->end_point.caps.dir_out = 1;
	}

	phy_ep->end_point.name = phy_ep->name;

	INIT_LIST_HEAD(&phy_ep->end_point.ep_list);

	if (!phy_ep->ep_num) {
		phy_ep->end_point.ops = &gadget_ep0_ops;
		config->gadget.ep0 = &phy_ep->end_point;

		if (config->gadget_dev->maxSpeed > USB_SPEED_HIGH)
			config->gadget.ep0->maxpacket = 9;
	} else {
		phy_ep->end_point.ops = &gadget_ep_ops;
		list_add_tail(&phy_ep->end_point.ep_list, &config->gadget.ep_list);
	}
}

static void gadget_init_endpoint(struct phytium_cusb *config)
{
	struct list_head *list;
	struct GADGET_EP *gadget_ep;

	if (!config)
		return;

	INIT_LIST_HEAD(&(config->gadget.ep_list));

	init_peripheral_ep(config, &config->endpoints_tx[0], config->gadget_dev->ep0, 1);
	init_peripheral_ep(config, &config->endpoints_rx[0], config->gadget_dev->ep0, 0);

	list_for_each(list, &config->gadget_dev->epList) {
		gadget_ep = (struct GADGET_EP *)list;
		if (gadget_ep->address & USB_DIR_IN)
			init_peripheral_ep(config, &config->endpoints_tx[gadget_ep->address & 0xf],
					gadget_ep, 1);
		else
			init_peripheral_ep(config, &config->endpoints_rx[gadget_ep->address & 0xf],
					gadget_ep, 0);
	}
}

static int gadget_setup(struct phytium_cusb *config)
{
	int ret = -1;

	config->gadget_obj->gadget_getDevInstance(config->gadget_priv, &config->gadget_dev);
	config->gadget.ops = &phytium_gadget_ops;
	config->gadget.max_speed = config->gadget_dev->maxSpeed;
	config->gadget.speed = USB_SPEED_HIGH;
	config->gadget.name = "phytium_gadget";
	config->gadget.is_otg = 0;

	gadget_init_endpoint(config);

	ret = usb_add_gadget_udc(config->dev, &config->gadget);
	if (ret)
		goto err;

	return 0;

err:
	config->gadget.dev.parent = NULL;
	device_unregister(&config->gadget.dev);
	return ret;
}

int phytium_gadget_reinit(struct phytium_cusb *config)
{
	struct GADGET_CTRL *ctrl;

	if (!config)
		return 0;

	ctrl = (struct GADGET_CTRL *)config->gadget_priv;
	if (!ctrl)
		return 0;

	gadgetStop(ctrl);

	config->gadget_obj->gadget_init(config->gadget_priv, &config->gadget_cfg,
			&config->gadget_callbacks, config->dev);

	return 0;
}

int phytium_gadget_init(struct phytium_cusb *config)
{
	int ret;

	if (!config)
		return 0;

	phytium_gadget_set_default_cfg(config);
	config->gadget_obj = &GadgetObj;

	config->dma_cfg.regBase = config->gadget_cfg.regBase + 0x400;
	config->dma_obj = DMA_GetInstance();
	config->dma_obj->dma_probe(&config->dma_cfg, &config->dma_sysreq);

	config->gadget_sysreq.privDataSize = sizeof(struct GADGET_CTRL);
	config->gadget_sysreq.trbMemSize = config->dma_sysreq.trbMemSize + GADGET_PRIV_BUFFER_SIZE;
	config->gadget_sysreq.privDataSize += config->dma_sysreq.privDataSize;

	config->gadget_priv = devm_kzalloc(config->dev,
			config->gadget_sysreq.privDataSize, GFP_KERNEL);
	if (!config->gadget_priv) {
		ret = -ENOMEM;
		goto err_probe;
	}
	config->gadget_cfg.trbAddr = dma_alloc_coherent(config->dev,
			config->gadget_sysreq.trbMemSize,
			(dma_addr_t *)&config->gadget_cfg.trbDmaAddr, GFP_KERNEL);
	if (!config->gadget_cfg.trbAddr) {
		ret = -ENOMEM;
		goto err_dma_coherent;
	}

	config->gadget_callbacks.connect		= gadget_callback_connect;
	config->gadget_callbacks.disconnect		= gadget_callback_disconnect;
	config->gadget_callbacks.setup			= gadget_callback_setup;
	config->gadget_callbacks.usbRequestMemAlloc	= gadget_callback_usbRequestMemAlloc;
	config->gadget_callbacks.usbRequestMemFree	= gadget_callback_usbRequestMemFree;

	ret = config->gadget_obj->gadget_init(config->gadget_priv, &config->gadget_cfg,
			&config->gadget_callbacks, config->dev);
	if (ret) {
		ret = -ENODEV;
		goto err_init;
	}

	//dev_set_drvdata(config->dev, config);

	gadget_setup(config);

	return 0;

err_init:
	dma_free_coherent(config->dev, config->gadget_sysreq.trbMemSize,
			config->gadget_cfg.trbAddr, config->gadget_cfg.trbDmaAddr);
err_dma_coherent:
err_probe:
	dev_set_drvdata(config->dev, NULL);

	return ret;
}

int phytium_gadget_uninit(struct phytium_cusb *config)
{
	if (config)
		usb_del_gadget_udc(&config->gadget);

	return 0;
}

#ifdef CONFIG_PM
int phytium_gadget_resume(void *priv)
{
	struct GADGET_CTRL *ctrl;
	uint32_t gen_cfg;
	unsigned long flags = 0;
	struct phytium_cusb *config = (struct phytium_cusb *)priv;

	if (!config)
		return 0;

	ctrl = (struct GADGET_CTRL *)config->gadget_priv;
	if (!ctrl)
		return 0;

	spin_lock_irqsave(&config->lock, flags);
	phytium_gadget_reinit(config);

	if (config->gadget_driver) {
		config->gadget_obj->gadget_start(config->gadget_priv);
		if (ctrl->phy_regs) {
			gen_cfg = phytium_read32(&ctrl->phy_regs->gen_cfg);
			gen_cfg = gen_cfg & (~BIT(7));
			phytium_write32(&ctrl->phy_regs->gen_cfg, gen_cfg);
		}
	}
	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

int phytium_gadget_suspend(void *priv)
{
	return 0;
}
#endif
