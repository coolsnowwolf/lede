#ifndef __MT7622_CR_H__
#define __MT7622_CR_H__
/*
	Please don't include this header outside of per chip scope
*/

/*
	CR HIF_SYS_SW_RST
	this CR can't be accessd directly, needs remap.
*/
#define HIF_REG_BASE			0x50000000
#define MT_HIF_SYS_SW_RST	    (HIF_REG_BASE + 0x4034)
#define HIF_DATA_PATH_RESET_N	(1 << 1)

#define HIF_PDMA_IO_READ32(_A, _R, _pV) \
	do {\
		UINT32 restore_remap_addr;\
		HW_IO_READ32(_A, 0x2504, &restore_remap_addr);\
		\
		HW_IO_WRITE32(_A, 0x2504, HIF_REG_BASE);\
		HW_IO_READ32(_A, (0x80000+((UINT32)_R&0xffff)), _pV);\
		\
		HW_IO_WRITE32(_A, 0x2504, restore_remap_addr);\
	} while (0)

#define HIF_PDMA_IO_WRITE32(_A, _R, _pV) \
	do {\
		UINT32 restore_remap_addr;\
		\
		HW_IO_READ32(_A, 0x2504, &restore_remap_addr);\
		\
		HW_IO_WRITE32(_A, 0x2504, HIF_REG_BASE);\
		HW_IO_WRITE32(_A, (0x80000+((UINT32)_R&0xffff)), _pV);\
		\
		HW_IO_WRITE32(_A, 0x2504, restore_remap_addr);\
	} while (0)

#ifdef RT_BIG_ENDIAN
	typedef union _WPDMA_GLO_CFG_STRUC	{
		struct {
			UINT32 rx_2b_offset:1;
			UINT32 clk_gate_dis:1;
			UINT32 byte_swap:1;
			UINT32 omit_tx_info:1;
			UINT32 omit_rx_info:1;
			UINT32 first_token_only:1;
			UINT32 force_tx_eof:1;
			UINT32 sw_rst:1;
			UINT32 rsv0:2;
			UINT32 mi_depth_rd_8_6:3;
			UINT32 mi_depth_rd_5_3:3;
			UINT32 mi_depth_rd_2_0:3;
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
		} MT7622_field;
		UINT32 word;
	} WPDMA_GLO_CFG_STRUC;
#else
	typedef union _WPDMA_GLO_CFG_STRUC	{
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
			UINT32 mi_depth_rd_2_0:3;
			UINT32 mi_depth_rd_5_3:3;
			UINT32 mi_depth_rd_8_6:3;
			UINT32 rsv0:2;
			UINT32 sw_rst:1;
			UINT32 force_tx_eof:1;
			UINT32 first_token_only:1;
			UINT32 omit_rx_info:1;
			UINT32 omit_tx_info:1;
			UINT32 byte_swap:1;
			UINT32 clk_gate_dis:1;
			UINT32 rx_2b_offset:1;
		} MT7622_field;
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

#endif /* __MT7622_CR_H__ */
