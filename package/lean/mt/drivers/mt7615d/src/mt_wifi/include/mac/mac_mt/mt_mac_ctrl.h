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
	mt_mac_ctrl.h
*/

#ifndef __MT_MAC_CTRL_H__
#define __MT_MAC_CTRL_H__

/* TODO: shiang-usw,remove this */
#define HW_BEACON_OFFSET		0x0200
/* TODO: ---End */


/* TODO: shiang-usw, mark by shiang, need to remove it */
typedef union GNU_PACKED _TXWI_STRUC {
	UINT32 word;
} TXWI_STRUC;

typedef union GNU_PACKED _RXWI_STRUC {
	UINT32 word;
} RXWI_STRUC;
/* TODO: --End */


struct _RTMP_ADAPTER;


#define TXS2MCU_AGGNUMS 31
#define TXS2HOST_AGGNUMS 31

enum {
	TXS_UNUSED,
	TXS_USED,
};

typedef struct _TXS_STATUS {
	UINT8 TxSPid;
	UINT8 State;
	UINT8 Type;
	UINT8 PktPid;
	UINT8 PktType;
	UINT8 PktSubType;
	UINT16 TxRate;
	UINT32 Priv;
} TXS_STATUS, *PTXS_STATYS;

#define TOTAL_PID_HASH_NUMS	0x10
#define TOTAL_PID_HASH_NUMS_PER_PKT_TYPE 0x8
#define TXS_WLAN_IDX_ALL 128
/*discuss with FW team, Host driver use PID from 0~127, FW use PID from 128 to 255 */
#define TXS_STATUS_NUM 128
typedef struct _TXS_CTL {
	/* TXS type hash table per pkt */
	DL_LIST TxSTypePerPkt[TOTAL_PID_HASH_NUMS];
	NDIS_SPIN_LOCK TxSTypePerPktLock[TOTAL_PID_HASH_NUMS];
	/* TXS type hash table per pkt type and subtype */
	DL_LIST TxSTypePerPktType[3][TOTAL_PID_HASH_NUMS_PER_PKT_TYPE];
	NDIS_SPIN_LOCK TxSTypePerPktTypeLock[3][TOTAL_PID_HASH_NUMS_PER_PKT_TYPE];
	ULONG TxS2McUStatusPerPkt;
	ULONG TxS2HostStatusPerPkt;
	ULONG TxS2McUStatusPerPktType[3];
	ULONG TxS2HostStatusPerPktType[3];
	ULONG TxSFormatPerPkt;
	ULONG TxSFormatPerPktType[3];
	UINT64 TxSStatusPerWlanIdx[2];
	ULONG TxSFailCount;
	UINT8 TxSPid;
	TXS_STATUS TxSStatus[TXS_STATUS_NUM];
	UINT32  TxSPidBitMap0_31;
	UINT32  TxSPidBitMap32_63;
	UINT32  TxSPidBitMap64_95;
	UINT32  TxSPidBitMap96_127;
} TXS_CTL, *PTXS_CTL;

#define TXS_TYPE0 0 /* Per Pkt */
#define TXS_TYPE1 1 /* Per Pkt Type */
#define TXS_DUMP_REPEAT 0xffffffff
typedef struct _TXS_TYPE {
	DL_LIST List;
	UINT8 Type;
	UINT8 PktPid;
	UINT8 PktType;
	UINT8 PktSubType;
	UINT8 Format;
	INT32(*TxSHandler)(struct _RTMP_ADAPTER *pAd, CHAR * Data, UINT32 Priv);
	BOOLEAN DumpTxSReport;
	ULONG DumpTxSReportTimes;
} TXS_TYPE, *PTXS_TYPE;

typedef INT32(*TXS_HANDLER)(struct _RTMP_ADAPTER *pAd, CHAR * Data, UINT32 Priv);

INT32 InitTxSTypeTable(struct _RTMP_ADAPTER *pAd);
INT32 InitTxSCommonCallBack(struct _RTMP_ADAPTER *pAd);
INT32 ExitTxSTypeTable(struct _RTMP_ADAPTER *pAd);
INT32 AddTxSTypePerPkt(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format,
					   TXS_HANDLER TxSHandler);
INT32 RemoveTxSTypePerPkt(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format);
INT32 TxSTypeCtlPerPkt(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format,
					   BOOLEAN TxS2Mcu, BOOLEAN TxS2Host,
					   BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes);
INT32 ParseTxSPacket_v1(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data);
INT32 ParseTxSPacket_v2(struct _RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data);
INT32 BcnTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv);

#ifdef STA_LP_PHASE_1_SUPPORT
INT32 NullFramePM1TxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv);
INT32 NullFramePM0TxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv);
#endif /* STA_LP_PHASE_1_SUPPORT */

INT32 AddTxSTypePerPktType(struct _RTMP_ADAPTER *pAd, UINT8 Type, UINT8 Subtype,
						   UINT8 Format, TXS_HANDLER TxSHandler);
INT32 RemoveTxSTypePerPktType(struct _RTMP_ADAPTER *pAd, UINT8 Type, UINT8 Subtype,
							  UINT8 Format);
INT32 TxSTypeCtlPerPktType(struct _RTMP_ADAPTER *pAd, UINT8 Type, UINT8 Subtype, UINT16 WlanIdx,
						   UINT8 Format, BOOLEAN TxS2Mcu, BOOLEAN TxS2Host,
						   BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes);
UINT8 AddTxSStatus(struct _RTMP_ADAPTER *pAd, UINT8 Type, UINT8 PktPid, UINT8 PktType,
				   UINT8 PktSubType, UINT16 TxRate, UINT32 Priv);
INT32 RemoveTxSStatus(struct _RTMP_ADAPTER *pAd, UINT8 TxSPid, UINT8 *Type, UINT8 *PktPid,
					  UINT8 *PktType, UINT8 *PktSubType, UINT16 *TxRate, UINT32 *Priv);

#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv);
#endif /* CFG_TDLS_SUPPORT */
INT32 ActionTxSHandler(struct _RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv);


typedef struct _TMR_CTRL_STRUCT {
	UCHAR HwTmrVersion;
	UCHAR TmrEnable;/* disable, initiator, responder */
	UCHAR TmrState; /* used to control CR enable/disable@initiator role. */
	UINT32 TmrCalResult;
} TMR_CTRL_STRUCT, *PTMR_CTRL_STRUCT;


#endif /* __MT_MAC_CTRL_H__ */
