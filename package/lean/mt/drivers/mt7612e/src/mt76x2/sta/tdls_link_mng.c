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

UCHAR	TdlsZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

UCHAR	CipherSuiteTDLSWpa2PskAes[] = {
		0x30,					// RSN IE
		0x14,					// Length	
		0x01, 0x00,				// Version
		0x00, 0x0F, 0xAC, 0x07,	// no group cipher
		0x01, 0x00,				// number of pairwise
		0x00, 0x0f, 0xAC, 0x04,	// unicast, AES
		0x01, 0x00,				// number of authentication method
		0x00, 0x0f, 0xAC, TDLS_AKM_SUITE_PSK,	// TDLS authentication
		0x00, 0x02,				// RSN capability, peer key enabled
		};
UCHAR	CipherSuiteTDLSLen = sizeof(CipherSuiteTDLSWpa2PskAes)/ sizeof(UCHAR);

/*
    ==========================================================================
    Description:
        dls state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the dls state machine
    Note:
        The state machine looks like this
        
                            DLS_IDLE
    MT2_MLME_DLS_REQUEST   MlmeDlsReqAction
    MT2_PEER_DLS_REQUEST   PeerDlsReqAction
    MT2_PEER_DLS_RESPONSE  PeerDlsRspAction
    MT2_MLME_DLS_TEARDOWN  MlmeTearDownAction
    MT2_PEER_DLS_TEARDOWN  PeerTearDownAction
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID TDLS_StateMachineInit(
	IN PRTMP_ADAPTER pAd, 
	IN STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	UCHAR i;
	PRT_802_11_TDLS	pTDLS = NULL;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)MAX_TDLS_STATE,
		(ULONG)MAX_TDLS_MSG, (STATE_MACHINE_FUNC)Drop, TDLS_IDLE, TDLS_MACHINE_BASE);
     
	/* the first column */
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_SETUP_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeSetupReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_REQ, (STATE_MACHINE_FUNC)TDLS_PeerSetupReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_RSP, (STATE_MACHINE_FUNC)TDLS_PeerSetupRspAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_CONF,(STATE_MACHINE_FUNC)TDLS_PeerSetupConfAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_TEAR_DOWN, (STATE_MACHINE_FUNC)TDLS_MlmeTearDownAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_TEAR_DOWN, (STATE_MACHINE_FUNC)TDLS_PeerTearDownAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_DISCOVER_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeDiscoveryReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_DISCOVER_REQ, (STATE_MACHINE_FUNC)TDLS_PeerDiscoveryReqAction);
#ifdef WFD_SUPPORT
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_TUNNELED_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeTunneledReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_TUNNELED_REQ, (STATE_MACHINE_FUNC)TDLS_PeerTunneledReqRspAction);
#endif /* WFD_SUPPORT */

	for (i = 0; i < MAX_NUMBER_OF_DLS_ENTRY; i++)
	{
		pAd->StaCfg.TdlsInfo.TDLSEntry[i].pAd = pAd;
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[i];

		RTMPInitTimer(pAd, &pTDLS->Timer, GET_TIMER_FUNCTION(TDLS_LinkTimeoutAction), pTDLS, FALSE);
	}

#ifdef UAPSD_SUPPORT
	TDLS_UAPSDP_Init(pAd, Sm);
#endif /* UAPSD_SUPPORT */
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupRequest(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS)
{
	ULONG			Timeout = TDLS_TIMEOUT;
	BOOLEAN			TimerCancelled;

	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_REQUEST);

	/* fill Dialog Token */
	pAd->StaCfg.TdlsInfo.TdlsDialogToken++;
	if (pAd->StaCfg.TdlsInfo.TdlsDialogToken == 0)
		pAd->StaCfg.TdlsInfo.TdlsDialogToken++;

	pTDLS->Token = pAd->StaCfg.TdlsInfo.TdlsDialogToken;
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	// fill capability
	TDLS_InsertCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill support rate
	TDLS_InsertSupportRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill country 
	TDLS_InsertCountryIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill ext rate
	TDLS_InsertExtRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill support channels
	TDLS_InsertSupportChannelIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill RSN if security is not NONE
	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{		
		UCHAR			CipherTmp[64] = {0};
		UCHAR			CipherTmpLen = 0;
		ULONG			tmp;
		
		// RSNIE (7.3.2.25)
		CipherTmpLen = CipherSuiteTDLSLen;	
			NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);
		
		// Insert RSN_IE to outgoing frame
		MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
				CipherTmpLen,						&CipherTmp,
				END_OF_ARGS);

		*pFrameLen = *pFrameLen + tmp;
	}

	// fill  Extended Capabilities (7.3.2.27)
	TDLS_InsertExtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
	
	// fill Qos Capability
	//TDLS_InsertQosCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
			
	// TPK Handshake if RSNA Enabled
	// Pack TPK Message 1 here! 
	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{
		FT_FTIE			FtIe;
		UINT32			KeyLifetime = pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime;	// sec
		UCHAR			Length;

		// FTIE (7.3.2.48)
		NdisZeroMemory(&FtIe, sizeof(FtIe));
		Length =  sizeof(FtIe);

		// generate SNonce
		GenRandom(pAd, pAd->CurrentAddress, FtIe.SNonce);
		hex_dump("TDLS - Generate SNonce ", FtIe.SNonce, 32);
		NdisMoveMemory(pTDLS->SNonce, FtIe.SNonce, 32);
		
		TDLS_InsertFTIE(pAd, 
				(pFrameBuf + *pFrameLen), 
				pFrameLen, 
				Length, 
				FtIe.MICCtr,
				FtIe.MIC, 
				FtIe.ANonce, 
				FtIe.SNonce);

		// Timeout Interval (7.3.2.49)
		TDLS_InsertTimeoutIntervalIE(pAd, 
				(pFrameBuf + *pFrameLen), 
				pFrameLen, 
				2, /* key lifetime interval */
				KeyLifetime);

		pTDLS->KeyLifetime = KeyLifetime;
	}

	// fill Supported Regulatory Classes
	TDLS_SupportedRegulatoryClasses(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

#ifdef DOT11_N_SUPPORT
	// fill HT Capability
	TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill 20/40 BSS Coexistence (7.3.2.61)
#ifdef DOT11N_DRAFT3
	TDLS_InsertBSSCoexistenceIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	// fill WMM
	TDLS_InsertWMMIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TRUE);

#ifdef WFD_SUPPORT
	{
		ULONG	WfdIeLen = 0, WfdIeBitmap;

		WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
			(0x1 << SUBID_WFD_COUPLED_SINK_INFO) | (0x1 << SUBID_WFD_LOCAL_IP_ADDR);
		WfdMakeWfdIE(pAd, WfdIeBitmap, (pFrameBuf + *pFrameLen), &WfdIeLen);
		*pFrameLen += WfdIeLen;
	}
#endif /* WFD_SUPPORT */

	// ==>> Set sendout timer
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
	RTMPSetTimer(&pTDLS->Timer, Timeout);

	// ==>> State Change
	pTDLS->Status = TDLS_MODE_WAIT_RESPONSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_RESPONSE);

	/* fill status code */
	TDLS_InsertStatusCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, StatusCode);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	if (StatusCode == MLME_SUCCESS)
	{
		// fill capability
		TDLS_InsertCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill support rate
		TDLS_InsertSupportRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill country 
		TDLS_InsertCountryIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill ext rate
		TDLS_InsertExtRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill support channels
		TDLS_InsertSupportChannelIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill Qos Capability
		//TDLS_InsertQosCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);


		// fill RSN 
		if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
		{		
			ULONG	tmp;

			// RSNIE (7.3.2.25)			
			// Insert RSN_IE of the Peer TDLS to outgoing frame
			MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
					RsnLen,								pRsnIe,
					END_OF_ARGS);
			*pFrameLen = *pFrameLen + tmp;
		}	

		// fill  Extended Capabilities (7.3.2.27)
		TDLS_InsertExtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill WMM
		TDLS_InsertWMMIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen,TRUE);

		// TPK Handshake if RSNA Enabled
		// Pack TPK Message 2 here! 
		if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
		{		
			FT_FTIE	*ft;
			UINT	key_len = 16;

			// FTIE (7.3.2.48)
			// Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
			
			// point to the element of IE
			ft = (FT_FTIE *)(pFTIe + 2);	
			// generate ANonce
			GenRandom(pAd, pAd->CurrentAddress, ft->ANonce);
			hex_dump("TDLS - Generate ANonce ", ft->ANonce, 32);
			// set MIC field to zero before MIC calculation
			NdisZeroMemory(ft->MIC, 16);
			// copy SNonce from peer TDLS
			NdisMoveMemory(ft->SNonce, pTDLS->SNonce, 32);
			// copy ANonce to TDLS entry
			NdisMoveMemory(pTDLS->ANonce, ft->ANonce, 32);

			// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// Derive TPK-KCK for MIC key, TPK-TK for direct link data
			TDLS_FTDeriveTPK(
					pTDLS->MacAddr,	/* MAC Address of Initiator */
					pAd->CurrentAddress, /* I am Responder */
					pTDLS->ANonce, 
					pTDLS->SNonce, 
					pAd->CommonCfg.Bssid,
					key_len,
					pTDLS->TPK,
					pTDLS->TPKName);

			// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

			////////////////////////////////////////////////////////////////////////
			// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
			// 1. MAC_I (6 bytes)
			// 2. MAC_R (6 bytes)
			// 3. Transaction Sequence = 2 (1 byte)
			// 4. Link Identifier (20 bytes)
			// 5. RSN IE without the IE header (20 bytes)
			// 6. Timeout Interval IE (7 bytes)
			// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)	
			{
				UCHAR	content[512];
				ULONG	c_len = 0;
				ULONG	tmp_len = 0;
				UCHAR	Seq = 2;
				UCHAR	mic[16];
				UCHAR	LinkIdentifier[20];
				UINT	tmp_aes_len = 0;

				NdisZeroMemory(LinkIdentifier, 20);
				LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
				LinkIdentifier[1] = 18;
				NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
				NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
				NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);

				NdisZeroMemory(mic, sizeof(mic));

				/* make a header frame for calculating MIC. */
				MakeOutgoingFrame(content,					&tmp_len,
			                      MAC_ADDR_LEN,				pTDLS->MacAddr,
			                      MAC_ADDR_LEN,				pAd->CurrentAddress,
			                      1,						&Seq,
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate Link Identifier */			
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      20,					LinkIdentifier,
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate RSNIE */
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      RsnLen,					pRsnIe,
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate Timeout Interval IE */
				MakeOutgoingFrame(content + c_len,     &tmp_len,
								  7,					pTIIe,
								  END_OF_ARGS);
				c_len += tmp_len;

				/* concatenate FTIE */			
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      (sizeof(FT_FTIE) + 2),	pFTIe,
			                      END_OF_ARGS);
				c_len += tmp_len;

				/* Calculate MIC */				
				//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

				/* Compute AES-128-CMAC over the concatenation */
				tmp_aes_len = AES_KEY128_LENGTH;
    			AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

				// Fill Mic to ft struct
				NdisMoveMemory(ft->MIC, mic, 16);

			}
			////////////////////////////////////////////////////////////////////////
			
			// Insert FT_IE to outgoing frame
			TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					sizeof(FT_FTIE), 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);

			// Timeout Interval (7.3.2.49)
			// Insert TI_IE to outgoing frame
			TDLS_InsertTimeoutIntervalIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					2, /* key lifetime interval */
					pTDLS->KeyLifetime);
		}	

		// fill Supported Regulatory Classes
		TDLS_SupportedRegulatoryClasses(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

#ifdef DOT11_N_SUPPORT
		// fill HT Capability
		TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill 20/40 BSS Coexistence (7.3.2.61)
#ifdef DOT11N_DRAFT3
		TDLS_InsertBSSCoexistenceIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

		/* fill link identifier */
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);

