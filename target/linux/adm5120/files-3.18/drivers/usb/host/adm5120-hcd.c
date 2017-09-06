/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci-hcd.c
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2004 David Brownell <dbrownell@users.sourceforge.net>
 *
 *   [ Initialisation is based on Linus'  ]
 *   [ uhci code and gregs ahcd fragments ]
 *   [ (C) Copyright 1999 Linus Torvalds  ]
 *   [ (C) Copyright 1999 Gregory P. Smith]
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/usb.h>
#include <linux/usb/otg.h>
#include <linux/usb/hcd.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/debugfs.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#define DRIVER_VERSION	"0.27.0"
#define DRIVER_AUTHOR	"Gabor Juhos <juhosg@openwrt.org>"
#define DRIVER_DESC	"ADMtek USB 1.1 Host Controller Driver"

/*-------------------------------------------------------------------------*/

#undef ADMHC_VERBOSE_DEBUG	/* not always helpful */

/* For initializing controller (mask in an HCFS mode too) */
#define	OHCI_CONTROL_INIT	OHCI_CTRL_CBSR

#define	ADMHC_INTR_INIT \
		(ADMHC_INTR_MIE | ADMHC_INTR_INSM | ADMHC_INTR_FATI \
		| ADMHC_INTR_RESI | ADMHC_INTR_TDC | ADMHC_INTR_BABI)

/*-------------------------------------------------------------------------*/

static const char hcd_name[] = "admhc-hcd";

#define	STATECHANGE_DELAY	msecs_to_jiffies(300)

#include "adm5120.h"

static void admhc_dump(struct admhcd *ahcd, int verbose);
static int admhc_init(struct admhcd *ahcd);
static void admhc_stop(struct usb_hcd *hcd);

#include "adm5120-dbg.c"
#include "adm5120-mem.c"
#include "adm5120-pm.c"
#include "adm5120-hub.c"
#include "adm5120-q.c"

/*-------------------------------------------------------------------------*/

/*
 * queue up an urb for anything except the root hub
 */
static int admhc_urb_enqueue(struct usb_hcd *hcd, struct urb *urb,
		gfp_t mem_flags)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	struct ed	*ed;
	struct urb_priv	*urb_priv;
	unsigned int	pipe = urb->pipe;
	int		td_cnt = 0;
	unsigned long	flags;
	int		ret = 0;

#ifdef ADMHC_VERBOSE_DEBUG
	spin_lock_irqsave(&ahcd->lock, flags);
	urb_print(ahcd, urb, "ENQEUE", usb_pipein(pipe), -EINPROGRESS);
	spin_unlock_irqrestore(&ahcd->lock, flags);
#endif

	/* every endpoint has an ed, locate and maybe (re)initialize it */
	ed = ed_get(ahcd, urb->ep, urb->dev, pipe, urb->interval);
	if (!ed)
		return -ENOMEM;

	/* for the private part of the URB we need the number of TDs */
	switch (ed->type) {
	case PIPE_CONTROL:
		if (urb->transfer_buffer_length > TD_DATALEN_MAX)
			/* td_submit_urb() doesn't yet handle these */
			return -EMSGSIZE;

		/* 1 TD for setup, 1 for ACK, plus ... */
		td_cnt = 2;
		/* FALLTHROUGH */
	case PIPE_BULK:
		/* one TD for every 4096 Bytes (can be up to 8K) */
		td_cnt += urb->transfer_buffer_length / TD_DATALEN_MAX;
		/* ... and for any remaining bytes ... */
		if ((urb->transfer_buffer_length % TD_DATALEN_MAX) != 0)
			td_cnt++;
		/* ... and maybe a zero length packet to wrap it up */
		if (td_cnt == 0)
			td_cnt++;
		else if ((urb->transfer_flags & URB_ZERO_PACKET) != 0
			&& (urb->transfer_buffer_length
				% usb_maxpacket(urb->dev, pipe,
					usb_pipeout(pipe))) == 0)
			td_cnt++;
		break;
	case PIPE_INTERRUPT:
		/*
		 * for Interrupt IN/OUT transactions, each ED contains
		 * only 1 TD.
		 * TODO: check transfer_buffer_length?
		 */
		td_cnt = 1;
		break;
	case PIPE_ISOCHRONOUS:
		/* number of packets from URB */
		td_cnt = urb->number_of_packets;
		break;
	}

	urb_priv = urb_priv_alloc(ahcd, td_cnt, mem_flags);
	if (!urb_priv)
		return -ENOMEM;

	urb_priv->ed = ed;

	spin_lock_irqsave(&ahcd->lock, flags);
	/* don't submit to a dead HC */
	if (!HCD_HW_ACCESSIBLE(hcd)) {
		ret = -ENODEV;
		goto fail;
	}
	if (!HC_IS_RUNNING(hcd->state)) {
		ret = -ENODEV;
		goto fail;
	}

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (ret)
		goto fail;

	/* schedule the ed if needed */
	if (ed->state == ED_IDLE) {
		ret = ed_schedule(ahcd, ed);
		if (ret < 0) {
			usb_hcd_unlink_urb_from_ep(hcd, urb);
			goto fail;
		}
		if (ed->type == PIPE_ISOCHRONOUS) {
			u16	frame = admhc_frame_no(ahcd);

			/* delay a few frames before the first TD */
			frame += max_t (u16, 8, ed->interval);
			frame &= ~(ed->interval - 1);
			frame |= ed->branch;
			urb->start_frame = frame;

			/* yes, only URB_ISO_ASAP is supported, and
			 * urb->start_frame is never used as input.
			 */
		}
	} else if (ed->type == PIPE_ISOCHRONOUS)
		urb->start_frame = ed->last_iso + ed->interval;

	/* fill the TDs and link them to the ed; and
	 * enable that part of the schedule, if needed
	 * and update count of queued periodic urbs
	 */
	urb->hcpriv = urb_priv;
	td_submit_urb(ahcd, urb);

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "admhc_urb_enqueue", urb_priv->ed, 1);
#endif

