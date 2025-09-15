/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	band_steering.h
*/

#ifndef _BAND_STEERING_H_
#define __BAND_STEERING_H__

#ifdef BAND_STEERING

/* ioctl */
INT Show_BndStrg_List(PRTMP_ADAPTER	pAd, PSTRING	 arg);
INT Show_BndStrg_Info(PRTMP_ADAPTER	pAd, PSTRING	 arg);
INT Set_BndStrg_Enable(PRTMP_ADAPTER	pAd, PSTRING	 arg);
INT Set_BndStrg_RssiDiff(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_RssiLow(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_Age(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_HoldTime(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_CheckTime5G(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_FrmChkFlag(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_BndStrg_CndChkFlag(PRTMP_ADAPTER pAd, PSTRING arg);
#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(PRTMP_ADAPTER	pAd, PSTRING	 arg);
#endif /* BND_STRG_DBG */

INT BndStrg_Init(PRTMP_ADAPTER pAd);
INT BndStrg_Release(PRTMP_ADAPTER pAd);
INT BndStrg_TableInit(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table);
INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table);

BOOLEAN BndStrg_CheckConnectionReq(
		PRTMP_ADAPTER	pAd,
		PUCHAR pSrcAddr,
		UINT8 FrameType,
		PCHAR Rssi);

INT BndStrg_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable);
INT BndStrg_SetInfFlags(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BOOLEAN bInfReady);
BOOLEAN BndStrg_IsClientStay(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry);
INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);




/* Macro */
#define IS_BND_STRG_DUAL_BAND_CLIENT(_Control_Flags) \
	((_Control_Flags & fBND_STRG_CLIENT_SUPPORT_2G) && (_Control_Flags & fBND_STRG_CLIENT_SUPPORT_5G))

#define BND_STRG_CHECK_CONNECTION_REQ(_pAd, _wdev, _SrcAddr, _FrameType, _Rssi0, _Rssi1, _Rssi2, _pRet) \
{	\
	CHAR Rssi[3] = {0};	\
	Rssi[0] = _Rssi0 ? ConvertToRssi(_pAd, (CHAR)_Rssi0, RSSI_0) : 0;	\
	Rssi[1] = _Rssi1 ? ConvertToRssi(_pAd, (CHAR)_Rssi1, RSSI_1) : 0;	\
	Rssi[2] = _Rssi2 ? ConvertToRssi(_pAd, (CHAR)_Rssi2, RSSI_2) : 0;	\
\
	*_pRet = BndStrg_CheckConnectionReq( _pAd, 	\
								_SrcAddr,		\
								_FrameType,		\
								Rssi);	\
}

#ifdef BND_STRG_DBG
#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"
#define YLW(_text)  "\033[1;33m"_text"\033[0m"
#define BLUE(_text) "\033[1;36m"_text"\033[0m"

#define BND_STRG_DBGPRINT(_Level, _Fmt) DBGPRINT(_Level, _Fmt)
#else /* BND_STRG_DBG */
#define RED(_text)	 _text
#define GRN(_text) _text
#define YLW(_text) _text
#define BLUE(_text) _text

#define BND_STRG_DBGPRINT(_Level, _Fmt)
#endif /* !BND_STRG_DBG */

#ifdef BND_STRG_QA
#define BND_STRG_PRINTQAMSG(_table, _Addr, _Fmt) \
{	\
	if (MAC_ADDR_EQUAL(_table->MonitorAddr, _Addr))	\
		DBGPRINT(RT_DEBUG_OFF, _Fmt); \
}
#else
#define BND_STRG_PRINTQAMSG(_Level, _Fmt)
#endif /* BND_STRG_QA */

#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_H_ */

