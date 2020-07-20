/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
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
	wsc_v2.c
*/

#include    "rt_config.h"

#ifdef WSC_NFC_SUPPORT

extern UINT8 WPS_DH_G_VALUE[1];
extern UINT8 WPS_DH_P_VALUE[192];

typedef struct _NFC_CMD_INFO {
	USHORT vendor_id;
	UCHAR action;
	UCHAR type;
	USHORT data_len;
	UCHAR data[0];
} NFC_CMD_INFO;

/*
	Packet Type: PACKET_OTHERHOST
	Packet Protocol: 0x6605
	
	0                   1                   2                   3   
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Vendor Id           |    Action     |     Type      |   
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Length              |         Data¡K¡K
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

	Vendor: 0x14C3
	Action: b¡¦<7:6>: 0x0 ¡V To NFC, 0x1 ¡V From NFC
	        b¡¦<5:0>: 0x00 ¡V Get, 0x01 - Set
	Type:	
		0 ¡V Command Result (1 byte)
		1 -  Configuration (WSC TLV)
		2 ¡V Password (32 bytes)
		3 ¡V IP address (4 bytes) 
		4 ¡V RTSP port (4 bytes)
		5 ¡V NFC Status (1 byte)
				b'0: 1 - ON, 0 - OFF
				b'1: 1 - nfc device detect
		6 ¡V Wireless Radio Status (1byte)
*/
#define ETH_MTK_NFC		0x6605
#define NFC_VENDOR_ID	0x14C3
VOID NfcCommand(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR Action,
	IN  UCHAR Type,
	IN  SHORT DataLen,
	IN  PUCHAR pData) 
{
	PNET_DEV pNetDev = pAd->net_dev;
	UCHAR *pBuffer = NULL;
	USHORT len;
	USHORT vendor_id = NFC_VENDOR_ID, type = ETH_MTK_NFC, net_type;
	UCHAR Header802_3[LENGTH_802_3];
	UCHAR SrcMacAddr[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x66, 0x05};
	struct wifi_dev *wdev;
	MULTISSID_STRUCT *pMbss;

	len = sizeof(vendor_id) + sizeof(Action) + sizeof(Type) + sizeof(USHORT) + LENGTH_802_3;
	if (DataLen > 0)
		len += DataLen;

	os_alloc_mem(pAd, (UCHAR **)&pBuffer, (len*sizeof(UCHAR)));
	if (pBuffer == NULL)
		return;

	pMbss = &pAd->ApCfg.MBSSID[0];
	wdev = &pMbss->wdev;
	NdisZeroMemory(pBuffer, len);
	NdisZeroMemory(&Header802_3[0], LENGTH_802_3);

	NdisMoveMemory(&Header802_3[0], wdev->bssid, MAC_ADDR_LEN);
    NdisMoveMemory((&Header802_3[0] + MAC_ADDR_LEN), &SrcMacAddr[0], MAC_ADDR_LEN);
	net_type = htons(type);
    NdisMoveMemory((&Header802_3[0] + MAC_ADDR_LEN * 2), &net_type, LENGTH_802_3_TYPE);
	//hex_dump("Header802_3", &Header802_3[0], LENGTH_802_3);

	NdisMoveMemory(pBuffer, &Header802_3[0], LENGTH_802_3);
	NdisMoveMemory(pBuffer+LENGTH_802_3, &vendor_id, sizeof(vendor_id));
	NdisMoveMemory(pBuffer+LENGTH_802_3+sizeof(vendor_id), &Action, sizeof(Action));
	NdisMoveMemory(pBuffer+LENGTH_802_3+sizeof(vendor_id)+sizeof(Action), &Type, sizeof(Type));
	NdisMoveMemory(pBuffer+LENGTH_802_3+sizeof(vendor_id)+sizeof(Action)+sizeof(Type), &DataLen, sizeof(USHORT));
	if ((DataLen > 0) && (pData != NULL))
		NdisMoveMemory(pBuffer+LENGTH_802_3+sizeof(vendor_id)+sizeof(Action)+sizeof(Type)+sizeof(USHORT), pData, DataLen);
	hex_dump("NfcCommand", pBuffer, len);
	RtmpOSNotifyRawData(pNetDev, pBuffer, len, 0, ETH_MTK_NFC);
	os_free_mem(NULL,pBuffer);
	return;
}


VOID	NfcGenRandomPasswd(
	IN  	PRTMP_ADAPTER	pAd,
	IN  	PWSC_CTRL       pWscCtrl)
{
	UCHAR   idx = 0;
	
	NdisZeroMemory(&pWscCtrl->NfcPasswd[0], 32);

	for (idx = 0; idx < NFC_DEV_PASSWD_LEN; idx++)
	{
		pWscCtrl->NfcPasswd[idx] = RandomByte(pAd);
	}
	pWscCtrl->NfcPasswdLen = NFC_DEV_PASSWD_LEN;
	return;
}

