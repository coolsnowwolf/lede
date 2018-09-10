/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci-q.c
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/irq.h>
#include <linux/slab.h>

/*-------------------------------------------------------------------------*/

/*
 * URB goes back to driver, and isn't reissued.
 * It's completely gone from HC data structures.
 * PRECONDITION:  ahcd lock held, irqs blocked.
 */
static void
finish_urb(struct admhcd *ahcd, struct urb *urb, int status)
__releases(ahcd->lock)
__acquires(ahcd->lock)
{
	urb_priv_free(ahcd, urb->hcpriv);

	if (likely(status == -EINPROGRESS))
		status = 0;

	switch (usb_pipetype(urb->pipe)) {
	case PIPE_ISOCHRONOUS:
		admhcd_to_hcd(ahcd)->self.bandwidth_isoc_reqs--;
		break;
	case PIPE_INTERRUPT:
		admhcd_to_hcd(ahcd)->self.bandwidth_int_reqs--;
		break;
	}

#ifdef ADMHC_VERBOSE_DEBUG
	urb_print(ahcd, urb, "RET", usb_pipeout(urb->pipe), status);
#endif

	/* urb->complete() can reenter this HCD */
	usb_hcd_unlink_urb_from_ep(admhcd_to_hcd(ahcd), urb);
	spin_unlock(&ahcd->lock);
	usb_hcd_giveback_urb(admhcd_to_hcd(ahcd), urb, status);
	spin_lock(&ahcd->lock);
}


/*-------------------------------------------------------------------------*
 * ED handling functions
 *-------------------------------------------------------------------------*/

#if 0	/* FIXME */
/* search for the right schedule branch to use for a periodic ed.
 * does some load balancing; returns the branch, or negative errno.
 */
static int balance(struct admhcd *ahcd, int interval, int load)
{
	int	i, branch = -ENOSPC;

	/* iso periods can be huge; iso tds specify frame numbers */
	if (interval > NUM_INTS)
		interval = NUM_INTS;

	/* search for the least loaded schedule branch of that period
	 * that has enough bandwidth left unreserved.
	 */
	for (i = 0; i < interval ; i++) {
		if (branch < 0 || ahcd->load[branch] > ahcd->load[i]) {
			int	j;

			/* usb 1.1 says 90% of one frame */
			for (j = i; j < NUM_INTS; j += interval) {
				if ((ahcd->load[j] + load) > 900)
					break;
			}
			if (j < NUM_INTS)
				continue;
			branch = i;
		}
	}
	return branch;
}
#endif

/*-------------------------------------------------------------------------*/

#if 0	/* FIXME */
/* both iso and interrupt requests have periods; this routine puts them
 * into the schedule tree in the apppropriate place.  most iso devices use
 * 1msec periods, but that's not required.
 */
static void periodic_link(struct admhcd *ahcd, struct ed *ed)
{
	unsigned	i;

	admhc_vdbg(ahcd, "link %sed %p branch %d [%dus.], interval %d\n",
		(ed->hwINFO & cpu_to_hc32(ahcd, ED_ISO)) ? "iso " : "",
		ed, ed->branch, ed->load, ed->interval);

	for (i = ed->branch; i < NUM_INTS; i += ed->interval) {
		struct ed	**prev = &ahcd->periodic[i];
		__hc32		*prev_p = &ahcd->hcca->int_table[i];
		struct ed	*here = *prev;

		/* sorting each branch by period (slow before fast)
		 * lets us share the faster parts of the tree.
		 * (plus maybe: put interrupt eds before iso)
		 */
		while (here && ed != here) {
			if (ed->interval > here->interval)
				break;
			prev = &here->ed_next;
			prev_p = &here->hwNextED;
			here = *prev;
		}
		if (ed != here) {
			ed->ed_next = here;
			if (here)
				ed->hwNextED = *prev_p;
			wmb();
			*prev = ed;
			*prev_p = cpu_to_hc32(ahcd, ed->dma);
			wmb();
		}
		ahcd->load[i] += ed->load;
	}
	admhcd_to_hcd(ahcd)->self.bandwidth_allocated += ed->load / ed->interval;
}
#endif

/* link an ed into the HC chain */

