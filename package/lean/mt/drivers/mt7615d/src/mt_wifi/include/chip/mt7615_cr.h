#ifndef __MT7615_CR_H__
#define __MT7615_CR_H__
/*
	Please don't include this header outside of per chip scope
*/

#ifdef RT_BIG_ENDIAN
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct {
		UINT32 rx_2b_offset:1;
		UINT32 clk_gate_dis:1;
		UINT32 byte_swap:1;
		UINT32 omit_tx_info:1;
		UINT32 omit_rx_info:1;
		UINT32 rsv_26:1;
		UINT32 force_tx_eof:1;
		UINT32 sw_rst:1;
		UINT32 err_det_th:8;
		UINT32 mi_depth:3;
		UINT32 fifo_little_endian:1;
		UINT32 multi_dma_en:2;
		UINT32 share_fifo_en:1;
		UINT32 Desc32BEn:1;
		UINT32 BigEndian:1;
		UINT32 EnTXWriteBackDDONE:1;
		UINT32 WPDMABurstSIZE:2;
		UINT32 RxDMABusy:1;
		UINT32 EnableRxDMA:1;
		UINT32 TxDMABusy:1;
		UINT32 EnableTxDMA:1;
	} MT7615_field;
	struct {
		UINT32 rx_2b_offset:1;
		UINT32 rsv_30:1;
		UINT32 byte_swap:1;
		UINT32 omit_tx_info:1;
		UINT32 omit_rx_info:1;
		UINT32 first_token_only:1;
		UINT32 force_tx_eof:1;
		UINT32 sw_rst:1;
		UINT32 tx_bt_size_bit21:2;
		UINT32 mi_depth_8_6:3;
		UINT32 mi_depth_5_3:3;
		UINT32 mi_depth_2_0:3;
		UINT32 fifo_little_endian:1;
		UINT32 multi_dma_en:2;
		UINT32 tx_bt_size_bit0:1;
		UINT32 dis_bt_size_align:1;
		UINT32 big_endian:1;
		UINT32 en_tx_wb_ddone:1;
		UINT32 rx_bt_size:2;
		UINT32 en_rx_dma_buzy:1;
		UINT32 en_rx_dma:1;
		UINT32 en_tx_dma_buzy:1;
		UINT32 en_tx_dma:1;
	} MT7615_E3_field;
	UINT32 word;
} WPDMA_GLO_CFG_STRUC;
#else
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct {
		UINT32 EnableTxDMA:1;
		UINT32 TxDMABusy:1;
		UINT32 EnableRxDMA:1;
		UINT32 RxDMABusy:1;
		UINT32 WPDMABurstSIZE:2;
		UINT32 EnTXWriteBackDDONE:1;
		UINT32 BigEndian:1;
		UINT32 Desc32BEn:1;
		UINT32 share_fifo_en:1;
		UINT32 multi_dma_en:2;
		UINT32 fifo_little_endian:1;
		UINT32 mi_depth:3;
		UINT32 err_det_th:8;
		UINT32 sw_rst:1;
		UINT32 force_tx_eof:1;
		UINT32 rsv_26:1;
		UINT32 omit_rx_info:1;
		UINT32 omit_tx_info:1;
		UINT32 byte_swap:1;
		UINT32 clk_gate_dis:1;
		UINT32 rx_2b_offset:1;
	} MT7615_field;
	struct {
		UINT32 en_tx_dma:1;
		UINT32 en_tx_dma_buzy:1;
		UINT32 en_rx_dma:1;
		UINT32 en_rx_dma_buzy:1;
		UINT32 rx_bt_size:2;
		UINT32 en_tx_wb_ddone:1;
		UINT32 big_endian:1;
		UINT32 dis_bt_size_align:1;
		UINT32 tx_bt_size_bit0:1;
		UINT32 multi_dma_en:2;
		UINT32 fifo_little_endian:1;
		UINT32 mi_depth_2_0:3;
		UINT32 mi_depth_5_3:3;
		UINT32 mi_depth_8_6:3;
		UINT32 tx_bt_size_bit21:2;
		UINT32 sw_rst:1;
		UINT32 force_tx_eof:1;
		UINT32 first_token_only:1;
		UINT32 omit_rx_info:1;
		UINT32 omit_tx_info:1;
		UINT32 byte_swap:1;
		UINT32 rsv_30:1;
		UINT32 rx_2b_offset:1;
	} MT7615_E3_field;
	UINT32 word;
} WPDMA_GLO_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */

/*
	CR WPDMA_GLO_CFG[11:10] MULTI_DMA_EN definition
*/
#define MULTI_DMA_EN_DISABLE			0
#define MULTI_DMA_EN_FEATURE_1			1
#define MULTI_DMA_EN_FEATURE_2			2
#define MULTI_DMA_EN_FEATURE_2_PREFETCH		3


#endif /* __MT7615_CR_H__ */
