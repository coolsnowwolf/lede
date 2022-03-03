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
	mt_sdio.h
*/


#ifndef __MT_SDIO_H__
#define __MT_SDIO_H__

#include "rt_config.h"
#include "mtsdio_io.h"
#include "mtsdio_data.h"

struct _RTMP_ADAPTER;
struct _TX_BLK;

void rt_sdio_interrupt(struct sdio_func *func);
void InitSDIODevice(VOID *ad_src);

USHORT MtSDIO_WriteSubTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteSingleTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteFragTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *freeCnt);
USHORT MtSDIO_WriteMultiTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frmNum, USHORT *freeCnt);
VOID MtSDIO_FinalWriteTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT mpdu_len, USHORT TxIdx);

VOID MtSDIODataLastTxIdx(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, USHORT TxIdx);
VOID MtSDIODataKickOut(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR QueIdx);
int MtSDIOMgmtKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, PNDIS_PACKET pkt, UCHAR *pSrcBufVA, UINT SrcBufLen);
VOID MtSDIONullFrameKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, UCHAR *pNullFrm, UINT32 frmLen);

#endif