static int ed_schedule(struct admhcd *ahcd, struct ed *ed)
{
	struct ed *old_tail;

	if (admhcd_to_hcd(ahcd)->state == HC_STATE_QUIESCING)
		return -EAGAIN;

	ed->state = ED_OPER;

	old_tail = ahcd->ed_tails[ed->type];

	ed->ed_next = old_tail->ed_next;
	if (ed->ed_next) {
		ed->ed_next->ed_prev = ed;
		ed->hwNextED = cpu_to_hc32(ahcd, ed->ed_next->dma);
	}
	ed->ed_prev = old_tail;

	old_tail->ed_next = ed;
	old_tail->hwNextED = cpu_to_hc32(ahcd, ed->dma);

	ahcd->ed_tails[ed->type] = ed;

	admhc_dma_enable(ahcd);

	return 0;
}

/*-------------------------------------------------------------------------*/

#if 0	/* FIXME */
/* scan the periodic table to find and unlink this ED */
static void periodic_unlink(struct admhcd *ahcd, struct ed *ed)
{
	int	i;

	for (i = ed->branch; i < NUM_INTS; i += ed->interval) {
		struct ed	*temp;
		struct ed	**prev = &ahcd->periodic[i];
		__hc32		*prev_p = &ahcd->hcca->int_table[i];

		while (*prev && (temp = *prev) != ed) {
			prev_p = &temp->hwNextED;
			prev = &temp->ed_next;
		}
		if (*prev) {
			*prev_p = ed->hwNextED;
			*prev = ed->ed_next;
		}
		ahcd->load[i] -= ed->load;
	}

	admhcd_to_hcd(ahcd)->self.bandwidth_allocated -= ed->load / ed->interval;
	admhc_vdbg(ahcd, "unlink %sed %p branch %d [%dus.], interval %d\n",
		(ed->hwINFO & cpu_to_hc32(ahcd, ED_ISO)) ? "iso " : "",
		ed, ed->branch, ed->load, ed->interval);
}
#endif

/* unlink an ed from the HC chain.
 * just the link to the ed is unlinked.
 * the link from the ed still points to another operational ed or 0
 * so the HC can eventually finish the processing of the unlinked ed
 * (assuming it already started that, which needn't be true).
 *
 * ED_UNLINK is a transient state: the HC may still see this ED, but soon
 * it won't.  ED_SKIP means the HC will finish its current transaction,
 * but won't start anything new.  The TD queue may still grow; device
 * drivers don't know about this HCD-internal state.
 *
 * When the HC can't see the ED, something changes ED_UNLINK to one of:
 *
 *  - ED_OPER: when there's any request queued, the ED gets rescheduled
 *    immediately.  HC should be working on them.
 *
 *  - ED_IDLE:  when there's no TD queue. there's no reason for the HC
 *    to care about this ED; safe to disable the endpoint.
 *
 * When finish_unlinks() runs later, after SOF interrupt, it will often
 * complete one or more URB unlinks before making that state change.
 */
static void ed_deschedule(struct admhcd *ahcd, struct ed *ed)
{

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "ED-DESCHED", ed, 1);
#endif

	ed->hwINFO |= cpu_to_hc32(ahcd, ED_SKIP);
	wmb();
	ed->state = ED_UNLINK;

	/* remove this ED from the HC list */
	ed->ed_prev->hwNextED = ed->hwNextED;

	/* and remove it from our list also */
	ed->ed_prev->ed_next = ed->ed_next;

	if (ed->ed_next)
		ed->ed_next->ed_prev = ed->ed_prev;

	if (ahcd->ed_tails[ed->type] == ed)
		ahcd->ed_tails[ed->type] = ed->ed_prev;
}

/*-------------------------------------------------------------------------*/

static struct ed *ed_create(struct admhcd *ahcd, unsigned int type, u32 info)
{
	struct ed *ed;
	struct td *td;

	ed = ed_alloc(ahcd, GFP_ATOMIC);
	if (!ed)
		goto err;

	/* dummy td; end of td list for this ed */
	td = td_alloc(ahcd, GFP_ATOMIC);
	if (!td)
		goto err_free_ed;

