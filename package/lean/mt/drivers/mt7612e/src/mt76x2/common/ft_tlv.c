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
    ft_tlv.c
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Fonchi Wu  12-02-2008    created for 11r soft-AP
 */

#ifdef DOT11R_FT_SUPPORT


#include "rt_config.h"
#include "dot11r_ft.h"


VOID FT_InsertMdIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 pMdId,
	IN FT_CAP_AND_POLICY FtCapPlc)
{

	ULONG TempLen;
	UINT8 Length;
	UCHAR MDIE = IE_FT_MDIE;

	Length = 3;
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&MDIE,
						1,				&Length,
						2,				(PUCHAR)pMdId,
						1,				(PUCHAR)&FtCapPlc.word,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return; 
}

VOID FT_InsertFTIE(
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

VOID FT_FTIE_InsertKhIdSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_SUB_ELEMENT_ID SubId,
	IN PUINT8 pKhId,
	IN UINT8 KhIdLen)
{
	ULONG TempLen;

	if (SubId != FT_R0KH_ID && SubId != FT_R1KH_ID)
	{
		DBGPRINT(RT_DEBUG_ERROR,("(%s): unknow SubId (%d)\n",
			__FUNCTION__, SubId));
		return;
	}

	/* The lenght of R1KHID must only be 6 octects. */
	if ((SubId == FT_R1KH_ID) && (KhIdLen != 6))
	{
		DBGPRINT(RT_DEBUG_ERROR,("(%s): Invalid R1KHID length (%d)\n",
			__FUNCTION__, KhIdLen));
		return;
	}

	/* The length of R0KHID must in range of 1 to 48 octects.*/
	if ((SubId == FT_R0KH_ID) && ((KhIdLen > 48) && (KhIdLen < 1))) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("(%s): Invalid R0KHID length (%d)\n",
			__FUNCTION__, KhIdLen));
	}		

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&KhIdLen,
						KhIdLen,		(PUCHAR)pKhId,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;	
}

VOID FT_FTIE_InsertGTKSubIE(
	IN PRTMP_ADAPTER pAd,
	IN 	PUCHAR 	pFrameBuf,
	OUT PULONG pFrameLen,
	IN 	PUINT8 	pGtkSubIe,
	IN 	UINT8 	GtkSubIe_len)
{
	ULONG TempLen;
	UINT8 Length;
	UINT8 SubId;

	SubId = FT_GTK;
	Length = GtkSubIe_len;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Length,
						Length,			pGtkSubIe,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;	
}


VOID FT_InsertTimeoutIntervalIE(
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



#endif /* DOT11R_FT_SUPPORT */