#ifdef WFD_SUPPORT
		{
			ULONG	WfdIeLen = 0, WfdIeBitmap;

			WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
				(0x1 << SUBID_WFD_COUPLED_SINK_INFO) | (0x1 << SUBID_WFD_LOCAL_IP_ADDR);
			WfdMakeWfdIE(pAd, WfdIeBitmap, (pFrameBuf + *pFrameLen), &WfdIeLen);
			*pFrameLen += WfdIeLen;
		}
#endif /* WFD_SUPPORT */

	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupConfirm(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_CONFIRM);

	/* fill status code */
	TDLS_InsertStatusCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, StatusCode);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	// fill Qos Capability
	//TDLS_InsertEDCAParameterSetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS);

	// TPK Handshake if RSNA Enabled
	// Pack TPK Message 3 here! 
	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{		
		FT_FTIE	*ft;
		ULONG	tmp;
			
		// RSNIE (7.3.2.25)			
		// Insert RSN_IE of the Peer TDLS to outgoing frame
		MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
							RsnLen,					pRsnIe,
							END_OF_ARGS);
		*pFrameLen = *pFrameLen + tmp;
		
				
		// FTIE (7.3.2.48)
		// Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
	
		// point to the element of IE
		ft = (FT_FTIE *)(pFTIe + 2);	
		// set MIC field to zero before MIC calculation
		NdisZeroMemory(ft->MIC, 16);

		////////////////////////////////////////////////////////////////////////
		// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
		// 1. MAC_I (6 bytes)
		// 2. MAC_R (6 bytes)
		// 3. Transaction Sequence = 2 (1 byte)
		// 4. Link Identifier (20 bytes)
		// 5. RSN IE without the IE header (20 bytes)
		// 6. Timeout Interval IE (7 bytes)
		// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)	
		{
			UCHAR	content[512];
			ULONG	c_len = 0;
			ULONG	tmp_len = 0;
			UCHAR	Seq = 3;
			UCHAR	mic[16];
			UCHAR	LinkIdentifier[20];
			UINT	tmp_aes_len = 0;

			NdisZeroMemory(LinkIdentifier, 20);
			LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
			LinkIdentifier[1] = 18;
			NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
			NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
			NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);

			NdisZeroMemory(mic, sizeof(mic));

			/* make a header frame for calculating MIC. */
			MakeOutgoingFrame(content,					&tmp_len,
								MAC_ADDR_LEN,			pAd->CurrentAddress,
								MAC_ADDR_LEN,			pTDLS->MacAddr,
								1,						&Seq,
								END_OF_ARGS);
			c_len += tmp_len;					

			/* concatenate Link Identifier */			
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								20,					LinkIdentifier,
								END_OF_ARGS);
			c_len += tmp_len;					

				
			/* concatenate RSNIE */
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								RsnLen,					pRsnIe,
								END_OF_ARGS);
			c_len += tmp_len;					

			/* concatenate Timeout Interval IE */
			MakeOutgoingFrame(content + c_len,     &tmp_len,
								7,					pTIIe,
								END_OF_ARGS);
			c_len += tmp_len;

			/* concatenate FTIE */			
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								(sizeof(FT_FTIE) + 2),	pFTIe,
								END_OF_ARGS);
			c_len += tmp_len;	

			/* Calculate MIC */				
			//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

			/* Compute AES-128-CMAC over the concatenation */
			tmp_aes_len = AES_KEY128_LENGTH;
    		AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

			// Fill Mic to ft struct
			NdisMoveMemory(ft->MIC, mic, 16);
		}
		////////////////////////////////////////////////////////////////////////
	
		// Insert FT_IE to outgoing frame
		TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					sizeof(FT_FTIE), 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);

		// Timeout Interval (7.3.2.49)
		// Insert TI_IE to outgoing frame
		TDLS_InsertTimeoutIntervalIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					2, /* key lifetime interval */
					pTDLS->KeyLifetime);

	}

#ifdef DOT11_N_SUPPORT
	// fill HT Capability
	TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11_N_SUPPORT //

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	// fill WMM Parameter IE
	TDLS_InsertWMMParameterIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

#ifdef WFD_SUPPORT
	{
		ULONG	WfdIeLen = 0, WfdIeBitmap;

		WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
			(0x1 << SUBID_WFD_COUPLED_SINK_INFO) | (0x1 << SUBID_WFD_LOCAL_IP_ADDR);
		WfdMakeWfdIE(pAd, WfdIeBitmap, (pFrameBuf + *pFrameLen), &WfdIeLen);
		*pFrameLen += WfdIeLen;
	}
#endif /* WFD_SUPPORT */

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildTeardown(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS,
	IN	UINT16	ReasonCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_TEARDOWN);

	/* fill reason code */
	TDLS_InsertReasonCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ReasonCode);

	// FTIE includes if RSNA Enabled
	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{	
		UCHAR		FTIe[128];	
		FT_FTIE		*ft = NULL;
		UCHAR		content[256];
		ULONG		c_len = 0;
		ULONG		tmp_len = 0;
		UCHAR		seq = 4;
		UCHAR		mic[16];
		UCHAR		LinkIdentifier[20];
		UINT		tmp_aes_len = 0;

		NdisZeroMemory(LinkIdentifier, 20);
		LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
		LinkIdentifier[1] = 18;

		NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
		if (pTDLS->bInitiator)
		{
			NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
			NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);
		}
		else
		{
			NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
			NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
		}


		// FTIE (7.3.2.48)
		// The contents of FTIE in the TDLS Teardown frame shall be the same as that included
		// in the TPK Handshake Message3 with the exception of the MIC field.

		/* Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce) */
			
		// point to the element of IE
		NdisZeroMemory(FTIe, sizeof(FTIe));
		FTIe[0] = IE_FT_FTIE;
		FTIe[1] = sizeof(FT_FTIE);
		ft = (PFT_FTIE)&FTIe[2];	
		NdisMoveMemory(ft->ANonce, pTDLS->ANonce, 32);
		NdisMoveMemory(ft->SNonce, pTDLS->SNonce, 32);

		////////////////////////////////////////////////////////////////////////
		// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
		// 1. Link Identifier (20 bytes)
		// 2. Reason Code (2 bytes)
		// 3. Dialog token (1 byte)
		// 4. Transaction Sequence = 4 (1 byte)
		// 5. FTIE with the MIC field of FTIE set to zero (84 bytes)	
		
		/* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
		MakeOutgoingFrame(content,            		&tmp_len,
						sizeof(LinkIdentifier),		LinkIdentifier,	
						2,							&ReasonCode,
						1,							&pTDLS->Token,
						1,							&seq,
						END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate FTIE */
		MakeOutgoingFrame(content + c_len,		&tmp_len,
							FTIe[1] + 2,		FTIe,  
						END_OF_ARGS);
		c_len += tmp_len;					
		
		/* Calculate MIC */
		NdisZeroMemory(mic, sizeof(mic));
		//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);
		
		/* Fill Mic to ft struct */
		NdisMoveMemory(ft->MIC, mic, 16);
		////////////////////////////////////////////////////////////////////////

		// Insert FT_IE to outgoing frame
		TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					FTIe[1], 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);
	}

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg1Process(
	IN	PRTMP_ADAPTER		pAd, 
#ifdef WFD_SUPPORT
	IN	ULONG				WfdSubelementLen,
#endif /* WFD_SUPPORT */
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen)
{
	USHORT			StatusCode = MLME_SUCCESS;
	UCHAR			CipherTmp[64] = {0};
	UCHAR			CipherTmpLen = 0;
	FT_FTIE			*ft = NULL;

	/* Validate RsnIE */
	/* */
	if (RsnLen == 0) /* RSN not exist */
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) /* Smaller version */
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	//if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
	if (((pRsnIe[20] & 0x2) != 0) || ((pRsnIe[21] & 0x2) == 0))
		return MLME_INVALID_RSN_CAPABILITIES;

	//if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
	// todo make check
	//if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, (RsnLen - 2)) == 0))
	//	return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->MIC, TdlsZeroSsid, 16) == 0) || (RTMPEqualMemory(ft->ANonce, TdlsZeroSsid, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	//if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < keyLifeTime))
	if ((TILen != 7) || (pTIIe[2] != 2))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;

	return StatusCode;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg2Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen,
	OUT	PUCHAR				pTPK,
	OUT PUCHAR				pTPKName)
{
	USHORT		StatusCode = MLME_SUCCESS;
	UCHAR		CipherTmp[64] = {0};
	UCHAR		CipherTmpLen = 0;
	FT_FTIE		*ft = NULL;			
	UCHAR		oldMic[16];
	UCHAR		LinkIdentifier[20];
	UINT		key_len = 16;
	UINT32	keyLifeTime = pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime;
	
	/* Validate RsnIE */
	/* */
	if (RsnLen == 0) /* RSN not exist */
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) /* Smaller version */
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	//if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
	if (((pRsnIe[20] & 0x2) != 0) || ((pRsnIe[21] & 0x2) == 0))
		return MLME_INVALID_RSN_CAPABILITIES;

	// todo make check
	//if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
	//if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, (RsnLen - 2)) == 0))
		//return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < keyLifeTime))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;


	// Validate the MIC field of FTIE
	//
	
	// point to the element of IE
	ft = (PFT_FTIE)(pFTIe + 2);	
	// backup MIC fromm the peer TDLS
	NdisMoveMemory(oldMic, ft->MIC, 16);

	
	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/* Derive TPK-KCK for MIC key, TPK-TK for direct link data */
	TDLS_FTDeriveTPK(
			pAd->CurrentAddress, /* I am Initiator */
			pTDLS->MacAddr,	/* MAC Address of Responder */
			ft->ANonce, 
			ft->SNonce, 
			pAd->CommonCfg.Bssid,
			key_len,
			pTPK,
			pTPKName);
	
	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	
	/* set MIC field to zero before MIC calculation */
	NdisZeroMemory(ft->MIC, 16);

	/* Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC) */
	NdisZeroMemory(LinkIdentifier, 20);
	LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
	LinkIdentifier[1] = 18;
	NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
	NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
	NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);

	/*////////////////////////////////////////////////////////////////////*/
	/* The MIC field of FTIE shall be calculated on the concatenation, in the following order, of */
	/* 1. MAC_I (6 bytes) */
	/* 2. MAC_R (6 bytes) */
	/* 3. Transaction Sequence = 2 (1 byte) */
	/* 4. Link Identifier (20 bytes) */
	/* 5. RSN IE without the IE header (20 bytes) */
	/* 6. Timeout Interval IE (7 bytes) */
	/* 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes) */
	{
		UCHAR	content[512];
		ULONG	c_len = 0;
		ULONG	tmp_len = 0;
		UCHAR	Seq = 2;
		UCHAR	mic[16];
		UINT	tmp_aes_len = 0;
		
		NdisZeroMemory(mic, sizeof(mic));
			
		/* make a header frame for calculating MIC. */
		MakeOutgoingFrame(content,					&tmp_len,
	                      MAC_ADDR_LEN,				pAd->CurrentAddress,
	                      MAC_ADDR_LEN,				pTDLS->MacAddr,
	                      1,						&Seq,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Link Identifier */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					LinkIdentifier,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate RSNIE */
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
							RsnLen,					pRsnIe,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Timeout Interval IE */
		MakeOutgoingFrame(content + c_len,     &tmp_len,
						  7,					pTIIe,
						  END_OF_ARGS);
		c_len += tmp_len;
		
		/* concatenate FTIE */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
							(sizeof(FT_FTIE) + 2),	pFTIe,
	                      END_OF_ARGS);
		c_len += tmp_len;
		
		/* Calculate MIC */				
		//AES_128_CMAC(pTPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTPK, 16, mic, &tmp_aes_len);

		NdisMoveMemory(ft->MIC, mic, 16);

	}
	/*////////////////////////////////////////////////////////////////////*/
	
	if (RTMPEqualMemory(oldMic, ft->MIC, 16) == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS_TPKMsg2Process() MIC Error!!! \n"));
		return MLME_REQUEST_DECLINED;
	}

	return StatusCode;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg3Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen)
{
	USHORT			StatusCode = MLME_SUCCESS;
	UCHAR			CipherTmp[64] = {0};
	UCHAR			CipherTmpLen = 0;
	FT_FTIE			*ft = NULL;			
	UCHAR			oldMic[16];
	UCHAR			LinkIdentifier[20];
	
	/* Validate RsnIE */
	/* */
	if (RsnLen == 0) /* RSN not exist */
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) /* Smaller version */
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	//if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
	if (((pRsnIe[20] & 0x2) != 0) || ((pRsnIe[21] & 0x2) == 0))
		return MLME_INVALID_RSN_CAPABILITIES;

	// todo make check
	//if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, (RsnLen - 2)) == 0)) // Invalid Pairwise Cipher
		//return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0) || (RTMPEqualMemory(ft->ANonce, pTDLS->ANonce, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < pTDLS->KeyLifetime))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;

	// Validate the MIC field of FTIE
	//

	// point to the element of IE
	ft = (PFT_FTIE)(pFTIe + 2);	
	// backup MIC fromm the peer TDLS
	NdisMoveMemory(oldMic, ft->MIC, 16);

		
	// set MIC field to zero before MIC calculation
	NdisZeroMemory(ft->MIC, 16);

	// Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
	NdisZeroMemory(LinkIdentifier, 20);
	LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
	LinkIdentifier[1] = 18;
	NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
	NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
	NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);

	/*////////////////////////////////////////////////////////////////////*/
	/* The MIC field of FTIE shall be calculated on the concatenation, in the following order, of */
	/* 1. MAC_I (6 bytes) */
	/* 2. MAC_R (6 bytes) */
	/* 3. Transaction Sequence = 3 (1 byte) */
	/* 4. Link Identifier (20 bytes) */
	/* 5. RSN IE without the IE header (20 bytes) */
	/* 6. Timeout Interval IE (7 bytes) */
	/* 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes) */
	{
		UCHAR	content[512];
		ULONG	c_len = 0;
		ULONG	tmp_len = 0;
		UCHAR	Seq = 3;
		UCHAR	mic[16];
		UINT	tmp_aes_len = 0;
		
		NdisZeroMemory(mic, sizeof(mic));
			
		/* make a header frame for calculating MIC. */
		MakeOutgoingFrame(content,					&tmp_len,
	                      MAC_ADDR_LEN,				pTDLS->MacAddr,
	                      MAC_ADDR_LEN,				pAd->CurrentAddress,
	                      1,						&Seq,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Link Identifier */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					LinkIdentifier,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate RSNIE */
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
							RsnLen,				pRsnIe,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Timeout Interval IE */
		MakeOutgoingFrame(content + c_len,     &tmp_len,
						  7,					pTIIe,
						  END_OF_ARGS);
		c_len += tmp_len;
		
		/* concatenate FTIE */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
							(sizeof(FT_FTIE) + 2),	pFTIe,
	                      END_OF_ARGS);
		c_len += tmp_len;
		
		/* Calculate MIC */				
		//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

		NdisMoveMemory(ft->MIC, mic, 16);

	}
	/*////////////////////////////////////////////////////////////////////*/
	
	if (RTMPEqualMemory(oldMic, ft->MIC, 16) == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS_TPKMsg3Process() MIC Error!!! \n"));
		return MLME_REQUEST_DECLINED;
	}

	
	return StatusCode;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_DiscoveryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	/* fill action code */
	TDLS_InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
							CATEGORY_TDLS, TDLS_ACTION_CODE_DISCOVERY_REQUEST);

	/* fill Dialog Token */
	pAd->StaCfg.TdlsInfo.TdlsDialogToken++;

	if (pAd->StaCfg.TdlsInfo.TdlsDialogToken == 0)
		pAd->StaCfg.TdlsInfo.TdlsDialogToken++;

	TDLS_InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, pAd->StaCfg.TdlsInfo.TdlsDialogToken);

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pOutBuffer + FrameLen), &FrameLen, pAd->CurrentAddress, pMacAddr);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS discovery request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

