// SPDX-License-Identifier: GPL-2.0

#include <linux/dma-mapping.h>
#include <linux/usb.h>
#include<linux/usb/hcd.h>
#include <linux/interrupt.h>
//#include "list.h"
#include "core.h"
#include "dma.h"
#include "hw-regs.h"

#define DRV_NAME "phytium_usb"

#define HOST_GENERIC_EP_CONTROLL 0x00
#define HOST_GENERIC_EP_ISOC 0x01
#define HOST_GENERIC_EP_BULK 0x02
#define HOST_GENERIC_EP_INT 0x03

#define HOST_ESTALL	1
#define HOST_EUNHANDLED	2
#define HOST_EAUTOACK	3
#define HOST_ESHUTDOWN	4

#define HOST_EP_NUM	16

static inline struct HOST_REQ *getUsbRequestEntry(struct list_head *list)
{
	return (struct HOST_REQ *)((uintptr_t)list - (uintptr_t)&(((struct HOST_REQ *)0)->list));
}

static inline struct HOST_EP_PRIV *getUsbHEpPrivEntry(struct list_head *list)
{
	struct HOST_EP_PRIV *hostEpPriv;

	if (list_empty(list))
		return NULL;

	hostEpPriv = (struct HOST_EP_PRIV *)((uintptr_t)list -
			(uintptr_t)&(((struct HOST_EP_PRIV *)0)->node));

	return hostEpPriv;
}

static struct HOST_REQ *getNextReq(struct HOST_EP *usbEp)
{
	struct list_head *queue;

	if (!usbEp)
		return NULL;

	queue = &usbEp->reqList;

	if (list_empty(queue))
		return NULL;

	return getUsbRequestEntry(queue->next);
}

static void host_SetVbus(struct HOST_CTRL *priv, uint8_t isOn)
{
	uint8_t otgctrl = phytium_read8(&priv->regs->otgctrl);

	if (isOn) {
		if (!(otgctrl & OTGCTRL_BUSREQ) || (otgctrl & OTGCTRL_ABUSDROP)) {
			otgctrl &= ~OTGCTRL_ABUSDROP;
			otgctrl |= OTGCTRL_BUSREQ;
			phytium_write8(&priv->regs->otgctrl, otgctrl);
		}
		priv->otgState = HOST_OTG_STATE_A_WAIT_BCON;
	} else {
		if ((otgctrl & OTGCTRL_BUSREQ) || (otgctrl & OTGCTRL_ABUSDROP)) {
			otgctrl |= OTGCTRL_ABUSDROP;
			otgctrl &= ~OTGCTRL_BUSREQ;
			phytium_write8(&priv->regs->otgctrl, otgctrl);
		}
		priv->otgState = HOST_OTG_STATE_A_IDLE;
	}
}

static inline void disconnectHostDetect(struct HOST_CTRL *priv)
{
	uint8_t otgctrl, otgstate;
	uint32_t gen_cfg;

	if (!priv)
		return;

	otgctrl = phytium_read8(&priv->regs->otgctrl);
	if ((otgctrl & OTGCTRL_ASETBHNPEN) && priv->otgState == HOST_OTG_STATE_A_SUSPEND)
		pr_info("Device no Response\n");

	phytium_write8(&priv->regs->otgirq, OTGIRQ_CONIRQ);
retry:
	otgstate = phytium_read8(&priv->regs->otgstate);
	if ((otgstate == HOST_OTG_STATE_A_HOST || otgstate == HOST_OTG_STATE_B_HOST)) {
		pr_info("IRQ OTG: DisconnIrq Babble\n");
		goto retry;
	}

	phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST | ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST);
	phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0 | 0x04);
	phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | FIFOCTRL_IO_TX | 0 | 0x04);

	priv->portStatus = USB_PORT_STAT_POWER;
	priv->portStatus |= USB_PORT_STAT_C_CONNECTION << 16;

	if (priv->hostCallbacks.portStatusChange)
		priv->hostCallbacks.portStatusChange(priv);

	if (priv->otgState == HOST_OTG_STATE_A_SUSPEND)
		host_SetVbus(priv, 1);

	priv->otgState = HOST_OTG_STATE_A_IDLE;
	if (priv->custom_regs) {
		phytium_write32(&priv->custom_regs->wakeup, 1);
	} else {
		gen_cfg = phytium_read32(&priv->vhub_regs->gen_cfg);
		gen_cfg |= BIT(1);
		phytium_write32(&priv->vhub_regs->gen_cfg, gen_cfg);
	}
}

static inline void A_IdleDetect(struct HOST_CTRL *priv, uint8_t otgstate)
{
	uint8_t otgctrl;

	if (!priv)
		return;

	phytium_write8(&priv->regs->otgirq, OTGIRQ_IDLEIRQ);

	if (otgstate != HOST_OTG_STATE_A_IDLE) {
		pr_info("IRQ OTG: A_IDLE Babble\n");
		return;
	}

	priv->portStatus = 0;
	otgctrl = phytium_read8(&priv->regs->otgctrl);
	otgctrl &= ~OTGCTRL_ASETBHNPEN;
	phytium_write8(&priv->regs->otgctrl, otgctrl);

	host_SetVbus(priv, 1);

	priv->otgState = HOST_OTG_STATE_A_IDLE;
}

static inline void B_IdleDetect(struct HOST_CTRL *priv, uint8_t otgstate)
{
	uint8_t otgctrl, usbcs;

	if (!priv)
		return;

	phytium_write8(&priv->regs->otgirq, OTGIRQ_IDLEIRQ);

	if (otgstate != HOST_OTG_STATE_B_IDLE) {
		pr_info("IRQ OTG: B_IDLE Babble\n");
		return;
	}

	otgctrl = phytium_read8(&priv->regs->otgctrl);
	otgctrl &= ~OTGCTRL_ASETBHNPEN;
	phytium_write8(&priv->regs->otgctrl, otgctrl);

	host_SetVbus(priv, 0);

	priv->otgState = HOST_OTG_STATE_B_IDLE;

	usbcs = phytium_read8(&priv->regs->usbcs);
	usbcs &= ~USBCS_DISCON;
	phytium_write8(&priv->regs->usbcs, usbcs);
}

static uint32_t waitForBusyBit(struct HOST_CTRL *priv, struct HostEp *hwEp)
{
	uint8_t *csReg;
	uint8_t flag = CS_BUSY;
	uint8_t buf = 0;
	uint8_t val = CS_BUSY;
	uint8_t otgstate;
	uint8_t bufflag = 0;

	if (!priv || !hwEp)
		return 0;

	if (hwEp->isInEp)
		return 0;

	if (hwEp->hwEpNum == 0) {
		csReg = &priv->regs->ep0cs;
		flag = EP0CS_TXBUSY_MASK;
		buf = 0;
	} else {
		csReg = &priv->regs->ep[hwEp->hwEpNum - 1].txcs;
		buf = phytium_read8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon) & CON_BUF;
	}

	while ((val & flag) || bufflag == 0) {
		otgstate = phytium_read8(&priv->regs->otgstate);
		if (otgstate != HOST_OTG_STATE_B_HOST && otgstate != HOST_OTG_STATE_A_HOST) {
			priv->ep0State = HOST_EP0_STAGE_IDLE;
			return HOST_ESHUTDOWN;
		}

		val = phytium_read8(csReg);
		if (((val & CS_NPAK) >> CS_NPAK_OFFSET) == buf || buf == 0)
			bufflag = 1;
		else
			bufflag = 0;
	}

	return 0;
}

static inline void connectHostDetect(struct HOST_CTRL *priv, uint8_t otgState)
{
	uint32_t gen_cfg;

	if (!priv)
		return;
	pr_debug("otgState:0x%x pirv->otgState:0x%x\n", otgState, priv->otgState);
	if (priv->custom_regs) {
		phytium_write32(&priv->custom_regs->wakeup, 0);
	} else {
		gen_cfg = phytium_read32(&priv->vhub_regs->gen_cfg);
		gen_cfg &= ~BIT(1);
		phytium_write32(&priv->vhub_regs->gen_cfg, gen_cfg);
	}

	phytium_write8(&priv->regs->otgirq, OTGIRQ_CONIRQ);

	if ((otgState != HOST_OTG_STATE_A_HOST) && (otgState != HOST_OTG_STATE_B_HOST))
		return;

	if ((priv->otgState == HOST_OTG_STATE_A_PERIPHERAL)
			|| (priv->otgState == HOST_OTG_STATE_B_PERIPHERAL))
		priv->otgState = otgState;

	priv->ep0State = HOST_EP0_STAGE_IDLE;

	priv->portStatus &= ~(USB_PORT_STAT_LOW_SPEED | USB_PORT_STAT_HIGH_SPEED |
			USB_PORT_STAT_ENABLE);

	priv->portStatus |= USB_PORT_STAT_C_CONNECTION | (USB_PORT_STAT_C_CONNECTION << 16);
	priv->dmaDrv->dma_controllerReset(priv->dmaController);
	priv->port_resetting = 1;
	host_SetVbus(priv, 1);

	switch (phytium_read8(&priv->regs->speedctrl)) {
	case SPEEDCTRL_HS:
		priv->portStatus |= USB_PORT_STAT_HIGH_SPEED;
		pr_debug("detect High speed device\n");
		break;
	case SPEEDCTRL_FS:
		priv->portStatus &= ~(USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_LOW_SPEED);
		pr_debug("detect Full speed device\n");
		break;
	case SPEEDCTRL_LS:
		priv->portStatus |= USB_PORT_STAT_LOW_SPEED;
		pr_debug("detect Low speed device\n");
		break;
	}

	priv->vBusErrCnt = 0;
	priv->dmaDrv->dma_setHostMode(priv->dmaController);

	if (priv->hostCallbacks.portStatusChange)
		priv->hostCallbacks.portStatusChange(priv);

	priv->otgState = otgState;
}

static void hostOtgIrq(struct HOST_CTRL *priv)
{
	uint8_t otgirq, otgien;
	uint8_t otgstatus, otgstate;
	uint8_t otgctrl;

	if (!priv)
		return;

	otgirq = phytium_read8(&priv->regs->otgirq);
	otgien = phytium_read8(&priv->regs->otgien);
	otgstatus = phytium_read8(&priv->regs->otgstatus);
	otgstate = phytium_read8(&priv->regs->otgstate);
	otgirq &= otgien;

	if (!otgirq)
		return;

	if (otgirq & OTGIRQ_BSE0SRPIRQ) {
		otgirq &= ~OTGIRQ_BSE0SRPIRQ;
		phytium_write8(&priv->regs->otgirq, OTGIRQ_BSE0SRPIRQ);

		otgctrl = phytium_read8(&priv->regs->otgctrl);
		otgctrl &= ~OTGIRQ_BSE0SRPIRQ;
		phytium_write8(&priv->regs->otgctrl, otgctrl);
	}

	if (otgirq & OTGIRQ_SRPDETIRQ) {
		otgirq &= ~OTGIRQ_SRPDETIRQ;
		phytium_write8(&priv->regs->otgirq, OTGIRQ_SRPDETIRQ);

		otgctrl = phytium_read8(&priv->regs->otgctrl);
		otgctrl &= ~OTGIRQ_SRPDETIRQ;
		phytium_write8(&priv->regs->otgctrl, otgctrl);
	}

	if (otgirq & OTGIRQ_VBUSERRIRQ) {
		otgirq &= ~OTGIRQ_VBUSERRIRQ;
		phytium_write8(&priv->regs->otgirq, OTGIRQ_VBUSERRIRQ);

		if (otgstate != HOST_OTG_STATE_A_VBUS_ERR) {
			pr_info("IRQ OTG: VBUS ERROR Babble\n");
			return;
		}

		host_SetVbus(priv, 0);
		priv->otgState = HOST_OTG_STATE_A_VBUS_ERR;
		if (priv->portStatus & USB_PORT_STAT_CONNECTION) {
			priv->portStatus = USB_PORT_STAT_POWER | (USB_PORT_STAT_C_CONNECTION << 16);
			if (priv->hostCallbacks.portStatusChange)
				priv->hostCallbacks.portStatusChange(priv);
			return;
		}

		if (priv->vBusErrCnt >= 3) {
			priv->vBusErrCnt = 0;
			pr_info("%s %d VBUS OVER CURRENT\n", __func__, __LINE__);
			priv->portStatus |= USB_PORT_STAT_OVERCURRENT |
				(USB_PORT_STAT_C_OVERCURRENT << 16);

			phytium_write8(&priv->regs->otgirq, OTGIRQ_IDLEIRQ);
		} else {
			priv->vBusErrCnt++;
			host_SetVbus(priv, 1);
			phytium_write8(&priv->regs->otgirq, OTGIRQ_IDLEIRQ);
		}
	}

	if (otgirq & OTGIRQ_CONIRQ) {
		if (priv->otgState == HOST_OTG_STATE_A_HOST ||
				priv->otgState == HOST_OTG_STATE_B_HOST ||
				priv->otgState == HOST_OTG_STATE_A_SUSPEND) {
			if (otgstate == HOST_OTG_STATE_A_WAIT_VFALL ||
					otgstate == HOST_OTG_STATE_A_WAIT_BCON ||
					otgstate == HOST_OTG_STATE_A_SUSPEND)
				disconnectHostDetect(priv);
		} else if (priv->otgState != HOST_OTG_STATE_A_HOST &&
				priv->otgState != HOST_OTG_STATE_B_HOST &&
				priv->otgState != HOST_OTG_STATE_A_SUSPEND)
			connectHostDetect(priv, otgstate);

		phytium_write8(&priv->regs->otgirq, OTGIRQ_CONIRQ);
	}

	if (otgirq & OTGIRQ_IDLEIRQ) {
		if (!(otgstatus & OTGSTATUS_ID))
			A_IdleDetect(priv, otgstate);
		else
			B_IdleDetect(priv, otgstate);
	}

	phytium_write8(&priv->regs->otgirq, OTGIRQ_IDCHANGEIRQ |
			OTGIRQ_SRPDETIRQ);
}

