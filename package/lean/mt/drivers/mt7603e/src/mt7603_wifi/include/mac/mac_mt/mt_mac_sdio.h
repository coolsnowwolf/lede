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

	Module Name:
	mt_mac_sdio.h
*/

#ifndef __MAC_SDIO_H__
#define __MAC_SDIO_H__

#include "rtmp_type.h"
#include "phy/phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"

#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags)				\
			{													\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				if (pAd->DeQueueRunning[QueIdx])						\
				{														\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
					DBGPRINT(RT_DEBUG_TRACE, ("DeQueueRunning[%d]= TRUE!\n", QueIdx));		\
					continue;											\
				}														\
				else													\
				{														\
					pAd->DeQueueRunning[QueIdx] = TRUE;					\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
				}														\
			}

#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)						\
			do{															\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				pAd->DeQueueRunning[QueIdx] = FALSE;					\
				RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);	\
			}while(0)

#define	RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) 1

#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)			\
		do{}while(0)

#define NEED_QUEUE_BACK_FOR_AGG(_pAd, _QueIdx, _freeNum, _TxFrameType) 	0	

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
	MtSDIO_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
	MtSDIO_WriteSingleTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)


#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
	MtSDIO_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
	MtSDIO_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)	\
	MtSDIO_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)

#define HAL_LastTxIdx(pAd, QueIdx,TxIdx) \
	MtSDIODataLastTxIdx(pAd, QueIdx,TxIdx)

#define HAL_KickOutTx(pAd, pTxBlk, QueIdx)	\
	MtSDIODataKickOut(pAd, pTxBlk, QueIdx)

#define HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)	\
	MtSDIOMgmtKickOut(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
		do{}while(0)

#define GET_TXRING_FREENO(_pAd, _QueIdx)	(_QueIdx) /*(_pAd->TxRing[_QueIdx].TxSwFreeIdx) */
#define GET_MGMTRING_FREENO(_pAd)			(_pAd->MgmtRing.TxSwFreeIdx)

/* Insert the BA bitmap to ASIC for the Wcid entry */
#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _Aid, _TID, _SN, _basize, _type)					\
		do{																\
		}while(0)

/* Remove the BA bitmap from ASIC for the Wcid entry */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _Wcid, _TID, _type)				\
		do{																\
		}while(0)


/*
  *	MLME Related MACRO 
*/

#define RTMP_MLME_PRE_SANITY_CHECK(pAd)								\
	do{}while(0)
/*
	{	if ((pAd->StaCfg.bHardwareRadio == TRUE) && 					\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&		\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))) {	\
			RTEnqueueInternalCmd(pAd, CMDTHREAD_CHECK_GPIO, NULL, 0); } }
*/
#define RTMP_MLME_RESET_STATE_MACHINE(pAd)	\
	do{}while(0)
/*
		        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_RESET_CONF, 0, NULL, 0);	\
		        RTMP_MLME_HANDLER(pAd);
*/
#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)\
	do{}while(0)
/*
	{	RTEnqueueInternalCmd(_pAd, CMDTHREAD_802_11_COUNTER_MEASURE, _pEntry, sizeof(MAC_TABLE_ENTRY));	\
		RTMP_MLME_HANDLER(_pAd);									\
	}
*/

/*
  *	Power Save Related MACRO 
  */

#define RTMP_MLME_RADIO_ON(pAd) \
		do{}while(0)

#define RTMP_MLME_RADIO_OFF(pAd) \
		do{}while(0)

#define RTMP_STA_ENTRY_MAC_RESET(pAd, Wcid)	\
		do{}while(0)

/* MAC Search table */
/* add this entry into ASIC RX WCID search table */
#define RTMP_STA_ENTRY_ADD(pAd, pEntry)	\
    do{}while(0)
/*						\
{																\
	RT_SET_ASIC_WCID Info;									\
																\
	Info.WCID = pEntry->wcid;									\
	NdisMoveMemory(Info.Addr, pEntry->Addr, MAC_ADDR_LEN);		\
																\
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SET_CLIENT_MAC_ENTRY, 	\
							&Info, sizeof(RT_SET_ASIC_WCID));	\
}
*/

/* ----------------- Security Related MACRO ----------------- */

/* Set Asic WCID Attribute table */
#define RTMP_SET_WCID_SEC_INFO(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)	\
                 do{}while(0)
/* Set Asic WCID IV/EIV table */
#define RTMP_ASIC_WCID_IVEIV_TABLE(_pAd, _Wcid, _uIV, _uEIV)	\
                 do{}while(0)
/*
{																\
	RT_ASIC_WCID_IVEIV_ENTRY Info;							\
																\
	Info.Wcid = _Wcid;											\
	Info.Iv = _uIV;												\
	Info.Eiv = _uEIV;											\
																\
	RTEnqueueInternalCmd(_pAd, CMDTHREAD_SET_ASIC_WCID_IVEIV, 	\
							&Info, 								\
							sizeof(RT_ASIC_WCID_IVEIV_ENTRY));	\
}
*/
/* Set Asic WCID Attribute table */
#define RTMP_ASIC_WCID_ATTR_TABLE(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)\
                 do{}while(0)
/* Set Asic Pairwise key table */
#define RTMP_ASIC_PAIRWISE_KEY_TABLE(_pAd, _WCID, _pCipherKey)	\
                 do{}while(0)
/* Set Asic Shared key table */
#define RTMP_ASIC_SHARED_KEY_TABLE(_pAd, _BssIndex, _KeyIdx, _pCipherKey) \
                 do{}while(0)

#define RTMP_UPDATE_PROTECT(_pAd, _OperationMode, _SetMask, _bDisableBGProtect, _bNonGFExist)	\
                 do{}while(0)


/* Remove Pairwise Key table */
#define RTMP_REMOVE_PAIRWISE_KEY_ENTRY(_pAd, _Wcid)\
                 do{}while(0)
#define MT_ADDREMOVE_KEY(_pAd, _AddRemove, _BssIdx, _KeyIdx, _Wcid, _KeyTabFlag, _pCipherKey, _PeerAddr)\
   do{}while(0)


#define RTMP_OS_IRQ_RELEASE(_pAd, _NetDev)

VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

#endif /*__MAC_SDIO_H__ */

