/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Abstract:
	IEEE P802.11w

 */
#ifndef __PMF_H
#define __PMF_H
#ifdef DOT11W_PMF_SUPPORT

VOID PMF_PeerAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_MlmeSAQueryReq(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry);

VOID PMF_PeerSAQueryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_PeerSAQueryRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_DeriveIGTK(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR * output);

VOID PMF_InsertIGTKKDE(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);

BOOLEAN PMF_ExtractIGTKKDE(
	IN PUCHAR pBuf,
	IN INT buf_len,
	OUT PUCHAR IGTK,
	OUT UCHAR * IGTKLEN,
	OUT PUCHAR IPN,
	OUT UINT8 * IGTK_KeyIdx);

BOOLEAN PMF_MakeRsnIeGMgmtCipher(
	IN SECURITY_CONFIG * pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR * rsn_len);

UINT PMF_RsnCapableValidation(
	IN PUINT8 pRsnie,
	IN UINT rsnie_len,
	IN BOOLEAN self_MFPC,
	IN BOOLEAN self_MFPR,
	IN UINT32 self_igtk_cipher,
	IN UCHAR end_field,
	IN struct _SECURITY_CONFIG *pSecConfigEntry);

BOOLEAN PMF_PerformTxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHeader_802_11,
	IN UINT SrcBufLen,
	IN UINT8 tx_hw_hdr_len,
	OUT UCHAR * prot);

BOOLEAN	PMF_PerformRxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK * pRxBlk);

int PMF_RobustFrameClassify(
	IN PHEADER_802_11 pHdr,
	IN PUCHAR pFrame,
	IN UINT	frame_len,
	IN PUCHAR pData,
	IN BOOLEAN IsRx);

#ifdef SOFT_ENCRYPT
int PMF_EncapBIPAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len);
#endif /* SOFT_ENCRYPT */

void PMF_AddMMIE(
	IN PMF_CFG * pPmfCfg,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len);

void rtmp_read_pmf_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer);

INT Set_PMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFSA_Q_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

#endif /* DOT11W_PMF_SUPPORT */

#endif /* __PMF_H */

