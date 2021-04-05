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
	whnat_hw.h
*/

#ifndef _WHNAT_HW_H
#define _WHNAT_HW_H

struct whnat_entry;
struct wed_entry;
struct whnat_hif_cfg;
struct whnat_cputracer;
#ifdef ERR_RECOVERY
struct wed_ser_state;
#endif /*ERR_RECOVERY*/

#define DMADONE_DONE 1
#define WED_POLL_MAX 100


#define PCIE_INT_STA_OFFSET (1 << 16)
#define PCIE_POLL_MODE_ASSERT 1
#define PCIE_POLL_MODE_ALWAYS 2
#define FREE_CR_SIZE (sizeof(unsigned int))


enum {
	WHNAT_DMA_DISABLE,
	WHNAT_DMA_TX,
	WHNAT_DMA_RX,
	WHNAT_DMA_TXRX
};

enum {
	WHNAT_RESET_IDX_ONLY,
	WHNAT_RESET_IDX_MODULE,
	WHNAT_RESET_ALL,
};


void whnat_hal_int_ctrl(struct whnat_entry *whnat, unsigned char enable);
void whnat_hal_eint_ctrl(struct whnat_entry *whnat, unsigned char enable);
void whnat_hal_eint_init(struct whnat_entry *entry);
void whnat_hal_eint_stat_get(struct whnat_entry *whnat, unsigned int *state);
void whnat_hal_wdma_init(struct whnat_entry *whnat);
void whnat_hal_wdma_ring_init(struct whnat_entry *whnat);
void whnat_hal_wpdma_ring_init(struct whnat_entry *whnat);
void whnat_hal_dma_ctrl(struct whnat_entry *whnat, unsigned char txrx);
void whnat_hal_hif_init(struct whnat_hif_cfg *hif);
void whnat_hal_trace_set(struct whnat_cputracer *tracer);
void whnat_hal_bfm_update(struct wed_entry *wed, unsigned char reduce);
void whnat_hal_bfm_freecnt(struct wed_entry *wed, unsigned int *cnt);
int whnat_hal_tx_reset(struct wed_entry *wed, unsigned int reset_type);
int whnat_hal_rx_reset(struct wed_entry *wed, unsigned int reset_type);
void whnat_hal_cr_mirror_set(struct whnat_entry *whnat, unsigned char enable);
void whnat_hal_pcie_map(struct whnat_entry *whnat);
void whnat_hal_hw_restore(struct whnat_entry *whnat);
int whnat_hal_hw_reset(struct whnat_entry *whnat, unsigned int reset_type);
void whnat_hal_wed_init(struct whnat_entry *entry);
unsigned int whnat_wifi_cr_get(char type, unsigned int cr);
void whnat_hal_pdma_mask_set(struct whnat_entry *whnat);
#ifdef ERR_RECOVERY
void whnat_hal_ser_trigger(struct whnat_entry *whnat);
void whnat_hal_ser_update(struct wed_entry *wed, struct wed_ser_state *state);
#endif /*ERR_RECOVERY*/


int whnat_hal_io_read(
	void *cookie,
	unsigned int addr,
	unsigned int *value);

int whnat_hal_io_write(
	void *cookie,
	unsigned int addr,
	unsigned int value);


void whnat_hal_cr_handle(
	struct whnat_entry *entry,
	char type,
	char is_write,
	unsigned long addr,
	unsigned int *cr_value);

#endif
