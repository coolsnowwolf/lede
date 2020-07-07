/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/


#ifndef __RRM_H
#define __RRM_H

#ifdef DOT11K_RRM_SUPPORT
#include "rtmp_type.h"
#include "rrm_cmm.h"

#define RRM_QUIET_CNT_DEC(_V, _M) \
	((_V) == 0 ? (_V = _M) : (_V)--)

#define IS_RRM_ENABLE(_P) \
	((_P)->RrmCfg.bDot11kRRMEnable == TRUE)

#define IS_RRM_CAPABLE(_P) \
	((_P)->CapabilityInfo & RRM_CAP_BIT)

#define IS_RRM_BEACON_ACTIVE_MEASURE(_P) \
	((_P)->RrmEnCap.field.BeaconActiveMeasureCap == 1)

#define IS_RRM_BEACON_PASSIVE_MEASURE(_P) \
	((_P)->RrmEnCap.field.BeaconPassiveMeasureCap == 1)

#define IS_RRM_BEACON_TABLE_MEASURE(_P) \
	((_P)->RrmEnCap.field.BeaconTabMeasureCap == 1)

#define IS_RRM_QUIET(_P) \
	((_P)->RrmCfg.QuietCB.QuietState == RRM_QUIET_SILENT)

#define RRM_BCNREQ_MODE_OFFSET 13

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
void RRM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_Dot11kRRM_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_BeaconReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BeaconReq_RandInt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_LinkMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_TxStreamMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_RRM_Selftest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT RRM_InfoDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_CfgInit(
	IN PRTMP_ADAPTER pAd);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_QuietUpdata(
	IN PRTMP_ADAPTER pAd);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertBcnReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBcnReq);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertBcnReqSsidSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pSsid,
	IN UINT8 SsidLen);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertBcnReqRepCndSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 RepCnd,
	IN UINT8 Threshold);

VOID RRM_InsertBcnReqRepDetailSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Detail);

/*
	==========================================================================
	Description:
		Insert RRM Enable Capabilitys IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertRRMEnCapIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN INT BssIdx);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertNeighborRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN RRM_PNEIGHBOR_REP_INFO pNeighborRepInfo);
/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertNeighborTSFOffsetSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT16 TSFOffset,
	IN UINT16 BcnInterval);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertQuietIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 QuietCnt,
	IN UINT8 QuietPeriod,
	IN UINT8 QuietDuration,
	IN UINT8 QuietOffset);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertBssACDelayIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertBssAvailableACIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertRequestIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ie_num,
	IN PUINT8 ie_list);


VOID RRM_InsertRequestIE_11KV_API(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pRequest,
	IN UINT8 RequestLen);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertTxStreamReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_InsertTxStreamReqTriggerReportSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf);


/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_EnqueueBcnReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 IfIdx,
	IN PRRM_MLME_BCN_REQ_INFO pMlmeBcnReq);


#ifdef CONFIG_11KV_API_SUPPORT
INT rrm_send_beacon_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_bcn_req_info pBcnReq,
	IN UINT32 BcnReqLen);

int check_rrm_BcnReq_custom_params(
	IN RTMP_ADAPTER *pAd,
	IN p_bcn_req_info pBcnReq);

VOID set_rrm_BcnReq_optional_params(
	IN RTMP_ADAPTER *pAd,
	IN p_bcn_req_info p_beacon_req);

void compose_rrm_BcnReq_ie(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR beacon_req_ie,
	OUT PUINT32 beacon_req_ie_len,
	IN p_bcn_req_info p_beacon_req,
	IN UINT8 measure_req_token,
	IN UCHAR ifidx);

void wext_send_bcn_rsp_event(PNET_DEV net_dev,
	PUCHAR peer_mac_addr,
	PUCHAR bcn_rsp,
	UINT32 bcn_rsp_len,
	UINT8 dia_token);
#endif /* COFNIG_11KV_API_SUPPORT */



BOOLEAN RRM_CheckBssAndStaSecurityMatch(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BSS_ENTRY *pBssEntry
);

