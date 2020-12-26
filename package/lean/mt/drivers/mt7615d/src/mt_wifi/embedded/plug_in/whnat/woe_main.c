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
	whnat.c
*/

/*Main module of WHNAT, use to for register hook function and probe for wifi chip*/

#include <linux/init.h>
#include <linux/module.h>

#include "woe.h"
#include "wed_def.h"


/*
 *   Defination
*/
#define DRIVER_DESC "Register for MTK WIFI-to-Eth Offload Engain "
#define WHNAT_PLATFORM_DEV_NAME "whnat_dev"

static struct whnat_ctrl sys_wc;

/*
*
*/

static const struct of_device_id whnat_of_ids[] = {
	{	.compatible = "mediatek,wed", },
	{	.compatible = "mediatek,wed2", },
	{ },
};

/*whnat support list write here*/
static const unsigned int whnat_support_list[] = {
	0x7615,
	0
};


/*local function*/
/*
*
*/
static unsigned char whnat_cap_support(void *cookie)
{
	unsigned int i = 0;
	unsigned int chip_id;

	chip_id = wifi_chip_id_get(cookie);

	while (whnat_support_list[i] != 0) {
		if (chip_id == whnat_support_list[i]) {
			WHNAT_DBG(WHNAT_DBG_OFF, "%s(): chip_id=%x is in WHNAT support list\n", __func__, chip_id);
			return 1;
		}

		i++;
	}

	return 0;
}

/*
*
*/
static struct whnat_entry *whnat_entry_acquire(void *cookie)
{
	int i;
	unsigned int slot_id;
	struct whnat_ctrl *wc = whnat_ctrl_get();
	struct whnat_entry *entry = NULL;
	/*search by slot*/
	slot_id = wifi_slot_get(cookie);

	/*check is acquired or not*/
	for (i = 0; i < wc->whnat_num; i++) {
		entry = &wc->entry[i];
		if (entry->slot_id == slot_id) {
			entry->wifi.cookie = cookie;
			WHNAT_DBG(WHNAT_DBG_OFF,
				"%s(): PCIE SLOT:%d, hook to WHNAT,entry id=%d\n", __func__, i, entry->idx);
			return entry;
		}
	}

	return NULL;
}

/*
*
*/
static void whnat_entry_release(struct whnat_entry *whnat)
{
	whnat->wifi.cookie = NULL;
	memset(&whnat->pdriver, 0, sizeof(whnat->pdriver));
}

/*
*
*/
static struct whnat_entry *whnat_entry_acquire_for_pdev(void *pdev)
{
	unsigned char i;
	struct whnat_ctrl *wc = whnat_ctrl_get();
	struct whnat_entry *entry;

	for (i = 0; i < wc->whnat_num; i++) {
		entry = &wc->entry[i];

		if (entry->pdev == NULL && entry->pdriver.probe != NULL) {
			entry->pdev = pdev;
			return &wc->entry[i];
		}
	}

	return NULL;
}

/*
*
*/
static struct whnat_entry *whnat_entry_get_for_pdev(void *pdev)
{
	unsigned char i;
	struct whnat_ctrl *wc = whnat_ctrl_get();
	struct whnat_entry *entry;

	for (i = 0; i < wc->whnat_num; i++) {
		entry = &wc->entry[i];

		if (entry->pdev == pdev)
			return &wc->entry[i];
	}

	return NULL;
}


/*
*
*/
static void whnat_entry_release_pdev(struct whnat_entry *whnat)
{
	whnat->pdev = NULL;
}

/*
*
*/
static int whnat_probe(struct platform_device *pdev)
{
	struct whnat_entry *whnat;
	struct wed_entry *wed;
	struct wdma_entry *wdma;
	struct wifi_entry *wifi;

	whnat = whnat_entry_acquire_for_pdev(pdev);

	if (whnat == NULL) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): probe fail since whnat is full!\n", __func__);
		return -1;
	}

	/*pre-allocate wifi information*/
	wifi = &whnat->wifi;
	wifi_cap_get(wifi);
	/*initial wed entry */
	wed = &whnat->wed;
	wed->whnat = whnat;
	wed_init(pdev, whnat->idx, wed);
	wed_entry_proc_init(whnat, wed);
	/*initial wdma entry*/
	wdma = &whnat->wdma;
	wdma->whnat = whnat;
	wdma_init(pdev, whnat->idx, wdma);
	wdma_entry_proc_init(whnat, wdma);
	/*wifi chip related setting*/
	wifi_chip_probe(wifi, wed->irq);
	/*after wifi probe should write wpdma mask to wed*/
	whnat_hal_pdma_mask_set(whnat);
	/*pcie mapping*/
	whnat_hal_pcie_map(whnat);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): platform device probe is done\n", __func__);
	return 0;
}


