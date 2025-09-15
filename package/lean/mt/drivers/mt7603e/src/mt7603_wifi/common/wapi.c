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
 ***************************************************************************

	Module Name:
	wapi.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Albert		2008-4-3      	Supoort WAPI protocol
*/
/*#include <linux/stdio.h> */
/*#include <linux/stdlib.h> */
/*#include <linux/string.h> */
/*#include <linux/time.h> */

#ifdef WAPI_SUPPORT

#include "rt_config.h"

/* WAPI AKM OUI */
UCHAR   OUI_WAI_CERT_AKM[4]   	= {0x00, 0x14, 0x72, 0x01};
UCHAR   OUI_WAI_PSK_AKM[4]   	= {0x00, 0x14, 0x72, 0x02};

/* WAPI CIPHER OUI */
UCHAR	OUI_WPI_CIPHER_SMS4[4] = {0x00, 0x14, 0x72, 0x01};

UCHAR	WAPI_TYPE[] = {0x88, 0xb4};

/* IV default value */
UCHAR 	AE_BCAST_PN[LEN_WAPI_TSC] = {0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};
UCHAR 	ASUE_UCAST_PN[LEN_WAPI_TSC] = {0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									   0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};
UCHAR 	AE_UCAST_PN[LEN_WAPI_TSC] = {0x37, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};

BUILD_TIMER_FUNCTION(RTMPWapiUskRekeyPeriodicExec);
BUILD_TIMER_FUNCTION(RTMPWapiMskRekeyPeriodicExec);

static void kd_hmac_sha256(	
    unsigned char 	*key, 
    unsigned int 	key_len,
    unsigned char 	*text, 
	unsigned int 	text_len,
    unsigned char 	*output, 
    unsigned int 	output_len)
{
	int i;

	for (i = 0; output_len/SHA256_DIGEST_SIZE; i++, output_len -= SHA256_DIGEST_SIZE)
	{
		RT_HMAC_SHA256(key, key_len, text, text_len, &output[i*SHA256_DIGEST_SIZE], SHA256_DIGEST_SIZE);
		text = &output[i*SHA256_DIGEST_SIZE];
		text_len = SHA256_DIGEST_SIZE;
	}

	if (output_len > 0)
		RT_HMAC_SHA256(key, key_len, text, text_len, &output[i*SHA256_DIGEST_SIZE], output_len);

}


/*
	========================================================================
	
	Routine Description:
		Build WAPI IE in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  		AuthMode	-	indicate the authentication mode 
    	WepStatus	-	indicate the encryption type
		
	Return Value:
		
	Note:
		
	========================================================================
*/
VOID RTMPInsertWapiIe(	
	IN	UINT			AuthMode,
	IN	UINT			WepStatus,
	OUT	PUCHAR			pWIe,
	OUT	UCHAR			*w_len)
{			
	WAPIIE	*pWapiHdr = (WAPIIE*)pWIe;
	WAPIIE_UCAST *pWIE_ucast;
	WAPIIE_MCAST *pWIE_mcast;

	*w_len = 0;

	/* Assign the verson as 1 */
	pWapiHdr->version = 1;

	/* Set the AKM count and suite */
	pWapiHdr->acount = 1;
	switch (AuthMode)
	{
		case Ndis802_11AuthModeWAICERT:
			NdisMoveMemory(pWapiHdr->auth[0].oui, OUI_WAI_CERT_AKM, 4);
			break;

		case Ndis802_11AuthModeWAIPSK:
			NdisMoveMemory(pWapiHdr->auth[0].oui, OUI_WAI_PSK_AKM, 4);
			break;
	}

	/* swap for big-endian platform */
	pWapiHdr->version = cpu2le16(pWapiHdr->version);
	pWapiHdr->acount = cpu2le16(pWapiHdr->acount);
	
	/* update current length */
	(*w_len) += sizeof(WAPIIE);	

	/* Set the unicast cipher and count */
	pWIE_ucast = (WAPIIE_UCAST*)(pWIe + (*w_len));
	pWIE_ucast->ucount = 1;
	NdisMoveMemory(pWIE_ucast->ucast[0].oui, OUI_WPI_CIPHER_SMS4, 4);

	/* swap for big-endian platform */
	pWIE_ucast->ucount = cpu2le16(pWIE_ucast->ucount);

	/* update current length */
	(*w_len) += sizeof(WAPIIE_UCAST);

	/* Set the multicast cipher and capability */
	pWIE_mcast = (WAPIIE_MCAST*)(pWIe + (*w_len));
	NdisMoveMemory(pWIE_mcast->mcast, OUI_WPI_CIPHER_SMS4, 4);
	pWIE_mcast->capability = 0;	/* Todo AlbertY - support pre-authentication */

	/* update current length */
	(*w_len) += sizeof(WAPIIE_MCAST);

}