NDIS_STATUS
TDLS_DiscoveryRspAction(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR	PeerToken,
	IN PUCHAR	pPeerMac)
{
	PUCHAR	pOutBuffer = NULL;
	FRAME_ACTION_HDR	Frame;
	ULONG	FrameLen = 0;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
	if(NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	ActHeaderInit(pAd, &Frame.Hdr, pPeerMac, pAd->CurrentAddress, pAd->CommonCfg.Bssid);
	
	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_TDLS_DISCOVERY_RSP;
	
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
				sizeof(FRAME_ACTION_HDR),	&Frame,
				END_OF_ARGS);

	/* fill action code */
	//TDLS_InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_TDLS, 14);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, PeerToken);

	// fill capability
	TDLS_InsertCapIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	// fill support rate
	TDLS_InsertSupportRateIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	// fill ext rate
	TDLS_InsertExtRateIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	// fill support channels
	TDLS_InsertSupportChannelIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{		
		UCHAR			CipherTmp[64] = {0};
		UCHAR			CipherTmpLen = 0;
		ULONG			tmp;
		
		// RSNIE (7.3.2.25)
		CipherTmpLen = CipherSuiteTDLSLen;	
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

		// Insert RSN_IE to outgoing frame
		MakeOutgoingFrame((pOutBuffer + FrameLen),	&tmp,
						CipherTmpLen,				&CipherTmp,
				END_OF_ARGS);

		FrameLen = FrameLen + tmp;
	}

	// fill  Extended Capabilities (7.3.2.27)
	TDLS_InsertExtCapIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	// TPK Handshake if RSNA Enabled
	// Pack TPK Message 1 here! 
	if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
	{
		FT_FTIE			FtIe;
		ULONG			KeyLifetime = pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime;	// sec
		UCHAR			Length;
		UCHAR			SNonce[32];	// Generated in Message 2, random variable

		// FTIE (7.3.2.48)
		NdisZeroMemory(&FtIe, sizeof(FtIe));
		Length =  sizeof(FtIe);

		// generate SNonce
		GenRandom(pAd, pAd->CurrentAddress, FtIe.SNonce);
		hex_dump("TDLS - Generate SNonce ", FtIe.SNonce, 32);
		NdisMoveMemory(SNonce, FtIe.SNonce, 32);
		
		TDLS_InsertFTIE(pAd, 
						(pOutBuffer + FrameLen), 
						&FrameLen, 
						Length, 
						FtIe.MICCtr,
						FtIe.MIC, 
						FtIe.ANonce, 
						FtIe.SNonce);

		// Timeout Interval (7.3.2.49)
		TDLS_InsertTimeoutIntervalIE(pAd, 
									(pOutBuffer + FrameLen), 
									&FrameLen, 
									2, /* key lifetime interval */
									KeyLifetime);
	}

	// fill Supported Regulatory Classes
	TDLS_SupportedRegulatoryClasses(pAd, (pOutBuffer + FrameLen), &FrameLen);

#ifdef DOT11_N_SUPPORT
	// fill HT Capability
	TDLS_InsertHtCapIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	// fill 20/40 BSS Coexistence (7.3.2.61)
#ifdef DOT11N_DRAFT3
	TDLS_InsertBSSCoexistenceIE(pAd, (pOutBuffer + FrameLen), &FrameLen);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pOutBuffer + FrameLen), &FrameLen, pPeerMac, pAd->CurrentAddress);

	hex_dump("TDLS discovery response send pack", pOutBuffer, FrameLen);

	MiniportMMRequest(pAd, QID_AC_VI, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupRequestAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	NdisZeroMemory(Header802_3, LENGTH_802_3);
	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, &pAd->CurrentAddress[0], TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	/* enter ACTIVE mode */
	TDLS_CHANGE_TO_ACTIVE(pAd);

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupRequest(pAd, pOutBuffer, &FrameLen, pTDLS);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS setup request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupResponseAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupResponse(pAd, pOutBuffer, &FrameLen, pTDLS, RsnLen, pRsnIe, FTLen, pFTIe, TILen, pTIIe, StatusCode);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS send setup response pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupConfirmAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupConfirm(pAd, pOutBuffer, &FrameLen, pTDLS, RsnLen, pRsnIe, FTLen, pFTIe, TILen, pTIIe, StatusCode);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS setup confirm pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	TDLS_RECOVER_POWER_SAVE(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_LinkTearDown(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN	bDirect)
{
	UCHAR		idx;
	BOOLEAN		TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	/* tear down tdls table entry */
	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

		if (pTDLS->Valid && (pTDLS->Status >= TDLS_MODE_CONNECTED))
		{
			pTDLS->Status	= TDLS_MODE_NONE;
			pTDLS->Valid	= FALSE;
			pTDLS->Token = 0;
			RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

			if (!VALID_WCID(pTDLS->MacTabMatchWCID))
			return;

			if (pAd->StaCfg.bRadio == TRUE)
			{
				if (bDirect)
					TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON, TRUE);
				else
					TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_DUE_TO_PEER_STA_UNREACHABLE, FALSE);
			}

			MacTableDeleteEntry(pAd,pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
		}
		else if (pTDLS->Valid)
		{
			pTDLS->Status	= TDLS_MODE_NONE;
			pTDLS->Valid	= FALSE;
			pTDLS->Token = 0;
			RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
		}	
	}

#ifdef TDLS_AUTOLINK_SUPPORT
	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
	TDLS_ClearEntryList(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList);
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
	TDLS_ClearEntryList(&pAd->StaCfg.TdlsInfo.TdlsBlackList);
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
#endif /* TDLS_AUTOLINK_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_TearDownPeerLink(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR	pPeerAddr,
	IN BOOLEAN	bDirect)
{
	UCHAR EntryIdx;
	BOOLEAN	 TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	// tear down tdls table entry
	for (EntryIdx = 0; EntryIdx < MAX_NUM_OF_TDLS_ENTRY; EntryIdx++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[EntryIdx];

		if ((pTDLS->Valid) && (pTDLS->Status >= TDLS_MODE_CONNECTED) && MAC_ADDR_EQUAL(pPeerAddr, pTDLS->MacAddr))
		{
			pTDLS->Status	= TDLS_MODE_NONE;
			pTDLS->Valid	= FALSE;
			RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

			if (!VALID_WCID(pTDLS->MacTabMatchWCID))
			return;

			if (bDirect)
				TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON, TRUE);
			else
				TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_DUE_TO_PEER_STA_UNREACHABLE, FALSE);

			MacTableDeleteEntry(pAd,pTDLS->MacTabMatchWCID, pTDLS->MacAddr);

			break;
		}
	}

	if (EntryIdx == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() - Peer MAC - not found !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_TearDownAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UINT16	ReasonCode,
	IN BOOLEAN	bDirect)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	idx = 1;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildTeardown(pAd, pOutBuffer, &FrameLen, pTDLS, ReasonCode);

	if (bDirect == TRUE)
		idx = pTDLS->MacTabMatchWCID;
	else
		idx = BSSID_WCID;

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[idx], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS teardown send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeDiscoveryReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
#ifdef TDLS_AUTOLINK_SUPPORT
	PLIST_HEADER pTdlsDiscoveryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
	PTDLS_DISCOVERY_ENTRY pTdlsPeer = NULL;
