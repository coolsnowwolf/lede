/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci.h
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

/*
 * __hc32 and __hc16 are "Host Controller" types, they may be equivalent to
 * __leXX (normally) or __beXX (given OHCI_BIG_ENDIAN), depending on the
 * host controller implementation.
 */
typedef __u32 __bitwise __hc32;
typedef __u16 __bitwise __hc16;

/*
 * OHCI Endpoint Descriptor (ED) ... holds TD queue
 * See OHCI spec, section 4.2
 *
 * This is a "Queue Head" for those transfers, which is why
 * both EHCI and UHCI call similar structures a "QH".
 */

#define TD_DATALEN_MAX	4096

#define ED_ALIGN	16
#define ED_MASK	((u32)~(ED_ALIGN-1))	/* strip hw status in low addr bits */

struct ed {
	/* first fields are hardware-specified */
	__hc32			hwINFO;      /* endpoint config bitmap */
	/* info bits defined by hcd */
#define ED_DEQUEUE	(1 << 27)
	/* info bits defined by the hardware */
#define ED_MPS_SHIFT	16
#define ED_MPS_MASK	((1 << 11)-1)
#define ED_MPS_GET(x)	(((x) >> ED_MPS_SHIFT) & ED_MPS_MASK)
#define ED_ISO		(1 << 15)		/* isochronous endpoint */
#define ED_SKIP		(1 << 14)
#define ED_SPEED_FULL	(1 << 13)		/* fullspeed device */
#define ED_INT		(1 << 11)		/* interrupt endpoint */
#define ED_EN_SHIFT	7			/* endpoint shift */
#define ED_EN_MASK	((1 << 4)-1)		/* endpoint mask */
#define ED_EN_GET(x)	(((x) >> ED_EN_SHIFT) & ED_EN_MASK)
#define ED_FA_MASK	((1 << 7)-1)		/* function address mask */
#define ED_FA_GET(x)	((x) & ED_FA_MASK)
	__hc32			hwTailP;	/* tail of TD list */
	__hc32			hwHeadP;	/* head of TD list (hc r/w) */
#define ED_C		(0x02)			/* toggle carry */
#define ED_H		(0x01)			/* halted */
	__hc32			hwNextED;	/* next ED in list */

	/* rest are purely for the driver's use */
	dma_addr_t		dma;		/* addr of ED */
	struct td		*dummy;		/* next TD to activate */

	struct list_head	urb_list;	/* list of our URBs */

	/* host's view of schedule */
	struct ed		*ed_next;	/* on schedule list */
	struct ed		*ed_prev;	/* for non-interrupt EDs */
	struct ed		*ed_rm_next;	/* on rm list */
	struct list_head	td_list;	/* "shadow list" of our TDs */

	/* create --> IDLE --> OPER --> ... --> IDLE --> destroy
	 * usually:  OPER --> UNLINK --> (IDLE | OPER) --> ...
	 */
	u8			state;		/* ED_{IDLE,UNLINK,OPER} */
#define ED_IDLE		0x00		/* NOT linked to HC */
#define ED_UNLINK	0x01		/* being unlinked from hc */
#define ED_OPER		0x02		/* IS linked to hc */

	u8			type;		/* PIPE_{BULK,...} */

	/* periodic scheduling params (for intr and iso) */
	u8			branch;
	u16			interval;
	u16			load;
	u16			last_iso;	/* iso only */

	/* HC may see EDs on rm_list until next frame (frame_no == tick) */
	u16			tick;
} __attribute__ ((aligned(ED_ALIGN)));

/*
 * OHCI Transfer Descriptor (TD) ... one per transfer segment
 * See OHCI spec, sections 4.3.1 (general = control/bulk/interrupt)
 * and 4.3.2 (iso)
 */

#define TD_ALIGN	32
#define TD_MASK	((u32)~(TD_ALIGN-1))	/* strip hw status in low addr bits */