#define NFC_WSC_TLV_SIZE 512
INT NfcBuildWscProfileTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PWSC_CREDENTIAL     pCredential = NULL;
	UCHAR               apidx = pWscCtrl->EntryIfIdx;
	UCHAR				*TB = NULL;
	UCHAR				*pData = NULL, *pSrcData = NULL;
	INT					CerLen = 0;
	USHORT              AuthType = 0;
    USHORT              EncrType = 0;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscCtrl->RegData;
	INT					Len = 0, templen = 0;
	struct wifi_dev *wdev;
	MULTISSID_STRUCT *pMbss;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscCtrl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	

	os_alloc_mem(pAd, (UCHAR **)&pData, (NFC_WSC_TLV_SIZE*sizeof(UCHAR)));
	if (pData == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}
	
	os_alloc_mem(pAd, (UCHAR **)&TB, (256*sizeof(UCHAR)));
	if (TB == NULL)
	{
		os_free_mem(NULL, pData);
		return NDIS_STATUS_RESOURCES;
	}

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	pSrcData = pData;
	NdisZeroMemory(pData, NFC_WSC_TLV_SIZE);
	NdisZeroMemory(&TB[0], 256);
	WscCreateProfileFromCfg(pAd, REGISTRAR_ACTION | AP_MODE, pWscCtrl, &pWscCtrl->WscProfile);
	pCredential = &pAd->ApCfg.MBSSID[apidx].WscControl.WscProfile.Profile[0];
	// Credential
	CerLen += AppendWSCTLV(WSC_ID_NW_INDEX, &TB[0], (PUCHAR)"1", 0);

    AuthType = cpu2be16(pCredential->AuthType);
    EncrType = cpu2be16(pCredential->EncrType);
    CerLen += AppendWSCTLV(WSC_ID_SSID, &TB[CerLen], pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
	CerLen += AppendWSCTLV(WSC_ID_AUTH_TYPE, &TB[CerLen], (UINT8 *)&AuthType, 0);
	CerLen += AppendWSCTLV(WSC_ID_ENCR_TYPE, &TB[CerLen], (UINT8 *)&EncrType, 0);
	CerLen += AppendWSCTLV(WSC_ID_NW_KEY_INDEX, &TB[CerLen], &pCredential->KeyIndex, 0);
	CerLen += AppendWSCTLV(WSC_ID_NW_KEY, &TB[CerLen], pCredential->Key, pCredential->KeyLength);
	CerLen += AppendWSCTLV(WSC_ID_MAC_ADDR, &TB[CerLen], wdev->bssid, 0);

	//    Prepare plain text
	// Reguired attribute item in M8 if Enrollee is STA.
	templen = AppendWSCTLV(WSC_ID_CREDENTIAL, pData, TB, CerLen);
	pData += templen;
	Len   += templen;

	/* Optional items. RF_Band, AP_Channel, MAC_Address */
	UCHAR RF_Band;
	if (pAd->CommonCfg.Channel > 14)
		RF_Band = 0x02; /* 5.0GHz */
	else
		RF_Band = 0x01; /* 2.4GHz */
			
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &RF_Band, 0);
	pData += templen;
	Len   += templen;

	USHORT Channel = 0;
	Channel = pAd->CommonCfg.Channel;
#ifdef RT_BIG_ENDIAN
	Channel = SWAP16(Channel);
#endif /* RT_BIG_ENDIAN */
	templen = AppendWSCTLV(WSC_ID_AP_CHANNEL, pData, (UINT8 *)&Channel, 0);
	pData += templen;
	Len   += templen;
		
	templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, wdev->bssid, 0);
	pData += templen;
	Len   += templen;
	
#ifdef WSC_V2_SUPPORT
	if (pWscCtrl->WscV2Info.bEnableWpsV2)
	{
		/* Version2 */
		WscGenV2Msg(pWscCtrl, 
					FALSE, 
					NULL, 
					0, 
					&pData, 
					&Len);
		
		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen)
		{
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}
#endif // WSC_V2_SUPPORT //

	hex_dump("NfcBuildWscProfileTLV - pData", pSrcData, Len);
	if (pbuf && (Len < NFC_WSC_TLV_SIZE))
	{
		NdisMoveMemory(pbuf, pSrcData, Len);
		*pBufLen = (USHORT)Len;
		hex_dump("NfcBuildWscProfileTLV", pbuf, *pBufLen);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: (Len=%d)\n", __FUNCTION__, Len));
		Status = NDIS_STATUS_RESOURCES;
	}

	os_free_mem(NULL, pSrcData);
	os_free_mem(NULL, TB);
	return Status;
}

INT NfcBuildOOBDevPasswdTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	IN	UCHAR HandoverType,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen)
{
	INT Status = NDIS_STATUS_SUCCESS;
	UCHAR *TB = NULL;
	PUCHAR pData = NULL, pSrcData = NULL;
	USHORT PasswdID = 0, len;
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscCtrl->RegData;
	INT Len = 0, templen = 0;
	INT DH_Len = 0, idx;
	UCHAR HashData[SHA256_DIGEST_SIZE];
	INT nfc_dev_passwd_len=0;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscCtrl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	
	
	os_alloc_mem(pAd, (UCHAR **)&pData, (NFC_WSC_TLV_SIZE*sizeof(UCHAR)));
	if (pData == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}

	os_alloc_mem(pAd, (UCHAR **)&TB, (128*sizeof(UCHAR)));
	if (pData == NULL)
	{
		os_free_mem(NULL, pData);
		return NDIS_STATUS_RESOURCES;
	}
	
	DH_Len = sizeof(pWscCtrl->RegData.Pke);
	/*
		Enrollee 192 random bytes for DH key generation 
	*/
	for (idx = 0; idx < 192; idx++)
		pWscCtrl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

    NdisZeroMemory(pWscCtrl->RegData.Pke, sizeof(pWscCtrl->RegData.Pke));
	RT_DH_PublicKey_Generate (
        WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
	    WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
	    pWscCtrl->RegData.EnrolleeRandom, sizeof(pWscCtrl->RegData.EnrolleeRandom),
	    pWscCtrl->RegData.Pke, (UINT *) &DH_Len);

    /* Need to prefix zero padding */
    if((DH_Len != sizeof(pWscCtrl->RegData.Pke)) &&
        (DH_Len < sizeof(pWscCtrl->RegData.Pke)))
    {
        UCHAR TempKey[192];
        INT DiffCnt;
        DiffCnt = sizeof(pWscCtrl->RegData.Pke) - DH_Len;

        NdisFillMemory(&TempKey, DiffCnt, 0);
        NdisCopyMemory(&TempKey[DiffCnt], pWscCtrl->RegData.Pke, DH_Len);
        NdisCopyMemory(pWscCtrl->RegData.Pke, TempKey, sizeof(TempKey));
        DH_Len += DiffCnt;
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Do zero padding!\n", __func__));
    }

	/* For Handover case, We may as Registrar 
		So keep the same public key for Registrar */
	RTMPMoveMemory(pWscCtrl->RegData.Pkr, pWscCtrl->RegData.Pke, DH_Len);
	hex_dump("Pkr", pWscCtrl->RegData.Pkr, DH_Len);
	hex_dump("Pke", pWscCtrl->RegData.Pkr, DH_Len);


	RT_SHA256(&pWscCtrl->RegData.Pke[0], 192, &HashData[0]);
	RTMPMoveMemory(&pWscCtrl->NfcPasswdHash[0], &HashData[0], NFC_DEV_PASSWD_HASH_LEN);
	hex_dump("NfcBuildOOBDevPasswdTLV - Public Key HashData", &HashData[0], 20);

	if (HandoverType == TYPE_PASSWDHO_S || HandoverType == TYPE_PASSWDHO_R)
	{
		PasswdID = DEV_PASS_ID_NFC_HANDOVER;
		pWscCtrl->NfcPasswdID = cpu2be16(PasswdID);
		NdisZeroMemory(&pWscCtrl->NfcPasswd[0], 32);
		pWscCtrl->NfcPasswdLen = NFC_DEV_PASSWD_LEN;
	}
	else
	{
		PasswdID = (RandomByte(pAd) << 8) + RandomByte(pAd);
		if (PasswdID < 0x10)
			PasswdID = 0x10;
		pWscCtrl->NfcPasswdID = cpu2be16(PasswdID);

		NfcGenRandomPasswd(pAd, pWscCtrl);
		hex_dump("NfcBuildOOBDevPasswdTLV - NfcPasswd", &pWscCtrl->NfcPasswd[0], NFC_DEV_PASSWD_LEN);
	}

	pSrcData = pData;
	NdisZeroMemory(pData, NFC_WSC_TLV_SIZE);
	NdisZeroMemory(&TB[0], 128);

	if (HandoverType == TYPE_PASSWDHO_R || HandoverType == TYPE_PASSWDHO_S)
	{
		/* Reserv for "Length of WSC attribute" */
		pData += 2;		
	}	

	hex_dump("NfcBuildOOBDevPasswdTLV - 1 pSrcData", pSrcData, Len);

	NdisMoveMemory(&TB[0], &HashData[0], NFC_DEV_PASSWD_HASH_LEN);	
	NdisMoveMemory(&TB[20], &pWscCtrl->NfcPasswdID, sizeof(pWscCtrl->NfcPasswdID));
	if (HandoverType == TYPE_PASSWORD)
	{
		/* New SPEC Handover remove this part. */
		NdisMoveMemory(&TB[22], &pWscCtrl->NfcPasswd[0], NFC_DEV_PASSWD_LEN); 
		nfc_dev_passwd_len = NFC_DEV_PASSWD_LEN;
	}	

	templen = AppendWSCTLV(WSC_ID_OOB_DEV_PWD, pData, &TB[0], NFC_DEV_PASSWD_HASH_LEN+sizeof(PasswdID)+nfc_dev_passwd_len);
	pData += templen;
	Len   += templen;
	
	hex_dump("NfcBuildOOBDevPasswdTLV - 2 pSrcData", pSrcData, Len);

	if (HandoverType == TYPE_PASSWDHO_S) /* Build for Handover Select Message */
	{
		templen = AppendWSCTLV(WSC_ID_SSID, pData, pAd->ApCfg.MBSSID[0].Ssid, pAd->ApCfg.MBSSID[0].SsidLen);
		pData += templen;
		Len   += templen;
		/* Optional items. RF_Band, AP_Channel and MAC_Address */
		UCHAR RF_Band;
		if (pAd->CommonCfg.Channel > 14)
			RF_Band = 0x02; /* 5.0GHz */
		else
			RF_Band = 0x01; /* 2.4GHz */
		
		templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &RF_Band, 0);
		pData += templen;
		Len   += templen;

		USHORT Channel = 0;
		Channel = pAd->CommonCfg.Channel;
#ifdef RT_BIG_ENDIAN
		Channel = SWAP16(Channel);
#endif /* RT_BIG_ENDIAN */
		templen = AppendWSCTLV(WSC_ID_AP_CHANNEL, pData, (UINT8 *)&Channel, 0);
		pData += templen;
		Len   += templen;
		
		templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, pAd->CommonCfg.Bssid, 0);
		pData += templen;
		Len   += templen;
	
	}
	else if (HandoverType == TYPE_PASSWDHO_R) /* Build for Handover Request Message */
	{
		templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWscCtrl->Wsc_Uuid_E[0], 0);
		pData += templen;
		Len   += templen;
	}
	
#ifdef WSC_V2_SUPPORT
	if (pWscCtrl->WscV2Info.bEnableWpsV2)
	{
		/* Version2 */
		WscGenV2Msg(pWscCtrl, 
					FALSE, 
					NULL, 
					0, 
					&pData, 
					&Len);
		
		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen)
		{
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}
#endif // WSC_V2_SUPPORT //

	if (HandoverType == TYPE_PASSWDHO_R || HandoverType == TYPE_PASSWDHO_S)
	{
		/*Assign for "Length of WSC attribute" */
		len = cpu2be16(Len);
		memcpy(pSrcData, &len, 2);	
		hex_dump("NfcBuildOOBDevPasswdTLV - pSrcData", pSrcData, Len+2);
	}
	else
		hex_dump("NfcBuildOOBDevPasswdTLV - pSrcData", pSrcData, Len);
	if (pbuf && (Len < NFC_WSC_TLV_SIZE))
	{
		if (HandoverType == TYPE_PASSWDHO_R || HandoverType == TYPE_PASSWDHO_S)
		{
			NdisMoveMemory(pbuf, pSrcData, Len+2);
			*pBufLen = (USHORT)Len+2;
		}
		else
		{
			NdisMoveMemory(pbuf, pSrcData, Len);
			*pBufLen = (USHORT)Len;
		}	
		hex_dump("NfcBuildOOBDevPasswdTLV", pbuf, *pBufLen);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: (Len=%d)\n", __FUNCTION__, Len));
		Status = NDIS_STATUS_RESOURCES;
	}

	os_free_mem(NULL, pSrcData);
	os_free_mem(NULL, TB);
	return Status;
}

static BOOLEAN	NfcProcessPasswdTV(
	IN	PRTMP_ADAPTER		pAdapter, 
	IN	PUCHAR				pPlainData,
	IN	INT					PlainLength,
	IN  PWSC_CTRL           pWscCtrl,
	IN	UCHAR			bHandOver)
{
	USHORT			WscType, WscLen;
	PUCHAR			pData;
	INT DH_Len = 0, idx;
	
	pData  = pPlainData;

	hex_dump("NfcProcessPasswdTV - PlainData", pPlainData, PlainLength);
	// Start to process WSC IEs within credential
	if (bHandOver)
	{
		pData +=2; /* Skip length of WSC attribute */
		PlainLength -= 2;
	}	
	
	while (PlainLength > 4)
	{
		WSC_IE	TLV_Recv;
		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		PlainLength -= 4;

		// Parse M2 WSC type and store to RegData structure
		switch (WscType)
		{
			case WSC_ID_OOB_DEV_PWD:
				hex_dump("NfcProcessPasswdTV - WSC_ID_OOB_DEV_PWD", pData, WscLen);				
				NdisMoveMemory(&pWscCtrl->PeerNfcPasswdHash[0], pData, NFC_DEV_PASSWD_HASH_LEN);
				NdisMoveMemory(&pWscCtrl->PeerNfcPasswdID, pData+20, sizeof(pWscCtrl->PeerNfcPasswdID));
				pWscCtrl->PeerNfcPasswdLen = WscLen - 2 - NFC_DEV_PASSWD_HASH_LEN;
				if (pWscCtrl->PeerNfcPasswdLen > NFC_DEV_PASSWD_LEN)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s --> PeerNfcPasswdLen(%d) > NFC_DEV_PASSWD_LEN(%d)\n", 
						__FUNCTION__, pWscCtrl->PeerNfcPasswdLen, NFC_DEV_PASSWD_LEN));
					pWscCtrl->PeerNfcPasswdLen = NFC_DEV_PASSWD_LEN;
				}
				if (bHandOver== TRUE)
				{
					if (pWscCtrl->PeerNfcPasswdID != DEV_PASS_ID_NFC_HANDOVER)
						DBGPRINT(RT_DEBUG_TRACE, ("%s --> PeerNfcPasswdID(%04x) it should 0x0007 \n", 
							__FUNCTION__, pWscCtrl->PeerNfcPasswdID));
				}
				NdisMoveMemory(&pWscCtrl->PeerNfcPasswd[0], pData+20+sizeof(pWscCtrl->PeerNfcPasswdID), pWscCtrl->PeerNfcPasswdLen);
				hex_dump("PeerNfcPasswd", pWscCtrl->PeerNfcPasswd, pWscCtrl->PeerNfcPasswdLen);

				if (bHandOver==FALSE)
				{
					/* Due to M3 & M4 use the same WscGenPSK1 function,
					need copy to NfcPasswd, too */				
					NdisMoveMemory(&pWscCtrl->NfcPasswd[0], &pWscCtrl->PeerNfcPasswd[0], pWscCtrl->PeerNfcPasswdLen);
					pWscCtrl->NfcPasswdLen = pWscCtrl->PeerNfcPasswdLen;
				}
				
				break;
			case WSC_ID_UUID_E:
				NdisMoveMemory(pWscCtrl->RegData.PeerInfo.Uuid, pData, WscLen);
				break;
			case WSC_ID_SSID:
				hex_dump("NfcProcessPasswdTV - WSC_ID_SSID", pData, WscLen);
				break;
			case WSC_ID_VENDOR_EXT:
				hex_dump("NfcProcessPasswdTV - WSC_ID_VENDOR_EXT", pData, WscLen);
				break;
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("%s --> Unknown IE 0x%04x\n", __FUNCTION__, WscType));
				break;
		}

		// Offset to net WSC Ie
		pData  += WscLen;
		PlainLength -= WscLen;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s --> bHandOver=%d", __FUNCTION__, bHandOver));
	if (bHandOver== FALSE)
	{
		DH_Len = sizeof(pWscCtrl->RegData.Pkr);
		// Enrollee 192 random bytes for DH key generation
		for (idx = 0; idx < 192; idx++)
			pWscCtrl->RegData.EnrolleeRandom[idx] = RandomByte(pAdapter);

        NdisZeroMemory(pWscCtrl->RegData.Pkr, sizeof(pWscCtrl->RegData.Pkr));
		RT_DH_PublicKey_Generate (
			WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
			WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
			pWscCtrl->RegData.EnrolleeRandom, sizeof(pWscCtrl->RegData.EnrolleeRandom),
			pWscCtrl->RegData.Pkr, (UINT *) &DH_Len);

        /* Need to prefix zero padding */
        if((DH_Len != sizeof(pWscCtrl->RegData.Pkr)) &&
            (DH_Len < sizeof(pWscCtrl->RegData.Pkr)))
        {
            UCHAR TempKey[192];
            INT DiffCnt;
            DiffCnt = sizeof(pWscCtrl->RegData.Pkr) - DH_Len;

            NdisFillMemory(&TempKey, DiffCnt, 0);
            NdisCopyMemory(&TempKey[DiffCnt], pWscCtrl->RegData.Pkr, DH_Len);
            NdisCopyMemory(pWscCtrl->RegData.Pkr, TempKey, sizeof(TempKey));
            DH_Len += DiffCnt;
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Do zero padding!\n", __func__));
        }

		hex_dump("Pkr", pWscCtrl->RegData.Pkr, 192);
	}

	return TRUE;
}


