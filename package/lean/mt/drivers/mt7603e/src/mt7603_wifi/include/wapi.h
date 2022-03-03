/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2005, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attempt
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wapi.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Albert		2008-4-3      	Supoort WAPI protocol
*/

#ifndef __WAPI_H__
#define __WAPI_H__

#include "wpa_cmm.h"

/* Increase TxIV value for next transmission */
/* Todo - When overflow occurred, do re-key mechanism */
#define	INC_TX_IV(_V, NUM)					\
{											\
	UCHAR	cnt = LEN_WAPI_TSC;				\
	do										\
	{										\
		cnt--;								\
		_V[cnt] = _V[cnt] + NUM;			\
		if (cnt == 0)						\
		{									\
			DBGPRINT(RT_DEBUG_TRACE, ("PN overflow!!!!\n"));	\
			break;							\
		}									\
	}while (_V[cnt] == 0);					\
}

#define IS_WAPI_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWAICERT) && ((a) <= Ndis802_11AuthModeWAIPSK))

/* The underlying chip supports hardware-based WPI-SMS4 encryption and de-encryption. */
#define IS_HW_WAPI_SUPPORT(__pAd)		(__pAd->chipCap.FlgIsHwWapiSup)
/* 
 =====================================	
 	function prototype in wapi_crypt.c
 =====================================	
*/
int wpi_cbc_mac_engine(
		unsigned char * maciv_in,
		unsigned char * in_data1,
		unsigned int 	in_data1_len,
		unsigned char * in_data2,
		unsigned int 	in_data2_len,
		unsigned char * pkey,
		unsigned char * mac_out);

int wpi_sms4_ofb_engine(
	unsigned char * pofbiv_in,
	unsigned char * pbw_in,
	unsigned int 	plbw_in,
	unsigned char * pkey,
	unsigned char * pcw_out);

VOID RTMPInsertWapiIe(
	IN	UINT			AuthMode,
	IN	UINT			WepStatus,
	OUT	PUCHAR			pWIe,
	OUT	UCHAR			*w_len);

BOOLEAN RTMPCheckWAIframe(
    IN PUCHAR           pData,
    IN ULONG            DataByteCount);

VOID RTMPConstructWPIIVHdr(
	IN	UCHAR			key_id,
	IN	UCHAR			*tx_iv, 
	OUT UCHAR 			*iv_hdr);

//#ifdef RTMP_RBUS_SUPPORT
INT RTMPSoftEncryptSMS4(
	IN	PUCHAR			pHeader,
	IN  PUCHAR			pData,
	IN	UINT32			data_len,				
	IN	UCHAR			key_id,
	IN	PUCHAR 			pKey,
	IN	PUCHAR			pIv);

INT RTMPSoftDecryptSMS4(
	IN		PUCHAR			pHdr,
	IN		BOOLEAN			bSanityIV,
	IN 		PCIPHER_KEY		pKey,
	INOUT 	PUCHAR			pData,
	INOUT 	UINT16			*DataByteCnt);

VOID RTMPDeriveWapiGTK(
	IN	PUCHAR			nmk,
	OUT	PUCHAR			gtk_ptr);

VOID RT_SMS4_TEST(
	IN UINT8			test);

INT SMS4_TEST(void);

/* 
 =====================================	
 	function prototype in wapi.c
 =====================================	
*/

BOOLEAN RTMPIsWapiCipher(
    IN PRTMP_ADAPTER    pAd,
    IN UCHAR           	apidx);

VOID RTMPIoctlQueryWapiConf(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

void rtmp_read_wapi_parms_from_file(
		IN  PRTMP_ADAPTER pAd, 
		char *tmpbuf, 
		char *buffer);

VOID RTMPWapiUskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID RTMPWapiMskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID RTMPInitWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPStartWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID RTMPCancelWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID RTMPGetWapiTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UINT			Wcid,
	OUT	UCHAR			*tx_tsc);

VOID WAPIInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	PMAC_TABLE_ENTRY	pEntry,
	BOOLEAN				bAE);

VOID WAPIInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	PUINT8				pGtk);

BOOLEAN WAPI_InternalCmdAction(
		IN  PRTMP_ADAPTER		pAd,
		IN	UCHAR				AuthMode,
		IN	UCHAR				apidx,
		IN	PUCHAR				pAddr,
		IN	UCHAR				flag);

#endif /* __WAPI_H__ */