	switch (type) {
	case PIPE_INTERRUPT:
		info |= ED_INT;
		break;
	case PIPE_ISOCHRONOUS:
		info |= ED_ISO;
		break;
	}

	ed->dummy = td;
	ed->state = ED_IDLE;
	ed->type = type;

	ed->hwINFO = cpu_to_hc32(ahcd, info);
	ed->hwTailP = cpu_to_hc32(ahcd, td->td_dma);
	ed->hwHeadP = ed->hwTailP;	/* ED_C, ED_H zeroed */

	return ed;

err_free_ed:
	ed_free(ahcd, ed);
err:
	return NULL;
}

/* get and maybe (re)init an endpoint. init _should_ be done only as part
 * of enumeration, usb_set_configuration() or usb_set_interface().
 */
static struct ed *ed_get(struct admhcd *ahcd,	struct usb_host_endpoint *ep,
	struct usb_device *udev, unsigned int pipe, int interval)
{
	struct ed		*ed;
	unsigned long		flags;

	spin_lock_irqsave(&ahcd->lock, flags);

	ed = ep->hcpriv;
	if (!ed) {
		u32		info;

		/* FIXME: usbcore changes dev->devnum before SET_ADDRESS
		 * succeeds ... otherwise we wouldn't need "pipe".
		 */
		info = usb_pipedevice(pipe);
		info |= (ep->desc.bEndpointAddress & ~USB_DIR_IN) << ED_EN_SHIFT;
		info |= le16_to_cpu(ep->desc.wMaxPacketSize) << ED_MPS_SHIFT;
		if (udev->speed == USB_SPEED_FULL)
			info |= ED_SPEED_FULL;

		ed = ed_create(ahcd, usb_pipetype(pipe), info);
		if (ed)
			ep->hcpriv = ed;
	}

	spin_unlock_irqrestore(&ahcd->lock, flags);

	return ed;
}

/*-------------------------------------------------------------------------*/

/* request unlinking of an endpoint from an operational HC.
 * put the ep on the rm_list
 * real work is done at the next start frame (SOFI) hardware interrupt
 * caller guarantees HCD is running, so hardware access is safe,
 * and that ed->state is ED_OPER
 */
static void start_ed_unlink(struct admhcd *ahcd, struct ed *ed)
{

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "ED-UNLINK", ed, 1);
#endif

	ed->hwINFO |= cpu_to_hc32(ahcd, ED_DEQUEUE);
	ed_deschedule(ahcd, ed);

	/* add this ED into the remove list */
	ed->ed_rm_next = ahcd->ed_rm_list;
	ahcd->ed_rm_list = ed;

	/* enable SOF interrupt */
	admhc_intr_ack(ahcd, ADMHC_INTR_SOFI);
	admhc_intr_enable(ahcd, ADMHC_INTR_SOFI);
	/* flush those writes */
	admhc_writel_flush(ahcd);

	/* SOF interrupt might get delayed; record the frame counter value that
	 * indicates when the HC isn't looking at it, so concurrent unlinks
	 * behave.  frame_no wraps every 2^16 msec, and changes right before
	 * SOF is triggered.
	 */
	ed->tick = admhc_frame_no(ahcd) + 1;
}

/*-------------------------------------------------------------------------*
 * TD handling functions
 *-------------------------------------------------------------------------*/

/* enqueue next TD for this URB (OHCI spec 5.2.8.2) */

static void
td_fill(struct admhcd *ahcd, u32 info, dma_addr_t data, int len,
	struct urb *urb, int index)
{
	struct td		*td, *td_pt;
	struct urb_priv		*urb_priv = urb->hcpriv;
	int			hash;
	u32			cbl = 0;

#if 1
	if (index == (urb_priv->td_cnt - 1) &&
			((urb->transfer_flags & URB_NO_INTERRUPT) == 0))
		cbl |= TD_IE;
#else
	if (index == (urb_priv->td_cnt - 1))
		cbl |= TD_IE;
#endif

	/* use this td as the next dummy */
	td_pt = urb_priv->td[index];

	/* fill the old dummy TD */
	td = urb_priv->td[index] = urb_priv->ed->dummy;
	urb_priv->ed->dummy = td_pt;

	td->ed = urb_priv->ed;
	td->next_dl_td = NULL;
	td->index = index;
	td->urb = urb;
	td->data_dma = data;
	if (!len)
		data = 0;

	if (data)
		cbl |= (len & TD_BL_MASK);

	info |= TD_OWN;

	/* setup hardware specific fields */
	td->hwINFO = cpu_to_hc32(ahcd, info);
	td->hwDBP = cpu_to_hc32(ahcd, data);
	td->hwCBL = cpu_to_hc32(ahcd, cbl);
	td->hwNextTD = cpu_to_hc32(ahcd, td_pt->td_dma);

	/* append to queue */
	list_add_tail(&td->td_list, &td->ed->td_list);

	/* hash it for later reverse mapping */
	hash = TD_HASH_FUNC(td->td_dma);
	td->td_hash = ahcd->td_hash[hash];
	ahcd->td_hash[hash] = td;

	/* HC might read the TD (or cachelines) right away ... */
	wmb();
	td->ed->hwTailP = td->hwNextTD;
}

/*-------------------------------------------------------------------------*/

/* Prepare all TDs of a transfer, and queue them onto the ED.
 * Caller guarantees HC is active.
 * Usually the ED is already on the schedule, so TDs might be
 * processed as soon as they're queued.
 */
static void td_submit_urb(struct admhcd *ahcd, struct urb *urb)
{
	struct urb_priv	*urb_priv = urb->hcpriv;
	dma_addr_t	data;
	int		data_len = urb->transfer_buffer_length;
	int		cnt = 0;
	u32		info = 0;
	int		is_out = usb_pipeout(urb->pipe);
	u32		toggle = 0;

	/* OHCI handles the bulk/interrupt data toggles itself.  We just
	 * use the device toggle bits for resetting, and rely on the fact
	 * that resetting toggle is meaningless if the endpoint is active.
	 */

	if (usb_gettoggle(urb->dev, usb_pipeendpoint(urb->pipe), is_out)) {
		toggle = TD_T_CARRY;
	} else {
		toggle = TD_T_DATA0;
		usb_settoggle(urb->dev, usb_pipeendpoint (urb->pipe),
			is_out, 1);
	}

	urb_priv->td_idx = 0;
	list_add(&urb_priv->pending, &ahcd->pending);

	if (data_len)
		data = urb->transfer_dma;
	else
		data = 0;

	/* NOTE:  TD_CC is set so we can tell which TDs the HC processed by
	 * using TD_CC_GET, as well as by seeing them on the done list.
	 * (CC = NotAccessed ... 0x0F, or 0x0E in PSWs for ISO.)
	 */
	switch (urb_priv->ed->type) {
	case PIPE_INTERRUPT:
		info = is_out
			? TD_T_CARRY | TD_SCC_NOTACCESSED | TD_DP_OUT
			: TD_T_CARRY | TD_SCC_NOTACCESSED | TD_DP_IN;

		/* setup service interval and starting frame number */
		info |= (urb->start_frame & TD_FN_MASK);
		info |= (urb->interval & TD_ISI_MASK) << TD_ISI_SHIFT;

		td_fill(ahcd, info, data, data_len, urb, cnt);
		cnt++;

		admhcd_to_hcd(ahcd)->self.bandwidth_int_reqs++;
		break;

	case PIPE_BULK:
		info = is_out
			? TD_SCC_NOTACCESSED | TD_DP_OUT
			: TD_SCC_NOTACCESSED | TD_DP_IN;

		/* TDs _could_ transfer up to 8K each */
		while (data_len > TD_DATALEN_MAX) {
			td_fill(ahcd, info | ((cnt) ? TD_T_CARRY : toggle),
				data, TD_DATALEN_MAX, urb, cnt);
			data += TD_DATALEN_MAX;
			data_len -= TD_DATALEN_MAX;
			cnt++;
		}

		td_fill(ahcd, info | ((cnt) ? TD_T_CARRY : toggle), data,
			data_len, urb, cnt);
		cnt++;

		if ((urb->transfer_flags & URB_ZERO_PACKET)
				&& (cnt < urb_priv->td_cnt)) {
			td_fill(ahcd, info | ((cnt) ? TD_T_CARRY : toggle),
				0, 0, urb, cnt);
			cnt++;
		}
		break;

	/* control manages DATA0/DATA1 toggle per-request; SETUP resets it,
	 * any DATA phase works normally, and the STATUS ack is special.
	 */
	case PIPE_CONTROL:
		/* fill a TD for the setup */
		info = TD_SCC_NOTACCESSED | TD_DP_SETUP | TD_T_DATA0;
		td_fill(ahcd, info, urb->setup_dma, 8, urb, cnt++);

		if (data_len > 0) {
			/* fill a TD for the data */
			info = TD_SCC_NOTACCESSED | TD_T_DATA1;
			info |= is_out ? TD_DP_OUT : TD_DP_IN;
			/* NOTE:  mishandles transfers >8K, some >4K */
			td_fill(ahcd, info, data, data_len, urb, cnt++);
		}

		/* fill a TD for the ACK */
		info = (is_out || data_len == 0)
			? TD_SCC_NOTACCESSED | TD_DP_IN | TD_T_DATA1
			: TD_SCC_NOTACCESSED | TD_DP_OUT | TD_T_DATA1;
		td_fill(ahcd, info, data, 0, urb, cnt++);

		break;

	/* ISO has no retransmit, so no toggle;
	 * Each TD could handle multiple consecutive frames (interval 1);
	 * we could often reduce the number of TDs here.
	 */
	case PIPE_ISOCHRONOUS:
		info = is_out
			? TD_T_CARRY | TD_SCC_NOTACCESSED | TD_DP_OUT
			: TD_T_CARRY | TD_SCC_NOTACCESSED | TD_DP_IN;

		for (cnt = 0; cnt < urb->number_of_packets; cnt++) {
			int frame = urb->start_frame;

			frame += cnt * urb->interval;
			frame &= TD_FN_MASK;
			td_fill(ahcd, info | frame,
				data + urb->iso_frame_desc[cnt].offset,
				urb->iso_frame_desc[cnt].length, urb, cnt);
		}
		admhcd_to_hcd(ahcd)->self.bandwidth_isoc_reqs++;
		break;
	}

	if (urb_priv->td_cnt != cnt)
		admhc_err(ahcd, "bad number of tds created for urb %p\n", urb);
}

