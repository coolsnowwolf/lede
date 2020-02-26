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
	wsc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Paul Lin	06-08-08		Initial
	JuemingChen 06-10-30        Do modifications and Add APIs for AP
*/

#include "rt_config.h"

#ifdef WSC_INCLUDED
#include "wsc_tlv.h"

static UCHAR	Wsc_Personal_String[] =  "Wi-Fi Easy and Secure Key Derivation";

#define IV_ENCR_DATA_LEN_512 512
#define IV_ENCR_DATA_LEN_144 144

#define WSC_TLV_ENT(TLV_PARA) (0x00ff & TLV_PARA)
#define WSC_TLV_BYTE1(TLV_PARA) (0x000f & TLV_PARA)
#define WSC_TLV_BYTE2(TLV_PARA) (0x000f & (TLV_PARA >> 4))

#define PLAIN_LEN_TOTAL_SIZE 512
/* Global reusable buffer */
static WSC_TLV_0B wsc_tlv_0b[] = {
	{/*0,*/0}, /*AP Channel*/	{/*0x1001,*/ 2}, /* WSC_ID_AP_CHANNEL */
	/*Association State*/		{/*0x1002,*/ 2}, /* WSC_ID_ASSOC_STATE */
	/*Authentication Type*/		{/*0x1003,*/ 2}, /* WSC_ID_AUTH_TYPE */
	/*Authentication Type Flags*/	{/*0x1004,*/ 2}, /* WSC_ID_AUTH_TYPE_FLAGS */
	/*Authenticator*/		{/*0x1005,*/ 8}, /* WSC_ID_AUTHENTICATOR */
	{/*0,*/0},
	{/*0,*/0},
	/*Config Methods*/		{/*0x1008,*/ 2}, /* WSC_ID_CONFIG_METHODS */
	/*Configuration Error*/		{/*0x1009,*/ 2}, /* WSC_ID_CONFIG_ERROR */
	/*Confirmation URL4*/		{/*0x100A,*/ 0x40}, /* <= 64B	WSC_ID_CONF_URL4 */
	/*Confirmation URL6*/		{/*0x100B,*/ 0x4C}, /* <= 76B	WSC_ID_CONF_URL6 */
	/*Connection Type*/		{/*0x100C,*/ 1}, /* WSC_ID_CONN_TYPE */
	/*Connection Type Flags*/	{/*0x100D,*/ 1}, /* WSC_ID_CONN_TYPE_FLAGS */
	/*Credential*/			{/*0x100E,*/ 0xff}, /* WSC_ID_CREDENTIAL */
	/*Encryption Type*/		{/*0x100F,*/ 2}, /* WSC_ID_ENCR_TYPE */
	/*Encryption Type Flags*/	{/*0x1010,*/ 2}, /* WSC_ID_ENCR_TYPE_FLAGS */
	/*Device Name*/			{/*0x1011,*/ 0x20}, /* <= 32B	WSC_ID_DEVICE_NAME */
	/*Device Password ID*/		{/*0x1012,*/ 2}, /* WSC_ID_DEVICE_PWD_ID */
	{/*0,*/0},
	/*E-Hash1*/			{/*0x1014,*/ 32}, /* WSC_ID_E_HASH1 */
	/*E-Hash2*/			{/*0x1015,*/ 32}, /* WSC_ID_E_HASH2 */
	/*E-SNonce1*/			{/*0x1016,*/ 16}, /* WSC_ID_E_SNONCE1 */
	/*E-SNonce2*/			{/*0x1017,*/ 16}, /* WSC_ID_E_SNONCE2 */
	/*Encrypted Settings*/		{/*0x1018,*/ 0xff}, /* WSC_ID_ENCR_SETTINGS */
	{/*0,*/0},
	/*Enrollee Nonce*/		{/*0x101A,*/ 16}, /* WSC_ID_ENROLLEE_NONCE */
	/*Feature_ID*/			{/*0x101B,*/ 4}, /* WSC_ID_FEATURE_ID */
	/*Identity*/			{/*0x101C,*/ 0x50}, /* <= 80B	WSC_ID_IDENTITY */
	{/*0,*/0}, /* WSC_ID_IDENTITY_PROOF */
	/*Key Wrap Authenticator*/	{/*0x101E,*/ 8}, /* WSC_ID_KEY_WRAP_AUTH */
	/*Key Identifier*/		{/*0x101F,*/ 16}, /* WSC_ID_KEY_IDENTIFIER */
	/*MAC Address*/			{/*0x1020,*/ 6}, /* WSC_ID_MAC_ADDR */
	/*Manufacturer*/		{/*0x1021,*/ 0x40}, /* <= 64B	WSC_ID_MANUFACTURER */
	/*Message Type*/		{/*0x1022,*/ 1}, /* WSC_ID_MSG_TYPE */
	/*Model Name*/			{/*0x1023,*/ 0x20}, /* <= 32B	WSC_ID_MODEL_NAME */
	/*Model Number*/		{/*0x1024,*/ 0x20}, /* <= 32B	WSC_ID_MODEL_NUMBER */
	{/*0,*/0},
	/*Network Index*/		{/*0x1026,*/ 1}, /* WSC_ID_NW_INDEX */

	/*
	*	Windows 7 WCN test only accept Len of Network Key item in credentail is zero
	*	when auth type of AP is OPEN/NONE
	*/
	/*Network Key*/			{/*0x1027,*/ 0}, /* <= 64B	WSC_ID_NW_KEY */
	/*Network Key Index*/		{/*0x1028,*/ 1}, /* WSC_ID_NW_KEY_INDEX */
	/*New Device Name*/		{/*0x1029,*/ 0x20}, /* <= 32B	WSC_ID_NEW_DEVICE_NAME */
	/*New Password*/		{/*0x102A,*/ 0x40}, /* <= 64B	WSC_ID_NEW_PWD */
	{/*0,*/0},
	/*OOB Device Password*/		{/*0x102C,*/ 0x3A}, /* <= 58B	WSC_ID_OOB_DEV_PWD */
	/*OS Version*/			{/*0x102D,*/ 4}, /* WSC_ID_OS_VERSION */
	{/*0,*/0},
	/*Power Level*/			{/*0x102F,*/ 1}, /* WSC_ID_POWER_LEVEL */
	/*PSK Current*/			{/*0x1030,*/ 1}, /* WSC_ID_PSK_CURRENT */
	/*PSK Max*/			{/*0x1031,*/ 1}, /* WSC_ID_PSK_MAX */
	/*Public Key*/			{/*0x1032,*/ 192}, /* WSC_ID_PUBLIC_KEY */
	/*Radio Enabled*/		{/*0x1033,*/ 1}, /* WSC_ID_RADIO_ENABLED */
	/*Reboot*/			{/*0x1034,*/ 1}, /* WSC_ID_REBOOT */
	/*Registrar Current*/		{/*0x1035,*/ 1}, /* WSC_ID_REGISTRAR_CURRENT */
	/*Registrar Established*/	{/*0x1036,*/ 1}, /* WSC_ID_REGISTRAR_ESTBLSHD */
	{/*0,*/0}, /* WSC_ID_REGISTRAR_LIST */
	/*Registrar Max*/		{/*0x1038,*/ 1}, /* WSC_ID_REGISTRAR_MAX */
	/*Registrar Nonce*/		{/*0x1039,*/ 16}, /* WSC_ID_REGISTRAR_NONCE */
	/*Request Type*/		{/*0x103A,*/ 1}, /* WSC_ID_REQ_TYPE */
	/*Response Type*/		{/*0x103B,*/ 1}, /* WSC_ID_RESP_TYPE */
	/*RF Bands*/			{/*0x103C,*/ 1}, /* WSC_ID_RF_BAND */
	/*R-Hash1*/			{/*0x103D,*/ 32}, /* WSC_ID_R_HASH1 */
	/*R-Hash2*/			{/*0x103E,*/ 32}, /* WSC_ID_R_HASH2 */
	/*R-SNonce1*/			{/*0x103F,*/ 16}, /* WSC_ID_R_SNONCE1 */
	/*R-SNonce2*/			{/*0x1040,*/ 16}, /* WSC_ID_R_SNONCE2 */
	/*Selected Registrar*/		{/*0x1041,*/ 1}, /* WSC_ID_SEL_REGISTRAR */
	/*Serial Number*/		{/*0x1042,*/ 0x20}, /* <= 32B	WSC_ID_SERIAL_NUM */
	{/*0,*/0},
	/*Simple Config State*/		{/*0x1044,*/ 1}, /* WSC_ID_SC_STATE */
	/*SSID*/			{/*0x1045,*/ 0x20}, /* <= 32B	WSC_ID_SSID */
	/*Total Networks*/		{/*0x1046,*/ 1}, /* WSC_ID_TOT_NETWORKS */
	/*UUID-E*/			{/*0x1047,*/ 16}, /* WSC_ID_UUID_E */
	/*UUID-R*/			{/*0x1048,*/ 16}, /* WSC_ID_UUID_R */
	/*WPS Vendor Extension*/	{/*0x1049,*/ 0x400}, /* WSC_ID_VENDOR_EXT */
	/*Version*/			{/*0x104A,*/ 1}, /* WSC_ID_VERSION */
	/*X.509 Certificate Request*/	{/*0x104B,*/ 0xff}, /* WSC_ID_X509_CERT_REQ */
	/*X.509 Certificate*/		{/*0x104C,*/ 0xff}, /* WSC_ID_X509_CERT */
	/*EAP Identity*/		{/*0x104D,*/ 0x40}, /* <= 64B	WSC_ID_EAP_IDENTITY */
	/*Message Counter*/		{/*0x104E,*/ 8}, /* WSC_ID_MSG_COUNTER */
	/*Public Key Hash*/		{/*0x104F,*/ 20}, /* WSC_ID_PUBKEY_HASH */
	/*Rekey Key*/			{/*0x1050,*/ 32}, /* WSC_ID_REKEY_KEY */
	/*Key Lifetime*/		{/*0x1051,*/ 4}, /* WSC_ID_KEY_LIFETIME */
	/*Permitted Config Methods*/	{/*0x1052,*/ 2}, /* WSC_ID_PERM_CFG_METHODS */
	/*Selected Registrar Config Method*/{/*0x1053,*/ 2}, /* WSC_ID_SEL_REG_CFG_METHODS */
	/*Primary Device Type*/		{/*0x1054,*/ 8}, /* WSC_ID_PRIM_DEV_TYPE */
	{/*0,*/0}, /* WSC_ID_SEC_DEV_TYPE_LIST */
	/*Portable Device*/		{/*0x1056,*/ 1}, /* WSC_ID_PORTABLE_DEVICE */
	/*AP Setup Locked*/		{/*0x1057,*/ 1}, /* WSC_ID_AP_SETUP_LOCKED */
	{/*0,*/0}, /* WSC_ID_APP_LIST */
	/*EAP Type*/			{/*0x1059,*/ 0x08}, /* <= 8B	WSC_ID_EAP_TYPE */
	{/*0,*/0},
	{/*0,*/0},
	{/*0,*/0},
	{/*0,*/0},
	{/*0,*/0},
	{/*0,*/0},
	/*Initialization Vector*/	{/*0x1060,*/ 32}, /* WSC_ID_INIT_VECTOR */
	/*Key Provided Automatically*/	{/*0x1061,*/ 1}, /* WSC_ID_KEY_PROVIDED_AUTO */
	/*802.1X Enabled*/		{/*0x1062,*/ 1}, /* WSC_ID_8021X_ENABLED */
	/*<Reserved for WFA> 0x106F V 0x1FFF*/
	/*<Unavailable> 0x000 V 0x0FFF,0x2000 V 0xFFFF*/
};