#endif // TDLS_AUTOLINK_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	NdisMoveMemory(PeerMacAddr, Elem->Msg, MAC_ADDR_LEN);

	if (INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_TRACE,
						("Send Discovery Request to Peer ::%02x:%02x:%02x:%02x:%02x:%02x !!!\n",
						PRINT_MAC(PeerMacAddr)));

		/* Build TDLS Discovery Request Frame */
		NStatus = TDLS_DiscoveryReqAction(pAd, PeerMacAddr);
		if (NStatus	!= NDIS_STATUS_SUCCESS)	
		{
#ifdef TDLS_AUTOLINK_SUPPORT
			RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
			TDLS_DelDiscoveryEntryByMAC(pTdlsDiscoveryEnList, PeerMacAddr);
			RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
#endif // TDLS_AUTOLINK_SUPPORT //
			DBGPRINT(RT_DEBUG_ERROR,("%s() - Build Discovery Setup Request Fail !!!\n", __FUNCTION__));
		}
		else
		{
#ifdef TDLS_AUTOLINK_SUPPORT
			pTdlsPeer = TDLS_FindDiscoveryEntry(pTdlsDiscoveryEnList, PeerMacAddr);
#endif // TDLS_AUTOLINK_SUPPORT //
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,
						("%s() - Build Discovery Setup Request Fail, Because STA is not connect to AP!!!\n",
						__FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

#ifdef WFD_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeTunneledReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
#ifdef TDLS_AUTOLINK_SUPPORT
	PLIST_HEADER pTdlsDiscoveryEnList = &pAd->StaCfg.TdlsDiscovPeerList;
	PTDLS_DISCOVERY_ENTRY pTdlsPeer = NULL;
#endif // TDLS_AUTOLINK_SUPPORT //

	DBGPRINT(RT_DEBUG_ERROR,("===> TDLS_MlmeTunneledProbeReqAction() \n"));

	NdisMoveMemory(PeerMacAddr, Elem->Msg, MAC_ADDR_LEN);

	if (INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Send Tunneled Probe Request to Peer ::%02x:%02x:%02x:%02x:%02x:%02x !!!\n", 
								PeerMacAddr[0], PeerMacAddr[1], PeerMacAddr[2],
								PeerMacAddr[3], PeerMacAddr[4], PeerMacAddr[5]));

		/* Build TDLS Tunneled Probe Request Frame */
		NStatus = TDLS_TunneledProbeRequest(pAd, PeerMacAddr);
		if (NStatus	!= NDIS_STATUS_SUCCESS)	
		{
#ifdef TDLS_AUTOLINK_SUPPORT
			RTMP_SEM_LOCK(&pAd->StaCfg.TdlsDiscovPeerListSemLock);
			TDLS_DelDiscoveryEntryByMAC(pTdlsDiscoveryEnList, PeerMacAddr);
			RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsDiscovPeerListSemLock);
#endif // TDLS_AUTOLINK_SUPPORT //
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeTunneledProbeReqAction() Build Tunneled Probe Request Fail !!!\n"));
		}
		else
		{
#ifdef TDLS_AUTOLINK_SUPPORT
			pTdlsPeer = TDLS_FindDiscoveryEntry(pTdlsDiscoveryEnList, PeerMacAddr);
#endif // TDLS_AUTOLINK_SUPPORT //
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeTunneledProbeReqAction() Build Tunneled Probe Request Fail, Because STA is not connect to AP!!!\n"));
	}

	DBGPRINT(RT_DEBUG_ERROR,("<=== TDLS_MlmeTunneledProbeReqAction() \n"));

	return;
}

/*
==========================================================================
	Description:
		
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTunneledReqRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	ULONG			RemainLen = Elem->MsgLen;
	PFRAME_802_11	pFrame;
	pFrame = (PFRAME_802_11)Elem->Msg;
	CHAR			*Ptr =(CHAR *)Elem->Msg;
	PRT_802_11_TDLS pTDLS = NULL;
	BOOLEAN			TimerCancelled;

	DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_PeerTunneledReqRspAction() \n"));

	/*	Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes)
		TDLS Action header(payload type + category + action)(3 bytes) and Payload (variable)
	*/
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3)) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("TDLS_PeerTunneledReq/RspAction --> Invaild packet length - (action header) \n"));
		return; 
	}

	// Offset to Payload Type
	Ptr += (LENGTH_802_11 + LENGTH_802_1_H + 1);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 1);

	DBGPRINT(RT_DEBUG_ERROR, ("TDLS_PeerTunneledReqRspAction --> Gategory = %02x.  OUI = %02x-%02x-%02x.\n", *Ptr, *(Ptr+1), *(Ptr+2), *(Ptr+3)));
	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsDiscovReqSanity --> Invaild packet length - (dialog token) \n"));
		return;
	}

	/* Frame Body Type */
	Ptr += 4;
	RemainLen -= 4;

	if (RemainLen < 7)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("PeerTdlsDiscovReqSanity --> remain length is not enough = %d.\n", RemainLen));
		return;
	}

	if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr3, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTunneledReqSanity --> It's not my BSSID[%02x:%02x:%02x:%02x:%02x:%02x] Addr3[%02x:%02x:%02x:%02x:%02x:%02x]\n", PRINT_MAC(pAd->CommonCfg.Bssid), PRINT_MAC(pFrame->Hdr.Addr3)));
		//return;
	}

	NdisMoveMemory(PeerAddr, pFrame->Hdr.Addr3, MAC_ADDR_LEN);

	if ((*Ptr == WFD_TUNNELED_PROBE_REQ) ||
		(*Ptr == WFD_TUNNELED_PROBE_RSP))
	{
		PWFD_DEVICE_INFO pWfd_info;
		SHORT			idx;

		for (idx = MAX_NUM_OF_TDLS_ENTRY - 1; idx >= 0; idx--)
		{
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

			if (pTDLS->Valid && MAC_ADDR_EQUAL(PeerAddr, pTDLS->MacAddr))
			{
				DBGPRINT(RT_DEBUG_TRACE,("%s::find the same entry and discard it\n", __FUNCTION__));
				break;
			}
		}

		if (idx < 0)
		{
			for (idx = (MAX_NUM_OF_TDLS_ENTRY - 1); idx >= 0; idx--)
			{
				pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

				if (!pTDLS->Valid)
				{
					RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
					pTDLS->Valid = TRUE;
					NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
					DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupReqRspAction() create a new entry \n"));
					
					break;
				}
			}
		}


		if (idx < 0)
		{
			// Table full !!!!!
			DBGPRINT(RT_DEBUG_ERROR,("%s::TDLSEntry table full(only can support %d TDLS session) \n",
									__FUNCTION__, MAX_NUM_OF_TDLS_ENTRY));
		}
		else if (pTDLS)
		{
			UCHAR	Addr2[6], SsidLen = 0;
			UCHAR	Ssid[32];
			ULONG		Peerip;
			UCHAR	DsChannel = 0;
			ULONG	P2PSubelementLen = 0;
			PUCHAR	P2pSubelement = NULL;
			ULONG	WfdSubelementLen = 0;
			PUCHAR	WfdSubelement = NULL;

			os_alloc_mem(pAd, &P2pSubelement, MAX_VIE_LEN);
	
			if (P2pSubelement == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=%d) failed\n", __FUNCTION__, MAX_VIE_LEN));
				return;
			}
	
			os_alloc_mem(pAd, &WfdSubelement, MAX_VIE_LEN);
	
			if (WfdSubelement == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=%d) failed\n", __FUNCTION__, MAX_VIE_LEN));
				os_free_mem(pAd, P2pSubelement);
				return;
			}

			 (TDLS_PeerTunneledProbeReqRspSanity(pAd, 
										Elem->Msg, 
										Elem->MsgLen, 
										Addr2, 
										Ssid, 
										&SsidLen, 
										&Peerip,
										&DsChannel,
										&P2PSubelementLen,
										P2pSubelement,
										&WfdSubelementLen,
										WfdSubelement));

			if (WfdSubelementLen > 0)
			{
				WfdParseSubElmt(pAd, 
								&pTDLS->WfdEntryInfo,
								(PVOID)WfdSubelement, 
								WfdSubelementLen);
			}
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s:: WFD Element IE Len is 0.\n", __FUNCTION__));
			
			if (P2pSubelement)
				os_free_mem(pAd, P2pSubelement);
			if (WfdSubelement)
				os_free_mem(pAd, WfdSubelement);	
			
			if (*Ptr == WFD_TUNNELED_PROBE_REQ)
			{
				DBGPRINT(RT_DEBUG_ERROR,("Receive Tunneled probe request frame\n"));
			
				/* Build TDLS Discovery Response Frame */
				NStatus = TDLS_TunneledProbeResponse(pAd, PeerAddr);
			
				if (NStatus != NDIS_STATUS_SUCCESS) 
				{
					DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_PeerTunneledReqRspAction() Build Tunneled Response Fail !!!\n"));
				}
			}
			else
				DBGPRINT(RT_DEBUG_ERROR,("Receive Tunneled probe response frame\n"));
		}
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s::unknown Type = %02x.\n", __FUNCTION__, *Ptr+4));

	return;
}
#endif /* WFD_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerDiscoveryReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	ULONG			RemainLen = Elem->MsgLen;
	CHAR			*Ptr =(CHAR *)Elem->Msg;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	/*	Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes)
		TDLS Action header(payload type + category + action)(3 bytes) and Payload (variable)
	*/
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3)) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return;	
	}

	// Offset to Dialog Token
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return;
	}

	Token = *Ptr;

	// Offset to Link Identifier
	Ptr += 1;
	RemainLen -= 1;

	// Get BSSID, SA and DA from payload and advance the pointer
	if ((RemainLen < 20) || (Ptr[0] != IE_TDLS_LINK_IDENTIFIER) || (Ptr[1] != 18))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (link identifier) \n", __FUNCTION__));
		return;
	}
	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
		return;
	}	
	else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->CurrentAddress))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
		return;
	}	

	NdisMoveMemory(PeerAddr, Ptr+8, MAC_ADDR_LEN);

	/* Build TDLS Discovery Response Frame */
	NStatus = TDLS_DiscoveryRspAction(pAd, Token, PeerAddr);

	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s() - Build Discovery Response Fail !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_DiscoveryRspPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN VOID		*Msg, 
	IN ULONG	MsgLen)
{
	USHORT			StatusCode = MLME_SUCCESS;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen, HtCapLen, ExtCapLen, RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	HT_CAPABILITY_IE		HtCap;
	EXT_CAP_INFO_ELEMENT	ExtCap;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	hex_dump("TDLS discovery response receive pack", Msg, MsgLen);

	if (!PeerTdlsDiscovRspSanity(
							pAd, 
							Msg, 
							MsgLen,
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&SupRateLen,	
							&SupRate[0],
							&ExtRateLen,
							&ExtRate[0],
							&HtCapLen,
							&HtCap,
							&ExtCapLen,
							&ExtCap,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		StatusCode = MLME_REQUEST_DECLINED;
	}

	COPY_MAC_ADDR(PeerAddr, &pFrame->Hdr.Addr2);

	if (StatusCode == MLME_SUCCESS)
	{
#ifdef TDLS_AUTOLINK_SUPPORT
		if (pAd->StaCfg.TdlsInfo.TdlsAutoLink)
		{
			PLIST_HEADER	pTdlsDiscovryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
			PTDLS_DISCOVERY_ENTRY	pTdlsPeer = NULL;

			pTdlsPeer = TDLS_FindDiscoveryEntry(pTdlsDiscovryEnList, PeerAddr);

			if (pTdlsPeer)
			{
				PLIST_HEADER	pTdlsDiscovEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
				CHAR	Rssi = -99;

				Rssi = RTMPMaxRssi(pAd, 
								   ConvertToRssi(pAd, (CHAR)Elem->Rssi0, RSSI_0),
								   ConvertToRssi(pAd, (CHAR)Elem->Rssi1, RSSI_1),
								   ConvertToRssi(pAd, (CHAR)Elem->Rssi2, RSSI_2));

				DBGPRINT(RT_DEBUG_ERROR, ("Discovery Response Rssi = %d !!!\n" , Rssi));

				if (pTdlsPeer->bConnected)
				{
					if ((pTdlsPeer->bConnectedFirstTime) && (pTdlsPeer->RetryCount == 1))
					{
						pTdlsPeer->AvgRssi0 = Rssi;
						pTdlsPeer->RetryCount--;
						pTdlsPeer->bConnectedFirstTime = FALSE;
						DBGPRINT(RT_DEBUG_ERROR,("Connected first time discovery !!!\n"));
					}
					else
					{
						if (pTdlsPeer->RetryCount == 1)
						{
							if ((Rssi < pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold) &&
								(pTdlsPeer->AvgRssi0 < pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold))
							{
								/* Tear Down */
								if (INFRA_ON(pAd))
								{
									MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
									USHORT		Reason = REASON_UNSPECIFY;
									INT			idx;

									DBGPRINT(RT_DEBUG_ERROR,("\n !!! Will tear down %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
															pTdlsPeer->Responder[0],
															pTdlsPeer->Responder[1],
															pTdlsPeer->Responder[2],
															pTdlsPeer->Responder[3],
															pTdlsPeer->Responder[4],
															pTdlsPeer->Responder[5]));

									idx = TDLS_SearchLinkId(pAd, pTdlsPeer->Responder);

									if (idx == -1 || idx == MAX_NUM_OF_TDLS_ENTRY)
									{
										DBGPRINT(RT_DEBUG_ERROR,("TDLS - can not find or full the LinkId!\n"));
									}
									else if (idx >= 0)
									{
										Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
										pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Valid	= FALSE;
										pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Status	= TDLS_MODE_NONE;
										TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[idx], Reason, FALSE);

										MlmeEnqueue(pAd,
													TDLS_STATE_MACHINE,
													MT2_MLME_TDLS_TEAR_DOWN,
													sizeof(MLME_TDLS_REQ_STRUCT),
													&MlmeTdlsReq,
													0);

									}
								}

								RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
								TDLS_DelDiscoveryEntryByMAC(pTdlsDiscovryEnList, PeerAddr);
								RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
							}
							else
							{
								pTdlsPeer->AvgRssi0 = Rssi;
								pTdlsPeer->RetryCount--;
							}
						}
						else if (pTdlsPeer->RetryCount > 1)
						{
							if (Rssi < pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold)
							{
								/* Tear Down */
								if (INFRA_ON(pAd))
								{
									MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
									USHORT		Reason = REASON_UNSPECIFY;
									INT			idx;

									DBGPRINT(RT_DEBUG_ERROR,("\n !!! Will tear down %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
															pTdlsPeer->Responder[0],
															pTdlsPeer->Responder[1],
															pTdlsPeer->Responder[2],
															pTdlsPeer->Responder[3],
															pTdlsPeer->Responder[4],
															pTdlsPeer->Responder[5]));

									idx = TDLS_SearchLinkId(pAd, pTdlsPeer->Responder);

									if (idx == -1 || idx == MAX_NUM_OF_TDLS_ENTRY)
									{
										DBGPRINT(RT_DEBUG_ERROR,("TDLS - can not find or full the LinkId!\n"));
									}
									else if (idx >= 0)
									{
										Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
										pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Valid	= FALSE;
										pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Status	= TDLS_MODE_NONE;
										TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[idx], Reason, FALSE);

										MlmeEnqueue(pAd,
													TDLS_STATE_MACHINE,
													MT2_MLME_TDLS_TEAR_DOWN,
													sizeof(MLME_TDLS_REQ_STRUCT),
													&MlmeTdlsReq,
													0);

									}
								}

								RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
								TDLS_DelDiscoveryEntryByMAC(pTdlsDiscovryEnList, PeerAddr);
								RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
							}
							else
							{
								pTdlsPeer->AvgRssi0 = Rssi;
								pTdlsPeer->RetryCount = 0;
							}
						}
					}
				}
				else
				{
					if (pTdlsPeer->CurrentState == TDLS_DISCOVERY_IDLE)
					{
						pTdlsPeer->CurrentState = TDLS_DISCOVERY_FIRST_TIME;
						pTdlsPeer->AvgRssi0 = Rssi;
					}
					else if (pTdlsPeer->CurrentState == TDLS_DISCOVERY_FIRST_TIME)
					{
						if ((pTdlsPeer->AvgRssi0 > pAd->StaCfg.TdlsInfo.TdlsAutoSetupRssiThreshold) &&
							(Rssi > pAd->StaCfg.TdlsInfo.TdlsAutoSetupRssiThreshold))
						{
							if (INFRA_ON(pAd) && (pTdlsPeer->bConnected == FALSE))
							{
								INT						LinkId = 0xff;

								LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
								if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
								{
									RT_802_11_TDLS	Tdls;

									DBGPRINT(RT_DEBUG_ERROR,("\n!!! Auto Setup TDLS to %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
														pTdlsPeer->Responder[0],
														pTdlsPeer->Responder[1],
														pTdlsPeer->Responder[2],
														pTdlsPeer->Responder[3],
														pTdlsPeer->Responder[4],
														pTdlsPeer->Responder[5]));
									NdisZeroMemory(&Tdls, sizeof(RT_802_11_TDLS));
									Tdls.TimeOut = 0;
									COPY_MAC_ADDR(Tdls.MacAddr, pTdlsPeer->Responder);
									Tdls.Valid = 1;

									pTdlsPeer->CurrentState = TDLS_DISCOVERY_TO_SETUP;

									MlmeEnqueue(pAd, 
												MLME_CNTL_STATE_MACHINE, 
												RT_OID_802_11_SET_TDLS_PARAM, 
												sizeof(RT_802_11_TDLS), 
												&Tdls,
												0);
								}
								else
								{
									PTDLS_DISCOVERY_ENTRY pPeerEntry = NULL;
									PLIST_HEADER	pTdlsDiscovEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
									PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;

									RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
									TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscovEnList, PeerAddr, TRUE);
									pPeerEntry = TDLS_FindDiscoveryEntry(pTdlsDiscovEnList, PeerAddr);
									if (pPeerEntry)
									{
										pPeerEntry->bConnectedFirstTime = TRUE;
										pPeerEntry->bConnected = TRUE;
										pPeerEntry->RetryCount = 0;
										pPeerEntry->CurrentState = TDLS_DISCOVERY_TO_SETUP_DONE;
									}
									RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);

									RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
									TDLS_DelBlackEntryByMAC(pTdlsBlackEnList, PeerAddr);
									RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
								}
							}
						}
						else
						{
							pTdlsPeer->CurrentState = TDLS_DISCOVERY_TO_SETUP_FAIL;
						}
					}
				}
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR,("Can't find discovery entry on discovery table !!!\n"));
			}
		}