/*-------------------------------------------------------------------------*
 * Done List handling functions
 *-------------------------------------------------------------------------*/

/* calculate transfer length/status and update the urb */
static int td_done(struct admhcd *ahcd, struct urb *urb, struct td *td)
{
	struct urb_priv *urb_priv = urb->hcpriv;
	u32	info;
	u32	bl;
	u32	tdDBP;
	int	type = usb_pipetype(urb->pipe);
	int	cc;
	int	status = -EINPROGRESS;

	info = hc32_to_cpup(ahcd, &td->hwINFO);
	tdDBP = hc32_to_cpup(ahcd, &td->hwDBP);
	bl = TD_BL_GET(hc32_to_cpup(ahcd, &td->hwCBL));
	cc = TD_CC_GET(info);

	/* ISO ... drivers see per-TD length/status */
	if (type == PIPE_ISOCHRONOUS) {
		/* TODO */
		int	dlen = 0;

		/* NOTE:  assumes FC in tdINFO == 0, and that
		 * only the first of 0..MAXPSW psws is used.
		 */
		if (info & TD_CC)	/* hc didn't touch? */
			return status;

		if (usb_pipeout(urb->pipe))
			dlen = urb->iso_frame_desc[td->index].length;
		else {
			/* short reads are always OK for ISO */
			if (cc == TD_CC_DATAUNDERRUN)
				cc = TD_CC_NOERROR;
			dlen = tdDBP - td->data_dma + bl;
		}

		urb->actual_length += dlen;
		urb->iso_frame_desc[td->index].actual_length = dlen;
		urb->iso_frame_desc[td->index].status = cc_to_error[cc];

		if (cc != TD_CC_NOERROR)
			admhc_vdbg(ahcd,
				"urb %p iso td %p (%d) len %d cc %d\n",
				urb, td, 1 + td->index, dlen, cc);

	/* BULK, INT, CONTROL ... drivers see aggregate length/status,
	 * except that "setup" bytes aren't counted and "short" transfers
	 * might not be reported as errors.
	 */
	} else {
		/* update packet status if needed (short is normally ok) */
		if (cc == TD_CC_DATAUNDERRUN
				&& !(urb->transfer_flags & URB_SHORT_NOT_OK))
			cc = TD_CC_NOERROR;

		if (cc != TD_CC_NOERROR && cc < TD_CC_HCD0)
			status = cc_to_error[cc];


		/* count all non-empty packets except control SETUP packet */
		if ((type != PIPE_CONTROL || td->index != 0) && tdDBP != 0)
			urb->actual_length += tdDBP - td->data_dma + bl;

		if (cc != TD_CC_NOERROR && cc < TD_CC_HCD0)
			admhc_vdbg(ahcd,
				"urb %p td %p (%d) cc %d, len=%d/%d\n",
				urb, td, td->index, cc,
				urb->actual_length,
				urb->transfer_buffer_length);
	}

	list_del(&td->td_list);
	urb_priv->td_idx++;

	return status;
}