static void hostErrorIrq(struct HOST_CTRL *priv)
{
	uint16_t txerrirq, txerrien;
	uint16_t rxerrirq, rxerrien;
	uint16_t i, mask;

	if (!priv)
		return;

	txerrirq = phytium_read16(&priv->regs->txerrirq);
	txerrien = phytium_read16(&priv->regs->txerrien);
	txerrirq &= txerrien;

	rxerrirq = phytium_read16(&priv->regs->rxerrirq);
	rxerrien = phytium_read16(&priv->regs->rxerrien);
	rxerrirq &= rxerrirq;
	if (!txerrirq && !rxerrirq)
		return;

	for (i = 0; i < HOST_EP_NUM; i++) {
		mask = 1 << i;
		if (rxerrirq & mask) {
			phytium_write16(&priv->regs->rxerrirq, mask);
			rxerrien &= ~mask;
			phytium_write16(&priv->regs->rxerrien, rxerrien);
			priv->dmaDrv->dma_errIsr(priv->dmaController, i, 0);
		}

		if (txerrirq & mask) {
			phytium_write16(&priv->regs->txerrirq, mask);
			txerrien &= ~mask;
			phytium_write16(&priv->regs->txerrien, txerrien);
			priv->dmaDrv->dma_errIsr(priv->dmaController, i, 1);
		}
	}
}

static uint32_t decodeErrorCode(uint8_t code)
{
	uint32_t status = 0;

	switch (code) {
	case ERR_NONE:
		status = 0;
		break;
	case ERR_CRC:
		pr_info("CRC Error\n");
		status = HOST_ESHUTDOWN;
		break;
	case ERR_DATA_TOGGLE_MISMATCH:
		pr_info("Toggle MisMatch Error\n");
		status = HOST_ESHUTDOWN;
		break;
	case ERR_STALL:
		pr_debug("Stall Error\n");
		status = HOST_ESTALL;
		break;
	case ERR_TIMEOUT:
		pr_debug("Timeout Error\n");
		status = HOST_ESHUTDOWN;
		break;
	case ERR_PID:
		pr_info("PID Error\n");
		status = HOST_ESHUTDOWN;
		break;
	case ERR_TOO_LONG_PACKET:
		pr_info("TOO_LONG_PACKET Error\n");
		status = HOST_ESHUTDOWN;
		break;
	case ERR_DATA_UNDERRUN:
		pr_info("UNDERRUN Error\n");
		status = HOST_ESHUTDOWN;
		break;
	}

	return status;
}

static struct HOST_EP_PRIV *getIntTransfer(struct list_head *head)
{
	struct list_head *listEntry = NULL;
	struct HOST_EP_PRIV *usbHEpPriv = NULL;
	struct HOST_EP_PRIV *usbHEpPrivActual = NULL;

	list_for_each(listEntry, head) {
		usbHEpPriv = getUsbHEpPrivEntry(listEntry);
		if (!usbHEpPrivActual)
			usbHEpPrivActual = usbHEpPriv;

		if (usbHEpPriv->frame < usbHEpPrivActual->frame)
			usbHEpPrivActual = usbHEpPriv;
	}

	return usbHEpPrivActual;
}

static void givebackRequest(struct HOST_CTRL *priv, struct HOST_REQ *usbReq, uint32_t status)
{
	if (!priv || !usbReq)
		return;

	list_del(&usbReq->list);

	if (usbReq->status == EINPROGRESS)
		usbReq->status = status;

	if (priv->hostCallbacks.givebackRequest)
		priv->hostCallbacks.givebackRequest(priv, usbReq, status);
}

static void hostEpProgram(struct HOST_CTRL *priv, struct HostEp *hwEp,
		struct HOST_REQ *usbReq, uintptr_t dmaBuff, uint32_t length)
{
	struct HOST_EP *usbHEp;
	struct HOST_EP_PRIV *usbEpPriv;
	uint32_t chMaxLen;
	uint8_t regCon = 0;
	uint8_t ep0cs;
	uint16_t txerrien = 0;
	uint16_t rxerrien = 0;
	uint32_t result;
	uint8_t txsoftimer, rxsoftimer;
	u8 retval = 0;

	if (!priv || !hwEp || !usbReq)
		return;

	usbHEp = hwEp->scheduledUsbHEp;
	usbEpPriv = (struct HOST_EP_PRIV *)usbHEp->hcPriv;

	if (!hwEp->channel) {
		if (usbEpPriv->type == USB_ENDPOINT_XFER_ISOC)
			hwEp->channel = priv->dmaDrv->dma_channelAlloc(priv->dmaController,
					!hwEp->isInEp, hwEp->hwEpNum, 1);
		else
			hwEp->channel = priv->dmaDrv->dma_channelAlloc(priv->dmaController,
					!hwEp->isInEp, hwEp->hwEpNum, 0);
	}

	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, hwEp->channel);

	pr_debug("chMaxLen:0x%x buffLength:0x%x\n", chMaxLen, usbReq->buffLength);
	if (usbReq->buffLength > chMaxLen)
		length = chMaxLen;

	switch (usbEpPriv->type) {
	case USB_ENDPOINT_XFER_CONTROL:
		regCon = CON_TYPE_CONTROL;
		break;
	case USB_ENDPOINT_XFER_BULK:
		regCon = CON_TYPE_BULK;
		break;
	case USB_ENDPOINT_XFER_INT:
		regCon = CON_TYPE_INT;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		if (usbEpPriv->isocEpConfigured)
			goto dma_program;

		usbEpPriv->isocEpConfigured = 1;
		regCon = CON_TYPE_ISOC;
		switch (usbHEp->desc.wMaxPacketSize >> 11) {
		case 0:
			regCon |= CON_TYPE_ISOC_1_ISOD;
			priv->dmaDrv->dma_setMaxLength(priv->dmaController,
					hwEp->channel, usbEpPriv->maxPacketSize);
			break;
		case 1:
			regCon |= CON_TYPE_ISOC_2_ISOD;
			priv->dmaDrv->dma_setMaxLength(priv->dmaController,
					hwEp->channel, 2 * 1024);
			break;
		case 2:
			priv->dmaDrv->dma_setMaxLength(priv->dmaController,
					hwEp->channel, 3 * 1024);
			regCon |= CON_TYPE_ISOC_3_ISOD;
			break;
		}
		break;
	}

	if (usbEpPriv->type != USB_ENDPOINT_XFER_ISOC) {
		if (!hwEp->hwEpNum) {
			if (phytium_read8(&priv->regs->ep0cs) & 0x4) {
				phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO |
					0 | 0x4);
				phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO |
					FIFOCTRL_IO_TX | 0 | 0x4);
			}
		}
		if (waitForBusyBit(priv, hwEp) > 0) {
			usbReq->status = HOST_ESHUTDOWN;
			givebackRequest(priv, usbReq, HOST_ESHUTDOWN);
			pr_info("something error happen\n");
			return;
		}
	}

	if (!hwEp->isInEp) {
		if (hwEp->hwEpNum) {
			regCon |= hwEp->hwBuffers - 1;
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon, regCon);
			if (usbEpPriv->type != USB_ENDPOINT_XFER_ISOC) {
				retval = priv->hostCallbacks.getEpToggle(priv,
						usbReq->usbDev, usbEpPriv->epNum, 0);
				if (retval) {
					phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
						ENDPRST_IO_TX);

					phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
						ENDPRST_TOGSETQ | ENDPRST_IO_TX | ENDPRST_FIFORST);
				} else {
					phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
						ENDPRST_IO_TX);

					phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
						ENDPRST_TOGRST | ENDPRST_IO_TX | ENDPRST_FIFORST);
				}
			}

			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon, regCon | CON_VAL);
			phytium_write16(&priv->regs->txmaxpack[hwEp->hwEpNum - 1],
					usbEpPriv->maxPacketSize);

			phytium_write8(&priv->regs->epExt[hwEp->hwEpNum - 1].txctrl,
					usbEpPriv->epNum);

			phytium_write8(&priv->regs->fnaddr, usbEpPriv->faddress);

	//		if (usbEpPriv->type == USB_ENDPOINT_XFER_INT)
			txsoftimer = phytium_read8(&priv->regs->txsoftimer[hwEp->hwEpNum].ctrl);
			txsoftimer = txsoftimer | BIT(1);
			phytium_write8(&priv->regs->txsoftimer[hwEp->hwEpNum].ctrl, txsoftimer);

			phytium_write16(&priv->regs->txsoftimer[hwEp->hwEpNum].timer,
					usbEpPriv->frame);

			phytium_write8(&priv->regs->txsoftimer[hwEp->hwEpNum].ctrl, 0x83);
		} else {
			phytium_write8(&priv->regs->fnaddr, usbEpPriv->faddress);
			phytium_write8(&priv->regs->ep0maxpack, usbEpPriv->maxPacketSize);
			phytium_write8(&priv->regs->ep0ctrl, usbEpPriv->epNum);

			if (priv->ep0State == HOST_EP0_STAGE_SETUP) {
				ep0cs = phytium_read8(&priv->regs->ep0cs);
				ep0cs |= EP0CS_HCSET;
				phytium_write8(&priv->regs->ep0cs, ep0cs);
			}
		}

		phytium_write16(&priv->regs->txerrirq, 1 << hwEp->hwEpNum);
		txerrien = phytium_read16(&priv->regs->txerrien);
		txerrien |= 1 << hwEp->hwEpNum;
		phytium_write16(&priv->regs->txerrien, txerrien);
	} else {
		if (hwEp->hwEpNum) {
			regCon |= hwEp->hwBuffers - 1;
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcon, regCon);

			if (usbEpPriv->type != USB_ENDPOINT_XFER_ISOC) {
				if (priv->hostCallbacks.getEpToggle) {
					retval = priv->hostCallbacks.getEpToggle(priv,
							usbReq->usbDev, usbEpPriv->epNum, 1);
					if (retval) {
						phytium_write8(&priv->regs->endprst, hwEp->hwEpNum);
						phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
								ENDPRST_TOGSETQ | ENDPRST_FIFORST);
					} else {
						phytium_write8(&priv->regs->endprst, hwEp->hwEpNum);
						phytium_write8(&priv->regs->endprst, hwEp->hwEpNum |
								ENDPRST_TOGRST | ENDPRST_FIFORST);
					}
				}
			}

			phytium_write16(&priv->regs->rxmaxpack[hwEp->hwEpNum - 1],
					usbEpPriv->maxPacketSize);

			phytium_write8(&priv->regs->epExt[hwEp->hwEpNum - 1].rxctrl,
					usbEpPriv->epNum);

			phytium_write8(&priv->regs->fnaddr, usbEpPriv->faddress);

			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcs, 1);
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcs, 1);
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcs, 1);
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcs, 1);

			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcon, regCon | CON_VAL);
			rxsoftimer = phytium_read8(&priv->regs->rxsoftimer[hwEp->hwEpNum].ctrl);
			rxsoftimer = rxsoftimer | BIT(1);
			phytium_write8(&priv->regs->rxsoftimer[hwEp->hwEpNum].ctrl, rxsoftimer);

			phytium_write16(&priv->regs->rxsoftimer[hwEp->hwEpNum].timer,
					usbEpPriv->frame);

			phytium_write8(&priv->regs->rxsoftimer[hwEp->hwEpNum].ctrl, 0x83);
		} else {
			phytium_write8(&priv->regs->fnaddr, usbEpPriv->faddress);
			phytium_write8(&priv->regs->ep0maxpack, usbEpPriv->maxPacketSize);
			phytium_write8(&priv->regs->ep0ctrl, usbEpPriv->epNum);

			if (priv->ep0State == HOST_EP0_STAGE_IN
					|| priv->ep0State == HOST_EP0_STAGE_STATUSIN)
				phytium_write8(&priv->regs->ep0cs, EP0CS_HCSETTOGGLE);
		}

		phytium_write16(&priv->regs->rxerrirq, 1 << hwEp->hwEpNum);
		rxerrien = phytium_read16(&priv->regs->rxerrien);
		rxerrien |= 1 << hwEp->hwEpNum;
		phytium_write16(&priv->regs->rxerrien, rxerrien);
	}