/*
*
*/
static int whnat_remove(struct platform_device *pdev)
{
	struct whnat_entry *whnat;
	struct wed_entry *wed;
	struct wdma_entry *wdma;
	struct wifi_entry *wifi;

	whnat = whnat_entry_get_for_pdev(pdev);

	if (whnat == NULL) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): remove faild since can't find whnat entry!\n", __func__);
		return -1;
	}

	wed = &whnat->wed;
	wdma = &whnat->wdma;
	wifi = &whnat->wifi;
	/*remove wifi related setting*/
	wifi_chip_remove(wifi);
	/*remove wdma related setting*/
	wdma_entry_proc_exit(whnat, wdma);
	wdma_exit(pdev, wdma);
	/*remove wed related setting*/
	wed_entry_proc_exit(whnat, wed);
	wed_exit(pdev, wed);
	/*reomve whnat entry from whnat_ctrl*/
	whnat_entry_release_pdev(whnat);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): whnat remove done!!\n", __func__);
	return 0;
}

/*
*
*/
static int whnat_driver_init(void *cookie)
{
	struct platform_driver *pdriver;
	struct whnat_entry *entry;
	char name[MAX_NAME_SIZE] = "";
	struct whnat_ctrl *wc = whnat_ctrl_get();

	if (!whnat_cap_support(cookie)) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): chip not support wifi hardware nat feature\n", __func__);
		goto err;
	}

	if (!wifi_whnat_en_get(cookie)) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): chip not enable wifi hardware nat feature!\n", __func__);
		goto err;
	}

	entry = whnat_entry_acquire(cookie);

	if (!entry) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): can't acquire a new whnat entry since full! cookie=%p\n", __func__, cookie);
		goto err;
	}

	snprintf(name, sizeof(name), "%s%d", WHNAT_PLATFORM_DEV_NAME, wc->whnat_driver_idx++);
	pdriver = &entry->pdriver;
	pdriver->probe = whnat_probe;
	pdriver->remove = whnat_remove;
	pdriver->driver.name = name;
	pdriver->driver.owner = THIS_MODULE;
	pdriver->driver.of_match_table = whnat_of_ids;
	whnat_entry_proc_init(wc, entry);
	return platform_driver_register(pdriver);
err:
	wifi_whnat_en_set(cookie, FALSE);
	return -1;
}


/*
*
*/
static void whnat_driver_exit(void *cookie)
{
	struct whnat_entry *whnat;
	struct whnat_ctrl *wc = whnat_ctrl_get();

	whnat = whnat_entry_search(cookie);

	if (!whnat)
		return;

	platform_driver_unregister(&whnat->pdriver);
	whnat_entry_release(whnat);
	whnat_entry_proc_exit(wc, whnat);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): whnat entry clear done!!\n", __func__);
}

/*
*
*/
static int whnat_entry_init(struct whnat_entry *whnat, unsigned int idx)
{
	struct whnat_cfg *cfg = &whnat->cfg;

	whnat->idx = idx;
	whnat->wifi.cookie = NULL;
	whnat->wifi.base_addr = 0;
	whnat->pdev = NULL;
	whnat->slot_id = wed_slot_map_get(idx);
	memset(cfg, 0, sizeof(*cfg));
	cfg->hw_tx_en = CFG_HW_TX_SUPPORT;
	cfg->cr_mirror_en = CFG_CR_MIRROR_SUPPORT;
	return 0;
}

/*
*
*/
static void whnat_entry_exit(struct whnat_entry *whnat)
{
	if (whnat->wifi.cookie)
		whnat_driver_exit(whnat->wifi.cookie);
}


/*
*
*/
static int whnat_ctrl_init(struct whnat_ctrl *wc)
{
	unsigned char num = wed_num_get();
	unsigned char i;
	unsigned int size;

	wc->whnat_num = num;
	wc->whnat_driver_idx = 0;
	size = sizeof(struct whnat_entry)*num;
	wc->entry = kmalloc(size, GFP_KERNEL);
	memset(wc->entry, 0, size);

	if (!wc->entry) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate whnat control entry faild!\n", __func__);
		return -1;
	}

	memset(wc->entry, 0, size);

	for (i = 0; i < num; i++)
		whnat_entry_init(&wc->entry[i], i);

	return 0;
}