fail:
	if (ret)
		urb_priv_free(ahcd, urb_priv);

	spin_unlock_irqrestore(&ahcd->lock, flags);
	return ret;
}

/*
 * decouple the URB from the HC queues (TDs, urb_priv);
 * reporting is always done
 * asynchronously, and we might be dealing with an urb that's
 * partially transferred, or an ED with other urbs being unlinked.
 */
static int admhc_urb_dequeue(struct usb_hcd *hcd, struct urb *urb,
		int status)
{
	struct admhcd *ahcd = hcd_to_admhcd(hcd);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&ahcd->lock, flags);

#ifdef ADMHC_VERBOSE_DEBUG
	urb_print(ahcd, urb, "DEQUEUE", 1, status);
#endif
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret) {
		/* Do nothing */
		;
	} else if (HC_IS_RUNNING(hcd->state)) {
		struct urb_priv *urb_priv;

		/* Unless an IRQ completed the unlink while it was being
		 * handed to us, flag it for unlink and giveback, and force
		 * some upcoming INTR_SF to call finish_unlinks()
		 */
		urb_priv = urb->hcpriv;
		if (urb_priv) {
			if (urb_priv->ed->state == ED_OPER)
				start_ed_unlink(ahcd, urb_priv->ed);
		}
	} else {
		/*
		 * with HC dead, we won't respect hc queue pointers
		 * any more ... just clean up every urb's memory.
		 */
		if (urb->hcpriv)
			finish_urb(ahcd, urb, status);
	}
	spin_unlock_irqrestore(&ahcd->lock, flags);

	return ret;
}

/*-------------------------------------------------------------------------*/

/* frees config/altsetting state for endpoints,
 * including ED memory, dummy TD, and bulk/intr data toggle
 */

static void admhc_endpoint_disable(struct usb_hcd *hcd,
		struct usb_host_endpoint *ep)
{
	struct admhcd		*ahcd = hcd_to_admhcd(hcd);
	unsigned long		flags;
	struct ed		*ed = ep->hcpriv;
	unsigned		limit = 1000;

	/* ASSERT:  any requests/urbs are being unlinked */
	/* ASSERT:  nobody can be submitting urbs for this any more */

