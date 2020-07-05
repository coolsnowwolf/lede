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

    Module Name:
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include "rt_config.h"

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertActField(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	Category,
	IN	UINT8	ActCode)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&Category,
						1,				&ActCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertStatusCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	StatusCode)
{
	ULONG TempLen;

	StatusCode = cpu2le16(StatusCode);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						2,				&StatusCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertReasonCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	ReasonCode)
{
	ULONG TempLen;

	ReasonCode = cpu2le16(ReasonCode);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						2,				&ReasonCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertDialogToken(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	DialogToken)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&DialogToken,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertLinkIdentifierIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	PUCHAR	pInitAddr,
	IN	PUCHAR	pRespAddr)
{
	ULONG TempLen;
	UCHAR TDLS_IE = IE_TDLS_LINK_IDENTIFIER;
	UCHAR TDLS_IE_LEN = TDLS_ELM_LEN_LINK_IDENTIFIER;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&TDLS_IE,
						1,				&TDLS_IE_LEN,
						6,				pAd->CommonCfg.Bssid,
						6,				pInitAddr,
						6,				pRespAddr,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertCapIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						2,				&pAd->StaActive.CapabilityInfo,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSSIDIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&SsidIe,
						1,							&pAd->CommonCfg.SsidLen, 
						pAd->CommonCfg.SsidLen,		pAd->CommonCfg.Ssid,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSupportRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&SupRateIe,
						1,						&pAd->StaActive.SupRateLen,
						pAd->StaActive.SupRateLen,	pAd->StaActive.SupRate,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertCountryIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
    // add country IE, power constraint IE
	if (pAd->CommonCfg.bCountryFlag)
	{
		ULONG TmpLen, TmpLen2=0;
		UCHAR TmpFrame[256];
		UCHAR CountryIe = IE_COUNTRY;

		NdisZeroMemory(TmpFrame, sizeof(TmpFrame));

		// prepare channel information
		{
			UCHAR regclass;
			UCHAR RegluatoryRxtIdent = 221;
			UCHAR CoverageClass = 0;

			regclass = TDLS_GetRegulatoryClass(pAd, pAd->CommonCfg.RegTransmitSetting.field.BW, pAd->CommonCfg.Channel);
			MakeOutgoingFrame(TmpFrame+TmpLen2,	&TmpLen,
								1,				&RegluatoryRxtIdent,
								1,				&regclass,
								1,				&CoverageClass,
								END_OF_ARGS);
			TmpLen2 += TmpLen;
		}

		// need to do the padding bit check, and concatenate it
		if ((TmpLen2%2) == 0)
		{
			UCHAR	TmpLen3 = TmpLen2 + 4;
			MakeOutgoingFrame(pFrameBuf,		&TmpLen,
							1,				&CountryIe,
							1,				&TmpLen3,
							3,				pAd->CommonCfg.CountryCode,
							TmpLen2+1,		TmpFrame,
							END_OF_ARGS);
		}
		else
		{
			UCHAR	TmpLen3 = TmpLen2+3;
			MakeOutgoingFrame(pFrameBuf,			&TmpLen,
								1,				&CountryIe,
								1,				&TmpLen3,
								3,				pAd->CommonCfg.CountryCode,
								TmpLen2,			TmpFrame,
								END_OF_ARGS);
		}

		*pFrameLen = *pFrameLen + TmpLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSupportChannelIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	if (pAd->StaCfg.TdlsInfo.TdlsChSwitchSupp)
	{
		UCHAR SupportChIe = IE_SUPP_CHANNELS;
		UCHAR ch_set[32], list_len;
		ULONG buf_len;

		NdisZeroMemory(ch_set, sizeof(ch_set));
		list_len = 0;
		if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
		{
			ch_set[list_len] = 1; // first channel
			ch_set[list_len+1] = 11; // channel number
			list_len += 2;
		}

		if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
		{
			ch_set[list_len] = 36; // first channel
			ch_set[list_len+1] = 8; // channel number
			list_len += 2;

			ch_set[list_len] = 149; // first channel
			ch_set[list_len+1] = 4; // channel number
			list_len += 2;
		}

		if (list_len > 0) {
			MakeOutgoingFrame(pFrameBuf, &buf_len,
								1,	&SupportChIe,
								1,	&list_len,
								list_len,	&ch_set[0],
								END_OF_ARGS);
			*pFrameLen = *pFrameLen + buf_len;
		}
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertExtRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if (pAd->StaActive.ExtRateLen != 0)
	{
		MakeOutgoingFrame(pFrameBuf,					&TempLen,
							1,							&ExtRateIe,
							1,						&pAd->StaActive.ExtRateLen,
							pAd->StaActive.ExtRateLen,	pAd->StaActive.ExtRate,							
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertQosCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	/* if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) || (pAd->CommonCfg.bWmmCapable)) */
	if (pAd->CommonCfg.bWmmCapable)
	{
		UCHAR QOS_CAP_IE = 46;
		UCHAR QOS_CAP_IE_LEN = 1;
		QBSS_STA_INFO_PARM QosInfo;

		NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

		if (pAd->StaCfg.UapsdInfo.bAPSDCapable)
		{
			QosInfo.UAPSD_AC_BE = pAd->CommonCfg.TDLS_bAPSDAC_BE;
			QosInfo.UAPSD_AC_BK = pAd->CommonCfg.TDLS_bAPSDAC_BK;
			QosInfo.UAPSD_AC_VI = pAd->CommonCfg.TDLS_bAPSDAC_VI;
			QosInfo.UAPSD_AC_VO = pAd->CommonCfg.TDLS_bAPSDAC_VO;
			QosInfo.MaxSPLength = pAd->CommonCfg.TDLS_MaxSPLength;
		}

		MakeOutgoingFrame(pFrameBuf,	&TempLen,
						1,				&QOS_CAP_IE,
						1,				&QOS_CAP_IE_LEN,
						1,				&QosInfo,
						END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertWMMIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	BOOLEAN	bEnable)
{
	ULONG TempLen;

	if (pAd->CommonCfg.bWmmCapable)
	{
		QBSS_STA_INFO_PARM QosInfo;
		UCHAR WmeParmIe[8] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01}; 

		NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

		if (pAd->StaCfg.UapsdInfo.bAPSDCapable)
		{
			if (bEnable)
			{
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.TDLS_bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.TDLS_bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.TDLS_bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.TDLS_bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.TDLS_MaxSPLength;
			}
			DBGPRINT(RT_DEBUG_ERROR, ("tdls uapsd> UAPSD %d %d %d %d %d!\n",
					pAd->CommonCfg.TDLS_bAPSDAC_BE,
					pAd->CommonCfg.TDLS_bAPSDAC_BK,
					pAd->CommonCfg.TDLS_bAPSDAC_VI,
					pAd->CommonCfg.TDLS_bAPSDAC_VO,
					pAd->CommonCfg.TDLS_MaxSPLength));
		}

		MakeOutgoingFrame(pFrameBuf,			&TempLen,
							8,				WmeParmIe,
							1,				&QosInfo,
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertEDCAParameterSetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS)
{
	ULONG TempLen;

	if ((WMODE_CAP_N(pAd->CommonCfg.PhyMode) || (pAd->CommonCfg.bWmmCapable)) && (pTDLS->bWmmCapable))
	{
		USHORT	idx;

		/* When the BSS is QoS capable, then the BSS QoS parameters shall be
		 * used by the TDLS peer STAs on the AP's channel, and the values 
		 * indicated inside the TDLS Setup Confirm frame apply only for the 
		 * off-channel. The EDCA parameters for the off-channel should be 
		 * the same as those on the AP's channel when QoS is supported by the BSS, 
		 * because this may optimize the channel switching process.
		 */

		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 

		/*  Reset EdcaParam */
		NdisZeroMemory(&pTDLS->EdcaParm, sizeof(EDCA_PARM));
		/* Enable EdcaParm used in non-QBSS. */
		pTDLS->EdcaParm.bValid = TRUE;

		pTDLS->EdcaParm.bQAck		   = FALSE;
		pTDLS->EdcaParm.bQueueRequest   = FALSE;
		pTDLS->EdcaParm.bTxopRequest    = FALSE;

		WmeParmIe[2] =  ((UCHAR)pTDLS->EdcaParm.bQAck << 4) + 
						((UCHAR)pTDLS->EdcaParm.bQueueRequest << 5) + 
						((UCHAR)pTDLS->EdcaParm.bTxopRequest << 6);

		pTDLS->EdcaParm.EdcaUpdateCount = 1;
		WmeParmIe[8] = pTDLS->EdcaParm.EdcaUpdateCount & 0x0f;

		WmeParmIe[8] |= pTDLS->EdcaParm.bAPSDCapable << 7;

		/* By hardcoded */
		pTDLS->EdcaParm.Aifsn[0] = 3;
		pTDLS->EdcaParm.Aifsn[1] = 7;
		pTDLS->EdcaParm.Aifsn[2] = 2;
		pTDLS->EdcaParm.Aifsn[3] = 2;

		pTDLS->EdcaParm.Cwmin[0] = 4;
		pTDLS->EdcaParm.Cwmin[1] = 4;
		pTDLS->EdcaParm.Cwmin[2] = 3;
		pTDLS->EdcaParm.Cwmin[3] = 2;

		pTDLS->EdcaParm.Cwmax[0] = 10;
		pTDLS->EdcaParm.Cwmax[1] = 10;
		pTDLS->EdcaParm.Cwmax[2] = 4;
		pTDLS->EdcaParm.Cwmax[3] = 3;

		pTDLS->EdcaParm.Txop[0]  = 0;
		pTDLS->EdcaParm.Txop[1]  = 0;
		pTDLS->EdcaParm.Txop[2]  = 96;
		pTDLS->EdcaParm.Txop[3]  = 48;

		for (idx=QID_AC_BE; idx<=QID_AC_VO; idx++)
		{
			WmeParmIe[10+ (idx*4)] = (idx << 5)								+	  /* b5-6 is ACI */
								   ((UCHAR)pTDLS->EdcaParm.bACM[idx] << 4) 	+	  /* b4 is ACM */
								   (pTDLS->EdcaParm.Aifsn[idx] & 0x0f);			  /* b0-3 is AIFSN */
			WmeParmIe[11+ (idx*4)] = (pTDLS->EdcaParm.Cwmax[idx] << 4)		+	  /* b5-8 is CWMAX */
								   (pTDLS->EdcaParm.Cwmin[idx] & 0x0f);			  /* b0-3 is CWMIN */
			WmeParmIe[12+ (idx*4)] = (UCHAR)(pTDLS->EdcaParm.Txop[idx] & 0xff);	  /* low byte of TXOP */
			WmeParmIe[13+ (idx*4)] = (UCHAR)(pTDLS->EdcaParm.Txop[idx] >> 8);	  /* high byte of TXOP */
		}

		MakeOutgoingFrame(pFrameBuf,		&TempLen,
						  26,				WmeParmIe,
						  END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}


/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertWMMParameterIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	if ((WMODE_CAP_N(pAd->CommonCfg.PhyMode)) || (pAd->CommonCfg.bWmmCapable))
	{
		QBSS_STA_INFO_PARM QosInfo;
		ULONG TempLen;
		USHORT	idx;

		/* When the BSS is QoS capable, then the BSS QoS parameters shall be
		 * used by the TDLS peer STAs on the AP's channel, and the values 
		 * indicated inside the TDLS Setup Confirm frame apply only for the 
		 * off-channel. The EDCA parameters for the off-channel should be 
		 * the same as those on the AP's channel when QoS is supported by the BSS, 
		 * because this may optimize the channel switching process.
		 */

		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 


		NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

		if (pAd->StaCfg.UapsdInfo.bAPSDCapable)
		{

			QosInfo.UAPSD_AC_BE = pAd->CommonCfg.TDLS_bAPSDAC_BE;
			QosInfo.UAPSD_AC_BK = pAd->CommonCfg.TDLS_bAPSDAC_BK;
			QosInfo.UAPSD_AC_VI = pAd->CommonCfg.TDLS_bAPSDAC_VI;
			QosInfo.UAPSD_AC_VO = pAd->CommonCfg.TDLS_bAPSDAC_VO;
			QosInfo.MaxSPLength = pAd->CommonCfg.TDLS_MaxSPLength;

			DBGPRINT(RT_DEBUG_ERROR, ("tdls uapsd> UAPSD %d %d %d %d %d!\n",
					pAd->CommonCfg.TDLS_bAPSDAC_BE,
					pAd->CommonCfg.TDLS_bAPSDAC_BK,
					pAd->CommonCfg.TDLS_bAPSDAC_VI,
					pAd->CommonCfg.TDLS_bAPSDAC_VO,
					pAd->CommonCfg.TDLS_MaxSPLength));
		}

		WmeParmIe[8] |= *(PUCHAR)&QosInfo;

		for (idx=QID_AC_BE; idx<=QID_AC_VO; idx++)
		{
			WmeParmIe[10+ (idx*4)] = (idx << 5)								+	  // b5-6 is ACI
								   ((UCHAR)pAd->CommonCfg.APEdcaParm.bACM[idx] << 4) 	+	  // b4 is ACM
								   (pAd->CommonCfg.APEdcaParm.Aifsn[idx] & 0x0f);			  // b0-3 is AIFSN
			WmeParmIe[11+ (idx*4)] = (pAd->CommonCfg.APEdcaParm.Cwmax[idx] << 4)		+	  // b5-8 is CWMAX
								   (pAd->CommonCfg.APEdcaParm.Cwmin[idx] & 0x0f);			  // b0-3 is CWMIN
			WmeParmIe[12+ (idx*4)] = (UCHAR)(pAd->CommonCfg.APEdcaParm.Txop[idx] & 0xff);	  // low byte of TXOP
			WmeParmIe[13+ (idx*4)] = (UCHAR)(pAd->CommonCfg.APEdcaParm.Txop[idx] >> 8);	  // high byte of TXOP
		}

		MakeOutgoingFrame(pFrameBuf,			&TempLen,
							26,				WmeParmIe,
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

#ifdef DOT11_N_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertHtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		UCHAR HtLen;
		HT_CAPABILITY_IE HtCapabilityTmp;

		HtLen = sizeof(HT_CAPABILITY_IE);
#ifndef RT_BIG_ENDIAN
		NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
		NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
		HtCapabilityTmp.HtCapInfo.ChannelWidth = pAd->CommonCfg.RegTransmitSetting.field.BW;
		
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
							1,			&HtCapIe,
							1,			&HtLen,
							HtLen,		&HtCapabilityTmp, 
							END_OF_ARGS);
#else
		NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
		NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
		HtCapabilityTmp.HtCapInfo.ChannelWidth = pAd->CommonCfg.RegTransmitSetting.field.BW;

		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));

		MakeOutgoingFrame(pFrameBuf,	&TempLen,
							1,			&HtCapIe,
							1,			&HtLen,
							HtLen,		&HtCapabilityTmp, 
							END_OF_ARGS);
#endif

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}

#ifdef DOT11N_DRAFT3
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
/* 20/40 BSS Coexistence (7.3.2.61) */
VOID
TDLS_InsertBSSCoexistenceIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;

	//if (pAd->CommonCfg.BACapability.field.b2040CoexistScanSup == 1)
	{
		UCHAR Length = 1;
		BSS_2040_COEXIST_IE BssCoexistence;

		memset(&BssCoexistence, 0, sizeof(BSS_2040_COEXIST_IE));
		BssCoexistence.field.InfoReq = 1;

		MakeOutgoingFrame(pFrameBuf,		&TempLen,
							1,				&BssCoexistIe,
							1,				&Length,
							1,				&BssCoexistence.word,							
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertExtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;
	UCHAR Length = sizeof(EXT_CAP_INFO_ELEMENT);
	EXT_CAP_INFO_ELEMENT	extCapInfo;

	Length = sizeof(EXT_CAP_INFO_ELEMENT);

	NdisZeroMemory(&extCapInfo, Length);
			
#ifdef DOT11N_DRAFT3
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) && 
		(WMODE_CAP_N(pAd->CommonCfg.PhyMode)) &&
		(pAd->CommonCfg.Channel <= 14))
	{
			extCapInfo.BssCoexistMgmtSupport = 1;
	}
#endif // DOT11N_DRAFT3 //

	if (pAd->StaCfg.TdlsInfo.TdlsChSwitchSupp)
		extCapInfo.TDLSChSwitchSupport = 1;

	if (pAd->StaCfg.UapsdInfo.bAPSDCapable)
		extCapInfo.UAPSDBufSTASupport = 1;

	extCapInfo.TDLSSupport = 1;

		MakeOutgoingFrame(pFrameBuf,		&TempLen,
							1,				&ExtCapIe,
							1,				&Length,
					sizeof(EXT_CAP_INFO_ELEMENT),	&extCapInfo,
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_InsertFTIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce)
{
	ULONG TempLen;
	UINT16 MICCtrBuf;
	UCHAR FTIE = IE_FT_FTIE;

	MICCtrBuf = cpu2le16(MICCtr.word);
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&FTIE,
						1,				&Length,
						2,				(PUCHAR)&MICCtrBuf,
						16,				(PUCHAR)pMic,
						32,				(PUCHAR)pANonce,
						32,				(PUCHAR)pSNonce,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_InsertTimeoutIntervalIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue)
{
	ULONG TempLen;
	UINT8 Length;
	UINT8 TimeOutIntervalIE;
	UINT8 TimeoutType;
	UINT32 TimeoutValueBuf;

	Length = 5;
	TimeOutIntervalIE = IE_FT_TIMEOUT_INTERVAL;
	TimeoutType = Type;
	TimeoutValueBuf = cpu2le32(TimeOutValue);

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&TimeOutIntervalIE,
						1,				&Length,
						1,				(PUCHAR)&TimeoutType,
						4,				(PUCHAR)&TimeoutValueBuf,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertTargetChannel(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	TargetCh)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,			&TempLen,
						1,				&TargetCh,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertRegulatoryClass(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	TargetCh,
	IN	UINT8	ChWidth)
{
	ULONG TempLen;
	UCHAR regclass;
	UCHAR BandWidth = 0;

	if (ChWidth != 0)
		BandWidth = 1;

	regclass = TDLS_GetRegulatoryClass(pAd, BandWidth, TargetCh);

		MakeOutgoingFrame(pFrameBuf,			&TempLen,
							1,				&regclass,
							END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InsertSecondaryChOffsetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UCHAR	Offset)
{
	ULONG TempLen;
	UCHAR length = 1;

	MakeOutgoingFrame(pFrameBuf,			&TempLen,
						1,				&NewExtChanIe,
						1,				&length,
						1,				&Offset,							
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


VOID
TDLS_InsertChannelSwitchTimingIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	USHORT	SwitchTime,
	IN	USHORT	SwitchTimeOut)
{
	ULONG TempLen;
	UCHAR TDLS_IE = IE_TDLS_CHANNEL_SWITCH_TIMING;
	UCHAR TDLS_IE_LEN = 4;
	UINT16	SwitchTimeBuf = cpu2le16(SwitchTime);
	UINT16	SwitchTimeOutBuf = cpu2le16(SwitchTimeOut); 

	MakeOutgoingFrame(pFrameBuf,			&TempLen,
						1,				&TDLS_IE,
						1,				&TDLS_IE_LEN,
						2,				&SwitchTimeBuf,
						2,				&SwitchTimeOutBuf,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_SupportedRegulatoryClasses(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen)
{
	ULONG TempLen;
	UCHAR TDLS_IE = IE_SUPP_REG_CLASS;
	//UCHAR Length = 6;
	//UCHAR SuppClassesList[] = {1,2,3,32,33};
	UCHAR Length = 16;
	UCHAR SuppClassesList[] = {1, 2, 3, 4, 12, 22, 23, 24, 25, 27, 28, 29, 30, 32, 33};
	UCHAR regclass;

	regclass = TDLS_GetRegulatoryClass(pAd, pAd->CommonCfg.RegTransmitSetting.field.BW, pAd->CommonCfg.Channel);

	MakeOutgoingFrame(pFrameBuf,			&TempLen,
						1,				&TDLS_IE,
						1,				&Length,
						1,				&regclass,
						15,				SuppClassesList,							
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

#ifdef UAPSD_SUPPORT
VOID TDLS_InsertPuBufferStatus(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	UCHAR						*pPeerMac)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	ULONG TempLen;
	UINT8 Length;
	UINT8 IeIdPuBufferStatus;
	UINT8 PuBufferStatus;
	UINT8 FlgIsAnyPktForBK, FlgIsAnyPktForBE;
	UINT8 FlgIsAnyPktForVI, FlgIsAnyPktForVO;


	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, pPeerMac);

	if (pMacEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
				__FUNCTION__));
		return;
	}

	/* init */
	Length = 1;
	IeIdPuBufferStatus = IE_TDLS_PU_BUFFER_STATUS;
	PuBufferStatus = 0;

	/* get queue status */
	UAPSD_QueueStatusGet(pAd, pMacEntry,
						&FlgIsAnyPktForBK, &FlgIsAnyPktForBE,
						&FlgIsAnyPktForVI, &FlgIsAnyPktForVO);
	PuBufferStatus |= (FlgIsAnyPktForBK == TRUE)? 0x01: 0x00;
	PuBufferStatus |= (FlgIsAnyPktForBE == TRUE)? 0x02: 0x00;
	PuBufferStatus |= (FlgIsAnyPktForVI == TRUE)? 0x04: 0x00;
	PuBufferStatus |= (FlgIsAnyPktForVO == TRUE)? 0x08: 0x00;

	/* init element */
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&IeIdPuBufferStatus,
						1,				&Length,
						1,				&PuBufferStatus,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}
#endif /* UAPSD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */

