/*
 * SDRAM init values
 *
 * Copyright 2007, Broadcom Corporation
 * Copyright 2009, OpenWrt.org
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef _sdinitvals_h_
#define _sdinitvals_h_

/* SDRAM refresh control (refresh) register bits */
#define SDRAM_REF(p)    (((p)&0xff) | SDRAM_REF_EN)     /* Refresh period */
#define SDRAM_REF_EN    0x8000          /* Writing 1 enables periodic refresh */

/* SDRAM Core default Init values (OCP ID 0x803) */
#define MEM4MX16X2      0x419   /* 16 MB */

#define SDRAM_INIT	MEM4MX16X2
#define SDRAM_BURSTFULL 0x0000  /* Use full page bursts */
#define SDRAM_CONFIG    SDRAM_BURSTFULL
#define SDRAM_REFRESH   SDRAM_REF(0x40)

#endif /* _sdinitvals_h_ */
