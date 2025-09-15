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
	mtsdio_data.h
*/

typedef struct _SDIOWorkTask{
	DL_LIST List;
	UINT32 Txcnt;
	UINT32 RxCnt;
	UINT32 CmdCnt;
} SDIOWorkTask;

typedef struct _SDIOTxPacket{
	DL_LIST List;
	PNDIS_PACKET NetPkt;
} SDIOTxPacket;

INT32 MTSDIOCmdTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
INT32 MTSDIODataTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
VOID MTSDIOAddWorkerTaskList(struct _RTMP_ADAPTER *pAd);
VOID MTSDIODataWorkerTask(struct _RTMP_ADAPTER *pAd);
VOID MTSDIODataIsr(struct _RTMP_ADAPTER *pAd);