struct td {
	/* first fields are hardware-specified */
	__hc32		hwINFO;		/* transfer info bitmask */

	/* hwINFO bits */
#define TD_OWN		(1 << 31)		/* owner of the descriptor */
#define TD_CC_SHIFT	27			/* condition code */
#define TD_CC_MASK	0xf
#define TD_CC		(TD_CC_MASK << TD_CC_SHIFT)
#define TD_CC_GET(x)	(((x) >> TD_CC_SHIFT) & TD_CC_MASK)

#define TD_EC_SHIFT	25			/* error count */
#define TD_EC_MASK	0x3
#define TD_EC		(TD_EC_MASK << TD_EC_SHIFT)
#define TD_EC_GET(x)	((x >> TD_EC_SHIFT) & TD_EC_MASK)
#define TD_T_SHIFT	23			/* data toggle state */
#define TD_T_MASK	0x3
#define TD_T		(TD_T_MASK << TD_T_SHIFT)
#define TD_T_DATA0	(0x2 << TD_T_SHIFT)	/* DATA0 */
#define TD_T_DATA1	(0x3 << TD_T_SHIFT)	/* DATA1 */
#define TD_T_CARRY	(0x0 << TD_T_SHIFT)	/* uses ED_C */
#define TD_T_GET(x)	(((x) >> TD_T_SHIFT) & TD_T_MASK)
#define TD_DP_SHIFT	21			/* direction/pid */
#define TD_DP_MASK	0x3
#define TD_DP		(TD_DP_MASK << TD_DP_SHIFT)
#define TD_DP_GET	(((x) >> TD_DP_SHIFT) & TD_DP_MASK)
#define TD_DP_SETUP	(0x0 << TD_DP_SHIFT)	/* SETUP pid */
#define TD_DP_OUT	(0x1 << TD_DP_SHIFT)	/* OUT pid */
#define TD_DP_IN	(0x2 << TD_DP_SHIFT)	/* IN pid */
#define TD_ISI_SHIFT	8			/* Interrupt Service Interval */
#define TD_ISI_MASK	0x3f
#define TD_ISI_GET(x)	(((x) >> TD_ISI_SHIFT) & TD_ISI_MASK)
#define TD_FN_MASK	0x3f			/* frame number */
#define TD_FN_GET(x)	((x) & TD_FN_MASK)

	__hc32		hwDBP;		/* Data Buffer Pointer (or 0) */
	__hc32		hwCBL;		/* Controller/Buffer Length */

	/* hwCBL bits */
#define TD_BL_MASK	0xffff		/* buffer length */
#define TD_BL_GET(x)	((x) & TD_BL_MASK)
#define TD_IE		(1 << 16)	/* interrupt enable */
	__hc32		hwNextTD;	/* Next TD Pointer */

	/* rest are purely for the driver's use */
	__u8		index;
	struct ed	*ed;
	struct td	*td_hash;	/* dma-->td hashtable */
	struct td	*next_dl_td;
	struct urb	*urb;

	dma_addr_t	td_dma;		/* addr of this TD */
	dma_addr_t	data_dma;	/* addr of data it points to */

	struct list_head td_list;	/* "shadow list", TDs on same ED */

	u32		flags;
#define TD_FLAG_DONE	(1 << 17)	/* retired to done list */
#define TD_FLAG_ISO	(1 << 16)	/* copy of ED_ISO */
} __attribute__ ((aligned(TD_ALIGN)));	/* c/b/i need 16; only iso needs 32 */

/*
 * Hardware transfer status codes -- CC from td->hwINFO
 */
#define TD_CC_NOERROR		0x00
#define TD_CC_CRC		0x01
#define TD_CC_BITSTUFFING	0x02
#define TD_CC_DATATOGGLEM	0x03
#define TD_CC_STALL		0x04
#define TD_CC_DEVNOTRESP	0x05
#define TD_CC_PIDCHECKFAIL	0x06
#define TD_CC_UNEXPECTEDPID	0x07
#define TD_CC_DATAOVERRUN	0x08
#define TD_CC_DATAUNDERRUN	0x09
    /* 0x0A, 0x0B reserved for hardware */
