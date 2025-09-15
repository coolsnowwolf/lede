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
	woe_mt7615.h
*/


#ifndef _WOE_MT7615_H_
#define _WOE_MT7615_H_

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <os/rt_linux_txrx_hook.h>

extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
#ifdef MULTI_INF_SUPPORT
/*EXPORT symbol from wifi drvier*/
extern int multi_inf_get_idx(VOID *pAd);
#endif /*MULTI_INF_SUPPORT*/

#define WIFI_RING_OFFSET		0x10
#define WIFI_TX_RING_SIZE		(1024)
#define WIFI_PDMA_TXD_SIZE		(TXD_SIZE)
#define WIFI_TX_TOKEN_CNT		(DEFAUT_WHNAT_PKT_TX_TOKEN_ID_MAX+1)
#define WIFI_TX_1ST_BUF_SIZE	128
#define WIFI_RX1_RING_SIZE		(512)
#define WIFI_TX_BUF_SIZE		1900
#define WIFI_IMR_VAL			0xfff000e3

/*CR usage remapping*/
#define WIFI_TX_RING0_BASE	MT_WPDMA_TX_RING0_CTRL0
#define WIFI_TX_RING0_CNT	MT_WPDMA_TX_RING0_CTRL1
#define WIFI_TX_RING0_CIDX	MT_WPDMA_TX_RING0_CTRL2
#define WIFI_TX_RING0_DIDX	MT_WPDMA_TX_RING0_CTRL3
#define WIFI_TX_RING1_BASE	MT_WPDMA_TX_RING1_CTRL0
#define WIFI_TX_RING1_CNT	MT_WPDMA_TX_RING1_CTRL1
#define WIFI_TX_RING1_CIDX	MT_WPDMA_TX_RING1_CTRL2
#define WIFI_TX_RING1_DIDX	MT_WPDMA_TX_RING1_CTRL3

#define WIFI_RX_RING1_BASE	MT_WPDMA_RX_RING1_CTRL0
#define WIFI_RX_RING1_CNT	MT_WPDMA_RX_RING1_CTRL1
#define WIFI_RX_RING1_CIDX	MT_WPDMA_RX_RING1_CTRL2
#define WIFI_RX_RING1_DIDX	MT_WPDMA_RX_RING1_CTRL3

#define WIFI_INT_STA		MT_INT_SOURCE_CSR
#define WIFI_INT_MSK		MT_INT_MASK_CSR
#define WIFI_WPDMA_GLO_CFG	MT_WPDMA_GLO_CFG
#define WIFI_WPDMA_GLO_CFG_FLD_TX_DMA_EN                    (0)
#define WIFI_WPDMA_GLO_CFG_FLD_TX_DMA_BUSY                  (1)
#define WIFI_WPDMA_GLO_CFG_FLD_RX_DMA_EN                    (2)
#define WIFI_WPDMA_GLO_CFG_FLD_RX_DMA_BUSY                  (3)
#define WIFI_WPDMA_RESET_PTR WPDMA_RST_PTR
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DRX_IDX1				(17)
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DTX_IDX0				(0)
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DTX_IDX1				(1)


#ifdef ERR_RECOVERY
#define WIFI_MCU_INT_EVENT MT_MCU_INT_EVENT
#define WIFI_ERR_RECOV_STOP_IDLE	ERR_RECOV_STOP_IDLE
#define WIFI_ERR_RECOV_STOP_PDMA0	ERR_RECOV_STOP_PDMA0
#define WIFI_ERR_RECOV_RESET_PDMA0	ERR_RECOV_RESET_PDMA0
#define WIFI_ERR_RECOV_STOP_IDLE_DONE ERR_RECOV_STOP_IDLE_DONE
#define WIFI_TRIGGER_SER			MCU_INT_SER_TRIGGER_FROM_HOST
#endif

#define WIFI_TXD_INIT(_txd) (((struct _TXD_STRUC *) _txd)->DMADONE = DMADONE_DONE)


#endif /*_WOE_MT7615_H_*/
