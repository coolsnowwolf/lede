#ifndef __AP_CFG_H__
#define __AP_CFG_H__

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

#ifdef VENDOR_FEATURE6_SUPPORT
VOID RTMPAPGetAssoMacTable(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);
#endif /* VENDOR_FEATURE6_SUPPORT */

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER	pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT                 cmd);

VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

INT RTMPIoctlRXStatistics(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetMacTableStaInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);


VOID RTMPAPIoctlE2PROM(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_IOCTL_INPUT_STRUCT * wrq);

#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))
VOID RTMPAPIoctlBBP(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_IOCTL_INPUT_STRUCT * wrq);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RTMP_RF_RW_SUPPORT */

#endif /* DBG */

VOID RtmpDrvMaxRateGet(
	IN	VOID					*pReserved,
	/*	IN	PHTTRANSMIT_SETTING		pHtPhyMode, */
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN	UINT8					Antenna,
	OUT	UINT32 * pRate);

#ifdef WSC_AP_SUPPORT
VOID RTMPGetCurrentCred(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
/*add by woody */
#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
VOID RTMPAR9IoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);


VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif/*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */
#endif /* WSC_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaAid(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaRsn(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);


#ifdef RADIUS_ACCOUNTING_SUPPORT
VOID RTMPIoctlQueryStaData(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_ACCOUNTING_SUPPORT */

#ifdef RADIUS_MAC_ACL_SUPPORT
PRT_802_11_RADIUS_ACL_ENTRY RadiusFindAclEntry(
	PLIST_HEADER            pCacheList,
	IN      PUCHAR          pMacAddr);

VOID RTMPIoctlAddRadiusMacAuthCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlDelRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlClearRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

INT Set_AP_Daemon_Status(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 WorkSpaceID,
	IN BOOLEAN Status);

INT Set_AP_IE(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * IE,
	IN UINT32 IELen);

#ifdef CONFIG_HOTSPOT
INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * ANQPReq,
	IN UINT32 ANQPReqLen);
#endif

INT	ApCfg_Set_PerMbssMaxStaNum_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN RTMP_STRING * arg);

INT	ApCfg_Set_IdleTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_IE(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *IE,
	IN UINT32 IELen);

struct apcfg_parameters {
	LONG cfg_mode[2]; /*WirelessMode*/
	ULONG tx_power_percentage; /*TxPower*/
	ULONG tx_preamble; /*TxPreamble*/
	UINT32 conf_len_thld; /*RTSThreshold*/
	UINT32 oper_len_thld;
	UINT32 conf_frag_thld; /*FragThreshold*/
	UINT32 oper_frag_thld;
	BOOLEAN bEnableTxBurst;	/*TxBurst*/
	BOOLEAN bUseShortSlotTime;	/*ShortSlot*/
#ifdef DOT11_N_SUPPORT
	UCHAR conf_ht_bw; /*HT_BW*/
	UCHAR oper_ht_bw;
#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable; /*HT_BSSCoexistence*/
#endif
	UCHAR ht_tx_streams; /*HT_TxStream*/
	UCHAR ht_rx_streams; /*HT_RxStream*/
	BOOLEAN bBADecline; /*HT_BADecline*/
	UINT32 AutoBA; /*HT_AutoBA*/
	UINT32 AmsduEnable; /*HT_AMSDU*/
	UINT32 RxBAWinLimit; /*HT_BAWinSize*/
	UCHAR ht_gi; /*HT_GI*/
	UCHAR ht_stbc; /*HT_STBC*/
	UCHAR ht_ldpc; /*HT_LDPC*/
	BOOLEAN bRdg; /*HT_RDG*/
#endif

	BOOLEAN HT_DisallowTKIP; /*HT_DisallowTKIP*/

#ifdef DOT11_VHT_AC
	UCHAR conf_vht_bw; /*VHT_BW*/
	UCHAR oper_vht_bw;
	UCHAR vht_sgi; /*VHT_SGI*/
	UCHAR vht_stbc; /*VHT_STBC*/
	UCHAR vht_bw_signal; /*VHT_BW_SIGNAL*/
	UCHAR vht_ldpc; /*VHT_LDPC*/
	BOOLEAN g_band_256_qam; /*G_BAND_256QAM*/
#endif

	BOOLEAN bIEEE80211H; /*IEEE80211H*/

#ifdef MT_DFS_SUPPORT
	BOOLEAN bDfsEnable; /*DfsEnable*/
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	BOOLEAN DfsZeroWaitSupport; /*DfsZeroWait*/
#endif

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ETxBfEnCond; /*ETxBfEnCond*/
#endif
#endif

	UINT32 ITxBfEn; /*ITxBfEn*/

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG MUTxRxEnable; /*MUTxRxEnable*/
#endif
#endif
	UCHAR channel;
	UCHAR CentralChannel;
	UCHAR ext_channel;
};


#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */

INT set_qiscdump_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);

#endif /* __AP_CFG_H__ */

