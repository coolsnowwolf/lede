/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from fragments of the OHCI driver.
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2004 David Brownell <dbrownell@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#define OHCI_SCHED_ENABLES \
	(OHCI_CTRL_CLE|OHCI_CTRL_BLE|OHCI_CTRL_PLE|OHCI_CTRL_IE)

#ifdef	CONFIG_PM
static int admhc_restart(struct admhcd *ahcd);

static int admhc_rh_suspend(struct admhcd *ahcd, int autostop)
__releases(ahcd->lock)
__acquires(ahcd->lock)
{
	int			status = 0;

	ahcd->hc_control = admhc_readl(ahcd, &ahcd->regs->control);
	switch (ahcd->hc_control & OHCI_CTRL_HCFS) {
	case OHCI_USB_RESUME:
		admhc_dbg(ahcd, "resume/suspend?\n");
		ahcd->hc_control &= ~OHCI_CTRL_HCFS;
		ahcd->hc_control |= OHCI_USB_RESET;
		admhc_writel(ahcd, ahcd->hc_control, &ahcd->ahcd->regs->control);
		(void) admhc_readl(ahcd, &ahcd->regs->control);
		/* FALL THROUGH */
	case OHCI_USB_RESET:
		status = -EBUSY;
		admhc_dbg(ahcd, "needs reinit!\n");
		goto done;
	case OHCI_USB_SUSPEND:
		if (!ahcd->autostop) {
			admhc_dbg(ahcd, "already suspended\n");
			goto done;
		}
	}
	admhc_dbg(ahcd, "%s root hub\n",
			autostop ? "auto-stop" : "suspend");

	/* First stop any processing */
	if (!autostop && (ahcd->hc_control & OHCI_SCHED_ENABLES)) {
		ahcd->hc_control &= ~OHCI_SCHED_ENABLES;
		admhc_writel(ahcd, ahcd->hc_control, &ahcd->ahcd->regs->control);
		ahcd->hc_control = admhc_readl(ahcd, &ahcd->regs->control);
		admhc_writel(ahcd, OHCI_INTR_SF, &ahcd->regs->intrstatus);

		/* sched disables take effect on the next frame,
		 * then the last WDH could take 6+ msec
		 */
		admhc_dbg(ahcd, "stopping schedules ...\n");
		ahcd->autostop = 0;
		spin_unlock_irq (&ahcd->lock);
		msleep (8);
		spin_lock_irq(&ahcd->lock);
	}
	dl_done_list (ahcd);
	finish_unlinks (ahcd, admhc_frame_no(ahcd));

	/* maybe resume can wake root hub */
	if (device_may_wakeup(&admhcd_to_hcd(ahcd)->self.root_hub->dev) ||
			autostop)
		ahcd->hc_control |= OHCI_CTRL_RWE;
	else {
		admhc_writel(ahcd, OHCI_INTR_RHSC, &ahcd->regs->intrdisable);
		ahcd->hc_control &= ~OHCI_CTRL_RWE;
	}

	/* Suspend hub ... this is the "global (to this bus) suspend" mode,
	 * which doesn't imply ports will first be individually suspended.
	 */
	ahcd->hc_control &= ~OHCI_CTRL_HCFS;
	ahcd->hc_control |= OHCI_USB_SUSPEND;
	admhc_writel(ahcd, ahcd->hc_control, &ahcd->ahcd->regs->control);
	(void) admhc_readl(ahcd, &ahcd->regs->control);

	/* no resumes until devices finish suspending */
	if (!autostop) {
		ahcd->next_statechange = jiffies + msecs_to_jiffies (5);
		ahcd->autostop = 0;
	}

done:
	return status;
}

static inline struct ed *find_head(struct ed *ed)
{
	/* for bulk and control lists */
	while (ed->ed_prev)
		ed = ed->ed_prev;
	return ed;
}

