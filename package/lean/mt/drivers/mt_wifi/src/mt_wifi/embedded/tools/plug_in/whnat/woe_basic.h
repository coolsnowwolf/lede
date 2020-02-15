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

	Module Name: wifi_offload
	whnat_basic.h
*/

#ifndef _WHNAT_BASIC_H_
#define _WHNAT_BASIC_H_

#include <linux/list.h>
#include <linux/file.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>

#include "woe_wifi.h"
#include "woe_hw.h"

/*configuration*/
/*Label 0*/
#define CFG_INTER_AGENT_SUPPORT 1

/*Label 1*/
#define CFG_TX_SUPPORT			1
#define CFG_RX_SUPPORT			1

/*Label 2*/
#define CFG_CR_MIRROR_SUPPORT	1
#define CFG_DELAY_INT_SUPPORT	1

/*Label 3*/
#define CFG_HW_TX_SUPPORT		1

/*Label 4*/
#define CFG_DYNAMIC_BM_SUPPORT	0
#define CFG_WDMA_RECYCLE		0

/*should remove when feature is ready or fix*/
#define CFG_WORK_AROUND_128_ALIGN	1
/*should not enable since hw fixed*/
#define CFG_WORK_AROUND_INT_POLL	0
/*should not enable since hw fixed*/
#define CFG_WORK_AROUND_WDMA_RETURN_IDLE 0

/*workaround*/
#if CFG_WORK_AROUND_128_ALIGN
#define WED_WORK_AROUND_128_ALIGN
#endif

#if CFG_WORK_AROUND_INT_POLL
#define WED_WORK_AROUND_INT_POLL
#endif

#if CFG_WORK_AROUND_WDMA_RETURN_IDLE
#define WED_WORK_AROUND_WDMA_RETURN_IDLE
#endif

/*compiler flag rule*/
#if CFG_INTER_AGENT_SUPPORT
#define WED_INTER_AGENT_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT)
#define WED_TX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_RX_SUPPORT)
#define WED_RX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_CR_MIRROR_SUPPORT)
#define WED_CR_MIRROR_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_DELAY_INT_SUPPORT)
#define WED_DELAY_INT_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_HW_TX_SUPPORT)
#define WED_HW_TX_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_HW_TX_SUPPORT && CFG_DYNAMIC_BM_SUPPORT)
#define WED_DYNAMIC_BM_SUPPORT
#endif

#if (CFG_INTER_AGENT_SUPPORT && CFG_TX_SUPPORT && CFG_RX_SUPPORT && CFG_HW_TX_SUPPORT && CFG_WDMA_RECYCLE)
#define WED_WDMA_RECYCLE
#endif

#define MAX_NAME_SIZE 64

enum {
	WHNAT_DBG_START,
	WHNAT_DBG_OFF = WHNAT_DBG_START,
	WHNAT_DBG_ERR,
	WHNAT_DBG_INF,
	WHNAT_DBG_LOU,
	WHNAT_DBG_END
};


enum {
	WHNAT_CR_START = 0,
	WHNAT_CR_WED = WHNAT_CR_START,
	WHNAT_CR_WDMA,
	WHNAT_CR_WIFI,
	WHNAT_CR_END
};




/*platform register Read/Write*/
#define WHNAT_IO_WRITE32(_A, _R, _V)								\
	writel(_V, (void *)((_A)->base_addr + (_R)))

#define WHNAT_IO_READ32(_A, _R, _pV)								\
	(*(_pV) = readl((void *)((_A)->base_addr + (_R))))



#define WHNAT_DBG_EN 1
/*debug macro*/
#if WHNAT_DBG_EN
#define WHNAT_DBG(lev, fmt, args...) \
	{if (lev <= whnat_log_get())	pr_info(fmt, ## args); }
#else
#define WHNAT_DBG(lev, fmt, args...)
#endif




struct whnat_dma_buf {
	unsigned long alloc_size;
	void *alloc_va;
	struct list_head list;
	dma_addr_t alloc_pa;
};

struct whnat_dma_cb {
	unsigned long alloc_size;
	void *alloc_va;
	dma_addr_t alloc_pa;
	struct sk_buff *pkt;
	dma_addr_t pkt_pa;
	unsigned short token_id;
};


struct whnat_ring {
	struct whnat_dma_cb cell[WIFI_TX_RING_SIZE];
	struct whnat_dma_buf desc;
	unsigned int hw_desc_base;
	unsigned int hw_cidx_addr;
	unsigned int hw_didx_addr;
	unsigned int hw_cnt_addr;
};

char whnat_log_get(void);
void whnat_log_set(char log);

void whnat_dump_dmabuf(struct whnat_dma_buf *buf);
void whnat_dump_dmacb(struct whnat_dma_cb *cb);

int whnat_dma_buf_alloc(struct platform_device *dev, struct whnat_dma_buf *dma_buf, UINT size);
void whnat_dma_buf_free(struct platform_device *dev, struct whnat_dma_buf *dma_buf);

void whnat_dump_raw(char *str, unsigned char *va, unsigned int size);
long whnat_str_tol(const char *str, char **endptr, int base);
#endif /*_WHNAT_BASIC_H_*/
