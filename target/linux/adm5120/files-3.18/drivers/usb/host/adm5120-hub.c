/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci-hub.c
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2004 David Brownell <dbrownell@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

/*-------------------------------------------------------------------------*/

/*
 * ADM5120 Root Hub ... the nonsharable stuff
 */

#define dbg_port(hc, label, num, value) \
	admhc_dbg(hc, \
		"%s port%d " \
		"= 0x%08x%s%s%s%s%s%s%s%s%s%s%s%s\n", \
		label, num, value, \
		(value & ADMHC_PS_PRSC) ? " PRSC" : "", \
		(value & ADMHC_PS_OCIC) ? " OCIC" : "", \
		(value & ADMHC_PS_PSSC) ? " PSSC" : "", \
		(value & ADMHC_PS_PESC) ? " PESC" : "", \
		(value & ADMHC_PS_CSC) ? " CSC" : "", \
		\
		(value & ADMHC_PS_LSDA) ? " LSDA" : "", \
		(value & ADMHC_PS_PPS) ? " PPS" : "", \
		(value & ADMHC_PS_PRS) ? " PRS" : "", \
		(value & ADMHC_PS_POCI) ? " POCI" : "", \
		(value & ADMHC_PS_PSS) ? " PSS" : "", \
		\
		(value & ADMHC_PS_PES) ? " PES" : "", \
		(value & ADMHC_PS_CCS) ? " CCS" : "" \
		);

#define dbg_port_write(hc, label, num, value) \
	admhc_dbg(hc, \
		"%s port%d " \
		"= 0x%08x%s%s%s%s%s%s%s%s%s%s%s%s\n", \
		label, num, value, \
		(value & ADMHC_PS_PRSC) ? " PRSC" : "", \
		(value & ADMHC_PS_OCIC) ? " OCIC" : "", \
		(value & ADMHC_PS_PSSC) ? " PSSC" : "", \
		(value & ADMHC_PS_PESC) ? " PESC" : "", \
		(value & ADMHC_PS_CSC) ? " CSC" : "", \
		\
		(value & ADMHC_PS_CPP) ? " CPP" : "", \
		(value & ADMHC_PS_SPP) ? " SPP" : "", \
		(value & ADMHC_PS_SPR) ? " SPR" : "", \
		(value & ADMHC_PS_CPS) ? " CPS" : "", \
		(value & ADMHC_PS_SPS) ? " SPS" : "", \
		\
		(value & ADMHC_PS_SPE) ? " SPE" : "", \
		(value & ADMHC_PS_CPE) ? " CPE" : "" \
		);

/*-------------------------------------------------------------------------*/

/* build "status change" packet (one or two bytes) from HC registers */

static int
admhc_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	int		i, changed = 0, length = 1;
	int		any_connected = 0;
	unsigned long	flags;
	u32		status;

	spin_lock_irqsave(&ahcd->lock, flags);
	if (!HCD_HW_ACCESSIBLE(hcd))
		goto done;

	/* init status */
	status = admhc_read_rhdesc(ahcd);
	if (status & (ADMHC_RH_LPSC | ADMHC_RH_OCIC))
		buf[0] = changed = 1;
	else
		buf[0] = 0;
	if (ahcd->num_ports > 7) {
		buf[1] = 0;
		length++;
	}

	/* look at each port */
	for (i = 0; i < ahcd->num_ports; i++) {
		status = admhc_read_portstatus(ahcd, i);

		/* can't autostop if ports are connected */
		any_connected |= (status & ADMHC_PS_CCS);

		if (status & (ADMHC_PS_CSC | ADMHC_PS_PESC | ADMHC_PS_PSSC
				| ADMHC_PS_OCIC | ADMHC_PS_PRSC)) {
			changed = 1;
			if (i < 7)
				buf[0] |= 1 << (i + 1);
			else
				buf[1] |= 1 << (i - 7);
		}
	}

	if (admhc_root_hub_state_changes(ahcd, changed,
			any_connected))
		set_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	else
		clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);

done:
	spin_unlock_irqrestore(&ahcd->lock, flags);

	return changed ? length : 0;
}

/*-------------------------------------------------------------------------*/

static int admhc_get_hub_descriptor(struct admhcd *ahcd, char *buf)
{
	struct usb_hub_descriptor *desc = (struct usb_hub_descriptor *)buf;
	u32 rh = admhc_read_rhdesc(ahcd);
	u16 temp;

	desc->bDescriptorType = USB_DT_HUB;	/* Hub-descriptor */
	desc->bPwrOn2PwrGood = ADMHC_POTPGT/2;	/* use default value */
	desc->bHubContrCurrent = 0x00;		/* 0mA */

	desc->bNbrPorts = ahcd->num_ports;
	temp = 1 + (ahcd->num_ports / 8);
	desc->bDescLength = USB_DT_HUB_NONVAR_SIZE + 2 * temp;

	/* FIXME */
	temp = 0;
	if (rh & ADMHC_RH_NPS)		/* no power switching? */
	    temp |= 0x0002;
	if (rh & ADMHC_RH_PSM)		/* per-port power switching? */
	    temp |= 0x0001;
	if (rh & ADMHC_RH_NOCP)		/* no overcurrent reporting? */
	    temp |= 0x0010;
	else if (rh & ADMHC_RH_OCPM)	/* per-port overcurrent reporting? */
	    temp |= 0x0008;
	desc->wHubCharacteristics = (__force __u16)cpu_to_hc16(ahcd, temp);

	/* ports removable, and usb 1.0 legacy PortPwrCtrlMask */
	desc->u.hs.DeviceRemovable[0] = 0;
	desc->u.hs.DeviceRemovable[0] = ~0;

	return 0;
}

static int admhc_get_hub_status(struct admhcd *ahcd, char *buf)
{
	struct usb_hub_status *hs = (struct usb_hub_status *)buf;
	u32 t = admhc_read_rhdesc(ahcd);
	u16 status, change;

	status = 0;
	status |= (t & ADMHC_RH_LPS) ? HUB_STATUS_LOCAL_POWER : 0;
	status |= (t & ADMHC_RH_OCI) ? HUB_STATUS_OVERCURRENT : 0;

	change = 0;
	change |= (t & ADMHC_RH_LPSC) ? HUB_CHANGE_LOCAL_POWER : 0;
	change |= (t & ADMHC_RH_OCIC) ? HUB_CHANGE_OVERCURRENT : 0;

	hs->wHubStatus = (__force __u16)cpu_to_hc16(ahcd, status);
	hs->wHubChange = (__force __u16)cpu_to_hc16(ahcd, change);

	return 0;
}

static int admhc_get_port_status(struct admhcd *ahcd, unsigned port, char *buf)
{
	struct usb_port_status *ps = (struct usb_port_status *)buf;
	u32 t = admhc_read_portstatus(ahcd, port);
	u16 status, change;

	status = 0;
	status |= (t & ADMHC_PS_CCS) ? USB_PORT_STAT_CONNECTION : 0;
	status |= (t & ADMHC_PS_PES) ? USB_PORT_STAT_ENABLE : 0;
	status |= (t & ADMHC_PS_PSS) ? USB_PORT_STAT_SUSPEND : 0;
	status |= (t & ADMHC_PS_POCI) ? USB_PORT_STAT_OVERCURRENT : 0;
	status |= (t & ADMHC_PS_PRS) ? USB_PORT_STAT_RESET : 0;
	status |= (t & ADMHC_PS_PPS) ? USB_PORT_STAT_POWER : 0;
	status |= (t & ADMHC_PS_LSDA) ? USB_PORT_STAT_LOW_SPEED : 0;

	change = 0;
	change |= (t & ADMHC_PS_CSC) ? USB_PORT_STAT_C_CONNECTION : 0;
	change |= (t & ADMHC_PS_PESC) ? USB_PORT_STAT_C_ENABLE : 0;
	change |= (t & ADMHC_PS_PSSC) ? USB_PORT_STAT_C_SUSPEND : 0;
	change |= (t & ADMHC_PS_OCIC) ? USB_PORT_STAT_C_OVERCURRENT : 0;
	change |= (t & ADMHC_PS_PRSC) ? USB_PORT_STAT_C_RESET : 0;

	ps->wPortStatus = (__force __u16)cpu_to_hc16(ahcd, status);
	ps->wPortChange = (__force __u16)cpu_to_hc16(ahcd, change);

	return 0;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_USB_OTG

static int admhc_start_port_reset(struct usb_hcd *hcd, unsigned port)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	u32			status;

	if (!port)
		return -EINVAL;
	port--;

	/* start port reset before HNP protocol times out */
	status = admhc_read_portstatus(ahcd, port);
	if (!(status & ADMHC_PS_CCS))
		return -ENODEV;

	/* khubd will finish the reset later */
	admhc_write_portstatus(ahcd, port, ADMHC_PS_PRS);
	return 0;
}

static void start_hnp(struct admhcd *ahcd);

#else

#define	admhc_start_port_reset		NULL

#endif

/*-------------------------------------------------------------------------*/


/* See usb 7.1.7.5:  root hubs must issue at least 50 msec reset signaling,
 * not necessarily continuous ... to guard against resume signaling.
 * The short timeout is safe for non-root hubs, and is backward-compatible
 * with earlier Linux hosts.
 */
#ifdef	CONFIG_USB_SUSPEND
#define	PORT_RESET_MSEC		50
#else
#define	PORT_RESET_MSEC		10
#endif

/* this timer value might be vendor-specific ... */
#define	PORT_RESET_HW_MSEC	10

/* wrap-aware logic morphed from <linux/jiffies.h> */
#define tick_before(t1, t2) ((s16)(((s16)(t1)) - ((s16)(t2))) < 0)

/* called from some task, normally khubd */
static inline int admhc_port_reset(struct admhcd *ahcd, unsigned port)
{
	u32 t;

	admhc_vdbg(ahcd, "reset port%d\n", port);
	t = admhc_read_portstatus(ahcd, port);
	if (!(t & ADMHC_PS_CCS))
		return -ENODEV;

	admhc_write_portstatus(ahcd, port, ADMHC_PS_SPR);
	mdelay(10);
	admhc_write_portstatus(ahcd, port, (ADMHC_PS_SPE | ADMHC_PS_CSC));
	mdelay(100);

	return 0;
}

static inline int admhc_port_enable(struct admhcd *ahcd, unsigned port)
{
	u32 t;

	admhc_vdbg(ahcd, "enable port%d\n", port);
	t = admhc_read_portstatus(ahcd, port);
	if (!(t & ADMHC_PS_CCS))
		return -ENODEV;

	admhc_write_portstatus(ahcd, port, ADMHC_PS_SPE);

	return 0;
}

static inline int admhc_port_disable(struct admhcd *ahcd, unsigned port)
{
	u32 t;

	admhc_vdbg(ahcd, "disable port%d\n", port);
	t = admhc_read_portstatus(ahcd, port);
	if (!(t & ADMHC_PS_CCS))
		return -ENODEV;

	admhc_write_portstatus(ahcd, port, ADMHC_PS_CPE);

	return 0;
}

static inline int admhc_port_write(struct admhcd *ahcd, unsigned port,
		u32 val)
{
#ifdef ADMHC_VERBOSE_DEBUG
	dbg_port_write(ahcd, "write", port, val);
#endif
	admhc_write_portstatus(ahcd, port, val);

	return 0;
}

static int admhc_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buf, u16 wLength)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	int		ports = ahcd->num_ports;
	int		ret = 0;

	if (unlikely(!HCD_HW_ACCESSIBLE(hcd)))
		return -ESHUTDOWN;

	switch (typeReq) {
	case ClearHubFeature:
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
#if 0			/* FIXME */
			admhc_writel(ahcd, ADMHC_RH_OCIC,
					&ahcd->regs->roothub.status);
#endif
		case C_HUB_LOCAL_POWER:
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;

		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			ret = admhc_port_disable(ahcd, wIndex);
			break;
		case USB_PORT_FEAT_SUSPEND:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_CPS);
			break;
		case USB_PORT_FEAT_POWER:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_CPP);
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_CSC);
			break;
		case USB_PORT_FEAT_C_ENABLE:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_PESC);
			break;
		case USB_PORT_FEAT_C_SUSPEND:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_PSSC);
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_OCIC);
			break;
		case USB_PORT_FEAT_C_RESET:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_PRSC);
			break;
		default:
			goto error;
		}
		break;
	case GetHubDescriptor:
		ret = admhc_get_hub_descriptor(ahcd, buf);
		break;
	case GetHubStatus:
		ret = admhc_get_hub_status(ahcd, buf);
		break;
	case GetPortStatus:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;

		ret = admhc_get_port_status(ahcd, wIndex, buf);
		break;
	case SetHubFeature:
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
			/* FIXME:  this can be cleared, yes? */
		case C_HUB_LOCAL_POWER:
			break;
		default:
			goto error;
		}
		break;
	case SetPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;

		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			ret = admhc_port_enable(ahcd, wIndex);
			break;
		case USB_PORT_FEAT_RESET:
			ret = admhc_port_reset(ahcd, wIndex);
			break;
		case USB_PORT_FEAT_SUSPEND:
#ifdef	CONFIG_USB_OTG
			if (hcd->self.otg_port == (wIndex + 1)
					&& hcd->self.b_hnp_enable)
				start_hnp(ahcd);
			else
#endif
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_SPS);
			break;
		case USB_PORT_FEAT_POWER:
			ret = admhc_port_write(ahcd, wIndex, ADMHC_PS_SPP);
			break;
		default:
			goto error;
		}
		break;

	default:
error:
		/* "protocol stall" on error */
		ret = -EPIPE;
	}

	return ret;
}

