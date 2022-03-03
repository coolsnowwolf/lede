/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_ez.c

	Abstract:
	Easy Setup APIs

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#ifdef WH_EZ_SETUP
#include "rt_config.h"

UINT32 AuthMode_to_AKM_map(NDIS_802_11_AUTHENTICATION_MODE authMode)
{
	UINT32 _AKMMap;

	CLEAR_SEC_AKM(_AKMMap);

	switch(authMode)
	{
		case Ndis802_11AuthModeOpen:
			SET_AKM_OPEN(_AKMMap);
			break;
		case Ndis802_11AuthModeShared:
			SET_AKM_SHARED(_AKMMap);
			break;
		case Ndis802_11AuthModeAutoSwitch:
			SET_AKM_AUTOSWITCH(_AKMMap);
			break;
		case Ndis802_11AuthModeWPA:
			SET_AKM_WPA1(_AKMMap);
			break;
		case Ndis802_11AuthModeWPAPSK:
			SET_AKM_WPA1PSK(_AKMMap);
			break;
		case Ndis802_11AuthModeWPANone:
			SET_AKM_WPANONE(_AKMMap);
			break;
		case Ndis802_11AuthModeWPA2:
			SET_AKM_WPA2(_AKMMap);
			break;
		case Ndis802_11AuthModeWPA2PSK:
			SET_AKM_WPA2PSK(_AKMMap);
			break;
		case Ndis802_11AuthModeWPA1WPA2:
			//SET_AKM_FT_WPA2(_AKMMap); //Arvind : not found
			break;
        case Ndis802_11AuthModeWPA1PSKWPA2PSK:
			//SET_AKM_FT_WPA2PSK(_AKMMap); //Arvind : not found
			break;
#ifdef WAPI_SUPPORT
		case Ndis802_11AuthModeWAICERT :
			SET_AKM_WAICERT(_AKMMap);
			break;		
		case Ndis802_11AuthModeWAIPSK :
			SET_AKM_WPIPSK(_AKMMap);
			break;
#endif			
		default:
			break;
	}

	return _AKMMap;
}

NDIS_802_11_AUTHENTICATION_MODE AKM_map_to_AuthMode(UINT32 _AKMMap)
{

	NDIS_802_11_AUTHENTICATION_MODE authMode;
	authMode = 0;
	
	if(IS_AKM_OPEN(_AKMMap))
		authMode =Ndis802_11AuthModeOpen;
	else if(IS_AKM_SHARED(_AKMMap))
		authMode =Ndis802_11AuthModeShared;
	else if(IS_AKM_AUTOSWITCH(_AKMMap))
		authMode =Ndis802_11AuthModeAutoSwitch;
	else if(IS_AKM_WPA1(_AKMMap))
		authMode =Ndis802_11AuthModeWPA;
	else if(IS_AKM_WPA1PSK(_AKMMap))
		authMode =Ndis802_11AuthModeWPAPSK;
	else if(IS_AKM_WPANONE(_AKMMap))
		authMode =Ndis802_11AuthModeWPANone;
	else if(IS_AKM_WPA2(_AKMMap))
		authMode =Ndis802_11AuthModeWPA2;
	else if(IS_AKM_WPA2PSK(_AKMMap))
		authMode =Ndis802_11AuthModeWPA2PSK;
#ifdef WAPI_SUPPORT
	else if(IS_AKM_WAICERT(_AKMMap))
		authMode =Ndis802_11AuthModeWAICERT;
	else if(IS_AKM_WPIPSK(_AKMMap))
		authMode =Ndis802_11AuthModeWAIPSK;
#endif

	return authMode;
}

UINT32 WepStatus_to_PairwiseCipher(NDIS_802_11_WEP_STATUS WepStatus)
{
	UINT32 PairwiseCipher;

	PairwiseCipher = 0x0;

	switch(WepStatus)
	{
		case Ndis802_11WEPDisabled:
			SET_CIPHER_NONE(PairwiseCipher);
			break;
		case Ndis802_11WEPEnabled:
			SET_CIPHER_WEP(PairwiseCipher);
			break;
		case Ndis802_11TKIPEnable:
			SET_CIPHER_TKIP(PairwiseCipher);
			break;
		case Ndis802_11AESEnable:
			SET_CIPHER_CCMP128(PairwiseCipher);
			break;
		case Ndis802_11TKIPAESMix:
			SET_CIPHER_TKIP(PairwiseCipher);
			SET_CIPHER_CCMP128(PairwiseCipher);
			break;
		default:
			break;
	}

	return PairwiseCipher;
}


NDIS_802_11_WEP_STATUS PairwiseCipher_to_WepStatus(UINT32 PairwiseCipher)
{

	NDIS_802_11_WEP_STATUS WepStatus;
	WepStatus = 0;
	
	if(IS_CIPHER_NONE(PairwiseCipher))
		WepStatus =Ndis802_11WEPDisabled;
	else if(IS_CIPHER_WEP(PairwiseCipher))
		WepStatus =Ndis802_11WEPEnabled;
	else if(IS_CIPHER_TKIP(PairwiseCipher))
		WepStatus =Ndis802_11TKIPEnable;
	else if(IS_CIPHER_CCMP128(PairwiseCipher))
		WepStatus =Ndis802_11AESEnable;
	else if(IS_CIPHER_TKIP(PairwiseCipher) && IS_CIPHER_CCMP128(PairwiseCipher))
		WepStatus =Ndis802_11TKIPAESMix;

	return WepStatus;
}




#endif