/*
*
*/
static void whnat_ctrl_exit(struct whnat_ctrl *wc)
{
	unsigned char num = wed_num_get();
	unsigned char i;

	wc->whnat_num = num;

	for (i = 0; i < num; i++)
		whnat_entry_exit(&wc->entry[i]);

	kfree(wc->entry);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): whnat entry free done!!\n", __func__);
}

/*hook handler*/
/*
*
*/
static int whnat_ring_init(void *cookie)
{
	struct whnat_entry *whnat;

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): cookie=%p\n", __func__, cookie);
	whnat = whnat_entry_search(cookie);

	if (!whnat) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): whnat can't find for cookie:%p\n", __func__, cookie);
		return -1;
	}

	WHNAT_DBG(WHNAT_DBG_INF, "%s(): set whnat %p hw setting,idx=%d\n", __func__, whnat, whnat->idx);
	/*initial wed*/
	whnat_hal_wed_init(whnat);
	/*initial wdma*/
	whnat_hal_wdma_init(whnat);
	/*hw setting for wdma enable*/
	whnat_hal_wdma_ring_init(whnat);
	/*hw setting for wed enable*/
	whnat_hal_wpdma_ring_init(whnat);
	/*enable wed interrupt*/
	whnat_hal_int_ctrl(whnat, TRUE);
	/*enable wed ext interrupt*/
	whnat_hal_eint_init(whnat);
	whnat_hal_eint_ctrl(whnat, TRUE);

	/*enable HW CR mirror or not*/
	if (whnat->cfg.cr_mirror_en) {
		wifi_chip_cr_mirror_set(&whnat->wifi, TRUE);
		whnat_hal_cr_mirror_set(whnat, TRUE);
	}

	return 0;
}


/*
*
*/
static void whnat_ring_exit(void *cookie)
{
	struct whnat_entry *whnat;

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): cookie=%p\n", __func__, cookie);
	whnat = whnat_entry_search(cookie);

	if (!whnat) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): whnat can't find for cookie:%p\n", __func__, cookie);
		return;
	}

	if (whnat->cfg.cr_mirror_en) {
		wifi_chip_cr_mirror_set(&whnat->wifi, FALSE);
		whnat_hal_cr_mirror_set(whnat, FALSE);
	}

	/*flush all hw path*/
	wifi_tx_tuple_reset();
	/*Reset Ring and HW setting*/
	whnat_hal_dma_ctrl(whnat, WHNAT_DMA_DISABLE);
	whnat_hal_eint_ctrl(whnat, FALSE);
	whnat_hal_int_ctrl(whnat, FALSE);
	whnat_hal_hw_reset(whnat, WHNAT_RESET_ALL);
}

/*
*
*/
static void whnat_wlan_tx(void *cookie, UCHAR *tx_info)
{
	struct whnat_entry *whnat;

	WHNAT_DBG(WHNAT_DBG_INF, "%s(): cookie=%p\n", __func__, cookie);
	whnat = whnat_entry_search(cookie);

	if (!whnat)
		return;

	if (wifi_hw_tx_allow(cookie,tx_info) == FALSE)
		return;

#ifdef WED_HW_TX_SUPPORT
	wifi_tx_tuple_add(whnat, tx_info);
#endif /*WED_HW_TX_SUPPORT*/
}


/*
*
*/
static void whnat_isr_handler(void *cookie)
{
	struct whnat_entry *entry;
	unsigned int status = 0;

	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): cookie=%p\n", __func__, cookie);
	entry = whnat_entry_search(cookie);

	if (!entry)
		return;

	whnat_hal_eint_ctrl(entry, FALSE);
	whnat_hal_eint_stat_get(entry, &status);
	wed_eint_handle(&entry->wed, status);
	whnat_hal_eint_ctrl(entry, TRUE);
}

/*
*
*/
static void whnat_dma_handler(void *cookie, int *dma_info)
{
	struct whnat_entry *entry;
	unsigned char dma_cfg;

	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): cookie=%p\n", __func__, cookie);
	entry = whnat_entry_search(cookie);

	if (!entry)
		return;

	wifi_dma_cfg_wrapper(*dma_info, &dma_cfg);
	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): Set DMA =%d\n", __func__, dma_cfg);
	whnat_hal_dma_ctrl(entry, dma_cfg);
}