	if (!ed)
		return;

#ifdef ADMHC_VERBOSE_DEBUG
	spin_lock_irqsave(&ahcd->lock, flags);
	admhc_dump_ed(ahcd, "EP-DISABLE", ed, 1);
	spin_unlock_irqrestore(&ahcd->lock, flags);
#endif

rescan:
	spin_lock_irqsave(&ahcd->lock, flags);

	if (!HC_IS_RUNNING(hcd->state)) {
sanitize:
		ed->state = ED_IDLE;
		finish_unlinks(ahcd, 0);
	}

	switch (ed->state) {
	case ED_UNLINK:		/* wait for hw to finish? */
		/* major IRQ delivery trouble loses INTR_SOFI too... */
		if (limit-- == 0) {
			admhc_warn(ahcd, "IRQ INTR_SOFI lossage\n");
			goto sanitize;
		}
		spin_unlock_irqrestore(&ahcd->lock, flags);
		schedule_timeout_uninterruptible(1);
		goto rescan;
	case ED_IDLE:		/* fully unlinked */
		if (list_empty(&ed->td_list)) {
			td_free(ahcd, ed->dummy);
			ed_free(ahcd, ed);
			break;
		}
		/* else FALL THROUGH */
	default:
		/* caller was supposed to have unlinked any requests;
		 * that's not our job.  can't recover; must leak ed.
		 */
		admhc_err(ahcd, "leak ed %p (#%02x) state %d%s\n",
			ed, ep->desc.bEndpointAddress, ed->state,
			list_empty(&ed->td_list) ? "" : " (has tds)");
		td_free(ahcd, ed->dummy);
		break;
	}

	ep->hcpriv = NULL;

	spin_unlock_irqrestore(&ahcd->lock, flags);
}

static int admhc_get_frame_number(struct usb_hcd *hcd)
{
	struct admhcd *ahcd = hcd_to_admhcd(hcd);

	return admhc_frame_no(ahcd);
}

static void admhc_usb_reset(struct admhcd *ahcd)
{
#if 0
	ahcd->hc_control = admhc_readl(ahcd, &ahcd->regs->control);
	ahcd->hc_control &= OHCI_CTRL_RWC;
	admhc_writel(ahcd, ahcd->hc_control, &ahcd->regs->control);
#else
	/* FIXME */
	ahcd->host_control = ADMHC_BUSS_RESET;
	admhc_writel(ahcd, ahcd->host_control, &ahcd->regs->host_control);
#endif
}

/* admhc_shutdown forcibly disables IRQs and DMA, helping kexec and
 * other cases where the next software may expect clean state from the
 * "firmware".  this is bus-neutral, unlike shutdown() methods.
 */
static void
admhc_shutdown(struct usb_hcd *hcd)
{
	struct admhcd *ahcd;

	ahcd = hcd_to_admhcd(hcd);
	admhc_intr_disable(ahcd, ADMHC_INTR_MIE);
	admhc_dma_disable(ahcd);
	admhc_usb_reset(ahcd);
	/* flush the writes */
	admhc_writel_flush(ahcd);
}

/*-------------------------------------------------------------------------*
 * HC functions
 *-------------------------------------------------------------------------*/

static void admhc_eds_cleanup(struct admhcd *ahcd)
{
	if (ahcd->ed_tails[PIPE_INTERRUPT]) {
		ed_free(ahcd, ahcd->ed_tails[PIPE_INTERRUPT]);
		ahcd->ed_tails[PIPE_INTERRUPT] = NULL;
	}

	if (ahcd->ed_tails[PIPE_ISOCHRONOUS]) {
		ed_free(ahcd, ahcd->ed_tails[PIPE_ISOCHRONOUS]);
		ahcd->ed_tails[PIPE_ISOCHRONOUS] = NULL;
	}

	if (ahcd->ed_tails[PIPE_CONTROL]) {
		ed_free(ahcd, ahcd->ed_tails[PIPE_CONTROL]);
		ahcd->ed_tails[PIPE_CONTROL] = NULL;
	}

	if (ahcd->ed_tails[PIPE_BULK]) {
		ed_free(ahcd, ahcd->ed_tails[PIPE_BULK]);
		ahcd->ed_tails[PIPE_BULK] = NULL;
	}

	ahcd->ed_head = NULL;
}

#define ED_DUMMY_INFO	(ED_SPEED_FULL | ED_SKIP)

