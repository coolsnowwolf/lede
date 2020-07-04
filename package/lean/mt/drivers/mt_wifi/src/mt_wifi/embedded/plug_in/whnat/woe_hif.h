/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: whnat
	whnat_hif.h
*/

#ifndef _WHNAT_HIF_H
#define _WHNAT_HIF_H

#include "woe_basic.h"

/* #define WHNAT_CPU_TRACER */
#define WHNAT_CPU_TRACER_DEFAULT_EN 1
#define WHNAT_CPU_TRACER_DEFAULT_ADDR 0x1a143424
#define WHNAT_CPU_TRACER_DEFAULT_MASK 0

struct whnat_cputracer {
	unsigned long base_addr;
	unsigned char irq;
	unsigned char trace_en;
	unsigned int trace_addr;
	unsigned int trace_mask;
};

struct whnat_hif_cfg {
	unsigned long base_addr;
	unsigned int pcie_base[2];
	unsigned int wpdma_base[2];
	struct whnat_cputracer tracer;
};

enum {
	WHNAT_TRACE_EN,
	WHNAT_TRACE_ADDR,
	WHNAT_TRACE_MSK,
};

int whnat_hif_init(struct whnat_hif_cfg *hif);
int whnat_hif_exit(struct whnat_hif_cfg *hif);



#define PCIE_MIRRO_BASE 0

#define PCIE0_MAP	(PCIE_MIRRO_BASE + 0x0)
#define PCIE1_MAP	(PCIE_MIRRO_BASE + 0x4)
#define WED0_MAP	(PCIE_MIRRO_BASE + 0x8)
#define WED1_MAP	(PCIE_MIRRO_BASE + 0xC)

#define PCIE_MAP_FLD_PCIE_REMAP_EN		0
#define PCIE_MAP_FLD_PCIE_REMAP		1
#define PCIE_MAP_FLD_PCIE_ADDR_REMAP	12

#define CPU_TRACER_BASE 0

#define CPU_TRACER_CFG	(CPU_TRACER_BASE + 0x0)
#define CPU_TRACER_WP_ADDR (CPU_TRACER_BASE + 0xC)
#define CPU_TRACER_WP_MASK (CPU_TRACER_BASE + 0x10)
#define CPU_TRACER_TO_TIME (CPU_TRACER_BASE + 0x4)
#define CPU_TRACER_CR_TIME (CPU_TRACER_BASE + 0x8)



#define CPU_TRACER_CON_BUS_DBG_EN      0
#define CPU_TRACER_CON_TIMEOUT_EN      1
#define CPU_TRACER_CON_SLV_ERR_EN      2
#define CPU_TRACER_CON_WP_EN           3
#define CPU_TRACER_CON_IRQ_AR_EN       4
#define CPU_TRACER_CON_IRQ_AW_EN       5
/* DE uses exotic name for ESO items... we use another human-readable name */
#define CPU_TRACER_CON_IRQ_WP_EN       6
#define CPU_TRACER_CON_IRQ_CLR         7
#define CPU_TRACER_CON_IRQ_AR_STA      8
#define CPU_TRACER_CON_IRQ_AW_STA      9
#define CPU_TRACER_CON_IRQ_WP_STA      10

#define PCIE_BASE_ADDR0 0x1A143000
#define PCIE_BASE_ADDR1 0x1A145000

/*default usage, should get from pcie device*/
#define WPDMA_BASE_ADDR0 0x20004000
#define WPDMA_BASE_ADDR1 0x20104000
#define WPDMA_OFFSET 0x4000

#endif