int AppendWSCTLV(USHORT index, OUT UCHAR *obuf,
				 IN UCHAR *ibuf, IN USHORT varlen)
{
	USHORT len = 0, dataLen = 0, tag = cpu2be16(index);

	/*
	*	The max len of WPS Vendor Extension is 1024B
	*/
	if (varlen != (USHORT)0)
		dataLen = varlen;
	else {
		if (WSC_TLV_ENT(index) <= ((sizeof(wsc_tlv_0b) / sizeof(WSC_TLV_0B)) - 1))
			dataLen = wsc_tlv_0b[WSC_TLV_ENT(index)].len;
	}

	NdisMoveMemory(obuf, &tag, 2);
	len = cpu2be16(dataLen);
	memcpy(obuf + 2, &len, 2);

	if (dataLen != 0)
		memcpy(obuf + 4, ibuf, dataLen);

	return dataLen + 4;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process elements encryption settings
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		1. Recv M4 M5 M6 M7
*
*	========================================================================
*/
static VOID	WscParseEncrSettings(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PUCHAR pPlainData,
	IN  INT PlainLength,
	IN  PWSC_CTRL pWscControl)
{
	USHORT	WscType, WscLen, HmacLen;
	PUCHAR	pData;
	UCHAR	Hmac[8], Temp[32];
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;

	NdisZeroMemory(&Hmac, sizeof(Hmac));
	NdisZeroMemory(&Temp, sizeof(Temp));
	HmacLen = (USHORT)(PlainLength - 12);
	pData  = pPlainData;

	/* Start to process WSC IEs */
	while (PlainLength > 4) {
		WSC_IE	TLV_Encr;

		memcpy((UINT8 *)&TLV_Encr, pData, 4);
		WscType = be2cpu16(TLV_Encr.Type);
		WscLen  = be2cpu16(TLV_Encr.Length);
		pData  += 4;
		PlainLength -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_E_SNONCE1:
			/* for verification with our enrollee nonce */
			NdisMoveMemory(pReg->Es1, pData, WscLen);
			break;

		case WSC_ID_E_SNONCE2:
			/* for verification with our enrollee nonce */
			NdisMoveMemory(pReg->Es2, pData, WscLen);
			break;

		case WSC_ID_R_SNONCE1:
			/* for verification with our enrollee nonce */
			NdisMoveMemory(pReg->Rs1, pData, WscLen);
			break;

		case WSC_ID_R_SNONCE2:
			/* for verification with our enrollee nonce */
			NdisMoveMemory(pReg->Rs2, pData, WscLen);
			break;

		case WSC_ID_KEY_WRAP_AUTH:
			NdisMoveMemory(Hmac, pData, WscLen);
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscParseEncrSettings --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		PlainLength -= WscLen;
	}

	/* Validate HMAC, reuse KDK buffer */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pPlainData, HmacLen, Temp, SHA256_DIGEST_SIZE);

	if (RTMPEqualMemory(Hmac, Temp, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("WscParseEncrSettings --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				 ("MD --> 0x%08x-%08x\n",
				  (UINT)cpu2be32(*((PUINT)&Temp[0])), (UINT)cpu2be32(*((PUINT)&Temp[4]))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				 ("calculated --> 0x%08x-%08x\n",
				  (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
	}
}

/*
*	========================================================================
*
*	Routine Description:
*		Process credentials within AP encryption settings
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		1. Recv M8
*
*	========================================================================
*/
BOOLEAN	WscProcessCredential(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PUCHAR pPlainData,
	IN  INT PlainLength,
	IN  PWSC_CTRL pWscControl)
{
	USHORT WscType, WscLen, Cnt = 0, CurrentIdx = 0, Idx, tmpVal = 0;
	PUCHAR pData, pTmp;
	PWSC_PROFILE pProfile;
#ifdef WSC_V2_SUPPORT
	BOOLEAN bReject = FALSE;
#endif /* WSC_V2_SUPPORT */
	pData  = pPlainData;
	/* Cleanup Old contents */
	NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
	pProfile = (PWSC_PROFILE) &pWscControl->WscProfile;
	/*CurrentIdx = pWscControl->EntryIfIdx; */
	/* Init Profile number */
	Cnt = 0;
	hex_dump("WscProcessCredential - PlainData", pPlainData, PlainLength);

	/* Start to process WSC IEs within credential */
	while (PlainLength > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		PlainLength -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_NW_INDEX:
			/* A new profile, add the cnt and save to database */
			CurrentIdx = Cnt;	/* since the index start from 0, we have to minus 1 */
			Cnt++;
			break;

		case WSC_ID_SSID:
			/* Find the exact length of SSID without null terminator */
			pTmp = pData;

			for (Idx = 0; Idx < WscLen; Idx++) {
				if (*(pTmp++) == 0x0)
					break;
			}

			pProfile->Profile[CurrentIdx].SSID.SsidLength = Idx;

			if (RTMPCheckStrPrintAble((CHAR *)pData, Idx) || (pWscControl->bCheckMultiByte == FALSE))
				NdisMoveMemory(pProfile->Profile[CurrentIdx].SSID.Ssid, pData, pProfile->Profile[CurrentIdx].SSID.SsidLength);
			else
				return FALSE;

			break;

		case WSC_ID_AUTH_TYPE:
			tmpVal = get_unaligned((PUSHORT) pData);
			pProfile->Profile[CurrentIdx].AuthType = cpu2be16(tmpVal);
			break;

		case WSC_ID_ENCR_TYPE:
			tmpVal = get_unaligned((PUSHORT) pData);
			pProfile->Profile[CurrentIdx].EncrType = cpu2be16(tmpVal);
			break;

		case WSC_ID_NW_KEY_INDEX:
			/* Netork Key Index: 1 ~ 4 */
			pProfile->Profile[CurrentIdx].KeyIndex = (*pData);
			break;

		case WSC_ID_NW_KEY:
			if (WscLen == 0)
				break;

			if (RTMPCheckStrPrintAble((CHAR *)pData, WscLen) || (pWscControl->bCheckMultiByte == FALSE)) {
				pProfile->Profile[CurrentIdx].KeyLength = WscLen;
				NdisMoveMemory(pProfile->Profile[CurrentIdx].Key, pData, pProfile->Profile[CurrentIdx].KeyLength);
			} else
				return FALSE;

			break;

		case WSC_ID_MAC_ADDR:
			/*
			*	Some AP (ex. Buffalo WHR-G300N WPS AP) would change BSSID during WPS processing.
			*	STA shall not change MacAddr of credential form AP.
			*/
			RTMPMoveMemory(pProfile->Profile[CurrentIdx].MacAddr, pData, MAC_ADDR_LEN);
			break;

		case WSC_ID_KEY_WRAP_AUTH:
			/* Not used here, since we already verify it at decryption */
			break;

		case WSC_ID_CREDENTIAL:
			/* Credential IE, The WscLen include all length within profile, we need to modify it */
			/* to be able to parse all profile fields */
			WscLen = 0;
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					 ("WscProcessCredential --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		PlainLength -= WscLen;
	}

	/* Svae the total number */
	pProfile->ProfileCnt = (UINT)Cnt;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/*
		*	Check all credentials
		*/
		for (Idx = 0; Idx < pProfile->ProfileCnt; Idx++) {
			PWSC_CREDENTIAL pCredential = &pProfile->Profile[Idx];

			if (pCredential->EncrType == WSC_ENCRTYPE_WEP) {
				bReject = TRUE;
				/* Cleanup contents */
				NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
			}
		}

		if (bReject)
			return FALSE;
	}

#endif /* WSC_V2_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			 ("WscProcessCredential --> %d profile retrieved from credential\n", Cnt));
	return TRUE;
}

/* return 0  to success ,1 to failed */
int WscDeriveKey(
	unsigned char *kdk, unsigned int kdk_len,
	unsigned char *prsnlString, unsigned int str_len,
	unsigned char *key, unsigned int keyBits)
{
	unsigned int i = 0, iterations = 0;
	unsigned char input[64], output[128];
	unsigned char hmac[32];
	unsigned int temp;

	NdisZeroMemory(&input, sizeof(input));
	NdisZeroMemory(&output, sizeof(output));
	NdisZeroMemory(&hmac, sizeof(hmac));
	iterations = ((keyBits / 8) + 32 - 1) / 32;
	/*Prepare the input buffer. During the iterations, we need only replace the */
	/*value of i at the start of the buffer. */
	temp = cpu2be32(i);
	memcpy(input, &temp, 4);
	memcpy(input + 4, prsnlString, str_len);
	temp = cpu2be32(keyBits);
	memcpy(input + 4 + str_len, &temp, 4);

	for (i = 0; i < iterations; i++) {
		/*Set the current value of i at the start of the input buffer */
		temp = cpu2be32(i + 1); /*i should start at 1 */
		memcpy(input, &temp, 4);
		RT_HMAC_SHA256(kdk, kdk_len, input, 4 + str_len + 4, hmac, SHA256_DIGEST_SIZE);
		memcpy(output + i * 32, hmac, 32);
	}

	/*Sanity check */
	if (keyBits / 8 > (32 * iterations)) {
		return 1; /*failed */
	}

	memcpy(key, output, 80);
	return 0; /*success */
}

VOID WscGenPSK1(
	PRTMP_ADAPTER pAd,
	PWSC_CTRL pWscCtrl,
	UCHAR *pPSK1)
{
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscCtrl->RegData;
	PUCHAR pTempPsk = NULL;
	os_alloc_mem(NULL, &pTempPsk, SHA256_DIGEST_SIZE);

	if (pTempPsk == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s - pTempPsk alloc failed.", __func__));
		return;
	}

	{
		/* Generate PSK1 */
		if (pReg->PinCodeLen == 4)
			RT_HMAC_SHA256(pReg->AuthKey, 32, &pReg->PIN[0], 2, pTempPsk, SHA256_DIGEST_SIZE);
		else
			RT_HMAC_SHA256(pReg->AuthKey, 32, &pReg->PIN[0], 4, pTempPsk, SHA256_DIGEST_SIZE);
	}

	NdisMoveMemory(pPSK1, pTempPsk, SHA256_DIGEST_SIZE);
	os_free_mem(pTempPsk);
}

VOID WscGenPSK2(
	PRTMP_ADAPTER pAd,
	PWSC_CTRL pWscCtrl,
	UCHAR *pPSK2)
{
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscCtrl->RegData;
	PUCHAR pTempPsk = NULL;
	os_alloc_mem(NULL, &pTempPsk, SHA256_DIGEST_SIZE);

	if (pTempPsk == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s - pTempPsk alloc failed.", __func__));
		return;
	}

	{
		/* Generate PSK2 */
		if (pReg->PinCodeLen == 4)
			RT_HMAC_SHA256(pReg->AuthKey, 32, &pReg->PIN[2], 2, pTempPsk, SHA256_DIGEST_SIZE);
		else
			RT_HMAC_SHA256(pReg->AuthKey, 32, &pReg->PIN[4], 4, pTempPsk, SHA256_DIGEST_SIZE);
	}

	NdisMoveMemory(pPSK2, pTempPsk, SHA256_DIGEST_SIZE);
	os_free_mem(pTempPsk);
}


/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M1 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx EAP-Req(ID)
*		1. Change the correct parameters
*		2. Build M1
*
*	========================================================================
*/
int BuildMessageM1(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[1];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	INT					idx;
	USHORT				ConfigError = 0, ConfigMethods = 0;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	UCHAR				CurOpMode = 0xFF;
	struct wifi_dev *wdev = NULL;
	wdev = (struct wifi_dev *)pWscControl->wdev;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */

	/* Enrollee 16 byte E-S1 generation */
	for (idx = 0; idx < 16; idx++)
		pReg->Es1[idx] = RandomByte(pAdapter);

	/* Enrollee 16 byte E-S2 generation */
	for (idx = 0; idx < 16; idx++)
		pReg->Es2[idx] = RandomByte(pAdapter);

	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M1 */
	TB[0] = WSC_ID_MESSAGE_M1;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. UUID_E, last 6 bytes use MAC */
	/*templen = AppendWSCTLV(WSC_ID_UUID_E, pData, pReg->EnrolleeInfo.Uuid, 0); */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWscControl->Wsc_Uuid_E[0], 0);
	pData += templen;
	Len   += templen;
	/* 4. MAC address */
	if (wdev)
		templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, wdev->if_addr, 0);
	else
		templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, pReg->SelfInfo.MacAddr, 0);

	pData += templen;
	Len   += templen;

	/* Enrollee Nonce, first generate and save to Wsc Control Block */
	for (idx = 0; idx < 16; idx++) {
		pReg->SelfNonce[idx] = RandomByte(pAdapter);
		pReg->EnrolleeNonce[idx] = pReg->SelfNonce[idx];
	}

	/* 5. Enrollee Nonce, first generate and save to Wsc Control Block */
	NdisMoveMemory(pReg->EnrolleeNonce, pReg->SelfNonce, 16);
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->SelfNonce, 0);
	pData += templen;
	Len   += templen;
	/* 6. Public Key, 192 bytes */
	templen = AppendWSCTLV(WSC_ID_PUBLIC_KEY, pData, pReg->Pke, 0);
	pData += templen;
	Len   += templen;
	/* 7. Authentication Type Flags */
	templen = AppendWSCTLV(WSC_ID_AUTH_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.AuthTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* 8. Encryption Type Flags */
	templen = AppendWSCTLV(WSC_ID_ENCR_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.EncrTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* 9. Connection Type Flag ESS */
	templen = AppendWSCTLV(WSC_ID_CONN_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/*10. Config Method */
	/*pReg->SelfInfo.ConfigMethods = cpu2be16(pWscControl->WscConfigMethods);*/
	ConfigMethods = pWscControl->WscConfigMethods;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/*
		*	WIN8 WCN Win8 WCN Wireless Push Button Auth (Push Button Test) V2
		*	need to see PBC capability in M1. @20131002
		*
		*	WPS Certification only check WSC_ID_CONFIG_METHODS of probe response.
		*	@20160803
		*/
	} else
#endif /* WSC_V2_SUPPORT */
	{
		/*
		*	WSC 1.0 WCN logo testing has AP PBC Enrollee testing item.
		*	We cannot remove PBC capability here.
		*/
		ConfigMethods = (pWscControl->WscConfigMethods & 0x00FF);
	}

	ConfigMethods = cpu2be16(ConfigMethods);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&ConfigMethods, 0);
	pData += templen;
	Len   += templen;

	/*11. Simple Config State (Not Configured) */
	if (CurOpMode == AP_MODE)
		pReg->SelfInfo.ScState = pWscControl->WscConfStatus;

	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&pReg->SelfInfo.ScState, 0);
	pData += templen;
	Len   += templen;
	/*12. Manufacture */
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData, pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;
	/*13. Model Name */
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	/*14. Model Number */
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;
	/*15. Serial Number */
	templen = AppendWSCTLV(WSC_ID_SERIAL_NUM, pData, pReg->SelfInfo.SerialNumber, strlen((RTMP_STRING *) pReg->SelfInfo.SerialNumber));
	pData += templen;
	Len   += templen;
	/*16. Primary Device Type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/*17. Device Name */
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;
	/*18. RF Band */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/*19. Associate state (Not associated) */
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&pReg->SelfInfo.AssocState, 0);
	pData += templen;
	Len   += templen;
	/*20. Device Password ID */
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&pReg->SelfInfo.DevPwdId, 0);

	pData += templen;
	Len   += templen;
	/*21. Configure Error */
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&ConfigError, 0);
	pData += templen;
	Len   += templen;
	/*22. OS Version Not associated) */
	templen = AppendWSCTLV(WSC_ID_OS_VERSION, pData, (UINT8 *)&pReg->SelfInfo.OsVersion, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */

#ifdef WSC_V2_SUPPORT

	/* Extra attribute that is not defined in WSC Sepc. */
	if (pWscControl->WscV2Info.bEnableWpsV2 && pWscTLV->pTlvData && pWscTLV->TlvLen) {
		templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
		pData += templen;
		Len   += templen;
	}

#endif /* WSC_V2_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM1 - bUPnPMsgTimerRunning = %d, pWscControl->WscUseUPnP = %d, pWscControl->EapMsgRunning = %d\n",
			 pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerRunning,
			 pWscControl->WscUseUPnP,
			 pWscControl->EapMsgRunning));

	/* Fixed WCN vista logo 2 registrar test item issue. */
	/* Also prevent that WCN GetDeviceInfo disturbs EAP processing. */
	if (pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerRunning ||
		(pWscControl->WscUseUPnP && pWscControl->EapMsgRunning))
		;
	else {
		/* Copy the content to Regdata for lasttx information */
		pReg->LastTx.Length = Len;
		NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
	}

	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM1.\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M2 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M1
