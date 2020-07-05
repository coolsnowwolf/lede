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
INT Show_BndStrg_List(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Show_BndStrg_Info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_Enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_BndStrg_Param(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);
#endif /* BND_STRG_DBG */

INT BndStrg_Init(PRTMP_ADAPTER pAd);
INT BndStrg_Release(PRTMP_ADAPTER pAd);
INT BndStrg_TableInit(PRTMP_ADAPTER pAd, INT apidx);
INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table);
PBND_STRG_CLI_TABLE Get_BndStrgTable(PRTMP_ADAPTER pAd, INT apidx);
PBND_STRG_CLI_ENTRY BndStrg_TableLookup(PBND_STRG_CLI_TABLE table, PUCHAR pAddr);

/* WPS_BandSteering Support */
PWPS_WHITELIST_ENTRY FindWpsWhiteListEntry(PLIST_HEADER pWpsWhiteList, PUCHAR pMacAddr);
VOID AddWpsWhiteList(PLIST_HEADER pWpsWhiteList, PUCHAR pMacAddr);
VOID DelWpsWhiteListExceptMac(PLIST_HEADER pWpsWhiteList, PUCHAR pMacAddr);
VOID ClearWpsWhiteList(PLIST_HEADER pWpsWhiteList);

PBS_LIST_ENTRY FindBsListEntry(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID AddBsListEntry(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID DelBsListEntry(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID ClearBsList(PLIST_HEADER pBsList);


BOOLEAN BndStrg_CheckConnectionReq(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev,
	PUCHAR pSrcAddr,
	struct raw_rssi_info *rssi_info,
	ULONG MsgType,
	PEER_PROBE_REQ_PARAM * ProbeReqParam);



INT BndStrg_Tb_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable, CHAR *IfName);
INT BndStrg_SetInfFlags(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PBND_STRG_CLI_TABLE table, BOOLEAN bInfReady);
INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx);
#ifdef VENDOR_FEATURE5_SUPPORT
void BndStrg_GetNvram(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx);
void BndStrg_SetNvram(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx);
INT Show_BndStrg_NvramTable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif /* VENDOR_FEATURE5_SUPPORT */
INT Set_BndStrg_BssIdx(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
void BndStrg_UpdateEntry(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry, IE_LISTS *ie_list, BOOLEAN bConnStatus);
UINT8 GetNssFromHTCapRxMCSBitmask(UINT32 RxMCSBitmask);
void BndStrgSetProfileParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer);
void BndStrgHeartBeatMonitor(PRTMP_ADAPTER	pAd);
INT BndStrgSendMsg(PRTMP_ADAPTER pAd, BNDSTRG_MSG *msg);
void BndStrg_send_BTM_req(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *PeerMACAddr, IN RTMP_STRING *BTMReq, IN UINT32 BTMReqLen, PBND_STRG_CLI_TABLE table);
void BndStrg_Send_NeighborReport(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table);


#define IS_VALID_MAC(addr) \
	((addr[0])|(addr[1])|(addr[2])|(addr[3])|(addr[4])|(addr[5]))

#ifdef BND_STRG_DBG
#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"
#define YLW(_text)  "\033[1;33m"_text"\033[0m"
#define BLUE(_text) "\033[1;36m"_text"\033[0m"

#define BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt)
#else /* BND_STRG_DBG */
#define RED(_text)	 _text
#define GRN(_text) _text
#define YLW(_text) _text
#define BLUE(_text) _text

#define BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, _Level, _Fmt)
#endif /* !BND_STRG_DBG */

#ifdef BND_STRG_QA
#define BND_STRG_PRINTQAMSG(_table, _Addr, _Fmt) \
{	\
	if (MAC_ADDR_EQUAL(_table->MonitorAddr, _Addr))	\
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, _Fmt); \
}
#else
#define BND_STRG_PRINTQAMSG(_Level, _Fmt)
#endif /* BND_STRG_QA */

#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_H_ */

