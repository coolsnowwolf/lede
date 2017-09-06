/*
 * wl_glue.h: Broadcom WL support module providing a unified SSB/BCMA handling.
 * Copyright (C) 2011 Jo-Philipp Wich <jo@mein.io>
 */

#include <linux/types.h>

typedef void * (*wl_glue_attach_cb_t)(u16, u16, ulong, void *, u32);
typedef void (*wl_glue_remove_cb_t)(void *);

enum wl_glue_bus_type {
	WL_GLUE_BUS_TYPE_UNSPEC,
	WL_GLUE_BUS_TYPE_SSB,
	WL_GLUE_BUS_TYPE_BCMA
};

extern void wl_glue_set_attach_callback(wl_glue_attach_cb_t cb);
extern void wl_glue_set_remove_callback(wl_glue_remove_cb_t cb);
extern int wl_glue_register(void);
extern int wl_glue_unregister(void);
extern struct device * wl_glue_get_dmadev(void *);