#define TD_CC_BUFFEROVERRUN	0x0C
#define TD_CC_BUFFERUNDERRUN	0x0D
    /* 0x0E, 0x0F reserved for HCD */
#define TD_CC_HCD0		0x0E
#define TD_CC_NOTACCESSED	0x0F

/*
 * preshifted status codes
 */
#define TD_SCC_NOTACCESSED	(TD_CC_NOTACCESSED << TD_CC_SHIFT)


/* map OHCI TD status codes (CC) to errno values */
static const int cc_to_error[16] = {
	/* No  Error  */	0,
	/* CRC Error  */	-EILSEQ,
	/* Bit Stuff  */	-EPROTO,
	/* Data Togg  */	-EILSEQ,
	/* Stall      */	-EPIPE,
	/* DevNotResp */	-ETIME,
	/* PIDCheck   */	-EPROTO,
	/* UnExpPID   */	-EPROTO,
	/* DataOver   */	-EOVERFLOW,
	/* DataUnder  */	-EREMOTEIO,
	/* (for hw)   */	-EIO,
	/* (for hw)   */	-EIO,
	/* BufferOver */	-ECOMM,
	/* BuffUnder  */	-ENOSR,
	/* (for HCD)  */	-EALREADY,
	/* (for HCD)  */	-EALREADY
};

#define NUM_INTS	32

/*
 * This is the structure of the OHCI controller's memory mapped I/O region.
 * You must use readl() and writel() (in <asm/io.h>) to access these fields!!
 * Layout is in section 7 (and appendix B) of the spec.
 */
struct admhcd_regs {
	__hc32	gencontrol;	/* General Control */
	__hc32	int_status;	/* Interrupt Status */
	__hc32	int_enable;	/* Interrupt Enable */
	__hc32	reserved00;
	__hc32	host_control;	/* Host General Control */
	__hc32	reserved01;
	__hc32	fminterval;	/* Frame Interval */
	__hc32	fmnumber;	/* Frame Number */
	__hc32	reserved02;
	__hc32	reserved03;
	__hc32	reserved04;
	__hc32	reserved05;
	__hc32	reserved06;
	__hc32	reserved07;
	__hc32	reserved08;
	__hc32	reserved09;
	__hc32	reserved10;
	__hc32	reserved11;
	__hc32	reserved12;
	__hc32	reserved13;
	__hc32	reserved14;
	__hc32	reserved15;
	__hc32	reserved16;
	__hc32	reserved17;
	__hc32	reserved18;
	__hc32	reserved19;
	__hc32	reserved20;
	__hc32	reserved21;
	__hc32	lsthresh;	/* Low Speed Threshold */
	__hc32	rhdesc;		/* Root Hub Descriptor */
#define MAX_ROOT_PORTS	2
	__hc32	portstatus[MAX_ROOT_PORTS]; /* Port Status */
	__hc32	hosthead;	/* Host Descriptor Head */
} __attribute__ ((aligned(32)));

/*
 * General Control register bits
 */
#define ADMHC_CTRL_UHFE	(1 << 0)	/* USB Host Function Enable */
#define ADMHC_CTRL_SIR	(1 << 1)	/* Software Interrupt request */
#define ADMHC_CTRL_DMAA	(1 << 2)	/* DMA Arbitration Control */
#define ADMHC_CTRL_SR	(1 << 3)	/* Software Reset */

/*
 * Host General Control register bits
 */
#define ADMHC_HC_BUSS		0x3		/* USB bus state */
#define   ADMHC_BUSS_RESET	0x0
#define   ADMHC_BUSS_RESUME	0x1
#define   ADMHC_BUSS_OPER	0x2
#define   ADMHC_BUSS_SUSPEND	0x3
#define ADMHC_HC_DMAE		(1 << 2)	/* DMA enable */