/*
    ==========================================================================
    Description:
		Check whether the received frame is WAPI frame.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		pData			-	the received frame
		DataByteCount 	-	the received frame's length				
       
    Return:
         TRUE 			-	This frame is WAPI frame
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPCheckWAIframe(
    IN PUCHAR           pData,
    IN ULONG            DataByteCount)
{
    if(DataByteCount < (LENGTH_802_1_H + LENGTH_WAI_H))
        return FALSE;


	/* Skip LLC header */
    if (NdisEqualMemory(SNAP_802_1H, pData, 6)) 
    {
        pData += 6;
    }
	/* Skip 2-bytes EAPoL type */
    if (NdisEqualMemory(WAPI_TYPE, pData, 2)) 
    {
    	DBGPRINT(RT_DEBUG_TRACE, ("--> Receive a WAI frame \n"));
        pData += 2;         
    }
    else    
        return FALSE;
	
    return TRUE;
}


/*
    ==========================================================================
    Description:
		Check whether the cipher is SMS4.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		apidx			-	interface index				
       
    Return:
         TRUE 			-	The cipher is SMS4
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPIsWapiCipher(
    IN PRTMP_ADAPTER    pAd,
    IN UCHAR           	apidx)
{
	NDIS_802_11_ENCRYPTION_STATUS	cipher_mode = Ndis802_11EncryptionDisabled;

	/* Currently, WAPI only support MBSS */
	if (apidx >= MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (apidx < MAX_MBSSID_NUM(pAd))
			cipher_mode = pAd->ApCfg.MBSSID[apidx].wdev.WepStatus;
	}
#endif /* CONFIG_AP_SUPPORT */

	if (cipher_mode == Ndis802_11EncryptionSMS4Enabled)
		return TRUE;
	
    return FALSE;
}

/*
    ==========================================================================
    Description:
		Insert the WPI-SMS4 IV header

		+-------+------+-------------+
		| KeyId | resv | sequence PN |
		+-------+------+-------------+

	Arguments:
		
    Return:
         
    ==========================================================================
*/
VOID RTMPConstructWPIIVHdr(
	IN	UCHAR			key_id,
	IN	UCHAR			*tx_iv,
	OUT UCHAR 			*iv_hdr)
{
	iv_hdr[0] = key_id;
	iv_hdr[1] = 0x00;

	NdisMoveMemory(&iv_hdr[2], tx_iv, LEN_WAPI_TSC);
}

VOID RTMPDeriveWapiGTK(
	IN	PUCHAR			nmk,
	OUT	PUCHAR			gtk_ptr)
{
	const char group_context[100] = "multicast or station key expansion for station unicast and multicast and broadcast";		

	NdisZeroMemory(gtk_ptr, 32);
	kd_hmac_sha256(nmk, 
				   16, 
				   (UCHAR *)group_context, 
				   strlen(group_context), 
				   gtk_ptr,
				   32);	
}

VOID RT_SMS4_TEST(
	IN UINT8			test)
{
	CIPHER_KEY		CipherKey;
	UINT16			data_len;
	UINT8			rcvd_data[50];
	UINT8 mac_hdr_qos[] = {0x88, 0x42, 0x00, 0x00, 0x08, 0xda, 0x75, 0x84, 
						0xd0, 0xcc, 0x27, 0xe8, 0x72, 0xaa, 0x2c, 0xb9, 
						0x6b, 0xbb, 0xea, 0x35, 0xa4, 0x20, 0x1e, 0xd2, 
						0xcf, 0x14};
	
	UINT8 payload_qos[] = {0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00};
	UINT8 pn[] = 	 {0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76, 
					  0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89};
	UINT8 key[] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
				   0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
				   0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
				   0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe};
		
	RTMPSoftEncryptSMS4(mac_hdr_qos, 
						payload_qos, 
						1, 
						1, 
						key, 
						pn);

	hex_dump("encrypted payload", payload_qos, 17);

	NdisZeroMemory(&CipherKey, sizeof(CIPHER_KEY));
	NdisMoveMemory(CipherKey.Key, key, 16);
	NdisMoveMemory(CipherKey.TxMic, &key[16], 8);
	NdisMoveMemory(CipherKey.RxMic, &key[24], 8);	
	CipherKey.KeyLen = 16;


	NdisZeroMemory(rcvd_data, 50);
	rcvd_data[0] = 1;
	data_len = 2;
	NdisMoveMemory(&rcvd_data[data_len], pn, 16);
	data_len += 16;
	NdisMoveMemory(&rcvd_data[data_len], payload_qos, 17);
	data_len += 17;


	if (RTMPSoftDecryptSMS4(mac_hdr_qos, 
							FALSE, 
							&CipherKey, 
							rcvd_data, 
							&data_len) == 0)
		hex_dump("decrypted payload", rcvd_data, data_len);
	else
		printk("decrypted fail\n");
}

/*
    ========================================================================

    Routine Description:
        In kernel mode read parameters from file

    Arguments:
        src                     the location of the file.
        dest                        put the parameters to the destination.
        Length                  size to read.

    Return Value:
        None

    Note:

    ========================================================================
*/
void rtmp_read_wapi_parms_from_file(
		IN  PRTMP_ADAPTER pAd, 
		RTMP_STRING *tmpbuf, 
		RTMP_STRING *buffer)
{	
	UINT32					ip_addr;
#ifdef CONFIG_AP_SUPPORT	
	INT						apidx = 0;
#endif /* CONFIG_AP_SUPPORT */
	RTMP_STRING tok_str[32];
	INT idx;

	PCOMMON_WAPI_INFO pInfo = &pAd->CommonCfg.comm_wapi_info;
	
	/* wapi interface name */
	if (RTMPGetKeyParameter("Wapiifname", tmpbuf, 32, buffer, TRUE))
	{
		if (strlen(tmpbuf) > 0)
		{
			NdisMoveMemory(pInfo->wapi_ifname, tmpbuf, strlen(tmpbuf));
			pInfo->wapi_ifname_len = strlen(tmpbuf); 
			
			DBGPRINT(RT_DEBUG_TRACE, ("Wapiifname=%s, len=%d\n", 
														pInfo->wapi_ifname, 
														pInfo->wapi_ifname_len));
		}
	}
	

	/* WapiAsCertPath */
	if (RTMPGetKeyParameter("WapiAsCertPath", tmpbuf, 128, buffer, TRUE))
	{
		if (strlen(tmpbuf) > 0)
		{
			NdisMoveMemory(pInfo->as_cert_path[0], tmpbuf, strlen(tmpbuf));
			pInfo->as_cert_path_len[0] = strlen(tmpbuf);
			pInfo->as_cert_no = 1;
			
			DBGPRINT(RT_DEBUG_TRACE, ("WapiAsCertPath=%s, len=%d\n", 
														pInfo->as_cert_path[0], 
														pInfo->as_cert_path_len[0]));
		}
	}

	/* WapiAsCertPath2 ~ WapiAsCertPath10 */
	for (idx = 1; idx < MAX_ID_NO; idx++)
	{
		sprintf(tok_str, "WapiAsCertPath%d", idx + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
			if (strlen(tmpbuf) > 0)
			{
				NdisMoveMemory(pInfo->as_cert_path[idx], tmpbuf, strlen(tmpbuf));
				pInfo->as_cert_path_len[idx] = strlen(tmpbuf);
				pInfo->as_cert_no++;
				
				DBGPRINT(RT_DEBUG_TRACE, ("WapiAsCertPath%d=%s, len=%d\n", 
															idx+1, 
															pInfo->as_cert_path[idx], 
															pInfo->as_cert_path_len[idx]));
			}
		}
	}
	
	/* WapiCaCertPath */
	if (RTMPGetKeyParameter("WapiCaCertPath", tmpbuf, 128, buffer, TRUE))
	{
		if (strlen(tmpbuf) > 0)
		{
			NdisMoveMemory(pInfo->ca_cert_path, tmpbuf, strlen(tmpbuf));
			pInfo->ca_cert_path_len = strlen(tmpbuf); 
			
			DBGPRINT(RT_DEBUG_TRACE, ("WapiCaCertPath=%s, len=%d\n", 
														pInfo->ca_cert_path, 
														pInfo->ca_cert_path_len));
		}
	}

	/* WapiUserCertPath */
	if (RTMPGetKeyParameter("WapiUserCertPath", tmpbuf, 128, buffer, TRUE))
	{
		if (strlen(tmpbuf) > 0)
		{
			NdisMoveMemory(pInfo->user_cert_path, tmpbuf, strlen(tmpbuf));
			pInfo->user_cert_path_len = strlen(tmpbuf); 
			
			DBGPRINT(RT_DEBUG_TRACE, ("WapiUserCertPath=%s, len=%d\n", 
														pInfo->user_cert_path, 
														pInfo->user_cert_path_len));
		}
	}

	/* WapiAsIpAddr */
	if (RTMPGetKeyParameter("WapiAsIpAddr", tmpbuf, 32, buffer, TRUE))
	{
		if (rtinet_aton(tmpbuf, &ip_addr))
     	{
            pInfo->wapi_as_ip = ip_addr;  
			DBGPRINT(RT_DEBUG_TRACE, ("WapiAsIpAddr=%s(%x)\n", tmpbuf, pInfo->wapi_as_ip));
		}	    
	}

	/* WapiAsPort */
	if (RTMPGetKeyParameter("WapiAsPort", tmpbuf, 32, buffer, TRUE))
	{
		pInfo->wapi_as_port = simple_strtol(tmpbuf, 0, 10); 
		DBGPRINT(RT_DEBUG_TRACE, ("WapiAsPort=%d\n", pInfo->wapi_as_port));			   
	}

	/* WapiUskRekeyMethod */
	if (RTMPGetKeyParameter("WapiUskRekeyMethod", tmpbuf, 32, buffer, TRUE))
	{		
		if ((strcmp(tmpbuf, "TIME") == 0) || (strcmp(tmpbuf, "time") == 0))
			pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_TIME;
		else if ((strcmp(tmpbuf, "PKT") == 0) || (strcmp(tmpbuf, "pkt") == 0))
			pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_PKT;
		else
			pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_DISABLE;

		DBGPRINT(RT_DEBUG_TRACE, ("WapiUskRekeyMethod=%d\n", pAd->CommonCfg.wapi_usk_rekey_method));			   
	}

	/* WapiUskRekeyThreshold */
	if (RTMPGetKeyParameter("WapiUskRekeyThreshold", tmpbuf, 32, buffer, TRUE))
	{			
		pAd->CommonCfg.wapi_usk_rekey_threshold = simple_strtol(tmpbuf, 0, 10); 
		DBGPRINT(RT_DEBUG_TRACE, ("WapiUskRekeyThreshold=%d\n", pAd->CommonCfg.wapi_usk_rekey_threshold));			   
	}

	/* WapiMskRekeyMethod */
	if (RTMPGetKeyParameter("WapiMskRekeyMethod", tmpbuf, 32, buffer, TRUE))
	{		
		if ((strcmp(tmpbuf, "TIME") == 0) || (strcmp(tmpbuf, "time") == 0))
			pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_TIME;
		else if ((strcmp(tmpbuf, "PKT") == 0) || (strcmp(tmpbuf, "pkt") == 0))
			pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_PKT;
		else
			pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_DISABLE;

		DBGPRINT(RT_DEBUG_TRACE, ("WapiMskRekeyMethod=%d\n", pAd->CommonCfg.wapi_msk_rekey_method));			   
	}

	/* WapiMskRekeyThreshold */
	if (RTMPGetKeyParameter("WapiMskRekeyThreshold", tmpbuf, 32, buffer, TRUE))
	{
		pAd->CommonCfg.wapi_msk_rekey_threshold = simple_strtol(tmpbuf, 0, 10); 
		DBGPRINT(RT_DEBUG_TRACE, ("WapiMskRekeyThreshold=%d\n", pAd->CommonCfg.wapi_msk_rekey_threshold));			   
	}
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{						
		RTMP_STRING tok_str[16];

		/* WapiPskX */
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			snprintf(tok_str, sizeof(tok_str), "WapiPsk%d", apidx + 1);
			
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].WAPIPassPhrase, 64);
			pAd->ApCfg.MBSSID[apidx].WAPIPassPhraseLen = 0;
			if(RTMPGetKeyParameter(tok_str, tmpbuf, 65, buffer, FALSE))
			{								    
			    if (strlen(tmpbuf) >= 8 && strlen(tmpbuf) <= 64)
			    {                                    
			        NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].WAPIPassPhrase, tmpbuf, strlen(tmpbuf));
			        pAd->ApCfg.MBSSID[apidx].WAPIPassPhraseLen = strlen(tmpbuf);
   					DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) WapiPsk=(%s), len=%d\n", apidx, tmpbuf, strlen(tmpbuf)));						
			    }
				else
				{
					if (pAd->ApCfg.MBSSID[apidx].wdev.AuthMode == Ndis802_11AuthModeWAIPSK)
					{
						pAd->ApCfg.MBSSID[apidx].wdev.AuthMode = Ndis802_11AuthModeOpen;
						pAd->ApCfg.MBSSID[apidx].wdev.WepStatus = Ndis802_11EncryptionDisabled;
					}
					DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) The length of WAPI PSKPassPhrase is invalid(len=%d). \n", apidx, strlen(tmpbuf)));
				}																			
			}
		}					
	}
#endif /* CONFIG_AP_SUPPORT */

	

	/* WapiPskType */
	if (RTMPGetKeyParameter("WapiPskType", tmpbuf, 32, buffer, TRUE))
	{		
		INT	err;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			RTMP_STRING *macptr;
			
			for (apidx = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), apidx++)
		    {
				err = 0;
			
				if (apidx >= pAd->ApCfg.BssidNum)
					break;

				/* HEX */
				if(simple_strtol(macptr, 0, 10) == 0)
				{
					pAd->ApCfg.MBSSID[apidx].WapiPskType = HEX_MODE;
		
					if (pAd->ApCfg.MBSSID[apidx].WAPIPassPhraseLen % 2 != 0)
					{
						err = 1;
						DBGPRINT(RT_DEBUG_ERROR, ("I/F(ra%d) The WAPI-PSK key length MUST be even in Hex mode\n", apidx));						
					}						
				}
				/* ASCII */
				else	
				{
					pAd->ApCfg.MBSSID[apidx].WapiPskType = ASCII_MODE;
				}
				
				if (err)
				{
					pAd->ApCfg.MBSSID[apidx].wdev.AuthMode = Ndis802_11AuthModeOpen;
					pAd->ApCfg.MBSSID[apidx].wdev.WepStatus = Ndis802_11EncryptionDisabled;
				}
				else
					DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) WapiPskType=%s\n", apidx, (pAd->ApCfg.MBSSID[apidx].WapiPskType == HEX_MODE) ? "HEX" : "ASCII"));
		    }
		}
#endif /* CONFIG_AP_SUPPORT */

				
	}

	/* Sanity check - USK rekey parameter */
	if (pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_DISABLE || 
		pAd->CommonCfg.wapi_usk_rekey_threshold == 0)
	{
		pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_DISABLE;
		pAd->CommonCfg.wapi_usk_rekey_threshold = 0;
	}

	/* Sanity check - MSK rekey parameter */
	if (pAd->CommonCfg.wapi_msk_rekey_method == REKEY_METHOD_DISABLE || 
		pAd->CommonCfg.wapi_msk_rekey_threshold == 0)
	{
		pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_DISABLE;
		pAd->CommonCfg.wapi_msk_rekey_threshold = 0;
	}

}

