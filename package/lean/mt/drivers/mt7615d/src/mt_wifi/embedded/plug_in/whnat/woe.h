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
	whnat.h
*/

#ifndef _WHNAT_H
#define _WHNAT_H

#include "wdma.h"
#include "wed.h"
#include "woe_hif.h"
#include "woe_basic.h"

struct whnat_cfg {
	char cr_mirror_en;
	char hw_tx_en;
};

struct whnat_entry {
	unsigned char idx;
	unsigned char slot_id;
	struct wifi_entry wifi;
	struct platform_driver pdriver;
	struct wdma_entry wdma;
	struct wed_entry wed;
	struct platform_dev *pdev;
	struct whnat_cfg cfg;
	void *proc;
	void *proc_stat;
	void *proc_cr;
	void *proc_cfg;
	void *proc_tx;
	void *proc_rx;
	void *proc_ctrl;
};


struct whnat_ctrl {
	unsigned char whnat_num;
	struct whnat_entry *entry;
	struct whnat_hif_cfg hif_cfg;
	unsigned int whnat_driver_idx;
	void *proc;
	void *proc_trace;
};

struct whnat_wifi_cr_map {
	unsigned int wifi_cr;
	unsigned int whnat_cr;
	char whnat_type;
};

void whnat_proc_handle(struct whnat_entry *entry);
void whnat_dump_cfg(struct whnat_entry *whnat);
void whnat_dump_txinfo(struct whnat_entry *whnat);
void whnat_dump_rxinfo(struct whnat_entry *whnat);


struct whnat_ctrl *whnat_ctrl_get(void);
struct whnat_entry *whnat_entry_search(void *cookie);


int whnat_entry_proc_init(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat);
void whnat_entry_proc_exit(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat);
int whnat_ctrl_proc_init(struct whnat_ctrl *whnat_ctrl);
void whnat_ctrl_proc_exit(struct whnat_ctrl *whnat_ctrl);
int wed_entry_proc_init(struct whnat_entry *whnat, struct wed_entry *wed);
void wed_entry_proc_exit(struct whnat_entry *whnat, struct wed_entry *wed);
int wdma_entry_proc_init(struct whnat_entry *whnat, struct wdma_entry *wdma);
void wdma_entry_proc_exit(struct whnat_entry *whnat, struct wdma_entry *wdma);





#endif /*_WHNAT_H*/