*		1. Change the correct parameters
*		2. Build M2
*
*	========================================================================
*/
int BuildMessageM2(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[1];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA		pReg;
	UCHAR				DHKey[32], KDK[32], KdkInput[38], KdfKey[80];
	INT					DH_Len;
	INT				    HmacLen = 0;
	INT					idx;
	USHORT				ConfigMethods;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	pReg = (PWSC_REG_DATA) &pWscControl->RegData;

	/* Enrollee 16 byte E-S1 generation */
	for (idx = 0; idx < 16; idx++)
		pReg->Es1[idx] = RandomByte(pAdapter);

	/* Enrollee 16 byte E-S2 generation */
	for (idx = 0; idx < 16; idx++)
		pReg->Es2[idx] = RandomByte(pAdapter);

	DH_Len = sizeof(pReg->SecretKey);
	NdisZeroMemory(pReg->SecretKey, sizeof(pReg->SecretKey));
	RT_DH_SecretKey_Generate(
		pReg->Pke, sizeof(pReg->Pke),
		WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
		pReg->EnrolleeRandom,  sizeof(pReg->EnrolleeRandom),
		pReg->SecretKey, (UINT *) &DH_Len);

	/* Need to prefix zero padding */
	if ((DH_Len != sizeof(pReg->SecretKey)) &&
		(DH_Len < sizeof(pReg->SecretKey))) {
		UCHAR TempKey[192];
		INT DiffCnt;

		DiffCnt = sizeof(pReg->SecretKey) - DH_Len;
		NdisFillMemory(&TempKey, DiffCnt, 0);
		NdisCopyMemory(&TempKey[DiffCnt], pReg->SecretKey, DH_Len);
		NdisCopyMemory(pReg->SecretKey, TempKey, sizeof(TempKey));
		DH_Len += DiffCnt;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Do zero padding!\n", __func__));
	}

	RT_SHA256(&pReg->SecretKey[0], 192, &DHKey[0]);
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M2 */
	TB[0] = WSC_ID_MESSAGE_M2;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;

	/* fixed config Windows 7 issue */
	/* Enrollee Nonce, first generate and save to Wsc Control Block */
	for (idx = 0; idx < 16; idx++) {
		pReg->SelfNonce[idx] = RandomByte(pAdapter);
		pReg->RegistrarNonce[idx] = pReg->SelfNonce[idx];
	}

	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
	/* UUID, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_UUID_R, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;
	/* Publikc Key */
	templen = AppendWSCTLV(WSC_ID_PUBLIC_KEY, pData, pReg->Pkr, 0);
	pData += templen;
	Len   += templen;
	/* Authentication Type Flags */
	templen = AppendWSCTLV(WSC_ID_AUTH_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.AuthTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Encrypt Type */
	templen = AppendWSCTLV(WSC_ID_ENCR_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.EncrTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Connection Type */
	templen = AppendWSCTLV(WSC_ID_CONN_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Config Method */
	ConfigMethods = pWscControl->WscConfigMethods;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/*
		*	In WPS 2.0.0
		*	In Test Item 4.1.1 ,Step 13
		*	On the sniffer device, verify that the M2 message that the APUT sends includes
		*	the Configuration Methods attribute. The Configuration Methods attribute in the
		*	WSC IE must reflect the correct configuration methods that the Internal Registrar
		*	supports.
		*/
	} else
		ConfigMethods = (pWscControl->WscConfigMethods & 0x00FF);

#endif /* WSC_V2_SUPPORT */
	ConfigMethods = cpu2be16(ConfigMethods);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&ConfigMethods, 0);
	pData += templen;
	Len   += templen;
	/* Manufacture Name */
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData, pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;
	/* Model Name */
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	/* Model Number */
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;
	/* Serial Number */
	templen = AppendWSCTLV(WSC_ID_SERIAL_NUM, pData, pReg->SelfInfo.SerialNumber, strlen((RTMP_STRING *) pReg->SelfInfo.SerialNumber));
	pData += templen;
	Len   += templen;
	/* Prime Device Type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* Device Name */
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;
	/* RF Band */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/* Assoc State */
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&pReg->SelfInfo.AssocState, 0);
	pData += templen;
	Len   += templen;
	/* Config Error */
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&pReg->SelfInfo.ConfigError, 0);
	pData += templen;
	Len   += templen;
	/* Device Password ID */
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&pReg->SelfInfo.DevPwdId, 0);

	pData += templen;
	Len   += templen;
	/* OS Version */
	templen = AppendWSCTLV(WSC_ID_OS_VERSION, pData, (UINT8 *)&pReg->SelfInfo.OsVersion, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/* Create KDK input data */
	NdisMoveMemory(&KdkInput[0], &pReg->EnrolleeNonce[0], 16);
	NdisMoveMemory(&KdkInput[16], &pReg->PeerInfo.MacAddr[0], 6);
	NdisMoveMemory(&KdkInput[22], pReg->RegistrarNonce, 16);
	/* Generate the KDK */
	RT_HMAC_SHA256(DHKey, 32,  KdkInput, 38, KDK, SHA256_DIGEST_SIZE);
	/* KDF */
	WscDeriveKey(KDK, 32, Wsc_Personal_String, (sizeof(Wsc_Personal_String) - 1), KdfKey, 640);
	/* Assign Key from KDF */
	NdisMoveMemory(pReg->AuthKey, &KdfKey[0], 32);
	NdisMoveMemory(pReg->KeyWrapKey, &KdfKey[32], 16);
	NdisMoveMemory(pReg->Emsk, &KdfKey[48], 32);
	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	/* 22. Hmac */
	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, KDK, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM2.\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M2D Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M1 process Error
*		1. Change the correct parameters
*		2. Build M2D
*
*	========================================================================
*/
int BuildMessageM2D(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[1];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M2D */
	TB[0] = WSC_ID_MESSAGE_M2D;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->SelfNonce, 0);
	/*templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0); */
	pData += templen;
	Len   += templen;
	/* UUID, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_UUID_R, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;
	/* 7. Authentication Type Flags */
	templen = AppendWSCTLV(WSC_ID_AUTH_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.AuthTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Encrypt Type */
	templen = AppendWSCTLV(WSC_ID_ENCR_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.EncrTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Connection Type */
	templen = AppendWSCTLV(WSC_ID_CONN_TYPE_FLAGS, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
	pData += templen;
	Len   += templen;
	/* Config Methods */
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&pReg->SelfInfo.ConfigMethods, 0);
	pData += templen;
	Len   += templen;
	/* Manufacturer Name */
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData, pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;
	/* Model Name */
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	/* Model Number */
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;
	/* Serial Number */
	templen = AppendWSCTLV(WSC_ID_SERIAL_NUM, pData, pReg->SelfInfo.SerialNumber, strlen((RTMP_STRING *) pReg->SelfInfo.SerialNumber));
	pData += templen;
	Len   += templen;
	/* Prime Device Type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* Device Name */
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;
	/* RF Band */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/* Assoc State */
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&pReg->SelfInfo.AssocState, 0);
	pData += templen;
	Len   += templen;
	/* Config Error */
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&pReg->SelfInfo.ConfigError, 0);
	pData += templen;
	Len   += templen;
	/* OS Version */
	templen = AppendWSCTLV(WSC_ID_OS_VERSION, pData, (UINT8 *)&pReg->SelfInfo.OsVersion, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM2D.\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M3 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M2
*		1. Change the correct parameters
*		2. Build M3
*
*	========================================================================
*/
int BuildMessageM3(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[32];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA		pReg = NULL;
	INT				    HmacLen;
	UCHAR				*pHash = NULL;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&pHash, 512);

	if (pHash == NULL)
		return Len;

	pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M3 */
	TB[0] = WSC_ID_MESSAGE_M3;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. E-Hash1 */
	/* */
	/* Generate PSK1 */
	WscGenPSK1(pAdapter, pWscControl, &TB[0]);
	/* Copy first 16 bytes to PSK1 */
	NdisMoveMemory(pReg->Psk1, TB, 16);
	hex_dump("Es1", pReg->Es1, 16);
	hex_dump("Es2", pReg->Es2, 16);
	/* Create input for E-Hash1 */
	NdisMoveMemory(pHash, pReg->Es1, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk1, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate E-Hash1 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, pReg->EHash1, SHA256_DIGEST_SIZE);
	templen = AppendWSCTLV(WSC_ID_E_HASH1, pData, pReg->EHash1, 0);
	pData += templen;
	Len   += templen;
	/* 5. E-Hash2 */
	/* */
	/* Generate PSK2 */
	WscGenPSK2(pAdapter, pWscControl, &TB[0]);
	/* Copy first 16 bytes to PSK2 */
	NdisMoveMemory(pReg->Psk2, TB, 16);
	/* Create input for E-Hash2 */
	NdisMoveMemory(pHash, pReg->Es2, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk2, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate E-Hash2 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, pReg->EHash2, SHA256_DIGEST_SIZE);
	templen = AppendWSCTLV(WSC_ID_E_HASH2, pData, pReg->EHash2, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Generate authenticator
	*	Combine last TX & RX message contents and validate the HMAC
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, TB, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);

	if (pHash)
		os_free_mem(pHash);

	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM3 :\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M4 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M3
*		1. Change the correct parameters
*		2. Build M4
*
*	========================================================================
*/
int BuildMessageM4(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[32];
	INT				Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA			pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	INT				HmacLen;
	UCHAR				KDK[32];
	UCHAR				Plain[128]; /*, IV_EncrData[144];//IV len 16,EncrData len 128 */
	UCHAR				*IV_EncrData = NULL;/*IV len 16,EncrData len 128 */
	UINT				PlainLen = 0, EncrLen;
	UCHAR				*pHash = NULL;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&pHash, 512);

	if (pHash == NULL)
		return Len;

	os_alloc_mem(NULL, (UCHAR **)&IV_EncrData, IV_ENCR_DATA_LEN_144);

	if (IV_EncrData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		goto LabelErr;
	}

	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M4 */
	TB[0] = WSC_ID_MESSAGE_M4;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. R-Hash1 */
	/* */
	/* Generate PSK1 */
	WscGenPSK1(pAdapter, pWscControl, &TB[0]);
	/* Copy first 16 bytes to PSK1 */
	NdisMoveMemory(pReg->Psk1, TB, 16);
	hex_dump("Es1", pReg->Es1, 16);
	hex_dump("Es2", pReg->Es2, 16);
	/* Create input for R-Hash1 */
	NdisMoveMemory(pHash, pReg->Es1, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk1, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate R-Hash1 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, pReg->RHash1, SHA256_DIGEST_SIZE);
	templen = AppendWSCTLV(WSC_ID_R_HASH1, pData, pReg->RHash1, 0);
	pData += templen;
	Len   += templen;
	/* 5. R-Hash2 */
	/* */
	/* Generate PSK2 */
	WscGenPSK2(pAdapter, pWscControl, &TB[0]);
	/* Copy first 16 bytes to PSK2 */
	NdisMoveMemory(pReg->Psk2, TB, 16);
	/* Create input for R-Hash2 */
	NdisMoveMemory(pHash, pReg->Es2, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk2, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate R-Hash2 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, pReg->RHash2, SHA256_DIGEST_SIZE);
	templen = AppendWSCTLV(WSC_ID_R_HASH2, pData, pReg->RHash2, 0);
	pData += templen;
	Len   += templen;
	/* 6a. Encrypted R-S1 */
	/*    Prepare plain text */
	PlainLen += AppendWSCTLV(WSC_ID_R_SNONCE1, &Plain[0], pReg->Es1, 0);
	/* Generate HMAC */
	RT_HMAC_SHA256(pReg->AuthKey, 32, &Plain[0], PlainLen, TB, SHA256_DIGEST_SIZE);

	if (PlainLen >= 0 && PlainLen <= 123)
		PlainLen += AppendWSCTLV(WSC_ID_KEY_WRAP_AUTH, &Plain[PlainLen], TB, 0);

	/* 6b. Encrypted Settings */
	/* Encrypt data */
	EncrLen = IV_ENCR_DATA_LEN_144 - 16;
	AES_CBC_Encrypt(Plain, PlainLen, pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), &IV_EncrData[0], 16, (UINT8 *) &IV_EncrData[16], (UINT *) &EncrLen);
	templen = AppendWSCTLV(WSC_ID_ENCR_SETTINGS, pData, &IV_EncrData[0], 16 + EncrLen);/*IVLen + EncrLen */
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Combine last TX & RX message contents and validate the HMAC
	*	We have to exclude last 12 bytes from last receive since it's authenticator value
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, KDK, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
LabelErr:

	if (pHash)
		os_free_mem(pHash);

	pWscControl->WscRetryCount = 0;

	if (IV_EncrData != NULL)
		os_free_mem(IV_EncrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM4 :\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M5 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M4
*		1. Change the correct parameters
*		2. Build M5
*
*	========================================================================
*/
int BuildMessageM5(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[32];
	INT				Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA			pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	PUCHAR				pAuth;
	INT				HmacLen;
	UCHAR				Plain[128]; /*, IV_EncrData[144];//IV len 16,EncrData len 128 */
	UCHAR				*IV_EncrData = NULL;/*IV len 16,EncrData len 128 */
	UINT				PlainLen = 0, EncrLen;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&IV_EncrData, IV_ENCR_DATA_LEN_144);

	if (IV_EncrData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return 0;
	}

	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M5 */
	TB[0] = WSC_ID_MESSAGE_M5;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4a. Encrypted E-S1 */
	/*    Prepare plain text */
	PlainLen += AppendWSCTLV(WSC_ID_E_SNONCE1, &Plain[0], pReg->Es1, 0);
	/* Generate HMAC */
	RT_HMAC_SHA256(pReg->AuthKey, 32, &Plain[0], PlainLen, TB, SHA256_DIGEST_SIZE);

	if (PlainLen >= 0 && PlainLen <= 123)
		PlainLen += AppendWSCTLV(WSC_ID_KEY_WRAP_AUTH, &Plain[PlainLen], TB, 0);

	/* 4b. Encrypted Settings */
	/* Encrypt data */
	EncrLen = IV_ENCR_DATA_LEN_144 - 16;
	AES_CBC_Encrypt(Plain, PlainLen, pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), &IV_EncrData[0], 16, (UINT8 *) &IV_EncrData[16], (UINT *) &EncrLen);
	templen = AppendWSCTLV(WSC_ID_ENCR_SETTINGS, pData, &IV_EncrData[0], 16 + EncrLen);/*IVLen + EncrLen */
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Generate authenticator
	*	Combine last TX & RX message contents and validate the HMAC
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, TB, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
	pWscControl->WscRetryCount = 0;

	if (IV_EncrData != NULL)
		os_free_mem(IV_EncrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM5 :\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M6 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M5
*		1. Change the correct parameters
*		2. Build M6
*
*	========================================================================
*/
int BuildMessageM6(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[32];
	INT				Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA			pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	INT				HmacLen;
	UCHAR				KDK[32];
	UCHAR				Plain[128]; /*, IV_EncrData[144];//IV len 16,EncrData len 128 */
	UCHAR				*IV_EncrData = NULL;/*IV len 16,EncrData len 128 */
	UINT				PlainLen = 0, EncrLen;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&IV_EncrData, 144);

	if (IV_EncrData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return 0;
	}

	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M6 */
	TB[0] = WSC_ID_MESSAGE_M6;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4a. Encrypted R-S2 */
	/*    Prepare plain text */
	PlainLen += AppendWSCTLV(WSC_ID_R_SNONCE2, &Plain[0], pReg->Es2, 0);
	/* Generate HMAC */
	RT_HMAC_SHA256(pReg->AuthKey, 32, &Plain[0], PlainLen, TB, SHA256_DIGEST_SIZE);

	if (PlainLen >= 0 && PlainLen <= 123)
		PlainLen += AppendWSCTLV(WSC_ID_KEY_WRAP_AUTH, &Plain[PlainLen], TB, 0);

	/* 4b. Encrypted Settings */
	/* Encrypt data */
	EncrLen = IV_ENCR_DATA_LEN_144 - 16;
	AES_CBC_Encrypt(Plain, PlainLen, pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), &IV_EncrData[0], 16, (UINT8 *) &IV_EncrData[16], (UINT *) &EncrLen);
	templen = AppendWSCTLV(WSC_ID_ENCR_SETTINGS, pData, &IV_EncrData[0], 16 + EncrLen);/*IVLen + EncrLen */
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Combine last TX & RX message contents and validate the HMAC
	*	We have to exclude last 12 bytes from last receive since it's authenticator value
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, KDK, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);
	pWscControl->WscRetryCount = 0;

	if (IV_EncrData != NULL)
		os_free_mem(IV_EncrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM6 :\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M7 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M6
*		1. Change the correct parameters
*		2. Build M7
*
*	========================================================================
*/
int BuildMessageM7(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[32];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	INT				    HmacLen;
	UCHAR				Plain[256], *IV_EncrData = NULL; /*IV len 16 ,EncrData len */
	INT					PlainLen = 0, EncrLen;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	UCHAR				CurOpMode = 0xFF;

	os_alloc_mem(NULL, (UCHAR **)&IV_EncrData, IV_ENCR_DATA_LEN_512);

	if (IV_EncrData == NULL)
		return 0;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M7 */
	TB[0] = WSC_ID_MESSAGE_M7;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4a. Encrypted E-S2 */
	/*    Prepare plain text */
	PlainLen += AppendWSCTLV(WSC_ID_E_SNONCE2, &Plain[0], pReg->Es2, 0);

	/* Marvell WPS AP doesn't accept STA includes profile in M7. 20070604 */
	if ((CurOpMode == AP_MODE) &&
		(pWscControl->EntryIfIdx < MIN_NET_DEVICE_FOR_APCLI)) {
		USHORT  authType;
		USHORT  encyType;
		PWSC_CREDENTIAL pCredential = &pWscControl->WscProfile.Profile[0];

		WscCreateProfileFromCfg(pAdapter, ENROLLEE_ACTION | AP_MODE, pWscControl, &pWscControl->WscProfile);
		authType = pCredential->AuthType;
		encyType = pCredential->EncrType;

		/*
		*	Some Win7 WSC 1.0 STA has problem to receive mixed authType and encyType.
		*	We need to check STA is WSC 1.0 or WSC 2.0 here.
		*	If STA is WSC 1.0, re-assign authType and encyType.
		*/
		if (pWscControl->RegData.PeerInfo.Version2 == 0) {
			if (authType == (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK))
				authType = WSC_AUTHTYPE_WPA2PSK;

			if (encyType == (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES))
				encyType = WSC_ENCRTYPE_AES;
		}

		authType = cpu2be16(authType);
		encyType = cpu2be16(encyType);

		if (PlainLen >= 0 && PlainLen <= 251)
			PlainLen += AppendWSCTLV(WSC_ID_SSID, &Plain[PlainLen], pCredential->SSID.Ssid, pCredential->SSID.SsidLength);

		if (PlainLen >= 0 && PlainLen <= 247)
			PlainLen += AppendWSCTLV(WSC_ID_MAC_ADDR, &Plain[PlainLen], pCredential->MacAddr, 0);

		if (PlainLen >= 0 && PlainLen <= 243)
			PlainLen += AppendWSCTLV(WSC_ID_AUTH_TYPE, &Plain[PlainLen], (UINT8 *)&authType, 0);

		if (PlainLen >= 0 && PlainLen <= 239)
			PlainLen += AppendWSCTLV(WSC_ID_ENCR_TYPE, &Plain[PlainLen], (UINT8 *)&encyType, 0);

		if (PlainLen >= 0 && PlainLen <= 235)
			PlainLen += AppendWSCTLV(WSC_ID_NW_KEY_INDEX, &Plain[PlainLen], &pCredential->KeyIndex, 0);

		if (PlainLen >= 0 && PlainLen <= 231)
			PlainLen += AppendWSCTLV(WSC_ID_NW_KEY, &Plain[PlainLen], pCredential->Key, pCredential->KeyLength);
	}

	/* Generate HMAC */
	RT_HMAC_SHA256(pReg->AuthKey, 32, &Plain[0], PlainLen, TB, SHA256_DIGEST_SIZE);

	if (PlainLen >= 0 && PlainLen <= 227)
		PlainLen += AppendWSCTLV(WSC_ID_KEY_WRAP_AUTH, &Plain[PlainLen], TB, 0);

	/* 4b. Encrypted Settings */
	/* Encrypt data */
	EncrLen = IV_ENCR_DATA_LEN_512 - 16;
	AES_CBC_Encrypt(Plain, PlainLen, pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), &IV_EncrData[0], 16, (UINT8 *) &IV_EncrData[16], (UINT *) &EncrLen);
	templen = AppendWSCTLV(WSC_ID_ENCR_SETTINGS, pData, IV_EncrData, 16 + EncrLen);/*IVLen + EncrLen */
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Generate authenticator
	*	Combine last TX & RX message contents and validate the HMAC
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, TB, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* Copy the content to Regdata for lasttx information */
	pReg->LastTx.Length = Len;
	NdisMoveMemory(pReg->LastTx.Data, pbuf, Len);

	if (IV_EncrData)
		os_free_mem(IV_EncrData);

	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM7 :\n"));
	return Len;
}