VOID	NfcParseRspCommand(
	IN  PRTMP_ADAPTER	pAd,
	IN  PUCHAR pData,
	IN  USHORT DataLen)
{
	NFC_CMD_INFO *pNfcCmdInfo = NULL;
	BOOLEAN bSetFromNfc = FALSE;
	PWSC_CTRL pWscCtrl = &pAd->ApCfg.MBSSID[0].WscControl;
	
	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));
	hex_dump("Packet", pData, DataLen);

	os_alloc_mem(pAd, (UCHAR **)&pNfcCmdInfo, (DataLen*sizeof(UCHAR)));
	if (pNfcCmdInfo)
	{
		NdisMoveMemory(pNfcCmdInfo, pData, DataLen);		
		hex_dump("Packet", &pNfcCmdInfo->data[0], pNfcCmdInfo->data_len);
		bSetFromNfc = ((pNfcCmdInfo->action & 0x41) == 0x41);
		DBGPRINT(RT_DEBUG_TRACE, ("==> vendor_id: 0x%04x, action = 0x%0x, type = %d, data_len = %u, bSetFromNfc = %d\n", 
								pNfcCmdInfo->vendor_id, pNfcCmdInfo->action, pNfcCmdInfo->type, pNfcCmdInfo->data_len, bSetFromNfc));
		switch(pNfcCmdInfo->type)
		{
			case TYPE_CMD_RESULT:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_CMD_RESULT(=%d): Command result = %d\n", pNfcCmdInfo->type, pNfcCmdInfo->data[0]));
				break;
			case TYPE_CONFIGURATION:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_CONFIGURATION(=%d)\n", pNfcCmdInfo->type));
				if (bSetFromNfc)
				{
					if (pNfcCmdInfo->data_len != 1)
					{						
						/* 
							Receive Configuration from NFC daemon. 
						*/
						if (WscProcessCredential(pAd, &pNfcCmdInfo->data[0], pNfcCmdInfo->data_len, pWscCtrl) == FALSE)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("ProcessCredential fail..\n"));
						}
						else
						{							
							if ((pAd->OpMode == OPMODE_AP) && (pWscCtrl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED))
							{				
								pWscCtrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
								WscBuildBeaconIE(pAd, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, 0, NULL, 0, AP_MODE);
								WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, 0, NULL, 0, AP_MODE);
								APUpdateAllBeaconFrame(pAd);
								
							}
							if (pWscCtrl->WscUpdatePortCfgTimerRunning)
							{
								BOOLEAN bCancel;
								RTMPCancelTimer(&pWscCtrl->WscUpdatePortCfgTimer, &bCancel);
							}
							else
								pWscCtrl->WscUpdatePortCfgTimerRunning = TRUE;
							RTMPSetTimer(&pWscCtrl->WscUpdatePortCfgTimer, 1000);
						}
					}
				}
				else
				{
					Set_NfcConfigurationToken_Proc(pAd, "1");
				}
				break;
			case TYPE_PASSWORD:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_PASSWORD(=%d)\n", pNfcCmdInfo->type));
				if (bSetFromNfc)
				{
					if (pNfcCmdInfo->data_len != 1)
					{
						/* 
							Receive Passwd from NFC daemon. 
						*/
						NfcProcessPasswdTV(pAd, &pNfcCmdInfo->data[0], pNfcCmdInfo->data_len, pWscCtrl, FALSE);
						WscGetConfWithoutTrigger(pAd, pWscCtrl, FALSE);
						pWscCtrl->bTriggerByNFC = TRUE;
						pWscCtrl->NfcModel = MODEL_PASSWORD_TOKEN;
					}
				}
				else
				{
					Set_NfcPasswdToken_Proc(pAd, "1");
				}
				break;
	 		/* New type for Handover */				
			case TYPE_PASSWDHO_S:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_PASSWDHO_S(=%d)\n", pNfcCmdInfo->type));
				if (bSetFromNfc)
				{
					if (pNfcCmdInfo->data_len != 1)
					{
						/* 
							Receive Passwd from NFC daemon. "So far" no this case.
							Due to AP always as Registrar in handover procedure, 
							AP only receive "Handover Request Message".
						*/
						NfcProcessPasswdTV(pAd, &pNfcCmdInfo->data[0], pNfcCmdInfo->data_len, pWscCtrl, TRUE);
						WscGetConfWithoutTrigger(pAd, pWscCtrl, FALSE);
						pWscCtrl->bTriggerByNFC = TRUE;
						pWscCtrl->NfcModel = MODEL_HANDOVER; /* 2 */
					}
				}
				else
				{
					Set_NfcPasswdToken_Proc(pAd, "2");
				}
				break;	
			case TYPE_PASSWDHO_R:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_PASSWDHO_R(=%d)\n", pNfcCmdInfo->type));
				if (bSetFromNfc)
				{
					if (pNfcCmdInfo->data_len != 1)
					{
						/* 
							Receive Passwd from NFC daemon. 
						*/
						NfcProcessPasswdTV(pAd, &pNfcCmdInfo->data[0], pNfcCmdInfo->data_len, pWscCtrl, TRUE);
						WscGetConfWithoutTrigger(pAd, pWscCtrl, FALSE);
						pWscCtrl->bTriggerByNFC = TRUE;
						pWscCtrl->NfcModel = MODEL_HANDOVER; /* 2 */
					}
				}
				else
				{
					/*
						"So far" no this case.
						Due to AP always as Registrar in handover procedure, 
						AP only send  "Handover Select Message".
					*/
					Set_NfcPasswdToken_Proc(pAd, "3");
				}
				break;
			case TYPE_NFC_STATUS:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_NFC_STATUS(=%d): NFC Status = %d\n", pNfcCmdInfo->type, pNfcCmdInfo->data[0]));
				pWscCtrl->NfcStatus = pNfcCmdInfo->data[0];
				break;
			case TYPE_WIFI_RADIO_STATUS:
				DBGPRINT(RT_DEBUG_TRACE, ("TYPE_WIFI_RADIO_STATUS(=%d)\n", pNfcCmdInfo->type));
				if (bSetFromNfc)
				{
					if (pNfcCmdInfo->data[0] == 1)
						MlmeRadioOn(pAd);
					else
						MlmeRadioOff(pAd);
				}
				else
				{
					UCHAR RadioStatus = 0;
					if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
						RadioStatus = 0;
					else
						RadioStatus = 1;
					NfcCommand(pAd, 0x01, TYPE_WIFI_RADIO_STATUS, 1, &RadioStatus);
				}
				break;
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("Unknow type(=%d)\n", pNfcCmdInfo->type));
				break;
		}
		os_free_mem(NULL, pNfcCmdInfo);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));
	return;
}