/*
*
*/
static void whnat_ser_handler(void *cookie, void *ser_ctrl)
{
#ifdef ERR_RECOVERY
	struct whnat_entry *entry;

	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): cookie=%p\n", __func__, cookie);
	entry = whnat_entry_search(cookie);

	if (!entry)
		return;

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): status=%d\n", __func__, wifi_ser_status(ser_ctrl));

	switch (wifi_ser_status(ser_ctrl)) {
	case WIFI_ERR_RECOV_STOP_IDLE: {
		if (entry->cfg.cr_mirror_en) {
			wifi_chip_cr_mirror_set(&entry->wifi, FALSE);
			whnat_hal_cr_mirror_set(entry, FALSE);
		}

		whnat_hal_eint_ctrl(entry, FALSE);
		whnat_hal_int_ctrl(entry, FALSE);
	}
	break;

	case WIFI_ERR_RECOV_STOP_IDLE_DONE: {
		whnat_hal_int_ctrl(entry, TRUE);
	}
	break;

	case WIFI_ERR_RECOV_STOP_PDMA0: {
		/*flush all hw path*/
		wifi_tx_tuple_reset();
		wed_ring_reset(&entry->wed);

		/*Reset TX path first*/
		if (whnat_hal_hw_reset(entry, WHNAT_RESET_IDX_ONLY) < 0) {
			WHNAT_DBG(WHNAT_DBG_INF, "%s(): Reset index faild, change to reset module!\n", __func__);
			whnat_hal_hw_reset(entry, WHNAT_RESET_IDX_MODULE);
		}
	}
	break;

	case WIFI_ERR_RECOV_RESET_PDMA0:
	default:
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): status=%d, do nothing\n", __func__, wifi_ser_status(ser_ctrl));
		break;
	}

#endif /*ERR_RECOVERY*/
}
/*
*
*/
static unsigned int whnat_handle(unsigned short hook, void *ad, void *priv)
{
	switch (hook) {
	case WLAN_HOOK_HIF_INIT:
		whnat_ring_init(ad);
		break;

	case WLAN_HOOK_HIF_EXIT:
		whnat_ring_exit(ad);
		break;

	case WLAN_HOOK_TX:
		whnat_wlan_tx(ad, priv);
		break;

	case WLAN_HOOK_SYS_UP:
		whnat_driver_init(ad);
		break;

	case WLAN_HOOK_SYS_DOWN:
		whnat_driver_exit(ad);
		break;

	case WLAN_HOOK_ISR:
		whnat_isr_handler(ad);
		break;

	case WLAN_HOOK_DMA_SET:
		whnat_dma_handler(ad, priv);
		break;

	case WLAN_HOOK_SER:
		whnat_ser_handler(ad, priv);
		break;

	default:
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): can't find whnat handle (hook: %u)\n", __func__, hook);
		break;
	}

	return 0;
}

/*global function*/
/*
*
*/
void whnat_proc_handle(struct whnat_entry *entry)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "Idx\t: %d\n", entry->idx);
	WHNAT_DBG(WHNAT_DBG_OFF, "Cookie\t: %p\n", entry->wifi.cookie);
	WHNAT_DBG(WHNAT_DBG_OFF, "WIFI base\t: %lx\n", entry->wifi.base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "Pdev\t: %p\n", entry->pdev);
	WHNAT_DBG(WHNAT_DBG_OFF, "Pdriver\t: %p\n", &entry->pdriver);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma\t: %p\n", &entry->wdma);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed\t: %p\n", &entry->wed);
	WHNAT_DBG(WHNAT_DBG_OFF, "log_evel\t: %d\n", whnat_log_get());
}

