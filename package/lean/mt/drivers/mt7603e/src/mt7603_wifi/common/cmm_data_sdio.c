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
	cmm_data_sdio.c
*/


#include "rt_config.h"
#include "rtmp.h"



USHORT MtSDIO_WriteSubTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt)
{

	return 0;
	
}

USHORT MtSDIO_WriteFragTxResource(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR fragNum,
	USHORT *freeCnt)
{
	return(0);
}


USHORT MtSDIO_WriteSingleTxResource(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *freeCnt)
{

}


USHORT MtSDIO_WriteMultiTxResource(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR frmNum,
	USHORT *freeCnt)
{
	NDIS_STATUS Status = 0;

	return(Status);
}


VOID MtSDIO_FinalWriteTxResource(
	RTMP_ADAPTER	*pAd,
	struct _TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT TxIdx)
{

}


VOID MtSDIODataLastTxIdx(
	RTMP_ADAPTER *pAd,
	UCHAR QueIdx,
	USHORT TxIdx)
{
	/* DO nothing for USB.*/
}


VOID MtSDIODataKickOut(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR QueIdx)
{
}


int MtSDIOMgmtKickOut(
	RTMP_ADAPTER *pAd, 
	UCHAR QueIdx,
	PNDIS_PACKET pPacket,
	UCHAR *pSrcBufVA,
	UINT SrcBufLen)
{
	return 0;
}


PNDIS_PACKET GetPacketFromRxRing(
      RTMP_ADAPTER *pAd,
      RX_BLK *pRxBlk,
      BOOLEAN  *pbReschedule,
      UINT32 *pRxPending,
      UCHAR RxRingNo)

{	
	UINT8 RXWISize = pAd->chipCap.RXWISize;
	UINT8 rx_hw_hdr_len = pAd->chipCap.RXWISize;
	PNDIS_PACKET pRxPacket = pAd->SDIORxPacket;
	PUCHAR buf = GET_OS_PKT_DATAPTR(pRxPacket);
	UINT32 len=GET_OS_PKT_LEN(pRxPacket);
	UINT32 i;

	pRxBlk->Flags = 0;
	DBGPRINT(RT_DEBUG_TRACE, ("%s===========start\n",__FUNCTION__));
	if(!pRxPacket){
		DBGPRINT(RT_DEBUG_OFF, ("%s=====return NULL !!!!!\n",__FUNCTION__));
		return NULL;
	}
	
	for(i=0;i<len;i++){
//		printk("%x,",*(buf+i));
		DBGPRINT(RT_DEBUG_OFF, ("%x",*(buf+i)));
	}
	printk("before parse_rx_packet_type \n");

	
	rx_hw_hdr_len = parse_rx_packet_type(pAd, pRxBlk, pRxPacket);
	
	if (rx_hw_hdr_len == 0)
	{
		pRxBlk->DataSize = 0;
	}
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CMD_RSP))
	{
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
		return NULL;
	}
	
	pRxBlk->pRxPacket = pRxPacket;
	pRxBlk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket);
	pRxBlk->pHeader = (HEADER_802_11 *)(pRxBlk->pData);

	return pRxPacket;
}