dma_program:
	result = priv->dmaDrv->dma_channelProgram(priv->dmaController, hwEp->channel,
			usbEpPriv->maxPacketSize, dmaBuff, length,
			(void *)usbReq->isoFramesDesc, usbReq->isoFramesNumber);
	if (result) {
		if (!hwEp->isInEp) {
			txerrien &= ~(1 << hwEp->hwEpNum);
			if (hwEp->hwEpNum)
				phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon, 0x08);
			phytium_write16(&priv->regs->txerrien, txerrien);
		} else {
			rxerrien &= ~(1 << hwEp->hwEpNum);
			if (hwEp->hwEpNum)
				phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcon, 0x08);
			phytium_write16(&priv->regs->rxerrien, rxerrien);
		}

		priv->dmaDrv->dma_channelRelease(priv->dmaController, hwEp->channel);
		hwEp->channel = NULL;
	}
}

static void hostStartReq(struct HOST_CTRL *priv, struct HOST_REQ *req)
{
	uintptr_t dmaBuff;
	uint32_t length;
	struct HOST_EP *hostEp;
	struct HOST_EP_PRIV *hostEpPriv;
	struct HOST_EP_PRIV *hostNewEpPriv;
	struct HOST_REQ *usbReq = NULL;
	struct HostEp *hwEp = NULL;
	int num;

	if (!priv || !req)
		return;

	hostEp = req->hcPriv;
	if (hostEp) {
		hostEpPriv = (struct HOST_EP_PRIV *)hostEp->hcPriv;
		if (hostEpPriv) {
			hostEpPriv->genericHwEp->state = HOST_EP_BUSY;
			switch (hostEpPriv->type) {
			case USB_ENDPOINT_XFER_CONTROL:
				usbReq = getNextReq(hostEp);

				priv->in[HOST_GENERIC_EP_CONTROLL].scheduledUsbHEp = hostEp;
				priv->ep0State = HOST_EP0_STAGE_SETUP;
				hostEpPriv->currentHwEp = hostEpPriv->genericHwEp;
				hostEpPriv->genericHwEp->scheduledUsbHEp = hostEp;
				dmaBuff = usbReq->setupDma;
				length = 8;
				pr_debug("packet info: %02x %02x %04x %04x %04x\n",
					usbReq->setup->bRequestType,
					usbReq->setup->bRequest,
					usbReq->setup->wValue,
					usbReq->setup->wIndex,
					usbReq->setup->wLength);
				hostEpProgram(priv, hostEpPriv->genericHwEp,
						usbReq, dmaBuff, length);
				break;
			case USB_ENDPOINT_XFER_BULK:
				hwEp = hostEpPriv->genericHwEp;
				usbReq = getNextReq(hostEp);
				hostEpPriv->currentHwEp = hwEp;
				hwEp->scheduledUsbHEp = hostEp;
				dmaBuff = usbReq->buffDma + usbReq->actualLength;
				length = usbReq->buffLength - usbReq->actualLength;
				hostEpProgram(priv, hwEp, usbReq, dmaBuff, length);
				break;
			case USB_ENDPOINT_XFER_INT:
				if (hostEpPriv->genericHwEp->scheduledUsbHEp)
					return;

				num = req->epNum - 1;
				if (hostEpPriv->isIn)
					hostNewEpPriv = getIntTransfer(&priv->intInHEpQueue[num]);
				else
					hostNewEpPriv = getIntTransfer(&priv->intOutHEpQueue[num]);

				hostNewEpPriv->currentHwEp = hostEpPriv->genericHwEp;
				hostEpPriv->genericHwEp->scheduledUsbHEp = hostNewEpPriv->usbHEp;
				usbReq = getNextReq(hostEp);
				dmaBuff = usbReq->buffDma + usbReq->actualLength;
				length = usbReq->buffLength - usbReq->actualLength;
				hostEpProgram(priv, hostEpPriv->genericHwEp,
						usbReq, dmaBuff, length);
				break;
			case USB_ENDPOINT_XFER_ISOC:
				hostEpPriv->currentHwEp = hostEpPriv->genericHwEp;
				hostEpPriv->genericHwEp->scheduledUsbHEp = hostEp;
				dmaBuff = req->buffDma + req->actualLength;
				length = req->buffLength - req->actualLength;
				hostEpProgram(priv, hostEpPriv->genericHwEp, req, dmaBuff, length);
				break;
			}
		}
	}
}


static void scheduleNextTransfer(struct HOST_CTRL *priv,
		struct HOST_REQ *usbReq, struct HostEp *hwEp)
{
	struct HOST_EP *usbEp;
	struct HOST_EP_PRIV *usbHEpPriv;
	uint8_t endprst;
	uint32_t status;
	struct HOST_REQ *usbNextReq = NULL;

	if (!priv || !usbReq || !hwEp)
		return;

	usbEp = hwEp->scheduledUsbHEp;
	usbHEpPriv = (struct HOST_EP_PRIV *)usbEp->hcPriv;
	if (!usbHEpPriv)
		return;

	status = (usbReq->status == EINPROGRESS) ? 0 : usbReq->status;
	switch (usbHEpPriv->type) {
	case USB_ENDPOINT_XFER_BULK:
	case USB_ENDPOINT_XFER_INT:
		if (hwEp->isInEp) {
			phytium_write8(&priv->regs->endprst, hwEp->hwEpNum);
			endprst = (phytium_read8(&priv->regs->endprst) & ENDPRST_TOGSETQ) ? 1 : 0;
			if (priv->hostCallbacks.setEpToggle)
				priv->hostCallbacks.setEpToggle(priv, usbReq->usbDev,
						usbHEpPriv->epNum, usbHEpPriv->isIn, endprst);
		} else {
			if (waitForBusyBit(priv, hwEp) > 0) {
				usbReq->status = HOST_ESHUTDOWN;
				givebackRequest(priv, usbReq, HOST_ESHUTDOWN);
				return;
			}

			phytium_write8(&priv->regs->endprst, hwEp->hwEpNum | ENDPRST_IO_TX);
			endprst = (phytium_read8(&priv->regs->endprst) & ENDPRST_TOGSETQ) ? 1 : 0;
			if (priv->hostCallbacks.setEpToggle)
				priv->hostCallbacks.setEpToggle(priv, usbReq->usbDev,
						usbHEpPriv->epNum, usbHEpPriv->isIn, endprst);
		}
		break;
	}

	if (usbHEpPriv->transferFinished)
		givebackRequest(priv, usbReq, status);

	if (list_empty(&usbEp->reqList)) {
		if (usbHEpPriv->type == USB_ENDPOINT_XFER_CONTROL)
			hwEp->state = HOST_EP_ALLOCATED;
		else {
			if (usbHEpPriv->genericHwEp == hwEp)
				hwEp->state = HOST_EP_ALLOCATED;
			else
				hwEp->state = HOST_EP_FREE;
		}

		usbHEpPriv->epIsReady = 0;
		usbHEpPriv->currentHwEp = NULL;
		hwEp->scheduledUsbHEp = NULL;

		if (hwEp->channel) {
			priv->dmaDrv->dma_channelRelease(priv->dmaController, hwEp->channel);
			hwEp->channel = NULL;
		}

		if (usb_endpoint_xfer_int(&usbEp->desc))
			list_del(&usbHEpPriv->node);
		usbHEpPriv = NULL;
	} else {
		if (usbHEpPriv->type == USB_ENDPOINT_XFER_INT) {
			usbHEpPriv->currentHwEp = NULL;
			hwEp->scheduledUsbHEp = NULL;
		}

		if (usbHEpPriv->type == USB_ENDPOINT_XFER_ISOC)
			return;
	}

	if (usbHEpPriv) {
		usbNextReq = getNextReq(usbHEpPriv->usbHEp);
		hostStartReq(priv, usbNextReq);
	}
}

static int32_t hostEp0Irq(struct HOST_CTRL *priv, uint8_t isIn)
{
	struct HostEp *hwEp;
	struct HOST_EP *hostEp;
	struct HOST_REQ *usbReq = NULL;
	struct HOST_EP_PRIV *usbHEpPriv;
	uint32_t status, length;
	uint8_t usbError;
	uint8_t nextStage = 0;
	int ret = 0;

	if (!priv)
		return ret;

	hwEp = isIn ? &priv->in[HOST_GENERIC_EP_CONTROLL] : &priv->out[HOST_GENERIC_EP_CONTROLL];
	hostEp = hwEp->scheduledUsbHEp;
	usbHEpPriv = (struct HOST_EP_PRIV *)hostEp->hcPriv;

	usbReq = getNextReq(hostEp);
	if (!usbReq)
		return 0;

	usbError = isIn ? phytium_read8(&priv->regs->rx0err) : phytium_read8(&priv->regs->tx0err);
	usbError = (usbError & ERR_TYPE) >> 2;
	status = decodeErrorCode(usbError);
	if (status) {
		usbReq->status = status;

		if (status == HOST_ESTALL) {
			ret = 1;
			priv->dmaDrv->dma_controllerReset(priv->dmaController);
		}

		phytium_write16(&priv->regs->rxerrirq, 1 << hwEp->hwEpNum);
		phytium_write16(&priv->regs->txerrirq, 1 << hwEp->hwEpNum);

		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0 | 0x4);
		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO |
				FIFOCTRL_IO_TX | 0 | 0x4);
	}

	length = priv->dmaDrv->dma_getActualLength(priv->dmaController, hwEp->channel);
	priv->dmaDrv->dma_channelRelease(priv->dmaController, hwEp->channel);
	hwEp->channel = NULL;

	if (usbReq->status == EINPROGRESS && priv->ep0State < HOST_EP0_STAGE_STATUSIN) {
		nextStage = 0;
		switch (priv->ep0State) {
		case HOST_EP0_STAGE_IN:
			pr_debug("Ep0 Data IN\n");
			usbHEpPriv->currentHwEp = &priv->out[HOST_GENERIC_EP_CONTROLL];
			usbReq->actualLength = length;
			priv->ep0State = HOST_EP0_STAGE_STATUSOUT;
			break;
		case HOST_EP0_STAGE_OUT:
			pr_debug("Ep0 Data OUT\n");
			usbHEpPriv->currentHwEp = &priv->in[HOST_GENERIC_EP_CONTROLL];
			usbReq->actualLength = length;
			priv->ep0State = HOST_EP0_STAGE_STATUSIN;
			break;
		case HOST_EP0_STAGE_SETUP:
			pr_debug("Ep0 Stage Setup\n");
			if (!usbReq->setup->wLength) {
				pr_debug("EP0_STAGE_STATUSIN\n");
				priv->ep0State = HOST_EP0_STAGE_STATUSIN;
				usbHEpPriv->currentHwEp = &priv->in[HOST_GENERIC_EP_CONTROLL];
				break;
			} else if (usbReq->setup->bRequestType & USB_DIR_IN) {
				pr_debug("EP0_STAGE_STAGE_IN\n");
				priv->ep0State = HOST_EP0_STAGE_IN;
				usbHEpPriv->currentHwEp = &priv->in[HOST_GENERIC_EP_CONTROLL];
				nextStage = 1;
				break;
			}
			priv->ep0State = HOST_EP0_STAGE_OUT;
			nextStage = 1;
			break;
		case HOST_EP0_STAGE_STATUSIN:
		case HOST_EP0_STAGE_STATUSOUT:
		case HOST_EP0_STAGE_ACK:
		case HOST_EP0_STAGE_IDLE:
		default:
			pr_debug("EP0 STAGE is %d\n", priv->ep0State);
			break;
		}

		if (nextStage) {
			length = usbReq->buffLength;
			hostEpProgram(priv, usbHEpPriv->currentHwEp, usbReq,
					usbReq->buffDma, length);
		} else
			hostEpProgram(priv, usbHEpPriv->currentHwEp, usbReq, usbReq->setupDma, 0);
	} else
		priv->ep0State = HOST_EP0_STAGE_IDLE;

	if (priv->ep0State == HOST_EP0_STAGE_IDLE || usbReq->status != EINPROGRESS) {
		usbHEpPriv->transferFinished = 1;
		scheduleNextTransfer(priv, usbReq, hwEp);
	}

	return 0;
}