#endif // TDLS_AUTOLINK_SUPPORT //
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeSetupReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS	pTDLS = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	MLME_TDLS_REQ_STRUCT *pInfo;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	pInfo = (MLME_TDLS_REQ_STRUCT *)Elem->Msg;
    
	pTDLS = pInfo->pTDLS;

	if (pAd->StaActive.ExtCapInfo.TDLSProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() - AP Prohibited TDLS !!!\n", __FUNCTION__));
		return;
	}
	
	/* Build TDLS Setup Request Frame */
	NStatus = TDLS_SetupRequestAction(pAd, pTDLS);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s() - Build Setup Request Fail !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerSetupReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS;
	SHORT			idx;
	ULONG			Timeout = TDLS_TIMEOUT;
	BOOLEAN			TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;
	RT_802_11_TDLS	TmpTDLS;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen, HtCapLen, ExtCapLen, RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	UCHAR			QosCapability;
	HT_CAPABILITY_IE		HtCap;
	EXT_CAP_INFO_ELEMENT	ExtCap;
	BOOLEAN			bDiscard = FALSE;
	BOOLEAN			bWmmCapable;
#ifdef WFD_SUPPORT
	ULONG	WfdSubelementLen = 0;
	PUCHAR	WfdSubelement = NULL;
#endif /* WFD_SUPPORT */
	
#ifdef WFD_SUPPORT
	os_alloc_mem(pAd, &WfdSubelement, MAX_VIE_LEN);
	if (WfdSubelement == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=%d) failed\n", __FUNCTION__, MAX_VIE_LEN));
		return;
	}
#endif /* WFD_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		goto CleanUp;
	
	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		goto CleanUp;

	if (pAd->StaActive.ExtCapInfo.TDLSProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() - AP Prohibited TDLS !!!\n", __FUNCTION__));
		goto CleanUp;
	}

	// Init all kinds of fields within the packet
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(&HtCap, sizeof(HtCap));
	NdisZeroMemory(&ExtCap, sizeof(ExtCap));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));

	hex_dump("TDLS setup request receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsSetupReqSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
#ifdef WFD_SUPPORT
							&WfdSubelementLen,
							WfdSubelement,
#endif /* WFD_SUPPORT */
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&SupRateLen,	
							&SupRate[0],
							&ExtRateLen,
							&ExtRate[0],
							&bWmmCapable,
							&QosCapability,
							&HtCapLen,
							&HtCap,
							&ExtCapLen,
							&ExtCap,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		StatusCode = MLME_REQUEST_DECLINED;
	}

	DBGPRINT(RT_DEBUG_TRACE,
					("%s() - received a request from %02x:%02x:%02x:%02x:%02x:%02x\n",
					__FUNCTION__, PRINT_MAC(PeerAddr)));

	if ((pAd->StaCfg.wdev.WepStatus != Ndis802_11WEPDisabled) && (RsnLen == 0))
	{
		StatusCode = MLME_INVALID_SECURITY_POLICY;
	}

#ifdef WFD_SUPPORT
	/* WFD spec. 4.5.3 Establish WFD Connection using TDLS
	 * If an associated infrastructure AP uses WEP or WPA for the link between the AP and the WFD device, 
	 * the WFD device shall not accept the TDLS Setup Request frame for WFD connection but reject it by 
	 * sending TDLS Setup Response frame with indicating status code as 5 (¡§Security disabled¡¨).
	*/
	if ((pAd->StaCfg.WfdCfg.bWfdEnable) && WfdSubelementLen)
	{
		if (RsnLen == 0)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::No RSN IE and peer is WFD device, return weak security(5)!\n", __FUNCTION__));
			StatusCode = MLME_SECURITY_WEAK;
		}
		if ((pAd->StaCfg.wdev.AuthMode != Ndis802_11AuthModeWPA2PSK) ||
			(pAd->StaCfg.wdev.WepStatus != Ndis802_11Encryption3Enabled)) // Pairwise cipher suite is not AES
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::AP Security is not WPA2PSK/AES and peer is WFD device, return weak security(5)!\n", __FUNCTION__));
			StatusCode = MLME_SECURITY_WEAK;
		}
	}