/*
*
*/
void whnat_dump_cfg(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;
	struct wifi_entry *wifi = &whnat->wifi;
	struct wdma_entry *wdma = &whnat->wdma;

	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED basic info:==========\n");
	dump_wed_value(wed, "WED_REV", WED_REV);
	dump_wed_value(wed, "WED_CTRL", WED_CTRL);
	dump_wed_value(wed, "WED_CTRL2", WED_CTRL2);
	dump_wed_value(wed, "WED_EX_INT_STA", WED_EX_INT_STA);
	dump_wed_value(wed, "WED_EX_INT_MSK", WED_EX_INT_MSK);
	dump_wed_value(wed, "WED_ST", WED_ST);
	dump_wed_value(wed, "WED_GLO_CFG", WED_GLO_CFG);
	dump_wed_value(wed, "WED_INT_STA", WED_INT_STA);
	dump_wed_value(wed, "WED_INT_MSK", WED_INT_MSK);
	dump_wed_value(wed, "WED_AXI_CTRL", WED_AXI_CTRL);
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED TX buf info:==========\n");
	dump_wed_value(wed, "WED_BM_ST", WED_BM_ST);
	dump_wed_value(wed, "WED_TX_BM_BASE", WED_TX_BM_BASE);
	dump_wed_value(wed, "WED_TX_BM_CTRL", WED_TX_BM_CTRL);
	dump_wed_value(wed, "WED_TX_BM_TKID", WED_TX_BM_TKID);
	dump_wed_value(wed, "WED_TX_BM_STS", WED_TX_BM_STS);
	dump_wed_value(wed, "WED_TX_BM_DYN_TH", WED_TX_BM_DYN_TH);
	dump_wed_value(wed, "WED_TX_BM_INTF", WED_TX_BM_INTF);
	dump_wed_value(wed, "WED_TX_BM_RECYC", WED_TX_BM_RECYC);
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED PCI Host Control:==========\n");
	dump_wed_value(wed, "WED_PCIE_CFG_BASE", WED_PCIE_CFG_BASE);
	dump_wed_value(wed, "WED_PCIE_OFST", WED_PCIE_OFST);
	dump_wed_value(wed, "WED_PCIE_INTS_TRIG", WED_PCIE_INTS_TRIG);
	dump_wed_value(wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED_WPDMA basic info:==========\n");
	dump_wed_value(wed, "WED_WPDMA_ST", WED_WPDMA_ST);
	dump_wed_value(wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC);
	dump_wed_value(wed, "WED_WPDMA_GLO_CFG", WED_WPDMA_GLO_CFG);
	dump_wed_value(wed, "WED_WPDMA_CFG_BASE", WED_WPDMA_CFG_BASE);
	dump_wed_value(wed, "WED_WPDMA_OFST0", WED_WPDMA_OFST0);
	dump_wed_value(wed, "WED_WPDMA_OFST1", WED_WPDMA_OFST1);
	dump_wed_value(wed, "WED_WPDAM_CTRL", WED_WPDAM_CTRL);
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED_WDMA basic info:==========\n");
	dump_wed_value(wed, "WED_WDMA_ST", WED_WDMA_ST);
	dump_wed_value(wed, "WED_WDMA_INFO", WED_WDMA_INFO);
	dump_wed_value(wed, "WED_WDMA_GLO_CFG", WED_WDMA_GLO_CFG);
	dump_wed_value(wed, "WED_WDMA_RST_IDX", WED_WDMA_RST_IDX);
	dump_wed_value(wed, "WED_WDMA_LOAD_DRV_IDX", WED_WDMA_LOAD_DRV_IDX);
	dump_wed_value(wed, "WED_WDMA_LOAD_CRX_IDX", WED_WDMA_LOAD_CRX_IDX);
	dump_wed_value(wed, "WED_WDMA_SPR", WED_WDMA_SPR);
	dump_wed_value(wed, "WED_WDMA_INT_STA_REC", WED_WDMA_INT_STA_REC);
	dump_wed_value(wed, "WED_WDMA_INT_TRIG", WED_WDMA_INT_TRIG);
	dump_wed_value(wed, "WED_WDMA_INT_CTRL", WED_WDMA_INT_CTRL);
	dump_wed_value(wed, "WED_WDMA_INT_CLR", WED_WDMA_INT_CLR);
	dump_wed_value(wed, "WED_WDMA_CFG_BASE", WED_WDMA_CFG_BASE);
	dump_wed_value(wed, "WED_WDMA_OFST0", WED_WDMA_OFST0);
	dump_wed_value(wed, "WED_WDMA_OFST1", WED_WDMA_OFST1);
	/*other part setting*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WDMA basic info:==========\n");
	dump_wdma_value(wdma, "WDMA_GLO_CFG", WDMA_GLO_CFG);
	dump_wdma_value(wdma, "WDMA_INT_MSK", WDMA_INT_MSK);
	dump_wdma_value(wdma, "WDMA_INT_STA", WDMA_INT_STA_REC);
	dump_wdma_value(wdma, "WDMA_INFO", WDMA_INFO);
	dump_wdma_value(wdma, "WDMA_DELAY_INT_CFG", WDMA_DELAY_INT_CFG);
	dump_wdma_value(wdma, "WDMA_FREEQ_THRES", WDMA_FREEQ_THRES);
	dump_wdma_value(wdma, "WDMA_INT_STS_GRP0", WDMA_INT_STS_GRP0);
	dump_wdma_value(wdma, "WDMA_INT_STS_GRP1", WDMA_INT_STS_GRP1);
	dump_wdma_value(wdma, "WDMA_INT_STS_GRP2", WDMA_INT_STS_GRP2);
	dump_wdma_value(wdma, "WDMA_INT_GRP1", WDMA_INT_GRP1);
	dump_wdma_value(wdma, "WDMA_INT_GRP2", WDMA_INT_GRP2);
	dump_wdma_value(wdma, "WDMA_SCH_Q01_CFG", WDMA_SCH_Q01_CFG);
	dump_wdma_value(wdma, "WDMA_SCH_Q23_CFG", WDMA_SCH_Q23_CFG);
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WPDMA basic info:==========\n");
	dump_wifi_value(wifi, "WPDMA_GLO_CFG", WIFI_WPDMA_GLO_CFG);
	dump_wifi_value(wifi, "WPDMA_INT_MSK", WIFI_INT_MSK);
	dump_wifi_value(wifi, "WPDMA_INT_STA", WIFI_INT_STA);
}

/*
*
*/
void whnat_dump_txinfo(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;
	struct wifi_entry *wifi = &whnat->wifi;
	struct wdma_entry *wdma = &whnat->wdma;

	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED TX ring info:==========\n");
	dump_wed_value(wed, "WED_TX0_MIB", WED_TX0_MIB);
	dump_wed_value(wed, "WED_TX1_MIB", WED_TX1_MIB);
	dump_wed_value(wed, "WED_TX0_BASE", WED_TX0_CTRL0);
	dump_wed_value(wed, "WED_TX0_CNT", WED_TX0_CTRL1);
	dump_wed_value(wed, "WED_TX0_CIDX", WED_TX0_CTRL2);
	dump_wed_value(wed, "WED_TX0_DIDX", WED_TX0_CTRL3);
	dump_wed_value(wed, "WED_TX1_BASE", WED_TX1_CTRL0);
	dump_wed_value(wed, "WED_TX1_CNT", WED_TX1_CTRL1);
	dump_wed_value(wed, "WED_TX1_CIDX", WED_TX1_CTRL2);
	dump_wed_value(wed, "WED_TX1_DIDX", WED_TX1_CTRL3);
	/*WPDMA status from WED*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED WPDMA TX ring info:==========\n");
	dump_wed_value(wed, "WED_WPDMA_TX0_MIB", WED_WPDMA_TX0_MIB);
	dump_wed_value(wed, "WED_WPDMA_TX0_BASE", WED_WPDMA_TX0_CTRL0);
	dump_wed_value(wed, "WED_WPDMA_TX0_CNT", WED_WPDMA_TX0_CTRL1);
	dump_wed_value(wed, "WED_WPDMA_TX0_CIDX", WED_WPDMA_TX0_CTRL2);
	dump_wed_value(wed, "WED_WPDMA_TX0_DIDX", WED_WPDMA_TX0_CTRL3);
	dump_wed_value(wed, "WED_WPDMA_TX0_COHERENT_MIB", WED_WPDMA_TX0_COHERENT_MIB);
	dump_wed_value(wed, "WED_WPDMA_TX1_MIB", WED_WPDMA_TX1_MIB);
	dump_wed_value(wed, "WED_WPDMA_TX1_BASE", WED_WPDMA_TX1_CTRL0);
	dump_wed_value(wed, "WED_WPDMA_TX1_CNT", WED_WPDMA_TX1_CTRL1);
	dump_wed_value(wed, "WED_WPDMA_TX1_CIDX", WED_WPDMA_TX1_CTRL2);
	dump_wed_value(wed, "WED_WPDMA_TX1_DIDX", WED_WPDMA_TX1_CTRL3);
	dump_wed_value(wed, "WED_WPDMA_TX1_COHERENT_MIB", WED_WPDMA_TX1_COHERENT_MIB);
	/*WPDMA*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WPDMA TX ring info:==========\n");
	dump_wifi_value(wifi, "WPDMA_TX0_BASE", WIFI_TX_RING0_BASE);
	dump_wifi_value(wifi, "WPDMA_TX0_CNT", WIFI_TX_RING0_CNT);
	dump_wifi_value(wifi, "WPDMA_TX0_CRX_IDX", WIFI_TX_RING0_CIDX);
	dump_wifi_value(wifi, "WPDMA_TX0_DRX_IDX", WIFI_TX_RING0_DIDX);
	dump_wifi_value(wifi, "WPDMA_TX1_BASE", WIFI_TX_RING1_BASE);
	dump_wifi_value(wifi, "WPDMA_TX1_CNT", WIFI_TX_RING1_CNT);
	dump_wifi_value(wifi, "WPDMA_TX1_CRX_IDX", WIFI_TX_RING1_CIDX);
	dump_wifi_value(wifi, "WPDMA_TX1_DRX_IDX", WIFI_TX_RING1_DIDX);
	/*WDMA status from WED*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED WDMA RX ring info:==========\n");
	dump_wed_value(wed, "WED_WDMA_RX0_MIB", WED_WDMA_RX0_MIB);
	dump_wed_value(wed, "WED_WDMA_RX0_BASE", WED_WDMA_RX0_BASE);
	dump_wed_value(wed, "WED_WDMA_RX0_CNT", WED_WDMA_RX0_CNT);
	dump_wed_value(wed, "WED_WDMA_RX0_CRX_IDX", WED_WDMA_RX0_CRX_IDX);
	dump_wed_value(wed, "WED_WDMA_RX0_DRX_IDX", WED_WDMA_RX0_DRX_IDX);
	dump_wed_value(wed, "WED_WDMA_RX0_THRES_CFG", WED_WDMA_RX0_THRES_CFG);
	dump_wed_value(wed, "WED_WDMA_RX0_RECYCLE_MIB", WED_WDMA_RX0_RECYCLE_MIB);
	dump_wed_value(wed, "WED_WDMA_RX0_PROCESSED_MIB", WED_WDMA_RX0_PROCESSED_MIB);
	dump_wed_value(wed, "WED_WDMA_RX1_MIB", WED_WDMA_RX1_MIB);
	dump_wed_value(wed, "WED_WDMA_RX1_BASE", WED_WDMA_RX1_BASE);
	dump_wed_value(wed, "WED_WDMA_RX1_CNT", WED_WDMA_RX1_CNT);
	dump_wed_value(wed, "WED_WDMA_RX1_CRX_IDX", WED_WDMA_RX1_CRX_IDX);
	dump_wed_value(wed, "WED_WDMA_RX1_DRX_IDX", WED_WDMA_RX1_DRX_IDX);
	dump_wed_value(wed, "WED_WDMA_RX1_THRES_CFG", WED_WDMA_RX1_THRES_CFG);
	dump_wed_value(wed, "WED_WDMA_RX1_RECYCLE_MIB", WED_WDMA_RX1_RECYCLE_MIB);
	dump_wed_value(wed, "WED_WDMA_RX1_PROCESSED_MIB", WED_WDMA_RX1_PROCESSED_MIB);
	/*WDMA*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED WDMA RX ring info:==========\n");
	dump_wdma_value(wdma, "WDMA_RX_BASE_PTR_0", WDMA_RX_BASE_PTR_0);
	dump_wdma_value(wdma, "WDMA_RX_MAX_CNT_0", WDMA_RX_MAX_CNT_0);
	dump_wdma_value(wdma, "WDMA_RX_CRX_IDX_0", WDMA_RX_CRX_IDX_0);
	dump_wdma_value(wdma, "WDMA_RX_DRX_IDX_0", WDMA_RX_DRX_IDX_0);
	dump_wdma_value(wdma, "WDMA_RX_BASE_PTR_1", WDMA_RX_BASE_PTR_1);
	dump_wdma_value(wdma, "WDMA_RX_MAX_CNT_1", WDMA_RX_MAX_CNT_1);
	dump_wdma_value(wdma, "WDMA_RX_CRX_IDX_1", WDMA_RX_CRX_IDX_1);
	dump_wdma_value(wdma, "WDMA_RX_DRX_IDX_1", WDMA_RX_DRX_IDX_1);
}

/*
*
*/
void whnat_dump_rxinfo(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;
	struct wifi_entry *wifi = &whnat->wifi;

	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED RX ring info:==========\n");
	dump_wed_value(wed, "WED_RX1_MIB", WED_RX1_MIB);
	dump_wed_value(wed, "WED_RX1_BASE", WED_RX1_CTRL0);
	dump_wed_value(wed, "WED_RX1_CNT", WED_RX1_CTRL1);
	dump_wed_value(wed, "WED_RX1_CIDX", WED_RX1_CTRL2);
	dump_wed_value(wed, "WED_RX1_DIDX", WED_RX1_CTRL3);
	/*WPDMA status from WED*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WPDMA RX ring info:==========\n");
	dump_wed_value(wed, "WED_WPDMA_RX1_MIB", WED_WPDMA_RX1_MIB);
	dump_wed_value(wed, "WED_WPDMA_RX1_BASE", WED_WPDMA_RX1_CTRL0);
	dump_wed_value(wed, "WED_WPDMA_RX1_CNT", WED_WPDMA_RX1_CTRL1);
	dump_wed_value(wed, "WED_WPDMA_RX1_CIDX", WED_WPDMA_RX1_CTRL2);
	dump_wed_value(wed, "WED_WPDMA_RX1_DIDX", WED_WPDMA_RX1_CTRL3);
	dump_wed_value(wed, "WED_WPDMA_RX1_COHERENT_MIB", WED_WPDMA_RX1_COHERENT_MIB);
	/*WPDMA*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WPDMA RX ring info:==========\n");
	dump_wifi_value(wifi, "WPDMA_RX1_BASE", WIFI_RX_RING1_BASE);
	dump_wifi_value(wifi, "WPDMA_RX1_CNT", WIFI_RX_RING1_CNT);
	dump_wifi_value(wifi, "WPDMA_RX1_CRX_IDX", WIFI_RX_RING1_CIDX);
	dump_wifi_value(wifi, "WPDMA_RX1_DRX_IDX", WIFI_RX_RING1_DIDX);
}

/*
*
*/
struct whnat_entry *whnat_entry_search(void *cookie)
{
	unsigned char i;
	struct whnat_ctrl *wc = whnat_ctrl_get();

	for (i = 0; i < wc->whnat_num; i++) {

		if (wc->entry[i].wifi.cookie && wc->entry[i].wifi.cookie == cookie)
			return &wc->entry[i];
	}

	return NULL;
}

/*
* global function
*/
struct whnat_ctrl *whnat_ctrl_get(void)
{
	return &sys_wc;
}



/*
 *   Gloab variable
 */
static struct mt_wlan_hook_ops  whnat_ops = {
	.name	= "WHNAT",
	.hooks	=	(1 << WLAN_HOOK_HIF_INIT) |
	(1 << WLAN_HOOK_HIF_EXIT) |
	(1 << WLAN_HOOK_DMA_SET) |
	(1 << WLAN_HOOK_SYS_UP) |
	(1 << WLAN_HOOK_SYS_DOWN) |
	(1 << WLAN_HOOK_ISR) |
	(1 << WLAN_HOOK_TX) |
	(1 << WLAN_HOOK_SER),
	.fun	= whnat_handle,
	.priority   =  WLAN_HOOK_PRI_WOE
};


static int __init whnat_module_init(void)
{
	struct whnat_ctrl *wc = whnat_ctrl_get();

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): module init and register callback for whnat\n", __func__);
	/*initial global whnat control*/
	memset(wc, 0, sizeof(struct whnat_ctrl));
	/*register hook function*/
	mt_wlan_hook_register(&whnat_ops);
	/*register proc*/
	whnat_ctrl_proc_init(wc);
	/*initial whnat driver and check */
	whnat_ctrl_init(wc);
	/*initial pci cr mirror cfg*/
	whnat_hif_init(&wc->hif_cfg);
	return 0;
}

static void __exit whnat_module_exit(void)
{
	struct whnat_ctrl *wc = whnat_ctrl_get();

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): whnat module exist\n", __func__);
	whnat_hif_exit(&wc->hif_cfg);
	whnat_ctrl_exit(wc);
	whnat_ctrl_proc_exit(wc);
	mt_wlan_hook_unregister(&whnat_ops);
}

module_init(whnat_module_init);
module_exit(whnat_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("mt7615 within mt7622");