static void updateTimeIntTransfer(struct list_head *head, struct HOST_EP_PRIV *lastFinished)
{
	struct list_head *listEntry = NULL;
	struct HOST_EP_PRIV *usbHEpPriv = NULL;
	uint16_t time = lastFinished->frame;

	list_for_each(listEntry, head) {
		usbHEpPriv = getUsbHEpPrivEntry(listEntry);
		if (usbHEpPriv == lastFinished) {
			lastFinished->frame = lastFinished->interval;
			continue;
		}

		if (usbHEpPriv->frame < time)
			usbHEpPriv->frame = 0;
		else
			lastFinished->interval = usbHEpPriv->frame;
	}
}

static int32_t hostEpXIrq(struct HOST_CTRL *priv, uint8_t hwEpNum, uint8_t isIn, bool resubmit)
{
	struct HostEp *hwEp;
	struct HOST_EP *hostEp;
	struct HOST_EP_PRIV *usbHEpPriv;
	struct HOST_REQ *usbReq;

	uint8_t usbError, rxcon, txcon;
	uint32_t status, length, chMaxLen;

	if (!priv)
		return -EINVAL;

	hwEp = isIn ? &priv->in[hwEpNum] : &priv->out[hwEpNum];
	hostEp = hwEp->scheduledUsbHEp;
	if (!hostEp)
		return -EINVAL;

	usbHEpPriv = (struct HOST_EP_PRIV *)hostEp->hcPriv;

	usbReq = getNextReq(hostEp);
	if (!usbReq)
		return 0;

	if (isIn)
		usbError = phytium_read8(&priv->regs->epExt[hwEpNum - 1].rxerr);
	else
		usbError = phytium_read8(&priv->regs->epExt[hwEpNum - 1].txerr);

	usbError = (usbError & ERR_TYPE) >> 2;
	status = decodeErrorCode(usbError);
	if (status) {
		pr_debug("%s %d Aborting\n", __func__, __LINE__);
		if (isIn)
			phytium_write16(&priv->regs->rxerrirq, 1 << hwEpNum);
		else
			phytium_write16(&priv->regs->txerrirq, 1 << hwEpNum);
		priv->dmaDrv->dma_channelAbort(priv->dmaController, hwEp->channel);
	}

	length = priv->dmaDrv->dma_getActualLength(priv->dmaController, hwEp->channel);
	chMaxLen = priv->dmaDrv->dma_getMaxLength(priv->dmaController, hwEp->channel);

	if (status != 0)
		usbReq->status = status;

	usbReq->actualLength += length;

	if (!resubmit)
		usbHEpPriv->transferFinished = 1;

	if (length == chMaxLen) {
		if ((usbReq->buffLength - usbReq->actualLength) == 0)
			usbHEpPriv->transferFinished = 1;
		else
			usbHEpPriv->transferFinished = 0;
	}

	if (usbHEpPriv->type != USB_ENDPOINT_XFER_ISOC) {
		if (!hwEp->isInEp) {
			txcon = phytium_read8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon);
			txcon &= ~CON_VAL;
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].txcon, txcon);
		} else {
			rxcon = phytium_read8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcon);
			rxcon &= ~CON_VAL;
			phytium_write8(&priv->regs->ep[hwEp->hwEpNum - 1].rxcon, rxcon);
		}
		priv->dmaDrv->dma_channelRelease(priv->dmaController, hwEp->channel);
		hwEp->channel = NULL;
	}

	if (usbHEpPriv->type == USB_ENDPOINT_XFER_INT) {
		if (usbHEpPriv->isIn)
			updateTimeIntTransfer(&priv->intInHEpQueue[hwEp->hwEpNum - 1], usbHEpPriv);
		else
			updateTimeIntTransfer(&priv->intOutHEpQueue[hwEp->hwEpNum - 1], usbHEpPriv);
	}

	scheduleNextTransfer(priv, usbReq, hwEp);

	return 0;
}


static void host_CallbackTransfer(void *priv, uint8_t epNum, uint8_t isDirTx, bool resubmit)
{
	struct HOST_CTRL *host_priv = (struct HOST_CTRL *)priv;
	int i;
	int ret = 0;

	if (!epNum) {
		ret = hostEp0Irq(host_priv, !isDirTx);
		if (resubmit | ret) {
			for (i = 1; i < HOST_EP_NUM; i++) {
				hostEpXIrq(host_priv, i, !isDirTx, true);
				hostEpXIrq(host_priv, i, isDirTx, true);
			}
		}
	} else
		hostEpXIrq(host_priv, epNum, !isDirTx, false);
}

static int hc_reset(struct usb_hcd *hcd)
{
	hcd->speed = HCD_USB2;
	hcd->self.root_hub->speed = USB_SPEED_HIGH;

	return 0;
}

static int hc_start(struct usb_hcd *hcd)
{
	hcd->state = HC_STATE_RUNNING;
	return 0;
}

static void hc_stop(struct usb_hcd *hcd)
{
	struct phytium_cusb *config = *(struct phytium_cusb **)hcd->hcd_priv;

	if (config)
		config->host_obj->host_stop(config->host_priv);

	if (config->host_cfg.trbAddr) {
		dma_free_coherent(config->dev, config->host_sysreq.trbMemSize,
				config->host_cfg.trbAddr, config->host_cfg.trbDmaAddr);
		config->host_cfg.trbAddr = NULL;
	}

	if (config->host_priv) {
		devm_kfree(config->dev, config->host_priv);
		config->host_priv = NULL;
	}

	hcd->state = HC_STATE_HALT;
}

static void hc_shutdown(struct usb_hcd *hcd)
{
}

static void host_endpoint_update(struct phytium_cusb *config,
		struct HOST_USB_DEVICE *udev, struct usb_host_endpoint *ep)
{
	int epnum;
	struct HOST_EP *hostEp;

	if (!config || !udev || !ep)
		return;

	epnum = usb_endpoint_num(&ep->desc);
	if (epnum > MAX_INSTANCE_EP_NUM)
		epnum = MAX_INSTANCE_EP_NUM;

	if (usb_endpoint_dir_out(&ep->desc)) {
		if (udev->out_ep[epnum] == NULL) {
			hostEp = kzalloc(sizeof(struct HOST_EP) +
				config->host_obj->host_epGetPrivateDataSize(config->host_priv),
				GFP_ATOMIC);
			udev->out_ep[epnum] = hostEp;
		} else
			hostEp = udev->out_ep[epnum];
	} else {
		if (udev->in_ep[epnum] == NULL) {
			hostEp = kzalloc(sizeof(struct HOST_EP) +
				config->host_obj->host_epGetPrivateDataSize(config->host_priv),
				GFP_ATOMIC);
			udev->in_ep[epnum] = hostEp;
		} else
			hostEp = udev->in_ep[epnum];
	}

	hostEp->desc = *(struct usb_endpoint_descriptor *)(&ep->desc);
	hostEp->userExt = (void *)ep;
	INIT_LIST_HEAD(&hostEp->reqList);
	hostEp->hcPriv = &((uint8_t *)hostEp)[sizeof(struct HOST_EP)];
}

static int hc_urb_enqueue(struct usb_hcd *hcd, struct urb *urb, gfp_t mem_flags)
{
	unsigned long flags;
	u32 isoFrameSize;
	int retval;
	int index;
	struct HOST_REQ *req;
	struct HOST_CTRL *priv;
	struct HOST_USB_DEVICE *usbDev;
	struct usb_host_endpoint *host_ep;
	struct usb_endpoint_descriptor *host_ep_desc;
	struct phytium_cusb *config = *(struct phytium_cusb **)hcd->hcd_priv;

	if (!config)
		return -ENODEV;

	priv = config->host_priv;
	if (!priv)
		return -ENODEV;

	usbDev = priv->host_devices_table[urb->dev->slot_id];
	if (!usbDev)
		return -ENODEV;

	host_ep = urb->ep;
	host_ep_desc = &host_ep->desc;

	spin_lock_irqsave(&config->lock, flags);
	retval = usb_hcd_link_urb_to_ep(hcd, urb);
	if (retval < 0) {
		spin_unlock_irqrestore(&config->lock, flags);
		return retval;
	}
	spin_unlock_irqrestore(&config->lock, flags);
	isoFrameSize = urb->number_of_packets * sizeof(struct HOST_ISOFRAMESDESC);
	req = kzalloc((sizeof(struct HOST_REQ) +
				isoFrameSize), mem_flags);
	if (!req) {
		spin_lock_irqsave(&config->lock, flags);
		usb_hcd_unlink_urb_from_ep(hcd, urb);
		spin_unlock_irqrestore(&config->lock, flags);
		return -ENOMEM;
	}

	req->isoFramesDesc = NULL;
	req->isoFramesNumber = urb->number_of_packets;
	req->epNum = usb_endpoint_num(host_ep_desc);
	if (req->epNum > MAX_INSTANCE_EP_NUM)
		req->epNum = MAX_INSTANCE_EP_NUM;

	if (usb_endpoint_dir_in(host_ep_desc)) {
		if (!usbDev->in_ep[req->epNum])
			host_endpoint_update(config, usbDev, host_ep);
	} else {
		if (!usbDev->out_ep[req->epNum])
			host_endpoint_update(config, usbDev, host_ep);
	}

	if (usb_endpoint_xfer_isoc(host_ep_desc)) {
		req->isoFramesDesc = (struct HOST_ISOFRAMESDESC *)(&req[1]);
		for (index = 0; index < urb->number_of_packets; index++) {
			req->isoFramesDesc[index].length = urb->iso_frame_desc[index].length;
			req->isoFramesDesc[index].offset = urb->iso_frame_desc[index].offset;
		}
	}

	spin_lock_irqsave(&config->lock, flags);
	INIT_LIST_HEAD(&req->list);
	req->userExt = (void *)urb;
	req->actualLength = urb->actual_length;
	req->bufAddress = urb->transfer_buffer;
	req->buffDma = urb->transfer_dma;
	req->buffLength = urb->transfer_buffer_length;
	req->epIsIn = usb_endpoint_dir_in(host_ep_desc);
	req->eptype = usb_endpoint_type(host_ep_desc);
	req->faddress = usb_pipedevice(urb->pipe);
	req->interval = urb->interval;
	req->reqUnlinked = 0;
	req->setup = (struct usb_ctrlrequest *)urb->setup_packet;
	req->setupDma = urb->setup_dma;
	req->status = EINPROGRESS;
	req->usbDev = &usbDev->udev;
	req->usbEp = req->epIsIn ? usbDev->in_ep[req->epNum] : usbDev->out_ep[req->epNum];
	if (!req->epNum)
		usbDev->ep0_hep.desc.wMaxPacketSize = urb->dev->ep0.desc.wMaxPacketSize;

	req->hcPriv = (void *)req->usbEp;
	urb->hcpriv = req;
	host_ep->hcpriv = (void *)usbDev;
	retval = config->host_obj->host_reqQueue(config->host_priv, req);
	if (retval) {
		usb_hcd_unlink_urb_from_ep(hcd, urb);
		urb->hcpriv = NULL;
		spin_unlock_irqrestore(&config->lock, flags);
		kfree(req);
		return -retval;
	}
	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static int hc_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status)
{
	unsigned long flags;
	struct HOST_CTRL *priv;
	int ret = 0;
	struct phytium_cusb *config = *(struct phytium_cusb **)hcd->hcd_priv;

	if (!config)
		return -ENODEV;

	priv = config->host_priv;
	if (!priv->host_devices_table[urb->dev->slot_id])
		return -ENODEV;

	spin_lock_irqsave(&config->lock, flags);
	if (usb_hcd_check_unlink_urb(hcd, urb, status))
		goto done;

	if (!urb->hcpriv)
		goto err_giveback;

	ret = config->host_obj->host_reqDequeue(priv, urb->hcpriv, status);
	kfree(urb->hcpriv);
	urb->hcpriv = NULL;
done:
	spin_unlock_irqrestore(&config->lock, flags);
	return ret;

err_giveback:
	kfree(urb->hcpriv);
	usb_hcd_unlink_urb_from_ep(hcd, urb);
	spin_unlock_irqrestore(&config->lock, flags);
	usb_hcd_giveback_urb(hcd, urb, -ESHUTDOWN);
	return ret;
}