#endif /* WFD_SUPPORT */
#ifdef TDLS_AUTOLINK_SUPPORT
	if (pAd->StaCfg.TdlsInfo.TdlsAutoLink)
	{
		PTDLS_BLACK_ENTRY pTdlsBlackEntry = NULL;
		PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;

		RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
		pTdlsBlackEntry = TDLS_FindBlackEntry(pTdlsBlackEnList, PeerAddr);
		RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);

		if (pTdlsBlackEntry)
		{
			if (pTdlsBlackEntry->CurrentState == TDLS_BLACK_TDLS_BY_TEARDOWN)
				StatusCode = MLME_REQUEST_DECLINED;
		}
	}
#endif // TDLS_AUTOLINK_SUPPORT //

	/* Find table to update parameters. */
	if (StatusCode == MLME_SUCCESS)
	{
		for (idx = MAX_NUM_OF_TDLS_ENTRY - 1; idx >= 0; idx--)
		{
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

			if (pTDLS->Valid && MAC_ADDR_EQUAL(PeerAddr, pTDLS->MacAddr))
			{
				if (pTDLS->Status == TDLS_MODE_WAIT_RESPONSE)
				{
					if (RTMPCompareMemory(PeerAddr, pAd->CurrentAddress, MAC_ADDR_LEN) == 2)
					{
						/*
							11.20.2 TDLS Link Establishment

							4.	The TDLS setup request frame is received after sending a TDLS Setup Request frame and before
								receiving the corresponding TDLS Setup Response frame, and the source address of the received
								TDLS Setup Request frame is lower than its own MAC address. In this case, the TDLS responder
								STA shall terminate the TDLS setup it initiated. The TDLS responder STA shall send a response
								frame.
						*/
						RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
						pTDLS->Token = 0;
						pTDLS->Valid = FALSE;
						NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
						pTDLS->Status = TDLS_MODE_NONE;
						idx = -1;
						DBGPRINT(RT_DEBUG_ERROR,("%s() - find the same entry \n", __FUNCTION__));
					}
					else
					{
						/*
							11.20.2 TDLS Link Establishment

							3.	The TDLS setup request is received after sending a TDLS Setup Request frame and before
								receiving the corresponding TDLS Setup Response frame, and the source address of the received
								TDLS Setup Request frame is higher than its own MAC address, in which case the TDLS
								responder STA shall silently discard the message and the TDLS responder STA shall send no
								TDLS Setup Response frame.
						*/
						bDiscard = TRUE;
						DBGPRINT(RT_DEBUG_ERROR,("%s() - find the same entry and discard it\n", __FUNCTION__));
					}
				}
				else if (pTDLS->Status == TDLS_MODE_CONNECTED)
				{
					if(pTDLS->bInitiator == FALSE)
					{
						/*
							If a TDLS Setup Request frame is received from a TDLS responder STA with
							which a currently active TDLS session exists, then the receiving STA shall
							tear down the existing TDLS direct link as if a TDLS Teardown frame was received,
							and respond with a TDLS Setup Response frame.
						*/
						if (!VALID_WCID(pTDLS->MacTabMatchWCID))
							goto CleanUp;

						MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);

						RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
						pTDLS->Token = 0;
						pTDLS->Valid = FALSE;
						NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
						pTDLS->Status = TDLS_MODE_NONE;
					}
				}

				break;
			}
		}

		if (bDiscard == TRUE)
			goto CleanUp;

		if (idx >= 0)
		{
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];
			if (pTDLS->Token == Token)
			{
				DBGPRINT(RT_DEBUG_WARN, ("%s() - receive the same token TDLS request !!!\n", __FUNCTION__));
				goto CleanUp;;
			}
		}

		/* Can not find in table, create a new one */
		if (idx < 0)
		{
			for (idx = (MAX_NUM_OF_TDLS_ENTRY - 1); idx >= 0; idx--)
			{
				pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

				if (!pTDLS->Valid)
				{
					RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
					pTDLS->Valid = TRUE;
					NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
					DBGPRINT(RT_DEBUG_TRACE, ("%s() -create a new entry \n", __FUNCTION__));
					
					break;
				}
			}
		}


		if (idx < 0)
		{
			/* Table full !!!!! */
			StatusCode = MLME_REQUEST_DECLINED;
			DBGPRINT(RT_DEBUG_ERROR, ("%s() - TDLSEntry table full(only can support %d TDLS session) \n",
						__FUNCTION__, MAX_NUM_OF_TDLS_ENTRY));
		}
		else if (pTDLS)
		{	
			//
			// Process TPK Handshake Message 1 here!
			//
			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{
				USHORT Result;

				// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
				Result = TDLS_TPKMsg1Process(pAd, 
#ifdef WFD_SUPPORT
				WfdSubelementLen,
#endif /* WFD_SUPPORT */
				pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
				if (Result != MLME_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - TDLS_TPKMsg1Process() Failed, reason=%d \n",
								__FUNCTION__, Result));

					if (Result == MLME_REQUEST_DECLINED)	// if mic error , ignore
					{
						goto CleanUp;
					}
					else
					{
						StatusCode = Result;
						goto send_out;
					}
				}

				// Copy SNonce, Key lifetime
				pTDLS->KeyLifetime = le2cpu32(*((PULONG) (TIIe + 3)));
				NdisMoveMemory(pTDLS->SNonce, &FTIe[52], 32);
			}


			//
			// Update temporarliy settings. Try to match Initiator's capabilities
			//
			pTDLS->Token = Token;

			// I am Responder.And peer are Initiator
			pTDLS->bInitiator = TRUE;
			pTDLS->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

			// Copy Initiator's supported rate and filter out not supported rate
			pTDLS->SupRateLen = SupRateLen;
			NdisMoveMemory(pTDLS->SupRate, SupRate, SupRateLen);
			RTMPCheckRates(pAd, pTDLS->SupRate, &pTDLS->SupRateLen);
			pTDLS->ExtRateLen = ExtRateLen;
			NdisMoveMemory(pTDLS->ExtRate, ExtRate, ExtRateLen);
			RTMPCheckRates(pAd, pTDLS->ExtRate, &pTDLS->ExtRateLen);

			// Filter out un-supported ht rates
			if ((HtCapLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				// HtCapability carries Responder's capability, so not copy from Initiator here.
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
				pTDLS->HtCapabilityLen = SIZE_HT_CAP_IE;
				NdisMoveMemory(&pTDLS->HtCapability, &HtCap, SIZE_HT_CAP_IE);
			}
			else
			{
				pTDLS->HtCapabilityLen = 0;
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
			}

			// Copy extended capability
			NdisMoveMemory(&pTDLS->TdlsExtCap, &ExtCap, sizeof(EXT_CAP_INFO_ELEMENT));

			// Copy QOS related information
			pTDLS->QosCapability = QosCapability;
			pTDLS->bWmmCapable = bWmmCapable;
#ifdef WFD_SUPPORT
			if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
				WfdSubelementLen > 0)
			{
				WfdParseSubElmt(pAd, 
								&pTDLS->WfdEntryInfo,
								(PVOID)WfdSubelement, 
								WfdSubelementLen);

				/* WFD spec. 4.5.3 Establish WFD Connection using TDLS 
				 * If the type of WFD device indicated in WFD Device Type bits (B1B0) in WFD Device Information field in
				 * WFD IE within the received TDLS Setup Request frame is different from the expected value,
				 * the recipient should respond using TDLS Setup Response frame with setting Status Code to 38
				 * (The request has not been successful as one or more parameters have invalid values).
				*/
				if (pTDLS->WfdEntryInfo.wfd_devive_type == WFD_SOURCE_PRIMARY_SINK)
					StatusCode = MLME_REQUEST_WITH_INVALID_PARAM;
				else
				{
					switch (pAd->StaCfg.WfdCfg.DeviceType)
					{
						case WFD_SOURCE:
							if (pTDLS->WfdEntryInfo.wfd_devive_type == WFD_SOURCE)
								StatusCode = MLME_REQUEST_WITH_INVALID_PARAM;
							break;
						case WFD_PRIMARY_SINK:
						case WFD_SECONDARY_SINK:
							if ((pTDLS->WfdEntryInfo.wfd_devive_type == WFD_PRIMARY_SINK) ||
								(pTDLS->WfdEntryInfo.wfd_devive_type == WFD_SECONDARY_SINK))
								StatusCode = MLME_REQUEST_WITH_INVALID_PARAM;
							break;
					}
				}
				
				if (StatusCode == MLME_REQUEST_WITH_INVALID_PARAM)
					DBGPRINT(RT_DEBUG_ERROR, ("%s:: Peer device type is not the expected value,\nMy WFD Device Type = %d, Peer WFD Device Type = %d\n",
					__FUNCTION__, pAd->StaCfg.WfdCfg.DeviceType, pTDLS->WfdEntryInfo.wfd_devive_type));
				else					
					DBGPRINT(RT_DEBUG_TRACE, ("%s:: My WFD Device Type = %d, Peer WFD Device Type = %d\n", __FUNCTION__, 
						pAd->StaCfg.WfdCfg.DeviceType, pTDLS->WfdEntryInfo.wfd_devive_type));
			}
#endif /* WFD_SUPPORT */
		}
	}

send_out:

	if (StatusCode != MLME_SUCCESS)
	{
		NdisZeroMemory(&TmpTDLS, sizeof(RT_802_11_TDLS));
		pTDLS = &TmpTDLS;
		NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
		pTDLS->Token = Token;
	}

	TDLS_SetupResponseAction(pAd, pTDLS, RsnLen, RsnIe, FTLen, FTIe,  TILen, TIIe, StatusCode);

	if (StatusCode == MLME_SUCCESS)
	{
		/*  Set sendout timer */
		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
		RTMPSetTimer(&pTDLS->Timer, Timeout);
		pTDLS->Valid = TRUE;

		/* State Change */
		pTDLS->Status = TDLS_MODE_WAIT_CONFIRM;

	}

CleanUp:
#ifdef WFD_SUPPORT
	os_free_mem(pAd, WfdSubelement);
#endif /* WFD_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerSetupRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
	BOOLEAN			TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen, HtCapLen, ExtCapLen, RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	UCHAR			QosCapability;
	BOOLEAN			bWmmCapable;
	HT_CAPABILITY_IE		HtCap;
	EXT_CAP_INFO_ELEMENT	ExtCap;
	INT						LinkId = 0xff;
	UCHAR					TPK[LEN_PMK], TPKName[LEN_PMK_NAME];
	PMAC_TABLE_ENTRY		pMacEntry = NULL;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
#ifdef WFD_SUPPORT
	ULONG	WfdSubelementLen = 0;
	PUCHAR	WfdSubelement = NULL;
#endif /* WFD_SUPPORT */
		
#ifdef WFD_SUPPORT
	os_alloc_mem(pAd, &WfdSubelement, MAX_VIE_LEN);
	if (WfdSubelement == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=%d) failed\n", __FUNCTION__, MAX_VIE_LEN));
		return;
	}
#endif /* WFD_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		goto CleanUp;

	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		goto CleanUp;

	hex_dump("TDLS setup response receive pack", Elem->Msg, Elem->MsgLen);

	// Init all kinds of fields within the packet
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(&HtCap, sizeof(HtCap));
	NdisZeroMemory(&ExtCap, sizeof(ExtCap));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));

	if (!PeerTdlsSetupRspSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
#ifdef WFD_SUPPORT
							&WfdSubelementLen,
							WfdSubelement,
#endif /* WFD_SUPPORT */
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&SupRateLen,	
							SupRate,
							&ExtRateLen,
							ExtRate,
							&bWmmCapable,
							&QosCapability,
							&HtCapLen,
							&HtCap,
							&ExtCapLen,
							&ExtCap,
							&StatusCode,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		LocalStatusCode = MLME_REQUEST_DECLINED;
	}


	DBGPRINT(RT_DEBUG_ERROR, ("%s() - received a response from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n",
				__FUNCTION__, PRINT_MAC(PeerAddr), StatusCode));

	if (StatusCode != MLME_SUCCESS)
	{
		COPY_MAC_ADDR(PeerAddr, &pFrame->Hdr.Addr3);
	}

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s() - can not find the LinkId!\n", __FUNCTION__));
		goto CleanUp;
	}

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];
	// Cancel the timer since the received packet to me.
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

	// Received a error code from the Peer TDLS.
	// Let's terminate the setup procedure right now.
	if (StatusCode != MLME_SUCCESS)
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;