static int admhc_eds_init(struct admhcd *ahcd)
{
	struct ed *ed;

	ed = ed_create(ahcd, PIPE_INTERRUPT, ED_DUMMY_INFO);
	if (!ed)
		goto err;

	ahcd->ed_tails[PIPE_INTERRUPT] = ed;

	ed = ed_create(ahcd, PIPE_ISOCHRONOUS, ED_DUMMY_INFO);
	if (!ed)
		goto err;

	ahcd->ed_tails[PIPE_ISOCHRONOUS] = ed;
	ed->ed_prev = ahcd->ed_tails[PIPE_INTERRUPT];
	ahcd->ed_tails[PIPE_INTERRUPT]->ed_next = ed;
	ahcd->ed_tails[PIPE_INTERRUPT]->hwNextED = cpu_to_hc32(ahcd, ed->dma);

	ed = ed_create(ahcd, PIPE_CONTROL, ED_DUMMY_INFO);
	if (!ed)
		goto err;

	ahcd->ed_tails[PIPE_CONTROL] = ed;
	ed->ed_prev = ahcd->ed_tails[PIPE_ISOCHRONOUS];
	ahcd->ed_tails[PIPE_ISOCHRONOUS]->ed_next = ed;
	ahcd->ed_tails[PIPE_ISOCHRONOUS]->hwNextED = cpu_to_hc32(ahcd, ed->dma);

	ed = ed_create(ahcd, PIPE_BULK, ED_DUMMY_INFO);
	if (!ed)
		goto err;

	ahcd->ed_tails[PIPE_BULK] = ed;
	ed->ed_prev = ahcd->ed_tails[PIPE_CONTROL];
	ahcd->ed_tails[PIPE_CONTROL]->ed_next = ed;
	ahcd->ed_tails[PIPE_CONTROL]->hwNextED = cpu_to_hc32(ahcd, ed->dma);

	ahcd->ed_head = ahcd->ed_tails[PIPE_INTERRUPT];

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "ed intr", ahcd->ed_tails[PIPE_INTERRUPT], 1);
	admhc_dump_ed(ahcd, "ed isoc", ahcd->ed_tails[PIPE_ISOCHRONOUS], 1);
	admhc_dump_ed(ahcd, "ed ctrl", ahcd->ed_tails[PIPE_CONTROL], 1);
	admhc_dump_ed(ahcd, "ed bulk", ahcd->ed_tails[PIPE_BULK], 1);
#endif

	return 0;

err:
	admhc_eds_cleanup(ahcd);
	return -ENOMEM;
}

/* init memory, and kick BIOS/SMM off */

static int admhc_init(struct admhcd *ahcd)
{
	struct usb_hcd *hcd = admhcd_to_hcd(ahcd);
	int ret;

	admhc_disable(ahcd);
	ahcd->regs = hcd->regs;

	/* Disable HC interrupts */
	admhc_intr_disable(ahcd, ADMHC_INTR_MIE);

	/* Read the number of ports unless overridden */
	if (ahcd->num_ports == 0)
		ahcd->num_ports = admhc_read_rhdesc(ahcd) & ADMHC_RH_NUMP;

	ret = admhc_mem_init(ahcd);
	if (ret)
		goto err;

	/* init dummy endpoints */
	ret = admhc_eds_init(ahcd);
	if (ret)
		goto err;

	create_debug_files(ahcd);

	return 0;

err:
	admhc_stop(hcd);
	return ret;
}

/*-------------------------------------------------------------------------*/

/* Start an OHCI controller, set the BUS operational
 * resets USB and controller
 * enable interrupts
 */