static void hc_endpoint_disable(struct usb_hcd *hcd, struct usb_host_endpoint *ld_ep)
{
	struct HOST_USB_DEVICE *usbDev;
	int ep_num = usb_endpoint_num(&ld_ep->desc);

	if (ep_num > MAX_INSTANCE_EP_NUM)
		ep_num = MAX_INSTANCE_EP_NUM;

	usbDev = (struct HOST_USB_DEVICE *)ld_ep->hcpriv;
	if (!usbDev)
		return;

	if (ld_ep->desc.bEndpointAddress) {
		if (usb_endpoint_dir_in(&ld_ep->desc)) {
			if (!usbDev->in_ep[ep_num]) {
				usbDev->in_ep[ep_num]->userExt = NULL;
				INIT_LIST_HEAD(&usbDev->in_ep[ep_num]->reqList);
				kfree(usbDev->in_ep[ep_num]);
				usbDev->in_ep[ep_num] = NULL;
			}
		} else {
			if (!usbDev->out_ep[ep_num]) {
				usbDev->out_ep[ep_num]->userExt = NULL;
				INIT_LIST_HEAD(&usbDev->out_ep[ep_num]->reqList);
				kfree(usbDev->out_ep[ep_num]);
				usbDev->out_ep[ep_num] = NULL;
			}
		}
	}
}

static int hc_get_frame(struct usb_hcd *hcd)
{
	return 0;
}

static int hc_alloc_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct HOST_USB_DEVICE *usbDev;
	struct phytium_cusb *config;
	struct HOST_CTRL *priv;
	unsigned long flags = 0;
	int index;
	int slot = -EINVAL;

	if (!hcd || !udev)
		return -EINVAL;

	config = *(struct phytium_cusb **)hcd->hcd_priv;
	if (!config)
		return 0;

	spin_lock_irqsave(&config->lock, flags);
	priv = config->host_priv;
	for (index = 0; index < MAX_SUPPORTED_DEVICES; index++) {
		if (priv->host_devices_table[index] == NULL) {
			slot = index;
			break;
		}
	}
	spin_unlock_irqrestore(&config->lock, flags);

	if (slot < 0)
		return -ENOMEM;

	usbDev = kzalloc((sizeof(struct HOST_USB_DEVICE) +
				config->host_obj->host_epGetPrivateDataSize(priv)), GFP_KERNEL);
	if (!usbDev)
		return -ENOMEM;

	usbDev->ep0_hep.hcPriv = &((uint8_t *)usbDev)[sizeof(struct HOST_USB_DEVICE)];
	usbDev->udev.userExt = (void *)usbDev;
	usbDev->ld_udev = udev;
	usbDev->ld_udev->slot_id = slot;
	usbDev->ep0_hep.desc.bLength = 7;
	usbDev->ep0_hep.desc.bDescriptorType = USB_DT_ENDPOINT;
	usbDev->in_ep[0] = &usbDev->ep0_hep;
	usbDev->out_ep[0] = &usbDev->ep0_hep;
	INIT_LIST_HEAD(&usbDev->ep0_hep.reqList);

	priv->host_devices_table[slot] = usbDev;

	return 1;
}

static void hc_free_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct HOST_CTRL *priv;
	struct HOST_USB_DEVICE *usbDev;
	struct phytium_cusb *config;
	int i;

	if (!hcd || !udev)
		return;

	config = *(struct phytium_cusb **)(hcd->hcd_priv);
	if (!config)
		return;

	priv = (struct HOST_CTRL *)config->host_priv;
	usbDev = priv->host_devices_table[udev->slot_id];
	if (!usbDev)
		return;

	usbDev->in_ep[0] = NULL;
	usbDev->out_ep[0] = NULL;
	for (i = 1; i < HOST_EP_NUM; i++) {
		if (usbDev->in_ep[i])
			hc_endpoint_disable(hcd,
					(struct usb_host_endpoint *)usbDev->in_ep[i]->userExt);
		if (usbDev->out_ep[i])
			hc_endpoint_disable(hcd,
					(struct usb_host_endpoint *)usbDev->out_ep[i]->userExt);
	}

	priv->host_devices_table[udev->slot_id] = NULL;
	usbDev->ld_udev->slot_id = 0;
	usbDev->udev.userExt = (void *)NULL;
	kfree(usbDev);
}

static int hc_reset_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct HOST_CTRL *priv;
	struct HOST_USB_DEVICE *usb_device;
	struct phytium_cusb *config;

	if (!hcd || !udev)
		return -EINVAL;

	config = *(struct phytium_cusb **)hcd->hcd_priv;
	priv = (struct HOST_CTRL *)config->host_priv;
	usb_device = priv->host_devices_table[udev->slot_id];
	if (!usb_device)
		return -ENODEV;

	usb_device->udev.speed = usb_device->ld_udev->speed;
	usb_device->udev.devnum = usb_device->ld_udev->devnum;

	if (USB_SPEED_HIGH == usb_device->udev.speed || USB_SPEED_FULL == usb_device->udev.speed)
		usb_device->ep0_hep.desc.wMaxPacketSize = 64;
	else
		usb_device->ep0_hep.desc.wMaxPacketSize = 8;

	pr_debug("speed:%d ep0 wMaxPacketSize:%d\n", usb_device->udev.speed,
			usb_device->ep0_hep.desc.wMaxPacketSize);

	return 0;
}

static int hc_update_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct HOST_CTRL *priv;
	struct HOST_USB_DEVICE *usb_device;
	struct phytium_cusb *config;

	if (!hcd || !udev)
		return -EINVAL;

	config = *(struct phytium_cusb **)(hcd->hcd_priv);
	priv = (struct HOST_CTRL *)config->host_priv;

	usb_device = priv->host_devices_table[udev->slot_id];
	if (!usb_device)
		return -ENODEV;

	usb_device->udev.devnum = udev->devnum;

	return 0;
}

static int hc_add_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	return 0;
}

static int hc_drop_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	return 0;
}

static int hc_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct HOST_CTRL *priv;
	struct phytium_cusb *config = *(struct phytium_cusb **)hcd->hcd_priv;

	if (!config)
		return 0;

	priv = (struct HOST_CTRL *)config->host_priv;
	if (!priv)
		return 0;

	if (priv->portStatus & 0xffff0000) {
		*buf = 0x02;
		return 1;
	}

	return 0;
}

#ifdef CONFIG_PM
static int hc_bus_suspend(struct usb_hcd *hcd)
{
	unsigned long flags;
	struct phytium_cusb *config = *(struct phytium_cusb **)(hcd->hcd_priv);

	if (!config)
		return 0;

	spin_lock_irqsave(&config->lock, flags);
	hcd->state = HC_STATE_SUSPENDED;
	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}

static int hc_bus_resume(struct usb_hcd *hcd)
{
	unsigned long flags;
	struct phytium_cusb *config = *(struct phytium_cusb **)(hcd->hcd_priv);

	if (!config)
		return 0;

	spin_lock_irqsave(&config->lock, flags);
	hcd->state = HC_STATE_RESUMING;
	spin_unlock_irqrestore(&config->lock, flags);
	return 0;
}
#endif

static void host_giveback_request(struct HOST_CTRL *priv,
		struct HOST_REQ *req, uint32_t status)
{
	struct urb *urb_req;
	int urb_status = 0;
	int i = 0;
	struct phytium_cusb *config;

	if (!priv || !req)
		return;

	urb_req = req->userExt;
	urb_req->actual_length = req->actualLength;

	switch (status) {
	case HOST_ESTALL:
		urb_status = -EPIPE;
		break;
	case HOST_EUNHANDLED:
		urb_status = -EPROTO;
		break;
	case HOST_ESHUTDOWN:
		urb_status = -ESHUTDOWN;
		break;
	default:
		urb_status = status;
	}
	pr_debug("complete %p %pS (%d) dev%d ep%d%s %d/%d\n",
			urb_req, urb_req->complete, urb_status,
			usb_pipedevice(urb_req->pipe),
			usb_pipeendpoint(urb_req->pipe),
			usb_pipein(urb_req->pipe) ? "in" : "out",
			urb_req->actual_length,
			urb_req->transfer_buffer_length);

	if (usb_endpoint_xfer_isoc(&urb_req->ep->desc)) {
		for (i = 0; i < urb_req->number_of_packets; i++) {
			urb_req->iso_frame_desc[i].status = 0;
			urb_req->iso_frame_desc[i].actual_length = req->isoFramesDesc[i].length;
		}
	}

	config = dev_get_drvdata(priv->dev);
	usb_hcd_unlink_urb_from_ep(config->hcd, urb_req);
	spin_unlock(&config->lock);
	usb_hcd_giveback_urb(config->hcd, urb_req, urb_status);
	kfree(req);
	spin_lock(&config->lock);
}

static void host_rh_port_status_change(struct HOST_CTRL *priv)
{
	uint32_t statusHub;
	char *status;
	uint32_t retval;
	struct usb_ctrlrequest setup;
	struct phytium_cusb *config;

	if (!priv)
		return;

	config = dev_get_drvdata(priv->dev);
	if (!config)
		return;

	status = (char *)&statusHub;
	hc_hub_status_data(config->hcd, status);

	if (status) {
		setup.bRequestType = USB_TYPE_CLASS | USB_RECIP_OTHER | USB_DIR_IN;//to host
		setup.bRequest = USB_REQ_GET_STATUS;
		setup.wValue = 0;
		setup.wIndex = 1;
		setup.wLength = 4;
		retval = config->host_obj->host_vhubControl(priv, &setup, (uint8_t *)&statusHub);
		if (retval)
			return;

		if (status[1] & USB_PORT_STAT_C_CONNECTION) {
			if (status[0] & USB_PORT_STAT_CONNECTION) {
				if (config->hcd->status_urb)
					usb_hcd_poll_rh_status(config->hcd);
				else
					usb_hcd_resume_root_hub(config->hcd);
			} else {
				usb_hcd_resume_root_hub(config->hcd);
				usb_hcd_poll_rh_status(config->hcd);
			}
		}
	} else
		usb_hcd_resume_root_hub(config->hcd);
}

static void host_set_ep_toggle(struct HOST_CTRL *priv,
		struct HOST_DEVICE *udev, u8 ep_num, u8 is_in, u8 toggle)
{
	struct HOST_USB_DEVICE *device;

	if (!priv || !udev)
		return;

	device = (struct HOST_USB_DEVICE *)udev->userExt;

	usb_settoggle(device->ld_udev, ep_num, !is_in, toggle);
}

