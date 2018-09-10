/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci-mem.c
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

/*-------------------------------------------------------------------------*/

/*
 * OHCI deals with three types of memory:
 *	- data used only by the HCD ... kmalloc is fine
 *	- async and periodic schedules, shared by HC and HCD ... these
 *	  need to use dma_pool or dma_alloc_coherent
 *	- driver buffers, read/written by HC ... the hcd glue or the
 *	  device driver provides us with dma addresses
 *
 * There's also "register" data, which is memory mapped.
 * No memory seen by this driver (or any HCD) may be paged out.
 */

/*-------------------------------------------------------------------------*/

static void admhc_hcd_init(struct admhcd *ahcd)
{
	ahcd->next_statechange = jiffies;
	spin_lock_init(&ahcd->lock);
	INIT_LIST_HEAD(&ahcd->pending);
}

/*-------------------------------------------------------------------------*/

static int admhc_mem_init(struct admhcd *ahcd)
{
	ahcd->td_cache = dma_pool_create("admhc_td",
		admhcd_to_hcd(ahcd)->self.controller,
		sizeof(struct td),
		TD_ALIGN, /* byte alignment */
		0 /* no page-crossing issues */
		);
	if (!ahcd->td_cache)
		goto err;

	ahcd->ed_cache = dma_pool_create("admhc_ed",
		admhcd_to_hcd(ahcd)->self.controller,
		sizeof(struct ed),
		ED_ALIGN, /* byte alignment */
		0 /* no page-crossing issues */
		);
	if (!ahcd->ed_cache)
		goto err_td_cache;

	return 0;

err_td_cache:
	dma_pool_destroy(ahcd->td_cache);
	ahcd->td_cache = NULL;
err:
	return -ENOMEM;
}

static void admhc_mem_cleanup(struct admhcd *ahcd)
{
	if (ahcd->td_cache) {
		dma_pool_destroy(ahcd->td_cache);
		ahcd->td_cache = NULL;
	}

	if (ahcd->ed_cache) {
		dma_pool_destroy(ahcd->ed_cache);
		ahcd->ed_cache = NULL;
	}
}

/*-------------------------------------------------------------------------*/

/* ahcd "done list" processing needs this mapping */
static inline struct td *dma_to_td(struct admhcd *ahcd, dma_addr_t td_dma)
{
	struct td *td;

	td_dma &= TD_MASK;
	td = ahcd->td_hash[TD_HASH_FUNC(td_dma)];
	while (td && td->td_dma != td_dma)
		td = td->td_hash;

	return td;
}

/* TDs ... */
static struct td *td_alloc(struct admhcd *ahcd, gfp_t mem_flags)
{
	dma_addr_t	dma;
	struct td	*td;

	td = dma_pool_alloc(ahcd->td_cache, mem_flags, &dma);
	if (!td)
		return NULL;

	/* in case ahcd fetches it, make it look dead */
	memset(td, 0, sizeof *td);
	td->hwNextTD = cpu_to_hc32(ahcd, dma);
	td->td_dma = dma;
	/* hashed in td_fill */

	return td;
}

static void td_free(struct admhcd *ahcd, struct td *td)
{
	struct td **prev = &ahcd->td_hash[TD_HASH_FUNC(td->td_dma)];

	while (*prev && *prev != td)
		prev = &(*prev)->td_hash;
	if (*prev)
		*prev = td->td_hash;
#if 0
	/* TODO: remove */
	else if ((td->hwINFO & cpu_to_hc32(ahcd, TD_DONE)) != 0)
		admhc_dbg(ahcd, "no hash for td %p\n", td);
#else
	else if ((td->flags & TD_FLAG_DONE) != 0)
		admhc_dbg(ahcd, "no hash for td %p\n", td);
#endif
	dma_pool_free(ahcd->td_cache, td, td->td_dma);
}

/*-------------------------------------------------------------------------*/

/* EDs ... */
static struct ed *ed_alloc(struct admhcd *ahcd, gfp_t mem_flags)
{
	dma_addr_t	dma;
	struct ed	*ed;

	ed = dma_pool_alloc(ahcd->ed_cache, mem_flags, &dma);
	if (!ed)
		return NULL;

	memset(ed, 0, sizeof(*ed));
	ed->dma = dma;

	INIT_LIST_HEAD(&ed->td_list);
	INIT_LIST_HEAD(&ed->urb_list);

	return ed;
}

static void ed_free(struct admhcd *ahcd, struct ed *ed)
{
	dma_pool_free(ahcd->ed_cache, ed, ed->dma);
}

/*-------------------------------------------------------------------------*/

/* URB priv ... */
static void urb_priv_free(struct admhcd *ahcd, struct urb_priv *urb_priv)
{
	int i;

	for (i = 0; i < urb_priv->td_cnt; i++)
		if (urb_priv->td[i])
			td_free(ahcd, urb_priv->td[i]);

	list_del(&urb_priv->pending);
	kfree(urb_priv);
}

static struct urb_priv *urb_priv_alloc(struct admhcd *ahcd, int num_tds,
		gfp_t mem_flags)
{
	struct urb_priv	*priv;

	/* allocate the private part of the URB */
	priv = kzalloc(sizeof(*priv) + sizeof(struct td) * num_tds, mem_flags);
	if (!priv)
		goto err;

	/* allocate the TDs (deferring hash chain updates) */
	for (priv->td_cnt = 0; priv->td_cnt < num_tds; priv->td_cnt++) {
		priv->td[priv->td_cnt] = td_alloc(ahcd, mem_flags);
		if (priv->td[priv->td_cnt] == NULL)
			goto err_free;
	}

	INIT_LIST_HEAD(&priv->pending);

	return priv;

err_free:
	urb_priv_free(ahcd, priv);
err:
	return NULL;
}