/*
*	========================================================================
*
*	Routine Description:
*		Build WSC M8 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after M7
*		1. Change the correct parameters
*		2. Build M8
*
*	========================================================================
*/
int BuildMessageM8(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR *TB = NULL;
	INT Len = 0, templen = 0;
	PUCHAR pData = (PUCHAR)pbuf, pAuth;
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	INT HmacLen;
	UCHAR KDK[32];
	UCHAR *IV_EncrData = NULL; /*IV len 16 ,EncrData len */
	UCHAR *Plain = NULL;
	INT CerLen = 0, PlainLen = 0, EncrLen;
	PWSC_CREDENTIAL pCredential = NULL;
	USHORT AuthType = 0;
	USHORT EncrType = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR               apidx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	UCHAR				CurOpMode = 0xFF;
	os_alloc_mem(NULL, (UCHAR **)&IV_EncrData, IV_ENCR_DATA_LEN_512);

	if (!IV_EncrData)
		return 0;

	os_alloc_mem(NULL, (UCHAR **)&TB, 256);

	if (TB == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		goto LabelErr;
	}

	os_alloc_mem(NULL, (UCHAR **)&Plain, PLAIN_LEN_TOTAL_SIZE);

	if (Plain == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		goto LabelErr;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, M8 */
	TB[0] = WSC_ID_MESSAGE_M8;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		{
			WscCreateProfileFromCfg(pAdapter,
									REGISTRAR_ACTION | AP_MODE,
									pWscControl,
									&pWscControl->WscProfile);
			pCredential = &pAdapter->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	/* 4a. Encrypted R-S1 */
	CerLen += AppendWSCTLV(WSC_ID_NW_INDEX, &TB[0], (PUCHAR)"1", 0);

	if (pCredential == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: pWscControl == NULL!\n", __func__));
		goto LabelErr;
	}

	AuthType = pCredential->AuthType;
	EncrType = pCredential->EncrType;
	/*
	*	Some Win7 WSC 1.0 STA has problem to receive mixed authType and encyType.
	*	We need to check STA is WSC 1.0 or WSC 2.0 here.
	*	If STA is WSC 1.0, re-assign authType and encyType.
	*/
	{
		if (AuthType == (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK))
			AuthType = WSC_AUTHTYPE_WPA2PSK;

		if (EncrType == (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES))
			EncrType = WSC_ENCRTYPE_AES;
	}

	AuthType = cpu2be16(AuthType);
	EncrType = cpu2be16(EncrType);
	CerLen += AppendWSCTLV(WSC_ID_SSID, &TB[CerLen], pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
	CerLen += AppendWSCTLV(WSC_ID_AUTH_TYPE, &TB[CerLen], (UINT8 *)&AuthType, 0);
	CerLen += AppendWSCTLV(WSC_ID_ENCR_TYPE, &TB[CerLen], (UINT8 *)&EncrType, 0);
	CerLen += AppendWSCTLV(WSC_ID_NW_KEY_INDEX, &TB[CerLen], &pCredential->KeyIndex, 0);
	CerLen += AppendWSCTLV(WSC_ID_NW_KEY, &TB[CerLen], pCredential->Key, pCredential->KeyLength);
	CerLen += AppendWSCTLV(WSC_ID_MAC_ADDR, &TB[CerLen], pCredential->MacAddr, 0);
	/*    Prepare plain text */
		if ((CurOpMode == AP_MODE)
		   ) {
			/* Reguired attribute item in M8 if Enrollee is STA. */
			PlainLen += AppendWSCTLV(WSC_ID_CREDENTIAL, &Plain[0], TB, CerLen);

			if (PlainLen > PLAIN_LEN_TOTAL_SIZE) {
				/*Plain buf overflow*/
				os_free_mem(Plain);
				Plain = NULL;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Plain buf overflow!!!\n", __func__));
				goto LabelErr;
			}

		}

	/* Generate HMAC */
	RT_HMAC_SHA256(pReg->AuthKey, 32, &Plain[0], PlainLen, TB, SHA256_DIGEST_SIZE);
	PlainLen += AppendWSCTLV(WSC_ID_KEY_WRAP_AUTH, &Plain[PlainLen], TB, 0);

	if (PlainLen > PLAIN_LEN_TOTAL_SIZE) {
		/*Plain buf overflow*/
		os_free_mem(Plain);
		Plain = NULL;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Plain buf overflow!!!\n", __func__));
		goto LabelErr;
	}

	/* 4b. Encrypted Settings */
	/* Encrypt data */
	EncrLen = IV_ENCR_DATA_LEN_512 - 16;
	AES_CBC_Encrypt(Plain, PlainLen, pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), &IV_EncrData[0], 16, (UINT8 *) &IV_EncrData[16], (UINT *) &EncrLen);
	templen = AppendWSCTLV(WSC_ID_ENCR_SETTINGS, pData, IV_EncrData, 16 + EncrLen);/*IVLen + EncrLen */
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	/*
	*	Combine last TX & RX message contents and validate the HMAC
	*	We have to exclude last 12 bytes from last receive since it's authenticator value
	*/
	HmacLen = Len + pReg->LastRx.Length;

	if (pAdapter->pHmacData) {
		pAuth = (PUCHAR) pAdapter->pHmacData;
		NdisMoveMemory(pAuth, pReg->LastRx.Data, pReg->LastRx.Length);
		pAuth += pReg->LastRx.Length;
		NdisMoveMemory(pAuth, pbuf, Len);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	templen = AppendWSCTLV(WSC_ID_AUTHENTICATOR, pData, KDK, 0);
	pData += templen;
	Len   += templen;
LabelErr:

	if (IV_EncrData)
		os_free_mem(IV_EncrData);

	pWscControl->WscRetryCount = 0;

	if (TB != NULL)
		os_free_mem(TB);

	if (Plain != NULL)
		os_free_mem(Plain);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageM8 :\n"));
	return Len;
}

int BuildMessageDONE(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[1];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, WSC DONE */
	TB[0] = WSC_MSG_WSC_DONE;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageDONE :\n"));
	return Len;
}

int BuildMessageACK(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[1];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, WSC ACK */
	TB[0] = WSC_MSG_WSC_ACK;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageACK :\n"));
	return Len;
}

int BuildMessageNACK(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf)
{
	UCHAR				TB[2];
	INT					Len = 0, templen = 0;
	PUCHAR				pData = (PUCHAR)pbuf;
	PWSC_REG_DATA		pReg = (PWSC_REG_DATA) &pWscControl->RegData;
	USHORT              ConfigError = htons(pReg->SelfInfo.ConfigError);
#ifdef WSC_V2_SUPPORT
	PWSC_TLV			pWscTLV = &pWscControl->WscV2Info.ExtraTlv;
#endif /* WSC_V2_SUPPORT */
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Message Type, WSC NACK */
	TB[0] = WSC_ID_MESSAGE_NACK;
	templen = AppendWSCTLV(WSC_ID_MSG_TYPE, pData, TB, 0);
	pData += templen;
	Len   += templen;
	/* 3. Enrollee Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_ENROLLEE_NONCE, pData, pReg->EnrolleeNonce, 0);
	pData += templen;
	Len   += templen;
	/* 4. Registrar Nonce, 16 bytes */
	templen = AppendWSCTLV(WSC_ID_REGISTRAR_NONCE, pData, pReg->RegistrarNonce, 0);
	pData += templen;
	Len   += templen;
	/* 5. Error */
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&ConfigError, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	pWscControl->WscRetryCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("BuildMessageNACK :\n"));
	return Len;
}


/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M1 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M1
*		1. Change the correct parameters
*		2. Process M1
*
*	========================================================================
*/
int ProcessMessageM1(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg)
{
	int					ret = WSC_ERROR_NO_ERROR, DH_Len = 0, idx;
	PUCHAR				pData = NULL;
	USHORT				WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};
	UCHAR				CurOpMode = 0xFF;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	pReg->PeerInfo.Version2 = 0;
	{
		DH_Len = sizeof(pReg->Pkr);
		NdisZeroMemory(pReg->Pkr, sizeof(pReg->Pkr));

		/* Enrollee 192 random bytes for DH key generation */
		for (idx = 0; idx < 192; idx++)
			pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAdapter);

		RT_DH_PublicKey_Generate(
			WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
			WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
			pWscControl->RegData.EnrolleeRandom, sizeof(pWscControl->RegData.EnrolleeRandom),
			pReg->Pkr, (UINT *) &DH_Len);

		/* Need to prefix zero padding */
		if ((DH_Len != sizeof(pReg->Pkr)) &&
			(DH_Len < sizeof(pReg->Pkr))) {
			UCHAR TempKey[192];
			INT DiffCnt;

			DiffCnt = sizeof(pReg->Pkr) - DH_Len;
			NdisFillMemory(&TempKey, DiffCnt, 0);
			NdisCopyMemory(&TempKey[DiffCnt], pReg->Pkr, DH_Len);
			NdisCopyMemory(pReg->Pkr, TempKey, sizeof(TempKey));
			DH_Len += DiffCnt;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Do zero padding!\n", __func__));
		}

	}

	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_UUID_E))] |= (1 << WSC_TLV_BYTE1(WSC_ID_UUID_E));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MAC_ADDR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MAC_ADDR));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PUBLIC_KEY))] |= (1 << WSC_TLV_BYTE1(WSC_ID_PUBLIC_KEY));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTH_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTH_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONN_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONN_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_METHODS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_METHODS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SC_STATE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_SC_STATE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MANUFACTURER))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MANUFACTURER));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NAME))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NAME));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NUMBER))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NUMBER));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SERIAL_NUM))] |= (1 << WSC_TLV_BYTE1(WSC_ID_SERIAL_NUM));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PRIM_DEV_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_PRIM_DEV_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_NAME))] |= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_NAME));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_RF_BAND))] |= (1 << WSC_TLV_BYTE1(WSC_ID_RF_BAND));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ASSOC_STATE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ASSOC_STATE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_ERROR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_ERROR));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_PWD_ID))] |= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_PWD_ID));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_OS_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_OS_VERSION));
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;
	NdisZeroMemory(&pWscControl->WscPeerInfo, sizeof(WSC_PEER_DEV_INFO));

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M1 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M1)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_UUID_E:
			NdisMoveMemory(pReg->PeerInfo.Uuid, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_UUID_E))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_UUID_E));
			break;

		case WSC_ID_MAC_ADDR:
			NdisMoveMemory(pReg->PeerInfo.MacAddr, pData, WscLen);
			NdisMoveMemory(pWscControl->WscPeerInfo.WscPeerMAC, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MAC_ADDR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MAC_ADDR));
			break;

		case WSC_ID_ENROLLEE_NONCE:
			NdisMoveMemory(pReg->EnrolleeNonce, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
			break;

		case WSC_ID_PUBLIC_KEY:
			/* Get Enrollee Public Key */
			NdisMoveMemory(pReg->Pke, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PUBLIC_KEY))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_PUBLIC_KEY));
			break;

		case WSC_ID_AUTH_TYPE_FLAGS:
			pReg->PeerInfo.AuthTypeFlags = *((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTH_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTH_TYPE_FLAGS));
			break;

		case WSC_ID_ENCR_TYPE_FLAGS:
			pReg->PeerInfo.EncrTypeFlags = *((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_TYPE_FLAGS));
			break;

		case WSC_ID_CONN_TYPE_FLAGS:
			pReg->PeerInfo.ConnTypeFlags = *pData;
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONN_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONN_TYPE_FLAGS));
			break;

		case WSC_ID_CONFIG_METHODS:
			pReg->PeerInfo.ConfigMethods = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_METHODS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_METHODS));
			break;

		case WSC_ID_SC_STATE:
			pReg->PeerInfo.ScState = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SC_STATE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_SC_STATE));
			break;

		case WSC_ID_MANUFACTURER:
			NdisMoveMemory(&pReg->PeerInfo.Manufacturer, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerManufacturer, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MANUFACTURER))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MANUFACTURER));
			break;

		case WSC_ID_MODEL_NAME:
			NdisMoveMemory(&pReg->PeerInfo.ModelName, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerModelName, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NAME))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NAME));
			break;

		case WSC_ID_MODEL_NUMBER:
			NdisMoveMemory(&pReg->PeerInfo.ModelNumber, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerModelNumber, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NUMBER))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NUMBER));
			break;

		case WSC_ID_SERIAL_NUM:
			NdisMoveMemory(&pReg->PeerInfo.SerialNumber, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerSerialNumber, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SERIAL_NUM))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_SERIAL_NUM));
			break;

		case WSC_ID_PRIM_DEV_TYPE:
			NdisMoveMemory(&pReg->PeerInfo.PriDeviceType, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PRIM_DEV_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_PRIM_DEV_TYPE));
			break;

		case WSC_ID_DEVICE_NAME:
			NdisMoveMemory(&pReg->PeerInfo.DeviceName, pData, WscLen);
			NdisMoveMemory(pWscControl->WscPeerInfo.WscPeerDeviceName, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_NAME))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_NAME));
			break;

		case WSC_ID_RF_BAND:
			pReg->PeerInfo.RfBand = *pData;
			/*if() ret = WSC_ERROR_CHAN24_NOT_SUPP; */
			/*if() ret = WSC_ERROR_CHAN50_NOT_SUPP; */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_RF_BAND))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_RF_BAND));
			break;

		case WSC_ID_ASSOC_STATE:
			pReg->PeerInfo.AssocState = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ASSOC_STATE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ASSOC_STATE));
			break;

		case WSC_ID_CONFIG_ERROR:
			pReg->PeerInfo.ConfigError = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_ERROR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_ERROR));
			break;

		case WSC_ID_DEVICE_PWD_ID:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("   WPS Registrar DPID %04x\n", pReg->SelfInfo.DevPwdId));

			if (get_unaligned((PUSHORT) pData) == WSC_DEVICEPWDID_DEFAULT) { /**(PUSHORT) pData) */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx WPS           DPID PIN\n"));
				pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
			} else if (get_unaligned((PUSHORT) pData) == WSC_DEVICEPWDID_PUSH_BTN) { /**(PUSHORT) pData) */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx WPS           DPID PBC\n"));
				pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
			} else
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx WPS           DPID unsupport\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_PWD_ID))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_PWD_ID));
			break;

		case WSC_ID_OS_VERSION:
			pReg->PeerInfo.OsVersion = get_unalignedlong((PULONG) pData);/**((PULONG) pData); */
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_OS_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_OS_VERSION));
			break;

		case WSC_ID_VENDOR_EXT:
