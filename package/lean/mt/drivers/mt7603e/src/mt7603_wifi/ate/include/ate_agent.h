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
	ate_agent.h
*/

#ifndef __ATE_AGENT_H__
#define __ATE_AGENT_H__

struct _RTMP_ADAPTER;

INT32 SetTxStop(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetRxStop(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 RtmpDoAte(struct _RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, RTMP_STRING *wrq_name);
VOID ATE_QA_Statistics(struct _RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI, RXINFO_STRUC *pRxInfo, PHEADER_802_11 pHeader);

#ifdef DBG
INT32 SetEERead(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetEEWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetBBPRead(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetBBPWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetRFWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#endif /* DBG */ 
VOID rt_ee_read_all(struct _RTMP_ADAPTER *pAd, UINT16 *Data);
VOID rt_ee_write_all(struct _RTMP_ADAPTER pAd, UINT16 *Data);
VOID rt_ee_write_bulk(struct _RTMP_ADAPTER pAd, UINT16 *Data, UINT16 offset, UINT16 length);
INT32 SetATEDa(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATESa(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEBssid(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEInitChan(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetADCDump(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower0(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower1(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPowerEvaluation(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxAntenna(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxAntenna(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 Default_Set_ATE_TX_FREQ_OFFSET_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxFreqOffset(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 Default_Set_ATE_TX_BW_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxLength(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxCount(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxMcs(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxStbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxGi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxFer(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEReadRF(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATELoadE2p(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#endif /* RTMP_EFUSE_SUPPORT */
INT32 SetATEReadE2p(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEAutoAlc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxGi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETempSensor(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIpg(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEPayload(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEFixedPayload(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETtr(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEShow(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEHelp(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
VOID ATESampleRssi(struct _RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI);
VOID  ATEPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext, 
						PVOID SystemSpecific2, PVOID SystemSpecific3);
INT32 SetATE(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEChannel(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 MT_ATEInit(struct _RTMP_ADAPTER *pAd);
PNDIS_PACKET ATEPayloadInit(struct _RTMP_ADAPTER *pAd, UINT32 TxIdx);
INT32 ATEPayloadAlloc(struct _RTMP_ADAPTER *pAd, UINT32 Index);
VOID EEReadAll(struct _RTMP_ADAPTER *pAd, UINT16 *Data);
#endif