INT Set_NfcStatus_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN PSTRING			arg)
{
	UCHAR data = 0;

	/*
		Action: b¡¦<7:6>: 0x00 ¡V Request, 0x01 ¡V Notify
        		b¡¦<5:0>: 0x00 ¡V Get, 0x01 - Set
	*/
	UCHAR action = 1, type = TYPE_NFC_STATUS; /* 5 - NFC Status */

	if (simple_strtol(arg, 0, 10) != 0)
		data = 1;

	data = (UCHAR)simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: Set NFC Status(=%d)\n", __FUNCTION__, data));
	NfcCommand(pAd, action, type, 1, &data);
	
	return TRUE;
}

INT Set_NfcPasswdToken_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN PSTRING			arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	PWSC_CTRL	pWscCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl;
	PUCHAR pBuf = NULL;
	USHORT BufLen = 0;
	UCHAR action = 1, type = TYPE_PASSWORD;
	UCHAR val=0;
	BOOLEAN		bHandover=FALSE;
	/*
		Action: b¡¦<7:6>: 0x00 ¡V Request, 0x01 ¡V Notify
        		b¡¦<5:0>: 0x00 ¡V Get, 0x01 - Set
	*/
	val = (UCHAR)simple_strtol(arg, 0, 10);
	if (val == 1) /* Password Token */
	{
		bHandover = FALSE;
		type = TYPE_PASSWORD;
	}
	else if (val == 2) /* Handover Select */
	{
		bHandover = TRUE;
		type = TYPE_PASSWDHO_S;
	}	
	else if (val == 3) /* Handover Request */
	{
		bHandover = TRUE;
		type = TYPE_PASSWDHO_R;
	}	

	os_alloc_mem(pAd, (UCHAR **)&pBuf, (NFC_WSC_TLV_SIZE*sizeof(UCHAR)));
	if (pBuf == NULL)
	{
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: NfcBuildOOBDevPasswdTLV value(=%d) type(=%d)\n", __FUNCTION__,val,  type));
	NfcBuildOOBDevPasswdTLV(pAd, pWscCtrl, type, pBuf, &BufLen);
	if (pBuf && (BufLen != 0))
	{
		pWscCtrl->bTriggerByNFC = TRUE;
		if (type ==TYPE_PASSWORD)
			pWscCtrl->NfcModel = MODEL_PASSWORD_TOKEN;
		else
			pWscCtrl->NfcModel = MODEL_HANDOVER;
		
		DBGPRINT(RT_DEBUG_TRACE, ("%s: NfcBuildOOBDevPasswdTLV NfcModel(=%d)\n", __FUNCTION__,pWscCtrl->NfcModel));
		NfcCommand(pAd, action, type, BufLen, pBuf);
		os_free_mem(NULL, pBuf);
	}
	
	return TRUE;
}