#ifdef WSC_V2_SUPPORT

			if (pWscControl->WscV2Info.bEnableWpsV2) {
				UCHAR tmp_data_len = 0;

				WscParseV2SubItem(WFA_EXT_ID_VERSION2, pData, WscLen, &pReg->PeerInfo.Version2, &tmp_data_len);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM1 --> Version2 = %x\n", pReg->PeerInfo.Version2));
			}

#endif /* WSC_V2_SUPPORT */

			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM1 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM1 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M2 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M2
*		1. Change the correct parameters
*		2. Process M2
*
*	========================================================================
*/
int ProcessMessageM2(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl,
	IN  VOID *precv,
	IN  INT Length,
	IN  UCHAR apidx,
	OUT PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32];
	UCHAR DHKey[32], KdkInput[38], KdfKey[80];
	INT DH_Len;
	PUCHAR pData = NULL;
	USHORT WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	pReg->PeerInfo.Version2 = 0;
	RTMPZeroMemory(KDK, 32);
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_UUID_R))] |= (1 << WSC_TLV_BYTE1(WSC_ID_UUID_R));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PUBLIC_KEY))] |= (1 << WSC_TLV_BYTE1(WSC_ID_PUBLIC_KEY));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTH_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTH_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONN_TYPE_FLAGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONN_TYPE_FLAGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_METHODS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_METHODS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MANUFACTURER))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MANUFACTURER));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NAME))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NAME));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NUMBER))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NUMBER));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SERIAL_NUM))] |= (1 << WSC_TLV_BYTE1(WSC_ID_SERIAL_NUM));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PRIM_DEV_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_PRIM_DEV_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_NAME))] |= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_NAME));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_RF_BAND))] |= (1 << WSC_TLV_BYTE1(WSC_ID_RF_BAND));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ASSOC_STATE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ASSOC_STATE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_ERROR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_ERROR));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_PWD_ID))] |= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_PWD_ID));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_OS_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_OS_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;
#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAdapter, pReg->PeerInfo.MacAddr);
#endif
	NdisZeroMemory(&pWscControl->WscPeerInfo, sizeof(WSC_PEER_DEV_INFO));

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M2)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_ENROLLEE_NONCE:

			/* for verification with our enrollee nonce */
			if (RTMPCompareMemory(pReg->SelfNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M2 Compare enrollee nonce mismatched\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
			break;

		case WSC_ID_REGISTRAR_NONCE:
			NdisMoveMemory(pReg->RegistrarNonce, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
			break;

		case WSC_ID_UUID_R:
			NdisMoveMemory(pReg->PeerInfo.Uuid, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_UUID_R))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_UUID_R));
			break;

		case WSC_ID_PUBLIC_KEY:
			/* Get Registrar Public Key */
			NdisMoveMemory(&pReg->Pkr, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PUBLIC_KEY))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_PUBLIC_KEY));
			break;

		case WSC_ID_AUTH_TYPE_FLAGS:
			pReg->PeerInfo.AuthTypeFlags = get_unaligned((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTH_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTH_TYPE_FLAGS));
			break;

		case WSC_ID_ENCR_TYPE_FLAGS:
			pReg->PeerInfo.EncrTypeFlags = get_unaligned((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_TYPE_FLAGS));
			break;

		case WSC_ID_CONN_TYPE_FLAGS:
			pReg->PeerInfo.ConnTypeFlags = *pData;
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONN_TYPE_FLAGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONN_TYPE_FLAGS));
			break;

		case WSC_ID_CONFIG_METHODS:
			pReg->PeerInfo.ConfigMethods = get_unaligned((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_METHODS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_METHODS));
			break;

		case WSC_ID_MANUFACTURER:
			NdisMoveMemory(&pReg->PeerInfo.Manufacturer, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerManufacturer, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MANUFACTURER))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MANUFACTURER));
			break;

		case WSC_ID_MODEL_NAME:
			NdisMoveMemory(&pReg->PeerInfo.ModelName, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerModelName, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NAME))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NAME));
			break;

		case WSC_ID_MODEL_NUMBER:
			NdisMoveMemory(&pReg->PeerInfo.ModelNumber, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerModelNumber, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MODEL_NUMBER))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MODEL_NUMBER));
			break;

		case WSC_ID_SERIAL_NUM:
			NdisMoveMemory(&pReg->PeerInfo.SerialNumber, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerSerialNumber, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_SERIAL_NUM))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_SERIAL_NUM));
			break;

		case WSC_ID_PRIM_DEV_TYPE:
			NdisMoveMemory(&pReg->PeerInfo.PriDeviceType, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_PRIM_DEV_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_PRIM_DEV_TYPE));
			break;

		case WSC_ID_DEVICE_NAME:
			NdisMoveMemory(&pReg->PeerInfo.DeviceName, pData, WscLen);
			NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerDeviceName, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_NAME))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_NAME));
			break;

		case WSC_ID_RF_BAND:
			pReg->PeerInfo.RfBand = *pData;
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_RF_BAND))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_RF_BAND));
			break;

		case WSC_ID_ASSOC_STATE:
			pReg->PeerInfo.AssocState = get_unaligned((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ASSOC_STATE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ASSOC_STATE));
			break;

		case WSC_ID_CONFIG_ERROR:
			pReg->PeerInfo.ConfigError = get_unaligned((PUSHORT) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_CONFIG_ERROR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_CONFIG_ERROR));
			break;

		case WSC_ID_DEVICE_PWD_ID:
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_DEVICE_PWD_ID))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_DEVICE_PWD_ID));
			break;

		case WSC_ID_OS_VERSION:
			pReg->PeerInfo.OsVersion = get_unalignedlong((PULONG) pData);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_OS_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_OS_VERSION));
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
			break;

		case WSC_ID_VENDOR_EXT:
#ifdef WSC_V2_SUPPORT
			if (pWscControl->WscV2Info.bEnableWpsV2) {
				UCHAR tmp_data_len = 0;

				WscParseV2SubItem(WFA_EXT_ID_VERSION2, pData, WscLen, &pReg->PeerInfo.Version2, &tmp_data_len);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2 --> Version2 = %x\n", pReg->PeerInfo.Version2));
			}

#endif /* WSC_V2_SUPPORT */
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		NdisMoveMemory(&pWscControl->WscPeerInfo.WscPeerMAC, &pWscControl->RegData.PeerInfo.MacAddr, 6);
		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	DH_Len = sizeof(pReg->SecretKey);
	NdisZeroMemory(pReg->SecretKey, sizeof(pReg->SecretKey));
	RT_DH_SecretKey_Generate(
		pReg->Pkr, sizeof(pReg->Pkr),
		WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
		pReg->EnrolleeRandom,  sizeof(pReg->EnrolleeRandom),
		pReg->SecretKey, (UINT *) &DH_Len);

	/* Need to prefix zero padding */
	if ((DH_Len != sizeof(pReg->SecretKey)) &&
		(DH_Len < sizeof(pReg->SecretKey))) {
		UCHAR TempKey[192];
		INT DiffCnt;

		DiffCnt = sizeof(pReg->SecretKey) - DH_Len;
		NdisFillMemory(&TempKey, DiffCnt, 0);
		NdisCopyMemory(&TempKey[DiffCnt], pReg->SecretKey, DH_Len);
		NdisCopyMemory(pReg->SecretKey, TempKey, sizeof(TempKey));
		DH_Len += DiffCnt;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Do zero padding!\n", __func__));
	}

	/* Compute the DHKey based on the DH secret */
	RT_SHA256(&pReg->SecretKey[0], 192, &DHKey[0]);
	/* Create KDK input data */
	NdisMoveMemory(&KdkInput[0], pReg->SelfNonce, 16);
	NdisMoveMemory(&KdkInput[16], pReg->SelfInfo.MacAddr, 6);
	NdisMoveMemory(&KdkInput[22], pReg->RegistrarNonce, 16);
	/* Generate the KDK */
	RT_HMAC_SHA256(DHKey, 32,  KdkInput, 38, KDK, SHA256_DIGEST_SIZE);
	/* KDF */
	WscDeriveKey(KDK, 32, Wsc_Personal_String, (sizeof(Wsc_Personal_String) - 1), KdfKey, 640);
	/* Assign Key from KDF */
	NdisMoveMemory(pReg->AuthKey, &KdfKey[0], 32);
	NdisMoveMemory(pReg->KeyWrapKey, &KdfKey[32], 16);
	NdisMoveMemory(pReg->Emsk, &KdfKey[48], 32);
	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM2 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

	/* out : */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M2D Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- rewrite buffer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M2D