/* 
    ==========================================================================
    Description:
        It only shall be queried by wapi daemon for querying the related 
        configuration. This routine process the WAPI configuration for per BSS.
        
	==========================================================================
*/
static VOID RTMPQueryWapiConfPerBss(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PWAPI_CONF		wapi_conf_ptr,
	IN	UCHAR			apidx)
{
	PMBSS_WAPI_INFO pConf = &wapi_conf_ptr->mbss_wapi_info[apidx];		
 
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	
		if (pMbss->wdev.if_dev != NULL)
		{
			PNET_DEV dev = pMbss->wdev.if_dev;
			
			NdisMoveMemory(pConf->ifname, RtmpOsGetNetDevName(dev), strlen(RtmpOsGetNetDevName(dev)));
			pConf->ifname_len = strlen(RtmpOsGetNetDevName(dev));			
		}
		else
		{
			RTMP_STRING slot_name[IFNAMSIZ];

			snprintf(slot_name, sizeof(slot_name), "ra%d", apidx);
			NdisMoveMemory(pConf->ifname, slot_name, strlen(slot_name));
			pConf->ifname_len = strlen(slot_name);					
		}

		/* Decide the authentication mode */
		if (pMbss->wdev.AuthMode == Ndis802_11AuthModeWAICERT)
			pConf->auth_mode = WAPI_AUTH_CERT;
		else if (pMbss->wdev.AuthMode == Ndis802_11AuthModeWAIPSK)
			pConf->auth_mode = WAPI_AUTH_PSK;
		else
			pConf->auth_mode = WAPI_AUTH_DISABLE;

		/* Fill in WAI pre-shared key */
		if (pMbss->WAPIPassPhraseLen > 0)
		{
			if (pMbss->WapiPskType == HEX_MODE)
			{
				pConf->psk_len = pMbss->WAPIPassPhraseLen / 2;
				AtoH((RTMP_STRING *) pMbss->WAPIPassPhrase, (PUCHAR) pConf->psk, pConf->psk_len);
			}
			else
			{
				pConf->psk_len = pMbss->WAPIPassPhraseLen; 
				NdisMoveMemory(pConf->psk, pMbss->WAPIPassPhrase, pConf->psk_len);
			}
		}

		/* Fill in WIE */
		if (pMbss->RSNIE_Len[0] > 0)
		{
			pConf->wie_len = pMbss->RSNIE_Len[0] + 2; 

			pConf->wie[0] = IE_WAPI;
			pConf->wie[1] = pMbss->RSNIE_Len[0];
			NdisMoveMemory(&pConf->wie[2], pMbss->RSN_IE[0], pMbss->RSNIE_Len[0]);
		}
	}
#endif /* CONFIG_AP_SUPPORT */


}


/* 
    ==========================================================================
    Description:
        It only shall be queried by wapi daemon for querying the related 
        configuration.        
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlQueryWapiConf(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	UCHAR		apidx;	
	UCHAR		*buf = NULL;
	PWAPI_CONF	pConf;
	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlQueryWapiConf==>\n"));

	/* Allocate memory for WAPI configuration */
	os_alloc_mem(NULL, (PUCHAR *)&buf, sizeof(WAPI_CONF));

	if (buf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: allocate memory fail\n", __FUNCTION__));
		return;
	}

	pConf = (PWAPI_CONF)buf;
	
	NdisZeroMemory((PUCHAR)pConf, sizeof(WAPI_CONF));
	
	/* get MBSS number */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pConf->mbss_num = pAd->ApCfg.BssidNum;		
	}
#endif /* CONFIG_AP_SUPPORT */
	
	/* Set common configuration */
	NdisMoveMemory(&pConf->comm_wapi_info, &pAd->CommonCfg.comm_wapi_info, sizeof(COMMON_WAPI_INFO));

	for (apidx = 0; apidx < pConf->mbss_num; apidx++)
	{
		RTMPQueryWapiConfPerBss(pAd, pConf, apidx);				
	}
				
	wrq->u.data.length = sizeof(WAPI_CONF);
	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	os_free_mem(NULL, buf);
}

/*
    ==========================================================================
    Description:
        Timer execution function for periodically updating WAPI key.
    Return:
    ==========================================================================
*/  
VOID RTMPWapiUskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    UINT32          	tmp_cnt = 0;    
    PMAC_TABLE_ENTRY 	pEntry = (PMAC_TABLE_ENTRY)FunctionContext;
	PRTMP_ADAPTER 		pAd = (PRTMP_ADAPTER)pEntry->pAd;
	
	
    if (pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_TIME)
    {
		tmp_cnt = (++pEntry->wapi_usk_rekey_cnt);
    }
	else if (pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
	{
		/* the unit is 1K packets */
		tmp_cnt = pEntry->wapi_usk_rekey_cnt/1000;
	}
	else
		return;

	/* Trigger rekey procedure */
	if (tmp_cnt > pAd->CommonCfg.wapi_usk_rekey_threshold)
	{		
		pEntry->wapi_usk_rekey_cnt = 0;
		WAPI_InternalCmdAction(pAd, 
			   				   pEntry->AuthMode,
			   				   pEntry->func_tb_idx,
			   				   pEntry->Addr,
			   				   WAI_MLME_UPDATE_USK);
	}
				         
}


