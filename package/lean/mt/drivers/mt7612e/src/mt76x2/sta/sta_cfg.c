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
    sta_ioctl.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#include	"rt_config.h"



INT Set_AutoReconnect_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg);

INT Set_AdhocN_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg);


#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg);
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */


#ifdef CARRIER_DETECTION_SUPPORT
INT Set_StaCarrierDetect_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
INT	Set_DyncVgaEnable_Proc(
	IN RTMP_ADAPTER		*pAd, 
	IN PSTRING			arg);
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef IWSC_SUPPORT
INT	Set_IWscLimitedUI_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef IWSC_TEST_SUPPORT
INT	Set_IWscDefaultSecurity_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_IWscSmpbcEnrScanOnly_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_IWscEmptySubmaskList_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_IWscBlockConnection_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* IWSC_TEST_SUPPORT */

INT	Set_IWscOOB_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_IWscSinglePIN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* IWSC_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
INT set_monitor_channel(PRTMP_ADAPTER ad, PSTRING arg)
{
	INT bw = 0, channel = 0, pri_idx = 0, rv = 0;
	UINT rf_val = 0;
	UINT8 ext_ch;
	
	if (arg) {
		rv = sscanf(arg, "%d,%d,%d", &(bw), &(channel), &(pri_idx));
		DBGPRINT(RT_DEBUG_TRACE, ("%s():rv = %d, bw = %d, channel = %d, pri_idx = %d\n", __FUNCTION__, rv, bw, channel, pri_idx));
		ad->CommonCfg.Channel = channel;
		if (rv == 3) {
			if (bw == BW_40) {
				if (pri_idx == 0) {
					ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel + 2;
					ext_ch = EXTCHA_ABOVE;
				} else if (pri_idx == 1) {
					ext_ch = EXTCHA_BELOW;
					if (channel == 14) {
						ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel - 1;
					} else {
						ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel - 2;
					}
				} else {
					DBGPRINT(RT_DEBUG_ERROR, ("pri_idx(%d) is invalid\n"));
					return FALSE;
				}	
			}
		} else if (rv == 2) {
			if (bw == BW_20) {
				ad->CommonCfg.CentralChannel = channel;
			} else if ((bw == BW_80) && channel > 14) {
#ifdef DOT11_VHT_AC
				ad->CommonCfg.vht_bw = VHT_BW_80;
				ad->CommonCfg.vht_cent_ch = vht_cent_ch_freq(ad, ad->CommonCfg.Channel);
#endif	
			} else {
				return FALSE;
			}
		} else {

			return FALSE; 
		} 
	}

	bbp_set_bw(ad, bw);

	/* TX/Rx : control channel setting */
	rtmp_mac_set_ctrlch(ad, ext_ch);
	bbp_set_ctrlch(ad, ext_ch);

#ifdef DOT11_VHT_AC
	if (ad->CommonCfg.BBPCurrentBW == BW_80)
		ad->hw_cfg.cent_ch = ad->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		ad->hw_cfg.cent_ch = ad->CommonCfg.CentralChannel;
	
	AsicSwitchChannel(ad, ad->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(ad, ad->hw_cfg.cent_ch);

	return TRUE;
}
#endif
#ifdef ED_MONITOR
//let run-time turn on/off
INT set_ed_chk_proc(RTMP_ADAPTER *pAd, PSTRING arg);
INT set_ed_block_tx_thresh(RTMP_ADAPTER *pAd, PSTRING arg);
INT set_ed_false_cca_threshold(RTMP_ADAPTER *pAd, PSTRING arg);
INT set_ed_threshold(RTMP_ADAPTER *pAd, PSTRING arg);
INT show_ed_stat_proc(RTMP_ADAPTER *pAd, PSTRING arg);
INT set_ed_current_rssi_threhold_proc(RTMP_ADAPTER *pAd, PSTRING arg);
INT set_ed_debug_proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* ED_MONITOR */
INT show_ed_cnt_for_channel_quality(RTMP_ADAPTER *pAd, PSTRING arg);


static struct {
	PSTRING name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, PSTRING arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"DriverVersion",				Set_DriverVersion_Proc},
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",		Set_CountryRegionABand_Proc},
	{"SSID",						Set_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"Channel",					Set_Channel_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"HtBw",		                Set_HtBw_Proc},
	{"HtMcs",		                Set_HtMcs_Proc},
	{"HtGi",		                Set_HtGi_Proc},
	{"HtOpMode",		            Set_HtOpMode_Proc},
	{"HtStbc",					Set_HtStbc_Proc},
	{"HtExtcha",		            Set_HtExtcha_Proc},
	{"HtMpduDensity",		        Set_HtMpduDensity_Proc},
	{"HtBaWinSize",		        	Set_HtBaWinSize_Proc},
	{"HtRdg",		        		Set_HtRdg_Proc},
	{"HtAmsdu",		        		Set_HtAmsdu_Proc},
	{"HtAutoBa",		        	Set_HtAutoBa_Proc},
	{"HtBaDecline",				Set_BADecline_Proc},
	{"HtProtect",		        	Set_HtProtect_Proc},
	{"HtMimoPs",		        	Set_HtMimoPs_Proc},
	{"HtDisallowTKIP",				Set_HtDisallowTKIP_Proc},
#ifdef DOT11N_DRAFT3
	{"HtBssCoex",				Set_HT_BssCoex_Proc},
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	{"VhtBw",					Set_VhtBw_Proc},
	{"VhtStbc",					Set_VhtStbc_Proc},
	{"VhtBwSignal",				set_VhtBwSignal_Proc},
#endif /* DOT11_VHT_AC */

#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},       
#endif /* AGGREGATION_SUPPORT */

	{"WmmCapable",					Set_WmmCapable_Proc},
	{"IEEE80211H",					Set_IEEE80211H_Proc},
    {"NetworkType",                 Set_NetworkType_Proc},
	{"AuthMode",					Set_AuthMode_Proc},
	{"EncrypType",					Set_EncrypType_Proc},
	{"DefaultKeyID",				Set_DefaultKeyID_Proc},
	{"Key1",						Set_Key1_Proc},
	{"Key2",						Set_Key2_Proc},
	{"Key3",						Set_Key3_Proc},
	{"Key4",						Set_Key4_Proc},
	{"WPAPSK",						Set_WPAPSK_Proc},
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"PSMode",                      Set_PSMode_Proc},
#ifdef DBG
	{"Debug",						Set_Debug_Proc},
	{"DebugFunc",					Set_DebugFunc_Proc},
#endif /* DBG */

#ifdef TXBF_SUPPORT
#ifndef MT76x2
	{"TxBfTag",				        Set_TxBfTag_Proc},
	{"ReadITxBf",				    Set_ReadITxBf_Proc},
	{"WriteITxBf",				    Set_WriteITxBf_Proc},
	{"StatITxBf",				    Set_StatITxBf_Proc},
	{"ReadETxBf",				    Set_ReadETxBf_Proc},
	{"WriteETxBf",				    Set_WriteETxBf_Proc},
	{"StatETxBf",				    Set_StatETxBf_Proc},
	{"ETxBfTimeout",		        Set_ETxBfTimeout_Proc},
#ifdef STA_ITXBF_SUPPORT
	{"ITxBfTimeout",		        Set_ITxBfTimeout_Proc},
	{"InvTxBfTag",				    Set_InvTxBfTag_Proc},
	{"ITxBfCal",				    Set_ITxBfCal_Proc},
	{"ITxBfDivCal",				    Set_ITxBfDivCal_Proc},
	{"ITxBfLnaCal",				    Set_ITxBfLnaCal_Proc},
#endif /* STA_ITXBF_SUPPORT */
#else
#ifdef STA_ITXBF_SUPPORT
	{"InvTxBfTag",					Set_InvTxBfTag_Proc},
	{"ITxBfDivCal", 					Set_ITxBfDivCal_Proc},
	{"ITxBfEn", 						Set_ITxBfEn_Proc},
#endif	
#endif
	{"ETxBfEnCond",					Set_StaETxBfEnCond_Proc},
	{"ETxBfCodebook",				Set_ETxBfCodebook_Proc},
	{"ETxBfCoefficient",			Set_ETxBfCoefficient_Proc},
	{"ETxBfGrouping",				Set_ETxBfGrouping_Proc},
	{"ETxBfNoncompress",			Set_ETxBfNoncompress_Proc},
	{"ETxBfIncapable",				Set_ETxBfIncapable_Proc},
	{"NoSndgCntThrd",				Set_NoSndgCntThrd_Proc},
	{"NdpSndgStreams",				Set_NdpSndgStreams_Proc},
	{"TriggerSounding",				Set_Trigger_Sounding_Proc},

#ifdef MT76x2
	{"TxBfProfileTagHelp",     Set_TxBfProfileTag_Help},
    {"TxBfProfileTagValid",    Set_TxBfProfileTagValid},
	{"TxBfProfileTagTimeOut",  Set_TxBfProfileTag_TimeOut},
	{"TxBfProfileTagMatrix",   Set_TxBfProfileTag_Matrix},
    {"TxBfProfileTagSNR",      Set_TxBfProfileTag_SNR},
    {"TxBfProfileTagTxScale",  Set_TxBfProfileTag_TxScale},
    {"TxBfProfileTagMac",      Set_TxBfProfileTag_MAC},
    {"TxBfProfileTagFlg",      Set_TxBfProfileTag_Flg},
    {"TxBfProfileTagRead",     Set_TxBfProfileTagRead},
	{"TxBfProfileTagWrite",    Set_TxBfProfileTagWrite},
    {"TxBfProfileDataRead",    Set_TxBfProfileDataRead},
    {"TxBfProfileDataWrite",   Set_TxBfProfileDataWrite},
	{"TxBfProfileDataReadAll", Set_TxBfProfileDataReadAll},
    {"TxBfProfileDataWriteAll",Set_TxBfProfileDataWriteAll},
#endif
#endif /* TXBF_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	{"StreamMode",					Set_StreamMode_Proc},
	{"StreamModeMCS",				Set_StreamModeMCS_Proc},
#endif /* STREAM_MODE_SUPPORT */

	{"RateAlg",					Set_RateAlg_Proc},
#ifdef NEW_RATE_ADAPT_SUPPORT
	{"LowTrafficThrd",				Set_LowTrafficThrd_Proc},
	{"TrainUpRule",					Set_TrainUpRule_Proc},
	{"TrainUpRuleRSSI",				Set_TrainUpRuleRSSI_Proc},
	{"TrainUpLowThrd",				Set_TrainUpLowThrd_Proc},
	{"TrainUpHighThrd",				Set_TrainUpHighThrd_Proc},
	{"RateTable",					Set_RateTable_Proc},
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef PRE_ANT_SWITCH
	{"PreAntSwitch",		        Set_PreAntSwitch_Proc},
	{"PreAntSwitchRSSI",		    Set_PreAntSwitchRSSI_Proc},
	{"PreAntSwitchTimeout",		    Set_PreAntSwitchTimeout_Proc},
#endif /* PRE_ANT_SWITCH */


#ifdef DBG_CTRL_SUPPORT
	{"DebugFlags",					Set_DebugFlags_Proc},
#ifdef INCLUDE_DEBUG_QUEUE
	{"DebugQueue",					Set_DebugQueue_Proc},
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef RALINK_ATE
	{"ATE",							Set_ATE_Proc},
	{"ATEDA",						Set_ATE_DA_Proc},
	{"ATESA",						Set_ATE_SA_Proc},
	{"ATEBSSID",					Set_ATE_BSSID_Proc},
	{"ATECHANNEL",					Set_ATE_CHANNEL_Proc},
	{"ATEINITCHAN",					Set_ATE_INIT_CHAN_Proc},
	{"ADCDump",					Set_ADCDump_Proc},
#ifdef RTMP_TEMPERATURE_CALIBRATION
	{"ATETEMPCAL",				Set_ATE_TEMP_CAL_Proc},
#endif /* RTMP_TEMPERATURE_CALIBRATION */
#ifdef RTMP_INTERNAL_TX_ALC
	{"ATETSSICBA",					Set_ATE_TSSI_CALIBRATION_Proc},
	{"ATETSSICBAEX",					Set_ATE_TSSI_CALIBRATION_EX_Proc},
#if defined(RT3350) || defined(RT3352)
	{"ATETSSICALBRENABLE",				RT335x_Set_ATE_TSSI_CALIBRATION_ENABLE_Proc},
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef  RTMP_TEMPERATURE_COMPENSATION
	{"ATEREADEXTSSI",			Set_ATE_READ_EXTERNAL_TSSI_Proc},
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	{"ATETXPOW0",					Set_ATE_TX_POWER0_Proc},
	{"ATETXPOW1",					Set_ATE_TX_POWER1_Proc},
#ifdef DOT11N_SS3_SUPPORT
	{"ATETXPOW2",					Set_ATE_TX_POWER2_Proc},
#endif /* DOT11N_SS3_SUPPORT */
	{"ATETXPOWEVAL",				Set_ATE_TX_POWER_EVALUATION_Proc},
	{"ATETXANT",					Set_ATE_TX_Antenna_Proc},
	{"ATERXANT",					Set_ATE_RX_Antenna_Proc},
	{"ATETXFREQOFFSET",				Set_ATE_TX_FREQ_OFFSET_Proc},
	{"ATETXBW",						Set_ATE_TX_BW_Proc},
	{"ATETXLEN",					Set_ATE_TX_LENGTH_Proc},
	{"ATETXCNT",					Set_ATE_TX_COUNT_Proc},
	{"ATETXMCS",					Set_ATE_TX_MCS_Proc},
	{"ATETXSTBC",					Set_ATE_TX_STBC_Proc},
	{"ATETXMODE",					Set_ATE_TX_MODE_Proc},
	{"ATETXGI",						Set_ATE_TX_GI_Proc},
	{"ATERXFER",					Set_ATE_RX_FER_Proc},
	{"ATERRF",						Set_ATE_Read_RF_Proc},
#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
	{"ATEWRF1",						Set_ATE_Write_RF1_Proc},
	{"ATEWRF2",						Set_ATE_Write_RF2_Proc},
	{"ATEWRF3",						Set_ATE_Write_RF3_Proc},
	{"ATEWRF4",						Set_ATE_Write_RF4_Proc},
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */
	{"ATELDE2P",				    Set_ATE_Load_E2P_Proc},
#ifdef RTMP_EFUSE_SUPPORT
	{"ATELDE2PFROMBUF",			Set_ATE_Load_E2P_From_Buf_Proc},
#endif /* RTMP_EFUSE_SUPPORT */
	{"ATERE2P",						Set_ATE_Read_E2P_Proc},
#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUUPORT */
#ifdef SINGLE_SKU_V2
	{"ATESINGLESKU",				Set_ATE_SINGLE_SKU_Proc},
#endif
	{"ATEAUTOALC",					Set_ATE_AUTO_ALC_Proc},	
	{"ATEIPG",						Set_ATE_IPG_Proc},
	{"ATEPAYLOAD",					Set_ATE_Payload_Proc},
	{"ATEFIXEDPAYLOAD",				Set_ATE_Fixed_Payload_Proc},
#ifdef TXBF_SUPPORT
	{"ATETXBF",						Set_ATE_TXBF_Proc},
	{"ATETXSOUNDING",				Set_ATE_TXSOUNDING_Proc},
	{"ATETXBFDIVCAL",				Set_ATE_TXBF_DIVCAL_Proc},
	{"ATETXBFLNACAL",				Set_ATE_TXBF_LNACAL_Proc},
	{"ATETxBfGolden",				Set_ATE_TXBF_GOLDEN_Proc},
#endif /* TXBF_SUPPORT */
	{"ATESHOW",						Set_ATE_Show_Proc},
	{"ATEHELP",						Set_ATE_Help_Proc},

#ifdef RALINK_QA
	{"TxStop",						Set_TxStop_Proc},
	{"RxStop",						Set_RxStop_Proc},	
#endif /* RALINK_QA */

#ifdef MT76x2
	{"ATEDOCAL",					Set_ATE_DO_CALIBRATION_Proc},
	{"ATELoadCR",					Set_ATE_Load_CR_Proc},
#endif /* MT76x2 */
#endif /* RALINK_ATE */

#ifdef WPA_SUPPLICANT_SUPPORT
    {"WpaSupport",                  Set_Wpa_Support},
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
	{"WscUuidE",					Set_WscUUIDE_Proc},
	{"WscGetConf",					Set_WscGetConf_Proc},
    {"WscVendorPinCode",            Set_WscVendorPinCode_Proc},
#ifdef WSC_V2_SUPPORT
	{"WscForceSetAP",               Set_WscForceSetAP_Proc},
#endif /* WSC_V2_SUPPORT */
#ifdef IWSC_SUPPORT
	{"IWscLimitedUI",            	Set_IWscLimitedUI_Proc},
#ifdef IWSC_TEST_SUPPORT
	{"IWscDefaultSecurity",        	Set_IWscDefaultSecurity_Proc},
	{"IWscSmpbcScanOnly",        	Set_IWscSmpbcEnrScanOnly_Proc},
	{"IWscEmptySubmaskList",		Set_IWscEmptySubmaskList_Proc},
	{"IWscBlockConnection", 		Set_IWscBlockConnection_Proc},
#endif /* IWSC_TEST_SUPPORT */
	{"IWscOOB",            			Set_IWscOOB_Proc},
	{"IWscSinglePIN",            	Set_IWscSinglePIN_Proc},
#endif // IWSC_SUPPORT //
#endif /* WSC_STA_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
	{"EthConvertMode",              Set_EthConvertMode_Proc},
	{"EthCloneMac",					Set_EthCloneMac_Proc},
#endif /* ETH_CONVERT_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
	{"ft",							FT_Ioctl},
#endif /* DOT11R_FT_SUPPORT */

	{"FixedTxMode",                 Set_FixedTxMode_Proc},
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	{"OpMode",						Set_OpMode_Proc},
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef ETH_CONVERT_SUPPORT
	{"FragFlag",		            Set_FragTest_Proc},
#endif /* ETH_CONVERT_SUPPORT */
#ifdef DOT11_N_SUPPORT
    {"TGnWifiTest",                 Set_TGnWifiTest_Proc},
#endif /* DOT11_N_SUPPORT */
#ifdef QOS_DLS_SUPPORT
	{"DlsAddEntry",					Set_DlsAddEntry_Proc},
	{"DlsTearDownEntry",			Set_DlsTearDownEntry_Proc},
#endif /* QOS_DLS_SUPPORT */
	{"LongRetry",	        		Set_LongRetryLimit_Proc},
	{"ShortRetry",	        		Set_ShortRetryLimit_Proc},
	{"AutoFallBack",	        	Set_AutoFallBack_Proc},
#ifdef EXT_BUILD_CHANNEL_LIST
	{"11dClientMode",				Set_Ieee80211dClientMode_Proc},
	{"CountryCode", 				Set_ExtCountryCode_Proc},
	{"DfsType", 					Set_ExtDfsType_Proc},
	{"ChannelListAdd",				Set_ChannelListAdd_Proc},
	{"ChannelListShow", 			Set_ChannelListShow_Proc},
	{"ChannelListDel",				Set_ChannelListDel_Proc},
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef CARRIER_DETECTION_SUPPORT
	{"CarrierDetect",				Set_StaCarrierDetect_Proc},
#endif /* CARRIER_DETECTION_SUPPORT */


#ifdef RTMP_EFUSE_SUPPORT
	{"efuseLoadFromBin",			set_eFuseLoadFromBin_Proc}, /* For backward compatible, the usage is the same as bufferLoadFromBin + bufferWriteBack */
	{"efuseFreeNumber",			set_eFuseGetFreeBlockCount_Proc},
	{"efuseDump",				set_eFusedump_Proc},
#ifdef RALINK_ATE
	{"bufferLoadFromEfuse",		Set_LoadEepromBufferFromEfuse_Proc},
	{"efuseBufferModeWriteBack",	set_eFuseBufferModeWriteBack_Proc}, /* For backward compatible, the usage is the same as bufferWriteBack */
#endif /* RALINK_ATE */
#endif /* RTMP_EFUSE_SUPPORT */
	{"bufferLoadFromBin", 			Set_LoadEepromBufferFromBin_Proc},
	{"bufferWriteBack", 			Set_EepromBufferWriteBack_Proc},



#ifdef DOT11R_FT_SUPPORT
	{"FtApBssid",                  	Set_FtApBssid_Proc},
	{"FtSupport",					Set_FtSupport_Proc},	
#endif /* DOT11R_FT_SUPPORT */
	{"BeaconLostTime",				Set_BeaconLostTime_Proc},
	{"AutoRoaming",					Set_AutoRoaming_Proc},
	{"SiteSurvey",					Set_SiteSurvey_Proc},
	{"ForceTxBurst",				Set_ForceTxBurst_Proc},

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
	{"WlanLed",					Set_WlanLed_Proc},
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
	{"TdlsCapable",					Set_TdlsCapable_Proc},
	{"TdlsSetup",					Set_TdlsSetup_Proc},
	{"TdlsTearDown",				Set_TdlsTearDown_Proc},
	{"TdlsDiscoveryReq",				Set_TdlsDiscoveryReq_Proc},
	{"TdlsTPKLifeTime",				Set_TdlsTPKLifeTime_Proc},
#ifdef WFD_SUPPORT
	{"TdlsTunneledReq",				Set_TdlsTunneledReqProc},
#endif /* WFD_SUPPORT */
	{"TdlsAcceptWeakSec",			Set_TdlsAcceptWeakSecurityProc},
	{"TdlsChannelSwitch",			Set_TdlsChannelSwitch_Proc},
	{"TdlsChannelSwitchBW",			Set_TdlsChannelSwitchBW_Proc},
	{"TdlsChannelSwitchDisable",	Set_TdlsChannelSwitchDisable_Proc},
#ifdef TDLS_AUTOLINK_SUPPORT
	{"TdlsAutoLink",					Set_TdlsAutoLinkProc},
	{"TdlsRssiMeasurementPeriod",		Set_TdlsRssiMeasurementPeriodProc},
	{"TdlsAutoDiscoveryPeriod",			Set_TdlsAutoDiscoveryPeriodProc},
	{"TdlsAutoSetupRssiThreshold",		Set_TdlsAutoSetupRssiThresholdProc},
	{"TdlsDisabledPeriodByTeardown",		Set_TdlsDisabledPeriodByTeardownProc},
	{"TdlsAutoTeardownRssiThreshold",	Set_TdlsAutoTeardownRssiThresholdProc},
#endif // TDLS_AUTOLINK_SUPPORT //
#ifdef UAPSD_SUPPORT
	{"tdls",						TDLS_Ioctl},
#endif /* UAPSD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef XLINK_SUPPORT
	{"XlinkMode",					Set_XlinkMode_Proc},
#endif /* XLINK_SUPPORT */


	{"AutoReconnect", 				Set_AutoReconnect_Proc},
	{"AdhocN",						Set_AdhocN_Proc},
#ifdef AGS_SUPPORT
	{"Ags",						Show_AGS_Proc},
#endif /* AGS_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
	{"wow_enable",					Set_WOW_Enable},
	{"wow_gpio",					Set_WOW_GPIO},
	{"wow_delay",					Set_WOW_Delay},
	{"wow_hold",                    Set_WOW_Hold},
	{"wow_inband",                  Set_WOW_InBand},
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	{"VcoPeriod",					Set_VcoPeriod_Proc},
#ifdef CONFIG_SNIFFER_SUPPORT
	{"MonitorMode", 				Set_MonitorMode_Proc},
#endif /* CONFIG_SNIFFER_SUPPORT */

#ifdef WFD_SUPPORT
	{"WfdEnable",						Set_WfdEnable_Proc},
#ifdef RT_CFG80211_SUPPORT
	{"WfdInsertIe",						Set_WfdInsertIe_Proc},
#endif /* RT_CFG80211_SUPPORT */
	{"WfdDevType",						Set_WfdDeviceType_Proc},
	{"WfdCouple",						Set_WfdCouple_Proc},
	{"WfdSessionAvail", 				Set_WfdSessionAvailable_Proc},
	{"WfdCP",							Set_WfdCP_Proc},
	{"WfdRtspPort",						Set_WfdRtspPort_Proc},
	{"WfdMaxThroughput",				Set_WfdMaxThroughput_Proc},
	{"WfdLocalIp",						Set_WfdLocalIp_Proc},
	{"WfdPeerRtspPort",					Set_PeerRtspPort_Proc},
#endif /* WFD_SUPPORT */

#ifdef SINGLE_SKU
	{"ModuleTxpower",				Set_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */

#ifdef DOT11W_PMF_SUPPORT
        {"PMFMFPC",                                     Set_PMFMFPC_Proc},
        {"PMFMFPR",                                     Set_PMFMFPR_Proc},
        {"PMFSHA256",                                   Set_PMFSHA256_Proc},
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	{"fpga_on", set_fpga_mode},
	{"dataphy", set_data_phy_mode},
	{"databw", set_data_bw},
	{"dataldpc", set_data_ldpc},
	{"datamcs", set_data_mcs},
	{"datagi", set_data_gi},
	{"databasize", set_data_basize},
	{"txcnt", set_tx_kickcnt},
#endif /* CONFIG_FPGA_MODE */
	{"rf", set_rf},
#ifdef DYNAMIC_VGA_SUPPORT
	{"DyncVgaEnable", Set_DyncVgaEnable_Proc},
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef MICROWAVE_OVEN_SUPPORT
	{"MO_FalseCCATh",               Set_MO_FalseCCATh_Proc},
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef ED_MONITOR
	//let run-time turn on/off	
	{"ed_chk", set_ed_chk_proc},
	{"ed_current_rssi_th", set_ed_current_rssi_threhold_proc},	
	{"ed_debug", set_ed_debug_proc},
	{"ed_th", set_ed_threshold},
	{"ed_false_cca_th", set_ed_false_cca_threshold},
	{"ed_blk_cnt", set_ed_block_tx_thresh},
	{"ed_stat", show_ed_stat_proc},
#endif /* ED_MONITOR */
	{"ed_count_show", show_ed_cnt_for_channel_quality},

#ifdef CONFIG_SNIFFER_SUPPORT
	{"mc", set_monitor_channel},
#endif
	{"tssi_enable", set_tssi_enable},
#ifdef CONFIG_WIFI_TEST
	{"pbf_loopback", set_pbf_loopback},
	{"pbf_rx_drop", set_pbf_rx_drop},
#endif
	{"fw_debug", set_fw_debug},
#ifdef RT_CFG80211_SUPPORT
	{"DisableCfg2040Scan",				Set_DisableCfg2040Scan_Proc},
#endif	
	{NULL,}
};


INT RTMPSTAPrivIoctlSet(
	IN RTMP_ADAPTER *pAd, 
	IN PSTRING SetProcName,
	IN PSTRING ProcArg)
{
	int ret = 0;
	
	for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++)            
	{                                                                                                                           
	    if (strcmp(SetProcName, PRTMP_PRIVATE_SET_PROC->name) == 0)                                                               
	    {						                                                                                                
	        if(!PRTMP_PRIVATE_SET_PROC->set_proc(pAd, ProcArg))                                                              
	        {	/*FALSE:Set private failed then return Invalid argument */
			    return NDIS_STATUS_FAILURE;                                                                                               
	        }                                                                                                                   
		    break;	/*Exit for loop. */
	    }
	}
	
	if(PRTMP_PRIVATE_SET_PROC->name == NULL)                                                                                    
	{  /*Not found argument */
	 	DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_setparam:: (iwpriv) Not Support Set Command [%s=%s]\n", SetProcName, ProcArg));
	    return -EINVAL;                                                                                                       
	   
	}         

	return ret;
}


/* 
    ==========================================================================
    Description:
        Set SSID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_SSID_Proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
    NDIS_802_11_SSID Ssid, *pSsid=NULL;
    BOOLEAN StateMachineTouched = FALSE;
    int success = TRUE;


	/*
		Set the AutoReconnectSsid to prevent it reconnect to old SSID
		Since calling this indicate user don't want to connect to that SSID anymore.
	*/
	pAd->MlmeAux.AutoReconnectSsidLen= 32;
	NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

	if( strlen(arg) <= MAX_LEN_OF_SSID)
	{
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
		if (strlen(arg) != 0)
		{
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		}
		else   /*ANY ssid */
		{    
			Ssid.SsidLength = 0; 
			memcpy(Ssid.Ssid, "", 0);
			pAd->StaCfg.BssType = BSS_INFRA;	
			pAd->StaCfg.wdev.AuthMode = Ndis802_11AuthModeOpen;
			pAd->StaCfg.wdev.WepStatus  = Ndis802_11EncryptionDisabled;		    
		}	 
		pSsid = &Ssid;

		if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		{
			RTMP_MLME_RESET_STATE_MACHINE(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		}

		if ((pAd->StaCfg.WpaPassPhraseLen >= 8) &&
			(pAd->StaCfg.WpaPassPhraseLen <= 64))
		{
			UCHAR keyMaterial[40];

			RTMPZeroMemory(pAd->StaCfg.PMK, 32);
			if (pAd->StaCfg.WpaPassPhraseLen == 64)
			{
				AtoH((PSTRING) pAd->StaCfg.WpaPassPhrase, pAd->StaCfg.PMK, 32);
			}
			else
			{
				RtmpPasswordHash((PSTRING) pAd->StaCfg.WpaPassPhrase, Ssid.Ssid, Ssid.SsidLength, keyMaterial);
				NdisMoveMemory(pAd->StaCfg.PMK, keyMaterial, 32);		
			}
		}

		/* Record the desired user settings to MlmeAux */
		NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid.Ssid, Ssid.SsidLength);
		pAd->MlmeAux.SsidLen = (UCHAR)Ssid.SsidLength;

		NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, Ssid.Ssid, Ssid.SsidLength);
		pAd->MlmeAux.AutoReconnectSsidLen = (UCHAR)Ssid.SsidLength;

		pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
		pAd->StaCfg.bSkipAutoScanConn = FALSE;
		pAd->bConfigChanged = TRUE;
		pAd->StaCfg.bNotFirstScan = FALSE;     

		MlmeEnqueue(pAd, 
						MLME_CNTL_STATE_MACHINE, 
						OID_802_11_SSID,
						sizeof(NDIS_802_11_SSID),
						(VOID *)pSsid, 0);

		StateMachineTouched = TRUE;
		if (Ssid.SsidLength == MAX_LEN_OF_SSID)
			hex_dump("Set_SSID_Proc::Ssid", Ssid.Ssid, Ssid.SsidLength);
		else
			DBGPRINT(RT_DEBUG_TRACE, ("Set_SSID_Proc::(Len=%d,Ssid=%s)\n", Ssid.SsidLength, Ssid.Ssid));
	}
	else
		success = FALSE;

	if (StateMachineTouched) /* Upper layer sent a MLME-related operations */
		RTMP_MLME_HANDLER(pAd);

    return success;
}


/* 
    ==========================================================================
    Description:
        Set WmmCapable Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bWmmCapable;

	bWmmCapable = simple_strtol(arg, 0, 10);

	if ((bWmmCapable == 1)
		)
		pAd->CommonCfg.bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		pAd->CommonCfg.bWmmCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_WmmCapable_Proc::(bWmmCapable=%d)\n", 
		pAd->CommonCfg.bWmmCapable));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Network Type(Infrastructure/Adhoc mode)
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_NetworkType_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg)
{
    UINT32	Value = 0;

    if (strcmp(arg, "Adhoc") == 0)
	{
		if (pAd->StaCfg.BssType != BSS_ADHOC)
		{				    
			/* Config has changed */
			pAd->bConfigChanged = TRUE;
            if (MONITOR_ON(pAd))
            {
                RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL);
                RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
				Value &= (~0x80);
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
                OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
                pAd->StaCfg.bAutoReconnect = TRUE;
                LinkDown(pAd, FALSE);
            }
			if (INFRA_ON(pAd))
			{
				/*BOOLEAN Cancelled; */
				/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
				/* Since calling this indicate user don't want to connect to that SSID anymore. */
				pAd->MlmeAux.AutoReconnectSsidLen= 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);		
				
				LinkDown(pAd, FALSE);

				DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event BB!\n"));
			}
#ifdef DOT11_N_SUPPORT
			SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
		}			
		pAd->StaCfg.BssType = BSS_ADHOC;
		RTMP_OS_NETDEV_SET_TYPE(pAd->net_dev, pAd->StaCfg.OriDevType);
		
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(AD-HOC)\n"));
	}
    else if (strcmp(arg, "Infra") == 0)
	{
		if (pAd->StaCfg.BssType != BSS_INFRA)
		{			    
			/* Config has changed */
			pAd->bConfigChanged = TRUE;
            if (MONITOR_ON(pAd))
            {
                RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL);
                RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
				Value &= (~0x80);
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
                OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
                pAd->StaCfg.bAutoReconnect = TRUE;
                LinkDown(pAd, FALSE);
            }
			if (ADHOC_ON(pAd))
			{
				/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
				/* Since calling this indicate user don't want to connect to that SSID anymore. */
				pAd->MlmeAux.AutoReconnectSsidLen= 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);			
			
				LinkDown(pAd, FALSE);
			}
#ifdef DOT11_N_SUPPORT
			SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
		}			
		pAd->StaCfg.BssType = BSS_INFRA;
		RTMP_OS_NETDEV_SET_TYPE(pAd->net_dev, pAd->StaCfg.OriDevType);
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(INFRA)\n"));            
	}
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	/*
		Monitor2 is for 3593 11n wireshark sniffer tool.
		The name, Monitor2, follows the command format in RT2883.
	*/
    else if ((strcmp(arg, "Monitor") == 0) || (strcmp(arg, "Monitor2") == 0))
#else
    else if (strcmp(arg, "Monitor") == 0)
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	{
		BCN_TIME_CFG_STRUC csr;
		UCHAR rf_channel;
		UINT8 rf_bw, ext_ch;

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
		if (strcmp(arg, "Monitor2") == 0)
			pAd->StaCfg.BssMonitorFlag |= MONITOR_FLAG_11N_SNIFFER;
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		/* disable all periodic state machine */
		pAd->StaCfg.bAutoReconnect = FALSE;
		/* reset all mlme state machine */
		RTMP_MLME_RESET_STATE_MACHINE(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("fOP_STATUS_MEDIA_STATE_CONNECTED \n"));
		if (pAd->CommonCfg.CentralChannel == 0)
		{
#ifdef DOT11_N_SUPPORT
			if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A | WMODE_AN))
				pAd->CommonCfg.CentralChannel = 36;
			else
#endif /* DOT11_N_SUPPORT */
				pAd->CommonCfg.CentralChannel = 6;
		}
#ifdef DOT11_N_SUPPORT
		else
			N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */


		/* same procedure with window driver */
#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
			pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40 &&
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
			/* 40MHz ,control channel at lower */
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
			ext_ch = EXTCHA_ABOVE;
			rf_bw = BW_40;
			rf_channel = pAd->CommonCfg.CentralChannel;
		}
		else if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
	                 pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40 &&
		              pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)
		{
			/* 40MHz ,control channel at upper */
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
			ext_ch = EXTCHA_BELOW;
			rf_bw = BW_40;
			rf_channel = pAd->CommonCfg.CentralChannel;
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			/* 20MHz */
			rf_bw = BW_20;
			ext_ch = EXTCHA_NONE;
			rf_channel = pAd->CommonCfg.Channel;
		}

		AsicSetChannel(pAd, rf_channel, rf_bw, ext_ch, FALSE);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():BW_%s, CtrlChannel(%d), CentralChannel(%d) \n", 
					__FUNCTION__, (rf_bw == BW_40 ? "40" : "20"),
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.CentralChannel));


		/* Enable Rx with promiscuous reception */
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, 0x3);
		/* ASIC supporsts sniffer function with replacing RSSI with timestamp. */
		/*RTMP_IO_READ32(pAdapter, MAC_SYS_CTRL, &Value); */
		/*Value |= (0x80); */
		/*RTMP_IO_WRITE32(pAdapter, MAC_SYS_CTRL, Value); */

		/* disable sync */
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
		csr.field.bBeaconGen = 0;
		csr.field.bTBTTEnable = 0;
		csr.field.TsfSyncMode = 0;
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	    
		pAd->StaCfg.BssType = BSS_MONITOR;
		RTMP_OS_NETDEV_SET_TYPE_MONITOR(pAd->net_dev);
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(MONITOR)\n"));
	}

	/* Set Rx Filter after eaxctly know what mode currently we work on */
	AsicSetRxFilter(pAd);

    /* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key */
    pAd->StaCfg.WpaState = SS_NOTUSE;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_NetworkType_Proc::(NetworkType=%d)\n", pAd->StaCfg.BssType));

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Authentication mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_AuthMode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

    if (rtstrcasecmp(arg, "WEPAUTO") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeAutoSwitch;
    else if (rtstrcasecmp(arg, "OPEN") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeOpen;
    else if (rtstrcasecmp(arg, "SHARED") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeShared;
    else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
    else if (rtstrcasecmp(arg, "WPANONE") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWPANone;
    else if ((rtstrcasecmp(arg, "WPA2PSK") == TRUE) || (rtstrcasecmp(arg, "WPAPSKWPA2PSK") == TRUE))
        wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;    
#ifdef WPA_SUPPLICANT_SUPPORT    
    else if (rtstrcasecmp(arg, "WPA") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWPA;    
    else if (rtstrcasecmp(arg, "WPA2") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWPA2;
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WAPI_SUPPORT    
    else if (rtstrcasecmp(arg, "WAICERT") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWAICERT;    
    else if (rtstrcasecmp(arg, "WAIPSK") == TRUE)
        wdev->AuthMode = Ndis802_11AuthModeWAIPSK;
#endif /* WAPI_SUPPORT */
    else
        return FALSE;  

    wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_AuthMode_Proc::(AuthMode=%d)\n", wdev->AuthMode));

    return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Encryption Type
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_EncrypType_Proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

    if (rtstrcasecmp(arg, "NONE") == TRUE)
    {
        if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
            return TRUE;    /* do nothing */
            
        wdev->WepStatus     = Ndis802_11WEPDisabled;
        pAd->StaCfg.PairCipher    = Ndis802_11WEPDisabled;
	    pAd->StaCfg.GroupCipher   = Ndis802_11WEPDisabled;
    }
    else if (rtstrcasecmp(arg, "WEP") == TRUE)
    {
        if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
            return TRUE;    /* do nothing */
            
        wdev->WepStatus     = Ndis802_11WEPEnabled;
        pAd->StaCfg.PairCipher    = Ndis802_11WEPEnabled;
	    pAd->StaCfg.GroupCipher   = Ndis802_11WEPEnabled;		
    }
    else if (rtstrcasecmp(arg, "TKIP") == TRUE)
    {
        if (wdev->AuthMode < Ndis802_11AuthModeWPA)
            return TRUE;    /* do nothing */
            
        wdev->WepStatus     = Ndis802_11TKIPEnable;
        pAd->StaCfg.PairCipher    = Ndis802_11TKIPEnable;
	    pAd->StaCfg.GroupCipher   = Ndis802_11TKIPEnable;
    }
    else if (rtstrcasecmp(arg, "AES") == TRUE)
    {
        if (wdev->AuthMode < Ndis802_11AuthModeWPA)
            return TRUE;    /* do nothing */
            
        wdev->WepStatus     = Ndis802_11AESEnable;
        pAd->StaCfg.PairCipher    = Ndis802_11AESEnable;
	    pAd->StaCfg.GroupCipher   = Ndis802_11AESEnable;
    }
#ifdef WAPI_SUPPORT
	else if (rtstrcasecmp(arg, "SMS4") == TRUE)
    {
        if ((wdev->AuthMode != Ndis802_11AuthModeWAICERT) &&
			(wdev->AuthMode != Ndis802_11AuthModeWAIPSK))
            return TRUE;    /* do nothing */
            
        wdev->WepStatus     = Ndis802_11EncryptionSMS4Enabled;
        pAd->StaCfg.PairCipher    = Ndis802_11EncryptionSMS4Enabled;
	    pAd->StaCfg.GroupCipher   = Ndis802_11EncryptionSMS4Enabled;
    }
#endif /* WAPI_SUPPORT */
    else
        return FALSE;

	if (pAd->StaCfg.BssType == BSS_ADHOC)
	{
		/* Build all corresponding channel information */
		RTMPSetPhyMode(pAd, pAd->CommonCfg.cfg_wmode);
	}

    DBGPRINT(RT_DEBUG_TRACE, ("Set_EncrypType_Proc::(EncrypType=%d)\n", wdev->WepStatus));

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Default Key ID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_DefaultKeyID_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    ULONG                               KeyIdx;

    KeyIdx = simple_strtol(arg, 0, 10);
    if((KeyIdx >= 1 ) && (KeyIdx <= 4))
        pAdapter->StaCfg.wdev.DefaultKeyId = (UCHAR) (KeyIdx - 1 );
    else
        return FALSE;  /*Invalid argument */

    DBGPRINT(RT_DEBUG_TRACE, ("Set_DefaultKeyID_Proc::(DefaultKeyID=%d)\n", pAdapter->StaCfg.wdev.DefaultKeyId));

    return TRUE;
}

INT Set_Wep_Key_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PSTRING         Key,
    IN  INT             KeyLen,
    IN  INT             KeyId)
{
    int    i;
    UCHAR  CipherAlg = CIPHER_WEP64;
	struct wifi_dev *wdev = &pAdapter->StaCfg.wdev;
	
    if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    /* do nothing */

    if ((KeyId < 0) || (KeyId > 3))	
    {
		DBGPRINT(RT_DEBUG_TRACE, ("Set_Wep_Key_Proc::Invalid KeyId (=%d)\n", KeyId));
		return FALSE;
    }	

    switch (KeyLen)
    {
        case 5: /* wep 40 Ascii type */
            pAdapter->SharedKey[BSS0][KeyId].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][KeyId].Key, Key, KeyLen);
            CipherAlg = CIPHER_WEP64;
            break;

        case 10: /* wep 40 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(Key+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][KeyId].KeyLen = KeyLen / 2 ;
            AtoH(Key, pAdapter->SharedKey[BSS0][KeyId].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            break;

        case 13: /* wep 104 Ascii type */
            pAdapter->SharedKey[BSS0][KeyId].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][KeyId].Key, Key, KeyLen);
            CipherAlg = CIPHER_WEP128;
            break;

        case 26: /* wep 104 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(Key+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][KeyId].KeyLen = KeyLen / 2 ;
            AtoH(Key, pAdapter->SharedKey[BSS0][KeyId].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            break;
			
        default: /* Invalid argument */
            DBGPRINT(RT_DEBUG_ERROR, ("Set_Wep_Key_Proc::Invalid argument (=%s)\n", Key));
            return FALSE;
    }

    pAdapter->SharedKey[BSS0][KeyId].CipherAlg = CipherAlg;

    /* Set keys (into ASIC) */
    if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
        ;   /* not support */
    else    /* Old WEP stuff */
    {
        AsicAddSharedKeyEntry(pAdapter,
                              0,
                              0,
                              &pAdapter->SharedKey[BSS0][KeyId]);
    }

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set WEP KEY1
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Key1_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    /* do nothing */
    
    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: /*wep 40 Ascii type */
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][0].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Ascii"));       
            break;
        case 10: /*wep 40 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][0].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Hex"));     
            break;
        case 13: /*wep 104 Ascii type */
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][0].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Ascii"));       
            break;
        case 26: /*wep 104 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][0].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Hex"));     
            break;
        default: /*Invalid argument */
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::Invalid argument (=%s)\n", arg));       
            return FALSE;
    }
    
    pAdapter->SharedKey[BSS0][0].CipherAlg = CipherAlg;

    /* Set keys (into ASIC) */
    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        ;   /* not support */
    else    /* Old WEP stuff */
    {
        AsicAddSharedKeyEntry(pAdapter, 
                              0, 
                              0, 
                              &pAdapter->SharedKey[BSS0][0]);
    }
    
    return TRUE;
}
/* 
    ==========================================================================

    Description:
        Set WEP KEY2
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Key2_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    /* do nothing */
    
    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: /*wep 40 Ascii type */
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][1].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 10: /*wep 40 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][1].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Hex"));
            break;
        case 13: /*wep 104 Ascii type */
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][1].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 26: /*wep 104 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][1].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Hex"));
            break;
        default: /*Invalid argument */
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }
    pAdapter->SharedKey[BSS0][1].CipherAlg = CipherAlg;

    /* Set keys (into ASIC) */
    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        ;   /* not support */
    else    /* Old WEP stuff */
    {
        AsicAddSharedKeyEntry(pAdapter, 
                              0, 
                              1, 
                              &pAdapter->SharedKey[BSS0][1]);
    }        
    
    return TRUE;
}
/* 
    ==========================================================================
    Description:
        Set WEP KEY3
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Key3_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    /* do nothing */
    
    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: /*wep 40 Ascii type */
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][2].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Ascii)\n", arg));
            break;
        case 10: /*wep 40 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][2].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Hex)\n", arg));
            break;
        case 13: /*wep 104 Ascii type */
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][2].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Ascii)\n", arg));
            break;
        case 26: /*wep 104 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][2].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Hex)\n", arg));
            break;
        default: /*Invalid argument */
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }
    pAdapter->SharedKey[BSS0][2].CipherAlg = CipherAlg;
    
    /* Set keys (into ASIC) */
    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        ;   /* not support */
    else    /* Old WEP stuff */
    {
        AsicAddSharedKeyEntry(pAdapter, 
                              0, 
                              2, 
                              &pAdapter->SharedKey[BSS0][2]);
    }
    
    return TRUE;
}
/* 
    ==========================================================================
    Description:
        Set WEP KEY4
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Key4_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    /* do nothing */
    
    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: /*wep 40 Ascii type */
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][3].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 10: /*wep 40 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][3].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Hex"));
            break;
        case 13: /*wep 104 Ascii type */
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][3].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 26: /*wep 104 Hex type */
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  /*Not Hex value; */
            }
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][3].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Hex"));
            break;
        default: /*Invalid argument */
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    } 
    pAdapter->SharedKey[BSS0][3].CipherAlg = CipherAlg;
    
    /* Set keys (into ASIC) */
    if (pAdapter->StaCfg.wdev.AuthMode >= Ndis802_11AuthModeWPA)
        ;   /* not support */
    else    /* Old WEP stuff */
    {
        AsicAddSharedKeyEntry(pAdapter, 
                              0, 
                              3, 
                              &pAdapter->SharedKey[BSS0][3]);
    }
    
    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set WPA PSK key
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_WPAPSK_Proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
    int status;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;


    if ((wdev->AuthMode != Ndis802_11AuthModeWPAPSK) && 
        (wdev->AuthMode != Ndis802_11AuthModeWPA2PSK) &&
	    (wdev->AuthMode != Ndis802_11AuthModeWPANone)
#ifdef WAPI_SUPPORT
		 && (wdev->AuthMode != Ndis802_11AuthModeWAICERT)
		 && (wdev->AuthMode != Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
		)
        return TRUE;    /* do nothing */
        
    DBGPRINT(RT_DEBUG_TRACE, ("Set_WPAPSK_Proc::(WPAPSK=%s)\n", arg));

	status = RT_CfgSetWPAPSKKey(pAd, arg, strlen(arg), pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, pAd->StaCfg.PMK);
	if (status == FALSE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WPAPSK_Proc(): Set key failed!\n"));
		return FALSE;
	}
	NdisZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
    NdisMoveMemory(pAd->StaCfg.WpaPassPhrase, arg, strlen(arg));
    pAd->StaCfg.WpaPassPhraseLen = (UINT)strlen(arg);

#ifdef WSC_STA_SUPPORT
    NdisZeroMemory(pAd->StaCfg.WscControl.WpaPsk, 64);
    pAd->StaCfg.WscControl.WpaPskLen = 0;    
    NdisMoveMemory(pAd->StaCfg.WscControl.WpaPsk, arg, strlen(arg));
    pAd->StaCfg.WscControl.WpaPskLen = (INT)strlen(arg);
#endif /* WSC_STA_SUPPORT */

#ifdef WAPI_SUPPORT
	NdisZeroMemory(pAd->StaCfg.WAPIPassPhrase, 64);
    pAd->StaCfg.WAPIPassPhraseLen = 0;    
    NdisMoveMemory(pAd->StaCfg.WAPIPassPhrase, arg, strlen(arg));
    pAd->StaCfg.WAPIPassPhraseLen = (UINT)strlen(arg);
#endif /* WAPI_SUPPORT */

    if(pAd->StaCfg.BssType == BSS_ADHOC &&
       wdev->AuthMode == Ndis802_11AuthModeWPANone)
    {
        pAd->StaCfg.WpaState = SS_NOTUSE;     
    }
    else
    {
        /* Start STA supplicant state machine */
        pAd->StaCfg.WpaState = SS_START;
    }    

    return TRUE;
}



/* 
    ==========================================================================
    Description:
        Set Power Saving mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_PSMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    if (pAdapter->StaCfg.BssType == BSS_INFRA)
    {
        if ((strcmp(arg, "Max_PSP") == 0) || 
			(strcmp(arg, "max_psp") == 0) ||
			(strcmp(arg, "MAX_PSP") == 0))
        {
            /* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
            /* to exclude certain situations. */
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            pAdapter->StaCfg.DefaultListenCount = 5;
        
        }
        else if ((strcmp(arg, "Fast_PSP") == 0) || 
				 (strcmp(arg, "fast_psp") == 0) ||
                 (strcmp(arg, "FAST_PSP") == 0))
        {
            /* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
            /* to exclude certain situations. */
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeFast_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
            pAdapter->StaCfg.DefaultListenCount = 3;
        }
        else if ((strcmp(arg, "Legacy_PSP") == 0) || 
                 (strcmp(arg, "legacy_psp") == 0) || 
                 (strcmp(arg, "LEGACY_PSP") == 0))
        {
            /* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
            /* to exclude certain situations. */
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
            pAdapter->StaCfg.DefaultListenCount = 3;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		pAdapter->StaCfg.DefaultListenCount = 1;
#endif // defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) //
        }
        else
        {
            /*Default Ndis802_11PowerModeCAM */
            /* clear PSM bit immediately */
            RTMP_SET_PSM_BIT(pAdapter, PWR_ACTIVE);
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;

        }

        DBGPRINT(RT_DEBUG_TRACE, ("Set_PSMode_Proc::(PSMode=%ld)\n", pAdapter->StaCfg.WindowsPowerMode));
    }
    else
        return FALSE;

        
    return TRUE;
}

#ifdef WPA_SUPPLICANT_SUPPORT
/* 
    ==========================================================================
    Description:
        Set WpaSupport flag.
    Value:
        0: Driver ignore wpa_supplicant.
        1: wpa_supplicant initiates scanning and AP selection.
        2: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Wpa_Support(
    IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{

    if ( simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
    else if ( simple_strtol(arg, 0, 10) == 1)
        pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
    else if ( simple_strtol(arg, 0, 10) == 2)
        pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE_WITH_WEB_UI;
    else
        pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_Wpa_Support::(WpaSupplicantUP=%d)\n",
				pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP));
    
    return TRUE;    
}
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef WSC_STA_SUPPORT
#define WSC_GET_CONF_MODE_EAP	1
#define WSC_GET_CONF_MODE_UPNP	2
INT	 Set_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	pWscControl = &pAd->StaCfg.WscControl;

    if ( simple_strtol(arg, 0, 10) == 0)
        pWscControl->WscConfMode = WSC_DISABLE;
    else if ( simple_strtol(arg, 0, 10) == 1)
        pWscControl->WscConfMode = WSC_ENROLLEE;
    else if ( simple_strtol(arg, 0, 10) == 2)
        pWscControl->WscConfMode = WSC_REGISTRAR;
    else
        pWscControl->WscConfMode = WSC_DISABLE;

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra0) Set_WscConfMode_Proc::(WscConfMode(0,1,2)=%d)\n", pWscControl->WscConfMode));

#ifdef IWSC_SUPPORT
#ifdef IWSC_TEST_SUPPORT
	if (pAd->StaCfg.BssType == BSS_ADHOC)
	{
		PIWSC_INFO pIWscInfo = &pAd->StaCfg.IWscInfo;
		pIWscInfo->IWscConfMode = WSC_DISABLE;
		if ( simple_strtol(arg, 0, 10) == 1)
	        pIWscInfo->IWscConfMode = WSC_ENROLLEE;
		else if ( simple_strtol(arg, 0, 10) == 2)
	        pIWscInfo->IWscConfMode = WSC_REGISTRAR;
	    else
			return FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra0) Set_WscConfMode_Proc::(IWscConfMode=%d)\n", pIWscInfo->IWscConfMode));
	}
#endif /* IWSC_TEST_SUPPORT */
#endif /* IWSC_SUPPORT */

	return TRUE;
}

INT	Set_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR     IsAPConfigured = 1;

	IsAPConfigured = (UCHAR)simple_strtol(arg, 0, 10);

	if ((IsAPConfigured  > 0) && (IsAPConfigured  <= 2))
        pAd->StaCfg.WscControl.WscConfStatus = IsAPConfigured;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscConfStatus_Proc:: Set failed!!(WscConfStatus=%s), WscConfStatus is 1 or 2 \n", arg));
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscConfStatus_Proc:: WscConfStatus is not changed (%d) \n", pAd->StaCfg.WscControl.WscConfStatus));
		return FALSE;  /*Invalid argument */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WscConfStatus_Proc::(WscConfStatus=%d)\n", pAd->StaCfg.WscControl.WscConfStatus));

	return TRUE;
}

INT Set_WscSsid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	ULONG		ApIdx = 0;
	
	pWscControl = &pAd->StaCfg.WscControl;

	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if( (strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
    {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);

		//add SSID into pAd->MlmeAux.Ssid
        NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
        NdisMoveMemory(pAd->MlmeAux.Ssid, arg, strlen(arg));
        pAd->MlmeAux.SsidLen = strlen(arg);

		NdisZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		ApIdx = WscSearchWpsApBySSID(pAd,
									 pWscControl->WscSsid.Ssid, 
									 pWscControl->WscSsid.SsidLength, 
									 WSC_PIN_MODE);
		if (ApIdx != BSS_NOT_FOUND)
		{
			NdisMoveMemory(pWscControl->WscBssid, pAd->ScanTab.BssEntry[ApIdx].Bssid,MAC_ADDR_LEN);
			pAd->MlmeAux.Channel = pAd->ScanTab.BssEntry[ApIdx].Channel;
		}
		
		hex_dump("Set_WscSsid_Proc:: WscBssid", pWscControl->WscBssid, MAC_ADDR_LEN);		
		
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscSsid_Proc:: (Select SsidLen=%d,Ssid=%s)\n", 
				pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid));
	}
	else
		return FALSE;	/*Invalid argument */

	return TRUE;	

}


INT Set_WscBssid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR					MacAddr[MAC_ADDR_LEN];

	if (rtstrmactohex(arg, (PSTRING) &MacAddr[0]) == FALSE)
		return FALSE;

	RTMPZeroMemory(pAd->StaCfg.WscControl.WscBssid, MAC_ADDR_LEN);
	RTMPMoveMemory(pAd->StaCfg.WscControl.WscBssid, MacAddr, MAC_ADDR_LEN);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WscBssid_Proc:: %02x:%02x:%02x:%02x:%02x:%02x\n",
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]));
	 
	return TRUE;
}

INT	Set_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT	        WscMode;
	PWSC_CTRL	pWscControl;
	pWscControl = &pAd->StaCfg.WscControl;
	
	WscMode = (INT)simple_strtol(arg, 0, 10);
    
	if ((WscMode == WSC_SMPBC_MODE) && 
		(pAd->StaCfg.BssType == BSS_INFRA))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscMode_Proc:: Set failed!!(Set_WscMode_Proc=%s)\n", arg));
		return FALSE;  /*Invalid argument */
	}
	
    if ((WscMode == WSC_PIN_MODE ) || 
		(WscMode == WSC_PBC_MODE) ||
		(WscMode == WSC_SMPBC_MODE))
    {
    	/* save wsc mode */
        pWscControl->WscMode = WscMode;
    }
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscMode_Proc:: Set failed!!(Set_WscMode_Proc=%s)\n", arg));
		return FALSE;  /*Invalid argument */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WscMode_Proc::(WscMode=%d)\n",  pWscControl->WscMode));

	return TRUE;
}

INT	Set_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	BOOLEAN     validatePin;
	UINT        PinCode = 0;

	pWscControl = &pAd->StaCfg.WscControl;

	PinCode = simple_strtol(arg, 0, 10); /* When PinCode is 03571361, return value is 3571361. */
	if ( strlen(arg) == 4 )
		validatePin = TRUE;
	else
		validatePin = ValidateChecksum(PinCode);
	if (validatePin)
    {
    	if (pWscControl->WscRejectSamePinFromEnrollee && 
             (PinCode == pWscControl->WscLastPinFromEnrollee))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("PIN authentication or communication error occurs!!\n"
                     "Registrar does NOT accept the same PIN again!(PIN:%08u)\n", PinCode));
			return FALSE;
		}
		else
		{
			pWscControl->WscRejectSamePinFromEnrollee = FALSE;
			pWscControl->WscPinCode = PinCode;
			pWscControl->WscLastPinFromEnrollee = pWscControl->WscPinCode;
			if ( strlen(arg) == 4)
			{
				pWscControl->WscPinCodeLen = 4;
			}
			else
			{	
				pWscControl->WscPinCodeLen = 8;
			}
			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
			
#ifdef IWSC_SUPPORT
			if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
				(pWscControl->bWscTrigger == TRUE))
			{
				MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_PEER_PIN, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
			}
#endif /* IWSC_SUPPORT */
		}        	    
	}
	else
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscPinCode_Proc:: Checksum is invalid\n"));
        return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra0) Set_WscPinCode_Proc::(PinCode=%d)\n", pWscControl->WscPinCode));

	return TRUE;
}

INT	Set_WscUUIDE_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL           pWscControl;
	int i,UUIDLen;
	UCHAR				tmp_Uuid_Str[UUID_LEN_STR];
	UCHAR				Wsc_Uuid_E[UUID_LEN_HEX];
	UCHAR				uuidTmpStr[UUID_LEN_STR+2];
	WSC_UUID_T uuid_t;
	
	UUIDLen = strlen(arg);
	if ( UUIDLen != 32)
	{
		DBGPRINT(RT_DEBUG_ERROR,("Length of UUID key is wrong length=%d\n",UUIDLen));
		return FALSE;
	}
	NdisMoveMemory(tmp_Uuid_Str, arg, UUIDLen);
    	pWscControl = &pAd->StaCfg.WscControl;

	for(i=0; i < UUIDLen; i++)
	{
		if( !isxdigit(tmp_Uuid_Str[i]) )
		{
			DBGPRINT(RT_DEBUG_ERROR,("Input %d not Hex Value\n",i));
			return FALSE;  //Not Hex value;
		}
	}
	AtoH(tmp_Uuid_Str, Wsc_Uuid_E, UUID_LEN_HEX);
	NdisMoveMemory(&uuid_t, Wsc_Uuid_E, UUID_LEN_HEX);
	NdisZeroMemory(uuidTmpStr, sizeof(uuidTmpStr));
	sprintf(uuidTmpStr, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
			(unsigned int)uuid_t.timeLow, uuid_t.timeMid, uuid_t.timeHi_Version, uuid_t.clockSeqHi_Var, uuid_t.clockSeqLow, 
			uuid_t.node[0], uuid_t.node[1], uuid_t.node[2], uuid_t.node[3], uuid_t.node[4], uuid_t.node[5]);

	if (strlen(uuidTmpStr) > UUID_LEN_STR)
		DBGPRINT(RT_DEBUG_ERROR, ("ERROR:UUID String size too large!\n"));

	strncpy((PSTRING)pWscControl->Wsc_Uuid_Str, uuidTmpStr, UUID_LEN_STR);
	NdisMoveMemory(&pWscControl->Wsc_Uuid_E[0], Wsc_Uuid_E, UUID_LEN_HEX);

	DBGPRINT(RT_DEBUG_TRACE, ("The UUID Hex string is:"));
	for (i=0; i< 16; i++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%02x", (pWscControl->Wsc_Uuid_E[i] & 0xff)));
	}
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("The UUID ASCII string is:%s!\n", pWscControl->Wsc_Uuid_Str));
	return 0;
}

INT	Set_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL           pWscControl;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
    INT	                idx;
    BOOLEAN             StateMachineTouched = FALSE;
    
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
    
#ifdef IWSC_SUPPORT
	if (pAd->StaCfg.BssType == BSS_ADHOC)
	{
		if (pAd->StaCfg.WscControl.bWscTrigger)
			IWSC_Stop(pAd, TRUE);
		else
			IWSC_Stop(pAd, FALSE);

		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
			(pAd->StaCfg.IWscInfo.IpMethod == IWSC_IPV4_ASSIGNMENT) &&
			(pAd->StaCfg.IWscInfo.RegDepth != 0) &&
			(pAd->StaCfg.IWscInfo.AvaSubMaskListCount == 0))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Empty Available IPv4 Submask List. Cannot be Registrar\n"));
			pAd->StaCfg.WscControl.WscStatus = STATUS_WSC_EMPTY_IPV4_SUBMASK_LIST;
			return TRUE;
		}
		MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_MLME_START, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
		return TRUE;
	}
#endif /* IWSC_SUPPORT */
    
    pWscControl = &pAd->StaCfg.WscControl;
    pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

    if (pWscControl->WscConfMode == WSC_DISABLE)
    {
        pWscControl->bWscTrigger = FALSE;
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGetConf_Proc: WPS is disabled.\n"));
		return FALSE;
    }

	WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
			FALSE,
#endif /* CONFIG_AP_SUPPORT */
			pWscControl);    
    
    /* trigger wsc re-generate public key */
    pWscControl->RegData.ReComputePke = 1;

	/* Change to init state before sending a disassociation frame */
	pAd->StaCfg.WscControl.WscState = WSC_STATE_INIT;

	/* 0. Send a disassoication frame */
	if (INFRA_ON(pAd))
	{
		MLME_DISASSOC_REQ_STRUCT	DisassocReq;

		if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		{
			RTMP_MLME_RESET_STATE_MACHINE(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		}
										
		/* Set to immediately send the media disconnect event */
		pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

		DBGPRINT(RT_DEBUG_TRACE, ("disassociate with current AP before starting WPS\n"));
		DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
		MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
					sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, 0);

		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
		RTMP_MLME_HANDLER(pAd);

		/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
		pAd->MlmeAux.AutoReconnectSsidLen= 32;
		NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

		pWscControl->bWscTrigger = FALSE;	/* check to disable */
		OS_WAIT(500);  /* leave enough time for this DISASSOC frame */
	}
	else if (ADHOC_ON(pAd))
	{
		USHORT	TmpWscMode;
		/*
			Set the AutoReconnectSsid to prevent it reconnect to old SSID
			Since calling this indicate user don't want to connect to that SSID anymore.
		*/
		pAd->MlmeAux.AutoReconnectSsidLen= 32;
		NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);			
		if (pWscControl->WscMode == 1)
			TmpWscMode = DEV_PASS_ID_PIN;
		else
			TmpWscMode = DEV_PASS_ID_PBC;
		AsicDisableSync(pAd);
		WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, TRUE, TmpWscMode, pWscControl->WscConfigMethods, BSS0, NULL, 0, STA_MODE);
		if (pWscControl->WscConfMode == WSC_REGISTRAR)
		{
			WscBuildProbeRespIE(pAd,
				WSC_MSGTYPE_REGISTRAR,
				pWscControl->WscConfStatus,
				TRUE,
				TmpWscMode,
				pWscControl->WscConfigMethods,
				BSS0,
				NULL,
				0,
				STA_MODE);
			MakeIbssBeacon(pAd);
			AsicEnableIbssSync(pAd);
		}
		else
		{
			WscBuildProbeRespIE(pAd,
				WSC_MSGTYPE_ENROLLEE_INFO_ONLY,
				pWscControl->WscConfStatus,
				TRUE,
				TmpWscMode,
				pWscControl->WscConfigMethods,
				BSS0,
				NULL,
				0,
				STA_MODE);
			LinkDown(pAd, FALSE);
		}
		
	}

    pWscControl->bWscTrigger = TRUE;
    pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;

	/* */
	/* Action : PIN, PBC */
	/* */
	if (pWscControl->WscMode == 1)
	{
		INT		WaitCnt = 0;
		/* PIN  - default */

		/* 2. Enqueue BSSID/SSID connection command */
		if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		{
			RTMP_MLME_RESET_STATE_MACHINE(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		}

		WscInitRegistrarPair(pAd, pWscControl, BSS0);

		/*
			We need to make sure target AP is in the scan table.
		*/
		pAd->StaCfg.bSkipAutoScanConn = TRUE;
		while ((ScanRunning(pAd) == TRUE) && (WaitCnt++ < 200))
			OS_WAIT(500);

		pAd->MlmeAux.AutoReconnectSsidLen= 0;
		pAd->bConfigChanged = TRUE;
		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					OID_802_11_BSSID,
					MAC_ADDR_LEN,
					pAd->StaCfg.WscControl.WscBssid, 0);
		pWscControl->WscState = WSC_STATE_START;
		StateMachineTouched = TRUE;

    	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
    	pWscControl->Wsc2MinsTimerRunning = TRUE;
    	pWscControl->WscStatus = STATUS_WSC_LINK_UP;

		/*WscSendUPnPConfReqMsg(pAd, apidx, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].Bssid, 3, 0); */
	}
	else
	{
		if ((pAd->StaCfg.BssType == BSS_INFRA) ||
			(pWscControl->WscConfMode == WSC_ENROLLEE))
		{
			pWscControl->WscSsid.SsidLength = 0;
			NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
			RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
			pWscControl->WscPBCBssCount = 0;
			/* WPS - SW PBC */
			WscPushPBCAction(pAd, pWscControl);
			StateMachineTouched = TRUE;
		}
		else
		{
			WscInitRegistrarPair(pAd, pWscControl, BSS0);
			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
			RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
			pWscControl->Wsc2MinsTimerRunning = TRUE;
			pWscControl->WscState = WSC_STATE_LINK_UP;
		}
	}
    
#ifdef WSC_LED_SUPPORT
#ifdef CONFIG_WIFI_LED_SUPPORT
	/* Change FW default mode to WPS LED share mode*/
	pAd->LedCntl.MCULedCntl.word &= 0x80;
	pAd->LedCntl.MCULedCntl.word |= WPS_LED_MODE_SHARE;
	
	if (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
	{
		WPSLEDStatus = LED_WPS_PRE_STAGE;
		RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_PREPOST_WIFI_LED_TIMEOUT);
	}
	else
#endif /* CONFIG_WIFI_LED_SUPPORT */
    	WPSLEDStatus = LED_WPS_IN_PROCESS;
		RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */


    /* Enrollee 192 random bytes for DH key generation */
	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);
    
	if (pWscControl->WscProfileRetryTimerRunning)
	{
		BOOLEAN Cancelled;
		pWscControl->WscProfileRetryTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscProfileRetryTimer, &Cancelled);
	}

	if (StateMachineTouched) /* Upper layer sent a MLME-related operations */
		RTMP_MLME_HANDLER(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGetConf_Proc trigger WSC state machine\n"));

	return TRUE;
}

#ifdef WSC_V2_SUPPORT
INT Set_WscForceSetAP_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg)
{
	PWSC_V2_INFO pWscV2Info = &pAd->StaCfg.WscControl.WscV2Info;
	
	if (simple_strtol(arg, 0, 10) == 0)
		pWscV2Info->bForceSetAP = FALSE;
	else
		pWscV2Info->bForceSetAP = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_WscForceSetAP_Proc::(bForceSetAP=%d)\n", pWscV2Info->bForceSetAP));
	return TRUE;
}
#endif /* WSC_V2_SUPPORT */


#ifdef IWSC_SUPPORT
INT	Set_IWscLimitedUI_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	pWscControl = &pAd->StaCfg.WscControl;
	
	if ( simple_strtol(arg, 0, 10) == 0)
		pAd->StaCfg.IWscInfo.bLimitedUI = FALSE;
	else
		pAd->StaCfg.IWscInfo.bLimitedUI = TRUE;    

	DBGPRINT(RT_DEBUG_TRACE, ("Set_IWscLimitedUI_Proc::(bLimitedUI=%d)\n",  pAd->StaCfg.IWscInfo.bLimitedUI));
	return TRUE;
}

#ifdef IWSC_TEST_SUPPORT
INT	Set_IWscDefaultSecurity_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	UINT8		TmpValue;
	
	pWscControl = &pAd->StaCfg.WscControl;
	TmpValue = (UINT8)simple_strtol(arg, 0, 10);
	if ((TmpValue != 0) && (TmpValue <= 3))
	{
		pAd->StaCfg.IWscInfo.IWscDefaultSecurity = TmpValue;
		DBGPRINT(RT_DEBUG_TRACE, ("%s::(IWscDefaultSecurity=%d)\n", __FUNCTION__, pAd->StaCfg.IWscInfo.IWscDefaultSecurity));
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s::(Value=%d)\n", __FUNCTION__, TmpValue));
		return FALSE;
	}	
}

INT	Set_IWscSmpbcEnrScanOnly_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PIWSC_INFO pIWscInfo;

	pIWscInfo = &pAd->StaCfg.IWscInfo;
	if ( simple_strtol(arg, 0, 10) == 0)
		pIWscInfo->bIwscSmpbcScanningOnly = FALSE;
	else
		pIWscInfo->bIwscSmpbcScanningOnly = TRUE;
}

INT	Set_IWscEmptySubmaskList_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PIWSC_INFO pIWscInfo;

	pIWscInfo = &pAd->StaCfg.IWscInfo;
	if ( simple_strtol(arg, 0, 10) == 0)
		pIWscInfo->bEmptySubmaskList = FALSE;
	else
		pIWscInfo->bEmptySubmaskList = TRUE;
}

INT	Set_IWscBlockConnection_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PIWSC_INFO pIWscInfo;

	pIWscInfo = &pAd->StaCfg.IWscInfo;
	if ( simple_strtol(arg, 0, 10) == 0)
		pIWscInfo->bBlockConnection = FALSE;
	else
		pIWscInfo->bBlockConnection = TRUE;

	if (pIWscInfo->bBlockConnection)
	{
		INT i;
		
		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) 
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		}

        if (pAd->MacTab.Size == 0)
        {			                
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
        } 
	}
}
#endif /* IWSC_TEST_SUPPORT */

INT	Set_IWscOOB_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	NDIS_802_11_SSID	Ssid;
	USHORT WpaPskLen = 0;

	if (pAd->StaCfg.BssType != BSS_ADHOC)
	{
		if (INFRA_ON(pAd))
		{
			/*BOOLEAN Cancelled; */
			/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
			/* Since calling this indicate user don't want to connect to that SSID anymore. */
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);		
			
			LinkDown(pAd, FALSE);
		}
#ifdef DOT11_N_SUPPORT
		SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
		pAd->StaCfg.BssType = BSS_ADHOC;
	}

	pWscControl = &pAd->StaCfg.WscControl;
	
	pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
	
	pWscControl->WscConfMode = WSC_DISABLE;
	pWscControl->WscState = WSC_STATE_OFF;
	pWscControl->WscStatus = STATUS_WSC_IDLE;
	pWscControl->bWscTrigger = FALSE;
	NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
	sprintf(&Ssid.Ssid[0], "IWSC%02X%02X%02X", RandomByte(pAd), RandomByte(pAd), RandomByte(pAd));
	Ssid.SsidLength = strlen(&Ssid.Ssid[0]);

	WscGenRandomKey(pAd, pWscControl, pWscControl->WpaPsk, &WpaPskLen);
	pWscControl->WpaPskLen = (INT)WpaPskLen;
	NdisZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
	NdisMoveMemory(pAd->StaCfg.WpaPassPhrase, pWscControl->WpaPsk, pWscControl->WpaPskLen);
	pAd->StaCfg.WpaPassPhraseLen = pWscControl->WpaPskLen;
	pAd->StaCfg.wdev.AuthMode = Ndis802_11AuthModeWPA2PSK;
	pAd->StaCfg.wdev.WepStatus = Ndis802_11AESEnable;
	if ((pAd->StaCfg.WpaPassPhraseLen >= 8) &&
		(pAd->StaCfg.WpaPassPhraseLen <= 64))
	{
		UCHAR keyMaterial[40];
		
		RTMPZeroMemory(pAd->StaCfg.PMK, 32);
		if (pAd->StaCfg.WpaPassPhraseLen == 64)
		{
		    AtoH((PSTRING) pAd->StaCfg.WpaPassPhrase, pAd->StaCfg.PMK, 32);
		}
		else
		{
		    RtmpPasswordHash((PSTRING) pAd->StaCfg.WpaPassPhrase, Ssid.Ssid, Ssid.SsidLength, keyMaterial);
		    NdisMoveMemory(pAd->StaCfg.PMK, keyMaterial, 32);		
		}
	}

	pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
	pAd->bConfigChanged = TRUE;
	pAd->StaCfg.IWscInfo.bDoNotChangeBSSID = FALSE;

    MlmeEnqueue(pAd, 
                MLME_CNTL_STATE_MACHINE, 
                OID_802_11_SSID,
                sizeof(NDIS_802_11_SSID),
                (VOID *)&Ssid, 0);
	
	RTMP_MLME_HANDLER(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_IWscOOB_Proc\n"));
	return TRUE;
}

INT	Set_IWscSinglePIN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PWSC_CTRL	pWscControl;
	pWscControl = &pAd->StaCfg.WscControl;
	
	if ( simple_strtol(arg, 0, 10) == 0)
		pAd->StaCfg.IWscInfo.bSinglePIN = FALSE;
	else
		pAd->StaCfg.IWscInfo.bSinglePIN = TRUE;    

	DBGPRINT(RT_DEBUG_TRACE, ("Set_IWscSinglePIN_Proc::(bSinglePIN=%d)\n",  pAd->StaCfg.IWscInfo.bSinglePIN));
	return TRUE;
}
#endif /* IWSC_SUPPORT */

#endif /* WSC_STA_SUPPORT */

INT Set_TGnWifiTest_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bTGnWifiTest = FALSE;
    else
        pAd->StaCfg.bTGnWifiTest = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_TGnWifiTest_Proc::(bTGnWifiTest=%d)\n", pAd->StaCfg.bTGnWifiTest));
	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAdapter->StaCfg.IEEE80211dClientMode = Rt802_11_D_None;
    else if (simple_strtol(arg, 0, 10) == 1)
        pAdapter->StaCfg.IEEE80211dClientMode = Rt802_11_D_Flexible;
    else if (simple_strtol(arg, 0, 10) == 2)
        pAdapter->StaCfg.IEEE80211dClientMode = Rt802_11_D_Strict;
    else
        return FALSE;  

    DBGPRINT(RT_DEBUG_TRACE, ("Set_Ieee802dMode_Proc::(IEEEE0211dMode=%d)\n", pAdapter->StaCfg.IEEE80211dClientMode));
    return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef CARRIER_DETECTION_SUPPORT
INT Set_StaCarrierDetect_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->CommonCfg.CarrierDetect.Enable = FALSE;
    else
        pAd->CommonCfg.CarrierDetect.Enable = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_StaCarrierDetect_Proc::(CarrierDetect.Enable=%d)\n", pAd->CommonCfg.CarrierDetect.Enable));
	return TRUE;
}
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
INT Set_DyncVgaEnable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING		arg)
{
	UINT Enable;
	UINT32 bbp_val;

	Enable = simple_strtol(arg, 0, 10);

	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = (Enable > 0) ? TRUE : FALSE;

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable == FALSE)
	{
		RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val);
		bbp_val = (bbp_val & 0xffff80ff) | (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 << 8);
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val);
	
		if (pAd->CommonCfg.RxStream >= 2) {
			RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val);
			bbp_val = (bbp_val & 0xffff80ff) | (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1 << 8);
			RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val);
		}
	}
		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_DyncVgaEnable_Proc::(enable = %d)\n", pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable));

	return TRUE;
}
#endif /* DYNAMIC_VGA_SUPPORT */

INT	Show_Adhoc_MacTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			extra,
	IN	UINT32			size)
{
	INT i;
	
	sprintf(extra, "\n");

#ifdef DOT11_N_SUPPORT
	sprintf(extra, "%sHT Operating Mode : %d\n", extra, pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
#endif /* DOT11_N_SUPPORT */

	sprintf(extra + strlen(extra), "\n%-19s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
			"MAC", "AID", "BSS", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC");
	
	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		
		if (strlen(extra) > (size - 30))
		    break;
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry)) && (pEntry->Sst == SST_ASSOC))
		{
			sprintf(extra + strlen(extra), "%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			sprintf(extra + strlen(extra), "%-4d", (int)pEntry->Aid);
			sprintf(extra + strlen(extra), "%-4d", (int)pEntry->apidx);
			sprintf(extra + strlen(extra), "%-7d", pEntry->RssiSample.AvgRssi0);
			sprintf(extra + strlen(extra), "%-7d", pEntry->RssiSample.AvgRssi1);
			sprintf(extra + strlen(extra), "%-7d", pEntry->RssiSample.AvgRssi2);
			sprintf(extra + strlen(extra), "%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			sprintf(extra + strlen(extra), "%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
			sprintf(extra + strlen(extra), "%-6d", pEntry->HTPhyMode.field.MCS);
			sprintf(extra + strlen(extra), "%-6d", pEntry->HTPhyMode.field.ShortGI);
			sprintf(extra + strlen(extra), "%-6d", pEntry->HTPhyMode.field.STBC);
			sprintf(extra + strlen(extra), "%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			sprintf(extra + strlen(extra), "\n");
		}
	} 

	return TRUE;
}

#ifdef DOT11R_FT_SUPPORT
INT Set_FtApBssid_Proc(
    IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG 					BssIdx = 0;
	MLME_JOIN_REQ_STRUCT    JoinReq;
	UCHAR					MacAddr[MAC_ADDR_LEN];

	if (rtstrmactohex(arg, (PSTRING) &MacAddr[0]) == FALSE)
		return FALSE;

	if (pAd->StaCfg.Dot11RCommInfo.bFtSupport == FALSE)
		return FALSE;
	
	/*
		find the desired BSS in the latest SCAN result table
		search 2.4G band first
	*/
	BssIdx = BssTableSearch(&pAd->ScanTab, MacAddr, 1);	
	/*
		search 5G band, if AP does not exist in 2.4G band
	*/
	if (BssIdx == BSS_NOT_FOUND) 
		BssIdx = BssTableSearch(&pAd->ScanTab, MacAddr, 36);
	if (BssIdx == BSS_NOT_FOUND) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - BSSID not found. reply NDIS_STATUS_NOT_ACCEPTED\n"));
		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
		return TRUE;
	}	

	if (pAd->CommonCfg.Channel != pAd->ScanTab.BssEntry[BssIdx].Channel)
	{
		MLME_FT_REQ_STRUCT		FtReq;
		DBGPRINT(RT_DEBUG_TRACE, ("Dot11r: pBss->Channel = %d. Use OTD.\n", pAd->ScanTab.BssEntry[BssIdx].Channel));
		FT_OTD_ActParmFill(pAd, 
						   &FtReq, 
						   MacAddr, 
						   pAd->ScanTab.BssEntry[BssIdx].AuthMode, 
						   &pAd->StaCfg.Dot11RCommInfo.MdIeInfo, 
						   &pAd->StaCfg.Dot11RCommInfo.FtIeInfo, 
						   pAd->StaCfg.ResVarIELen, pAd->StaCfg.ResVarIEs);
		
		MlmeEnqueue(pAd, FT_OTD_ACT_STATE_MACHINE, FT_OTD_MT2_MLME_REQ, 
					sizeof(MLME_FT_REQ_STRUCT), &FtReq, 0);
	}
	else
	{
		/* copy the matched BSS entry from ScanTab to MlmeAux.SsidBssTab. */
		/* Because we need this entry to become the JOIN target in later on SYNC state machine */
		pAd->MlmeAux.BssIdx = 0;
		pAd->MlmeAux.SsidBssTab.BssNr = 1;
		NdisMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[0], &pAd->ScanTab.BssEntry[BssIdx], sizeof(BSS_ENTRY));
	
		JoinParmFill(pAd, &JoinReq, 0);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT),
					&JoinReq, 0);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_JOIN; 
	}
    
    return TRUE;    
}

INT Set_FtSupport_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.Dot11RCommInfo.bFtSupport = FALSE;
    else
        pAd->StaCfg.Dot11RCommInfo.bFtSupport = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_FTSupport_Proc::(bFtSupport=%d)\n", pAd->StaCfg.Dot11RCommInfo.bFtSupport));
	return TRUE;
}
#endif /* DOT11R_FT_SUPPORT */

INT Set_BeaconLostTime_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
	ULONG ltmp = (ULONG)simple_strtol(arg, 0, 10);

	if ((ltmp != 0) && (ltmp <= 60))
		pAd->StaCfg.BeaconLostTime = (ltmp * OS_HZ);

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_BeaconLostTime_Proc::(BeaconLostTime=%ld)\n", pAd->StaCfg.BeaconLostTime));
	return TRUE;
}

INT Set_AutoRoaming_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bAutoRoaming = FALSE;
    else
        pAd->StaCfg.bAutoRoaming = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_AutoRoaming_Proc::(bAutoRoaming=%d)\n", pAd->StaCfg.bAutoRoaming));
	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set site_survey
    ==========================================================================
*/

INT Set_ForceTxBurst_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bForceTxBurst = FALSE;
    else
        pAd->StaCfg.bForceTxBurst = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_ForceTxBurst_Proc::(bForceTxBurst=%d)\n", pAd->StaCfg.bForceTxBurst));
	return TRUE;
}

#ifdef XLINK_SUPPORT
INT Set_XlinkMode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg)
{
	UINT32 Value = 0;

    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.PSPXlink = 0;
    else
        pAd->StaCfg.PSPXlink = 1;

	AsicSetRxFilter(pAd);

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
	Value &= (~0x80);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_XlinkMode_Proc::(PSPXlink=%d)\n", pAd->StaCfg.PSPXlink));
	return TRUE;
}
#endif /* XLINK_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
INT Set_FragTest_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
    	pAdapter->StaCfg.bFragFlag = FALSE;
	else
		pAdapter->StaCfg.bFragFlag = TRUE;
	return TRUE;
}
#endif /* ETH_CONVERT_SUPPORT */


VOID RTMPAddKey(RTMP_ADAPTER *pAd, PNDIS_802_11_KEY pKey)
{
	ULONG KeyIdx;
	MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;


    DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddKey ------>\n"));

	if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
	{
		if (pKey->KeyIndex & 0x80000000)
		{
		    if (wdev->AuthMode == Ndis802_11AuthModeWPANone)
            {
                NdisZeroMemory(pAd->StaCfg.PMK, 32);
                NdisMoveMemory(pAd->StaCfg.PMK, pKey->KeyMaterial, pKey->KeyLength);
                goto end;
            }
		    /* Update PTK */
		    NdisZeroMemory(&pAd->SharedKey[BSS0][0], sizeof(CIPHER_KEY));  
            pAd->SharedKey[BSS0][0].KeyLen = LEN_TK;
            NdisMoveMemory(pAd->SharedKey[BSS0][0].Key, pKey->KeyMaterial, LEN_TK);
#ifdef WPA_SUPPLICANT_SUPPORT            
            if (pAd->StaCfg.PairCipher == Ndis802_11TKIPEnable)
            {
                NdisMoveMemory(pAd->SharedKey[BSS0][0].RxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);            
                NdisMoveMemory(pAd->SharedKey[BSS0][0].TxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
            }
            else
#endif /* WPA_SUPPLICANT_SUPPORT */
            {
            	NdisMoveMemory(pAd->SharedKey[BSS0][0].TxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);            
                NdisMoveMemory(pAd->SharedKey[BSS0][0].RxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
            }

            /* Decide its ChiperAlg */
        	if (pAd->StaCfg.PairCipher == Ndis802_11TKIPEnable)
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_TKIP;
        	else if (pAd->StaCfg.PairCipher == Ndis802_11AESEnable)
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
        	else
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_NONE; 

            /* Update these related information to MAC_TABLE_ENTRY */
        	pEntry = &pAd->MacTab.Content[BSSID_WCID];
            NdisMoveMemory(pEntry->PairwiseKey.Key, pAd->SharedKey[BSS0][0].Key, LEN_TK);            
        	NdisMoveMemory(pEntry->PairwiseKey.RxMic, pAd->SharedKey[BSS0][0].RxMic, LEN_TKIP_MIC);
        	NdisMoveMemory(pEntry->PairwiseKey.TxMic, pAd->SharedKey[BSS0][0].TxMic, LEN_TKIP_MIC);
        	pEntry->PairwiseKey.CipherAlg = pAd->SharedKey[BSS0][0].CipherAlg;

			/* Add Pair-wise key to Asic */
		    AsicAddPairwiseKeyEntry(
		        pAd,
        						  (UCHAR)pEntry->wcid,
		        &pEntry->PairwiseKey);

			RTMPSetWcidSecurityInfo(pAd,
									BSS0,
									0,
									pEntry->PairwiseKey.CipherAlg,
									(UCHAR)pEntry->wcid,
									PAIRWISEKEYTABLE);

            if (wdev->AuthMode >= Ndis802_11AuthModeWPA2)
            {
                /* set 802.1x port control */
				STA_PORT_SECURED(pAd);
            }
		}
        else
        {
            /* Update GTK */
            wdev->DefaultKeyId = (pKey->KeyIndex & 0xFF);
            NdisZeroMemory(&pAd->SharedKey[BSS0][wdev->DefaultKeyId], sizeof(CIPHER_KEY));  
            pAd->SharedKey[BSS0][wdev->DefaultKeyId].KeyLen = LEN_TK;
            NdisMoveMemory(pAd->SharedKey[BSS0][wdev->DefaultKeyId].Key, pKey->KeyMaterial, LEN_TK);
#ifdef WPA_SUPPLICANT_SUPPORT            
            if (pAd->StaCfg.GroupCipher == Ndis802_11TKIPEnable)
            {
                NdisMoveMemory(pAd->SharedKey[BSS0][wdev->DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);            
                NdisMoveMemory(pAd->SharedKey[BSS0][wdev->DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);        	
            }
            else
#endif /* WPA_SUPPLICANT_SUPPORT */                
            {
            	NdisMoveMemory(pAd->SharedKey[BSS0][wdev->DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);            
                NdisMoveMemory(pAd->SharedKey[BSS0][wdev->DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);        	
            }

            /* Update Shared Key CipherAlg */
    		pAd->SharedKey[BSS0][wdev->DefaultKeyId].CipherAlg = CIPHER_NONE;
    		if (pAd->StaCfg.GroupCipher == Ndis802_11TKIPEnable)
    			pAd->SharedKey[BSS0][wdev->DefaultKeyId].CipherAlg = CIPHER_TKIP;
    		else if (pAd->StaCfg.GroupCipher == Ndis802_11AESEnable)
    			pAd->SharedKey[BSS0][wdev->DefaultKeyId].CipherAlg = CIPHER_AES;

            /* Update group key information to ASIC Shared Key Table */
        	AsicAddSharedKeyEntry(pAd, 
        						  BSS0, 
        						  wdev->DefaultKeyId, 
        						  &pAd->SharedKey[BSS0][wdev->DefaultKeyId]);


            /* set 802.1x port control */
			STA_PORT_SECURED(pAd);
        }
	}
	else	/* dynamic WEP from wpa_supplicant */
	{
		UCHAR	CipherAlg;
    	PUCHAR	Key;

		if(pKey->KeyLength == 32)
			goto end;
		
		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4)
		{
			/* it is a default shared key, for Pairwise key setting */
			if (pKey->KeyIndex & 0x80000000)
			{
				pEntry = MacTableLookup(pAd, pKey->BSSID);

				if (pEntry)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddKey: Set Pair-wise Key\n"));
		
					/* set key material and key length */
 					pEntry->PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);
					
					/* set Cipher type */
					if (pKey->KeyLength == 5)
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP64;
					else
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP128;
						
					/* Add Pair-wise key to Asic */
					AsicAddPairwiseKeyEntry(
						pAd, 
						(UCHAR)pEntry->wcid,
                		&pEntry->PairwiseKey);

					/* update WCID attribute table and IVEIV table for this entry */
					RTMPSetWcidSecurityInfo(pAd, 
											BSS0, 
											KeyIdx, 
											pEntry->PairwiseKey.CipherAlg, 
											pEntry->wcid, 
											PAIRWISEKEYTABLE);
				}	
			}
			else	
            {
				/* Default key for tx (shared key) */
				wdev->DefaultKeyId = (UCHAR) KeyIdx;
                     
				/* set key material and key length */
				pAd->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);
				
				/* Set Ciper type */
				if (pKey->KeyLength == 5)
					pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP64;
				else
					pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP128;
			
    			CipherAlg = pAd->SharedKey[BSS0][KeyIdx].CipherAlg;
    			Key = pAd->SharedKey[BSS0][KeyIdx].Key;

				/* Set Group key material to Asic */
    			AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);
		
			}
		}
	}
end:
	return;
}



/*
    ==========================================================================
    Description:
        Site survey entry point

    NOTE:
    ==========================================================================
*/
VOID StaSiteSurvey(
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType)
{
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		/*
		 * Still scanning, ignore this scanning.
		 */
		DBGPRINT(RT_DEBUG_TRACE, ("StaSiteSurvey:: Scanning now\n"));
		return;
	}
	if (INFRA_ON(pAd))
	{
		pAd->StaCfg.bImprovedScan = TRUE;
		pAd->StaCfg.ScanChannelCnt = 0;	/* reset channel counter to 0 */
	}

#ifdef WSC_STA_SUPPORT
	if (ScanType == SCAN_WSC_ACTIVE)
		pAd->StaCfg.bImprovedScan = FALSE;
#endif /* WSC_STA_SUPPORT */
			
	if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
	{
		RTMP_MLME_RESET_STATE_MACHINE(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
	}

	NdisGetSystemUpTime(&pAd->StaCfg.LastScanTime);

	
	if (pSsid)
		MlmeEnqueue(pAd, 
			MLME_CNTL_STATE_MACHINE, 
			OID_802_11_BSSID_LIST_SCAN, 
			pSsid->SsidLength,
			pSsid->Ssid, 
			0);
	else
		MlmeEnqueue(pAd, 
			MLME_CNTL_STATE_MACHINE, 
			OID_802_11_BSSID_LIST_SCAN, 
			0,
			"",
			0);
	
	RTMP_MLME_HANDLER(pAd);
}

INT Set_AutoReconnect_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bAutoReconnect = FALSE;
	else
		pAd->StaCfg.bAutoReconnect = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_AutoReconnect_Proc::(bAutoReconnect=%d)\n", pAd->StaCfg.bAutoReconnect));
	return TRUE;
}

INT Set_AdhocN_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg)
{
#ifdef DOT11_N_SUPPORT
	if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bAdhocN = FALSE;
	else
		pAd->StaCfg.bAdhocN = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_AdhocN_Proc::(bAdhocN=%d)\n", pAd->StaCfg.bAdhocN));
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}


#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg)
{
	BOOLEAN bWlanLed;
#ifdef CONFIG_SWMCU_SUPPORT
	PSWMCU_LED_CONTROL pSWMCULedCntl = &pAd->LedCntl.SWMCULedCntl;
#endif /* CONFIG_SWMCU_SUPPORT */
	BOOLEAN Cancelled;

	bWlanLed = (ULONG) simple_strtol(arg, 0, 10);
#ifdef CONFIG_SWMCU_SUPPORT
	if (bWlanLed != pSWMCULedCntl->bWlanLed)
#endif /* CONFIG_SWMCU_SUPPORT */
	{
		if (bWlanLed)
			RTMPStartLEDMode(pAd);
		else
			RTMPExitLEDMode(pAd);
	}
        
	return TRUE;
}
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
/* set WOW enable */
INT Set_WOW_Enable(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg)
{
	UINT32 Val;
	UINT8 Pin = pAd->WOW_Cfg.nSelectedGPIO;	
	ULONG Value = simple_strtol(arg, 0, 10);	
		
	if (Value != 1)
		Value = 0;  /* default is disable */

	pAd->WOW_Cfg.bEnable = (BOOLEAN)Value;

	/* pull GPIO high */
	RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Val);
	if (Pin <= 7)
	{
		Val &= ~(1UL << (Pin+8));	/* direction: 0 out, 1 in */
		Val |= 1UL << Pin;			/* data */
	}
	else
	{
		Val &= ~(1UL << (Pin+16)); 	/* direction: 0 out, 1 in */
		Val |= 1UL << (Pin+8);		/* data */
	}
	RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Val);
	
	DBGPRINT(RT_DEBUG_ERROR, ("WOW_Enable = %d, GPIO = %x\n", pAd->WOW_Cfg.bEnable, Val));

	return TRUE;
}

/* set GPIO pin for wake-up signal */
INT Set_WOW_GPIO(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg)
{
	ULONG Value = simple_strtol(arg, 0, 10);	
		
	if (Value > 13)
		Value = 1;  /* default GPIO pin */

	pAd->WOW_Cfg.nSelectedGPIO = (UINT8)Value;
	DBGPRINT(RT_DEBUG_ERROR, ("WOW_GPIO = %d\n", pAd->WOW_Cfg.nSelectedGPIO));

	return TRUE;
}

/* set delay time for WOW really enable */
INT Set_WOW_Delay(
        IN PRTMP_ADAPTER        pAd,
        IN PSTRING              arg)
{
	ULONG Value = simple_strtol(arg, 0, 10);	
		
	if (Value > 255)
		Value = 3; /* default delay time */

	pAd->WOW_Cfg.nDelay = (UINT8)Value;
	DBGPRINT(RT_DEBUG_ERROR, ("WOW_Delay = %d, equal to %d sec\n", pAd->WOW_Cfg.nDelay, (pAd->WOW_Cfg.nDelay+1)*3));

	return TRUE;
}

/* set wake up hold time */
INT Set_WOW_Hold(
		IN PRTMP_ADAPTER		pAd,
		IN PSTRING				arg)
{
	ULONG Value = simple_strtol(arg, 0, 10);

	if (Value > 255)
		Value = 1; /* pule hold time - unit: 10ms */

	pAd->WOW_Cfg.nHoldTime = (UINT8)Value;
	DBGPRINT(RT_DEBUG_ERROR, ("WOW_Hold = %d, equal to %d ms\n", pAd->WOW_Cfg.nHoldTime, (pAd->WOW_Cfg.nHoldTime)*10));

	return TRUE;
}

/* set wake up signal type */
INT Set_WOW_InBand(
		IN PRTMP_ADAPTER		pAd,
		IN PSTRING				arg)
{
	ULONG Value = simple_strtol(arg, 0, 10);

	if (Value != 1)
		Value = 0; /* use GPIO to wakeup system */

	pAd->WOW_Cfg.bInBand = (UINT8)Value;
	DBGPRINT(RT_DEBUG_ERROR, ("WOW_Inband = %d\n", pAd->WOW_Cfg.bInBand));

	return TRUE;
}
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

INT RTMPSetInformation(
    IN RTMP_ADAPTER *pAd,
    INOUT RTMP_IOCTL_INPUT_STRUCT *rq,
    IN  INT cmd)
{
    RTMP_IOCTL_INPUT_STRUCT *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
    NDIS_802_11_SSID Ssid;
    NDIS_802_11_MAC_ADDRESS Bssid;
    RT_802_11_PHY_MODE PhyMode;
    RT_802_11_STA_CONFIG StaConfig;
    NDIS_802_11_RATES aryRates;
    RT_802_11_PREAMBLE Preamble;
    NDIS_802_11_WEP_STATUS WepStatus;
    NDIS_802_11_AUTHENTICATION_MODE AuthMode = Ndis802_11AuthModeMax;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType;
    NDIS_802_11_RTS_THRESHOLD RtsThresh = 0;
    NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh;
    NDIS_802_11_POWER_MODE PowerMode;
    PNDIS_802_11_KEY pKey = NULL;
    PNDIS_802_11_WEP pWepKey =NULL;
    PNDIS_802_11_REMOVE_KEY pRemoveKey = NULL;
	NDIS_802_11_CONFIGURATION *pConfig = NULL;
    NDIS_802_11_NETWORK_TYPE NetType;
    ULONG Now;
    UINT KeyIdx = 0;
    INT Status = NDIS_STATUS_SUCCESS, MaxPhyMode = PHY_11G;
    ULONG PowerTemp;
    BOOLEAN RadioState;
    BOOLEAN StateMachineTouched = FALSE;
     PNDIS_802_11_PASSPHRASE ppassphrase = NULL;     
#ifdef DOT11_N_SUPPORT
	OID_SET_HT_PHYMODE HT_PhyMode;	/*11n */
#endif /* DOT11_N_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT    
    PNDIS_802_11_PMKID pPmkId = NULL;
    BOOLEAN IEEE8021xState = FALSE;
    BOOLEAN IEEE8021x_required_keys = FALSE;
    UCHAR wpa_supplicant_enable = 0;
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef SNMP_SUPPORT	
	TX_RTY_CFG_STRUC tx_rty_cfg;
	ULONG ShortRetryLimit, LongRetryLimit;
	UCHAR ctmp;
#endif /* SNMP_SUPPORT */


#ifdef WAPI_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* WAPI_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	UINT	WPSLedMode10 = 0;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */


	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

#ifdef DOT11_N_SUPPORT
	MaxPhyMode = PHY_11N_5G;
#endif /* DOT11_N_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("-->RTMPSetInformation(),	0x%08x\n", cmd&0x7FFF));
	switch(cmd & 0x7FFF) {
		case RT_OID_802_11_COUNTRY_REGION:
			if (wrq->u.data.length < sizeof(UCHAR))
				Status = -EINVAL;
			/* Only avaliable when EEPROM not programming */
            else if (!(pAd->CommonCfg.CountryRegion & 0x80) && !(pAd->CommonCfg.CountryRegionForABand & 0x80))
			{
				ULONG   Country;

				Status = copy_from_user(&Country, wrq->u.data.pointer, wrq->u.data.length);
				pAd->CommonCfg.CountryRegion = (UCHAR)(Country & 0x000000FF);
				pAd->CommonCfg.CountryRegionForABand = (UCHAR)((Country >> 8) & 0x000000FF);
				/* Build all corresponding channel information */
				RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);

				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_COUNTRY_REGION (A:%d  B/G:%d)\n", pAd->CommonCfg.CountryRegionForABand,
				    pAd->CommonCfg.CountryRegion));
            }
            break;
        case OID_802_11_BSSID_LIST_SCAN:
			RTMP_GetCurrentSystemTick(&Now);
/*            Now = jiffies; */
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_BSSID_LIST_SCAN, TxCnt = %d \n", pAd->RalinkCounters.LastOneSecTotalTxCount));

            if (MONITOR_ON(pAd))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
                break;
            }

			/*Benson add 20080527, when radio off, sta don't need to scan */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
				break;
				
			if (pAd->RalinkCounters.LastOneSecTotalTxCount > 100)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;
            }
            
            if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) &&
				((wdev->AuthMode == Ndis802_11AuthModeWPA) || 
				(wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;
            }

#ifdef WSC_STA_SUPPORT
			if ((pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
				(pAd->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP))
			{				
				DBGPRINT(RT_DEBUG_TRACE, ("!!! WPS processing now! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
				Status = NDIS_STATUS_SUCCESS;
				break;			
			}
#endif /* WSC_STA_SUPPORT */

			StaSiteSurvey(pAd, NULL, SCAN_ACTIVE);
            break;
        case OID_802_11_SSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
                Status = -EINVAL;
            else
            {
            	PSTRING pSsidString = NULL;
                Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);

				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SSID (Len=%d,Ssid=%s)\n", Ssid.SsidLength, Ssid.Ssid));
                if (Ssid.SsidLength > MAX_LEN_OF_SSID)
                    Status = -EINVAL;
                else
                {
                	if (Ssid.SsidLength == 0)
		    		{
                		Set_SSID_Proc(pAd, "");
		    		}
					else
                    {
						os_alloc_mem(pAd, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID+1);
						if (pSsidString)
						{
							NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
							NdisMoveMemory(pSsidString, Ssid.Ssid, Ssid.SsidLength);
							pSsidString[MAX_LEN_OF_SSID] = 0x00;
							Set_SSID_Proc(pAd, pSsidString);
							os_free_mem(NULL, pSsidString);
						}
						else
							Status = -ENOMEM;
                    }
                }
            }
            break;
		case OID_802_11_SET_PASSPHRASE:
			os_alloc_mem(pAd, (UCHAR **)&ppassphrase, wrq->u.data.length);

    	    if(ppassphrase== NULL)
            {
               	Status = -ENOMEM;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed!!\n"));
               	break;
            }
			else
           	{
               	Status = copy_from_user(ppassphrase, wrq->u.data.pointer, wrq->u.data.length);

				if (Status)
            	{
                	Status  = -EINVAL;
                	DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed (length mismatch)!!\n"));
           		}
           		else
           		{
					if(ppassphrase->KeyLength < 8 || ppassphrase->KeyLength > 64)
					{
						Status  = -EINVAL;
                    	DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_PASSPHRASE, Failed (len less than 8 or greater than 64)!!\n"));
					}
					else
					{
	                    			/* set key passphrase and length */
	 					NdisZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
	    				NdisMoveMemory(pAd->StaCfg.WpaPassPhrase, &ppassphrase->KeyMaterial, ppassphrase->KeyLength);
	   					pAd->StaCfg.WpaPassPhraseLen = ppassphrase->KeyLength;
						hex_dump("pAd->StaCfg.WpaPassPhrase", pAd->StaCfg.WpaPassPhrase, 64);
						printk("WpaPassPhrase=%s\n",pAd->StaCfg.WpaPassPhrase);
					}
                }
            }
         	os_free_mem(NULL, ppassphrase);
	   		break;
		
        case OID_802_11_BSSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_MAC_ADDRESS))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&Bssid, wrq->u.data.pointer, wrq->u.data.length);

                /* tell CNTL state machine to call NdisMSetInformationComplete() after completing */
                /* this request, because this request is initiated by NDIS. */
                pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 

				/* Prevent to connect AP again in STAMlmePeriodicExec */
				pAd->MlmeAux.AutoReconnectSsidLen= 32;

                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
                {
                    RTMP_MLME_RESET_STATE_MACHINE(pAd);
                    DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
                }
                MlmeEnqueue(pAd, 
                            MLME_CNTL_STATE_MACHINE, 
                            OID_802_11_BSSID, 
                            sizeof(NDIS_802_11_MAC_ADDRESS),
                            (VOID *)&Bssid, 0);
                Status = NDIS_STATUS_SUCCESS;
                StateMachineTouched = TRUE;

                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        PRINT_MAC(Bssid)));
            }
            break;
        case RT_OID_802_11_RADIO:
            if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&RadioState, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_RADIO (=%d)\n", RadioState));
                if (pAd->StaCfg.bSwRadio != RadioState)
                {
                    pAd->StaCfg.bSwRadio = RadioState;
                    if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
                    {
                        pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                        if (pAd->StaCfg.bRadio == TRUE)
                        {
                            MlmeRadioOn(pAd);
                            /* Update extra information */
							pAd->ExtraInfo = EXTRA_INFO_CLEAR;
                        }
                        else
                        {
                        	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				            {
				                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
						        {
						            RTMP_MLME_RESET_STATE_MACHINE(pAd);
						            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
						        }
				            }
							
                            MlmeRadioOff(pAd);
                            /* Update extra information */
							pAd->ExtraInfo = SW_RADIO_OFF;
                        }
                    }
                }
            }
            break;
        case RT_OID_802_11_PHY_MODE:
		if (wrq->u.data.length != sizeof(RT_802_11_PHY_MODE))
			Status  = -EINVAL;
		else
		{
			// TODO: shiang-6590, fix this for PhyMode!!
			Status = copy_from_user(&PhyMode, wrq->u.data.pointer, wrq->u.data.length);
			if (PhyMode <= MaxPhyMode)
			{
				pAd->CommonCfg.cfg_wmode = PhyMode;
				RTMPSetPhyMode(pAd, PhyMode);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_PHY_MODE (=%d)\n", PhyMode));
		}
		break;
        case RT_OID_802_11_STA_CONFIG:
            if (wrq->u.data.length != sizeof(RT_802_11_STA_CONFIG))
                Status  = -EINVAL;
            else
            {
            	UINT32	Value;
				
                Status = copy_from_user(&StaConfig, wrq->u.data.pointer, wrq->u.data.length);
                pAd->CommonCfg.bEnableTxBurst = StaConfig.EnableTxBurst;
                pAd->CommonCfg.UseBGProtection = StaConfig.UseBGProtection;
                pAd->CommonCfg.bUseShortSlotTime = 1; /* 2003-10-30 always SHORT SLOT capable */
                if ((pAd->CommonCfg.PhyMode != StaConfig.AdhocMode) &&
					(StaConfig.AdhocMode <= MaxPhyMode))
                {
                    /* allow dynamic change of "USE OFDM rate or not" in ADHOC mode */
                    /* if setting changed, need to reset current TX rate as well as BEACON frame format */
                    if (pAd->StaCfg.BssType == BSS_ADHOC)
                    {
				RTMPSetPhyMode(pAd, StaConfig.AdhocMode);
				MlmeUpdateTxRates(pAd, FALSE, 0);
				MakeIbssBeacon(pAd);           /* re-build BEACON frame */
				AsicEnableIbssSync(pAd);   /* copy to on-chip memory */
                    }
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_STA_CONFIG (Burst=%d, Protection=%ld,ShortSlot=%d\n",
                                        pAd->CommonCfg.bEnableTxBurst,
                                        pAd->CommonCfg.UseBGProtection,
                                        pAd->CommonCfg.bUseShortSlotTime));

		AsicSetRxFilter(pAd);
            }
            break;
        case OID_802_11_DESIRED_RATES:
            if (wrq->u.data.length != sizeof(NDIS_802_11_RATES))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&aryRates, wrq->u.data.pointer, wrq->u.data.length);
                NdisZeroMemory(pAd->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
                NdisMoveMemory(pAd->CommonCfg.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DESIRED_RATES (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
                    pAd->CommonCfg.DesireRate[0],pAd->CommonCfg.DesireRate[1],
                    pAd->CommonCfg.DesireRate[2],pAd->CommonCfg.DesireRate[3],
                    pAd->CommonCfg.DesireRate[4],pAd->CommonCfg.DesireRate[5],
                    pAd->CommonCfg.DesireRate[6],pAd->CommonCfg.DesireRate[7] ));
                /* Changing DesiredRate may affect the MAX TX rate we used to TX frames out */
                MlmeUpdateTxRates(pAd, FALSE, 0);
            }
            break;
        case RT_OID_802_11_PREAMBLE:
            if (wrq->u.data.length != sizeof(RT_802_11_PREAMBLE))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&Preamble, wrq->u.data.pointer, wrq->u.data.length);
                if (Preamble == Rt802_11PreambleShort)
                {
                    pAd->CommonCfg.TxPreamble = Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
                }
                else if ((Preamble == Rt802_11PreambleLong) || (Preamble == Rt802_11PreambleAuto))
                {
                    /* if user wants AUTO, initialize to LONG here, then change according to AP's */
                    /* capability upon association. */
                    pAd->CommonCfg.TxPreamble = Preamble;
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
                }
                else
                {
                    Status = -EINVAL;
                    break;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_PREAMBLE (=%d)\n", Preamble));
            }
            break;
        case OID_802_11_WEP_STATUS:
            if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
                /* Since TKIP, AES, WEP are all supported. It should not have any invalid setting */
                if (WepStatus <= Ndis802_11Encryption3KeyAbsent)
                {
                    if (pAd->StaCfg.wdev.WepStatus != WepStatus)
                    {
                        /* Config has changed */
                        pAd->bConfigChanged = TRUE;
                    }
                    pAd->StaCfg.wdev.WepStatus     = WepStatus;
                    pAd->StaCfg.PairCipher    = WepStatus;
                	pAd->StaCfg.GroupCipher   = WepStatus;

			if (pAd->StaCfg.BssType == BSS_ADHOC)
				RTMPSetPhyMode(pAd, pAd->CommonCfg.cfg_wmode);
                }
                else
                {
                    Status  = -EINVAL;
                    break;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEP_STATUS (=%d)\n",WepStatus));
            }
            break;
        case OID_802_11_AUTHENTICATION_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE)) 
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);
                if (AuthMode > Ndis802_11AuthModeMax)
                {
                    Status  = -EINVAL;
                    break;
                }
                else
                {
                    if (wdev->AuthMode != AuthMode)
                    {
                        /* Config has changed */
                        pAd->bConfigChanged = TRUE;
                    }
                    wdev->AuthMode = AuthMode;
                }
                wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_AUTHENTICATION_MODE (=%d) \n", wdev->AuthMode));
            }
            break;
        case OID_802_11_INFRASTRUCTURE_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&BssType, wrq->u.data.pointer, wrq->u.data.length);
							
				if (BssType == Ndis802_11IBSS)
					Set_NetworkType_Proc(pAd, "Adhoc");
				else if (BssType == Ndis802_11Infrastructure)
					Set_NetworkType_Proc(pAd, "Infra");
                else if (BssType == Ndis802_11Monitor)
					Set_NetworkType_Proc(pAd, "Monitor");
                else
                {
                    Status  = -EINVAL;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_INFRASTRUCTURE_MODE (unknown)\n"));
                }
            }
            break;
	 case OID_802_11_REMOVE_WEP:
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_WEP\n"));
            if (wrq->u.data.length != sizeof(NDIS_802_11_KEY_INDEX))
            {
				Status = -EINVAL;
            }
            else 
            {
		KeyIdx = *(NDIS_802_11_KEY_INDEX *) wrq->u.data.pointer;

		if (KeyIdx & 0x80000000)
		{
			/* Should never set default bit when remove key */
			Status = -EINVAL;
		}
		else
		{
			KeyIdx = KeyIdx & 0x0fffffff;
			if (KeyIdx >= 4){
				Status = -EINVAL;
			}
			else
			{
						pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
						pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
						AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)KeyIdx);
			}
		}
            }
            break;
        case RT_OID_802_11_RESET_COUNTERS:
            NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
            NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
            NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));
            pAd->Counters8023.RxNoBuffer   = 0;
			pAd->Counters8023.GoodReceives = 0;
			pAd->Counters8023.RxNoBuffer   = 0;
#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
			{
				int i;
				for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
					NdisZeroMemory(&pAd->MacTab.Content[i].TxBFCounters, sizeof(pAd->MacTab.Content[i].TxBFCounters));		
			}
#endif /* TXBF_SUPPORT */

            DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_RESET_COUNTERS \n"));
            break;
        case OID_802_11_RTS_THRESHOLD:
            if (wrq->u.data.length != sizeof(NDIS_802_11_RTS_THRESHOLD))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&RtsThresh, wrq->u.data.pointer, wrq->u.data.length);
                if (RtsThresh > MAX_RTS_THRESHOLD)
				{
                    Status = -EINVAL;
					RtsThresh = 0; /* avoid compile warning in printk() */
                }
				else
                    pAd->CommonCfg.RtsThreshold = (USHORT)RtsThresh;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_RTS_THRESHOLD (=%ld)\n",RtsThresh));
            break;
        case OID_802_11_FRAGMENTATION_THRESHOLD:
            if (wrq->u.data.length != sizeof(NDIS_802_11_FRAGMENTATION_THRESHOLD))
			{
                Status  = -EINVAL;
				FragThresh = 0; /* avoid compile warning in printk() */
            }
			else
            {
                Status = copy_from_user(&FragThresh, wrq->u.data.pointer, wrq->u.data.length);
                pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
                if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD)
                {
                    if (FragThresh == 0)
                    {
                        pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
                        pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
                    }
                    else
                        Status  = -EINVAL;
                }
                else
                    pAd->CommonCfg.FragmentThreshold = (USHORT)FragThresh;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_FRAGMENTATION_THRESHOLD (=%ld) \n",FragThresh));
            break;
        case OID_802_11_POWER_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_POWER_MODE))
			{
                Status = -EINVAL;
				PowerMode = 0; /* avoid compile warning in printk() */
            }
			else
            {
                Status = copy_from_user(&PowerMode, wrq->u.data.pointer, wrq->u.data.length);
                if (PowerMode == Ndis802_11PowerModeCAM) 
                	Set_PSMode_Proc(pAd, "CAM");
                else if (PowerMode == Ndis802_11PowerModeMAX_PSP) 
                	Set_PSMode_Proc(pAd, "Max_PSP");
                else if (PowerMode == Ndis802_11PowerModeFast_PSP) 
					Set_PSMode_Proc(pAd, "Fast_PSP");
                else if (PowerMode == Ndis802_11PowerModeLegacy_PSP) 
					Set_PSMode_Proc(pAd, "Legacy_PSP");
                else
                    Status = -EINVAL;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_POWER_MODE (=%d)\n",PowerMode));
            break;
         case RT_OID_802_11_TX_POWER_LEVEL_1:
			if (wrq->u.data.length  < sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&PowerTemp, wrq->u.data.pointer, wrq->u.data.length);
				if (PowerTemp > 100)
					PowerTemp = 0xffffffff;  /* AUTO */
				pAd->CommonCfg.TxPowerDefault = PowerTemp; /*keep current setting. */
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;			
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_TX_POWER_LEVEL_1 (=%ld)\n", pAd->CommonCfg.TxPowerPercentage));
			}	
	        break;
		case OID_802_11_NETWORK_TYPE_IN_USE: 
			if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_TYPE))
				Status = -EINVAL;
			else 
			{
				Status = copy_from_user(&NetType, wrq->u.data.pointer, wrq->u.data.length);

				if (NetType == Ndis802_11DS)
					RTMPSetPhyMode(pAd, WMODE_B);
				else if (NetType == Ndis802_11OFDM24)
					RTMPSetPhyMode(pAd, WMODE_B | WMODE_G);
				else if (NetType == Ndis802_11OFDM5)
					RTMPSetPhyMode(pAd, WMODE_A);
				else 
					Status = -EINVAL;

                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_NETWORK_TYPE_IN_USE (=%d)\n",NetType));
		    }	    
			break;
        /* For WPA PSK PMK key */
        case RT_OID_802_11_ADD_WPA:
			os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
            if(pKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            
            Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
            if ((pKey->Length != wrq->u.data.length) ||
				(pKey->KeyLength > LEN_PMK))
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA, Failed!!\n"));
            }
            else
            {
                if ((wdev->AuthMode != Ndis802_11AuthModeWPAPSK) &&
				    (wdev->AuthMode != Ndis802_11AuthModeWPA2PSK) &&
				    (wdev->AuthMode != Ndis802_11AuthModeWPANone) )
                {
                    Status = -EOPNOTSUPP;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA, Failed!! [AuthMode != WPAPSK/WPA2PSK/WPANONE]\n"));
                }
                else if ((wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
						 (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK) ||
						 (wdev->AuthMode == Ndis802_11AuthModeWPANone) )     /* Only for WPA PSK mode */
				{		
                    NdisMoveMemory(pAd->StaCfg.PMK, &pKey->KeyMaterial, pKey->KeyLength);
                    /* Use RaConfig as PSK agent. */
                    /* Start STA supplicant state machine */
                    if (wdev->AuthMode != Ndis802_11AuthModeWPANone)
                        pAd->StaCfg.WpaState = SS_START;

                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
                }
                else
                {   
                    pAd->StaCfg.WpaState = SS_NOTUSE;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
                }
            }
			os_free_mem(NULL, pKey);
            break;
        case OID_802_11_REMOVE_KEY:
			os_alloc_mem(pAd, (UCHAR **)&pRemoveKey, wrq->u.data.length);
            if(pRemoveKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            
            Status = copy_from_user(pRemoveKey, wrq->u.data.pointer, wrq->u.data.length);
            if (pRemoveKey->Length != wrq->u.data.length)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!\n"));
            }
            else
            {
                if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
                {
                    RTMPWPARemoveKeyProc(pAd, pRemoveKey);
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Remove WPA Key!!\n"));
                }
                else
                {
                    KeyIdx = pRemoveKey->KeyIndex;

                    if (KeyIdx & 0x80000000)
                    {
                        /* Should never set default bit when remove key */
                        Status  = -EINVAL;
                        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!(Should never set default bit when remove key)\n"));
                    }
                    else
                    {
                        KeyIdx = KeyIdx & 0x0fffffff;
                        if (KeyIdx > 3)
                        {
                            Status  = -EINVAL;
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY, Failed!!(KeyId[%d] out of range)\n", KeyIdx));
                        }
                        else
                        {
                            pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
                            pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
                            AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)KeyIdx); 
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_REMOVE_KEY (id=0x%x, Len=%d-byte)\n", pRemoveKey->KeyIndex, pRemoveKey->Length));
                        }
                    }
                }
            }
			os_free_mem(NULL, pRemoveKey);
            break;        
        /* New for WPA */
        case OID_802_11_ADD_KEY:
			os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
            if(pKey == NULL)
            {
                Status = -ENOMEM;
                break;
            }
            Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
            if ((pKey->Length != wrq->u.data.length) ||
				(pKey->KeyLength > LEN_PMK))
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_KEY, Failed!!\n"));
            }
            else
            {
                RTMPAddKey(pAd, pKey);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_KEY (id=0x%x, Len=%d-byte)\n", pKey->KeyIndex, pKey->KeyLength));
            }
			os_free_mem(NULL, pKey);
            break;
        case OID_802_11_CONFIGURATION:
            if (wrq->u.data.length != sizeof(NDIS_802_11_CONFIGURATION))
                Status  = -EINVAL;
            else
            {
				os_alloc_mem(NULL, (UCHAR **)&pConfig, sizeof(NDIS_802_11_CONFIGURATION));
				if (pConfig != NULL)
				{
					Status = copy_from_user(pConfig, wrq->u.data.pointer, wrq->u.data.length);
/*                pConfig = &Config; */

	                if ((pConfig->BeaconPeriod >= 20) && (pConfig->BeaconPeriod <=400))
	                     pAd->CommonCfg.BeaconPeriod = (USHORT) pConfig->BeaconPeriod;
	                
	                pAd->StaActive.AtimWin = (USHORT) pConfig->ATIMWindow;
	                MAP_KHZ_TO_CHANNEL_ID(pConfig->DSConfig, pAd->CommonCfg.Channel);
	                /* */
					/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
					/* */
					pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
					
	                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_CONFIGURATION (BeacnPeriod=%ld,AtimW=%ld,Ch=%d)\n",
	                    pConfig->BeaconPeriod, pConfig->ATIMWindow, pAd->CommonCfg.Channel));
	                /* Config has changed */
	                pAd->bConfigChanged = TRUE;
					os_free_mem(NULL, pConfig);
				}
            }
            break;
#ifdef DOT11_N_SUPPORT
		case RT_OID_802_11_SET_HT_PHYMODE:
			if (wrq->u.data.length	!= sizeof(OID_SET_HT_PHYMODE))
				Status = -EINVAL;
			else 
			{
			    POID_SET_HT_PHYMODE	pHTPhyMode = &HT_PhyMode;
                
				Status = copy_from_user(&HT_PhyMode, wrq->u.data.pointer, wrq->u.data.length);				
				DBGPRINT(RT_DEBUG_TRACE, ("Set::pHTPhyMode	(PhyMode = %d,TransmitNo = %d, HtMode =	%d,	ExtOffset =	%d , MCS = %d, BW =	%d,	STBC = %d, SHORTGI = %d) \n", 
				pHTPhyMode->PhyMode, pHTPhyMode->TransmitNo,pHTPhyMode->HtMode,pHTPhyMode->ExtOffset,
				pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
				if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
					RTMPSetHT(pAd,	pHTPhyMode);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_HT_PHYMODE(MCS=%d,BW=%d,SGI=%d,STBC=%d)\n", 
				wdev->HTPhyMode.field.MCS, wdev->HTPhyMode.field.BW, wdev->HTPhyMode.field.ShortGI,
				wdev->HTPhyMode.field.STBC));
			break;
#endif /* DOT11_N_SUPPORT */
		case RT_OID_802_11_SET_APSD_SETTING:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				ULONG apsd ;
				Status = copy_from_user(&apsd, wrq->u.data.pointer,	wrq->u.data.length);

				/*-------------------------------------------------------------------
				|B31~B7	|	B6~B5	 |	 B4	 |	 B3	 |	B2	 |	B1	 |	   B0		|
				---------------------------------------------------------------------
				| Rsvd	| Max SP Len | AC_VO | AC_VI | AC_BK | AC_BE | APSD	Capable	|
				---------------------------------------------------------------------*/
				pAd->StaCfg.UapsdInfo.bAPSDCapable = (apsd & 0x00000001) ? TRUE :	FALSE;
				pAd->CommonCfg.bAPSDAC_BE = ((apsd	& 0x00000002) >> 1)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_BK = ((apsd	& 0x00000004) >> 2)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_VI = ((apsd	& 0x00000008) >> 3)	? TRUE : FALSE;
				pAd->CommonCfg.bAPSDAC_VO = ((apsd	& 0x00000010) >> 4)	? TRUE : FALSE;
				pAd->CommonCfg.MaxSPLength	= (UCHAR)((apsd	& 0x00000060) >> 5);

				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_APSD_SETTING (apsd=0x%lx, APSDCap=%d, [BE,BK,VI,VO]=[%d/%d/%d/%d],	MaxSPLen=%d)\n", apsd, pAd->StaCfg.UapsdInfo.bAPSDCapable,
					pAd->CommonCfg.bAPSDAC_BE,	pAd->CommonCfg.bAPSDAC_BK,	pAd->CommonCfg.bAPSDAC_VI,	pAd->CommonCfg.bAPSDAC_VO,	pAd->CommonCfg.MaxSPLength));
			}
			break;

		case RT_OID_802_11_SET_APSD_PSM:
			if (wrq->u.data.length	!= sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				/* Driver needs to notify AP when PSM changes */
				Status = copy_from_user(&pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.pointer, wrq->u.data.length);
				if (pAd->CommonCfg.bAPSDForcePowerSave	!= pAd->StaCfg.Psm)
				{
					RTMP_SET_PSM_BIT(pAd,	pAd->CommonCfg.bAPSDForcePowerSave);
					RTMPSendNullFrame(pAd,	pAd->CommonCfg.TxRate, TRUE, pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_APSD_PSM (bAPSDForcePowerSave:%d)\n",	pAd->CommonCfg.bAPSDForcePowerSave));
			}
			break;
#ifdef QOS_DLS_SUPPORT
		case RT_OID_802_11_SET_DLS:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				BOOLEAN	oldvalue = pAd->CommonCfg.bDLSCapable;
				Status = copy_from_user(&pAd->CommonCfg.bDLSCapable, wrq->u.data.pointer, wrq->u.data.length);
				if (oldvalue &&	!pAd->CommonCfg.bDLSCapable)
				{
					int	i;
					/* tear	down local dls table entry */
					for	(i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
					{
						if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
						{
							pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
							pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
							RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
						}
					}

					/* tear	down peer dls table	entry */
					for	(i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
					{
						if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
						{
							pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
							pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
							RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
						}
					}
				}

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS (=%d)\n", pAd->CommonCfg.bDLSCapable));
			}
			break;

		case RT_OID_802_11_SET_DLS_PARAM:
			if (wrq->u.data.length	!= sizeof(RT_802_11_DLS_UI))
				Status = -EINVAL;
			else 
			{
				RT_802_11_DLS	Dls;

				NdisZeroMemory(&Dls, sizeof(RT_802_11_DLS));
				RTMPMoveMemory(&Dls, wrq->u.data.pointer, sizeof(RT_802_11_DLS_UI));
				MlmeEnqueue(pAd, 
							MLME_CNTL_STATE_MACHINE, 
							RT_OID_802_11_SET_DLS_PARAM, 
							sizeof(RT_802_11_DLS), 
							&Dls, 0);
				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS_PARAM \n"));
			}
			break;
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
		case RT_OID_802_11_SET_TDLS:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else 
			{
				BOOLEAN	oldvalue = pAd->StaCfg.TdlsInfo.bTDLSCapable;

				Status = copy_from_user(&pAd->StaCfg.TdlsInfo.bTDLSCapable, wrq->u.data.pointer, wrq->u.data.length);
				if (oldvalue &&	!pAd->StaCfg.TdlsInfo.bTDLSCapable)
				{
					/* tear	down local dls table entry */
					TDLS_LinkTearDown(pAd, TRUE);
				}

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_DLS (=%d)\n", pAd->CommonCfg.bDLSCapable));
			}
			break;

		case RT_OID_802_11_SET_TDLS_PARAM:
			if (wrq->u.data.length	!= sizeof(RT_802_11_TDLS_UI))
				Status = -EINVAL;
			else 
			{
				RT_802_11_TDLS		TDLS;

				/* Initialized mlme request */
				RTMPZeroMemory(&TDLS, sizeof(RT_802_11_TDLS));
				RTMPMoveMemory(&TDLS, wrq->u.data.pointer, sizeof(RT_802_11_TDLS_UI));				

				MlmeEnqueue(pAd, 
							MLME_CNTL_STATE_MACHINE, 
							RT_OID_802_11_SET_TDLS_PARAM, 
							sizeof(RT_802_11_TDLS), 
							&TDLS, 0);

				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_TDLS_PARAM \n"));
			}
			break;
#endif /* DOT11Z_TDLS_SUPPORT */

		case RT_OID_802_11_SET_WMM:
			if (wrq->u.data.length	!= sizeof(BOOLEAN))
				Status = -EINVAL;
			else 
			{
				Status = copy_from_user(&pAd->CommonCfg.bWmmCapable, wrq->u.data.pointer, wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_SET_WMM (=%d)	\n", pAd->CommonCfg.bWmmCapable));
			}
			break;

		case OID_802_11_DISASSOCIATE:
			/* */
			/* Set NdisRadioStateOff to	TRUE, instead of called	MlmeRadioOff. */
			/* Later on, NDIS_802_11_BSSID_LIST_EX->NumberOfItems should be	0 */
			/* when	query OID_802_11_BSSID_LIST. */
			/* */
			/* TRUE:  NumberOfItems	will set to	0. */
			/* FALSE: NumberOfItems	no change. */
			/* */
			pAd->CommonCfg.NdisRadioStateOff =	TRUE;
			/* Set to immediately send the media disconnect	event */
			pAd->MlmeAux.CurrReqIsFromNdis	= TRUE;			
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DISASSOCIATE	\n"));

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
			if (pAd->StaCfg.WscControl.bSkipWPSTurnOffLED == FALSE)
			{
				UCHAR WPSLEDStatus = LED_WPS_TURN_LED_OFF;
				BOOLEAN Cancelled;
				RTMPSetLED(pAd, WPSLEDStatus);
				/* Cancel the WPS LED timer. */
				RTMPCancelTimer(&pAd->StaCfg.WscControl.WscLEDTimer, &Cancelled);
			}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */

			if (INFRA_ON(pAd))
			{
				if (pAd->Mlme.CntlMachine.CurrState !=	CNTL_IDLE)
				{
					RTMP_MLME_RESET_STATE_MACHINE(pAd);
					DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME	busy, reset	MLME state machine !!!\n"));
				}

				MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE,
					OID_802_11_DISASSOCIATE,
					0,
					NULL, 0);

				StateMachineTouched	= TRUE;
			}
			break;

#ifdef DOT11_N_SUPPORT
		case RT_OID_802_11_SET_IMME_BA_CAP:
				if (wrq->u.data.length != sizeof(OID_BACAP_STRUC))
					Status = -EINVAL;
				else
				{
					OID_BACAP_STRUC Orde ;
					Status = copy_from_user(&Orde, wrq->u.data.pointer, wrq->u.data.length);
					if (Orde.Policy > BA_NOTUSE)
					{
						Status = NDIS_STATUS_INVALID_DATA;
					}
					else if (Orde.Policy == BA_NOTUSE)
					{
						pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
						pAd->CommonCfg.BACapability.field.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.AmsduEnable = Orde.AmsduEnable;
						pAd->CommonCfg.DesiredHtPhy.AmsduSize= Orde.AmsduSize;
						pAd->CommonCfg.DesiredHtPhy.MimoPs= Orde.MMPSmode;
						pAd->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;
						/* UPdata to HT IE */
						pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = Orde.AmsduSize;
						pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = Orde.MpduDensity;
					}
					else
					{
                        pAd->CommonCfg.BACapability.field.AutoBA = Orde.AutoBA;
						pAd->CommonCfg.BACapability.field.Policy = IMMED_BA; /* we only support immediate BA. */
						pAd->CommonCfg.BACapability.field.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.MpduDensity = Orde.MpduDensity;
						pAd->CommonCfg.DesiredHtPhy.AmsduEnable = Orde.AmsduEnable;
						pAd->CommonCfg.DesiredHtPhy.AmsduSize= Orde.AmsduSize;
						pAd->CommonCfg.DesiredHtPhy.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.BACapability.field.MMPSmode = Orde.MMPSmode;
							
						/* UPdata to HT IE */
						pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = Orde.MMPSmode;
						pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = Orde.AmsduSize;
						pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = Orde.MpduDensity;
						
						if (pAd->CommonCfg.BACapability.field.RxBAWinLimit > MAX_RX_REORDERBUF)
							pAd->CommonCfg.BACapability.field.RxBAWinLimit = MAX_RX_REORDERBUF;

					}

					pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
					DBGPRINT(RT_DEBUG_TRACE, ("Set::(Orde.AutoBA = %d) (Policy=%d)(ReBAWinLimit=%d)(TxBAWinLimit=%d)(AutoMode=%d)\n",Orde.AutoBA, pAd->CommonCfg.BACapability.field.Policy,
						pAd->CommonCfg.BACapability.field.RxBAWinLimit,pAd->CommonCfg.BACapability.field.TxBAWinLimit, pAd->CommonCfg.BACapability.field.AutoBA));
					DBGPRINT(RT_DEBUG_TRACE, ("Set::(MimoPs = %d)(AmsduEnable = %d) (AmsduSize=%d)(MpduDensity=%d)\n",pAd->CommonCfg.DesiredHtPhy.MimoPs, pAd->CommonCfg.DesiredHtPhy.AmsduEnable,
						pAd->CommonCfg.DesiredHtPhy.AmsduSize, pAd->CommonCfg.DesiredHtPhy.MpduDensity));
				}

				break;
		case RT_OID_802_11_ADD_IMME_BA:
			DBGPRINT(RT_DEBUG_TRACE, (" Set :: RT_OID_802_11_ADD_IMME_BA \n"));
			if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
					Status = -EINVAL;
			else 
			{
				UCHAR		        index;
				OID_ADD_BA_ENTRY    BA;
				MAC_TABLE_ENTRY     *pEntry;

				Status = copy_from_user(&BA, wrq->u.data.pointer, wrq->u.data.length);
				if (BA.TID > (NUM_OF_TID-1))
				{
					Status = NDIS_STATUS_INVALID_DATA;
					break;
				}
				else
				{
					/*BATableInsertEntry */
					/*As ad-hoc mode, BA pair is not limited to only BSSID. so add via OID. */
					index = BA.TID;
					/* in ad hoc mode, when adding BA pair, we should insert this entry into MACEntry too */
					pEntry = MacTableLookup(pAd, BA.MACAddr);
					if (!pEntry)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_802_11_ADD_IMME_BA. break on no connection.----:%x:%x\n", BA.MACAddr[4], BA.MACAddr[5]));
						break;
					}
					if (BA.IsRecipient == FALSE)
					{
					    if (pEntry->bIAmBadAtheros == TRUE)
							pAd->CommonCfg.BACapability.field.RxBAWinLimit = 0x10;

						BAOriSessionSetUp(pAd, pEntry, index, 0, 100, TRUE);
					}
					else 
					{
						/*BATableInsertEntry(pAd, pEntry->Aid, BA.MACAddr, 0, 0xffff, BA.TID, BA.nMSDU, BA.IsRecipient); */
					}

					DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_802_11_ADD_IMME_BA. Rec = %d. Mac = %x:%x:%x:%x:%x:%x . \n",
						BA.IsRecipient, BA.MACAddr[0], BA.MACAddr[1], BA.MACAddr[2], BA.MACAddr[2]
						, BA.MACAddr[4], BA.MACAddr[5]));
				}
			}
			break;

		case RT_OID_802_11_TEAR_IMME_BA:
			DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA \n"));
			if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
					Status = -EINVAL;
			else 
			{
				POID_ADD_BA_ENTRY	pBA;
				MAC_TABLE_ENTRY *pEntry;
				
				os_alloc_mem(pAd, (UCHAR **)&pBA, wrq->u.data.length);
				if (pBA == NULL)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA kmalloc() can't allocate enough memory\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{
					Status = copy_from_user(pBA, wrq->u.data.pointer, wrq->u.data.length);
					DBGPRINT(RT_DEBUG_TRACE, ("Set :: RT_OID_802_11_TEAR_IMME_BA(TID=%d, bAllTid=%d)\n", pBA->TID, pBA->bAllTid));
					
					if (!pBA->bAllTid && (pBA->TID > (NUM_OF_TID-1)))
					{
						Status = NDIS_STATUS_INVALID_DATA;
						os_free_mem(NULL, pBA);
						break;
					}
					
					if (pBA->IsRecipient == FALSE)
					{
						pEntry = MacTableLookup(pAd, pBA->MACAddr);
						DBGPRINT(RT_DEBUG_TRACE, (" pBA->IsRecipient == FALSE\n"));
						if (pEntry)
						{
							DBGPRINT(RT_DEBUG_TRACE, (" pBA->pEntry\n"));
							BAOriSessionTearDown(pAd, pEntry->wcid, pBA->TID, FALSE, TRUE);
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("Set :: Not found pEntry \n"));
					}
					else
					{
						pEntry = MacTableLookup(pAd, pBA->MACAddr);
						if (pEntry)
						{
							BARecSessionTearDown( pAd, (UCHAR)pEntry->wcid, pBA->TID, TRUE);
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("Set :: Not found pEntry \n"));
					}
					os_free_mem(NULL, pBA);
				}
            }
            break;
#endif /* DOT11_N_SUPPORT */

        /* For WPA_SUPPLICANT to set static wep key */
    	case OID_802_11_ADD_WEP:
			os_alloc_mem(pAd, (UCHAR **)&pWepKey, wrq->u.data.length);

    	    if(pWepKey == NULL)
            {
                Status = -ENOMEM;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed!!\n"));
                break;
            }
            Status = copy_from_user(pWepKey, wrq->u.data.pointer, wrq->u.data.length);
            if (Status)
            {
                Status  = -EINVAL;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed (length mismatch)!!\n"));
            }
            else
            {
		        KeyIdx = pWepKey->KeyIndex & 0x0fffffff;
                /* KeyIdx must be 0 ~ 3 */
                if (KeyIdx >= 4)
    			{
                    Status  = -EINVAL;
                    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, Failed (KeyIdx must be smaller than 4)!!\n"));
                }
                else
                {
                    UCHAR CipherAlg = 0;
                    PUCHAR Key;

					/* Zero the specific shared key */
					NdisZeroMemory(&pAd->SharedKey[BSS0][KeyIdx], sizeof(CIPHER_KEY));

                    /* set key material and key length */
                    pAd->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
                    NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pWepKey->KeyMaterial, pWepKey->KeyLength);

                    switch(pWepKey->KeyLength)
                    {
                        case 5:
                            CipherAlg = CIPHER_WEP64;
                            break;
                        case 13:
                            CipherAlg = CIPHER_WEP128;
                            break;
                        default:
                            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP, only support CIPHER_WEP64(len:5) & CIPHER_WEP128(len:13)!!\n"));
                            Status = -EINVAL;
                            break;
                    }
                    pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;

                    /* Default key for tx (shared key) */
                    if (pWepKey->KeyIndex & 0x80000000)
                    {
#ifdef WPA_SUPPLICANT_SUPPORT
                        NdisZeroMemory(&pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[KeyIdx], sizeof(CIPHER_KEY));
								
                        /* set key material and key length */
                        pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
                        NdisMoveMemory(pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[KeyIdx].Key, 
								&pWepKey->KeyMaterial,
								pWepKey->KeyLength);
                        pAd->StaCfg.wpa_supplicant_info.DesireSharedKeyId = KeyIdx;
                        pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[KeyIdx].CipherAlg = CipherAlg;
#endif /* WPA_SUPPLICANT_SUPPORT */                    
                        wdev->DefaultKeyId = (UCHAR) KeyIdx;
                    }
                    
#ifdef WPA_SUPPLICANT_SUPPORT
					if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
						(wdev->AuthMode >= Ndis802_11AuthModeWPA))
					{
						Key = pWepKey->KeyMaterial;
						
						/* Set Group key material to Asic */
    					AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);
						
						STA_PORT_SECURED(pAd);
					}
                    else if (wdev->PortSecured == WPA_802_1X_PORT_SECURED)
#endif /* WPA_SUPPLICANT_SUPPORT */
                    {
                        Key = pAd->SharedKey[BSS0][KeyIdx].Key;

                        /* Set key material and cipherAlg to Asic */
        				AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);	
                        
                        if (pWepKey->KeyIndex & 0x80000000)
                        {
    						/* Assign pairwise key info */
    						RTMPSetWcidSecurityInfo(pAd, 
												 BSS0, 
												 KeyIdx, 
												 CipherAlg, 												 
												 BSSID_WCID, 
												 SHAREDKEYTABLE);
                        }
                    }
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_ADD_WEP (id=0x%x, Len=%d-byte), Port %s Secured\n",
								pWepKey->KeyIndex, pWepKey->KeyLength,
								(wdev->PortSecured == WPA_802_1X_PORT_SECURED) ? "" : "NOT"));
				}
            }
			os_free_mem(NULL, pWepKey);
            break;
#ifdef WPA_SUPPLICANT_SUPPORT
	    case OID_SET_COUNTERMEASURES:
            if (wrq->u.data.length != sizeof(int))
                Status  = -EINVAL;
            else
            {
                int enabled = 0;
                Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);
                if (enabled == 1)
                    pAd->StaCfg.bBlockAssoc = TRUE;
                else
                    /* WPA MIC error should block association attempt for 60 seconds */
                    pAd->StaCfg.bBlockAssoc = FALSE;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_SET_COUNTERMEASURES bBlockAssoc=%s\n", pAd->StaCfg.bBlockAssoc ? "TRUE":"FALSE"));
            }
	        break;
        case RT_OID_WPA_SUPPLICANT_SUPPORT:
			if (wrq->u.data.length != sizeof(UCHAR))
                Status  = -EINVAL;
            else
            {
				Status = copy_from_user(&wpa_supplicant_enable, wrq->u.data.pointer, wrq->u.data.length);
				if (wpa_supplicant_enable & WPA_SUPPLICANT_ENABLE_WPS)
					pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
				else
				{
					pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = wpa_supplicant_enable;
					pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP &= 0x7F;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPA_SUPPLICANT_SUPPORT (=0x%02X)\n",
							pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP));
			}
            break;	   
        case OID_802_11_DEAUTHENTICATION:
            if (wrq->u.data.length != sizeof(MLME_DEAUTH_REQ_STRUCT))
                Status  = -EINVAL;
            else
            {
                MLME_DEAUTH_REQ_STRUCT      *pInfo;
				MLME_QUEUE_ELEM *MsgElem;

				os_alloc_mem(pAd, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));
                if (MsgElem == NULL)
                {
                	DBGPRINT(RT_DEBUG_ERROR, ("%s():alloc memory failed!\n", __FUNCTION__));
                        return -EINVAL;
                }

                pInfo = (MLME_DEAUTH_REQ_STRUCT *) MsgElem->Msg;
                Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
                MlmeDeauthReqAction(pAd, MsgElem);
				os_free_mem(NULL, MsgElem);
				
                if (INFRA_ON(pAd))
                {
                    LinkDown(pAd, FALSE);
                    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason));
            }
            break;
        case OID_802_11_DROP_UNENCRYPTED:
            if (wrq->u.data.length != sizeof(int))
                Status  = -EINVAL;
            else
            {
                int enabled = 0;
                Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);
                if (enabled == 1)
                    pAd->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                else
                    pAd->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_SECURED;
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pAd->MacTab.Content[BSSID_WCID].PortSecured = pAd->StaCfg.wdev.PortSecured;
				NdisReleaseSpinLock(&pAd->MacTabLock);
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DROP_UNENCRYPTED (=%d)\n", enabled));
            }
            break;
        case OID_802_11_SET_IEEE8021X:
            if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&IEEE8021xState, wrq->u.data.pointer, wrq->u.data.length);                				
		        pAd->StaCfg.wdev.IEEE8021X = IEEE8021xState;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_IEEE8021X (=%d)\n", IEEE8021xState));
            }
            break;
        case OID_802_11_SET_IEEE8021X_REQUIRE_KEY:	
			if (wrq->u.data.length != sizeof(BOOLEAN))
				 Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&IEEE8021x_required_keys, wrq->u.data.pointer, wrq->u.data.length);                				
				pAd->StaCfg.wpa_supplicant_info.IEEE8021x_required_keys = IEEE8021x_required_keys;				
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SET_IEEE8021X_REQUIRE_KEY (%d)\n", IEEE8021x_required_keys));
			}	
			break;
        case OID_802_11_PMKID:
			os_alloc_mem(pAd, (UCHAR **)&pPmkId, wrq->u.data.length);

	        if(pPmkId == NULL) {
                Status = -ENOMEM;
                break;
            }
            Status = copy_from_user(pPmkId, wrq->u.data.pointer, wrq->u.data.length);
	  	  
	        /* check the PMKID information */
	        if (pPmkId->BSSIDInfoCount == 0)
                NdisZeroMemory(pAd->StaCfg.SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
	        else
	        {
		        PBSSID_INFO	pBssIdInfo;
		        UINT		BssIdx;
		        UINT		CachedIdx;

		        for (BssIdx = 0; BssIdx < pPmkId->BSSIDInfoCount; BssIdx++)
		        {
			        /* point to the indexed BSSID_INFO structure */
			        pBssIdInfo = (PBSSID_INFO) ((PUCHAR) pPmkId + 2 * sizeof(UINT) + BssIdx * sizeof(BSSID_INFO));
			        /* Find the entry in the saved data base. */
			        for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			        {
				        /* compare the BSSID */
				        if (NdisEqualMemory(pBssIdInfo->BSSID, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, sizeof(NDIS_802_11_MAC_ADDRESS)))
					        break;			
			        }

			        /* Found, replace it */
			        if (CachedIdx < PMKID_NO)
			        {
				        DBGPRINT(RT_DEBUG_OFF, ("Update OID_802_11_PMKID, idx = %d\n", CachedIdx));
				        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
				        pAd->StaCfg.SavedPMKNum++;
			        }
			        /* Not found, replace the last one */
			        else
			        {
				        /* Randomly replace one */
				        CachedIdx = (pBssIdInfo->BSSID[5] % PMKID_NO);
				        DBGPRINT(RT_DEBUG_OFF, ("Update OID_802_11_PMKID, idx = %d\n", CachedIdx));
				        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
			        }				
		        }
			}
			if(pPmkId) 
				os_free_mem(NULL, pPmkId);
	        break;

		case RT_OID_WPS_PROBE_REQ_IE:
			if (pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe)
			{
				os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe);
				pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
			}
			pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
			os_alloc_mem(pAd, (UCHAR **)&(pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe), wrq->u.data.length);
			if (pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe)
			{
				Status = copy_from_user(pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe, wrq->u.data.pointer, wrq->u.data.length);

				if (Status)
            	{
                	Status  = -EINVAL;
					if (pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe)
					{
						os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe);
						pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
					}
					pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
                	DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPS_PROBE_REQ_IE, Failed (copy_from_user failed)!!\n"));
           		}
           		else
           		{
					pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = wrq->u.data.length;
					hex_dump("WpsProbeReqIe",
								pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe,
								pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen);
					DBGPRINT(RT_DEBUG_TRACE, ("Set::RT_OID_WPS_PROBE_REQ_IE, WpsProbeReqIeLen = %d!!\n",
								pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen));
           		}
			}
			else
				Status = -ENOMEM;
			break;
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
		case RT_OID_WSC_EAPMSG:
			{
				RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
				PUCHAR pUPnPMsg = NULL;
				UINT msgLen = 0, Machine = 0, msgType = 0;
				int retVal, senderID = 0;

				DBGPRINT(RT_DEBUG_TRACE, ("WSC::RT_OID_WSC_EAPMSG, wrq->u.data.length=%d!\n", wrq->u.data.length));
			
				msgLen = wrq->u.data.length;				
				os_alloc_mem(pAd, (UCHAR **)&pUPnPMsg, msgLen);
				if (pUPnPMsg == NULL)
					Status = -EINVAL;
				else
				{
					memset(pUPnPMsg, 0, msgLen);
					retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);
					
					msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
					senderID = *((int *)&msgHdr->Addr2);
					/*assign the STATE_MACHINE type */
					{
                        Machine = WSC_STATE_MACHINE;
						msgType = WSC_EAPOL_UPNP_MSG;
												
						retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg);
						if((retVal == FALSE) && (msgHdr->envID != 0))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("MlmeEnqueuForWsc return False and envID=0x%x!\n", msgHdr->envID));
							Status = -EINVAL;
						}
					}

					os_free_mem(NULL, pUPnPMsg);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_EAPMSG finished!\n"));
			}
			break;
        case RT_OID_WSC_SET_PROFILE:
            if (wrq->u.data.length != sizeof(WSC_PROFILE))
                Status = -EINVAL;
            else
            {
                PWSC_PROFILE pWscProfile = &pAd->StaCfg.WscControl.WscProfile;
                NdisZeroMemory(pWscProfile, sizeof(WSC_PROFILE));
                Status = copy_from_user(pWscProfile, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_PROFILE:: ProfileCnt = %d\n", pWscProfile->ProfileCnt));
            }
            break;
            
        case RT_OID_WSC_SET_CONF_MODE: /* WPS disable, Enrollee or Registrar */
            if (wrq->u.data.length != sizeof(INT))
                Status = -EINVAL;
            else
            {
                INT WscConfMode = 0;
                Status = copy_from_user(&WscConfMode, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
					if (WscConfMode == 2)
						WscConfMode = 4;

                    switch(WscConfMode)
                    {
                        case WSC_ENROLLEE:
                            Set_WscConfMode_Proc(pAd, "1");
                            break;
                        case WSC_REGISTRAR:
                            Set_WscConfMode_Proc(pAd, "2");
							WscConfMode = 2;
                            break;
                        case WSC_DISABLE:
                        default:
                            Set_WscConfMode_Proc(pAd, "0");
                            break;
                    }                    
                }
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_CONF_MODE:: WscConfMode = %d\n", WscConfMode));
            }
            break;
        case RT_OID_WSC_SET_MODE:
            if (wrq->u.data.length != sizeof(INT))
                Status = -EINVAL;
            else
            {
                INT WscMode = 0; /* PIN or PBC */
                Status = copy_from_user(&WscMode, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
                    if (WscMode == WSC_PIN_MODE) /* PIN */
                    {
                        if (Set_WscMode_Proc(pAd, "1") == FALSE)
							Status = -EINVAL;
                    }
                    else if (WscMode == WSC_PBC_MODE) /* PBC */
                    {
                        if (Set_WscMode_Proc(pAd, "2") == FALSE)
							Status = -EINVAL;
                    }
                    else if (WscMode == WSC_SMPBC_MODE) /* SMPBC */
                    {
						if (Set_WscMode_Proc(pAd, "3") == FALSE)
							Status = -EINVAL;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_MODE::unknown WscMode = %d\n", WscMode));
                        Status = -EINVAL;
                    }
                }
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_MODE::WscMode = %d\n", WscMode));
            }
            break;
        case RT_OID_WSC_SET_PIN_CODE:
            if (wrq->u.data.length != 8) /* PIN Code Length is 8 */
                Status = -EINVAL;
            else
            {
                CHAR PinCode[9] = {0};
                Status = copy_from_user(&PinCode[0], wrq->u.data.pointer, wrq->u.data.length);
                if (Status == 0)
                {
                    if (Set_WscPinCode_Proc(pAd, (PSTRING) &PinCode[0]) == FALSE)
                        Status = -EINVAL;
                }
            }
            break;
        case RT_OID_WSC_SET_SSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
                Status = -EINVAL;
            else
            {
                NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
                Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);
				Ssid.Ssid[NDIS_802_11_LENGTH_SSID-1] = 0x00;
                Set_WscSsid_Proc(pAd, (PSTRING) Ssid.Ssid);
            }
            break;
        case RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:
            if (wrq->u.data.length != sizeof(UINT))
                Status = -EINVAL;
            else
            {
                UINT wsc_profile_index = 0; /* PIN or PBC */
                PWSC_CTRL   pWscControl = &pAd->StaCfg.WscControl;
                unsigned long	IrqFlags;
                
                Status = copy_from_user(&wsc_profile_index, wrq->u.data.pointer, wrq->u.data.length);
                if (wsc_profile_index < pWscControl->WscProfile.ProfileCnt)
                {                    
                    RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
                    WscWriteConfToPortCfg(pAd, pWscControl, &pWscControl->WscProfile.Profile[wsc_profile_index], TRUE);
                    RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
                    pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
                    LinkDown(pAd, TRUE);                    
                }
                else
                    DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:: wrong wsc_profile_index(%d)\n", wsc_profile_index));                
            }
            break;
        case RT_OID_WSC_DRIVER_AUTO_CONNECT:
            if (wrq->u.data.length != sizeof(UCHAR))
                Status = -EINVAL;
            else
            {
                Status = copy_from_user(&pAd->StaCfg.WscControl.WscDriverAutoConnect, wrq->u.data.pointer, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_DRIVER_AUTO_CONNECT::WscDriverAutoConnect is %d\n", 
                                            pAd->StaCfg.WscControl.WscDriverAutoConnect));
            }
            break;
        case RT_OID_WSC_SET_PASSPHRASE:
            if (wrq->u.data.length > 64 || wrq->u.data.length < 8)
                Status = -EINVAL;
            else
            {
                Status = copy_from_user(pAd->StaCfg.WscControl.WpaPsk, wrq->u.data.pointer, wrq->u.data.length);
                NdisZeroMemory(pAd->StaCfg.WscControl.WpaPsk, 64);
                pAd->StaCfg.WscControl.WpaPskLen = wrq->u.data.length;
                DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_SET_PASSPHRASE::KeyLen(%d)\n", pAd->StaCfg.WscControl.WpaPskLen));
            }
            break;
#endif /* WSC_STA_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
		case RT_OID_LED_WPS_MODE10:
			if(!(pAd->StaCfg.bRadio) ||
				(wrq->u.data.length != sizeof(UINT)))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&WPSLedMode10, wrq->u.data.pointer, wrq->u.data.length);
				if((WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_ON) && 
					(WPSLedMode10 != LINK_STATUS_WPS_MODE10_FLASH) && 
					(WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_OFF))
				{
					Status = NDIS_STATUS_INVALID_DATA;
					DBGPRINT(RT_DEBUG_INFO, ("WPS LED Mode 10::Parameter of LED Mode 10 must be 0x00, or 0x01, or 0x02\n"));
				}
				else
				{ 
#ifdef RTMP_MAC_PCI
					RTMPSetLED(pAd, WPSLedMode10);
#endif /* RTMP_MAC_PCI */
				}
			}
			break;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef SNMP_SUPPORT
		case OID_802_11_SHORTRETRYLIMIT:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SHORTRETRYLIMIT (ShortRetryLimit=%ld)\n", ShortRetryLimit));
			}
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT \n"));
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT (LongRetryLimit=%ld)\n", LongRetryLimit));
			}
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE\n"));
			os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
			if (pKey == NULL)
			{
				Status = -EINVAL;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n"));
				break;
			}

			Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
			/*pKey = &WepKey; */
			
			if ( pKey->Length != wrq->u.data.length)
			{
				Status = -EINVAL;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n"));
			}
			KeyIdx = pKey->KeyIndex & 0x0fffffff;
			DBGPRINT(RT_DEBUG_TRACE,("pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex, pKey->KeyLength));

			/* it is a shared key */
			if (KeyIdx >= 4)
				Status = -EINVAL;
			else
			{
				pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(&pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key, &pKey->KeyMaterial, pKey->KeyLength);
				if (pKey->KeyIndex & 0x80000000)
				{
					/* Default key for tx (shared key) */
					pAd->StaCfg.DefaultKeyId = (UCHAR) KeyIdx;
				}
				/*RestartAPIsRequired = TRUE; */
			}
			os_free_mem(NULL, pKey);
			break;


		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYID \n"));

			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
				Status = copy_from_user(&pAd->StaCfg.DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

			break;


		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_CURRENTCHANNEL \n"));
			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
			{
				STRING	ChStr[5] = {0};
				Status = copy_from_user(&ctmp, wrq->u.data.pointer, wrq->u.data.length);
				snprintf(ChStr, sizeof(ChStr), "%d", ctmp);
				Set_Channel_Proc(pAd, ChStr);
			}
			break;
#endif /* SNMP_SUPPORT */


#ifdef WAPI_SUPPORT
		case OID_802_11_WAPI_PID:
			{
				unsigned long wapi_pid = 0;
    			if (copy_from_user(&pObj->wapi_pid, wrq->u.data.pointer, wrq->u.data.length))
				{
					Status = -EFAULT; 	
				}
    			else
    			{
					RTMP_GET_OS_PID(pObj->wapi_pid, wapi_pid);
					pObj->wapi_pid_nr = wapi_pid;
					DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_WAPI_PID::(WapiPid=%lu(0x%x))\n", wapi_pid, pObj->wapi_pid));
				}
    		}
			break;
		case OID_802_11_PORT_SECURE_STATE:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_PORT_SECURE_STATE, len=%d/%d\n", wrq->u.data.length, sizeof(WAPI_PORT_SECURE_STRUCT)));
			if (wrq->u.data.length != sizeof(WAPI_PORT_SECURE_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_PORT_SECURE_STRUCT  wapi_port;

				Status = copy_from_user(&wapi_port, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if (INFRA_ON(pAd))
					{						
						if (NdisEqualMemory(pAd->MlmeAux.Bssid, wapi_port.Addr, MAC_ADDR_LEN))
						{
							switch (wapi_port.state)
							{
								case WAPI_PORT_SECURED:
									pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
									pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
									RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
									break;
								
								default:
									pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
									pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
									RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
									break;								
							}	
							DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_PORT_SECURE_STATE (state=%d)\n", wapi_port.state));
						}
					}
				}				
            }
			break;

		case OID_802_11_UCAST_KEY_INFO:
			if (wrq->u.data.length != sizeof(WAPI_UCAST_KEY_STRUCT))
                Status  = -EINVAL;
            else
            {                								
				MAC_TABLE_ENTRY 		*pEntry = NULL;
				WAPI_UCAST_KEY_STRUCT   wapi_ukey;

				Status = copy_from_user(&wapi_ukey, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if (INFRA_ON(pAd))
					{						
						if (NdisEqualMemory(pAd->MlmeAux.Bssid, wapi_ukey.Addr, MAC_ADDR_LEN))
						{
							pEntry = &pAd->MacTab.Content[BSSID_WCID];
							pEntry->usk_id = wapi_ukey.key_id;
							NdisMoveMemory(pAd->StaCfg.PTK, wapi_ukey.PTK, 64);	
							NdisMoveMemory(pEntry->PTK, wapi_ukey.PTK, 64);
							
							/* Install pairwise key */
							WAPIInstallPairwiseKey(pAd, pEntry, FALSE);

							/* Start or re-start USK rekey mechanism, if necessary. */
							RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
							RTMPStartWapiRekeyTimerAction(pAd, pEntry);

							DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_UCAST_KEY_INFO complete\n"));
							hex_dump("WAPI UCAST KEY", pAd->StaCfg.PTK, 64);
						}
					}	
				}				
            }
			break;	

		case OID_802_11_MCAST_KEY_INFO:
			if (wrq->u.data.length != sizeof(WAPI_MCAST_KEY_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_MCAST_KEY_STRUCT   wapi_mkey;

				Status = copy_from_user(&wapi_mkey, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {                	
                	/* Obtain the NMK and tx_iv of AE */
                	pAd->StaCfg.DefaultKeyId = wapi_mkey.key_id;
                	/*NdisMoveMemory(pAd->StaCfg.rx_iv, wapi_mkey.m_tx_iv, LEN_WAPI_TSC); */
                	NdisMoveMemory(pAd->StaCfg.NMK, wapi_mkey.NMK, 16);

					/* Calculate GTK */
					RTMPDeriveWapiGTK(pAd->StaCfg.NMK, pAd->StaCfg.GTK);
                														
					/* Install Shared key */
					WAPIInstallSharedKey(pAd, 
										 pAd->StaCfg.GroupCipher, 
										 BSS0, 
										 pAd->StaCfg.DefaultKeyId, 
										 MCAST_WCID,
										 pAd->StaCfg.GTK);
																		
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_MCAST_KEY_INFO complete\n"));
				}				
            }
			break;								
#endif /* WAPI_SUPPORT */

#ifdef XLINK_SUPPORT
		case RT_OID_802_11_SET_PSPXLINK_MODE:
			if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&pAd->StaCfg.PSPXlink, wrq->u.data.pointer, wrq->u.data.length);
				/*if (pAd->StaCfg.PSPXlink)
					RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS)*/
				DBGPRINT(RT_DEBUG_TRACE,("Set::RT_OID_802_11_SET_PSPXLINK_MODE(=%d) \n", pAd->StaCfg.PSPXlink));
            }
			break;
#endif /* XLINK_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
		case RT_OID_802_11R_SUPPORT:
			if (wrq->u.data.length != sizeof(BOOLEAN))
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&pAd->StaCfg.Dot11RCommInfo.bFtSupport, wrq->u.data.pointer, wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Set::OID_802_11R_SUPPORT(=%d) \n", pAd->StaCfg.Dot11RCommInfo.bFtSupport));
            }
			break;
#endif /* DOT11R_FT_SUPPORT */


        default:
            DBGPRINT(RT_DEBUG_TRACE, ("Set::unknown IOCTL's subcmd = 0x%08x\n", cmd));
            Status = -EOPNOTSUPP;
            break;
    }


    return Status;
}

INT RTMPQueryInformation(
    IN  PRTMP_ADAPTER pAd,
    IN  OUT RTMP_IOCTL_INPUT_STRUCT    *rq,
    IN  INT                 cmd)
{
    RTMP_IOCTL_INPUT_STRUCT				*wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
    NDIS_802_11_BSSID_LIST_EX           *pBssidList = NULL;
    PNDIS_WLAN_BSSID_EX                 pBss;
    NDIS_802_11_SSID                    Ssid;
    NDIS_802_11_CONFIGURATION           *pConfiguration = NULL;
    RT_802_11_LINK_STATUS               *pLinkStatus = NULL;
    RT_802_11_STA_CONFIG                *pStaConfig = NULL;
    NDIS_802_11_STATISTICS              *pStatistics = NULL;
    NDIS_802_11_RTS_THRESHOLD           RtsThresh;
    NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh;
    NDIS_802_11_POWER_MODE              PowerMode;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType;
    RT_802_11_PREAMBLE                  PreamType;
    NDIS_802_11_AUTHENTICATION_MODE AuthMode;
    NDIS_802_11_WEP_STATUS WepStatus;
    NDIS_MEDIA_STATE MediaState;
    ULONG BssBufSize, ulInfo=0, NetworkTypeList[4], apsd = 0, RateValue=0;
    USHORT BssLen = 0;
    PUCHAR pBuf = NULL, pPtr;
    INT Status = NDIS_STATUS_SUCCESS;
    UINT we_version_compiled;
    UCHAR i, Padding = 0;
    BOOLEAN RadioState;
    STRING driverVersion[8];
    OID_SET_HT_PHYMODE *pHTPhyMode = NULL;
    HTTRANSMIT_SETTING	HTPhyMode;
#ifdef WSC_STA_SUPPORT
	UINT	                            WscPinCode = 0;
	PWSC_PROFILE						pProfile;
#endif /* WSC_STA_SUPPORT */

#ifdef SNMP_SUPPORT	
	DefaultKeyIdxValue			*pKeyIdxValue;
	INT							valueLen;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						tmp[64];
#endif /*SNMP */



	struct wifi_dev *wdev = &pAd->StaCfg.wdev;


    switch(cmd) 
    {
        case RT_OID_DEVICE_NAME:
            wrq->u.data.length = sizeof(pAd->nickname);
            Status = copy_to_user(wrq->u.data.pointer, pAd->nickname, wrq->u.data.length);
            break;        
        case RT_OID_VERSION_INFO:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_VERSION_INFO \n"));
			wrq->u.data.length = 8*sizeof(CHAR);
			snprintf(&driverVersion[0], sizeof(driverVersion), "%s", STA_DRIVER_VERSION);
			driverVersion[7] = '\0';
			if (copy_to_user(wrq->u.data.pointer, &driverVersion[0], wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            break;

        case OID_802_11_BSSID_LIST:
#ifdef WPA_SUPPLICANT_SUPPORT
			if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
			{
				pAd->StaCfg.wpa_supplicant_info.WpaSupplicantScanCount = 0;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID_LIST (%d BSS returned)\n",
            				pAd->ScanTab.BssNr));
			pAd->StaCfg.bSkipAutoScanConn = FALSE;
            /* Claculate total buffer size required */
            BssBufSize = sizeof(ULONG);
            
            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
            {
                /* Align pointer to 4 bytes boundary. */
                /*Padding = 4 - (pAd->ScanTab.BssEntry[i].VarIELen & 0x0003); */
                /*if (Padding == 4) */
                /*    Padding = 0; */
                BssBufSize += (sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen + Padding);
            }

            /* For safety issue, we add 256 bytes just in case */
            BssBufSize += 256;
            /* Allocate the same size as passed from higher layer */
			os_alloc_mem(pAd, (UCHAR **)&pBuf, BssBufSize);
            if(pBuf == NULL)
            {
                Status = -ENOMEM;
                break;
            }

            /* Init 802_11_BSSID_LIST_EX structure */
            NdisZeroMemory(pBuf, BssBufSize);
            pBssidList = (PNDIS_802_11_BSSID_LIST_EX) pBuf;
            pBssidList->NumberOfItems = pAd->ScanTab.BssNr;
            
            /* Calculate total buffer length */
            BssLen = 4; /* Consist of NumberOfItems */
            /* Point to start of NDIS_WLAN_BSSID_EX */
            /* pPtr = pBuf + sizeof(ULONG); */
            pPtr = (PUCHAR) &pBssidList->Bssid[0];
            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
            {
                pBss = (PNDIS_WLAN_BSSID_EX) pPtr;
                NdisMoveMemory(&pBss->MacAddress, &pAd->ScanTab.BssEntry[i].Bssid, MAC_ADDR_LEN);
                if ((pAd->ScanTab.BssEntry[i].Hidden == 1) && (pAd->StaCfg.bShowHiddenSSID == FALSE))
                {
                    /* */
					/* We must return this SSID during 4way handshaking, otherwise Aegis will failed to parse WPA infomation */
					/* and then failed to send EAPOl farame. */
					/* */
					if ((wdev->AuthMode >= Ndis802_11AuthModeWPA) && (wdev->PortSecured != WPA_802_1X_PORT_SECURED))
					{
						pBss->Ssid.SsidLength = pAd->ScanTab.BssEntry[i].SsidLen;
						NdisMoveMemory(pBss->Ssid.Ssid, pAd->ScanTab.BssEntry[i].Ssid, pAd->ScanTab.BssEntry[i].SsidLen);
					}
					else
                    	pBss->Ssid.SsidLength = 0;
                }
                else
                {
                    pBss->Ssid.SsidLength = pAd->ScanTab.BssEntry[i].SsidLen;
                    NdisMoveMemory(pBss->Ssid.Ssid, pAd->ScanTab.BssEntry[i].Ssid, pAd->ScanTab.BssEntry[i].SsidLen);
                }
                pBss->Privacy = pAd->ScanTab.BssEntry[i].Privacy;
                pBss->Rssi = pAd->ScanTab.BssEntry[i].Rssi - pAd->BbpRssiToDbmDelta;
				pBss->MinSNR = pAd->ScanTab.BssEntry[i].MinSNR;
                pBss->NetworkTypeInUse = NetworkTypeInUseSanity(&pAd->ScanTab.BssEntry[i]);
                pBss->Configuration.Length = sizeof(NDIS_802_11_CONFIGURATION);
                pBss->Configuration.BeaconPeriod = pAd->ScanTab.BssEntry[i].BeaconPeriod;  
                pBss->Configuration.ATIMWindow = pAd->ScanTab.BssEntry[i].AtimWin;
				/*NdisMoveMemory(&pBss->QBssLoad, &pAd->ScanTab.BssEntry[i].QbssLoad, sizeof(QBSS_LOAD_UI)); */

                MAP_CHANNEL_ID_TO_KHZ(pAd->ScanTab.BssEntry[i].Channel, pBss->Configuration.DSConfig);

                if (pAd->ScanTab.BssEntry[i].BssType == BSS_INFRA) 
                    pBss->InfrastructureMode = Ndis802_11Infrastructure;
                else
                    pBss->InfrastructureMode = Ndis802_11IBSS;

                NdisMoveMemory(pBss->SupportedRates, pAd->ScanTab.BssEntry[i].SupRate, pAd->ScanTab.BssEntry[i].SupRateLen);
                NdisMoveMemory(pBss->SupportedRates + pAd->ScanTab.BssEntry[i].SupRateLen,
                               pAd->ScanTab.BssEntry[i].ExtRate,
                               pAd->ScanTab.BssEntry[i].ExtRateLen);

                if (pAd->ScanTab.BssEntry[i].VarIELen == 0)
                {
                    pBss->IELength = sizeof(NDIS_802_11_FIXED_IEs);
                    NdisMoveMemory(pBss->IEs, &pAd->ScanTab.BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
                    pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
                }
                else
                {
                    pBss->IELength = (ULONG)(sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen);
                    pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
                    NdisMoveMemory(pBss->IEs, &pAd->ScanTab.BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
                    NdisMoveMemory(pBss->IEs + sizeof(NDIS_802_11_FIXED_IEs), pAd->ScanTab.BssEntry[i].VarIEs, pAd->ScanTab.BssEntry[i].VarIELen);
                    pPtr += pAd->ScanTab.BssEntry[i].VarIELen;
                }
                pBss->Length = (ULONG)(sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + pAd->ScanTab.BssEntry[i].VarIELen + Padding);

				if (RtmpOsWirelessExtVerGet() < 17)
				{
	                if ((BssLen + pBss->Length) < wrq->u.data.length)
						BssLen += pBss->Length;
	                else
	                {
	                    pBssidList->NumberOfItems = i;
	                    break;
	                }
				}
				else
					BssLen += pBss->Length;

            }

			if (RtmpOsWirelessExtVerGet() < 17)
				wrq->u.data.length = BssLen;
			else
			{
	            if (BssLen > wrq->u.data.length)
	            {
					os_free_mem(NULL, pBssidList);
	                return -E2BIG;
	            }
	            else
	                wrq->u.data.length = BssLen;
			}

            Status = copy_to_user(wrq->u.data.pointer, pBssidList, BssLen);
			os_free_mem(NULL, pBssidList);
            break;
        case OID_802_3_CURRENT_ADDRESS:
            wrq->u.data.length = MAC_ADDR_LEN;
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
            break;
        case OID_GEN_MEDIA_CONNECT_STATUS:
            if (pAd->IndicateMediaState == NdisMediaStateConnected)
                MediaState = NdisMediaStateConnected;
            else
                MediaState = NdisMediaStateDisconnected;
                
            wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
            Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
            break;   
        case OID_802_11_BSSID:
            if (INFRA_ON(pAd) || ADHOC_ON(pAd))
            {
                Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));

            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BSSID(=EMPTY)\n"));
                Status = -ENOTCONN;
            }
            break;
        case OID_802_11_SSID:
			NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
			NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
            Ssid.SsidLength = pAd->CommonCfg.SsidLen;
			memcpy(Ssid.Ssid, pAd->CommonCfg.Ssid,	Ssid.SsidLength);
            wrq->u.data.length = sizeof(NDIS_802_11_SSID);
            Status = copy_to_user(wrq->u.data.pointer, &Ssid, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SSID (Len=%d, ssid=%s)\n", Ssid.SsidLength,Ssid.Ssid));
            break;
        case RT_OID_802_11_QUERY_LINK_STATUS:
			os_alloc_mem(pAd, (UCHAR **)&pLinkStatus, sizeof(RT_802_11_LINK_STATUS));
            if (pLinkStatus)
            {
                pLinkStatus->CurrTxRate = RateIdTo500Kbps[pAd->CommonCfg.TxRate];   /* unit : 500 kbps */
                pLinkStatus->ChannelQuality = pAd->Mlme.ChannelQuality;
                pLinkStatus->RxByteCount = pAd->RalinkCounters.ReceivedByteCount;
                pLinkStatus->TxByteCount = pAd->RalinkCounters.TransmittedByteCount;
        		pLinkStatus->CentralChannel = pAd->CommonCfg.CentralChannel;
                wrq->u.data.length = sizeof(RT_802_11_LINK_STATUS);
                Status = copy_to_user(wrq->u.data.pointer, pLinkStatus, wrq->u.data.length);
				os_free_mem(NULL, pLinkStatus);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LINK_STATUS\n"));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LINK_STATUS(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case OID_802_11_CONFIGURATION:
			os_alloc_mem(pAd, (UCHAR **)&pConfiguration, sizeof(NDIS_802_11_CONFIGURATION));
            if (pConfiguration)
            {
                pConfiguration->Length = sizeof(NDIS_802_11_CONFIGURATION);
                pConfiguration->BeaconPeriod = pAd->CommonCfg.BeaconPeriod;
                pConfiguration->ATIMWindow = pAd->StaActive.AtimWin;
                MAP_CHANNEL_ID_TO_KHZ(pAd->CommonCfg.Channel, pConfiguration->DSConfig);
                wrq->u.data.length = sizeof(NDIS_802_11_CONFIGURATION);
                Status = copy_to_user(wrq->u.data.pointer, pConfiguration, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CONFIGURATION(BeaconPeriod=%ld,AtimW=%ld,Channel=%d) \n", 
                                        pConfiguration->BeaconPeriod, pConfiguration->ATIMWindow, pAd->CommonCfg.Channel));
				os_free_mem(NULL, pConfiguration);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CONFIGURATION(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
		case RT_OID_802_11_SNR_0:
			if ((pAd->StaCfg.LastSNR0 > 0))
			{
				ulInfo = ConvertToSnr(pAd, pAd->StaCfg.LastSNR0);
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo));
			}
            else
			    Status = -EFAULT;
			break;
		case RT_OID_802_11_SNR_1:
			if ((pAd->Antenna.field.RxPath	> 1) && 
                (pAd->StaCfg.LastSNR1 > 0))
			{
				ulInfo = ConvertToSnr(pAd, pAd->StaCfg.LastSNR1);
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(pAd->StaCfg.LastSNR1=%d)\n",pAd->StaCfg.LastSNR1));
			break;

#ifdef DOT11N_SS3_SUPPORT
		case RT_OID_802_11_SNR_2:
			if ((pAd->Antenna.field.RxPath	> 2) && 
                (pAd->StaCfg.LastSNR2 > 0))
			{
				ulInfo = ConvertToSnr(pAd, pAd->StaCfg.LastSNR2);
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(pAd->StaCfg.LastSNR2=%d)\n",pAd->StaCfg.LastSNR2));
			break;
#endif /* DOT11N_SS3_SUPPORT */


        case OID_802_11_RSSI_TRIGGER:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_RSSI_TRIGGER(=%ld)\n", ulInfo));
            break;
		case OID_802_11_RSSI:
        case RT_OID_802_11_RSSI:
			ulInfo = pAd->StaCfg.RssiSample.LastRssi0;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
		case RT_OID_802_11_RSSI_1:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi1;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
        case RT_OID_802_11_RSSI_2:
            ulInfo = pAd->StaCfg.RssiSample.LastRssi2;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			break;
        case OID_802_11_STATISTICS:
			os_alloc_mem(pAd, (UCHAR **)&pStatistics, sizeof(NDIS_802_11_STATISTICS));
            if (pStatistics)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS \n"));
                /* add the most up-to-date h/w raw counters into software counters */
			    NICUpdateRawCounters(pAd);
                
                /* Sanity check for calculation of sucessful count */
                if (pAd->WlanCounters.TransmittedFragmentCount.QuadPart < pAd->WlanCounters.RetryCount.QuadPart)
                    pAd->WlanCounters.TransmittedFragmentCount.QuadPart = pAd->WlanCounters.RetryCount.QuadPart;

		pStatistics->TransmittedFragmentCount.QuadPart = pAd->WlanCounters.TransmittedFragmentCount.QuadPart + pAd->WlanCounters.MulticastTransmittedFrameCount.QuadPart;;
                pStatistics->MulticastTransmittedFrameCount.QuadPart = pAd->WlanCounters.MulticastTransmittedFrameCount.QuadPart;
                pStatistics->FailedCount.QuadPart = pAd->WlanCounters.FailedCount.QuadPart;
                pStatistics->RetryCount.QuadPart = pAd->WlanCounters.RetryCount.QuadPart;
                pStatistics->MultipleRetryCount.QuadPart = pAd->WlanCounters.MultipleRetryCount.QuadPart;
                pStatistics->RTSSuccessCount.QuadPart = pAd->WlanCounters.RTSSuccessCount.QuadPart;
                pStatistics->RTSFailureCount.QuadPart = pAd->WlanCounters.RTSFailureCount.QuadPart;
                pStatistics->ACKFailureCount.QuadPart = pAd->WlanCounters.ACKFailureCount.QuadPart;
                pStatistics->FrameDuplicateCount.QuadPart = pAd->WlanCounters.FrameDuplicateCount.QuadPart;
                pStatistics->ReceivedFragmentCount.QuadPart = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
                pStatistics->MulticastReceivedFrameCount.QuadPart = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;
#ifdef DBG	
                pStatistics->FCSErrorCount = pAd->RalinkCounters.RealFcsErrCount;
#else
                pStatistics->FCSErrorCount.QuadPart = pAd->WlanCounters.FCSErrorCount.QuadPart;
                pStatistics->FrameDuplicateCount.u.LowPart = pAd->WlanCounters.FrameDuplicateCount.u.LowPart / 100;
#endif
			pStatistics->TransmittedFrameCount.QuadPart = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
			pStatistics->WEPUndecryptableCount.QuadPart = pAd->WlanCounters.WEPUndecryptableCount.QuadPart;
                wrq->u.data.length = sizeof(NDIS_802_11_STATISTICS);
                Status = copy_to_user(wrq->u.data.pointer, pStatistics, wrq->u.data.length);
				os_free_mem(NULL, pStatistics);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS(mem alloc failed)\n"));
                Status = -EFAULT;
            }
            break;

#ifdef TXBF_SUPPORT
	case RT_OID_802_11_QUERY_TXBF_TABLE:
		if (pAd->chipCap.FlgHwTxBfCap)
		{
			INT i;
			RT_802_11_TXBF_TABLE *pMacTab;

			pMacTab = (RT_802_11_TXBF_TABLE *)kmalloc(sizeof(RT_802_11_TXBF_TABLE), MEM_ALLOC_FLAG);
			if (pMacTab)
			{
				pMacTab->Num = 0;
				for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
				{
					if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
					{
						NdisMoveMemory(&pMacTab->Entry[pMacTab->Num], &pAd->MacTab.Content[i].TxBFCounters, sizeof(RT_COUNTER_TXBF));
						pMacTab->Num++;
					}
				}

				wrq->u.data.length = sizeof(RT_802_11_TXBF_TABLE);
				Status = copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length);
				os_free_mem(NULL, pMacTab);
			}
			else
			{
				Status = -EFAULT;
			}
		}
		else
			Status = -EFAULT;
		
		break;
#endif /* TXBF_SUPPORT */

        case OID_GEN_RCV_OK:
            ulInfo = pAd->Counters8023.GoodReceives;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            break;
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = pAd->Counters8023.RxNoBuffer;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            break;
        case RT_OID_802_11_PHY_MODE:
            ulInfo = (ULONG)pAd->CommonCfg.PhyMode;
            wrq->u.data.length = sizeof(ulInfo);
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PHY_MODE (=%ld)\n", ulInfo));
            break;
        case RT_OID_802_11_STA_CONFIG:
			os_alloc_mem(pAd, (UCHAR **)&pStaConfig, sizeof(RT_802_11_STA_CONFIG));
            if (pStaConfig)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG\n"));
                pStaConfig->EnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
                pStaConfig->EnableTurboRate = 0;
                pStaConfig->UseBGProtection = pAd->CommonCfg.UseBGProtection;
                pStaConfig->UseShortSlotTime = pAd->CommonCfg.bUseShortSlotTime;
                /*pStaConfig->AdhocMode = pAd->StaCfg.AdhocMode; */
                pStaConfig->HwRadioStatus = (pAd->StaCfg.bHwRadio == TRUE) ? 1 : 0;
                pStaConfig->Rsv1 = 0;
                pStaConfig->SystemErrorBitmap = pAd->SystemErrorBitmap;
                wrq->u.data.length = sizeof(RT_802_11_STA_CONFIG);
                Status = copy_to_user(wrq->u.data.pointer, pStaConfig, wrq->u.data.length);
				os_free_mem(NULL, pStaConfig);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(mem alloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case OID_802_11_RTS_THRESHOLD:
            RtsThresh = pAd->CommonCfg.RtsThreshold;
            wrq->u.data.length = sizeof(RtsThresh);
            Status = copy_to_user(wrq->u.data.pointer, &RtsThresh, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_RTS_THRESHOLD(=%ld)\n", RtsThresh));
            break;
        case OID_802_11_FRAGMENTATION_THRESHOLD:
            FragThresh = pAd->CommonCfg.FragmentThreshold;
            if (pAd->CommonCfg.bUseZeroToDisableFragment == TRUE)
                FragThresh = 0;
            wrq->u.data.length = sizeof(FragThresh);
            Status = copy_to_user(wrq->u.data.pointer, &FragThresh, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_FRAGMENTATION_THRESHOLD(=%ld)\n", FragThresh));
            break;
        case OID_802_11_POWER_MODE:
            PowerMode = pAd->StaCfg.WindowsPowerMode;
            wrq->u.data.length = sizeof(PowerMode);
            Status = copy_to_user(wrq->u.data.pointer, &PowerMode, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_POWER_MODE(=%d)\n", PowerMode));
            break;
        case RT_OID_802_11_RADIO:
            RadioState = (BOOLEAN) pAd->StaCfg.bSwRadio;
            wrq->u.data.length = sizeof(RadioState);
            Status = copy_to_user(wrq->u.data.pointer, &RadioState, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_RADIO (=%d)\n", RadioState));
            break;
        case OID_802_11_INFRASTRUCTURE_MODE:
            if (pAd->StaCfg.BssType == BSS_ADHOC)
                BssType = Ndis802_11IBSS;
            else if (pAd->StaCfg.BssType == BSS_INFRA)
                BssType = Ndis802_11Infrastructure;
            else if (pAd->StaCfg.BssType == BSS_MONITOR)
                BssType = Ndis802_11Monitor;
            else
                BssType = Ndis802_11AutoUnknown;

            wrq->u.data.length = sizeof(BssType);
            Status = copy_to_user(wrq->u.data.pointer, &BssType, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_INFRASTRUCTURE_MODE(=%d)\n", BssType));
            break;
        case RT_OID_802_11_PREAMBLE:
            PreamType = pAd->CommonCfg.TxPreamble;
            wrq->u.data.length = sizeof(PreamType);
            Status = copy_to_user(wrq->u.data.pointer, &PreamType, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PREAMBLE(=%d)\n", PreamType));
            break;
        case OID_802_11_AUTHENTICATION_MODE:
            AuthMode = pAd->StaCfg.wdev.AuthMode;
            wrq->u.data.length = sizeof(AuthMode);
            Status = copy_to_user(wrq->u.data.pointer, &AuthMode, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_AUTHENTICATION_MODE(=%d)\n", AuthMode));
            break;
        case OID_802_11_WEP_STATUS:
            WepStatus = pAd->StaCfg.wdev.WepStatus;
            wrq->u.data.length = sizeof(WepStatus);
            Status = copy_to_user(wrq->u.data.pointer, &WepStatus, wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WEP_STATUS(=%d)\n", WepStatus));
            break;
        case OID_802_11_TX_POWER_LEVEL:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPower, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_TX_POWER_LEVEL %x\n",pAd->CommonCfg.TxPower));
			break;
        case RT_OID_802_11_TX_POWER_LEVEL_1:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPowerPercentage, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_TX_POWER_LEVEL_1 (=%ld)\n", pAd->CommonCfg.TxPowerPercentage));
			break;
        case OID_802_11_NETWORK_TYPES_SUPPORTED:
			if ((pAd->RfIcType == RFIC_2850) ||
				(pAd->RfIcType == RFIC_2750) ||
				(pAd->RfIcType == RFIC_3052) ||
				(pAd->RfIcType == RFIC_3053) || 
				(pAd->RfIcType == RFIC_2853) || 
				(pAd->RfIcType == RFIC_3853) ||
				(pAd->RfIcType == RFIC_5592))
			{
				NetworkTypeList[0] = 3;                 /* NumberOfItems = 3 */
				NetworkTypeList[1] = Ndis802_11DS;      /* NetworkType[1] = 11b */
				NetworkTypeList[2] = Ndis802_11OFDM24;  /* NetworkType[2] = 11g */
				NetworkTypeList[3] = Ndis802_11OFDM5;   /* NetworkType[3] = 11a */
                wrq->u.data.length = 16;
				Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
			}
			else
			{
				NetworkTypeList[0] = 2;                 /* NumberOfItems = 2 */
				NetworkTypeList[1] = Ndis802_11DS;      /* NetworkType[1] = 11b */
				NetworkTypeList[2] = Ndis802_11OFDM24;  /* NetworkType[2] = 11g */
			    wrq->u.data.length = 12;
				Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_NETWORK_TYPES_SUPPORTED\n"));
				break;
	    case OID_802_11_NETWORK_TYPE_IN_USE:
            wrq->u.data.length = sizeof(ULONG);
			if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A))
				ulInfo = Ndis802_11OFDM5;
			else if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, (WMODE_G | WMODE_B)) ||
					WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G))
				ulInfo = Ndis802_11OFDM24;
			else
				ulInfo = Ndis802_11DS;
            Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;
        	case RT_OID_802_11_QUERY_LAST_RX_RATE:
            		ulInfo = (ULONG)pAd->LastRxRate;
            		wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", ulInfo));
			break;
		case RT_OID_802_11_QUERY_LAST_TX_RATE:
			ulInfo = (ULONG)pAd->LastTxRate;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_TX_RATE (=%lx)\n", ulInfo));
			break;
        		case RT_OID_802_11_QUERY_MAP_REAL_RX_RATE:
			RateValue=0;
			HTPhyMode.word =(USHORT) pAd->LastRxRate;
			RtmpDrvRateGet(pAd, HTPhyMode.field.MODE, HTPhyMode.field.ShortGI,
				 HTPhyMode.field.BW,HTPhyMode.field.MCS,
				 newRateGetAntenna(HTPhyMode.field.MCS, HTPhyMode.field.MODE),&RateValue);
			RateValue /= 500000;
			RateValue /= 2;
			
			wrq->u.data.length = sizeof(RateValue);
			Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", RateValue));
			break;
		case RT_OID_802_11_QUERY_MAP_REAL_TX_RATE:
			RateValue=0;
			HTPhyMode.word = (USHORT)pAd->LastTxRate;
			RtmpDrvRateGet(pAd, HTPhyMode.field.MODE, HTPhyMode.field.ShortGI,
				 HTPhyMode.field.BW,HTPhyMode.field.MCS,
				 newRateGetAntenna(HTPhyMode.field.MCS, HTPhyMode.field.MODE),&RateValue);
			RateValue /= 500000;
			RateValue /= 2;
			wrq->u.data.length = sizeof(RateValue);
			Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_LAST_TX_RATE (=%ld)\n", RateValue));
			break;
		case RT_OID_802_11_QUERY_TX_PHYMODE:
			ulInfo = (ULONG)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_TX_PHYMODE (=%lx)\n", ulInfo));
			break;
        case RT_OID_802_11_QUERY_EEPROM_VERSION:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->EepromVersion, wrq->u.data.length);
            break;
        case RT_OID_802_11_QUERY_FIRMWARE_VERSION:
            wrq->u.data.length = sizeof(ULONG);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->FirmwareVersion, wrq->u.data.length);
			break;
	    case RT_OID_802_11_QUERY_NOISE_LEVEL:
			{
			UCHAR noise = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi0,
										pAd->StaCfg.RssiSample.AvgRssi1,
										pAd->StaCfg.RssiSample.AvgRssi2) - 
										RTMPMinSnr(pAd, pAd->StaCfg.RssiSample.AvgSnr0, 
										pAd->StaCfg.RssiSample.AvgSnr1);
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer, &noise, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_NOISE_LEVEL (=%d)\n", noise));
			}
			break;
	    case RT_OID_802_11_EXTRA_INFO:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ExtraInfo, wrq->u.data.length);
	        DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_EXTRA_INFO (=%ld)\n", pAd->ExtraInfo));
	        break;
	    case RT_OID_WE_VERSION_COMPILED:
	        wrq->u.data.length = sizeof(UINT);
	        we_version_compiled = RtmpOsWirelessExtVerGet();
	        Status = copy_to_user(wrq->u.data.pointer, &we_version_compiled, wrq->u.data.length);
	        break;
		case RT_OID_802_11_QUERY_APSD_SETTING:
			apsd = (pAd->StaCfg.UapsdInfo.bAPSDCapable | (pAd->CommonCfg.bAPSDAC_BE << 1) | (pAd->CommonCfg.bAPSDAC_BK << 2)
				| (pAd->CommonCfg.bAPSDAC_VI << 3)	| (pAd->CommonCfg.bAPSDAC_VO << 4)	| (pAd->CommonCfg.MaxSPLength << 5));

			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &apsd, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_APSD_SETTING (=0x%lx,APSDCap=%d,AC_BE=%d,AC_BK=%d,AC_VI=%d,AC_VO=%d,MAXSPLen=%d)\n", 
				apsd,pAd->StaCfg.UapsdInfo.bAPSDCapable,pAd->CommonCfg.bAPSDAC_BE,pAd->CommonCfg.bAPSDAC_BK,pAd->CommonCfg.bAPSDAC_VI,pAd->CommonCfg.bAPSDAC_VO,pAd->CommonCfg.MaxSPLength));
			break;
		case RT_OID_802_11_QUERY_APSD_PSM:
			wrq->u.data.length = sizeof(ULONG);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_APSD_PSM (=%d)\n", pAd->CommonCfg.bAPSDForcePowerSave));			
			break;
		case RT_OID_802_11_QUERY_WMM:
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bWmmCapable, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_WMM (=%d)\n",	pAd->CommonCfg.bWmmCapable));
			break;


#ifdef WPA_SUPPLICANT_SUPPORT
        case RT_OID_NEW_DRIVER:
            {
                UCHAR enabled = 1;
    	        wrq->u.data.length = sizeof(UCHAR);
    	        Status = copy_to_user(wrq->u.data.pointer, &enabled, wrq->u.data.length);
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_NEW_DRIVER (=%d)\n", enabled));
            }
	        break;
        case RT_OID_WPA_SUPPLICANT_SUPPORT:
	        wrq->u.data.length = sizeof(UCHAR);
	        Status = copy_to_user(wrq->u.data.pointer,
							&pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP,
							wrq->u.data.length);
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WPA_SUPPLICANT_SUPPORT (=%d)\n",
					pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP));
	        break;
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
		case RT_OID_WSC_QUERY_STATUS:
			wrq->u.data.length = sizeof(INT);
			if (copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.WscControl.WscStatus, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_STATUS (=%d)\n", pAd->StaCfg.WscControl.WscStatus));
			break;

		case RT_OID_WSC_PIN_CODE:
			wrq->u.data.length = sizeof(UINT);
			WscPinCode = pAd->StaCfg.WscControl.WscEnrolleePinCode;
			
			if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_PIN_CODE (=%d)\n", WscPinCode));
			break;

        case RT_OID_WSC_QUERY_DEFAULT_PROFILE:
			wrq->u.data.length = sizeof(WSC_PROFILE);
			os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));
			if (pProfile == NULL)
			{
				Status = -EFAULT;
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_QUERY_DEFAULT_PROFILE fail!\n"));
				break;
			}

			RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
            WscCreateProfileFromCfg(pAd, STA_MODE, &pAd->StaCfg.WscControl, pProfile);
			if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WSC Profile:\n"));
				DBGPRINT(RT_DEBUG_TRACE, ("SSID = %s\n", pProfile->Profile[0].SSID.Ssid));
				DBGPRINT(RT_DEBUG_TRACE, ("AuthType = %s\n", WscGetAuthTypeStr(pProfile->Profile[0].AuthType)));
				DBGPRINT(RT_DEBUG_TRACE, ("EncrpType = %s\n", WscGetEncryTypeStr(pProfile->Profile[0].EncrType)));

				if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WEP Key = %s\n", pProfile->Profile[0].Key));
					DBGPRINT(RT_DEBUG_TRACE, ("DefaultKey ID = %d\n", pProfile->Profile[0].KeyIndex));
				}
				else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) || (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PassPhrase Key = %s\n", pProfile->Profile[0].Key));
                    pProfile->Profile[0].KeyIndex = 1;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("\n"));
			}

			os_free_mem(NULL, pProfile);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_DEFAULT_PROFILE \n"));
			break;
		case RT_OID_802_11_WSC_QUERY_PROFILE:
			wrq->u.data.length = sizeof(WSC_PROFILE);
			os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));
			if (pProfile == NULL)
			{
				Status = -EFAULT;
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_802_11_WSC_QUERY_PROFILE fail!\n"));
				break;
			}

			RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
			NdisMoveMemory(pProfile, &pAd->StaCfg.WscControl.WscProfile, sizeof(WSC_PROFILE));
            if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE))
            {
                pProfile->Profile[0].KeyLength = 0;
                NdisZeroMemory(pProfile->Profile[0].Key, 64);
            }
			if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WSC Profile:\n"));
				DBGPRINT(RT_DEBUG_TRACE, ("SSID = %s\n", pProfile->Profile[0].SSID.Ssid));
				DBGPRINT(RT_DEBUG_TRACE, ("AuthType = %s\n", WscGetAuthTypeStr(pProfile->Profile[0].AuthType)));
				DBGPRINT(RT_DEBUG_TRACE, ("EncrpType = %s\n", WscGetEncryTypeStr(pProfile->Profile[0].EncrType)));

				if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WEP Key = %s\n", pProfile->Profile[0].Key));
					DBGPRINT(RT_DEBUG_TRACE, ("DefaultKey ID = %d\n", pProfile->Profile[0].KeyIndex));
				}
				else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) || (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PassPhrase Key = %s\n", pProfile->Profile[0].Key));
                    pProfile->Profile[0].KeyIndex = 1;
				}
				DBGPRINT(RT_DEBUG_TRACE, ("\n"));
			}

			os_free_mem(NULL, pProfile);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WSC_QUERY_PROFILE \n"));
			break;
			
		case RT_OID_WSC_UUID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_UUID \n"));
			wrq->u.data.length = UUID_LEN_STR;
			if (copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.WscControl.Wsc_Uuid_Str[0], UUID_LEN_STR))
			{
				Status = -EFAULT;
			}
			break;
		case RT_OID_WSC_MAC_ADDRESS:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_MAC_ADDRESS \n"));
			wrq->u.data.length = MAC_ADDR_LEN;
			if (copy_to_user(wrq->u.data.pointer, pAd->CurrentAddress, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;
#endif /* WSC_STA_SUPPORT */
        case RT_OID_DRIVER_DEVICE_NAME:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_DRIVER_DEVICE_NAME \n"));
			wrq->u.data.length = 16;
			if (copy_to_user(wrq->u.data.pointer, pAd->StaCfg.dev_name, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
            break;
        case RT_OID_802_11_QUERY_HT_PHYMODE:
		os_alloc_mem(pAd, (UCHAR **)&pHTPhyMode, sizeof(OID_SET_HT_PHYMODE));
		if (pHTPhyMode)
		{           
			pHTPhyMode->PhyMode = pAd->CommonCfg.PhyMode;
			pHTPhyMode->HtMode = (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE;
			pHTPhyMode->BW = (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.BW;
			pHTPhyMode->MCS= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS;
			pHTPhyMode->SHORTGI= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI;
			pHTPhyMode->STBC= (UCHAR)pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC;

			pHTPhyMode->ExtOffset = ((pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel) ? (EXTCHA_BELOW) : (EXTCHA_ABOVE));
			wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);
			if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
				Status = -EFAULT;
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
				pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset));
			DBGPRINT(RT_DEBUG_TRACE, (" %s(): (.word = %x )\n",
					__FUNCTION__, pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word));
			os_free_mem(NULL, pHTPhyMode);
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(mem alloc failed)\n"));
			Status = -EFAULT;
		}
		break;
        case RT_OID_802_11_COUNTRY_REGION:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_COUNTRY_REGION \n"));
			wrq->u.data.length = sizeof(ulInfo);
            ulInfo = pAd->CommonCfg.CountryRegionForABand;
            ulInfo = (ulInfo << 8)|(pAd->CommonCfg.CountryRegion);
			if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            break;
        case RT_OID_802_11_QUERY_DAT_HT_PHYMODE:
		os_alloc_mem(pAd, (UCHAR **)&pHTPhyMode, sizeof(OID_SET_HT_PHYMODE));
            if (pHTPhyMode)
            {           
                pHTPhyMode->PhyMode = wmode_2_cfgmode(pAd->CommonCfg.PhyMode);
    			pHTPhyMode->HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
    			pHTPhyMode->BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
    			pHTPhyMode->MCS= (UCHAR)pAd->StaCfg.wdev.DesiredTransmitSetting.field.MCS;
    			pHTPhyMode->SHORTGI= (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;
    			pHTPhyMode->STBC= (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
    	
                wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);
                if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
    				Status = -EFAULT;
    			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
    				pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset));
    			DBGPRINT(RT_DEBUG_TRACE, ("%s(): (.word = %x )\n",
						__FUNCTION__, pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word));
			os_free_mem(NULL, pHTPhyMode);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_STA_CONFIG(mem alloc failed)\n"));
                Status = -EFAULT;
            }
            break;
        case RT_OID_QUERY_MULTIPLE_CARD_SUPPORT:            
			wrq->u.data.length = sizeof(UCHAR);
            i = 0;
#ifdef MULTIPLE_CARD_SUPPORT
            i = 1;
#endif /* MULTIPLE_CARD_SUPPORT */
			if (copy_to_user(wrq->u.data.pointer, &i, wrq->u.data.length))
            {
				Status = -EFAULT;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_QUERY_MULTIPLE_CARD_SUPPORT(=%d) \n", i));
            break;
#ifdef SNMP_SUPPORT
		case RT_OID_802_11_MAC_ADDRESS:
            wrq->u.data.length = MAC_ADDR_LEN;
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREROUI:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREROUI \n"));
			wrq->u.data.length = ManufacturerOUI_LEN;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTURERNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTURERNAME \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case RT_OID_802_11_RESOURCETYPEIDNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_RESOURCETYPEIDNAME \n"));
			wrq->u.data.length = strlen(ResourceTypeIdName);
			Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
			break;

		case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED \n"));
			ulInfo = 1; /* 1 is support wep else 2 is not support. */
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case RT_OID_802_11_POWERMANAGEMENTMODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_POWERMANAGEMENTMODE \n"));
			if (pAd->StaCfg.Psm == PSMP_ACTION)
				ulInfo = 1; /* 1 is power active else 2 is power save. */
			else
				ulInfo = 2;
			
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WEPDEFAULTKEYVALUE \n"));
			pKeyIdxValue = wrq->u.data.pointer;
			DBGPRINT(RT_DEBUG_TRACE,("KeyIdxValue.KeyIdx = %d, \n",pKeyIdxValue->KeyIdx));
			valueLen = pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen;
			NdisMoveMemory(pKeyIdxValue->Value,
						   &pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key,
						   valueLen);
			pKeyIdxValue->Value[valueLen]='\0';

			wrq->u.data.length = sizeof(DefaultKeyIdxValue);

			Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE,("DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x \n", 
										pAd->StaCfg.DefaultKeyId, 
										wrq->u.data.length, 
										pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen,
										pAd->SharedKey[BSS0][0].Key[0],
										pAd->SharedKey[BSS0][1].Key[0],
										pAd->SharedKey[BSS0][2].Key[0],
										pAd->SharedKey[BSS0][3].Key[0]));
			break;

		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPDEFAULTKEYID \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.DefaultKeyId, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("DefaultKeyId =%d \n", pAd->StaCfg.DefaultKeyId));
			break;

		case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPKEYMAPPINGLENGTH \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer,
									&pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen,
									wrq->u.data.length);
			break;

		case OID_802_11_SHORTRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SHORTRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			ShortRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT);
			DBGPRINT(RT_DEBUG_TRACE, ("ShortRetryLimit =%ld\n", ShortRetryLimit));
			Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_LONGRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			LongRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG);
			DBGPRINT(RT_DEBUG_TRACE, ("LongRetryLimit =%ld\n", LongRetryLimit));
			Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
			break;
			
		case RT_OID_802_11_PRODUCTID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRODUCTID \n"));
		
#ifdef RTMP_MAC_PCI
			{
			
				USHORT  device_id;
				if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
			    	pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
				else 
					DBGPRINT(RT_DEBUG_TRACE, (" pci_dev = NULL\n"));
				snprintf((PSTRING)tmp, sizeof(tmp), "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
			}
#endif /* RTMP_MAC_PCI */
			wrq->u.data.length = strlen((PSTRING)tmp);
			Status = copy_to_user(wrq->u.data.pointer, tmp, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREID \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CURRENTCHANNEL \n"));
			wrq->u.data.length = sizeof(UCHAR);
			DBGPRINT(RT_DEBUG_TRACE, ("sizeof UCHAR=%d, channel=%d \n", sizeof(UCHAR), pAd->CommonCfg.Channel));
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Channel, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;
#endif /*SNMP_SUPPORT */
		
		case OID_802_11_BUILD_CHANNEL_EX:
			{
				UCHAR value;
				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_BUILD_CHANNEL_EX \n"));
				wrq->u.data.length = sizeof(UCHAR);
#ifdef EXT_BUILD_CHANNEL_LIST
				DBGPRINT(RT_DEBUG_TRACE, ("Support EXT_BUILD_CHANNEL_LIST.\n"));
				value = 1;
#else
				DBGPRINT(RT_DEBUG_TRACE, ("Doesn't support EXT_BUILD_CHANNEL_LIST.\n"));
				value = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
				Status = copy_to_user(wrq->u.data.pointer, &value, 1);
				DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			}
			break;

		case OID_802_11_GET_CH_LIST:
			{
				PRT_CHANNEL_LIST_INFO pChListBuf;

				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_CH_LIST \n"));
				if (pAd->ChannelListNum == 0)
				{
					wrq->u.data.length = 0;
					break;
				}

				os_alloc_mem(pAd, (UCHAR **)&pChListBuf, sizeof(RT_CHANNEL_LIST_INFO));
				if (pChListBuf == NULL)
				{
					wrq->u.data.length = 0;
					break;
				}

				pChListBuf->ChannelListNum = pAd->ChannelListNum;
				for (i = 0; i < pChListBuf->ChannelListNum; i++)
					pChListBuf->ChannelList[i] = pAd->ChannelList[i].Channel;

				wrq->u.data.length = sizeof(RT_CHANNEL_LIST_INFO);
				Status = copy_to_user(wrq->u.data.pointer, pChListBuf, sizeof(RT_CHANNEL_LIST_INFO));
				DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));

				if (pChListBuf)
					os_free_mem(NULL, pChListBuf);
			}
			break;

		case OID_802_11_GET_COUNTRY_CODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_COUNTRY_CODE \n"));
			wrq->u.data.length = 2;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.CountryCode, 2);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;

#ifdef EXT_BUILD_CHANNEL_LIST
		case OID_802_11_GET_CHANNEL_GEOGRAPHY:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_GET_CHANNEL_GEOGRAPHY \n"));
			wrq->u.data.length = 1;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Geography, 1);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;			
#endif /* EXT_BUILD_CHANNEL_LIST */



#ifdef QOS_DLS_SUPPORT
		case RT_OID_802_11_QUERY_DLS:
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bDLSCapable, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_DLS(=%d)\n", pAd->CommonCfg.bDLSCapable));
			break;

		case RT_OID_802_11_QUERY_DLS_PARAM:
			{
				PRT_802_11_DLS_INFO	pDlsInfo;
				os_alloc_mem(pAd, (UCHAR **)&pDlsInfo, sizeof(RT_802_11_DLS_INFO));
				if (pDlsInfo == NULL)
					break;

				for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
				{
					RTMPMoveMemory(&pDlsInfo->Entry[i], &pAd->StaCfg.DLSEntry[i], sizeof(RT_802_11_DLS_UI));
				}

				pDlsInfo->num = MAX_NUM_OF_DLS_ENTRY;
				wrq->u.data.length = sizeof(RT_802_11_DLS_INFO);
				Status = copy_to_user(wrq->u.data.pointer, pDlsInfo, wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_QUERY_DLS_PARAM\n"));

				if (pDlsInfo)
					os_free_mem(NULL, pDlsInfo);
			}
			break;
#endif /* QOS_DLS_SUPPORT */
#ifdef WAPI_SUPPORT
		case OID_802_11_WAPI_CONFIGURATION:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::Get WAPI Configuration(%d)\n", sizeof(WAPI_CONF)));
			RTMPIoctlQueryWapiConf(pAd, wrq);	
			break;			
		case OID_802_11_WAPI_IE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WAPI_IE\n"));
			if (wrq->u.data.length != sizeof(WAPI_WIE_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_WIE_STRUCT   wapi_ie;
				MAC_TABLE_ENTRY		*pEntry;

				NdisZeroMemory(&wapi_ie, sizeof(WAPI_WIE_STRUCT));
				NdisMoveMemory(wapi_ie.addr, wrq->u.data.pointer, MAC_ADDR_LEN);

				pEntry = &pAd->MacTab.Content[BSSID_WCID];
						
				if ((NdisEqualMemory(pEntry->Addr, wapi_ie.addr, MAC_ADDR_LEN)) && 
					(pEntry->RSNIE_Len > 0))
				{										
					wapi_ie.wie_len = pEntry->RSNIE_Len;
					NdisMoveMemory(wapi_ie.wie, pEntry->RSN_IE, pEntry->RSNIE_Len);						
				}
								
				if (copy_to_user(wrq->u.data.pointer, &wapi_ie, wrq->u.data.length))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
				}								
            }
			break;		
#endif /* WAPI_SUPPORT */

#ifdef XLINK_SUPPORT
		case OID_802_11_SET_PSPXLINK_MODE:
			wrq->u.data.length = sizeof(BOOLEAN);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.PSPXlink, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SET_PSPXLINK_MODE(=%d)\n", pAd->StaCfg.PSPXlink));
			break;
#endif /* XLINK_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
#ifdef WFD_SUPPORT
		/*
		 * Query WFD TDLS connection status
		 * Value:
		 * 	0: TDLS connecting or no link
		 * 	4: TDLS connected
		 * 	6: Peer PC (Preferred Connectivity) bit of WFD device is P2P
		 * 	7: TDLS weak security
		*/
		case RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS:
			{
				INT i;
				UCHAR bFound = CONNECTING_OR_NO_LINK;

				if (pAd->StaCfg.WfdCfg.PeerPC == WFD_PC_P2P)
				{
					bFound = WFD_PEER_PC_P2P;
					pAd->StaCfg.WfdCfg.PeerPC = WFD_PC_TDLS;
					DBGPRINT(RT_DEBUG_ERROR, ("RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS::Peer WFD PC is P2P!\n"));
				}
				else if (pAd->StaCfg.WfdCfg.TdlsSecurity == WFD_TDLS_WEAK_SECURITY)
				{
					bFound = WFD_PEER_TDLS_WEAK_SECURITY;
					pAd->StaCfg.WfdCfg.TdlsSecurity = WFD_TDLS_STRONG_SECURITY;
					DBGPRINT(RT_DEBUG_ERROR, ("RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS::TDLS weak security!\n"));
				}
				else
				{
					for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
					{
						if ((pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
							break;
					}
					if (i == MAX_NUM_OF_TDLS_ENTRY)
						bFound = CONNECTING_OR_NO_LINK;
					else
						bFound = TDLS_LINKED;
					if (bFound == TDLS_LINKED)
						DBGPRINT(RT_DEBUG_ERROR, ("RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS:: TDLS Connected!\n"));
				}
				wrq->u.data.length = sizeof(UCHAR);
				Status = copy_to_user(wrq->u.data.pointer, &bFound, wrq->u.data.length);
			}
			break;
			
			/*
			 * Query peer WFD TDLS IP address
			 * 	Field		  | Size (octets)  | Value	    | Description
			 * 	---------------------------------------------------------------------------------------------
			 * 	Version	  | 1			 | 1		    | Version 1: IPv4 address field follows
			 * 	IPv4 address  | 4			 | (IP address) | This field is the IPv4 host address of the STA
			*/
		case RT_OID_802_11_QUERY_WFD_TDLS_PEER_IP_ADDR:
			{
				INT i;
				UCHAR peer_ip_addr[5] = {0};
				for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
				{
					if ((pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
					{
						DBGPRINT(RT_DEBUG_ERROR, ("Peer IP Addr = %d.%d.%d.%d\n", 
								pAd->StaCfg.TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[1], 
								pAd->StaCfg.TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[2],
								pAd->StaCfg.TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[3],
								pAd->StaCfg.TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[4]));
						RTMPMoveMemory(&peer_ip_addr, &pAd->StaCfg.TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie, sizeof(peer_ip_addr));
						break;
					}
				}
				wrq->u.data.length = sizeof(peer_ip_addr);
				Status = copy_to_user(wrq->u.data.pointer, &peer_ip_addr, wrq->u.data.length);
			}
			break;
#endif /* WFD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
		case OID_802_11_QUERY_WirelessMode:
			wrq->u.data.length = sizeof(UCHAR);
	    	Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.PhyMode, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_QUERY_WirelessMode(=%d)\n", pAd->CommonCfg.PhyMode));
			break;
#endif /* RTMP_RBUS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
		case OID_802_11R_SUPPORT:
			wrq->u.data.length = sizeof(BOOLEAN);
            Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.Dot11RCommInfo.bFtSupport, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11R_SUPPORT(=%d)\n", pAd->StaCfg.Dot11RCommInfo.bFtSupport));
			break;

		case  OID_802_11R_MDID:
			wrq->u.data.length = 2;
			memset(wrq->u.data.pointer,0x00,2);
			Status = copy_to_user(wrq->u.data.pointer, pAd->StaCfg.Dot11RCommInfo.MdIeInfo.MdId, 2);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11R_MDID(=%x%x)\n", 
										pAd->StaCfg.Dot11RCommInfo.MdIeInfo.MdId[0],
										pAd->StaCfg.Dot11RCommInfo.MdIeInfo.MdId[1]));
			break;
#endif /* DOT11R_FT_SUPPORT */


#ifdef IWSC_SUPPORT
		case RT_OID_IWSC_SELF_IPV4:
			if (wrq->u.data.length != sizeof(UINT32))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Query Failed::RT_OID_IWSC_SELF_IPV4(=%d)\n", wrq->u.data.length));
                Status  = -EINVAL;
			}
            else
            {
            	Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.IWscInfo.SelfIpv4Addr, sizeof(UINT32));
            	DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_IWSC_SELF_IPV4(=0x%08x), Status = %d\n", 
					pAd->StaCfg.IWscInfo.SelfIpv4Addr, Status));
            }
			break;

		case RT_OID_IWSC_REGISTRAR_IPV4:
			if (wrq->u.data.length != sizeof(UINT32))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Query Failed::RT_OID_IWSC_REGISTRAR_IPV4(=%d)\n", wrq->u.data.length));
                Status  = -EINVAL;
			}
            else
            {
            	Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.IWscInfo.RegIpv4Addr, sizeof(UINT32));
            	DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_IWSC_REGISTRAR_IPV4(=0x%08x), Status = %d\n", 
					pAd->StaCfg.IWscInfo.RegIpv4Addr, Status));
            }
			break;

		case RT_OID_IWSC_SMPBC_ENROLLEE_COUNT:
			if (wrq->u.data.length != sizeof(UINT8))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Query Failed::RT_OID_IWSC_SMPBC_ENROLLEE_COUNT(=%d)\n", wrq->u.data.length));
                Status  = -EINVAL;
			}
            else
            {
            	Status = copy_to_user(wrq->u.data.pointer, &pAd->StaCfg.IWscInfo.SmpbcEnrolleeCount, sizeof(UINT8));
            	DBGPRINT(RT_DEBUG_INFO, ("Query::RT_OID_IWSC_SMPBC_ENROLLEE_COUNT(=0x%08x), Status = %d\n", 
					pAd->StaCfg.IWscInfo.SmpbcEnrolleeCount, Status));
            }
			break;
#endif /* IWSC_SUPPORT */
#ifdef CONFIG_WIFI_TEST
		case OID_WIFI_TEST_BBP:

			break;
		case OID_WIFI_TEST_BBP32:
			{
				UINT32 Index;
				UINT32 j = 0;
				struct bbp32_info *Info;

				os_alloc_mem(pAd, (UCHAR **)&pBuf, wrq->u.data.length);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}				

				Status = copy_from_user(pBuf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct bbp32_info *)pBuf;

				printk("Info->bbp_start = %x\n",  Info->bbp_start);
				printk("Info->bbp_end = %x\n",  Info->bbp_end);
				
				
				for (Index = Info->bbp_start; Index <= Info->bbp_end; Index += 4)
				{
					UINT32 Value;
					RTMP_IO_READ32(pAd, Index + pAd->chipCap.BBPMemMapOffset, &Value);
					printk("Offset = %x\n", Index + pAd->chipCap.BBPMemMapOffset);
					printk("Value = %x\n", Value);
					NdisMoveMemory(Info->bbp_value + j, &Value, 4);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}


			break;
		case OID_WIFI_TEST_RF:

			break;
#ifdef RLT_RF
		case OID_WIFI_TEST_RF_BANK:
			{
				UINT16 Index;
				UINT16 j = 0;
				struct rf_bank_info *Info;

				os_alloc_mem(pAd, (UCHAR **)&pBuf, wrq->u.data.length);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}				

				Status = copy_from_user(pBuf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct rf_bank_info *)pBuf;

				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_bank = %x\n", Info->rf_bank));
				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_start = %x\n",  Info->rf_start));
				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_end = %x\n",  Info->rf_end));
				
				for (Index = Info->rf_start; Index <= Info->rf_end; Index ++)
				{
					UINT8 Value;
					rlt_rf_read(pAd, Info->rf_bank, Index, &Value);
					DBGPRINT(RT_DEBUG_OFF, ("Offset = %x\n", Index));
					DBGPRINT(RT_DEBUG_OFF, ("Value = %x\n", Value));
					NdisMoveMemory(Info->rf_value + j, &Value, 1);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}
			break;
#endif
#ifdef MT_RF
		case OID_WIFI_TEST_RF_INDEX:
			{
				UINT16 Index;
				UINT16 j = 0;
				struct rf_index_info *Info;
				char *buf;

				os_alloc_mem(pAd, (UCHAR **)&buf, wrq->u.data.length);

				if (!buf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("memory is not available\n"));
					break;
				}				

				Status = copy_from_user(buf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct rf_bank_info *)buf;

				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_index = %x\n", Info->rf_index));
				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_start = %x\n",  Info->rf_start));
				DBGPRINT(RT_DEBUG_OFF, ("Info->rf_end = %x\n",  Info->rf_end));
				
				for (Index = Info->rf_start; Index <= Info->rf_end; Index += 4)
				{
					UINT32 Value;
					mt_rf_read(pAd, Info->rf_index, Index, &Value);
					DBGPRINT(RT_DEBUG_OFF, ("Offset = %x\n", Index));
					DBGPRINT(RT_DEBUG_OFF, ("Value = %x\n", Value));
					NdisMoveMemory(Info->rf_value + j, &Value, 4);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, buf, wrq->u.data.length);

				os_free_mem(NULL, buf);
			}
			break;
#endif
		case OID_WIFI_TEST_MEM_MAP_INFO:
			{
				UINT16 Index;
				UINT16 j = 0;
				struct mem_map_info *Info;

				os_alloc_mem(pAd, (UCHAR **)&pBuf, wrq->u.data.length);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}				

				Status = copy_from_user(pBuf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct mem_map_info *)pBuf;

				printk("Info->base = %x\n", Info->base);
				printk("Info->mem_map_start = %x\n",  Info->mem_map_start);
				printk("Info->mem_map_end = %x\n",  Info->mem_map_end);
				
				
				for (Index = Info->mem_map_start; Index <= Info->mem_map_end; Index += 4)
				{
					UINT32 Value;
					read_reg(pAd, Info->base, Index, &Value);
					NdisMoveMemory(Info->mem_map_value + j, &Value, 4);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}
			break;
		case OID_WIFI_TEST_E2P:
			{
				UINT16 Index;
				UINT16 j = 0;
				struct e2p_info *Info;

				os_alloc_mem(pAd, (UCHAR **)&pBuf, wrq->u.data.length);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}				

				Status = copy_from_user(pBuf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct e2p_info *)pBuf;

				printk("Info->e2p_start = %x\n",  Info->e2p_start);
				printk("Info->e2p_end = %x\n",  Info->e2p_end);
				
				
				for (Index = Info->e2p_start; Index <= Info->e2p_end; Index += 2)
				{
					UINT16 Value;
					RT28xx_EEPROM_READ16(pAd, Index, Value);
					NdisMoveMemory(Info->e2p_value + j, &Value, 2);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}
			break;
		case OID_WIFI_TEST_MAC:
			{
				UINT32 Index;
				UINT32 j = 0;
				struct mac_info *Info;

				os_alloc_mem(pAd, (UCHAR **)&pBuf, wrq->u.data.length);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}				

				Status = copy_from_user(pBuf, wrq->u.data.pointer, wrq->u.data.length);
				Info = (struct mac_info *)pBuf;

				printk("Info->mac_start = %x\n",  Info->mac_start);
				printk("Info->mac_end = %x\n",  Info->mac_end);
				
				
				for (Index = Info->mac_start; Index <= Info->mac_end; Index += 4)
				{
					UINT32 Value;
					RTMP_IO_READ32(pAd, Index + pAd->chipCap.MacMemMapOffset, &Value);
					printk("Offset = %x\n", Index + pAd->chipCap.MacMemMapOffset);
					printk("Value = %x\n", Value);
					NdisMoveMemory(Info->mac_value + j, &Value, 4);
					j++;
				}

				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}
			break;
		case OID_WIFI_TEST_BBP_NUM:
			{
				struct bbp32_info Info;
				Info.bbp_start = pAd->chipCap.BBPStart;
				Info.bbp_end = pAd->chipCap.BBPEnd;
				wrq->u.data.length = sizeof(Info);
				Status = copy_to_user(wrq->u.data.pointer, &Info, wrq->u.data.length);
			}
			break;
		case OID_WIFI_TEST_RF_NUM:

			break;
#ifdef RLT_RF
		case OID_WIFI_TEST_RF_BANK_OFFSET:
			{
				struct rf_bank_info *Info;
				struct RF_BANK_OFFSET *Offset;
				UINT8 Index;
				
				os_alloc_mem(pAd, (UCHAR **)&pBuf, sizeof(*Info) * pAd->chipCap.RFBankNum);

				if (!pBuf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}

				Info = (struct rf_bank_info *)pBuf;
				Offset = pAd->chipCap.RFBankOffset;
	
				DBGPRINT(RT_DEBUG_OFF, ("pAd->chipCap.RFBankNum = %d\n", pAd->chipCap.RFBankNum));

				for (Index = 0; Index < pAd->chipCap.RFBankNum; Index++)
				{
					Info->rf_bank = Offset->RFBankIndex;
					Info->rf_start = Offset->RFStart;
					Info->rf_end =Offset->RFEnd;
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_bank = %d\n", Info->rf_bank));
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_start = %x\n", Info->rf_start));
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_end = %x\n", Info->rf_end));
					Info++;
					Offset++;
				}

				wrq->u.data.length = sizeof(*Info) * pAd->chipCap.RFBankNum; 
				Status = copy_to_user(wrq->u.data.pointer, pBuf, wrq->u.data.length);

				os_free_mem(NULL, pBuf);
			}
			break;
#endif
#ifdef MT_RF
		case OID_WIFI_TEST_RF_INDEX_OFFSET:
			{
				struct rf_index_info *Info;
				struct RF_INDEX_OFFSET *Offset;
				UINT8 Index;
				char *buf;
				
				os_alloc_mem(pAd, (UCHAR **)&buf, sizeof(*Info) * pAd->chipCap.RFIndexNum);

				if (!buf)
				{
					Status = -EINVAL;
					DBGPRINT(RT_DEBUG_ERROR, ("Memory is not available\n"));
					break;
				}

				Info = (struct rf_index_info *)buf;
				Offset = pAd->chipCap.RFIndexOffset;
	
				DBGPRINT(RT_DEBUG_OFF, ("pAd->chipCap.RFIndexNum = %d\n", pAd->chipCap.RFIndexNum));

				for (Index = 0; Index < pAd->chipCap.RFIndexNum; Index++)
				{
					Info->rf_index = Offset->RFIndex;
					Info->rf_start = Offset->RFStart;
					Info->rf_end =Offset->RFEnd;
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_index = %d\n", Info->rf_index));
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_start = %x\n", Info->rf_start));
					DBGPRINT(RT_DEBUG_OFF, ("Info->rf_end = %x\n", Info->rf_end));
					Info++;
					Offset++;
				}

				wrq->u.data.length = sizeof(*Info) * pAd->chipCap.RFIndexNum; 
				Status = copy_to_user(wrq->u.data.pointer, buf, wrq->u.data.length);

				os_free_mem(NULL, buf);
			}
			break;
#endif
		case OID_WIFI_TEST_MAC_NUM:
			{
				struct mac_info Info;
				Info.mac_start = pAd->chipCap.MacStart;
				Info.mac_end = pAd->chipCap.MacEnd;
				wrq->u.data.length = sizeof(Info);
				Status = copy_to_user(wrq->u.data.pointer, &Info, wrq->u.data.length);
			}
			break;
		case OID_WIFI_TEST_E2P_NUM:
			{
				struct e2p_info Info;
				Info.e2p_start = pAd->chipCap.E2PStart;
				Info.e2p_end = pAd->chipCap.E2PEnd;
				wrq->u.data.length = sizeof(Info);
				Status = copy_to_user(wrq->u.data.pointer, &Info, wrq->u.data.length);
			}
			break;		
		case OID_WIFI_TEST_MEM_MAP_NUM:
			{
				struct mem_map_info Info;
				Info.mem_map_start = pAd->chipCap.MemMapStart;
				Info.mem_map_end = pAd->chipCap.MemMapEnd;
				wrq->u.data.length = sizeof(Info);
				Status = copy_to_user(wrq->u.data.pointer, &Info, wrq->u.data.length);
			}
			break;
		case OID_WIFI_TEST_PHY_MODE:
			{
				struct phy_mode_info info;
				info.data_phy = pAd->fpga_ctl.rx_data_phy;
				info.data_bw = pAd->fpga_ctl.rx_data_bw;
				info.data_ldpc = pAd->fpga_ctl.rx_data_ldpc;
				info.data_mcs = pAd->fpga_ctl.rx_data_mcs;
				info.data_gi = pAd->fpga_ctl.rx_data_gi;
				info.data_stbc =  pAd->fpga_ctl.rx_data_stbc;
				wrq->u.data.length = sizeof(info);
				Status = copy_to_user(wrq->u.data.pointer, &info, wrq->u.data.length);
			}
			break;
#endif
    default:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::unknown IOCTL's subcmd = 0x%08x\n", cmd));
            Status = -EOPNOTSUPP;
            break;
    }
    return Status;
}


#ifdef DBG
/* 
    ==========================================================================
    Description:
        Read / Write MAC
    Arguments:
        pAd                    Pointer to our adapter
        wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
               2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPIoctlMAC(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	PSTRING this_char, value;
	INT j = 0, k = 0;
	STRING *msg = NULL;
	STRING *arg = NULL;
	UINT32 macAddr = 0, macValue = 0;
	UCHAR temp[16];
	STRING temp2[16];
	INT Status;
	BOOLEAN bIsPrintAllMAC = FALSE;



	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(STRING)*1024);
	if (!msg)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelOK;
	}
	memset(msg, 0x00, 1024);

	os_alloc_mem(NULL, (UCHAR **)&arg, sizeof(STRING)*255);
	if (arg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelOK;
	}
	memset(arg, 0x00, 255);

DBGPRINT(RT_DEBUG_OFF, ("%s():wrq->u.data.length=%d, wrq->u.data.pointer=%s!\n", __FUNCTION__, wrq->u.data.length, wrq->u.data.pointer));
	if (wrq->u.data.length > 1)
	{   
		Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		arg[254] = 0x00;
		sprintf(msg, "\n");
		
		/*Parsing Read or Write */
		this_char = arg;
		if (!*this_char)
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!value || !*value)
		{ /*Read */
			if(strlen(this_char) > 4)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}

			/* Mac Addr */
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}
			
			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			if(strlen(this_char) == 4)
			{
				AtoH(this_char, temp, 2);
				macAddr = *temp*256 + temp[1];
				if (macAddr < 0xFFFF)
				{
					RTMP_IO_READ32(pAd, macAddr, &macValue);
					DBGPRINT(RT_DEBUG_TRACE, ("MacAddr=0x%04x, MacValue=%x\n", macAddr, macValue));
					sprintf(msg+strlen(msg), "[0x%04x]:%08x  ", macAddr , macValue);
				}
				else
				{/*Invalid parametes, so default printk all mac */
					bIsPrintAllMAC = TRUE;
					goto next;
				}
			}
		}
		else
		{ /*Write */
			memcpy(&temp2, value, strlen(value));
			temp2[strlen(value)] = '\0';

			/* Sanity check */
			if((strlen(this_char) > 4) || strlen(temp2) > 8)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}

			j = strlen(temp2);
			while(j-- > 0)
			{
				if(temp2[j] > 'f' || temp2[j] < '0')
					goto LabelOK;
			}

			/*MAC Addr */
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}

			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			/*MAC value */
			k = j = strlen(temp2);
			while(j-- > 0)
			{
				temp2[8-k+j] = temp2[j];
			}
			
			while(k < 8)
				temp2[7-k++]='0';
			temp2[8]='\0';

			AtoH(this_char, temp, 2);
			macAddr = *temp*256 + temp[1];

			AtoH(temp2, temp, 4);
			macValue = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];

			/* debug mode */
			if (macAddr == (HW_DEBUG_SETTING_BASE + 4))
			{
				/* 0x2bf4: byte0 non-zero: enable R17 tuning, 0: disable R17 tuning */
				if (macValue & 0x000000ff) 
				{
					pAd->BbpTuning.bEnable = TRUE;
					DBGPRINT(RT_DEBUG_TRACE,("turn on R17 tuning\n"));
				}
				else
				{
					pAd->BbpTuning.bEnable = FALSE;
					RTMP_CHIP_RX_SENSITIVITY_TUNING(pAd);
				}
				goto LabelOK;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("MacAddr=0x%04x, MacValue=0x%x\n", macAddr, macValue));
			
			RTMP_IO_WRITE32(pAd, macAddr, macValue);
			sprintf(msg+strlen(msg), "[0x%04x]:%08x  ", macAddr, macValue);
		}
	}
	else
		bIsPrintAllMAC = TRUE;
next:
	if (bIsPrintAllMAC)
	{
		UINT32 *pBufMac = NULL, *pBuf;
		UINT32 AddrStart = 0x1000, AddrEnd = 0x1800;
		UINT32 IdAddr;

		ASSERT((AddrEnd >= AddrStart));
		/* *2 for safe */
		os_alloc_mem(NULL, (UCHAR **)&pBufMac, (AddrEnd - AddrStart)*2);
		if (pBufMac != NULL)
		{
			pBuf = pBufMac;
			for(IdAddr=AddrStart; IdAddr<=AddrEnd; IdAddr+=4, pBuf++)
				RTMP_IO_READ32(pAd, IdAddr, pBuf);
			RtmpDrvAllMacPrint(pAd, pBufMac, AddrStart, AddrEnd, 4);
#if defined(RT65xx) || defined(MT7601)
			if (IS_RT65XX(pAd)) {
				pBuf = pBufMac;
				AddrStart = 0x0; AddrEnd = 0x800;
				for(IdAddr=AddrStart; IdAddr<=AddrEnd; IdAddr+=4, pBuf++)
					RTMP_IO_READ32(pAd, IdAddr, pBuf);
				RtmpDrvAllMacPrint(pAd, pBufMac, AddrStart, AddrEnd, 4);
			}
#endif /* defined(RT65xx) || defined(MT7601) */
			os_free_mem(NULL, pBufMac);
		}
	}
	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");

	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	
LabelOK:
	if (msg != NULL)
		os_free_mem(NULL, msg);
	if (arg != NULL)
		os_free_mem(NULL, arg);

	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlMAC\n\n"));
	return;
}

/* 
    ==========================================================================
    Description:
        Read / Write E2PROM
    Arguments:
        pAd                    Pointer to our adapter
        wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 e2p 0     	==> read E2PROM where Addr=0x0
               2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
    ==========================================================================
*/
VOID RTMPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	INT					j = 0, k = 0;
/*	STRING				msg[1024]; */
	STRING				*msg = NULL;
/*	STRING				arg[255]; */
	STRING				*arg = NULL;
	USHORT				eepAddr = 0;
	UCHAR				temp[16];
	STRING				temp2[16];
	USHORT				eepValue;
	int					Status;
	BOOLEAN				bIsPrintAllE2P = FALSE;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(STRING)*1024);
	if (msg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelOK;
	}
	os_alloc_mem(NULL, (UCHAR **)&arg, sizeof(STRING)*255);
	if (arg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelOK;
	}

	memset(msg, 0x00, 1024);
	memset(arg, 0x00, 255);
	if (wrq->u.data.length > 1) /*No parameters. */
	{   
	    Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		sprintf(msg, "\n");
		arg[254] = 0x00;

	    /*Parsing Read or Write */
		this_char = arg;
		
		
		if (!*this_char)
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!value || !*value)
		{ /*Read */

			/* Sanity check */
			if(strlen(this_char) > 4)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}

			/* E2PROM addr */
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}
			
			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			if(strlen(this_char) == 4)
			{
				AtoH(this_char, temp, 2);
				eepAddr = *temp*256 + temp[1];					
				if (eepAddr < 0xFFFF)
				{
					RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
					sprintf(msg+strlen(msg), "[0x%04X]:0x%04X  ", eepAddr , eepValue);
				}
				else
				{/*Invalid parametes, so default printk all bbp */
					bIsPrintAllE2P = TRUE;
					goto next;
				}
			}
		}
		else
		{ /*Write */
			memcpy(&temp2, value, strlen(value));
			temp2[strlen(value)] = '\0';

			/* Sanity check */
			if((strlen(this_char) > 4) || strlen(temp2) > 8)
				goto next;

			j = strlen(this_char);
			while(j-- > 0)
			{
				if(this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}
			j = strlen(temp2);
			while(j-- > 0)
			{
				if(temp2[j] > 'f' || temp2[j] < '0')
					goto LabelOK;
			}

			/*MAC Addr */
			k = j = strlen(this_char);
			while(j-- > 0)
			{
				this_char[4-k+j] = this_char[j];
			}

			while(k < 4)
				this_char[3-k++]='0';
			this_char[4]='\0';

			/*MAC value */
			k = j = strlen(temp2);
			while(j-- > 0)
			{
				temp2[4-k+j] = temp2[j];
			}
			
			while(k < 4)
				temp2[3-k++]='0';
			temp2[4]='\0';

			AtoH(this_char, temp, 2);
			eepAddr = *temp*256 + temp[1];

			AtoH(temp2, temp, 2);
			eepValue = *temp*256 + temp[1];

			RT28xx_EEPROM_WRITE16(pAd, eepAddr, eepValue);
			sprintf(msg+strlen(msg), "[0x%02X]:%02X  ", eepAddr, eepValue);
		}
	}
	else
		bIsPrintAllE2P = TRUE;
next:
	if (bIsPrintAllE2P)
	{
		USHORT *pMacContent, *pMacIndex;
		UINT32 ContentLen;
		UINT32 AddrEnd = 0xFE;


		ContentLen = AddrEnd+2;
		os_alloc_mem(NULL, (UCHAR **)&pMacContent, ContentLen);
		if (pMacContent == NULL)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("-->2) %s: Allocate memory fail!\n", __FUNCTION__));
		}
		else
		{
			/* get MAC content */
			USHORT eepAddr = 0;
			USHORT eepValue;

			pMacIndex = pMacContent;
			while(eepAddr <= AddrEnd)
			{
				RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
				*pMacIndex = eepValue;

				pMacIndex ++;
				eepAddr += 2;
			}

			/* print content to a file */
			RtmpDrvAllE2PPrint(pAd, pMacContent, AddrEnd, 2);
			os_free_mem(NULL, pMacContent);
		}

	}
	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");


	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

LabelOK:
	if (msg != NULL)
		os_free_mem(NULL, msg);
	if (arg != NULL)
		os_free_mem(NULL, arg);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlE2PROM\n"));
	return;
}

#ifdef RLT_RF
VOID RTMPIoctlRF_rlt(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	UCHAR				regRF = 0, rf_bank = 0;
	PSTRING				mpool, msg;
	PSTRING				arg;
	PSTRING				ptr;
	INT					rfId, maxRFIdx, bank_Id;
	LONG				rfValue;
	BOOLEAN				bIsPrintAllRF = FALSE, bFromUI;
	INT					memLen = sizeof(CHAR) * (2048+256+12);
	INT					argLen;
	
	maxRFIdx = pAdapter->chipCap.MaxNumOfRfId;

	DBGPRINT(RT_DEBUG_TRACE, ("==>RTMPIoctlRF (maxRFIdx = %d)\n", maxRFIdx));

	memLen = 12*(maxRFIdx+1)*MAC_RF_BANK;
	os_alloc_mem(NULL, (UCHAR **)&mpool, memLen);
	if (mpool == NULL) {
		return;
	}

	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;
	
	NdisZeroMemory(mpool, memLen);
	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (PSTRING)((ULONG)(msg+2048+3) & (ULONG)~0x03);
	argLen = strlen((char *)(wrq->u.data.pointer));

// ++Alan test
	if (wrq->u.data.length > 1) /*No parameters. */
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		arg[254] = 0x00;
		ptr = arg;
		sprintf(msg, "\n");
	    /*Parsing Read or Write */
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
		if (!*this_char)
			goto next;

		if ((value = strchr(this_char,'=')) != NULL)
			*value++ = 0;


		if (!value || !*value)
		{ /*Read */

			if ((value = strchr(this_char,'_')) != NULL)
				*value++ = 0;

			if ( !value )
			{
				bIsPrintAllRF = TRUE;
				goto next;
			} 
			
			if ( (sscanf((PSTRING) this_char, "%d", &(bank_Id)) == 1) && (sscanf((PSTRING) value, "%d", &(rfId)) == 1))
			{
			
				if ( (rfId <= maxRFIdx) && (bank_Id <= MAC_RF_BANK))
				{
						rlt_rf_read(pAdapter, bank_Id, rfId, &regRF);

						sprintf(msg+strlen(msg), "BANK%d_R%02d:%02X  ", bank_Id, rfId, regRF);
				}
				else
				{/*Invalid parametes, so default printk all RF */
					bIsPrintAllRF = TRUE;
					goto next;
				}
			}
			else
			{
				/* Invalid parametes, so default printk all RF */
				bIsPrintAllRF = TRUE;
				goto next;
			}
		}
		else
		{ /*Write */
			if ( sscanf(value, "%lx", &(rfValue)) == 1)
			{
				if ((value = strchr(this_char,'_')) != NULL)
					*value++ = 0;

				if ( !value )
				{
					bIsPrintAllRF = TRUE;
					break;
				} 

				if ( (sscanf((PSTRING)this_char, "%d", &(bank_Id)) == 1) && (sscanf((PSTRING)value, "%d", &(rfId)) == 1))
				{

					if ( (rfId <= maxRFIdx) && (bank_Id <= MAC_RF_BANK) )
					{
						rlt_rf_write(pAdapter, bank_Id, rfId, rfValue);
						sprintf(msg+strlen(msg), "BANK%d_R%02d:%02X  ", bank_Id, rfId, rfValue);
					}
					else
					{
						bIsPrintAllRF = TRUE;
						break;
					}
				}
				else
				{
					bIsPrintAllRF = TRUE;
					break;
				}

			}
			else
			{
				bIsPrintAllRF = TRUE;
					break;
			}
		}

		
		}

	}
	else 
		bIsPrintAllRF = TRUE;
// --Alan test

next:
	if (bIsPrintAllRF)
	{
		RTMPZeroMemory(msg, memLen);
		sprintf(msg, "\n");
		for (bank_Id = 0; bank_Id <= MAC_RF_BANK; bank_Id++)
		{
			if (IS_MT76x0(pAdapter))
			{
				if ((bank_Id <=4) && (bank_Id >=1))
					continue;
			}
			for (rfId = 0; rfId <= maxRFIdx; rfId++)
			{
				rlt_rf_read(pAdapter, bank_Id, rfId, &regRF);
				sprintf(msg+strlen(msg), "%d %03d %02X\n", bank_Id, rfId, regRF);
			}
		}
		RtmpDrvAllRFPrint(NULL, msg, strlen(msg));
		/* Copy the information into the user buffer */

#ifdef LINUX
		wrq->u.data.length = strlen("Dump to RFDump.txt");
		if (copy_to_user(wrq->u.data.pointer, "Dump to RFDump.txt", wrq->u.data.length)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
#endif /* LINUX */
	}	
	else
	{
		if(strlen(msg) == 1)
			sprintf(msg+strlen(msg), "===>Error command format!");

		wrq->u.data.length = strlen(msg);
		if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
	}

	os_free_mem(NULL, mpool);
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlRF\n"));
}
#endif /* RLT_RF */

#ifdef MT_RF
VOID RTMPIoctlRF_mt(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	CHAR				*value;
	UINT				regRF = 0;
	CHAR				*mpool, *msg; /*msg[2048]; */
	BOOLEAN				bIsPrintAllRF = TRUE;
	UINT				rfidx =0, offset = 0;
	INT					memLen = sizeof(CHAR) * 9000; //(2048+256+12);

	DBGPRINT(RT_DEBUG_TRACE, ("==>%s\n", __FUNCTION__));

	os_alloc_mem(NULL, (UCHAR **)&mpool, memLen);
	if (mpool == NULL) {
		return;
	}
	
	NdisZeroMemory(mpool, memLen);
	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);

	if (bIsPrintAllRF)
	{
		RTMPZeroMemory(msg, memLen);
		sprintf(msg, "\n");
		for (rfidx = 0; rfidx <= 1; rfidx++)
		{
			for (offset = 0; offset <= 0x3ff; offset+=4)
			{
				mt_rf_read(pAd, rfidx, offset, &regRF);
				sprintf(msg+strlen(msg), "%d %03x = %08X\n", rfidx, offset, regRF);
			}
			offset = 0xfff;
			mt_rf_read(pAd, rfidx, offset, &regRF);
			sprintf(msg+strlen(msg), "%d %03x = %08X\n", rfidx, offset, regRF);
		}
		RtmpDrvAllRFPrint(NULL, msg, strlen(msg));
		
		DBGPRINT(RT_DEBUG_TRACE, ("strlen(msg)=%d\n", (UINT32)strlen(msg)));
		/* Copy the information into the user buffer */
#ifdef LINUX
		wrq->u.data.length = strlen("Dump to RFDump.txt");
		if (copy_to_user(wrq->u.data.pointer, "Dump to RFDump.txt", wrq->u.data.length)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
#endif /* LINUX */
	}	

	os_free_mem(NULL, mpool);
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlRF\n\n"));
}
#endif /* MT_RF */

#ifdef RTMP_RF_RW_SUPPORT
/* 
    ==========================================================================
    Description:
        Read / Write RF register
Arguments:
    pAd                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 rf                ==> read all RF registers
               2.) iwpriv ra0 rf 1              ==> read RF where RegID=1
               3.) iwpriv ra0 rf 1=10		    ==> write RF R1=0x10
    ==========================================================================
*/
VOID RTMPIoctlRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	CHAR				*this_char;
	CHAR				*value;
	UCHAR				regRF = 0;
	CHAR				*mpool, *msg; /*msg[2048]; */
	CHAR				*arg; /*arg[255]; */
	CHAR				*ptr;
	INT					rfId;
	LONG				rfValue;
	BOOLEAN				bIsPrintAllRF = FALSE;
	int 				maxRFIdx;
	

#ifdef RLT_RF
	if (IS_MT76x0(pAd) || IS_MT7601(pAd) || IS_RT6352(pAd)) {
		RTMPIoctlRF_rlt(pAd, wrq);
		return;
	}
#endif /* RLT_RF */

#ifdef MT_RF
	if (IS_MT76x2(pAd)) {
		RTMPIoctlRF_mt(pAd, wrq);
		return;
	}
#endif

	maxRFIdx = pAd->chipCap.MaxNumOfRfId;


	os_alloc_mem(pAd, (UCHAR **)&mpool, sizeof(CHAR)*(2048+256+12));
	if (mpool == NULL) {
		return;
	}

	msg = (CHAR *)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (CHAR *)((ULONG)(msg+2048+3) & (ULONG)~0x03);

	memset(msg, 0x00, 2048);
	memset(arg, 0x00, 255);
	if (wrq->u.data.length > 1) /*No parameters. */
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		arg[254] = 0x00;
		ptr = arg;
		sprintf(msg, "\n");
	    /*Parsing Read or Write */
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
		if (!*this_char)
			goto next;

		if ((value = strchr(this_char,'=')) != NULL)
			*value++ = 0;

		if (!value || !*value)
		{ /*Read */
			if (sscanf((PSTRING) this_char, "%d", &(rfId)) == 1)
			{
					if (rfId <= maxRFIdx)
				{
					RT30xxReadRFRegister(pAd, rfId, &regRF);

						sprintf(msg+strlen(msg), "R%02d:%02X  ", rfId, regRF);
				}
				else
				{/*Invalid parametes, so default printk all RF */
					bIsPrintAllRF = TRUE;
					goto next;
				}
			}
			else
			{
				/* Invalid parametes, so default printk all RF */
				bIsPrintAllRF = TRUE;
				goto next;
			}
		}
		else
		{ /*Write */
				if ((sscanf((PSTRING)this_char, "%d", &(rfId)) == 1) && (sscanf(value, "%lx", &(rfValue)) == 1))
			{
					if (rfId <= maxRFIdx)
						{
							RT30xxReadRFRegister(pAd, rfId, &regRF);
						RT30xxWriteRFRegister(pAd, rfId, rfValue);
							/*Read it back for showing */
							RT30xxReadRFRegister(pAd, rfId, &regRF);
						sprintf(msg+strlen(msg), "R%02d:%02X\n", rfId, regRF);
				                }
				else
					{
					bIsPrintAllRF = TRUE;
						break;
				}
			}
			else
				{
				bIsPrintAllRF = TRUE;
					break;
				}
			}
		}
	}
	else
		bIsPrintAllRF = TRUE;

next:
	/* Copy the information into the user buffer */
	if (bIsPrintAllRF)
	{
		memset(msg, 0x00, 2048);
		sprintf(msg, "\n");
		for (rfId = 0; rfId <= maxRFIdx; rfId++)
		{
			RT30xxReadRFRegister(pAd, rfId, &regRF);
			sprintf(msg+strlen(msg), "R%d = 0x%02X\n", rfId, regRF);
		}
		wrq->u.data.length = strlen(msg);
		if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
	}
	else
	{
		if(strlen(msg) == 1)
			sprintf(msg+strlen(msg), "===>Error command format!");

		wrq->u.data.length = strlen(msg);
		if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
	}

	os_free_mem(NULL, mpool);
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlRF\n\n"));
}
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* DBG */


#ifdef RT65xx
VOID RTMPIoctlBbp32(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN CHAR *extra,
	IN UINT32 size)
{
	PSTRING this_char;
	PSTRING value = NULL;
	UINT32 regBBP = 0, bbpId;
	UINT32 bbpValue;
	BOOLEAN bIsPrintAllBBP = FALSE;

DBGPRINT(RT_DEBUG_OFF, ("Into RTMPIoctlBbp32()!\n"));

	if (wrq->u.data.length > 1) /*No parameters. */
	{
		sprintf(extra, "\n");
					
		/*Parsing Read or Write */
		this_char = wrq->u.data.pointer;
		DBGPRINT(RT_DEBUG_INFO, ("this_char=%s\n", this_char));
		if (!*this_char)                                                                            
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;		
		
		if (!value || !*value)
		{
			/*Read */
			DBGPRINT(RT_DEBUG_INFO, ("this_char=%s, value=%s\n", this_char, value));

			if (sscanf(this_char, "%x", &(bbpId)) == 1)
			{  
				if ((bbpId <= 0x2fff) && (bbpId >= 0x2000))
				{                                                                                   
					RTMP_BBP_IO_READ32(pAd, bbpId, &regBBP);                          
					sprintf(extra+strlen(extra), "BBP[0x%04x]:%08x\n", bbpId, regBBP);
					wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
					DBGPRINT(RT_DEBUG_INFO, ("msg=%s\n", extra));
				}                                                                                   
				else                                                                                
				{
					/*Invalid parametes, so default printk all bbp */
					bIsPrintAllBBP = TRUE;
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ /*Invalid parametes, so default printk all bbp */
				bIsPrintAllBBP = TRUE;
				goto next;                                                                              
			}                                                                                       
		}                                                                                           
		else                                                                                        
		{
			/*Write */
			DBGPRINT(RT_DEBUG_OFF, ("this_char=%s, value=%s\n", this_char, value));
			if ((sscanf(this_char, "%x", &(bbpId)) == 1) && (sscanf(value, "%x", &(bbpValue)) == 1))
			{
				DBGPRINT(RT_DEBUG_OFF, ("BBP[0x%04x]:%08x\n", bbpId, bbpValue));
				if ((bbpId <= 0x2fff) && (bbpId >= 0x2000))
				{                                                                                   
					RTMP_BBP_IO_WRITE32(pAd, bbpId, bbpValue);          
					/* read it back for showing */                                                      
					RTMP_BBP_IO_READ32(pAd, bbpId, &regBBP);                          
					sprintf(extra+strlen(extra), "BBP[0x%04x]:%08x\n", bbpId, regBBP);
					wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
					DBGPRINT(RT_DEBUG_OFF, ("msg=%s\n", extra));                                       
				}                                                                                   
				else                                                                                
				{
					/*Invalid parametes, so default printk all bbp */
					bIsPrintAllBBP = TRUE;                                                          
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ /*Invalid parametes, so default printk all bbp */
				bIsPrintAllBBP = TRUE;                                                              
				goto next;                                                                              
			}                                                                                       
		}                             
		}
	else
		bIsPrintAllBBP = TRUE;

next:    
	if (bIsPrintAllBBP)
	{   
		static RTMP_REG_PAIR bbp_regs[]={
			{CORE_R1,	CORE_R44},
			{IBI_R0,		IBI_R11},
			{AGC1_R0,	AGC1_R63},
			{TXC_R1, 	TXC_R1},
			{RXC_R1,	RXC_R4},
			{TXO_R8,	TXO_R8},
			{TXBE_R0,	TXBE_R17},
			{RXFE_R0,	RXFE_R4},
			{RXO_R13,	RXO_R28},
		};
		UINT32 reg, i;


		memset(extra, 0x00, size);
		sprintf(extra, "\n");
		for (i = 0; i < sizeof(bbp_regs) / sizeof(RTMP_REG_PAIR); i++)
		{
			for (reg = bbp_regs[i].Register; reg <= bbp_regs[i].Value; reg += 4)
			{
				if (strlen(extra) >= (size - 20))
					break;
				RTMP_BBP_IO_READ32(pAd, reg, &regBBP);
				sprintf(extra+strlen(extra), "%04x = %08x\n", reg, regBBP);  /* edit by johnli, change display format */
			}
		}
		
        wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
        DBGPRINT(RT_DEBUG_TRACE, ("wrq->length = %d\n", wrq->u.data.length));
	}
}
#endif /* RT65xx */


VOID RTMPIoctlBbp(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN CHAR *extra,
	IN UINT32 size)
{
#ifdef RTMP_BBP
	PSTRING this_char;
	PSTRING value = NULL;
	UCHAR regBBP = 0;
	UINT32 bbpId;
	UINT32 bbpValue;
	BOOLEAN bIsPrintAllBBP = FALSE;
#endif /* RTMP_BBP */

#ifdef RT65xx
	if (IS_RT65XX(pAd)) {
		RTMPIoctlBbp32(pAd, wrq, extra, size);
		return;
	}
#endif /* RT65xx */

#ifdef RTMP_BBP
	if (wrq->u.data.length > 1) /*No parameters. */
	{
		sprintf(extra, "\n");
					
		/*Parsing Read or Write */
		this_char = wrq->u.data.pointer;
		DBGPRINT(RT_DEBUG_TRACE, ("this_char=%s\n", this_char));
		if (!*this_char)                                                                            
			goto next;

		if ((value = rtstrchr(this_char, '=')) != NULL)
			*value++ = 0;		
		
		if (!value || !*value)
		{ /*Read */
			DBGPRINT(RT_DEBUG_TRACE, ("this_char=%s, value=%s\n", this_char, value));
			if (sscanf(this_char, "%d", &(bbpId)) == 1)                                             
			{  
				if (bbpId <= pAd->chipCap.MaxNumOfBbpId)
				{                                                                                   
#ifdef RALINK_ATE
					if (ATE_ON(pAd))
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);
					}
					else
#endif /* RALINK_ATE */
					{
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);                          
					}
					sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
                    wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
					DBGPRINT(RT_DEBUG_TRACE, ("msg=%s\n", extra));
				}                                                                                   
				else                                                                                
				{/*Invalid parametes, so default printk all bbp */
					bIsPrintAllBBP = TRUE;
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ /*Invalid parametes, so default printk all bbp */
				bIsPrintAllBBP = TRUE;
				goto next;                                                                              
			}                                                                                       
		}                                                                                           
		else                                                                                        
		{ /*Write */
			if ((sscanf(this_char, "%d", &(bbpId)) == 1) && (sscanf(value, "%x", &(bbpValue)) == 1))
			{
				if (bbpId <= pAd->chipCap.MaxNumOfBbpId)
				{                                                                                   
#ifdef RALINK_ATE
					if (ATE_ON(pAd))
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpValue);          
						/* read it back for showing */                                                      
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);  
					}
					else
#endif /* RALINK_ATE */
					{
					    RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpValue);
    					/* read it back for showing */                                                      
    					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);
			}
					sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
                    wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
					DBGPRINT(RT_DEBUG_TRACE, ("msg=%s\n", extra));                                       
				}                                                                                   
				else                                                                                
				{/*Invalid parametes, so default printk all bbp */
					bIsPrintAllBBP = TRUE;                                                          
					goto next;                                                                          
				}                                                                                   
			}                                                                                       
			else                                                                                    
			{ /*Invalid parametes, so default printk all bbp */
				bIsPrintAllBBP = TRUE;                                                              
				goto next;                                                                              
			}                                                                                       
		}                             
	}
	else
		bIsPrintAllBBP = TRUE;

next:    
	if (bIsPrintAllBBP)
	{   
		memset(extra, 0x00, size);
		sprintf(extra, "\n");
		for (bbpId = 0; bbpId <= pAd->chipCap.MaxNumOfBbpId; bbpId++)
		{
		    if (strlen(extra) >= (size - 20))
                break;
#ifdef RALINK_ATE
			if (ATE_ON(pAd))
			{
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP); 
			}
			else
#endif /* RALINK_ATE */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, &regBBP);
			/*sprintf(extra+strlen(extra), "R%02d[0x%02X]:%02X    ", bbpId, bbpId, regBBP);
			if (bbpId%5 == 4)*/
			sprintf(extra+strlen(extra), "%03d = %02X\n", bbpId, regBBP);  /* edit by johnli, change display format */
		}
		
        wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
        DBGPRINT(RT_DEBUG_TRACE, ("wrq->length = %d\n", wrq->u.data.length));
	}
#endif /* RTMP_BBP*/

}




#ifdef DOT11_N_SUPPORT
void	getBaInfo(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			pOutBuf,
	IN	UINT32			size)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_TDLS(pEntry)) && (pEntry->Sst == SST_ASSOC))
			|| IS_ENTRY_WDS(pEntry) || IS_ENTRY_MESH(pEntry))
		{		
			sprintf(pOutBuf, "%s\n%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (AP) -\n",
                pOutBuf,
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], pEntry->Aid);
			
			sprintf(pOutBuf, "%s[Recipient]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BARecWcidArray[j] != 0)
				{
					pRecBAEntry =&pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
					sprintf(pOutBuf, "%sTID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d\n", pOutBuf, j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
				}
			}
			sprintf(pOutBuf, "%s\n", pOutBuf);

			sprintf(pOutBuf, "%s[Originator]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BAOriWcidArray[j] != 0)
				{
					pOriBAEntry =&pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];
					sprintf(pOutBuf, "%sTID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n", pOutBuf, j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence, pEntry->TxSeq[j]);
				}
			}
			sprintf(pOutBuf, "%s\n\n", pOutBuf);
		}
        if (strlen(pOutBuf) > (size - 30))
                break;
	}

	return;
}
#endif /* DOT11_N_SUPPORT */


VOID RTMPIoctlShow(
	IN	PRTMP_ADAPTER			pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	UINT32					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SHOW *pIoctlShow = (RT_CMD_STA_IOCTL_SHOW *)pData;
	POS_COOKIE pObj;
	INT Status = 0;
	char *extra = (char *)pIoctlShow->pData;
	UINT32 size = (UINT32)(pIoctlShow->MaxSize);


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	{
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
	}

    switch(subcmd)
    {
#ifdef ETH_CONVERT_SUPPORT
#ifdef MAT_SUPPORT
        case SHOW_IPV4_MAT_INFO:
            {
                extern VOID getIPMacTbInfo(MAT_STRUCT *, char *, ULONG);
                getIPMacTbInfo(&pAd->MatCfg, extra, size);
                wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            }
            break;
        case SHOW_IPV6_MAT_INFO:
            {
                extern VOID getIPv6MacTbInfo(MAT_STRUCT *, char *, ULONG);
                getIPv6MacTbInfo(&pAd->MatCfg, extra, size);
                wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            }
            break;
        case SHOW_ETH_CLONE_MAC:
            snprintf(extra, size, "%02X:%02X:%02X:%02X:%02X:%02X\n", pAd->EthConvert.EthCloneMac[0],
                                                              pAd->EthConvert.EthCloneMac[1],
                                                              pAd->EthConvert.EthCloneMac[2],
                                                              pAd->EthConvert.EthCloneMac[3],
                                                              pAd->EthConvert.EthCloneMac[4],
                                                              pAd->EthConvert.EthCloneMac[5]);
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;
#endif /* MAT_SUPPORT */
#endif /* ETH_CONVERT_SUPPORT */

        case SHOW_CONN_STATUS:
            if (MONITOR_ON(pAd))
            {
#ifdef DOT11_N_SUPPORT
                if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
                    pAd->CommonCfg.RegTransmitSetting.field.BW)
                    snprintf(extra, size, "Monitor Mode(CentralChannel %d)\n", pAd->CommonCfg.CentralChannel);
                else
#endif /* DOT11_N_SUPPORT */
                    snprintf(extra, size, "Monitor Mode(Channel %d)\n", pAd->CommonCfg.Channel);
            }
            else
            {
                if (pAd->IndicateMediaState == NdisMediaStateConnected)
            	{        	    
            	    if (INFRA_ON(pAd))
                    {   
                    snprintf(extra, size, "Connected(AP: %s[%02X:%02X:%02X:%02X:%02X:%02X])\n", 
                                    pAd->CommonCfg.Ssid, 
                                    pAd->CommonCfg.Bssid[0],
                                    pAd->CommonCfg.Bssid[1],
                                    pAd->CommonCfg.Bssid[2],
                                    pAd->CommonCfg.Bssid[3],
                                    pAd->CommonCfg.Bssid[4],
                                    pAd->CommonCfg.Bssid[5]);
            		DBGPRINT(RT_DEBUG_TRACE ,("Ssid=%s ,Ssidlen = %d\n",pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen));
            	}
                    else if (ADHOC_ON(pAd))
                        snprintf(extra, size, "Connected\n");
            	}
            	else
            	{
            	    snprintf(extra, size, "Disconnected\n");
            		DBGPRINT(RT_DEBUG_TRACE ,("ConnStatus is not connected\n"));
            	}
            }
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;
        case SHOW_DRVIER_VERION:
            snprintf(extra, size, "Driver version-%s, %s %s\n", STA_DRIVER_VERSION, __DATE__, __TIME__ );
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;
#ifdef DOT11_N_SUPPORT
        case SHOW_BA_INFO:
            getBaInfo(pAd, extra, size);
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;
#endif /* DOT11_N_SUPPORT */
		case SHOW_DESC_INFO:
			{
				Show_DescInfo_Proc(pAd, NULL);
				wrq->u.data.length = 0; /* 1: size of '\0' */
			}
			break;
        case RAIO_OFF:
			/* Link down first if any association exists*/
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			{
				if (INFRA_ON(pAd) || ADHOC_ON(pAd))
				{
					MLME_DISASSOC_REQ_STRUCT DisReq;
					MLME_QUEUE_ELEM *pMsgElem;

					os_alloc_mem(NULL, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));
					if (pMsgElem)
					{
						COPY_MAC_ADDR(&DisReq.Addr, pAd->CommonCfg.Bssid);
						DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

						pMsgElem->Machine = ASSOC_STATE_MACHINE;
						pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
						pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
						NdisMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
					
						MlmeDisassocReqAction(pAd, pMsgElem);
						os_free_mem(NULL, pMsgElem);
						
						RtmpusecDelay(1000);
					}
				}
			}

            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		        {
		            RTMP_MLME_RESET_STATE_MACHINE(pAd);
		            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		        }
            }
            pAd->StaCfg.bSwRadio = FALSE;
            if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == FALSE)
                {
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_CMD_RADIO_OFF);
                    MlmeRadioOff(pAd);
                    /* Update extra information */
			pAd->ExtraInfo = SW_RADIO_OFF;
                } 
            }
            snprintf(extra, size, "Radio Off\n");
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;
        case RAIO_ON:
            pAd->StaCfg.bSwRadio = TRUE;
            /*if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio)) */
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == TRUE)
                {
					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_CMD_RADIO_OFF);
                    MlmeRadioOn(pAd);
                    /* Update extra information */
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
                }
            }
            snprintf(extra, size, "Radio On\n");
            wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
            break;


#ifdef QOS_DLS_SUPPORT
		case SHOW_DLS_ENTRY_INFO:
			{
				Set_DlsEntryInfo_Display_Proc(pAd, NULL);
				wrq->u.data.length = 0; /* 1: size of '\0' */
			}
			break;
#endif /* QOS_DLS_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		case SHOW_TDLS_ENTRY_INFO:
			{
#ifdef DOT11Z_TDLS_SUPPORT
				Set_TdlsEntryInfo_Display_Proc(pAd, NULL);
#else
				cfg_tdls_EntryInfo_Display_Proc(pAd, NULL);
#endif
				wrq->u.data.length = 0; /* 1: size of '\0' */
			}
			break;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		case SHOW_CFG_VALUE:
			{
				Status = RTMPShowCfgValue(pAd, (PSTRING) wrq->u.data.pointer,
										extra, pIoctlShow->MaxSize);
				if (Status == 0)
					wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			}
			break;
		case SHOW_ADHOC_ENTRY_INFO:
			Show_Adhoc_MacTable_Proc(pAd, extra, size);
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;


	case SHOW_DEV_INFO:
		show_devinfo_proc(pAd, NULL);
		wrq->u.data.length = 0;
		break;

	case SHOW_STA_INFO:
		Show_MacTable_Proc(pAd, NULL);
		wrq->u.data.length = 0;
		break;

	case SHOW_TR_INFO:
		show_trinfo_proc(pAd, NULL);
		wrq->u.data.length = 0;
		break;

	case SHOW_SYS_INFO:
		show_sysinfo_proc(pAd, NULL);
		wrq->u.data.length = 0;
		break;

	case SHOW_PWR_INFO:
		show_pwr_info(pAd, NULL);
		wrq->u.data.length = 0;
		break;

	case SHOW_DIAGNOSE_INFO:
		Show_Diag_Proc(pAd, NULL);
		wrq->u.data.length = 0;
		break;	

        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d\n", __FUNCTION__, subcmd));
            break;
    }
}


/* ------------------- Functions for Standard IOCTL ------------------------- */
#define RTMP_STA_STANDARD_IOCTL_HANDLE(__pAd, __pData, __Data, __SubCmd)	\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWFREQ:									\
		return RtmpIoctl_rt_ioctl_siwfreq(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWFREQ:									\
		return RtmpIoctl_rt_ioctl_giwfreq(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWMODE:									\
		return RtmpIoctl_rt_ioctl_siwmode(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWMODE:									\
		return RtmpIoctl_rt_ioctl_giwmode(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWAP:									\
		return RtmpIoctl_rt_ioctl_siwap(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWAP:									\
		return RtmpIoctl_rt_ioctl_giwap(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWSCAN:									\
		return RtmpIoctl_rt_ioctl_siwscan(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWSCAN:									\
		return RtmpIoctl_rt_ioctl_giwscan(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWESSID:									\
		return RtmpIoctl_rt_ioctl_siwessid(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWESSID:									\
		return RtmpIoctl_rt_ioctl_giwessid(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWNICKN:									\
		return RtmpIoctl_rt_ioctl_siwnickn(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWNICKN:									\
		return RtmpIoctl_rt_ioctl_giwnickn(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWRTS:									\
		return RtmpIoctl_rt_ioctl_siwrts(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWRTS:									\
		return RtmpIoctl_rt_ioctl_giwrts(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWFRAG:									\
		return RtmpIoctl_rt_ioctl_siwfrag(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWFRAG:									\
		return RtmpIoctl_rt_ioctl_giwfrag(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODE:								\
		return RtmpIoctl_rt_ioctl_siwencode(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODE:								\
		return RtmpIoctl_rt_ioctl_giwencode(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWMLME:									\
		return RtmpIoctl_rt_ioctl_siwmlme(__pAd, __pData, __Data, __SubCmd);\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWAUTH:									\
		return RtmpIoctl_rt_ioctl_siwauth(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWAUTH:									\
		return RtmpIoctl_rt_ioctl_giwauth(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT:								\
		return RtmpIoctl_rt_ioctl_siwencodeext(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODEEXT:								\
		return RtmpIoctl_rt_ioctl_giwencodeext(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE:									\
		return RtmpIoctl_rt_ioctl_siwgenie(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWGENIE:									\
		return RtmpIoctl_rt_ioctl_giwgenie(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA:									\
		return RtmpIoctl_rt_ioctl_siwpmksa(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWRATE:									\
		return RtmpIoctl_rt_ioctl_siwrate(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWRATE:									\
		return RtmpIoctl_rt_ioctl_giwrate(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIFHWADDR:								\
		return RtmpIoctl_rt_ioctl_gifhwaddr(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWPRIVRSSI:								\
		return RtmpIoctl_rt_ioctl_rssi(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_U32_ITEM:							\
		return RtmpIoctl_rt_private_set_wsc_u32_item(__pAd, __pData, __Data);\
	case CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_STR_ITEM:							\
		return RtmpIoctl_rt_private_set_wsc_string_item(__pAd, __pData, __Data);\
	case CMD_RTPRIV_IOCTL_STA_IW_GET_STATISTICS:							\
		return RtmpIoctl_rt_private_get_statistics(__pAd, __pData, __Data);	\
	;


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWFREQ.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwfreq(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_FREQ *pIoctlFreq = (RT_CMD_STA_IOCTL_FREQ *)pData;
	int 	chan = -1;
	ULONG	freq;
	
	if ( pIoctlFreq->m > 100000000 )
		freq = pIoctlFreq->m / 100000;
	else if ( pIoctlFreq->m > 100000 )
		freq = pIoctlFreq->m / 100;
	else
		freq = pIoctlFreq->m;


	if((pIoctlFreq->e == 0) && (freq <= 1000))
		chan = pIoctlFreq->m;	/* Setting by channel number */
	else
	{
		MAP_KHZ_TO_CHANNEL_ID( freq , chan); /* Setting by frequency - search the table , like 2.412G, 2.422G, */
	}

    if (ChannelSanity(pAd, chan) == TRUE)
    {
	pAd->CommonCfg.Channel = chan;
		/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
		pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
		/*save connect info*/
		pAd->StaCfg.ConnectinfoChannel = pAd->CommonCfg.Channel;	
	DBGPRINT(RT_DEBUG_ERROR, ("==>rt_ioctl_siwfreq::SIOCSIWFREQ(Channel=%d)\n", pAd->CommonCfg.Channel));
    }
    else
        return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWFREQ.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwfreq(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR ch;
	ULONG	m = 2412000;

		ch = pAd->CommonCfg.Channel;

	DBGPRINT(RT_DEBUG_TRACE,("==>rt_ioctl_giwfreq  %d\n", ch));

	MAP_CHANNEL_ID_TO_KHZ(ch, m);
	*(ULONG *)pData = m;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWMODE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwmode(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	switch(Data)
	{
		case RTMP_CMD_STA_MODE_ADHOC:
			Set_NetworkType_Proc(pAd, "Adhoc");
			break;
		case RTMP_CMD_STA_MODE_INFRA:
			Set_NetworkType_Proc(pAd, "Infra");
			break;
		case RTMP_CMD_STA_MODE_MONITOR:
			Set_NetworkType_Proc(pAd, "Monitor");
			break;
	}
	
	/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key */
	pAd->StaCfg.WpaState = SS_NOTUSE;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWMODE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwmode(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	if (ADHOC_ON(pAd))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_ADHOC;
	else if (INFRA_ON(pAd))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_INFRA;
	else if (MONITOR_ON(pAd))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_MONITOR;
	else
		*(ULONG *)pData = RTMP_CMD_STA_MODE_AUTO;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWAP.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwap(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR *pBssid = (UCHAR *)pData;


	if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
	{
		RTMP_MLME_RESET_STATE_MACHINE(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
	}

	/* tell CNTL state machine to call NdisMSetInformationComplete() after completing */
	/* this request, because this request is initiated by NDIS. */
	pAd->MlmeAux.CurrReqIsFromNdis = FALSE; 
	/* Prevent to connect AP again in STAMlmePeriodicExec */
	pAd->MlmeAux.AutoReconnectSsidLen= 32;

	if (MAC_ADDR_EQUAL(pBssid, ZERO_MAC_ADDR))
	{
		if (INFRA_ON(pAd))
		{
			LinkDown(pAd, FALSE);
		}
	}
	else
	{
		MlmeEnqueue(pAd, 
			MLME_CNTL_STATE_MACHINE, 
			OID_802_11_BSSID, 
			sizeof(NDIS_802_11_MAC_ADDRESS),
			(VOID *)pBssid, 0);
	}
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWAP.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwap(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	if (INFRA_ON(pAd) || ADHOC_ON(pAd))
		NdisMoveMemory(pData, pAd->CommonCfg.Bssid, MAC_ADDR_LEN);
#ifdef WPA_SUPPLICANT_SUPPORT    
	/* Add for RT2870 */
	else if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
		NdisMoveMemory(pData, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
#endif /* WPA_SUPPLICANT_SUPPORT */
	else
		return NDIS_STATUS_FAILURE;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWSCAN.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_siwscan(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	/*
		Can not use SIOCGIWSCAN definition, it is used in wireless.h
		We will not see the definition in MODULE.
		The definition can be saw in UTIL and NETIF.
	*/
/* #if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT) */
	RT_CMD_STA_IOCTL_SCAN *pConfig = (RT_CMD_STA_IOCTL_SCAN *)pData;
	int Status = NDIS_STATUS_SUCCESS;


#ifdef ANDROID_SUPPORT
	if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#ifdef IFUP_IN_PROBE
		|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))
		|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
	)
	{
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_SCAN, -1, NULL, NULL, 0);
		return NDIS_STATUS_SUCCESS;
	}
#endif /* ANDROID_SUPPORT */

	pConfig->Status = 0;

	if (MONITOR_ON(pAd))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
		pConfig->Status = RTMP_IO_EINVAL;
		return NDIS_STATUS_FAILURE;
    }


#ifdef WPA_SUPPLICANT_SUPPORT
	if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
	{
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantScanCount++;
	}
#endif /* WPA_SUPPLICANT_SUPPORT */

    pAd->StaCfg.bSkipAutoScanConn = TRUE;
	do{

		if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) &&
			((wdev->AuthMode == Ndis802_11AuthModeWPA) || 
				(wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)) &&	
			(wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

#ifdef WPA_SUPPLICANT_SUPPORT
		if (pConfig->FlgScanThisSsid)
		{
				NDIS_802_11_SSID          Ssid;
				Ssid.SsidLength = pConfig->SsidLen;
				DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwscan:: req.essid_len-%d, essid-%s\n", pConfig->SsidLen, pConfig->pSsid));
				NdisZeroMemory(&Ssid.Ssid, NDIS_802_11_LENGTH_SSID);
				NdisMoveMemory(Ssid.Ssid, pConfig->pSsid, Ssid.SsidLength);
				StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE);
		}
		else
#endif /* WPA_SUPPLICANT_SUPPORT */
		StaSiteSurvey(pAd, NULL, SCAN_ACTIVE);
	}while(0);

	pConfig->Status = Status;
/* #endif */ /* SIOCGIWSCAN || RT_CFG80211_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Set the signal quality.

Arguments:
	*pSignal		- signal structure
	pBssEntry		- the BSS information

Return Value:
	None

Note:
========================================================================
*/
static void set_quality(
	IN RT_CMD_STA_IOCTL_BSS *pSignal,
	IN BSS_ENTRY *pBssEntry)
{
	memcpy(pSignal->Bssid, pBssEntry->Bssid, MAC_ADDR_LEN);

	/* Normalize Rssi */
	if (pBssEntry->Rssi >= -50)
        pSignal->ChannelQuality = 100;
	else if (pBssEntry->Rssi >= -80) /* between -50 ~ -80dbm */
		pSignal->ChannelQuality = (__u8)(24 + ((pBssEntry->Rssi + 80) * 26)/10);
	else if (pBssEntry->Rssi >= -90)   /* between -80 ~ -90dbm */
        pSignal->ChannelQuality = (__u8)((pBssEntry->Rssi + 90) * 26)/10;   
	else
		pSignal->ChannelQuality = 0;
        
    pSignal->Rssi = (__u8)(pBssEntry->Rssi);

    if (pBssEntry->Rssi >= -70)
		pSignal->Noise = -92;
	else
		pSignal->Noise = pBssEntry->Rssi - pBssEntry->MinSNR;		
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWSCAN.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwscan(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SCAN_TABLE *pIoctlScan = (RT_CMD_STA_IOCTL_SCAN_TABLE *)pData;
	RT_CMD_STA_IOCTL_BSS_TABLE *pBssTable;
	BSS_ENTRY *pBssEntry;
	UINT32 IdBss;


	pIoctlScan->BssNr = 0;


#ifdef WPA_SUPPLICANT_SUPPORT
	if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
	{
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantScanCount = 0;
	}
#endif /* WPA_SUPPLICANT_SUPPORT */

	pIoctlScan->BssNr = pAd->ScanTab.BssNr;
	if (pIoctlScan->BssNr == 0)
		return NDIS_STATUS_SUCCESS;

	os_alloc_mem(NULL, (UCHAR **)&(pIoctlScan->pBssTable),
				pAd->ScanTab.BssNr * sizeof(RT_CMD_STA_IOCTL_BSS_TABLE));
	if (pIoctlScan->pBssTable == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory fail!\n"));
		return NDIS_STATUS_FAILURE;
	}

	for(IdBss=0; IdBss<pAd->ScanTab.BssNr; IdBss++)
	{
		HT_CAP_INFO capInfo = pAd->ScanTab.BssEntry[IdBss].HtCapability.HtCapInfo;

		pBssTable = pIoctlScan->pBssTable + IdBss;
		pBssEntry = &pAd->ScanTab.BssEntry[IdBss];

		memcpy(pBssTable->Bssid, pBssEntry->Bssid, ETH_ALEN);
		pBssTable->Channel = pBssEntry->Channel;
		pBssTable->BssType = pBssEntry->BssType;
		pBssTable->HtCapabilityLen = pBssEntry->HtCapabilityLen;
		memcpy(pBssTable->SupRate, pBssEntry->SupRate, 12);
		pBssTable->SupRateLen = pBssEntry->SupRateLen;
		memcpy(pBssTable->ExtRate, pBssEntry->ExtRate, 12);
		pBssTable->ExtRateLen = pBssEntry->ExtRateLen;
		pBssTable->SsidLen = pBssEntry->SsidLen;
		memcpy(pBssTable->Ssid, pBssEntry->Ssid, 32);
		pBssTable->CapabilityInfo = pBssEntry->CapabilityInfo;
		pBssTable->ChannelWidth = capInfo.ChannelWidth;
		pBssTable->ShortGIfor40 = capInfo.ShortGIfor40;
		pBssTable->ShortGIfor20 = capInfo.ShortGIfor20;
		pBssTable->MCSSet = pBssEntry->HtCapability.MCSSet[1];
		pBssTable->WpaIeLen = pBssEntry->WpaIE.IELen;
		pBssTable->pWpaIe = pBssEntry->WpaIE.IE;
		pBssTable->RsnIeLen = pBssEntry->RsnIE.IELen;
		pBssTable->pRsnIe = pBssEntry->RsnIE.IE;
		pBssTable->WpsIeLen = pBssEntry->WpsIE.IELen;
		pBssTable->pWpsIe = pBssEntry->WpsIE.IE;
		pBssTable->VHTCapabilityLen = pBssEntry->vht_cap_len;
		pBssTable->FlgIsPrivacyOn = CAP_IS_PRIVACY_ON(pBssEntry->CapabilityInfo);
		set_quality(&pBssTable->Signal, pBssEntry);
	}

	memcpy(pIoctlScan->MainSharedKey[0], pAd->SharedKey[BSS0][0].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[1], pAd->SharedKey[BSS0][1].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[2], pAd->SharedKey[BSS0][2].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[3], pAd->SharedKey[BSS0][3].Key, 16);

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWESSID.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwessid(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SSID *pSsid = (RT_CMD_STA_IOCTL_SSID *)pData;

	if (pSsid->FlgAnySsid)
	{
		PSTRING	pSsidString = NULL;

		/* Includes null character. */
		os_alloc_mem(NULL, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID+1);
		if (pSsidString)
		{
			NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
			NdisMoveMemory(pSsidString, pSsid->pSsid, pSsid->SsidLen);
			if (Set_SSID_Proc(pAd, pSsidString) == FALSE)
			{
				os_free_mem(NULL, pSsidString);
				pSsid->Status = RTMP_IO_EINVAL;
				return NDIS_STATUS_SUCCESS;
			}
			os_free_mem(NULL, pSsidString);
		}
		else
		{
			pSsid->Status = RTMP_IO_ENOMEM;
			return NDIS_STATUS_SUCCESS;
		}
	}
	else
	{
		/* ANY ssid */
		if (Set_SSID_Proc(pAd, "") == FALSE)
		{
			pSsid->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_SUCCESS;
		}
	}
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWESSID.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwessid(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SSID *pSsid = (RT_CMD_STA_IOCTL_SSID *)pData;

	if (MONITOR_ON(pAd))
	{
		pSsid->SsidLen = 0;
		return NDIS_STATUS_SUCCESS;
	}

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is connected\n"));
		pSsid->SsidLen = pAd->CommonCfg.SsidLen;
		memcpy(pSsid->pSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
	}
	else
	{/*the ANY ssid was specified */
		pSsid->SsidLen = 0;
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is not connected, ess\n"));
	}
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWNICKN.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwnickn(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	memset(pAd->nickname, 0, IW_ESSID_MAX_SIZE + 1);
	memcpy(pAd->nickname, pData, Data);
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNICKN.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwnickn(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_NICK_NAME *IoctlName = (RT_CMD_STA_IOCTL_NICK_NAME *)pData;


	if (IoctlName->NameLen > strlen((PSTRING) pAd->nickname) + 1)
		IoctlName->NameLen = strlen((PSTRING) pAd->nickname) + 1;
	if (IoctlName->NameLen > 0) {
		memcpy(IoctlName->pName, pAd->nickname, IoctlName->NameLen-1);
		IoctlName->pName[IoctlName->NameLen-1] = '\0';
	}
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWRTS.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwrts(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	pAd->CommonCfg.RtsThreshold = Data;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWRTS.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwrts(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	*(USHORT *)pData = pAd->CommonCfg.RtsThreshold;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWFRAG.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwfrag(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	pAd->CommonCfg.FragmentThreshold = Data;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWFRAG.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwfrag(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	*(USHORT *)pData = pAd->CommonCfg.FragmentThreshold;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
#define NR_WEP_KEYS 				4
#define MAX_WEP_KEY_SIZE			13
#define MIN_WEP_KEY_SIZE			5
INT RtmpIoctl_rt_ioctl_siwencode(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	if ((pIoctlSec->length == 0) &&
        (pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_DISABLED))
	{
		pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
		pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
		wdev->WepStatus = Ndis802_11WEPDisabled;
	        wdev->AuthMode = Ndis802_11AuthModeOpen;
	        goto done;
	}
	else if (pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_RESTRICTED ||
				pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_OPEN)
	{
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
		pAd->StaCfg.PairCipher = Ndis802_11WEPEnabled;
		pAd->StaCfg.GroupCipher = Ndis802_11WEPEnabled;
		wdev->WepStatus = Ndis802_11WEPEnabled;
		if (pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_RESTRICTED)
			wdev->AuthMode = Ndis802_11AuthModeShared;
	    	else
			wdev->AuthMode = Ndis802_11AuthModeOpen;
	}
    
    if (pIoctlSec->length > 0) 
	{
		int keyIdx = pIoctlSec->KeyIdx; /*(erq->flags & IW_ENCODE_INDEX) - 1; */
		/* Check the size of the key */
		if (pIoctlSec->length > MAX_WEP_KEY_SIZE) 
		{
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_SUCCESS;
		}
		/* Check key index */
		if ((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
        {
            DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::Wrong keyIdx=%d! Using default key instead (%d)\n", 
                                        keyIdx, wdev->DefaultKeyId));
            
            /*Using default key */
			keyIdx = wdev->DefaultKeyId;   
        }
		else
			wdev->DefaultKeyId = keyIdx;

        NdisZeroMemory(pAd->SharedKey[BSS0][keyIdx].Key,  16);
		
		if (pIoctlSec->length == MAX_WEP_KEY_SIZE)
        {      
			pAd->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
            pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
		}
		else if (pIoctlSec->length == MIN_WEP_KEY_SIZE)
        {      
            pAd->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
            pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
		}
		else
			/* Disable the key */
			pAd->SharedKey[BSS0][keyIdx].KeyLen = 0;

		/* Check if the key is not marked as invalid */
		if(!(pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_NOKEY)) 
		{
			/* Copy the key in the driver */
			NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, pIoctlSec->pData, pIoctlSec->length);
        }
	} 
    else 
			{
		/* Do we want to just set the transmit key index ? */
		int index = pIoctlSec->KeyIdx; /*(erq->flags & IW_ENCODE_INDEX) - 1; */
		if ((index >= 0) && (index < 4)) 
        {      
			pAd->StaCfg.wdev.DefaultKeyId = index;
            }
        else
			/* Don't complain if only change the mode */
		if(!(pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_MODE))
		{
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_SUCCESS;
		}
	}
		
done:
    DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::erq->flags=%x\n", pIoctlSec->flags));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::AuthMode=%x\n", wdev->AuthMode));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::DefaultKeyId=%x, KeyLen = %d\n",
					wdev->DefaultKeyId , pAd->SharedKey[BSS0][wdev->DefaultKeyId].KeyLen));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::WepStatus=%x\n", wdev->WepStatus));
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwencode(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	int kid;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	kid = pIoctlSec->KeyIdx; /*erq->flags & IW_ENCODE_INDEX; */
	DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_giwencode %d\n", kid));

	if (wdev->WepStatus == Ndis802_11WEPDisabled)
	{
		pIoctlSec->length = 0;
		pIoctlSec->flags = RT_CMD_STA_IOCTL_SECURITY_DISABLED;
	} 
	else if ((kid > 0) && (kid <=4))
	{
		/* copy wep key */
		pIoctlSec->KeyIdx = kid;
		if (pIoctlSec->length > pAd->SharedKey[BSS0][kid-1].KeyLen)
			pIoctlSec->length = pAd->SharedKey[BSS0][kid-1].KeyLen;
		memcpy(pIoctlSec->pData, pAd->SharedKey[BSS0][kid-1].Key, pIoctlSec->length);
		if (wdev->AuthMode == Ndis802_11AuthModeShared)
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */
		
	}
	else if (kid == 0)
	{
		if (wdev->AuthMode == Ndis802_11AuthModeShared)
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */
		pIoctlSec->length = pAd->SharedKey[BSS0][wdev->DefaultKeyId].KeyLen;
		memcpy(pIoctlSec->pData, pAd->SharedKey[BSS0][wdev->DefaultKeyId].Key, pIoctlSec->length);
		/* copy default key ID */
		if (wdev->AuthMode == Ndis802_11AuthModeShared)
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */
		pIoctlSec->KeyIdx = wdev->DefaultKeyId + 1;			/* NB: base 1 */
		pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_ENABLED;	/* XXX */
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWMLME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwmlme(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data,
	IN	UINT32					Subcmd)
{
	MLME_QUEUE_ELEM				*pMsgElem = NULL;
	MLME_DISASSOC_REQ_STRUCT	DisAssocReq;
	MLME_DEAUTH_REQ_STRUCT      DeAuthReq;
	ULONG						reason_code = (ULONG)Data;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));
	if (pMsgElem == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	switch(Subcmd)
	{
		case RT_CMD_STA_IOCTL_IW_MLME_DEAUTH:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DEAUTH\n", __FUNCTION__));
			COPY_MAC_ADDR(DeAuthReq.Addr, pAd->CommonCfg.Bssid);
			DeAuthReq.Reason = reason_code;
			pMsgElem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
			NdisMoveMemory(pMsgElem->Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
			MlmeDeauthReqAction(pAd, pMsgElem);
			if (INFRA_ON(pAd))
			{
			    LinkDown(pAd, FALSE);
			    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			}
			break;
		case RT_CMD_STA_IOCTL_IW_MLME_DISASSOC:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DISASSOC\n", __FUNCTION__));
			NdisZeroMemory(pAd->StaCfg.ConnectinfoSsid, MAX_LEN_OF_SSID);
			NdisZeroMemory(pAd->StaCfg.ConnectinfoBssid, MAC_ADDR_LEN);
			pAd->StaCfg.ConnectinfoSsidLen  = 0;
			pAd->StaCfg.ConnectinfoBssType  = 1;
			pAd->StaCfg.ConnectinfoChannel = 0;
			
			COPY_MAC_ADDR(DisAssocReq.Addr, pAd->CommonCfg.Bssid);
			DisAssocReq.Reason =  reason_code;

			pMsgElem->Machine = ASSOC_STATE_MACHINE;
			pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(pMsgElem->Msg, &DisAssocReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, pMsgElem);
			break;
	}
	
	if (pMsgElem != NULL)
		os_free_mem(NULL, pMsgElem);

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWAUTH.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwauth(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SECURITY_ADV *pIoctlWpa = (RT_CMD_STA_IOCTL_SECURITY_ADV *)pData;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	switch (pIoctlWpa->flags)
	{
    	case RT_CMD_STA_IOCTL_WPA_VERSION:
            if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_VERSION1)
            {            
                wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
			if (pAd->StaCfg.BssType == BSS_ADHOC)
				wdev->AuthMode = Ndis802_11AuthModeWPANone;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_VERSION2)
                wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
			
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_VERSION - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
            break;
    	case RT_CMD_STA_IOCTL_WPA_PAIRWISE:
            if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_NONE)
            {
                wdev->WepStatus = Ndis802_11WEPDisabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_WEP40 ||
                     pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_WEP104)
            {
                wdev->WepStatus = Ndis802_11WEPEnabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPEnabled;
#ifdef WPA_SUPPLICANT_SUPPORT                
                pAd->StaCfg.wdev.IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_TKIP)
            {
                wdev->WepStatus = Ndis802_11TKIPEnable;
                pAd->StaCfg.PairCipher = Ndis802_11TKIPEnable;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_CCMP)
            {
                wdev->WepStatus = Ndis802_11AESEnable;
                pAd->StaCfg.PairCipher = Ndis802_11AESEnable;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_PAIRWISE - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
            break;
    	case RT_CMD_STA_IOCTL_WPA_GROUP:
            if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_NONE)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_WEP40)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11GroupWEP40Enabled;
            }
			else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_WEP104)
            {
				pAd->StaCfg.GroupCipher = Ndis802_11GroupWEP104Enabled;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_TKIP)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11TKIPEnable;
            }
            else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_CCMP)
            {
                pAd->StaCfg.GroupCipher = Ndis802_11AESEnable;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_GROUP - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
            break;
    	case RT_CMD_STA_IOCTL_WPA_KEY_MGMT:
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
			pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP &= 0x7F;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
            if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_KEY_MGMT_1X)
            { 
                if (wdev->AuthMode == Ndis802_11AuthModeWPAPSK)
                {
                    wdev->AuthMode = Ndis802_11AuthModeWPA;
#ifdef WPA_SUPPLICANT_SUPPORT                    
                    wdev->IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
                }
                else if (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
                {
                    wdev->AuthMode = Ndis802_11AuthModeWPA2;
#ifdef WPA_SUPPLICANT_SUPPORT
                    wdev->IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
                }
#ifdef WPA_SUPPLICANT_SUPPORT                
                else
                    /* WEP 1x */
                    wdev->IEEE8021X = TRUE;
#endif /* WPA_SUPPLICANT_SUPPORT */                
            }
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
            else if (pIoctlWpa->value == 0)
            {
				wdev->PortSecured = WPA_802_1X_PORT_SECURED;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_KEY_MGMT - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
            break;
    	case RT_CMD_STA_IOCTL_WPA_AUTH_RX_UNENCRYPTED_EAPOL:
            break;
    	case RT_CMD_STA_IOCTL_WPA_AUTH_PRIVACY_INVOKED:
            /*if (pIoctlWpa->value == 0)
			{
                pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
                pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
                pAd->StaCfg.PairCipher = Ndis802_11WEPDisabled;
        	    pAd->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
            }*/            
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_PRIVACY_INVOKED - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
    		break;
    	case RT_CMD_STA_IOCTL_WPA_AUTH_DROP_UNENCRYPTED:
            if (pIoctlWpa->value != 0)
                wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{
				wdev->PortSecured = WPA_802_1X_PORT_SECURED;
			}
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_DROP_UNENCRYPTED - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
    		break;
    	case RT_CMD_STA_IOCTL_WPA_AUTH_80211_AUTH_ALG: 
				wdev->AuthMode = Ndis802_11AuthModeAutoSwitch;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_80211_AUTH_ALG - param->value = %d!\n", __FUNCTION__, pIoctlWpa->value));
			break;
    	case RT_CMD_STA_IOCTL_WPA_AUTH_WPA_ENABLED:
    		DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_ENABLED - Driver supports WPA!(param->value = %d)\n", __FUNCTION__, pIoctlWpa->value));
    		break;
	case RT_CMD_STA_IOCTL_WPA_AUTH_COUNTERMEASURES:
		if (pIoctlWpa->value == 1 )
			pAd->StaCfg.bBlockAssoc = TRUE;
		else	
			pAd->StaCfg.bBlockAssoc = FALSE;
}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWAUTH.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwauth(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY_ADV *pIoctlWpa = (RT_CMD_STA_IOCTL_SECURITY_ADV *)pData;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	switch (pIoctlWpa->flags) {
	case RT_CMD_STA_IOCTL_WPA_AUTH_DROP_UNENCRYPTED:
        pIoctlWpa->value = (wdev->WepStatus == Ndis802_11WEPDisabled) ? 0 : 1;
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_80211_AUTH_ALG:
        pIoctlWpa->value = (wdev->AuthMode == Ndis802_11AuthModeShared) ? 0 : 1;
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_WPA_ENABLED:
		pIoctlWpa->value = (wdev->AuthMode >= Ndis802_11AuthModeWPA) ? 1 : 0;
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Set security key.

Arguments:
	pAd				- WLAN control block pointer
	keyIdx			- key index
	CipherAlg		- cipher algorithm
	bGTK			- 1: the key is group key
	*pKey			- the key

Return Value:
	None

Note:
========================================================================
*/
void fnSetCipherKey(
    IN  PRTMP_ADAPTER   pAd,
    IN  INT             keyIdx,    
    IN  UCHAR           CipherAlg,
    IN  BOOLEAN         bGTK,
    IN  UCHAR			*pKey)
{
    NdisZeroMemory(&pAd->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
    pAd->SharedKey[BSS0][keyIdx].KeyLen = LEN_TK;
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, pKey, LEN_TK);
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].TxMic, pKey + LEN_TK, LEN_TKIP_MIC);
    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].RxMic, pKey + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CipherAlg;

	if (!bGTK)
	{
			MAC_TABLE_ENTRY  *pEntry;
	            /* Update these related information to MAC_TABLE_ENTRY */
	        	pEntry = &pAd->MacTab.Content[BSSID_WCID];
	            	NdisMoveMemory(pEntry->PairwiseKey.Key, pAd->SharedKey[BSS0][keyIdx].Key, LEN_TK);            
	        	NdisMoveMemory(pEntry->PairwiseKey.RxMic, pAd->SharedKey[BSS0][keyIdx].RxMic, LEN_TKIP_MIC);
	        	NdisMoveMemory(pEntry->PairwiseKey.TxMic, pAd->SharedKey[BSS0][keyIdx].TxMic, LEN_TKIP_MIC);
	        	pEntry->PairwiseKey.CipherAlg = pAd->SharedKey[BSS0][keyIdx].CipherAlg;

				pEntry->PairwiseKey.KeyLen = LEN_TK;
			/* Add Pair-wise key to Asic */
		    	AsicAddPairwiseKeyEntry(
		        pAd, 
		        (UCHAR)pEntry->wcid, 
		        &pEntry->PairwiseKey);

		RTMPSetWcidSecurityInfo(pAd, 
	    						BSS0, 
									0, 
									pEntry->PairwiseKey.CipherAlg,
									(UCHAR)pEntry->wcid, 
									PAIRWISEKEYTABLE);		
	}
	else
	{
			pAd->StaCfg.wdev.DefaultKeyId = keyIdx;
		    	/* Update group key information to ASIC Shared Key Table */
			AsicAddSharedKeyEntry(pAd, 
								  BSS0,
								  keyIdx,
								  &pAd->SharedKey[BSS0][keyIdx]);
	}
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_siwencodeext(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	int keyIdx;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

    if (pIoctlSec->flags == RT_CMD_STA_IOCTL_SECURITY_DISABLED)
	{
        keyIdx = pIoctlSec->KeyIdx; /*(encoding->flags & IW_ENCODE_INDEX) - 1; */
        /* set BSSID wcid entry of the Pair-wise Key table as no-security mode */
	    AsicRemovePairwiseKeyEntry(pAd, BSSID_WCID);
        pAd->SharedKey[BSS0][keyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)keyIdx);
        NdisZeroMemory(&pAd->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
        DBGPRINT(RT_DEBUG_TRACE, ("%s::Remove all keys!\n", __FUNCTION__));
    }
	else
    {
        /* Get Key Index and convet to our own defined key index */
    	keyIdx = pIoctlSec->KeyIdx; /*(encoding->flags & IW_ENCODE_INDEX) - 1; */
    	if((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
    		return NDIS_STATUS_FAILURE;
					
        if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY)
        {
            wdev->DefaultKeyId = keyIdx;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::DefaultKeyId = %d\n", __FUNCTION__, wdev->DefaultKeyId));
        }

        switch (pIoctlSec->Alg) {
    		case RT_CMD_STA_IOCTL_SECURITY_ALG_NONE:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_NONE\n", __FUNCTION__));
    			break;
    		case RT_CMD_STA_IOCTL_SECURITY_ALG_WEP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_WEP - ext->key_len = %d, keyIdx = %d\n", __FUNCTION__, pIoctlSec->length, keyIdx));
    			if (pIoctlSec->length == MAX_WEP_KEY_SIZE)
                {      
        			pAd->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
                    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
				}
        		else if (pIoctlSec->length == MIN_WEP_KEY_SIZE)
                {      
                    pAd->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
                    pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
				}
        		else
                    return NDIS_STATUS_FAILURE;
                                
                NdisZeroMemory(pAd->SharedKey[BSS0][keyIdx].Key,  16);
			    NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, pIoctlSec->pData, pIoctlSec->length);

				if ((pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP40Enabled) ||
					(pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP104Enabled))				
				{										
					/* Set Group key material to Asic */
					AsicAddSharedKeyEntry(pAd, BSS0, keyIdx, &pAd->SharedKey[BSS0][keyIdx]);										

/*
	STA doesn't need to set WCID attribute for group key
*/
					STA_PORT_SECURED(pAd);					    				
				}
    			break;
            case RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_TKIP - keyIdx = %d, ext->key_len = %d\n", __FUNCTION__, keyIdx, pIoctlSec->length));
                if (pIoctlSec->length == 32)
                {
                	if (wdev->AuthMode == Ndis802_11AuthModeWPANone)
                	{
                		RTMPZeroMemory(pAd->StaCfg.PMK, LEN_PMK);
                		RTMPMoveMemory(pAd->StaCfg.PMK, pIoctlSec->pData, pIoctlSec->length);
                	}
					else
					{
	                    if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY)
	                    {
	                        fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, FALSE, pIoctlSec->pData);
	                        if (wdev->AuthMode >= Ndis802_11AuthModeWPA2)
	                        {
	                            STA_PORT_SECURED(pAd);
	                        }
						}
	                    else if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY)
	                    {
	                        fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, TRUE, pIoctlSec->pData);
			
	                        /* set 802.1x port control */
	            	        STA_PORT_SECURED(pAd);
	                    }
					}
                }
                else
                    return NDIS_STATUS_FAILURE;
                break;
            case RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP:
				if (wdev->AuthMode == Ndis802_11AuthModeWPANone)
            	{
            		RTMPZeroMemory(pAd->StaCfg.PMK, LEN_PMK);
            		RTMPMoveMemory(pAd->StaCfg.PMK, pIoctlSec->pData, pIoctlSec->length);
            	}
				else
				{
	                if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY)
					{
	                    fnSetCipherKey(pAd, keyIdx, CIPHER_AES, FALSE, pIoctlSec->pData);
	                    if (wdev->AuthMode >= Ndis802_11AuthModeWPA2)
						{
	                    	STA_PORT_SECURED(pAd);
						}
	                }
	                else if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY)
	                {
	                    fnSetCipherKey(pAd, keyIdx, CIPHER_AES, TRUE, pIoctlSec->pData);
	                    
	                    /* set 802.1x port control */
	        	        STA_PORT_SECURED(pAd);
	                }
				}
                break;
    		default:
    			return NDIS_STATUS_FAILURE;
		}
    }

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODEEXT.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwencodeext(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	int idx;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	idx = pIoctlSec->KeyIdx;
	if (idx) 
	{
		if (idx < 1 || idx > 4)
		{
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_FAILURE;
		}
		idx--;

		if ((wdev->WepStatus == Ndis802_11TKIPEnable) ||
			(wdev->WepStatus == Ndis802_11AESEnable))
		{
			if (idx != wdev->DefaultKeyId)
			{
				pIoctlSec->Status = 0;
				pIoctlSec->length = 0;
				return NDIS_STATUS_FAILURE;
			}
		}
	} 
	else
		idx = wdev->DefaultKeyId;

	pIoctlSec->KeyIdx = idx + 1;

	pIoctlSec->length = 0;
	switch(wdev->WepStatus) {
		case Ndis802_11WEPDisabled:
			pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_NONE;
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_DISABLED;		
			break;
		case Ndis802_11WEPEnabled:
			pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_WEP;
			if (pAd->SharedKey[BSS0][idx].KeyLen > pIoctlSec->MaxKeyLen)
			{
				pIoctlSec->Status = RTMP_IO_E2BIG;
				return NDIS_STATUS_FAILURE;
			}
			else
			{
				pIoctlSec->length = pAd->SharedKey[BSS0][idx].KeyLen;				
				pIoctlSec->pData = (PCHAR)&(pAd->SharedKey[BSS0][idx].Key[0]);
			}
			break;
		case Ndis802_11TKIPEnable:
		case Ndis802_11AESEnable:
			if (wdev->WepStatus == Ndis802_11TKIPEnable)
				pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else
				pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;
			
			if (pIoctlSec->MaxKeyLen < 32)
			{
				pIoctlSec->Status = RTMP_IO_E2BIG;
				return NDIS_STATUS_FAILURE;
			}
			else
			{
				pIoctlSec->length = 32;
				pIoctlSec->pData = (PCHAR)&pAd->StaCfg.PMK[0];
			}
			break;
		default:
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwgenie(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WPA_SUPPLICANT_SUPPORT
	ULONG length = (ULONG)Data;


	if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
	{
		DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_siwgenie\n"));
		pAd->StaCfg.wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;
		if ((length > 0) && (pData == NULL))
		{
			return NDIS_STATUS_FAILURE;
		}
		else if (length) 
		{
			if (pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe)
			{
				os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe);
				pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe = NULL;
			}
			os_alloc_mem(NULL, (UCHAR **)&pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe, length);
			if (pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe)
			{
				pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = length;
				NdisMoveMemory(pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe, pData, pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen);
				pAd->StaCfg.wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = TRUE;
			}
			else
				pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = 0;
		}
	}
#endif /* WPA_SUPPLICANT_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWGENIE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwgenie(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_RSN_IE *IoctlRsnIe = (RT_CMD_STA_IOCTL_RSN_IE *)pData;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	if ((pAd->StaCfg.RSNIE_Len == 0) ||
		(wdev->AuthMode < Ndis802_11AuthModeWPA))
	{
		IoctlRsnIe->length = 0;
		return NDIS_STATUS_SUCCESS;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	/*
		Can not use SIOCSIWGENIE definition, it is used in wireless.h
		We will not see the definition in MODULE.
		The definition can be saw in UTIL and NETIF.
	*/
/* #ifdef SIOCSIWGENIE */
	if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE &&
		(pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen > 0))
	{
		if (IoctlRsnIe->length < pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen)
			return NDIS_STATUS_FAILURE;

		IoctlRsnIe->length = pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen;
		memcpy(IoctlRsnIe->pRsnIe, pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe, pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen);
	}
	else
/* #endif */ /* SIOCSIWGENIE */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
	{
		UCHAR RSNIe = IE_WPA;
		
		if (IoctlRsnIe->length < (pAd->StaCfg.RSNIE_Len + 2)) /* ID, Len */
			return NDIS_STATUS_FAILURE;
		IoctlRsnIe->length = pAd->StaCfg.RSNIE_Len + 2;
		
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA2PSK) ||
            		(wdev->AuthMode == Ndis802_11AuthModeWPA2))
			RSNIe = IE_RSN;
		
		IoctlRsnIe->pRsnIe[0] = (char)RSNIe;
		IoctlRsnIe->pRsnIe[1] = pAd->StaCfg.RSNIE_Len;
		memcpy(IoctlRsnIe->pRsnIe+2, &pAd->StaCfg.RSN_IE[0], pAd->StaCfg.RSNIE_Len);
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_siwpmksa(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_PMA_SA *pIoctlPmaSa = (RT_CMD_STA_IOCTL_PMA_SA *)pData;
	INT	CachedIdx = 0, idx = 0;


	switch(pIoctlPmaSa->Cmd)
	{
		case RT_CMD_STA_IOCTL_PMA_SA_FLUSH:
			NdisZeroMemory(pAd->StaCfg.SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_FLUSH\n"));
			break;
		case RT_CMD_STA_IOCTL_PMA_SA_REMOVE:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        /* compare the BSSID */
		        if (NdisEqualMemory(pIoctlPmaSa->pBssid, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
		        {
		        	NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN);
					NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].PMKID, 16);
					for (idx = CachedIdx; idx < (pAd->StaCfg.SavedPMKNum - 1); idx++)
					{
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].BSSID[0], &pAd->StaCfg.SavedPMK[idx+1].BSSID[0], MAC_ADDR_LEN);
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].PMKID[0], &pAd->StaCfg.SavedPMK[idx+1].PMKID[0], 16);
					}
					pAd->StaCfg.SavedPMKNum--;
			        break;
		        }
	        }
			
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_REMOVE\n"));
			break;
		case RT_CMD_STA_IOCTL_PMA_SA_ADD:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        /* compare the BSSID */
		        if (NdisEqualMemory(pIoctlPmaSa->pBssid, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
			        break;			
	        }

	        /* Found, replace it */
	        if (CachedIdx < PMKID_NO)
	        {
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pIoctlPmaSa->pBssid, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pIoctlPmaSa->pPmkid, 16);
		        pAd->StaCfg.SavedPMKNum++;
	        }
	        /* Not found, replace the last one */
	        else
	        {
		        /* Randomly replace one */
		        CachedIdx = (pIoctlPmaSa->pBssid[5] % PMKID_NO);
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pIoctlPmaSa->pBssid, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pIoctlPmaSa->pPmkid, 16);
	        }
			
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_ADD\n"));
			break;
		default:
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - Unknow Command!!\n"));
			break;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWRATE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_siwrate(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_RATE_SET *pCmdRate = (RT_CMD_RATE_SET *)pData;
	UINT32 rate = pCmdRate->Rate;
	UINT32 fixed = pCmdRate->Fixed;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

    /*
    	rate = -1 => auto rate
	rate = X, => fixed = 1 and rate fixed at X
    */
    if (rate == -1)
    {
        /*Auto Rate */
        wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;	
		wdev->bAutoTxRateSwitch = TRUE;
		if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) ||
			(pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
			RTMPSetDesiredRates(pAd, -1);
			
#ifdef DOT11_N_SUPPORT
			SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
	}
	else
	{
		if (fixed)
		{
			wdev->bAutoTxRateSwitch = FALSE;
			if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) ||
				(pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
				RTMPSetDesiredRates(pAd, rate);
			else
			{
				wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
#ifdef DOT11_N_SUPPORT
				SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
			}
			DBGPRINT(RT_DEBUG_TRACE,
					("rt_ioctl_siwrate::(HtMcs=%d)\n", wdev->DesiredTransmitSetting.field.MCS));
		}
		else
		{
			/* TODO: rate = X, fixed = 0 => (rates <= X) */
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWRATE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_giwrate(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
    int rate_index = 0, rate_count = 0;
	HTTRANSMIT_SETTING ht_setting; 
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

    
    if ((wdev->bAutoTxRateSwitch == FALSE) &&
        (INFRA_ON(pAd)) &&
        ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) || (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM)))
        ht_setting.word = wdev->HTPhyMode.word;
    else
        ht_setting.word = pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;
RtmpDrvRateGet(pAd,ht_setting.field.MODE,ht_setting.field.ShortGI,ht_setting.field.BW
	            ,ht_setting.field.MCS,newRateGetAntenna(ht_setting.field.MCS, ht_setting.field.MODE),
	            (UINT32 *)pData);

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIFHWADDR.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_gifhwaddr(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	memcpy(pData, pAd->CurrentAddress, ETH_ALEN);
	return NDIS_STATUS_SUCCESS;
}

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWPRIVRSSI.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_rssi(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{

        (*(CHAR *)pData) =  pAd->StaCfg.RssiSample.AvgRssi0;
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_PARAM_PRE.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_ioctl_setparam(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj;


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	{
		pObj->ioctl_if_type = INT_MAIN;
        pObj->ioctl_if = MAIN_MBSSID;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_U32_ITEM.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_private_set_wsc_u32_item(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WSC_STA_SUPPORT
	RT_CMD_STA_IOCTL_WSC_U32_ITEM *pIoctlWscU32 = (RT_CMD_STA_IOCTL_WSC_U32_ITEM *)pData;
    u32 subcmd = *(pIoctlWscU32->pUWrq);
    PWSC_PROFILE    pWscProfile = NULL;
   	u32 value = 0;

	pWscProfile = &pAd->StaCfg.WscControl.WscProfile;

    switch(subcmd)
    {
        case WSC_CREDENTIAL_COUNT:
            value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_COUNT, value = %d\n", __FUNCTION__, value));
            if (value <= 8)
            {
                pWscProfile->ProfileCnt = value;
            }
            else
                pIoctlWscU32->Status = RTMP_IO_EINVAL;
            break;        
        case WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX:
            value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX, value = %d\n", __FUNCTION__, value));
            if ((value <= 7) &&
                (value < pWscProfile->ProfileCnt))
            {
                WscWriteConfToPortCfg(pAd, &pAd->StaCfg.WscControl, &pAd->StaCfg.WscControl.WscProfile.Profile[value], TRUE);
                pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
                LinkDown(pAd, TRUE);
            }
            else
                pIoctlWscU32->Status = RTMP_IO_EINVAL;
            break;
        case WSC_SET_DRIVER_AUTO_CONNECT:
            value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_DRIVER_AUTO_CONNECT, value = %d\n", __FUNCTION__, value));
            if ((value == 0x00) || 
				(value == 0x01) || 
				(value == 0x02))
            {
                pAd->StaCfg.WscControl.WscDriverAutoConnect = value;
            }
            else
                pIoctlWscU32->Status = RTMP_IO_EINVAL;
            break;
        case WSC_SET_CONF_MODE:
            value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_CONF_MODE, value = %d\n", __FUNCTION__, value));
		if (value == 2)
			value = 4;
            switch(value)
            {
                case WSC_DISABLE:
                    Set_WscConfMode_Proc(pAd, "0");
                    break;
                case WSC_ENROLLEE:
                    Set_WscConfMode_Proc(pAd, "1");
                    break;
                case WSC_REGISTRAR:
                    Set_WscConfMode_Proc(pAd, "2");
                    break;
                default:
                    pIoctlWscU32->Status = RTMP_IO_EINVAL;
                    break;
            }
            break;
        case WSC_SET_MODE:
            value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_MODE, value = %d\n", __FUNCTION__, value));
            switch(value)
            {
                case WSC_PIN_MODE:
                    if (Set_WscMode_Proc(pAd, "1") == FALSE)
						pIoctlWscU32->Status = RTMP_IO_EINVAL;
                    break;
                case WSC_PBC_MODE:
                    if (Set_WscMode_Proc(pAd, "2") == FALSE)
						pIoctlWscU32->Status = RTMP_IO_EINVAL;
                    break;
				case WSC_SMPBC_MODE:
					if (Set_WscMode_Proc(pAd, "3") == FALSE)
						pIoctlWscU32->Status = RTMP_IO_EINVAL;
					break;
                default:
                    pIoctlWscU32->Status = RTMP_IO_EINVAL;
                    break;
            }
            break;
        case WSC_START:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_START\n", __FUNCTION__));
            Set_WscGetConf_Proc(pAd, "1");
            break;
        case WSC_STOP:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_STOP\n", __FUNCTION__));
						
            /* Disassociate the link if WPS is working. */
        	if ( INFRA_ON(pAd) && 
                 (pAd->StaCfg.WscControl.bWscTrigger == TRUE) && 
                 (pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) )
        	{
        		MLME_DISASSOC_REQ_STRUCT	DisReq;
        										
        		/* Set to immediately send the media disconnect event */
        		pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

        		DBGPRINT(RT_DEBUG_TRACE, ("disassociate with current AP \n"));
        		DisassocParmFill(pAd, &DisReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
        		MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
        					sizeof(MLME_DISASSOC_REQ_STRUCT), &DisReq, 0);

        		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
				RTMP_MLME_HANDLER(pAd);
        	}

#ifdef IWSC_SUPPORT
			if (pAd->StaCfg.BssType == BSS_ADHOC)
			{
				pAd->StaCfg.WscControl.WscConfMode = WSC_DISABLE;
				pAd->StaCfg.WscControl.WscState = WSC_STATE_INIT;
				pAd->StaCfg.WscControl.WscStatus = STATUS_WSC_IDLE;
				if (pAd->StaCfg.WscControl.bWscTrigger)
					IWSC_Stop(pAd, TRUE);
				else
					IWSC_Stop(pAd, FALSE);
			}
			else
#endif /* IWSC_SUPPORT */
			{
	        	/* Turn off WSC state matchine */
	        	WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
	        			FALSE,
#endif /* CONFIG_AP_SUPPORT */
	        			&pAd->StaCfg.WscControl);
	            pAd->StaCfg.WscControl.WscConfMode = WSC_DISABLE;
				BssTableDeleteEntry(&pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.Bssid, pAd->MlmeAux.Channel);
			}
            break;
        case WSC_GEN_PIN_CODE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_GEN_PIN_CODE\n", __FUNCTION__));
            Set_WscGenPinCode_Proc(pAd, "1");
            break;

		case WSC_AP_BAND:
			value = *(pIoctlWscU32->pUWrq + 1);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_PBC_BAND, value = %d\n", __FUNCTION__, value));
			if (value < PREFERRED_WPS_AP_PHY_TYPE_MAXIMUM)
			{
				pAd->StaCfg.WscControl.WpsApBand= value;
			}
			break;
			
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d, value = %d\n", __FUNCTION__, subcmd, value));
            break;
    }
#endif /* WSC_STA_SUPPORT */
    
    return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_STR_ITEM.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_private_set_wsc_string_item(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WSC_STA_SUPPORT
	RT_CMD_STA_IOCTL_WSC_STR_ITEM *pIoctlWscStr = (RT_CMD_STA_IOCTL_WSC_STR_ITEM *)pData;
/*    int  Status=0; */
    UINT32 subcmd = pIoctlWscStr->Subcmd;
    UINT32 tmpProfileIndex = (UINT32)(pIoctlWscStr->pData[0] - 0x30);
    UINT32 dataLen;
    PWSC_PROFILE    pWscProfile = NULL;
    USHORT  tmpAuth = 0, tmpEncr = 0;
	char *extra = (char *)pIoctlWscStr->pData;

	pWscProfile = &pAd->StaCfg.WscControl.WscProfile;

    if ((subcmd != WSC_SET_SSID) && 
		(subcmd != WSC_SET_PIN) &&
		(subcmd != WSC_SET_BSSID) &&
        (tmpProfileIndex > 7))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - subcmd = %d, tmpProfileIndex = %d\n", __FUNCTION__, subcmd, tmpProfileIndex));
		pIoctlWscStr->Status = RTMP_IO_EINVAL;
        return NDIS_STATUS_SUCCESS;
    }

    if ((subcmd != WSC_SET_SSID) && 
		(subcmd != WSC_SET_PIN) &&
		(subcmd != WSC_SET_BSSID))
    /* extra: "1 input_string", dwrq->length includes '\0'. 3 is size of [index, blank and '\0'] */
		dataLen = pIoctlWscStr->length - 3;
    else
        dataLen = pIoctlWscStr->length;
    
    switch(subcmd)
    {
        case WSC_CREDENTIAL_SSID:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_SSID(%s)\n", __FUNCTION__, extra+2));
			if (dataLen == (NDIS_802_11_LENGTH_SSID+1))
				dataLen = NDIS_802_11_LENGTH_SSID;
            if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID)
            {
                pWscProfile->Profile[tmpProfileIndex].SSID.SsidLength = dataLen;
                NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, NDIS_802_11_LENGTH_SSID);
                NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, extra+2, dataLen);
            }
            else
				pIoctlWscStr->Status = RTMP_IO_E2BIG;
            break;
        case WSC_CREDENTIAL_AUTH_MODE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_AUTH_MODE(%s)\n", __FUNCTION__, extra+2));
            if ((tmpAuth = WscGetAuthTypeFromStr(extra+2)) != 0)
            {
                pWscProfile->Profile[tmpProfileIndex].AuthType = tmpAuth;
            }
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
            break;
        case WSC_CREDENTIAL_ENCR_TYPE:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_ENCR_TYPE(%s)\n", __FUNCTION__, extra+2));
            if ((tmpEncr = WscGetEncrypTypeFromStr(extra+2)) != 0)
            {
                pWscProfile->Profile[tmpProfileIndex].EncrType = tmpEncr;
            }
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
            break;
        case WSC_CREDENTIAL_KEY_INDEX:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_KEY_INDEX(%s)\n", __FUNCTION__, extra+2));
            if ( *(extra+2) >= 0x31 && *(extra+2) <= 0x34)
            {
                pWscProfile->Profile[tmpProfileIndex].KeyIndex = (UCHAR)*(extra+2) - 0x30;
            }
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
            break;
        case WSC_CREDENTIAL_KEY:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_KEY(%s)\n", __FUNCTION__, extra+2));
            if ((dataLen >= 8 && dataLen <= 64) ||
                (dataLen == 5 || dataLen == 10 || dataLen == 13 || dataLen == 26))
            {
                pWscProfile->Profile[tmpProfileIndex].KeyLength = dataLen;
                NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].Key, 64);
                NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].Key, extra+2, dataLen);
            }
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
            break;
        case WSC_CREDENTIAL_MAC:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_CREDENTIAL_MAC(%s)\n", __FUNCTION__, extra+2));
            {
                INT sscanf_rv = 0;
                UINT tmp_val[6] = {0};
                sscanf_rv = sscanf(extra+2, "%02x:%02x:%02x:%02x:%02x:%02x", 
                                                          &tmp_val[0],
                                                          &tmp_val[1],
                                                          &tmp_val[2],
                                                          &tmp_val[3],
                                                          &tmp_val[4],
                                                          &tmp_val[5]);
                if ( sscanf_rv == 6)
                {
                    int ii;
                    NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].MacAddr, 6);
                    for (ii=0; ii<6; ii++)
                        pWscProfile->Profile[tmpProfileIndex].MacAddr[ii] = (UCHAR)tmp_val[ii];
                }
                else
                    pIoctlWscStr->Status = RTMP_IO_EINVAL;
            }            
            break;
        case WSC_SET_SSID:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_SSID(%s)\n", __FUNCTION__, extra));
			if (dataLen == (NDIS_802_11_LENGTH_SSID+1))
				dataLen = NDIS_802_11_LENGTH_SSID;
            if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID)
            {
            	Set_WscSsid_Proc(pAd, (PSTRING) extra);
            }
            else
				pIoctlWscStr->Status = RTMP_IO_E2BIG;
            break;
		case WSC_SET_PIN:
        	DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_PIN, value = (%s)\n", __FUNCTION__, extra));
			if ( dataLen > 0 )
			{
				if (Set_WscPinCode_Proc(pAd, extra) == FALSE)
					pIoctlWscStr->Status = RTMP_IO_EINVAL;
			}
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
            break;
		case WSC_SET_BSSID:
			if ( dataLen > 0 )
			{
				if (Set_WscBssid_Proc(pAd, (PSTRING) extra) == FALSE)
					pIoctlWscStr->Status = RTMP_IO_EINVAL;
			}
            else
                pIoctlWscStr->Status = RTMP_IO_EINVAL;
			DBGPRINT(RT_DEBUG_TRACE, ("%s - WSC_SET_BSSID\n", __FUNCTION__));
			break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d\n", __FUNCTION__, subcmd));
            break;
    }
#endif /* WSC_STA_SUPPORT */
    
    return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_IW_GET_STATISTICS.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT
RtmpIoctl_rt_private_get_statistics(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	char *extra = (char *)pData;
	ULONG txCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per, plr;
#endif
#ifdef WSC_STA_SUPPORT
	UINT32 MaxSize = (UINT32)Data;
#endif /* WSC_STA_SUPPORT */


    sprintf(extra, "\n\n");

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		txCount = pAd->ate.TxDoneCount;
	else
#endif /* RALINK_ATE */
		txCount = (ULONG)pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;

    sprintf(extra+strlen(extra), "Tx success                      = %lu\n", txCount);
#ifdef ENHANCED_STAT_DISPLAY
	per = txCount==0? 0: 1000*(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart)/(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(extra+strlen(extra), "Tx retry count                  = %lu, PER=%ld.%1ld%%\n",
									(ULONG)pAd->WlanCounters.RetryCount.u.LowPart,
									per/10, per % 10);
	plr = txCount==0? 0: 10000*pAd->WlanCounters.FailedCount.u.LowPart/(pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(extra+strlen(extra), "Tx fail to Rcv ACK after retry  = %lu, PLR=%ld.%02ld%%\n",
									(ULONG)pAd->WlanCounters.FailedCount.u.LowPart, plr/100, plr%100);
#else
    sprintf(extra+strlen(extra), "Tx retry count          		  = %lu\n", (ULONG)pAd->WlanCounters.RetryCount.u.LowPart);
    sprintf(extra+strlen(extra), "Tx fail to Rcv ACK after retry  = %lu\n", (ULONG)pAd->WlanCounters.FailedCount.u.LowPart);
    sprintf(extra+strlen(extra), "RTS Success Rcv CTS             = %lu\n", (ULONG)pAd->WlanCounters.RTSSuccessCount.u.LowPart);
    sprintf(extra+strlen(extra), "RTS Fail Rcv CTS                = %lu\n", (ULONG)pAd->WlanCounters.RTSFailureCount.u.LowPart);
#endif /* ENHANCED_STAT_DISPLAY */

    sprintf(extra+strlen(extra), "Rx success                      = %lu\n", (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart);
#ifdef ENHANCED_STAT_DISPLAY
	per = pAd->WlanCounters.ReceivedFragmentCount.u.LowPart==0? 0: 1000*(pAd->WlanCounters.FCSErrorCount.u.LowPart)/(pAd->WlanCounters.FCSErrorCount.u.LowPart+pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
    sprintf(extra+strlen(extra), "Rx with CRC                     = %ld, PER=%ld.%1ld%%\n",
										(ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart, per/10, per % 10);
    sprintf(extra+strlen(extra), "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(extra+strlen(extra), "Rx duplicate frame              = %lu\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

    sprintf(extra+strlen(extra), "False CCA                       = %lu\n", (ULONG)pAd->RalinkCounters.FalseCCACnt);
#else
    sprintf(extra+strlen(extra), "Rx with CRC                     = %lu\n", (ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart);
    sprintf(extra+strlen(extra), "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(extra+strlen(extra), "Rx duplicate frame              = %lu\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

    sprintf(extra+strlen(extra), "False CCA (one second)          = %lu\n", (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt);
#endif /* ENHANCED_STAT_DISPLAY */

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		if (pAd->ate.RxAntennaSel == 0)
		{
    		sprintf(extra+strlen(extra), "RSSI-A                          = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
			sprintf(extra+strlen(extra), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->ate.LastRssi1 - pAd->BbpRssiToDbmDelta));
			sprintf(extra+strlen(extra), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->ate.LastRssi2 - pAd->BbpRssiToDbmDelta));
		}
		else
		{
    		sprintf(extra+strlen(extra), "RSSI                            = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
		}
	}
	else
#endif /* RALINK_ATE */
	{
#ifdef ENHANCED_STAT_DISPLAY
		sprintf(extra+strlen(extra), "RSSI                            = %ld %ld %ld\n",
				(LONG)(pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta),
				(LONG)(pAd->StaCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta),
				(LONG)(pAd->StaCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));

		// Display Last Rx Rate and BF SNR of first Associated entry in MAC table
		if (pAd->MacTab.Size > 0)
		{
			static char *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};
			static char *bw[3] = {"20M", "40M", "80M"};
			static char *fec_coding[2] = {"bcc", "ldpc"};
			int i;
    		    	
			for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
			{
				PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);
				if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst==SST_ASSOC)
				{
					UINT32 lastRxRate = pEntry->LastRxRate;
					UINT32 lastTxRate = pEntry->LastTxRate;
			
#ifdef RT65xx			
					if (IS_RT65XX(pAd)) {
						if (((lastTxRate >> 13) & 0x7) == 0x04) {
							sprintf(extra+strlen(extra), "Last TX Rate                    = MCS%d, %dSS, %s, %s, %cGI, %s%s\n",
							lastTxRate & 0x0F,
							(((lastTxRate >> 4) & 0x3) + 1),
							fec_coding[((lastTxRate >> 6) & 0x1)],
							bw[((lastTxRate >> 7) & 0x3)],
							((lastTxRate >> 9) & 0x1)? 'S': 'L',
							phyMode[(lastTxRate >> 13) & 0x7],
							((lastTxRate >> 10) & 0x3)? ", STBC": " ");

						} else {
							sprintf(extra+strlen(extra), "Last TX Rate                    = MCS%d, %s, %s, %cGI, %s%s\n",
							lastTxRate & 0x3F,
							fec_coding[((lastTxRate >> 6) & 0x1)],	
							bw[((lastTxRate >> 7) & 0x3)],
							((lastTxRate >> 9) & 0x1)? 'S': 'L',
							phyMode[(lastTxRate >> 13) & 0x7],
							((lastTxRate >> 10) & 0x3)? ", STBC": " ");
						}
						
						if (((lastRxRate >> 13) & 0x7) == 0x04) {
							sprintf(extra+strlen(extra), "Last RX Rate                    = MCS%d, %dSS, %s, %s, %cGI, %s%s\n",
								lastRxRate & 0x0F,
								(((lastRxRate >> 4) & 0x3) + 1),
								fec_coding[((lastRxRate >> 6) & 0x1)],
								bw[((lastRxRate >> 7) & 0x3)],
								((lastRxRate >> 9) & 0x1)? 'S': 'L',
								phyMode[(lastRxRate >> 13) & 0x7],
								((lastRxRate >> 10) & 0x3)? ", STBC": " ");
						} else {
							sprintf(extra+strlen(extra), "Last RX Rate                    = MCS%d, %s, %s, %cGI, %s%s\n",
								lastRxRate & 0x3F,
								fec_coding[((lastRxRate >> 6) & 0x1)],	
								bw[((lastRxRate >> 7) & 0x3)],
								((lastRxRate >> 9) & 0x1)? 'S': 'L',
								phyMode[(lastRxRate >> 13) & 0x7],
								((lastRxRate >> 10) & 0x3)? ", STBC": " ");
						}
					}

					else
#endif
					{
						sprintf(extra+strlen(extra), "Last RX Rate                    = MCS %d, %2dM, %cGI, %s%s\n",
						lastRxRate & 0x7F,  ((lastRxRate>>7) & 0x1)? 40: 20,
						((lastRxRate>>8) & 0x1)? 'S': 'L',
						phyMode[(lastRxRate>>14) & 0x3],
						((lastRxRate>>9) & 0x3)? ", STBC": " ");
					}
					break;

				}
			}
		}
#else    		    	
		sprintf(extra+strlen(extra), "RSSI-A                          = %ld\n", (LONG)(pAd->StaCfg.RssiSample.AvgRssi0 - pAd->BbpRssiToDbmDelta));
		sprintf(extra+strlen(extra), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->StaCfg.RssiSample.AvgRssi1 - pAd->BbpRssiToDbmDelta));
        	sprintf(extra+strlen(extra), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->StaCfg.RssiSample.AvgRssi2 - pAd->BbpRssiToDbmDelta));
#endif /* ENHANCED_STAT_DISPLAY */

		sprintf(extra+strlen(extra), "SNR-A                          = %ld\n", (LONG)(pAd->StaCfg.RssiSample.AvgSnr0));
        	sprintf(extra+strlen(extra), "SNR-B (if available)           = %ld\n\n", (LONG)(pAd->StaCfg.RssiSample.AvgSnr1));
	}

#ifdef WPA_SUPPLICANT_SUPPORT
    sprintf(extra+strlen(extra), "WpaSupplicantUP                 = %d\n\n", pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	sprintf(extra+strlen(extra), "FtSupport                       = %d\n\n", pAd->StaCfg.Dot11RCommInfo.bFtSupport);
#endif /* DOT11R_FT_SUPPORT */

#ifdef WSC_STA_SUPPORT
	/* display pin code */
	if (pAd->StaCfg.WscControl.WscEnrolleePinCodeLen == 8)	
		sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%08u\n", pAd->StaCfg.WscControl.WscEnrolleePinCode);
	else
		sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%04u\n", pAd->StaCfg.WscControl.WscEnrolleePinCode);
{
	char	mode_str[16]={0};
	ULONG	wps_status, wps_state;
    int     idx = 0;

	wps_state = pAd->StaCfg.WscControl.WscState;
	wps_status = pAd->StaCfg.WscControl.WscStatus;
	
	if (pAd->StaCfg.WscControl.WscMode == WSC_PIN_MODE)
		sprintf(mode_str, "PIN -");
	else
		sprintf(mode_str, "PBC -");
	
		sprintf(extra+strlen(extra), "WPS Information(Driver Auto-Connect is %s - %d):\n",
	                                                  pAd->StaCfg.WscControl.WscDriverAutoConnect ? "Enabled":"Disabled",
	                                                  pAd->StaCfg.WscControl.WscDriverAutoConnect);
	/* display pin code */
	/*sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%08u\n", pAd->StaCfg.WscControl.WscEnrolleePinCode); */
	/* display status */
	if ((wps_state == WSC_STATE_OFF) || (wps_status & 0xff00))
	{
		if (wps_status == STATUS_WSC_CONFIGURED)
		{
			sprintf(extra+strlen(extra), "WPS messages exchange successfully !!!\n");
		}
		else if ((wps_status == STATUS_WSC_NOTUSED))
		{
			sprintf(extra+strlen(extra), "WPS not used.\n");
		}
		else if(wps_status & 0xff00)	/* error message */
		{
			if (wps_status == STATUS_WSC_PBC_TOO_MANY_AP)
				sprintf(extra+strlen(extra), "%s Too many PBC AP. Stop WPS. \n", mode_str);
			else if (wps_status == STATUS_WSC_PBC_NO_AP)
				sprintf(extra+strlen(extra), "%s No available PBC AP. Please wait... \n", mode_str);
			else if (wps_status & 0x0100)
				sprintf(extra+strlen(extra), "%s Proceed to get the Registrar profile. Please wait... \n", mode_str);
			else	/* status of eap failed */
				sprintf(extra+strlen(extra), "WPS didn't complete !!!\n");
		}
		else
		{
			/* wrong state */
		}
	}
	else
	{
		sprintf(extra+strlen(extra), "%s WPS Proceed. Please wait... \n", mode_str);
	}
	sprintf(extra+strlen(extra), "\n");
    sprintf(extra+strlen(extra), "WPS Profile Count               = %d\n", pAd->StaCfg.WscControl.WscProfile.ProfileCnt);
    for (idx = 0; idx < pAd->StaCfg.WscControl.WscProfile.ProfileCnt ; idx++)
    {
        PWSC_CREDENTIAL pCredential = &pAd->StaCfg.WscControl.WscProfile.Profile[idx];
		char ssid_print[MAX_LEN_OF_SSID + 1];
		NdisZeroMemory(&ssid_print[0], MAX_LEN_OF_SSID + 1);
        if (strlen(extra) + sizeof(WSC_CREDENTIAL) >= MaxSize)
        {
            break;
        }
        
        sprintf(extra+strlen(extra), "Profile[%d]:\n", idx);        
		NdisMoveMemory(&ssid_print[0], pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
        sprintf(extra+strlen(extra), "SSID                            = %s\n", ssid_print);
        sprintf(extra+strlen(extra), "MAC                             = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                                                                           pCredential->MacAddr[0],
                                                                           pCredential->MacAddr[1],
                                                                           pCredential->MacAddr[2],
                                                                           pCredential->MacAddr[3],
                                                                           pCredential->MacAddr[4],
                                                                           pCredential->MacAddr[5]);
        sprintf(extra+strlen(extra), "AuthType                        = %s\n", WscGetAuthTypeStr(pCredential->AuthType));
        sprintf(extra+strlen(extra), "EncrypType                      = %s\n", WscGetEncryTypeStr(pCredential->EncrType)); 
        sprintf(extra+strlen(extra), "KeyIndex                        = %d\n", pCredential->KeyIndex);
        if (pCredential->KeyLength != 0)
        {
            if (pCredential->AuthType & (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_WPANONE))
            {
            if (pCredential->KeyLength < 64)
                sprintf(extra+strlen(extra), "Key                             = %s\n", pCredential->Key);
            else
            {
                char key_print[65] = {0};
                NdisMoveMemory(key_print, pCredential->Key, 64);
                sprintf(extra+strlen(extra), "Key                             = %s\n", key_print);
            }
        }
            else if ((pCredential->AuthType == WSC_AUTHTYPE_OPEN) ||
                     (pCredential->AuthType == WSC_AUTHTYPE_SHARED))
            {
                /*check key string is ASCII or not */
                if (RTMPCheckStrPrintAble((PCHAR)pCredential->Key, (UCHAR)pCredential->KeyLength))
                    sprintf(extra+strlen(extra), "Key                             = %s\n", pCredential->Key);
                else
                {
                    int idx;
                    sprintf(extra+strlen(extra), "Key                             = ");
                    for (idx = 0; idx < pCredential->KeyLength; idx++)
                        sprintf(extra+strlen(extra), "%02X", pCredential->Key[idx]);
                    sprintf(extra+strlen(extra), "\n");
                }
            }
        }
#ifdef IWSC_SUPPORT
		if (pAd->StaCfg.BssType == BSS_ADHOC)
		{
			PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
			sprintf(extra+strlen(extra), "Credential Registrar IPv4 Addr  = %d:%d:%d:%d\n", 			
				(pCredential->RegIpv4Addr & 0xFF000000)>>24,
				(pCredential->RegIpv4Addr & 0x00FF0000)>>16,
				(pCredential->RegIpv4Addr & 0x0000FF00)>>8,
				(pCredential->RegIpv4Addr & 0x000000FF));
			sprintf(extra+strlen(extra), "Credential Entrollee IPv4 Addr  = %d:%d:%d:%d\n", 			
				(pCredential->EnrIpv4Addr & 0xFF000000)>>24,
				(pCredential->EnrIpv4Addr & 0x00FF0000)>>16,
				(pCredential->EnrIpv4Addr & 0x0000FF00)>>8,
				(pCredential->EnrIpv4Addr & 0x000000FF));
			sprintf(extra+strlen(extra), "\nSelf IPv4 Addr                  = %d:%d:%d:%d\n", 
				(pIWscInfo->SelfIpv4Addr & 0xFF000000)>>24,
				(pIWscInfo->SelfIpv4Addr & 0x00FF0000)>>16,
				(pIWscInfo->SelfIpv4Addr & 0x0000FF00)>>8,
				(pIWscInfo->SelfIpv4Addr & 0x000000FF));
			sprintf(extra+strlen(extra), "IPv4 Subnet Mask                = %d:%d:%d:%d\n", 
				(pIWscInfo->Ipv4SubMask & 0xFF000000)>>24,
				(pIWscInfo->Ipv4SubMask & 0x00FF0000)>>16,
				(pIWscInfo->Ipv4SubMask & 0x0000FF00)>>8,
				(pIWscInfo->Ipv4SubMask & 0x000000FF));
			sprintf(extra+strlen(extra), "AvaSubMaskListCount             = %d", pIWscInfo->AvaSubMaskListCount);
		}
#endif /* IWSC_SUPPORT */
    }
    sprintf(extra+strlen(extra), "\n");
}
#endif /* WSC_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* Display Tx Aggregation statistics */
	DisplayTxAgg(pAd);
#endif /* DOT11_N_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Communication with DRIVER module, whatever IOCTL.

Arguments:
	pAdSrc			- WLAN control block pointer
	*pRequest		- the request from IOCTL
	Command			- communication command
	Subcmd			- communication sub-command
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RTMP_STA_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*pRequest,
	IN	INT						Command,
	IN	USHORT					Subcmd,
	IN	VOID					*pData,
	IN  ULONG					Data,
	IN  USHORT                  priv_flags)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS;

	{	/* determine this ioctl command is comming from which interface. */
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
	}


	/* handle by command */
	switch(Command)
	{
		case CMD_RT_PRIV_IOCTL:
			if (Subcmd & OID_GET_SET_TOGGLE)
				Status = RTMPSetInformation(pAd, pRequest,  Subcmd);
			else
				Status = RTMPQueryInformation(pAd, pRequest, Subcmd);
			break;

		case CMD_RTPRIV_IOCTL_PARAM_SET:
		{
			RT_CMD_PARAM_SET *pCmdParam = (RT_CMD_PARAM_SET *)pData;
			PSTRING this_char = pCmdParam->pThisChar;
			PSTRING value = pCmdParam->pValue;

			Status = RTMPSTAPrivIoctlSet(pAd, this_char, value);
		}
			break;

		case CMD_RTPRIV_IOCTL_SITESURVEY_GET:
			RTMPIoctlGetSiteSurvey(pAd, pRequest);
			break;

		case CMD_RTPRIV_IOCTL_MAC:
			RTMPIoctlMAC(pAd, pRequest);
			break;

		case CMD_RTPRIV_IOCTL_E2P:
			RTMPIoctlE2PROM(pAd, pRequest);
			break;

		case CMD_RTPRIV_IOCTL_RF:
#ifdef RTMP_RF_RW_SUPPORT
			RTMPIoctlRF(pAd, pRequest);
#endif /* RTMP_RF_RW_SUPPORT */
			break;

		case CMD_RTPRIV_IOCTL_BBP:
			RTMPIoctlBbp(pAd, pRequest, pData, Data);
			break;

		case CMD_RTPRIV_IOCTL_SHOW:
			RTMPIoctlShow(pAd, pRequest, Subcmd, pData, Data);
			break;

		case CMD_RTPRIV_IOCTL_SITESURVEY:
			StaSiteSurvey(pAd, (NDIS_802_11_SSID *)pData, Data);
			break;

		case CMD_RTPRIV_IOCTL_CHID_2_FREQ:
			RTMP_MapChannelID2KHZ(Data, (UINT32 *)pData);
			break;

		case CMD_RTPRIV_IOCTL_FREQ_2_CHID:
			RTMP_MapKHZ2ChannelID(Data, (UINT32 *)pData);
			break;

		case CMD_RTPRIV_IOCTL_ORI_DEV_TYPE_SET:
			pAd->StaCfg.OriDevType = Data;
#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
	NdisAllocateSpinLock(pAd, &pAd->StaCtIf.Lock);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */
			
			break;
		case CMD_RTPRIV_IOCTL_STA_SCAN_SANITY_CHECK:
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		    {
				/*
				 * Still scanning, indicate the caller should try again.
				 */
				pAd->StaCfg.bSkipAutoScanConn = TRUE;
				return NDIS_STATUS_FAILURE;
			}

			if (pAd->StaCfg.bImprovedScan)
			{
				/*
				 * Fast scanning doesn't complete yet.
				 */
				pAd->StaCfg.bSkipAutoScanConn = TRUE;
				return NDIS_STATUS_FAILURE;
			}
			break;

		case CMD_RTPRIV_IOCTL_STA_SCAN_END:
		    pAd->StaCfg.bSkipAutoScanConn = FALSE;
			DBGPRINT(RT_DEBUG_ERROR ,("===>rt_ioctl_giwscan. %d(%d) BSS returned, data->length = %ld\n",pAd->ScanTab.BssNr , pAd->ScanTab.BssNr, Data));
			break;

		case CMD_RTPRIV_IOCTL_BSS_LIST_GET:
		{
			RT_CMD_STA_IOCTL_BSS_LIST *pBssList = (RT_CMD_STA_IOCTL_BSS_LIST *)pData;
			RT_CMD_STA_IOCTL_BSS *pList;
			UINT32 i;

			pBssList->BssNum = pAd->ScanTab.BssNr;
			for (i = 0; i <pBssList->MaxNum ; i++)
			{
				if (i >=  pAd->ScanTab.BssNr)
					break;
				pList = (pBssList->pList) + i;
				set_quality(pList, &pAd->ScanTab.BssEntry[i]);
			}
		}
			break;

		/* ------------------------------------------------------------------ */
		/* for standard IOCTL in LINUX OS */

		RTMP_STA_STANDARD_IOCTL_HANDLE(pAd, pData, Data, Subcmd);

		/* ------------------------------------------------------------------ */

		default:
			/* for IOCTL that also can be used in AP mode */
			Status = RTMP_COM_IoctlHandle(pAd, pRequest, Command, Subcmd, pData, Data);
			break;
	}

	return Status;
}