/*-------------------------------------------------------------------------*/

static void ed_halted(struct admhcd *ahcd, struct td *td, int cc)
{
	struct urb		*urb = td->urb;
	struct urb_priv		*urb_priv = urb->hcpriv;
	struct ed		*ed = td->ed;
	struct list_head	*tmp = td->td_list.next;
	__hc32			toggle = ed->hwHeadP & cpu_to_hc32(ahcd, ED_C);

	admhc_dump_ed(ahcd, "ed halted", td->ed, 1);
	/* clear ed halt; this is the td that caused it, but keep it inactive
	 * until its urb->complete() has a chance to clean up.
	 */
	ed->hwINFO |= cpu_to_hc32(ahcd, ED_SKIP);
	wmb();
	ed->hwHeadP &= ~cpu_to_hc32(ahcd, ED_H);

	/* Get rid of all later tds from this urb. We don't have
	 * to be careful: no errors and nothing was transferred.
	 * Also patch the ed so it looks as if those tds completed normally.
	 */
	while (tmp != &ed->td_list) {
		struct td	*next;

		next = list_entry(tmp, struct td, td_list);
		tmp = next->td_list.next;

		if (next->urb != urb)
			break;

		/* NOTE: if multi-td control DATA segments get supported,
		 * this urb had one of them, this td wasn't the last td
		 * in that segment (TD_R clear), this ed halted because
		 * of a short read, _and_ URB_SHORT_NOT_OK is clear ...
		 * then we need to leave the control STATUS packet queued
		 * and clear ED_SKIP.
		 */
		list_del(&next->td_list);
		urb_priv->td_cnt++;
		ed->hwHeadP = next->hwNextTD | toggle;
	}

	/* help for troubleshooting:  report anything that
	 * looks odd ... that doesn't include protocol stalls
	 * (or maybe some other things)
	 */
	switch (cc) {
	case TD_CC_DATAUNDERRUN:
		if ((urb->transfer_flags & URB_SHORT_NOT_OK) == 0)
			break;
		/* fallthrough */
	case TD_CC_STALL:
		if (usb_pipecontrol(urb->pipe))
			break;
		/* fallthrough */
	default:
		admhc_dbg(ahcd,
			"urb %p path %s ep%d%s %08x cc %d --> status %d\n",
			urb, urb->dev->devpath,
			usb_pipeendpoint (urb->pipe),
			usb_pipein(urb->pipe) ? "in" : "out",
			hc32_to_cpu(ahcd, td->hwINFO),
			cc, cc_to_error[cc]);
	}
}

/*-------------------------------------------------------------------------*/