VOID RRM_EnqueueNeighborRep(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UINT8 DialogToken,
	IN PCHAR pSsid,
	IN UINT8 SsidLen);

VOID RRM_EnqueueLinkMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 apidx);

VOID RRM_EnqueueTxStreamMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 apidx,
	IN PRRM_MLME_TRANSMIT_REQ_INFO pMlmeTxMeasureReq);
/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
BOOLEAN RRM_PeerNeighborReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PCHAR * pSsid,
	OUT PUINT8 pSsidLen);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
BOOLEAN RRM_PeerMeasureReportSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REPORT_INFO pMeasureReportInfo,
	OUT PVOID * pMeasureRep);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
VOID RRM_PeerNeighborReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID RRM_PeerMeasureRepAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

#ifdef FTM_SUPPORT
VOID RRM_InsertNeighborMsmtRptLocationLCISubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN	PRRM_CONFIG pRrmCfg,
	IN PMSMT_RPT_SUBELEMENT pLciHdr,
	IN PUSAGE_SUBELEMENT pLciUsage,
	IN PZ_ELEMENT pLciZ,
	IN PLCI_FIELD pLci,
	IN BOOLEAN bSetZRpt
);

VOID RRM_InsertNeighborMsmtRptLocationCIVICSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN	PRRM_CONFIG pRrmCfg,
	IN  PMSMT_RPT_SUBELEMENT pCivicHdr,
	IN  PLOCATION_CIVIC pCivic,
	IN	UINT8 * pCA_Value
);

#endif /* FTM_SUPPORT */
	
void init_rrm_capabilities(PRRM_CONFIG pRrmCfg, BSS_STRUCT *pMBss);

int set_rrm_capabilities(RTMP_ADAPTER *pAd, UINT8 *rrm_capabilities);

int rrm_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);

int RRM_EnqueueBcnReqAction(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 IfIdx,
	IN p_bcn_req_data_t p_bcn_req_data);

int Set_Dot11kRRM_Enable(RTMP_ADAPTER *pAd, UINT8 enable);

#ifdef CONFIG_11KV_API_SUPPORT
int rrm_send_beacon_req(RTMP_ADAPTER *pAd,
	p_bcn_req_data_t p_bcn_req_data, UINT32 bcn_req_len);
#else
int rrm_send_beacon_req(RTMP_ADAPTER *pAd, p_bcn_req_data_t p_bcn_req_data);
#endif /* COFNIG_11KV_API_SUPPORT */

void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

#ifdef CONFIG_11KV_API_SUPPORT
int rrm_send_nr_rsp_param(RTMP_ADAPTER *pAd,
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data,
	UINT32 nr_rsp_data_len);

int rrm_send_nr_rsp_ie(RTMP_ADAPTER *pAd,
	p_nr_rsp_data_t p_nr_rsp_data,
	UINT32 nr_rsp_data_len);


int check_rrm_nrrsp_custom_params(RTMP_ADAPTER *pAd,
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data,
	UINT32 nr_rsp_data_len);

void compose_rrm_nrrsp_ie(RTMP_ADAPTER *pAd, PUCHAR nr_rsp_ie, PUINT32 p_ie_len,
	struct nr_info *p_candidate_info, UINT8 cnt);

VOID send_nr_rsp_param_toair(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem);

VOID send_nr_resp_toair(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem);

void wext_send_nr_req_event(
	IN PNET_DEV net_dev,
	IN const char *peer_mac_addr,
	IN const char *nr_req,
	IN UINT16 nr_req_len);


int rrm_send_nr_rsp(
	IN RTMP_ADAPTER *pAd,
	IN p_nr_rsp_data_t p_nr_rsp_data);


int rrm_set_handle_nr_req_flag(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 by_daemon);

VOID  NRRspTimeout(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem);


enum NR_STATE NRPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);


VOID NRStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);

VOID RRMBcnReqStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);

enum BCN_STATE BCNPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

#endif /* CONFIG_11KV_API_SUPPORT*/
#endif /* DOT11K_RRM_SUPPORT */

#endif /* __RRM_H */

