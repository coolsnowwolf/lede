/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_smac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __MT_MAC_H__
#define __MT_MAC_H__


#include "mac/mac_mt/top.h"

#ifndef COMPOS_WIN
#endif /* COMPOS_WIN */



struct _RTMP_ADAPTER;

typedef struct _TX_RADIO_SET {
	BOOLEAN ItxBFEnable;/* IBF */
	BOOLEAN EtxBFEnable;/* EBF */
	BOOLEAN  ShortGI;
	BOOLEAN  Ldpc;
	BOOLEAN  Stbc;
	UINT8	   CurrentPerPktBW;/* BW_20, BW_40 for Fixed Rate */
	UINT8	    Premable;
	UINT8	    RateCode;
	UINT8	    PhyMode;
} TX_RADIO_SET_T;

typedef struct _TMAC_INFO {
	UINT16 PktLen;
	UINT8 WifiHdrLen; /*This80211HdrLen, wifi_hdr_len*/
	UINT8 QueIdx;
	UINT8 PortIdx;
	UINT8 WmmSet;
	BOOLEAN UsbNextValid; /*Check with Lens*/
	UINT8 Wcid;
	BOOLEAN bAckRequired;
	UINT8 UserPriority;
	UINT8 OwnMacIdx;
	UINT32 CipherAlg; /*Q: bProtectFrame*/
	UINT8 HdrPad; /*W:LengthQosPAD*/
	UINT8 FragIdx; /*W:FragmentByteForTxD*/
	UINT8 BarSsnCtrl;
	UINT8 Pid; /*W: PacketID*/
	UINT8 AntPri;
	UINT8 SpeEn;
	CHAR  PowerOffset;
	BOOLEAN	TimingMeasure;
	TX_RADIO_SET_T TxRadioSet;
	BOOLEAN LongFmt;
	BOOLEAN NeedTrans;
	BOOLEAN MoreData;
	BOOLEAN Eosp;
	BOOLEAN EtherFrame;
	BOOLEAN VlanFrame;
	BOOLEAN BmcPkt;
	BOOLEAN FixRate;
	BOOLEAN BaDisable;
	BOOLEAN TxS2Host;
	BOOLEAN TxS2Mcu;
	UINT8 TxSFmt;
	UINT8 MaxTxTime;
	UINT8 RemainTxCnt;
	UINT8 MpduHdrLen;
	UINT16 FrmType;
	UINT16 SubType;
	UINT16 Sn;
	UINT8 VhtNss;
	UCHAR band_idx;
} TMAC_INFO;

VOID MtWriteTMacInfo(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TMAC_INFO *TxInfo);

#include "mac/mac_mt/dmac/mt_dmac.h"

#endif /* __MT_MAC_H__ */