/* there are some urbs/eds to unlink; called in_irq(), with HCD locked */
static void
finish_unlinks(struct admhcd *ahcd, u16 tick)
{
	struct ed	*ed, **last;

rescan_all:
	for (last = &ahcd->ed_rm_list, ed = *last; ed != NULL; ed = *last) {
		struct list_head	*entry, *tmp;
		int			completed, modified;
		__hc32			*prev;

		/* only take off EDs that the HC isn't using, accounting for
		 * frame counter wraps and EDs with partially retired TDs
		 */
		if (likely(HC_IS_RUNNING(admhcd_to_hcd(ahcd)->state))) {
			if (tick_before(tick, ed->tick)) {
skip_ed:
				last = &ed->ed_rm_next;
				continue;
			}
#if 0
			if (!list_empty(&ed->td_list)) {
				struct td	*td;
				u32		head;

				td = list_entry(ed->td_list.next, struct td,
							td_list);
				head = hc32_to_cpu(ahcd, ed->hwHeadP) &
								TD_MASK;

				/* INTR_WDH may need to clean up first */
				if (td->td_dma != head)
					goto skip_ed;
			}
#endif
		}

		/* reentrancy:  if we drop the schedule lock, someone might
		 * have modified this list.  normally it's just prepending
		 * entries (which we'd ignore), but paranoia won't hurt.
		 */
		*last = ed->ed_rm_next;
		ed->ed_rm_next = NULL;
		modified = 0;

		/* unlink urbs as requested, but rescan the list after
		 * we call a completion since it might have unlinked
		 * another (earlier) urb
		 *
		 * When we get here, the HC doesn't see this ed.  But it
		 * must not be rescheduled until all completed URBs have
		 * been given back to the driver.
		 */
rescan_this:
		completed = 0;
		prev = &ed->hwHeadP;
		list_for_each_safe(entry, tmp, &ed->td_list) {
			struct td	*td;
			struct urb	*urb;
			struct urb_priv	*urb_priv;
			__hc32		savebits;
			u32		tdINFO;
			int		status;

			td = list_entry(entry, struct td, td_list);
			urb = td->urb;
			urb_priv = td->urb->hcpriv;

			if (!urb->unlinked) {
				prev = &td->hwNextTD;
				continue;
			}

			if ((urb_priv) == NULL)
				continue;

			/* patch pointer hc uses */
			savebits = *prev & ~cpu_to_hc32(ahcd, TD_MASK);
			*prev = td->hwNextTD | savebits;
			/* If this was unlinked, the TD may not have been
			 * retired ... so manually save dhe data toggle.
			 * The controller ignores the value we save for
			 * control and ISO endpoints.
			 */
			tdINFO = hc32_to_cpup(ahcd, &td->hwINFO);
			if ((tdINFO & TD_T) == TD_T_DATA0)
				ed->hwHeadP &= ~cpu_to_hc32(ahcd, ED_C);
			else if ((tdINFO & TD_T) == TD_T_DATA1)
				ed->hwHeadP |= cpu_to_hc32(ahcd, ED_C);

			/* HC may have partly processed this TD */
#ifdef ADMHC_VERBOSE_DEBUG
			urb_print(ahcd, urb, "PARTIAL", 0);
#endif
			status = td_done(ahcd, urb, td);

			/* if URB is done, clean up */
			if (urb_priv->td_idx == urb_priv->td_cnt) {
				modified = completed = 1;
				finish_urb(ahcd, urb, status);
			}
		}
		if (completed && !list_empty(&ed->td_list))
			goto rescan_this;

		/* ED's now officially unlinked, hc doesn't see */
		ed->state = ED_IDLE;
		ed->hwHeadP &= ~cpu_to_hc32(ahcd, ED_H);
		ed->hwNextED = 0;
		wmb();
		ed->hwINFO &= ~cpu_to_hc32(ahcd, ED_SKIP | ED_DEQUEUE);

		/* but if there's work queued, reschedule */
		if (!list_empty(&ed->td_list)) {
			if (HC_IS_RUNNING(admhcd_to_hcd(ahcd)->state))
				ed_schedule(ahcd, ed);
		}

		if (modified)
			goto rescan_all;
	}
}

/*-------------------------------------------------------------------------*/
/*
 * Process normal completions (error or success) and clean the schedules.
 *
 * This is the main path for handing urbs back to drivers.  The only other
 * normal path is finish_unlinks(), which unlinks URBs using ed_rm_list,
 * instead of scanning the (re-reversed) donelist as this does.
 */