/*
 * Interrupt Status/Enable register bits
 */
#define ADMHC_INTR_SOFI	(1 << 4)	/* start of frame */
#define ADMHC_INTR_RESI	(1 << 5)	/* resume detected */
#define ADMHC_INTR_6	(1 << 6)	/* unknown */
#define ADMHC_INTR_7	(1 << 7)	/* unknown */
#define ADMHC_INTR_BABI	(1 << 8)	/* babble detected */
#define ADMHC_INTR_INSM	(1 << 9)	/* root hub status change */
#define ADMHC_INTR_SO	(1 << 10)	/* scheduling overrun */
#define ADMHC_INTR_FNO	(1 << 11)	/* frame number overflow */
#define ADMHC_INTR_TDC	(1 << 20)	/* transfer descriptor completed */
#define ADMHC_INTR_SWI	(1 << 29)	/* software interrupt */
#define ADMHC_INTR_FATI	(1 << 30)	/* fatal error */
#define ADMHC_INTR_INTA	(1 << 31)	/* interrupt active */

#define ADMHC_INTR_MIE	(1 << 31)	/* master interrupt enable */

/*
 * SOF Frame Interval register bits
 */
#define ADMHC_SFI_FI_MASK	((1 << 14)-1)	/* Frame Interval value */
#define ADMHC_SFI_FSLDP_SHIFT	16
#define ADMHC_SFI_FSLDP_MASK	((1 << 15)-1)
#define ADMHC_SFI_FIT		(1 << 31)	/* Frame Interval Toggle */

/*
 * SOF Frame Number register bits
 */
#define ADMHC_SFN_FN_MASK	((1 << 16)-1)	/* Frame Number Mask */
#define ADMHC_SFN_FR_SHIFT	16		/* Frame Remaining Shift */
#define ADMHC_SFN_FR_MASK	((1 << 14)-1)	/* Frame Remaining Mask */
#define ADMHC_SFN_FRT		(1 << 31)	/* Frame Remaining Toggle */

/*
 * Root Hub Descriptor register bits
 */
#define ADMHC_RH_NUMP	0xff		/* number of ports */
#define	ADMHC_RH_PSM	(1 << 8)	/* power switching mode */
#define	ADMHC_RH_NPS	(1 << 9)	/* no power switching */
#define	ADMHC_RH_OCPM	(1 << 10)	/* over current protection mode */
#define	ADMHC_RH_NOCP	(1 << 11)	/* no over current protection */
#define	ADMHC_RH_PPCM	(0xff << 16)	/* port power control */

#define ADMHC_RH_LPS	(1 << 24)	/* local power switch */
#define ADMHC_RH_OCI	(1 << 25)	/* over current indicator */

/* status change bits */
#define ADMHC_RH_LPSC	(1 << 26)	/* local power switch change */
#define ADMHC_RH_OCIC	(1 << 27)	/* over current indicator change */

#define ADMHC_RH_DRWE	(1 << 28)	/* device remote wakeup enable */
#define ADMHC_RH_CRWE	(1 << 29)	/* clear remote wakeup enable */

#define ADMHC_RH_CGP	(1 << 24)	/* clear global power */
#define ADMHC_RH_SGP	(1 << 26)	/* set global power */

/*
 * Port Status register bits
 */
#define ADMHC_PS_CCS	(1 << 0)	/* current connect status */
#define ADMHC_PS_PES	(1 << 1)	/* port enable status */
#define ADMHC_PS_PSS	(1 << 2)	/* port suspend status */
#define ADMHC_PS_POCI	(1 << 3)	/* port over current indicator */
#define ADMHC_PS_PRS	(1 << 4)	/* port reset status */
#define ADMHC_PS_PPS	(1 << 8)	/* port power status */
#define ADMHC_PS_LSDA	(1 << 9)	/* low speed device attached */