static u8 host_get_ep_toggle(struct HOST_CTRL *priv,
		struct HOST_DEVICE *udev, u8 ep_num, u8 is_in)
{
	struct HOST_USB_DEVICE *device;

	if (!priv || !udev)
		return 0;

	device = (struct HOST_USB_DEVICE *)udev->userExt;

	return usb_gettoggle(device->ld_udev, ep_num, !is_in);
}

static uint32_t initEndpoints(struct HOST_CTRL *priv)
{
	int epNum;

	if (!priv)
		return 0;

	priv->hwEpInCount = 0;
	priv->hwEpOutCount = 0;
	phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0);
	phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | FIFOCTRL_IO_TX | 0);

	for (epNum = 0; epNum < HOST_EP_NUM; epNum++) {
		priv->in[epNum].isInEp = 1;
		priv->in[epNum].hwEpNum = epNum;
		if (priv->hostCfg.epIN[epNum].bufferingValue) {
			priv->in[epNum].hwMaxPacketSize = priv->hostCfg.epIN[epNum].maxPacketSize;
			priv->in[epNum].hwBuffers = priv->hostCfg.epIN[epNum].bufferingValue;
			priv->in[epNum].state = HOST_EP_FREE;
			priv->in[epNum].channel = NULL;
			priv->hwEpInCount++;

			if (epNum) {
				phytium_write16(&priv->regs->rxstaddr[epNum - 1].addr,
						priv->hostCfg.epIN[epNum].startBuf);
				phytium_write8(&priv->regs->ep[epNum - 1].rxcon, 0x08);
				phytium_write8(&priv->regs->fifoctrl, FIFOCTRL_FIFOAUTO | epNum);
				phytium_write8(&priv->regs->irqmode[epNum - 1].inirqmode, 0x80);
			}
		} else
			priv->in[epNum].state = HOST_EP_NOT_IMPLEMENTED;

		priv->out[epNum].isInEp = 0;
		priv->out[epNum].hwEpNum = epNum;
		if (priv->hostCfg.epOUT[epNum].bufferingValue) {
			priv->out[epNum].hwMaxPacketSize = priv->hostCfg.epOUT[epNum].maxPacketSize;
			priv->out[epNum].hwBuffers = priv->hostCfg.epOUT[epNum].bufferingValue;
			priv->out[epNum].state = HOST_EP_FREE;
			priv->out[epNum].channel = NULL;
			priv->hwEpInCount++;

			if (epNum) {
				phytium_write16(&priv->regs->txstaddr[epNum - 1].addr,
						priv->hostCfg.epOUT[epNum].startBuf);
				phytium_write8(&priv->regs->ep[epNum - 1].txcon, 0x08);
				phytium_write8(&priv->regs->fifoctrl,
						FIFOCTRL_FIFOAUTO | FIFOCTRL_IO_TX | epNum);
				phytium_write8(&priv->regs->irqmode[epNum - 1].outirqmode, 0x80);
			}
		} else
			priv->out[epNum].state = HOST_EP_NOT_IMPLEMENTED;
	}

	return 0;
}

int32_t hostInit(struct HOST_CTRL *priv, struct HOST_CFG *config,
		struct HOST_CALLBACKS *callbacks, struct device *pdev, bool isVhubHost)
{
	int index;

	if (!config || !priv || !callbacks || !pdev)
		return -EINVAL;

	priv->dev = pdev;
	priv->hostCallbacks = *callbacks;
	priv->hostCfg = *config;
	priv->regs = (struct HW_REGS *)config->regBase;
	priv->hostDrv = HOST_GetInstance();

	priv->dmaDrv = DMA_GetInstance();
	priv->dmaCfg.dmaModeRx = 0xFFFF;
	priv->dmaCfg.dmaModeTx = 0xFFFF;
	priv->dmaCfg.regBase = config->regBase + 0x400;
	priv->dmaCfg.trbAddr = config->trbAddr;
	priv->dmaCfg.trbDmaAddr = config->trbDmaAddr;
	priv->dmaCallback.complete = host_CallbackTransfer;

	priv->dmaController = (void *)(priv + 1);
	priv->dmaDrv->dma_init(priv->dmaController, &priv->dmaCfg, &priv->dmaCallback);
	priv->dmaDrv->dma_setParentPriv(priv->dmaController, priv);

	INIT_LIST_HEAD(&priv->ctrlHEpQueue);

	for (index = 0; index < MAX_INSTANCE_EP_NUM; index++) {
		INIT_LIST_HEAD(&priv->isoInHEpQueue[index]);
		INIT_LIST_HEAD(&priv->isoOutHEpQueue[index]);
		INIT_LIST_HEAD(&priv->intInHEpQueue[index]);
		INIT_LIST_HEAD(&priv->intOutHEpQueue[index]);
		INIT_LIST_HEAD(&priv->bulkInHEpQueue[index]);
		INIT_LIST_HEAD(&priv->bulkOutHEpQueue[index]);
	}

	phytium_write8(&priv->regs->cpuctrl, BIT(1));
	phytium_write8(&priv->regs->otgctrl, OTGCTRL_ABUSDROP);
	phytium_write8(&priv->regs->ep0maxpack, 0x40);

	//disable interrupts
	phytium_write8(&priv->regs->otgien, 0x0);
	phytium_write8(&priv->regs->usbien, 0x0);
	phytium_write16(&priv->regs->txerrien, 0x0);
	phytium_write16(&priv->regs->rxerrien, 0x0);

	//clear all interrupt except otg idle irq
	phytium_write8(&priv->regs->otgirq, 0xFE);
	phytium_write8(&priv->regs->usbirq, 0xFF);
	phytium_write16(&priv->regs->txerrirq, 0xFF);
	phytium_write16(&priv->regs->rxerrirq, 0xFF);

	phytium_write8(&priv->regs->tawaitbcon, 0x80);

	initEndpoints(priv);
	priv->otgState = HOST_OTG_STATE_B_IDLE;

	//reset all endpoint
	phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST | ENDPRST_IO_TX);
	phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST);

	if (isVhubHost)
		priv->vhub_regs = (struct VHUB_REGS *)(config->phy_regBase);
	else
		priv->custom_regs = (struct CUSTOM_REGS *)(config->phy_regBase);

	return 0;
}

void hostDestroy(struct HOST_CTRL *priv)
{
}

void hostStart(struct HOST_CTRL *priv)
{
	uint8_t otgstate, usbien;

	if (!priv)
		return;

	priv->dmaDrv->dma_start(priv->dmaController);
	usbien = phytium_read8(&priv->regs->usbien);
	usbien = usbien | USBIR_URES | USBIR_SUSP;
	phytium_write8(&priv->regs->usbien, usbien);
retry:
	otgstate = phytium_read8(&priv->regs->otgstate);
	switch (otgstate) {
	case HOST_OTG_STATE_A_IDLE:
		priv->ep0State = HOST_EP0_STAGE_IDLE;
		priv->otgState = HOST_OTG_STATE_A_IDLE;
		phytium_write8(&priv->regs->otgirq, OTGIRQ_IDLEIRQ);
		host_SetVbus(priv, 1);
		break;
	case HOST_OTG_STATE_B_IDLE:
		host_SetVbus(priv, 1);
		break;
	case HOST_OTG_STATE_A_WAIT_VFALL:
		goto retry;
	}

	phytium_write8(&priv->regs->otgien, OTGIRQ_CONIRQ |
			OTGIRQ_VBUSERRIRQ | OTGIRQ_SRPDETIRQ);
}

void hostStop(struct HOST_CTRL *priv)
{
	if (!priv)
		return;

	phytium_write8(&priv->regs->otgien, 0x0);
	phytium_write8(&priv->regs->usbien, 0x0);
	phytium_write16(&priv->regs->txerrien, 0x0);
	phytium_write16(&priv->regs->rxerrien, 0x0);

	phytium_write8(&priv->regs->otgirq, 0xFE);
	phytium_write8(&priv->regs->usbirq, 0xFF);
	phytium_write16(&priv->regs->txerrirq, 0xFF);
	phytium_write16(&priv->regs->rxerrirq, 0xFF);

	host_SetVbus(priv, 0);
	priv->dmaDrv->dma_stop(priv->dmaController);
}

static void handleReset(struct HOST_CTRL *priv)
{
	if (!priv)
		return;

	if (priv->otgState == HOST_OTG_STATE_A_WAIT_BCON
			|| priv->otgState == HOST_OTG_STATE_B_WAIT_ACON) {
		switch (phytium_read8(&priv->regs->speedctrl)) {
		case SPEEDCTRL_HS:
			priv->portStatus |= USB_PORT_STAT_HIGH_SPEED;
			break;
		case SPEEDCTRL_FS:
			priv->portStatus &= ~(USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_LOW_SPEED);
			break;
		case SPEEDCTRL_LS:
			priv->portStatus |= USB_PORT_STAT_LOW_SPEED;
			break;
		}

		priv->dmaDrv->dma_setHostMode(priv->dmaController);

		if (priv->hostCallbacks.portStatusChange)
			priv->hostCallbacks.portStatusChange(priv);

		switch (phytium_read8(&priv->regs->otgstate)) {
		case HOST_OTG_STATE_B_HOST:
			priv->otgState = HOST_OTG_STATE_B_HOST;
			break;
		case HOST_OTG_STATE_A_HOST:
			break;
		default:
			priv->otgState = HOST_OTG_STATE_A_HOST;
			break;
		}
	}
}

void hostIsr(struct HOST_CTRL *priv)
{
	uint8_t usbirq, usbien;

	if (!priv)
		return;

	usbirq = phytium_read8(&priv->regs->usbirq);
	usbien = phytium_read8(&priv->regs->usbien);
	pr_debug("raw usbirq:0x%x usbien:0x%x\n", usbirq, usbien);
	usbirq = usbirq & usbien;

	hostErrorIrq(priv);
	hostOtgIrq(priv);

	if (!usbirq)
		goto DMA_IRQ;

	if (usbirq & USBIR_URES) {
		phytium_write8(&priv->regs->usbirq, USBIR_URES);
		priv->port_resetting = 0;
		handleReset(priv);
	}

	if (usbirq & USBIR_SOF)
		phytium_write8(&priv->regs->usbirq, USBIR_SOF);

	if (usbirq & USBIR_SUSP) {
		pr_debug("clear suspend irq\n");
		phytium_write8(&priv->regs->usbirq, USBIR_SUSP);
		phytium_write8(&priv->regs->clkgate, 0x7);
	}

	return;
DMA_IRQ:
	priv->dmaDrv->dma_isr(priv->dmaController);
}

int32_t hostEpDisable(struct HOST_CTRL *priv, struct HOST_EP *ep)
{
	return 0;
}

unsigned int get_endpoint_interval(struct usb_endpoint_descriptor desc,
		int speed)
{
	unsigned int interval = 0;

	switch (speed) {
	case USB_SPEED_HIGH:
		if (usb_endpoint_xfer_control(&desc) || usb_endpoint_xfer_bulk(&desc)) {
			if (desc.bInterval == 0)
				return interval;
			interval = fls(desc.bInterval) - 1;
			interval = clamp_val(interval, 0, 15);
			interval = 1 << interval;
			if (interval != desc.bInterval)
				pr_debug("rounding to %d microframes, desc %d microframes\n",
						interval, desc.bInterval);
			break;
		}

		if (usb_endpoint_xfer_isoc(&desc) || usb_endpoint_xfer_int(&desc)) {
			interval = clamp_val(desc.bInterval, 1, 16) - 1;
			interval = 1 << interval;
			if (interval != desc.bInterval - 1)
				pr_debug("rounding to %d %sframes\n", interval,
						speed == USB_SPEED_FULL ? "" : "micro");
		}
		break;
	case USB_SPEED_FULL:
		if (usb_endpoint_xfer_isoc(&desc)) {
			interval = clamp_val(desc.bInterval, 1, 16) - 1;
			if (interval != desc.bInterval - 1)
				pr_debug("rounding to %d %sframes\n", 1 << interval,
						speed == USB_SPEED_FULL ? "" : "micro");
			interval += 3;
			break;
		}
	/* fall through */
	case USB_SPEED_LOW:
		if (usb_endpoint_xfer_int(&desc) || usb_endpoint_xfer_isoc(&desc)) {
			interval = fls(desc.bInterval * 8) - 1;
			interval = clamp_val(interval, 3, 10);
			if ((1 << interval) != desc.bInterval * 8)
				pr_debug("rounding to %d microframes, desc %d microframes\n",
						1 << interval, desc.bInterval);
		}
	}

	return interval;
}