/* caller has locked the root hub */
static int admhc_rh_resume(struct admhcd *ahcd)
__releases(ahcd->lock)
__acquires(ahcd->lock)
{
	struct usb_hcd		*hcd = admhcd_to_hcd (ahcd);
	u32			temp, enables;
	int			status = -EINPROGRESS;
	int			autostopped = ahcd->autostop;

	ahcd->autostop = 0;
	ahcd->hc_control = admhc_readl(ahcd, &ahcd->regs->control);

	if (ahcd->hc_control & (OHCI_CTRL_IR | OHCI_SCHED_ENABLES)) {
		/* this can happen after resuming a swsusp snapshot */
		if (hcd->state == HC_STATE_RESUMING) {
			admhc_dbg(ahcd, "BIOS/SMM active, control %03x\n",
					ahcd->hc_control);
			status = -EBUSY;
		/* this happens when pmcore resumes HC then root */
		} else {
			admhc_dbg(ahcd, "duplicate resume\n");
			status = 0;
		}
	} else switch (ahcd->hc_control & OHCI_CTRL_HCFS) {
	case OHCI_USB_SUSPEND:
		ahcd->hc_control &= ~(OHCI_CTRL_HCFS|OHCI_SCHED_ENABLES);
		ahcd->hc_control |= OHCI_USB_RESUME;
		admhc_writel(ahcd, ahcd->hc_control, &ahcd->ahcd->regs->control);
		(void) admhc_readl(ahcd, &ahcd->regs->control);
		admhc_dbg(ahcd, "%s root hub\n",
				autostopped ? "auto-start" : "resume");
		break;
	case OHCI_USB_RESUME:
		/* HCFS changes sometime after INTR_RD */
		admhc_dbg(ahcd, "%swakeup root hub\n",
				autostopped ? "auto-" : "");
		break;
	case OHCI_USB_OPER:
		/* this can happen after resuming a swsusp snapshot */
		admhc_dbg(ahcd, "snapshot resume? reinit\n");
		status = -EBUSY;
		break;
	default:		/* RESET, we lost power */
		admhc_dbg(ahcd, "lost power\n");
		status = -EBUSY;
	}
	if (status == -EBUSY) {
		if (!autostopped) {
			spin_unlock_irq (&ahcd->lock);
			(void) ahcd_init (ahcd);
			status = admhc_restart (ahcd);
			spin_lock_irq(&ahcd->lock);
		}
		return status;
	}
	if (status != -EINPROGRESS)
		return status;
	if (autostopped)
		goto skip_resume;
	spin_unlock_irq (&ahcd->lock);

	/* Some controllers (lucent erratum) need extra-long delays */
	msleep (20 /* usb 11.5.1.10 */ + 12 /* 32 msec counter */ + 1);

	temp = admhc_readl(ahcd, &ahcd->regs->control);
	temp &= OHCI_CTRL_HCFS;
	if (temp != OHCI_USB_RESUME) {
		admhc_err (ahcd, "controller won't resume\n");
		spin_lock_irq(&ahcd->lock);
		return -EBUSY;
	}

	/* disable old schedule state, reinit from scratch */
	admhc_writel(ahcd, 0, &ahcd->regs->ed_controlhead);
	admhc_writel(ahcd, 0, &ahcd->regs->ed_controlcurrent);
	admhc_writel(ahcd, 0, &ahcd->regs->ed_bulkhead);
	admhc_writel(ahcd, 0, &ahcd->regs->ed_bulkcurrent);
	admhc_writel(ahcd, 0, &ahcd->regs->ed_periodcurrent);
	admhc_writel(ahcd, (u32) ahcd->hcca_dma, &ahcd->ahcd->regs->hcca);

	/* Sometimes PCI D3 suspend trashes frame timings ... */
	periodic_reinit(ahcd);

	/* the following code is executed with ahcd->lock held and
	 * irqs disabled if and only if autostopped is true
	 */

skip_resume:
	/* interrupts might have been disabled */
	admhc_writel(ahcd, OHCI_INTR_INIT, &ahcd->regs->int_enable);
	if (ahcd->ed_rm_list)
		admhc_writel(ahcd, OHCI_INTR_SF, &ahcd->regs->int_enable);

	/* Then re-enable operations */
	admhc_writel(ahcd, OHCI_USB_OPER, &ahcd->regs->control);
	(void) admhc_readl(ahcd, &ahcd->regs->control);
	if (!autostopped)
		msleep (3);

	temp = ahcd->hc_control;
	temp &= OHCI_CTRL_RWC;
	temp |= OHCI_CONTROL_INIT | OHCI_USB_OPER;
	ahcd->hc_control = temp;
	admhc_writel(ahcd, temp, &ahcd->regs->control);
	(void) admhc_readl(ahcd, &ahcd->regs->control);

	/* TRSMRCY */
	if (!autostopped) {
		msleep (10);
		spin_lock_irq(&ahcd->lock);
	}
	/* now ahcd->lock is always held and irqs are always disabled */

	/* keep it alive for more than ~5x suspend + resume costs */
	ahcd->next_statechange = jiffies + STATECHANGE_DELAY;

	/* maybe turn schedules back on */
	enables = 0;
	temp = 0;
	if (!ahcd->ed_rm_list) {
		if (ahcd->ed_controltail) {
			admhc_writel(ahcd,
					find_head (ahcd->ed_controltail)->dma,
					&ahcd->regs->ed_controlhead);
			enables |= OHCI_CTRL_CLE;
			temp |= OHCI_CLF;
		}
		if (ahcd->ed_bulktail) {
			admhc_writel(ahcd, find_head (ahcd->ed_bulktail)->dma,
				&ahcd->regs->ed_bulkhead);
			enables |= OHCI_CTRL_BLE;
			temp |= OHCI_BLF;
		}
	}
	if (hcd->self.bandwidth_isoc_reqs || hcd->self.bandwidth_int_reqs)
		enables |= OHCI_CTRL_PLE|OHCI_CTRL_IE;
	if (enables) {
		admhc_dbg(ahcd, "restarting schedules ... %08x\n", enables);
		ahcd->hc_control |= enables;
		admhc_writel(ahcd, ahcd->hc_control, &ahcd->ahcd->regs->control);
		if (temp)
			admhc_writel(ahcd, temp, &ahcd->regs->cmdstatus);
		(void) admhc_readl(ahcd, &ahcd->regs->control);
	}

	return 0;
}