/* status change bits */
#define ADMHC_PS_CSC	(1 << 16)	/* connect status change */
#define ADMHC_PS_PESC	(1 << 17)	/* port enable status change */
#define ADMHC_PS_PSSC	(1 << 18)	/* port suspend status change */
#define ADMHC_PS_OCIC	(1 << 19)	/* over current indicator change */
#define ADMHC_PS_PRSC	(1 << 20)	/* port reset status change */

/* port feature bits */
#define ADMHC_PS_CPE	(1 << 0)	/* clear port enable */
#define ADMHC_PS_SPE	(1 << 1)	/* set port enable */
#define ADMHC_PS_SPS	(1 << 2)	/* set port suspend */
#define ADMHC_PS_CPS	(1 << 3)	/* clear suspend status */
#define ADMHC_PS_SPR	(1 << 4)	/* set port reset */
#define ADMHC_PS_SPP	(1 << 8)	/* set port power */
#define ADMHC_PS_CPP	(1 << 9)	/* clear port power */

/*
 * the POTPGT value is not defined in the ADMHC, so define a dummy value
 */
#define ADMHC_POTPGT	2		/* in ms */

/* hcd-private per-urb state */
struct urb_priv {
	struct ed		*ed;
	struct list_head	pending;	/* URBs on the same ED */

	u32			td_cnt;		/* # tds in this request */
	u32			td_idx;		/* index of the current td */
	struct td		*td[0];		/* all TDs in this request */
};

#define TD_HASH_SIZE    64    /* power'o'two */
/* sizeof (struct td) ~= 64 == 2^6 ... */
#define TD_HASH_FUNC(td_dma) ((td_dma ^ (td_dma >> 6)) % TD_HASH_SIZE)

/*
 * This is the full ADMHCD controller description
 *
 * Note how the "proper" USB information is just
 * a subset of what the full implementation needs. (Linus)
 */

struct admhcd {
	spinlock_t		lock;

	/*
	 * I/O memory used to communicate with the HC (dma-consistent)
	 */
	struct admhcd_regs __iomem *regs;

	/*
	 * hcd adds to schedule for a live hc any time, but removals finish
	 * only at the start of the next frame.
	 */

	struct ed		*ed_head;
	struct ed		*ed_tails[4];

	struct ed		*ed_rm_list;	/* to be removed */

	struct ed		*periodic[NUM_INTS];	/* shadow int_table */

#if 0	/* TODO: remove? */
	/*
	 * OTG controllers and transceivers need software interaction;
	 * other external transceivers should be software-transparent
	 */
	struct otg_transceiver	*transceiver;
	void (*start_hnp)(struct admhcd *ahcd);
#endif

	/*
	 * memory management for queue data structures
	 */
	struct dma_pool		*td_cache;
	struct dma_pool		*ed_cache;
	struct td		*td_hash[TD_HASH_SIZE];
	struct list_head	pending;

	/*
	 * driver state
	 */
	int			num_ports;
	int			load[NUM_INTS];
	u32			host_control;	/* copy of the host_control reg */
	unsigned long		next_statechange;	/* suspend/resume */
	u32			fminterval;		/* saved register */
	unsigned		autostop:1;	/* rh auto stopping/stopped */

	unsigned long		flags;		/* for HC bugs */
#define	OHCI_QUIRK_AMD756	0x01			/* erratum #4 */
#define	OHCI_QUIRK_SUPERIO	0x02			/* natsemi */
#define	OHCI_QUIRK_INITRESET	0x04			/* SiS, OPTi, ... */
#define	OHCI_QUIRK_BE_DESC	0x08			/* BE descriptors */
#define	OHCI_QUIRK_BE_MMIO	0x10			/* BE registers */
#define	OHCI_QUIRK_ZFMICRO	0x20			/* Compaq ZFMicro chipset*/
	/* there are also chip quirks/bugs in init logic */

#ifdef DEBUG
	struct dentry		*debug_dir;
	struct dentry		*debug_async;
	struct dentry		*debug_periodic;
	struct dentry		*debug_registers;
#endif
};

