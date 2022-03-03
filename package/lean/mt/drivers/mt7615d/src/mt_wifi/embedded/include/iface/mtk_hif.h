/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __MTK_HIF_H_
#define __MTK_HIF_H_

struct mtk_hif_dev {
	char priv[0] __aligned(NETDEV_ALIGN);
};

struct mtk_hif {
	const struct mtk_hif_ops *ops;
	struct mtk_hif_dev hif_dev __aligned(NETDEV_ALIGN);
};

/**
 * @write_tx_resource:HIF resource arrangement for MSDU TX packet transfer.
 * @write_multi_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer.
 * @write_final_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer for total bytes
 *	 update in tx hw header.
 * @kickout_data_tx: TX data packet kick-out to HW to start transfer.
 * @get_pkt_from_rx_resource: get RX packet from HIF resource.
 * @get_tx_resource_free_num: get tx data HIF resource available numbers
 * @get_mgmt_resource_free_num: get management HIF resource available numbers
 * @get_bcn_resource_free_num: get beacon HIF resource available numbers
 * @get_cmd_resource_free_num: get command HIF resource available numbers
 * @get_fw_loading_resource_free_num: get fw loading HIF resource available numbers
 * @is_tx_resource_empty: check if tx data HIF resource is empty or not
 * @is_rx_resource_full: check if rx HIF resource is full or not
 * @get_rx_resource_pending_num: get rx HIF resource pending numbers
 *
 */
struct mtk_hif_ops {
	u16 (*write_tx_resource)(struct mtk_hif_dev *hif_dev, struct _TX_BLK *tx_blk,
							 bool is_last, u16 *free_num);
	u16 (*write_multi_tx_resource)(struct mtk_hif_dev *hif_dev, struct _TX_BLK *tx_blk,
								   u8 frame_num, u16 *free_num);
	void (*write_final_tx_resource)(struct mtk_hif_dev *hif_dev, struct _TX_BLK *tx_blk,
									u16 total_mpdu_size, u16 first_tx_idx);
	u16 (*write_frag_tx_resource)(struct mtk_hif_dev *hif_dev, struct _TX_BLK *tx_blk,
								  u8 frag_num, u16 *free_num);
	void (*kickout_data_tx)(struct mtk_hif_dev *hif_dev, struct _TX_BLK *tx_blk, UCHAR que_idx);

	PNDIS_PACKET(*get_pkt_from_rx_resource)(struct mtk_hif_dev *hif_dev, bool *re_schedule, unsigned int *rx_pending, u8 ring_no);
	UINT32(*get_tx_resource_free_num)(struct mtk_hif_dev *hif_dev, UINT8 que_idx);
	u8 *src_va, unsigned int src_buf_len);
};

#endif