static int admhc_bus_suspend(struct usb_hcd *hcd)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	int		rc;

	spin_lock_irq(&ahcd->lock);

	if (unlikely(!HCD_HW_ACCESSIBLE(hcd)))
		rc = -ESHUTDOWN;
	else
		rc = admhc_rh_suspend(ahcd, 0);
	spin_unlock_irq(&ahcd->lock);
	return rc;
}

static int admhc_bus_resume(struct usb_hcd *hcd)
{
	struct admhcd		*ahcd = hcd_to_admhcd(hcd);
	int			rc;

	if (time_before(jiffies, ahcd->next_statechange))
		msleep(5);

	spin_lock_irq(&ahcd->lock);

	if (unlikely(!HCD_HW_ACCESSIBLE(hcd)))
		rc = -ESHUTDOWN;
	else
		rc = admhc_rh_resume(ahcd);
	spin_unlock_irq(&ahcd->lock);

	/* poll until we know a device is connected or we autostop */
	if (rc == 0)
		usb_hcd_poll_rh_status(hcd);
	return rc;
}

/* Carry out polling-, autostop-, and autoresume-related state changes */
static int admhc_root_hub_state_changes(struct admhcd *ahcd, int changed,
		int any_connected)
{
	int	poll_rh = 1;

	switch (ahcd->hc_control & OHCI_CTRL_HCFS) {

	case OHCI_USB_OPER:
		/* keep on polling until we know a device is connected
		 * and RHSC is enabled */
		if (!ahcd->autostop) {
			if (any_connected ||
					!device_may_wakeup(&admhcd_to_hcd(ahcd)
						->self.root_hub->dev)) {
				if (admhc_readl(ahcd, &ahcd->regs->int_enable) &
						OHCI_INTR_RHSC)
					poll_rh = 0;
			} else {
				ahcd->autostop = 1;
				ahcd->next_statechange = jiffies + HZ;
			}

		/* if no devices have been attached for one second, autostop */
		} else {
			if (changed || any_connected) {
				ahcd->autostop = 0;
				ahcd->next_statechange = jiffies +
						STATECHANGE_DELAY;
			} else if (time_after_eq(jiffies,
						ahcd->next_statechange)
					&& !ahcd->ed_rm_list
					&& !(ahcd->hc_control &
						OHCI_SCHED_ENABLES)) {
				ahcd_rh_suspend(ahcd, 1);
			}
		}
		break;

	/* if there is a port change, autostart or ask to be resumed */
	case OHCI_USB_SUSPEND:
	case OHCI_USB_RESUME:
		if (changed) {
			if (ahcd->autostop)
				admhc_rh_resume(ahcd);
			else
				usb_hcd_resume_root_hub(admhcd_to_hcd(ahcd));
		} else {
			/* everything is idle, no need for polling */
			poll_rh = 0;
		}
		break;
	}
	return poll_rh;
}