static void ed_unhalt(struct admhcd *ahcd, struct ed *ed, struct urb *urb)
{
	struct list_head *entry, *tmp;
	__hc32 toggle = ed->hwHeadP & cpu_to_hc32(ahcd, ED_C);

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "UNHALT", ed, 0);
#endif
	/* clear ed halt; this is the td that caused it, but keep it inactive
	 * until its urb->complete() has a chance to clean up.
	 */
	ed->hwINFO |= cpu_to_hc32(ahcd, ED_SKIP);
	wmb();
	ed->hwHeadP &= ~cpu_to_hc32(ahcd, ED_H);

	list_for_each_safe(entry, tmp, &ed->td_list) {
		struct td *td = list_entry(entry, struct td, td_list);
		__hc32 info;

		if (td->urb != urb)
			break;

		info = td->hwINFO;
		info &= ~cpu_to_hc32(ahcd, TD_CC | TD_OWN);
		td->hwINFO = info;

		ed->hwHeadP = td->hwNextTD | toggle;
		wmb();
	}

}

static void ed_intr_refill(struct admhcd *ahcd, struct ed *ed)
{
	__hc32 toggle = ed->hwHeadP & cpu_to_hc32(ahcd, ED_C);

	ed->hwHeadP = ed->hwTailP | toggle;
}


static inline int is_ed_halted(struct admhcd *ahcd, struct ed *ed)
{
	return ((hc32_to_cpup(ahcd, &ed->hwHeadP) & ED_H) == ED_H);
}

static inline int is_td_halted(struct admhcd *ahcd, struct ed *ed,
		struct td *td)
{
	return ((hc32_to_cpup(ahcd, &ed->hwHeadP) & TD_MASK) ==
		(hc32_to_cpup(ahcd, &td->hwNextTD) & TD_MASK));
}

static void ed_update(struct admhcd *ahcd, struct ed *ed)
{
	struct list_head *entry, *tmp;

#ifdef ADMHC_VERBOSE_DEBUG
	admhc_dump_ed(ahcd, "UPDATE", ed, 1);
#endif

	list_for_each_safe(entry, tmp, &ed->td_list) {
		struct td *td = list_entry(entry, struct td, td_list);
		struct urb *urb = td->urb;
		struct urb_priv *urb_priv = urb->hcpriv;
		int status;

		if (hc32_to_cpup(ahcd, &td->hwINFO) & TD_OWN)
			break;

		/* update URB's length and status from TD */
		status = td_done(ahcd, urb, td);
		if (is_ed_halted(ahcd, ed) && is_td_halted(ahcd, ed, td))
			ed_unhalt(ahcd, ed, urb);

		if (ed->type == PIPE_INTERRUPT)
			ed_intr_refill(ahcd, ed);

		/* If all this urb's TDs are done, call complete() */
		if (urb_priv->td_idx == urb_priv->td_cnt)
			finish_urb(ahcd, urb, status);

		/* clean schedule:  unlink EDs that are no longer busy */
		if (list_empty(&ed->td_list)) {
			if (ed->state == ED_OPER)
				start_ed_unlink(ahcd, ed);

		/* ... reenabling halted EDs only after fault cleanup */
		} else if ((ed->hwINFO & cpu_to_hc32(ahcd,
						ED_SKIP | ED_DEQUEUE))
					== cpu_to_hc32(ahcd, ED_SKIP)) {
			td = list_entry(ed->td_list.next, struct td, td_list);
#if 0
			if (!(td->hwINFO & cpu_to_hc32(ahcd, TD_DONE))) {
				ed->hwINFO &= ~cpu_to_hc32(ahcd, ED_SKIP);
				/* ... hc may need waking-up */
				switch (ed->type) {
				case PIPE_CONTROL:
					admhc_writel(ahcd, OHCI_CLF,
						&ahcd->regs->cmdstatus);
					break;
				case PIPE_BULK:
					admhc_writel(ahcd, OHCI_BLF,
						&ahcd->regs->cmdstatus);
					break;
				}
			}
#else
			if ((td->hwINFO & cpu_to_hc32(ahcd, TD_OWN)))
				ed->hwINFO &= ~cpu_to_hc32(ahcd, ED_SKIP);
#endif
		}

	}
}

/* there are some tds completed; called in_irq(), with HCD locked */
static void admhc_td_complete(struct admhcd *ahcd)
{
	struct ed	*ed;

	for (ed = ahcd->ed_head; ed; ed = ed->ed_next) {
		if (ed->state != ED_OPER)
			continue;

		ed_update(ahcd, ed);
	}
}