int32_t hostReqQueue(struct HOST_CTRL *priv, struct HOST_REQ *req)
{
	struct HOST_EP_PRIV *hostEpPriv;
	struct list_head *hEpQueue = NULL;
	uint8_t idleQueue = 0;

	if (!priv || !req)
		return -EINVAL;

	list_add_tail((struct list_head *)&req->list, (struct list_head *)&req->usbEp->reqList);
	hostEpPriv = (struct HOST_EP_PRIV *)req->usbEp->hcPriv;

	if (hostEpPriv->epIsReady) {
		if (usb_endpoint_xfer_isoc(&req->usbEp->desc)) {
			hostEpPriv->isocEpConfigured = 1;
			hostStartReq(priv, req);
		}
		return 0;
	}

	hostEpPriv->epIsReady = 1;
	hostEpPriv->maxPacketSize = req->usbEp->desc.wMaxPacketSize;
	hostEpPriv->interval = req->interval;
	hostEpPriv->epNum = req->usbEp->desc.bEndpointAddress & 0xf;
	hostEpPriv->faddress = req->faddress;
	hostEpPriv->usbHEp = req->usbEp;
	hostEpPriv->isIn = req->epIsIn;

	hostEpPriv->frame = get_endpoint_interval(req->usbEp->desc, req->usbDev->speed);
	hostEpPriv->interval = hostEpPriv->frame;
	switch (usb_endpoint_type(&req->usbEp->desc)) {
	case USB_ENDPOINT_XFER_CONTROL:
		hostEpPriv->isIn = 0;
		hEpQueue = &priv->ctrlHEpQueue;
		hostEpPriv->type = USB_ENDPOINT_XFER_CONTROL;
		break;
	case USB_ENDPOINT_XFER_BULK:
		hEpQueue = hostEpPriv->isIn ? &priv->bulkInHEpQueue[req->epNum - 1] :
			&priv->bulkOutHEpQueue[req->epNum - 1];
		hostEpPriv->type = USB_ENDPOINT_XFER_BULK;
		break;
	case USB_ENDPOINT_XFER_INT:
		hEpQueue = hostEpPriv->isIn ? &priv->intInHEpQueue[req->epNum - 1] :
			&priv->intOutHEpQueue[req->epNum - 1];
		hostEpPriv->type = USB_ENDPOINT_XFER_INT;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		hEpQueue = hostEpPriv->isIn ? &priv->isoInHEpQueue[req->epNum - 1] :
			&priv->isoOutHEpQueue[req->epNum - 1];
		hostEpPriv->type = USB_ENDPOINT_XFER_ISOC;
		break;
	default:
		break;
	}

	if (hostEpPriv->isIn)
		hostEpPriv->genericHwEp = &priv->in[req->epNum];
	else
		hostEpPriv->genericHwEp = &priv->out[req->epNum];

	hostEpPriv->genericHwEp->state = HOST_EP_ALLOCATED;
	hostEpPriv->genericHwEp->refCount++;

	if (list_empty(hEpQueue)) {
		if (hostEpPriv->genericHwEp->state == HOST_EP_BUSY) {
			pr_err("Error:Hardware endpoint %d is busy\n",
					hostEpPriv->genericHwEp->hwEpNum);
			return -EINVAL;
		}
		idleQueue = 1;
	}

	if (usb_endpoint_xfer_int(&req->usbEp->desc))
		list_add_tail(&hostEpPriv->node, hEpQueue);

	if (idleQueue)
		hostStartReq(priv, req);

	return 0;
}

static int abortActuallyUsbRequest(struct HOST_CTRL *priv,
		struct HOST_REQ *req, struct HOST_EP *usbEp)
{
	struct HOST_EP_PRIV *usbEpPriv;
	struct HostEp *hostEp;
	uint16_t rxerrien = 0;
	uint16_t txerrien = 0;
	uint8_t rxcon, txcon;

	if (!priv || !req || !usbEp)
		return -EINVAL;

	usbEpPriv = (struct HOST_EP_PRIV *)usbEp->hcPriv;
	hostEp = usbEpPriv->currentHwEp;

	usbEpPriv->transferFinished = 1;

	if (hostEp->isInEp) {
		if (hostEp->hwEpNum) {
			rxcon = phytium_read8(&priv->regs->ep[hostEp->hwEpNum - 1].rxcon);
			rxcon = rxcon & (~BIT(7));
			phytium_write8(&priv->regs->ep[hostEp->hwEpNum - 1].rxcon, rxcon);
		}
		rxerrien = phytium_read16(&priv->regs->rxerrien);
		rxerrien &= ~(1 << hostEp->hwEpNum);
		phytium_write16(&priv->regs->rxerrien, rxerrien);
		phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST |
				ENDPRST_IO_TX | hostEp->hwEpNum);
	} else {
		if (hostEp->hwEpNum) {
			txcon = phytium_read8(&priv->regs->ep[hostEp->hwEpNum - 1].txcon);
			txcon = txcon & (~BIT(7));
			phytium_write8(&priv->regs->ep[hostEp->hwEpNum - 1].txcon, txcon);
		}
		txerrien = phytium_read16(&priv->regs->txerrien);
		txerrien &= ~(1 << hostEp->hwEpNum);
		phytium_write16(&priv->regs->txerrien, txerrien);
		phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | hostEp->hwEpNum);
	}

	scheduleNextTransfer(priv, req, hostEp);

	return 0;
}

int32_t hostReqDequeue(struct HOST_CTRL *priv, struct HOST_REQ *req, uint32_t status)
{
	struct HOST_EP *usbEp;
	struct HOST_EP_PRIV *usbEpPriv;
	int ret = 0;

	if (!priv || !req)
		return -EINVAL;

	usbEp = req->usbEp;
	usbEpPriv = (struct HOST_EP_PRIV *)usbEp->hcPriv;

	pr_debug("Dequeue usbReq:%p dev%d usbEp%d%s\n", req, req->faddress, req->epNum,
			req->epIsIn ? "in" : "out");

	if (!usbEpPriv->epIsReady)
		return 0;

	if (!usbEpPriv->currentHwEp || req->list.prev != &usbEp->reqList) {
		givebackRequest(priv, req, status);
		if (list_empty(&usbEp->reqList)) {
			usbEpPriv->epIsReady = 0;
			usbEpPriv->currentHwEp = NULL;
			if (usb_endpoint_xfer_int(&usbEp->desc))
				list_del(&usbEpPriv->node);
		}
	} else
		ret = abortActuallyUsbRequest(priv, req, usbEp);

	if (usbEpPriv->isocEpConfigured)
		usbEpPriv->isocEpConfigured = 0;

	return ret;
}

int32_t hostVHubStatusData(struct HOST_CTRL *priv, uint8_t *status)
{
	return 0;
}

int32_t hostGetDevicePD(struct HOST_CTRL *priv)
{
	return 0;
}

int32_t hostGetPrivateDataSize(struct HOST_CTRL *priv)
{
	if (!priv)
		return 0;

	return sizeof(struct HOST_EP_PRIV);
}

static int hub_descriptor(struct usb_hub_descriptor *buf)
{
	buf->bDescLength = 9;
	buf->bDescriptorType = 0x29;
	buf->bNbrPorts = 1;
	buf->wHubCharacteristics = 0x11;
	buf->bPwrOn2PwrGood = 5;
	buf->bHubContrCurrent = 0;
	buf->u.hs.DeviceRemovable[0] = 0x2;

	return 0;
}

static int HubPortSuspend(struct HOST_CTRL *priv, u16 on)
{
	uint8_t otgctrl;

	if (!priv)
		return 0;

	otgctrl = phytium_read8(&priv->regs->otgctrl);

	if (on) {
		otgctrl &= ~OTGCTRL_BUSREQ;
		otgctrl &= ~OTGCTRL_BHNPEN;

		priv->portStatus |= USB_PORT_STAT_SUSPEND;

		switch (phytium_read8(&priv->regs->otgstate)) {
		case HOST_OTG_STATE_A_HOST:
			priv->otgState = HOST_OTG_STATE_A_SUSPEND;
			otgctrl |= OTGCTRL_ASETBHNPEN;
			break;
		case HOST_OTG_STATE_B_HOST:
			priv->otgState = HOST_OTG_STATE_B_HOST_2;
			break;
		default:
			break;
		}
		phytium_write8(&priv->regs->otgctrl, otgctrl);
	} else {
		otgctrl |= OTGCTRL_BUSREQ;
		otgctrl &= ~OTGCTRL_ASETBHNPEN;
		phytium_write8(&priv->regs->otgctrl, otgctrl);
		priv->portStatus |= USB_PORT_STAT_RESUME;
	}
	return 0;
}

static void HubPortReset(struct HOST_CTRL *priv, uint8_t on)
{
	uint8_t speed;

	if (!priv)
		return;

	if (on) {
		phytium_write16(&priv->regs->txerrirq, 0xFFFF);
		phytium_write16(&priv->regs->txirq, 0xFFFF);
		phytium_write16(&priv->regs->rxerrirq, 0xFFFF);
		phytium_write16(&priv->regs->rxirq, 0xFFFF);

		phytium_write8(&priv->regs->endprst, ENDPRST_IO_TX);
		phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST |
				ENDPRST_TOGRST | ENDPRST_IO_TX);
		phytium_write8(&priv->regs->endprst, ENDPRST_FIFORST | ENDPRST_TOGRST);
		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0 | 0x04);
		phytium_write8(&priv->regs->ep0fifoctrl, FIFOCTRL_FIFOAUTO |
				FIFOCTRL_IO_TX | 0 | 0x04);

		priv->portStatus |= USB_PORT_STAT_RESET;
		priv->portStatus &= ~USB_PORT_STAT_ENABLE;
		priv->port_resetting = 0;
	} else {
		speed = phytium_read8(&priv->regs->speedctrl);
		if (speed == SPEEDCTRL_HS)
			priv->portStatus |= USB_PORT_STAT_HIGH_SPEED;
		else if (speed == SPEEDCTRL_FS)
			priv->portStatus &= ~(USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_LOW_SPEED);
		else
			priv->portStatus |= USB_PORT_STAT_LOW_SPEED;

		priv->portStatus &= ~USB_PORT_STAT_RESET;
		priv->portStatus |= USB_PORT_STAT_ENABLE | (USB_PORT_STAT_C_RESET << 16)
			| (USB_PORT_STAT_C_ENABLE << 16);

		if (priv->hostCallbacks.portStatusChange)
			priv->hostCallbacks.portStatusChange(priv);
	}
}

static int get_PortStatus(struct HOST_CTRL *priv, u16 wIndex, uint8_t *buff)
{
	uint32_t temp = 0;

	if (!priv || !buff)
		return 0;

	if ((wIndex & 0xff) != 1)
		return 1;

	if (priv->portStatus & USB_PORT_STAT_RESET) {
		if (!priv->port_resetting)
			HubPortReset(priv, 0);
	}

	if (priv->portStatus & USB_PORT_STAT_RESUME) {
		priv->portStatus &= ~(USB_PORT_STAT_SUSPEND | USB_PORT_STAT_RESUME);
		priv->portStatus |= USB_PORT_STAT_C_SUSPEND << 16;
		if (priv->hostCallbacks.portStatusChange)
			priv->hostCallbacks.portStatusChange(priv);
	}

	temp = priv->portStatus & (~USB_PORT_STAT_RESUME);
	buff[0] = temp;
	buff[1] = temp >> 8;
	buff[2] = temp >> 16;
	buff[3] = temp >> 24;

	return 0;
}