static int admhc_run(struct admhcd *ahcd)
{
	u32			val;
	int			first = ahcd->fminterval == 0;
	struct usb_hcd		*hcd = admhcd_to_hcd(ahcd);

	admhc_disable(ahcd);

	/* boot firmware should have set this up (5.1.1.3.1) */
	if (first) {
		val = admhc_readl(ahcd, &ahcd->regs->fminterval);
		ahcd->fminterval = val & ADMHC_SFI_FI_MASK;
		if (ahcd->fminterval != FI)
			admhc_dbg(ahcd, "fminterval delta %d\n",
				ahcd->fminterval - FI);
		ahcd->fminterval |=
			(FSLDP(ahcd->fminterval) << ADMHC_SFI_FSLDP_SHIFT);
		/* also: power/overcurrent flags in rhdesc */
	}

#if 0	/* TODO: not applicable */
	/* Reset USB nearly "by the book".  RemoteWakeupConnected has
	 * to be checked in case boot firmware (BIOS/SMM/...) has set up
	 * wakeup in a way the bus isn't aware of (e.g., legacy PCI PM).
	 * If the bus glue detected wakeup capability then it should
	 * already be enabled; if so we'll just enable it again.
	 */
	if ((ahcd->hc_control & OHCI_CTRL_RWC) != 0)
		device_set_wakeup_capable(hcd->self.controller, 1);
#endif

	switch (ahcd->host_control & ADMHC_HC_BUSS) {
	case ADMHC_BUSS_OPER:
		val = 0;
		break;
	case ADMHC_BUSS_SUSPEND:
		/* FALLTHROUGH ? */
	case ADMHC_BUSS_RESUME:
		ahcd->host_control = ADMHC_BUSS_RESUME;
		val = 10 /* msec wait */;
		break;
	/* case ADMHC_BUSS_RESET: */
	default:
		ahcd->host_control = ADMHC_BUSS_RESET;
		val = 50 /* msec wait */;
		break;
	}
	admhc_writel(ahcd, ahcd->host_control, &ahcd->regs->host_control);

	/* flush the writes */
	admhc_writel_flush(ahcd);

	msleep(val);
	val = admhc_read_rhdesc(ahcd);
	if (!(val & ADMHC_RH_NPS)) {
		/* power down each port */
		for (val = 0; val < ahcd->num_ports; val++)
			admhc_write_portstatus(ahcd, val, ADMHC_PS_CPP);
	}
	/* flush those writes */
	admhc_writel_flush(ahcd);

	/* 2msec timelimit here means no irqs/preempt */
	spin_lock_irq(&ahcd->lock);

	admhc_writel(ahcd, ADMHC_CTRL_SR,  &ahcd->regs->gencontrol);
	val = 30;	/* ... allow extra time */
	while ((admhc_readl(ahcd, &ahcd->regs->gencontrol) & ADMHC_CTRL_SR) != 0) {
		if (--val == 0) {
			spin_unlock_irq(&ahcd->lock);
			admhc_err(ahcd, "USB HC reset timed out!\n");
			return -1;
		}
		udelay(1);
	}

	/* enable HOST mode, before access any host specific register */
	admhc_writel(ahcd, ADMHC_CTRL_UHFE,  &ahcd->regs->gencontrol);

	/* Tell the controller where the descriptor list is */
	admhc_writel(ahcd, (u32)ahcd->ed_head->dma, &ahcd->regs->hosthead);

	periodic_reinit(ahcd);

	/* use rhsc irqs after khubd is fully initialized */
	set_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	hcd->uses_new_polling = 1;

#if 0
	/* wake on ConnectStatusChange, matching external hubs */
	admhc_writel(ahcd, RH_HS_DRWE, &ahcd->regs->roothub.status);
#else
	/* FIXME roothub_write_status (ahcd, ADMHC_RH_DRWE); */
#endif

	/* Choose the interrupts we care about now, others later on demand */
	admhc_intr_ack(ahcd, ~0);
	admhc_intr_enable(ahcd, ADMHC_INTR_INIT);

	admhc_writel(ahcd, ADMHC_RH_NPS | ADMHC_RH_LPSC, &ahcd->regs->rhdesc);

	/* flush those writes */
	admhc_writel_flush(ahcd);

	/* start controller operations */
	ahcd->host_control = ADMHC_BUSS_OPER;
	admhc_writel(ahcd, ahcd->host_control, &ahcd->regs->host_control);

	val = 20;
	while ((admhc_readl(ahcd, &ahcd->regs->host_control)
			& ADMHC_HC_BUSS) != ADMHC_BUSS_OPER) {
		if (--val == 0) {
			spin_unlock_irq(&ahcd->lock);
			admhc_err(ahcd, "unable to setup operational mode!\n");
			return -1;
		}
		mdelay(1);
	}

	hcd->state = HC_STATE_RUNNING;

	ahcd->next_statechange = jiffies + STATECHANGE_DELAY;

#if 0
	/* FIXME: enabling DMA is always failed here for an unknown reason */
	admhc_dma_enable(ahcd);

	val = 200;
	while ((admhc_readl(ahcd, &ahcd->regs->host_control)
			& ADMHC_HC_DMAE) != ADMHC_HC_DMAE) {
		if (--val == 0) {
			spin_unlock_irq(&ahcd->lock);
			admhc_err(ahcd, "unable to enable DMA!\n");
			admhc_dump(ahcd, 1);
			return -1;
		}
		mdelay(1);
	}

#endif

	spin_unlock_irq(&ahcd->lock);

	mdelay(ADMHC_POTPGT);

	return 0;
}