*		1. Change the correct parameters
*		2. Process M2D
*
*	========================================================================
*/
int ProcessMessageM2D(
	IN	PRTMP_ADAPTER		pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg)
{
	int					ret = WSC_ERROR_NO_ERROR;
	PUCHAR				pData = NULL;
	USHORT				WscType, WscLen;
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M2D)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			break;

		case WSC_ID_ENROLLEE_NONCE:

			/* for verification with our enrollee nonce */
			if (RTMPCompareMemory(pReg->EnrolleeNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M2 Compare enrollee nonce mismatched\n"));
			break;

		case WSC_ID_REGISTRAR_NONCE:
			NdisMoveMemory(pReg->RegistrarNonce, pData, WscLen);
			break;

		case WSC_ID_UUID_R:
			NdisMoveMemory(pReg->PeerInfo.Uuid, pData, WscLen);
			break;

		case WSC_ID_PUBLIC_KEY:
			/* There shall be no Public transmitted in M2D */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2D --> Receive WSC_ID_PUBLIC_KEY!! werid!\n"));
			break;

		case WSC_ID_AUTH_TYPE_FLAGS:
			pReg->PeerInfo.AuthTypeFlags = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			break;

		case WSC_ID_ENCR_TYPE_FLAGS:
			pReg->PeerInfo.EncrTypeFlags = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			break;

		case WSC_ID_CONN_TYPE_FLAGS:
			pReg->PeerInfo.ConnTypeFlags = *pData;
			break;

		case WSC_ID_CONFIG_METHODS:
			pReg->PeerInfo.ConfigMethods = be2cpu16(get_unaligned((PUSHORT) pData));/*be2cpu16(*((PUSHORT) pData)); */
			break;

		case WSC_ID_MANUFACTURER:
			NdisMoveMemory(&pReg->PeerInfo.Manufacturer, pData, WscLen);
			break;

		case WSC_ID_MODEL_NAME:
			NdisMoveMemory(&pReg->PeerInfo.ModelName, pData, WscLen);
			break;

		case WSC_ID_MODEL_NUMBER:
			NdisMoveMemory(&pReg->PeerInfo.ModelNumber, pData, WscLen);
			break;

		case WSC_ID_SERIAL_NUM:
			NdisMoveMemory(&pReg->PeerInfo.SerialNumber, pData, WscLen);
			break;

		case WSC_ID_PRIM_DEV_TYPE:
			NdisMoveMemory(&pReg->PeerInfo.PriDeviceType, pData, WscLen);
			break;

		case WSC_ID_DEVICE_NAME:
			NdisMoveMemory(&pReg->PeerInfo.DeviceName, pData, WscLen);
			break;

		case WSC_ID_RF_BAND:
			pReg->PeerInfo.RfBand = *pData;
			break;

		case WSC_ID_ASSOC_STATE:
			pReg->PeerInfo.AssocState = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			break;

		case WSC_ID_CONFIG_ERROR:
			pReg->PeerInfo.ConfigError = get_unaligned((PUSHORT) pData);/**((PUSHORT) pData); */
			break;

		case WSC_ID_DEVICE_PWD_ID:
			break;

		case WSC_ID_OS_VERSION:
			pReg->PeerInfo.OsVersion = get_unalignedlong((PULONG)pData);
			break;

		case WSC_ID_AUTHENTICATOR:
			/* No authenticator in M2D */
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2D --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM2D :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M3 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M3
*		1. Change the correct parameters
*		2. Process M3
*
*	========================================================================
*/
int ProcessMessageM3(
	IN	PRTMP_ADAPTER pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32];
	PUCHAR pData = NULL;
	USHORT WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};

	RTMPZeroMemory(KDK, 32);
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_E_HASH1))] |= (1 << WSC_TLV_BYTE1(WSC_ID_E_HASH1));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_E_HASH2))] |= (1 << WSC_TLV_BYTE1(WSC_ID_E_HASH2));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M3 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M3)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_REGISTRAR_NONCE:

			/* for verification with our Registrar nonce */
			if (RTMPCompareMemory(pReg->RegistrarNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M3 Compare Registrar nonce mismatched\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
			break;

		case WSC_ID_E_HASH1:
			NdisMoveMemory(&pReg->EHash1[0], pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_E_HASH1))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_E_HASH1));
			break;

		case WSC_ID_E_HASH2:
			NdisMoveMemory(&pReg->EHash2[0], pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_E_HASH2))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_E_HASH2));
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM3 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM3 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM3 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M4 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M4
*		1. Change the correct parameters
*		2. Process M4
*
*	========================================================================
*/
int ProcessMessageM4(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl,
	IN  VOID *precv,
	IN  INT Length,
	OUT PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32], RHash[32];
	INT EncrLen;
	PUCHAR pData = NULL;
	UCHAR *IV_DecrData = NULL; /*IV len 16 ,DecrData len */
	UCHAR *pHash = NULL; /*Reuse IV_DecrData memory */
	USHORT WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};

	RTMPZeroMemory(KDK, 32);
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_R_HASH1))] |= (1 << WSC_TLV_BYTE1(WSC_ID_R_HASH1));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_R_HASH2))] |= (1 << WSC_TLV_BYTE1(WSC_ID_R_HASH2));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
	os_alloc_mem(NULL, (UCHAR **)&IV_DecrData, 512);

	if (!IV_DecrData) {
		ret = WSC_ERROR_CAN_NOT_ALLOCMEM;
		return ret;
	}

	pHash = IV_DecrData;
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M4)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_ENROLLEE_NONCE:

			/* for verification with our enrollee nonce */
			if (RTMPCompareMemory(pReg->EnrolleeNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M4 Compare enrollee nonce mismatched\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
			break;

		case WSC_ID_R_HASH1:
			NdisMoveMemory(&pReg->RHash1, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_R_HASH1))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_R_HASH1));
			break;

		case WSC_ID_R_HASH2:
			NdisMoveMemory(&pReg->RHash2, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_R_HASH2))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_R_HASH2));
			break;

		case WSC_ID_ENCR_SETTINGS:

			/* There shall have smoe kind of length check */
			if (WscLen <= 16)
				break;

			if (WscLen > 512) {
				/* ApEncrSetting is not enough */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ApEncrSettings array size is not enough, require %d\n", WscLen));
				break;
			}

			NdisMoveMemory(IV_DecrData, pData, WscLen);
			EncrLen = sizeof(pReg->ApEncrSettings);
			AES_CBC_Decrypt(IV_DecrData + 16, (WscLen - 16), pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), IV_DecrData, 16, (UINT8 *) pReg->ApEncrSettings, (UINT *) &EncrLen);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("M4 ApEncrSettings len = %d\n", EncrLen));
			/* Parse encryption settings */
			WscParseEncrSettings(pAdapter, pReg->ApEncrSettings, EncrLen, pWscControl);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM4 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Verify R-Hash1 */
	/* Create input for R-Hash1 */
	NdisMoveMemory(pHash, pReg->Rs1, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk1, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate R-Hash1 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, RHash, SHA256_DIGEST_SIZE);

	if (RTMPCompareMemory(pReg->RHash1, RHash, 32) != 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM4 --> RHash1 not matched\n"));
		ret = WSC_ERROR_DEV_PWD_AUTH_FAIL;
		goto out;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM4 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

out:

	if (IV_DecrData)
		os_free_mem(IV_DecrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM4 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M5 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M5
*		1. Change the correct parameters
*		2. Process M5
*
*	========================================================================
*/
int ProcessMessageM5(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl,
	IN  VOID *precv,
	IN  INT Length,
	OUT PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32], EHash[32];
	INT EncrLen;
	PUCHAR pData = NULL;
	UCHAR *IV_DecrData = NULL; /*IV len 16 ,DecrData len */
	UCHAR *pHash = NULL; /*Reuse IV_DecrData memory */
	USHORT WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};

	RTMPZeroMemory(KDK, 32);
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
	os_alloc_mem(NULL, (UCHAR **)&IV_DecrData, 512);

	if (!IV_DecrData) {
		ret = WSC_ERROR_CAN_NOT_ALLOCMEM;
		return ret;
	}

	pHash = IV_DecrData;
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M5)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_REGISTRAR_NONCE:
			/* for verification with our Registrar nonce */
			if (RTMPCompareMemory(pReg->RegistrarNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M5 Compare Registrar nonce mismatched\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_REGISTRAR_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_REGISTRAR_NONCE));
			break;

		case WSC_ID_ENCR_SETTINGS:

			/* There shall have smoe kind of length check */
			if (WscLen <= 16)
				break;

			if (WscLen > 512) {
				/* ApEncrSetting is not enough */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ApEncrSettings array size is not enough, require %d\n", WscLen));
				break;
			}

			NdisMoveMemory(IV_DecrData, pData, WscLen);
			EncrLen = sizeof(pReg->ApEncrSettings);
			AES_CBC_Decrypt(IV_DecrData + 16, (WscLen - 16), pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), IV_DecrData, 16, (UINT8 *) pReg->ApEncrSettings, (UINT *) &EncrLen);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("M5 ApEncrSettings len = %d\n", EncrLen));
			/* Parse encryption settings */
			WscParseEncrSettings(pAdapter, pReg->ApEncrSettings, EncrLen, pWscControl);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM4 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
	}

	/* Verify E-Hash1 */
	/* Create input for E-Hash1 */
	NdisMoveMemory(pHash, pReg->Es1, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk1, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate E-Hash1 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, EHash, SHA256_DIGEST_SIZE);

	if (RTMPCompareMemory(pReg->EHash1, EHash, 32) != 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM5 --> EHash1 not matched\n"));
		pReg->SelfInfo.ConfigError = WSC_ERROR_HASH_FAIL;
		ret = WSC_ERROR_HASH_FAIL;
		goto out;
	}

	if (pAdapter->pHmacData)
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM5 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