/* convert between an hcd pointer and the corresponding ahcd_hcd */
static inline struct admhcd *hcd_to_admhcd(struct usb_hcd *hcd)
{
	return (struct admhcd *)(hcd->hcd_priv);
}
static inline struct usb_hcd *admhcd_to_hcd(const struct admhcd *ahcd)
{
	return container_of((void *)ahcd, struct usb_hcd, hcd_priv);
}

/*-------------------------------------------------------------------------*/

#ifndef DEBUG
#define STUB_DEBUG_FILES
#endif	/* DEBUG */

#ifdef DEBUG
#	define admhc_dbg(ahcd, fmt, args...) \
		printk(KERN_DEBUG "adm5120-hcd: " fmt, ## args)
#else
#	define admhc_dbg(ahcd, fmt, args...) do { } while (0)
#endif

#define admhc_err(ahcd, fmt, args...) \
	printk(KERN_ERR "adm5120-hcd: " fmt, ## args)
#define admhc_info(ahcd, fmt, args...) \
	printk(KERN_INFO "adm5120-hcd: " fmt, ## args)
#define admhc_warn(ahcd, fmt, args...) \
	printk(KERN_WARNING "adm5120-hcd: " fmt, ## args)

#ifdef ADMHC_VERBOSE_DEBUG
#	define admhc_vdbg admhc_dbg
#else
#	define admhc_vdbg(ahcd, fmt, args...) do { } while (0)
#endif

/*-------------------------------------------------------------------------*/

/*
 * While most USB host controllers implement their registers and
 * in-memory communication descriptors in little-endian format,
 * a minority (notably the IBM STB04XXX and the Motorola MPC5200
 * processors) implement them in big endian format.
 *
 * In addition some more exotic implementations like the Toshiba
 * Spider (aka SCC) cell southbridge are "mixed" endian, that is,
 * they have a different endianness for registers vs. in-memory
 * descriptors.
 *
 * This attempts to support either format at compile time without a
 * runtime penalty, or both formats with the additional overhead
 * of checking a flag bit.
 *
 * That leads to some tricky Kconfig rules howevber. There are
 * different defaults based on some arch/ppc platforms, though
 * the basic rules are:
 *
 * Controller type              Kconfig options needed
 * ---------------              ----------------------
 * little endian                CONFIG_USB_ADMHC_LITTLE_ENDIAN
 *
 * fully big endian             CONFIG_USB_ADMHC_BIG_ENDIAN_DESC _and_
 *                              CONFIG_USB_ADMHC_BIG_ENDIAN_MMIO
 *
 * mixed endian                 CONFIG_USB_ADMHC_LITTLE_ENDIAN _and_
 *                              CONFIG_USB_OHCI_BIG_ENDIAN_{MMIO,DESC}
 *
 * (If you have a mixed endian controller, you -must- also define
 * CONFIG_USB_ADMHC_LITTLE_ENDIAN or things will not work when building
 * both your mixed endian and a fully big endian controller support in
 * the same kernel image).
 */

#ifdef CONFIG_USB_ADMHC_BIG_ENDIAN_DESC
#ifdef CONFIG_USB_ADMHC_LITTLE_ENDIAN
#define big_endian_desc(ahcd)	(ahcd->flags & OHCI_QUIRK_BE_DESC)
#else
#define big_endian_desc(ahcd)	1		/* only big endian */
#endif
#else
#define big_endian_desc(ahcd)	0		/* only little endian */
#endif

#ifdef CONFIG_USB_ADMHC_BIG_ENDIAN_MMIO
#ifdef CONFIG_USB_ADMHC_LITTLE_ENDIAN
#define big_endian_mmio(ahcd)	(ahcd->flags & OHCI_QUIRK_BE_MMIO)
#else
#define big_endian_mmio(ahcd)	1		/* only big endian */
#endif
#else
#define big_endian_mmio(ahcd)	0		/* only little endian */
#endif

/*
 * Big-endian read/write functions are arch-specific.
 * Other arches can be added if/when they're needed.
 *
 */
static inline unsigned int admhc_readl(const struct admhcd *ahcd,
	__hc32 __iomem *regs)
{
#ifdef CONFIG_USB_ADMHC_BIG_ENDIAN_MMIO
	return big_endian_mmio(ahcd) ?
		readl_be(regs) :
		readl(regs);
#else
	return readl(regs);
#endif
}

static inline void admhc_writel(const struct admhcd *ahcd,
	const unsigned int val, __hc32 __iomem *regs)
{
#ifdef CONFIG_USB_ADMHC_BIG_ENDIAN_MMIO
	big_endian_mmio(ahcd) ?
		writel_be(val, regs) :
		writel(val, regs);
#else
		writel(val, regs);
#endif
}

static inline void admhc_writel_flush(const struct admhcd *ahcd)
{
#if 0
	/* TODO: remove? */
	(void) admhc_readl(ahcd, &ahcd->regs->gencontrol);
#endif
}


/*-------------------------------------------------------------------------*/

/* cpu to ahcd */
static inline __hc16 cpu_to_hc16(const struct admhcd *ahcd, const u16 x)
{
	return big_endian_desc(ahcd) ?
		(__force __hc16)cpu_to_be16(x) :
		(__force __hc16)cpu_to_le16(x);
}

static inline __hc16 cpu_to_hc16p(const struct admhcd *ahcd, const u16 *x)
{
	return big_endian_desc(ahcd) ?
		cpu_to_be16p(x) :
		cpu_to_le16p(x);
}

static inline __hc32 cpu_to_hc32(const struct admhcd *ahcd, const u32 x)
{
	return big_endian_desc(ahcd) ?
		(__force __hc32)cpu_to_be32(x) :
		(__force __hc32)cpu_to_le32(x);
}

static inline __hc32 cpu_to_hc32p(const struct admhcd *ahcd, const u32 *x)
{
	return big_endian_desc(ahcd) ?
		cpu_to_be32p(x) :
		cpu_to_le32p(x);
}

/* ahcd to cpu */
static inline u16 hc16_to_cpu(const struct admhcd *ahcd, const __hc16 x)
{
	return big_endian_desc(ahcd) ?
		be16_to_cpu((__force __be16)x) :
		le16_to_cpu((__force __le16)x);
}

static inline u16 hc16_to_cpup(const struct admhcd *ahcd, const __hc16 *x)
{
	return big_endian_desc(ahcd) ?
		be16_to_cpup((__force __be16 *)x) :
		le16_to_cpup((__force __le16 *)x);
}

static inline u32 hc32_to_cpu(const struct admhcd *ahcd, const __hc32 x)
{
	return big_endian_desc(ahcd) ?
		be32_to_cpu((__force __be32)x) :
		le32_to_cpu((__force __le32)x);
}

static inline u32 hc32_to_cpup(const struct admhcd *ahcd, const __hc32 *x)
{
	return big_endian_desc(ahcd) ?
		be32_to_cpup((__force __be32 *)x) :
		le32_to_cpup((__force __le32 *)x);
}

/*-------------------------------------------------------------------------*/

static inline u16 admhc_frame_no(const struct admhcd *ahcd)
{
	u32	t;

	t = admhc_readl(ahcd, &ahcd->regs->fmnumber) & ADMHC_SFN_FN_MASK;
	return (u16)t;
}

static inline u16 admhc_frame_remain(const struct admhcd *ahcd)
{
	u32	t;

	t = admhc_readl(ahcd, &ahcd->regs->fmnumber) >> ADMHC_SFN_FR_SHIFT;
	t &= ADMHC_SFN_FR_MASK;
	return (u16)t;
}

/*-------------------------------------------------------------------------*/

static inline void admhc_disable(struct admhcd *ahcd)
{
	admhcd_to_hcd(ahcd)->state = HC_STATE_HALT;
}

#define	FI		0x2edf		/* 12000 bits per frame (-1) */
#define	FSLDP(fi)	(0x7fff & ((6 * ((fi) - 1200)) / 7))
#define	FIT		ADMHC_SFI_FIT
#define LSTHRESH	0x628		/* lowspeed bit threshold */

static inline void periodic_reinit(struct admhcd *ahcd)
{
#if 0
	u32	fi = ahcd->fminterval & ADMHC_SFI_FI_MASK;
	u32	fit = admhc_readl(ahcd, &ahcd->regs->fminterval) & FIT;

	/* TODO: adjust FSLargestDataPacket value too? */
	admhc_writel(ahcd, (fit ^ FIT) | ahcd->fminterval,
					&ahcd->regs->fminterval);
#else
	u32	fit = admhc_readl(ahcd, &ahcd->regs->fminterval) & FIT;

	/* TODO: adjust FSLargestDataPacket value too? */
	admhc_writel(ahcd, (fit ^ FIT) | ahcd->fminterval,
					&ahcd->regs->fminterval);
#endif
}

static inline u32 admhc_read_rhdesc(struct admhcd *ahcd)
{
	return admhc_readl(ahcd, &ahcd->regs->rhdesc);
}

static inline u32 admhc_read_portstatus(struct admhcd *ahcd, int port)
{
	return admhc_readl(ahcd, &ahcd->regs->portstatus[port]);
}

static inline void admhc_write_portstatus(struct admhcd *ahcd, int port,
		u32 value)
{
	admhc_writel(ahcd, value, &ahcd->regs->portstatus[port]);
}

static inline void roothub_write_status(struct admhcd *ahcd, u32 value)
{
	/* FIXME: read-only bits must be masked out */
	admhc_writel(ahcd, value, &ahcd->regs->rhdesc);
}

static inline void admhc_intr_disable(struct admhcd *ahcd, u32 ints)
{
	u32	t;

	t = admhc_readl(ahcd, &ahcd->regs->int_enable);
	t &= ~(ints);
	admhc_writel(ahcd, t, &ahcd->regs->int_enable);
	/* TODO: flush writes ?*/
}

static inline void admhc_intr_enable(struct admhcd *ahcd, u32 ints)
{
	u32	t;

	t = admhc_readl(ahcd, &ahcd->regs->int_enable);
	t |= ints;
	admhc_writel(ahcd, t, &ahcd->regs->int_enable);
	/* TODO: flush writes ?*/
}

static inline void admhc_intr_ack(struct admhcd *ahcd, u32 ints)
{
	admhc_writel(ahcd, ints, &ahcd->regs->int_status);
}

static inline void admhc_dma_enable(struct admhcd *ahcd)
{
	u32 t;

	t = admhc_readl(ahcd, &ahcd->regs->host_control);
	if (t & ADMHC_HC_DMAE)
		return;

	t |= ADMHC_HC_DMAE;
	admhc_writel(ahcd, t, &ahcd->regs->host_control);
	admhc_vdbg(ahcd, "DMA enabled\n");
}

static inline void admhc_dma_disable(struct admhcd *ahcd)
{
	u32 t;

	t = admhc_readl(ahcd, &ahcd->regs->host_control);
	if (!(t & ADMHC_HC_DMAE))
		return;

	t &= ~ADMHC_HC_DMAE;
	admhc_writel(ahcd, t, &ahcd->regs->host_control);
	admhc_vdbg(ahcd, "DMA disabled\n");
}