INT Set_NfcConfigurationToken_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN PSTRING			arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	PWSC_CTRL	pWscCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl;
	PUCHAR pBuf = NULL;
	USHORT BufLen = 0;
	/*
		Action: b¡¦<7:6>: 0x00 ¡V Request, 0x01 ¡V Notify
        		b¡¦<5:0>: 0x00 ¡V Get, 0x01 - Set
	*/
	UCHAR action = 1, type = TYPE_CONFIGURATION;

	os_alloc_mem(pAd, (UCHAR **)&pBuf, (NFC_WSC_TLV_SIZE*sizeof(UCHAR)));
	if (pBuf == NULL)
	{
		return FALSE;
	}

	NfcBuildWscProfileTLV(pAd, pWscCtrl, pBuf, &BufLen);
	if (pBuf && (BufLen != 0))
	{
		NfcCommand(pAd, action, type, BufLen, pBuf);
		os_free_mem(NULL, pBuf);
	}

	if ((pAd->OpMode == OPMODE_AP) && (pWscCtrl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED))
	{				
		pWscCtrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
		WscBuildBeaconIE(pAd, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, 0, NULL, 0,AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, 0, NULL, 0,AP_MODE);
		APUpdateAllBeaconFrame(pAd);
		
	}
	if (pWscCtrl->WscUpdatePortCfgTimerRunning)
	{
		BOOLEAN bCancel;
		RTMPCancelTimer(&pWscCtrl->WscUpdatePortCfgTimer, &bCancel);
	}
	else
		pWscCtrl->WscUpdatePortCfgTimerRunning = TRUE;
	RTMPSetTimer(&pWscCtrl->WscUpdatePortCfgTimer, 1000);
	return TRUE;
}

INT Get_NfcStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
 	UCHAR data = 0;
	/*
		Action: b¡¦<7:6>: 0x0 ¡V To NFC, 0x1 ¡V From NFC
        		b¡¦<5:0>: 0x0 ¡V Get, 0x01 - Set
	*/
	UCHAR action = 0, type = TYPE_NFC_STATUS; /* 5 - NFC Status */
	
	NfcCommand(pAd, action, type, 0, NULL);
	
    return TRUE;
}

/*
	NfcPasswdCompute:
	1 - use hex device passwd to calculate PSK1 & PSK2.
	2 - the device password is expressed in hexadecimal using ASCII characters (two characters per octet, uppercase letters only) to caculate PSK1 & PSK2.
*/
INT	Set_DoWpsByNFC_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING 			arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
    BOOLEAN     bFromApCli = FALSE;
    PWSC_CTRL   pWscCtrl;
	UINT32		val = 0;

	pWscCtrl = &pAd->ApCfg.MBSSID[apidx].WscControl;

	val = simple_strtol(arg, 0, 10);
	if (val == 0)
		pWscCtrl->bTriggerByNFC = FALSE;
	else
	{
		pWscCtrl->bTriggerByNFC = TRUE;
		pWscCtrl->NfcPasswdCaculate = val;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s : bTriggerByNFC=%d, NfcPasswdCaculate = %d\n", 
		__FUNCTION__, pWscCtrl->bTriggerByNFC, pWscCtrl->NfcPasswdCaculate));

	return TRUE;
}

INT Set_NfcRegenPK_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING 			arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	BOOLEAN     bFromApCli = FALSE;
	PWSC_CTRL   pWscCtrl;
	UINT32		val = 0;

	pWscCtrl = &pAd->ApCfg.MBSSID[apidx].WscControl;

	val = simple_strtol(arg, 0, 10);
	if (val == 1)
		pWscCtrl->bRegenPublicKey = 1;
	else
		pWscCtrl->bRegenPublicKey = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s : bRegenPublicKey=%d, \n", 
	__FUNCTION__, pWscCtrl->bRegenPublicKey));
}	
#endif /* WSC_NFC_SUPPORT */

