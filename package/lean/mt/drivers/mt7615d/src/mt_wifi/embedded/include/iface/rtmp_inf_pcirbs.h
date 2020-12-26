#ifndef __RTMP_INF_PCIRBS_H__
#define __RTMP_INF_PCIRBS_H__

struct _RTMP_ADAPTER;
struct _TX_BLK;

#define fRTMP_ADAPTER_NEED_STOP_TX	0

#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)								\
	{																			\
		POS_COOKIE pObj = (POS_COOKIE)(_pAd->OS_Cookie);						\
		RtmpOSIRQRelease(_NetDev, _pAd->infType, pObj->pci_dev, NULL);			\
	}


/* ----------------- TX Related MACRO ----------------- */
#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags, deq_info)		do {} while (0)
#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)		do {} while (0)


#define RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
	((freeNum) >= (ULONG)(pTxBlk->TotalFragNum + RTMP_GET_PACKET_FRAGMENTS(pPacket) + 3)) /* rough estimate we will use 3 more descriptor. */
#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)	\
	do {} while (0)

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
	/* RtmpPCI_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)*/

USHORT	mt_pci_write_frag_tx_resource(struct _RTMP_ADAPTER *pAd,
									  struct _TX_BLK *pTxBlk,
									  UCHAR fragNum,
									  USHORT *FreeNumber);

#define HAL_LastTxIdx(_pAd, _QueIdx, _LastTxIdx) \
	/*RtmpPCIDataLastTxIdx(_pAd, _QueIdx,_LastTxIdx)*/

#define HAL_KickOutTxBMC(_pAd, _pTxBlk, _QueIdx)	\
	HIF_IO_WRITE32((_pAd), (_pAd)->TxBmcRing.hw_cidx_addr, (_pAd)->TxBmcRing.TxCpuIdx)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
	MiniportMMRequest(_pAd, _QueIdx, _pNullFrame, _frameLen)

UINT32 pci_get_tx_resource_free_num_nolock(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
UINT32 pci_get_tx_resource_free_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
UINT32 pci_get_rx_resource_pending_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
UINT32 pci_get_tx_bcn_free_num(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
UINT32 pci_get_tx_mgmt_free_num(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_is_tx_resource_empty(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_is_rx_resource_empty(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
UINT32 pci_get_tx_ctrl_free_num(struct _RTMP_ADAPTER *pAd);


UINT16 GET_TXRING_FREENO(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx);

/* TODO: shiang-MT7615, fix me after the cmd ring and N9 is ready for Beacon handling!! */
#define GET_MGMTRING_FREENO(_pAd, _RingIdx)\
	pci_get_tx_mgmt_free_num(_pAd, _RingIdx)
#define GET_BCNRING_FREENO(_pAd, _RingIdx)\
	pci_get_tx_bcn_free_num(_pAd, _RingIdx)

#define IS_TXRING_EMPTY(_pAd, _QueIdx)\
	pci_is_tx_resource_empty(_pAd, _QueIdx)

#define GET_RXRING_PENDINGNO(_pAd, _QueIdx) \
	pci_get_rx_resource_pending_num(_pAd, _QueIdx)

#define IS_RXRING_FULL(_pAd, _QueIdx)\
	pci_is_rx_resource_empty(_pAd, _QueIdx)

#ifdef CONFIG_ANDES_SUPPORT
#define GET_CTRLRING_FREENO(_pAd) \
	pci_get_tx_ctrl_free_num(_pAd)

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
#define GET_FWDWLORING_FREENO(_Ring) \
	(((_Ring)->TxSwFreeIdx > (_Ring)->TxCpuIdx)	? \
	((_Ring)->TxSwFreeIdx - (_Ring)->TxCpuIdx - 1) \
	:	\
	((_Ring)->TxSwFreeIdx + (_Ring)->ring_size - (_Ring)->TxCpuIdx - 1))
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* CONFIG_ANDES_SUPPORT */

USHORT mtd_pci_write_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber);

USHORT mt_pci_write_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber);

USHORT mt_pci_write_multi_rx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR frameNum,
	USHORT *FreeNumber);

USHORT rt_pci_write_multi_rx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR frameNum,
	USHORT *FreeNumber);

VOID pci_kickout_data_tx(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *tx_blk,
	UCHAR que_idx);

VOID mt_pci_write_final_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT FirstTxIdx);

USHORT RtmpPCI_WriteFragTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN UCHAR fragNum,
	OUT	USHORT *FreeNumber);

VOID RtmpPCI_FinalWriteTxResource(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN USHORT totalMPDUSize,
	IN USHORT FirstTxIdx);

VOID pci_inc_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
VOID pci_dec_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_get_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
INT pci_set_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state);
UINT32 pci_check_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);

VOID tx_dma_done_func(struct _RTMP_ADAPTER *pAd);
VOID rx_done_func(struct _RTMP_ADAPTER *pAd);
VOID rx1_done_func(struct _RTMP_ADAPTER *pAd);
VOID tr_done_func(struct _RTMP_ADAPTER *pAd);
VOID mt_mac_recovery_func(struct _RTMP_ADAPTER *pAd);
VOID mt_mac_fw_own_func(struct _RTMP_ADAPTER *pAd);
VOID mt_subsys_int_func(struct _RTMP_ADAPTER *pAd);

#endif /* __RTMP_INF_PCIRBS_H__ */