static int set_PortFeature(struct HOST_CTRL *priv, u16 wValue, u16 wIndex)
{
//	struct HW_Regs *regs = priv->regs;

	if ((wIndex & 0xff) != 1)
		return 1;

	switch (wValue) {
	case USB_PORT_FEAT_CONNECTION:
		break;
	case USB_PORT_FEAT_ENABLE:
		break;
	case USB_PORT_FEAT_SUSPEND:
		HubPortSuspend(priv, 1);
		break;
	case USB_PORT_FEAT_OVER_CURRENT:
		break;
	case USB_PORT_FEAT_RESET:
		HubPortReset(priv, 1);
		break;
	case USB_PORT_FEAT_L1:
		break;
	case USB_PORT_FEAT_POWER:
		hostStart(priv);
		break;
	case USB_PORT_FEAT_LOWSPEED:
		break;
	case USB_PORT_FEAT_C_CONNECTION:
		break;
	case USB_PORT_FEAT_C_ENABLE:
		break;
	case USB_PORT_FEAT_C_SUSPEND:
		break;
	case USB_PORT_FEAT_C_OVER_CURRENT:
		break;
	case USB_PORT_FEAT_INDICATOR:
		break;
	case USB_PORT_FEAT_C_PORT_L1:
		break;
	default:
		break;
	}
	priv->portStatus |= 1 << wValue;

	return 0;
}

static int Clear_PortFeature(struct HOST_CTRL *priv, u16 wValue, u16 wIndex)
{
	if ((wIndex & 0xff) != 1)
		return 1;

	switch (wValue) {
	case USB_PORT_FEAT_CONNECTION:
		break;
	case USB_PORT_FEAT_ENABLE:
		break;
	case USB_PORT_FEAT_SUSPEND:
		HubPortSuspend(priv, 0);
		break;
	case USB_PORT_FEAT_OVER_CURRENT:
		break;
	case USB_PORT_FEAT_RESET:
		break;
	case USB_PORT_FEAT_L1:
		break;
	case USB_PORT_FEAT_POWER:
		break;
	case USB_PORT_FEAT_LOWSPEED:
		break;
	case USB_PORT_FEAT_C_CONNECTION:
		break;
	case USB_PORT_FEAT_C_ENABLE:
		break;
	case USB_PORT_FEAT_C_SUSPEND:
		break;
	case USB_PORT_FEAT_C_OVER_CURRENT:
		break;
	case USB_PORT_FEAT_INDICATOR:
		break;
	case USB_PORT_FEAT_C_PORT_L1:
		break;
	default:
		break;
	}
	priv->portStatus &= ~(1 << wValue);

	return 0;
}

static int hc_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buf, u16 wLength)
{
	unsigned long flags = 0;
	int retval = 0;
	struct HOST_CTRL *priv;
	struct phytium_cusb *config = *(struct phytium_cusb **)hcd->hcd_priv;

	if (!config)
		return -EINVAL;

	if (!buf)
		return -EINVAL;

	if (unlikely(!HCD_HW_ACCESSIBLE(hcd))) {
		spin_unlock_irqrestore(&config->lock, flags);
		return -ESHUTDOWN;
	}

	priv = (struct HOST_CTRL *)config->host_priv;
	if (!priv)
		return -EINVAL;

	spin_lock_irqsave(&config->lock, flags);
	switch (typeReq) {
	case GetHubStatus:
		break;
	case GetPortStatus:
		get_PortStatus(priv, wIndex, (uint8_t *)buf);
		break;
	case GetHubDescriptor:
		hub_descriptor((struct usb_hub_descriptor *)buf);
		break;
	case SetPortFeature:
		set_PortFeature(priv, wValue, wIndex);
		break;
	case ClearPortFeature:
		retval = Clear_PortFeature(priv, wValue, wIndex);
		break;
	case SetHubFeature:
		break;
	case ClearHubFeature:
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&config->lock, flags);

	return retval;
}

int32_t hostVHubControl(struct HOST_CTRL *priv, struct usb_ctrlrequest *setup, uint8_t *buff)
{
	uint16_t request;
	uint32_t retval = 0;

	if (!priv || !setup || !buff)
		return -EINVAL;

	request = setup->bRequestType << 0x08 | setup->bRequest;
	switch (request) {
	case ClearHubFeature:
		break;
	case SetHubFeature:
		break;
	case ClearPortFeature:
		retval = Clear_PortFeature(priv, setup->wValue, setup->wIndex);
		break;
	case SetPortFeature:
		retval = set_PortFeature(priv, setup->wValue, setup->wIndex);
		break;
	case GetHubDescriptor:
		retval = hub_descriptor((struct usb_hub_descriptor *)buff);
		break;
	case GetHubStatus:
		break;
	case GetPortStatus:
		retval = get_PortStatus(priv, setup->wIndex, (uint8_t *)buff);
		break;
	default:
		retval = EOPNOTSUPP;
		break;
	}

	return retval;
}

struct HOST_OBJ hostDrv = {
	.host_init	=	hostInit,
	.host_destroy	=	hostDestroy,
	.host_start	=	hostStart,
	.host_stop	=	hostStop,
	.host_isr	=	hostIsr,

	//endpoint operation
	.host_epDisable =	hostEpDisable,
	.host_reqQueue =	hostReqQueue,
	.host_reqDequeue =	hostReqDequeue,
	.host_vhubStatusData =	hostVHubStatusData,
	.host_vhubControl =	hostVHubControl,
	.host_getDevicePD =	hostGetDevicePD,
	.host_epGetPrivateDataSize = hostGetPrivateDataSize,
};

static struct hc_driver host_driver = {
	.description	=	"phytium-hcd",
	.product_desc	=	"Phytium Host USB Driver",
	.hcd_priv_size	=	sizeof(struct phytium_cusb *),
	.flags		=	HCD_MEMORY | HCD_USB2 | HCD_DMA,
	.reset		=	hc_reset,
	.start		=	hc_start,
	.stop		=	hc_stop,
	.shutdown	=	hc_shutdown,
	.urb_enqueue	=	hc_urb_enqueue,
	.urb_dequeue	=	hc_urb_dequeue,
	.endpoint_disable =	hc_endpoint_disable,
	.get_frame_number =	hc_get_frame,
	.alloc_dev	=	hc_alloc_dev,
	.free_dev	=	hc_free_dev,
	.reset_device	=	hc_reset_device,
	.update_device	=	hc_update_device,
	.add_endpoint	=	hc_add_endpoint,
	.drop_endpoint	=	hc_drop_endpoint,
	.hub_status_data =	hc_hub_status_data,
	.hub_control	=	hc_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend	=	hc_bus_suspend,
	.bus_resume	=	hc_bus_resume,
#endif
};

static int phytium_host_set_default_cfg(struct phytium_cusb *config)
{
	int index;

	config->host_cfg.regBase = (uintptr_t)config->regs;
	config->host_cfg.phy_regBase = (uintptr_t)config->phy_regs;
	config->host_cfg.dmultEnabled = 1;
	config->host_cfg.memoryAlignment = 0;
	config->host_cfg.dmaSupport = 1;
	config->host_cfg.isEmbeddedHost = 1;

	for (index = 0; index < HOST_EP_NUM; index++) {
		if (index == 0) {
			config->host_cfg.epIN[index].bufferingValue = 1;
			config->host_cfg.epIN[index].maxPacketSize = 64;
			config->host_cfg.epIN[index].startBuf = 0;

			config->host_cfg.epOUT[index].bufferingValue = 1;
			config->host_cfg.epOUT[index].maxPacketSize = 64;
			config->host_cfg.epOUT[index].startBuf = 0;
		} else {
			config->host_cfg.epIN[index].bufferingValue = 2;
			config->host_cfg.epIN[index].maxPacketSize = 1024;
			config->host_cfg.epIN[index].startBuf = 64 + 2 * 1024 * (index - 1);

			config->host_cfg.epOUT[index].bufferingValue = 2;
			config->host_cfg.epOUT[index].maxPacketSize = 1024;
			config->host_cfg.epOUT[index].startBuf = 64 + 2 * 1024 * (index - 1);
		}
	}

	return 0;
}

static int phytium_host_reinit(struct phytium_cusb *config)
{
	struct HOST_CTRL *ctrl;

	if (!config || !config->host_priv)
		return 0;

	ctrl = (struct HOST_CTRL *)config->host_priv;

	usb_root_hub_lost_power(config->hcd->self.root_hub);
	hostStop(ctrl);

	ctrl->portStatus = 0;

	config->host_obj->host_init(config->host_priv, &config->host_cfg,
			&config->host_callbacks, config->dev, config->isVhubHost);

	return 0;
}

int phytium_host_init(struct phytium_cusb *config)
{
	int ret;

	if (!config)
		return 0;

	phytium_host_set_default_cfg(config);
	config->host_obj = HOST_GetInstance();
	config->dma_cfg.regBase = config->host_cfg.regBase + 0x400;

	config->dma_obj = DMA_GetInstance();
	config->dma_obj->dma_probe(&config->dma_cfg, &config->dma_sysreq);

	config->host_sysreq.privDataSize = sizeof(struct HOST_CTRL);
	config->host_sysreq.trbMemSize = config->dma_sysreq.trbMemSize;
	config->host_sysreq.privDataSize += config->dma_sysreq.privDataSize;

	config->host_priv = devm_kzalloc(config->dev, config->host_sysreq.privDataSize, GFP_KERNEL);
	if (!config->host_priv) {
		ret = -ENOMEM;
		goto err_probe;
	}

	config->host_cfg.trbAddr = dma_alloc_coherent(config->dev, config->host_sysreq.trbMemSize,
			(dma_addr_t *)&config->host_cfg.trbDmaAddr, GFP_KERNEL);
	if (!config->host_cfg.trbAddr) {
		ret = -ENOMEM;
		goto err_dma_coherent;
	}

	config->host_callbacks.portStatusChange = host_rh_port_status_change;
	config->host_callbacks.getEpToggle = host_get_ep_toggle;
	config->host_callbacks.setEpToggle = host_set_ep_toggle;
	config->host_callbacks.givebackRequest = host_giveback_request;

	config->host_obj->host_init(config->host_priv, &config->host_cfg,
			&config->host_callbacks, config->dev, config->isVhubHost);

	config->hcd = usb_create_hcd(&host_driver, config->dev, dev_name(config->dev));
	if (!config->hcd) {
		ret = -ENODEV;
		goto err_host;
	}

	*config->hcd->hcd_priv = (unsigned long)config;
	config->hcd->self.uses_pio_for_control = 0;
	config->hcd->uses_new_polling = 1;
	config->hcd->has_tt = 1;

	dev_set_drvdata(config->dev, config);

	config->hcd->self.otg_port = 1;
	config->hcd->power_budget = 500;
	ret = usb_add_hcd(config->hcd, 0, 0);
	if (ret < 0)
		goto err_setup;

	return 0;
err_setup:
	usb_put_hcd(config->hcd);
err_host:
	config->host_obj->host_destroy(config->host_priv);
	dma_free_coherent(config->dev, config->host_sysreq.trbMemSize,
			config->host_cfg.trbAddr, config->host_cfg.trbDmaAddr);
err_dma_coherent:
err_probe:
	dev_set_drvdata(config->dev, NULL);
	return ret;
}

int phytium_host_uninit(struct phytium_cusb *config)
{
	if (!config)
		return 0;

	if (config->hcd) {
		usb_remove_hcd(config->hcd);
		usb_put_hcd(config->hcd);
		config->hcd = NULL;
	}

	return 0;
}

#ifdef CONFIG_PM
int phytium_host_resume(void *priv)
{
	int otgctrl;
	struct phytium_cusb *config = (struct phytium_cusb *)priv;
	struct HOST_CTRL *ctrl = (struct HOST_CTRL *)config->host_priv;

	if (!ctrl)
		return -EINVAL;

	otgctrl = phytium_read8(&ctrl->regs->otgctrl);
	otgctrl |= 1;
	phytium_write8(&ctrl->regs->otgctrl, otgctrl);

	phytium_host_reinit(config);

	return 0;
}

int phytium_host_suspend(void *priv)
{
	int otgctrl;
	struct phytium_cusb *config = (struct phytium_cusb *)priv;
	struct HOST_CTRL *ctrl = (struct HOST_CTRL *)config->host_priv;

	if (!ctrl)
		return -EINVAL;

	otgctrl = phytium_read8(&ctrl->regs->otgctrl);
	otgctrl = otgctrl & (~1);
	phytium_write8(&ctrl->regs->otgctrl, otgctrl);

	return 0;
}
#endif

struct HOST_OBJ *HOST_GetInstance(void)
{
	return &hostDrv;
}