/*-------------------------------------------------------------------------*/

/* an interrupt happens */

static irqreturn_t admhc_irq(struct usb_hcd *hcd)
{
	struct admhcd *ahcd = hcd_to_admhcd(hcd);
	struct admhcd_regs __iomem *regs = ahcd->regs;
	u32 ints;

	ints = admhc_readl(ahcd, &regs->int_status);
	if ((ints & ADMHC_INTR_INTA) == 0) {
		/* no unmasked interrupt status is set */
		return IRQ_NONE;
	}

	ints &= admhc_readl(ahcd, &regs->int_enable);

	if (ints & ADMHC_INTR_FATI) {
		/* e.g. due to PCI Master/Target Abort */
		admhc_disable(ahcd);
		admhc_err(ahcd, "Fatal Error, controller disabled\n");
		admhc_dump(ahcd, 1);
		admhc_usb_reset(ahcd);
	}

	if (ints & ADMHC_INTR_BABI) {
		admhc_intr_disable(ahcd, ADMHC_INTR_BABI);
		admhc_intr_ack(ahcd, ADMHC_INTR_BABI);
		admhc_err(ahcd, "Babble Detected\n");
	}

	if (ints & ADMHC_INTR_INSM) {
		admhc_vdbg(ahcd, "Root Hub Status Change\n");
		ahcd->next_statechange = jiffies + STATECHANGE_DELAY;
		admhc_intr_ack(ahcd, ADMHC_INTR_RESI | ADMHC_INTR_INSM);

		/* NOTE: Vendors didn't always make the same implementation
		 * choices for RHSC.  Many followed the spec; RHSC triggers
		 * on an edge, like setting and maybe clearing a port status
		 * change bit.  With others it's level-triggered, active
		 * until khubd clears all the port status change bits.  We'll
		 * always disable it here and rely on polling until khubd
		 * re-enables it.
		 */
		admhc_intr_disable(ahcd, ADMHC_INTR_INSM);
		usb_hcd_poll_rh_status(hcd);
	} else if (ints & ADMHC_INTR_RESI) {
		/* For connect and disconnect events, we expect the controller
		 * to turn on RHSC along with RD.  But for remote wakeup events
		 * this might not happen.
		 */
		admhc_vdbg(ahcd, "Resume Detect\n");
		admhc_intr_ack(ahcd, ADMHC_INTR_RESI);
		set_bit(HCD_FLAG_POLL_RH, &hcd->flags);
		if (ahcd->autostop) {
			spin_lock(&ahcd->lock);
			admhc_rh_resume(ahcd);
			spin_unlock(&ahcd->lock);
		} else
			usb_hcd_resume_root_hub(hcd);
	}

	if (ints & ADMHC_INTR_TDC) {
		admhc_vdbg(ahcd, "Transfer Descriptor Complete\n");
		admhc_intr_ack(ahcd, ADMHC_INTR_TDC);
		if (HC_IS_RUNNING(hcd->state))
			admhc_intr_disable(ahcd, ADMHC_INTR_TDC);
		spin_lock(&ahcd->lock);
		admhc_td_complete(ahcd);
		spin_unlock(&ahcd->lock);
		if (HC_IS_RUNNING(hcd->state))
			admhc_intr_enable(ahcd, ADMHC_INTR_TDC);
	}

	if (ints & ADMHC_INTR_SO) {
		/* could track INTR_SO to reduce available PCI/... bandwidth */
		admhc_vdbg(ahcd, "Schedule Overrun\n");
	}

#if 1
	spin_lock(&ahcd->lock);
	if (ahcd->ed_rm_list)
		finish_unlinks(ahcd, admhc_frame_no(ahcd));

	if ((ints & ADMHC_INTR_SOFI) != 0 && !ahcd->ed_rm_list
			&& HC_IS_RUNNING(hcd->state))
		admhc_intr_disable(ahcd, ADMHC_INTR_SOFI);
	spin_unlock(&ahcd->lock);
#else
	if (ints & ADMHC_INTR_SOFI) {
		admhc_vdbg(ahcd, "Start Of Frame\n");
		spin_lock(&ahcd->lock);

		/* handle any pending ED removes */
		finish_unlinks(ahcd, admhc_frameno(ahcd));

		/* leaving INTR_SOFI enabled when there's still unlinking
		 * to be done in the (next frame).
		 */
		if ((ahcd->ed_rm_list == NULL) ||
			HC_IS_RUNNING(hcd->state) == 0)
			/*
			 * disable INTR_SOFI if there are no unlinking to be
			 * done (in the next frame)
			 */
			admhc_intr_disable(ahcd, ADMHC_INTR_SOFI);

		spin_unlock(&ahcd->lock);
	}
#endif

	if (HC_IS_RUNNING(hcd->state)) {
		admhc_intr_ack(ahcd, ints);
		admhc_intr_enable(ahcd, ADMHC_INTR_MIE);
		admhc_writel_flush(ahcd);
	}

	return IRQ_HANDLED;
}