/*
    ==========================================================================
    Description:
        Timer execution function for periodically updating WAPI key.
    Return:
    ==========================================================================
*/  
VOID RTMPWapiMskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    UINT            i;
    UINT32          tmp_cnt = 0;    
    PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)FunctionContext;


	/* if no any WAPI STA associated, don't do anything. */
	if (pAd->MacTab.fAnyWapiStation == FALSE)
		return;

	/* increase counter for TIME method */
    if (pAd->CommonCfg.wapi_msk_rekey_method == REKEY_METHOD_TIME)
    {
		tmp_cnt = (++pAd->CommonCfg.wapi_msk_rekey_cnt);
    }
	else if (pAd->CommonCfg.wapi_msk_rekey_method == REKEY_METHOD_PKT)
	{
		/* the unit is 1K packets */
		tmp_cnt = pAd->CommonCfg.wapi_msk_rekey_cnt/1000;
	}
	else
		return;	

	if (tmp_cnt > pAd->CommonCfg.wapi_msk_rekey_threshold)
	{
		pAd->CommonCfg.wapi_msk_rekey_cnt = 0;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			INT apidx = 0;
    		UINT cnt;
			
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
			{				
				UINT m_wcid;
			
				pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId = pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId == 0 ? 1 : 0;
				inc_iv_byte(pAd->ApCfg.MBSSID[apidx].key_announce_flag, LEN_WAPI_TSC, 1);

				/* Generate NMK randomly */
				for (cnt = 0; cnt < 16; cnt++)
					pAd->ApCfg.MBSSID[apidx].NMK[cnt] = RandomByte(pAd);
			
				RTMPDeriveWapiGTK(pAd->ApCfg.MBSSID[apidx].NMK, pAd->ApCfg.MBSSID[apidx].GTK);				

				GET_GroupKey_WCID(pAd, m_wcid, apidx);
				/* Install Shared key */
				WAPIInstallSharedKey(pAd, 
									 pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus, 
									 apidx, 
									 pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId, 
									 m_wcid,
									 pAd->ApCfg.MBSSID[apidx].GTK);
			}					
		}
#endif /* CONFIG_AP_SUPPORT */
		
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			MAC_TABLE_ENTRY  *pEntry;
			STA_TR_ENTRY *tr_entry = NULL;

			pEntry = &pAd->MacTab.Content[i];
			tr_entry = &pAd->MacTab.tr_entry[i];
			if (IS_ENTRY_CLIENT(pEntry) && 
				(pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled) &&
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				WAPI_InternalCmdAction(pAd, 
				   				   	   pEntry->AuthMode,
			   						   pEntry->func_tb_idx,
			   						   pEntry->Addr,
			   						   WAI_MLME_UPDATE_MSK);
			}
		}
	}
   		
}


VOID RTMPInitWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	if (pEntry)
	{
		DBGPRINT(RT_DEBUG_TRACE, (" RTMPInitWapiRekeyTimerAction : WAPI USK rekey timer (wcid-%d) \n", pEntry->wcid));
		RTMPInitTimer(pAd, &pEntry->WapiUskRekeyTimer, GET_TIMER_FUNCTION(RTMPWapiUskRekeyPeriodicExec), pEntry, TRUE);
		pEntry->WapiUskRekeyTimerRunning = FALSE;
	}
	else
	{
		RTMPInitTimer(pAd, &pAd->CommonCfg.WapiMskRekeyTimer, GET_TIMER_FUNCTION(RTMPWapiMskRekeyPeriodicExec), pAd, TRUE);
		pAd->CommonCfg.WapiMskRekeyTimerRunning = FALSE;
	}
}

VOID RTMPStartWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{	
	if (pEntry)
	{
		if ((pAd->CommonCfg.wapi_usk_rekey_method != REKEY_METHOD_DISABLE) && 
			(pAd->CommonCfg.wapi_usk_rekey_threshold > 0))
		{
			/* Regularly check the timer */
			if (pEntry->WapiUskRekeyTimerRunning == FALSE)
			{
				RTMPSetTimer(&pEntry->WapiUskRekeyTimer, WAPI_KEY_UPDATE_EXEC_INTV);

				pEntry->WapiUskRekeyTimerRunning = TRUE;
				pEntry->wapi_usk_rekey_cnt = 0;
				DBGPRINT(RT_DEBUG_TRACE, (" RTMPStartWapiRekeyTimerAction : WAPI USK rekey timer is started (%d) \n", pAd->CommonCfg.wapi_usk_rekey_threshold));
			}							
		}
	}
	else
	{
		if ((pAd->CommonCfg.wapi_msk_rekey_method != REKEY_METHOD_DISABLE) && 
			(pAd->CommonCfg.wapi_msk_rekey_threshold > 0))
		{
			/* Regularly check the timer */
			if (pAd->CommonCfg.WapiMskRekeyTimerRunning == FALSE)
			{
				RTMPSetTimer(&pAd->CommonCfg.WapiMskRekeyTimer, WAPI_KEY_UPDATE_EXEC_INTV);

				pAd->CommonCfg.WapiMskRekeyTimerRunning = TRUE;
				pAd->CommonCfg.wapi_msk_rekey_cnt = 0;
				DBGPRINT(RT_DEBUG_TRACE, (" RTMPStartWapiRekeyTimerAction : WAPI MSK rekey timer is started \n"));				
			}
		}
	}

}