/*-------------------------------------------------------------------------*/

/* must not be called from interrupt context */
static int admhc_restart(struct admhcd *ahcd)
{
	int temp;
	int i;
	struct urb_priv *priv;

	/* mark any devices gone, so they do nothing till khubd disconnects.
	 * recycle any "live" eds/tds (and urbs) right away.
	 * later, khubd disconnect processing will recycle the other state,
	 * (either as disconnect/reconnect, or maybe someday as a reset).
	 */
	spin_lock_irq(&ahcd->lock);
	admhc_disable(ahcd);
	usb_root_hub_lost_power(admhcd_to_hcd(ahcd)->self.root_hub);
	if (!list_empty(&ahcd->pending))
		admhc_dbg(ahcd, "abort schedule...\n");
		list_for_each_entry(priv, &ahcd->pending, pending) {
		struct urb	*urb = priv->td[0]->urb;
		struct ed	*ed = priv->ed;

		switch (ed->state) {
		case ED_OPER:
			ed->state = ED_UNLINK;
			ed->hwINFO |= cpu_to_hc32(ahcd, ED_DEQUEUE);
			ed_deschedule (ahcd, ed);

			ed->ed_next = ahcd->ed_rm_list;
			ed->ed_prev = NULL;
			ahcd->ed_rm_list = ed;
			/* FALLTHROUGH */
		case ED_UNLINK:
			break;
		default:
			admhc_dbg(ahcd, "bogus ed %p state %d\n",
					ed, ed->state);
		}

		if (!urb->unlinked)
			urb->unlinked = -ESHUTDOWN;
	}
	finish_unlinks(ahcd, 0);
	spin_unlock_irq(&ahcd->lock);

	/* paranoia, in case that didn't work: */

	/* empty the interrupt branches */
	for (i = 0; i < NUM_INTS; i++) ahcd->load[i] = 0;
	for (i = 0; i < NUM_INTS; i++) ahcd->hcca->int_table[i] = 0;

	/* no EDs to remove */
	ahcd->ed_rm_list = NULL;

	/* empty control and bulk lists */
	ahcd->ed_controltail = NULL;
	ahcd->ed_bulktail    = NULL;

	if ((temp = admhc_run(ahcd)) < 0) {
		admhc_err(ahcd, "can't restart, %d\n", temp);
		return temp;
	} else {
		/* here we "know" root ports should always stay powered,
		 * and that if we try to turn them back on the root hub
		 * will respond to CSC processing.
		 */
		i = ahcd->num_ports;
		while (i--)
			admhc_writel(ahcd, RH_PS_PSS,
				&ahcd->regs->portstatus[i]);
		admhc_dbg(ahcd, "restart complete\n");
	}
	return 0;
}

#else	/* CONFIG_PM */

static inline int admhc_rh_resume(struct admhcd *ahcd)
{
	return 0;
}

/* Carry out polling-related state changes.
 * autostop isn't used when CONFIG_PM is turned off.
 */
static int admhc_root_hub_state_changes(struct admhcd *ahcd, int changed,
		int any_connected)
{
	/* If INSM is enabled, don't poll */
	if (admhc_readl(ahcd, &ahcd->regs->int_enable) & ADMHC_INTR_INSM)
		return 0;

	/* If no status changes are pending, enable status-change interrupts */
	if (!changed) {
		admhc_intr_enable(ahcd, ADMHC_INTR_INSM);
		return 0;
	}

	return 1;
}

#endif	/* CONFIG_PM */