/*-------------------------------------------------------------------------*/

static void admhc_stop(struct usb_hcd *hcd)
{
	struct admhcd *ahcd = hcd_to_admhcd(hcd);

	admhc_dump(ahcd, 1);

	flush_scheduled_work();

	admhc_usb_reset(ahcd);
	admhc_intr_disable(ahcd, ADMHC_INTR_MIE);

	free_irq(hcd->irq, hcd);
	hcd->irq = -1;

	remove_debug_files(ahcd);
	admhc_eds_cleanup(ahcd);
	admhc_mem_cleanup(ahcd);
}

/*-------------------------------------------------------------------------*/

#ifdef CONFIG_ADM5120
#include "adm5120-drv.c"
#define PLATFORM_DRIVER		usb_hcd_adm5120_driver
#endif

#if	!defined(PLATFORM_DRIVER)
#error "missing bus glue for admhc-hcd"
#endif

#define DRIVER_INFO DRIVER_DESC " version " DRIVER_VERSION

static int __init admhc_hcd_mod_init(void)
{
	int ret = 0;

	if (usb_disabled())
		return -ENODEV;

	pr_info("%s: " DRIVER_INFO "\n", hcd_name);
	pr_info("%s: block sizes: ed %Zd td %Zd\n", hcd_name,
		sizeof(struct ed), sizeof(struct td));
	set_bit(USB_OHCI_LOADED, &usb_hcds_loaded);

#ifdef DEBUG
	admhc_debug_root = debugfs_create_dir("admhc", usb_debug_root);
	if (!admhc_debug_root) {
		ret = -ENOENT;
		goto error_debug;
	}
#endif

#ifdef PLATFORM_DRIVER
	ret = platform_driver_register(&PLATFORM_DRIVER);
	if (ret < 0)
		goto error_platform;
#endif

	return ret;

#ifdef PLATFORM_DRIVER
	platform_driver_unregister(&PLATFORM_DRIVER);
error_platform:
#endif

#ifdef DEBUG
	debugfs_remove(admhc_debug_root);
	admhc_debug_root = NULL;
error_debug:
#endif
	clear_bit(USB_OHCI_LOADED, &usb_hcds_loaded);
	return ret;
}
module_init(admhc_hcd_mod_init);

static void __exit admhc_hcd_mod_exit(void)
{
	platform_driver_unregister(&PLATFORM_DRIVER);
#ifdef DEBUG
	debugfs_remove(admhc_debug_root);
#endif
	clear_bit(USB_OHCI_LOADED, &usb_hcds_loaded);
}
module_exit(admhc_hcd_mod_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_INFO);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