VOID RTMPCancelWapiRekeyTimerAction(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{	
	if(pEntry)
	{
		if (pEntry->WapiUskRekeyTimerRunning == TRUE)
		{
			BOOLEAN	Cancelled;

			RTMPCancelTimer(&pEntry->WapiUskRekeyTimer, &Cancelled);
			pEntry->wapi_usk_rekey_cnt = 0;
			pEntry->WapiUskRekeyTimerRunning = FALSE;
		}
	}
	else
	{
		if (pAd->CommonCfg.WapiMskRekeyTimerRunning == TRUE)
		{
			BOOLEAN	Cancelled;

			RTMPCancelTimer(&pAd->CommonCfg.WapiMskRekeyTimer, &Cancelled);
			pAd->CommonCfg.wapi_msk_rekey_cnt = 0;
			pAd->CommonCfg.WapiMskRekeyTimerRunning = FALSE;
		}
	}
	
}

/*
	========================================================================
	
	Routine Description:
		Prepare a L2 frame to wapi daemon to trigger WAPI state machine

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:
		
	Note:
		
	========================================================================
*/
BOOLEAN WAPI_InternalCmdAction(
		IN  PRTMP_ADAPTER		pAd,
		IN	UCHAR				AuthMode,
		IN	UCHAR				apidx,
		IN	PUCHAR				pAddr,
		IN	UCHAR				flag)
{
	// TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx
    if ((AuthMode == Ndis802_11AuthModeWAICERT) || 
		(AuthMode == Ndis802_11AuthModeWAIPSK))
	{				
		UCHAR			WAPI_IE[] = {0x88, 0xb4};
		UINT8			frame_len = LENGTH_802_3 + 12; /* 12 indicates the WAPI internal command length */
		UCHAR			FrameBuf[frame_len];
		UINT8			offset = 0;
		
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);
		
		/* Prepare the 802.3 header */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pAddr, WAPI_IE); 
		}
#endif /* CONFIG_AP_SUPPORT */
		offset += LENGTH_802_3;

		/* Prepare the specific WAPI header */
		NdisMoveMemory(&FrameBuf[offset], RALINK_OUI, 3);
		offset += 3;

		/* Set the state of this command */
		FrameBuf[offset] = flag;

		DBGPRINT(RT_DEBUG_TRACE, ("Trigger WAPI for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pAddr)));

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;	

	}	

	return TRUE;
}	


VOID RTMPGetWapiTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UINT			Wcid,
	OUT	UCHAR			*tx_tsc)
{
	USHORT			offset;	
	int				i;

	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;
		UINT32 wapi_pn_base = 0, wapi_pn_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
			wapi_pn_base = RLT_WAPI_PN_TABLE_BASE;
			wapi_pn_size = RLT_WAPI_PN_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
			wapi_pn_base = WAPI_PN_TABLE_BASE;
			wapi_pn_size = WAPI_PN_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		NdisZeroMemory(tx_tsc, LEN_WAPI_TSC);

		/* Read IVEIV from Asic */
		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);				
		for (i=0 ; i < iveiv_tb_size; i++)
			RTMP_IO_READ8(pAd, offset+i, &tx_tsc[i]); 

		/* Read WAPI PM from Asic */
		offset = wapi_pn_base + (Wcid * wapi_pn_size);
		for (i=0 ; i < wapi_pn_size; i++)
			RTMP_IO_READ8(pAd, offset+i, &tx_tsc[iveiv_tb_size + i]); 

		DBGPRINT(RT_DEBUG_TRACE, ("%s : WCID(%d) offset(%u)", __FUNCTION__, Wcid,offset));			
		hex_dump("TxTsc", tx_tsc, LEN_WAPI_TSC);
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n", 
									__FUNCTION__));
	}	
	
}