#ifdef WFD_SUPPORT
		if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
			(StatusCode == MLME_SECURITY_WEAK))
			pAd->StaCfg.WfdCfg.TdlsSecurity = WFD_TDLS_WEAK_SECURITY;
#endif /* WFD_SUPPORT */
		
		DBGPRINT(RT_DEBUG_ERROR,
				("%s() - received a failed StatusCode, terminate the setup procedure \n", __FUNCTION__));

		goto CleanUp;
	}
	else
		StatusCode = LocalStatusCode;

	// 
	// Validate the content on Setup Response Frame
	//
	while (StatusCode == MLME_SUCCESS)
	{		
		// Invalid TDLS State
		if (pTDLS->Status != TDLS_MODE_WAIT_RESPONSE)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s() - Not in TDLS_MODE_WAIT_RESPONSE STATE\n", __FUNCTION__));
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}

		// Is the same Dialog Token?
		if (pTDLS->Token != Token)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s() - Not match with Dialig Token my token = %d, peer token = %d\n",
							__FUNCTION__, pTDLS->Token, Token));
			
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}
		
		// Process TPK Handshake Message 2 here!
		if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
		{	
			USHORT Result;

			// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
			Result = TDLS_TPKMsg2Process(pAd, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen, TPK, TPKName);
			if (Result != MLME_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s() - TDLS_TPKMsg2Process() Failed, reason=%d \n", __FUNCTION__, Result));
				if (Result == MLME_REQUEST_DECLINED)	// if mic error , ignore
				{
					goto CleanUp;
				}
				else
				{
					StatusCode = Result;
					goto send_out;
				}
			}
			// Copy ANonce, Key lifetime, TPK, TPK Name
			pTDLS->KeyLifetime =  le2cpu32(*((PULONG) (TIIe + 3)));
			NdisMoveMemory(pTDLS->ANonce, &FTIe[20], 32);
				NdisMoveMemory(pTDLS->TPK, TPK, LEN_PMK);
			NdisMoveMemory(pTDLS->TPKName, TPKName, LEN_PMK_NAME);
		}

		// Update parameters
		if (StatusCode == MLME_SUCCESS)
		{
			// I am Initiator. And peer are Responder
			pTDLS->bInitiator = FALSE;
			// Capabilities
			pTDLS->CapabilityInfo = CapabilityInfo;

			pTDLS->SupRateLen = SupRateLen;
			pTDLS->ExtRateLen = ExtRateLen;
			
			// Copy ht capabilities from the Peer TDLS
			if ((HtCapLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				NdisMoveMemory(&pTDLS->HtCapability, &HtCap, HtCapLen);
				pTDLS->HtCapabilityLen = HtCapLen;				
			}
			else
			{
				pTDLS->HtCapabilityLen = 0;
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
			}

			// Copy extended capability
			NdisMoveMemory(&pTDLS->TdlsExtCap, &ExtCap, sizeof(EXT_CAP_INFO_ELEMENT));

			// Copy QOS related information
			pTDLS->QosCapability = QosCapability;
			pTDLS->bWmmCapable = bWmmCapable;
			
			// Copy TPK related information
			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{
				// SNonce, Key lifetime
			}
#ifdef WFD_SUPPORT
			if (WfdSubelementLen > 0)
			{
				WfdParseSubElmt(pAd, 
								&pTDLS->WfdEntryInfo,
								(PVOID)WfdSubelement, 
								WfdSubelementLen);
			}
#endif /* WFD_SUPPORT */

		}

		break;
	}

	//
	// Insert into mac table
	//
	if (StatusCode == MLME_SUCCESS)
	{
#ifdef WFA_TESTBED_FUNCTION_SUPPORT
		if (pAd->StaCfg.bSetFailOnSetupConf == TRUE)
			goto send_out;
#endif // WFA_TESTBED_FUNCTION_SUPPORT //

		// allocate one MAC entry
		pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);

		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			DBGPRINT(RT_DEBUG_ERROR,("%s() - MacTable Entry exist !!!\n", __FUNCTION__));
		else
			pMacEntry = MacTableInsertEntry(pAd,
							pTDLS->MacAddr,
							&pAd->StaCfg.wdev,
							BSS0 + MIN_NET_DEVICE_FOR_TDLS,
							OPMODE_STA,
							TRUE);

		if (pMacEntry)
		{
			pTDLS->MacTabMatchWCID = pMacEntry->Aid;
			pMacEntry->AuthMode = pAd->StaCfg.wdev.AuthMode;
			pMacEntry->WepStatus = pAd->StaCfg.wdev.WepStatus;
			pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			pMacEntry->Sst = SST_ASSOC;

#ifdef UAPSD_SUPPORT
			/* update UAPSD */
			UAPSD_AssocParse(pAd, pMacEntry, &QosCapability,
							pAd->StaCfg.UapsdInfo.bAPSDCapable);
#endif /* UAPSD_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE,
						("MacTableInsertTDlsEntry - allocate entry #%d, Total= %d\n",
						pMacEntry->Aid, pAd->MacTab.Size));

			// Set WMM capability
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
			{
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
				DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  WMM Capable\n"));
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			TDLS_InitPeerEntryRateCapability(pAd,
											pMacEntry,
											&CapabilityInfo,
											SupRateLen,
											SupRate,
											HtCapLen,
											&HtCap);

			RTMPSetSupportMCS(pAd,
								OPMODE_STA,
								pMacEntry,
								SupRate,
								SupRateLen,
								ExtRate,
								ExtRateLen,
#ifdef DOT11_VHT_AC
								0,
								NULL,
#endif /* DOT11_VHT_AC */
								&HtCap,
								HtCapLen);

			//
			// Install Peer Key if RSNA Enabled
			//
			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{	
				// Write to ASIC on-chip table.
				if ( pMacEntry->Aid > 1)
				{
					CIPHER_KEY		PairwiseKey;

					PairwiseKey.CipherAlg = CIPHER_AES;

					// Set Peer Key
					PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(PairwiseKey.Key, &pTDLS->TPK[16], LEN_TK);

					RTMP_ASIC_PAIRWISE_KEY_TABLE(pAd,
											pMacEntry->Aid,
											&PairwiseKey);															

					RTMP_SET_WCID_SEC_INFO(pAd, 
										BSS0,
										0, 
										PairwiseKey.CipherAlg, 
										pMacEntry->Aid,
										PAIRWISEKEYTABLE);

					NdisMoveMemory(&pMacEntry->PairwiseKey, &PairwiseKey, sizeof(CIPHER_KEY));

					pMacEntry->AuthMode = Ndis802_11AuthModeWPA2PSK;
					pMacEntry->WepStatus = Ndis802_11Encryption3Enabled;
					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}	
				
			}
		}
		else
		{
			StatusCode = MLME_REQUEST_DECLINED;
			DBGPRINT(RT_DEBUG_ERROR,("%s() - MacTableInsertEntry failed\n", __FUNCTION__));
		}
	}
	else
		StatusCode = MLME_REQUEST_DECLINED;

send_out:	

	if (StatusCode == MLME_SUCCESS)
	{
		TDLS_SetupConfirmAction(pAd, pTDLS, RsnLen, RsnIe, FTLen, FTIe, TILen, TIIe, StatusCode);

		pTDLS->ChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;
		pTDLS->ChSwitchTimeout = TDLS_CHANNEL_SWITCH_TIMEOUT;
		pTDLS->Status = TDLS_MODE_CONNECTED;
		pAd->StaCfg.TdlsInfo.TdlsForcePowerSaveWithAP = FALSE;

		TDLS_UAPSD_ENTRY_INIT(pTDLS);

#ifdef TDLS_AUTOLINK_SUPPORT
		if (pAd->StaCfg.TdlsInfo.TdlsAutoLink)
		{
			PTDLS_DISCOVERY_ENTRY pPeerEntry = NULL;
			PLIST_HEADER	pTdlsDiscovEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
			PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;

			RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
			TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscovEnList, pTDLS->MacAddr, TRUE);
			pPeerEntry = TDLS_FindDiscoveryEntry(pTdlsDiscovEnList, pTDLS->MacAddr);
			if (pPeerEntry)
			{
				pPeerEntry->bConnectedFirstTime = TRUE;
				pPeerEntry->bConnected = TRUE;
				pPeerEntry->RetryCount = 0;
				pPeerEntry->CurrentState = TDLS_DISCOVERY_TO_SETUP_DONE;
			}
			RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);

			RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
			TDLS_DelBlackEntryByMAC(pTdlsBlackEnList, pTDLS->MacAddr);
			RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
		}
#endif // TDLS_AUTOLINK_SUPPORT //
	}
	else
	{
		if ((pTDLS->Status >= TDLS_MODE_CONNECTED) && (pTDLS->Token == Token))
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s() - receive the same token TDLS response !!!\n", __FUNCTION__));
			goto CleanUp;
		}

		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;
	}

CleanUp:
#ifdef WFD_SUPPORT
	os_free_mem(pAd, WfdSubelement);
#endif /* WFD_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerSetupConfAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
	BOOLEAN			TimerCancelled;	
	PRT_802_11_TDLS	pTDLS = NULL;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	EDCA_PARM		EdcaParm;
	INT				LinkId = 0xff;
	UCHAR			RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	TDLS_LINK_IDENT_ELEMENT	LinkIdent;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		return;
	
	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS setup confirm receive pack", Elem->Msg, Elem->MsgLen);

	// Init all kinds of fields within the packet
	NdisZeroMemory(&EdcaParm, sizeof(EdcaParm));
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));
	NdisZeroMemory(&LinkIdent, sizeof(LinkIdent));

	if (!PeerTdlsSetupConfSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&EdcaParm,
							&StatusCode,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		LocalStatusCode = MLME_REQUEST_DECLINED;
	}

	if (StatusCode != MLME_SUCCESS)
	{
		COPY_MAC_ADDR(PeerAddr, &pFrame->Hdr.Addr3);
	}

	DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() received a confirm from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n",
							PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5], StatusCode));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		return;

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];
	// Cancel the timer since the received packet to me.
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);



	// Received a error code from the Peer TDLS.
	// Let's terminate the setup procedure right now.
	if (StatusCode != MLME_SUCCESS)
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;

#ifdef WFD_SUPPORT
		if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
			(StatusCode == MLME_SECURITY_WEAK))
			pAd->StaCfg.WfdCfg.TdlsSecurity = WFD_TDLS_WEAK_SECURITY;
