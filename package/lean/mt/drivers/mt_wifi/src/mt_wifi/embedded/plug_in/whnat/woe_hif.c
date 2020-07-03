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
	whnat_hif.c
*/

#include "woe_hif.h"

#define PCIE_MIRRO_DEV_NODE "mediatek,pcie-mirror"
#define BUS_TRACE_DEV_NODE "mediatek,BUS_DBG"

#ifdef WHNAT_CPU_TRACER
/*
*
*/
static irqreturn_t cputracer_isr(int irq, void *data)
{
	unsigned int value;
	unsigned int sta;
	struct whnat_cputracer *tracer = (struct whnat_cputracer *)data;

	WHNAT_IO_READ32(tracer, CPU_TRACER_CFG, &sta);
	value = sta | (1 << CPU_TRACER_CON_IRQ_CLR);

	if (sta & (1 << CPU_TRACER_CON_IRQ_WP_STA))
		WHNAT_DBG(WHNAT_DBG_OFF, "[TRACKER] Watch address: 0x%x was touched\n", tracer->trace_addr);

	if (sta & (1 << CPU_TRACER_CON_IRQ_AR_STA))
		WHNAT_DBG(WHNAT_DBG_OFF, "[TRAKER] Read time out trigger\n");

	if (sta & (1 << CPU_TRACER_CON_IRQ_AW_STA))
		WHNAT_DBG(WHNAT_DBG_OFF, "[TRAKER] Write time out trigger\n");

	WHNAT_IO_WRITE32(tracer, CPU_TRACER_CFG, value);
	return IRQ_HANDLED;
}

/*
*
*/
static void whnat_hif_cputracer_probe(struct whnat_hif_cfg *hif)
{
	struct device_node *node = NULL;
	struct whnat_cputracer *tracer = &hif->tracer;

	node = of_find_compatible_node(NULL, NULL, BUS_TRACE_DEV_NODE);
	tracer->base_addr = (unsigned long)of_iomap(node, 0);
	tracer->irq = irq_of_parse_and_map(node, 0);
	tracer->trace_addr = WHNAT_CPU_TRACER_DEFAULT_ADDR;
	tracer->trace_en = WHNAT_CPU_TRACER_DEFAULT_EN;
	tracer->trace_mask = WHNAT_CPU_TRACER_DEFAULT_MASK;

	if (request_irq(tracer->irq, (irq_handler_t)cputracer_isr, IRQF_TRIGGER_LOW, "cpu_tracer", (tracer)))
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): register irq: %d for cpu tracer fail!\n", __func__, tracer->irq);
	else
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): register irq: %d for cpu tracer ok!\n", __func__, tracer->irq);

	whnat_hal_trace_set(tracer);
}
#endif /*WHNAT_CPU_TRACER*/

#ifdef WED_CR_MIRROR_SUPPORT
/*
*
*/
static void whnat_hif_pcie_mirror_probe(struct whnat_hif_cfg *hif)
{
	struct device_node *node = NULL;
	/*CR mirror*/
	node = of_find_compatible_node(NULL, NULL, PCIE_MIRRO_DEV_NODE);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): get node=%p\n", __func__, &node);
	/* iomap registers */
	hif->base_addr = (unsigned long)of_iomap(node, 0);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): hif_cfg base addr=%lx\n", __func__, hif->base_addr);
	whnat_hal_hif_init(hif);
}
#endif /*WED_CR_MIRROR_SUPPORT*/

/*
*
*/
static void whnat_hif_pcie_probe(struct whnat_hif_cfg *hif)
{
	hif->pcie_base[0] = PCIE_BASE_ADDR0;
	hif->pcie_base[1] = PCIE_BASE_ADDR1;
	hif->wpdma_base[0] = WPDMA_BASE_ADDR0;
	hif->wpdma_base[1] = WPDMA_BASE_ADDR1;
}

/*
*
*/
int whnat_hif_init(struct whnat_hif_cfg *hif)
{
	whnat_hif_pcie_probe(hif);
#ifdef WED_CR_MIRROR_SUPPORT
	whnat_hif_pcie_mirror_probe(hif);
#endif /*WED_CR_MIRROR_SUPPORT*/
#ifdef WHNAT_CPU_TRACER
	whnat_hif_cputracer_probe(hif);
#endif /*WHNAT_CPU_TRACER*/
	return 0;
}

/*
*
*/
int whnat_hif_exit(struct whnat_hif_cfg *hif)
{
	if (hif) {
#ifdef WED_CR_MIRROR_SUPPORT
		iounmap((void *)hif->base_addr);
#endif /*WED_CR_MIRROR_SUPPORT*/
#ifdef WHNAT_CPU_TRACER
		{
			struct whnat_cputracer *tracer = &hif->tracer;

			WHNAT_IO_WRITE32(tracer, CPU_TRACER_CFG, 0);
			iounmap((void *)tracer->base_addr);
		}
#endif
		memset(hif, 0, sizeof(*hif));
	}

	return 0;
}

