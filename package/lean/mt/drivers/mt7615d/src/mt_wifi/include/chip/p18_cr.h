#ifndef __P18_CR_H__
#define __P18_CR_H__
/*
	Please don't include this header outside of per chip scope
*/

/*
	CR CONN_HIF_RST
*/
#define MT_CONN_HIF_RST			(MT_HIF_BASE + 0x0100)
#define CONN_HIF_LOGIC_RST_N	(1 << 4)
#define DMASHDL_ALL_RST_N		(1 << 5)
/*
	CR CONN_HIF_BUSY_STATUS
*/
#define MT_CONN_HIF_BUSY_STATUS	(MT_HIF_BASE + 0x0138)
#define CONN_HIF_BUSY			(1 << 31)

#ifdef RT_BIG_ENDIAN
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct {
		UINT32 rx_2b_offset:1;
		UINT32 clk_gate_dis:1;
		UINT32 byte_swap:1;
		UINT32 omit_tx_info:1;
		UINT32 omit_rx_info:1;
		UINT32 pdma_addr_ext_en:1;
		UINT32 force_tx_eof:1;
		UINT32 rsv0:3;
		UINT32 trx_pfet_arb_mi_depth:3;
		UINT32 trx_dfet_arb_mi_depth:3;
		UINT32 trx_pfet_dfet_mi_depth:3;
		UINT32 fifo_little_endian:1;
		UINT32 multi_dma_en:2;
		UINT32 fw_ring_bp_tx_sch:1;
		UINT32 Desc32BEn:1;
		UINT32 BigEndian:1;
		UINT32 EnTXWriteBackDDONE:1;
		UINT32 WPDMABurstSIZE:2;
		UINT32 RxDMABusy:1;
		UINT32 EnableRxDMA:1;
		UINT32 TxDMABusy:1;
		UINT32 EnableTxDMA:1;
	} P18_field;
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
		UINT32 fw_ring_bp_tx_sch:1;
		UINT32 multi_dma_en:2;
		UINT32 fifo_little_endian:1;
		UINT32 trx_pfet_dfet_mi_depth:3;
		UINT32 trx_dfet_arb_mi_depth:3;
		UINT32 trx_pfet_arb_mi_depth:3;
		UINT32 rsv0:3;
		UINT32 force_tx_eof:1;
		UINT32 pdma_addr_ext_en:1;
		UINT32 omit_rx_info:1;
		UINT32 omit_tx_info:1;
		UINT32 byte_swap:1;
		UINT32 clk_gate_dis:1;
		UINT32 rx_2b_offset:1;
	} P18_field;
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

#endif /* __P18_CR_H__ */