VOID WAPIInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	PMAC_TABLE_ENTRY	pEntry,
	BOOLEAN				bAE)
{	
	PCIPHER_KEY	pKey;

	pKey = &pEntry->PairwiseKey;
    NdisZeroMemory(pKey, sizeof(CIPHER_KEY));   

	/* Assign the pairwise cipher algorithm	*/
    if (pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled)
        pKey->CipherAlg = CIPHER_SMS4;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : fails (wcid-%d)\n", 
										__FUNCTION__, pEntry->wcid));	
		return;
	}	

	/* Prepare pair-wise key material */
   	pKey->KeyLen = LEN_TK;
    NdisMoveMemory(pKey->Key, &pEntry->PTK[0], 16);
	NdisMoveMemory(pKey->TxMic, &pEntry->PTK[16], 8);
	NdisMoveMemory(pKey->RxMic, &pEntry->PTK[24], 8);			

	/* Initial TSC for unicast */
	if (bAE)
		NdisMoveMemory(pKey->TxTsc, AE_UCAST_PN, LEN_WAPI_TSC);
	else
		NdisMoveMemory(pKey->TxTsc, ASUE_UCAST_PN, LEN_WAPI_TSC);
	NdisZeroMemory(pKey->RxTsc, LEN_WAPI_TSC);

	/* HW_WAPI is supported in RT3883 or later */
	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		UINT32	CONST_WAPI_PN = 0x5C365C36;
	
		/* Set unicast packet's PN to Asic. */
		if (bAE)
			AsicUpdateWCIDIVEIV(pAd, pEntry->wcid, CONST_WAPI_PN + 1, CONST_WAPI_PN);
		else
			AsicUpdateWCIDIVEIV(pAd, pEntry->wcid, CONST_WAPI_PN, CONST_WAPI_PN);				
		AsicUpdateWAPIPN(pAd, pEntry->wcid, CONST_WAPI_PN, CONST_WAPI_PN);

		/* Add Pair-wise key to Asic */
	    AsicAddPairwiseKeyEntry(
	        pAd, 
	        (UCHAR)pEntry->wcid, 
	        &pEntry->PairwiseKey);

		/* update WCID attribute table and IVEIV table for this entry */
		AsicUpdateWcidAttributeEntry(
			pAd, 
			pEntry->func_tb_idx, 
			pEntry->usk_id, 
			pEntry->PairwiseKey.CipherAlg,
			(UCHAR)pEntry->wcid,
			PAIRWISEKEYTABLE);        

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			CmdProcAddRemoveKey(pAd, 0, pEntry->func_tb_idx, 0, pEntry->wcid, PAIRWISEKEYTABLE, &pEntry->PairwiseKey, pEntry->Addr);
#endif /* MT_MAC */
	}
	
}


VOID WAPIInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	PUINT8				pGtk)
{
	UINT32			CONST_WAPI_PN = 0x5C365C36;
	PCIPHER_KEY 	pSharedKey;
	
	if (BssIdx >= MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The BSS-index(%d) is out of range for MBSSID link. \n", 
									__FUNCTION__, BssIdx));	
		return;
	}

	pSharedKey = &pAd->SharedKey[BssIdx][KeyIdx];
	NdisZeroMemory(pSharedKey, sizeof(CIPHER_KEY));
	
	if (GroupCipher == Ndis802_11EncryptionSMS4Enabled)
		pSharedKey->CipherAlg = CIPHER_SMS4;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : fails (IF/ra%d) \n", 
										__FUNCTION__, BssIdx));	
		return;
	}
			
	/* Assign key material into SW key table */
	pSharedKey->KeyLen = LEN_TK;
	NdisMoveMemory(pSharedKey->Key, pGtk, LEN_TK);
	NdisMoveMemory(pSharedKey->TxMic, pGtk + 16, LEN_TKIP_MIC);
	NdisMoveMemory(pSharedKey->RxMic, pGtk + 24, LEN_TKIP_MIC);            

	/* Initial TSC for B/Mcast */
	NdisMoveMemory(pSharedKey->TxTsc, AE_BCAST_PN, LEN_WAPI_TSC);
	NdisZeroMemory(pSharedKey->RxTsc, LEN_WAPI_TSC);		

	/* HW_WAPI is supported in RT3883 or later */
	if (IS_HW_WAPI_SUPPORT(pAd))
	{			
		/* Install Group Key to MAC ASIC */
	    AsicAddSharedKeyEntry(
					pAd, 
					BssIdx, 
					KeyIdx, 
					pSharedKey);

		/* When Wcid isn't zero, it means that this is a Authenticator Role. 
		   Only Authenticator entity needs to set HW IE/EIV table (0x6000),
		   WAPI PN table (0x7800)
		   and WCID attribute table (0x6800) for group key. */
		if (Wcid != 0)
		{
			/* Set 16-bytes PN to Asic. */
			AsicUpdateWCIDIVEIV(pAd, Wcid, CONST_WAPI_PN, CONST_WAPI_PN);
			AsicUpdateWAPIPN(pAd, Wcid, CONST_WAPI_PN, CONST_WAPI_PN);

			/* update Group key information to ASIC */
			AsicUpdateWcidAttributeEntry(
				pAd, 
				BssIdx, 
				KeyIdx, 
				pSharedKey->CipherAlg,
				Wcid,
				SHAREDKEYTABLE);        
		}

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			CmdProcAddRemoveKey(pAd, 0, BssIdx, KeyIdx, Wcid, SHAREDKEYTABLE, pSharedKey, BROADCAST_ADDR);
#endif
	}
}

#endif /* WAPI_SUPPORT */