out:

	if (IV_DecrData)
		os_free_mem(IV_DecrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM5 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M6 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M6
*		1. Change the correct parameters
*		2. Process M6
*
*	========================================================================
*/
int ProcessMessageM6(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl,
	IN  VOID *precv,
	IN  INT Length,
	OUT PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32], RHash[32];
	INT EncrLen;
	PUCHAR pData = NULL;
	UCHAR *IV_DecrData = NULL; /*IV len 16 ,DecrData len */
	UCHAR *pHash = NULL; /*Reuse IV_DecrData memory */
	USHORT WscType, WscLen, FieldCheck[7] = {0, 0, 0, 0, 0, 0, 0};

	RTMPZeroMemory(KDK, 32);
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] |= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] |= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
	FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] |= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
	os_alloc_mem(NULL, (UCHAR **)&IV_DecrData, 512);

	if (!IV_DecrData) {
		ret = WSC_ERROR_CAN_NOT_ALLOCMEM;
		return ret;
	}

	pHash = IV_DecrData;
	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = cpu2be16(TLV_Recv.Type);
		WscLen  = cpu2be16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_VERSION))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_VERSION));
			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M6)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_MSG_TYPE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_MSG_TYPE));
			break;

		case WSC_ID_ENROLLEE_NONCE:

			/* for verification with our enrollee nonce */
			if (RTMPCompareMemory(pReg->EnrolleeNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M6 Compare enrollee nonce mismatched\n"));

			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENROLLEE_NONCE))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENROLLEE_NONCE));
			break;

		case WSC_ID_ENCR_SETTINGS:

			/* There shall have smoe kind of length check */
			if (WscLen <= 16)
				break;

			if (WscLen > 512) {
				/* ApEncrSetting is not enough */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ApEncrSettings array size is not enough, require %d\n", WscLen));
				break;
			}

			NdisMoveMemory(IV_DecrData, pData, WscLen);
			EncrLen = sizeof(pReg->ApEncrSettings);
			AES_CBC_Decrypt(IV_DecrData + 16, (WscLen - 16), pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), IV_DecrData, 16, (UINT8 *) pReg->ApEncrSettings, (UINT *) &EncrLen);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("M6 ApEncrSettings len = %d\n", EncrLen));
			/* Parse encryption settings */
			WscParseEncrSettings(pAdapter, pReg->ApEncrSettings, EncrLen, pWscControl);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_ENCR_SETTINGS))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_ENCR_SETTINGS));
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			FieldCheck[(WSC_TLV_BYTE2(WSC_ID_AUTHENTICATOR))] ^= (1 << WSC_TLV_BYTE1(WSC_ID_AUTHENTICATOR));
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM6 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Verify R-Hash2 */
	/* Create input for R-Hash1 */
	NdisMoveMemory(pHash, pReg->Rs2, 16);
	NdisMoveMemory(pHash + 16, pReg->Psk2, 16);
	NdisMoveMemory(pHash + 32, pReg->Pke, 192);
	NdisMoveMemory(pHash + 224, pReg->Pkr, 192);
	/* Generate R-Hash2 */
	RT_HMAC_SHA256(pReg->AuthKey, 32, pHash, 416, RHash, SHA256_DIGEST_SIZE);

	if (RTMPCompareMemory(pReg->RHash2, RHash, 32) != 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM6 --> RHash2 not matched\n"));
		ret = WSC_ERROR_DEV_PWD_AUTH_FAIL;
		goto out;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM6 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (FieldCheck[0] || FieldCheck[1] || FieldCheck[2] || FieldCheck[3] || FieldCheck[4] || FieldCheck[5] || FieldCheck[6])
		ret = WSC_ERROR_WANTING_FIELD;

out:

	if (IV_DecrData)
		os_free_mem(IV_DecrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM6 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M7 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M7
*		1. Change the correct parameters
*		2. Process M7
*
*	========================================================================
*/
int ProcessMessageM7(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl,
	IN  VOID *precv,
	IN  INT Length,
	OUT PWSC_REG_DATA pReg)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32];
	INT EncrLen;
	PUCHAR pData = NULL;
	USHORT WscType, WscLen;
	UCHAR *IV_DecrData = NULL; /*IV len 16 ,DecrData len */

	RTMPZeroMemory(KDK, 32);
	os_alloc_mem(NULL, (UCHAR **)&IV_DecrData, 1024);

	if (!IV_DecrData) {
		ret = WSC_ERROR_CAN_NOT_ALLOCMEM;
		return ret;
	}

	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M2 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M7)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			break;

		case WSC_ID_REGISTRAR_NONCE:

			/* for verification with our Registrar nonce */
			if (RTMPCompareMemory(pReg->RegistrarNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M5 Compare Registrar nonce mismatched\n"));

			break;

		case WSC_ID_ENCR_SETTINGS:

			/* There shall have smoe kind of length check */
			if (WscLen <= 16)
				break;

			if (WscLen > 1024) {
				/* ApEncrSetting is not enough */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ApEncrSettings array size is not enough, require %d\n", WscLen));
				break;
			}

			NdisMoveMemory(IV_DecrData, pData, WscLen);
			EncrLen = sizeof(pReg->ApEncrSettings);
			AES_CBC_Decrypt(IV_DecrData + 16, (WscLen - 16), pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), IV_DecrData, 16, (UINT8 *) pReg->ApEncrSettings, (UINT *) &EncrLen);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("M7 ApEncrSettings len = %d\n", EncrLen));
			/* Parse encryption settings */
			WscParseEncrSettings(pAdapter, pReg->ApEncrSettings, EncrLen, pWscControl);
			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM7 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM7 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (IV_DecrData)
		os_free_mem(IV_DecrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM7 :\n"));
	return ret;
}

/*
*	========================================================================
*
*	Routine Description:
*		Process WSC M8 Message
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pbuf		- recv buffer
*		Length		- recv Length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after Rx M8
*		1. Change the correct parameters
*		2. Process M8
*
*	========================================================================
*/
int ProcessMessageM8(
	IN  PRTMP_ADAPTER pAdapter,
	IN  VOID *precv,
	IN  INT Length,
	IN  PWSC_CTRL pWscControl)
{
	int ret = WSC_ERROR_NO_ERROR;
	INT HmacLen;
	UCHAR Hmac[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, KDK[32];
	INT EncrLen;
	PUCHAR pData = NULL;
	USHORT WscType, WscLen;
	UCHAR *IV_DecrData = NULL; /*IV len 16 ,DecrData len */
	PWSC_REG_DATA pReg = &pWscControl->RegData;

	RTMPZeroMemory(KDK, 32);
	os_alloc_mem(NULL, (UCHAR **)&IV_DecrData, 1024);

	if (!IV_DecrData) {
		ret = WSC_ERROR_CAN_NOT_ALLOCMEM;
		return ret;
	}

	/* Copy the content to Regdata for lastRx information */
	/* Length must include authenticator IE size */
	pReg->LastRx.Length = Length;
	NdisMoveMemory(pReg->LastRx.Data, precv, Length);
	pData = pReg->LastRx.Data;

	/* Start to process WSC IEs */
	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		/* Parse M8 WSC type and store to RegData structure */
		switch (WscType) {
		case WSC_ID_VERSION:
			if (pReg->SelfInfo.Version != *pData)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Version mismatched %02x\n", *pData));

			break;

		case WSC_ID_MSG_TYPE:
			if ((*pData) != WSC_ID_MESSAGE_M8)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Rx WPS Message Type mismatched %02x\n", *pData));

			break;

		case WSC_ID_ENROLLEE_NONCE:

			/* for verification with our enrollee nonce */
			if (RTMPCompareMemory(pReg->EnrolleeNonce, pData, WscLen) != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Rx M8 Compare enrollee nonce mismatched\n"));

			break;

		case WSC_ID_ENCR_SETTINGS:

			/* There shall have smoe kind of length check */
			if (WscLen <= 16)
				break;

			if (WscLen > 1024) {
				/* ApEncrSetting is not enough */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ApEncrSettings array size is not enough, require %d\n", WscLen));
				break;
			}

			NdisMoveMemory(IV_DecrData, pData, WscLen);
			EncrLen = sizeof(pReg->ApEncrSettings);
			AES_CBC_Decrypt(IV_DecrData + 16, (WscLen - 16), pReg->KeyWrapKey, sizeof(pReg->KeyWrapKey), IV_DecrData, 16, (UINT8 *) pReg->ApEncrSettings, (UINT *) &EncrLen);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("M8 ApEncrSettings len = %d\n", EncrLen));

			/* Parse encryption settings */
			if (WscProcessCredential(pAdapter, pReg->ApEncrSettings, EncrLen, pWscControl) == FALSE) {
				if (IV_DecrData)
					os_free_mem(IV_DecrData);

				return WSC_ERROR_SETUP_LOCKED;
			}

			break;

		case WSC_ID_AUTHENTICATOR:
			NdisMoveMemory(Hmac, pData, WscLen);
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM8 --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	/* Combine last TX & RX message contents and validate the HMAC */
	/* We have to exclude last 12 bytes from last receive since it's authenticator value */
	HmacLen = pReg->LastTx.Length + pReg->LastRx.Length - 12;

	if (pAdapter->pHmacData) {
		NdisMoveMemory(pAdapter->pHmacData, pReg->LastTx.Data, pReg->LastTx.Length);
		NdisMoveMemory(pAdapter->pHmacData + pReg->LastTx.Length, pReg->LastRx.Data, pReg->LastRx.Length - 12);
		/* Validate HMAC, reuse KDK buffer */
		RT_HMAC_SHA256(pReg->AuthKey, 32, pAdapter->pHmacData, HmacLen, KDK, SHA256_DIGEST_SIZE);
	}

	if (RTMPEqualMemory(Hmac, KDK, 8) != 1) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("ProcessMessageM8 --> HMAC not match\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("MD --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT) KDK)), (UINT)cpu2be32(*((PUINT)(KDK + 4)))));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("calculated --> 0x%08x-%08x\n", (UINT)cpu2be32(*((PUINT)&Hmac[0])), (UINT)cpu2be32(*((PUINT)&Hmac[4]))));
		ret = WSC_ERROR_HMAC_FAIL;
	}

	if (IV_DecrData)
		os_free_mem(IV_DecrData);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ProcessMessageM8 :\n"));
	return ret;
}
#endif /* WSC_INCLUDED */