#endif /* WFD_SUPPORT */
		
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() received a failed StatusCode, terminate the setup procedure \n"));

		return;
	}
	else
		StatusCode = LocalStatusCode;

	// 
	// Validate the content on Setup Confirm Frame
	//
	while (StatusCode == MLME_SUCCESS)
	{		
		// Invalid TDLS State
		if (pTDLS->Status != TDLS_MODE_WAIT_CONFIRM)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() Not in TDLS_MODE_WAIT_CONFIRM STATE\n"));		
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}

		// Is the same Dialog Token?
		if (pTDLS->Token != Token)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() Not match with Dialig Token \n"));
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}

		// Process TPK Handshake Message 3 here!
		if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
		{		
			USHORT Result;

			// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
			Result = TDLS_TPKMsg3Process(pAd, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
			if (Result != MLME_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR,("TDLS - TPKMsg3Process() Failed, reason=%d \n", Result));
				StatusCode = Result;
				break;
			}
		}

		// Update parameters
		if (StatusCode == MLME_SUCCESS)
		{
			// I am Responder.And peer are Initiator
			pTDLS->bInitiator = TRUE;
			
			// Copy EDCA Parameters
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
				NdisMoveMemory(&pTDLS->EdcaParm, &EdcaParm, sizeof(EDCA_PARM));
			else
				NdisZeroMemory(&pTDLS->EdcaParm, sizeof(EDCA_PARM));

			// Copy TPK related information
			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{
				// SNonce, Key lifetime
			}
		}

		break;
	}

	//
	// Insert into mac table
	//
	if (StatusCode == MLME_SUCCESS)
	{
		// allocate one MAC entry
		pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);
		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - MacTable Entry exist !!!\n"));
		else
			pMacEntry = MacTableInsertEntry(pAd, pTDLS->MacAddr, &pAd->StaCfg.wdev, BSS0 + MIN_NET_DEVICE_FOR_TDLS, OPMODE_STA, TRUE);

		if (pMacEntry)
		{
			pTDLS->MacTabMatchWCID = pMacEntry->Aid;
			pMacEntry->AuthMode = pAd->StaCfg.wdev.AuthMode;
			pMacEntry->WepStatus = pAd->StaCfg.wdev.WepStatus;
			pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			pMacEntry->Sst = SST_ASSOC;
			pMacEntry->MatchTdlsEntryIdx = LinkId;

#ifdef UAPSD_SUPPORT
			/* update UAPSD */
			pTDLS->QosCapability |= (pTDLS->EdcaParm.EdcaUpdateCount & 0x0f);

			UAPSD_AssocParse(pAd, pMacEntry, &pTDLS->QosCapability,
							pAd->StaCfg.UapsdInfo.bAPSDCapable);
#endif /* UAPSD_SUPPORT */

			DBGPRINT(RT_DEBUG_ERROR, ("MacTableInsertTDlsEntry - allocate entry #%d, Total= %d\n",pMacEntry->Aid, pAd->MacTab.Size));

			// Set WMM capability
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
			{
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
				DBGPRINT(RT_DEBUG_ERROR, ("TDLS -  WMM Capable\n"));
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			TDLS_InitPeerEntryRateCapability(pAd,
											pMacEntry,
											&pTDLS->CapabilityInfo,
											pTDLS->SupRateLen,
											pTDLS->SupRate,
											pTDLS->HtCapabilityLen,
											&pTDLS->HtCapability);

			RTMPSetSupportMCS(pAd,
								OPMODE_STA,
								pMacEntry,
								pTDLS->SupRate,
								pTDLS->SupRateLen,
								pTDLS->ExtRate,
								pTDLS->ExtRateLen,
#ifdef DOT11_VHT_AC
								pTDLS->vht_cap_len,
								pTDLS->vht_cap,
#endif /* DOT11_VHT_AC */
								&pTDLS->HtCapability,
								pTDLS->HtCapabilityLen);

			//
			// Install Peer Key if RSNA Enabled
			//
			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{	
				// Write to ASIC on-chip table.
				if ( pMacEntry->Aid > 1)
				{
					CIPHER_KEY PairwiseKey;

					PairwiseKey.CipherAlg = CIPHER_AES;

					// Set Peer Key
					PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(PairwiseKey.Key, &pTDLS->TPK[16], LEN_TK);

					RTMP_ASIC_PAIRWISE_KEY_TABLE(pAd,
											pMacEntry->Aid,
											&PairwiseKey);															

					RTMP_SET_WCID_SEC_INFO(pAd, 
										BSS0,
										0, 
										PairwiseKey.CipherAlg, 
										pMacEntry->Aid,
										PAIRWISEKEYTABLE);

					NdisMoveMemory(&pMacEntry->PairwiseKey, &PairwiseKey, sizeof(CIPHER_KEY));

					pMacEntry->AuthMode = Ndis802_11AuthModeWPA2PSK;
					pMacEntry->WepStatus = Ndis802_11Encryption3Enabled;
					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}	
				
			}

			pTDLS->Status = TDLS_MODE_CONNECTED;
			pTDLS->ChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;
			pTDLS->ChSwitchTimeout = TDLS_CHANNEL_SWITCH_TIMEOUT;
			pAd->StaCfg.TdlsInfo.TdlsForcePowerSaveWithAP = FALSE;

			TDLS_UAPSD_ENTRY_INIT(pTDLS);

#ifdef TDLS_AUTOLINK_SUPPORT
			if (pAd->StaCfg.TdlsInfo.TdlsAutoLink)
			{
				PTDLS_DISCOVERY_ENTRY pPeerEntry = NULL;
				PLIST_HEADER	pTdlsDiscovEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
				PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;

				RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
				TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscovEnList, pTDLS->MacAddr, TRUE);
				pPeerEntry = TDLS_FindDiscoveryEntry(pTdlsDiscovEnList, pTDLS->MacAddr);
				if (pPeerEntry)
				{
					pPeerEntry->bConnectedFirstTime = TRUE;
					pPeerEntry->bConnected = TRUE;
					pPeerEntry->RetryCount = 0;
					pPeerEntry->CurrentState = TDLS_DISCOVERY_TO_SETUP_DONE;
				}
				RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);

				RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
				TDLS_DelBlackEntryByMAC(pTdlsBlackEnList, pTDLS->MacAddr);
				RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
			}
#endif // TDLS_AUTOLINK_SUPPORT //
		}
		else
		{
			StatusCode = MLME_REQUEST_DECLINED;
			pTDLS->Status = TDLS_MODE_NONE;
			pTDLS->Token = 0;
			pTDLS->Valid = FALSE;
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_PeerSetupConfAction() MacTableInsertEntry failed\n"));
		}
	}
	else
	{
		if ((pTDLS->Status >= TDLS_MODE_CONNECTED) && (pTDLS->Token == Token))
		{
			DBGPRINT(RT_DEBUG_WARN, ("TDLS - TDLS_PeerSetupConfAction() receive the same token TDLS confirm !!!\n"));
			return;
		}

		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Token = 0;
		pTDLS->Valid = FALSE;
		StatusCode = MLME_REQUEST_DECLINED;
	}

	pAd->StaCfg.TdlsInfo.TdlsDialogToken = Token;

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	PRT_802_11_TDLS	pTDLS = NULL;
	UINT16			ReasonCode;
	BOOLEAN			IsViaAP = FALSE;

	DBGPRINT(RT_DEBUG_ERROR,("TDLS ===> TDLS_MlmeTearDownAction() \n"));

	if (!MlmeTdlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pTDLS, &ReasonCode, &IsViaAP))
		return;

	/* Build TDLS Setup Request Frame via direct link*/
	NStatus = TDLS_TearDownAction(pAd, pTDLS, ReasonCode, TRUE);

	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeTearDownAction() Build Setup Request Fail !!!\n"));
	}
	else
	{
		BOOLEAN TimerCancelled;

		if (!VALID_WCID(pTDLS->MacTabMatchWCID))
			return;

		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Token = 0;
		pTDLS->Valid = FALSE;
		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

		MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
	}

	DBGPRINT(RT_DEBUG_ERROR,("TDLS <=== TDLS_MlmeTearDownAction() \n"));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR				SA[MAC_ADDR_LEN];
	USHORT				ReasonCode;
	PRT_802_11_TDLS		pTDLS = NULL;
	INT					LinkId = 0xff;
	BOOLEAN				IsInitator;
	BOOLEAN				TimerCancelled;
	UCHAR				FTLen;
	UCHAR				FTIe[128];
	PHEADER_802_11 pHdr;

	DBGPRINT(RT_DEBUG_ERROR,("TDLS ===> TDLS_PeerTearDownAction() \n"));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS TearDown receive pack", Elem->Msg, Elem->MsgLen);

	// Init FTIe
	NdisZeroMemory(FTIe, sizeof(FTIe));
	
	if (!PeerTdlsTearDownSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								SA,
								&IsInitator,
								&ReasonCode,
								&FTLen,
								FTIe))
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x Sanity Check Fail\n",
								SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));
		return;
	}

	pHdr = (PHEADER_802_11)Elem->Msg;

	if (pHdr->FC.FrDs == 1)
	{
		COPY_MAC_ADDR(SA, &pHdr->Addr3);
	}
	else
	{
		COPY_MAC_ADDR(SA, &pHdr->Addr2);
	}

	DBGPRINT(RT_DEBUG_OFF,("TDLS - PeerTdlsTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x with ReasonCode=%d\n",
							SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], ReasonCode));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, SA);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsTearDownAction() can not find from %02x:%02x:%02x:%02x:%02x:%02x on TDLS entry !!!\n",
								SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));
		return;
	}
	
	// Point to the current Link ID
	pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	// Cancel the timer since the received packet to me.
	//RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

	// Drop mismatched identifier.
	if (pTDLS->bInitiator != IsInitator)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsTearDownAction() pTDLS->bInitiator = %x parse = %x !!!\n",
								pTDLS->bInitiator, IsInitator));
		return;
	}


	// clear tdls table entry
	if (pTDLS->Valid && MAC_ADDR_EQUAL(SA, pTDLS->MacAddr))
	{
#ifdef TDLS_AUTOLINK_SUPPORT
		PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;
		PLIST_HEADER	pTdlsDiscoveryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
#endif // TDLS_AUTOLINK_SUPPORT //

		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Token = 0;
		pTDLS->Valid = FALSE;
		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

		MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, SA);

#ifdef TDLS_AUTOLINK_SUPPORT
		RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
		TDLS_InsertBlackEntryByMAC(pTdlsBlackEnList, SA, TDLS_BLACK_TDLS_BY_TEARDOWN);
		RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);

		RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
		TDLS_DelDiscoveryEntryByMAC(pTdlsDiscoveryEnList, SA);
		RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
#endif // TDLS_AUTOLINK_SUPPORT //
	}

	DBGPRINT(RT_DEBUG_ERROR,("TDLS <=== TDLS_PeerTearDownAction() \n"));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_LinkTimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRT_802_11_TDLS 		pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER			pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("TdlsTimeout - Failed to wait for the response, terminate the setup procedure (%02x:%02x:%02x:%02x:%02x:%02x)\n",
								pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5]));

	/*
		11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA
		When no corresponding TDLS Peer Traffic Response frame has been
		received within dot11TDLSResponseTimeout after sending a TDLS Peer
		Traffic Indication frame, the STA shall tear down the direct link.
	*/
	pAd = pTDLS->pAd;
	if (pTDLS->FlgIsWaitingUapsdTraRsp == TRUE)
	{
		/* timeout for traffic response frame */

		/* TODO: tear down the link with the peer */
		DBGPRINT(RT_DEBUG_OFF, ("tdls uapsd> traffic rsp timeout!!!\n"));
		pTDLS->FlgIsWaitingUapsdTraRsp = FALSE;
		TDLS_TearDownPeerLink(pAd, pTDLS->MacAddr, FALSE);
		return;
	}

	if ((pTDLS) && (pTDLS->Valid) && (pTDLS->Status < TDLS_MODE_CONNECTED))
	{
		pTDLS->Valid	= FALSE;
		pTDLS->Status	= TDLS_MODE_NONE;
	}
}
#endif /* DOT11Z_TDLS_SUPPORT */

